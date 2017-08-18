#include "TRMSConsole.h"

#include <QString>
#include <QByteArray>
#include <QDebug>
#include <QSettings>

#include <QtCore/QCoreApplication>

#include <QThread>

#ifdef __linux__
#include <arpa/inet.h>
#include "zlib.h"
#endif

#include <cstdlib>
#include <cstdio>
#include <ctime>

#ifndef __linux__
#include<Winsock2.h>
#pragma comment(lib,"ws2_32.lib")
#endif

#include "Utils.h"
#include "ClientDecoder.h"
#include "MessageDef.h"

static uint32_t setFlag(bool isSuccess, bool isCompressed, bool isCypt){

    uint32_t flag = 0;

    if (!isSuccess)
    {
        flag = flag | 0x1;
    }

    if (isCompressed)
    {
        flag = flag | (0x1 << 2);
    }

    if (isCypt)
    {
        flag = flag | (0x1 << 3);
    }
    return flag;
}

static void initClientInf(TCP_CLIENT_INFO& info){

    info.custom_socket = NULL;
    info.LoginTime.assign("");
    info.LastSendTime = 0;
    info.SendTime = 0;
    info.PackageType = 0;
    info.CUR_STEP = RECV_HEAD;
    info.CUR_IDX = 0;
    memset(&info.bFieldBuf, 0, 16);
    info.iPackageFLAG = 0;
    info.iLength = 0;
    info.iTrue_Length = 0;
    info.msRecv.clear(); info.msRecv.shrink_to_fit();
    info.IPAddress.assign("");
    info.MAC.assign("");
    info.OSBootTime.assign("");
    info.BaseInfo.assign("");
    info.DiskSpaceInfo.assign("");
}


TRMSConsole::TRMSConsole(QObject *parent)
    : QObject(parent),
    m_tcpSocket(new QTcpSocket(this)),
    m_isLogin(false)

{
    
    qDebug() << "TRMSConsole entry";

    m_decoder = new ClientDecoder(m_isLogin);

    initClientInf(m_client_info);
    m_client_info.custom_socket = m_tcpSocket;

    //ini read
    QSettings iniSettings("config.ini", QSettings::IniFormat);
    QString ip = iniSettings.value("/main/ip").toString();
    QString port = iniSettings.value("/main/port").toString();

    qDebug() << "connectting host is: " << ip;
    qDebug() << "connectting port is: " << port;
    
    m_tcpSocket->connectToHost(ip,port.toInt());


    connect(m_tcpSocket, SIGNAL(readyRead()), this, SLOT(revData()));

    connect(m_tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),

        this, SLOT(displayError(QAbstractSocket::SocketError)));

    while (!login())
    {
        QThread::msleep(3000); //签到失败就每隔3秒重试签到，直到签到成功才有状态会送
        qDebug() << "login fails and reconnecting";
    }
    
    m_sendTaskTimer.setInterval(5000);
    connect(&m_sendTaskTimer, SIGNAL(timeout()), this, SLOT(sendTask()));
    m_sendTaskTimer.start();
    qDebug() << "login success and Starting send Task";
    

    qDebug() << "TRMSConsole leave";
}

TRMSConsole::~TRMSConsole()
{
    if (m_tcpSocket) {
        
        delete m_tcpSocket;
    }

    if (m_decoder)
    {
        delete m_decoder;
    }


}

void TRMSConsole::revData()
{
    qDebug() << " socket data arrived";

    if (m_tcpSocket->bytesAvailable() <= 0) return;
    
    QByteArray recvBuffer = m_tcpSocket->readAll();
    m_decoder->addRecvBuf((uint8_t*)recvBuffer.data(), recvBuffer.size());
    
    //粘包拆包完毕返回true
    if (m_decoder->decodeData(&m_client_info))
    {
        m_decoder->decodePackage(&m_client_info);
        size_t total_size_msg = m_client_info.iLength + sizeof(MsgHeader);
        m_decoder->clearRecvBufFirst(total_size_msg);
        initClientInf(m_client_info);
    }
}

void TRMSConsole::displayError(QAbstractSocket::SocketError  sockErr)
{ 
    qDebug() << "Socket Error";
    qDebug() << "App is Quiting now";
    qApp->exit();
}

