#include "Socket.h"
#include "string.h"
#include "stdio.h"

#ifndef WIN32
	#include <unistd.h>
	#include <netdb.h>
	#include <arpa/inet.h>
	#include <sys/ioctl.h>
	#include <fcntl.h>
	#include <sys/poll.h>
	#include <errno.h>

	#include <sys/types.h>
	#include <netinet/tcp.h>
#endif

#ifdef WIN32
using namespace SOCK;
#endif

CSocketBase::CSocketBase()
{
	memset(&m_sockAddr, 0, sizeof(SOCKADDR));
	m_nSocketType = SOCK_STREAM;
	m_hSocket = INVALID_SOCKET;
	m_nConnectType = CSocketBase::ctNone;
}

CSocketBase::~CSocketBase()
{
	Close();
}

bool CSocketBase::GetPeerName(SOCKADDR *lpSockAddr, int *lpSockAddrLen)
{
	if (m_hSocket == INVALID_SOCKET)
		return false;

#ifdef WIN32
	return (SOCKET_ERROR != getpeername(m_hSocket, lpSockAddr, lpSockAddrLen));
#else
	return (SOCKET_ERROR != getpeername(m_hSocket, lpSockAddr, (socklen_t *)lpSockAddrLen));
#endif
}

bool CSocketBase::GetPeerName(char *lpszPeerAddress, int &nPeerPort)
{
	SOCKADDR_IN sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));

	int nSockAddrLen = sizeof(sockAddr);
	bool bResult = GetPeerName((SOCKADDR *)&sockAddr, &nSockAddrLen);
	if (bResult)
	{
		nPeerPort = ntohs(sockAddr.sin_port);
		strcpy(lpszPeerAddress, inet_ntoa(sockAddr.sin_addr));
	}

	return bResult;
}

bool CSocketBase::GetSockName(SOCKADDR *lpSockAddr, int* lpSockAddrLen)
{
	if (m_hSocket == INVALID_SOCKET)
		return false;

#ifdef WIN32
	return (SOCKET_ERROR != getsockname(m_hSocket, lpSockAddr, lpSockAddrLen));
#else
	return (SOCKET_ERROR != getsockname(m_hSocket, lpSockAddr, (socklen_t *)lpSockAddrLen));
#endif
}

bool CSocketBase::GetSockName(char *lpszSocketAddress, int &nSocketPort)
{
	SOCKADDR_IN sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));

	int nSockAddrLen = sizeof(sockAddr);
	bool bResult = GetSockName((SOCKADDR*)&sockAddr, &nSockAddrLen);
	if (bResult)
	{
		nSocketPort = ntohs(sockAddr.sin_port);
		strcpy(lpszSocketAddress, inet_ntoa(sockAddr.sin_addr));
	}

	return bResult;
}

bool CSocketBase::Init(int nSocketType/* = SOCK_STREAM*/)
{
	m_nSocketType = nSocketType;
	m_hSocket = socket(AF_INET, m_nSocketType, 0);
	if (m_hSocket == INVALID_SOCKET)
		return false;

#ifdef TIME_OUT  // 设置收发数据超时 2015-07-01 zhangl
#ifdef WIN32
	int nNetTimeout = 3000;  // 超时时间3秒
	//设置发送超时
	setsockopt(m_hSocket, SOL_SOCKET, SO_SNDTIMEO, (char *)&nNetTimeout, sizeof(int));
	//设置接收超时
	setsockopt(m_hSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&nNetTimeout, sizeof(int));
#else
	struct timeval timeout = { 3, 0 };  //超时时间3秒
	//设置发送超时
	setsockopt(m_hSocket,SOL_SOCKET, SO_SNDTIMEO,(char *)&timeout, sizeof(struct timeval));
	//设置接收超时
	setsockopt(m_hSocket,SOL_SOCKET, SO_RCVTIMEO,(char *)&timeout, sizeof(struct timeval));
#endif
#endif

	return true;
}

bool CSocketBase::Init(SOCKET hSocket, const SOCKADDR *lpSockAddr, int nSockAddrLen)
{
	if (! Attach(hSocket))
		return false;

	if (nSockAddrLen > sizeof(SOCKADDR))
		nSockAddrLen = sizeof(SOCKADDR);

	memcpy(&m_sockAddr, lpSockAddr, nSockAddrLen);

	return true;
}

