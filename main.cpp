#include "linearequation.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LinearEquation w;
    w.show();
    return a.exec();
}
