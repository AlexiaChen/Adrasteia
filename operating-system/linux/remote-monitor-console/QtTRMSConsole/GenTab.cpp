#include "GenTab.h"

#ifdef __linux__
#include <arpa/inet.h>
#include "zlib.h"
#endif

#ifndef __linux__
#include<Winsock2.h>
#pragma comment(lib,"ws2_32.lib")
#endif

#include <iostream>
#include "Utils.h"
#include "Public.h"


void p_InitTab_Client_Login(TCLIENT_LOGIN *P)
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

void p_InitTab_Client_State(TCLIENT_STATE *P)
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

void p_InitTab_ShutDownCmd(TSHUTDOWNCMD *P)
{
    memset(P->ShutDownCmd, '\0', 1025);
    strcat(P->ShutDownCmd, "shutdown -s -f");
}

int p_TabPack_Client_Login(char *cBuf, int iBufLen, TCLIENT_LOGIN *client_login)
{
    if (CLIENT_LOGIN_LEN >= iBufLen)
    {
        /*要打包的字符串长度超过缓冲区长度，直接返回失败-1*/
        return -1;
    }
    sprintf(cBuf, "%-16s%-18s%-20s%-4096s", \
        client_login->IPAddress, client_login->MAC, client_login->OSBootTime, client_login->BaseInfo);
    return 0;
}

