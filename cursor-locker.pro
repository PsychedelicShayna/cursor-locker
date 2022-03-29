QT += core gui widgets multimedia

TARGET = cursor-locker
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS QT_MESSAGELOGCONTEXT
QMAKE_CXXFLAGS += /std:c++17 /O2

CONFIG(debug, debug|release): DEFINES += DEBUG
CONFIG(release, debug|release): DEFINES += RELEASE

# Include qt-hotkey-recorder-widget
INCLUDEPATH += qt-hotkey-recorder-widget/source/
SOURCES     += qt-hotkey-recorder-widget/source/hotkey_recorder_widget.cpp
HEADERS     += qt-hotkey-recorder-widget/source/hotkey_recorder_widget.hpp

# Include qt-vkid-table-widget
INCLUDEPATH += qt-vkid-table-widget/source/
SOURCES     += qt-vkid-table-widget/source/vkid_table_widget.cpp
HEADERS     += qt-vkid-table-widget/source/vkid_table_widget.hpp

SOURCES +=                                      \
    source/debugging.cpp                        \
    source/keyboard_modifier_list_widget.cpp    \
    source/main.cxx                             \
    source/main_window_dialog.cxx               \
    source/process_scanner_dialog.cxx           \
    source/json_settings_dialog.cxx             \
    source/process_scanner.cpp                  \
    source/vkid_table_widget_dialog.cxx

HEADERS +=                                      \
    source/anonymous_event_filter.hpp           \
    source/debugging.hpp                        \
    source/keyboard_modifier_list_widget.hpp    \
    source/main_window_dialog.hxx               \
    source/process_scanner_dialog.hxx           \
    source/json_settings_dialog.hxx             \
    source/process_scanner.hpp                  \
    source/vkid_table_widget_dialog.hxx

FORMS +=                                \
    source/main_window_dialog.ui        \
    source/process_scanner_dialog.ui    \
    source/json_settings_dialog.ui      \
    source/vkid_table_widget_dialog.ui

LIBS += \
    -lUser32

RESOURCES += \
    resources.qrc
