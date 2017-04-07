/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* �ļ���: HandleDB.h
* ��  ��: zhangl  		�汾��1.0		��  �ڣ�2015-07-15
* ��  �������ݿ⴦��
* ��  ����
* �����б�:
* �޸���־��
*   No.   ����		  ����		       �޸�����
* ------------------------------------------------------------------------
*************************************************************************/
#ifndef HANDLE_DB_H
#define HANDLE_DB_H

#include "DBConnPool.h"
#include "DDSDef.h"
#include "ProductDef.h"
#include "NwfdClientDef.h"
#include <QDateTime>
#include <QMutex>
#include <QMutexLocker>

/************************************************************************/
/* �� ���� HandleDB                                                     */
/* �� �ࣺ                                                              */
/* ˵ ���� ���ݿ⴦��                                                   */
/* �� ����                                                              */
/************************************************************************/
class HandleDB
{
private:
	HandleDB();

public:
	~HandleDB();
	static HandleDB & getClass();

public:
	/* ��ʼ����Ʒ */
	int InitProduct(HASH_PRODUCT hasProduct);

	/* ���ݷַ����� */
	int SaveDDSFileInfo(ST_DDSFILE stDDSFile, int nFileSize, QDateTime tFileCreate, ST_DDS  stDDS);
	int GetDDSJob(MAP_JOB & mapJob);
	int UpdateJobResult(ST_JOB stJob);
	int DeleteDDS(QString strDate);

	/* ���ݺϲ����� */
	int SaveMergeDataInfo(int nClientID, QString strProductKey, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond,
		int nForecastTime, int nTimeRange, float fLon1, float fLon2, float fLat1, float fLat2);
	int GetMergeData(QString strProductKey, LIST_MERGEDATA & listMergeData);
	int UpdateMergeResult(ST_MergeData stMergeData);
	int UpdateMergeStatus(QString status, int mergeID);

	/* �ͻ������� */
	int SaveClientDataInfo(int nClientID, const QString &strProductKey, const QString &strRelatedProductKey, const QString &messageType,int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond,
		int nStartForecastTime,int nForecastTime, int nTimeRange, float fLon1, float fLon2, float fLat1, float fLat2);
	int GetFirstClientDataInfo(ST_DB_MSG_ITEM* outItem);
	
	int DeleteClientData(QString strDate);
	int DeleteTaskData(QString strDate);
	int DeleteStationRecvData(QString strDate);
	int DeleteStationSendData(QString strDate);
	int DeleteMqRecvData(QString strDate);
	int DeleteMqSendData(QString strDate);

	/* վ������ */
	int SaveStationData(int nClientID, QString strCityType, QString strDataType, QString strStationNo, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond,
		int nForecastTime, int nTimeRange, float * fData, int nDataLen);

	/*������Ϣ����*/      //note: һ����˵�������ݿ�����Ϣ�����ǲ���õģ����ǿ������󳡾��������ݿ���ȫ����Ҳ���ã��������ˡ� 
	//for reference:http://blog.jooq.org/2014/09/26/using-your-rdbms-for-messaging-is-totally-ok/ 
	int GetTaskReprocessMessage(ST_DB_TASK_MSG* msg);
	int UpdateTaskMessage(ST_DB_TASK_MSG* msg);


private:
	int ExecSQL(QString strSQL);

private:
	QMutex  m_mutexInsert;

};

#endif // HANDLE_DB_H
