#include <QtGui/QApplication>
#include "dm.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DM w;
    w.show();

    return a.exec();
}
