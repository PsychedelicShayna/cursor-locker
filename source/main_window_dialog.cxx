#include "main_window_dialog.hxx"
#include "ui_mainwindow_dialog.h"

enum struct ACTIVATION_METHOD {
    NOTHING         =   0b00000000,
    HOTKEY          =   0b00000001,
    PROCESS_IMAGE   =   0b00000010,
    WINDOW_TITLE    =   0b00000100,
};

enum struct HOTKEY_MOD {
    NONE    = NULL,
    ALT     = MOD_ALT,
    CONTROL = MOD_CONTROL,
    SHIFT   = MOD_SHIFT,
    WIN     = MOD_WIN
};

bool MainWindow::loadJsonConfig(const QString& json_file_path) {
    QDebugConsoleContext dbg_console_context { dbgConsole, "loadJsonConfig" };

    QFile json_file { json_file_path };

    if(!json_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        dbgConsole->log({"Encountered an IO error when trying to read from a JSON config file @ \"",
                         json_file_path, "\", are you sure you have permission to read from this location?"
                        }, QDebugConsole::LL_ERROR);
        return false;
    }

    QByteArray json_bytes { json_file.readAll() };
    json_file.close();

    QJsonParseError json_parse_error;

    QJsonDocument json_document {
        QJsonDocument::fromJson(json_bytes, &json_parse_error)
    };

    if(json_parse_error.error != QJsonParseError::NoError) {
        dbgConsole->log({"Encountered JSON parsing error: ",
                         json_parse_error.errorString()
                        }, QDebugConsole::LL_ERROR);

        return false;
    }

    QJsonObject       json_object    { json_document.object() };
    QList<QString>    json_keys      { json_object.keys()     };

    bool handler_error { false };

    for(const auto& handler_pair : jsonConfigValueHandlers) {
        const auto& matching_key {
            std::find_if(json_keys.begin(), json_keys.end(), [&](const QString& key) -> bool {
                return handler_pair.first == key;
            })
        };

        if(matching_key != json_keys.end()) {
            bool error_value { handler_pair.second(*matching_key, json_object[*matching_key]) };
            handler_error |= error_value;

            if(!error_value) {
                dbgConsole->log({"JSON Value handler returned false for key! \"", *matching_key, "\""}, QDebugConsole::LL_WARNING);
            }
        }
    }

    return handler_error;
}

bool MainWindow::loadJsonConfig() {
    return loadJsonConfig(jsonConfigFilePath);
}

bool MainWindow::dumpJsonConfigTemplate(const QString& json_file_path) {
    QDebugConsoleContext dbg_console_context { dbgConsole, "dumpJsonConfigTemplate" };

    QJsonDocument json_template_document { QJsonObject {
            { "vkid",       "" },
            { "image",      "" },
            { "title",      "" },
            { "method",     "" },
            { "muted",   false }
    }};

    QByteArray json_bytes { json_template_document.toJson(QJsonDocument::JsonFormat::Indented) };

    QFile json_file { json_file_path };

    if(!json_file.open(QIODevice::WriteOnly)) {
        dbgConsole->log({"Encountered an IO error when trying to create a new JSON config file @ \"",
                         json_file_path, "\", are you sure you have permission to write to this location?"
                        }, QDebugConsole::LL_ERROR);

        return false;
    }

    qint64 bytes_written { json_file.write(json_bytes) };
    json_file.close();

    if(bytes_written != json_bytes.size()) {
        dbgConsole->log({"Wrote ", QString::number(bytes_written),
                         "bytes when trying to create a new JSON config file @ \"",
                         json_file_path, "\", when the data was ", QString::number(json_bytes.size()),
                         " bytes long! This should not be happening."
                        }, QDebugConsole::LL_ERROR);
        return false;
    }

    return true;
}

bool MainWindow::dumpJsonConfigTemplate() {
    return dumpJsonConfigTemplate(jsonConfigFilePath);
}

void MainWindow::insertActivationParameterWidget(QWidget* widget, bool enable, bool unhide) {
    if(widget != nullptr) {
        if(widget->layout() != ui->hlActivationParameter) {
            ui->hlActivationParameter->insertWidget(0, widget, 0);
        }

        widget->setEnabled(enable);
        widget->setHidden(!unhide);
    }
}

