#include "main_window_dlg.hxx"
#include "ui_main_window_dlg.h"

enum struct MONITOR_FOR {
    NOTHING         =   0b00000000,
    KEYBIND         =   0b00000001,
    PROCESS_IMAGE   =   0b00000010,
    WINDOW_TITLE    =   0b00000100,
};

void MainWindow::monitoringWorker() {
    for(;;) {
        HWND foreground_window_handle = GetForegroundWindow();
        RECT cursor_cage_rect;

        GetWindowRect(foreground_window_handle, &cursor_cage_rect);

        if(monitoringWorkerMode == MONITOR_FOR::KEYBIND) {
            static bool clip_state_toggle = 0;
            
            if((GetAsyncKeyState(monitoringWorkerVkid) & 0x8000) != 0) {
                clip_state_toggle ^= 1;
                
                ClipCursor(clip_state_toggle ? &cursor_cage_rect : nullptr);

                Beep(clip_state_toggle ? 500 : 700, 20);
                Beep(clip_state_toggle ? 700 : 500, 20);

                logToConsole({clip_state_toggle ? "Activated" : "Deactivated", " lock."});

                Sleep(500);
            } else {
                ClipCursor(clip_state_toggle ? &cursor_cage_rect : nullptr);
            }
            
            Sleep(60);
        } else if(monitoringWorkerMode == MONITOR_FOR::PROCESS_IMAGE) {
            HANDLE running_tasks_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,  0);

            if(running_tasks_snapshot == INVALID_HANDLE_VALUE) {
                logToConsole("Invalid handle value for snapshot of type TH32CS_SNAPPROCESS. Cannot see running tasks.");
                continue;
            } else {
                PROCESSENTRY32 process_entry_32;
                process_entry_32.dwSize = sizeof(PROCESSENTRY32);

                if(Process32First(running_tasks_snapshot, &process_entry_32)) {
                    static bool first_find = true;
                    bool process_found = false;

                    do {
                        if(!stricmp(monitoringWorkerImage, process_entry_32.szExeFile)) {
                            process_found = true;
                            break;
                        }
                    } while(Process32Next(running_tasks_snapshot, &process_entry_32));

                    if(process_found) {
                        ClipCursor(&cursor_cage_rect);

                        if(first_find) {
                            logToConsole({"Found target process: ", QString(monitoringWorkerImage)});
                            first_find = false;

                            Beep(500, 20);
                            Beep(700, 20);
                        }
                    } else {
                        ClipCursor(nullptr);

                        if(!first_find) {
                            logToConsole({"Lost target process: ", QString(monitoringWorkerImage)});
                            first_find = true;

                            Beep(700, 20);
                            Beep(500, 20);
                        }
                    }
                }
            }

            CloseHandle(running_tasks_snapshot);

            Sleep(1000);
        } else if(monitoringWorkerMode == MONITOR_FOR::WINDOW_TITLE) {
            static bool first_find = true;

            char window_title_buffer[256];
            std::fill(window_title_buffer, window_title_buffer + sizeof(window_title_buffer), 0x00);

            HWND foreground_window = GetForegroundWindow();
            GetWindowTextA(foreground_window, window_title_buffer, sizeof(window_title_buffer));

            if(!stricmp(monitoringWorkerTitle, window_title_buffer)) {
                ClipCursor(&cursor_cage_rect);

                if(first_find) {
                    logToConsole({"Found target window: ", QString(monitoringWorkerTitle)});
                    first_find = false;

                    Beep(500, 20);
                    Beep(700, 20);
                }
            } else {
                    ClipCursor(nullptr);

                    if(!first_find) {
                        logToConsole({"Lost target window: ", QString(monitoringWorkerTitle)});
                        first_find = true;

                        Beep(700, 20);
                        Beep(500, 20);
                    }
            }

            Sleep(1000);
        }
    }
}

