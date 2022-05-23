#include "main_window_dialog.hxx"
#include "ui_main_window_dialog.h"

enum struct ACTIVATION_METHOD {
    NOTHING         =   0b00000000,
    HOTKEY          =   0b00000001,
    PROCESS_IMAGE   =   0b00000010,
    WINDOW_TITLE    =   0b00000100,
};

qsizetype MainWindowDialog::loadQssStylesheet(const QString& style_sheet_path, QByteArray& out_bytes) const {
    QFileInfo style_sheet_info { style_sheet_path };

    if(!style_sheet_info.exists() || !style_sheet_info.isFile()) {
        return -1;
    }

    QFile style_sheet_file { style_sheet_info.absoluteFilePath() };

    if(style_sheet_file.open(QFile::ReadOnly | QFile::Text)) {
        const QByteArray& style_sheet_bytes { style_sheet_file.readAll() };
        style_sheet_file.close();

        out_bytes = style_sheet_bytes;

        return style_sheet_bytes.size();
    }

    return -2;
}

qsizetype MainWindowDialog::loadAndApplyQssStylesheet(const QString& style_sheet_path) {
    QByteArray style_sheet_bytes;

    const qsizetype& bytes_read {
        loadQssStylesheet(style_sheet_path, style_sheet_bytes)
    };

    switch(bytes_read) {
    case -1:
        qWarning() << "Skipping stylesheet loading, as the stylesheet could not be found, using default Windows style instead.";
        break;

    case -2:
        qCritical() << "Encountered an I/O error when attempting to open stylesheet file for reading, location: \"" << styleSheetFilePath << "\" - are you sure you have permission to read from this location?";
        break;

    default:
        const QFileInfo style_sheet_info   { style_sheet_path };
        const QFileInfo resource_file_info { QString { "%1/%2.rcc" }.arg(style_sheet_info.path(), style_sheet_info.completeBaseName()) };

        if(resource_file_info.exists() && resource_file_info.isFile()) {
            if(QResource::registerResource(resource_file_info.absoluteFilePath())) {
                qInfo() << "Registered resource file:" << resource_file_info.absoluteFilePath();
            } else {
                qWarning() << "Failed to register resource file:" << resource_file_info.absoluteFilePath();
            }
        } else {
            qInfo() << "Could not find a matching resource file for the stylesheet that was found. Path that was checked:" << resource_file_info.absoluteFilePath();
        }

        setStyleSheet(style_sheet_bytes);
        break;
    }

    return bytes_read;
}

qsizetype MainWindowDialog::loadAndApplyQssStylesheet() {
    return loadAndApplyQssStylesheet(styleSheetFilePath);
}

// Activation Method
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void MainWindowDialog::insertActivationParameterWidget(QWidget* widget, bool enable, bool unhide) {
    if(widget != nullptr) {
        if(widget->layout() != ui->hlActivationParameter) {
            ui->hlActivationParameter->insertWidget(0, widget, 0);
        }

        widget->setEnabled(enable);
        widget->setHidden(!unhide);
    }
}

void MainWindowDialog::removeActivationParameterWidget(QWidget* widget, bool disable, bool hide) {
    if(widget != nullptr) {
        if(widget->layout() == ui->hlActivationParameter) {
            ui->hlActivationParameter->removeWidget(widget);
        }

        widget->setEnabled(!disable);
        widget->setHidden(hide);
    }
}

