/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: DataProcesser.h
* 作  者: Jeff		版本：1.0		日  期：
* 描  述：网络协议解析处理
* 其  他：
* 功能列表:
* 修改日志：
*   No. 日期		作者		修改内容
*
*************************************************************************/
#ifndef DATAPROCESSER_H
#define DATAPROCESSER_H

#include "stdio.h"
#include "ProtocolDef.h"
#include "NetworkManager.h"
#include <QString>
#include <QByteArray>

/************************************************************************/
/* 缓冲区定义                                                           */
/************************************************************************/
#define MAX_MSG_INFO_LEN  4096  // 消息内容最大为256字节，暂定
#define MAX_MSGLIST_LEN   10    // 消息队列最大存储10条未处理数据

typedef struct _MSG
{
	STD_MSG_HEAD stMsgHead;              // 消息头
	char  szMsgInfo[MAX_MSG_INFO_LEN];   // 消息内容
}ST_MSG;

/************************************************************************/
/* 类 名： DataProcesser                                                */
/* 父 类：                                                              */
/* 说 明： 网络数据解析处理类                                           */
/* 描 述：                                                              */
/************************************************************************/
class DataProcesser
{
public:
    DataProcesser(char * request, int len);
    ~DataProcesser();

public:
    int process(QString strClientIP, QByteArray & response);

private:
	// 相应命令响应处理
	int HandleHeartbeat(int nClientID, QByteArray & response);
	int HandleLogin(ST_MSG &stMsg, QByteArray & response);
	int HandleGridData(ST_MSG &stMsg, QByteArray & response);
	int HandleStationData(ST_MSG &stMsg, QByteArray & response);
	int HandleStationFile(ST_MSG &stMsg, QByteArray & response);
	int HandleMultiStationData(ST_MSG &stMsg, QByteArray & response);
	int HandleGrid2Station(ST_MSG &stMsg, QByteArray & response);
	int HandleStation2Grid(ST_MSG &stMsg, QByteArray & response);
	int HandleRainProcess(ST_MSG &stMsg, QByteArray & response);
	int HandleStationCfg(ST_MSG &stMsg, QByteArray & response);
	int HandleErrorCmd(int nClientID, QByteArray & response);

	// 协议打包解包
	int Pack();
	int UnPack(const char *pOrigData, int nOrigSize, STD_MSG_HEAD &stMsgHead);
	int UnPack(const char *pOrigData, int nOrigSize, ST_MSG &stMsg);

private:
	char * _request;
	int  m_nDataLen;
};

#endif // DATAPROCESSER_H
