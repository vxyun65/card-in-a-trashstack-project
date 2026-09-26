#include "SearchableObject.h"

SearchableObject::SearchableObject(int x, int y,
                                   std::shared_ptr<CombatStrategy> strat,
                                   int clutter, int energy_drain,
                                   const std::string& object_name)
    : strategy(strat), is_active(true), max_clutter(clutter) {
  health_points = clutter;
  attack_damage = energy_drain;
  name = object_name;
  grid_x = x;
  grid_y = y;
}

int SearchableObject::getSearchProgress() const {
  if (max_clutter <= 0) return 100;
  int left = health_points > 0 ? health_points : 0;
  return (max_clutter - left) * 100 / max_clutter;
}

void SearchableObject::takeDamage(int damage) { health_points -= damage; }
int SearchableObject::getHealthPoints() const { return health_points; }
int SearchableObject::getAttackDamage() const { return attack_damage; }
bool SearchableObject::isAlive() const { return health_points > 0; }
int SearchableObject::calculateDamageTo(const Character& target) const {
  return strategy->calculateDamage(*this, target);
}
bool SearchableObject::getIsActive() const { return is_active; }
void SearchableObject::setIsActive(bool active) { is_active = active; }
