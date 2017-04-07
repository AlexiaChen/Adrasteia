#include "log.h" // log日志
#include "TaskCached.h"
//#include "nwfdmainapp.h"
#include <QDir>
#include <QFileInfoList>
#include <QStringList>

/************************************************************************/
/* 构造函数                                                             */
/************************************************************************/
TaskCached::TaskCached()
{
	m_strTaskName = "缓存连接检测任务";

	//m_pNwfdMainApp = NULL;
	m_pCachedImpl = NULL;
	m_bReady = true;
	m_bFirst = true;
	m_bSetFlg = false;
	m_nReConTryNum = 0;

	m_emTaskState = _TASK_DEAD; 

	LOG_(LOGID_DEBUG, LOG_F("Leave TaskCached()"));
}

/************************************************************************/
/* 析构函数                                                             */
/************************************************************************/
TaskCached::~TaskCached()
{
	LOG_(LOGID_DEBUG, LOG_F("Leave ~TaskCached()"));
	// 停止线程执行
	LOG_(LOGID_DEBUG, LOG_F("[TaskCached]开始停止线程执行."));

	m_bReady = false;
	wait();

	// 删除缓存连接
	if (m_pCachedImpl)
	{
		delete m_pCachedImpl;
		m_pCachedImpl = NULL;
	}

	LOG_(LOGID_DEBUG, LOG_F("~缓存维护任务（TaskCached）析构."));

	LOG_(LOGID_DEBUG, LOG_F("Leave ~TaskCached()"));
}


/************************************************************************/
/* 设置产品列表                                                         */
/************************************************************************/
void TaskCached::Init(ST_CACHED stCached/*, NwfdMainApp *pNwfdMainApp*/)
{
	m_stCached = stCached;
	//m_pNwfdMainApp = pNwfdMainApp;
}

/************************************************************************/
/* 任务处理执行方法                                                     */
/************************************************************************/
void TaskCached::run()
{
	LOG_(LOGID_INFO, LOG_F("[TaskCached] 缓存连接开始检测执行."));
	if (m_pCachedImpl == NULL)
	{
		// 创建连接
		m_pCachedImpl = new CachedImpl(m_stCached);
		if (m_pCachedImpl == NULL)
		{
			LOG_(LOGID_ERROR, LOG_F("[TaskCached] 创建缓存连接失败，结束检测."));

			return;
		}
	}

	m_nReConTryNum = 1;
	m_bFirst = true;
	m_bSetFlg = false;

	while (m_bReady)
	{
		// 缓存连接状态检查
		if (m_pCachedImpl->ConnectStatus() != SUCCESS)
		{
			// 等待10秒后，重新连接
			sleep(10 * m_nReConTryNum);
			m_pCachedImpl->ReConnect();
			m_nReConTryNum++;
			if (m_nReConTryNum++ > 10 )
			{
				m_nReConTryNum = 10;
			}

			continue;
		}

		// 缓存连接正常
		m_nReConTryNum = 1;

		// 首次设置缓存标记
		if (!m_bSetFlg)
		{
			m_bSetFlg = m_pCachedImpl->SetChachedFlg("NLWFD");

			sleep(5);
			continue;
		}

		// 检测标记
		char * szValue = m_pCachedImpl->GetCachedFlg();
		if (szValue != NULL)
		{
			m_bSetFlg = true;
		}
		else
		{
			// 说明缓存服务器发生过重启的情况
			m_bSetFlg = false;
		}
#ifndef DEBUG
		// 释放申请的空间
		if (szValue)
		{
			free(szValue);
			szValue = NULL;
		}
#endif

		// 缓存服务器发生重启，需要对相关的任务进行重置，将数据重新添加到缓存中
		if (!m_bSetFlg)
		{
			// 重置任务
			//m_pNwfdMainApp->Reset();

			// 重新添加
			m_bSetFlg = m_pCachedImpl->SetChachedFlg("NLWFD");
		}
		
		// 延时等待10秒,等待下一次检测
		sleep(10);
	}

	LOG_(LOGID_INFO, LOG_F("[TaskCached] 缓存连接检查任务执行结束"));
}
