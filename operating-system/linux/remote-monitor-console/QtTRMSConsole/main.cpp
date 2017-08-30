#include "TRMSConsole.h"


int main(int argc, char *argv[])
{
   

    TRMSConsole console(argv[1],argv[2]);

    console.exec();
    return 0;
}
