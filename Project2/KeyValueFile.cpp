#include "KeyValueFile.h"

#include <cstdlib>

#include "AssetFile.h"

bool KeyValueFile::load(const std::string& file_name) {
  std::vector<std::string> lines;
  if (!AssetFile::readLines(file_name, lines)) return false;

  values.clear();
  for (const std::string& line : lines) {
    std::string clean = AssetFile::trim(line);
    if (clean.empty() || clean[0] == ';') continue;
    size_t separator = clean.find('=');
    if (separator == std::string::npos) continue;
    values[AssetFile::trim(clean.substr(0, separator))] =
        AssetFile::trim(clean.substr(separator + 1));
  }
  return true;
}

std::vector<std::string> KeyValueFile::findMissing(
    const std::vector<std::string>& keys) const {
  std::vector<std::string> missing;
  for (const std::string& key : keys) {
    if (values.find(key) == values.end()) missing.push_back(key);
  }
  return missing;
}

std::string KeyValueFile::getString(const std::string& key) const {
  auto found = values.find(key);
  // Если текста нет, показываем ключ - так сразу видно, чего не хватает в файле
  return found != values.end() ? found->second : "[" + key + "]";
}

std::string KeyValueFile::format(const std::string& key,
                                 const Arguments& arguments) const {
  std::string text = getString(key);
  for (const auto& argument : arguments) {
    std::string placeholder = "{" + argument.first + "}";
    size_t position = 0;
    while ((position = text.find(placeholder, position)) != std::string::npos) {
      text.replace(position, placeholder.size(), argument.second);
      position += argument.second.size();
    }
  }
  return text;
}

int KeyValueFile::getInt(const std::string& key) const {
  return std::atoi(getString(key).c_str());
}

double KeyValueFile::getDouble(const std::string& key) const {
  return std::atof(getString(key).c_str());
}
