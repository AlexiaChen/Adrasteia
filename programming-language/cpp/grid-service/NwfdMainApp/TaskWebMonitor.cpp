#include "TaskWebMonitor.h"

#include "log.h" // log日志
#include "ProductDef.h"
#include "HandleDB.h"
#include "NwfdErrorCode.h"

TaskWebMonitor::TaskWebMonitor()
{
	LOG_(LOGID_DEBUG, LOG_F("Entry TaskWebMonitor()"));
	
	m_emTaskState = _TASK_READ;

	
	LOG_(LOGID_DEBUG, LOG_F("Leave TaskWebMonitor()"));
}

TaskWebMonitor::~TaskWebMonitor()
{
	LOG_(LOGID_DEBUG, LOG_F("Entry ~TaskWebMonitor()"));
	// 停止线程执行
	m_bReady = false;
	wait();
	LOG_(LOGID_DEBUG, LOG_F("Leave ~TaskWebMonitor()"));
}


bool TaskWebMonitor::AddTask(QString strProductKey, TaskBase * pTask)
{
	if (pTask == NULL)
	{
		return false;
	}

	TaskBase * pOrigTask = FindTask(strProductKey);
	if (pOrigTask)
	{
		// 已经存在
		LOG_(LOGID_DEBUG, LOG_F("任务已存在，添加失败（Key=%s）."), LOG_STR(strProductKey));

		return false;
	}

	// 添加到任务列表中
	QMutexLocker locker(&m_mutexTask);
	m_hasProductTask.insert(strProductKey, pTask);

	return true;
}

void TaskWebMonitor::run()
{
	if (m_emTaskState != _TASK_READ)
	{
		// 如果任务没有在准备状态，不予执行此次任务
		LOG_(LOGID_INFO, LOG_F("任务没有完成准备工作，不能执行，任务结束（当前状态为：%d）."), (int)m_emTaskState);
		return;
	}

	m_bReady = true;

	while (m_bReady)
	{
		// 任务状态机状态转换机制
		switch (m_emTaskState)
		{
		case _TASK_READ:    // 任务执行准备中
		{
			m_emTaskState = _TASK_RUNNING;
			break;
		}
		case _TASK_RUNNING: // 任务执行中
		{
			// 执行任务
			ExecMonitorTask();

			// 当前暂不判断执行结果，是否执行成功都将以正常状态结束（以准备下次执行）
			m_emTaskState = _TASK_STOP;
			break;
		}
		case _TASK_STOP: //  任务结束停止
		{
			// 结束本次执行
			m_bReady = false;

			// 任务正常结束，状态改变为_TASK_READ，为下次执行做准备
			m_emTaskState = _TASK_READ;
			break;
		}
		case _TASK_INTERRUPTIBLE:   // 任务中断挂起
			break;

		case _TASK_UNINTERRUPTIBLE: // 任务不中断（等待）
			break;

		case _TASK_DEAD:    // 任务僵死
			m_bReady = false;  // 结束线程执行
			break;

		default:
			break;
		}
	}
}

bool TaskWebMonitor::ExecMonitorTask()
{
	

		ST_DB_TASK_MSG msg;
		if (HandleDB::getClass().GetTaskReprocessMessage(&msg) != SUCCESS)
		{
			return false;
		}
	

		TaskBase* pTask = NULL; 
		pTask = FindTask(msg.productKey);
		if (pTask == NULL)
		{
			return false;
		}

		pTask->start();
	
	
	return true;
}

TaskBase* TaskWebMonitor::FindTask(QString strProductKey)
{
	QMutexLocker locker(&m_mutexTask);

	TaskBase * pTask = NULL;

	if (m_hasProductTask.contains(strProductKey))
	{
		pTask = m_hasProductTask[strProductKey];
	}

	return pTask;
}
