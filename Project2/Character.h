#pragma once
#include <string>

// Базовый класс для всех участников "боя": героя и объектов, которые он
// обыскивает
class Character {
 protected:
  int health_points = 0;
  int attack_damage = 0;
  std::string name;
  int grid_x = 0;
  int grid_y = 0;

 public:
  virtual ~Character() = default;

  int getGridX() const { return grid_x; }
  int getGridY() const { return grid_y; }
  const std::string& getName() const { return name; }

  virtual void takeDamage(int damage) = 0;
  virtual int getHealthPoints() const = 0;
  virtual int getAttackDamage() const = 0;
  virtual bool isAlive() const = 0;
};
