#include "Pile.h"

Pile::Pile(int x, int y, int clutter, int energy_drain,
           const std::string& object_name)
    : SearchableObject(x, y, std::make_shared<AggressiveStrategy>(), clutter,
                       energy_drain, object_name) {}

std::string Pile::getSymbolName() const { return "pile"; }
