#include "main_wnd.hxx"
#include "ui_main_wnd.h"

enum struct MONITOR_FOR {
    NOTHING         =   0b00000000,
    KEYBIND         =   0b00000001,
    PROCESS_IMAGE   =   0b00000010,
    WINDOW_TITLE    =   0b00000100,
};

void MainWindow::BeepBoop(QList<QPair<int, int>> freqdur_list) {
    if(!muteBeepBoop) {
        for(QPair<int,int>& pair : freqdur_list) {
            Beep(pair.first, pair.second);
        }
    }
}

RECT MainWindow::getForegroundWindowRect() {
    HWND foreground_window_handle = GetForegroundWindow();
    RECT foreground_window_rect;

    GetWindowRect(foreground_window_handle, &foreground_window_rect);

    return foreground_window_rect;
}


void MainWindow::enableCursorLock() {
    RECT foreground_window_rect = getForegroundWindowRect();
    ClipCursor(&foreground_window_rect);
}

void MainWindow::disableCursorLock() {
    ClipCursor(nullptr);
}

bool MainWindow::toggleCursorLock() {
    static bool toggle = false;

    if(toggle ^= true) {
        enableCursorLock();
    } else {
        disableCursorLock();
    }

    return toggle;
}

bool MainWindow::registerTargetHotkey() {
    return RegisterHotKey(HWND(winId()), targetHotkeyId, MOD_NOREPEAT, targetHotkeyVkid);
}

bool MainWindow::unregisterTargetHotkey() {
    return UnregisterHotKey(HWND(winId()), targetHotkeyId);
}

void MainWindow::targetHotkeyVkidPressedSlot() {
    if(toggleCursorLock()) {
        logToConsole("Activated cursor lock via hotkey.");
        BeepBoop({{500,20}, {700, 20}});
    } else {
        logToConsole("Deactivated cursor lock via hotkey.");
        BeepBoop({{700,20}, {500, 20}});
    }
}

void MainWindow::activateIfTargetImagePresent() {
    HANDLE process_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,  0);

    if(process_snapshot == INVALID_HANDLE_VALUE) {
        logToConsole("Invalid handle value for snapshot of type TH32CS_SNAPPROCESS. Cannot see running tasks.");
        return;
    }

    PROCESSENTRY32 process_entry_32;
    process_entry_32.dwSize = sizeof(PROCESSENTRY32);

    if(Process32First(process_snapshot, &process_entry_32)) {
        static bool first_find = true;
        bool process_found = false;

        do {
            if(targetProcessImageName == process_entry_32.szExeFile) {
                process_found = true;
                break;
            }
        } while(Process32Next(process_snapshot, &process_entry_32));

        if(process_found) {
            enableCursorLock();

            if(first_find) {
                logToConsole({"Found target process: ", targetProcessImageName});
                first_find = false;

                BeepBoop({{500,20}, {700,20}});
            }
        } else {
            disableCursorLock();

            if(!first_find) {
                logToConsole({"Lost target process: ", targetProcessImageName});
                first_find = true;

                BeepBoop({{700,20}, {500,20}});
            }
        }
    }

    CloseHandle(process_snapshot);
}

void MainWindow::activateIfForegroundWindowMatchesTarget() {
    static bool first_find = true;

    char window_title_buffer[256];
    std::fill(window_title_buffer, window_title_buffer + sizeof(window_title_buffer), 0x00);

    HWND foreground_window = GetForegroundWindow();
    GetWindowTextA(foreground_window, window_title_buffer, sizeof(window_title_buffer));

    if(targetForegroundWindowTitle == window_title_buffer) {
        enableCursorLock();

        if(first_find) {
            logToConsole({"Found target window: ", targetForegroundWindowTitle});
            first_find = false;

            BeepBoop({{500,20}, {700,20}});
        }
    } else {
        disableCursorLock();

        if(!first_find) {
            logToConsole({"Lost target window: ", targetForegroundWindowTitle});
            first_find = true;

            BeepBoop({{700,20}, {500,20}});
        }
    }
}

