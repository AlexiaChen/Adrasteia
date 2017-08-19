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
#include <cstdint>

#ifndef __linux__
#include<Winsock2.h>
#pragma comment(lib,"ws2_32.lib")
#endif

#include "Utils.h"
#include "p_TabStruct.h"

static void initClientInf(TCLIENT_INFO& info){

    info.Socket = NULL;
    info.LoginTime = 0;
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
    info.CPU = 0;
    info.Memory = 0;
}



TRMSConsole::TRMSConsole(QObject *parent)
    : QObject(parent),
    m_tcpSocket(new QTcpSocket(this)),
    m_isRecvEnd(false)

{
    
    qDebug() << "TRMSConsole entry";

    initClientInf(m_client_info);

    //ini read
    QSettings iniSettings("config.ini", QSettings::IniFormat);
    QString ip = iniSettings.value("/main/ip").toString();
    QString port = iniSettings.value("/main/port").toString();

    qDebug() << "connectting host is: " << ip;
    qDebug() << "connectting port is: " << port;

    initSiganlSlot();
    
    m_tcpSocket->connectToHost(ip, port.toInt());
    
    m_sendTaskTimer.setInterval(5000);
    connect(&m_sendTaskTimer, SIGNAL(timeout()), this, SLOT(sendTask()));
    m_sendTaskTimer.start();
    

    qDebug() << "TRMSConsole leave";
}

TRMSConsole::~TRMSConsole()
{
    if (m_tcpSocket) {
        
        delete m_tcpSocket;
    }
}

void TRMSConsole::revData()
{
    qDebug() << " socket data arrived";

    if (m_tcpSocket->bytesAvailable() <= 0) return;
    
    QByteArray recvBuffer = m_tcpSocket->readAll();
   //���
    AnaData((uint8_t*)recvBuffer.data(), recvBuffer.size(), &m_client_info, m_isRecvEnd);
    if (m_isRecvEnd)
    {
        AnaPackage(&m_client_info);
        initClientInf(m_client_info);
        m_isRecvEnd = false;
    }
}

void TRMSConsole::displayError(QAbstractSocket::SocketError  sockErr)
{ 
    qDebug() << "Socket Error";

    //ini read
    QSettings iniSettings("config.ini", QSettings::IniFormat);
    QString ip = iniSettings.value("/main/ip").toString();
    QString port = iniSettings.value("/main/port").toString();

    qDebug() << "re-connectting host is: " << ip;
    qDebug() << "re-connectting port is: " << port;

    m_tcpSocket->close();
    disconnectSinalSlot();
    initSiganlSlot();
    m_tcpSocket->connectToHost(ip, port.toInt());
}

void TRMSConsole::sendTask()
{
    //�ͻ���״̬����
    sendClientState();
  
}


void  TRMSConsole::login()
{
    //send login msg

    qDebug() << "login() entry";
   
    
    TCLIENT_LOGIN tLogin;
    p_InitTab_Client_Login(&tLogin);
    strcpy(tLogin.IPAddress, Utils::localIP().trimmed().toStdString().c_str());
    strcpy(tLogin.MAC, Utils::localMAC().trimmed().toStdString().c_str());
    strcpy(tLogin.OSBootTime, Utils::lastStartupTime().trimmed().toStdString().c_str());

    strncpy(tLogin.BaseInfo, Utils::processesInf().trimmed().toStdString().c_str(), 4096);

    int iSendBufLen = CLIENT_LOGIN_LEN + 1;

    char * cSendBuf = new char[iSendBufLen];
    p_TabPack_Client_Login(cSendBuf, iSendBufLen, &tLogin);
    int iPackageLength = iSendBufLen + SYS_HEAD_LEN;

    uint8_t * bPackageData = new uint8_t[iPackageLength];

    
    CreatPackage("1001", cSendBuf, iSendBufLen, &bPackageData, iPackageLength);
    SendData(m_tcpSocket,bPackageData, iPackageLength);

    delete[] cSendBuf;
    delete[] bPackageData;

    qDebug() << "login() leave";

    return;
}

void TRMSConsole::slotConnected()
{
    qDebug() << "connected";
    login();
}

