#pragma once
#include <memory>
#include <string>
#include "Character.h"
#include "CombatStrategy.h"

// Предмет в комнате, который нужно обыскать. health_points - сколько вещей
// осталось перебрать, attack_damage - сколько энергии отнимает обыск
class SearchableObject : public Character {
protected:
    std::shared_ptr<CombatStrategy> strategy;
    bool is_active;
    int max_clutter;

public:
    SearchableObject(int x, int y, std::shared_ptr<CombatStrategy> strat, int clutter, int energy_drain,
                     const std::string& object_name);
    virtual ~SearchableObject() = default;

    // Имя символа из symbols.txt, которым объект рисуется на карте
    virtual std::string getSymbolName() const = 0;
    // Сколько процентов объекта уже обыскано
    int getSearchProgress() const;

    void takeDamage(int damage) override;
    int getHealthPoints() const override;
    int getAttackDamage() const override;
    bool isAlive() const override;
    void move(int delta_x, int delta_y) override;
    int calculateDamageTo(const Character& target) const;
    bool getIsActive() const;
    void setIsActive(bool active);
};
