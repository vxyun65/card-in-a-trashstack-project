#include "Game.h"

#include <algorithm>

#include "AssetFile.h"
#include "Closet.h"
#include "CombatSystem.h"
#include "EnergyDrink.h"
#include "Pile.h"

namespace {

// Символы на карте в assets/map.txt
constexpr char kHeroCell = 'H';
constexpr char kPileCell = 'P';
constexpr char kClosetCell = 'C';
constexpr char kSmallDrinkCell = 'e';
constexpr char kBigDrinkCell = 'E';

// Всё, что игра берёт из внешних файлов, - проверяем при запуске
const std::vector<std::string> kSettingKeys = {
    "max_energy",      "step_energy_cost",    "search_power",
    "card_chance",     "pile_clutter",        "pile_energy_drain",
    "closet_clutter",  "closet_energy_drain", "small_drink_energy",
    "big_drink_energy"};

const std::vector<std::string> kTextKeys = {"window_title",
                                            "title",
                                            "press_to_start",
                                            "press_to_exit",
                                            "energy_label",
                                            "legend_header",
                                            "inventory_header",
                                            "inventory_empty",
                                            "inventory_item",
                                            "controls",
                                            "start_hint",
                                            "blocked",
                                            "finish_search_first",
                                            "nothing_to_search",
                                            "search_start",
                                            "search_continue",
                                            "search_nothing",
                                            "pile_name",
                                            "closet_name",
                                            "small_drink_name",
                                            "big_drink_name",
                                            "drink_description",
                                            "pickup",
                                            "inventory_full",
                                            "use_item",
                                            "item_not_needed",
                                            "no_item",
                                            "win",
                                            "lose"};

const std::vector<std::string> kSymbolNames = {
    "hero_up",    "hero_left",  "hero_down",   "hero_right", "pile",
    "closet",     "drink",      "wall",        "floor",      "energy_high",
    "energy_mid", "energy_low", "energy_empty"};

void addMissing(std::vector<std::string>& errors, const std::string& file_name,
                const std::vector<std::string>& missing) {
  for (const std::string& key : missing)
    errors.push_back(file_name + ": no \"" + key + "\"");
}

struct GridPosition {
  int x;
  int y;
};

constexpr GridPosition kSteps[] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};

// Клетки, до которых герой может дойти со старта (сама стартовая не считается).
// Обход в ширину, чтобы объект не оказался в месте, отрезанном стенами
std::vector<GridPosition> findReachableCells(const TileMap& map, int start_x,
                                             int start_y) {
  std::vector<std::vector<bool>> visited(
      map.getHeight(), std::vector<bool>(map.getWidth(), false));
  std::vector<GridPosition> reachable;
  std::deque<GridPosition> queue = {{start_x, start_y}};
  visited[start_y][start_x] = true;
  while (!queue.empty()) {
    GridPosition cell = queue.front();
    queue.pop_front();
    if (cell.x != start_x || cell.y != start_y) reachable.push_back(cell);
    for (const GridPosition& step : kSteps) {
      GridPosition next = {cell.x + step.x, cell.y + step.y};
      if (map.isWalkable(next.x, next.y) && !visited[next.y][next.x]) {
        visited[next.y][next.x] = true;
        queue.push_back(next);
      }
    }
  }
  return reachable;
}

}  // namespace

Game::Game()
    : renderer(console, symbols, texts),
      state(GameState::kPlaying),
      is_searching(false),
      current_target(nullptr),
      rng(std::random_device{}()) {}

bool Game::init() {
  if (!loadAssets()) return false;
  console.setTitle(texts.getString("window_title"));
  symbols.fitToConsole(console);
  addMessage(texts.getString("start_hint"));
  return true;
}

bool Game::loadAssets() {
  std::vector<std::string> errors;
  auto check = [&errors](bool loaded, const std::string& file_name) {
    if (!loaded) errors.push_back("can't read " + file_name);
    return loaded;
  };

  if (check(settings.load("settings.txt"), "settings.txt")) {
    addMissing(errors, "settings.txt", settings.findMissing(kSettingKeys));
  }
  if (check(texts.load("texts.txt"), "texts.txt")) {
    addMissing(errors, "texts.txt", texts.findMissing(kTextKeys));
  }
  if (check(symbols.load("symbols.txt"), "symbols.txt")) {
    addMissing(errors, "symbols.txt", symbols.findMissing(kSymbolNames));
  }
  check(AssetFile::readLines("title.txt", title_art), "title.txt");
  check(AssetFile::readLines("story.txt", story), "story.txt");
  check(AssetFile::readLines("win.txt", win_art), "win.txt");
  check(AssetFile::readLines("lose.txt", lose_art), "lose.txt");
  check(tile_map.load("map.txt"), "map.txt");

  // Объекты создаём, только когда настройки, тексты и карта уже прочитаны
  if (errors.empty()) spawnEntities(errors);
  if (!errors.empty()) {
    showErrors(errors);
    return false;
  }
  return true;
}