void MainWindowDialog::changeActivationMethod(int method_index) {
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
        qInfo() << "Activation method set to nothing.";
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

bool MainWindowDialog::changeActivationMethod(const QString& method) {
    if(JsonSettingsDialog::ActivationMethodResolverSTOI.contains(method)) {
        ui->cbxActivationMethod->setCurrentIndex(JsonSettingsDialog::ActivationMethodResolverSTOI[method]);
        return true;
    }

    return false;
}

void MainWindowDialog::editActivationMethodParameter() {
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

void MainWindowDialog::clearActivationMethodParameter() {
    ui->linActivationParameter->clear();

    switch(selectedActivationMethod) {
    case ACTIVATION_METHOD::NOTHING : {
        break;
    }

    case ACTIVATION_METHOD::HOTKEY : {
        ampwHotkeyModifierDropdown->SetModifierCheckStateFromBitmask(WINMOD_NULLMOD);
        ampwHotkeyRecorder->ClearState();
        setAmpHotkeyVkid(0x00);
        break;
    }

    case ACTIVATION_METHOD::PROCESS_IMAGE : {
        setAmpProcessImageName("");
        break;
    }

    case ACTIVATION_METHOD::WINDOW_TITLE : {
        setAmpForegroundWindowTitle("");
        break;
    }
    }

    if(lastCursorLockStateSet) {
        setCursorLockEnabled(false);
        seLockDeactivated.play();
        qInfo() << "Cursor lock disabled, as activation method parameters have been cleared.";
    }
}



// Keyboard Modifier Dropdown
// -------------------------------------------------------------------------------------------------------------------------------------------------------------
void MainWindowDialog::handleModifierListBitmaskChanged(const quint32& bitmask) {
    ampHotkeyModifiersBitmask = bitmask;
}




// Hotkey Input / Recorder
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void MainWindowDialog::updateUiWithRecordedHotkey(HotkeyRecorderWidget::Hotkey windows_hotkey) {
    const QString& hex_vkid { QString { "0x%1" }.arg(QString::number(windows_hotkey.Vkid, 16), 2, QChar { '0' }) };

    ui->linActivationParameter->setText(hex_vkid);
    ampwHotkeyModifierDropdown->SetModifierCheckStateFromBitmask(windows_hotkey.Modifiers);

    qInfo() << "Ui has been set to use recorded hotkey: " << windows_hotkey.ToString();
}

void MainWindowDialog::updateHotkeyInputWithNewModifierBitmask(const quint32& new_modifier_bitmask) {
    ampwHotkeyRecorder->clear();
}




// VKID Table Dialog
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void MainWindowDialog::spawnVkidTableWidgetDialog() {
    if(vkidTableWidgetDialog == nullptr) {
        vkidTableWidgetDialog = new VkidTableWidgetDialog { this };

        connect(vkidTableWidgetDialog, &VkidTableWidgetDialog::destroyed, [this]() -> void {
            vkidTableWidgetDialog = nullptr;
        });

        vkidTableWidgetDialog->show();
    }
}




// Hotkey Activation Method
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
QString MainWindowDialog::setAmpHotkeyVkid(const quint32& vkid) {
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

QString MainWindowDialog::setAmpHotkeyVkid(const QString& vkid_hexstr) {
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
        qCritical() << "Failed to convert hexadecimal VKID string into an integer \"" << vkid_hexstr << "\"";
        return QString {};    // Return empty QString on conversion failure.
    }
}

bool MainWindowDialog::registerAmpHotkey() {
    BOOL result { RegisterHotKey(HWND(winId()), ampHotkeyId, MOD_NOREPEAT | ampHotkeyModifiersBitmask, ampHotkeyVkid) };

    qInfo() << "RegisterHotKey(HWND(winId), 0x"
            << QString::number(ampHotkeyId, 16)
            << ", MOD_NOREPEAT | "
            << QString::number(ampHotkeyModifiersBitmask)
            << ", 0x"
            << QString::number(ampHotkeyVkid, 16)
            << ") returned "
            << (result ? "true" : "false");

    return result;
}

bool MainWindowDialog::unregisterAmpHotkey() {
    BOOL result { UnregisterHotKey(HWND(winId()), ampHotkeyId) };

    if(result) {
        qInfo() << "UnregisterHotKey(HWND(winId), 0x"
                << QString::number(ampHotkeyId, 16)
                << ") returned true";
    }

    return result;
}

void MainWindowDialog::setAmToHotkey() {
    qInfo() << "Set activation method to: hotkey.";

    selectedActivationMethod = ACTIVATION_METHOD::HOTKEY;

    unregisterAmpHotkey();

    connect(this, &MainWindowDialog::targetHotkeyWasPressed,
            this, &MainWindowDialog::activateBecauseTargetHotkeyWasPressed);

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
    insertActivationParameterWidget(btnSpawnVkidTableWidgetDialog);
}

void MainWindowDialog::unsetAmToHotkey() {
    unregisterAmpHotkey();

    disconnect(this, &MainWindowDialog::targetHotkeyWasPressed,
               this, &MainWindowDialog::activateBecauseTargetHotkeyWasPressed);

    removeActivationParameterWidget(ampwHotkeyRecorder);
    removeActivationParameterWidget(ampwHotkeyModifierDropdown);
    removeActivationParameterWidget(btnSpawnVkidTableWidgetDialog);
}

void MainWindowDialog::activateBecauseTargetHotkeyWasPressed() {
    if(toggleCursorLockState()) {
        qInfo() << "Activated cursor lock via hotkey.";
        seLockActivated.play();
    } else {
        qInfo() << "Deactivated cursor lock via hotkey.";
        seLockDeactivated.play();
    }
}




// Process Scanner Dialog
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void MainWindowDialog::spawnProcessScannerDialog(ProcessScanner::SCAN_SCOPE process_scanner_scope) {
    if(processScannerDialog == nullptr) {
        qInfo() << "Constructing new ProcessScannerDialog instance.";

        processScannerDialog = new ProcessScannerDialog { this, process_scanner_scope };

        ui->btnEditActivationParameter->setEnabled(false);
        btnStartWindowGrabber->setEnabled(false);

        connect(processScannerDialog, &ProcessScannerDialog::destroyed, [&](QObject*) -> void {
            ui->btnEditActivationParameter->setEnabled(true);
            btnStartWindowGrabber->setEnabled(true);
            processScannerDialog = nullptr;
        });

        connect(processScannerDialog, &ProcessScannerDialog::treeSelectionMade, [&](QString selection, HWND window_handle) -> void {
            ui->linActivationParameter->setText(selection);
            delete processScannerDialog;
            processScannerDialog = nullptr;
        });

        processScannerDialog->show();
    }
}




// Process Image Activation Method
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void MainWindowDialog::setAmpProcessImageName(const QString& process_image_name) {
    amParamProcessImageName = process_image_name;

    if(selectedActivationMethod == ACTIVATION_METHOD::PROCESS_IMAGE) {
        ui->linActivationParameter->setText(amParamProcessImageName);
    }
}

void MainWindowDialog::setAmToProcessImageName() {
    selectedActivationMethod = ACTIVATION_METHOD::PROCESS_IMAGE;
    ui->linActivationParameter->setPlaceholderText("Image name, e.g. TESV.exe, SkyrimSE.exe, etc");

    if(amParamProcessImageName.size()) {
        ui->linActivationParameter->setText(amParamProcessImageName);

        qInfo() << "Loaded amParamProcessImageName \""
                << amParamProcessImageName
                << "\"";
    }

    timedActivationMethodConnection = connect(timedActivationMethodTimer,   SIGNAL(timeout()),
                                              this,                         SLOT(activateIfTargetProcessRunning()));

    insertActivationParameterWidget(btnSpawnProcessScanner, false);

    btnSpawnProcessScannerConnection = connect(btnSpawnProcessScanner, &QPushButton::clicked,
                                               std::bind(&MainWindowDialog::spawnProcessScannerDialog, this, ProcessScanner::PROCESS_MODE));

    qInfo() << "Activation method has been set to process image name.";
}

void MainWindowDialog::unsetAmToProcessImageName() {
    disconnect(timedActivationMethodConnection);
    removeActivationParameterWidget(btnSpawnProcessScanner);
    disconnect(btnSpawnProcessScannerConnection);
}

void MainWindowDialog::activateIfTargetProcessRunning() {
    if(!amParamProcessImageName.size()) {
        setCursorLockEnabled(false);
        processHasBeenFound = false;
        return;
    }

    HANDLE process_snapshot { CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,  0) };

    if(process_snapshot == INVALID_HANDLE_VALUE) {
        qCritical() << "Invalid handle value for snapshot of type TH32CS_SNAPPROCESS. Cannot see running tasks!";
        return;
    }

    PROCESSENTRY32 process_entry_32;
    process_entry_32.dwSize = sizeof(PROCESSENTRY32);

    if(Process32First(process_snapshot, &process_entry_32)) {
        bool process_included_in_scan { false };

        do {
            if(amParamProcessImageName == QString::fromWCharArray(process_entry_32.szExeFile)) {
                process_included_in_scan = true;
                break;
            }
        } while(Process32Next(process_snapshot, &process_entry_32));

        if(process_included_in_scan) {
            setCursorLockEnabled(true);

            if(!processHasBeenFound) {
                qInfo() << "Enabling lock because target process was found: "
                        << amParamProcessImageName;

                processHasBeenFound = true;

                seLockActivated.play();
            }
        } else {
            setCursorLockEnabled(false);

            if(processHasBeenFound) {
                qInfo() << "Disabling lock because target process was lost: "
                        << amParamProcessImageName;

                processHasBeenFound = false;

                seLockDeactivated.play();
            }
        }
    }

    CloseHandle(process_snapshot);
}




// Foreground Window Activation Method
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void MainWindowDialog::setAmpForegroundWindowTitle(const QString& foreground_window_title) {
    amParamForegroundWindowTitle = foreground_window_title;

    if(selectedActivationMethod == ACTIVATION_METHOD::WINDOW_TITLE) {
        ui->linActivationParameter->setText(amParamForegroundWindowTitle);
    }
}

void MainWindowDialog::setAmToForegroundWindowTitle() {
    qInfo() << "Activation mode set to window title. Right click the edit button to grab the name of the next foreground window that you select.";

    selectedActivationMethod = ACTIVATION_METHOD::WINDOW_TITLE;
    ui->linActivationParameter->setPlaceholderText("Window title, e.g. Skyrim, Skyrim Special Edition");

    if(amParamForegroundWindowTitle.size()) {
        ui->linActivationParameter->setText(amParamForegroundWindowTitle);

        qInfo() << "Set activation parameter text field to: "
                << amParamForegroundWindowTitle;
    }

    timedActivationMethodConnection = connect(timedActivationMethodTimer,    SIGNAL(timeout()),
                                              this,                          SLOT(activateIfForegroundWindowMatchesTarget()));

    insertActivationParameterWidget(btnStartWindowGrabber, false);
    insertActivationParameterWidget(btnSpawnProcessScanner, false);

    btnSpawnProcessScannerConnection = connect(btnSpawnProcessScanner, &QPushButton::clicked,
                                               std::bind(&MainWindowDialog::spawnProcessScannerDialog, this, ProcessScanner::WINDOW_MODE));
}

void MainWindowDialog::unsetAmToForegroundWindowTitle() {
    disconnect(timedActivationMethodConnection);
    removeActivationParameterWidget(btnSpawnProcessScanner);
    removeActivationParameterWidget(btnStartWindowGrabber);
    disconnect(btnSpawnProcessScannerConnection);
}

void MainWindowDialog::activateIfForegroundWindowMatchesTarget() {
    if(!amParamForegroundWindowTitle.size()) {
        setCursorLockEnabled(false);
        windowTitleHasBeenFound = false;
        return;
    }

    wchar_t window_title_buffer[256];
    ZeroMemory(window_title_buffer, sizeof(window_title_buffer));

    HWND foreground_window { GetForegroundWindow() };
    GetWindowText(foreground_window, window_title_buffer, sizeof(window_title_buffer));

    if(amParamForegroundWindowTitle == QString::fromWCharArray(window_title_buffer)) {
        setCursorLockEnabled(true);

        if(!windowTitleHasBeenFound) {
            qInfo() << "Enabling lock because target window was found: "
                    << amParamForegroundWindowTitle;

            windowTitleHasBeenFound = true;

            seLockActivated.play();
        }
    } else {
        setCursorLockEnabled(false);

        if(windowTitleHasBeenFound) {
            qInfo() << "Disabling lock because target window was lost: "
                    << amParamForegroundWindowTitle;

            windowTitleHasBeenFound = false;
            seLockDeactivated.play();
        }
    }
}



// Foreground Window Grabber
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void MainWindowDialog::onWindowGrabberTimerTimeout() {
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

            wchar_t window_title[256];
            ZeroMemory(window_title, sizeof(window_title));
            qint32 bytes_written { GetWindowText(foreground_window, window_title, sizeof(window_title)) };

            qInfo() << "GetWindowText() wrote "
                    << QString::number(bytes_written)
                    << " bytes to char window_title[256]";

            if(selectedActivationMethod == ACTIVATION_METHOD::WINDOW_TITLE && ui->btnEditActivationParameter->text() == "Confirm") {
                ui->btnEditActivationParameter->setText("Edit");

                ui->linActivationParameter->setEnabled(false);
                ui->cbxActivationMethod->setEnabled(true);

                btnSpawnProcessScanner->setEnabled(false);
                btnStartWindowGrabber->setEnabled(false);

                setAmpForegroundWindowTitle(QString::fromWCharArray(window_title));
            }
        } else {
            seWindowGrabberTick.play();
        }
    }
}

