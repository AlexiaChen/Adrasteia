/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: TaskProductMerge.h
* 作  者: zhangl		版本：1.0		日  期：2015/04/29
* 描  述：产品合并处理
* 其  他：
* 功能列表:
*   1.
* 修改日志：
*   No. 日期		作者		修改内容
*
*************************************************************************/
#ifndef TASKPRODUCTMERGE_H
#define TASKPRODUCTMERGE_H

#include "TaskBase.h"
#include "ProductDef.h"
#include "CachedImpl.h"
#include "HandleNwfdLib.h"

#include <QFileInfoList>

/************************************************************************/
/* 类 名： TaskProductMerge                                             */
/* 父 类： TaskBase                                                     */
/* 说 明： 产品合并处理类                                               */
/* 描 述：                                                              */
/************************************************************************/
class TaskProductMerge : public TaskBase
{
	Q_OBJECT

public:
	TaskProductMerge();
	~TaskProductMerge();

public:
	void Init(ST_PRODUCT stProduct, ST_CACHED stCached);

protected:
	void run();

private:
	bool InitCached();  // 初始化缓存
	bool ExecMergeTask(); // 执行任务

	bool HandleClientData();  // 1-对客户端发来的数据进行合并
	bool HandleLocalData();   // 2-扫描本地目录，进行数据合并
	bool HandleLocalDataFromMode(); // 按照扫描模式进行处理
	bool HandleMergeData();   // 3-此情况缓存中的数据就是最新的拼图数据，将其保存到文件中

	// 1-对客户端发来的数据进行合并相关处理
	float* GetSrcData(int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nForecastTime, int nTimeRange, nwfd_grib2_field* &stDataField, int &nCnt);
	float* GetClientData(int nClient, QString strProductKey);
	float* GetBlankData();

	bool GetMaskLonLat(ST_MASK stMask, int nPos, float &fLon, float &fLat);
	bool GetLonLat(float fLon1, float fLat1, float fDi, float fDj, int nNi, int nNj, int nPos, float &fLon, float &fLat);
	int GetMergerPos(float fMaskLon, float fMaskLat, float fStartLon, float fStartLat, int nNi);
	bool HandleComplete(float *fData, int nDataLen, int nCnt, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nForecastTime, int nTimeRange, int nFlg);
	int  WriteData(QString strSavePath, QString strSaveFile, float *fData, int nDataLen, int nCnt, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nForecastTime, int nTimeRange, QStringList lstDDS, bool bIsMergeFile);
	int  WriteData2Micaps(QString strSavePath, QString strSaveFile, float *fData, int nYear, int nMonth, int nDay, int nHour, int nForecastTime, QStringList lstDDS);

	// 2-扫描本地目录，进行数据合并相关处理
	void HandleClearHash();
	bool IsHasProcessed(QString strName);
	bool GetProductFileList(QString strPath, QString strSrcFile, QFileInfoList& lstFile);

	int ProcessLocalMerge(int nYear, int nMonth, int nDay, int nHour);
	int ProcessLocalMerge(int nYear, int nMonth, int nDay, int nHour, int nForecastTime, int nTimeRange);
	int ProcessLocalMerge(ST_MERGE_ITEM stMergeItem, QString strFileName);
	float * GetClientLocalData(QString strPath, QString strFileName, nwfd_grib2_field* &stDataField, int &nCnt);
	int GetClientLocalData(QString strPath, QString strFileName, int nYear, int nMonth, int nDay, int nHour, int nForecastTime, float* & fData, nwfd_grib2_field* &stDataField, int &nCnt);

	int SaveData2GribFile(float *fData, int nCnt, ST_PRODUCT stProduct, int nYear, int nMonth, int  nDay, int  nHour, int  nForecasttime, int nTimeRange, QString strGribFile, bool bIsMergeFile);
	int DataDistribution(QStringList lstDDS, QString strFileName, QString strPath);

	// 是否是最后处理时间
	bool bHandleLastTime(int nHour);
private:
	bool          m_bReady;       // run执行标记
	ST_PRODUCT    m_stProduct;    // 产品信息
	ST_CACHED     m_stCached;     // 缓存配置信息
	CachedImpl *  m_pCachedImpl;  // 缓存业务处理

	typedef QHash <QString, bool> Hash_ProductFile;  // 已处理成功的产品文件 <产品Key，处理结果>
	Hash_ProductFile  m_hasCompleteProduct_today;    // 已处理产品标记_今天
	Hash_ProductFile  m_hasCompleteProduct_yesterday;// 已处理产品标记_昨天
	QString           m_strClearHasFlg;              // 已处理产品Hash标记维护判断

	typedef QHash<QString, int>  Hash_MergeData;     // key:2015082708_003(YYYYMMDDHH_FFF), value:个数
	Hash_MergeData   m_hasMergeData;  // 当日合并数据记录 
	
};

#endif // TASKPRODUCTMERGE_H