void MainWindow::removeActivationParameterWidget(QWidget* widget, bool disable, bool hide) {
    if(widget != nullptr) {
        if(widget->layout() == ui->hlActivationParameter) {
            ui->hlActivationParameter->removeWidget(widget);
        }

        widget->setEnabled(!disable);
        widget->setHidden(hide);
    }
}

bool MainWindow::registerAMHotkey() {
    BOOL result { RegisterHotKey(HWND(winId()), amParamHotkeyId, MOD_NOREPEAT | amParamHotkeyModifiers, amParamHotkeyVkid) };

    dbgConsole->log({"RegisterHotKey(HWND(winId), 0x",
                     QString::number(amParamHotkeyId, 16), \
                     ", MOD_NOREPEAT | ",
                     QString::number(amParamHotkeyModifiers),
                     ", 0x",
                     QString::number(amParamHotkeyVkid, 16),
                     ") returned ",
                     result ? "true" : "false"
                 });

    return result;
}

bool MainWindow::unregisterAMHotkey() {
    BOOL result { UnregisterHotKey(HWND(winId()), amParamHotkeyId) };

    if(result) {
        dbgConsole->log({"UnregisterHotKey(HWND(winId), 0x", QString::number(amParamHotkeyId, 16), ") returned true"});
    }

    return result;
}

QString MainWindow::setAMParamHotkeyVkid(const uint8_t& vkid) {
    amParamHotkeyVkid = vkid;

    if(vkid && selectedActivationMethod == ACTIVATION_METHOD::HOTKEY) {
        unregisterAMHotkey();
        registerAMHotkey();
    }

    const QString& vkid_hexstr  {
        QString { "0x%1" }.arg(vkid, 2, 16, QLatin1Char { '0' })
    };

    ui->linActivationParameter->setText(vkid_hexstr);

    return vkid_hexstr;
}

QString MainWindow::setAMParamHotkeyVkid(const QString& vkid_hexstr) {
    QDebugConsoleContext dbg_console_context { dbgConsole, "setAMParamHotkeyVkid(QString)" };

    char hex_conversion_buffer[5];

    ZeroMemory(hex_conversion_buffer,
               sizeof(hex_conversion_buffer));

    bool conversion_success { false };
    int32_t vkid = vkid_hexstr.toInt(&conversion_success, 16);

    if(conversion_success) {
        return setAMParamHotkeyVkid(static_cast<uint8_t>(vkid));
    } else {
        dbgConsole->log({"Failed to convert hexadecimal VKID string into an integer \"",
                        vkid_hexstr, "\""
                        }, QDebugConsole::LL_ERROR);

        return QString {};    // Return empty QString on conversion failure.
    }
}

void MainWindow::setAMToHotkey() {
    QDebugConsoleContext dbg_console_context { dbgConsole, "setActivationMethodToHotkey" };

    dbgConsole->log("Set activation method to: hotkey.");
    dbgConsole->log("VKID List for hotkeys: http://www.kbdedit.com/manual/low_level_vk_list.html");

    selectedActivationMethod = ACTIVATION_METHOD::HOTKEY;

    unregisterAMHotkey();

    connect(this, &MainWindow::targetHotkeyWasPressed,
            this, &MainWindow::activateBecauseTargetHotkeyWasPressed);

    ui->linActivationParameter->setPlaceholderText("VKID, e.g. 0x6A (Numpad *)");

    if(amParamHotkeyVkid) {
        const QString& vkid_hexstr  {
            QString { "0x%1" }.arg(amParamHotkeyVkid, 4, 16, QLatin1Char { '0' })
        };

        ui->linActivationParameter->setText(vkid_hexstr);

        registerAMHotkey();
    }

    insertActivationParameterWidget(hotkeyInput, false);
    insertActivationParameterWidget(cblHotkeyModifiers, false);
}

