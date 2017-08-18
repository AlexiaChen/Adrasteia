#include "ClientDecoder.h"
#include "Utils.h"

#ifdef __linux__
#include <arpa/inet.h>
#include "zlib.h"
#endif

#include <cstdlib>
#include <cstdio>

#ifndef __linux__
#include<Winsock2.h>
#pragma comment(lib,"ws2_32.lib")
#endif

#include <QDebug>



ClientDecoder::ClientDecoder(uint8_t* recvBuf, size_t bufLength, bool &isLogin):
m_recv_buffer(bufLength), m_isLogin(isLogin)
{
    std::copy(recvBuf, recvBuf + bufLength, std::back_inserter(m_recv_buffer));

}

ClientDecoder::ClientDecoder(bool &isLogin) :m_isLogin(isLogin)
{
    
}


ClientDecoder::~ClientDecoder()
{
}

bool ClientDecoder::decodeData(TCP_CLIENT_INFO* clientInfo)
{
    uint8_t bChar;
    bool bRecvEnd = false;
    for (int i = 0; i < m_recv_buffer.size(); ++i)
    {
        bChar = m_recv_buffer[i];
        switch (clientInfo->CUR_STEP)
        {
        case RECV_HEAD: // 接收头判断  数据包前导符号
            if (bChar == 0x81)
            {
                clientInfo->CUR_STEP = RECV_SENDER;
            }
            break;
        case RECV_SENDER:
            clientInfo->CUR_STEP = RECV_RECEIVER;
            break;
        case RECV_RECEIVER:
            clientInfo->CUR_STEP = RECV_SENDTIME;
            break;
        case RECV_SENDTIME:// 此包的发送时间戳
            clientInfo->bFieldBuf[clientInfo->CUR_IDX++] = bChar;
            if (clientInfo->CUR_IDX == 4) // 时间戳的长度是4 bytes
            {
                clientInfo->bFieldBuf[4] = 0;
                memcpy(&(clientInfo->SendTime), clientInfo->bFieldBuf, 4);
                clientInfo->SendTime = ntohl(clientInfo->SendTime);
                clientInfo->CUR_STEP = RECV_TYPE;
                clientInfo->CUR_IDX = 0;
            }
            break;
        case RECV_TYPE: // 子包类型
            clientInfo->bFieldBuf[clientInfo->CUR_IDX++] = bChar;
            if (clientInfo->CUR_IDX == 6) {
                clientInfo->bFieldBuf[6] = 0;
                clientInfo->CUR_IDX = 0;
                clientInfo->PackageType = atoi((char*)(clientInfo->bFieldBuf));
                clientInfo->CUR_STEP = RECV_LENGTH;
            }
            break;
        case RECV_LENGTH://子包长度
            clientInfo->bFieldBuf[clientInfo->CUR_IDX++] = bChar;
            if (clientInfo->CUR_IDX == 4) {
                clientInfo->bFieldBuf[4] = 0;
                memcpy(&(clientInfo->iLength), clientInfo->bFieldBuf,
                    4);
                clientInfo->iLength = ntohl(clientInfo->iLength);
                clientInfo->CUR_STEP = RECV_TRUE_LENGTH;
                clientInfo->CUR_IDX = 0;
            }
            break;
        case RECV_TRUE_LENGTH: //子包压缩前长度
            clientInfo->bFieldBuf[clientInfo->CUR_IDX++] = bChar;
            if (clientInfo->CUR_IDX == 4) {
                clientInfo->bFieldBuf[4] = 0;
                memcpy(&(clientInfo->iTrue_Length), clientInfo->bFieldBuf,
                    4);
                clientInfo->iTrue_Length = ntohl(clientInfo->iTrue_Length);
                clientInfo->CUR_STEP = RECV_FLAG;
                clientInfo->CUR_IDX = 0;
            }
            break;
        case RECV_FLAG: // 标志位
            clientInfo->bFieldBuf[clientInfo->CUR_IDX++] = bChar;
            if (clientInfo->CUR_IDX == 4) {
                clientInfo->bFieldBuf[4] = 0;
                memcpy(&(clientInfo->iPackageFLAG), clientInfo->bFieldBuf,
                    4);
                clientInfo->iPackageFLAG = ntohl(clientInfo->iPackageFLAG);
                clientInfo->CUR_IDX = 0;
                clientInfo->CUR_STEP = RECV_DATA;
                clientInfo->msRecv.clear();
            }
            break;
        case RECV_DATA:
            if ((m_recv_buffer.size() - i) >= clientInfo->iLength) {
                // 如果剩余缓冲区比子包数据大，则直接把子包数据进行截取
                //msRecv->WriteBuffer(bBuffer + i,
                  //  clientInfo->iLength - msRecv->Size);
                
                size_t copy_size = clientInfo->iLength;
                std::copy_n(m_recv_buffer.begin() + i, 
                    copy_size,
                    std::back_inserter(clientInfo->msRecv));
                
               
                
                clientInfo->CUR_STEP = RECV_HEAD;
                clientInfo->CUR_IDX = 0;

                bRecvEnd = true;
                i += clientInfo->iLength;

                goto ret;
            }
            else { // 不够则全部拷贝,那么继续等待数据到来。
                // msRecv->WriteBuffer(bBuffer + i, iBufLength - i);
               // size_t copy_size = m_recv_buffer.size() - i;
               // std::copy_n(m_recv_buffer.begin() + i,
                 //   copy_size,
                  //  std::back_inserter(clientInfo->msRecv));
                //i = m_recv_buffer.size();
                bRecvEnd = false;
                goto ret;
            }
            break;
        default:
            break;
        }
    }
ret:

    return bRecvEnd;
}

