#include "main_window_dlg.hxx"
#include "ui_main_window_dlg.h"

void MainWindow::cursor_locker_worker() {
    std::stringstream conversion_stream;
    conversion_stream << std::this_thread::get_id();
    log_to_console({"Cursor locker worker started on thread ", QString::fromStdString(conversion_stream.str())});

    for(;;Sleep(1000)) {
        for(;cursor_locker_activated;Sleep(ui->hslid_lock_frequency->value())) {
            SetCursorPos(target_position_x, target_position_y);
        }
    }
}

void MainWindow::activator_worker() {
    std::stringstream conversion_stream;
    conversion_stream << std::this_thread::get_id();
    log_to_console({"Activator worker started on thread ", QString::fromStdString(conversion_stream.str())});

    for(;;Sleep(activation_mode ? 1000 : 50)) {
        if(activation_mode) {
            const std::string& process_name = ui->lin_process->text().toStdString();

            HANDLE running_tasks_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,  0);

            if(running_tasks_snapshot == INVALID_HANDLE_VALUE) {

                ui->txt_console->appendPlainText("! Invalid handle value for snapshot of type TH32CS_SNAPPROCESS. Cannot see running tasks.");
                continue;
            } else {
                PROCESSENTRY32 process_entry_32;
                process_entry_32.dwSize = sizeof(PROCESSENTRY32);

                if(Process32First(running_tasks_snapshot, &process_entry_32)) {
                    bool process_found = false;

                    do {
                        if(!stricmp(process_name.data(), process_entry_32.szExeFile)) {
                            process_found = true;
                            break;
                        }
                    } while(Process32Next(running_tasks_snapshot, &process_entry_32));

                    if(process_found && !cursor_locker_activated) {
                        set_lock_state(true);
                        log_to_console({"Found running process: ", QString::fromStdString(process_name)});
                    } else if(!process_found && cursor_locker_activated) {
                        set_lock_state(false);
                        log_to_console({"Process lost: ", QString::fromStdString(process_name)});
                    }
                }
            }

            CloseHandle(running_tasks_snapshot);
        } else {
            if((GetAsyncKeyState(numerical_vkid) & 0x8000) != 0) {
                invert_lock_state();
                Sleep(500);
            }
        }
    }
}

void MainWindow::update_frequency_group_title() {
    const uint32_t current_frequency = ui->hslid_lock_frequency->value();

    QString message_string("Locking Frequency [ 1ms - 200ms ] (");
    message_string += QString::fromStdString(std::to_string(current_frequency));
    message_string += ")";

    ui->grp_lock_frequency->setTitle(message_string);
}

void MainWindow::update_resolution_center() {
    const uint32_t resolution_x = ui->sbox_resX->value();
    const uint32_t resolution_y = ui->sbox_resY->value();

    target_position_x = resolution_x / 2;
    target_position_y = resolution_y / 2;

    QString target_position_string("~> ");

    target_position_string += QString::fromStdString(std::to_string(target_position_x));
    target_position_string += "x";
    target_position_string += QString::fromStdString(std::to_string(target_position_y));

    ui->lbl_center->setText(target_position_string);

    log_to_console({
        "Center resolution calculated (",
        QString::fromStdString(std::to_string(resolution_x)),
        "/2=", QString::fromStdString(std::to_string(target_position_x)),
        ", ", QString::fromStdString(std::to_string(resolution_y)),
        "/2=", QString::fromStdString(std::to_string(target_position_y)),
        ") ", target_position_string
    });
}

void MainWindow::invert_lock_state() {
    cursor_locker_activated = !cursor_locker_activated;

    if(cursor_locker_activated) {
        ui->btn_lock_state->setText("Unlock");
        Beep(500, 20);
        Beep(700, 20);
    }
    else {
        ui->btn_lock_state->setText("Lock");
        Beep(700, 20);
        Beep(500, 20);
    }

    log_to_console({"Lock ", cursor_locker_activated ? "activated." : "deactivated."});
}

