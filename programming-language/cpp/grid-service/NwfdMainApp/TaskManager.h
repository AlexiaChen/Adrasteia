/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: TaskManager.h
* 作  者: zhangl		版本：1.0		日  期：2015/04/28
* 描  述：任务管理类定义
* 其  他：
* 功能列表:
*   1. 进行任务执行时间判断
*   2. 进行任务调度
*  
* 修改日志：
*   No. 日期		作者		修改内容
*
*************************************************************************/
#ifndef TASKMANAGER_H
#define TASKMANAGER_H

/************************************************************************/
/* 头文件引用                                                           */
/************************************************************************/
#include "TaskBase.h"
#include <QThread>
#include <QMutex>
#include <QMutexLocker>

/************************************************************************/
/* 类 名： TaskManager                                                  */
/* 父 类： QThread                                                      */
/* 说 明： 任务调度管理类                                               */
/* 描 述：                                                              */
/************************************************************************/ 
class TaskManager : public QThread
{
	Q_OBJECT

private:
	TaskManager();

public:
	~TaskManager();
	static TaskManager * getInstance();

public:
	/* 任务，调度管理编号产生器 */
	int GetTaskSchedulID();
	int GetTaskPID();

	/* 添加任务和调度 */
	bool AddTask(int nTaskPID, TaskBase * pTask);
	bool AddTaskSchedul(int nTaskSchedulID, int nTaskPID, int nBeginTime, bool bLoop, int nInterval, bool bFirstExec = false);

	/* 外部停止线程执行的方法 */
	void Stop();

protected:
	void run();

private:
	/* 任务管理 */
	TaskBase * FindTask(int nTaskPID);
	void RemoveTask(int nTaskPID);

	/* 任务调度管理 */
	bool AddTaskSchedul(LPST_TASK_SCHEDUL pTaskSchedul);
	LPST_TASK_SCHEDUL FindTaskSchedul(int nTaskSchedulID);
	void RemoveTaskSchedul(int nTaskSchedulID);

	/* 任务调度执行 */
	void ExecTaskSchedul();

	/* 结束关闭操作  */
	void Close();

private:
	bool     m_bReady;

	/* 任务&任务调度 */
	HASH_TASK_SCHEDUL m_hasTaskSchedul;   // 任务调度列表
	HASH_TASK         m_hasTask;          // 任务列表
	QMutex            m_mutexTaskSchedul; // 任务调度锁
	QMutex            m_mutexTask;        // 任务锁

	/* ID管理 */
	int      m_nTaskSchedulIDSeq;  // 任务调度ID管理
	int      m_nTaskPIDSeq;        // 任务PID管理
	QMutex   m_mutexTaskSchedulID; // 任务调度ID锁
	QMutex   m_mutexTaskPID ;      // 任务PID锁
};

#endif // TASKMANAGER_H
