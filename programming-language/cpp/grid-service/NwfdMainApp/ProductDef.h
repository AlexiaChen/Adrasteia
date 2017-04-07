/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: ProductDef.h
* 作  者: zhangl		版本：1.0		日  期：2015/04/29
* 描  述：产品定义
* 其  他：
* 功能列表:
*   1.
* 修改日志：
*   No.    日期		作者			修改内容
*   1  2015/05/26  zhangl    对chains的格式进行修改
*************************************************************************/
#ifndef PRODUCT_DEF_H
#define PRODUCT_DEF_H

/************************************************************************/
/* 头文件引用                                                           */
/************************************************************************/
#include <QString>
#include <QList>
#include <QHash>
#include <QStringList>

/************************************************************************/
/* 1. 产品格式宏定义                                                    */
/************************************************************************/
#define  FORMAT_MICAPS4   "micaps4"
#define  FORMAT_MICAPS11  "micaps11"
#define  FORMAT_GRIB2     "grib2"
#define  FORMAT_UNKNOWN   "unknown"

// 合并方式 二选其一
#define  MERGE_WAY_FILE   1  // 通过文件内容
#define  MERGE_WAY_CACHED 2  // 通过缓存内容

/************************************************************************/
/* 2. 产品定时处理时间                                                  */
/************************************************************************/
typedef struct _PROTIME
{
	short nHour;     // 时
	short nMin;      // 分
	short nSecond;   // 秒
	int   nInterval; // 间隔
}ST_PROTIME;
typedef QList<ST_PROTIME> LIST_PROTIME;

typedef struct _SCANTIME
{
	LIST_PROTIME lstProTime;// 处理时间列表
	int nInterval;  // 默认时间间隔，当没有时间列表时，按照扫描间隔进行处理
}ST_SCANTIME;
/************************************************************************/
/* 3. 文件预报时间间隔                                                  */
/************************************************************************/
// 3.1 时间间隔列表
typedef struct _RANGE
{
	int  nForcastTime;
	int  nTimeRange;
}ST_RANGE;
typedef QList<ST_RANGE> LIST_RANGE;

// 3.2 起报时间
typedef struct _FORCAST
{
	int  nReportTime;   // 起报时间
	int  nFileCount;    // 文件个数
	int  nStartForecast;// 起始预报
	int  nEndForecast;  // 终止预报
	int  nStartTime;    // 处理-起始时
	int  nEndTime;      // 处理-起始分
	int  nLastHour;     // 文件合并-最迟合并时
	int  nLastMinute;   // 文件合并-最迟合并分
	LIST_RANGE lstRange;// 时间间隔列表
}ST_FORECAST;
typedef QHash<int, ST_FORECAST> HASH_FORECAST;

// 3.3 时间间隔
typedef struct _TIMERANGE
{
	HASH_FORECAST hashTimeRange;   // 时间间隔列表
}ST_TIMERANGE;

/************************************************************************/
/* 4. 文件保存结构体                                                    */
/************************************************************************/
typedef struct _FILESAVE
{
	bool    bDisabled;    // 是否禁用
	QString strSavePath;  // 产品保存目录
	QString strFileName;  // 产品保存文件名
	QString strFileFormat;// 产品文件保存类型默认是GRIB2
	QStringList lstDDS;   // 数据分发服务器列表
}ST_FILESAVE;

/************************************************************************/
/* 5. 产品数据加工处理                                                  */
/************************************************************************/
// 加工处理链
typedef struct _CHAIN
{
	QString strLibName;     // 调用动态库名称
	QString strParams;      // 参数
}ST_CHAIN;
typedef QList<ST_CHAIN> LIST_CHAINS;

// 数据加工处理
typedef struct _DATAPROC
{
	int nID;          // 数据加工处理编号
	QString strName;  // 数据加工处理名称标识
	ST_FILESAVE stFileSave; // 文件保存
	LIST_CHAINS lstChains;  // 数据加工处理链
}ST_DATAPROC;

