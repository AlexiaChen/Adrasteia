/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: TaskStationMsg.h
* 作  者: zhangl		版本：1.0		日  期：2015/09/15
* 描  述：站点数据处理
* 其  他：
* 功能列表:
*   1.
* 修改日志：
*   No. 日期		作者		修改内容
*
*************************************************************************/
#ifndef TASK_STATION_MSG_H
#define TASK_STATION_MSG_H

#include "TaskBase.h"
#include "StationDef.h"
#include "CachedImpl.h"

#include <QFileInfoList>
#include <QMutex>
#include <QMutexLocker>
/************************************************************************/
/* 类 名： TaskProductMsg                                               */
/* 父 类： TaskBase                                                     */
/* 说 明： 产品合并处理类                                               */
/* 描 述：                                                              */
/************************************************************************/
class TaskStationMsg : public TaskBase
{
	Q_OBJECT

public:
	TaskStationMsg();
	~TaskStationMsg();

public:
	void Init(ST_CACHED stCached);
	void AddMsg(ST_StationMsg stStationMsg);

protected:
	void run();

private:
	bool InitCached();  // 初始化缓存
	bool ExecTask();    // 执行任务
	bool Process();     // 具体处理

	int ProcessStationFile(ST_StationMsg stStationMsg); // 处理站点文件消息
	int ProcessStation(ST_StationMsg stStationMsg); // 处理单站点消息
	int ProcessMultiStation(ST_StationMsg stStationMsg); // 处理多站点消息
	int ProcessMultiDiffStation(ST_StationMsg stStationMsg); // 处理多站点差值消息-进行站点到格点插值处理请求

private:
	bool           m_bReady;       // run执行标记
	ST_CACHED      m_stCached;     // 缓存配置信息
	CachedImpl *   m_pCachedImpl;  // 缓存业务处理

	LIST_STATION_MSG m_lstStationMsg; // 站点消息列表
	QMutex         m_mutexMsg;     // 互斥锁
};

#endif // TASK_STATION_RAW_H
