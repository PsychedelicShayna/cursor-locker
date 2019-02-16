#include "headers/ProcessChecker.hh"

void ProcessChecker::check_() {
    while(!this->thread_terminate) {
        if(this->thread_enabled) {
            bool process_found = false;
        
            void* snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            if(snapshot == INVALID_HANDLE_VALUE) {
                CloseHandle(snapshot);
                continue;            
            }
            
            PROCESSENTRY32 process; process.dwSize = sizeof(process);
            
            if(!Process32First(snapshot, &process)) {
                CloseHandle(snapshot);
                continue;
            }
          
            do {
                if(!_stricmp(process.szExeFile, this->settings.imageName)) {
                    process_found = true;
                    break;
                }
            } while(Process32Next(snapshot, &process));

            if(process_found != this->mouseLocker_.thread_enabled) {
                bool state = (this->mouseLocker_.thread_enabled ^= true);
                if(this->settings.play_sound) Beep(state ? 900 : 700, 30);
            }

            CloseHandle(snapshot);     
        }
        
        Sleep(this->settings.polling_rate);
    }
}

void ProcessChecker::kill() {
    this->thread_enabled = false;
    this->thread_terminate = true;

    this->check_thread_.join();
}

ProcessChecker::ProcessChecker(const char* image) : IConditionChecker() {
    this->thread_enabled = false;
    this->thread_terminate = false;

    this->settings.mlsettings = &(this->mouseLocker_.settings);
    this->settings.imageName = image;
    this->settings.play_sound = true;
    this->settings.polling_rate = 500;

    this->check_thread_ = std::thread(&ProcessChecker::check_, this);    
}

ProcessChecker::~ProcessChecker() {
    std::cout << "ProcessChecker destructor called." << std::endl;
    kill();
}