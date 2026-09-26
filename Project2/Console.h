#pragma once
#include <string>

enum class Key { kOther, kUp, kDown, kLeft, kRight, kSpace, kEscape, kDigit };

struct KeyPress {
    Key key = Key::kOther;
    int digit = 0; // для Key::kDigit - цифра от 1 до 9
};

// Обёртка над консолью Windows: цветной Unicode-вывод и чтение клавиш.
// Игра рисуется в отдельном экранном буфере, после выхода консоль
// возвращается в прежний вид
class Console {
private:
    void* original_output; // HANDLE, чтобы не подключать windows.h в заголовке
    void* output;
    void* input;

public:
    static constexpr int kDefaultColor = 7;

    Console();
    ~Console();
    Console(const Console&) = delete;
    Console& operator=(const Console&) = delete;

    void setTitle(const std::string& title);
    void clear();
    void moveTo(int x, int y);
    void write(const std::string& text, int color = kDefaultColor);
    void clearRestOfLine();
    // Сколько колонок консоли на самом деле занимает текст
    int measureWidth(const std::string& text);
    // Клавиши читаются по виртуальным кодам, поэтому WASD работает в любой раскладке
    KeyPress readKey();
    // Небольшая пауза, чтобы случайное нажатие не пролистнуло экран
    void waitForKey();
};
