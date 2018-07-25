#include "timekeeper.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TimeKeeper w;
    w.show();

    return a.exec();
}
