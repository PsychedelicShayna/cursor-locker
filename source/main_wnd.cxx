#include "main_wnd.hxx"
#include "ui_main_wnd.h"

enum struct ACTIVATION_METHOD {
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

        logToConsole({"Read ", QString::number(style_sheet.size()), " bytes from stylesheet @ ", QString::fromStdString(file_path)}, CLOG_INFO);

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

void MainWindow::displayLogMessagesInConsole(bool just_add_latest) {
    auto l_print_message_pair = [this](const QPair<CONSOLE_LOG_LEVELS, QString>& pair) -> void {
        if(minimumLogLevel > pair.first) return;

        ui->txt_console->moveCursor(QTextCursor::End);

        static const QMap<CONSOLE_LOG_LEVELS, QString>& loglevel_resolver {
            {CLOG_INFO, "[INFO] "}, {CLOG_WARNING, "[WARNING] "},
            {CLOG_ERROR, "[ERROR] "}, {CLOG_EXCEPTION, "[EXCEPTION] "}
        };

        const QString& loglevel_str = loglevel_resolver.contains(pair.first)
                ? loglevel_resolver[pair.first]
                : "UNKNOWN";

        ui->txt_console->insertPlainText(loglevel_str + pair.second);
    };

    if(just_add_latest) {
        const auto& latest_pair = logMessages.last();
        l_print_message_pair(latest_pair);
    } else {
        ui->txt_console->clear();
        for(const auto& pair : logMessages) {
            l_print_message_pair(pair);
        }
    }

    ui->txt_console->verticalScrollBar()->setValue(ui->txt_console->verticalScrollBar()->maximum());
}

void MainWindow::logToConsole(const QList<QString>& message_list, CONSOLE_LOG_LEVELS loglevel) {
    QString concatenated_messages;

    for(const auto& message : message_list) {
        concatenated_messages.append(message);
    }

    concatenated_messages += '\n';

    logMessages.append(QPair<CONSOLE_LOG_LEVELS, QString>  {loglevel, concatenated_messages});
    displayLogMessagesInConsole();
}

void MainWindow::logToConsole(const char* message, CONSOLE_LOG_LEVELS loglevel) {
    logToConsole({QString(message)}, loglevel);
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
    bool result = RegisterHotKey(HWND(winId()), targetHotkeyId, MOD_NOREPEAT, targetHotkeyVkid);
    logToConsole({"RegisterHotKey(HWND(winId), 0x", QString::number(targetHotkeyId, 16), ", MOD_NOREPEAT, 0x", QString::number(targetHotkeyVkid, 16), ") returned ", result ? "true" : "false"}, CLOG_INFO);
    return result;
}

bool MainWindow::unregisterTargetHotkey() {
    bool result = UnregisterHotKey(HWND(winId()), targetHotkeyId);

    if(result) {
        logToConsole({"UnregisterHotKey(HWND(winId), 0x", QString::number(targetHotkeyId, 16), ") returned true"}, CLOG_INFO);
    }

    return result;
}

void MainWindow::closeEvent(QCloseEvent*) {
    logToConsole("closeEvent impl called", CLOG_INFO);
    disableCursorLock();
}

bool MainWindow::nativeEvent(const QByteArray& event_type, void* message, long* result) {
    Q_UNUSED(event_type);
    Q_UNUSED(result);

    MSG* msg = reinterpret_cast<MSG*>(message);

    if (msg->message == WM_HOTKEY) {
        logToConsole({"nativeEvent got MSG WM_HOTKEY, wParam=0x", QString::number(msg->wParam, 16), ", lParam=0x", QString::number(msg->lParam, 16)}, CLOG_INFO);

        if(msg->wParam == static_cast<uint64_t>(targetHotkeyId)) {
            emit targetHotkeyVkidPressedSignal();
            return true;
        }
    }

    return QMainWindow::nativeEvent(event_type, message, result);
}

void MainWindow::targetHotkeyVkidPressedSlot() {
    if(toggleCursorLock()) {
        logToConsole("Activated cursor lock via hotkey.", CLOG_INFO);
        beepBoop({{500,20}, {700, 20}});
    } else {
        logToConsole("Deactivated cursor lock via hotkey.", CLOG_INFO);
        beepBoop({{700,20}, {500, 20}});
    }
}

void MainWindow::activateIfTargetImagePresent() {
    HANDLE process_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,  0);

