#include "Inventory.h"

bool Inventory::isFull() const {
  return static_cast<int>(items.size()) >= kMaxItems;
}

void Inventory::add(std::unique_ptr<Item> item) {
  if (!isFull()) items.push_back(std::move(item));
}

int Inventory::getSize() const { return static_cast<int>(items.size()); }

Item* Inventory::getItem(int slot) const {
  if (slot < 0 || slot >= getSize()) return nullptr;
  return items[slot].get();
}

void Inventory::remove(int slot) {
  if (slot < 0 || slot >= getSize()) return;
  items.erase(items.begin() + slot);
}
