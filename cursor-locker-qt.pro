QT += core gui widgets

TARGET = cursor-locker
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS
QMAKE_CXXFLAGS += /std:c++17

CONFIG(debug, debug|release): DEFINES += DEBUG
CONFIG(release, debug|release): DEFINES += RELEASE

SOURCES += \
    source\main.cxx \
    source\main_wnd.cxx

HEADERS += \
    source\main_wnd.hxx \
    source\json.hxx

FORMS += \
    source\main_wnd.ui

LIBS += \
    -lUser32