void TRMSConsole::slotDisConnected()
{
    
    qDebug() << "socket disconnected, re-login";

    //ini read
    QSettings iniSettings("config.ini", QSettings::IniFormat);
    QString ip = iniSettings.value("/main/ip").toString();
    QString port = iniSettings.value("/main/port").toString();

    qDebug() << "re-connectting host is: " << ip;
    qDebug() << "re-connectting port is: " << port;

    m_tcpSocket->close();
    disconnectSinalSlot();
    initSiganlSlot();
    m_tcpSocket->connectToHost(ip, port.toInt());
}

void TRMSConsole::sendClientState()
{

    qDebug() << "sendClientState() entry";

    TCLIENT_STATE tState;
    p_InitTab_Client_State(&tState);
    strcpy(tState.IPAddress, Utils::localIP().trimmed().toStdString().c_str());
    strcpy(tState.MAC, Utils::localMAC().trimmed().toStdString().c_str());
    strcpy(tState.OSBootTime, Utils::lastStartupTime().trimmed().toStdString().c_str());
    strcpy(tState.UseCPU, Utils::cpuUsage().trimmed().toStdString().c_str());
    strcpy(tState.UserMemory, Utils::memUsage().trimmed().toStdString().c_str());
    strncpy(tState.DiskSpaceInfo, Utils::hardDiskUsage().trimmed().toStdString().c_str(),1024);
    

    int iSendBufLen = CLIENT_STATE_LEN + 1;

    char * cSendBuf = new char[iSendBufLen];
    p_TabPack_Client_State(cSendBuf, iSendBufLen, &tState);
    
    int iPackageLength = iSendBufLen + SYS_HEAD_LEN;
    uint8_t * bPackageData = new uint8_t[iPackageLength];


    CreatPackage("1002", cSendBuf, iSendBufLen, &bPackageData, iPackageLength);
    SendData(m_tcpSocket, bPackageData, iPackageLength);

    delete[] cSendBuf;
    delete[] bPackageData;

    qDebug() << "sendClientState() leave";

    return;
}

void TRMSConsole::p_InitTab_Client_Login(TCLIENT_LOGIN *P)
{
    memset(P->IPAddress, '\0', 17);
    strcat(P->IPAddress, " ");
    memset(P->MAC, '\0', 19);
    strcat(P->MAC, " ");
    memset(P->OSBootTime, '\0', 21);
    strcat(P->OSBootTime, " ");
    memset(P->BaseInfo, '\0', 4097);
    strcat(P->BaseInfo, " ");
}

void TRMSConsole::p_InitTab_Client_State(TCLIENT_STATE *P)
{
    memset(P->IPAddress, '\0', 17);
    strcat(P->IPAddress, " ");
    memset(P->MAC, '\0', 19);
    strcat(P->MAC, " ");
    memset(P->OSBootTime, '\0', 21);
    strcat(P->OSBootTime, " ");
    memset(P->UseCPU, '\0', 4);
    strcat(P->UseCPU, " ");
    memset(P->UserMemory, '\0', 4);
    strcat(P->UserMemory, " ");
    memset(P->DiskSpaceInfo, '\0', 1025);
    strcat(P->DiskSpaceInfo, " ");
}

void TRMSConsole::p_InitTab_ShutDownCmd(TSHUTDOWNCMD *P)
{
    memset(P->ShutDownCmd, '\0', 1025);
    strcat(P->ShutDownCmd, "shutdown -s -f");
}

int TRMSConsole::p_TabPack_Client_Login(char *cBuf, int iBufLen, TCLIENT_LOGIN *client_login)
{
    if (CLIENT_LOGIN_LEN >= iBufLen)
    {
        /*Ҫ������ַ������ȳ������������ȣ�ֱ�ӷ���ʧ��-1*/
        return -1;
    }
    sprintf(cBuf, "%-16s%-18s%-20s%-4096s", \
        client_login->IPAddress, client_login->MAC, client_login->OSBootTime, client_login->BaseInfo);
    return 0;
}

