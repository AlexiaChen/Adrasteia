/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: StationDef.h
* 作  者: zhangl		版本：1.0		日  期：2015/08/04
* 描  述：站定义
* 其  他：
* 功能列表:
*   1.
* 修改日志：
*   No. 日期		作者		     修改内容
*************************************************************************/
#ifndef STATION_DEF_H
#define STATION_DEF_H

#include "ProductDef.h"
#include <QDateTime>
#include <QList>

/************************************************************************/
/* 1. 站点数据定义                                                      */
/************************************************************************/
typedef struct _station
{
	char   stationNo[10]; // 站号
	float  fLon;      // 经度
	float  fLat;      // 纬度
	float  fHeight;   // 高度
	int    nRow;      // 行个数
	int    nCol;      // 列个数
	float* fData;     // 数据
}st_station;

/************************************************************************/
/* 2.1  元数据列表定义                                                  */
/************************************************************************/
typedef struct _SRC_ITEM
{
	QString strDataType;     // 数据类型： SCMOC 国家指导报; SPVT 省指导报; SCMOU 预报员预报; SPCC 省订正预报
	QString strSymbol;       // 标识符号
	QString strDescription;  // 描述
	QString strSrcFolder;    // 源文件存放目录
	QString strSrcFileName;  // 源文件名格式
	ST_FILE_FORMAT stFileNameFormat; // 源文件名解析
	QStringList lstDDS;   // 数据分发服务器列表-用于直接转送文件的情况
}ST_SRC_ITEM;

// 站点合并顺序列表
typedef QList<ST_SRC_ITEM>  LIST_SRC_ITEM;

/************************************************************************/
/* 2.2  起报时间定义和合并时间定义                                      */
/************************************************************************/
typedef struct _REPORTTIME
{
	int  nReportTime;   // 起报时间
	int  nSaveHour;     // 文件生成-时
	int  nSaveMinute;   // 文件生成-分
	QString strSaveStation;  // 文件保存的站点列表-从配置的文件中读取站点信息
	QString strSaveRule;     // 文件保存规则
	QString strSaveType;     // 保存类型 SCMOC，SPCC
	ST_FILESAVE stSaveFile;  // 文件保存目录和名称
}ST_REPORTTIME;

typedef QList<ST_REPORTTIME> LIST_REPORT_TIMES; // 时次定义

/************************************************************************/
/* 2.3 站点配置定义                                                     */
/************************************************************************/
typedef struct _STATION_CFG
{
	QString  strRole;          // 处理方式
	QString  strIDSymbol;      // 城市ID标号
	QString  strCityType;      // 城市类型：城镇 town; 大城市 bigcity
	int      nTimeRange;       // 时间间隔 - 文件中数据间隔
	int      nMaxForecastTime; // 最大预报时效 - 文件中最大预报时效
									 
	ST_SCANTIME    stScanTime;       // 扫描时间
	LIST_SRC_ITEM  lstSrcItem;       // 元数据列表
	ST_CACHEDSAVE  stCachedSave;     // 缓存存储设置
	LIST_REPORT_TIMES lstReportTimes;// 时次处理列表

}ST_STATION_CFG;

typedef QList<ST_STATION_CFG> LIST_STATION_CFG;  // 站点处理配置信息列表

/************************************************************************/
/* 3 站点信息                                                           */
/************************************************************************/
typedef struct _STATION_INFO
{
	char   stationNo[10];// 站号
	char   stationName[256]; //站点名称
	float  fLon;      // 经度
	float  fLat;      // 纬度
	float  fHeight;   // 高度
}ST_STATION_INFO;

typedef QList<ST_STATION_INFO> LIST_STATION_INFO; // 站号信息列表
typedef QHash<QString, ST_STATION_INFO> HASH_STATION_INFO; // 站号信息列表

/************************************************************************/
/* 4 客户端数据获取结构体                                               */
/************************************************************************/
struct nwfd_station_data
{
	char  type[32];      // 数据类型 SCMOC，SNWFD, SPVT, SPCC，SCMOU...
	char  stationNo[10]; // 站号
	float lon;     // 经度
	float lat;     // 纬度
	float height;  // 高度
	int   length;  // 数据列数（数据长度）
					    
