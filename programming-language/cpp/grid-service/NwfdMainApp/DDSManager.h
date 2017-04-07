/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* �ļ���: DDSManager.h
* ��  ��: zhangl  		�汾��1.0		��  �ڣ�2015-07-14
* ��  �������ݷַ������߳�
* ��  ����
* �����б�:
*   1. �������ݿ����ӳ�
*   2. �������ݴ���������ӵ����ݷַ���Ϣ���浽���ݿ���
* �޸���־��
*   No.   ����		  ����		       �޸�����
* ------------------------------------------------------------------------
*************************************************************************/
#ifndef DDS_MANAGER_H
#define DDS_MANAGER_H

#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include "DDSDef.h"
#include "DBConnPool.h"
#include "TaskDDS.h"

/************************************************************************/
/* �� ���� CDDSManager                                                  */
/* �� �ࣺ QThread                                                      */
/* ˵ ���� ���ݷַ�������                                               */
/* �� ����                                                              */
/************************************************************************/
class CDDSManager : public QThread
{
private:
	CDDSManager();

public:
	~CDDSManager();
	static CDDSManager & getClass();

public:
	void Init(HASH_DDS  hasDDS);
	void SetDDSTask(TaskDDS* pTaskDDS);
	void Stop();/* �ⲿֹͣ�߳�ִ�еķ��� */
	bool IsDisabled();
	int DataDistribution(QString strKey, QStringList lstDDS, QString strFileName, QString strPath, bool bForceFlg = false);
	int AddDDSFile(ST_DDSFILE stDDSFile);

protected:
	void run();

private:
	int ExecTask();
	int Proccess(ST_DDSFILE stDDSFile);

private:
	bool           m_bReady;
	bool           m_bDisabled;   // �Ƿ����
	HASH_DDS       m_hasDDS;      // ���ݷַ�Ŀ�ĵ��б�
	CDBConnPool*   m_pDBConnPool; // ���ݿ�����ָ��
	LIST_DDSFILE   m_lstDDSFile;  // ���ݷַ��ļ��б�
	QMutex         m_mutexList;   // �ļ��б���

	TaskDDS*       m_pTaskDDS;    // ���ݷַ�ִ������
};

#endif //DDS_MANAGER_H
