#include "log.h"
#include "nwfdtcpsocket.h"
#include "DataProcesser.h"
#include <QtConcurrent/QtConcurrent>
#include <QHostAddress>
#include <QDebug>

NwfdTcpSocket::NwfdTcpSocket(qintptr socketDescriptor, QObject *parent) 
	: QTcpSocket(parent)
{
	this->setSocketDescriptor(socketDescriptor);

	connect(this, &NwfdTcpSocket::readyRead, this, &NwfdTcpSocket::readData);

	/*
	dis = connect(this, &NwfdTcpSocket::disconnected,
		[&](){
		qDebug() << "disconnect " << this->peerAddress().toString() <<":" << this->peerPort();
		emit sockDisConnect(socketID, this->peerAddress().toString(), this->peerPort(), QThread::currentThread());//发送断开连接的用户信息
		this->deleteLater();
	});
	*/
	
	//dis =  connect(this, SIGNAL(NwfdTcpSocket::disconnected), this, SLOT(slotdisconnect()));
	connect(this, SIGNAL(NwfdTcpSocket::disconnected), this, SLOT(slotdisconnect()));
	
	connect(&watcher, &QFutureWatcher<QByteArray>::finished, this, &NwfdTcpSocket::startNext);
	connect(&watcher, &QFutureWatcher<QByteArray>::canceled, this, &NwfdTcpSocket::startNext);

	//qDebug() << "new connect" << this->peerAddress().toString() << ":" << this->peerPort();
	LOG_(LOGID_NETWORK, LOG_F("客户端[%s:%d]建立连接"), LOG_STR(this->peerAddress().toString()), this->peerPort());

	printf("NwfdTcpSocket\n");
}

NwfdTcpSocket::~NwfdTcpSocket()
{

}

void NwfdTcpSocket::slotdisconnect()
{
	//qDebug() << "disconnect " << this->peerAddress().toString() << ":" << this->peerPort();

	LOG_(LOGID_NETWORK, LOG_F("客户端[%s:%d]断开连接"), LOG_STR(this->peerAddress().toString()), this->peerPort());

	//发送断开连接的用户信息
	emit sockDisConnect(socketID, this->peerAddress().toString(), this->peerPort(), QThread::currentThread());
	
	this->deleteLater();
}

void NwfdTcpSocket::disConTcp(int i)
{
	if (i == socketID)
	{
		this->disconnectFromHost();
	}
	else if (i == -1) //-1为全部断开
	{
		//disconnect(dis); //先断开连接的信号槽，防止二次析构
		// todo
		this->disconnectFromHost();
		this->deleteLater();
	}
}

/************************************************************************/
/* 从socket里读出客户端发送来的数据，然后在服务器端做处理               */
/************************************************************************/
void NwfdTcpSocket::readData()
{
	// 解析收到的数据
	QByteArray data = handleData(this->readAll(), this->peerAddress().toString(), this->peerPort());

	if (!data.isNull() && data.size() > 0)
	{
		// 如果不为空，表示返回应答数据
		// todo sendData()
		sendData(data.data(), data.size());
	}
}

/************************************************************************/
/* 该方法是对处理做处理的具体的方法。这个需要回头改成一个独立的方法，拎出去。 */
/************************************************************************/
QByteArray NwfdTcpSocket::handleData(QByteArray data, const QString &ip, qint16 port)
{
	//qDebug() << "[Server] Read data from client[" << ip << ":" << port << "]: [" + data + "]";
	LOG_(LOGID_NETWORK, LOG_F("[recv]接收到客户端[%s:%d]发送的数据"), LOG_STR(this->peerAddress().toString()), this->peerPort());

	// 构造处理器
	DataProcesser dataProcesser(data.data(), data.length());

	// 响应数据
	QByteArray responseData;

	// 对数据进行解析处理
	int nErr = dataProcesser.process(ip, responseData);

	// 解析结果 nErr
	if (nErr != 0)
	{
		LOG_(LOGID_NETWORK, LOG_F("[recv]客户端[%s:%d]发送的数据解析失败，errcode=%d"), LOG_STR(this->peerAddress().toString()), this->peerPort(), nErr);
	}
	if (responseData.size() == 0)
	{
		LOG_(LOGID_NETWORK, LOG_F("[recv]客户端[%s:%d]发送的数据解析后不进行应答响应"), LOG_STR(this->peerAddress().toString()), this->peerPort(), nErr);
	}

	return responseData;
}

void NwfdTcpSocket::startNext()
{
	this->write(watcher.future().result());
	if (!datas.isEmpty())
	{
		watcher.setFuture(QtConcurrent::run(this, &NwfdTcpSocket::handleData, datas.dequeue(), this->peerAddress().toString(), this->peerPort()));
	}
}


int NwfdTcpSocket::sendData(const char* message, int nsize)
{
	if (this->state() == QAbstractSocket::ConnectedState)
	{
		qint64 ret = this->writeData(message, nsize);
		qDebug() << "[Client] Send Message length: " + QString::number(ret) << " : " << message;
		LOG_(LOGID_NETWORK, LOG_F("[send]向客户端[%s:%d]发送返回数据"), LOG_STR(this->peerAddress().toString()), this->peerPort());
	}

	return -1;
}


