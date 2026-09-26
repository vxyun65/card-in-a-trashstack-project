#pragma once
#include "SearchableObject.h"

// Шкаф: энергии тратит мало, но обыскивать его долго
class Closet : public SearchableObject {
public:
    Closet(int x, int y, int clutter, int energy_drain, const std::string& object_name);
    std::string getSymbolName() const override;
};
