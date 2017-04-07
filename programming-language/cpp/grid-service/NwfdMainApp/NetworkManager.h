/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: NetworkManager.h
* 作  者: zhangl		版本：1.0		日  期：2015/07/16
* 描  述：网络数据处理
* 其  他：
* 功能列表:
* 修改日志：
*   No. 日期		作者		修改内容
*
*************************************************************************/
#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include "NwfdClientDef.h"
#include "ProductDef.h"
#include "TaskBase.h"
#include "CachedImpl.h"
#include "TaskTcpServer.h"
#include "TaskStationMsg.h"
#include <QString>
#include <QMutex>
#include <QMutexLocker>

/************************************************************************/
/* 类 名： NetworkManager                                               */
/* 父 类：                                                              */
/* 说 明： 网络数据解析处理类                                           */
/* 描 述：                                                              */
/************************************************************************/
class NetworkManager
{
private:
	NetworkManager();

public:
	~NetworkManager();
	static NetworkManager & getClass();

public:
	/* 初始化 */
	int Init(HASH_PRODUCT hasProduct, HASH_CLIENT  hasClient, ST_CACHED stCached);
	void SetTcpServer(TaskTcpServer*  pTaskTcpServer);
	void SetStationMsg(TaskStationMsg* pTaskStationMsg);
	void SetIsPublish(bool bIsPublish);

	/* 产品管理 */
	bool AddProduct(ST_PRODUCT stProduct);
	bool FindProduct(QString strProductKey);
	bool RemoveProduct(QString strProductKey);
	bool GetProduct(QString strProductKey, ST_PRODUCT &stProduct);

	/* 任务管理 */
	bool AddTask(QString strProductKey, TaskBase * pTask);
	TaskBase * FindTask(QString strProductKey);
	void RemoveTask(QString strProductKey);

	/* 客户端相关命令处理 */
	bool IsLegalClient(int nClientID, QString strClientIP);
	bool IsLogin(int nClientID, QString strUserName, QString strPasswd, bool &bAllowUpdate);
	int HandleGridData(int nClientID, QString strProductKey, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, 
		int nForecastTime, int nTimeRange, float fLon1, float fLon2, float fLat1, float fLat2);
	int HandleStationData(int nClientID, QString strCityType, QString strStationNo, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond,
		int nForecastTime, int nTimeRange, float fLat, float fLon, float fHeight);

	int HandleStationFile(int nClientID, QString strCityType, int nTimeRange, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond);
	int HandleMultiStationData(int nClientID, QString strDataType, int nTimeRange,int nEndForcast, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond);
	
	int HandleGrid2Station(int nClientID, QString strProductKey, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nStartForecast, int nEndForecast);
	int HandleStation2Grid(int nClientID, QString strDataType, int nTimeRange, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nStartForecast, int nEndForecast);
	int HandleRainProcess(int nClientID, QString strProductKey, QString strRelatedProductKey, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nStartForecast, int nEndForecast);

	int HandleUploadStationCfg(int nClientID);

	/* 拼图结果发布 */
	int PublishGrid(QString strProductKey, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nForecastTime, int nTimeRange);
	int PublishStation(QString strDataType, QString strCityType, QString strStationNo, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nForecastTime, int nTimeRange);
	int PublishMultiStation(QString strDataType,  int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nTimeRange);

private:
	bool InitCached();
	int  SaveClientData(int nClientID, QString strProductKey, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond,
		int nForecastTime, int nTimeRange, float fLon1, float fLon2, float fLat1, float fLat2);

private:
	/*处理上传站点配置的对应操作码*/
	int HandleAdd(const QString& stationNum, const QString& stationName,float lat,float lon,float height);
	int HandleDelete(const QString& stationNum, const QString& stationName, float lat, float lon, float height);
	int HandleModify(const QString& stationNum, const QString& stationName, float lat, float lon, float height);

	/* 降水处理 */
	bool HandleRainCorrection_Upward(CachedImpl * pCachedImpl, ST_PRODUCT stProduct, ST_PRODUCT stRelatedProduct, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nForecastTime);
	bool HandleRainCorrection_Downward(CachedImpl * pCachedImpl, ST_PRODUCT stProduct, ST_PRODUCT stRelatedProduct, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nForecastTime);

private:
	HASH_CLIENT       m_hasClient;       // 客户端配置列表
	HASH_PRODUCT      m_hasProduct;      // 管理的产品列表: 注此处的Key都是要合并的关联Key
	QMutex            m_mutexProduct;    // 锁
	HASH_PRODUCT_TASK m_hasProductTask;  // 任务列表 : 注此处的Key都是要合并的关联Key
	QMutex            m_mutexTask;       // 任务锁
	ST_CACHED         m_stCached;        // 缓存配置信息

	TaskTcpServer*    m_pTaskTcpServer;  // socketServer
	TaskStationMsg*   m_pTaskStationMsg; // 站点消息任务
	bool              m_bIsPublish;      // 发布总开关

	QMutex  m_mutexStationCfg;//站点配置锁

};


#endif //NETWORK_MANAGER_H