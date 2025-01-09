#include "mainwindow.h"

#include <QApplication>
#include <QTranslator>
#include <QResource>

#include <QString>
#include <QList>
#include "Utility/config.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    QList<QString> args;
    for (int i = 1; i < argc; ++i)
    {
        args.append(argv[i]);
    }

    if (args.contains("-k"))
    {
        Config::closable = false; // Kind of backdoor to close app. Alt+Backspace then Alt+F4.
        w.setWindowFlag(Qt::WindowStaysOnTopHint);
        w.showFullScreen();
    }
    else
    {
        w.show();
    }

    return a.exec();
}