void MainWindow::unsetAMToHotkey() {
    unregisterAMHotkey();

    disconnect(this, &MainWindow::targetHotkeyWasPressed,
               this, &MainWindow::activateBecauseTargetHotkeyWasPressed);

    removeActivationParameterWidget(cblHotkeyModifiers);
    removeActivationParameterWidget(hotkeyInput);
}

void MainWindow::setAMParamProcessImageName(const QString& process_image_name) {
    amParamProcessImageName = process_image_name;
    ui->linActivationParameter->setText(amParamProcessImageName);
}

void MainWindow::setAMToProcessImageName() {
    QDebugConsoleContext dbg_console_context { dbgConsole, "setAMToProcessImageName" };

    selectedActivationMethod = ACTIVATION_METHOD::PROCESS_IMAGE;
    ui->linActivationParameter->setPlaceholderText("Image name, e.g. TESV.exe, SkyrimSE.exe, etc");

    if(amParamProcessImageName.size()) {
        ui->linActivationParameter->setText(amParamProcessImageName);
        dbgConsole->log({"Loaded amParamProcessImageName \"", amParamProcessImageName, "\""});
    }

    timedActivationMethodConnection = connect(timedActivationMethodTimer,   SIGNAL(timeout()),
                                              this,                         SLOT(activateIfTargetProcessRunning()));

    insertActivationParameterWidget(btnSpawnProcessScanner);
    btnSpawnProcessScannerConnection = connect(btnSpawnProcessScanner, &QPushButton::clicked,
                                               std::bind(&MainWindow::spawnProcessScannerDialog, this, ProcessScanner::PROCESS_MODE));

    dbgConsole->log("Activation method has been set to process image name.");
}

void MainWindow::unsetAMToProcessImageName() {
    QDebugConsoleContext dbg_console_context { dbgConsole, "unsetAMToProcessImageName" };

    disconnect(timedActivationMethodConnection);
    removeActivationParameterWidget(btnSpawnProcessScanner);
    disconnect(btnSpawnProcessScannerConnection);
}

void MainWindow::setAMParamForegroundWindowTitle(const QString& foreground_window_title) {
    amParamForegroundWindowTitle = foreground_window_title;
    ui->linActivationParameter->setText(amParamForegroundWindowTitle);
}

void MainWindow::setAMToForegroundWindowTitle() {
    QDebugConsoleContext dbg_console_context { dbgConsole, "setAMToForegroundWindowTitle" };

    dbgConsole->log("Activation mode set to window title. Right click the edit button to grab the name of the next foreground window that you select.");

    selectedActivationMethod = ACTIVATION_METHOD::WINDOW_TITLE;
    ui->linActivationParameter->setPlaceholderText("Window title, e.g. Skyrim, Skyrim Special Edition");

    if(amParamForegroundWindowTitle.size()) {
        ui->linActivationParameter->setText(amParamForegroundWindowTitle);
        dbgConsole->log({"Set activation parameter text field to: ", amParamForegroundWindowTitle});
    }

    timedActivationMethodConnection = connect(timedActivationMethodTimer,   SIGNAL(timeout()),
                                              this,                         SLOT(activateIfForegroundWindowMatchesTarget()));

    insertActivationParameterWidget(btnSpawnProcessScanner);
    btnSpawnProcessScannerConnection = connect(btnSpawnProcessScanner, &QPushButton::clicked,
                                               std::bind(&MainWindow::spawnProcessScannerDialog, this, ProcessScanner::WINDOW_MODE));
}

void MainWindow::unsetAMToForegroundWindowTitle() {
    disconnect(timedActivationMethodConnection);
    removeActivationParameterWidget(btnSpawnProcessScanner);
    disconnect(btnSpawnProcessScannerConnection);
}

