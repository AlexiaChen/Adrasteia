#include "nwfdtcpserver.h"
#include "nwfdthreadhandle.h"

NwfdTcpServer::NwfdTcpServer(QObject *parent, int numConnections)
	: QTcpServer(parent)
{
	tcpClient = new QHash<int, NwfdTcpSocket *>;
	setMaxPendingConnections(numConnections);
}

NwfdTcpServer::~NwfdTcpServer()
{
	emit this->sentDisConnect(-1);
	delete tcpClient;
}

/************************************************************************/
/* 启动侦听服务                                                         */
/************************************************************************/
void NwfdTcpServer::start()
{
    this->listen(QHostAddress::Any, 1234);
}

void NwfdTcpServer::start(int nListenPort)
{
	this->listen(QHostAddress::Any, nListenPort);
}

/************************************************************************/
/* 设置最大连接数                                                         */
/************************************************************************/
void NwfdTcpServer::setMaxPendingConnections(int numConnections)
{
	this->QTcpServer::setMaxPendingConnections(numConnections);//调用Qtcpsocket函数，设置最大连接数，主要是使maxPendingConnections()依然有效
	this->maxConnections = numConnections;
}

/************************************************************************/
/* 接收链接，并将其放到多线程里                                         */
/************************************************************************/
void NwfdTcpServer::incomingConnection(qintptr socketDescriptor) //多线程必须在此函数里捕获新连接
{
	printf("incomingConnection\n");
	if (tcpClient->size() > maxPendingConnections())//继承重写此函数后，QTcpServer默认的判断最大连接数失效，自己实现
	{
		QTcpSocket tcp;
		tcp.setSocketDescriptor(socketDescriptor);
		tcp.disconnectFromHost();
		return;
	}

	printf("incomingConnection  1\n");
	QThread* th = NwfdThreadHandle::getClass().getThread();
	NwfdTcpSocket* tcpTemp = new NwfdTcpSocket(socketDescriptor);
	printf("incomingConnection  2\n");
	QString ip = tcpTemp->peerAddress().toString();
	qint16 port = tcpTemp->peerPort();
	printf("incomingConnection ip=%s,port=%d\n", ip.toLocal8Bit().data(), port);

	connect(tcpTemp, &NwfdTcpSocket::sockDisConnect, this, &NwfdTcpServer::sockDisConnectSlot); //NOTE:断开连接的处理，从列表移除，并释放断开的Tcpsocket，此槽必须实现，线程管理计数也是考的他
	connect(this, &NwfdTcpServer::sentDisConnect, tcpTemp, &NwfdTcpSocket::disConTcp);          //断开信号

	tcpTemp->moveToThread(th);//把tcp类移动到新的线程，从线程管理类中获取
	tcpClient->insert(socketDescriptor, tcpTemp);//插入到连接信息中

	emit connectClient(socketDescriptor, ip, port);
}

/************************************************************************/
/* 断开连接的用户信息                                                      */
/************************************************************************/
void NwfdTcpServer::sockDisConnectSlot(int handle, const QString & ip, quint16 prot, QThread * th)
{
	tcpClient->remove(handle);//连接管理中移除断开连接的socket
	NwfdThreadHandle::getClass().removeThread(th); //告诉线程管理类那个线程里的连接断开了
	// emit sockDisConnect(handle, ip, prot);
}

/************************************************************************/
/* 断开所有连接，线程计数器请0                                              */
/************************************************************************/
void NwfdTcpServer::clear()
{
	emit this->sentDisConnect(-1);
	NwfdThreadHandle::getClass().clear();
	tcpClient->clear();
}