void MainWindow::on_cbx_activation_method_currentIndexChanged(int index) {
    if(unregisterTargetHotkey() != 0) {
        disconnect(this, &MainWindow::targetHotkeyVkidPressedSignal, this, &MainWindow::targetHotkeyVkidPressedSlot);
    }

    if(selectedActivationMethodFunction != nullptr) {
        disconnect(checkActivationMethodTimer, &QTimer::timeout, this, selectedActivationMethodFunction);
        selectedActivationMethodFunction = nullptr;
        checkActivationMethodTimer->stop();
    }

    ui->lin_activation_parameter->clear();
    disableCursorLock();

    switch(index) {
        case 0 : {
            selectedActivationCondition = MONITOR_FOR::NOTHING;
            ui->lin_activation_parameter->setPlaceholderText("No activation method selected");
            // BeepBoop({{700,20}, {600,20}, {500,20}});
            break;
        }

        case 1 : {
            selectedActivationCondition = MONITOR_FOR::KEYBIND;
            connect(this, &MainWindow::targetHotkeyVkidPressedSignal, this, &MainWindow::targetHotkeyVkidPressedSlot);
            registerTargetHotkey();

            ui->lin_activation_parameter->setPlaceholderText("VKID, e.g. 0x6A (Numpad *)");

            if(targetHotkeyVkid != 0) {
                char hex_vkid[5];
                std::fill(hex_vkid, hex_vkid + sizeof(hex_vkid), 0x00);
                sprintf(hex_vkid, "0x%02X", targetHotkeyVkid);
                ui->lin_activation_parameter->setText(QString(hex_vkid));
            }

            logToConsole("Activation mode set to keybind, VKID list: http://www.kbdedit.com/manual/low_level_vk_list.html");

            break;
        }

        case 2 : {
            selectedActivationCondition = MONITOR_FOR::PROCESS_IMAGE;
            selectedActivationMethodFunction = &MainWindow::activateIfTargetImagePresent;
            ui->lin_activation_parameter->setPlaceholderText("Image name, e.g. TESV.exe, SkyrimSE.exe, etc");

            if(targetProcessImageName.size()) {
                ui->lin_activation_parameter->setText(QString(targetProcessImageName));
            }

            logToConsole("Activation mode set to process image.");

            break;
        }

        case 3 : {
            selectedActivationCondition = MONITOR_FOR::WINDOW_TITLE;
            selectedActivationMethodFunction = &MainWindow::activateIfForegroundWindowMatchesTarget;
            ui->lin_activation_parameter->setPlaceholderText("Window title, e.g. Skyrim, Skyrim Special Edition");

            if(targetForegroundWindowTitle.size()) {
                ui->lin_activation_parameter->setText(QString(targetForegroundWindowTitle));
            }

            logToConsole("Activation mode set to window title. Right click the edit button to grab the name of the next foreground window that you select.");

            break;
        }
    }

    if(selectedActivationMethodFunction != nullptr) {
        connect(checkActivationMethodTimer, &QTimer::timeout, this, selectedActivationMethodFunction);
        checkActivationMethodTimer->start(500);
    }
}

