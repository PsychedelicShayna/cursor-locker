#include "headers/UiCore.hh"

void UiCore::SetConsoleBufferSize(int16_t cols, int16_t rows) const {
    _COORD new_buffer_size = {cols, rows};
    void* console_buffer_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleScreenBufferSize(console_buffer_handle, new_buffer_size);
}

void UiCore::SetConsoleWindowSize(int16_t width, int16_t height) const {
    _SMALL_RECT new_window_size = {width, height, (uint16_t)NULL, (uint16_t)NULL};
    void* console_buffer_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleWindowInfo(console_buffer_handle, true, &new_window_size);
}

void UiCore::MoveCursor(int16_t width, int16_t height) const {
    _COORD coordinates = {width, height};
    void* console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleCursorPosition(console_handle, coordinates);
}

void UiCore::LockConsoleSize() const {
    HWND__* console_window_handle = GetConsoleWindow();
    int32_t old_window_style = GetWindowLongPtr(console_window_handle, GWL_STYLE);
    int32_t new_window_style = old_window_style ^ WS_MAXIMIZEBOX ^ WS_SIZEBOX;
    SetWindowLongPtr(console_window_handle, GWL_STYLE, new_window_style);
}

void UiCore::Dialogue_SelectMode(IConditionChecker** polymorphic_target, bool optional) const {
    system("cls");
    
    std::cout << "1.) Keyboard Shortcut" << std::endl;
    std::cout << "2.) Process Presence" << std::endl;

    if(optional) std::cout << std::endl << "0.) Back" << std::endl;

    int8_t choice = '\0';

    while(!(choice >= '1' && choice <= '2') && !(optional && choice == '0')) 
        choice = _getch();

    switch(choice) {
        case '0' : {
            return;
        }

        case '1' : {
           delete *polymorphic_target;
            *polymorphic_target = new InputChecker(0x6A);
            break;
        }

        case '2' : {
            delete *polymorphic_target;
            *polymorphic_target = new ProcessChecker("tesv.exe");
            break;
        }
    
        default : {
            std::cout << "An error has occurred." << std::endl;
            break;
        }
    };
}

void UiCore::Dialogue_Main() const {
    IConditionChecker* icc_polymorphic = new ProcessChecker("tesv.exe");
    this->Dialogue_SelectMode(&icc_polymorphic, false);
    bool exit_loop = false;

    while(!exit_loop) {
        system("cls");
        std::cout << "1.) Mode: ";

        switch(icc_polymorphic->ID) {
            case IConditionChecker::ICCID : {
                std::cout << "(ERROR) BASE POLYMORPHIC TYPE." << std::endl;
                break;
            }
            
            case InputChecker::ICID : {
                std::cout << "Keyboard Shortcut" << std::endl;
                break;
            }

            case ProcessChecker::PCID : {
                std::cout << "Process Presence" << std::endl;
                break;
            }

            default : {
                std::cout << "An error has occured. Default case." << std::endl;
                break;
            }
        }

        std::cout << "2.) State: " << (icc_polymorphic->thread_enabled  ? "Enabled" : "Disabled") << std::endl;
        std::cout << "3.) Configure" << std::endl;
        std::cout << std::endl << "0.) Exit" << std::endl;

        char selection = '\0';
        while(selection != '1' && selection != '2' && selection != '3' && selection != '0') 
            selection = _getch();

        switch(selection) {
            case '1' : {
                this->Dialogue_SelectMode(&icc_polymorphic, true);
                break;
            }

            case '2' : {
                icc_polymorphic->thread_enabled ^= true;
                break;
            }

            case '3' : {
                // TODO: Implement configure functions once those dialogues are complete.
                break;
            }

            case '0' : {
                exit_loop = true;
                break;
            }

            default : {
                std::cout << "An error has occurred." << std::endl;
                break;
            }
        }
    }

    
    icc_polymorphic->~IConditionChecker();
    free(icc_polymorphic);
}

UiCore::UiCore() {

}

UiCore::~UiCore() {

}
