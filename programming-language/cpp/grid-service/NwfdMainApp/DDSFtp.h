/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: DDSFtp.h
* 作  者: zhangl  		版本：1.0		日  期：2015-07-14
* 描  述：Ftp数据分发器基类
* 其  他：
* 功能列表:
* 修改日志：
*   No.   日期		  作者		       修改内容
* ------------------------------------------------------------------------
*************************************************************************/
#ifndef DDS_FTP_H
#define DDS_FTP_H

#include "DDSBase.h"
#include "FTP.h"

/************************************************************************/
/* 类 名： CDDSFtp                                                      */
/* 父 类： CDDSBase                                                     */
/* 说 明： FTP数据分发器                                                */
/* 描 述：                                                              */
/************************************************************************/
class CDDSFtp : public CDDSBase
{
	Q_OBJECT

public:
	CDDSFtp();
	virtual ~CDDSFtp();

protected:
	virtual void run();

private:
	bool CreateFtpManager(QString strRemoteIP, QString strUserName, QString strPassword);
private:
	bool     m_bReady;
	ST_DDS   m_stDDS;    // 传输配置
	CFTP    *m_ftp;      // ftp操作类
};

#endif //DDS_FTP_H
