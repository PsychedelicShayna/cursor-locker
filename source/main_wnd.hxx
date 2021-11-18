#ifndef MAIN_WINDOW_DLG_HXX
#define MAIN_WINDOW_DLG_HXX

#undef UNICODE

#include <QMainWindow>
#include <QMessageBox>
#include <QScrollBar>
#include <QTimer>
#include <QMenu>
#include <QPair>

#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <fstream>
#include <string>

#include "json.hxx"

namespace Ui {
    class MainWindow;
}

using Json = nlohmann::json;

enum struct ACTIVATION_METHOD;

enum CONSOLE_LOG_LEVELS {
    CLOG_INFO = 0, CLOG_WARNING = 1, CLOG_ERROR = 2, CLOG_EXCEPTION = 3
};

class MainWindow : public QMainWindow {
private:
    Ui::MainWindow* ui;
    Q_OBJECT

    // Currently selected activation method, which activationConditionChecker will use to determine what condition to check.
    ACTIVATION_METHOD selectedActivationMethod;

    // Utility Functions
    // ----------------------------------------------------------------------------------------------------
    // Loads a QSS stylesheet from a file, and applies it.
    bool loadStylesheetFile(const std::string&);

    // Allows the sequential passing of multiple frequency/durations to the Windows Beep() function.
    // Callable like beepBoop({{500,20}, {700,20}});
    void beepBoop(QList<QPair<int, int>> freqdur_list);
    // Stops beepBoop from functioning when enabled.
    bool muteBeepBoop;

    // displayLogMessagesInConsole will display log messages equal to or higher to this message level.
    CONSOLE_LOG_LEVELS minimumLogLevel;

    QList<QPair<CONSOLE_LOG_LEVELS, QString>> logMessages;
    void displayLogMessagesInConsole(bool just_add_latest=true);

    // Functions to simplify logging messages to the UI's debug console.
    void logToConsole(const QList<QString>&, CONSOLE_LOG_LEVELS loglevel = CLOG_INFO);
    void logToConsole(const char*, CONSOLE_LOG_LEVELS loglevel = CLOG_INFO);

    // Return the RECT of the current foreground window.
    RECT getForegroundWindowRect();

    // Applies ClipCursor to the foreground window RECT
    void enableCursorLock();

    // Calls ClipCursor with nullptr to disable clipping.
    void disableCursorLock();

    // Toggles enableCursorLock/disableCursorLock based on a static bool.
    bool toggleCursorLock();
    // ----------------------------------------------------------------------------------------------------

    // Hotkey Activation Method
    // ----------------------------------------------------------------------------------------------------
    // The ID by which the registered hotkey is refered to in WinAPI, assigned once in constructor.
    const uint32_t targetHotkeyId;

    // The VKID of the registered hotkey, which determines what key needs to be pressed to trigger the activation method.
    uint8_t targetHotkeyVkid;

    // Registers a hotkey in WinAPI using targetHotkeyVkid, identified by targetHotkeyId.
    bool registerTargetHotkey();

    // Unregisters the previously registered hotkey in WinAPI identified by targetHotkeyId
    bool unregisterTargetHotkey();

    // Process Image Activation Method
    // ----------------------------------------------------------------------------------------------------
    // Stores the process image name that should be used by activateIfTargetImagePresent.
    QString targetProcessImageName;

    // Foreground Window Title Activation Method
    // ----------------------------------------------------------------------------------------------------
    // Stores the window title that should be used by activateIfForegroundWindowMatchesTarget.
    QString targetForegroundWindowTitle;

    // Virtual Function Implementations
    // ----------------------------------------------------------------------------------------------------
    // Override of QMainWindow's closeEvent, to ensure the cursor doesn't remain caged when the window closes.
    void closeEvent(QCloseEvent*);

    // Implementation of virtual function to handle native Windows thread queue events, namely those sent by RegisterHotKey.
    // If the event type matches a WM_HOTKEY event, then the HotkeyPressed signal is emitted.
    bool nativeEvent(const QByteArray& event_type, void* message, long* result);
    // ----------------------------------------------------------------------------------------------------

signals:
    // Signal emitted by nativeEvent when it receives a registered hotkey pressed native event, and
    // the hotkey's ID (wParam) matches targetHotkeyId.
    void targetHotkeyVkidPressedSignal();

private slots:
    // The WinAPI hotkey press event is captured by the nativeEvent() virtual function implementation,
    // and is then re-emitted as a Qt signal which is connected to this function/slot.
    void targetHotkeyVkidPressedSlot();
    // ----------------------------------------------------------------------------------------------------

    // Function that checks whether or not targetProcessImageName is currently running, and activates or
    // deactivates the cursor lock accordingly. Connected to checkActivationMethodTimer's timeout() signal
    // when actively selected as an activation method.
    void activateIfTargetImagePresent();
    // ----------------------------------------------------------------------------------------------------

    // Function that checks if the current foreground window's title matches the title stored inside of
    // targetForegroundWindowTitle, and activates or deactivates the cursor lock accordingly. Connected to
    // checkActivationMethodTimer's timeout() signal when actively selected as an activation method.
    void activateIfForegroundWindowMatchesTarget();
    // ----------------------------------------------------------------------------------------------------

    // Connected in constructor to cbx_activation_method's CurrentIndexChanged(int) signal.
    void changeActivationMethod(int method_index);

    // Connected in constructor to btn_edit_activation_parameter's clicked() signal.
    void editActivationMethodParameter();

    /* Starts the foreground window grabber, which performs a maximum of 15 checks spaced 500ms apart, to see if the current foreground window,
     * as returned by WinAPI, differs from the program's foreground window, indicating that another window was selected. If another window has
     * been selected, then the timer performing these checks is reset, and the target foreground window is set to the selected foreground window.
     * Equally, if the foreground window stays the same after 15 checks have been performed, the timer is also reset, and no changes are made.
     * This function only works if the current activation method is set to Window title, otherwise any call made to it will be ignored.
     * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
    void startForegroundWindowGrabber(); //////
    // - - - - - - - - - - - - - - - - - - ///

    // Connected in constructor to btn_mutebeepboop's clicked() signal.
    void toggleMuteBeepBoop();

    // Display the debug console's custom context menu.
    void showConsoleContextMenu(const QPoint& point);

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
};

#endif // MAIN_WINDOW_DLG_HXX