bool MainWindow::loadStylesheetFile(const std::string& file_path) {
    std::ifstream input_stream { file_path, std::ios::binary };

    if(input_stream.good()) {
        std::string style_sheet { std::istreambuf_iterator<char>(input_stream), std::istreambuf_iterator<char>() };
        input_stream.close();

        dbgConsole->log({"Read ", QString::number(style_sheet.size()), " bytes from stylesheet @ ", QString::fromStdString(file_path)});

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

RECT MainWindow::getForegroundWindowRect() {
    HWND foreground_window_handle { GetForegroundWindow() };
    RECT foreground_window_rect;

    GetWindowRect(foreground_window_handle, &foreground_window_rect);

    return foreground_window_rect;
}

void MainWindow::enableCursorLock() {
    RECT foreground_window_rect { getForegroundWindowRect() };
    ClipCursor(&foreground_window_rect);
}

void MainWindow::disableCursorLock() {
    ClipCursor(nullptr);
}

bool MainWindow::toggleCursorLock() {
    static bool toggle { false };

    if(toggle ^= true) {
        enableCursorLock();
    } else {
        disableCursorLock();
    }

    return toggle;
}

bool MainWindow::nativeEvent(const QByteArray& event_type, void* message, qintptr* result) {
    Q_UNUSED(event_type);
    Q_UNUSED(result);

    MSG* msg { reinterpret_cast<MSG*>(message) };

    /* msg->lParam Is a 64-bit integer and stores the VKID of the pressed hotkey in byte 3/8 (little-endian)
     * and so it cannot be directly compared with amParamHotkeyVkid, as it stores the VKID in byte 1/1 so the
     * comparison will always fail. Instead, lParam should be bitshifted 16 bits to the right so that the VKID
     * is located in byte 1/8, and then bitwise AND with 0xFF should be performed to exclude any other bits that
     * may be present in the 64-bit integer, and leave only the bits in position 1/8 which should be the VKID.
     * After this is done, it may be compared as normal without issue.
     *
     *                                                  lParam >> 16
     * ????     ????     ????     ????     ????     |--VKID-----------v
     * 00000000 00000000 00000000 00000000 00000000 01101010 00000000 00000000
     * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

    if (msg->message == WM_HOTKEY) {
        if(msg->wParam == amParamHotkeyId && ((msg->lParam >> 16) & 0xFF) == amParamHotkeyVkid) {
            emit targetHotkeyWasPressed();
            return true;
        }

        dbgConsole->log({"nativeEvent got MSG WM_HOTKEY, wParam=0x",
                      QString::number(msg->wParam, 16),
                      ", lParam=0x",
                      QString::number(msg->lParam, 16)
                     });
    }

    return QMainWindow::nativeEvent(event_type, message, result);
}

void MainWindow::activateBecauseTargetHotkeyWasPressed() {
    if(toggleCursorLock()) {
        dbgConsole->log("Activated cursor lock via hotkey.");
        beepBoop({{500,20}, {700, 20}});
    } else {
        dbgConsole->log("Deactivated cursor lock via hotkey.");
        beepBoop({{700,20}, {500, 20}});
    }
}

void MainWindow::activateIfTargetProcessRunning() {
    if(!amParamProcessImageName.size()) return;

    HANDLE process_snapshot { CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,  0) };

    if(process_snapshot == INVALID_HANDLE_VALUE) {
        dbgConsole->log("Invalid handle value for snapshot of type TH32CS_SNAPPROCESS. Cannot see running tasks!", QDebugConsole::QDebugConsole::LL_ERROR);
        return;
    }

    PROCESSENTRY32 process_entry_32;
    process_entry_32.dwSize = sizeof(PROCESSENTRY32);

    if(Process32First(process_snapshot, &process_entry_32)) {
        static bool first_find { true };
        bool process_found { false };

        do {
            if(amParamProcessImageName == process_entry_32.szExeFile) {
                process_found = true;
                break;
            }
        } while(Process32Next(process_snapshot, &process_entry_32));

        if(process_found) {
            enableCursorLock();

            if(first_find) {
                dbgConsole->log({"Enabling lock because target process was found: ", amParamProcessImageName});
                first_find = false;

                beepBoop({{500,20}, {700,20}});
            }
        } else {
            disableCursorLock();

            if(!first_find) {
                dbgConsole->log({"Disabling lock because target process was lost: ", amParamProcessImageName});
                first_find = true;

                beepBoop({{700,20}, {500,20}});
            }
        }
    }

    CloseHandle(process_snapshot);
}

void MainWindow::activateIfForegroundWindowMatchesTarget() {
    if(!amParamForegroundWindowTitle.size()) return;

    static bool first_find { true };

    char window_title_buffer[256];
    std::fill(window_title_buffer, window_title_buffer + sizeof(window_title_buffer), 0x00);

    HWND foreground_window { GetForegroundWindow() };
    GetWindowTextA(foreground_window, window_title_buffer, sizeof(window_title_buffer));

    if(amParamForegroundWindowTitle == window_title_buffer) {
        enableCursorLock();

        if(first_find) {
            dbgConsole->log({"Enabling lock because target window was found: ", amParamForegroundWindowTitle});
            first_find = false;

            beepBoop({{500,20}, {700,20}});
        }
    } else {
        disableCursorLock();

        if(!first_find) {
            dbgConsole->log({"Disabling lock because target window was lost: ", amParamForegroundWindowTitle});
            first_find = true;

            beepBoop({{700,20}, {500,20}});
        }
    }
}

void MainWindow::changeActivationMethod(int method_index) {
    /* Clear linActivationParameter's text to allow the placeholder text of
     * the selected activation method to be displayed later on.
     * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
    ui->linActivationParameter->clear();

    disableCursorLock(); // Ensure the cursor lock is disabled before switching over to a new activation method.

    unsetAMToHotkey();
    unsetAMToProcessImageName();
    unsetAMToForegroundWindowTitle();

    switch(method_index) {
    case 0 :
        dbgConsole->log("Activation method set to nothing.");

        selectedActivationMethod = ACTIVATION_METHOD::NOTHING;
        ui->linActivationParameter->setPlaceholderText("No activation method selected");
        break;

    case 1 :
        setAMToHotkey();
        break;

    case 2 :
        setAMToProcessImageName();
        break;

    case 3 :
        setAMToForegroundWindowTitle();
        break;
    }

    timedActivationMethodTimer->start(500);
}

bool MainWindow::changeActivationMethod(const QString& method) {
    static const QMap<QString, int32_t>& resolver {
        { "",        0 },
        { "hotkey",  1 },
        { "image",   2 },
        { "title",   3 }
    };

    if(!resolver.contains(method)) return false;
    ui->cbxActivationMethod->setCurrentIndex(resolver[method]);
    return true;
}

void MainWindow::handleModifierCheckeStateChange(QStandardItem* item_changed) {
    const WINAPI_MODIFIER& matching_modifier {
        QStringToWinApiKbModifier(item_changed->text())
    };

    if(item_changed->checkState() == Qt::Checked) {
        amParamHotkeyModifiers |= matching_modifier;
    } else {
        amParamHotkeyModifiers &= ~matching_modifier;
    }
}

void MainWindow::handleHotkeyRecorded(QHotkeyInput::WindowsHotkey windows_hotkey) {
    setAMParamHotkeyVkid(windows_hotkey.Vkid);
    amParamHotkeyModifiers = windows_hotkey.Modifiers;
    dbgConsole->log({"Recorded new activation hotke: ", windows_hotkey.ToString()});
}

/*
void MainWindow::changeHotkeyModifier(int modifier_index) {

    switch(modifier_index) {
    case(0) :
        amParamHotkeyModifier = HOTKEY_MOD::NONE;
        break;

    case(1) :
        amParamHotkeyModifier = HOTKEY_MOD::ALT;
        break;

    case(2) :
        amParamHotkeyModifier = HOTKEY_MOD::CONTROL;
        break;

    case(3) :
        amParamHotkeyModifier = HOTKEY_MOD::SHIFT;
        break;

    case(4) :
        amParamHotkeyModifier = HOTKEY_MOD::WIN;
        break;
    }

}
*/

void MainWindow::editActivationMethodParameter() {
    if(ui->btnEditActivationParameter->text() == "Edit" && selectedActivationMethod != ACTIVATION_METHOD::NOTHING) {
        ui->btnEditActivationParameter->setText("Confirm");
        ui->linActivationParameter->setEnabled(true);
        ui->cbxActivationMethod->setEnabled(false);

        if(selectedActivationMethod == ACTIVATION_METHOD::HOTKEY) {
            cblHotkeyModifiers->setEnabled(true);
            hotkeyInput->setEnabled(true);
            hotkeyInput->StartRecording();
        }
    } else if(ui->btnEditActivationParameter->text() == "Confirm") {
        ui->btnEditActivationParameter->setText("Edit");
        ui->linActivationParameter->setEnabled(false);
        ui->cbxActivationMethod->setEnabled(true);

        cblHotkeyModifiers->setEnabled(false);
        hotkeyInput->setEnabled(false);
        hotkeyInput->StopRecording();

        switch(selectedActivationMethod) {
        case ACTIVATION_METHOD::HOTKEY : {
            setAMParamHotkeyVkid(ui->linActivationParameter->text());
            break;
        }

        case ACTIVATION_METHOD::PROCESS_IMAGE :
            setAMParamProcessImageName(ui->linActivationParameter->text());
            break;

        case ACTIVATION_METHOD::WINDOW_TITLE :
            setAMParamForegroundWindowTitle(ui->linActivationParameter->text());
            break;

        case ACTIVATION_METHOD::NOTHING :
            break;
        }
    }
}

void MainWindow::startForegroundWindowGrabber() {
    static QTimer* grab_window_timer { nullptr };

    if(grab_window_timer == nullptr) {
        grab_window_timer = new QTimer { this };

        connect(grab_window_timer, &QTimer::timeout,
                [this]() -> void {
                    static const uint32_t max_timeouts { 15 };
                    static uint32_t timeout_counter { 0 };

                    const auto& reset_window_grabber {
                        [&]() -> void {
                            grab_window_timer->stop();
                            timeout_counter = 0;
                            ui->linActivationParameter->clear();
                            ui->btnEditActivationParameter->setEnabled(true);
                            ui->cbxActivationMethod->setEnabled(true);
                        }};

                    if(++timeout_counter > max_timeouts) {
                        reset_window_grabber();
                    } else {
                        beepBoop({{200, 20}});

                        ui->linActivationParameter->setText(
                            "Switch to target window within timeframe: "
                            + QString::number(timeout_counter)
                            + " / "
                            + QString::number(max_timeouts)
                        );

                        HWND foreground_window { GetForegroundWindow() };

                        // Other window has been selected, as foreground_window doesn't match this program's window.
                        if(foreground_window != HWND(winId())) {
                            beepBoop({{900, 20}, {900, 20}});
                            reset_window_grabber();

                            char window_title[256];
                            std::fill(window_title, window_title + sizeof(window_title), 0x00);
                            int32_t bytes_written { GetWindowText(foreground_window, window_title, sizeof(window_title)) };
                            dbgConsole->log({"GetWindowText() wrote ", QString::number(bytes_written), " bytes to char window_title[256]"});

                            amParamForegroundWindowTitle = QString { window_title };
                            ui->linActivationParameter->setText(amParamForegroundWindowTitle);
                        }
                    }
                });
    }

    if(selectedActivationMethod == ACTIVATION_METHOD::WINDOW_TITLE
            && ui->btnEditActivationParameter->text() == "Edit"
            && ui->btnEditActivationParameter->isEnabled()
            ) {
        ui->btnEditActivationParameter->setEnabled(false);
        ui->cbxActivationMethod->setEnabled(false);
        grab_window_timer->start(500);
    }
}

void MainWindow::spawnProcessScannerDialog(ProcessScanner::SCAN_SCOPE process_scanner_scope) {
    dbgConsole->log("Called ProcessScannerDialog::show");

    if(processScannerDialog == nullptr) {
        processScannerDialog = new ProcessScannerDialog(process_scanner_scope, this);
        processScannerDialog->setAttribute(Qt::WA_DeleteOnClose);
        ui->btnEditActivationParameter->setEnabled(false);
        ui->cbxActivationMethod->setEnabled(false);

        connect(processScannerDialog, &ProcessScannerDialog::destroyed, [&](QObject*) -> void {
            ui->btnEditActivationParameter->setEnabled(true);
            ui->cbxActivationMethod->setEnabled(true);
            processScannerDialog = nullptr;
        });

        connect(processScannerDialog, &ProcessScannerDialog::treeSelectionMade, [&](const QString& selection) -> void {
            ui->linActivationParameter->setText(selection);

            if(selectedActivationMethod == ACTIVATION_METHOD::WINDOW_TITLE) {
                setAMParamForegroundWindowTitle(selection);
            } else if(selectedActivationMethod == ACTIVATION_METHOD::PROCESS_IMAGE) {
                setAMParamProcessImageName(selection);
            }

            delete processScannerDialog;
            processScannerDialog = nullptr;
        });
    }

    processScannerDialog->show();
}

void MainWindow::setMuteBeepBoopState(bool state) {
    muteBeepBoop = state;
    ui->btnMuteBeepBoop->setText(state ? "Unmute" : "Mute");
}

void MainWindow::toggleMuteBeepBoop() {
    setMuteBeepBoopState(muteBeepBoop ^ true);
}

MainWindow::MainWindow(QWidget* parent)
    :
      // Debug Console & Related Widgets Initialization

      QMainWindow                   { parent                     },
      ui                            { new Ui::MainWindow         },

      dbgConsole                    { new QDebugConsole { this } },
      vblDebugConsoleLayout         { new QVBoxLayout            },
      dbgConsoleContextMenu         { new QMenu { dbgConsole }   },

      jsonConfigFilePath            { "./defaults.json"          },

      selectedActivationMethod      { ACTIVATION_METHOD::NOTHING },

      // Hotkey activation method member variables initialization
      cblHotkeyModifiers            { new QCheckBoxList { this } },
      hotkeyInput                   { new QHotkeyInput  { this } },
      amParamHotkeyModifiers        { WINMOD_NULLMOD             },
      amParamHotkeyId               { 0x1A4                      },
      amParamHotkeyVkid             { 0x000                      },

      // Process scanner member variables initialization.
      processScannerDialog          { nullptr                    },     // ProcessScannerDialog instance, must be nullptr as spawnProcessScannerDialog takes care of construction and destruction.
      btnSpawnProcessScanner        { new QPushButton   { this } },     // QPushButton connected to spawnProcessScannerDialog further down in the constructor.
      amParamProcessImageName       { QString { "" }             },
      amParamForegroundWindowTitle  { QString { "" }             },

      timedActivationMethodTimer    { new QTimer        { this } },

      muteBeepBoop                  { false                      }

      // Not needed
      // btnGrabProcessImage      { new QPushButton   { this }  },
      // btnGrabForegroundWindow  { new QPushButton   { this }  }

{
    ui->setupUi(this);


    // Configure Main Window Flags & Set Initial Window Size
    // --------------------------------------------------
    setWindowFlags(
                Qt::Dialog
                | Qt::CustomizeWindowHint
                | Qt::WindowTitleHint
                | Qt::WindowCloseButtonHint
                | Qt::WindowMinimizeButtonHint
                | Qt::WindowMaximizeButtonHint
                );

    HWND desktop_window_handle { GetDesktopWindow() };
    RECT desktop_window_rect;

    GetWindowRect(desktop_window_handle, &desktop_window_rect);
    CloseHandle(desktop_window_handle);

    resize(20 * desktop_window_rect.right / 100, 16 * desktop_window_rect.bottom / 100);

    // Debug console layout configuration.
    vblDebugConsoleLayout->addWidget(dbgConsole);
    ui->grpDebugConsole->setLayout(vblDebugConsoleLayout);

    cblHotkeyModifiers->setEnabled(false);
    cblHotkeyModifiers->setHidden(true);
    cblHotkeyModifiers->setMinimumWidth(90);
    cblHotkeyModifiers->AddCheckableItems({"ALT", "CONTROL", "SHIFT", "WIN"}, Qt::Checked);

    hotkeyInput->setHidden(true);

    // btnSpawnProcessScanner initial values.
    btnSpawnProcessScanner->setText("Select");
    btnSpawnProcessScanner->setEnabled(false);
    btnSpawnProcessScanner->setHidden(true);

    // Event Connections
    // ====================================================================================================
    // Class SLOT() connections.

    // Static QML UI File Widgets --------------------------------------------------
    connect(ui->cbxActivationMethod,                SIGNAL(currentIndexChanged(int)),
            this,                                   SLOT(changeActivationMethod(int)));

    connect(ui->btnEditActivationParameter,         SIGNAL(clicked()),
            this,                                   SLOT(editActivationMethodParameter()));

    connect(ui->btnMuteBeepBoop,                    SIGNAL(clicked()),
            this,                                   SLOT(toggleMuteBeepBoop()));

    // Dynamic Class Widgets --------------------------------------------------
    // connect(cbxHotkeyModifier,                      SIGNAL(currentIndexChanged(int)),
    //         this,                                   SLOT(changeHotkeyModifier(int)));

    connect(cblHotkeyModifiers->StandardItemModel,      SIGNAL(itemChanged(QStandardItem*)),
            this,                                       SLOT(handleModifierSelectionChanged(QStandardItem*)));

    connect(hotkeyInput,                                SIGNAL(WindowsHotkeyRecorded(QHotkeyInput::WindowsHotkey)),
            this,                                       SLOT(handleHotkeyRecorded(QHotkeyInput::WindowsHotkey)));

    jsonConfigValueHandlers = {
        {"title", [&](const QString&, QJsonValueRef value) -> bool {
             if(!value.isString()) return false;
             const auto& value_str { value.toString() };
             if(value_str.size()) setAMParamForegroundWindowTitle(value.toString());
             return true;
         }},

        {"image", [&](const QString&, QJsonValueRef value) -> bool {
             if(!value.isString()) return false;
             const auto& value_str { value.toString() };
             if(value_str.size()) setAMParamProcessImageName(value_str);
             return true;
         }},

        {"vkid", [&](const QString&, QJsonValueRef value) -> bool {
             if(!value.isString()) return false;
             const auto& value_str { value.toString() };
             if(value_str.size()) return setAMParamHotkeyVkid(value_str).size();
             return true;
         }},

        {"method", [&](const QString&, QJsonValueRef value) -> bool {
             if(!value.isString()) return false;
             const auto& value_str { value.toString() };
             if(value_str.size()) changeActivationMethod(value_str);
             return true;
         }},

        {"muted", [&](const QString&, QJsonValueRef value) -> bool {
             if(!value.isBool()) return false;
             setMuteBeepBoopState(value.toBool());
             return true;
         }}
    };

    QFileInfo json_file_info { jsonConfigFilePath };

    if(json_file_info.exists() && json_file_info.isFile()) {
        loadJsonConfig();
    } else {
        if(dumpJsonConfigTemplate()) {
            QMessageBox::information(this, "New JSON File Generated!", "No JSON config file was found, so a new one has been generated at this location: \"" + json_file_info.absoluteFilePath() + "\" - please fill it in if you wish to have default settings.");
        } else {
            QMessageBox::critical(this, "Failed To Generate JSON File!", "No JSON file could be found, so an attempt was made to generate a new one, but it failed! The program can still continue, but no default values can be loaded. Are you sure you have permission to write to this location? - \"" + json_file_info.absoluteFilePath() + "\"");
        }
    }

    if(loadStylesheetFile("./style_sheet.qss")) {
        dbgConsole->log("Loaded QSS stylesheet ./style_sheet.qss - theme has been applied.");
    } else {
        dbgConsole->log("Cannot open style_sheet.qss, using default Windows style.", QDebugConsole::LL_WARNING);
    }

    ui->btnEditActivationParameter->installEventFilter(
                new LambdaEventFilter<>(this, [](QObject* parent, QObject* watched, QEvent* event) -> bool {
                    MainWindow* main_window { reinterpret_cast<MainWindow*>(parent) };

                    if(event->type() == QEvent::MouseButtonPress)   {
                        QMouseEvent* mouse_event { reinterpret_cast<QMouseEvent*>(event) };

                        if(mouse_event->button() == Qt::RightButton) {
                            if(main_window->selectedActivationMethod == ACTIVATION_METHOD::WINDOW_TITLE) {
                                main_window->startForegroundWindowGrabber();
                            }
                        }
                    }

                    return false;
                }));
}

MainWindow::~MainWindow() {
    disableCursorLock();
    delete ui;
}
