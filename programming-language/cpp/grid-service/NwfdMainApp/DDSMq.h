/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* �ļ���: DDSMq.h
* ��  ��: zhangl  		�汾��1.0		��  �ڣ�2015-07-14
* ��  ����Ftp���ݷַ�������
* ��  ����
* �����б�:
* �޸���־��
*   No.   ����		  ����		       �޸�����
* ------------------------------------------------------------------------
*************************************************************************/
#ifndef DDS_MQ_H
#define DDS_MQ_H

#include "DDSBase.h"

/************************************************************************/
/* �� ���� CDDSMq                                                       */
/* �� �ࣺ CDDSBase                                                     */
/* ˵ ���� MQ���ݷַ���                                                 */
/* �� ����                                                              */
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
	ST_DDS   m_stDDS;    // ��������
};

#endif // DDS_MQ_H