bool CSocketBase::Attach(SOCKET hSocket)
{
	m_nSocketType = SOCK_STREAM;
	m_nConnectType = CSocketBase::ctAccept;

	if (hSocket != INVALID_SOCKET)
	{
		m_hSocket = hSocket;
		return true;
	}

	return false;
}

SOCKET CSocketBase::Detach()
{
	SOCKET hSocket = m_hSocket;
	m_hSocket = INVALID_SOCKET;

	return hSocket;
}

bool CSocketBase::Bind(int nSocketPort, const char *lpszSocketAddress/* = NULL*/)
{
	SOCKADDR_IN sockAddr;
	memset(&sockAddr, 0, sizeof(SOCKADDR_IN));

	sockAddr.sin_family = AF_INET;
	if (lpszSocketAddress == NULL)
		sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	else
		sockAddr.sin_addr.s_addr = inet_addr(lpszSocketAddress);;
	sockAddr.sin_port = htons(nSocketPort);

	return Bind((SOCKADDR*)&sockAddr, sizeof(sockAddr));
}

bool CSocketBase::Bind(const SOCKADDR *lpSockAddr, int nSockAddrLen)
{
	if (m_hSocket == INVALID_SOCKET)
		return false;

	m_nConnectType = CSocketBase::ctServer;
	return (SOCKET_ERROR != bind(m_hSocket, lpSockAddr, nSockAddrLen));
}

bool CSocketBase::IOCtl(long lCommand, long *lpArgument)
{
	if (m_hSocket == INVALID_SOCKET)
		return false;

#ifdef WIN32
	return (SOCKET_ERROR != ioctlsocket(m_hSocket, lCommand, (unsigned long *)lpArgument));
#else
	return (SOCKET_ERROR != ioctl(m_hSocket, lCommand, (unsigned long *)lpArgument));
#endif
}

bool CSocketBase::Listen(int nConnectionBacklog)
{
	if (m_hSocket == INVALID_SOCKET)
		return false;

	return (SOCKET_ERROR != listen(m_hSocket, nConnectionBacklog));
}

void CSocketBase::Close()
{
	if (m_hSocket != INVALID_SOCKET)
	{
#ifdef WIN32
		closesocket(m_hSocket);
#else
		::close(m_hSocket);
#endif

		m_hSocket = INVALID_SOCKET;
	}
}

bool CSocketBase::Connect(const char *lpszHostAddress, int nHostPort)
{
	SOCKADDR_IN sockAddr;
	memset(&sockAddr,0,sizeof(sockAddr));

	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = inet_addr(lpszHostAddress);

	if (sockAddr.sin_addr.s_addr == INADDR_NONE)
	{
		hostent *lphost;
		lphost = gethostbyname(lpszHostAddress);
		if (lphost != NULL)
			sockAddr.sin_addr.s_addr = ((in_addr *)lphost->h_addr)->s_addr;
		else
			return false;
	}

	sockAddr.sin_port = htons(nHostPort);

	return Connect((SOCKADDR*)&sockAddr, sizeof(sockAddr));
}

bool CSocketBase::Connect(const SOCKADDR *lpSockAddr, int nSockAddrLen)
{
	if (m_hSocket == INVALID_SOCKET)
		return false;

	m_nConnectType = CSocketBase::ctConnect;
	return (connect(m_hSocket, lpSockAddr, nSockAddrLen) != SOCKET_ERROR);
}

#define MAXSENDTIMES 2
#define MAXREADTIMES 2
#define SRINTERVAL 	   10

int CSocketBase::Receive(void *lpBuf, int iLen, int nFlags/* = 0*/)
{
	if (m_hSocket == INVALID_SOCKET)
		return -1;

	int iRet = 0, iRead = 0;

	while(iLen - iRead > 0)
	{
		iRet = recv(m_hSocket, (char *)lpBuf + iRead, iLen - iRead, nFlags);
		if (iRet <= -1)
		{
			return iRet;
		}
		else if (iRet == 0)
		{
			return -1;
		}
		iRead += iRet;
		if (iRead == iLen)
		{
			return iLen;
		}
		else
		{
			return iRead;
		}

	}

	return iLen;
}

