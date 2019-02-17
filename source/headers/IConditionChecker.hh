#ifndef ICONDITIONCHECKER
#define ICONDITIONCHECKER

#include <iostream> // remove this include
#include <thread>       // Multi-threading support.
#include <cstdint>      // Alternative integer definitions.

#include "MouseLocker.hh"

/**
 * This interface (abstract class) offers a base class that controls a protected MouseLocker instance.
 * It contains a pure virtual method that should be overriden in a subclass, and a thread that should be bound-
 * -to the pure virtual method in the subclass. The intent of this method is to run in a loop in the background
 * and activate/deactivate the MouseLocker instance based on certain conditions that vary depending on the
 * implementation of the pure virtual method in the subclass.
 */

class IConditionChecker {
protected:
    MouseLocker mouseLocker_;       // Instance of the MouseLocker class. This locks the mouse to the primary monitor.
    std::thread check_thread_;      // The primary thread of this class, check_ will be bound to it..
    
    // This pure virtual method is bound to check_thread_. This needs to be overriden and bound in a subclass. 
    virtual void check_() = 0;
    
public:
    const static uint8_t ICCID = (uint8_t)0b0001;    // Constant ID to identify this class.
    uint8_t ID;                             // 8-Bit Integer identification, to identify what polymorphic subclass is used. This gets assigned in a subclass constructor.

    bool thread_enabled;            // Bool that when true, enables the functionality of the check_ method. When false, check_ keeps running in a loop, but does nothing.
    bool thread_terminate;          // Bool that when true, stops the loop in the check_ method, allowing check_ to return.

    struct ISettings {
        MouseLocker::Settings* mlsettings;      // Pointer to the mouseLocker_ settings struct.
    };

    IConditionChecker();
    virtual ~IConditionChecker();
};

#endif