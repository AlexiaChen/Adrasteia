/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: HandleDB.h
* 作  者: zhangl  		版本：1.0		日  期：2015-07-15
* 描  述：数据库处理
* 其  他：
* 功能列表:
* 修改日志：
*   No.   日期		  作者		       修改内容
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
/* 类 名： HandleDB                                                     */
/* 父 类：                                                              */
/* 说 明： 数据库处理                                                   */
/* 描 述：                                                              */
/************************************************************************/
class HandleDB
{
private:
	HandleDB();

public:
	~HandleDB();
	static HandleDB & getClass();

public:
	/* 初始化产品 */
	int InitProduct(HASH_PRODUCT hasProduct);

	/* 数据分发处理 */
	int SaveDDSFileInfo(ST_DDSFILE stDDSFile, int nFileSize, QDateTime tFileCreate, ST_DDS  stDDS);
	int GetDDSJob(MAP_JOB & mapJob);
	int UpdateJobResult(ST_JOB stJob);
	int DeleteDDS(QString strDate);

	/* 数据合并处理 */
	int SaveMergeDataInfo(int nClientID, QString strProductKey, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond,
		int nForecastTime, int nTimeRange, float fLon1, float fLon2, float fLat1, float fLat2);
	int GetMergeData(QString strProductKey, LIST_MERGEDATA & listMergeData);
	int UpdateMergeResult(ST_MergeData stMergeData);
	int UpdateMergeStatus(QString status, int mergeID);

	/* 客户端数据 */
	int SaveClientDataInfo(int nClientID, const QString &strProductKey, const QString &strRelatedProductKey, const QString &messageType,int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond,
		int nStartForecastTime,int nForecastTime, int nTimeRange, float fLon1, float fLon2, float fLat1, float fLat2);
	int GetFirstClientDataInfo(ST_DB_MSG_ITEM* outItem);
	
	int DeleteClientData(QString strDate);
	int DeleteTaskData(QString strDate);
	int DeleteStationRecvData(QString strDate);
	int DeleteStationSendData(QString strDate);
	int DeleteMqRecvData(QString strDate);
	int DeleteMqSendData(QString strDate);

	/* 站点数据 */
	int SaveStationData(int nClientID, QString strCityType, QString strDataType, QString strStationNo, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond,
		int nForecastTime, int nTimeRange, float * fData, int nDataLen);

	/*任务消息队列*/      //note: 一般来说，用数据库做消息队列是不大好的，但是考虑需求场景，用数据库完全符合也够用，还是用了。 
	//for reference:http://blog.jooq.org/2014/09/26/using-your-rdbms-for-messaging-is-totally-ok/ 
	int GetTaskReprocessMessage(ST_DB_TASK_MSG* msg);
	int UpdateTaskMessage(ST_DB_TASK_MSG* msg);


private:
	int ExecSQL(QString strSQL);

private:
	QMutex  m_mutexInsert;

};

#endif // HANDLE_DB_H
