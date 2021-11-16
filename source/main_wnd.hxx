#ifndef MAIN_WINDOW_DLG_HXX
#define MAIN_WINDOW_DLG_HXX

#undef UNICODE

#include <QMainWindow>
#include <QMessageBox>
#include <QTimer>

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

    bool muteBeepBoop;
    void BeepBoop(QList<QPair<int, int>> freqdur_list);

    // Currently selected activation method, which activationConditionChecker will use to determine what condition to check.
    MONITOR_FOR selectedActivationCondition;

    RECT getForegroundWindowRect();
    void enableCursorLock();
    void disableCursorLock();
    bool toggleCursorLock();

    uint32_t targetHotkeyId;
    uint8_t targetHotkeyVkid;
    bool registerTargetHotkey();
    bool unregisterTargetHotkey();
    void targetHotkeyVkidPressedSlot();

    QString targetProcessImageName;
    void activateIfTargetImagePresent();

    QString targetForegroundWindowTitle;
    void activateIfForegroundWindowMatchesTarget();

    // Points to the member function that the timeout() signal from the below timer is connected to.
    void(MainWindow::*selectedActivationMethodFunction)();
    // Timer whose timeout() signal periodically calls the selected activation method checker function.
    // Only applies to activationMethodSlot
    QTimer* checkActivationMethodTimer;

    // Checks the state of the activation condition, and activates or deactivates the cursor lock accordingly.
    void activationConditionChecker();


    void logToConsole(const QList<QString>&);
    void logToConsole(const char*);

    // Override of QMainWindow's closeEvent, to ensure the cursor doesn't remain caged when the window closes.
    void closeEvent(QCloseEvent*);

    // Implementation of virtual function to handle native Windows thread queue events, namely those sent by RegisterHotKey.
    // If the event type matches a WM_HOTKEY event, then the HotkeyPressed signal is emitted.
    bool nativeEvent(const QByteArray& event_type, void* message, long* result);

signals:
    void targetHotkeyVkidPressedSignal();

private slots:
    void on_cbx_activation_method_currentIndexChanged(int);
    void on_btn_edit_activation_parameter_clicked();
    void on_btn_edit_activation_parameter_right_clicked();
    void on_btn_mutebeepboop_clicked();

private:
    // OLD CODE BELOW THIS LINE
    // ----------------------------------------------------------------------------------------------------

/*
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

    // The target of AcquireWindowThread. Monitors foreground window changes, activated by right clicking the edit activation parameter button.
    std::atomic<bool> acquireWindowThreadSignal;
    [[noreturn]] void acquireWindowWorker();

    // Bool which mutes the beeping and booping of the program when its state changes.
    std::atomic<bool> muteBeepBoop;

    std::mutex consoleMutex; // Multiple threads may log to the console, therefore a mutex is used to avoid data races.
    */



public:
    // Thread that runs monitoringWorker.
    // std::thread MonitoringThread;

    // Thread that runs acquireWindowWorker.
    // std::thread AcquireWindowThread;



    bool LoadStylesheetFile(const std::string&);

    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
};

#endif // MAIN_WINDOW_DLG_HXX
