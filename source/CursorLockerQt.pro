QT += core gui widgets

TARGET = CursorLocker
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++17

CONFIG(debug, debug|release): DEFINES += DEBUG
CONFIG(release, debug|release): DEFINES += RELEASE

SOURCES += \
    main.cxx \
    main_window_dlg.cxx

HEADERS += \
    main_window_dlg.hxx \
    json.hxx

FORMS += \
    main_window_dlg.ui

LIBS += \
    -lUser32