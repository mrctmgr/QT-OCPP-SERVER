#include "ocpp.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    OCPP w;
    w.show();
    return a.exec();
}
