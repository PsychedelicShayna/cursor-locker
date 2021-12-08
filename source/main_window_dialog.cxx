#include "main_window_dialog.hxx"
#include "ui_main_window_dialog.h"

enum struct ACTIVATION_METHOD {
    NOTHING         =   0b00000000,
    HOTKEY          =   0b00000001,
    PROCESS_IMAGE   =   0b00000010,
    WINDOW_TITLE    =   0b00000100,
};

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



// Keyboard Modifier Dropdown
// -------------------------------------------------------------------------------------------------------------------------------------------------------------
void MainWindow::handleModifierListBitmaskChanged(const quint32& bitmask) {
    ampHotkeyModifiersBitmask = bitmask;
}




// Hotkey Input / Recorder
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void MainWindow::updateUiWithRecordedWindowsHotkey(QHotkeyInput::WindowsHotkey windows_hotkey) {
    QDebugConsoleContext dbg_console_context { dbgConsole, "updateUiWithRecordedWindowsHotkey" };

    ui->linActivationParameter->setText(QString::number(windows_hotkey.Vkid, 16));
    ampwHotkeyModifierDropdown->SetModifierCheckStateFromBitmask(windows_hotkey.Modifiers);

    dbgConsole->log({"Ui has been set to use recorded hotkey: ", windows_hotkey.ToString()});
}

void MainWindow::updateHotkeyInputWithNewModifierBitmask(const quint32& new_modifier_bitmask) {
    ampwHotkeyRecorder->clear();
}





// VKID Table Dialog
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void MainWindow::spawnVkidTableDialog() {
    if(vkidTableDialog == nullptr) {
        vkidTableDialog = new VkidTableDialog { this };

        connect(vkidTableDialog, &VkidTableDialog::destroyed, [&]() -> void {
            vkidTableDialog = nullptr;
        });

        vkidTableDialog->show();
    }
}




// Hotkey Activation Method
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
QString MainWindow::setAmpHotkeyVkid(const quint32& vkid) {
    ampHotkeyVkid = vkid;

    unregisterAmpHotkey();

    if(selectedActivationMethod == ACTIVATION_METHOD::HOTKEY) {
        if(vkid) {
            registerAmpHotkey();

            const QString& vkid_hexstr  {
                QString { "0x%1" }.arg(vkid, 2, 16, QLatin1Char { '0' })
            };

            ui->linActivationParameter->setText(vkid_hexstr);

            return vkid_hexstr;
        } else {
            ui->linActivationParameter->clear();
        }
    }

    return QString {};
}

QString MainWindow::setAmpHotkeyVkid(const QString& vkid_hexstr) {
    QDebugConsoleContext dbg_console_context { dbgConsole, "setAmpHotkeyVkid(QString)" };

    if(!vkid_hexstr.size()) {
        return setAmpHotkeyVkid(0x00);
    }

    char hex_conversion_buffer[5];

    ZeroMemory(hex_conversion_buffer,
               sizeof(hex_conversion_buffer));

    bool conversion_success { false };
    qint32 vkid = vkid_hexstr.toInt(&conversion_success, 16);

    if(conversion_success) {
        return setAmpHotkeyVkid(static_cast<uint8_t>(vkid));
    } else {
        dbgConsole->log({"Failed to convert hexadecimal VKID string into an integer \"",
                        vkid_hexstr, "\""
                        }, QDebugConsole::LL_ERROR);

        return QString {};    // Return empty QString on conversion failure.
    }
}

bool MainWindow::registerAmpHotkey() {
    QDebugConsoleContext { dbgConsole, "registerAmpHotkey" };

    BOOL result { RegisterHotKey(HWND(winId()), ampHotkeyId, MOD_NOREPEAT | ampHotkeyModifiersBitmask, ampHotkeyVkid) };

    dbgConsole->log({"RegisterHotKey(HWND(winId), 0x",
                     QString::number(ampHotkeyId, 16), \
                     ", MOD_NOREPEAT | ",
                     QString::number(ampHotkeyModifiersBitmask),
                     ", 0x",
                     QString::number(ampHotkeyVkid, 16),
                     ") returned ",
                     result ? "true" : "false"
                 });

    return result;
}

