#include "Hero.h"

Hero::Hero(const TileMap& map, int start_x, int start_y, int energy_limit, int step_cost, int search_power)
    : tile_map(&map), facing(Direction::kDown), max_energy(energy_limit), step_energy_cost(step_cost) {
    health_points = max_energy;
    attack_damage = search_power;
    name = "hero";
    grid_x = start_x;
    grid_y = start_y;
}

Direction Hero::getFacing() const { return facing; }
int Hero::getMaxEnergy() const { return max_energy; }
Inventory& Hero::getInventory() { return inventory; }
const Inventory& Hero::getInventory() const { return inventory; }

void Hero::restoreEnergy(int amount) {
    health_points += amount;
    if (health_points > max_energy) health_points = max_energy;
}

void Hero::takeDamage(int damage) { health_points -= damage; }
int Hero::getHealthPoints() const { return health_points; }
int Hero::getAttackDamage() const { return attack_damage; }
bool Hero::isAlive() const { return health_points > 0; }

void Hero::move(int delta_x, int delta_y) {
    // Поворачиваемся в сторону нажатой клавиши, даже если там стена
    if (delta_y < 0) facing = Direction::kUp;
    else if (delta_y > 0) facing = Direction::kDown;
    else if (delta_x < 0) facing = Direction::kLeft;
    else if (delta_x > 0) facing = Direction::kRight;

    int new_x = grid_x + delta_x;
    int new_y = grid_y + delta_y;
    if (tile_map->isWalkable(new_x, new_y)) {
        grid_x = new_x;
        grid_y = new_y;
        // Каждый шаг утомляет героя
        health_points -= step_energy_cost;
    }
}