	int year;      // 年
	int month;     // 月
	int day;       // 日
	int hour;      // 时
	int minute;    // 分
	int second;    // 秒
	int forecast;  // 预报时间
	int timerange; // 时间间隔
};

/************************************************************************/
/* 5 数据库对应表结构                                                   */
/************************************************************************/
typedef struct _StationData
{
	int     nID;
	QString strStatus;
	QString strDataType;
	QString strCityType;

	int     nClientID;    // 客户端编号
	int     nStationType; // 1: 单个站点  2：站点文件

	int     nYear;
	int     nMonth;
	int     nDay;
	int     nHour;
	int     nMinute;
	int     nSecond;
	int     nForecastTime;
	int     nTimeRange;

	QString stationNo;      // 站点编号
	float temperature;      // 温度
	float humidity;         // 相对湿度
	float windDirection;    // 风向
	float windSpeed; 		// 风速
	float airPressure;		// 气压
	float precipitation;	// 降水量
	float cloud;			// 总云量
	float lCloud;			// 低云量
	float weather;			// 天气现象
	float visibility;		// 能见度
	float tMax;				// 最高气温
	float tMin;				// 最低气温
	float hMax;				// 最大相对湿度
	float hMin;				// 最小相对湿度
	float precipitation24H;	// 24小时累计降水量
	float rProbability12H;	// 12小时累计降水量
	float cloud12H;			// 12小时总云量
	float lCloud12H;		// 12小时低云量
	float weather12H;		// 12小时天气现象
	float windDirection12H;	// 12小时风向
	float windSpeed12H;		// 12小时风速

	QDateTime tCreateTime;	
	QDateTime tStartTime;	
	QDateTime tFinishTime;	
	QString strErrReason;	
	QString strErrLevel;	
}ST_StationData;

//  数据分发任务列表
typedef QList<ST_StationData>  LIST_STATIONDATA;

/************************************************************************/
/* 6 站点消息                                                           */
/************************************************************************/
#define STATION_TYPE_SINGLE     1   // 单个站点
#define STATION_TYPE_FILE       2   // 站点文件
#define STATION_TYPE_MULTI      3   // 多站点数据
#define STATION_TYPE_MULTI_DIFF 4   //多站点差值数据
typedef struct _StationMsg
{
	QString strCityType;
	QString strStationNo;
	QString strDataType;
	int     nClientID;    // 客户端编号
	int     nStationType; // 1: 单个站点  2：站点文件
	int     nYear;
	int     nMonth;
	int     nDay;
	int     nHour;
	int     nMinute;
	int     nSecond;
	int     nStartForecast;  // 起始预报时次
	int     nForecastTime;   // 预报时效 或 终止预报时效
	int     nTimeRange;
}ST_StationMsg;

//  数据分发任务列表
typedef QList<ST_StationMsg>  LIST_STATION_MSG;

/************************************************************************/
/* 7 站点格点转化配置                                                   */
/************************************************************************/
typedef struct _STATION_MAG
{
	bool bDisabled;         // 功能是否禁用
	QString strStationFile; // 站点文件
	QString strCityType;    // 转换后站点的城市类型 town
	QString strForecastType;// 转换后的预报类型  SPVT
	int nRadius;            // 站点-格点，影响半径 1
	int nValidTime;         // 缓存数据有效时间
	bool bIsPublish;        // 是否发布
	bool bIsStation2Grid;   // 是否自动进行站点到格点的处理
}ST_STATION_MAG;

/************************************************************************/
/* 8 站点单数据值                                                       */
/************************************************************************/
typedef struct _STATION_VALUE
{
	QString strStationNo; // 站号
	float lon;     // 经度
	float lat;     // 纬度
	float height;  // 高度
	float fValue;  // 值
}ST_STATION_VALUE;
typedef QList<ST_STATION_VALUE *> LIST_STATION_VALUE;

#endif // STATION_DEF_H