// 数据加工处理组
typedef QHash<int,ST_DATAPROC> HASH_CHAINS_GROUP;
/************************************************************************/
/* 6. 缓存保存                                                          */
/************************************************************************/
typedef struct _CACHEDSAVE
{
	bool bDisabled;   // 是否不生效
	int  nValidTime;  // 缓存中的有效时间 
}ST_CACHEDSAVE;

/************************************************************************/
/* 7. 文件格式解析位置信息                                              */
/************************************************************************/
// 位置信息
typedef struct _POS
{
	short iStart; // 起始位置
	short iLen;   // 数据长度
}ST_POS;

// 文件名解析依据
typedef struct _FILE_FORMAT
{
	int     nLength;   // 文件长度
	ST_POS  stYear;    // 年
	ST_POS  stMonth;   // 月
	ST_POS  stDay;     // 日
	ST_POS  stHour;    // 时
	ST_POS  stMinute;  // 分
	ST_POS  stSecond;  // 秒
	ST_POS  stForcast; // 预报
	ST_POS  stRange;   // 间隔
}ST_FILE_FORMAT;

/************************************************************************/
/* 8. 数据分发定义                                                      */
/************************************************************************/
// 数据分发服务器列表（编号）
typedef QList<int>  LIST_DDS_ID;  

// 数据分发源文件类型定义
typedef enum _FILE_TYPE
{
	_FILE_UNKNOWN = 0,  // 不知类型
	_FILE_GRIB2   = 1,  // 单个生成的grib2文件
	_FILE_MERGE   = 2,  // 合并后的grib2文件
	_FILE_CHAINS  = 3   // 数据加工处理后的grib2文件
}EM_FILE_TYPE;

// 数据分发文件-服务器关联定义
typedef struct _FILE_DDS
{
	EM_FILE_TYPE emType; // 数据分发文件类型
	QStringList  lstDDS;  // 数据分发服务器列表
}ST_FILE_DDS;

// 文件-服务器关联表
typedef QList<ST_FILE_DDS> LIST_FILE_DDS;

// 产品数据分发定义
typedef struct _PRODUCT_DDS
{
	bool    bDisabled;        // 是否禁用
	LIST_FILE_DDS lstFileDDS; // 数据分发列表
}ST_PRODUCT_DDS;

/************************************************************************/
/* 9. merge: template                                                   */
/************************************************************************/
typedef struct _TEMPLATE
{
	QString  strTemplateType; // 合并用模版类型 blank：空白缺测数据、product:指导报
	float    fBlankValue;     // 缺测值
}ST_TEMPLATE;

/************************************************************************/
/* 10. merge：mask文件配置                                              */
/************************************************************************/
typedef struct _MASK
{
	QString strMaskFile; // Mask文件全路径
	float fLon1; // 起始经度
	float fLon2; // 终止经度
	float fLat1; // 起始纬度
	float fLat2; // 终止纬度
	float fDi;   // 经度格距
	float fDj;   // 纬度格距
	int   nNi;   // 经度格点数
	int   nNj;   // 纬度格点数
}ST_MASK;

/************************************************************************/
/* 11. merge: 数据源合并定义                                            */
/************************************************************************/
typedef struct _MERGE_ITEM
{
	int nClientID; // 客户端编号
	float fLon1;   // 合并起始经度
	float fLon2;   // 合并终止经度
	float fLat1;   // 合并起始纬度
	float fLat2;   // 合并终止纬度

	QString strSrcFolder;  // 源文件存放目录
	QString strSrcFileName;// 源文件名格式
	ST_FILE_FORMAT stFileNameFormat; // 源文件名解析
}ST_MERGE_ITEM;

// 合并数据源列表
typedef QHash<int, ST_MERGE_ITEM>  HASH_MERGE_ITEM; 
typedef QList<ST_MERGE_ITEM>  LIST_MERGE_ITEM;

/************************************************************************/
/* 12. 转站点配置                                                       */
/************************************************************************/
typedef struct _STATION_PROC
{
	bool bDisabled;      // 是否禁用
	QString strProcLib;  // 处理用动态库
} ST_STATION_PROC;

