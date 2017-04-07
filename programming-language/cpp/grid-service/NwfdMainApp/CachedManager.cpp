#include "CachedManager.h"

/************************************************************************/
/* 构造函数                                                             */
/************************************************************************/
CachedManager::CachedManager()
{
	m_nUnUseCount = 0;
	m_nCachedOpenCount = 0;
}

/************************************************************************/
/* 析构函数                                                             */
/************************************************************************/
CachedManager::~CachedManager()
{
	DestroyAllConnect();
}

/************************************************************************/
/* 单例获取方法                                                         */
/************************************************************************/
CachedManager & CachedManager::getClass()
{
	static CachedManager objManager;
	return objManager;
}

/************************************************************************/
/* 初始化                                                               */
/************************************************************************/
int CachedManager::Init(ST_CACHED stCached, int nConnNum)
{
	// 配置信息
	m_stCached = stCached;
	m_nConnNum = nConnNum;
	if (m_nConnNum < MIN_CACHED_CONNECT_NUM)
	{
		m_nConnNum = MIN_CACHED_CONNECT_NUM;
	}

	// 初始化所有连接
	InitAllConnect();
	if (m_nCachedOpenCount == 0)
	{
		// 没有可用连接
		return CACHED_INIT_FAIL;
	}

	return SUCCESS;
}

/************************************************************************/
/* 初始化缓存连接                                                       */
/************************************************************************/
int CachedManager::InitCachedImpl(CachedImpl * &pCachedImpl)
{
	pCachedImpl = new CachedImpl(m_stCached);
	if (pCachedImpl == NULL)
	{
		return ERR_CREATE_CACHED;
	}

	// 获取缓存连接
	int nErrCode = pCachedImpl->GetProvider();
	if (nErrCode != SUCCESS)
	{
		return nErrCode;
	}

	return SUCCESS;
}

/************************************************************************/
/* 初始化所有连接                                                       */
/************************************************************************/
int CachedManager::InitAllConnect()
{
	// 初始化所有连接
	m_nCachedOpenCount = 0;// 打开成功的数据库个数
	m_nUnUseCount = 0;     // 未被使用的连接个数

	for (int nConnectID = 1; nConnectID <= m_nConnNum; nConnectID++)
	{
		ST_CACHED_CONNECT stCachedConnect;

		// 创建一个连接
		CachedImpl * pCachedImpl = NULL;
		int nCachedState = InitCachedImpl(pCachedImpl);
		if (nCachedState == SUCCESS)
		{
			// 正常连接的个数
			m_nCachedOpenCount++;
		}

		stCachedConnect.nConnectID = nConnectID;      // 连接管理ID
		stCachedConnect.nConnectState = nCachedState; // 连接状态正常
		stCachedConnect.bUseState = false;            // 使用状态未被使用

		// 未被使用的数据库连接数
		m_nUnUseCount++;

		// 插入队列中
		m_mapCached[nConnectID] = pCachedImpl;

		// 插入到数据库连接管理列表中
		m_lstCachedConnect.push_back(stCachedConnect);
	}

	return SUCCESS;
}

/************************************************************************/
/* 关闭所有连接                                                         */
/************************************************************************/
void CachedManager::DestroyAllConnect()
{
	// 删除缓存连接
	MAP_CACHED::iterator iter;
	for (iter = m_mapCached.begin(); iter != m_mapCached.end(); iter++)
	{
		CachedImpl * pCachedImpl = iter->second;
		if (pCachedImpl != NULL)
		{
			delete pCachedImpl;
			pCachedImpl = NULL;
		}
	}
	m_mapCached.clear();

	// 删除缓存管理
	m_lstCachedConnect.clear();

	m_nCachedOpenCount = 0;
	m_nUnUseCount = 0;
}

