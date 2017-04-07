/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: DDSManager.h
* 作  者: zhangl  		版本：1.0		日  期：2015-07-14
* 描  述：数据分发管理线程
* 其  他：
* 功能列表:
*   1. 创建数据库连接池
*   2. 将各数据处理任务添加的数据分发信息保存到数据库中
* 修改日志：
*   No.   日期		  作者		       修改内容
* ------------------------------------------------------------------------
*************************************************************************/
#ifndef DDS_MANAGER_H
#define DDS_MANAGER_H

#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include "DDSDef.h"
#include "DBConnPool.h"
#include "TaskDDS.h"

/************************************************************************/
/* 类 名： CDDSManager                                                  */
/* 父 类： QThread                                                      */
/* 说 明： 数据分发管理器                                               */
/* 描 述：                                                              */
/************************************************************************/
class CDDSManager : public QThread
{
private:
	CDDSManager();

public:
	~CDDSManager();
	static CDDSManager & getClass();

public:
	void Init(HASH_DDS  hasDDS);
	void SetDDSTask(TaskDDS* pTaskDDS);
	void Stop();/* 外部停止线程执行的方法 */
	bool IsDisabled();
	int DataDistribution(QString strKey, QStringList lstDDS, QString strFileName, QString strPath, bool bForceFlg = false);
	int AddDDSFile(ST_DDSFILE stDDSFile);

protected:
	void run();

private:
	int ExecTask();
	int Proccess(ST_DDSFILE stDDSFile);

private:
	bool           m_bReady;
	bool           m_bDisabled;   // 是否禁用
	HASH_DDS       m_hasDDS;      // 数据分发目的地列表
	CDBConnPool*   m_pDBConnPool; // 数据库连接指针
	LIST_DDSFILE   m_lstDDSFile;  // 数据分发文件列表
	QMutex         m_mutexList;   // 文件列表锁

	TaskDDS*       m_pTaskDDS;    // 数据分发执行任务
};

#endif //DDS_MANAGER_H