static void getPackageFlag(uint32_t dwFlag, bool &bError,
    bool &bCompress, bool &bCrypt)
{
    if ((dwFlag & 0x01) == 0x01)
        bError = true;
    if ((dwFlag & 0x04) == 0x04)
        bCompress = true;
    if ((dwFlag & 0x04) == 0x08)
        bCrypt = true;
}

void ClientDecoder::decodePackage(TCP_CLIENT_INFO* clientInfo)
{
    bool bError = false, bCompress = false, bCrypt = false;
    getPackageFlag(clientInfo->iPackageFLAG, bError, bCompress, bCrypt);
    
    //对于管理终端发送过来的指令协议的子包暂时不用处理,只用判断子包类型即可执行相应操作
    
    //uint8_t * recvBuf = NULL;

   // recvBuf = new uint8_t[clientInfo->iTrue_Length + 1];
   // memset(recvBuf, 0, clientInfo->iTrue_Length + 1);

    //if (bCompress)
    //{
      //  uint8_t * dataBuf = new uint8_t[clientInfo->iLength + 1];
       // std::copy_n(clientInfo->msRecv.begin(), clientInfo->msRecv.size(), dataBuf);

        // zlib 解压
#ifdef __linux__
        //ulong tlen;
        //ulong ilen = clientInfo->iLength;
        //int iRet = uncompress(recvBuf, &tlen, dataBuf, ilen);
#endif
        //delete[] dataBuf;
        //clientInfo->msRecv.clear();
        //clientInfo->msRecv.shrink_to_fit();
#ifdef __linux__
        //if (iRet != Z_OK)
       // {
         //   qDebug() << "uncompress failed, error code is : " << iRet;
           // delete[] recvBuf;
            //return;
        //}
#endif

    //}
    //else
    //{
      //  std::copy_n(clientInfo->msRecv.begin(), clientInfo->msRecv.size(), recvBuf);
        //clientInfo->msRecv.clear();
        //clientInfo->msRecv.shrink_to_fit();
    //}

    if (bError)
    {
       // qDebug() << "get data failed, and failed msg is : " << recvBuf;
        //delete[] recvBuf;
        return;
    }

    //根据数据包类型进行处理
    switch (clientInfo->PackageType)
    {
    case 2001: // 客户端签到成功返回包
        qDebug() << "login succeess";
        m_isLogin = true;
        break;
    case 2003: // 获取客户端运行状态
        qDebug() << "want to get client running status";
        break;
    case 2004: // 关闭客户端服务器
        qDebug() << "want to shutdown client physical machine";
        Utils::shutDown();
        break;
    default:
        qDebug() << "Error! unknown package type: " << clientInfo->PackageType;
        break;
    }

   // delete[] recvBuf;
}

