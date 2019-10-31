#include "main_window_dlg.hxx"
#include "ui_main_window_dlg.h"

enum struct MONITOR_FOR {
    NOTHING         =   0b00000000,
    KEYBIND         =   0b00000001,
    PROCESS_IMAGE   =   0b00000010,
    WINDOW_TITLE    =   0b00000100,
};

void MainWindow::monitoringWorker() {
    HWND desktop_window = GetDesktopWindow();

    RECT clip_cursor_rect;
    GetWindowRect(desktop_window, &clip_cursor_rect);

    clip_cursor_rect.bottom /= 2;
    clip_cursor_rect.right /= 2;
    clip_cursor_rect.top = clip_cursor_rect.bottom;
    clip_cursor_rect.left = clip_cursor_rect.right;

    for(;;) {
        if(monitoringWorkerMode == MONITOR_FOR::KEYBIND) {
            if((GetAsyncKeyState(monitoringWorkerVkid) & 0x8000) != 0) {
                static bool clip_state_toggle = 0;

                clip_state_toggle ^= 1;

                ClipCursor(clip_state_toggle ? &clip_cursor_rect : nullptr);

                Beep(clip_state_toggle ? 500 : 700, 20);
                Beep(clip_state_toggle ? 700 : 500, 20);

                logToConsole({clip_state_toggle ? "Activated" : "Deactivated", " lock."});

                Sleep(500);
            }

            Sleep(50);
        } else if(monitoringWorkerMode == MONITOR_FOR::PROCESS_IMAGE) {
            HANDLE running_tasks_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,  0);

            if(running_tasks_snapshot == INVALID_HANDLE_VALUE) {
                logToConsole({"Invalid handle value for snapshot of type TH32CS_SNAPPROCESS. Cannot see running tasks."});
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
                        ClipCursor(&clip_cursor_rect);

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
                ClipCursor(&clip_cursor_rect);

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

void MainWindow::on_cbx_activation_method_currentIndexChanged(int index) {
    switch(index) {
        case 0 : {
            monitoringWorkerMode = MONITOR_FOR::NOTHING;
            ui->lin_activation_parameter->setText("");
            ui->lin_activation_parameter->setPlaceholderText("No activation method selected");
            logToConsole({"Activation mode set to nothing."});
            break;
        }

        case 1 : {
            monitoringWorkerMode = MONITOR_FOR::KEYBIND;
            ui->lin_activation_parameter->setText("");
            ui->lin_activation_parameter->setPlaceholderText("VKID, default: 0x6A (Numpad *)");
            logToConsole({"Activation mode set to keybind, VKID list: http://www.kbdedit.com/manual/low_level_vk_list.html"});
            break;
        }

        case 2 : {
            monitoringWorkerMode = MONITOR_FOR::PROCESS_IMAGE;
            ui->lin_activation_parameter->setText("");
            ui->lin_activation_parameter->setPlaceholderText("E.g. TESV.exe, SkyrimSE.exe, etc");
            logToConsole({"Activation mode set to process image."});
            break;
        }

        case 3 : {
            monitoringWorkerMode = MONITOR_FOR::WINDOW_TITLE;
            ui->lin_activation_parameter->setText("");
            ui->lin_activation_parameter->setPlaceholderText("E.g. Skyrim, Skyrim Special Edition, etc");
            logToConsole({"Activation mode set to window title."});
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
        }

        ui->lin_activation_parameter->setEnabled(false);
        ui->btn_edit_activation_parameter->setText("Edit");
    }
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

    setWindowFlags(
        Qt::Dialog |
        Qt::CustomizeWindowHint |
        Qt::WindowTitleHint |
        Qt::WindowCloseButtonHint |
        Qt::WindowMinimizeButtonHint |
        Qt::WindowMaximizeButtonHint
    );

    monitoringWorkerMode    = MONITOR_FOR::NOTHING;
    monitoringWorkerVkid    = 0x00;
    monitoringWorkerImage   = new char[2];
    monitoringWorkerTitle   = new char[2];

    std::fill(monitoringWorkerImage.load(), monitoringWorkerImage.load() + 1, 0x00);
    std::fill(monitoringWorkerTitle.load(), monitoringWorkerTitle.load() + 1, 0x00);

    MonitoringThread = std::thread([this]() -> void { monitoringWorker(); });

    HWND desktop_window_handle = GetDesktopWindow();
    
    RECT desktop_window_rect;
    GetWindowRect(desktop_window_handle, &desktop_window_rect);
        
    resize(20 * desktop_window_rect.right / 100, 10 * desktop_window_rect.bottom / 100);

    if(LoadStylesheetFile("./style_sheet.qss")) {
        logToConsole({"style_sheet.qss Loaded"});
    } else {
        logToConsole({"Cannot open style_sheet.qss, using default style."});
    }
}

MainWindow::~MainWindow() {
    delete ui;
}