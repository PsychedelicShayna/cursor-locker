#include "main_wnd.hxx"
#include "ui_main_wnd.h"

enum struct MONITOR_FOR {
    NOTHING         =   0b00000000,
    KEYBIND         =   0b00000001,
    PROCESS_IMAGE   =   0b00000010,
    WINDOW_TITLE    =   0b00000100,
};

bool MainWindow::loadStylesheetFile(const std::string& file_path) {
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

void MainWindow::beepBoop(QList<QPair<int, int>> freqdur_list) {
    if(!muteBeepBoop) {
        for(QPair<int,int>& pair : freqdur_list) {
            Beep(pair.first, pair.second);
        }
    }
}

void MainWindow::logToConsole(const QList<QString>& message_list) {
    QString concatenated_messages;

    for(const auto& message : message_list) {
        concatenated_messages.append(message);
    }

    concatenated_messages += '\n';

    ui->txt_console->moveCursor(QTextCursor::End);
    ui->txt_console->insertPlainText(concatenated_messages);
}

void MainWindow::logToConsole(const char* message) {
    ui->txt_console->moveCursor(QTextCursor::End);
    ui->txt_console->insertPlainText(QString(message) + '\n');
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
        beepBoop({{500,20}, {700, 20}});
    } else {
        logToConsole("Deactivated cursor lock via hotkey.");
        beepBoop({{700,20}, {500, 20}});
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

                beepBoop({{500,20}, {700,20}});
            }
        } else {
            disableCursorLock();

            if(!first_find) {
                logToConsole({"Lost target process: ", targetProcessImageName});
                first_find = true;

                beepBoop({{700,20}, {500,20}});
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

            beepBoop({{500,20}, {700,20}});
        }
    } else {
        disableCursorLock();

        if(!first_find) {
            logToConsole({"Lost target window: ", targetForegroundWindowTitle});
            first_find = true;

            beepBoop({{700,20}, {500,20}});
        }
    }
}

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
        beepBoop({{200, 20}});
    }

    HWND foreground_window = GetForegroundWindow();

    if(foreground_window != HWND(winId())) {
        reset_timer_lambda();

        char window_title_buffer[256];
        std::fill(window_title_buffer, window_title_buffer + sizeof(window_title_buffer), 0x00);
        GetWindowText(foreground_window, window_title_buffer, sizeof(window_title_buffer));

        targetForegroundWindowTitle = QString(window_title_buffer);
        ui->lin_activation_parameter->setText(targetForegroundWindowTitle);

        beepBoop({{900, 20}, {900, 20}});
    }
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

