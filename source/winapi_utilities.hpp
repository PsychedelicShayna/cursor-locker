#ifndef WINAPI_UTILITIES_HPP
#define WINAPI_UTILITIES_HPP

#include <QString>
#include <QMap>

enum WINAPI_MODIFIER {
    WINMOD_NULLMOD    = NULL,
    WINMOD_ALT        = 0x01,
    WINMOD_CONTROL    = 0x02,
    WINMOD_SHIFT      = 0x04,
    WINMOD_WIN        = 0x08
};

// From QString conversions.
WINAPI_MODIFIER QStringToWinApiKbModifier(const QString&);
WINAPI_MODIFIER QtKeyToWinApiKbModifier(const Qt::Key&);

// To QString conversions.
QString WinApiKbModifierToQString(const WINAPI_MODIFIER& modifier, const bool& capitalize = false);
QString WinApiKbModifierBitmaskToQString(const quint32& modifiers, const bool& capitalize = false);

#endif // WINAPI_UTILITIES_HPP
