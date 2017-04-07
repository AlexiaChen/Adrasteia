/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: DDSMq.h
* 作  者: zhangl  		版本：1.0		日  期：2015-07-14
* 描  述：Ftp数据分发器基类
* 其  他：
* 功能列表:
* 修改日志：
*   No.   日期		  作者		       修改内容
* ------------------------------------------------------------------------
*************************************************************************/
#ifndef DDS_MQ_H
#define DDS_MQ_H

#include "DDSBase.h"

/************************************************************************/
/* 类 名： CDDSMq                                                       */
/* 父 类： CDDSBase                                                     */
/* 说 明： MQ数据分发器                                                 */
/* 描 述：                                                              */
/************************************************************************/
class CDDSMq :	public CDDSBase
{
public:
	CDDSMq();
	virtual ~CDDSMq();

protected:
	virtual void run();

private:
	int TransferFile(QString strLocalFile, QString strRemoteFile);

private:
	bool     m_bReady;
	ST_DDS   m_stDDS;    // 传输配置
};

#endif // DDS_MQ_H
