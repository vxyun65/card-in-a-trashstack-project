#pragma once
#include "Item.h"

class EnergyDrink : public Item {
 private:
  int energy_amount;

 public:
  EnergyDrink(const std::string& item_name, const std::string& item_description,
              int amount);
  std::string getSymbolName() const override;
  bool use(Hero& hero) override;
};