int CSocketBase::Send(const void *lpBuf, int iLen, int nFlags/* = 0*/)
{
	/////////////////////////////////////////////////////////
	// 添加互斥锁
#ifdef _SOCKET_MUTEX
	printf("start send \n");
	QMutexLocker locker(&m_mutexSnd);
#endif
	////////////////////////////////////////////////////////
	printf("start send 1111111111111111111\n");
	if (m_hSocket == INVALID_SOCKET)
		return -1;

	int iRet = 0, iSent = 0, iCnt = 0;

	while(iLen - iSent > 0)
	{
		printf("start send 222222222222222222\n");
		iRet = send(m_hSocket, (char *)lpBuf + iSent, iLen - iSent, nFlags);
		printf("start send 333333333333333333\n");
		if(iRet <= -1 )
			return iRet;

		iSent += iRet;
		if(iSent == iLen) return iLen;

		iCnt ++;
		if(iCnt >= MAXSENDTIMES)
			return -1;

#ifdef WIN32
		Sleep(SRINTERVAL);	//1ms
#else
		usleep(SRINTERVAL * 1000);	//1ms
#endif
	}

	printf("end send\n");
	return iLen;
}

int CSocketBase::ReceiveFrom(void *lpBuf, int nBufLen,
		SOCKADDR *lpSockAddr, int *lpSockAddrLen, int nFlags/* = 0*/)
{
	if (m_hSocket == INVALID_SOCKET)
		return -1;

#ifdef WIN32
	return recvfrom(m_hSocket, (char *)lpBuf, nBufLen, nFlags, lpSockAddr, lpSockAddrLen);
#else
	return recvfrom(m_hSocket, (char *)lpBuf, nBufLen, nFlags, lpSockAddr, (socklen_t *)lpSockAddrLen);
#endif
}

int CSocketBase::SendTo(const void *lpBuf, int nBufLen,
		int nHostPort, const char *lpszHostAddress/* = NULL*/, int nFlags/* = 0*/)
{
	SOCKADDR_IN sockAddr;
	memset(&sockAddr,0,sizeof(sockAddr));

	sockAddr.sin_family = AF_INET;
	if (lpszHostAddress == NULL)
		sockAddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	else
	{
		sockAddr.sin_addr.s_addr = inet_addr(lpszHostAddress);
		if (sockAddr.sin_addr.s_addr == INADDR_NONE)
		{
			hostent *lphost;
			lphost = gethostbyname(lpszHostAddress);
			if (lphost != NULL)
				sockAddr.sin_addr.s_addr = ((in_addr *)lphost->h_addr)->s_addr;
			else
				return SOCKET_ERROR;
		}
	}

	sockAddr.sin_port = htons(nHostPort);

	return SendTo(lpBuf, nBufLen, (SOCKADDR*)&sockAddr, sizeof(sockAddr), nFlags);
}

int CSocketBase::SendTo(const void *lpBuf, int nBufLen,
		const SOCKADDR *lpSockAddr, int nSockAddrLen, int nFlags/* = 0*/)
{
	if (m_hSocket == INVALID_SOCKET)
		return -1;

	return sendto(m_hSocket, (char *)lpBuf, nBufLen, nFlags, lpSockAddr, nSockAddrLen);
}

bool CSocketBase::SetSockOpt(int nOptionName, const void *lpOptionValue,
		int nOptionLen, int nLevel/* = SOL_SOCKET*/)
{
	if (m_hSocket == INVALID_SOCKET)
		return false;

	return (SOCKET_ERROR != setsockopt(m_hSocket, nLevel, nOptionName, (char *)lpOptionValue, nOptionLen));
}

bool CSocketBase::GetSockOpt(int nOptionName, void *lpOptionValue,
		int *lpOptionLen, int nLevel/* = SOL_SOCKET*/)
{
	if (m_hSocket == INVALID_SOCKET)
		return false;

#ifdef WIN32
	return (SOCKET_ERROR != getsockopt(m_hSocket, nLevel, nOptionName, (char *)lpOptionValue, lpOptionLen));
#else
	return (SOCKET_ERROR != getsockopt(m_hSocket, nLevel, nOptionName, (char *)lpOptionValue, (socklen_t *)lpOptionLen));
#endif
}

CSocketBase::operator SOCKET() const
{
	return m_hSocket;
}

SOCKET CSocketBase::GetHandle() const
{
	return m_hSocket;
}

bool CSocketBase::IsEqual(CSocketBase &sock)
{
	
	return false;
}