void MainWindowDialog::onWindowGrabberButtonClicked() {
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
void MainWindowDialog::spawnJsonSettingsDialog() {
    if(jsonSettingsDialog == nullptr) {
        jsonSettingsDialog = new JsonSettingsDialog { jsonConfigFilePath, this };

        connect(jsonSettingsDialog, &JsonSettingsDialog::destroyed, [&]() -> void {
            jsonSettingsDialog = nullptr;
        });

        jsonSettingsDialog->show();
    }
}

void MainWindowDialog::loadAndApplyJsonSettings() {
    QFileInfo json_file_info { jsonConfigFilePath };
    JsonSettingsDialog::JsonSettings json_settings;
    json_settings.StylesheetPath = styleSheetFilePath;    // Default stylesheet location.

    if(json_file_info.exists() && json_file_info.isFile()) {
        const qsizetype& bytes_read { json_settings.LoadFromFile(jsonConfigFilePath, this) };

        if(bytes_read > 0) {
            setAmpForegroundWindowTitle(json_settings.ForegroundWindowTitle);
            setAmpProcessImageName(json_settings.ProcessImageName);
            setAmpHotkeyVkid(json_settings.HotkeyVkid);
            setSoundEffectsMutedState(json_settings.InitialMuteState);
            ampwHotkeyModifierDropdown->SetModifierCheckStateFromBitmask(json_settings.HotkeyModifierBitmask);
            changeActivationMethod(json_settings.ActivationMethod);

            qInfo() << "Read"
                    << QString::number(bytes_read)
                    << "bytes from JSON file: "
                    << json_file_info.absoluteFilePath();
        } else {
            qCritical() << "JsonSettings::LoadFromFile returned a value <= 0:"
                        << QString::number(bytes_read);
        }
    } else {
        QMessageBox::information(this, "Generating New JSON File", "The required defaults.json file could not be found, generating a new one at this location: " + json_file_info.absoluteFilePath());

        qInfo() << "Generating a new JSON config file at this location, because an existing one could not be found:"
                << json_file_info.absoluteFilePath();

        const qsizetype& bytes_written { json_settings.SaveToFile(json_file_info.absoluteFilePath(), this) };

        if(bytes_written > 0) {
            qInfo() << "Wrote"
                    << QString::number(bytes_written)
                    << " bytes to JSON file:"
                    << json_file_info.absoluteFilePath();
        } else {
            qCritical() << "JsonSettings::SaveToFile returned a value <= 0:"
                        << QString::number(bytes_written);
        }
    }

    loadAndApplyQssStylesheet(json_settings.StylesheetPath);
}


// Sound Effects
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void MainWindowDialog::setSoundEffectsMutedState(bool state) {
    soundEffectsMuted = state;

    seLockActivated.setMuted(soundEffectsMuted);
    seLockDeactivated.setMuted(soundEffectsMuted);
    seWindowGrabberTick.setMuted(soundEffectsMuted);

    ui->btnMuteSoundEffects->setText(state ? "Unmute" : "Mute");
}

void MainWindowDialog::toggleSoundEffectsMuted() {
    soundEffectsMuted ^= true;
    setSoundEffectsMutedState(soundEffectsMuted);
}





// Cursor Lock
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void MainWindowDialog::setCursorLockEnabled(const bool& state) {
    if(state) {
        HWND foreground_window_hwnd { GetForegroundWindow() };
        RECT foreground_window_rect;

        if(foreground_window_hwnd != nullptr && IsWindow(foreground_window_hwnd)) {
            if(GetWindowRect(foreground_window_hwnd, &foreground_window_rect)) {
                ClipCursor(&foreground_window_rect);
                lastCursorLockStateSet = state;
            }
        }
    } else {
        ClipCursor(nullptr);
        lastCursorLockStateSet = state;
    }
}


bool MainWindowDialog::toggleCursorLockState() {
    setCursorLockEnabled(lastCursorLockStateSet ^ true);
    return lastCursorLockStateSet;
}






bool MainWindowDialog::nativeEvent(const QByteArray& event_type, void* message, qintptr* result) {
    Q_UNUSED(event_type)
    Q_UNUSED(result)


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

        qInfo() << "nativeEvent got MSG WM_HOTKEY, wParam=0x"
                << QString::number(msg->wParam, 16)
                << ", lParam=0x"
                << QString::number(msg->lParam, 16);
    }

    return QMainWindow::nativeEvent(event_type, message, result);
}

