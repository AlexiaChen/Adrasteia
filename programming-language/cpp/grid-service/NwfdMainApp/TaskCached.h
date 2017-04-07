/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: TaskCached.h
* 作  者: zhangl		版本：1.0		日  期：2015/12/16
* 描  述：缓存连接检测类
* 其  他：
* 功能列表:
* 修改日志：
*   No. 日期		作者		修改内容
*
*************************************************************************/
#ifndef TASK_CACHED_H
#define TASK_CACHED_H

/************************************************************************/
/* 头文件引用                                                           */
/************************************************************************/
#include "TaskBase.h"
#include "CachedImpl.h"

//class NwfdMainApp;
/************************************************************************/
/* 类 名： TaskCached                                                   */
/* 父 类： TaskBase                                                     */
/* 说 明： 缓存检测类                                                   */
/* 描 述：                                                              */
/************************************************************************/
class TaskCached : public TaskBase
{
	Q_OBJECT

public:
	TaskCached();
	~TaskCached();

public:
	void Init(ST_CACHED stCached/*, NwfdMainApp *pNwfdMainApp*/);

protected:
	void run();
	
private:
	bool         m_bReady;      // run执行标记
	bool         m_bFirst;      // 是否为首次
	bool         m_bSetFlg;     // 是否设置标记

	ST_CACHED    m_stCached;    // 缓存配置信息
	CachedImpl  *m_pCachedImpl; // 缓存连接指针
	//NwfdMainApp *m_pNwfdMainApp;// 主线程

	int m_nReConTryNum;         // 重连尝试次数
};

#endif // TASK_SYSCHECKUP_H
