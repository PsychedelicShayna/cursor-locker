#include "headers/InputChecker.hh"

void InputChecker::check_() {
    while(!this->thread_terminate) {
        if((this->thread_enabled) && ((GetAsyncKeyState(this->settings.virtual_keycode) & 0x8000) != 0)) {
            bool state = (this->mouseLocker_.thread_enabled ^= true);
            if(this->settings.play_sound)  Beep(state ? 900 : 700, 30);

            Sleep(500);
        }
        
        Sleep(60);
    }
}

void InputChecker::kill() {
    this->thread_enabled = false;
    this->thread_terminate = true;

    this->check_thread_.join();
}

InputChecker::InputChecker(uint8_t vkid) : IConditionChecker() {
    this->ID = this->ICID;

    this->settings.mlsettings = &(this->mouseLocker_.settings);
    this->settings.virtual_keycode = vkid;
    this->settings.polling_rate = 60;
    this->settings.input_delay = 500;
    this->settings.play_sound = true;

    this->check_thread_ = std::thread(&InputChecker::check_, this);
}

InputChecker::~InputChecker() {
    kill();
}