bool CSocketBase::IsEqual(const SOCKADDR *lpSockAddr, int nSockAddrLen)
{
	if (nSockAddrLen < sizeof(SOCKADDR_IN))
		return false;

	LPSOCKADDR_IN lpAddr1 = (LPSOCKADDR_IN)lpSockAddr;
	LPSOCKADDR_IN lpAddr2 = (LPSOCKADDR_IN)&m_sockAddr;
	if ((lpAddr1->sin_addr.s_addr == lpAddr2->sin_addr.s_addr) &&
		(lpAddr1->sin_port == lpAddr2->sin_port))
		return true;

	return false;
}

bool CSocketBase::IsEqual(const SOCKADDR *lpSockAddr1, const SOCKADDR *lpSockAddr2)
{
	if (lpSockAddr1 == NULL || lpSockAddr2 == NULL)
		return false;

	LPSOCKADDR_IN lpAddr1 = (LPSOCKADDR_IN)lpSockAddr1;
	LPSOCKADDR_IN lpAddr2 = (LPSOCKADDR_IN)lpSockAddr2;
	if ((lpAddr1->sin_addr.s_addr == lpAddr2->sin_addr.s_addr) &&
		(lpAddr1->sin_port == lpAddr2->sin_port))
		return true;

	return false;
}

SOCKET CSocketBase::Accept(SOCKADDR *lpSockAddr, int *lpSockAddrLen)
{
	if (m_hSocket == INVALID_SOCKET)
		return INVALID_SOCKET;

	if (lpSockAddr == NULL || lpSockAddrLen == NULL)
		return INVALID_SOCKET;

	if (m_nSocketType == SOCK_STREAM && m_nConnectType == CSocketBase::ctServer)
	{
#ifdef WIN32
		SOCKET hTemp = accept(m_hSocket, (LPSOCKADDR)lpSockAddr, lpSockAddrLen);
#else
		SOCKET hTemp = accept(m_hSocket, (LPSOCKADDR)lpSockAddr, (socklen_t *)lpSockAddrLen);
#endif
		return hTemp;
	}

	return INVALID_SOCKET;
}

CSocketBase *CSocketBase::Accept()
{
	return NULL;
}

bool CSocketBase::Select(int nMilliseconds/* = -1*/)
{
	
	return false;
}

int CSocketBase::GetSocketType() const
{
	return m_nSocketType;
}

int CSocketBase::GetConnectType() const
{
	return m_nConnectType;
}

#ifdef WIN32

bool CSocketBase::WSAStartup()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested = MAKEWORD(2, 0); 

	int nFlag = ::WSAStartup(wVersionRequested, &wsaData);
	if (nFlag != 0)
	{
		/* Tell the user that we couldn't find a usable */
		/* WinSock DLL.  
		*/  
		return false;
	}

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 0) 
	{
		/* Tell the user that we couldn't find a usable */
		/* WinSock DLL.                                  */    
		WSACleanup();
		return false;
	}

	return true;
}

#endif

/////////////////////////////////////////////////////////////////////////////
#ifdef _AFXDLL //MFC 
	#define CSocket_	SOCK::CSocket
#else
	#define CSocket_	CSocket
#endif

/////////////////////////////////////////////////////////////////////////////
CSocket_::CSocket()
{
	m_nID = -1;
	m_nGroup = -1;
	m_Times=0;
}

CSocket_::~CSocket()
{
}

void CSocket_::SetID(int nID)
{
	m_nID = nID;
}

int CSocket_::GetID() const
{
	return m_nID;
}

void CSocket_::SetTimes(int ntimes)
{
	if(ntimes==0)
	{
		m_Times=0;
	}
	else
	{
		m_Times++;
	}
}

int CSocket_::GetTimes() const
{
	return m_Times;
}

void CSocket_::SetGroup(int nGroup)
{
	m_nGroup = nGroup;
}

int CSocket_::GetGroup() const
{
	return m_nGroup;
}

bool CSocket_::Connect(const char *lpszHostAddress, int nHostPort)
{
	if (m_hSocket == INVALID_SOCKET)
	{
		if (! CSocketBase::Init(SOCK_STREAM))
			return false;
	}
	
	return CSocketBase::Connect(lpszHostAddress, nHostPort);
}

bool CSocket_::Connect(const SOCKADDR *lpSockAddr, int nSockAddrLen)
{
	bool rc = CSocketBase::Connect(lpSockAddr, nSockAddrLen);
	if (rc)
		OnConnect(0);
	else
	{
#ifdef WIN32
		int nErrorCode = WSAGetLastError();
#else
		int nErrorCode = errno;
#endif
		OnConnect(nErrorCode);
	}
	return rc;
}

