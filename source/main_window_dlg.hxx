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
#include <thread>
#include <atomic>
#include <mutex>

#include "json.hxx"

namespace Ui {
    class MainWindow;
}

using Json = nlohmann::json;

enum struct MONITOR_FOR;

class MainWindow : public QMainWindow {
private:
    Ui::MainWindow* ui;
    Q_OBJECT

    RECT desktopWindowRect;

    // Determines what the mopnitoringWorker should monitor for in order to activate the locker.
    std::atomic<MONITOR_FOR> monitoringWorkerMode;

    // Stores the VKID of the key that monitoringWorker should monitor for, when configured in keybinding mode.
    std::atomic<uint32_t> monitoringWorkerVkid;

    // Stores the image name of the process that monitoringWorker should monitor for, when configured in process presence mode.
    std::atomic<char*> monitoringWorkerImage;

    // Stores the title of the window that monitoringWorker should monitor for, when configured in window monitoring mode.
    std::atomic<char*> monitoringWorkerTitle;

    // The target of MonitoringThread. Monitors for conditions, and activates/deactivates the locking mechanism accordingly.
    [[noreturn]] void monitoringWorker();

    std::mutex consoleMutex; // Multiple threads may log to the console, therefore a mutex is used to avoid data races.
    void logToConsole(const QList<QString>&);
    void logToConsole(const char*);

    void closeEvent(QCloseEvent*);

private slots:
    void on_cbx_activation_method_currentIndexChanged(int);
    void on_btn_edit_activation_parameter_clicked();

public:
    // Thread that runs monitoringWorker
    std::thread MonitoringThread;

    bool LoadStylesheetFile(const std::string&);

    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
};

#endif // MAIN_WINDOW_DLG_HXX