int TRMSConsole::p_TabUnPack_Client_Login(char *cBuf, TCLIENT_LOGIN* client_login)
{
    int i = 0;
    char cTmpBuf[32];
    if (strlen(cBuf) < CLIENT_LOGIN_LEN)
    {
        /*Ҫ������ַ������Ȳ�����ֱ�ӷ���ʧ��-1*/
        return -1;
    }
    memset(client_login, 0, sizeof(TCLIENT_LOGIN));
    memcpy(client_login->IPAddress, cBuf + i, 16);
    // CutBothEndsSpace(client_login->IPAddress);
    if (strlen(client_login->IPAddress) == 0)
        strcpy(client_login->IPAddress, "NULL");
    i += 16;
    memcpy(client_login->MAC, cBuf + i, 18);
    // CutBothEndsSpace(client_login->MAC);
    if (strlen(client_login->MAC) == 0)
        strcpy(client_login->MAC, "NULL");
    i += 18;
    memcpy(client_login->OSBootTime, cBuf + i, 20);
    // CutBothEndsSpace(client_login->OSBootTime);
    if (strlen(client_login->OSBootTime) == 0)
        strcpy(client_login->OSBootTime, "NULL");
    i += 20;
    memcpy(client_login->BaseInfo, cBuf + i, 4096);
    // CutBothEndsSpace(client_login->BaseInfo);
    if (strlen(client_login->BaseInfo) == 0)
        strcpy(client_login->BaseInfo, "NULL");
    return 0;
}

int TRMSConsole::p_TabPack_Client_State(char *cBuf, int iBufLen, TCLIENT_STATE *client_state)
{
    if (CLIENT_STATE_LEN >= iBufLen)
    {
        /*Ҫ������ַ������ȳ������������ȣ�ֱ�ӷ���ʧ��-1*/
        return -1;
    }
    sprintf(cBuf, "%-16s%-18s%-20s%-3s%-3s%-1024s", \
        client_state->IPAddress, client_state->MAC, client_state->OSBootTime, client_state->UseCPU, client_state->UserMemory, client_state->DiskSpaceInfo);
    return 0;
}

int TRMSConsole::p_TabUnPack_Client_State(char *cBuf, TCLIENT_STATE* client_state)
{
    int i = 0;
    char cTmpBuf[32];
    if (strlen(cBuf) < CLIENT_STATE_LEN)
    {
        /*Ҫ������ַ������Ȳ�����ֱ�ӷ���ʧ��-1*/
        return -1;
    }
    memset(client_state, 0, sizeof(TCLIENT_STATE));
    memcpy(client_state->IPAddress, cBuf + i, 16);
    // CutBothEndsSpace(client_state->IPAddress);
    if (strlen(client_state->IPAddress) == 0)
        strcpy(client_state->IPAddress, "NULL");
    i += 16;
    memcpy(client_state->MAC, cBuf + i, 18);
    // CutBothEndsSpace(client_state->MAC);
    if (strlen(client_state->MAC) == 0)
        strcpy(client_state->MAC, "NULL");
    i += 18;
    memcpy(client_state->OSBootTime, cBuf + i, 20);
    // CutBothEndsSpace(client_state->OSBootTime);
    if (strlen(client_state->OSBootTime) == 0)
        strcpy(client_state->OSBootTime, "NULL");
    i += 20;
    memcpy(client_state->UseCPU, cBuf + i, 3);
    //CutBothEndsSpace(client_state->UseCPU);
    if (strlen(client_state->UseCPU) == 0)
        strcpy(client_state->UseCPU, "NULL");
    i += 3;
    memcpy(client_state->UserMemory, cBuf + i, 3);
    ///CutBothEndsSpace(client_state->UserMemory);
    if (strlen(client_state->UserMemory) == 0)
        strcpy(client_state->UserMemory, "NULL");
    i += 3;
    memcpy(client_state->DiskSpaceInfo, cBuf + i, 1024);
    //CutBothEndsSpace(client_state->DiskSpaceInfo);
    if (strlen(client_state->DiskSpaceInfo) == 0)
        strcpy(client_state->DiskSpaceInfo, "NULL");
    return 0;
}