int CSocket_::Receive(void *lpBuf, int nBufLen, int nFlags/* = 0*/)
{
	int rc = CSocketBase::Receive(lpBuf, nBufLen, nFlags);
	if (rc <= 0)
	{
#ifdef WIN32
		int nErrorCode = WSAGetLastError();
#else
		int nErrorCode = errno;
#endif
		OnException(nErrorCode);
	}

	return rc;
}

int CSocket_::Send(const void *lpBuf, int nBufLen, int nFlags/* = 0*/)
{
	int rc = CSocketBase::Send(lpBuf, nBufLen, nFlags);
	if (rc == -1 || rc < nBufLen)
	{
#ifdef WIN32
		int nErrorCode = WSAGetLastError();
#else
		int nErrorCode = errno;
#endif
		OnException(nErrorCode);
	}

	return rc;
}

CSocketBase *CSocket_::Clone()
{
	return new CSocket;
}

CSocketBase *CSocket_::Accept()
{
	if (m_hSocket == INVALID_SOCKET)
		return NULL;

	SOCKADDR_IN sockAddr;
	int iSockLen = sizeof(SOCKADDR_IN);

	SOCKET hTemp = CSocketBase::Accept((LPSOCKADDR)&sockAddr, &iSockLen);
	if (hTemp != INVALID_SOCKET)
	{
		CSocket *pNewSock = (CSocket *)Clone();
		if (pNewSock == NULL)
		{
#ifdef WIN32
			closesocket(hTemp);
#else
			::close(hTemp);
#endif
			return NULL;
		}

		pNewSock->Init(hTemp, (SOCKADDR *)&sockAddr, iSockLen);
		return pNewSock;
	}

	return NULL;
}

bool CSocket_::Select(int nMilliseconds/* = -1*/)
{
	fd_set fsRead, rsExcept;
	FD_ZERO(&fsRead);
	FD_ZERO(&rsExcept);

	struct timeval tv;
	tv.tv_sec = nMilliseconds / 1000;
	tv.tv_usec = (nMilliseconds % 1000) * 1000;

	int hMaxSock = m_hSocket;
	FD_SET(m_hSocket, &fsRead);
	FD_SET(m_hSocket, &rsExcept);

	int rc = select(hMaxSock + 1, &fsRead, 0, &rsExcept, &tv);
	if (rc == -1)
	{
		//error
		return false;
	}
	else if (rc == 0)
	{
		//no data
		return true;
	}

	if (FD_ISSET(m_hSocket, &fsRead))
	{
		if (m_nSocketType == SOCK_STREAM)
		{
			if (m_nConnectType == CSocketBase::ctServer)
				OnAccept();
			else
				OnReceive();	
		}
		else
		{
			OnReceive();
		}
	}

	if (FD_ISSET(m_hSocket, &rsExcept))
	{
		OnException(0);			
	}

	return true;
}

bool CSocket_::SetSvrSock(int iTout)
{
	bool bRet;
	int iSockOpt = 1;

	bRet = SetSockOpt(SO_REUSEADDR, &iSockOpt, sizeof(int));

#ifdef _DEBUG
	printf("YZB INF Info :: SO_REUSEADDR, return : %d\n", bRet);
#endif

	SetClnSock(iTout);

	return true;
}


/*bool CSocket_::SetClnSock(int iTout)
{
	bool bRet;	
	int iSockOpt = 1;
	
	bRet = SetSockOpt(SO_KEEPALIVE, &iSockOpt, sizeof(int));

#ifdef _DEBUG
	printf("YZB INF Info :: SO_KEEPALIVE, return : %d\n", bRet);
#endif
	
	struct linger lg;
	memset(&lg, 0, sizeof(linger));
	
	bRet = SetSockOpt(SO_LINGER, &lg, sizeof(lg));
	
#ifdef _DEBUG
	printf("YZB INF Info :: SO_LINGER, return : %d\n", bRet);
#endif
	
	struct timeval tv;
	memset(&tv, 0, sizeof(tv));
	
	tv.tv_sec = iTout;
	tv.tv_usec = 0;
	
	bRet = SetSockOpt(SO_RCVTIMEO, &tv, sizeof(tv));

#ifdef _DEBUG
	printf("YZB INF Info :: SO_RCVTIMEO, return : %d\n", bRet);
#endif

}
*/


