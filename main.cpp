#include "imageviewer.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ImageViewer w(argc, argv);
    w.show();
    return a.exec();
}
