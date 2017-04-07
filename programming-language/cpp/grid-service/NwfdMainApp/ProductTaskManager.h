/*************************************************************************
* Copyright (C), 2016,
* XXXXXXXXXXX Co.
* 文件名: ProductTaskManager.h
* 作  者: zhangl		版本：1.0		日  期：2016/06/12
* 描  述：产品扫描任务管理
* 其  他：
* 功能列表:
* 修改日志：
*   No. 日期		作者		修改内容
*
*************************************************************************/
#ifndef PRODUCT_TASK_MANAGER_H
#define PRODUCT_TASK_MANAGER_H

#include "ProductDef.h"
#include "TaskBase.h"
#include <QString>
#include <QMutex>
#include <QMutexLocker>

/************************************************************************/
/* 类 名： ProductTaskManager                                               */
/* 父 类：                                                              */
/* 说 明： 网络数据解析处理类                                           */
/* 描 述：                                                              */
/************************************************************************/
class ProductTaskManager
{
private:
	ProductTaskManager();

public:
	~ProductTaskManager();
	static ProductTaskManager & getClass();
	
public:
	/* 产品管理 */
	bool AddProduct(ST_PRODUCT stProduct);
	bool FindProduct(QString strProductKey);
	bool RemoveProduct(QString strProductKey);
	bool GetProduct(QString strProductKey, ST_PRODUCT &stProduct);

	/* 任务管理 */
	bool AddTask(QString strType,int nTimeRange, TaskBase * pTask);
	TaskBase * FindTask(QString strProductKey);
	void RemoveTask(QString strProductKey);

	bool StartTask(QString strType, int nTimeRange); // 启动任务

private:
	HASH_PRODUCT      m_hasProduct;      // 管理的产品列表: 注此处的Key都是要合并的关联Key
	QMutex            m_mutexProduct;    // 锁
	HASH_PRODUCT_TASK m_hasProductTask;  // 任务列表 : 注此处的Key都是要合并的关联Key
	QMutex            m_mutexTask;       // 任务锁
};


#endif // PRODUCT_TASK_MANAGER_H