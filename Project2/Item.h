#pragma once
#include <memory>
#include <string>
class Hero;

// Предмет, который можно подобрать и потом использовать
class Item {
protected:
    std::string name;
    std::string description;

public:
    Item(const std::string& item_name, const std::string& item_description);
    virtual ~Item() = default;

    const std::string& getName() const;
    const std::string& getDescription() const;
    // Имя символа из symbols.txt, которым предмет рисуется на карте
    virtual std::string getSymbolName() const = 0;
    // Применяет предмет. true - предмет израсходован
    virtual bool use(Hero& hero) = 0;
};

// Предмет, который лежит на карте
struct MapItem {
    int grid_x;
    int grid_y;
    std::unique_ptr<Item> item;
};
