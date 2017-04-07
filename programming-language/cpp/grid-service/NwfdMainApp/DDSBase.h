/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* �ļ���: DDSBase.h
* ��  ��: zhangl  		�汾��1.0		��  �ڣ�2015-07-14
* ��  �������ݷַ�������
* ��  ����
* �����б�:
* �޸���־��
*   No.   ����		  ����		       �޸�����
* ------------------------------------------------------------------------
*************************************************************************/
#ifndef DDS_BASE_H
#define DDS_BASE_H

#include "DDSDef.h"
#include <QThread>

/************************************************************************/
/* �� ���� CDDSBase                                                     */
/* �� �ࣺ QThread                                                      */
/* ˵ ���� ���ݷַ�������(�ӿ���)                                       */
/* �� ����                                                              */
/************************************************************************/
class CDDSBase : public QThread
{
	Q_OBJECT

public:
	CDDSBase();
	virtual ~CDDSBase();

	virtual void SetJobList(LIST_JOB  *lstJob);
	virtual bool IsComplete();
protected:
	virtual void run() = 0;

protected:
	LIST_JOB  *m_lstJob;
	bool       m_bComplete;
};

#endif //DDS_BASE_H
