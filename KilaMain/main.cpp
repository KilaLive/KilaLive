#include "KilaMain.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    KilaMain w;
    w.show();
    return a.exec();
}
