/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: TaskProductClientMerge.h
* 作  者: zhangl		版本：1.0		日  期：2015/10/10
* 描  述：客户端上传数据实时合并处理
* 其  他：
* 功能列表:
*   1.
* 修改日志：
*   No. 日期		作者		修改内容
*
*************************************************************************/
#ifndef TASK_PRODUCT_CLIENT_MERGE_H
#define TASK_PRODUCT_CLIENT_MERGE_H

#include "TaskBase.h"
#include "ProductDef.h"
#include "CachedImpl.h"
#include "HandleNwfdLib.h"
#include "NwfdClientDef.h"

#include <QFileInfoList>
/************************************************************************/
/* 类 名： TaskProductClientMerge                                        */
/* 父 类： TaskBase                                                     */
/* 说 明： 客户端上传数据合并处理类                                     */
/* 描 述：                                                              */
/************************************************************************/
class TaskProductClientMerge : public TaskBase
{
	Q_OBJECT

public:
	TaskProductClientMerge();
	~TaskProductClientMerge();

public:
	void Init(ST_PRODUCT stProduct, ST_CACHED stCached);
	//void InitProduct(ST_PRODUCT stProduct, HASH_PRODUCT& hasProduct);

protected:
	void run();

private:
	bool InitCached();  // 初始化缓存
	bool ExecTask(); // 执行任务

	bool HandleClientData(ST_MergeData stMergeData);  // 1-对客户端发来的数据进行合并
	bool HandleRainProc(ST_MergeData stMergeData);
	bool HandleGrib2Station(ST_MergeData stMergeData);


	// 1-对客户端发来的数据进行合并相关处理
	float* GetSrcData(int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nForecastTime, int nTimeRange, nwfd_grib2_field* &stDataField, int &nCnt);
	float* GetClientData(int nClient, QString strProductKey, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nForecastTime, int nTimeRange);

	bool GetMaskLonLat(ST_MASK stMask, int nPos, float &fLon, float &fLat);
	bool GetLonLat(float fLon1, float fLat1, float fDi, float fDj, int nNi, int nNj, int nPos, float &fLon, float &fLat);
	int GetMergerPos(float fMaskLon, float fMaskLat, float fStartLon, float fStartLat, int nNi);

	bool HandleComplete(float *fData, int nDataLen, int nCnt, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nForecastTime, int nTimeRange, int nFlg);
	bool IsAllowMerge(int nClientID);

	bool HandleRainProc(float *fData, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nForecastTime);
	bool HandleRainCorrection_Upward(ST_RAIN_PROC stRainPoc, float *fData, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nForecastTime);
	bool HandleRainCorrection_Downward(ST_RAIN_PROC stRainPoc, float *fData, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nForecastTime);

	/* 降水处理 */
	bool HandleRainCorrection_Upward(CachedImpl * pCachedImpl, ST_PRODUCT stProduct, ST_PRODUCT stRelatedProduct, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nForecastTime);
	bool HandleRainCorrection_Downward(CachedImpl * pCachedImpl, ST_PRODUCT stProduct, ST_PRODUCT stRelatedProduct, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nForecastTime);

private:
	bool          m_bReady;       // run执行标记
	ST_PRODUCT    m_stProduct;    // 产品信息
	//ST_DB_MSG_ITEM m_firstItem;     //队首的产品处理消息
	ST_CACHED     m_stCached;     // 缓存配置信息
	CachedImpl *  m_pCachedImpl;  // 缓存业务处理
	//HASH_PRODUCT m_hasProduct;

	typedef QHash <QString, bool> Hash_ProductFile;  // 已处理成功的产品文件 <产品Key，处理结果>
	Hash_ProductFile  m_hasCompleteProduct_today;    // 已处理产品标记_今天
	Hash_ProductFile  m_hasCompleteProduct_yesterday;// 已处理产品标记_昨天
	QString           m_strClearHasFlg;              // 已处理产品Hash标记维护判断

	typedef QHash<QString, int>  Hash_MergeData;     // key:2015082708_003(YYYYMMDDHH_FFF), value:个数
	Hash_MergeData   m_hasMergeData;  // 当日合并数据记录 
	
};

#endif // TASK_PRODUCT_CLIENT_MERGE_H
