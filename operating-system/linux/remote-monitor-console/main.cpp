#include <QtCore/QCoreApplication>
#include "TRMSConsole.h"

#include <vector>
#include <iterator>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    TRMSConsole console;

    return a.exec();

}