int TRMSConsole::p_TabPack_ShutDownCmd(char *cBuf, int iBufLen, TSHUTDOWNCMD *shutdowncmd)
{
    if (SHUTDOWNCMD_LEN >= iBufLen)
    {
        /*Ҫ������ַ������ȳ������������ȣ�ֱ�ӷ���ʧ��-1*/
        return -1;
    }
    sprintf(cBuf, "%-1024s", \
        shutdowncmd->ShutDownCmd);
    return 0;
}

int TRMSConsole::p_TabUnPack_ShutDownCmd(char *cBuf, TSHUTDOWNCMD* shutdowncmd)
{
    int i = 0;
    char cTmpBuf[32];
    if (strlen(cBuf) < SHUTDOWNCMD_LEN)
    {
        /*Ҫ������ַ������Ȳ�����ֱ�ӷ���ʧ��-1*/
        return -1;
    }
    memset(shutdowncmd, 0, sizeof(TSHUTDOWNCMD));
    memcpy(shutdowncmd->ShutDownCmd, cBuf + i, 1024);
    //CutBothEndsSpace(shutdowncmd->ShutDownCmd);
    if (strlen(shutdowncmd->ShutDownCmd) == 0)
        strcpy(shutdowncmd->ShutDownCmd, "NULL");
    return 0;
}

