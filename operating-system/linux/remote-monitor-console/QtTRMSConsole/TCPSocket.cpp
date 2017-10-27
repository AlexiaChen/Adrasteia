#define VERBOSE 1      // turn on or off debugging output

#include "TCPSocket.h"

#include <cstdio>
#include <iostream>

Client::Client(int iPort, const char* pStrHost,  bool* pResult)
{
    struct hostent*	he = NULL;

    m_iPort = iPort;
    m_iSock = -1;

    if (pResult)
        *pResult = false;

   

#ifdef _WIN32
    // For Windows, we need to fire up the Winsock DLL before we can do socket stuff.
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    // Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h 
    wVersionRequested = MAKEWORD(2, 2);

    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0)
    {
        // Tell the user that we could not find a usable Winsock DLL
       // perror("Client::Client, WSAStartup failed with error");
        return;
    }
#endif

    if (VERBOSE)
        //printf("Client: opening socket to %s on port = %d\n", pStrHost, m_iPort);

    if ((he = gethostbyname(pStrHost)) == NULL)
    {
        perror("Client::Client, gethostbyname");
        return;
    }

    if ((m_iSock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Client::Client, socket");
        return;
    }

    m_addrRemote.sin_family = AF_INET;
    m_addrRemote.sin_port = htons(m_iPort);
    m_addrRemote.sin_addr = *((struct in_addr *) he->h_addr);
    memset(&(m_addrRemote.sin_zero), 0, 8);

    if (connect(m_iSock, (struct sockaddr *) &m_addrRemote, sizeof(struct sockaddr)) == -1)
    {
        perror("Client::Client, connect m_iSock");
        return;
    }

    if (pResult)
        *pResult = true;
}

Client::~Client()
{
#ifdef _WIN32
    // Windows specific socket shutdown code
    WSACleanup();
#endif

}


// Send some bytes over the wire
bool Client::SendBytes(char* pVals, int iLen)
{
    int i = 0;

    if (send(m_iSock, (char *)pVals, iLen, 0) == -1)
    {
        perror("Client::SendBytes, send bytes");
        return false;
    }


    return true;
}




// Receive some bytes, returns number of bytes received.
int Client::RecvBytes(char* pVals, int iLen)
{
    int			i = 0;
    int			iNumBytes = 0;

    memset(pVals, 0, iLen);

    if ((iNumBytes = recv(m_iSock, pVals, iLen, 0)) == -1)
    {
        perror("Client::RecvBytes, recv");
        return -1;
    }

    if (iLen < iNumBytes)
    {
        //std::cout << "Recb Buffer size < recved size" << std::endl;
        return -1;
    }

    return iNumBytes;
}


// Shut down the socket.
bool Client::Close()
{
    if (shutdown(m_iSock, SD_BOTH) == -1)
        return false;
#ifndef _WIN32
    close(m_iSock);
#endif
    return true;
}
