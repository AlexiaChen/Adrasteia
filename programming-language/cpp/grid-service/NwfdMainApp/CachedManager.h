/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: CachedManager.h
* 作  者: zhangl		版本：1.0		日  期：2015/12/25
* 描  述：缓存连接管理
* 其  他：
* 功能列表:
*   1.
* 修改日志：
*   No. 日期		作者		修改内容
*************************************************************************/
#ifndef CACHED_MANAGER_H
#define CACHED_MANAGER_H

#include "CachedImpl.h"
#include <list>
#include <map>
#include <QMutex>

using namespace std;
/************************************************************************/
/* 类型定义                                                             */
/************************************************************************/
#define MIN_CACHED_CONNECT_NUM    2         // 最小缓存连接个数

// 缓存连接结构体
typedef struct _CACHED_CONNECT
{
	int     nConnectID;      // 连接ID
	int     nConnectState;   // 连接状态 0:正常；非0:异常
	bool    bUseState;       // 利用状态 true:正在使用；false:未被使用
} ST_CACHED_CONNECT;

// 连接管理列表
typedef list<ST_CACHED_CONNECT> LIST_CACHED_CONNECT;

// 缓存连接队列
typedef map<int, CachedImpl *> MAP_CACHED;  

/************************************************************************/
/* 类 名： CachedManager                                                */
/* 父 类： -                                                            */
/* 说 明： 缓存连接管理类                                               */
/* 描 述：                                                              */
/************************************************************************/
class CachedManager
{
public:
	CachedManager();

public:
	~CachedManager();
	static CachedManager & getClass();
    
	int Init(ST_CACHED stCached, int nConnNum);// 初始化
	void DestroyAllConnect();                  // 关闭所有连接

	int GetCachedStatus(); // 获取连接状态
	int ReconnectCached(); // 重新连接缓存

	int GetACachedImpl(CachedImpl * & cached);// 获取一个未用的连接
	void RestoreACachedImpl(int nConnectID);  // 释放一个使用的连接
private:
	int  InitAllConnect();    // 初始化所有连接

	int InitCachedImpl(CachedImpl * &pCachedImpl);

private:
	ST_CACHED   m_stCached;    // 缓存配置
	int         m_nConnNum;    // 缓存连接个数

	int         m_nUnUseCount;      // 未被使用的连接数
	int         m_nCachedOpenCount; // 打开成功的数据库个数
	MAP_CACHED  m_mapCached;        // 缓存连接列表
	LIST_CACHED_CONNECT m_lstCachedConnect; // 缓存连接管理列表

	QMutex      m_MutexConnect;  // 互斥锁
};

/************************************************************************/
/* 类 名： CachedGuard                                                  */
/* 父 类： -                                                            */
/* 说 明： 缓存连接守卫类--使用懒汉式单例                               */
/* 描 述： 利用构造和析构函数保证连接取出和归还必须成对，防止资源泄露   */
/************************************************************************/
class CachedGuard
{
public:
	CachedGuard(CachedImpl * & cached)
	{
		m_nConnectID = CachedManager::getClass().GetACachedImpl(cached);
	}

	// 传出缓存连接ID
	CachedGuard(CachedImpl * & cached, int &nConnectID)
	{
		nConnectID = CachedManager::getClass().GetACachedImpl(cached);
		m_nConnectID = nConnectID;
	}

	virtual ~CachedGuard()
	{
		// 通过连接管理编号进行管理
		CachedManager::getClass().RestoreACachedImpl(m_nConnectID);
	}

private:
	int m_nConnectID;  // 连接管理ID
};

#endif // CACHED_MANAGER_H

