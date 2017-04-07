#include "log.h"
#include "TaskDDS.h"
#include "HandleDB.h"
#include "DDSFtp.h"
#include "DDSMq.h"
#include <QDir>

using namespace std;

/************************************************************************/
/* 构造函数                                                             */
/************************************************************************/
TaskDDS::TaskDDS()
{
	LOG_(LOGID_DEBUG, LOG_F("数据分发任务（TaskDDS）构造."));
	m_strTaskName = "数据分发任务";

	m_bReady = true;
	m_emTaskState = _TASK_DEAD;
}

/************************************************************************/
/* 析构函数                                                             */
/************************************************************************/
TaskDDS::~TaskDDS()
{
	// 停止线程执行
	LOG_(LOGID_DEBUG, LOG_F("[TaskPID:%d]开始停止线程执行."), m_nTaskPID);

	m_bReady = false;
	wait();

	// 清除任务列表
	ClearJobMap();

	LOG_(LOGID_DEBUG, LOG_F("~数据分发任务（TaskDDS）析构."));
}

/************************************************************************/
/* 初始化                                                               */
/************************************************************************/
void TaskDDS::Init(HASH_DDS hasDDS)
{
	m_hasDDS = hasDDS;
	
	// todo  临时处理，使用ftp管理器
	if (m_hasDDS.size() <= 0)
	{
		return;
	}

	// 4、任务准备完毕，等待执行
	m_emTaskState = _TASK_READ;

	LOG_(LOGID_DEBUG, LOG_F("[TaskPID:%d]数据分发任务初始化完毕，准备就绪..."), m_nTaskPID);
}


/************************************************************************/
/* 任务处理执行方法                                                     */
/************************************************************************/
void TaskDDS::run()
{
	LOG_(LOGID_INFO, LOG_F("[TaskPID:%d] 数据分发任务开始执行."), m_nTaskPID);

	if (m_emTaskState != _TASK_READ)
	{
		// 如果任务没有在准备状态，不予执行此次任务
		LOG_(LOGID_INFO, LOG_F("[TaskPID:%d][Notice]任务没有完成准备工作，不能执行，任务结束（当前状态为：%d）."), m_nTaskPID, (int)m_emTaskState);
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
			try
			{
				// 执行任务
				ExecTask();
			}
			catch (...)
			{
				LOG_(LOGID_ERROR, LOG_F("[TaskPID:%d]执行扫描处理任务异常"), m_nTaskPID);
			}

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
		{
			// 结束线程执行
			m_bReady = false;
			break;
		}
		default:
			break;
		}
	}

	LOG_(LOGID_INFO, LOG_F("[TaskPID:%d] 数据分发任务执行结束."), m_nTaskPID);
}

/************************************************************************/
/* 执行任务                                                             */
/************************************************************************/
bool TaskDDS::ExecTask()
{
	// 获取数据库未处理的数据列表
	MAP_JOB  mapJob;
	int nErr = HandleDB::getClass().GetDDSJob(mapJob);
	if (nErr != 0)
	{
		// todo 获取等待处理数据失败
		return false;
	}

	if (mapJob.size() == 0)
	{
		return true;
	}

	// 根据类型创建相应的执行具体传输任务的通道线程-配置文件中的每个ID为一个
	MAP_JOB::iterator iter;
	for (iter = mapJob.begin(); iter != mapJob.end(); iter++)
	{
		QString strKey = iter.key();
		LIST_JOB *lstJob = iter.value();

		if (lstJob == NULL || lstJob->size() == 0)
		{
			// todo 没有要处理的数据
			continue;
		}

		// 创建相应类型的数据分发器
		QStringList lstKey = strKey.split("_");
		if (lstKey.size() != 2)
		{
			// todo  创建的map列表中的Key 异常
			continue;
		}

		CDDSBase * pDDS = NULL;
		if (m_mapDDSJob.contains(strKey))
		{
			// 已经创建此类型的数据分发器
			pDDS = m_mapDDSJob.value(strKey);
			if (pDDS)
			{
				// 将要处理的数据添加到执行传输任务的线程中
				pDDS->SetJobList(lstJob);
			}
			else
			{
				// todo 发生异常，暂不处理
				continue;
			}
		}
		else
		{
			QString strDDSType = lstKey.at(0);
			if (strDDSType.toLower() == "ftp")  
			{
				// ftp数据分发器
				pDDS = new CDDSFtp();
			}
			else if (strDDSType.toLower() == "mq")
			{
				// mq数据分发器
				pDDS = new CDDSMq();
			}
			else
			{
				// 其他类型，暂不支持
				continue;
			}

			if (pDDS == NULL)
			{
				// todo 创建失败
				continue;
			}

			// 将要处理的数据添加到执行传输任务的线程中
			pDDS->SetJobList(lstJob);

			m_mapDDSJob.insert(strKey, pDDS);
		}

		// 启动任务执行
		pDDS->start();
		LOG_(LOGID_INFO, LOG_F("[TaskDDS] 启动数据分发处理器%s."), LOG_STR(strKey));
	}

	// 检测各任务是否已经完成，或者执行线程是否运行正常
	// 暂时方案
	bool  bComplete = false;
	while (!bComplete)
	{
		// 延时20秒
		sleep(20);

		// 检测任务是否处理完成
		MAP_DDS_JOB::iterator iterDDSJob;
		for (iterDDSJob = m_mapDDSJob.begin(); iterDDSJob != m_mapDDSJob.end(); iterDDSJob++)
		{
			CDDSBase * pDDS = iterDDSJob.value();
			if (pDDS)
			{
				bComplete = pDDS->IsComplete();
				if (!bComplete)
				{
					break;
				}
			}
		}
	}

	// 任务处理完成，删除执行任务的数据分发器
	// ClearJobMap();

	return true;
}

/************************************************************************/
/* 清空任务列表                                                         */
/************************************************************************/
void TaskDDS::ClearJobMap()
{
	MAP_DDS_JOB::iterator iter;
	for (iter = m_mapDDSJob.begin(); iter != m_mapDDSJob.end(); iter++)
	{
		CDDSBase * pDDS = iter.value();
		if (pDDS)
		{
			delete pDDS;
			pDDS = NULL;
		}
	}

	m_mapDDSJob.clear();
}