void TRMSConsole::AnaData(uint8_t * bBuffer, int iBufLength, TCLIENT_INFO *CLIENT_INFO, bool &bRecvEnd)
{
    uint8_t bChar;
    std::vector<uint8_t> &msRecv = CLIENT_INFO->msRecv;

    for (int i = 0; i < iBufLength; i++) {
        bChar = bBuffer[i];
        switch (CLIENT_INFO->CUR_STEP) {
        case RECV_HEAD: // ����ͷ�ж�
            if (bChar == 0x81) {
                CLIENT_INFO->CUR_STEP = RECV_SENDER;
            }
            break;
        case RECV_SENDER: // �������жϣ��ͻ���ֻ�ܽ��ܽڵ���������͵�����
            /* Ŀǰ�������жϴ��� */
            /*
            if ( bChar != '1' )
            {
            server->CUR_STEP = RECV_HEAD
            return;
            } */
            CLIENT_INFO->CUR_STEP = RECV_RECEIVER;
            break;
        case RECV_RECEIVER: // �������жϣ������߱������Լ�Ҳ����˵��
            /* Ŀǰ�������жϴ��� */
            /*
            if ( bChar != '3' )
            {
            CUR_STEP = RECV_HEAD
            return;
            } */
            CLIENT_INFO->CUR_STEP = RECV_SENDTIME;
            break;
        case RECV_SENDTIME: // ��ȡʱ���
            CLIENT_INFO->bFieldBuf[CLIENT_INFO->CUR_IDX++] = bChar;
            if (CLIENT_INFO->CUR_IDX == 4) {
                CLIENT_INFO->bFieldBuf[CLIENT_INFO->CUR_IDX] = 0;
                memcpy(&(CLIENT_INFO->SendTime), CLIENT_INFO->bFieldBuf,
                    CLIENT_INFO->CUR_IDX);
                CLIENT_INFO->SendTime = ntohl(CLIENT_INFO->SendTime);
                CLIENT_INFO->CUR_STEP = RECV_TYPE;
                CLIENT_INFO->CUR_IDX = 0;
            }
            break;
        case RECV_TYPE: // �ж����ݰ�����
            CLIENT_INFO->bFieldBuf[CLIENT_INFO->CUR_IDX++] = bChar;
            if (CLIENT_INFO->CUR_IDX == 6) {
                CLIENT_INFO->bFieldBuf[CLIENT_INFO->CUR_IDX] = 0;
                CLIENT_INFO->CUR_IDX = 0;
                CLIENT_INFO->PackageType = atoi((char*)(CLIENT_INFO->bFieldBuf));
                CLIENT_INFO->CUR_STEP = RECV_LENGTH;
            }
            break;
        case RECV_LENGTH: // ��ȡ���ݳ���
            CLIENT_INFO->bFieldBuf[CLIENT_INFO->CUR_IDX++] = bChar;
            if (CLIENT_INFO->CUR_IDX == 4) {
                CLIENT_INFO->bFieldBuf[CLIENT_INFO->CUR_IDX] = 0;
                memcpy(&(CLIENT_INFO->iLength), CLIENT_INFO->bFieldBuf,
                    CLIENT_INFO->CUR_IDX);
                CLIENT_INFO->iLength = ntohl(CLIENT_INFO->iLength);
                CLIENT_INFO->CUR_STEP = RECV_TRUE_LENGTH;
                CLIENT_INFO->CUR_IDX = 0;
            }
            break;
        case RECV_TRUE_LENGTH: // ���ݵ���ʵ���ȣ������ѹ���������ֶ�Ϊѹ��ǰ����
            CLIENT_INFO->bFieldBuf[CLIENT_INFO->CUR_IDX++] = bChar;
            if (CLIENT_INFO->CUR_IDX == 4) {
                CLIENT_INFO->bFieldBuf[CLIENT_INFO->CUR_IDX] = 0;
                memcpy(&(CLIENT_INFO->iTrue_Length), CLIENT_INFO->bFieldBuf,
                    CLIENT_INFO->CUR_IDX);
                CLIENT_INFO->iTrue_Length = ntohl(CLIENT_INFO->iTrue_Length);
                CLIENT_INFO->CUR_STEP = RECV_FLAG;
                CLIENT_INFO->CUR_IDX = 0;
            }
            break;
        case RECV_FLAG: // ���ر���־��
            CLIENT_INFO->bFieldBuf[CLIENT_INFO->CUR_IDX++] = bChar;
            if (CLIENT_INFO->CUR_IDX == 4) {
                CLIENT_INFO->bFieldBuf[CLIENT_INFO->CUR_IDX] = 0;
                memcpy(&(CLIENT_INFO->iPackageFLAG), CLIENT_INFO->bFieldBuf,
                    CLIENT_INFO->CUR_IDX);
                CLIENT_INFO->iPackageFLAG = ntohl(CLIENT_INFO->iPackageFLAG);
                CLIENT_INFO->CUR_IDX = 0;
                CLIENT_INFO->CUR_STEP = RECV_DATA;
                // msRecv->Clear();
                msRecv.clear();
            }
            break;
        case RECV_DATA:
            // curStatus = "���ڽ������� "+ IntToStr(msRecv->Size)+"/"+IntToStr(iLength);
            if (/*msRecv->Size*/ msRecv.size() + (iBufLength - i) >= CLIENT_INFO->iLength) {
                // ���ʣ�໺�������Ӱ����ݴ���ֱ�Ӱ��Ӱ����ݽ��н�ȡ
                /// msRecv->WriteBuffer(bBuffer + i,
                //  CLIENT_INFO->iLength - msRecv->Size);

                std::copy_n(bBuffer + i, CLIENT_INFO->iLength - msRecv.size(), std::back_inserter(msRecv));
                CLIENT_INFO->CUR_STEP = RECV_HEAD;
                CLIENT_INFO->CUR_IDX = 0;

                bRecvEnd = true;
                i += CLIENT_INFO->iLength;
            }
            else { // ������ȫ���������ڴ�����
                //msRecv->WriteBuffer(bBuffer + i, iBufLength - i);
                std::copy_n(bBuffer + i, iBufLength - i, std::back_inserter(msRecv));
                i = iBufLength;
            }
            break;
        default:
            break;
        }
    }
}