void MainWindow::changeActivationMethod(int method_index) {
    disconnect(this, &MainWindow::targetHotkeyVkidPressedSignal, this, &MainWindow::targetHotkeyVkidPressedSlot);
    unregisterTargetHotkey();

    if(selectedActivationMethodFunction != nullptr) {
        disconnect(checkActivationMethodTimer, &QTimer::timeout, this, selectedActivationMethodFunction);
        selectedActivationMethodFunction = nullptr;
        checkActivationMethodTimer->stop();
    }

    ui->lin_activation_parameter->clear();
    disableCursorLock();

    switch(method_index) {
        case 0 : {
            selectedActivationMethod = MONITOR_FOR::NOTHING;
            ui->lin_activation_parameter->setPlaceholderText("No activation method selected");
            break;
        }

        case 1 : {
            selectedActivationMethod = MONITOR_FOR::KEYBIND;
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
            selectedActivationMethod = MONITOR_FOR::PROCESS_IMAGE;
            selectedActivationMethodFunction = &MainWindow::activateIfTargetImagePresent;
            ui->lin_activation_parameter->setPlaceholderText("Image name, e.g. TESV.exe, SkyrimSE.exe, etc");

            if(targetProcessImageName.size()) {
                ui->lin_activation_parameter->setText(QString(targetProcessImageName));
            }

            logToConsole("Activation mode set to process image.");

            break;
        }

        case 3 : {
            selectedActivationMethod = MONITOR_FOR::WINDOW_TITLE;
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

void MainWindow::editActivationMethodParameter() {
    if(ui->btn_edit_activation_parameter->text() == "Edit" && selectedActivationMethod != MONITOR_FOR::NOTHING) {
        ui->lin_activation_parameter->setEnabled(true);
        ui->btn_edit_activation_parameter->setText("Confirm");
    } else if(ui->btn_edit_activation_parameter->text() == "Confirm") {
        switch(selectedActivationMethod) {
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

void MainWindow::startForegroundWindowGrabber() {
    if(selectedActivationMethod == MONITOR_FOR::WINDOW_TITLE) {
        if(ui->btn_edit_activation_parameter->text() == "Edit" && ui->btn_edit_activation_parameter->isEnabled()) {
            ui->btn_edit_activation_parameter->setEnabled(false);
            targetNextForegroundWindowTimer->start(500);
        }
    }
}

void MainWindow::toggleMuteBeepBoop() {
    if(muteBeepBoop) {
        muteBeepBoop = false;
        ui->btn_mutebeepboop->setText("Mute");
    } else {
        muteBeepBoop = true;
        ui->btn_mutebeepboop->setText("Unmute");
    }
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow), targetHotkeyId(420) {
    ui->setupUi(this);

    // Configure Main Window Flags & Set Initial Window Size
    // --------------------------------------------------
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
    selectedActivationMethod = MONITOR_FOR::NOTHING;
    targetHotkeyVkid = 0;

    checkActivationMethodTimer = new QTimer(this);
    targetNextForegroundWindowTimer = new QTimer(this);

    // Event Connections
    // --------------------------------------------------
    connect(targetNextForegroundWindowTimer, &QTimer::timeout, this, &MainWindow::targetNextForegroundWindow);

    // Class SLOT() connections.
    connect(ui->cbx_activation_method, SIGNAL(currentIndexChanged(int)), this, SLOT(changeActivationMethod(int)));
    connect(ui->btn_edit_activation_parameter, SIGNAL(clicked()), this, SLOT(editActivationMethodParameter()));
    connect(ui->btn_mutebeepboop, SIGNAL(clicked()), this, SLOT(toggleMuteBeepBoop()));

    // Load JSON Defaults
    // --------------------------------------------------
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
                static const Json method_resolver = {{"vkid", 1}, {"image", 2}, {"title", 3}};

                if(method_resolver.contains(method_string)) {
                    ui->cbx_activation_method->setCurrentIndex(method_resolver[method_string].get<uint8_t>());
                    logToConsole({"defaults.json: Activation method exists!"});
                } else {
                    logToConsole({"defaults.json: Invalid activation method \"", QString::fromStdString(method_string), "\" for \"method\" key."});
                }
            }

            if(default_values.contains("muted") && default_values["muted"].is_boolean()) {
                const bool& muted_value = default_values["muted"].get<bool>();
                muteBeepBoop = muted_value;
                ui->btn_mutebeepboop->setText("Unmute");
            }
        }
    } else {
        std::ofstream output_stream("./defaults.json", std::ios::binary);
        
        if(output_stream.good()) {
            Json json_template = {
                {"vkid", ""},
                {"image", ""},
                {"title", ""},
                {"method", ""},
                {"muted", false}
            };
            
            const std::string& dumped_json = json_template.dump(4);
            
            output_stream.write(dumped_json.data(), dumped_json.size());
            output_stream.close();

            logToConsole("Generated defaults.json. If you wish to store default values, please fill it.");
        } else {
            logToConsole("Tried generating defaults.json, but the stream was bad. Are you sure this program has permission to write to the current directory?");
        }
        
    }
    
    if(loadStylesheetFile("./style_sheet.qss")) {
        logToConsole("Loaded QSS stylesheet ./style_sheet.qss - theme has been applied.");
    } else {
        logToConsole("Cannot open style_sheet.qss, using default Windows style.");
    }

    // Install Custom Event Filter For Right Clicks
    // --------------------------------------------------
    static struct RightClickEventFilter : public QObject {
        MainWindow* mainWindowParent;

        RightClickEventFilter(MainWindow* main_window_parent) : mainWindowParent(main_window_parent) {}

        inline bool eventFilter(QObject* watched, QEvent* event) override {
            if(event->type() == QEvent::MouseButtonPress)    {
                QMouseEvent* mouse_event = reinterpret_cast<QMouseEvent*>(event);

                if(mouse_event->button() == Qt::RightButton) {
                    mainWindowParent->startForegroundWindowGrabber();
                }
            }

            return false;
        }
    } right_click_event_filter(this);

    ui->btn_edit_activation_parameter->installEventFilter(&right_click_event_filter);
}

MainWindow::~MainWindow() {
    delete ui;
}
