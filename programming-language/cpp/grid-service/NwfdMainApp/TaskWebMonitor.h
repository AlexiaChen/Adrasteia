#ifndef TASKWEBMONITOR_H
#define TASKWEBMONITOR_H

#include "TaskBase.h"
#include <QMutexLocker>
#include <QMutex>

typedef QHash<QString, TaskBase*> HASH_PRODUCT_TASK;  // <ProductKey,�������>

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
	bool            m_bReady;          // runִ�б��

	QMutex            m_mutexTask;       // ������
	HASH_PRODUCT_TASK m_hasProductTask;
	
};

#endif // TASKWEBMONITOR_H
