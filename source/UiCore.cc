#include "headers/UserInterface.hh"

void UiCore::SetConsoleBufferSize(uint32_t cols, uint32_t rows) const {
    _COORD new_buffer_size = {(uint16_t)cols, (uint16_t)rows};
    void* console_buffer_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleScreenBufferSize(console_buffer_handle, new_buffer_size);
}

void UiCore::SetConsoleWindowSize(uint32_t width, uint32_t height) const {
    _SMALL_RECT new_window_size = {width, height, NULL, NULL};
    void* console_buffer_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleWindowInfo(console_buffer_handle, true, &new_window_size);
}

void UiCore::MoveCursor(uint32_t width, uint32_t height) const {
    _COORD coordinates = {(int16_t)width, (int16_t)height};
    void* console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleCursorPosition(console_handle, coordinates);
}

void UiCore::LockConsoleSize() const {
    HWND__* console_window_handle = GetConsoleWindow();
    int32_t old_window_style = GetWindowLongPtr(console_window_handle, GWL_STYLE);
    int32_t new_window_style = old_window_style ^ WS_MAXIMIZEBOX ^ WS_SIZEBOX;
    SetWindowLongPtr(console_window_handle, GWL_STYLE, new_window_style);
}

UiCore::UiCore() {

}

UiCore::~UiCore() {

}
