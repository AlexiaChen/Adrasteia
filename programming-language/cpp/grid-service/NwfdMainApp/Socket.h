/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: Socket.h
* 作  者: zhangl		版本：1.0		日  期：2015/06/27
* 描  述：底层socket基本操作
* 其  他：Windows Visual C++ 6.0, Linux gcc 3.4.3, FreeBSD gcc 3.4.6
* 功能列表:
* 修改日志：
*************************************************************************/
#ifndef __SOCKET_H_
#define __SOCKET_H_

//////////////////////////////////////
// 添加互斥锁
#ifndef _SOCKET_MUTEX
#define _SOCKET_MUTEX
#endif
#ifdef _SOCKET_MUTEX
#include <QMutex>
#include <QMutexLocker>
#endif
//////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
#ifndef _MAP_
	#ifdef WIN32
		#pragma warning (disable:4786)
	#endif
	#include <map>
#endif

#ifndef _LIST_
	#ifdef WIN32
		#pragma warning (disable:4786)
	#endif
	#include <list>
#endif

/////////////////////////////////////////////////////////////////////////////
#ifdef WIN32
	#ifndef _WINSOCKAPI_
	#include <winsock.h>
	#endif

	#pragma comment(lib, "Ws2_32.lib")
#else
	#ifndef __NETINET_IN_H
	#include <netinet/in.h>
	#endif

	#ifndef __SYS_SOCKET_H
	#include <sys/socket.h>
	#endif
#endif

/////////////////////////////////////////////////////////////////////////////
#ifdef WIN32
namespace SOCK {
#endif

/////////////////////////////////////////////////////////////////////////////
#ifndef WIN32
	typedef struct sockaddr SOCKADDR, *LPSOCKADDR;
	typedef struct sockaddr_in SOCKADDR_IN, *LPSOCKADDR_IN;
	typedef int SOCKET;

	#define SOCKET_ERROR	-1
	#define INVALID_SOCKET	-1
	#define INVALID_SOCKET	-1
#endif

/////////////////////////////////////////////////////////////////////////////
class CSocketBase
{
public:
	CSocketBase();
	virtual ~CSocketBase();

protected:
	int				m_nSocketType;
	SOCKADDR		m_sockAddr;
	SOCKET			m_hSocket;
#ifdef _SOCKET_MUTEX
	QMutex  m_mutexSnd;
#endif //_SOCKET_MUTEX
public:
	enum
	{
		ctNone		= 0,
		ctServer    = 1,
		ctConnect	= 2,
		ctAccept	= 3
	};

	int				m_nConnectType;
	
public:
	bool Init(int nSocketType = SOCK_STREAM);
	bool Init(SOCKET hSocket, const SOCKADDR *lpSockAddr, int nSockAddrLen);
	void Close();

	bool Bind(int nSocketPort, const char *lpszSocketAddress = NULL);
	bool Bind(const SOCKADDR *lpSockAddr, int nSockAddrLen);

	bool Listen(int nConnectionBacklog = SOMAXCONN);
	SOCKET Accept(SOCKADDR *lpSockAddr, int *lpSockAddrLen);
	virtual CSocketBase *Accept();

	bool Attach(SOCKET hSocket);
	SOCKET Detach();

	bool Connect(const char *lpszHostAddress, int nHostPort);
	virtual bool Connect(const SOCKADDR *lpSockAddr, int nSockAddrLen);

	virtual int Receive(void *lpBuf, int nBufLen, int nFlags = 0);
	virtual int Send(const void *lpBuf, int nBufLen, int nFlags = 0);

	int ReceiveFrom(void *lpBuf, int nBufLen,
		SOCKADDR *lpSockAddr, int *lpSockAddrLen, int nFlags = 0);

	int SendTo(const void *lpBuf, int nBufLen,
		int nHostPort, const char *lpszHostAddress = NULL, int nFlags = 0);
	int SendTo(const void *lpBuf, int nBufLen,
		const SOCKADDR *lpSockAddr, int nSockAddrLen, int nFlags = 0);