int p_TabUnPack_Client_Login(char *cBuf, TCLIENT_LOGIN* client_login)
{
    int i = 0;
    char cTmpBuf[32];
    if (strlen(cBuf) < CLIENT_LOGIN_LEN)
    {
        /*要解包的字符串长度不够，直接返回失败-1*/
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

int p_TabPack_Client_State(char *cBuf, int iBufLen, TCLIENT_STATE *client_state)
{
    if (CLIENT_STATE_LEN >= iBufLen)
    {
        /*要打包的字符串长度超过缓冲区长度，直接返回失败-1*/
        return -1;
    }
    sprintf(cBuf, "%-16s%-18s%-20s%-3s%-3s%-1024s", \
        client_state->IPAddress, client_state->MAC, client_state->OSBootTime, client_state->UseCPU, client_state->UserMemory, client_state->DiskSpaceInfo);
    return 0;
}

int p_TabUnPack_Client_State(char *cBuf, TCLIENT_STATE* client_state)
{
    int i = 0;
    char cTmpBuf[32];
    if (strlen(cBuf) < CLIENT_STATE_LEN)
    {
        /*要解包的字符串长度不够，直接返回失败-1*/
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

int p_TabPack_ShutDownCmd(char *cBuf, int iBufLen, TSHUTDOWNCMD *shutdowncmd)
{
    if (SHUTDOWNCMD_LEN >= iBufLen)
    {
        /*要打包的字符串长度超过缓冲区长度，直接返回失败-1*/
        return -1;
    }
    sprintf(cBuf, "%-1024s", \
        shutdowncmd->ShutDownCmd);
    return 0;
}



int p_TabUnPack_ShutDownCmd(char *cBuf, TSHUTDOWNCMD* shutdowncmd)
{
    char cTmpBuf[32];
    if (strlen(cBuf) > SHUTDOWNCMD_LEN)
    {
        //std::cout << "shutdown CMD length is over max" << std::endl;
        return -1;
    }
    
    memset(shutdowncmd->ShutDownCmd, 0, 1025);
    memcpy(shutdowncmd->ShutDownCmd, cBuf, strlen(cBuf));
    CutBothEndsSpace(shutdowncmd->ShutDownCmd);
    if (strlen(shutdowncmd->ShutDownCmd) == 0)
        strcpy(shutdowncmd->ShutDownCmd, "NULL");
    return 0;
}

void AnaData(unsigned char * bBuffer, int iBufLength, TCLIENT_INFO *CLIENT_INFO, bool &bRecvEnd)
{
    unsigned char bChar;

    for (int i = 0; i < iBufLength; i++) {
        bChar = bBuffer[i];
        switch (CLIENT_INFO->CUR_STEP) {
        case RECV_HEAD: // 接收头判断
            if (bChar == 0x81) {
                CLIENT_INFO->CUR_STEP = RECV_SENDER;
            }
            break;
        case RECV_SENDER: // 发送者判断，客户端只能接受节点服务器发送的数据
           
            CLIENT_INFO->CUR_STEP = RECV_RECEIVER;
            break;
        case RECV_RECEIVER: // 接收者判断，接收者必须是自己也就是说是
            /* 目前不进行判断处理 */
            CLIENT_INFO->CUR_STEP = RECV_SENDTIME;
            break;
        case RECV_SENDTIME: // 获取时间戳
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
        case RECV_TYPE: // 判断数据包类型
            CLIENT_INFO->bFieldBuf[CLIENT_INFO->CUR_IDX++] = bChar;
            if (CLIENT_INFO->CUR_IDX == 6) {
                CLIENT_INFO->bFieldBuf[CLIENT_INFO->CUR_IDX] = 0;
                CLIENT_INFO->CUR_IDX = 0;
                CLIENT_INFO->PackageType = atoi((char*)(CLIENT_INFO->bFieldBuf));
                CLIENT_INFO->CUR_STEP = RECV_LENGTH;
            }
            break;
        case RECV_LENGTH: // 获取数据长度
            CLIENT_INFO->bFieldBuf[CLIENT_INFO->CUR_IDX++] = bChar;
            if (CLIENT_INFO->CUR_IDX == 4) {
                CLIENT_INFO->bFieldBuf[CLIENT_INFO->CUR_IDX] = 0;
                memcpy(&(CLIENT_INFO->iLength), CLIENT_INFO->bFieldBuf,
                    CLIENT_INFO->CUR_IDX);
                CLIENT_INFO->iLength = ntohl(CLIENT_INFO->iLength);
                CLIENT_INFO->msRecv = new unsigned char[CLIENT_INFO->iLength+1];
                memset(CLIENT_INFO->msRecv, 0, CLIENT_INFO->iLength+1);
                CLIENT_INFO->msRecvLength = 0;
                CLIENT_INFO->CUR_STEP = RECV_TRUE_LENGTH;
                CLIENT_INFO->CUR_IDX = 0;
            }
            break;
        case RECV_TRUE_LENGTH: // 数据的真实长度，如果是压缩包，此字段为压缩前长度
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
        case RECV_FLAG: // 返回报标志。
            CLIENT_INFO->bFieldBuf[CLIENT_INFO->CUR_IDX++] = bChar;
            if (CLIENT_INFO->CUR_IDX == 4) {
                CLIENT_INFO->bFieldBuf[CLIENT_INFO->CUR_IDX] = 0;
                memcpy(&(CLIENT_INFO->iPackageFLAG), CLIENT_INFO->bFieldBuf,
                    CLIENT_INFO->CUR_IDX);
                CLIENT_INFO->iPackageFLAG = ntohl(CLIENT_INFO->iPackageFLAG);
                CLIENT_INFO->CUR_IDX = 0;
                CLIENT_INFO->CUR_STEP = RECV_DATA;
           
            }
            break;
        case RECV_DATA:
           
            if ( CLIENT_INFO->msRecvLength + (iBufLength - i) >= CLIENT_INFO->iLength) {
               

               // std::copy_n(bBuffer + i, CLIENT_INFO->iLength - msRecv.size(), std::back_inserter(msRecv));
                memcpy(CLIENT_INFO->msRecv + CLIENT_INFO->msRecvLength, bBuffer + i, CLIENT_INFO->iLength - CLIENT_INFO->msRecvLength);
                CLIENT_INFO->msRecvLength = CLIENT_INFO->iLength - CLIENT_INFO->msRecvLength;
               
                CLIENT_INFO->CUR_STEP = RECV_HEAD;
                CLIENT_INFO->CUR_IDX = 0;

                bRecvEnd = true;
                i += CLIENT_INFO->iLength;
            }
            else { // 不够则全部拷贝进内存流。
               
               // std::copy_n(bBuffer + i, iBufLength - i, std::back_inserter(msRecv));
                memcpy(CLIENT_INFO->msRecv + CLIENT_INFO->msRecvLength, bBuffer + i, iBufLength-i);
                CLIENT_INFO->msRecvLength += iBufLength - i;
               
                i = iBufLength;
            }
            break;
        default:
            break;
        }
    }
}

static void sendClientState(Client* client_tcp)
{

    //std::cout << "sendClientState() entry" << std::endl;

    TCLIENT_STATE tState;
    p_InitTab_Client_State(&tState);
    strcpy(tState.IPAddress, Utils::localIP().c_str());
    strcpy(tState.MAC, Utils::localMAC().c_str());
    strcpy(tState.OSBootTime, Utils::lastStartupTime().c_str());
    strcpy(tState.UseCPU, Utils::cpuUsage().c_str());
    //std::cout << "CPU Usage:" << Utils::cpuUsage().c_str() << std::endl;
    strcpy(tState.UserMemory, Utils::memUsage().c_str());
    //std::cout << "MEM Usage:" << Utils::memUsage().c_str() << std::endl;
    strncpy(tState.DiskSpaceInfo, Utils::hardDiskUsage().c_str(), 1024);
    //std::cout << "Disk Space info : " << Utils::hardDiskUsage().c_str() << std::endl;

    int iSendBufLen = CLIENT_STATE_LEN + 1;

    char cSendBuf[CLIENT_STATE_LEN + 1];
    p_TabPack_Client_State(cSendBuf, CLIENT_STATE_LEN + 1, &tState);

    int iPackageLength = 0;
    unsigned char bPackageData[1024 * 10] = {0};


    CreatPackage("1003", cSendBuf, iSendBufLen, bPackageData, iPackageLength);
    SendData(client_tcp, bPackageData, iPackageLength);


   // std::cout << "sendClientState() leave" << std::endl;

    return;
}

static void slotRemoteShutDown(const std::string & script)
{

   // std::cout << "remote shut down machine is received: " << script << std::endl;

#ifdef __linux__
    Utils::execCmd2(script);
#endif
}

void AnaPackage(Client* client_tcp, TCLIENT_INFO *serviceInfo)
{
    // 根据包类型进行不同的处理
    bool bError = false, bCompress = false, bCrypt = false;
    GetPackageFLAG(serviceInfo->iPackageFLAG, bError, bCompress, bCrypt);
    unsigned char * recvBuf;

    recvBuf = new unsigned char[serviceInfo->iTrue_Length + 1];
    memset(recvBuf, 0, serviceInfo->iTrue_Length + 1);

    memcpy(recvBuf, serviceInfo->msRecv, serviceInfo->msRecvLength );
   

    if (bError) { // 说明调用出现错误，需要提示错误信息。
        delete[]recvBuf;
        return;
    }
    
    // 根据包类型进行处理
    switch (serviceInfo->PackageType)
    {
    case 2001: // 客户端签到成功
        //emit logined();
        break;
    case 2002: // 客户端签退
        //emit loginOut();
        break;
    case 2003: // 客户端状态回送
        sendClientState(client_tcp);

        break;
    case 2004: //远程关机
       {   
           TSHUTDOWNCMD shut_down_t;
           CutBothEndsSpace((char*)recvBuf);
           //std::cout << "recv delete Space is: " << recvBuf << std::endl;

           int err_t = p_TabUnPack_ShutDownCmd((char*)recvBuf, &shut_down_t);
           if (err_t == 0)
           {
             //  std::cout << "ShutDownCmd is:" << shut_down_t.ShutDownCmd << std::endl;
               
               slotRemoteShutDown(std::string(shut_down_t.ShutDownCmd));
           }
       }
        break;
    default:
        //std::cout << "Sub Package Type is Unknown" << std::endl;
        break;
    }

    delete[]recvBuf;
    delete[] serviceInfo->msRecv;
}

void GetPackageFLAG(unsigned int dwFlag, bool &bError, bool &bCompress, bool &bCrypt)
{
    if ((dwFlag & 0x01) == 0x01)
        bError = true;
    if ((dwFlag & 0x04) == 0x04)
        bCompress = true;
    if ((dwFlag & 0x04) == 0x08)
        bCrypt = true;
}

void SetHeadPackage(unsigned char * bHeadBuffer, char * cType, int iSender, int iRecver, int iLength, int iTrue_Length, bool bCompressed)
{
    bHeadBuffer[0] = 0x81; 						// 设置头包引导符
    bHeadBuffer[1] = iSender;
    bHeadBuffer[2] = iRecver;

#ifdef WIN32
    DWORD dwSendTime = GetTickCount();
#endif

#ifdef __linux__
    unsigned int dwSendTime = time(NULL) * 1000;
#endif
    dwSendTime = htonl(dwSendTime);
    memcpy(bHeadBuffer + 3, &dwSendTime, 4); 	// 设置时间戳

    memcpy(bHeadBuffer + 7, cType, 6); 			// 设置子包类型 6位长度

    iLength = htonl(iLength);
    memcpy(bHeadBuffer + 13, &iLength, 4); 		// 设置子包长度

    iTrue_Length = htonl(iTrue_Length);
    memcpy(bHeadBuffer + 17, &iTrue_Length, 4); // 设置真实长度，压缩前长度

    unsigned int dwFlag = 0;
    if (bCompressed)
        dwFlag |= 0x04;
    dwFlag = htonl(dwFlag);
    memcpy(bHeadBuffer + 21, &dwFlag, 4); 		// 设置标志
}

void CreatPackage(char * cType, char *cSendBuf, int &iSendBufLen, unsigned char * bPackageData, int &iPackageLength)
{
    bool bCompressed = false;
    // 根据是否压缩属性判断是否需要对发送数据进行压缩
    unsigned long iDestLen = (iSendBufLen - 12) * 100.1 / 100.0 + 1; // 压缩后目标长度
   
    iDestLen = iSendBufLen;
  
    //std::cout << "Leave Create " << std::endl;
    memcpy(bPackageData + SYS_HEAD_LEN, cSendBuf, iDestLen);
    

    SetHeadPackage(bPackageData, cType, SENDER_MANGER,
        SENDER_WINDOWS | SENDER_LINUX, iDestLen, iSendBufLen, bCompressed);
    iPackageLength = iDestLen + SYS_HEAD_LEN;

    return;
}

void SendData(Client* client_tcp, unsigned char * bBuffer, int iDataLength)
{

    if (client_tcp->SendBytes((char*)bBuffer, iDataLength))
    {
        return ;
    }
    else
    {
        exit(-1);
    }
    

}