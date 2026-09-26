#include "SymbolTable.h"

#include <algorithm>
#include <sstream>

#include "AssetFile.h"
#include "Console.h"

bool SymbolTable::load(const std::string& file_name) {
  std::vector<std::string> lines;
  if (!AssetFile::readLines(file_name, lines)) return false;

  symbols.clear();
  order.clear();
  for (const std::string& line : lines) {
    std::string clean = AssetFile::trim(line);
    if (clean.empty() || clean[0] == ';') continue;

    // Формат строки: имя символ цвет пояснение
    std::istringstream stream(clean);
    std::string name;
    std::string glyph;
    int color = Console::kDefaultColor;
    if (!(stream >> name >> glyph >> color)) continue;
    std::string legend;
    std::getline(stream, legend);

    Symbol symbol;
    symbol.glyph = glyph == "space" ? " " : glyph;
    symbol.cell = symbol.glyph;
    symbol.color = color;
    symbol.legend = AssetFile::trim(legend);
    if (symbols.find(name) == symbols.end()) order.push_back(name);
    symbols[name] = symbol;
  }
  return true;
}

std::vector<std::string> SymbolTable::findMissing(
    const std::vector<std::string>& names) const {
  std::vector<std::string> missing;
  for (const std::string& name : names) {
    if (symbols.find(name) == symbols.end()) missing.push_back(name);
  }
  return missing;
}

void SymbolTable::fitToConsole(Console& console) {
  for (auto& entry : symbols) {
    Symbol& symbol = entry.second;
    int width = console.measureWidth(symbol.glyph);
    symbol.cell =
        symbol.glyph + std::string(std::max(0, kCellWidth - width), ' ');
  }
}

const Symbol& SymbolTable::get(const std::string& name) const {
  static const Symbol kUnknown = {"?", "? ", 12, "-"};
  auto found = symbols.find(name);
  return found != symbols.end() ? found->second : kUnknown;
}

const std::vector<std::string>& SymbolTable::getOrder() const { return order; }
