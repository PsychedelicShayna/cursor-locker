#include <QtWidgets/QApplication>
#include "main_window_dialog.hxx"

int main(int argc, char* argv[]) {
    QApplication application(argc, argv);
    MainWindowDialog main_window_dialog;
    main_window_dialog.show();
    return application.exec();
}
