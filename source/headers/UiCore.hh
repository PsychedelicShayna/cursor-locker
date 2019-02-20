#ifndef UICORE
#define UICORE

#include <Windows.h>
#include <sstream>
#include <string>
#include <cstdint>
#include <conio.h>
#include <cmath>

#include "IConditionChecker.hh"
#include "InputChecker.hh"
#include "ProcessChecker.hh"

/**
 * This class is responsible for the user interface, and is what the user interacts with when launching the program.
 * It imeplements the polymorphic IConditionChecker class, and handles the polymorphism. 
 * 
 * The interface itself is split among different Dialogue methods, each drawing a different dialogue.
 * Each dialogue is responsible for drawing, recieving input, and making requested alterations to 
 * the configuration.
 * 
 * Accompanying the dialogue methods, are some helper methods that the dialogues use to shorten code.
 * e.g. AutoNumericalInput.
 * 
 * The only public Dialogue method is Dialogue_Main() which automatically calls the other dialogue methods,
 * and is virtually an "entrypoint" to the dialogue tree.
 * 
 * ! Some methods aren't ever called, this is due to a mid-way change in structure. 
 * ! These methods are left in, as they're lightweight and may be used in future versions.
 */

class UiCore {
protected:

    /**
     * This converts a string representation of a number, in either decimal or hexadecimal format, into an integer.
     * It takes the integer type as a template argument, and also takes in a pointer to where the output should be stored.
     * If a problem is encountered, it will print the problem to the console before returning false. If no errors
     * are encountered, the method returns true. 
     * 
     * Hexadecimal numbers are detected through the 0x prefix.
     */
    template<typename T>
    bool AutoNumericalInput(const std::string&, T*) const;
    
    // Sets the size of the console buffer.
    void SetConsoleBufferSize(int16_t, int16_t) const;

    // Sets the size of the console window.
    void SetConsoleWindowSize(int16_t, int16_t) const;

    // Moves the console cursor to a specified row and column.
    void MoveCursor(int16_t, int16_t) const;

    // Disables the ability to resize the console window.
    void LockConsoleSize() const;

    // The dialogue for configuring the ProcessChecker type.
    void Dialogue_ProcessChecker(ProcessChecker**) const;

    // The dialogue for configuring the InputChecker type.
    void Dialogue_InputChecker(InputChecker**) const;

    // The dialogue for selecting and assigning a subclass to the polymorphci IConditionChecker type.
    void Dialogue_SelectMode(IConditionChecker**, bool) const;

public:

    // The main dialogue, where the other dialogues. This is the starting point of this class.
    void Dialogue_Main() const;

    UiCore();
    ~UiCore();
};

#endif