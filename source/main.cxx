#include <QApplication>

#include "main_wnd.hxx"

int main(int argc, char* argv[]) {
    QApplication application(argc, argv);
    MainWindow main_window;
    main_window.show();

    return application.exec();
}
