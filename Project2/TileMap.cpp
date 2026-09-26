#include "TileMap.h"

#include <algorithm>

#include "AssetFile.h"

bool TileMap::load(const std::string& file_name) {
  std::vector<std::string> lines;
  if (!AssetFile::readLines(file_name, lines)) return false;

  rows.clear();
  width = 0;
  for (const std::string& line : lines) {
    // Строки с ';' - комментарии с описанием символов карты
    if (line.empty() || line[0] == ';') continue;
    rows.push_back(line);
    width = std::max(width, static_cast<int>(line.size()));
  }
  // Короткие строки дополняем пустотой, чтобы карта была прямоугольной
  for (std::string& row : rows) row.resize(width, kVoid);
  return !rows.empty();
}

int TileMap::getWidth() const { return width; }
int TileMap::getHeight() const { return static_cast<int>(rows.size()); }

char TileMap::getCell(int grid_x, int grid_y) const {
  if (grid_x < 0 || grid_x >= width || grid_y < 0 || grid_y >= getHeight())
    return kVoid;
  return rows[grid_y][grid_x];
}

bool TileMap::isWalkable(int grid_x, int grid_y) const {
  char cell = getCell(grid_x, grid_y);
  return cell != kWall && cell != kVoid;
}
