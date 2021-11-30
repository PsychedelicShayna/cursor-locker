#ifndef QHOTKEYINPUT_HXX
#define QHOTKEYINPUT_HXX

#include <QLineEdit>
#include <QKeyEvent>
#include <QEvent>

#include "winapi_utilities.hpp"

class QHotkeyInput : public QLineEdit {
Q_OBJECT
public:
    struct WindowsHotkey {
        Qt::KeyboardModifiers QtModifiers;
        QList<Qt::Key> QtModifierKeys;
        Qt::Key QtKey;

        quint32 Modifiers;
        quint32 ScanCode;
        quint32 Vkid;

        bool operator==(const WindowsHotkey&) const;
        bool operator!=(const WindowsHotkey&) const;

        QString ToString() const;
        void Clear();
    };

signals:
    void WindowsHotkeyRecorded(QHotkeyInput::WindowsHotkey);

private:
    WindowsHotkey lastWindowsHotkeyEmitted;
    WindowsHotkey windowsHotkey;
    bool mainKeyEstablished;
    bool isRecording;

public slots:
    void StartRecording();
    void StopRecording();

public:
    const bool& IsRecording;
    virtual bool event(QEvent*) override;

    QHotkeyInput(QWidget* = nullptr);
};

#endif // QHOTKEYINPUT_HXX
