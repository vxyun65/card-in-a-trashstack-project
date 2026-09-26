#pragma once
#include <map>
#include <string>
#include <vector>
class Console;

struct Symbol {
  std::string glyph;  // символ из файла
  std::string cell;   // тот же символ, дополненный пробелами до ширины клетки
  int color = 7;
  std::string legend;  // пояснение для легенды ("-" - не показывать)
};

// Как объекты выглядят на экране. Загружается из assets/symbols.txt
class SymbolTable {
 private:
  std::map<std::string, Symbol> symbols;
  std::vector<std::string> order;

 public:
  // Каждая клетка карты занимает две колонки консоли
  static constexpr int kCellWidth = 2;

  bool load(const std::string& file_name);
  std::vector<std::string> findMissing(
      const std::vector<std::string>& names) const;
  // Символы вроде ▲ в некоторых шрифтах занимают две колонки, а не одну.
  // Меряем реальную ширину в консоли и выравниваем клетки, чтобы карта не
  // поехала
  void fitToConsole(Console& console);
  const Symbol& get(const std::string& name) const;
  // Порядок символов как в файле - в нём же строится легенда
  const std::vector<std::string>& getOrder() const;
};