void MainWindow::acquireWindowWorker() {
    for(;;Sleep(1000)) {
        if(acquireWindowThreadSignal.load() == true) {
            acquireWindowThreadSignal.store(false);

            HWND old_foreground_window = GetForegroundWindow();
            HWND new_foreground_window = old_foreground_window;

            for(uint32_t tries = 0; (new_foreground_window == old_foreground_window) && tries < 10; ++tries) {
                std::stringstream placeholder_stream;
                placeholder_stream << "Click on the target window to capture its title (" << tries << " / 10)";

                ui->lin_activation_parameter->clear();
                ui->lin_activation_parameter->setPlaceholderText(QString::fromStdString(placeholder_stream.str()));

                new_foreground_window = GetForegroundWindow();
                Beep(200, 20);
                Sleep(500);
            }

            if(new_foreground_window != old_foreground_window) {
                Beep(900, 20);
                std::array<char, 256> new_window_title_buffer;
                std::fill(new_window_title_buffer.begin(), new_window_title_buffer.end(), 0x00);

                GetWindowText(new_foreground_window, new_window_title_buffer.data(), new_window_title_buffer.size()-1);
                std::string new_window_title(new_window_title_buffer.data());

                delete monitoringWorkerTitle.load();
                monitoringWorkerTitle = new char[new_window_title.size() + 1];
                memset(monitoringWorkerTitle.load(), 0x00, new_window_title.size() + 1);
                std::copy(new_window_title.begin(), new_window_title.end(), monitoringWorkerTitle.load());

                ui->lin_activation_parameter->setText(QString::fromStdString(new_window_title));
            }

            ui->lin_activation_parameter->setPlaceholderText("E.g. Skyrim, Skyrim Special Edition, etc");
        }
    }
}

void MainWindow::on_cbx_activation_method_currentIndexChanged(int index) {
    switch(index) {
        case 0 : {
            monitoringWorkerMode = MONITOR_FOR::NOTHING;
            ui->lin_activation_parameter->setText("");
            ui->lin_activation_parameter->setPlaceholderText("No activation method selected");
            logToConsole("Activation mode set to nothing.");
            
            ClipCursor(NULL);
            
            Beep(700, 20);
            Beep(600, 20);
            Beep(500, 20);
            
            break;
        }

        case 1 : {
            monitoringWorkerMode = MONITOR_FOR::KEYBIND;
            
            if(monitoringWorkerVkid.load() != 0) {
                std::stringstream conversion_stream;
                conversion_stream << "0x" << std::uppercase << std::hex << monitoringWorkerVkid.load();
            
                std::string converted_string;
                conversion_stream >> converted_string;
            
                ui->lin_activation_parameter->setText(QString::fromStdString(converted_string));
            } else {
                ui->lin_activation_parameter->setText("");
            }
            
            ui->lin_activation_parameter->setPlaceholderText("VKID, e.g. 0x6A (Numpad *)");
            logToConsole("Activation mode set to keybind, VKID list: http://www.kbdedit.com/manual/low_level_vk_list.html");
            break;
        }

        case 2 : {
            monitoringWorkerMode = MONITOR_FOR::PROCESS_IMAGE;
            ui->lin_activation_parameter->setText(QString::fromStdString(monitoringWorkerImage.load()));
            ui->lin_activation_parameter->setPlaceholderText("E.g. TESV.exe, SkyrimSE.exe, etc");
            logToConsole("Activation mode set to process image.");
            break;
        }

        case 3 : {
            monitoringWorkerMode = MONITOR_FOR::WINDOW_TITLE;
            ui->lin_activation_parameter->setText(QString::fromStdString(monitoringWorkerTitle.load()));
            ui->lin_activation_parameter->setPlaceholderText("E.g. Skyrim, Skyrim Special Edition, etc");
            logToConsole("Activation mode set to window title. Right click the edit button to grab the name of the next foreground window automatically.");
            break;
        }
    }
}

