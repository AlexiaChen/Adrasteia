/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* �ļ���: DDSFtp.h
* ��  ��: zhangl  		�汾��1.0		��  �ڣ�2015-07-14
* ��  ����Ftp���ݷַ�������
* ��  ����
* �����б�:
* �޸���־��
*   No.   ����		  ����		       �޸�����
* ------------------------------------------------------------------------
*************************************************************************/
#ifndef DDS_FTP_H
#define DDS_FTP_H

#include "DDSBase.h"
#include "FTP.h"

/************************************************************************/
/* �� ���� CDDSFtp                                                      */
/* �� �ࣺ CDDSBase                                                     */
/* ˵ ���� FTP���ݷַ���                                                */
/* �� ����                                                              */
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
	ST_DDS   m_stDDS;    // ��������
	CFTP    *m_ftp;      // ftp������
};

#endif //DDS_FTP_H
