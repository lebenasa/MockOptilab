#include <QApplication>
#include <QQmlApplicationEngine>

#include "serialcapture.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    SerialCapture sc;
    sc.show();

    return app.exec();
}
