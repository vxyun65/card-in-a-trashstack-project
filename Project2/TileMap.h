#pragma once
#include <string>
#include <vector>

// Сетка комнаты из assets/map.txt: '#' - стена, пробел - пустота,
// остальные символы - пол (на нём стоят герой, предметы и объекты)
class TileMap {
 private:
  std::vector<std::string> rows;
  int width = 0;

 public:
  static constexpr char kWall = '#';
  static constexpr char kVoid = ' ';

  bool load(const std::string& file_name);
  int getWidth() const;
  int getHeight() const;
  char getCell(int grid_x, int grid_y) const;
  bool isWalkable(int grid_x, int grid_y) const;
};
