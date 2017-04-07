#include "FtpTest.h"
#include <QtWidgets/QApplication>
#include <QTextCodec>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    FtpTest w;
    w.show();
    return a.exec();
}
