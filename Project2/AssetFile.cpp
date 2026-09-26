#include "AssetFile.h"
#include <fstream>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

std::filesystem::path AssetFile::getAssetsDirectory() {
    // Ищем assets рядом с exe, а не в текущей папке, чтобы игра запускалась откуда угодно
    wchar_t buffer[MAX_PATH] = {};
    DWORD length = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    std::filesystem::path exe_path(std::wstring(buffer, length));
    return exe_path.parent_path() / L"assets";
}

std::string AssetFile::getAssetsDirectoryText() {
    auto text = getAssetsDirectory().u8string();
    return std::string(text.begin(), text.end());
}

bool AssetFile::readLines(const std::string& file_name, std::vector<std::string>& lines) {
    std::ifstream file(getAssetsDirectory() / file_name, std::ios::binary);
    if (!file) return false;

    lines.clear();
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        lines.push_back(line);
    }
    // Блокнот может сохранить файл с BOM в начале
    if (!lines.empty() && lines[0].compare(0, 3, "\xEF\xBB\xBF") == 0) lines[0].erase(0, 3);
    return true;
}

std::string AssetFile::trim(const std::string& text) {
    size_t begin = text.find_first_not_of(" \t");
    if (begin == std::string::npos) return "";
    size_t end = text.find_last_not_of(" \t");
    return text.substr(begin, end - begin + 1);
}
