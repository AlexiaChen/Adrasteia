#ifndef TRMS_CONSOLE_H
#define TRMS_CONSOLE_H

#include <QObject>
#include<QTcpSocket>
#include <QTimer>

#include "MessageDef.h"

class ClientDecoder;

class TRMSConsole : public QObject
{
    Q_OBJECT

public:
    TRMSConsole(QObject *parent = NULL);
    ~TRMSConsole();

    bool login();

private slots:

    void revData(); 
    void sendTask();

    void displayError(QAbstractSocket::SocketError);

private:
    QTcpSocket *m_tcpSocket;
    QTimer m_sendTaskTimer;
    ClientDecoder* m_decoder;
    TCP_CLIENT_INFO m_client_info;

    bool m_isLogin;
};

#endif