void MainWindow::on_btn_edit_activation_parameter_clicked() {
    if(ui->btn_edit_activation_parameter->text() == "Edit" && monitoringWorkerMode != MONITOR_FOR::NOTHING) {
        ui->lin_activation_parameter->setEnabled(true);
        ui->btn_edit_activation_parameter->setText("Confirm");
    } else if(ui->btn_edit_activation_parameter->text() == "Confirm") {
        switch(monitoringWorkerMode) {
            case MONITOR_FOR::KEYBIND : {
                try {
                    std::stringstream conversion_stream;
                    conversion_stream << std::hex << ui->lin_activation_parameter->text().toStdString();

                    uint32_t converted_vkid;
                    conversion_stream >> converted_vkid;

                    monitoringWorkerVkid = converted_vkid;
                } catch (const std::exception& exception) {
                    logToConsole({"Exception when trying to convert hexadecimal VKID text to an integer: ", exception.what()});
                }

                break;
            }

            case MONITOR_FOR::PROCESS_IMAGE : {
                delete monitoringWorkerImage;

                const std::string& process_image_string = ui->lin_activation_parameter->text().toStdString();
                monitoringWorkerImage = new char[process_image_string.size() + 1];
                std::copy(process_image_string.data(), process_image_string.data() + process_image_string.size() + 1, monitoringWorkerImage.load());
                break;
            }

            case MONITOR_FOR::WINDOW_TITLE : {
                delete monitoringWorkerTitle;

                const std::string& window_title_string = ui->lin_activation_parameter->text().toStdString();
                monitoringWorkerTitle = new char[window_title_string.size() + 1];
                std::copy(window_title_string.data(), window_title_string.data() + window_title_string.size() + 1, monitoringWorkerTitle.load());
                break;
            }

            case MONITOR_FOR::NOTHING : {
                break;
            }
        }

        ui->lin_activation_parameter->setEnabled(false);
        ui->btn_edit_activation_parameter->setText("Edit");
    }
}

void MainWindow::on_btn_edit_activation_parameter_right_clicked() {
    acquireWindowThreadSignal.store(true);
}

void MainWindow::logToConsole(const QList<QString>& message_list) {
    std::lock_guard<std::mutex> console_mutex_guard(consoleMutex);
    QString concatenated_messages;

    for(const auto& message : message_list) {
        concatenated_messages.append(message);
    }

    concatenated_messages += '\n';

    ui->txt_console->moveCursor(QTextCursor::End);
    ui->txt_console->insertPlainText(concatenated_messages);
}

void MainWindow::logToConsole(const char* message) {
    std::lock_guard<std::mutex> console_mutex_guard(consoleMutex);
    
    ui->txt_console->moveCursor(QTextCursor::End);
    ui->txt_console->insertPlainText(QString(message) + '\n');
}

void MainWindow::closeEvent(QCloseEvent*) {
    ClipCursor(nullptr);
}