bool MainWindow::unregisterAmpHotkey() {
    QDebugConsoleContext { dbgConsole, "unregisterAmpHotkey" };

    BOOL result { UnregisterHotKey(HWND(winId()), ampHotkeyId) };

    if(result) {
        dbgConsole->log({"UnregisterHotKey(HWND(winId), 0x", QString::number(ampHotkeyId, 16), ") returned true"});
    }

    return result;
}

void MainWindow::setAmToHotkey() {
    QDebugConsoleContext dbg_console_context { dbgConsole, "setActivationMethodToHotkey" };

    dbgConsole->log("Set activation method to: hotkey.");
    dbgConsole->log("VKID List for hotkeys: http://www.kbdedit.com/manual/low_level_vk_list.html");

    selectedActivationMethod = ACTIVATION_METHOD::HOTKEY;

    unregisterAmpHotkey();

    connect(this, &MainWindow::targetHotkeyWasPressed,
            this, &MainWindow::activateBecauseTargetHotkeyWasPressed);

    ui->linActivationParameter->setPlaceholderText("VKID, e.g. 0x6A (Numpad *)");

    ampHotkeyModifiersBitmask = ampwHotkeyModifierDropdown->GetModifierCheckStateAsBitmask();

    if(ampHotkeyVkid) {
        const QString& vkid_hexstr  {
            QString { "0x%1" }.arg(ampHotkeyVkid, 2, 16, QChar { '0' })
        };

        ui->linActivationParameter->setText(vkid_hexstr);

        registerAmpHotkey();
    }

    insertActivationParameterWidget(ampwHotkeyRecorder, false);
    insertActivationParameterWidget(ampwHotkeyModifierDropdown, false);
    insertActivationParameterWidget(btnSpawnVkidTableDialog);
}

void MainWindow::unsetAmToHotkey() {
    unregisterAmpHotkey();

    disconnect(this, &MainWindow::targetHotkeyWasPressed,
               this, &MainWindow::activateBecauseTargetHotkeyWasPressed);

    removeActivationParameterWidget(ampwHotkeyRecorder);
    removeActivationParameterWidget(ampwHotkeyModifierDropdown);
    removeActivationParameterWidget(btnSpawnVkidTableDialog);
}

void MainWindow::activateBecauseTargetHotkeyWasPressed() {
    if(toggleCursorLockState()) {
        dbgConsole->log("Activated cursor lock via hotkey.");
        seLockActivated.play();
    } else {
        dbgConsole->log("Deactivated cursor lock via hotkey.");
        seLockDeactivated.play();
    }
}




// Process Scanner Dialog
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void MainWindow::spawnProcessScannerDialog(ProcessScanner::SCAN_SCOPE process_scanner_scope) {
    QDebugConsoleContext { dbgConsole, "spawnProcessScannerDialog" };

    if(processScannerDialog == nullptr) {
        dbgConsole->log("Constructing new ProcessScannerDialog instance.");

        processScannerDialog = new ProcessScannerDialog(process_scanner_scope, this);

        ui->btnEditActivationParameter->setEnabled(false);
        btnStartWindowGrabber->setEnabled(false);

        connect(processScannerDialog, &ProcessScannerDialog::destroyed, [&](QObject*) -> void {
            ui->btnEditActivationParameter->setEnabled(true);
            btnStartWindowGrabber->setEnabled(true);
            processScannerDialog = nullptr;
        });

        connect(processScannerDialog, &ProcessScannerDialog::treeSelectionMade, [&](const QString& selection) -> void {
            ui->linActivationParameter->setText(selection);
            delete processScannerDialog;
            processScannerDialog = nullptr;
        });

        processScannerDialog->show();
    }
}




// Process Image Activation Method
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void MainWindow::setAmpProcessImageName(const QString& process_image_name) {
    amParamProcessImageName = process_image_name;

    if(selectedActivationMethod == ACTIVATION_METHOD::PROCESS_IMAGE) {
        ui->linActivationParameter->setText(amParamProcessImageName);
    }
}

