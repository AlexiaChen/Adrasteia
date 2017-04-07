#ifndef NWFDTCPSERVER_H
#define NWFDTCPSERVER_H

#include "nwfdtcpsocket.h"
#include <QTcpServer>

/**********************************************************************************************//**
 * \class   NwfdTcpServer
 *
 * \brief   A nwfd TCP server.
 *
 * \author  Jeff
 * \date    2015/3/27
 **************************************************************************************************/

class NwfdTcpServer : public QTcpServer
{
	Q_OBJECT

public:

	NwfdTcpServer(QObject *parent, int numConnections = 10000);
	~NwfdTcpServer();

    void start();
	void start(int nListenPort); // 启动某个端口的监听

    /*停止侦听服务*/
    void stop();

    /*重写设置最大连接数函数*/
	void setMaxPendingConnections(int numConnections);

signals:
	void connectClient(const int, const QString &, const quint16);//发送新用户连接信息
	void readData(const int, const QString &, quint16, const QByteArray &);//发送获得用户发过来的数据
	void sockDisConnect(int, QString, quint16);//断开连接的用户信息
	void sentData(const QByteArray &, const int);//向scoket发送消息
	void sentDisConnect(int i); //断开特定连接，并释放资源，-1为断开所有。

public slots:
	void clear(); //断开所有连接，线程计数器请0

protected slots:
	void sockDisConnectSlot(int handle, const QString & ip, quint16 prot, QThread *th);//断开连接的用户信息

protected:
	void incomingConnection(qintptr socketDescriptor);//覆盖已获取多线程

private:
	QHash<int, NwfdTcpSocket *> * tcpClient;//管理连接的map
	int maxConnections;
};

#endif // NWFDTCPSERVER_H
