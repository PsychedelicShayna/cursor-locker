#ifndef MAIN_WINDOW_DLG_HXX
#define MAIN_WINDOW_DLG_HXX

#undef UNICODE

#include <QMainWindow>

#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>

#include <algorithm>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
private:
    Ui::MainWindow* ui;
    Q_OBJECT

    std::atomic<uint32_t> target_position_x;
    std::atomic<uint32_t> target_position_y;

    std::atomic<bool> cursor_locker_activated;
    [[noreturn]] void cursor_locker_worker();

    std::atomic<bool> activation_mode;
    std::atomic<uint32_t> numerical_vkid;
    [[noreturn]] void activator_worker();

    std::mutex console_mutex;

    void log_to_console(const QList<QString>&);

private slots:
    void update_frequency_group_title();
    void update_resolution_center();

    void invert_lock_state();
    void set_lock_state(bool);

    void on_btn_edit_activation_clicked();
    void on_cbx_activation_currentIndexChanged(int);

public:
    std::thread cursor_locker_thread;
    std::thread activator_thread;

    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
};

#endif // MAIN_WINDOW_DLG_HXX