void MainWindow::set_lock_state(bool state) {
    cursor_locker_activated = state;

    if(state) {
        ui->btn_lock_state->setText("Unlock");
        Beep(500, 20);
        Beep(700, 20);
    }

    else {
        ui->btn_lock_state->setText("Lock");
        Beep(700, 20);
        Beep(500, 20);
    }

    log_to_console({"Lock ", cursor_locker_activated ? "activated." : "deactivated."});
}

void MainWindow::on_btn_edit_activation_clicked() {
    if(ui->btn_edit_activation->text() == "Edit") {
        ui->cbx_activation->setEnabled(false);

        if(ui->cbx_activation->currentIndex()) ui->lin_process->setEnabled(true);
        else ui->lin_vkid->setEnabled(true);

        ui->btn_edit_activation->setText("Confirm");
    } else if(ui->btn_edit_activation->text() == "Confirm") {
        if(ui->cbx_activation->currentIndex()) {
            ui->lin_process->setEnabled(false);
        }
        else {
            ui->lin_vkid->setEnabled(false);

            try {
                std::stringstream conversion_stream;
                conversion_stream << std::hex << ui->lin_vkid->text().toStdString();
                uint32_t converted; conversion_stream >> converted;
                numerical_vkid = converted;

            } catch (const std::exception& exception) {
                log_to_console({"Exception when converting VKID into a number: ", exception.what()});
            }
       }

        ui->cbx_activation->setEnabled(true);
        ui->btn_edit_activation->setText("Edit");
    }
}

void MainWindow::on_cbx_activation_currentIndexChanged(int index) {
    activation_mode = index;
    log_to_console({"Activation mode changed to ", index ? "process presence." : "keybinding."});
}

void MainWindow::log_to_console(const QList<QString>& message_list) {
    std::lock_guard<std::mutex> console_mutex_guard(console_mutex);
    QString concatenated_messages;

    for(const auto& message : message_list) {
        concatenated_messages.append(message);
    }

    concatenated_messages += '\n';

    ui->txt_console->moveCursor(QTextCursor::End);
    ui->txt_console->insertPlainText(concatenated_messages);
    ui->txt_console->moveCursor(QTextCursor::End);
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

    update_frequency_group_title();

    numerical_vkid = 0x6A;

    connect(ui->hslid_lock_frequency, SIGNAL(valueChanged(int)), this, SLOT(update_frequency_group_title()));
    connect(ui->sbox_resX, SIGNAL(valueChanged(int)), this, SLOT(update_resolution_center()));
    connect(ui->sbox_resY, SIGNAL(valueChanged(int)), this, SLOT(update_resolution_center()));
    connect(ui->btn_lock_state, SIGNAL(clicked()), this, SLOT(invert_lock_state()));

    HWND desktop_window_handle = GetDesktopWindow();
    RECT screen_rect;

    GetWindowRect(desktop_window_handle, &screen_rect);

    ui->sbox_resX->setValue(screen_rect.right);
    ui->sbox_resY->setValue(screen_rect.bottom);

    cursor_locker_activated = false;
    cursor_locker_thread = std::thread([this]() -> void { cursor_locker_worker(); });

    activation_mode = 0;
    activator_thread = std::thread([this]() -> void { activator_worker(); });

    log_to_console({"Detected resolution: ", QString::fromStdString(std::to_string(screen_rect.right)), "x", QString::fromStdString(std::to_string(screen_rect.bottom))});

    std::ifstream input_stream("./style_sheet.qss", std::ios::binary);

    if(input_stream.good()) {
        std::string style_sheet((std::istreambuf_iterator<char>(input_stream)),(std::istreambuf_iterator<char>()));
        input_stream.close();

        setStyleSheet(QString::fromStdString(style_sheet));
        log_to_console({"Applied style_sheet.qss"});
    } else {
        log_to_console({"Cannot find style_sheet.qss, using default style."});
    }
}

MainWindow::~MainWindow() {
    delete ui;
}