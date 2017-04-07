/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: TaskCheckup.h
* 作  者: zhangl		版本：1.0		日  期：2016/06/06
* 描  述：与编辑平台交互检测任务
* 其  他：
* 功能列表:
* 修改日志：
*   No. 日期		作者		修改内容
*
*************************************************************************/
#ifndef TASK_CHECKUP_H
#define TASK_CHECKUP_H

/************************************************************************/
/* 头文件引用                                                           */
/************************************************************************/
#include "TaskBase.h"

/************************************************************************/
/* 宏定义                                                               */
/************************************************************************/

// 系统清理维护
typedef struct _CHECKUP
{
	bool  bDisabled;       // 是否进行检查
	int   nInterval;       // 维护扫描时间

	QString strPath;       // 配置文件所在路径
	QString strCfgFileName;// 配置文件名称

}ST_CHECKUP;

typedef struct _SERVER_CFG
{
	QString strSourceType;
	QString strServerIP;
	QString strUserName;
	QString strPassword;
}ST_SERVER_CFG;
typedef QHash<QString, ST_SERVER_CFG> HASH_SERVER_CFG;

/************************************************************************/
/* 类 名： TaskCheckup                                                  */
/* 父 类： TaskBase                                                     */
/* 说 明： 系统清理类                                                   */
/* 描 述：                                                              */
/************************************************************************/
class TaskCheckup : public TaskBase
{
	Q_OBJECT

public:
	TaskCheckup();
	~TaskCheckup();
	void Init(ST_CHECKUP stCheckup);
	int GetConfigParam(QString &strRoot, QString &strCCCC, float &fLon1, float &fLon2, float &fLat1, float &fLat2, HASH_SERVER_CFG &hasServer);
	void AddTask(TaskBase * pTask);

public:

protected:
	void run();

private:
	bool ExecTask(); 

	int ReadConfigFile(QString &strRoot, QString &strCCCC, float &fLon1, float &fLon2, float &fLat1, float &fLat2);

private:
	bool         m_bReady;      // run执行标记
	bool         m_bFirst;      // 是否为首次
	ST_CHECKUP   m_stCheckup;   // 维护配置

	QString  m_strRoot;  // 数据存储的根路径
	QString  m_strCCCC;  // 省数据编码
	float  m_fLon1; // 省数据起始经度
	float  m_fLon2; // 省数据终止经度
	float  m_fLat1; // 省数据起始纬度
	float  m_fLat2; // 省数据终止纬度
	HASH_SERVER_CFG m_hasServerCfg; // FTP服务器配置
	bool   m_bIsServerUpdate; // FTP服务器配置有更新
	// 任务
	HASH_TASK m_hasTask;
};

#endif // TASK_CHECKUP_H
