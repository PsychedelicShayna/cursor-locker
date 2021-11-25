#ifndef MAIN_WINDOW_DLG_HXX
#define MAIN_WINDOW_DLG_HXX

#undef UNICODE

#include <QMainWindow>
#include <QPushButton>
#include <QMessageBox>
#include <QScrollBar>
#include <QMenu>
#include <QLine>

#include <QTimer>
#include <QPair>

#include <QMouseEvent>
#include <QEvent>

#ifndef WIN32_MEAN_AND_LEAN
#define WIN32_MEAN_AND_LEAN
#endif

#include <Windows.h>
#include <TlHelp32.h>

#include <iostream>
#include <fstream>
#include <string>
#include <tuple>

#include "qdebugconsole_widget.hpp"
#include "process_scanner_dialog.hxx"

#include "json.hxx"
using Json = nlohmann::json;


namespace Ui {
    class MainWindow;
}

enum struct ACTIVATION_METHOD;
enum struct HOTKEY_MOD;

class MainWindow : public QMainWindow {
Q_OBJECT

private:
    Ui::MainWindow*             ui;

    QDebugConsole*              dbgConsole;
    QVBoxLayout*                vblDebugConsoleLayout;

    void                        insertActivationParameterWidget(QWidget*, bool enable=true, bool unhide=true);
    void                        removeActivationParameterWidget(QWidget*, bool disable=true, bool hide=true);