void MainWindow::on_btn_edit_activation_parameter_clicked() {
    if(ui->btn_edit_activation_parameter->text() == "Edit" && selectedActivationCondition != MONITOR_FOR::NOTHING) {
        ui->lin_activation_parameter->setEnabled(true);
        ui->btn_edit_activation_parameter->setText("Confirm");
    } else if(ui->btn_edit_activation_parameter->text() == "Confirm") {
        switch(selectedActivationCondition) {
            case MONITOR_FOR::KEYBIND : {
                try {
                    unregisterTargetHotkey();

                    QString vkid_str = ui->lin_activation_parameter->text();

                    if(vkid_str.size()) {
                        uint8_t vkid = static_cast<uint8_t>(strtol(vkid_str.toStdString().c_str(), nullptr, 16));

                        if(vkid != 0) {
                            char hex_vkid[5];
                            std::fill(hex_vkid, hex_vkid + sizeof(hex_vkid), 0x00);
                            sprintf(hex_vkid, "0x%02X", vkid);
                            ui->lin_activation_parameter->setText(hex_vkid);

                            targetHotkeyVkid = vkid;
                            registerTargetHotkey();
                        } else {
                            logToConsole({"Invalid hexadecimal value '", vkid_str, "'"});
                            ui->lin_activation_parameter->clear();
                        }
                    }
                } catch (const std::exception& exception) {
                    logToConsole({"Exception when trying to convert hexadecimal VKID text to an integer: ", exception.what()});
                }

                break;
            }

            case MONITOR_FOR::PROCESS_IMAGE : {
                targetProcessImageName = ui->lin_activation_parameter->text();
                break;
            }

            case MONITOR_FOR::WINDOW_TITLE : {
                targetForegroundWindowTitle = ui->lin_activation_parameter->text();
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
    if(selectedActivationCondition == MONITOR_FOR::WINDOW_TITLE) {
        if(ui->btn_edit_activation_parameter->text() == "Edit" && ui->btn_edit_activation_parameter->isEnabled()) {
            ui->btn_edit_activation_parameter->setEnabled(false);
            targetNextForegroundWindowTimer->start(500);
        }
    }
}

void MainWindow::on_btn_mutebeepboop_clicked() {
    if(muteBeepBoop) {
        muteBeepBoop = false;
        ui->btn_mutebeepboop->setText("Mute");
    } else {
        muteBeepBoop = true;
        ui->btn_mutebeepboop->setText("Unmute");
    }
}

/*

void MainWindow::on_cbx_activation_method_currentIndexChanged(int index) {
    switch(index) {
        case 0 : {
            monitoringWorkerMode = MONITOR_FOR::NOTHING;
            ui->lin_activation_parameter->setText("");
            ui->lin_activation_parameter->setPlaceholderText("No activation method selected");
            logToConsole("Activation mode set to nothing.");

            ClipCursor(NULL);

            if(!muteBeepBoop) {
                Beep(700, 20);
                Beep(600, 20);
                Beep(500, 20);
            }

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
    if(monitoringWorkerMode == MONITOR_FOR::WINDOW_TITLE) {
        acquireWindowThreadSignal.store(true);
    }
}

void MainWindow::on_btn_mutebeepboop_clicked() {
    if(muteBeepBoop.load()) {
        muteBeepBoop = false;
        ui->btn_mutebeepboop->setText("Mute");
    } else {
        muteBeepBoop = true;
        ui->btn_mutebeepboop->setText("Unmute");
    }
}

*/


/*
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

                if(!muteBeepBoop) {
                    Beep(clip_state_toggle ? 500 : 700, 20);
                    Beep(clip_state_toggle ? 700 : 500, 20);
                }

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

                            if(!muteBeepBoop) {
                                Beep(500, 20);
                                Beep(700, 20);
                            }
                        }
                    } else {
                        ClipCursor(nullptr);

                        if(!first_find) {
                            logToConsole({"Lost target process: ", QString(monitoringWorkerImage)});
                            first_find = true;

                            if(!muteBeepBoop) {
                                Beep(700, 20);
                                Beep(500, 20);
                            }
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

                    if(!muteBeepBoop) {
                        Beep(500, 20);
                        Beep(700, 20);
                    }
                }
            } else {
                    ClipCursor(nullptr);

                    if(!first_find) {
                        logToConsole({"Lost target window: ", QString(monitoringWorkerTitle)});
                        first_find = true;

                        if(!muteBeepBoop) {
                            Beep(700, 20);
                            Beep(500, 20);
                        }
                    }
            }

            Sleep(1000);
        } else {
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

                if(!muteBeepBoop) {
                    Beep(200, 20);
                }

                Sleep(500);
            }

            if(new_foreground_window != old_foreground_window) {
                if(!muteBeepBoop) {
                    Beep(900, 20);
                    Beep(900, 20);
                    Sleep(3);
                }

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
*/

void MainWindow::targetNextForegroundWindow() {
    static uint32_t attempt_counter = 0;

    const auto& reset_timer_lambda = [&]() -> void {
        targetNextForegroundWindowTimer->stop();
        ui->lin_activation_parameter->clear();
        ui->btn_edit_activation_parameter->setEnabled(true);
        attempt_counter = 0;
    };

    if(++attempt_counter > 15) {
       reset_timer_lambda();
       return;
    } else {
        ui->lin_activation_parameter->setText("Switch to target window within timeframe (" + QString::number(attempt_counter) + " / 15 attempts)");
        BeepBoop({{200, 20}});
    }

    HWND foreground_window = GetForegroundWindow();

    if(foreground_window != HWND(winId())) {
        reset_timer_lambda();

        char window_title_buffer[256];
        std::fill(window_title_buffer, window_title_buffer + sizeof(window_title_buffer), 0x00);
        GetWindowText(foreground_window, window_title_buffer, sizeof(window_title_buffer));

        targetForegroundWindowTitle = QString(window_title_buffer);
        ui->lin_activation_parameter->setText(targetForegroundWindowTitle);

        BeepBoop({{900, 20}, {900, 20}});
    }
}

void MainWindow::logToConsole(const QList<QString>& message_list) {
    // std::lock_guard<std::mutex> console_mutex_guard(consoleMutex);
    QString concatenated_messages;

    for(const auto& message : message_list) {
        concatenated_messages.append(message);
    }

    concatenated_messages += '\n';

    ui->txt_console->moveCursor(QTextCursor::End);
    ui->txt_console->insertPlainText(concatenated_messages);
}

void MainWindow::logToConsole(const char* message) {
    // std::lock_guard<std::mutex> console_mutex_guard(consoleMutex);
    
    ui->txt_console->moveCursor(QTextCursor::End);
    ui->txt_console->insertPlainText(QString(message) + '\n');
}


void MainWindow::closeEvent(QCloseEvent*) {
    ClipCursor(nullptr);
}

bool MainWindow::nativeEvent(const QByteArray& event_type, void* message, long* result) {
    Q_UNUSED(event_type);
    Q_UNUSED(result);

    MSG* msg = reinterpret_cast<MSG*>(message);

    if (msg->message == WM_HOTKEY && msg->wParam == static_cast<uint64_t>(targetHotkeyId)) {
        emit targetHotkeyVkidPressedSignal();
        return true;
    }

    return QMainWindow::nativeEvent(event_type, message, result);
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
    
    resize(20 * desktop_window_rect.right / 100, 16 * desktop_window_rect.bottom / 100);

    // Member Variable Initialization
    // --------------------------------------------------
    muteBeepBoop = false;
    selectedActivationCondition = MONITOR_FOR::NOTHING;
    targetHotkeyVkid = 0;
    targetHotkeyId = 420;

    checkActivationMethodTimer = new QTimer(this);

    targetNextForegroundWindowTimer = new QTimer(this);
    connect(targetNextForegroundWindowTimer, &QTimer::timeout, this, &MainWindow::targetNextForegroundWindow);

    std::ifstream input_stream("./defaults.json", std::ios::binary);
    
    if(input_stream.good()) {
        std::string raw_json_string((std::istreambuf_iterator<char>(input_stream)), (std::istreambuf_iterator<char>()));
        input_stream.close();
        
        Json default_values;

        try {
            default_values = Json::parse(raw_json_string);
            logToConsole("Default values from defaults.json have been parsed.");
        } catch(const Json::exception& e) {
            logToConsole("Could not parse values from defaults.json, the file might be incorrectly formatted.");
            logToConsole({"Json::exception::what message: ", e.what()});
        } catch(const std::exception& e) {
            logToConsole("Unknown exception thrown when parsing values from defaults.json");
            logToConsole({"std::exception::what message: ", e.what()});
        }

        if(default_values.size()) {
            if(default_values.contains("vkid") && default_values["vkid"].is_string() && default_values["vkid"] != "") {
                const std::string& vkid_hex_string = default_values["vkid"].get<std::string>();
                uint8_t vkid = static_cast<uint8_t>(strtol(vkid_hex_string.c_str(), nullptr, 16));
                targetHotkeyVkid = vkid;

                logToConsole({"defaults.json: Loaded hotkey VKID '0x", QString::number(targetHotkeyVkid, 16), "'"});
            }

            if(default_values.contains("image") && default_values["image"].is_string() && default_values["image"] != "") {
                targetProcessImageName = QString::fromStdString(default_values["image"].get<std::string>());
                logToConsole({"defaults.json: Loaded process image name \"", targetProcessImageName, "\""});
            }

            if(default_values.contains("title") && default_values["title"].is_string() && default_values["title"] != "") {
                targetForegroundWindowTitle = QString::fromStdString(default_values["title"].get<std::string>());
                logToConsole({"defaults.json: Loaded foreground window title \"", targetForegroundWindowTitle, "\""});
            }

            if(default_values.contains("method") && default_values["method"].is_string() && default_values["method"] != "") {
                const std::string& method_string = default_values["method"].get<std::string>();
                static const Json method_resolver {{"vkid", 1}, {"image", 2}, {"title", 3}};

                if(method_resolver.contains(method_string)) {
                    ui->cbx_activation_method->setCurrentIndex(method_resolver[method_string].get<uint8_t>());
                } else {
                    logToConsole({"defaults.json: Invalid activation method \"", QString::fromStdString(method_resolver), "\" for \"method\" key."});
                }
            }
        }
    } else {
        std::ofstream output_stream("./defaults.json", std::ios::binary);
        
        if(output_stream.good()) {
            Json json_template = {
                {"vkid", ""},
                {"image", ""},
                {"title", ""},
                {"method", ""}
            };
            
            const std::string& dumped_json = json_template.dump(4);
            
            output_stream.write(dumped_json.data(), dumped_json.size());
            output_stream.close();
        }
        
        logToConsole("Generated defaults.json. If you wish to store default values, please fill it.");
    }
    
    if(LoadStylesheetFile("./style_sheet.qss")) {
        logToConsole("Loaded QSS stylesheet ./style_sheet.qss - theme has been applied.");
    } else {
        logToConsole("Cannot open style_sheet.qss, using default Windows style.");
    }
}

MainWindow::~MainWindow() {
    delete ui;
}
