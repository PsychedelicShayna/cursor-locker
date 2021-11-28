#ifndef QHOTKEYINPUT_HXX
#define QHOTKEYINPUT_HXX

#include <QLineEdit>
#include <QKeyEvent>
#include <QEvent>

class QHotkeyInput : public QLineEdit {
Q_OBJECT
public:
    struct WindowsHotkey {
        enum Modifier {
            WINMOD_ALT        = 0x01,
            WINMOD_CONTROL    = 0x02,
            WINMOD_SHIFT      = 0x04,
            WINMOD_WIN        = 0x08
        };

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
    void WindowsHotkeyRecorded(WindowsHotkey);

private:
    WindowsHotkey lastWindowsHotkeyEmitted;
    WindowsHotkey windowsHotkey;
    bool mainKeyEstablished;
    bool isRecording;

    quint32 convertQtKeyToWindowsModifier(const Qt::Key&) const;

public slots:
    void StartRecording();
    void StopRecording();

public:
    const bool& IsRecording;
    virtual bool event(QEvent*) override;

    QHotkeyInput(QWidget* = nullptr);
};

#endif // QHOTKEYINPUT_HXX
