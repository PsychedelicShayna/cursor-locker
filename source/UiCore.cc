#include "headers/UiCore.hh"

template<typename T>
bool UiCore::AutoNumericalInput(const std::string& string, T* output) const {
    uint32_t output_buffer;

    if(string.size() > 2 && (string.at(0) == '0' && string.at(1) == 'x')) {
        try {
            std::stringstream conversion_buffer;
            conversion_buffer << std::hex;
            conversion_buffer << string;
            conversion_buffer >> output_buffer;
        } catch(std::exception& exception) { 
            std::cout << "Failed to convert hex \"" << string << "\" to an unsigned 32-Bit integer." << std::endl;
            return false;
        }
    } else {
        try {
            output_buffer = std::stoi(string);
        } catch(std::exception& exception) {
            std::cout << "Failed to convert decimal \"" << string << "\" to an unsigned 32-Bit integer." << std::endl;
            return false;
        }
    }

    if(output_buffer <= pow(256, sizeof(T))) {
        *output = static_cast<T>(output_buffer);
    } else {
        std::cout << "Converted number exceeds maximum type size (" << output_buffer << " > " << pow(256,sizeof(T)) << ")" << std::endl;
        return false;
    }

    return true;
}

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

void UiCore::Dialogue_ProcessChecker(ProcessChecker** pc_target) const {
    while(true) {
        system("cls");
        std::cout << "1.) Image Name: " << (*pc_target)->settings.imageName << std::endl;
        std::cout << "2.) Polling Rate: " << (*pc_target)->settings.polling_rate << "ms" << std::endl;
        std::cout << "3.) Locking Rate: " << (*pc_target)->settings.mlsettings->setcursor_interval << "ms" << std::endl;
        std::cout << "4.) On/Off Sound: " << (((*pc_target)->settings.play_sound) ? "Enabled" : "Disabled") << std::endl;

        std::cout << std::endl << "0.) Return" << std::endl << std::endl;

        char selection = '\0';

        while(!(selection >= '0' && selection <= '4'))
            selection = _getch();

        switch(selection) {
            case '0' : {
                return;
            }

            case '1' : {
                std::cout << "New name ~ ";
                std::string new_value;
                std::cin >> new_value;
                
                realloc((*pc_target)->settings.imageName, strlen(new_value.c_str()));
                strcpy((*pc_target)->settings.imageName, new_value.c_str());
                break;
            }

            case '2' : {
                std::cout << "New rate(ms) ~ ";
                std::string string_input;
                std::cin >> string_input;

                uint16_t new_value;

                if(AutoNumericalInput<uint16_t>(string_input, &new_value)) {
                    (*pc_target)->settings.polling_rate = new_value;
                } else {
                    std::cout << "Press any key to continue." << std::endl;
                    _getch();
                }
                break;
            }

            case '3' : {
                std::cout << "New rate(ms) ~ ";
                std::string string_input;
                std::cin >> string_input;

                uint16_t new_value;

                if(AutoNumericalInput<uint16_t>(string_input, &new_value)) {
                    (*pc_target)->settings.mlsettings->setcursor_interval = new_value;
                } else {
                    std::cout << "Press any key to continue." << std::endl;
                    _getch();
                }
                break;
            }

            case '4' : {
                (*pc_target)->settings.play_sound ^= true;
                break;
            }
        }
    }
}

void UiCore::Dialogue_InputChecker(InputChecker** ic_target) const {
    while(true) {
        system("cls");
        std::cout << "1.) Trigger VKID: " << std::uppercase << std::hex << static_cast<uint32_t>((*ic_target)->settings.virtual_keycode) << std::dec << std::endl;
        std::cout << "2.) Polling Rate: " << (*ic_target)->settings.polling_rate << "ms" << std::endl;
        std::cout << "3.) Locking Rate: " << (*ic_target)->settings.mlsettings->setcursor_interval << "ms" << std::endl;
        std::cout << "4.) On/Off Sound: " << (((*ic_target)->settings.play_sound) ? "Enabled" : "Disabled") << std::endl;

        std::cout << std::endl << "0.) Return" << std::endl << std::endl;

        char selection = '\0';

        while(!(selection >= '0' && selection <= '4')) 
            selection = _getch();

        switch(selection) {
            case '0' : {
                return;
            }

            case '1' : {
                uint8_t value_buffer;

                std::cout << "New VKID ~ ";
                std::string string_input;
                std::cin >> string_input;

                if(AutoNumericalInput<uint8_t>(string_input, &value_buffer)) {
                    (*ic_target)->settings.virtual_keycode = value_buffer;
                } else {
                    std::cout << "Press any key to continue." << std::endl;
                    _getch();
                }

                break;
            }

            case '2' : {
                uint16_t value_buffer;

                std::cout << "New rate(ms) ~ ";
                std::string string_input;
                std::cin >> string_input;

                if(AutoNumericalInput<uint16_t>(string_input, &value_buffer)) {
                    (*ic_target)->settings.polling_rate = value_buffer;
                } else {
                    std::cout << "Press any key to continue." << std::endl;
                    _getch();
                }

                break;
            }

            case '3' : {
                uint16_t value_buffer;

                std::cout << "New rate(ms) ~ ";
                std::string string_input;
                std::cin >> string_input;

                if(AutoNumericalInput<uint16_t>(string_input, &value_buffer)) {
                    (*ic_target)->settings.mlsettings->setcursor_interval = value_buffer;
                } else {
                    std::cout << "Press any key to continue." << std::endl;
                    _getch();
                }

                break;
            }

            case '4' : {
                (*ic_target)->settings.play_sound ^= true;
                break;
            }
        };
    }
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
    IConditionChecker* ic_polymorphic = new ProcessChecker("tesv.exe");
    
    this->Dialogue_SelectMode(&ic_polymorphic, false);
    bool exit_loop = false;

    while(!exit_loop) {
        system("cls");
        std::cout << "1.) Mode: ";

        switch(ic_polymorphic->ID) {
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

        std::cout << "2.) State: " << (ic_polymorphic->thread_enabled  ? "Enabled" : "Disabled") << std::endl;
        std::cout << "3.) Configure" << std::endl;
        std::cout << std::endl << "0.) Exit" << std::endl;

        char selection = '\0';
        while(selection != '1' && selection != '2' && selection != '3' && selection != '0') 
            selection = _getch();

        switch(selection) {
            case '1' : {
                this->Dialogue_SelectMode(&ic_polymorphic, true);
                break;
            }

            case '2' : {
                ic_polymorphic->thread_enabled ^= true;
                break;
            }

            case '3' : {
                switch(ic_polymorphic->ID) {
                    case IConditionChecker::ICCID : {
                        break;
                    }

                    case InputChecker::ICID : {
                        this->Dialogue_InputChecker(reinterpret_cast<InputChecker**>(&ic_polymorphic));
                        break;
                    }

                    case ProcessChecker::PCID : {
                        this->Dialogue_ProcessChecker(reinterpret_cast<ProcessChecker**>(&ic_polymorphic));
                        break;
                    }
                };

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

    delete ic_polymorphic;
}

UiCore::UiCore() {

}

UiCore::~UiCore() {

}