bool CSocket_::SetClnSock(int iTout)
{
	bool bRet;	
	int iSockOpt = 1;
	
	bRet = SetSockOpt(SO_KEEPALIVE, &iSockOpt, sizeof(int));

#ifdef _DEBUG
	printf("YZB INF Info :: SO_KEEPALIVE, return : %d\n", bRet);
#endif
	
//	KeepAlive 实现
//	int keepIdle = 0;// 开始首次 KeepAlive 探测前的 TCP 空闭时间
//	int keepInterval = 60;// 两次 KeepAlive 探测间的时间间隔
//	int keepCount = 5;// 判定断开前的 KeepAlive 探测次数
//
//	bRet = SetSockOpt(TCP_KEEPIDLE, (void *)&keepIdle, sizeof(keepIdle), SOL_TCP);
//#ifdef _DEBUG
//	printf("INF info :: TCP_KEEPIDLE, return : %d\n", bRet);
//#endif
//
//	bRet = SetSockOpt(TCP_KEEPINTVL, (void *)&keepInterval, sizeof(keepInterval), SOL_TCP);
//#ifdef _DEBUG
//	printf("INF info :: TCP_KEEPINTVL, return : %d\n", bRet);
//#endif
//
//	bRet = SetSockOpt(TCP_KEEPCNT, (void *)&keepCount, sizeof(keepCount), SOL_TCP);
//#ifdef _DEBUG
//	printf("INF info :: TCP_KEEPCNT, return : %d\n", bRet);
//#endif

	struct linger lg;
	memset(&lg, 0, sizeof(linger));
	
	bRet = SetSockOpt(SO_LINGER, &lg, sizeof(lg));
	
#ifdef _DEBUG
	printf("YZB INF Info :: SO_LINGER, return : %d\n", bRet);
#endif
	
	struct timeval tv;
	memset(&tv, 0, sizeof(tv));
	
	tv.tv_sec = iTout;
	tv.tv_usec = 0;
	
	bRet = SetSockOpt(SO_RCVTIMEO, &tv, sizeof(tv));

#ifdef _DEBUG
	printf("YZB INF Info :: SO_RCVTIMEO, return : %d\n", bRet);
#endif

	return true;
}


/////////////////////////////////////////////////////////////////////////////
// CSocket Overridable callbacks

int CSocket_::OnReceive()
{
	return -1;
}

int CSocket_::OnSend()
{
	return -1;
}

int CSocket_::OnOutOfBandData()
{
	return -1;
}

int CSocket_::OnAccept()
{
	return -1;
}

int CSocket_::OnConnect(int nErrorCode)
{
	return -1;
}

int CSocket_::OnException(int nErrorCode)
{
	return -1;
}

/////////////////////////////////////////////////////////////////////////////

CSocketFactory::CSocketFactory()
{
}

CSocketFactory::~CSocketFactory()
{
	Close();
}

bool CSocketFactory::Attach(CSocket_ *pSock, int nID)
{
	return Attach(pSock, -1, nID);
}

bool CSocketFactory::Attach(CSocket_ *pSock, int nGroup, int nID)
{
	if (pSock == NULL)
		return false;

	Lock();

	bool bFound = false;
	LIST_SOCKET::iterator iter;
	for (iter = m_lstSock.begin(); iter != m_lstSock.end(); iter ++)
	{
		CSocket *pTempSock = *iter;
		if (pTempSock == pSock)
		{
			bFound = true;
			break;
		}
	}

	if (! bFound)
	{
		pSock->SetGroup(nGroup);
		pSock->SetID(nID);
		m_lstSock.insert(m_lstSock.end(), pSock);
	}

	Unlock();

	return ! bFound;
}

CSocket_ *CSocketFactory::Detach(int nID)
{
	Lock();

	CSocket *pSock = NULL;
	LIST_SOCKET::iterator iter;
	for (iter = m_lstSock.begin(); iter != m_lstSock.end(); iter ++)
	{
		CSocket *pTempSock = *iter;
		if (pTempSock)
		{
			if (pTempSock->GetID() == nID)
			{
				m_lstSock.erase(iter);
				pSock = pTempSock;
			}
		}
	}

	Unlock();

	return pSock;
}

CSocket_ *CSocketFactory::Detach(int nGroup, int nID)
{
	Lock();

	CSocket *pSock = NULL;
	LIST_SOCKET::iterator iter;
	for (iter = m_lstSock.begin(); iter != m_lstSock.end(); iter ++)
	{
		CSocket *pTempSock = *iter;
		if (pTempSock)
		{
			if (pTempSock->GetGroup() == nGroup && pTempSock->GetID() == nID)
			{
				m_lstSock.erase(iter);
				pSock = pTempSock;
				break;
			}
		}
	}

	Unlock();

	return pSock;
}

