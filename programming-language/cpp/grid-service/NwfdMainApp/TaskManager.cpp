#include "log.h" // log日志
#include "TaskManager.h"

/************************************************************************/
/* 构造函数                                                             */
/************************************************************************/
TaskManager::TaskManager()
{
	LOG_(LOGID_DEBUG, LOG_F("Entry TaskManager()"));
	
	LOG_(LOGID_DEBUG, LOG_F("任务调度管理器TaskManager构造."));

	m_nTaskSchedulIDSeq = 0;
	m_nTaskPIDSeq = 0;
	LOG_(LOGID_DEBUG, LOG_F("Leave TaskManager()"));
}

/************************************************************************/
/* 析构函数                                                             */
/************************************************************************/
TaskManager::~TaskManager()
{
	
	LOG_(LOGID_DEBUG, LOG_F("Entry ~TaskManager()"));
	
	Close();

	LOG_(LOGID_DEBUG, LOG_F("任务调度管理器TaskManager析构."));
	LOG_(LOGID_DEBUG, LOG_F("Leave ~TaskManager()"));
}

/************************************************************************/
/* 任务管理类单例调用方法                                               */
/************************************************************************/
TaskManager * TaskManager::getInstance()
{
	static TaskManager objTaskManager;
	return &objTaskManager;
}

/************************************************************************/
/* 任务管理线程外部停止方法                                             */
/************************************************************************/
void TaskManager::Stop()
{
	// 线程停止
	m_bReady = false;
}

/************************************************************************/
/* 任务调度编号产生器                                                   */
/************************************************************************/
int TaskManager::GetTaskSchedulID()
{
	QMutexLocker locker(&m_mutexTaskSchedulID);

	m_nTaskSchedulIDSeq++;

	return m_nTaskSchedulIDSeq;
}

/************************************************************************/
/* 任务PID号产生器                                                      */
/************************************************************************/
int TaskManager::GetTaskPID()
{
	QMutexLocker locker(&m_mutexTaskPID);

	m_nTaskPIDSeq++;

	return m_nTaskPIDSeq;
}

/************************************************************************/
/* 任务管理结束时，执行关闭操作                                         */
/************************************************************************/
void TaskManager::Close()
{
	LOG_(LOGID_DEBUG, LOG_F("任务调度管理器开始结束."));

	// 线程停止
	m_bReady = false;

	// 清空任务调度列表
	QMutexLocker locker(&m_mutexTaskSchedul);
	LPST_TASK_SCHEDUL pTaskSchedul = NULL;
	for (HASH_TASK_SCHEDUL::iterator iter = m_hasTaskSchedul.begin(); iter != m_hasTaskSchedul.end(); iter++)
	{
		pTaskSchedul = iter.value();
		if (pTaskSchedul)
		{ 
			delete pTaskSchedul;
			pTaskSchedul = NULL;
		}
	}
	m_hasTaskSchedul.clear();

	// 等待线程安全结束
	wait();

	LOG_(LOGID_DEBUG, LOG_F("任务调度管理器结束."));
}

/************************************************************************/
/* 任务管理线程执行方法                                                 */
/************************************************************************/
void TaskManager::run()
{
	LOG_(LOGID_INFO, LOG_F("任务调度管理器TaskManager开始执行."));

	m_bReady = true;

	while (m_bReady)
	{
		msleep(500);  // 执行间隔 500ms

		try
		{
			ExecTaskSchedul();  // 执行任务调度
		}
		catch (...)
		{
			LOG_(LOGID_ERROR, LOG_F("任务调度执行异常"));
		}
	}

	LOG_(LOGID_INFO, LOG_F("[Notice]任务调度管理器TaskManager停止执行"));
}

