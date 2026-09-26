#include "EnergyDrink.h"

#include "Hero.h"

EnergyDrink::EnergyDrink(const std::string& item_name,
                         const std::string& item_description, int amount)
    : Item(item_name, item_description), energy_amount(amount) {}

std::string EnergyDrink::getSymbolName() const { return "drink"; }

bool EnergyDrink::use(Hero& hero) {
  // Пить при полной энергии бессмысленно - напиток остаётся в инвентаре
  if (hero.getHealthPoints() >= hero.getMaxEnergy()) return false;
  hero.restoreEnergy(energy_amount);
  return true;
}
