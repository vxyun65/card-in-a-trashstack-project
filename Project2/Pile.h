#pragma once
#include "SearchableObject.h"

// Куча вещей: разбирается быстро, но отнимает много энергии
class Pile : public SearchableObject {
 public:
  Pile(int x, int y, int clutter, int energy_drain,
       const std::string& object_name);
  std::string getSymbolName() const override;
};
