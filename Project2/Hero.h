#pragma once
#include "Character.h"
#include "Inventory.h"
#include "TileMap.h"

enum class Direction { kUp, kLeft, kDown, kRight };

class Hero : public Character {
 private:
  const TileMap* tile_map;
  Direction facing;
  int max_energy;
  int step_energy_cost;
  Inventory inventory;

 public:
  Hero(const TileMap& map, int start_x, int start_y, int energy_limit,
       int step_cost, int search_power);

  Direction getFacing() const;
  int getMaxEnergy() const;
  Inventory& getInventory();
  const Inventory& getInventory() const;
  void restoreEnergy(int amount);
  void move(int delta_x, int delta_y);

  void takeDamage(int damage) override;
  int getHealthPoints() const override;
  int getAttackDamage() const override;
  bool isAlive() const override;
};
