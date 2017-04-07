#ifndef NWFDTCPSOCKET_H
#define NWFDTCPSOCKET_H

#include <QTcpSocket>
#include <QQueue>
#include <QFutureWatcher>
#include <QByteArray>

#include <QTime>

class NwfdTcpSocket : public QTcpSocket
{
	Q_OBJECT

public:
	NwfdTcpSocket(qintptr socketDescriptor, QObject *parent = 0);
	~NwfdTcpSocket();

	//用来处理数据的函数
	QByteArray handleData(QByteArray data, const QString & ip, qint16 port);
	int sendData(const char* message, int nsize);

signals:
	void sockDisConnect(const int, const QString &, const quint16, QThread *);//NOTE:断开连接的用户信息，此信号必须发出！线程管理类根据信号计数的
	
public slots:
	void disConTcp(int i);

protected slots:
	void readData();//接收数据
	void startNext();//处理下一个
	

private slots:
	void slotdisconnect();
	
protected:
	QFutureWatcher<QByteArray> watcher;
	QQueue<QByteArray> datas;

private:
	qintptr socketID;
	QMetaObject::Connection dis;
	
};

#endif // NWFDTCPSOCKET_H
