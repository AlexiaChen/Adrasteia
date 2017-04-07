/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: TaskDDS.h
* 作  者: zhangl		版本：1.0		日  期：2015/07/06
* 描  述：数据分发任务
* 其  他：
* 功能列表:
* 修改日志：
*   No. 日期		作者		修改内容
*
*************************************************************************/
#ifndef TASK_DDS_H
#define TASK_DDS_H

/************************************************************************/
/* 头文件引用                                                           */
/************************************************************************/
#include "TaskBase.h"
#include "DDSDef.h"
#include "ProductDef.h"
#include "DDSBase.h"
#include <QFileInfoList>
#include <QDateTime>
#include <QMutex>
#include <QMutexLocker>

/************************************************************************/
/* 执行任务的数据分发器列表                                             */
/************************************************************************/
typedef QMap<QString, CDDSBase*> MAP_DDS_JOB;

/************************************************************************/
/* 类 名： TaskDDS                                                      */
/* 父 类： TaskBase                                                     */
/* 说 明： 数据分发任务                                                 */
/* 描 述：                                                              */
/************************************************************************/
class TaskDDS : public TaskBase
{
	Q_OBJECT

public:
	TaskDDS();
	~TaskDDS();

public:
	void Init(HASH_DDS hasDDS);

protected:
	void run();

private:
	bool ExecTask();
	void ClearJobMap();

private:
	bool            m_bReady;       // run执行标记
	HASH_DDS        m_hasDDS;       // 数据分发列表
	MAP_DDS_JOB     m_mapDDSJob;    // 任务执行列表
};								    

#endif //TASK_DDS_H