void Game::spawnEntities(std::vector<std::string>& errors) {
  // Из карты берём место старта героя и то, какие объекты есть в комнате
  int hero_count = 0;
  int hero_x = 0;
  int hero_y = 0;
  std::vector<char> object_cells;
  for (int y = 0; y < tile_map.getHeight(); ++y) {
    for (int x = 0; x < tile_map.getWidth(); ++x) {
      char cell = tile_map.getCell(x, y);
      if (cell == kHeroCell) {
        hero_x = x;
        hero_y = y;
        ++hero_count;
      } else if (cell == kPileCell || cell == kClosetCell ||
                 cell == kSmallDrinkCell || cell == kBigDrinkCell) {
        object_cells.push_back(cell);
      }
    }
  }
  bool has_searchable = std::any_of(
      object_cells.begin(), object_cells.end(),
      [](char cell) { return cell == kPileCell || cell == kClosetCell; });
  if (hero_count != 1)
    errors.push_back("map.txt: there must be exactly one H (where you start)");
  if (!has_searchable)
    errors.push_back("map.txt: there must be at least one P or C");
  if (!errors.empty()) return;

  hero = std::make_unique<Hero>(
      tile_map, hero_x, hero_y, settings.getInt("max_energy"),
      settings.getInt("step_energy_cost"), settings.getInt("search_power"));

  // Каждую игру объекты лежат на новых местах: на случайных клетках, до которых
  // можно дойти
  std::vector<GridPosition> free_cells =
      findReachableCells(tile_map, hero_x, hero_y);
  if (free_cells.size() < object_cells.size()) {
    errors.push_back("map.txt: not enough floor for all objects");
    return;
  }
  std::shuffle(free_cells.begin(), free_cells.end(), rng);
  for (size_t i = 0; i < object_cells.size(); ++i) {
    int x = free_cells[i].x;
    int y = free_cells[i].y;
    switch (object_cells[i]) {
      case kPileCell:
        searchable_objects.push_back(
            std::make_unique<Pile>(x, y, settings.getInt("pile_clutter"),
                                   settings.getInt("pile_energy_drain"),
                                   texts.getString("pile_name")));
        break;
      case kClosetCell:
        searchable_objects.push_back(
            std::make_unique<Closet>(x, y, settings.getInt("closet_clutter"),
                                     settings.getInt("closet_energy_drain"),
                                     texts.getString("closet_name")));
        break;
      case kSmallDrinkCell:
        map_items.push_back(MapItem{x, y, createDrink("small_drink")});
        break;
      case kBigDrinkCell:
        map_items.push_back(MapItem{x, y, createDrink("big_drink")});
        break;
      default:
        break;
    }
  }
}

std::unique_ptr<Item> Game::createDrink(const std::string& kind) const {
  int amount = settings.getInt(kind + "_energy");
  std::string description =
      texts.format("drink_description", {{"amount", std::to_string(amount)}});
  return std::make_unique<EnergyDrink>(texts.getString(kind + "_name"),
                                       description, amount);
}

void Game::showErrors(const std::vector<std::string>& errors) {
  // Эти строки не берём из texts.txt: он сам может быть повреждён
  console.clear();
  console.moveTo(2, 1);
  console.write("The game can't start. Something is wrong with the files in:",
                12);
  console.moveTo(2, 2);
  console.write(AssetFile::getAssetsDirectoryText());
  int row = 4;
  for (const std::string& error : errors) {
    console.moveTo(2, row++);
    console.write("- " + error);
  }
  console.moveTo(2, row + 1);
  console.write("Press any key to exit...", 8);
  console.waitForKey();
}

void Game::run() {
  renderer.drawTitleScreen(title_art, story);
  console.waitForKey();

  while (state == GameState::kPlaying) {
    renderer.drawGame(tile_map, *hero, searchable_objects, map_items, messages);
    handleKey(console.readKey());
  }
  if (state == GameState::kQuit) return;

  bool is_victory = state == GameState::kVictory;
  renderer.drawEndScreen(is_victory ? win_art : lose_art,
                         texts.getString(is_victory ? "win" : "lose"),
                         is_victory);
  console.waitForKey();
}

