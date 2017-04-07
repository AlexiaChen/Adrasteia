/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: NwfdClientDef.h
* 作  者: zhangl  		版本：1.0		日  期：2015/07/13
* 描  述：客户端配置定义
* 其  他：
* 功能列表:
* 修改日志：
*   No.   日期		  作者		       修改内容
* ------------------------------------------------------------------------
*************************************************************************/
#ifndef NWFD_CLIENT_DEF_H
#define NWFD_CLIENT_DEF_H

#include <QString>
#include <QHash>
#include <QList>
#include <QDateTime>

/************************************************************************/
/* 客户端权限配置                                                       */
/************************************************************************/
typedef struct _CLIENT
{
	int nClientID;       // 客户端编号
	QString strName;     // 名称
	QString strClientIP; // 客户端IP
	QString strUserName; // 认证用户名
	QString strPassWord; // 认证密码
	bool bAllowUpload;   // 认证权限，是否允许上传

	QString strGridSavePath; // 格点数据保存目录
	QString strGridSaveFile; // 格点数据保存名称
	QString strStationSavePath; // 站点数据保存目录
	QString strStationSaveFile; // 站点数据保存名称

	int nCachedValidTime;// 缓存中数据保存有效期-格点
}ST_CLIENT_INFO;

// 客户端权限配置列表
typedef QHash <int, ST_CLIENT_INFO>  HASH_CLIENT;

/************************************************************************/
/* 客户端上传数据，数据库配置                                           */
/************************************************************************/
// 任务状态
//#define MERGE_WAITTING    "WaitProcess"     // 等待处理
//#define MERGE_PROCESSING  "processing"      // 处理中
//#define MERGE_COMPLETE    "complete"        // 完成处理
//#define MERGE_ABNORMAL    "abnormal"        // 异常 

typedef struct _MergeData
{
	int     nMergeID;
	QString strMergeStatus;
	QString strMergeType;  // 类型：G2S,RAINPROC,MERGE
	int     nClientID;
	QString strProductKey;
	QString strRelatedProductKey;
	int     nYear;
	int     nMonth;
	int     nDay;
	int     nHour;
	int     nMinute;
	int     nSecond;
	int     nStartForecastTime;
	int     nForecastTime;
	int     nTimeRange;
	float   fLon1;
	float   fLon2;
	float   fLat1;
	float   fLat2;
	QDateTime tCreateTime;
	QDateTime tStartTime;
	QDateTime tFinishTime;
	QString strErrReason;
	QString strErrLevel;
}ST_MergeData;
//  数据分发任务列表
typedef QList<ST_MergeData>  LIST_MERGEDATA;


#endif //NWFD_CLIENT_DEF_H