/************************************************************************/
/* 执行任务调度方法                                                     */
/************************************************************************/
void TaskManager::ExecTaskSchedul()
{
	// 获取当前时间
	time_t tiRunTime = time(NULL);

	// 任务执行列表
	LIST_TASK_SCHEDUL lstExecTask;

	// 任务调度时间判断
	m_mutexTaskSchedul.lock();

	LPST_TASK_SCHEDUL pTaskSchedul = NULL;
	for (HASH_TASK_SCHEDUL::iterator iter = m_hasTaskSchedul.begin(); iter != m_hasTaskSchedul.end(); iter++)
	{
		pTaskSchedul = iter.value();
		if (pTaskSchedul)
		{
			// 特殊处理，首次执行，不需要等待判断执行时间是否已到
			if (pTaskSchedul->bFirstExec)
			{
				// 将调度任务添加到执行列表中
				lstExecTask.insert(lstExecTask.end(), pTaskSchedul);

				// 取消首次执行标志
				pTaskSchedul->bFirstExec = false;

				LOG_(LOGID_INFO, LOG_F("[TaskPID:%d] 任务首次直接执行,加入执行队列"), pTaskSchedul->nTaskPID);

				// 此处理跳过此次时间判断逻辑
				continue;
			}

			// 当前时间 大于 执行时间，时间到，开始执行
			if (tiRunTime >= pTaskSchedul->tBeginTime)
			{
				// 将调度任务添加到执行列表中
				lstExecTask.insert(lstExecTask.end(), pTaskSchedul);

				// 调整下一次执行时间
				pTaskSchedul->tBeginTime += pTaskSchedul->nInterval;

				// 开始执行时间经计算后异常时，自动调整
				if (pTaskSchedul->tBeginTime < tiRunTime)
				{
					pTaskSchedul->tBeginTime = tiRunTime + pTaskSchedul->nInterval;
				}

				LOG_(LOGID_INFO, LOG_F("[TaskPID:%d] 时间到,任务加入执行队列，下次执行时间:%d"), pTaskSchedul->nTaskPID, pTaskSchedul->tBeginTime);
			}
		}
	}
	m_mutexTaskSchedul.unlock();

	// 执行任务
	int nTaskPID = 0;
	TaskBase * pTask = NULL;
	for (LIST_TASK_SCHEDUL::iterator iterExecTask = lstExecTask.begin(); iterExecTask != lstExecTask.end(); iterExecTask++)
	{
		pTaskSchedul = *iterExecTask;
		if (pTaskSchedul)
		{
			// 执行任务
			pTask = FindTask(pTaskSchedul->nTaskPID);
			if (pTask)
			{
				// 执行任务（异步,触发任务开始执行信号）
				pTask->start();

				LOG_(LOGID_INFO, LOG_F("[TaskPID:%d] 任务启动执行"), pTaskSchedul->nTaskPID);
			}

			// 如果此任务为一次性任务（bLoop==false），从调度列表中移除此调度
			if (!pTaskSchedul->bLoop)
			{
				LOG_(LOGID_INFO, LOG_F("[TaskPID:%d] 该任务为一次性任务，从任务调度中移除"), pTaskSchedul->nTaskPID);
				RemoveTaskSchedul(pTaskSchedul->nSchedulID);
			}
		}
	}
}

/************************************************************************/
/* 添加任务                                                             */
/************************************************************************/
bool TaskManager::AddTask(int nTaskPID, TaskBase * pTask)
{
	if (pTask == NULL)
	{
		return false;
	}

	TaskBase * pOrigTask = FindTask(pTask->GetTaskPID());
	if (pOrigTask)
	{
		// 已经存在
		LOG_(LOGID_DEBUG, LOG_F("任务已存在，添加失败（TaskPID=%d）."), pTask->GetTaskPID());

		return false;
	}

	// 添加到任务列表中
	QMutexLocker locker(&m_mutexTask);
	m_hasTask.insert(pTask->GetTaskPID(), pTask);

	return true;
}

/************************************************************************/
/* 查找任务                                                             */
/************************************************************************/
TaskBase * TaskManager::FindTask(int nTaskPID)
{
	QMutexLocker locker(&m_mutexTask);

	TaskBase * pTask = NULL;

	if (m_hasTask.contains(nTaskPID))
	{
		pTask = m_hasTask[nTaskPID];
	}

	return pTask;
}

/************************************************************************/
/* 删除任务                                                             */
/************************************************************************/
void TaskManager::RemoveTask(int nTaskPID)
{
	TaskBase * pTask = FindTask(nTaskPID);

	if (pTask)
	{
		m_mutexTask.lock();
		m_hasTask.remove(nTaskPID);
		m_mutexTask.unlock();

		delete pTask;
		pTask = NULL;
	}
}

