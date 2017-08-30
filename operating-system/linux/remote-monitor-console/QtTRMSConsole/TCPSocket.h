// A C++ socket client class
//
// Keith Vertanen 11/98, updated 12/08

#ifndef _CLIENT_H__
#define _CLIENT_H__

// Adds in the send/recv acks after each message.
#define DEBUG_ACK

static const int CLIENT_BUFF_SIZE = 64000;

#ifdef __linux__
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#endif

#ifdef _WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

// Duplicated from winsock2.h
#define SD_RECEIVE      0x00
#define SD_SEND         0x01
#define SD_BOTH         0x02

#endif

class Client
{
public:
    Client(int iPort, const char* pStrHost,bool* pResult);
    ~Client();

    bool				Close();										// Close the socket

    bool				SendBytes(char* pVals, int iLen);				// Send some bytes 
    int					RecvBytes(char* pVals, int iLen);  				// Receive some bytes


protected:
    int					m_iPort;							// Port I'm listening on
    int					m_iSock;							// Socket connection
    struct sockaddr_in	m_addrRemote;						// Connector's address information
    char*				m_pBuffer;							// Reuse the same memory for buffer

};

#endif