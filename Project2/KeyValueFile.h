#pragma once
#include <map>
#include <string>
#include <utility>
#include <vector>

// Файл вида "ключ = значение" (настройки и тексты игры).
// Строки, которые начинаются с ';', - комментарии
class KeyValueFile {
 private:
  std::map<std::string, std::string> values;

 public:
  using Arguments = std::vector<std::pair<std::string, std::string>>;

  bool load(const std::string& file_name);
  std::vector<std::string> findMissing(
      const std::vector<std::string>& keys) const;
  std::string getString(const std::string& key) const;
  // Берёт текст по ключу и подставляет значения вместо {имя}
  std::string format(const std::string& key, const Arguments& arguments) const;
  int getInt(const std::string& key) const;
  double getDouble(const std::string& key) const;
};