void TRMSConsole::AnaPackage(TCLIENT_INFO *serviceInfo)
{
    // ���ݰ����ͽ��в�ͬ�Ĵ���
    bool bError = false, bCompress = false, bCrypt = false;
    GetPackageFLAG(serviceInfo->iPackageFLAG, bError, bCompress, bCrypt);
    uint8_t * recvBuf;
    uint8_t * dataBuf;

    recvBuf = new uint8_t[serviceInfo->iTrue_Length + 1];
    memset(recvBuf, 0, serviceInfo->iTrue_Length + 1);

    if (bCompress) { // ˵�����ݰ���ѹ����ģ���Ҫ���н�ѹ��
        dataBuf = new uint8_t[serviceInfo->iLength + 1];

        //serviceInfo->msRecv->Seek(0, 0); // ��λ����ʼλ��
        // serviceInfo->msRecv->ReadBuffer(dataBuf, serviceInfo->msRecv->Size);

        std::copy_n(serviceInfo->msRecv.begin(), serviceInfo->msRecv.size(), dataBuf);

#ifdef __linux__
        size_t iTrue_Length;
        size_t iLength = serviceInfo->iLength;


        int iRet = uncompress((Bytef*)recvBuf, (uLongf*)&iTrue_Length, dataBuf, iLength);
       
        delete[]dataBuf;
        if (iRet != Z_OK) {
            qDebug() << "�������ݽ�ѹʧ��! �����룽" << iRet;
            delete[]recvBuf;
            return;
        }
#endif
    }
    else { // ����δѹ����ֱ�ӿ��������ջ�������
        //serviceInfo->msRecv->ReadBuffer(recvBuf, serviceInfo->msRecv->Size);
        std::copy_n(serviceInfo->msRecv.begin(), serviceInfo->msRecv.size(), recvBuf);
        
    }

    if (bError) { // ˵�����ó��ִ�����Ҫ��ʾ������Ϣ��

        qDebug() << "��ȡ����ʧ�ܣ�ʧ����ϢΪ: " << (char*)recvBuf;
        delete[]recvBuf;
        return;
    }
    // ���ݰ����ͽ��д���
    switch (serviceInfo->PackageType)
    {
    case 2001: // �ͻ���ǩ���ɹ�
        emit logined();
        break;
    case 2002: // �ͻ���ǩ��
        emit loginOut();
        break;
    case 2003: // �ͻ���״̬����
        emit clientState();
        break;
    case 2004: //Զ�̹ػ�
        TSHUTDOWNCMD shut_down_t;
        p_TabUnPack_ShutDownCmd((char*)recvBuf, &shut_down_t);
        emit remoteShutdown(QString(shut_down_t.ShutDownCmd));
    default:
        qDebug() << "Error!����δ֪���ݰ����ͣ�PackageType=" << serviceInfo->PackageType;

    }

    delete[]recvBuf;
}

void TRMSConsole::GetPackageFLAG(uint32_t dwFlag, bool &bError, bool &bCompress, bool &bCrypt)
{
    if ((dwFlag & 0x01) == 0x01)
        bError = true;
    if ((dwFlag & 0x04) == 0x04)
        bCompress = true;
    if ((dwFlag & 0x04) == 0x08)
        bCrypt = true;
}

void TRMSConsole::SetHeadPackage(uint8_t * bHeadBuffer, char * cType, int iSender, int iRecver, int iLength, int iTrue_Length, bool bCompressed)
{
    bHeadBuffer[0] = 0x81; 						// ����ͷ��������
    bHeadBuffer[1] = iSender;
    bHeadBuffer[2] = iRecver;

#ifdef WIN32
    DWORD dwSendTime = GetTickCount();
#endif

#ifdef __linux__
    uint32_t dwSendTime = time(NULL) * 1000;
#endif
    dwSendTime = htonl(dwSendTime);
    memcpy(bHeadBuffer + 3, &dwSendTime, 4); 	// ����ʱ���

    memcpy(bHeadBuffer + 7, cType, 6); 			// �����Ӱ����� 6λ����

    iLength = htonl(iLength);
    memcpy(bHeadBuffer + 13, &iLength, 4); 		// �����Ӱ�����

    iTrue_Length = htonl(iTrue_Length);
    memcpy(bHeadBuffer + 17, &iTrue_Length, 4); // ������ʵ���ȣ�ѹ��ǰ����

    uint32_t dwFlag = 0;
    if (bCompressed)
        dwFlag |= 0x04;
    dwFlag = htonl(dwFlag);
    memcpy(bHeadBuffer + 21, &dwFlag, 4); 		// ���ñ�־
}

