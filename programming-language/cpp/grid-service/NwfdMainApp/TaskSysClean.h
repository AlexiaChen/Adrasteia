/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: TaskSysClean.h
* 作  者: zhangl		版本：1.0		日  期：2015/05/06
* 描  述：缓存维护类，及时清理数据
* 其  他：
* 功能列表:
*   1. 维护缓存中，Product数据
*   2. 维护缓存中，Merge数据
*   3. 维护缓存中，Client数据
*   4. 维护数据库中的数据，保留30天数据
* 修改日志：
*   No. 日期		作者		修改内容
*
*************************************************************************/
#ifndef TASK_SYSCLEAN_H
#define TASK_SYSCLEAN_H

/************************************************************************/
/* 头文件引用                                                           */
/************************************************************************/
#include "TaskBase.h"
#include "ProductDef.h"
#include "CachedImpl.h"
#include "NwfdClientDef.h"
#include "StationDef.h"
#include "HandleDB.h"

/************************************************************************/
/* 宏定义                                                               */
/************************************************************************/
#define MAX_RECONNECT_NUM     10   // 最大重连次数10次
#define MIN_LOG_KEEP_DAY      2    // 日志最少保留天数
#define MIN_DB_KEEP_DAY       5    // 数据库最少保留天数
#define MIN_FILE_KEEP_DAY     15   // 文件最少保留天数

// 系统清理维护
typedef struct _SYSCLEAN
{
	bool  bDisabled;       // 是否进行检查
	int   nInterval;       // 维护扫描时间
	int   nLogKeepDay;     // 日志保留天数
	int   nDBKeepDay;      // 数据库保留天数
	int   nFileKeepDay;    // 文件保留天数
	QStringList lstFileDir;// 文件目录列表
}ST_SYSCLEAN;

/************************************************************************/
/* 类 名： TaskSysClean                                                 */
/* 父 类： TaskBase                                                     */
/* 说 明： 系统清理类                                                   */
/* 描 述：                                                              */
/************************************************************************/
class TaskSysClean : public TaskBase
{
	Q_OBJECT

public:
	TaskSysClean();
	~TaskSysClean();

public:
	void Init(ST_CACHED stCached, ST_SYSCLEAN stSysClean, HASH_PRODUCT hasProduct, HASH_CLIENT  hasClient, LIST_STATION_CFG lstStationCfg, ST_STATION_MAG stStationMag);
	// 设置
	void setRootPath(QString strRootPath);
	void setCCCC(QString strCCCC);

protected:
	void run();

private:
	bool ExecTask();  

	// 检查Key
	bool CheckProductKey();
	bool CheckMergeKey();

	// 产品
	bool CheckProduct();
	bool CheckProductData(QString strProductKey, int nValidTime);

	// 拼图
	bool CheckMerge();
	bool CheckMergeData(QString strProductKey, int nValidTime);

	// 客户端
	bool CheckClient();
	bool CheckClientData(int nClientID, QString strProductKey, int nValidTime);

	// 检查站点数据
	bool CheckStation();
	bool CheckStationData(QString strCityType, QString strDataType, int nTimeRange, const char * szStationNo, int nValidTime);
	bool CheckStationManage();

	// 公共方法
	bool FindProduct(QString strProductKey);
	bool IsValidTime(QString strProductTime, int nValidTime);

	// 检查日志
	bool CheckLog();

	// 检查数据库
	bool CheckDataBase();

	// 检查文件
	bool CheckFile();
	bool HandleFile(QString strPath);

private:
	QString      m_strRoot;     // 数据存储根目录
	QString      m_strCccc;     // CCCC4位省编码

	bool         m_bReady;      // run执行标记
	bool         m_bFirst;      // 是否为首次
	CachedImpl  *m_pCachedImpl; // 缓存连接指针
	ST_CACHED    m_stCached;    // 缓存配置信息
	ST_SYSCLEAN   m_stSysClean;   // 维护配置

	HASH_PRODUCT m_hasProduct;  // 产品列表
	HASH_CLIENT  m_hasClient;   // 客户端列表
	LIST_STATION_CFG m_lstStationCfg; // 站点配置列表
	ST_STATION_MAG   m_stStationMag;     // 站点管理：用于格点-站点转换

	int m_nReConTryNum;         // 重连尝试次数
};

#endif // TASK_SYSCLEAN_H