    /* QComboBox / dropdown that gets added dynamically to the left of the activation method parameter
     * text field when the hotkey activation method is selected. Allows a modifier to be selected in
     * conjunction with the hotkey VKID, e.g. Control, Alt, Shift, etc. Its value is stored as HOTKEY_MOD
     * enum value, in the targetHotkeyModifier member variable, whenever the selection changes.
     * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
    QComboBox*                  cbxHotkeyModifier;
    HOTKEY_MOD                  targetHotkeyModifier;  // The modifier associated with the VKID (e.g. control, alt, shift), used for the hotkey activation method.
    const uint32_t              targetHotkeyId;    // The ID used by WinAPI to identify the registered hotkey, used for the hotkey activation method.
    uint8_t                     targetHotkeyVkid;         // The virtual key ID that identifies the target key, used for the hotkey activation method.

    /* This button is dynamically added and removed depending on the activation method. When either
     * the window title or process image activation methods are selected, this button is inserted to
     * the left of the activation method parameter text field by the changeActivationMethod(int) slot,
     * which also connects the button to spawnProcessScannerDialog(ProcessScanner::SCAN_SCOPE) where
     * the SCAN_SCOPE value is bound to WINDOW_MODE or PROCESS_MODE via std::bind, depending on the
     * activation method that was selected when the changeActivationMethod(int) slot was called.
     * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
    ProcessScannerDialog*       processScannerDialog;
    QPushButton*                btnSpawnProcessScanner;
    QMetaObject::Connection     btnSpawnProcessScannerConnection;
    QString                     targetProcessImageName;             // The target process image name, used for the process image activation method.
    QString                     targetForegroundWindowTitle;        // The target window title, used for the window title activation method.

    /* For activation methods that require a timer (window title & image name), the below timer
     * will be used to call the member function associated with the activation method, by having
     * its timeout signal connected to that member function, and that connection will be stored
     * in the below QMetaObject::Connection instance, so that it can later be disconnected.
     * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
    QTimer*                     timedActivationMethodTimer;
    QMetaObject::Connection     timedActivationMethodConnection;

    /* Stores the currently selected activation method as an ENUM value. This can be reliably used
     * all over the class to determine what the current activation method is, for method-specific
     * functionality. It is set whenever the changeActivationMethod(int) SLOT is called, usually
     * as a result of the activation method dropdown's value having been changed.
     * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
    ACTIVATION_METHOD           selectedActivationMethod;

    // Utility Functions
    // ----------------------------------------------------------------------------------------------------
    // Loads a QSS stylesheet from a file, and applies it.
    bool loadStylesheetFile(const std::string&);

    // Allows the sequential passing of multiple frequency/durations to the Windows Beep() function.
    // Callable like beepBoop({{500,20}, {700,20}});
    void beepBoop(QList<QPair<int, int>> freqdur_list);
    // Stops beepBoop from functioning when enabled.
    bool muteBeepBoop;

    RECT getForegroundWindowRect();  // Return the RECT of the current foreground window in Windows.

    void enableCursorLock();    // Enable the cursor lock by calling ClipCursor with getForegroundWindowRect.
    void disableCursorLock();   // Disable the cursor lock by calling ClipCursor(nullptr) with a nullptr.
    bool toggleCursorLock();    // Toggles enableCursorLock/disableCursorLock based on a static bool.

    bool registerTargetHotkey();    // Registers a WinAPI hotkey using targetHotkeyId and targetHotkeyVkid.
    bool unregisterTargetHotkey();  // Unregisters the hotkey previously registered with targetHotkeyId.

    // Implementation of virtual function to handle native Windows thread queue events, namely those sent by RegisterHotKey.
    // If the event type matches a WM_HOTKEY event, then the HotkeyPressed signal is emitted.
    bool nativeEvent(const QByteArray& event_type, void* message, qintptr* result);
    // ----------------------------------------------------------------------------------------------------

signals:
    // Signal emitted by nativeEvent when it receives a registered hotkey pressed native event, and
    // the hotkey's ID (wParam) matches targetHotkeyId.
    void targetHotkeyWasPressed();

private slots:
    // The WinAPI hotkey press event is captured by the nativeEvent() virtual function implementation,
    // and is then re-emitted as a Qt signal which is connected to this function/slot.
    void activateBecauseTargetHotkeyWasPressed();
    // ----------------------------------------------------------------------------------------------------

    // Function that checks whether or not targetProcessImageName is currently running, and activates or
    // deactivates the cursor lock accordingly. Connected to checkActivationMethodTimer's timeout() signal
    // when actively selected as an activation method.
    void activateIfTargetProcessRunning();
    // ----------------------------------------------------------------------------------------------------

    // Function that checks if the current foreground window's title matches the title stored inside of
    // targetForegroundWindowTitle, and activates or deactivates the cursor lock accordingly. Connected to
    // checkActivationMethodTimer's timeout() signal when actively selected as an activation method.
    void activateIfForegroundWindowMatchesTarget();
    // ----------------------------------------------------------------------------------------------------

    // Connected in constructor to cbx_activation_method's CurrentIndexChanged(int) signal.
    void changeActivationMethod(int method_index);

    // Connected in constructor to cbxHotkeyModifier's CurrentIndexChanged(int) signal.
    void changeHotkeyModifier(int modifier_index);

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

    void spawnProcessScannerDialog(ProcessScanner::SCAN_SCOPE);

    // Connected in constructor to btn_mutebeepboop's clicked() signal.
    void toggleMuteBeepBoop();

    // Display the debug console's custom context menu.
    void showConsoleContextMenu(const QPoint& point);

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;
};

template<typename ...LARG>
class LambdaEventFilter : public QObject {
protected:
    struct AbstractLambdaContainer {
        virtual bool CallLambdaFunction(QObject*, QObject*, QEvent*) = 0;
        virtual ~AbstractLambdaContainer() = default;
    } *lambdaContainer;

    template<typename LF>
    class LambdaContainer : public AbstractLambdaContainer {
    protected:
        std::tuple<LARG...> lambdaArguments;
        LF lambdaFunction;

    public:
        bool CallLambdaFunction(QObject* parent, QObject* watched, QEvent* event) override {
            auto calling_arguments = std::tuple_cat(std::make_tuple(parent, watched, event), lambdaArguments);
            return std::apply(lambdaFunction, calling_arguments);
        }

        LambdaContainer(LF lambda_function, LARG... larg) : lambdaFunction(lambda_function) {
            lambdaArguments = std::make_tuple(larg...);
        }
    };

    QObject* parentQObject;

public:
    bool eventFilter(QObject* watched, QEvent* event) override {
        return lambdaContainer->CallLambdaFunction(parentQObject, watched, event);
    }

    template<typename LF>
    LambdaEventFilter(QObject* parent, LF lambda_function, LARG... larg) : parentQObject(parent) {
        lambdaContainer = new LambdaContainer<LF> { lambda_function, larg... };
    }

    ~LambdaEventFilter() override {
        if(lambdaContainer != nullptr) {
            delete lambdaContainer;
        }
    }
};

#endif // MAIN_WINDOW_DLG_HXX




