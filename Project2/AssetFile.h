#pragma once
#include <filesystem>
#include <string>
#include <vector>

// Все ресурсы игры лежат во внешних файлах в папке assets рядом с exe
class AssetFile {
public:
    static std::filesystem::path getAssetsDirectory();
    static std::string getAssetsDirectoryText();
    // Читает файл построчно (UTF-8), убирая '\r' и BOM. false - если файл не открылся
    static bool readLines(const std::string& file_name, std::vector<std::string>& lines);
    static std::string trim(const std::string& text);
};