bool CSocketFactory::Select(int nMilliseconds/* = -1*/)
{
	fd_set fsRead, rsExcept;
	FD_ZERO(&fsRead);
	FD_ZERO(&rsExcept);

	struct timeval tv;
	tv.tv_sec = nMilliseconds / 1000;
	tv.tv_usec = (nMilliseconds % 1000) * 1000;

	Lock();

	uint hMaxSock = 0;
	LIST_SOCKET_HANDLE listSockHandle;	
	LIST_SOCKET::iterator iter;
	for (iter = m_lstSock.begin(); iter != m_lstSock.end(); iter ++)
	{
		CSocket *pSock = *iter;
		if (pSock)
		{
			SOCKET hSocket = pSock->m_hSocket;
			if (hSocket != INVALID_SOCKET)
			{
				listSockHandle.insert(listSockHandle.end(), hSocket);

				FD_SET(hSocket, &fsRead);
				FD_SET(hSocket, &rsExcept);

				if (hMaxSock < hSocket)
					hMaxSock = hSocket;
			}
		}
	}

	Unlock();

	if (listSockHandle.size() == 0)
		return false;

	int rc = select(hMaxSock + 1, &fsRead, 0, &rsExcept, &tv);

	if (rc == -1)
	{
		//error
		return false;
	}
	else if (rc == 0)
	{
		//no data
		return true;
	}

	LIST_SOCKET_HANDLE::iterator iterH;
	for (iterH = listSockHandle.begin(); iterH != listSockHandle.end(); iterH ++)
	{
		bool bReadFlag = false;
		SOCKET hSocket = *iterH;
		CSocket *pSock = FromHandle(hSocket);
		if (pSock)
		{
			if (FD_ISSET(hSocket, &fsRead))
			{
				bReadFlag = true;
				if (pSock->GetSocketType() == SOCK_STREAM)
				{
					if (pSock->GetConnectType() == CSocket::ctServer)
						OnAccept(pSock);
					else
						OnReceive(pSock);
				}
				else
				{
					OnReceive(pSock);
				}
			}

			if (! bReadFlag) 
			{
				if (FD_ISSET(hSocket, &rsExcept))
				{
					OnException(pSock, 0);
				}
			}			
		}
	}

	return true;
}

bool CSocketFactory::Poll(int nMilliseconds/* = -1*/)
{
#ifndef WIN32
	//struct pollfd {
	//	int fd;           /* file descriptor */
	//	short events;     /* requested events */
	//	short revents;    /* returned events */
	//};

	int nSockCount = m_lstSock.size(), nCount = 0;
	struct pollfd *pPfd = new pollfd[nSockCount];
	if (pPfd == NULL)
		return false;

	Lock();

	LIST_SOCKET::iterator iter;
	for (iter = m_lstSock.begin(); iter != m_lstSock.end(); iter ++)
	{
		CSocket *pSock = *iter;
		if (pSock)
		{
			SOCKET hSocket = pSock->m_hSocket;
			if (hSocket != INVALID_SOCKET)
			{
				pPfd[nCount].fd = hSocket;
				pPfd[nCount].events = POLLRDNORM;
				pPfd[nCount].revents = 0;
				nCount ++;
			}
		}
	}

	Unlock();

	int rc = poll(pPfd, nCount, 5000);
	if (rc > 0)
	{
		for (int i = 0; i < nCount; i ++)
		{
			if (pPfd[i].revents & POLLRDNORM)
			{
				CSocket *pSock = FromHandle(pPfd[nCount].fd);
				if (pSock)
				{
					if (pSock->GetSocketType() == SOCK_STREAM)
					{
						if (pSock->GetConnectType() == CSocket::ctServer)
							OnAccept(pSock);
						else
							OnReceive(pSock);
					}
					else
					{
						OnReceive(pSock);
					}
				}
			}
		}

		return true;
	}

#endif
	return false;
}

CSocket_ *CSocketFactory::FromHandle(SOCKET hSocket)
{
	Lock();

	CSocket *pSock = NULL;
	LIST_SOCKET::iterator iter;
	for (iter = m_lstSock.begin(); iter != m_lstSock.end(); iter ++)
	{
		CSocket *pTempSock = *iter;
		if (pTempSock)
		{
			if (pTempSock->GetHandle() == hSocket)
			{
				pSock = pTempSock;
				break;
			}
		}
	}

	Unlock();

	return pSock;
}

