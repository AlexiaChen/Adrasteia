/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: TaskProductRaw.h
* 作  者: zhangl		版本：1.0		日  期：2015/04/29
* 描  述：产品扫描处理任务类
* 其  他：
* 功能列表:
*   1. 定时处理产品目录下的所有Micap4文件，生成Grib2文件并将数据存储到缓存中
*   2. 根据配置，对数据进行加工处理，生成Grib2文件
*   3. 根据配置，对一个时次的数据进行合并处理，存储到一个Grib2文件中
*
* 修改日志：
*   No. 日期		作者		修改内容
*
*************************************************************************/
#ifndef TASKPRODUCTRAW_H
#define TASKPRODUCTRAW_H

/************************************************************************/
/* 头文件引用                                                           */
/************************************************************************/
#include "TaskBase.h"
#include "ProductDef.h"
#include "CachedImpl.h"
#include "HandleDataProLib.h"
#include "NWFD-g2clib.h"
#include "StationDef.h"

#include <QFileInfoList>

/************************************************************************/
/* 宏定义                                                               */
/************************************************************************/
#define MAX_MERGE_FILE_NUM    200

/************************************************************************/
/* 类 名： TaskProductRaw                                               */
/* 父 类： TaskBase                                                     */
/* 说 明： 产品扫描处理类                                               */
/* 描 述：                                                              */
/************************************************************************/
class TaskProductRaw : public TaskBase
{
	Q_OBJECT

public:
	TaskProductRaw();
	~TaskProductRaw();

public:
	void Init(ST_PRODUCT stProduct, ST_CACHED stCached);
	// 设置
	void setRootPath(QString strRootPath);
	void setCCCC(QString strCCCC);
	void setGridRange(float lon1, float lon2, float lat1, float lat2);

private:
	bool InitChains();
	void ClearChainsGroup();
	void ClearChains(LIST_DATAPROC lstDataProc);
	bool InitCached();
	bool CheckCachedStatus();

protected:
	void run();

private:
	/* 执行扫描任务 */
	bool ExecRawTask();

	/* 按照扫描模式进行处理 */
	bool HandleLocalDataFromMode();
	int  HandleProcess(int nYear, int nMonth, int nDay, int nHour);
	int  HandleAutoProcess(int nYear, int nMonth, int nDay);

	/* 单个产品处理 */
	bool HandleProduct(QString strSrcFolder, int nYear, int nMonth, int nDay, int nHour);
	int Process(QString strPath, QString strFileName, int nYear, int nMonth, int nDay, int nHour, int nForecastTime);
	int AutoProcess(QString strPath, QString strFileName, int nYear, int nMonth, int nDay);

	bool SaveFile(QString strsSrcFileName, float *fData, int nYear, int nMonth, int nDay, int nHour, int nForecastTime, int nTimeRange);
	bool SaveCached(QString strsSrcFileName, float *fData, int nYear, int nMonth, int nDay, int nHour, int nForecastTime, int nTimeRange);
	bool Grid2Station(QString strsSrcFileName, float *fData, int nYear, int nMonth, int nDay, int nHour, int nForecastTime, int nTimeRange);
	bool DataProcess(QString strsSrcFileName, float *fData, int nYear, int nMonth, int nDay, int nHour, int nForecastTime, int nTimeRange);
	int  DataProcess(ST_DATAPROC stDataProc, LIST_DATAPROC lstDataProc, float *fData, nwfd_data_field *stDataFieldInfo, int nCnt);

	/* 文件合并处理 */
	bool HandleProductBatch(QString strSrcFolder, int nYear, int nMonth, int nDay, int nHour);
	int  ProcessBatch(QString strSrcFolder, int nYear, int nMonth, int nDay, int nHour, bool bFileTimeUpdate = false);
	
	/* 获取产品列表 */
	bool GetProductFileList(QString strSrcFolder, QString strFilterName, QFileInfoList& list);

	/* 处理标记 */
	void HandleClearHash();
	void SetProcessedResult(QString strFlg, bool bResult);
	bool IsHasProcessed(QString strFlg);
	bool IsHasProcessedBatch(QString strFlg);
	bool IshasNotWholeBatchProcessed(QString strBatchName);

	// 时间判断
	bool IsLastTimeReached(int nReportTime);
	bool IsProductTimeValid(int nYear, int nMonth, int nDay, int nHour, int nValidTime);
	bool FindDataProcInfo(int nID, ST_DATAPROC & stDataProc);

	void ClearBatchFile();
	void SetFileCreatedTime(QString strBatchName, QString strFile, QString strCreatedTime);
	bool IsFileTimeUpdate(QString strBatchName);

private:
	QString      m_strRoot;     // 数据存储根目录
	QString      m_strCccc;     // CCCC4位省编码
	float        m_fLon1;       // 起始经度
	float        m_fLon2;       // 终止经度
	float        m_fLat1;       // 起始纬度
	float        m_fLat2;       // 终止纬度

	bool          m_bReady;       // run执行标记
	CachedImpl *  m_pCachedImpl;  // 缓存业务处理
	ST_PRODUCT    m_stProduct;    // 产品信息
	ST_CACHED     m_stCached;     // 缓存配置信息
	HASH_DATAPROC_GROUP m_hasDataProcGroup;  // 数据加工处理组

	typedef QHash <QString, bool> Hash_ProductFile;  // 已处理成功的产品文件 <产品Key，处理结果>
	Hash_ProductFile  m_hasCompleteProduct_today;    // 已处理产品标记_今天
	Hash_ProductFile  m_hasCompleteBatch_today;      // 已处理过的产品批次_今天
	Hash_ProductFile  m_hasCompleteProduct_yesterday;// 已处理产品标记_昨天
	Hash_ProductFile  m_hasCompleteBatch_yesterday;  // 已处理过的产品批次_昨天

	Hash_ProductFile  m_hasNotWholeBatch_today;      // 已处理的不完整批次
	Hash_ProductFile  m_hasNotWholeBatch_yesterday;  // 已处理的不完整批次

    // 文件时间戳
	typedef QHash<QString, QString> Hash_FileTime;
	typedef QHash<QString, Hash_FileTime *> Hash_BatchTime;
	Hash_BatchTime    m_hasBatchFileTime;

	QString           m_strClearHasFlg;              // 已处理产品Hash标记维护判断
};

#endif // TASKPRODUCTRAW_H