void TRMSConsole::CreatPackage(char * cType, char *cSendBuf, int &iSendBufLen, uint8_t ** bPackageData, int &iPackageLength)
{
    bool bCompressed = false;
    // �����Ƿ�ѹ�������ж��Ƿ���Ҫ�Է������ݽ���ѹ��
    unsigned long iDestLen = (iSendBufLen - 12) * 100.1 / 100.0 + 1; // ѹ����Ŀ�곤��
    if (iSendBufLen > START_ZIP_LENGTH) { // �������ݳ��ȳ�������ѹ����ֵ����ѹ��
        *bPackageData = new uint8_t[iDestLen + SYS_HEAD_LEN];
        // ����һ���洢ѹ������Ļ��������ȷ��ͻ�������ͷ�����ȡ�
#ifdef __linux__
        compress((Bytef*)(*bPackageData) + SYS_HEAD_LEN, (uLongf*)&iDestLen, (Bytef*)cSendBuf,
            iSendBufLen); // ѹ��ʱ�ճ�ͷ��λ��
#endif
        bCompressed = true;
    }
    else {
        iDestLen = iSendBufLen;
        *bPackageData = new uint8_t[iDestLen + SYS_HEAD_LEN];
        memcpy(*bPackageData + SYS_HEAD_LEN, cSendBuf, iDestLen);
    }

    SetHeadPackage(*bPackageData, cType, SENDER_MANGER,
        SENDER_WINDOWS | SENDER_LINUX, iDestLen, iSendBufLen, bCompressed);
    iPackageLength = iDestLen + SYS_HEAD_LEN;

    return;
}

int TRMSConsole::SendData(QTcpSocket *Socket, uint8_t * bBuffer, int iDataLength)
{
    if (Socket == NULL || Socket->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "�����ѶϿ������ܷ������ݣ�";
        return -1;
    }

    QByteArray sendBuffer;

    sendBuffer = QByteArray::fromRawData((char*)bBuffer, iDataLength);
    int send_size = Socket->write(sendBuffer);
    Socket->flush();

    return send_size;
}

void TRMSConsole::slotLogined()
{
    qDebug() << "client logined";
}

void TRMSConsole::slotRemoteShutDown(const QString& script)
{
#ifdef WIN32
    qDebug() << "remote shut down machine is received: " << script;
#endif
#ifdef __linux__
    Utils::execCmd2(script);
#endif
}

void TRMSConsole::slotClientState()
{
    sendClientState();
}

void TRMSConsole::slotLoginOut()
{

}

void TRMSConsole::initSiganlSlot()
{
    connect(m_tcpSocket, SIGNAL(readyRead()), this, SLOT(revData()));

    connect(m_tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
        this, SLOT(displayError(QAbstractSocket::SocketError)));

    connect(m_tcpSocket, SIGNAL(connected()), this, SLOT(slotConnected()));
    connect(m_tcpSocket, SIGNAL(disconnected()), this, SLOT(slotDisConnected()));

    connect(this, SIGNAL(logined()), this, SLOT(slotLogined()));
    connect(this, SIGNAL(remoteShutdown(const QString&)), this, SLOT(slotRemoteShutDown(const QString&)));
    connect(this, SIGNAL(clientState()), this, SLOT(slotClientState()));
    connect(this, SIGNAL(loginOut()), this, SLOT(slotLoginOut()));
}

void TRMSConsole::disconnectSinalSlot()
{
    disconnect(m_tcpSocket, SIGNAL(readyRead()), this, SLOT(revData()));

    disconnect(m_tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
        this, SLOT(displayError(QAbstractSocket::SocketError)));

    disconnect(m_tcpSocket, SIGNAL(connected()), this, SLOT(slotConnected()));
    disconnect(m_tcpSocket, SIGNAL(disconnected()), this, SLOT(slotDisConnected()));

    disconnect(this, SIGNAL(logined()), this, SLOT(slotLogined()));
    disconnect(this, SIGNAL(remoteShutdown(const QString&)), this, SLOT(slotRemoteShutDown(const QString&)));
    disconnect(this, SIGNAL(clientState()), this, SLOT(slotClientState()));
    disconnect(this, SIGNAL(loginOut()), this, SLOT(slotLoginOut()));
}