	bool GetPeerName(char *lpszPeerAddress, int &nPeerPort);
	bool GetPeerName(SOCKADDR *lpSockAddr, int *lpSockAddrLen);

	bool GetSockName(char *lpszSocketAddress, int &nSocketPort);
	bool GetSockName(SOCKADDR *lpSockAddr, int *lpSockAddrLen);

	bool SetSockOpt(int nOptionName, const void *lpOptionValue,
		int nOptionLen, int nLevel = SOL_SOCKET);
	bool GetSockOpt(int nOptionName, void *lpOptionValue,
		int *lpOptionLen, int nLevel = SOL_SOCKET);

	operator SOCKET() const;
	SOCKET GetHandle() const;

	bool IsEqual(CSocketBase &sock);
	bool IsEqual(const SOCKADDR *lpSockAddr, int nSockAddrLen);
	static bool IsEqual(const SOCKADDR *lpSockAddr1, const SOCKADDR *lpSockAddr2);

	bool IOCtl(long lCommand, long *lpArgument);
	virtual bool Select(int nMilliseconds = -1);

	int GetSocketType() const;
	int GetConnectType() const;

#ifdef WIN32
	static bool WSAStartup();
#endif

};

/////////////////////////////////////////////////////////////////////////////
class CSocketFactory;

class CSocket : public CSocketBase
{
public:
	CSocket();
	virtual ~CSocket();

protected:
	int				m_nID;
	int				m_nGroup;
	int 				m_Times;
	
	friend class CSocketFactory;
	
public:	
	void SetID(int nID);
	int GetID() const;

	void SetTimes(int nGroup);
	int GetTimes() const;

	
	void SetGroup(int nGroup);
	int GetGroup() const;

	bool Connect(const char *lpszHostAddress, int nHostPort);
	virtual bool Connect(const SOCKADDR *lpSockAddr, int nSockAddrLen);

	virtual int Receive(void *lpBuf, int nBufLen, int nFlags = 0);
	virtual int Send(const void *lpBuf, int nBufLen, int nFlags = 0);

	virtual CSocketBase *Clone();
	virtual CSocketBase *Accept();
	virtual bool Select(int nMilliseconds = -1);

	bool SetSvrSock(int iTout = 0);
	bool SetClnSock(int iTout = 0);

protected:
	virtual int OnReceive();
	virtual int OnSend();
	virtual int OnOutOfBandData();
	virtual int OnAccept();
	virtual int OnConnect(int nErrorCode);
	virtual int OnException(int nErrorCode);

};

/////////////////////////////////////////////////////////////////////////////
class CSocketFactory
{
public:
	CSocketFactory();
	virtual ~CSocketFactory();

public:
	typedef std::list<CSocket *> LIST_SOCKET;

protected:
	typedef std::list<SOCKET> LIST_SOCKET_HANDLE;
	LIST_SOCKET		m_lstSock;

public:
	bool Attach(CSocket *pSock, int nID);
	bool Attach(CSocket *pSock, int nGroup, int nID);
	CSocket *Detach(int nID);
	CSocket *Detach(int nGroup, int nID);

	virtual bool Select(int nMilliseconds = -1);
	virtual bool Poll(int nMilliseconds = -1);
	virtual CSocketBase *Accept(CSocket *lpSock);

	int GetCount(int nGroupID);
	virtual CSocket *FromHandle(SOCKET hSocket);
	virtual CSocket *FromID(int nID);
	virtual bool FromGroup(int nGroupID, LIST_SOCKET &lstSock);

	void Close();
	void Close(CSocket *pSock);

	virtual bool Lock();
	virtual void Unlock();

protected:
	virtual int OnReceive(CSocket *lpSock);
	virtual int OnSend(CSocket *lpSock);
	virtual int OnOutOfBandData(CSocket *lpSock);
	virtual int OnAccept(CSocket *lpSock);
	virtual int OnConnect(CSocket *lpSock, int nErrorCode);
	virtual int OnException(CSocket *lpSock, int nErrorCode);

};

#ifdef WIN32
}
#endif

#endif //__SOCKET_H_
