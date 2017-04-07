#include "log.h" // log日志
#include "TaskTcpServer.h"
#include "DataProcesser.h"
#include "stdio.h"

/************************************************************************/
/* 构造函数&析构函数                                                    */
/************************************************************************/
TaskTcpServer::TaskTcpServer()
{
	LOG_(LOGID_DEBUG, LOG_F("Entry TaskTcpServer()"));
	
	LOG_(LOGID_DEBUG, LOG_F("TCP Server任务（TaskTcpServer）构造."));

	m_strTaskName = "TCP Server任务";

	//m_tcpServer = NULL;
	m_pSocket = NULL;
	m_nListenPort = 1324;
	m_nMaxConnection = 15;

	m_emTaskState = _TASK_DEAD;

	

	LOG_(LOGID_DEBUG, LOG_F("Leave TaskTcpServer()"));
}

TaskTcpServer::~TaskTcpServer()
{
	LOG_(LOGID_DEBUG, LOG_F("Entry TaskTcpServer()"));
	// 停止线程执行
	m_bReady = false;
	wait();

	// 关闭TCP Server
	//if (m_pSocket)
	//{
	//	m_tcpServer->close();
	//	delete m_tcpServer;  // ?
	//	m_tcpServer = NULL;
	//}
	CSocketFactory::Close();

	LOG_(LOGID_DEBUG, LOG_F("~TCP Server任务（TaskTcpServer）析构."));
	LOG_(LOGID_DEBUG, LOG_F("Leave ~TaskTcpServer()"));
}

/************************************************************************/
/* 初始化                                                               */
/************************************************************************/
void TaskTcpServer::Init(int nListenPort, int nMaxConnection)
{
	m_nListenPort = nListenPort;
	m_nMaxConnection = nMaxConnection;

	// 创建TCP Server Socket
	int nErr = InitTCPServer();
	if (nErr == 0)
	{
		// 创建TCPServer成功
		m_emTaskState = _TASK_READ;
	}
	else
	{
		LOG_(LOGID_ERROR, LOG_F("[TaskPID:%d] TCP服务c初始化失败，errcode=%d."), m_nTaskPID, nErr);

		// 创建TCPServerSocket失败，任务挂起
		m_emTaskState = _TASK_INTERRUPTIBLE;
	}
}

/************************************************************************/
/* 初始化TCPServer                                                      */
/************************************************************************/
int TaskTcpServer::InitTCPServer()
{
	m_pSocket = new CSocket;
	if (m_pSocket == NULL)
		return -1;

	if (!m_pSocket->Init(SOCK_STREAM))
	{
		delete m_pSocket;
		return -2;
	}
	m_pSocket->SetSvrSock();
	if (!m_pSocket->Bind(m_nListenPort))
	{
		delete m_pSocket;
		return -3;
	}
	m_pSocket->Listen();
	m_pSocket->SetGroup(-1);
	CSocketFactory::Attach(m_pSocket, 0);

	return 0;
}

/************************************************************************/
/* 添加管理产品                                                         */
/************************************************************************/
bool TaskTcpServer::AddProduct(ST_PRODUCT  stProduct)
{
	if (FindProduct(stProduct.strKey))
	{
		// 已存在
		return false;
	}

	// 添加到任务列表中
	QMutexLocker locker(&m_mutexProduct);
	m_hasProduct.insert(stProduct.strKey, stProduct);

	return true;
}

/************************************************************************/
/* 查找管理产品                                                         */
/************************************************************************/
bool TaskTcpServer::FindProduct(QString strProductKey)
{
	QMutexLocker locker(&m_mutexProduct);

	if (m_hasProduct.contains(strProductKey))
	{
		return true;
	}

	return false;
}

/************************************************************************/
/* 移除管理产品                                                         */
/************************************************************************/
bool TaskTcpServer::RemoveProduct(QString strProductKey)
{
	if (FindProduct(strProductKey))
	{
		m_mutexProduct.lock();
		m_hasProduct.remove(strProductKey);
		m_mutexProduct.unlock();
	}

	return true;
}

/************************************************************************/
/* 任务处理执行方法                                                     */
/************************************************************************/
void TaskTcpServer::run()
{
	
	// 设置该线程的优先级为最高
	this->setPriority(QThread::HighestPriority);
	
	LOG_(LOGID_INFO, LOG_F("[TaskPID:%d] TCP服务任务启动执行."), m_nTaskPID);
	if (m_emTaskState != _TASK_READ)
	{
		// 如果任务没有在准备状态，不予执行此次任务
		LOG_(LOGID_INFO, LOG_F("[TaskPID:%d][Notice] TcpServer任务没有完成准备工作，不能执行，任务结束（当前状态为：%d）."), m_nTaskPID, (int)m_emTaskState);
		return;
	}

	m_bReady = true;

	while (m_bReady)
	{
		if (!Select(5000))
		{
			if (m_bReady)
			{
				msleep(1);
			}
		}
	}
}

