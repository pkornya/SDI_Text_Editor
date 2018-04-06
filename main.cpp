#include "sdiwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    SdiWindow *window = new SdiWindow();
    window->show();

    return app.exec();
}