/************************************************************************/
/* 添加任务调度                                                         */
/************************************************************************/
bool TaskManager::AddTaskSchedul(int nTaskSchedulID, int nTaskPID, int nBeginTime, bool bLoop, int nInterval,bool bFirstExec)
{
	// 异常参数判断
	if (bLoop && nInterval <= 0)
	{
		// 如果是无间隔的循环调度，为恶意调度，不予执行
		LOG_(LOGID_DEBUG, LOG_F("（TaskPID=%d）该任务调度为无间隔循环调度，不予执行，添加失败"), nTaskPID);

		return false;
	}

	// 创建任务调度
	LPST_TASK_SCHEDUL pTaskSchedul = new ST_TASK_SCHEDUL;
	if (pTaskSchedul == NULL)
	{
		LOG_(LOGID_DEBUG, LOG_F("（TaskPID=%d）该任务调度创建失败"), nTaskPID);
		return false;
	}

	//  调度任务PID查询
	TaskBase * pTask = FindTask(nTaskPID);
	if (pTask == NULL)
	{
		// 没有管理的任务
		LOG_(LOGID_DEBUG, LOG_F("（TaskPID=%d）任务列表中未查到该任务"), nTaskPID);

		delete pTaskSchedul;
		pTaskSchedul = NULL;

		return false;
	}

	// 赋值
	pTaskSchedul->nSchedulID = nTaskSchedulID;
	pTaskSchedul->nTaskPID   = nTaskPID;
	pTaskSchedul->bFirstExec = bFirstExec;
	pTaskSchedul->tBeginTime = nBeginTime;
	if (pTaskSchedul->tBeginTime == 0)     
	{
		// 如果设置开始时间为0，执行时间为：当前时间+间隔之后的时间
		pTaskSchedul->tBeginTime = time(NULL) + nInterval;
	}

	pTaskSchedul->bLoop     = bLoop;
	pTaskSchedul->nInterval = nInterval;

	if (!AddTaskSchedul(pTaskSchedul))
	{
		delete pTaskSchedul;
		pTaskSchedul = NULL;

		return false;
	}

	return true;
}

/************************************************************************/
/* 任务调度管理                                                         */
/************************************************************************/
bool TaskManager::AddTaskSchedul(LPST_TASK_SCHEDUL pTaskSchedul)
{
	if (pTaskSchedul == NULL)
	{
		return false;
	}

	LPST_TASK_SCHEDUL pOrigTaskSchedul = FindTaskSchedul(pTaskSchedul->nSchedulID);
	if (pOrigTaskSchedul)
	{
		// 已经存在
		LOG_(LOGID_DEBUG, LOG_F("（SchedulID=%d）任务调度已存在"), pTaskSchedul->nSchedulID);

		return false;
	}

	// 添加到任务调度列表中
	QMutexLocker locker(&m_mutexTaskSchedul);
	m_hasTaskSchedul.insert(pTaskSchedul->nSchedulID, pTaskSchedul);

	return true;
}

/************************************************************************/
/* 查找任务调度                                                         */
/************************************************************************/
LPST_TASK_SCHEDUL TaskManager::FindTaskSchedul(int nTaskSchedulID)
{
	QMutexLocker locker(&m_mutexTaskSchedul);

	LPST_TASK_SCHEDUL pTaskSchedul = NULL;

	if (m_hasTaskSchedul.contains(nTaskSchedulID))
	{
		pTaskSchedul = m_hasTaskSchedul[nTaskSchedulID];
	}

	return pTaskSchedul;
}

/************************************************************************/
/* 删除任务调度                                                         */
/************************************************************************/
void TaskManager::RemoveTaskSchedul(int nTaskSchedulID)
{
	LPST_TASK_SCHEDUL pTaskSchedul = FindTaskSchedul(nTaskSchedulID);

	if (pTaskSchedul)
	{
		m_mutexTaskSchedul.lock();
		m_hasTaskSchedul.remove(nTaskSchedulID);
		m_mutexTaskSchedul.unlock();

		delete pTaskSchedul;
		pTaskSchedul = NULL;
	}
}