CSocket_ *CSocketFactory::FromID(int nID)
{
	Lock();

	CSocket *pSock = NULL;
	LIST_SOCKET::iterator iter;
	for (iter = m_lstSock.begin(); iter != m_lstSock.end(); iter ++)
	{
		CSocket *pTempSock = *iter;
		if (pTempSock)
		{
			if (pTempSock->GetID() == nID)
			{
				pSock = pTempSock;
				break;
			}
		}
	}

	Unlock();

	return pSock;
}

bool CSocketFactory::FromGroup(int nGroupID, CSocketFactory::LIST_SOCKET &lstSock)
{
	Lock();

	LIST_SOCKET::iterator iter;
	for (iter = m_lstSock.begin(); iter != m_lstSock.end(); iter ++)
	{
		CSocket *pTempSock = *iter;
		if (pTempSock)
		{
			if (pTempSock->GetConnectType() != CSocket::ctServer)
			{
				if (pTempSock->GetGroup() == nGroupID)
					lstSock.insert(lstSock.end(), pTempSock);
			}
		}
	}

	Unlock();

	return (lstSock.size() > 0);
}

int CSocketFactory::GetCount(int nGroupID)
{
	Lock();

	int nCount = 0;
	LIST_SOCKET::iterator iter;
	for (iter = m_lstSock.begin(); iter != m_lstSock.end(); iter ++)
	{
		CSocket *pTempSock = *iter;
		if (pTempSock)
		{
			if (pTempSock->GetGroup() == nGroupID)
				nCount ++;
		}
	}

	Unlock();

	return nCount;
}

void CSocketFactory::Close()
{
	Lock();

	LIST_SOCKET::iterator iter;
	for (iter = m_lstSock.begin(); iter != m_lstSock.end(); iter ++)
	{
		CSocket *pSock = *iter;
		if (pSock)
		{
			delete pSock;
			pSock = NULL;	
		}
	}

	m_lstSock.clear();

	Unlock();
}

CSocketBase *CSocketFactory::Accept(CSocket_ *lpSock)
{
	if (lpSock == NULL)
		return NULL;

	CSocket *pNewSock = (CSocket *)lpSock->Accept();
	
	if (pNewSock)
	{
		//TcpSvrInf
#ifdef _TCP_SVR		
		pNewSock->SetClnSock(5);	
#else	//MsgSvr
		pNewSock->SetClnSock();	
#endif

		Lock();
		m_lstSock.insert(m_lstSock.end(), pNewSock);
		Unlock();

		return pNewSock;
	}

	return NULL;
}

void CSocketFactory::Close(CSocket_ *pSock)
{
	if (pSock == NULL)
		return;

	Lock();
	
	LIST_SOCKET::iterator iter;
	for (iter = m_lstSock.begin(); iter != m_lstSock.end(); iter ++)
	{
		CSocket *pTempSock = *iter;
		if (pTempSock == pSock)
		{
			m_lstSock.erase(iter);
			break;
		}
	}

	delete pSock;	
	pSock = NULL;
	
	Unlock();
}

bool CSocketFactory::Lock()
{
	return true;
}

void CSocketFactory::Unlock()
{
}

/////////////////////////////////////////////////////////////////////////////
// CSocketFactory:: Overridable callbacks

int CSocketFactory::OnReceive(CSocket_ *lpSock)
{
	return -1;
}

int CSocketFactory::OnSend(CSocket_ *lpSock)
{
	return -1;
}

int CSocketFactory::OnOutOfBandData(CSocket_ *lpSock)
{
	return -1;
}

int CSocketFactory::OnAccept(CSocket_ *lpSock)
{
	if (lpSock == NULL)
		return 0;

	CSocketBase *pNewSock = Accept(lpSock);
	if (pNewSock)
		return 1;

	return 0;
}

int CSocketFactory::OnConnect(CSocket_ *lpSock, int nErrorCode)
{
	return -1;
}

int CSocketFactory::OnException(CSocket_ *lpSock, int nErrorCode)
{
	if (lpSock)
	{
		lpSock->OnException(nErrorCode);

		//default clost it
		Close(lpSock);

		return 1;
	}

	return 0;
}
