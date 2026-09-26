#include "Closet.h"

Closet::Closet(int x, int y, int clutter, int energy_drain,
               const std::string& object_name)
    : SearchableObject(x, y, std::make_shared<DefensiveStrategy>(), clutter,
                       energy_drain, object_name) {}

std::string Closet::getSymbolName() const { return "closet"; }
