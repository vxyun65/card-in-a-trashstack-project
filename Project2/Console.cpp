#include "Console.h"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

namespace {

// Файлы игры в UTF-8, а консоль Windows принимает UTF-16
std::wstring toWide(const std::string& utf8) {
    if (utf8.empty()) return std::wstring();
    int size = static_cast<int>(utf8.size());
    int length = MultiByteToWideChar(CP_UTF8, 0, utf8.data(), size, nullptr, 0);
    std::wstring wide(length, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.data(), size, &wide[0], length);
    return wide;
}

} // namespace

Console::Console() {
    original_output = GetStdHandle(STD_OUTPUT_HANDLE);
    input = GetStdHandle(STD_INPUT_HANDLE);

    HANDLE buffer = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                              nullptr, CONSOLE_TEXTMODE_BUFFER, nullptr);
    if (buffer != INVALID_HANDLE_VALUE && SetConsoleActiveScreenBuffer(buffer)) {
        output = buffer;
    }
    else {
        if (buffer != INVALID_HANDLE_VALUE) CloseHandle(buffer);
        output = original_output;
    }

    // Буфер размером с окно - без полосы прокрутки
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (GetConsoleScreenBufferInfo(output, &info)) {
        COORD window_size = { static_cast<SHORT>(info.srWindow.Right - info.srWindow.Left + 1),
                              static_cast<SHORT>(info.srWindow.Bottom - info.srWindow.Top + 1) };
        SetConsoleScreenBufferSize(output, window_size);
    }

    CONSOLE_CURSOR_INFO cursor = { 1, FALSE };
    SetConsoleCursorInfo(output, &cursor);
    clear();
}

Console::~Console() {
    if (output != original_output) {
        SetConsoleActiveScreenBuffer(original_output);
        CloseHandle(output);
    }
}

void Console::setTitle(const std::string& title) {
    SetConsoleTitleW(toWide(title).c_str());
}

void Console::clear() {
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (!GetConsoleScreenBufferInfo(output, &info)) return;
    DWORD cells = static_cast<DWORD>(info.dwSize.X) * info.dwSize.Y;
    COORD origin = { 0, 0 };
    DWORD written = 0;
    FillConsoleOutputCharacterW(output, L' ', cells, origin, &written);
    FillConsoleOutputAttribute(output, kDefaultColor, cells, origin, &written);
    SetConsoleCursorPosition(output, origin);
}

void Console::moveTo(int x, int y) {
    COORD position = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
    SetConsoleCursorPosition(output, position);
}

void Console::write(const std::string& text, int color) {
    SetConsoleTextAttribute(output, static_cast<WORD>(color));
    std::wstring wide = toWide(text);
    DWORD written = 0;
    WriteConsoleW(output, wide.c_str(), static_cast<DWORD>(wide.size()), &written, nullptr);
}

void Console::clearRestOfLine() {
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (!GetConsoleScreenBufferInfo(output, &info)) return;
    COORD position = info.dwCursorPosition;
    if (position.X >= info.dwSize.X) return;
    DWORD count = static_cast<DWORD>(info.dwSize.X - position.X);
    DWORD written = 0;
    FillConsoleOutputCharacterW(output, L' ', count, position, &written);
    FillConsoleOutputAttribute(output, kDefaultColor, count, position, &written);
}

int Console::measureWidth(const std::string& text) {
    moveTo(0, 0);
    write(text);
    int width = static_cast<int>(text.size());
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (GetConsoleScreenBufferInfo(output, &info)) width = info.dwCursorPosition.X;
    moveTo(0, 0);
    clearRestOfLine();
    return width;
}

KeyPress Console::readKey() {
    INPUT_RECORD record;
    DWORD read = 0;
    while (ReadConsoleInputW(input, &record, 1, &read)) {
        if (record.EventType != KEY_EVENT || !record.Event.KeyEvent.bKeyDown) continue;

        WORD code = record.Event.KeyEvent.wVirtualKeyCode;
        KeyPress press;
        switch (code) {
        case 'W': case VK_UP:    press.key = Key::kUp; break;
        case 'S': case VK_DOWN:  press.key = Key::kDown; break;
        case 'A': case VK_LEFT:  press.key = Key::kLeft; break;
        case 'D': case VK_RIGHT: press.key = Key::kRight; break;
        case VK_SPACE:           press.key = Key::kSpace; break;
        case VK_ESCAPE:          press.key = Key::kEscape; break;
        // Отдельно нажатые Shift, Ctrl, Alt и Caps Lock не считаем
        case VK_SHIFT: case VK_CONTROL: case VK_MENU: case VK_CAPITAL: continue;
        default:
            if (code >= '1' && code <= '9') {
                press.key = Key::kDigit;
                press.digit = code - '0';
            }
            else if (code >= VK_NUMPAD1 && code <= VK_NUMPAD9) {
                press.key = Key::kDigit;
                press.digit = code - VK_NUMPAD0;
            }
            break;
        }
        return press;
    }
    // Ввод недоступен (например, консоли нет) - выходим из игры, а не зависаем
    KeyPress quit;
    quit.key = Key::kEscape;
    return quit;
}

void Console::waitForKey() {
    Sleep(400);
    FlushConsoleInputBuffer(input);
    readKey();
}
