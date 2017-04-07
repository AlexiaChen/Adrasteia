#include "log.h"
#include "ProductTaskManager.h"

/************************************************************************/
/* 构造函数                                                             */
/************************************************************************/
ProductTaskManager::ProductTaskManager()
{
}

/************************************************************************/
/* 析构函数                                                             */
/************************************************************************/
ProductTaskManager::~ProductTaskManager()
{
}

/************************************************************************/
/* 单例                                                                 */
/************************************************************************/
ProductTaskManager & ProductTaskManager::getClass()
{
	static ProductTaskManager objProductTaskManager;
	return objProductTaskManager;
}

/************************************************************************/
/* 添加管理产品                                                         */
/************************************************************************/
bool ProductTaskManager::AddProduct(ST_PRODUCT  stProduct)
{
	// 此列表中的key为关联的ProductKey，而非配置的Key,每个ProductKey只有一个Key对应，否则错误
	if (FindProduct(stProduct.strRelatedKey))
	{
		// 已存在
		return false;
	}

	// 添加到任务列表中
	QMutexLocker locker(&m_mutexProduct);
	m_hasProduct.insert(stProduct.strRelatedKey, stProduct);

	return true;
}

/************************************************************************/
/* 查找管理产品                                                         */
/************************************************************************/
bool ProductTaskManager::FindProduct(QString strProductKey)
{
	QMutexLocker locker(&m_mutexProduct);

	if (m_hasProduct.contains(strProductKey))
	{
		return true;
	}

	return false;
}

/************************************************************************/
/* 移除管理产品                                                         */
/************************************************************************/
bool ProductTaskManager::RemoveProduct(QString strProductKey)
{
	if (FindProduct(strProductKey))
	{
		m_mutexProduct.lock();
		m_hasProduct.remove(strProductKey);
		m_mutexProduct.unlock();
	}

	return true;
}

/************************************************************************/
/* 获取某产品                                                           */
/************************************************************************/
bool ProductTaskManager::GetProduct(QString strProductKey, ST_PRODUCT &stProduct)
{
	if (!m_hasProduct.contains(strProductKey))
	{
		return false;
	}

	stProduct = m_hasProduct.value(strProductKey);

	return true;
}

/************************************************************************/
/* 添加任务                                                             */
/************************************************************************/
bool ProductTaskManager::AddTask(QString strType, int nTimeRange, TaskBase * pTask)
{
	if (pTask == NULL)
	{
		return false;
	}

	// KEY的格式
	QString strProductKey = QString("%1_%2").arg(strType).arg(nTimeRange);

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

/************************************************************************/
/* 查找任务                                                             */
/************************************************************************/
TaskBase * ProductTaskManager::FindTask(QString strProductKey)
{
	QMutexLocker locker(&m_mutexTask);

	TaskBase * pTask = NULL;

	if (m_hasProductTask.contains(strProductKey))
	{
		pTask = m_hasProductTask[strProductKey];
	}

	return pTask;
}

/************************************************************************/
/* 删除任务                                                             */
/************************************************************************/
void ProductTaskManager::RemoveTask(QString strProductKey)
{
	TaskBase * pTask = FindTask(strProductKey);

	if (pTask)
	{
		m_mutexTask.lock();
		m_hasProductTask.remove(strProductKey);
		m_mutexTask.unlock();

		delete pTask;
		pTask = NULL;
	}
}

/************************************************************************/
/* 启动任务                                                             */
/************************************************************************/
bool ProductTaskManager::StartTask(QString strType, int nTimeRange)
{
	// KEY的格式
	QString strProductKey = QString("%1_%2").arg(strType).arg(nTimeRange);
	if (!m_hasProductTask.contains(strProductKey))
	{
		return false;
	}

	TaskBase * pTask = m_hasProductTask.value(strProductKey);
	if (pTask == NULL)
	{
		return false;
	}

	// 启动任务
	pTask->start();

	return true;
}