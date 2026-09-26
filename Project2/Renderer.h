#pragma once
#include <deque>
#include <memory>
#include <string>
#include <vector>

#include "Console.h"
#include "Hero.h"
#include "Item.h"
#include "KeyValueFile.h"
#include "SearchableObject.h"
#include "SymbolTable.h"
#include "TileMap.h"

// Рисует экраны игры в консоли: карту с легендой, энергию, инвентарь и
// сообщения
class Renderer {
 private:
  struct Segment {
    std::string text;
    int color;
  };
  using Line = std::vector<Segment>;

  Console& console;
  const SymbolTable& symbols;
  const KeyValueFile& texts;
  size_t last_line_count = 0;

  Line buildEnergyLine(const Hero& hero) const;
  std::vector<Line> buildMapLines(
      const TileMap& map, const Hero& hero,
      const std::vector<std::unique_ptr<SearchableObject>>& objects,
      const std::vector<MapItem>& items) const;
  std::vector<Line> buildSideLines(const Inventory& inventory) const;
  // Выводит строки на экран и стирает то, что осталось от прошлого кадра
  void flush(const std::vector<Line>& lines);

 public:
  Renderer(Console& target, const SymbolTable& symbol_table,
           const KeyValueFile& text_table);

  void drawTitleScreen(const std::vector<std::string>& art,
                       const std::vector<std::string>& story);
  void drawGame(const TileMap& map, const Hero& hero,
                const std::vector<std::unique_ptr<SearchableObject>>& objects,
                const std::vector<MapItem>& items,
                const std::deque<std::string>& messages);
  void drawEndScreen(const std::vector<std::string>& art,
                     const std::string& message, bool is_victory);
};