/************************************************************************/
/* 13. 降水处理                                                         */
/************************************************************************/
typedef struct _RAIN_PROC
{
	int nRange;             // 时次间隔，3，6，12
	QString strRelatedKey;  // 关联Key
} ST_RAIN_PROC;
typedef QList<ST_RAIN_PROC> LIST_RAIN_PROC;

/************************************************************************/
/* 14. 产品配置信息结构体                                               */
/************************************************************************/
typedef struct _PRODUCT
{
	QString strKey;       // 产品标识Key(唯一)
	QString strName;      // 产品名称
	QString strRole;      // 产品业务类型（区别业务处理方法）
	QString strType;      // 产品类型
	QString strPublisher; // 发布者

	int   nCategory;      // 类别码
	int   nElement[2];    // 元素码
	int   nStatistical;   // 统计码
	int   nStatus;        // 状态码
	int   nCnt;           // 一个文件中的产品数据个数
	float fLon1;          // 起始经度
	float fLon2;          // 终止经度
	float fLat1;          // 起始纬度
	float fLat2;          // 终止纬度
	float fDi;            // 经度格距
	float fDj;            // 纬度格距
	float fOffSet;        // 数据偏移量
	int   nNi;            // 经度格点数
	int   nNj;            // 纬度格点数

	float fMissingVal;  // 缺测值
	QString strLineVal; // micaps文件中最后5个等值线相关数据

	int nTimeRange;       // 时间间隔
 	int nMaxForecastTime; // 最大预报时间
	QString strBusinessStatus; // 业务类型
	bool    bIsWarning;        // 是否是预警数据


	QString strSrcFolder;  // 源文件存放目录
	QString strSrcFileName;// 源文件名格式
	ST_FILE_FORMAT stFileNameFormat;// 源文件名格式

	QString        strDecMode;      // 解码模式
	QString        strScanMode;     // 扫描模式
	ST_SCANTIME    stScanTime;      // 扫描处理时间
	HASH_FORECAST  hasForecast;     // 预报配置列表
	ST_CACHEDSAVE  stCachedSave;    // 缓存保存
	ST_FILESAVE    stFileSave;      // 单个产品文件保存
	ST_FILESAVE    stMergeFileSave; // 合并后的产品保存
	HASH_CHAINS_GROUP  hasDataProc; // 数据加工处理组
	//ST_PRODUCT_DDS stProductDDS;  // 数据分发配置
	ST_STATION_PROC stStationProc;  // 格点转站点处理

	// merge专用 start
	QString      strRelatedKey;    // 关联产品Key
	ST_TEMPLATE  stTemplate;       // 合并用模版
	ST_MASK      stMask;           // mask文件定义
	int          nMergeWay;        // 合并方式：通过文件，缓存提取
	LIST_MERGE_ITEM lstMergeItem;  // 合并数据源列表
	LIST_RAIN_PROC  lstRainProc;   // 降水处理

	// merge专用 end
}ST_PRODUCT, *LPST_PRODUCT;


typedef struct _DB_MSG_ITEM{
	int clientID;
	int mergeID;
	QString productKey;
	QString relatedProductKey;
	QString messageType;

	int year;
	int month;
	int day;

	int hour;
	int min;
	int sec;

	int startForecastTime;
	int ForecastTime;
	int timeRange;

	float lon1;
	float lon2;
	float lat1;
	float lat2;
}ST_DB_MSG_ITEM;

typedef struct _DB_TASK_MSG{
	
	int id;
	
	QString taskStatus;
	QString productRole;
	QString productKey;

	int year;
	int month;
	int day;
	
	int reporttime;
	int forecasttime;

	QString folder;
	QString filename;

	QString starttime;
	QString finishtime;

	int processNum;

	QString errReason;


}ST_DB_TASK_MSG;

// 产品列表
typedef QList<ST_PRODUCT>  LIST_PRODUCT;
typedef QHash<QString, ST_PRODUCT> HASH_PRODUCT;  // <Key, ST_Product>

#endif //PRODUCT_DEF_H