void Game::handleKey(const KeyPress& press) {
  switch (press.key) {
    case Key::kUp:
      tryMove(0, -1);
      break;
    case Key::kDown:
      tryMove(0, 1);
      break;
    case Key::kLeft:
      tryMove(-1, 0);
      break;
    case Key::kRight:
      tryMove(1, 0);
      break;
    case Key::kSpace:
      if (is_searching)
        handleSearchTurn();
      else
        addMessage(texts.getString("nothing_to_search"));
      break;
    case Key::kDigit:
      useItem(press.digit - 1);
      break;
    case Key::kEscape:
      state = GameState::kQuit;
      break;
    default:
      break;
  }
}

void Game::tryMove(int delta_x, int delta_y) {
  // Пока обыск не закончен, уйти нельзя
  if (is_searching) {
    addMessage(texts.format("finish_search_first",
                            {{"name", current_target->getName()}}));
    return;
  }

  int old_x = hero->getGridX();
  int old_y = hero->getGridY();
  hero->move(delta_x, delta_y);
  if (hero->getGridX() == old_x && hero->getGridY() == old_y) {
    addMessage(texts.getString("blocked"));
    return;
  }
  // Энергия могла закончиться на этом шаге
  if (!hero->isAlive()) {
    state = GameState::kDefeat;
    return;
  }
  checkCell();
}

void Game::checkCell() {
  int x = hero->getGridX();
  int y = hero->getGridY();

  for (auto& object : searchable_objects) {
    if (object->getIsActive() && object->getGridX() == x &&
        object->getGridY() == y) {
      is_searching = true;
      current_target = object.get();
      addMessage(texts.format("search_start", {{"name", object->getName()}}));
      return;
    }
  }

  for (auto it = map_items.begin(); it != map_items.end(); ++it) {
    if (it->grid_x != x || it->grid_y != y) continue;

    std::string item_name = it->item->getName();
    Inventory& inventory = hero->getInventory();
    if (inventory.isFull()) {
      addMessage(texts.format("inventory_full", {{"name", item_name}}));
      return;
    }
    inventory.add(std::move(it->item));
    map_items.erase(it);
    addMessage(texts.format(
        "pickup",
        {{"name", item_name}, {"slot", std::to_string(inventory.getSize())}}));
    return;
  }
}

void Game::handleSearchTurn() {
  bool search_continues = CombatSystem::executeOneTurn(*hero, *current_target);
  if (search_continues) {
    addMessage(texts.format(
        "search_continue",
        {{"name", current_target->getName()},
         {"progress", std::to_string(current_target->getSearchProgress())}}));
    return;
  }

  if (hero->isAlive()) {
    // В последнем необысканном объекте карта лежит гарантированно
    bool is_last_object = countActiveObjects() == 1;
    current_target->setIsActive(false);
    if (is_last_object || rollCardFound()) {
      state = GameState::kVictory;
    } else {
      addMessage(texts.format("search_nothing",
                              {{"name", current_target->getName()}}));
    }
  } else {
    state = GameState::kDefeat;
  }
  is_searching = false;
  current_target = nullptr;
}

void Game::useItem(int slot) {
  Inventory& inventory = hero->getInventory();
  Item* item = inventory.getItem(slot);
  if (item == nullptr) {
    addMessage(texts.format("no_item", {{"slot", std::to_string(slot + 1)}}));
    return;
  }

  std::string item_name = item->getName();
  std::string description = item->getDescription();
  if (item->use(*hero)) {
    inventory.remove(slot);
    addMessage(texts.format(
        "use_item", {{"name", item_name}, {"description", description}}));
  } else {
    addMessage(texts.format("item_not_needed", {{"name", item_name}}));
  }
}

void Game::addMessage(const std::string& message) {
  messages.push_back(message);
  if (messages.size() > kMessageLogSize) messages.pop_front();
}

int Game::countActiveObjects() const {
  int count = 0;
  for (const auto& object : searchable_objects) {
    if (object->getIsActive()) ++count;
  }
  return count;
}

bool Game::rollCardFound() {
  double chance = std::clamp(settings.getDouble("card_chance"), 0.0, 1.0);
  std::bernoulli_distribution card_chance(chance);
  return card_chance(rng);
}
