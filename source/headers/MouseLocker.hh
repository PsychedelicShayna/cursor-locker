#ifndef MOUSE_LOCKER
#define MOUSE_LOCKER

#include <Windows.h>	// Windows API functions.
#include <thread>		// Multi-threading support.
#include <cstdint>      // Alternative integer definitions.

/**
 * This class is responsible for locking the cursor to a certain position on the screen.
 * A separate thread is used to do this, as it would otherwise interrupt the main thread.
 */

class MouseLocker {
protected:
    std::thread setcursor_thread_;	    // The thread that will be bound to the setcursor_ method.

    // Finds the default resolution of the primary monitor.
    std::pair<uint32_t, uint32_t> find_resolution_() const;

    /**
     * This method repeatedly sets the position of a cursor to a certain position, virtually locking it in that position.
     * The method consists of a loop, with the condition being the inverse of setcursor_thread_tlexit_ variable. 
     * If setcursor_thread_tlexit_ is set to true, then the outer loop of this method will break, allowing the thread
     * that the method is running on, to join with the primary thread.
     * 
     * The secondary setcursor_thread_active_ variable, has to be true, as the cursor setting code is within an if statement
     * of this variable. This is so that the cursor can be locked/unlocked without having to restart or create a new thread.
     * 
     * In short, setcursor_thread_active_ enables/disables the method without returning from it. 
     * While setcursor_thread_tlexit_ allows the method to break out of the loop, and return. 
     * 
     * The delay between each cursor position set, is determined by the setcursor_interval variable, in the public settings struct.
     * Likewise delay between each evaluation of the setcursor_thread_active_ state is determined by the setcursor_evalinterval variable
     * in the same public settings struct.
     * 
     * The screen coordinates od where the cursor is locked to, is determined by the integer pair, setcursor_coordinates
     * in the public settings struct.
     */
    
    void setcursor_() const;

public:
    bool thread_enabled;		// Boolean that when true, enables the cursor locking mechanism, when false, thread still runs, but does nothing.
    bool thread_terminate;		// Boolean that when true, stops the loop in setcursor_, allowing the setcursor_thread_ to join.

    struct Settings  {
        std::pair<uint32_t, uint32_t> setcursor_coordinates;        // The coordinates of where the cursor should be set to. 
        uint16_t setcursor_interval;                                // The how frequently the cursor position should be set (in milliseconds) 
        uint16_t setcursor_evalinterval;                            // The delay before thread_enabled is re-evaluated in setcursor_()
    } settings;

    void kill();    // Sets thread_enabled to false, thread_terminate to true, and joins the setcursor_thread_.

    MouseLocker();
    virtual ~MouseLocker();
};

#endif