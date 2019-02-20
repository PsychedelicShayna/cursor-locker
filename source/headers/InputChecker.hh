#ifndef INPUTCHECKER
#define INPUTCHECKER

#include <Windows.h>                // Windows API functions.
#include <thread>                   // Multi-threading support.
#include <cstdint>                  // Alternative integer definitions.

#include "IConditionChecker.hh"     // IConditionChecker interface / superclass..

/**
 * This is an implementation of the IConditionChecker interface. This implementation focuses on checking
 * for user input. The check_ pure virtual method is overriden with a method that asynchronously checks
 * for a specific key being pressed. This key is specified through the virtual_keycode setting.
 * This variable should be a VKID (Virtual Key ID). Uppon being pressed, it will toggle the mouseLock_ thread.
 * List of valid VKIDs: http://cherrytree.at/misc/vk.htm
 */

class InputChecker : public IConditionChecker {
protected:
    void check_() override;         // The overriden virtual method that will contain the condition checking code.

public:
    const static uint8_t ICID = (uint8_t)0b00100;    // Constant ID to identify this class.

    struct Settings : ISettings {
        uint8_t virtual_keycode;    // The virtual keycode of the key that should be checked by the check_ method.

        uint16_t polling_rate;      // How frequently (in milliseconds) the check_ method should be shecking for input.
        uint16_t input_delay;       // The delay after an input is registered before continuing to check. Helps avoid over-sensitivity.

        bool play_sound;            // Whether or not to play a sound when input is registered.
    } settings;

    void kill();    // Method that sets thread_enabled to false, and thread_terminate to true, and joins the check_thread_ in the superclass.

    InputChecker(uint8_t);
    ~InputChecker() override;
};

#endif