#include <QApplication>
#include "pinoutresolver.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    PinoutResolver w;
    w.show();
    
    return a.exec();
}
