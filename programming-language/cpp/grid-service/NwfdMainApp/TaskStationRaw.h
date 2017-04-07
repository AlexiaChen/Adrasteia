/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: TaskStation.h
* 作  者: zhangl		版本：1.0		日  期：2015/09/15
* 描  述：站点数据处理
* 其  他：
* 功能列表:
*   1.
* 修改日志：
*   No. 日期		作者		修改内容
*
*************************************************************************/
#ifndef TASK_STATION_RAW_H
#define TASK_STATION_RAW_H

#include "TaskBase.h"
#include "StationDef.h"
#include "CachedImpl.h"

#include <QFileInfoList>
/************************************************************************/
/* 类 名： TaskProductMerge                                             */
/* 父 类： TaskBase                                                     */
/* 说 明： 产品合并处理类                                               */
/* 描 述：                                                              */
/************************************************************************/
class TaskStationRaw : public TaskBase
{
	Q_OBJECT

public:
	TaskStationRaw();
	~TaskStationRaw();

public:
	void Init(ST_STATION_CFG stStationCfg, ST_CACHED stCached);

protected:
	void run();

private:
	bool InitCached();  // 初始化缓存
	bool ExecTask();    // 执行任务
	bool RawProcess();  // 扫描处理
	bool MergeProcess();// 数据保存处理

	// 获取文件列表
	bool GetProductFileList(QString strSrcFolder, QString strSrcFile, ST_FILE_FORMAT stFileNameFormat, QFileInfoList& lstFile);
	// 保存数据到缓存中
	int  SaveData2Cached(ST_SRC_ITEM stSrcItem, int nYear, int nMonth, int nDay, int nHour, st_station * pStation, int nStationCnt);

	// 保存数据处理
	bool IsTimeOut(int nHour, int nMinute);
	int  SaveProcess(ST_REPORTTIME stForecast, int nYear, int nMonth, int nDay, int nHour);
	int  GetStationData(char * strStationNo, QString strSaveRule, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, float * &fData, int &nRow, int &nCol);

	int  nwfd_get_stationdata_forecasttime(const char * strCityType, const char * lstRule, int nTimeRange, const char * lstStationNo, 
		int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, float * &fData, nwfd_station_data *& stDataInfo, int &nCnt);

	time_t GetTime(int year, int month, int day, int hour, int minute, int second, int forecast = -1);
	void GetForecastTime(int nYear, int nMonth, int nDay, int nHour, int nForecastTime, int &year, int &month, int &day, int &hour);
	int  GetStationList(QString strSaveStation, LIST_STATION_INFO &lstStationInfo);

	// 标记处理
	bool IsHasProcessed(QString strFile);
	bool IsHasSaved(QString strTime);
	void HandleClearHash();
	
	int DataDistribution(QStringList lstDDS, QString strFileName, QString strPath);
private:
	bool           m_bReady;       // run执行标记
	ST_STATION_CFG m_stStationCfg; // 站点配置信息
	ST_CACHED      m_stCached;     // 缓存配置信息
	CachedImpl *   m_pCachedImpl;  // 缓存业务处理

	QString        m_strClearHasFlg;// 已处理产品Hash标记维护判断

	typedef QHash <QString, bool> Hash_ProductFile;  // 已处理成功的产品文件 <产品Key，处理结果>
	Hash_ProductFile  m_hasCompleteProduct_today;    // 已处理产品标记_今天
	Hash_ProductFile  m_hasCompleteProduct_yesterday;// 已处理产品标记_昨天
	Hash_ProductFile  m_hasCompleteSave_today;       // 已保存过的时效_今天
	Hash_ProductFile  m_hasCompleteSave_yesterday;   // 已保存过的时效_昨天

};

#endif // TASK_STATION_RAW_H