void MainWindow::setAmToProcessImageName() {
    QDebugConsoleContext dbg_console_context { dbgConsole, "setAmToProcessImageName" };

    selectedActivationMethod = ACTIVATION_METHOD::PROCESS_IMAGE;
    ui->linActivationParameter->setPlaceholderText("Image name, e.g. TESV.exe, SkyrimSE.exe, etc");

    if(amParamProcessImageName.size()) {
        ui->linActivationParameter->setText(amParamProcessImageName);
        dbgConsole->log({"Loaded amParamProcessImageName \"", amParamProcessImageName, "\""});
    }

    timedActivationMethodConnection = connect(timedActivationMethodTimer,   SIGNAL(timeout()),
                                              this,                         SLOT(activateIfTargetProcessRunning()));

    insertActivationParameterWidget(btnSpawnProcessScanner, false);

    btnSpawnProcessScannerConnection = connect(btnSpawnProcessScanner, &QPushButton::clicked,
                                               std::bind(&MainWindow::spawnProcessScannerDialog, this, ProcessScanner::PROCESS_MODE));

    dbgConsole->log("Activation method has been set to process image name.");
}

void MainWindow::unsetAmToProcessImageName() {
    QDebugConsoleContext dbg_console_context { dbgConsole, "unsetAmToProcessImageName" };

    disconnect(timedActivationMethodConnection);
    removeActivationParameterWidget(btnSpawnProcessScanner);
    disconnect(btnSpawnProcessScannerConnection);
}

void MainWindow::activateIfTargetProcessRunning() {
    if(!amParamProcessImageName.size()) {
        setCursorLockEnabled(false);
        return;
    }

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
            setCursorLockEnabled(true);

            if(first_find) {
                dbgConsole->log({"Enabling lock because target process was found: ", amParamProcessImageName});
                first_find = false;

                seLockActivated.play();
            }
        } else {
            setCursorLockEnabled(false);

            if(!first_find) {
                dbgConsole->log({"Disabling lock because target process was lost: ", amParamProcessImageName});
                first_find = true;

                seLockDeactivated.play();
            }
        }
    }

    CloseHandle(process_snapshot);
}




// Foreground Window Activation Method
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void MainWindow::setAmpForegroundWindowTitle(const QString& foreground_window_title) {
    amParamForegroundWindowTitle = foreground_window_title;

    if(selectedActivationMethod == ACTIVATION_METHOD::WINDOW_TITLE) {
        ui->linActivationParameter->setText(amParamForegroundWindowTitle);
    }
}

void MainWindow::setAmToForegroundWindowTitle() {
    QDebugConsoleContext dbg_console_context { dbgConsole, "setAmToForegroundWindowTitle" };

    dbgConsole->log("Activation mode set to window title. Right click the edit button to grab the name of the next foreground window that you select.");

    selectedActivationMethod = ACTIVATION_METHOD::WINDOW_TITLE;
    ui->linActivationParameter->setPlaceholderText("Window title, e.g. Skyrim, Skyrim Special Edition");

    if(amParamForegroundWindowTitle.size()) {
        ui->linActivationParameter->setText(amParamForegroundWindowTitle);
        dbgConsole->log({"Set activation parameter text field to: ", amParamForegroundWindowTitle});
    }

    timedActivationMethodConnection = connect(timedActivationMethodTimer,    SIGNAL(timeout()),
                                              this,                          SLOT(activateIfForegroundWindowMatchesTarget()));

    insertActivationParameterWidget(btnStartWindowGrabber, false);
    insertActivationParameterWidget(btnSpawnProcessScanner, false);
    btnSpawnProcessScannerConnection = connect(btnSpawnProcessScanner, &QPushButton::clicked,
                                               std::bind(&MainWindow::spawnProcessScannerDialog, this, ProcessScanner::WINDOW_MODE));

}

void MainWindow::unsetAmToForegroundWindowTitle() {
    disconnect(timedActivationMethodConnection);
    removeActivationParameterWidget(btnSpawnProcessScanner);
    removeActivationParameterWidget(btnStartWindowGrabber);
    disconnect(btnSpawnProcessScannerConnection);
}

