/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: DataManager.h
* 作  者: zhangl		版本：1.0		日  期：2015/11/20
* 描  述：数据管理器（用于同步格点和站点数据）
* 其  他：
* 功能列表:
* 修改日志：
*   No. 日期		作者		修改内容
*
*************************************************************************/
#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include "DataManagerDef.h"
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
/* 类 名： DataManager                                                  */
/* 父 类：                                                              */
/* 说 明： 数据管理器                                                   */
/* 描 述：                                                              */
/************************************************************************/
class DataManager
{
private:
	DataManager();

public:
	~DataManager();
	static DataManager & getClass();

public:
	HASH_STATION_INFO &GetStationList();
	QString  GetForecastType();
	bool IsStation2Grid();

	/* 初始化 */
	int Init(HASH_PRODUCT hasProduct, ST_CACHED stCached, ST_STATION_MAG stStationMag);
	int Grid2Station(QString strType, int nTimeRange, QString strProductKey, float * fGridData, float fStartLon, float fStartLat, float fDi, float fDj, int nNi, int nNj, int nYear, int nMonth, int nDay, int nHour, int nForecastTime, bool bFirst = false);
	int Station2Grid(QString strStationNo,int nTimeRange, float * fStationData, int nDataLen, int nYear, int nMonth, int nDay, int nHour, int nForecastTime, QString &strCityType, QString &strForecastType, int &nStationRange, bool & bIsUpdate);
	int MultiStation2Grid(QString strDataType, LIST_STATION_VALUE  lstStationDiffVal, float fStartLon, float fStartLat, float fEndLon, float fEndLat, int nYear, int nMonth, int nDay, int nHour, int nTimeRange, int nForecastTime);

	int SaveMultiStation(QString strType, int nYear, int nMonth, int nDay, int nHour, int nTimeRange, int nMaxForecastTime);

	int SetStationData(QString strType, int nTimeRange, int nYear, int nMonth, int nDay, int nHour, int nForecastTime, QString strStationNo, float fData);
	int GetStationData(QString strType, int nTimeRange, int nYear, int nMonth, int nDay, int nHour, int nForecastTime, QString strStationNo, float &fData);

	int flushStationInfoToMemcached();
	int flushStationInfoToLocal();

private:
	bool InitCached();
	int  InitStationList();
	bool GetGridRange(float fStationLon, float fStationLat, float fRadius, float fStartLon, float fStartLat, float fDi, float fDj, int nNi, int nNj, float &fLon1, float &fLon2, float &fLat1, float &fLat2);
	bool GetStationList(float fLon1, float fLon2, float fLat1, float fLat2, float fRadius, LIST_STATION_VALUE &lstStation);
	
	void SetStationProduct(QString strType, int nTimeRange, QString strProductKey);
	bool GetProduct(QString strType, int nTimeRange, ST_PRODUCT &stProduct);

	void ClearStationData();

private:
	ST_CACHED         m_stCached;        // 缓存配置信息
	CachedImpl *      m_pCachedImpl;     // 缓存业务处理
	ST_STATION_MAG    m_stStationMag;    // 站点-格点处理配置

	HASH_PRODUCT      m_hasProduct;      // 格点产品列表
	HASH_STATION_INFO m_hasStaitonInfo;  // 站点列表

	HASH_STATION_DATA m_hasStationData;  // 存储所有插值的站点数据
	QMutex            m_mutexStationData;// 散列站点锁

	HASH_STATION_PRODUCT m_hasStationProduct; // 站点产品匹配表

};

#endif // DATA_MANAGER_H


