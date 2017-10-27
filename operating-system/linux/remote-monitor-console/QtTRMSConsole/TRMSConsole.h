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

    
    void exec();
private:

    Client *m_client_tcp;
    bool m_isConnected;
    std::string m_ip;
    std::string m_port;
};

#endif
