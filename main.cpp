#include <QtGlobal>

#include "mainwindow.h"
#include "dlaraoke.h"

int main(int argc, char *argv[])
{
    qDebug("main(): Start");
    DLaraoke a(argc, argv);
    MainWindow w;
    w.showMaximized();

    return a.exec();
}