void ClientDecoder::addRecvBuf(uint8_t* recvBuf, size_t bufLength)
{
    m_recv_buffer.shrink_to_fit();
    std::copy(recvBuf, recvBuf + bufLength, std::back_inserter(m_recv_buffer));
}

void ClientDecoder::clearRecvBufFirst(size_t size)
{
    m_recv_buffer.erase(m_recv_buffer.begin(), m_recv_buffer.begin() + size);
    m_recv_buffer.shrink_to_fit();
}

bool ClientDecoder::decodeData2(TCP_CLIENT_INFO* clientINfo)
{
    if (m_recv_buffer.size() < PACKAGE_HEAD_SIZE) // 小于消息头，就继续读取,知道大于等于消息头
    {
        return false;
    }

    if (clientINfo->CUR_STEP != RECV_DATA)
    {
        std::vector<uint8_t> package_head;
        std::move(m_recv_buffer.begin(), m_recv_buffer.begin() + PACKAGE_HEAD_SIZE, std::back_inserter(package_head));
        //m_recv_buffer.erase(m_recv_buffer.begin(), m_recv_buffer.begin() + PACKAGE_HEAD_SIZE);

        // 发送时间戳 4bytes
        char tmp_buffer[256] = { 0 };
        std::copy_n(package_head.begin() + 3, 4, tmp_buffer);
        memcpy(&(clientINfo->SendTime), tmp_buffer,
            4);
        clientINfo->SendTime = ntohl(clientINfo->SendTime);

        // 子包类型 6bytes
        memset(tmp_buffer, 0, 256);
        std::copy_n(package_head.begin() + 7, 6, tmp_buffer);
        clientINfo->PackageType = atoi(tmp_buffer);

        //子包长度 4 bytes
        memset(tmp_buffer, 0, 256);
        std::copy_n(package_head.begin() + 13, 4, tmp_buffer);
        memcpy(&(clientINfo->iLength), tmp_buffer,
            4);
        clientINfo->iLength = ntohl(clientINfo->iLength);

        //子包压缩前的真实长度 4bytes
        memset(tmp_buffer, 0, 256);
        std::copy_n(package_head.begin() + 17, 4, tmp_buffer);
        memcpy(&(clientINfo->iTrue_Length), tmp_buffer,
            4);
        clientINfo->iTrue_Length = ntohl(clientINfo->iTrue_Length);

        //FLAG字段  4 bytes
        memset(tmp_buffer, 0, 256);
        std::copy_n(package_head.begin() + 21, 4, tmp_buffer);
        memcpy(&(clientINfo->iPackageFLAG), tmp_buffer,
            4);
        clientINfo->iPackageFLAG = ntohl(clientINfo->iPackageFLAG);

        clientINfo->CUR_STEP = RECV_DATA;
    }

    if (clientINfo->CUR_STEP == RECV_DATA)
    {
        size_t sub_packet_size = clientINfo->iLength;

        if (m_recv_buffer.size() < sub_packet_size + PACKAGE_HEAD_SIZE)
        {
            return false;
        }

        //完整的包读完,就拷贝子包
        std::copy_n(m_recv_buffer.begin() + PACKAGE_HEAD_SIZE, sub_packet_size, std::back_inserter(clientINfo->msRecv));
        m_recv_buffer.erase(m_recv_buffer.begin(), m_recv_buffer.begin() + PACKAGE_HEAD_SIZE + sub_packet_size);
        m_recv_buffer.shrink_to_fit();
        return true;
    }

    return false;
}
