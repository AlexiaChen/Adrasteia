#include "TRMSConsole.h"


#ifdef __linux__
#include <arpa/inet.h>
#include "zlib.h"
#endif

#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#ifdef __linux__
#include <pthread.h>
#endif


#ifndef __linux__
#include<Winsock2.h>
#pragma comment(lib,"ws2_32.lib")
#endif

#include "Utils.h"
#include "TCPSocket.h"
#include "p_TabStruct.h"

static TCLIENT_INFO m_client_info;
static bool m_isRecvEnd = false;


static void initClientInf(TCLIENT_INFO& info){

    //info.Socket = NULL;
    info.LoginTime = 0;
    info.LastSendTime = 0;
    info.SendTime = 0;
    info.PackageType = 0;
    info.CUR_STEP = RECV_HEAD;
    info.CUR_IDX = 0;
    memset(&info.bFieldBuf, 0, 16);
    info.iPackageFLAG = 0;
    info.iLength = 0;
    info.iTrue_Length = 0;
    info.msRecv = NULL;
    info.msRecvLength = 0;
    info.IPAddress.assign("");
    info.MAC.assign("");
    info.OSBootTime.assign("");
    info.BaseInfo.assign("");
    info.DiskSpaceInfo.assign("");
    info.CPU = 0;
    info.Memory = 0;
}



TRMSConsole::TRMSConsole(const std::string& ip, const std::string& port)
 
{
    
   // std::cout << "TRMSConsole entry" << std::endl;
    m_client_tcp = NULL;
    m_isRecvEnd = false;
    m_isConnected = false;
    initClientInf(m_client_info);

    m_ip = ip;
    m_port = port;

    //std::cout << "connectting host is: " << ip << std::endl;
   // std::cout << "connectting port is: " << port << std::endl;


    m_client_tcp = new Client(std::atoi(port.c_str()), ip.c_str(), &m_isConnected);
 
   // std::cout << "TRMSConsole leave" << std::endl;
}

TRMSConsole::~TRMSConsole()
{
    
}

static std::string  stringTrim(const std::string & s)
{
    char *new_str = NULL;
    size_t first_valid_char = 0;
    size_t last_valid_char = 0;

    if (s.empty()) {
        return std::string("");
    }

    size_t size = s.size();

    new_str = (char*)malloc(size + 30);
    if (!new_str) {
        return std::string("");
    }

    for (first_valid_char = 0; first_valid_char < s.length(); first_valid_char++) {
        if (s.at(first_valid_char) != ' ' && s.at(first_valid_char) != '\n' \
            && s.at(first_valid_char) != '\r') {
            break;
        }
    }

    for (last_valid_char = s.length() - 1; last_valid_char >= 0; last_valid_char--) {
        if (s.at(last_valid_char) != ' ' && s.at(last_valid_char) != '\n' \
            && s.at(last_valid_char) != '\r' && s.at(last_valid_char) != 0) {
            break;
        }
    }

    const char *str = s.data();

    if (first_valid_char >= last_valid_char) {
        new_str[0] = 0;
    }
    else {
        memcpy(new_str, (str + first_valid_char), (last_valid_char - first_valid_char + 1));
        new_str[last_valid_char - first_valid_char + 1] = 0;
    }

    std::string temp = std::string(new_str);
    free(new_str);
    return temp;
}


void  TRMSConsole::login()
{
   
   // std::cout << "login() entry" << std::endl;
    
    TCLIENT_LOGIN tLogin;
    p_InitTab_Client_Login(&tLogin);
    strcpy(tLogin.IPAddress, Utils::localIP().c_str());
    //std::cout << "local IP leave" << Utils::localIP().c_str() << std::endl;
    strcpy(tLogin.MAC, Utils::localMAC().c_str());
    //std::cout << "local MAC leave" << Utils::localMAC().c_str() << std::endl;
    strncpy(tLogin.OSBootTime, Utils::lastStartupTime().c_str(),21);
    //std::cout << "local OS Time leave" << stringTrim(Utils::lastStartupTime()).c_str() << std::endl;
    
    strncpy(tLogin.BaseInfo, Utils::processesInf().c_str(), 4096);
    //std::cout << "Base Info" << Utils::processesInf().c_str() << std::endl;


    char cSendBuf[CLIENT_LOGIN_LEN + 1];
    p_TabPack_Client_Login(cSendBuf, CLIENT_LOGIN_LEN + 1, &tLogin);
    //std::cout << "p_TabPack_Client_Login Leave" << std::endl;
    int iPackageLength = 0;

    //std::cout << "Create Package Entry" << std::endl;

    unsigned char  bPackageData[1024 * 10] = {0};
    
    int iSendBufLen = CLIENT_LOGIN_LEN + 1;
    CreatPackage("1001", cSendBuf, iSendBufLen, bPackageData, iPackageLength);

    //std::cout << "Create Package Leave" << std::endl;
  
    SendData(m_client_tcp,bPackageData, iPackageLength);
    
    //std::cout << "login() leave" << std::endl;

    return;
}

void TRMSConsole::iniRead(std::string& ip, std::string &port)
{
    std::ifstream infile("config.ini");
    std::string line;
    while (std::getline(infile, line))
    {
        std::istringstream iss(line);
        std::string key, value;
        if (!(iss >> key >> value)) { //std::cout << "ini read failed" << std::endl; 
            break; } // error

        if (key.compare("ip") == 0)
        {
            ip.assign(value);
        }
        if (key.compare("port") == 0)
        {
            port.assign(value);
        }
    }
}



 void *recvThreadFunc(void* value_this)
{
    
    Client * tcp = (Client*)value_this;

    FILE *fp = NULL;
    
    while (true)
    {
        unsigned char recvBuffer[1024 * 10] = {0};
        int recv_size = tcp->RecvBytes((char*)recvBuffer, 1024 * 10);
        if (recv_size == -1) exit(-1);
       
        
        //²ð°ü
        AnaData(recvBuffer, recv_size, &m_client_info, m_isRecvEnd);
        if (m_isRecvEnd)
        {
            AnaPackage(tcp, &m_client_info);
            initClientInf(m_client_info);
            m_isRecvEnd = false;
        }
    }

    return NULL;
    
}

void TRMSConsole::exec()
{
    while (!m_isConnected)
    {
        delete m_client_tcp;
        m_client_tcp = NULL;
        
        std::string ip, port;
        ip = m_ip;
        port = m_port;

       // std::cout << "re-connectting host is: " << ip << std::endl;
        //std::cout << "re-connectting port is: " << port << std::endl;
        m_client_tcp = new Client(std::atoi(port.c_str()), ip.c_str(), &m_isConnected);

    }

#ifdef __linux__
    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, &recvThreadFunc, (void*)m_client_tcp);
#endif
   
    
    login();
#ifdef __linux__
    void *status;
    pthread_join(recv_thread, &status);
#endif
    
}