void MainWindow::activateIfForegroundWindowMatchesTarget() {
    if(!amParamForegroundWindowTitle.size()) {
        setCursorLockEnabled(false);
        return;
    }

    static bool first_find { true };

    char window_title_buffer[256];
    std::fill(window_title_buffer, window_title_buffer + sizeof(window_title_buffer), 0x00);

    HWND foreground_window { GetForegroundWindow() };
    GetWindowTextA(foreground_window, window_title_buffer, sizeof(window_title_buffer));

    if(amParamForegroundWindowTitle == window_title_buffer) {
        setCursorLockEnabled(true);

        if(first_find) {
            dbgConsole->log({"Enabling lock because target window was found: ", amParamForegroundWindowTitle});
            first_find = false;

            seLockActivated.play();
            // beepBoop({{500,20}, {700,20}});
        }
    } else {
        setCursorLockEnabled(false);

        if(!first_find) {
            dbgConsole->log({"Disabling lock because target window was lost: ", amParamForegroundWindowTitle});
            first_find = true;

            seLockDeactivated.play();
        }
    }
}



// Foreground Window Grabber
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void MainWindow::onWindowGrabberTimerTimeout() {
    const auto& stop_and_reset {
        [&]() -> void {
            btnStartWindowGrabber->setText("Grab");

            windowGrabberTimer->stop();
            windowGrabberTimerTimeoutCounter = 0;

            ui->linActivationParameter->clear();
            ui->linActivationParameter->setEnabled(true);
            ui->btnEditActivationParameter->setEnabled(true);
            btnSpawnProcessScanner->setEnabled(true);
        }};

    if(++windowGrabberTimerTimeoutCounter > windowGrabberTimerMaxTimeouts) {
        stop_and_reset();
    } else {
        // soundEffectWindowGrabberTick.play();

        ui->linActivationParameter->setText(
                    "Switch to target window within timeframe: "
                    + QString::number(windowGrabberTimerTimeoutCounter)
                    + " / "
                    + QString::number(windowGrabberTimerMaxTimeouts)
                    );

        HWND foreground_window { GetForegroundWindow() };

        // Other window has been selected, as foreground_window doesn't match this program's window.
        if(foreground_window != HWND(winId())) {
            stop_and_reset();

            char window_title[256];
            ZeroMemory(window_title, sizeof(window_title));
            qint32 bytes_written { GetWindowText(foreground_window, window_title, sizeof(window_title)) };
            dbgConsole->log({"GetWindowText() wrote ", QString::number(bytes_written), " bytes to char window_title[256]"});

            if(QString { window_title } != amParamForegroundWindowTitle) {
                seWindowGrabbed.play();
            }

            ui->linActivationParameter->setText(window_title);
        } else {
            seWindowGrabberTick.play();
        }
    }
}

void MainWindow::onWindowGrabberButtonClicked() {
    if(btnStartWindowGrabber->text() == "Grab") {
        btnStartWindowGrabber->setText("Stop");

        ui->linActivationParameter->setEnabled(false);
        ui->btnEditActivationParameter->setEnabled(false);
        btnSpawnProcessScanner->setEnabled(false);

        windowGrabberTimer->start(500);
    } else if(btnStartWindowGrabber->text() == "Stop") {
        btnStartWindowGrabber->setText("Grab");

        windowGrabberTimer->stop();
        windowGrabberTimerTimeoutCounter = 0;

        ui->linActivationParameter->clear();
        ui->linActivationParameter->setEnabled(true);
        ui->btnEditActivationParameter->setEnabled(true);
        btnSpawnProcessScanner->setEnabled(true);
    }
}



// JSON Settings Dialog
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void MainWindow::spawnJsonSettingsDialog() {
    if(jsonSettingsDialog == nullptr) {
        jsonSettingsDialog = new JsonSettingsDialog { jsonConfigFilePath, this };

        connect(jsonSettingsDialog, &JsonSettingsDialog::destroyed, [&]() -> void {
            jsonSettingsDialog = nullptr;
        });

        jsonSettingsDialog->show();
    }
}


// Sound Effects
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void MainWindow::setSoundEffectsMutedState(bool state) {
    soundEffectsMuted = state;

    seLockActivated.setMuted(soundEffectsMuted);
    seLockDeactivated.setMuted(soundEffectsMuted);
    seWindowGrabbed.setMuted(soundEffectsMuted);
    seWindowGrabberTick.setMuted(soundEffectsMuted);

    ui->btnMuteSoundEffects->setText(state ? "Unmute" : "Mute");
}

