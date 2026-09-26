#include "Renderer.h"
#include <algorithm>
#include <map>

namespace {

// Цвета консоли Windows: 7 - серый, 8 - тёмно-серый, 10 - зелёный,
// 11 - голубой, 13 - розовый, 14 - жёлтый, 15 - белый
constexpr int kTitleColor = 15;
constexpr int kTextColor = 7;
constexpr int kHintColor = 8;
constexpr int kHeaderColor = 14;
constexpr int kArtColor = 11;
constexpr int kWinColor = 10;
constexpr int kLoseColor = 13;
constexpr int kNewMessageColor = 15;
constexpr int kOldMessageColor = 8;

constexpr int kLeftMargin = 2;
constexpr int kTopMargin = 1;
constexpr int kPanelGap = 4;
constexpr int kBarLength = 20;

std::string repeat(const std::string& text, int count) {
    std::string result;
    for (int i = 0; i < count; ++i) result += text;
    return result;
}

std::string heroSymbolName(Direction facing) {
    switch (facing) {
    case Direction::kUp:    return "hero_up";
    case Direction::kLeft:  return "hero_left";
    case Direction::kRight: return "hero_right";
    default:                return "hero_down";
    }
}

} // namespace

Renderer::Renderer(Console& target, const SymbolTable& symbol_table, const KeyValueFile& text_table)
    : console(target), symbols(symbol_table), texts(text_table) {
}

void Renderer::drawTitleScreen(const std::vector<std::string>& art, const std::vector<std::string>& story) {
    std::vector<Line> lines;
    lines.push_back({ { texts.getString("title"), kTitleColor } });
    lines.push_back({});
    for (const std::string& row : art) lines.push_back({ { row, kArtColor } });
    lines.push_back({});
    for (const std::string& row : story) lines.push_back({ { row, kTextColor } });
    lines.push_back({});
    lines.push_back({ { texts.getString("press_to_start"), kHintColor } });
    flush(lines);
}

void Renderer::drawGame(const TileMap& map, const Hero& hero,
                        const std::vector<std::unique_ptr<SearchableObject>>& objects,
                        const std::vector<MapItem>& items, const std::deque<std::string>& messages) {
    std::vector<Line> lines;
    lines.push_back({ { texts.getString("title"), kTitleColor } });
    lines.push_back(buildEnergyLine(hero));
    lines.push_back({});

    // Слева карта, справа легенда и инвентарь
    std::vector<Line> map_lines = buildMapLines(map, hero, objects, items);
    std::vector<Line> side_lines = buildSideLines(hero.getInventory());
    std::string empty_map_row(map.getWidth() * SymbolTable::kCellWidth, ' ');
    size_t row_count = std::max(map_lines.size(), side_lines.size());
    for (size_t row = 0; row < row_count; ++row) {
        Line line = row < map_lines.size() ? map_lines[row] : Line{ { empty_map_row, kTextColor } };
        line.push_back({ std::string(kPanelGap, ' '), kTextColor });
        if (row < side_lines.size()) line.insert(line.end(), side_lines[row].begin(), side_lines[row].end());
        lines.push_back(line);
    }

    lines.push_back({});
    lines.push_back({ { texts.getString("controls"), kTextColor } });
    lines.push_back({});
    for (size_t i = 0; i < messages.size(); ++i) {
        bool is_newest = i + 1 == messages.size();
        lines.push_back({ { "> " + messages[i], is_newest ? kNewMessageColor : kOldMessageColor } });
    }
    flush(lines);
}

void Renderer::drawEndScreen(const std::vector<std::string>& art, const std::string& message, bool is_victory) {
    std::vector<Line> lines;
    lines.push_back({ { texts.getString("title"), kTitleColor } });
    lines.push_back({});
    for (const std::string& row : art) lines.push_back({ { row, is_victory ? kWinColor : kLoseColor } });
    lines.push_back({});
    lines.push_back({ { message, kTitleColor } });
    lines.push_back({});
    lines.push_back({ { texts.getString("press_to_exit"), kHintColor } });
    flush(lines);
}

