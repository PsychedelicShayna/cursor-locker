#ifndef MAIN_WINDOW_DLG_HXX
#define MAIN_WINDOW_DLG_HXX

#ifndef _UNICODE
#define _UNICODE
#endif

#ifndef UNICODE
#define UNICODE
#endif

#ifndef WIN32_MEAN_AND_LEAN
#define WIN32_MEAN_AND_LEAN
#endif

#include <Windows.h>

#include <QtGui/QMouseEvent>
#include <QtCore/QEvent>

#include <QtCore/QTimer>
#include <QtCore/QPair>

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

#include <QtCore/QFileInfo>
#include <QtCore/QFile>

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QMenu>

#include <QtMultimedia/QSoundEffect>
#include <QtCore/QResource>

#include <hotkey_recorder_widget.hpp>
#include <vkid_table_widget.hpp>

#include "process_scanner_dialog.hxx"
#include "json_settings_dialog.hxx"
#include "vkid_table_widget_dialog.hxx"

#include "keyboard_modifier_list_widget.hpp"

#include "anonymous_event_filter.hpp"

namespace Ui {
    class MainWindowDialog;
}

enum struct ACTIVATION_METHOD;

class MainWindowDialog : public QMainWindow {
Q_OBJECT
protected:
    Ui::MainWindowDialog* ui;

    const QString styleSheetFilePath;

    Q_SLOT qsizetype loadQssStylesheet(const QString&, QByteArray& out_bytes) const;
    Q_SLOT qsizetype loadAndApplyQssStylesheet(const QString&);
    Q_SLOT qsizetype loadAndApplyQssStylesheet();

    // Activation Method
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    ACTIVATION_METHOD selectedActivationMethod;

    void insertActivationParameterWidget(QWidget*, bool enable=true, bool unhide=true);
    void removeActivationParameterWidget(QWidget*, bool disable=true, bool hide=true);

    Q_SLOT void changeActivationMethod(int method_index);    // Connected in constructor to cbx_activation_method's CurrentIndexChanged(int) signal.
    Q_SLOT bool changeActivationMethod(const QString&);      // Overload that maps QString to activation method index and calls changeActivationMethod(int) overload.

    Q_SLOT void editActivationMethodParameter();    // Connected in constructor to btn_edit_activation_parameter's clicked() signal.
    Q_SLOT void clearActivationMethodParameter();

    // Keyboard Modifier Dropdown
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    QKbModifierList*    ampwHotkeyModifierDropdown;
    Q_SLOT void         handleModifierListBitmaskChanged(const quint32& bitmask);


    // Hotkey Input / Recorder
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    HotkeyRecorderWidget*     ampwHotkeyRecorder;    // Instance of HotkeyRecorderWidget that records hotkeys entered into it, and updates the relevant UI components with the recorded hotkey information.
    Q_SLOT void               updateUiWithRecordedHotkey(HotkeyRecorderWidget::Hotkey);
    Q_SLOT void               updateHotkeyInputWithNewModifierBitmask(const quint32& bitmask);


    // VKID Table Dialog
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    VkidTableWidgetDialog*    vkidTableWidgetDialog;
    QPushButton*              btnSpawnVkidTableWidgetDialog;
    Q_SLOT void               spawnVkidTableWidgetDialog();


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
    bool           processHasBeenFound;
    QString        amParamProcessImageName;                   // The process image name that will be used for the process image name activation method.
    void           setAmpProcessImageName(const QString&);    // Changes the process image name activation method parameter to a new value.

    void           setAmToProcessImageName();                 // Sets the activation method for the cursor lock to process image name mode.
    void           unsetAmToProcessImageName();               // Unsets the activation method from process image name mode.

    Q_SLOT void    activateIfTargetProcessRunning();


    // Foreground Window Activation Method
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    bool           windowTitleHasBeenFound;
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
    Q_SLOT void            loadAndApplyJsonSettings();


    // Sound Effects
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    QSoundEffect    seLockActivated, seLockDeactivated;      // Sound effects that play when the cursor lock is activated or deactivated.
    QSoundEffect    seWindowGrabberTick;                     // Sound effect that's played when a new foreground window is grabbed, or the window grabber times out.
    bool            soundEffectsMuted;                       // Stores the mute toggle state of the above sound effects.

    Q_SLOT void     setSoundEffectsMutedState(bool);
    Q_SLOT void     toggleSoundEffectsMuted();


    // Cursor Lock
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    bool lastCursorLockStateSet;
    void setCursorLockEnabled(const bool&);
    bool toggleCursorLockState();


    // Override of nativeEvent in order to handle Windows message queue events, namely those sent when a hotkey
    // that was previously registered using RegisterHotKey() was pressed. targetHotkeyWasPressed signal is emitted
    // if the event type is WM_HOTKEY, and the hotkey ID and VKID match those used to register the hotkey.
    Q_SIGNAL void targetHotkeyWasPressed();
    virtual bool nativeEvent(const QByteArray& event_type, void* message, qintptr* result) override;


    // Override of mousePressEvent, used to clear focus from input widgets when a widgetless area of the dialog is clicked.
    virtual void mousePressEvent(QMouseEvent* mouse_press_event) override;
    // ----------------------------------------------------------------------------------------------------


public:
    explicit MainWindowDialog(QWidget* parent = nullptr);
    virtual ~MainWindowDialog() override;
};


#endif // MAIN_WINDOW_DLG_HXX




