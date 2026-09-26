#pragma once
#include <memory>
#include <vector>

#include "Item.h"

// Предметы, которые герой несёт с собой. Слоты нумеруются с 0
class Inventory {
 private:
  std::vector<std::unique_ptr<Item>> items;

 public:
  // Предметы выбираются клавишами 1-9
  static constexpr int kMaxItems = 9;

  bool isFull() const;
  void add(std::unique_ptr<Item> item);
  int getSize() const;
  Item* getItem(int slot) const;
  void remove(int slot);
};
