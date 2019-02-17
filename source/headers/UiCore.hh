#ifndef UICORE
#define UICORE

#include <Windows.h>
#include <cstdint>
#include <conio.h>

#include "IConditionChecker.hh"
#include "InputChecker.hh"
#include "ProcessChecker.hh"

class UiCore {
protected:
    void SetConsoleBufferSize(int16_t, int16_t) const;
    void SetConsoleWindowSize(int16_t, int16_t) const;
    void MoveCursor(int16_t, int16_t) const;
    void LockConsoleSize() const;

public:
    void Dialogue_ProcessChecker() const;
    void Dialogue_InputChecker() const;
    void Dialogue_SelectMode(IConditionChecker**, bool) const;
    void Dialogue_Main() const;

    UiCore();
    ~UiCore();
};

#endif