void MainWindow::toggleSoundEffectsMuted() {
    soundEffectsMuted ^= true;
    setSoundEffectsMutedState(soundEffectsMuted);
}





// Cursor Lock
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void MainWindow::setCursorLockEnabled(const bool& state) {
    if(state) {
        HWND foreground_window_hwnd { GetForegroundWindow() };
        RECT foreground_window_rect;

        GetWindowRect(foreground_window_hwnd, &foreground_window_rect);
        ClipCursor(&foreground_window_rect);
    } else {
        ClipCursor(nullptr);
    }

    lastCursorLockStateSet = state;
}

bool MainWindow::toggleCursorLockState() {
    setCursorLockEnabled(lastCursorLockStateSet ^ true);
    return lastCursorLockStateSet;
}






bool MainWindow::nativeEvent(const QByteArray& event_type, void* message, qintptr* result) {
    Q_UNUSED(event_type);
    Q_UNUSED(result);

    MSG* msg { reinterpret_cast<MSG*>(message) };

    /* msg->lParam Is a 64-bit integer and stores the VKID of the pressed hotkey in byte 3/8 (little-endian)
     * and so it cannot be directly compared with ampHotkeyVkid, as it stores the VKID in byte 1/1 so the
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
        if(msg->wParam == ampHotkeyId && ((msg->lParam >> 16) & 0xFF) == ampHotkeyVkid) {
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


void MainWindow::changeActivationMethod(int method_index) {
    /* Clear linActivationParameter's text to allow the placeholder text of
     * the selected activation method to be displayed later on.
     * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
    ui->linActivationParameter->clear();
    timedActivationMethodTimer->stop();

    setCursorLockEnabled(false); // Ensure the cursor lock is disabled before switching over to a new activation method.

    unsetAmToHotkey();
    unsetAmToProcessImageName();
    unsetAmToForegroundWindowTitle();

    static const QList<quint32>& timed_activation_methods_indexes { 2, 3 };

    switch(method_index) {
    case 0 :
        dbgConsole->log("Activation method set to nothing.");

        selectedActivationMethod = ACTIVATION_METHOD::NOTHING;
        ui->linActivationParameter->setPlaceholderText("No activation method selected");
        break;

    case 1 :
        setAmToHotkey();
        break;

    case 2 :
        setAmToProcessImageName();
        break;

    case 3 :
        setAmToForegroundWindowTitle();
        break;
    }

    if(timed_activation_methods_indexes.contains(method_index)) {
        timedActivationMethodTimer->start(500);
    }
}

bool MainWindow::changeActivationMethod(const QString& method) {
    if(JsonSettingsDialog::ActivationMethodResolverSTOI.contains(method)) {
        ui->cbxActivationMethod->setCurrentIndex(JsonSettingsDialog::ActivationMethodResolverSTOI[method]);
        return true;
    }

    return false;
}

void MainWindow::editActivationMethodParameter() {
    if(ui->btnEditActivationParameter->text() == "Edit" && selectedActivationMethod != ACTIVATION_METHOD::NOTHING) {
        ui->btnEditActivationParameter->setText("Confirm");
        ui->linActivationParameter->setEnabled(true);
        ui->cbxActivationMethod->setEnabled(false);

        btnSpawnProcessScanner->setEnabled(true);
        btnStartWindowGrabber->setEnabled(true);

        if(selectedActivationMethod == ACTIVATION_METHOD::HOTKEY) {
            ampwHotkeyModifierDropdown->setEnabled(true);
            ampwHotkeyRecorder->setEnabled(true);
            ampwHotkeyRecorder->StartRecording();
        }
    } else if(ui->btnEditActivationParameter->text() == "Confirm") {
        ui->btnEditActivationParameter->setText("Edit");
        ui->linActivationParameter->setEnabled(false);
        ui->cbxActivationMethod->setEnabled(true);

        btnSpawnProcessScanner->setEnabled(false);
        btnStartWindowGrabber->setEnabled(false);

        ampwHotkeyModifierDropdown->setEnabled(false);
        ampwHotkeyRecorder->setEnabled(false);
        ampwHotkeyRecorder->StopRecording();

        switch(selectedActivationMethod) {
        case ACTIVATION_METHOD::HOTKEY : {
            ampHotkeyModifiersBitmask = ampwHotkeyModifierDropdown->GetModifierCheckStateAsBitmask();
            setAmpHotkeyVkid(ui->linActivationParameter->text());
            ampwHotkeyRecorder->clear();
            break;
        }

        case ACTIVATION_METHOD::PROCESS_IMAGE :
            setAmpProcessImageName(ui->linActivationParameter->text());
            break;

        case ACTIVATION_METHOD::WINDOW_TITLE :
            setAmpForegroundWindowTitle(ui->linActivationParameter->text());
            break;

        case ACTIVATION_METHOD::NOTHING :
            break;
        }
    }
}

MainWindow::MainWindow(QWidget* parent)
    :
      // Debug Console & Related Widgets Initialization

      QMainWindow                         { parent                       },
      ui                                  { new Ui::MainWindow           },

      dbgConsole                          { new QDebugConsole   { this } },
      vblDebugConsoleLayout               { new QVBoxLayout              },
      dbgConsoleContextMenu               { new QMenu { dbgConsole }     },

      selectedActivationMethod            { ACTIVATION_METHOD::NOTHING   },

      // Hotkey activation method member variables initialization
      ampwHotkeyModifierDropdown          { new QKbModifierList { this } },
      ampwHotkeyRecorder                  { new QHotkeyInput    { this } },


      vkidTableDialog                     { nullptr                      },
      btnSpawnVkidTableDialog             { new QPushButton     { this } },

      ampHotkeyModifiersBitmask           { WINMOD_NULLMOD               },
      ampHotkeyVkid                       { 0x000                        },
      ampHotkeyId                         { 0x1A4                        },


      // Process scanner member variables initialization.
      processScannerDialog                { nullptr                      },     // ProcessScannerDialog instance, must be nullptr as spawnProcessScannerDialog takes care of construction and destruction.
      btnSpawnProcessScanner              { new QPushButton     { this } },     // QPushButton connected to spawnProcessScannerDialog further down in the constructor.

      timedActivationMethodTimer          { new QTimer          { this } },

      amParamProcessImageName             { QString { "" }               },
      amParamForegroundWindowTitle        { QString { "" }               },

      // Foreground window grabber
      windowGrabberTimerMaxTimeouts       { 15                           },
      windowGrabberTimerTimeoutCounter    { NULL                         },
      windowGrabberTimer                  { new QTimer          { this } },
      btnStartWindowGrabber               { new QPushButton     { this } },

      jsonConfigFilePath                  { "./defaults.json"            },
      jsonSettingsDialog                  { nullptr                      },

      soundEffectsMuted                   { false                        }

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

    resize(static_cast<qint32>(minimumWidth() * 1.2), height());

    vblDebugConsoleLayout->addWidget(dbgConsole);
    ui->grpDebugConsole->setLayout(vblDebugConsoleLayout);

    seLockActivated.setSource(QUrl::fromLocalFile(":/sounds/lock-activated.wav"));
    seLockDeactivated.setSource(QUrl::fromLocalFile(":/sounds/lock-deactivated.wav"));

    seWindowGrabbed.setSource(QUrl::fromLocalFile(":/sounds/window-grabbed.wav"));
    seWindowGrabberTick.setSource(QUrl::fromLocalFile(":/sounds/window-grabber-tick.wav"));

    // Set ampwHotkeyModifierDropdown initial values.
    ampwHotkeyModifierDropdown->setEnabled(false);
    ampwHotkeyModifierDropdown->setHidden(true);
    ampwHotkeyModifierDropdown->setMinimumWidth(90);
    ampwHotkeyModifierDropdown->addItem("Modifiers");
    ampwHotkeyModifierDropdown->AddItemsFromBitmask(WINMOD_ALT | WINMOD_CONTROL | WINMOD_SHIFT | WINMOD_WIN);

    // Set ampwHotkeyRecorder initial values.
    ampwHotkeyRecorder->setHidden(true);
    ampwHotkeyRecorder->setPlaceholderText("Hotkey Recorder");

    // btnSpawnProcessScanner initial values.
    btnSpawnProcessScanner->setText("Select");
    btnSpawnProcessScanner->setEnabled(false);
    btnSpawnProcessScanner->setHidden(true);
    btnSpawnProcessScanner->setMinimumHeight(25);

    btnStartWindowGrabber->setText("Grab");
    btnStartWindowGrabber->setEnabled(false);
    btnStartWindowGrabber->setHidden(true);
    btnStartWindowGrabber->setMinimumHeight(25);

    btnSpawnVkidTableDialog->setText("VKID Table");
    btnSpawnVkidTableDialog->setMinimumHeight(25);
    btnSpawnVkidTableDialog->setEnabled(false);
    btnSpawnVkidTableDialog->setHidden(true);




    // Qt SIGNAL/SLOT Connections
    // ----------------------------------------------------------------------------------------------------
    connect(ui->cbxActivationMethod,           SIGNAL(currentIndexChanged(int)),
            this,                              SLOT(changeActivationMethod(int)));

    connect(ui->btnEditActivationParameter,    SIGNAL(clicked()),
            this,                              SLOT(editActivationMethodParameter()));

    connect(ui->btnMuteSoundEffects,           SIGNAL(clicked()),
            this,                              SLOT(toggleSoundEffectsMuted()));

    connect(ampwHotkeyRecorder,                SIGNAL(WindowsHotkeyRecorded(QHotkeyInput::WindowsHotkey)),
            this,                              SLOT(updateUiWithRecordedWindowsHotkey(QHotkeyInput::WindowsHotkey)));

    connect(ampwHotkeyModifierDropdown,        SIGNAL(ModifierBitmaskChanged(const quint32&)),
            this,                              SLOT(updateHotkeyInputWithNewModifierBitmask(const quint32&)));

    connect(windowGrabberTimer,                SIGNAL(timeout()),
            this,                              SLOT(onWindowGrabberTimerTimeout()));

    connect(btnStartWindowGrabber,             SIGNAL(clicked()),
            this,                              SLOT(onWindowGrabberButtonClicked()));

    connect(btnSpawnVkidTableDialog,           SIGNAL(clicked()),
            this,                              SLOT(spawnVkidTableDialog()));

    connect(ui->btnSettings,                   SIGNAL(clicked()),
            this,                              SLOT(spawnJsonSettingsDialog()));




    // Load JSON Settings
    // ----------------------------------------------------------------------------------------------------
    QFileInfo json_file_info { jsonConfigFilePath };

    if(json_file_info.exists() && json_file_info.isFile()) {
        const JsonSettingsDialog::JsonSettings& json_settings { JsonSettingsDialog::LoadSettingsFromJsonFile(this, jsonConfigFilePath) };

        setAmpForegroundWindowTitle(json_settings.ForegroundWindowTitle);
        setAmpProcessImageName(json_settings.ProcessImageName);
        setAmpHotkeyVkid(json_settings.HotkeyVkid);
        setSoundEffectsMutedState(json_settings.InitialMuteState);

        ampwHotkeyModifierDropdown->SetModifierCheckStateFromBitmask(json_settings.HotkeyModifierBitmask);

        changeActivationMethod(json_settings.ActivationMethod);
    }



    // Load Style Sheet
    // ----------------------------------------------------------------------------------------------------
    QFileInfo style_sheet_file_info { styleSheetFilePath };

    if(style_sheet_file_info.exists() && style_sheet_file_info.isFile()) {
        QFile style_sheet_file { style_sheet_file_info.absoluteFilePath() };

        if(style_sheet_file.open(QFile::ReadOnly | QFile::OpenModeFlag::Text)) {
            QByteArray style_sheet_bytes { style_sheet_file.readAll() };
            style_sheet_file.close();

            setStyleSheet(style_sheet_bytes);
        } else {
            dbgConsole->log({"Encountered an I/O error when attempting to open stylesheet file for reading, location: \"", style_sheet_file_info.absoluteFilePath(), "\" - are you sure you have permission to read from this location?"}, QDebugConsole::LL_ERROR);
        }
    } else {
        dbgConsole->log("Skipping stylesheet loading, as the stylesheet could not be found, using default Windows style instead.", QDebugConsole::LL_WARNING);
    }
}

MainWindow::~MainWindow() {
    setCursorLockEnabled(false);
    delete ui;
}
