#include "winapi_utilities.hpp"

WINAPI_MODIFIER QStringToWinApiKbModifier(const QString& modifier_string) {
    static const QMap<QString, WINAPI_MODIFIER>& conversion_table {
        { "ALT",     WINMOD_ALT     },
        { "CTRL",    WINMOD_CONTROL },
        { "CONTROL", WINMOD_CONTROL },
        { "SHIFT",   WINMOD_SHIFT   },
        { "SHFT",    WINMOD_SHIFT   },
        { "WIN",     WINMOD_WIN     }
    };

    return conversion_table.contains(modifier_string) ? conversion_table[modifier_string] : WINMOD_NULLMOD;
}

WINAPI_MODIFIER QtKeyToWinApiKbModifier(const Qt::Key& qtkey) {
    static const QMap<Qt::Key, WINAPI_MODIFIER>& conversion_table {
        { Qt::Key_Alt,        WINMOD_ALT     },
        { Qt::Key_Control,    WINMOD_CONTROL },
        { Qt::Key_Shift,      WINMOD_SHIFT   },
        { Qt::Key_Meta,       WINMOD_WIN     }
    };

    return conversion_table.contains(qtkey) ? conversion_table[qtkey] : WINMOD_NULLMOD;
}

QString WinApiKbModifierToQString(const WINAPI_MODIFIER& winapi_modifier, const bool& capitalized) {
    static const QMap<WINAPI_MODIFIER, QString>& conversion_table {
        { WINMOD_CONTROL,    "Control" },
        { WINMOD_SHIFT,      "Shift"   },
        { WINMOD_ALT,        "Alt"     },
        { WINMOD_WIN,        "Win"     }
    };

    QString converted_string {
        conversion_table.contains(winapi_modifier) ? conversion_table[winapi_modifier] : ""
    };

    if(capitalized && converted_string.size()) {
        for(QChar& character : converted_string) {
            character = character.toUpper();
        }
    }

    return converted_string;
}

QString WinApiKbModifierBitmaskToQString(const quint32& winapi_modifier, const bool& capitalized) {
    QString string_sequence;

    if(winapi_modifier & WINMOD_CONTROL)
        string_sequence += WinApiKbModifierToQString(WINMOD_CONTROL, capitalized) + " + ";

    if(winapi_modifier & WINMOD_SHIFT)
        string_sequence += WinApiKbModifierToQString(WINMOD_SHIFT, capitalized) + " + ";

    if(winapi_modifier & WINMOD_ALT)
        string_sequence += WinApiKbModifierToQString(WINMOD_ALT, capitalized) + " + ";

    if(winapi_modifier & WINMOD_WIN)
        string_sequence += WinApiKbModifierToQString(WINMOD_WIN, capitalized) + " + ";

    return string_sequence;
}


