#ifndef MAIN_WINDOW_DLG_HXX
#define MAIN_WINDOW_DLG_HXX

#undef UNICODE

#include <QMainWindow>
#include <QPushButton>
#include <QMessageBox>
#include <QScrollBar>
#include <QMenu>
#include <QLine>

#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>
#include <QFile>

#include <QStandardItemModel>
#include <QStandardItem>

#include <QTimer>
#include <QPair>

#include <QMouseEvent>
#include <QEvent>

#include <QtMultimedia/QSoundEffect>

#ifndef WIN32_MEAN_AND_LEAN
#define WIN32_MEAN_AND_LEAN
#endif

#include <Windows.h>
#include <TlHelp32.h>

#include <iostream>
#include <fstream>
#include <string>
#include <tuple>


#include "process_scanner_dialog.hxx"
#include "json_settings_dialog.hxx"
#include "vkid_table_dialog.hxx"

#include "winapi_utilities.hpp"
#include "debug_console_widget.hpp"
#include "hotkey_input_widget.hpp"
#include "check_box_list_widget.hpp"

namespace Ui {
    class MainWindow;
}

enum struct ACTIVATION_METHOD;

class MainWindow : public QMainWindow {
Q_OBJECT
protected:
    Ui::MainWindow* ui;

    QDebugConsole*  dbgConsole;               // The QDebugConsole instance shown at the bottom of the GUI, used for logging.
    QVBoxLayout*    vblDebugConsoleLayout;    // Layout that dbgConsole is contained in, set in the constructor.

    QMenu*      dbgConsoleContextMenu;    // The console's context menu QMenu that gets presented when showConsoleContextMenu is called.
    QAction*    dbgCCMActionClear;
    QMenu*      dbgCCMSubMenuLogLevels;



    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     * Stores the currently selected activation method as an ENUM value. This can be reliably used
     * all over the class to determine what the current activation method is, for method-specific
     * functionality. It is set whenever the changeActivationMethod(int) SLOT is called, usually
     * as a result of the activation method dropdown's value having been changed.
     * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
    ACTIVATION_METHOD selectedActivationMethod;

    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     * Helper functions to insert or remove any widget from the activation parameter layout defined
     * in the UI file, which contains the activation parameter text field. Used to insert widgets
     * like cbxHotkeyModifier, and btnSpawnProcessScanner depending on selected activation method.
     * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
    void insertActivationParameterWidget(QWidget*, bool enable=true, bool unhide=true);
    void removeActivationParameterWidget(QWidget*, bool disable=true, bool hide=true);


    // Keyboard Modifier Dropdown
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    QKbModifierList*    ampwHotkeyModifierDropdown;
    Q_SLOT void         handleModifierListBitmaskChanged(const quint32& bitmask);


    // Hotkey Input / Recorder
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    QHotkeyInput*       ampwHotkeyRecorder;             // Instance of QHotkeyInput that records hotkeys entered into it, and updates the relevant UI components with the recorded hotkey information.
    Q_SLOT void         updateUiWithRecordedWindowsHotkey(QHotkeyInput::WindowsHotkey);
    Q_SLOT void         updateHotkeyInputWithNewModifierBitmask(const quint32& bitmask);


    // VKID Table Dialog
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    VkidTableDialog*    vkidTableDialog;
    QPushButton*        btnSpawnVkidTableDialog;
    Q_SLOT void         spawnVkidTableDialog();


    // Hotkey Activation Method
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    quint32        ampHotkeyModifiersBitmask;                  // Bitmask that stores the WinApi modifier key bitmask values that will be used to register the hotkey.
    quint32        ampHotkeyVkid;                              // The virtual key ID that identifies the target key that will be used to register the hotkey.
    quint32        ampHotkeyId;                                // The ID used by WinApi to identify the registered hotkey, which can later be used to unregister it.

    QString        setAmpHotkeyVkid(const quint32&);           // Changes the VKID that will be used for the hotkey to another value.
    QString        setAmpHotkeyVkid(const QString&);           // Overload that converts the QString into a quint32 before passing it to the primary overload.

    bool           registerAmpHotkey();                        // Registers a WinAPI hotkey using ampHotkeyId and ampHotkeyVkid.
    bool           unregisterAmpHotkey();                      // Unregisters the hotkey previously registered with ampHotkeyId.

    void           setAmToHotkey();                            // Set the activation method to hotkey.
    void           unsetAmToHotkey();                          // Unset the hotkey activation method.

    Q_SLOT void    activateBecauseTargetHotkeyWasPressed();