void TRMSConsole::sendTask()
{
    qDebug() << "Send Task";

    qDebug() << "OS Last startup time : " << Utils::lastStartupTime();

    //qDebug() << "Hard Disk Usage" << Utils::hardDiskUsage();

    qDebug() << "Local IP: " << Utils::localIP();

    qDebug() << "Local MAC: " << Utils::localMAC();

    qDebug() << "CPU Usage: " << Utils::cpuUsage();

    qDebug() << "MEM Usage: " << Utils::memUsage();

    SubMsg1003 sub_msg;
    memset(&sub_msg, 0, sizeof(SubMsg1003));
    strcpy(sub_msg.ip, Utils::localIP().toStdString().c_str());
    strcpy(sub_msg.mac, Utils::localMAC().toStdString().c_str());
    strcpy(sub_msg.os_boot_time, Utils::lastStartupTime().toStdString().c_str());
    strcpy(sub_msg.cpu_usage, Utils::cpuUsage().toStdString().c_str());
    strcpy(sub_msg.mem_usage, Utils::memUsage().toStdString().c_str());
    strcpy(sub_msg.disk_space, Utils::hardDiskUsage().toStdString().c_str());
    
    uint32_t sub_size = sizeof(SubMsg1003);

    MsgHeader header;
    memset(&header, 0, sizeof(MsgHeader));
    header.head = htons(0x81);
    header.sender = htons(0x03);
    header.sendtime = htonl(time(NULL) * 1000);
    strcpy((char*)&header.type, std::to_string(1003).c_str());
    header.length = htonl(sub_size);
    header.true_length = htonl(sub_size);
    header.flag = htonl(setFlag(true, false, false));

    uint8_t raw_data[sizeof(MsgHeader) + sizeof(SubMsg1003)];

    memcpy(raw_data, (char*)&header, sizeof(MsgHeader));
    memcpy(raw_data + sizeof(MsgHeader), (char*)&sub_msg, sizeof(SubMsg1003));

    QByteArray sendBuffer;

    sendBuffer.fromRawData((char*)raw_data, sizeof(MsgHeader) + sizeof(SubMsg1003));
    m_tcpSocket->write(sendBuffer);
}


bool TRMSConsole::login()
{
    //send login msg

    qDebug() << "login() entry";
    SubMsg1001 sub_msg;
    memset(&sub_msg, 0, sizeof(SubMsg1001));
    strcpy(sub_msg.ip, Utils::localIP().trimmed().toStdString().c_str());
    qDebug() << "localIP() leave";
    strcpy(sub_msg.mac, Utils::localMAC().trimmed().toStdString().c_str());
    qDebug() << "localMAC() leave";
    qDebug() << Utils::lastStartupTime().trimmed().toStdString().c_str();
    strcpy(sub_msg.os_boot_time, Utils::lastStartupTime().trimmed().toStdString().c_str());
    qDebug() << "lastStartupTime() leave";
    qDebug() << Utils::processesInf().trimmed().toStdString().c_str();
    strcpy(sub_msg.base_info, Utils::processesInf().trimmed().toStdString().c_str());
    qDebug() << "processesInf() leave";

    uint32_t sub_size = sizeof(SubMsg1001);

    MsgHeader header;
    memset(&header, 0, sizeof(MsgHeader));
    header.head = htons(0x81);
    header.sender = htons(0x03);
    header.sendtime = htonl(time(NULL)*1000);
    strcpy((char*)&header.type, std::to_string(1001).c_str());
    header.length = htonl(sub_size);
    header.true_length = htonl(sub_size);
    header.flag = htonl(setFlag(true, false, false));

    uint8_t raw_data[sizeof(MsgHeader) + sizeof(SubMsg1001)];
    
    memcpy(raw_data, (char*)&header, sizeof(MsgHeader));
    memcpy(raw_data + sizeof(MsgHeader), (char*)&sub_msg, sizeof(SubMsg1001));
    
    QByteArray sendBuffer;

    sendBuffer.fromRawData((char*)raw_data, sizeof(MsgHeader) + sizeof(SubMsg1001));
    m_tcpSocket->write(sendBuffer);

    qDebug() << "login() leave";

    return m_isLogin;
}
