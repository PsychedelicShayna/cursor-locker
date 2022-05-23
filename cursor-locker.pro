QT += core gui widgets multimedia

TARGET = cursor-locker
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS QT_MESSAGELOGCONTEXT
QMAKE_CXXFLAGS += /std:c++17 /O2

CONFIG(debug, debug|release): DEFINES += DEBUG
CONFIG(release, debug|release): DEFINES += RELEASE

# SUBMODULE: Qt Process Scanner Widget
# ==================================================
INCLUDEPATH += submodules/qt-process-scanner-widget/

SOURCES += \
    submodules/qt-process-scanner-widget/process_scanner.cpp \
    submodules/qt-process-scanner-widget/process_scanner_dialog.cxx

HEADERS += \
    submodules/qt-process-scanner-widget/process_scanner.hpp \
    submodules/qt-process-scanner-widget/process_scanner_dialog.hxx

FORMS += \
    submodules/qt-process-scanner-widget/process_scanner_dialog.ui
# ==================================================

# SUBMODULE: Qt Hotkey Recorder Widget
# ==================================================
INCLUDEPATH += submodules/qt-hotkey-recorder-widget/
SOURCES     += submodules/qt-hotkey-recorder-widget/hotkey_recorder_widget.cpp
HEADERS     += submodules/qt-hotkey-recorder-widget/hotkey_recorder_widget.hpp
# ==================================================


# SUBMODULE: Qt Vkid Table Widget
# ==================================================
INCLUDEPATH += submodules/qt-vkid-table-widget/
SOURCES     += submodules/qt-vkid-table-widget/vkid_table_widget.cpp
HEADERS     += submodules/qt-vkid-table-widget/vkid_table_widget.hpp
# ==================================================


SOURCES += \
    source/main.cpp \
    source/debugging.cpp \
    source/keyboard_modifier_list_widget.cpp \
    source/main_window_dialog.cxx \
#    source/process_scanner.cpp \
#    source/process_scanner_dialog.cxx \
    source/json_settings_dialog.cxx \
    source/vkid_table_widget_dialog.cxx

HEADERS += \
    source/anonymous_event_filter.hpp \
    source/debugging.hpp \
    source/keyboard_modifier_list_widget.hpp \
    source/main_window_dialog.hxx \
    source/json_settings_dialog.hxx \
#    source/process_scanner.hpp \
#    source/process_scanner_dialog.hxx \
    source/vkid_table_widget_dialog.hxx

FORMS += \
    source/main_window_dialog.ui \
#    source/process_scanner_dialog.ui \
    source/json_settings_dialog.ui \
    source/vkid_table_widget_dialog.ui

LIBS += \
    -lUser32

RESOURCES += \
    resources/resources.qrc
