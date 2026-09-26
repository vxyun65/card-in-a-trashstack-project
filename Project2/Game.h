#pragma once
#include <deque>
#include <memory>
#include <random>
#include <string>
#include <vector>
#include "Console.h"
#include "Hero.h"
#include "Item.h"
#include "KeyValueFile.h"
#include "Renderer.h"
#include "SearchableObject.h"
#include "SymbolTable.h"
#include "TileMap.h"

enum class GameState { kPlaying, kVictory, kDefeat, kQuit };

class Game {
private:
    // Сколько последних сообщений видно под картой
    static constexpr size_t kMessageLogSize = 5;

    // Консоль и ресурсы объявлены первыми: Renderer хранит ссылки на них
    Console console;
    KeyValueFile settings;
    KeyValueFile texts;
    SymbolTable symbols;
    TileMap tile_map;
    std::vector<std::string> title_art;
    std::vector<std::string> story;
    std::vector<std::string> win_art;
    std::vector<std::string> lose_art;

    std::unique_ptr<Hero> hero;
    std::vector<std::unique_ptr<SearchableObject>> searchable_objects;
    std::vector<MapItem> map_items;
    std::deque<std::string> messages;
    Renderer renderer;
    GameState state;
    bool is_searching;
    SearchableObject* current_target;
    std::mt19937 rng;

    bool loadAssets();
    void spawnEntities(std::vector<std::string>& errors);
    std::unique_ptr<Item> createDrink(const std::string& kind) const;
    void showErrors(const std::vector<std::string>& errors);

    void handleKey(const KeyPress& press);
    void tryMove(int delta_x, int delta_y);
    void checkCell();
    void handleSearchTurn();
    void useItem(int slot);
    void addMessage(const std::string& message);
    int countActiveObjects() const;
    bool rollCardFound();

public:
    Game();
    // Загружает ресурсы из папки assets. false - если чего-то не хватает
    bool init();
    void run();
};
