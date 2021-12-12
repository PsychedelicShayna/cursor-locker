QT += core gui widgets multimedia

TARGET = cursor-locker
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS
QMAKE_CXXFLAGS += /std:c++17 /O2

CONFIG(debug, debug|release): DEFINES += DEBUG
CONFIG(release, debug|release): DEFINES += RELEASE

SOURCES +=                              \
    source/keyboard_modifier_list_widget.cpp \
    source/main.cxx                     \
    source/main_window_dialog.cxx       \
    source/process_scanner_dialog.cxx   \
    source/json_settings_dialog.cxx     \
    source/debug_console_widget.cpp     \
    source/hotkey_input_widget.cpp      \
    source/process_scanner.cpp          \
    source/vkid_table_dialog.cxx \
    source/winapi_utilities.cpp

HEADERS +=                              \
    source/anonymous_event_filter.hpp \
    source/keyboard_modifier_list_widget.hpp \
    source/main_window_dialog.hxx       \
    source/process_scanner_dialog.hxx   \
    source/json_settings_dialog.hxx     \
    source/debug_console_widget.hpp     \
    source/hotkey_input_widget.hpp      \
    source/process_scanner.hpp          \
    source/vkid_table_dialog.hxx        \
    source/winapi_utilities.hpp

FORMS +=                                \
    source/main_window_dialog.ui        \
    source/process_scanner_dialog.ui    \
    source/json_settings_dialog.ui      \
    source/vkid_table_dialog.ui

LIBS += \
    -lUser32

RESOURCES += \
    resources.qrc