    // Process Scanner Dialog
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    ProcessScannerDialog*      processScannerDialog;                // The ProcessScannerDialog instance pointer that spawnProcessScannerDialog manages the construction and destruction of.
    QPushButton*               btnSpawnProcessScanner;              // Button connected to spawnProcessScannerDialog; allows for the selection of window titles and process images from a dialog.
    QMetaObject::Connection    btnSpawnProcessScannerConnection;    // The connection between btnSpawnProcessScanner and spawnProcessScanerDialog, so that it may later be disconnected.

    Q_SLOT void                spawnProcessScannerDialog(ProcessScanner::SCAN_SCOPE);


    // Timer For Timed Activation Methods (Process Image Name & Foreground Window Title)
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    QTimer*                    timedActivationMethodTimer;         // Timer that executes the activation method slot that it's connected to, when applicable.
    QMetaObject::Connection    timedActivationMethodConnection;    // Stores the connection between timedActivationMethodTimer's timeout signal, and the activation method slot.


    // Process Image Activation Method
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    QString        amParamProcessImageName;                   // The process image name that will be used for the process image name activation method.
    void           setAmpProcessImageName(const QString&);    // Changes the process image name activation method parameter to a new value.

    void           setAmToProcessImageName();                 // Sets the activation method for the cursor lock to process image name mode.
    void           unsetAmToProcessImageName();               // Unsets the activation method from process image name mode.

    Q_SLOT void    activateIfTargetProcessRunning();


    // Foreground Window Activation Method
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    QString        amParamForegroundWindowTitle;                   // The window title that will be used for the window title activation method.
    void           setAmpForegroundWindowTitle(const QString&);    // Changes the foreground window title activation method parameter to a new value.

    void           setAmToForegroundWindowTitle();                 // Sets the activation method for the cursor lock to window title mode.
    void           unsetAmToForegroundWindowTitle();               // Unsets the activation method from foreground window title.

    Q_SLOT void    activateIfForegroundWindowMatchesTarget();


    // Foreground Window Grabber
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    const quint32    windowGrabberTimerMaxTimeouts;
    quint32          windowGrabberTimerTimeoutCounter;
    QTimer*          windowGrabberTimer;
    QPushButton*     btnStartWindowGrabber;

    Q_SLOT void      onWindowGrabberTimerTimeout();
    Q_SLOT void      onWindowGrabberButtonClicked();


    // JSON Settings Dialog
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    QString                jsonConfigFilePath;
    JsonSettingsDialog*    jsonSettingsDialog;
    Q_SLOT void            spawnJsonSettingsDialog();


    // Sound Effects
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    QSoundEffect    seLockActivated, seLockDeactivated;      // Sound effects that play when the cursor lock is activated or deactivated.
    QSoundEffect    seWindowGrabbed, seWindowGrabberTick;    // Sound effect that's played when a new foreground window is grabbed, or the window grabber times out.
    bool            soundEffectsMuted;                               // Stores the mute toggle state of the above sound effects.

    Q_SLOT void     setSoundEffectsMutedState(bool);
    Q_SLOT void     toggleSoundEffectsMuted();


    // Cursor Lock
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    bool lastCursorLockStateSet;
    void setCursorLockEnabled(const bool&);
    bool toggleCursorLockState();


    // Implementation of virtual function to handle native Windows thread queue events, namely those sent by RegisterHotKey.
    // If the event type matches a WM_HOTKEY event, then the HotkeyPressed signal is emitted.
    bool nativeEvent(const QByteArray& event_type, void* message, qintptr* result) override;
    // ----------------------------------------------------------------------------------------------------

    const QString styleSheetFilePath;

signals:
    // Signal emitted by nativeEvent when it receives a registered hotkey pressed native event, and
    // the hotkey's ID (wParam) matches amParamHotkeyId.
    void targetHotkeyWasPressed();

protected slots:
    void changeActivationMethod(int method_index);     // Connected in constructor to cbx_activation_method's CurrentIndexChanged(int) signal.
    bool changeActivationMethod(const QString&);     // Overload that maps QString to activation method index and calls changeActivationMethod(int) overload.

    void editActivationMethodParameter();     // Connected in constructor to btn_edit_activation_parameter's clicked() signal.

    /* Starts the foreground window grabber, which performs a maximum of 15 checks spaced 500ms apart, to see if the current foreground window,
     * as returned by WinAPI, differs from the program's foreground window, indicating that another window was selected. If another window has
     * been selected, then the timer performing these checks is reset, and the target foreground window is set to the selected foreground window.
     * Equally, if the foreground window stays the same after 15 checks have been performed, the timer is also reset, and no changes are made.
     * This function only works if the current activation method is set to Window title, otherwise any call made to it will be ignored.
     * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
    // void foregroundWindowGrabberTimerTimeout();


public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;
};


#endif // MAIN_WINDOW_DLG_HXX




