#ifndef TRMS_CONSOLE_H
#define TRMS_CONSOLE_H

#include <string>
#include <vector>

#include "p_TabStruct.h"
#include "TCPSocket.h"
#include "GenTab.h"


class TRMSConsole 
{
public:
    TRMSConsole(const std::string& ip, const std::string& port);
    ~TRMSConsole();

    void login();
    void exec();

   

private:

    void iniRead(std::string& ip, std::string &port);
    //   自动生成的打包解包代码

   


private:

    Client *m_client_tcp;
    bool m_isConnected;
    std::string m_ip;
    std::string m_port;
};

#endif
