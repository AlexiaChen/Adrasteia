#ifndef DDS_DEF_H
#define DDS_DEF_H

#include <QString>
#include <QList>
#include <QMap>
#include <QHash>
#include <QDateTime>

/************************************************************************/
/* 宏定义                                                               */
/************************************************************************/
#define  MAX_DDS_FILELIST   10000   // 数据分发列表最大未处理个数为10000个
#define  MAX_EXEC_COUNT     3       // 每个文件最大处理次数
/************************************************************************/
/* 1. 数据分发服务配置                                                  */
/************************************************************************/
// 1.1 数据分发服务器类型
typedef enum _DDS_TYPE
{
	_UNKNOWN = 0,
	_FTP     = 1,
	_MQ      = 2,
	_HTTP	 = 3
}EM_DDS_TYPE;

// 1.2 数据分发目的地定义
typedef struct _DDS
{
	QString  strID;          // 标识号
	//EM_DDS_TYPE emType;    // 数据分发服务器类型
	QString strDDSType;      // 数据分发服务器类型
	QString strRemoteIP;     // 服务器地址
	QString strUserName;     // 用户名
	QString strPassword;     // 密码
	QString strRmotePath;    // 文件存储根目录
}ST_DDS;

// 1.3  数据分发目的地列表
typedef QHash<QString, ST_DDS> HASH_DDS; // dds列表

/************************************************************************/
/* 2. 数据分发服务配置                                                  */
/************************************************************************/
typedef struct _DDS_CFG
{
	bool bDisabled;  // 是否禁用
	int  nInterval;  // 扫描时间间隔
	HASH_DDS hasDDS; // 数据分发列表
}ST_DDS_CFG;

/************************************************************************/
/* 3. 数据分发文件信息                                                  */
/************************************************************************/
typedef struct _DDSFILE
{
	QString strProductKey;  // 产品Key
	QString strFileName;    // 文件名
	QString strFilePath;    // 文件路径
	QString strDDSID;       // 数据分发目的地编号
	int     nFailedNum;     // 失败次数
	bool    bForceFlg;      // 强制发送标记
}ST_DDSFILE;

// 数据分发文件列表
typedef QList<ST_DDSFILE>  LIST_DDSFILE;


/************************************************************************/
/* 4. 分发任务列表                                                      */
/************************************************************************/
// 任务状态
//#define JOB_WAITTING    "WaitProcess"     // 等待处理
//#define JOB_PROCESSING  "processing"  // 处理中
//#define JOB_COMPLETE    "complete"    // 完成处理
//#define JOB_ABNORMAL    "abnormal"    // 异常 

typedef struct _JOB
{
	int  nJobID;
	QString strJobStatus;
	QString strProductKey;
	QString strFileName;
	QString strFilePath;
	int nFileSize;
	QDateTime tCreateTime;
	QDateTime tStartTime;
	QDateTime tFinishTime;
	QString strDdsType;
	QString strRemoteIP;
	QString strRemotePath;
	QString strRemoteUser;
	QString strRemotePassword;
	QString strRemoteFileName;
	int nRemoteFileSize;
	QString strErrReason;
	QString strErrLevel ;
}ST_JOB;
//  数据分发任务列表
typedef QList<ST_JOB>  LIST_JOB;

// 分类列表
typedef QMap<QString, LIST_JOB*> MAP_JOB;  // 按照任务目的地分类


#endif //DDS_DEF_H
