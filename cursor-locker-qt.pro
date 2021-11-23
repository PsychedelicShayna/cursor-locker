QT += core gui widgets

TARGET = cursor-locker
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS
QMAKE_CXXFLAGS += /std:c++17 /O2

CONFIG(debug, debug|release): DEFINES += DEBUG
CONFIG(release, debug|release): DEFINES += RELEASE

SOURCES += \
    source/process_scanner.cpp \
    source\main.cxx \
    source\mainwindow_dialog.cxx \
    source\windowbrowser_dialog.cxx \
    source\qdebugconsole_widget.cpp

HEADERS += \
    source/process_scanner.hpp \
    source\mainwindow_dialog.hxx \
    source\windowbrowser_dialog.hxx \
    source\qdebugconsole_widget.hpp \
    source\json.hxx

FORMS += \
    source\mainwindow_dialog.ui \
    source\windowbrowser_dialog.ui

LIBS += \
    -lUser32