bool MainWindow::LoadStylesheetFile(const std::string& file_path) {
    std::ifstream input_stream(file_path, std::ios::binary);
    if(input_stream.good()) {
        std::string style_sheet((std::istreambuf_iterator<char>(input_stream)), (std::istreambuf_iterator<char>()));
        input_stream.close();

        setStyleSheet(QString::fromStdString(style_sheet));
        return true;
    } else {
        return false;
    }
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    static struct RightClickEventFilter : public QObject {
        MainWindow* mainWindowParent;

        RightClickEventFilter(MainWindow* main_window_parent) : mainWindowParent(main_window_parent) {}

        inline bool eventFilter(QObject* watched, QEvent* event) override {
            if(event->type() == QEvent::MouseButtonPress)    {
                QMouseEvent* mouse_event = reinterpret_cast<QMouseEvent*>(event);

                if(mouse_event->button() == Qt::RightButton) {
                    mainWindowParent->on_btn_edit_activation_parameter_right_clicked();
                }
            }

            return false;
        }
    } right_click_event_filter(this);

    ui->btn_edit_activation_parameter->installEventFilter(&right_click_event_filter);

    setWindowFlags(
        Qt::Dialog |
        Qt::CustomizeWindowHint |
        Qt::WindowTitleHint |
        Qt::WindowCloseButtonHint |
        Qt::WindowMinimizeButtonHint |
        Qt::WindowMaximizeButtonHint
    );
    
    HWND desktop_window_handle = GetDesktopWindow();
    RECT desktop_window_rect;

    GetWindowRect(desktop_window_handle, &desktop_window_rect);
    CloseHandle(desktop_window_handle);
    
    resize(20 * desktop_window_rect.right / 100, 10 * desktop_window_rect.bottom / 100);

    monitoringWorkerMode    = MONITOR_FOR::NOTHING;
    monitoringWorkerVkid    = 0x00;
    monitoringWorkerImage   = new char[2];
    monitoringWorkerTitle   = new char[2];
    acquireWindowThreadSignal = false;

    std::fill(monitoringWorkerImage.load(), monitoringWorkerImage.load() + 1, 0x00);
    std::fill(monitoringWorkerTitle.load(), monitoringWorkerTitle.load() + 1, 0x00);
    
    std::ifstream input_stream("./defaults.json", std::ios::binary);
    
    if(input_stream.good()) {
        std::string raw_json_string((std::istreambuf_iterator<char>(input_stream)), (std::istreambuf_iterator<char>()));
        input_stream.close();
        
        Json default_values = Json::parse(raw_json_string);
        
        logToConsole("Default values from defaults.json have been parsed.");
        
        for(const auto& pair : default_values.items()) {
            if(pair.key() == "vkid") {
                std::stringstream conversion_stream;
                conversion_stream << std::hex << pair.value().get<std::string>();
                
                uint32_t converted_value = 0;
                conversion_stream >> converted_value;
                
                monitoringWorkerVkid = converted_value;
                
                logToConsole({"Loaded VKID(", QString::fromStdString(pair.value()), ") from defaults.json"});
            } else if(pair.key() == "image") {
                const std::string& image = pair.value();
                
                char* heap_copy = new char[image.size() + 1];                
                std::copy(image.data(), image.data() + image.size() + 1, heap_copy);
                
                delete monitoringWorkerImage.load();                
                monitoringWorkerImage.store(heap_copy);                
                
                logToConsole({"Loaded image name(", QString::fromStdString(pair.value()), ") from defaults.json"});
            } else if(pair.key() == "title") {
                const std::string& title = pair.value();
                
                char* heap_copy = new char[title.size() + 1];
                std::copy(title.data(), title.data() + title.size() + 1, heap_copy);
                
                delete monitoringWorkerTitle.load();
                monitoringWorkerTitle.store(heap_copy);
                
                logToConsole({"Loaded window title(", QString::fromStdString(pair.value()),") from defaults.json"});
            }
        }
    } else {
        std::ofstream output_stream("./defaults.json", std::ios::binary);
        
        if(output_stream.good()) {
            Json json_template = {
                {"vkid", ""},
                {"image", ""},
                {"title", ""}
            };
            
            const std::string& dumped_json = json_template.dump(4);
            
            output_stream.write(dumped_json.data(), dumped_json.size());
            output_stream.close();
        }
        
        logToConsole("Generated defaults.json. If you wish to store default values, please fill it.");
    }
        
    MonitoringThread = std::thread([this]() -> void { monitoringWorker(); });
    AcquireWindowThread = std::thread([this]() -> void { acquireWindowWorker(); });
    
    if(LoadStylesheetFile("./style_sheet.qss")) {
        logToConsole("style_sheet.qss Loaded");
    } else {
        logToConsole("Cannot open style_sheet.qss, using default style.");
    }
}

MainWindow::~MainWindow() {
    delete ui;
}