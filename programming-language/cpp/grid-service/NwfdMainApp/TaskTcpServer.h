/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: TaskTcpServer.h
* 作  者: zhangl		版本：1.0		日  期：2015/05/06
* 描  述：TcpServer类
* 其  他：
* 功能列表:
*   1.
* 修改日志：
*   No. 日期		作者		修改内容
*
*************************************************************************/

#ifndef TASK_TCP_SERVER_H
#define TASK_TCP_SERVER_H

/************************************************************************/
/* 头文件引用                                                           */
/************************************************************************/
#include "TaskBase.h"
//#include "nwfdtcpserver.h"
#include "Socket.h"
#include "ProductDef.h"
#include <QMutex>
#include <QMutexLocker>

#ifdef _WIN32
using namespace SOCK;
#endif 
/************************************************************************/
/* 消息缓冲区定义                                                       */
/************************************************************************/
#define  MAX_RECV_BUFFER     2048   //   

/************************************************************************/
/* socket列表                                                           */
/************************************************************************/
//typedef QHash<QString, CSocketBase *> HASH_SOCKET;
typedef QHash<CSocketBase *,QString> HASH_SOCKET;

/************************************************************************/
/* 类 名： TaskTcpServer                                                */
/* 父 类： TaskBase                                                     */
/* 说 明： TCP Server类                                                 */
/* 描 述：                                                              */
/************************************************************************/
class TaskTcpServer : public TaskBase, public CSocketFactory
{
	Q_OBJECT

public:
	TaskTcpServer();
	~TaskTcpServer();

public:
	void Init(int nListenPort, int nMaxConnection = 15);
	int InitTCPServer();
	bool AddProduct(ST_PRODUCT stProduct);
	bool FindProduct(QString strProductKey);
	bool RemoveProduct(QString strProductKey);

	// 广播数据
	void Broadcast(char * szSendMsg, int nLen);

protected:
	void run();

	virtual int OnReceive(CSocket *lpSock);
	virtual int OnAccept(CSocket *lpSock);
	virtual int OnException(CSocket *lpSock, int nErrorCode);

private:
	bool            m_bReady;          // run执行标记
	//NwfdTcpServer * m_tcpServer;       // TCPServer
	CSocket        *m_pSocket;     
	int             m_nListenPort;     // 侦听端口
	int             m_nMaxConnection;  // 最大连接数
	HASH_PRODUCT    m_hasProduct;      // 管理的产品列表
	QMutex          m_mutexProduct;    // 锁

	char    m_szRecvBuff[MAX_RECV_BUFFER];  // 数据接受缓冲区
	int     m_nRecvSize;

	HASH_SOCKET     m_hasClient;       // 客户端列表
	QMutex          m_mutexClient;
	QMutex          m_mutexBroadcast;
};

#endif // TASK_TCP_SERVER_H

