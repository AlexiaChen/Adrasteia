#ifndef TASKWEBMONITOR_H
#define TASKWEBMONITOR_H

#include "TaskBase.h"
#include <QMutexLocker>
#include <QMutex>

typedef QHash<QString, TaskBase*> HASH_PRODUCT_TASK;  // <ProductKey,任务基类>

class TaskWebMonitor : public TaskBase
{
public:
	TaskWebMonitor();
	~TaskWebMonitor();

public:
	bool AddTask(QString strProductKey, TaskBase * pTask);

protected:
	void run();

private:
	bool ExecMonitorTask();
	TaskBase * FindTask(QString strProductKey);
	
private:
	bool            m_bReady;          // run执行标记

	QMutex            m_mutexTask;       // 任务锁
	HASH_PRODUCT_TASK m_hasProductTask;
	
};

#endif // TASKWEBMONITOR_H