void MainWindowDialog::mousePressEvent(QMouseEvent* mouse_press_event) {
    QWidget* focused_widget { QApplication::focusWidget() };

    if(qobject_cast<HotkeyRecorderWidget*>(focused_widget) || qobject_cast<QLineEdit*>(focused_widget)) {
        focused_widget->clearFocus();
    }

    return QMainWindow::mousePressEvent(mouse_press_event);
}

MainWindowDialog::MainWindowDialog(QWidget* parent)
    :
      // Debug Console & Related Widgets Initialization

      QMainWindow                         { parent                            },
      ui                                  { new Ui::MainWindowDialog          },

      styleSheetFilePath                  { "styles/indigo.qss"               },

      selectedActivationMethod            { ACTIVATION_METHOD::NOTHING        },

      // Hotkey activation method member variables initialization
      ampwHotkeyModifierDropdown          { new QKbModifierList      { this } },
      ampwHotkeyRecorder                  { new HotkeyRecorderWidget { this } },


      vkidTableWidgetDialog               { nullptr                           },
      btnSpawnVkidTableWidgetDialog       { new QPushButton          { this } },

      ampHotkeyModifiersBitmask           { WINMOD_NULLMOD                    },
      ampHotkeyVkid                       { 0x000                             },
      ampHotkeyId                         { 0x1A4                             },


      // Process scanner member variables initialization.
      processScannerDialog                { nullptr                           },     // ProcessScannerDialog instance, must be nullptr as spawnProcessScannerDialog takes care of construction and destruction.
      btnSpawnProcessScanner              { new QPushButton          { this } },     // QPushButton connected to spawnProcessScannerDialog further down in the constructor.

      timedActivationMethodTimer          { new QTimer               { this } },

      // Process Image Name
      processHasBeenFound                 { false                             },
      amParamProcessImageName             { QString { "" }                    },

      // Foreground Window Title
      windowTitleHasBeenFound             { false                             },
      amParamForegroundWindowTitle        { QString { "" }                    },

      // Foreground Window Grabber
      windowGrabberTimerMaxTimeouts       { 15                                },
      windowGrabberTimerTimeoutCounter    { NULL                              },
      windowGrabberTimer                  { new QTimer               { this } },
      btnStartWindowGrabber               { new QPushButton          { this } },

      jsonConfigFilePath                  { "./defaults.json"                 },
      jsonSettingsDialog                  { nullptr                           },

      soundEffectsMuted                   { false                             }

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

    seLockActivated.setSource(QUrl::fromLocalFile(":/sounds/lock-activated.wav"));
    seLockDeactivated.setSource(QUrl::fromLocalFile(":/sounds/lock-deactivated.wav"));
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
    btnSpawnProcessScanner->setMinimumWidth(90);

    btnStartWindowGrabber->setText("Grab");
    btnStartWindowGrabber->setEnabled(false);
    btnStartWindowGrabber->setHidden(true);
    btnStartWindowGrabber->setMinimumWidth(90);

    btnSpawnVkidTableWidgetDialog->setText("VKID Table");
    btnSpawnVkidTableWidgetDialog->setEnabled(false);
    btnSpawnVkidTableWidgetDialog->setHidden(true);
    btnSpawnVkidTableWidgetDialog->setMinimumWidth(90);

    setMaximumHeight(height());    // Disable vertical resizing.

    // Qt SIGNAL/SLOT Connections
    // ----------------------------------------------------------------------------------------------------
    connect(ui->cbxActivationMethod,           &QComboBox::currentIndexChanged,
            this,                              static_cast<void(MainWindowDialog::*)(int)>(&MainWindowDialog::changeActivationMethod));

    connect(ui->btnEditActivationParameter,    &QPushButton::clicked,
            this,                              &MainWindowDialog::editActivationMethodParameter);

    connect(ui->btnClearActivationParameter,   &QPushButton::clicked,
            this,                              &MainWindowDialog::clearActivationMethodParameter);

    connect(ui->btnMuteSoundEffects,           &QPushButton::clicked,
            this,                              &MainWindowDialog::toggleSoundEffectsMuted);

    connect(ampwHotkeyRecorder,                &HotkeyRecorderWidget::HotkeyRecorded,
            this,                              &MainWindowDialog::updateUiWithRecordedHotkey);

    connect(ampwHotkeyModifierDropdown,        &QKbModifierList::ModifierBitmaskChanged,
            this,                              &MainWindowDialog::updateHotkeyInputWithNewModifierBitmask);

    connect(windowGrabberTimer,                &QTimer::timeout,
            this,                              &MainWindowDialog::onWindowGrabberTimerTimeout);

    connect(btnStartWindowGrabber,             &QPushButton::clicked,
            this,                              &MainWindowDialog::onWindowGrabberButtonClicked);

    connect(btnSpawnVkidTableWidgetDialog,     &QPushButton::clicked,
            this,                              &MainWindowDialog::spawnVkidTableWidgetDialog);

    connect(ui->btnSettings,                   &QPushButton::clicked,
            this,                              &MainWindowDialog::spawnJsonSettingsDialog);

    ui->btnSettings->installEventFilter(new AnonymousEventFilter<MainWindowDialog*> {
                                            [](QObject*, QEvent* event, MainWindowDialog* parent) -> bool {
                                                if(event->type() == QEvent::MouseButtonRelease) {
                                                    QMouseEvent* mouse_event { reinterpret_cast<QMouseEvent*>(event) };

                                                    if(mouse_event->button() == Qt::MouseButton::RightButton && parent->selectedActivationMethod == ACTIVATION_METHOD::NOTHING) {
                                                        parent->loadAndApplyJsonSettings();
                                                        event->accept();
                                                        return true;
                                                    }
                                                }

                                                return false;
                                            }, this});

    // QSS Stylesheet Polish Connections
    // ----------------------------------------------------------------------------------------------------
    connect(ui->linActivationParameter,        &QLineEdit::textChanged, [&]() -> void { style()->polish(ui->linActivationParameter); });
    connect(ampwHotkeyRecorder,                &QLineEdit::textChanged, [&]() -> void { style()->polish(ampwHotkeyRecorder);         });

    // Load JSON Settings
    // ----------------------------------------------------------------------------------------------------
    loadAndApplyJsonSettings();    // Also responsible for loading and applying stylesheet, based on the JSON file values.
}

MainWindowDialog::~MainWindowDialog() {
    setCursorLockEnabled(false);
    delete ui;
}
