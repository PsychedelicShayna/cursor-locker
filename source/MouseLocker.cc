#include "headers/MouseLocker.hh"

std::pair<uint32_t, uint32_t> MouseLocker::find_resolution_() const {
    RECT desktop_rectangle;
    HWND__* desktop_window_handle;
    
    desktop_window_handle = GetDesktopWindow();
    GetWindowRect(desktop_window_handle, &desktop_rectangle);
    return std::make_pair(desktop_rectangle.right, desktop_rectangle.bottom);
}

void MouseLocker::setcursor_() const {
    while (!this->thread_terminate) {
        if (this->thread_enabled) {
            SetCursorPos(
                this->settings.setcursor_coordinates.first, 
                this->settings.setcursor_coordinates.second
            );

            Sleep(this->settings.setcursor_interval);
            continue;
        }

        Sleep(this->settings.setcursor_evalinterval);
    }
}

void MouseLocker::kill() {
    this->thread_enabled = false;
    this->thread_terminate = true;

    this->setcursor_thread_.join();
}

MouseLocker::MouseLocker() {
    this->thread_enabled = false;
    this->thread_terminate = false;

    std::pair<uint32_t, uint32_t> monitor_resolution;
    monitor_resolution = find_resolution_(); 

    this->settings.setcursor_coordinates = {
        monitor_resolution.first  / 2,
        monitor_resolution.second / 2
    };
   
    this->settings.setcursor_interval = 30;
    this->settings.setcursor_evalinterval = 500;
    
    this->setcursor_thread_ = std::thread(&MouseLocker::setcursor_, this);
}

MouseLocker::~MouseLocker() {
   this->kill();
}