/************************************************************************/
/* 接收到数据                                                           */
/************************************************************************/
int TaskTcpServer::OnReceive(CSocket *lpSock)
{
	if (lpSock == NULL)
		return 0;

	char szIP[32];
	int nPort = 0;
	memset(szIP, 0x00, sizeof(szIP));
	lpSock->GetPeerName(szIP, nPort);
	QString strIP = QString::fromLocal8Bit(szIP);
	LOG_(LOGID_NETWORK, LOG_F("[recv]接收到客户端[%s:%d]发送的数据"), LOG_STR(strIP), nPort);

	memset(m_szRecvBuff, 0x00, sizeof(MAX_RECV_BUFFER));
	m_nRecvSize = 0;

	int rc = lpSock->Receive(m_szRecvBuff, MAX_RECV_BUFFER, 0);
	if (rc == -1)
	{
		// 异常，断开连接
		OnException(lpSock, 1);
		return false;
	}

	// 构造处理器
	DataProcesser dataProcesser(m_szRecvBuff, rc);

	// 响应数据
	QByteArray responseData;

	// 对数据进行解析处理
	int nErr = dataProcesser.process(szIP, responseData);
	// 解析结果 nErr
	if (nErr != 0)
	{
		LOG_(LOGID_NETWORK, LOG_F("[recv]客户端[%s:%d]发送的数据解析失败，errcode=%d"), LOG_STR(strIP), nPort, nErr);
	}
	if (responseData.size() == 0)
	{
		LOG_(LOGID_NETWORK, LOG_F("[recv]客户端[%s:%d]发送的数据解析后不进行应答响应"), LOG_STR(strIP), nPort, nErr);
	}
	else
	{
		// 如果不为空，表示返回应答数据
		int nFlag = lpSock->Send(responseData.data(), responseData.length(), 0);
		if (nFlag == -1 || nFlag < responseData.length())
		{
			LOG_(LOGID_NETWORK, LOG_F("[send]向客户端[%s:%d]发送应答响应失败 flg=%d."), LOG_STR(strIP), nPort, nFlag);
		}
		else
		{
			LOG_(LOGID_NETWORK, LOG_F("[send]向客户端[%s:%d]发送应答响应成功."), LOG_STR(strIP), nPort);
		}
	}

	return true;
}

/************************************************************************/
/* 连接异常处理                                                         */
/************************************************************************/
int TaskTcpServer::OnException(CSocket *lpSock, int nErrorCode)
{
	if (lpSock == NULL)
		return 0;

	if (m_hasClient.contains(lpSock))
	{
		QString strIP = m_hasClient.value(lpSock);
		LOG_(LOGID_NETWORK, LOG_F("客户端[%s]连接断开"), LOG_STR(strIP));
	}

	// 从客户端列表中删除
	m_mutexClient.lock();
	m_hasClient.remove(lpSock);
	m_mutexClient.unlock();

	// 断开连接
	CSocketFactory::Close(lpSock);

	return 1;
}

/************************************************************************/
/* 接收到一条连接                                                       */
/************************************************************************/
int TaskTcpServer::OnAccept(CSocket *lpSock)
{
	CSocketBase *pNewSock = Accept(lpSock);
	if (pNewSock)
	{
		char szIP[32];
		int nPort = 0;
		memset(szIP, 0x00, sizeof(szIP));
		pNewSock->GetPeerName(szIP, nPort);

		QString strIP = QString::fromLocal8Bit(szIP);
		LOG_(LOGID_NETWORK, LOG_F("客户端[%s:%d]建立连接"), LOG_STR(strIP), nPort);

		// Key
		QString strValue = QString("%1:%2").arg(szIP).arg(nPort);

		// 添加到列表中
		m_mutexClient.lock();
		m_hasClient.insert(pNewSock, strValue);
		m_mutexClient.unlock();

		return 1;
	}

	return 0;
}

/************************************************************************/
/* 广播数据                                                             */
/************************************************************************/
void TaskTcpServer::Broadcast(char * szSendMsg, int nLen)
{
	printf("Broadcast >>>>>>>>>>>>>>> %d >>>>\n", m_hasClient.size());
	LOG_(LOGID_NETWORK, LOG_F("[Broadcast]发送广播消息开始 客户端个数=%d."), m_hasClient.size());

	// 锁
	QMutexLocker locker(&m_mutexClient);
	
	// 遍历客户端，广播消息
	HASH_SOCKET::iterator iter;
	for (iter = m_hasClient.begin(); iter != m_hasClient.end(); iter++)
	{
		CSocketBase *lpSock = iter.key();
		QString strIP = iter.value();
		if (lpSock == NULL)
		{
			continue;
		}

		try
		{
			// 发送数据
			int nFlag = lpSock->Send(szSendMsg, nLen, 0);
			if (nFlag == -1 || nFlag < nLen)
			{
				LOG_(LOGID_NETWORK, LOG_F("[Broadcast]向客户端[%s]发送广播消息失败 flg=%d."), LOG_STR(strIP), nFlag);
			}
			else
			{
				LOG_(LOGID_NETWORK, LOG_F("[Broadcast]向客户端[%s]发送广播消息成功."), LOG_STR(strIP));
			}
		}
		catch (...)
		{
			LOG_(LOGID_NETWORK, LOG_F("[Broadcast]向客户端[%s]发送广播消息出现异常."), LOG_STR(strIP));
		}
	}

}


