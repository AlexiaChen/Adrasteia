/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: TaskBase.h
* 作  者: zhangl		版本：1.0		日  期：2015/04/29
* 描  述：任务基类(任务接口)定义
* 其  他：
* 功能列表:
*   1.
* 修改日志：
*   No. 日期		作者		修改内容
*
*************************************************************************/
#ifndef TASKBASE_H
#define TASKBASE_H

#include "TaskDef.h"
#include <QThread>
#include <QHash>

// 任务类型定义
#define TASK_DOWNLOAD 1   // 下载任务
#define TASK_UPLOAD   2   // 上传任务
#define TASK_RAW      3   // 扫描解码任务
#define TASK_CLEAN    4   // 系统清理任务

/************************************************************************/
/* 类 名： TaskBase                                                     */
/* 父 类： QThread                                                      */
/* 说 明： 任务基类(接口类)                                             */
/* 描 述：                                                              */
/************************************************************************/
class TaskBase : public QThread
{
	Q_OBJECT

public:
	TaskBase();
	virtual ~TaskBase();

	void  SetTaskPID(int nTaskPID);
	int   GetTaskPID();
	void  SetTaskState(EM_TASK_STATE emTaskState);
	EM_TASK_STATE GetTaskState();
	void  SetTaskName(QString strTaskName);
	QString GetTaskName();
	void SetTaskType(int nType);
	int  GetTaskType();

protected:
	virtual void run() = 0;

protected:
	int           m_nTaskPID;       // 任务PID号
	EM_TASK_STATE m_emTaskState;    // 任务执行状态机
	QString       m_strTaskName;    // 任务名称
	int           m_nType;          // 任务类型
};

/************************************************************************/
/* 任务列表定义                                                         */
/************************************************************************/
typedef QHash<int, TaskBase*>  HASH_TASK;  // <TaskPID,任务基类>
typedef QHash<QString, TaskBase*> HASH_PRODUCT_TASK;  // <ProductKey,任务基类>

#endif // TASKBASE_H
