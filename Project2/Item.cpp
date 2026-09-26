#include "Item.h"

Item::Item(const std::string& item_name, const std::string& item_description)
    : name(item_name), description(item_description) {
}

const std::string& Item::getName() const { return name; }
const std::string& Item::getDescription() const { return description; }