Renderer::Line Renderer::buildEnergyLine(const Hero& hero) const {
    int energy = std::max(0, hero.getHealthPoints());
    int max_energy = std::max(1, hero.getMaxEnergy());
    // Округляем вверх: пока энергия есть, в полоске есть хотя бы одно деление
    int filled = std::min(kBarLength, (energy * kBarLength + max_energy - 1) / max_energy);
    double ratio = static_cast<double>(energy) / max_energy;
    const Symbol& full = symbols.get(ratio > 0.5 ? "energy_high" : (ratio > 0.25 ? "energy_mid" : "energy_low"));
    const Symbol& empty = symbols.get("energy_empty");

    Line line;
    line.push_back({ texts.getString("energy_label") + " [", kTextColor });
    line.push_back({ repeat(full.glyph, filled), full.color });
    line.push_back({ repeat(empty.glyph, kBarLength - filled), empty.color });
    line.push_back({ "] " + std::to_string(energy) + "/" + std::to_string(hero.getMaxEnergy()), kTextColor });
    return line;
}

std::vector<Renderer::Line> Renderer::buildMapLines(const TileMap& map, const Hero& hero,
                                                    const std::vector<std::unique_ptr<SearchableObject>>& objects,
                                                    const std::vector<MapItem>& items) const {
    // Сначала стены и пол, поверх - предметы, объекты и герой
    std::vector<std::vector<const Symbol*>> cells(map.getHeight(),
                                                  std::vector<const Symbol*>(map.getWidth(), nullptr));
    for (int y = 0; y < map.getHeight(); ++y) {
        for (int x = 0; x < map.getWidth(); ++x) {
            if (map.getCell(x, y) == TileMap::kWall) cells[y][x] = &symbols.get("wall");
            else if (map.isWalkable(x, y)) cells[y][x] = &symbols.get("floor");
        }
    }

    auto place = [&](int x, int y, const std::string& symbol_name) {
        if (y >= 0 && y < map.getHeight() && x >= 0 && x < map.getWidth()) cells[y][x] = &symbols.get(symbol_name);
    };
    for (const MapItem& map_item : items) place(map_item.grid_x, map_item.grid_y, map_item.item->getSymbolName());
    for (const auto& object : objects) {
        if (object->getIsActive()) place(object->getGridX(), object->getGridY(), object->getSymbolName());
    }
    place(hero.getGridX(), hero.getGridY(), heroSymbolName(hero.getFacing()));

    std::vector<Line> lines;
    for (const auto& row : cells) {
        Line line;
        for (const Symbol* cell : row) {
            if (cell != nullptr) line.push_back({ cell->cell, cell->color });
            else line.push_back({ std::string(SymbolTable::kCellWidth, ' '), kTextColor });
        }
        lines.push_back(line);
    }
    return lines;
}

std::vector<Renderer::Line> Renderer::buildSideLines(const Inventory& inventory) const {
    std::vector<Line> lines;
    lines.push_back({ { texts.getString("legend_header"), kHeaderColor } });

    // Символы с одинаковым пояснением показываем в одной строке: "^ < v > You"
    std::vector<std::string> legend_order;
    std::map<std::string, Line> legend_symbols;
    for (const std::string& name : symbols.getOrder()) {
        const Symbol& symbol = symbols.get(name);
        if (symbol.legend.empty() || symbol.legend == "-") continue;
        if (legend_symbols.find(symbol.legend) == legend_symbols.end()) legend_order.push_back(symbol.legend);
        legend_symbols[symbol.legend].push_back({ symbol.cell, symbol.color });
    }
    for (const std::string& legend : legend_order) {
        Line line = legend_symbols[legend];
        line.push_back({ " " + legend, kTextColor });
        lines.push_back(line);
    }

    lines.push_back({});
    lines.push_back({ { texts.getString("inventory_header"), kHeaderColor } });
    if (inventory.getSize() == 0) lines.push_back({ { texts.getString("inventory_empty"), kHintColor } });
    for (int slot = 0; slot < inventory.getSize(); ++slot) {
        const Item* item = inventory.getItem(slot);
        std::string text = texts.format("inventory_item", { { "slot", std::to_string(slot + 1) },
                                                            { "name", item->getName() },
                                                            { "description", item->getDescription() } });
        lines.push_back({ { text, kTextColor } });
    }
    return lines;
}

void Renderer::flush(const std::vector<Line>& lines) {
    for (size_t row = 0; row < lines.size(); ++row) {
        console.moveTo(kLeftMargin, kTopMargin + static_cast<int>(row));
        for (const Segment& segment : lines[row]) console.write(segment.text, segment.color);
        console.clearRestOfLine();
    }
    // Если прошлый кадр был длиннее, стираем его хвост
    for (size_t row = lines.size(); row < last_line_count; ++row) {
        console.moveTo(0, kTopMargin + static_cast<int>(row));
        console.clearRestOfLine();
    }
    last_line_count = lines.size();
}