/************************************************************************/
/* 获取连接状态                                                         */
/************************************************************************/
int CachedManager::GetCachedStatus()
{
	int nConnectID;
	CachedImpl * pCachedImpl = NULL;
	int nCachedStatusCnt = 0;

	for (LIST_CACHED_CONNECT::iterator iter = m_lstCachedConnect.begin(); iter != m_lstCachedConnect.end(); iter++)
	{
		nConnectID = (*iter).nConnectID; // 数据库连接的ID号
		pCachedImpl = m_mapCached[nConnectID]; // 缓存连接

		// 之前创建有问题的
		if ((*iter).nConnectState != SUCCESS)
		{
			if (pCachedImpl)
			{
				delete pCachedImpl;
				pCachedImpl = NULL;
			}

			// 重新创建
			(*iter).nConnectState = InitCachedImpl(pCachedImpl);
			m_mapCached[nConnectID] = pCachedImpl;
		}

		// 连接状态判断
		if (pCachedImpl)
		{
			if (pCachedImpl->ConnectStatus() == SUCCESS)
			{
				nCachedStatusCnt++;
			}
		}
	}

	if (nCachedStatusCnt == 0)
	{
		return CACHED_CONNECT_ABNORMAL;
	}

	return SUCCESS;
}

/************************************************************************/
/* 重新连接缓存                                                         */
/************************************************************************/
int CachedManager::ReconnectCached()
{
	int nConnectID;
	CachedImpl * pCachedImpl = NULL;
	for (LIST_CACHED_CONNECT::iterator iter = m_lstCachedConnect.begin(); iter != m_lstCachedConnect.end(); iter++)
	{
		if ((*iter).bUseState)
		{
			continue;
		}

		nConnectID = (*iter).nConnectID; // 数据库连接的ID号
		pCachedImpl = m_mapCached[nConnectID]; // 缓存连接

		// 之前创建有问题的
		if ((*iter).nConnectState != SUCCESS)
		{
			if (pCachedImpl)
			{
				delete pCachedImpl;
				pCachedImpl = NULL;
			}

			// 重新创建,替换原队列内容
			(*iter).nConnectState = InitCachedImpl(pCachedImpl);
			m_mapCached[nConnectID] = pCachedImpl;
		}
		else
		{
			pCachedImpl->ReConnect();
		}
	}

	return SUCCESS;
}

/************************************************************************/
/* 获取一个未用的连接                                                   */
/************************************************************************/
int CachedManager::GetACachedImpl(CachedImpl * & pCachedImpl)
{
	int nConnectID;
	nConnectID = -1;

	int nCount = 0;

	// 未被利用数=0 ，延时等待释放
	while (m_nUnUseCount == 0)
	{
		// 延时
		delay(50);         // 100毫秒
		if (nCount++ == 10) // 共等待500秒
		{
			break;
		}
	}

	QMutexLocker lock(&m_MutexConnect);

	// 从连接管理列表中找到一个未被使用的连接
	for (LIST_CACHED_CONNECT::iterator iter = m_lstCachedConnect.begin(); iter != m_lstCachedConnect.end(); iter++)
	{
		if (!(*iter).bUseState &&( (*iter).nConnectState == SUCCESS))
		{
			nConnectID = (*iter).nConnectID; // 数据库连接的ID号
			pCachedImpl = m_mapCached[nConnectID]; // 缓存连接

			(*iter).bUseState = true;        // 更改标记为使用
			m_nUnUseCount--;                 // 未被利用数
			break;
		}
	}
	 
	return nConnectID;
}

/************************************************************************/
/* 释放一个使用的连接                                                   */
/************************************************************************/
void CachedManager::RestoreACachedImpl(int nConnectID)
{
	QMutexLocker lock(&m_MutexConnect);

	// 从连接管理列表中找到一个未被使用的连接
	LIST_CACHED_CONNECT::iterator iter = m_lstCachedConnect.begin();
	for (; iter != m_lstCachedConnect.end(); iter++)
	{
		if ((*iter).nConnectID == nConnectID)
		{
			(*iter).bUseState = false; // 更改标记为未被利用
			m_nUnUseCount++;           // 未被利用数
			break;
		}
	}
}


