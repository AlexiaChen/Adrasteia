/*************************************************************************
* Copyright (C), 2016,
* XXXXXXXXXXX Co.
* �ļ���: ProductTaskManager.h
* ��  ��: zhangl		�汾��1.0		��  �ڣ�2016/06/12
* ��  ������Ʒɨ���������
* ��  ����
* �����б�:
* �޸���־��
*   No. ����		����		�޸�����
*
*************************************************************************/
#ifndef PRODUCT_TASK_MANAGER_H
#define PRODUCT_TASK_MANAGER_H

#include "ProductDef.h"
#include "TaskBase.h"
#include <QString>
#include <QMutex>
#include <QMutexLocker>

/************************************************************************/
/* �� ���� ProductTaskManager                                               */
/* �� �ࣺ                                                              */
/* ˵ ���� �������ݽ���������                                           */
/* �� ����                                                              */
/************************************************************************/
class ProductTaskManager
{
private:
	ProductTaskManager();

public:
	~ProductTaskManager();
	static ProductTaskManager & getClass();
	
public:
	/* ��Ʒ���� */
	bool AddProduct(ST_PRODUCT stProduct);
	bool FindProduct(QString strProductKey);
	bool RemoveProduct(QString strProductKey);
	bool GetProduct(QString strProductKey, ST_PRODUCT &stProduct);

	/* ������� */
	bool AddTask(QString strType,int nTimeRange, TaskBase * pTask);
	TaskBase * FindTask(QString strProductKey);
	void RemoveTask(QString strProductKey);

	bool StartTask(QString strType, int nTimeRange); // ��������

private:
	HASH_PRODUCT      m_hasProduct;      // ����Ĳ�Ʒ�б�: ע�˴���Key����Ҫ�ϲ��Ĺ���Key
	QMutex            m_mutexProduct;    // ��
	HASH_PRODUCT_TASK m_hasProductTask;  // �����б� : ע�˴���Key����Ҫ�ϲ��Ĺ���Key
	QMutex            m_mutexTask;       // ������
};


#endif // PRODUCT_TASK_MANAGER_H