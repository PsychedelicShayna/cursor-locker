#include <QtWidgets/QApplication>
#include <iostream>

#include "main_window_dialog.hxx"
#include "debugging.hpp"

int main(int argc, char* argv[]) {
    for(int i { 0 }; i < argc; ++i) {
        const char* argument { argv[i] };

        if(!_stricmp(argument, "--debug")) {
            Debugging::SpawnDebugConsole();
        }
    }

    qInstallMessageHandler(Debugging::DebugMessageHandler);

    QApplication application(argc, argv);
    MainWindowDialog main_window_dialog;
    main_window_dialog.show();

    return application.exec();
}
