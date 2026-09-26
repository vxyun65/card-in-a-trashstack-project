#include "CombatStrategy.h"
#include "Character.h"

int AggressiveStrategy::calculateDamage(const Character& attacker, const Character& defender) const {
    // Куча: каждый ход обыска отнимает много энергии
    return attacker.getAttackDamage() + 3;
}

int DefensiveStrategy::calculateDamage(const Character& attacker, const Character& defender) const {
    // Шкаф: энергии уходит мало, зато обыск долгий
    return attacker.getAttackDamage() - 1 > 0 ? attacker.getAttackDamage() - 1 : 1;
}