    if(process_snapshot == INVALID_HANDLE_VALUE) {
        logToConsole("Invalid handle value for snapshot of type TH32CS_SNAPPROCESS. Cannot see running tasks!", CLOG_ERROR);
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
                logToConsole({"Found target process: ", targetProcessImageName}, CLOG_INFO);
                first_find = false;

                beepBoop({{500,20}, {700,20}});
            }
        } else {
            disableCursorLock();

            if(!first_find) {
                logToConsole({"Lost target process: ", targetProcessImageName}, CLOG_INFO);
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

void MainWindow::changeActivationMethod(int method_index) {
    /* For activation methods that require a timer (window title & image name), the below timer
     * will be used to call the member function associated with the activation method, whose address
     * will be stored in the below member function pointer. The address is stored because when switching
     * to a new activation method, the timeout signal has to be disconnected from the old member function.
     * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
    static QTimer* timed_activation_method_timer = nullptr;
    static void(MainWindow::*timed_activation_method_mfptr)() = nullptr;

    // One-time allocation for static timed_activation_method_timer
    if(timed_activation_method_timer == nullptr) {
        timed_activation_method_timer = new QTimer(this);
    }

    /* Disconnect the timeout signal from the member function associated with the activation method, to
     * ensure that the timer doesn't keep executing the old activation method after it has been connected
     * to a new activation method member function. Also ensure the timer is stopped, and reset the member
     * function pointer to nullptr so this block doesn't run again in case no activation method is selected.
     * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
    if(timed_activation_method_mfptr != nullptr) {
        disconnect(timed_activation_method_timer, &QTimer::timeout, this, timed_activation_method_mfptr);
        timed_activation_method_mfptr = nullptr;
        timed_activation_method_timer->stop();
    }

    // Ensure that hotkeys are unregistered, and the hotkey pressed signal is disconnected from the slot.
    disconnect(this, &MainWindow::targetHotkeyVkidPressedSignal, this, &MainWindow::targetHotkeyVkidPressedSlot);
    unregisterTargetHotkey();

    /* Clear lin_activation_parameter's text to allow the placeholder text of
     * the selected activation method to be displayed later on.
     * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
    ui->lin_activation_parameter->clear();

    // Ensure the cursor lock is disabled before switching over to a new activation method.
    disableCursorLock();

    logToConsole("--------------------------------------------------");

    switch(method_index) {
    case 0 : {
        logToConsole("Activation method set to nothing.", CLOG_INFO);

        selectedActivationMethod = ACTIVATION_METHOD::NOTHING;
        ui->lin_activation_parameter->setPlaceholderText("No activation method selected");
        break;
    }

    case 1 : {
        logToConsole("Activation mode set to VKID keybind.", CLOG_INFO);
        logToConsole("VKID list: http://www.kbdedit.com/manual/low_level_vk_list.html", CLOG_INFO);

        selectedActivationMethod = ACTIVATION_METHOD::KEYBIND;
        connect(this, &MainWindow::targetHotkeyVkidPressedSignal, this, &MainWindow::targetHotkeyVkidPressedSlot);
        registerTargetHotkey();

        ui->lin_activation_parameter->setPlaceholderText("VKID, e.g. 0x6A (Numpad *)");

        if(targetHotkeyVkid != 0) {
            char hex_vkid[5];
            std::fill(hex_vkid, hex_vkid + sizeof(hex_vkid), 0x00);
            sprintf(hex_vkid, "0x%02X", targetHotkeyVkid);
            ui->lin_activation_parameter->setText(QString(hex_vkid));
        }

        break;
    }

    case 2 : {
        logToConsole("Activation mode set to process image.", CLOG_INFO);

        selectedActivationMethod = ACTIVATION_METHOD::PROCESS_IMAGE;
        timed_activation_method_mfptr = &MainWindow::activateIfTargetImagePresent;
        ui->lin_activation_parameter->setPlaceholderText("Image name, e.g. TESV.exe, SkyrimSE.exe, etc");

        if(targetProcessImageName.size()) {
            ui->lin_activation_parameter->setText(QString(targetProcessImageName));
        }

        break;
    }

    case 3 : {
        logToConsole("Activation mode set to window title. Right click the edit button to grab the name of the next foreground window that you select.", CLOG_INFO);

        selectedActivationMethod = ACTIVATION_METHOD::WINDOW_TITLE;
        timed_activation_method_mfptr = &MainWindow::activateIfForegroundWindowMatchesTarget;
        ui->lin_activation_parameter->setPlaceholderText("Window title, e.g. Skyrim, Skyrim Special Edition");

        if(targetForegroundWindowTitle.size()) {
            ui->lin_activation_parameter->setText(QString(targetForegroundWindowTitle));
        }

        break;
    }
    }

    if(timed_activation_method_mfptr != nullptr) {
        connect(timed_activation_method_timer, &QTimer::timeout, this, timed_activation_method_mfptr);
        timed_activation_method_timer->start(500);
    }
}

void MainWindow::editActivationMethodParameter() {
    if(ui->btn_edit_activation_parameter->text() == "Edit" && selectedActivationMethod != ACTIVATION_METHOD::NOTHING) {
        ui->lin_activation_parameter->setEnabled(true);
        ui->btn_edit_activation_parameter->setText("Confirm");
    } else if(ui->btn_edit_activation_parameter->text() == "Confirm") {
        switch(selectedActivationMethod) {
        case ACTIVATION_METHOD::KEYBIND : {
            unregisterTargetHotkey();

            QString vkid_str = ui->lin_activation_parameter->text();

            if(vkid_str.size()) {
                uint8_t vkid = static_cast<uint8_t>(strtol(vkid_str.toStdString().c_str(), nullptr, 16));

                if(vkid != 0) {
                    char hex_vkid[6];
                    std::fill(hex_vkid, hex_vkid + sizeof(hex_vkid), 0x00);
                    sprintf(hex_vkid, "0x%02X", vkid);
                    ui->lin_activation_parameter->setText(hex_vkid);

                    targetHotkeyVkid = vkid;
                    registerTargetHotkey();
                } else {
                    logToConsole({"Invalid hexadecimal value '", vkid_str, "' for activation method parameter."}, CLOG_WARNING);
                    ui->lin_activation_parameter->clear();
                }
            }

            break;
        }

        case ACTIVATION_METHOD::PROCESS_IMAGE : {
            targetProcessImageName = ui->lin_activation_parameter->text();
            break;
        }

        case ACTIVATION_METHOD::WINDOW_TITLE : {
            targetForegroundWindowTitle = ui->lin_activation_parameter->text();
            break;
        }

        case ACTIVATION_METHOD::NOTHING : {
            break;
        }
        }

        ui->lin_activation_parameter->setEnabled(false);
        ui->btn_edit_activation_parameter->setText("Edit");
    }
}

void MainWindow::startForegroundWindowGrabber() {
    static QTimer* grab_window_timer = nullptr;

    if(grab_window_timer == nullptr) {
        grab_window_timer = new QTimer(this);

        connect(grab_window_timer, &QTimer::timeout,
                [this]() -> void {
                    static const uint32_t max_timeouts = 15;
                    static uint32_t timeout_counter = 0;

                    const auto& reset_window_grabber = [&]() -> void {
                        grab_window_timer->stop();
                        timeout_counter = 0;
                        ui->lin_activation_parameter->clear();
                        ui->btn_edit_activation_parameter->setEnabled(true);
                    };

                    if(++timeout_counter > max_timeouts) {
                        reset_window_grabber();
                    } else {
                        beepBoop({{200, 20}});

                        ui->lin_activation_parameter->setText(
                            "Switch to target window within timeframe: "
                            + QString::number(timeout_counter)
                            + " / "
                            + QString::number(max_timeouts)
                        );

                        HWND foreground_window = GetForegroundWindow();

                        // Other window has been selected, as foreground_window doesn't match this program's window.
                        if(foreground_window != HWND(winId())) {
                            beepBoop({{900, 20}, {900, 20}});
                            reset_window_grabber();

                            char window_title[256];
                            std::fill(window_title, window_title + sizeof(window_title), 0x00);
                            size_t bytes_written = GetWindowText(foreground_window, window_title, sizeof(window_title));
                            logToConsole({"GetWindowText() wrote ", QString::number(bytes_written), " bytes to char window_title[256]"});

                            targetForegroundWindowTitle = QString(window_title);
                            ui->lin_activation_parameter->setText(targetForegroundWindowTitle);
                        }
                    }
                });
    }

    if(selectedActivationMethod == ACTIVATION_METHOD::WINDOW_TITLE
            && ui->btn_edit_activation_parameter->text() == "Edit"
            && ui->btn_edit_activation_parameter->isEnabled()
            ) {
        ui->btn_edit_activation_parameter->setEnabled(false);
        grab_window_timer->start(500);
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

void MainWindow::showConsoleContextMenu(const QPoint& point) {
    static QMenu* context_menu = nullptr;
    const QPoint global_point = ui->txt_console->mapToGlobal(point);

    if(context_menu == nullptr) {
        context_menu = new QMenu(this);
        QAction* ac_clear_console = context_menu->addAction("Clear Console");

        QMenu* submenu_loglevels = context_menu->addMenu("Log Levels");
        submenu_loglevels->addAction(">= INFO")->setData(0);
        submenu_loglevels->addAction(">= WARNING")->setData(1);
        submenu_loglevels->addAction(">= ERROR")->setData(2);
        submenu_loglevels->addAction(">= EXCEPTION")->setData(3);

        connect(context_menu, &QMenu::triggered, [=](QAction* action) -> void {
            if(action == ac_clear_console) {
                logMessages.clear();
                displayLogMessagesInConsole(false);
            } else if(action->parentWidget() == submenu_loglevels) {
                minimumLogLevel = static_cast<CONSOLE_LOG_LEVELS>(action->data().toInt());
                displayLogMessagesInConsole(false);

                for(QAction* other_action : submenu_loglevels->actions()) {
                    other_action->setChecked(false);
                }

                action->setCheckable(true);
                action->setChecked(true);
            }
        });
    }

    context_menu->popup(global_point);
}

MainWindow::MainWindow(QWidget* parent)
    :
      QMainWindow(parent),
      ui(new Ui::MainWindow),
      minimumLogLevel(CLOG_INFO),
      targetHotkeyId(420)
{
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
    selectedActivationMethod = ACTIVATION_METHOD::NOTHING;
    targetHotkeyVkid = 0;

    // Event Connections
    // --------------------------------------------------
    // Class SLOT() connections.
    connect(ui->cbx_activation_method, SIGNAL(currentIndexChanged(int)), this, SLOT(changeActivationMethod(int)));
    connect(ui->btn_edit_activation_parameter, SIGNAL(clicked()), this, SLOT(editActivationMethodParameter()));
    connect(ui->btn_mutebeepboop, SIGNAL(clicked()), this, SLOT(toggleMuteBeepBoop()));

    connect(ui->txt_console, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showConsoleContextMenu(const QPoint&)));

    // Load JSON Defaults
    // --------------------------------------------------
    std::ifstream input_stream("./defaults.json", std::ios::binary);

    if(input_stream.good()) {
        std::string raw_json_string((std::istreambuf_iterator<char>(input_stream)), (std::istreambuf_iterator<char>()));
        input_stream.close();

        Json default_values;

        try {
            default_values = Json::parse(raw_json_string);
            logToConsole("(defaults.json) Default values from defaults.json have been parsed.", CLOG_INFO);
        } catch(const Json::exception& e) {
            logToConsole("(defaults.json) Could not parse values from defaults.json, the file might be incorrectly formatted.", CLOG_ERROR);
            logToConsole({"(defaults.json) Json::exception::what -- ", e.what()}, CLOG_EXCEPTION);
            QMessageBox::critical(this, "JSON Exception", "Error when parsing defaults.json, the file might be incorrectly formatted.");
        } catch(const std::exception& e) {
            logToConsole("(defaults.json) Non-JSON exception thrown when parsing values from defaults.json", CLOG_ERROR);
            logToConsole({"(defaults.json) std::exception::what -- ", e.what()}, CLOG_EXCEPTION);
            QMessageBox::critical(this, "Unknown Exception", "Error when parsing defaults.json, the file might be incorrectly formatted, but the exception isn't a JSON exception.");
        }

        if(default_values.size()) {
            if(default_values.contains("vkid")) {
                if(default_values["vkid"].is_string() && default_values["vkid"] != "") {
                    const std::string& vkid_hex_string = default_values["vkid"].get<std::string>();
                    uint8_t vkid = static_cast<uint8_t>(strtol(vkid_hex_string.c_str(), nullptr, 16));
                    targetHotkeyVkid = vkid;

                    logToConsole({"(defaults.json) Loaded activation hotkey VKID \"0x", QString::number(targetHotkeyVkid, 16), "\""}, CLOG_INFO);
                } else {
                    logToConsole("(defaults.json) Invalid value type for \"vkid\" key, should be a string.", CLOG_WARNING);
                    QMessageBox::warning(this, "Invalid JSON Value Type", "Value of the \"vkid\" key in defaults.json is not a string! Ignoring its value.");
                }
            }

            if(default_values.contains("image")) {
                if(default_values["image"].is_string() && default_values["image"] != "") {
                    targetProcessImageName = QString::fromStdString(default_values["image"].get<std::string>());
                    logToConsole({"(defaults.json) Loaded process image name \"", targetProcessImageName, "\""}, CLOG_INFO);
                } else if(!default_values["image"].is_string()) {
                    logToConsole("(defaults.json) Invalid value type for \"image\" key, should be a string.", CLOG_WARNING);
                    QMessageBox::warning(this, "Invalid JSON Value Type", "Value of the \"image\" key in defaults.json is not a string! Ignoring its value.");
                }
            }

            if(default_values.contains("title")) {
                if(default_values["title"].is_string() && default_values["title"] != "") {
                    targetForegroundWindowTitle = QString::fromStdString(default_values["title"].get<std::string>());
                    logToConsole({"(defaults.json) Loaded foreground window title \"", targetForegroundWindowTitle, "\""}, CLOG_INFO);
                } else if(!default_values["title"].is_string()) {
                    logToConsole("(defaults.json) Invalid value type for \"title\" key, should be a string.", CLOG_WARNING);
                    QMessageBox::warning(this, "Invalid JSON Value Type", "Value of the \"title\" key in defaults.json is not a string! Ignoring its value.");
                }
            }

            if(default_values.contains("method")) {
                if(default_values["method"].is_string() && default_values["method"] != "") {
                    const std::string& method_string = default_values["method"].get<std::string>();
                    static const Json method_resolver = {{"vkid", 1}, {"image", 2}, {"title", 3}};

                    if(method_resolver.contains(method_string)) {
                        ui->cbx_activation_method->setCurrentIndex(method_resolver[method_string].get<uint8_t>());
                        logToConsole({"(defaults.json) Loaded default activation method \"", QString::fromStdString(method_string), "\""}, CLOG_INFO);
                    } else {
                        logToConsole({"(defaults.json) Invalid activation method \"", QString::fromStdString(method_string), "\" for \"method\" key."}, CLOG_WARNING);
                        QMessageBox::warning(this, "Invalid Activation Method", "Invalid activation method configured in defaults.json, \"" + QString::fromStdString(method_string) + "\" - leave blank to disable, or pick from: vkid, title, window");
                    }
                } else if(!default_values["method"].is_string()) {
                    logToConsole("(defaults.json) Invalid value type for \"method\" key, should be a string.", CLOG_WARNING);
                    QMessageBox::warning(this, "Invalid JSON Value Type", "Value of the \"method\" key in defaults.json is not a string! Ignoring its value.");
                }
            }

            if(default_values.contains("muted")) {
                if(default_values["muted"].is_boolean()) {
                    const bool& muted_value = default_values["muted"].get<bool>();
                    muteBeepBoop = muted_value;
                    ui->btn_mutebeepboop->setText("Unmute");
                    logToConsole("(defaults.json) Loaded default mute state.", CLOG_INFO);
                } else {
                    logToConsole("(defaults.json) Invalid value type for \"muted\" key, should be a boolean.", CLOG_WARNING);
                    QMessageBox::warning(this, "Invalid JSON Value Type", "Value of the \"muted\" key in defaults.json is not a boolean value! Must be true or false, without quotes.");
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
                {"method", ""},
                {"muted", false}
            };

            const std::string& dumped_json = json_template.dump(4);

            output_stream.write(dumped_json.data(), dumped_json.size());
            output_stream.close();

            logToConsole("(defaults.json) Generated new defaults.json file using blank template.", CLOG_INFO);
            QMessageBox::information(this, "Generated defaults.json", "The defaults.json file has been generated, if you wish to store default program values, please fill it in.");
        } else {
            logToConsole("(defaults.json) Bad ofstream when attempting to generate a new defaults.json file! Most likely a permission error.", CLOG_ERROR);
            QMessageBox::critical(this, "Bad Stream", "Attempted to create a defaults.json file, but the stream was bad. Are you sure this program has permission to write to the current directory?");
        }
    }


    if(loadStylesheetFile("./style_sheet.qss")) {
        logToConsole("Loaded QSS stylesheet ./style_sheet.qss - theme has been applied.", CLOG_INFO);
    } else {
        logToConsole("Cannot open style_sheet.qss, using default Windows style.", CLOG_WARNING);
    }

    // Install Custom Event Filter For Right Clicks
    // --------------------------------------------------
    static struct RightClickEventFilter : public QObject {
        MainWindow* mainWindowParent;

        RightClickEventFilter(MainWindow* main_window_parent) : mainWindowParent(main_window_parent) {}

        bool eventFilter(QObject* watched, QEvent* event) override {
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
