#ifndef UICORE
#define UICORE

#include <Windows.h>
#include <cstdint>

class UiCore {
public:
    void SetConsoleBufferSize(uint32_t, uint32_t) const;
    void SetConsoleWindowSize(uint32_t, uint32_t) const;
    void MoveCursor(uint32_t, uint32_t) const;
    void LockConsoleSize() const;

public:
    UiCore();
    ~UiCore();
};

#endif