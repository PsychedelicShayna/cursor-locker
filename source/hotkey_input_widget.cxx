#include "hotkey_input_widget.hxx"

bool QHotkeyInput::WindowsHotkey::operator==(const WindowsHotkey& other) const {
    return     QtModifiers      == other.QtModifiers
            && QtModifierKeys   == other.QtModifierKeys
            && QtKey            == other.QtKey
            && Modifiers        == other.Modifiers
            && ScanCode         == other.ScanCode
            && Vkid             == other.Vkid;
}

bool QHotkeyInput::WindowsHotkey::operator!=(const WindowsHotkey& other) const {
    return !(*this==other);
}

QString QHotkeyInput::WindowsHotkey::ToString() const {
    QString sequence_string;

    if(Modifiers & WINMOD_WIN)     sequence_string += "Win + ";
    if(Modifiers & WINMOD_CONTROL) sequence_string += "Control + ";
    if(Modifiers & WINMOD_SHIFT)   sequence_string += "Shift + ";
    if(Modifiers & WINMOD_ALT)     sequence_string += "Alt + ";

    if(Vkid) {
        sequence_string += QKeySequence { QtKey }.toString();
    }

    return sequence_string;
}

void QHotkeyInput::WindowsHotkey::Clear() {
    QtModifierKeys.clear();

    QtModifiers = Qt::KeyboardModifier::NoModifier;
    QtKey = static_cast<Qt::Key>(NULL);

    Modifiers    = NULL;
    ScanCode     = NULL;
    Vkid         = NULL;
}

quint32 QHotkeyInput::convertQtKeyToWindowsModifier(const Qt::Key& qtkey) const {
    static const QMap<Qt::Key, quint32> converter {
        { Qt::Key_Control,    WindowsHotkey::WINMOD_CONTROL },
        { Qt::Key_Shift,      WindowsHotkey::WINMOD_SHIFT   },
        { Qt::Key_Alt,        WindowsHotkey::WINMOD_ALT     },
        { Qt::Key_Meta,       WindowsHotkey::WINMOD_WIN     }
    };

    if(converter.contains(qtkey)) {
        return converter[qtkey];
    }

    return NULL;
}

bool QHotkeyInput::event(QEvent* event) {
    const auto& event_type { event->type() };

    if(event_type != QEvent::KeyPress && event_type != QEvent::KeyRelease) {
        return QLineEdit::event(event);
    }

    if(!isRecording) return false;

    QKeyEvent*        key_event           { reinterpret_cast<QKeyEvent*>(event)    };
    const Qt::Key&    qtkey               { static_cast<Qt::Key>(key_event->key()) };
    const quint32&    windows_modifier    { convertQtKeyToWindowsModifier(qtkey)   };

    if(event_type == QEvent::KeyRelease) {
        if(windowsHotkey.Vkid && !windows_modifier && windowsHotkey != lastWindowsHotkeyEmitted) {
            emit WindowsHotkeyRecorded(windowsHotkey);
            lastWindowsHotkeyEmitted = windowsHotkey;
            mainKeyEstablished = true;
        } else if(!windowsHotkey.Vkid && windows_modifier) {
            windowsHotkey.Modifiers &= ~windows_modifier;

            if(!windowsHotkey.Modifiers) {
                windowsHotkey.Clear();
            }

            setText(windowsHotkey.ToString());
        }
    }

    else if(event_type == QEvent::KeyPress && !key_event->isAutoRepeat() && key_event->key()) {
        if(mainKeyEstablished && windows_modifier) {
            windowsHotkey.Clear();
            mainKeyEstablished = false;
        }

        if(windows_modifier) {
            windowsHotkey.Modifiers |= windows_modifier;
            windowsHotkey.QtModifierKeys.append(qtkey);
        } else {
            windowsHotkey.Vkid        = key_event->nativeVirtualKey();
            windowsHotkey.ScanCode    = key_event->nativeScanCode();
            windowsHotkey.QtKey       = qtkey;
        }

        setText(windowsHotkey.ToString());
    }

    event->setAccepted(true);
    return true;
}

void QHotkeyInput::StartRecording() {
    if(!isRecording) {
        isRecording = true;
    }
}

void QHotkeyInput::StopRecording() {
    if(isRecording) {
        isRecording = false;
        windowsHotkey.Clear();
    }
}

QHotkeyInput::QHotkeyInput(QWidget* parent)
    :
      QLineEdit             { parent      },
      mainKeyEstablished    { false       },
      isRecording           { false       },
      IsRecording           { isRecording }
{

}
