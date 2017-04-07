/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: TaskDef.h
* 作  者: zhangl		版本：1.0		日  期：2015/04/29
* 描  述：任务调度宏定义，结构体定义，枚举类型定义
* 其  他：
* 功能列表:
*   1.
* 修改日志：
*   No. 日期		作者		修改内容
*
*************************************************************************/
#ifndef TASK_DEF_H
#define TASK_DEF_H

#include "time.h"
#include <QList>
#include <QHash>
#include <QDateTime>

/************************************************************************/
/* 1. 宏定义                                                            */
/************************************************************************/
#define  TASK_EXEC_TIMED       0  // 定时任务
#define  TASK_EXEC_INTERVL     1  // 间隔执行任务

/************************************************************************/
/* 2. 任务调度结构体定义                                                */
/************************************************************************/
typedef struct _TASK_SCHEDUL
{
	int           nSchedulID;  // 任务调度编号
	bool          bFirstExec;  // 是否首次执行（true:不进行时间判断，直接执行）
	time_t        tBeginTime;  // 开始执行时间
	bool          bLoop;       // 是否循环执行 (false:执行一次，之后不再执行）
	unsigned int  nInterval;   // 任务执行间隔
	int           nTaskPID;    // 调度的任务PID号*
} ST_TASK_SCHEDUL, *LPST_TASK_SCHEDUL;

// 任务时间调度列表
typedef QList<LPST_TASK_SCHEDUL>       LIST_TASK_SCHEDUL;
typedef QHash<int, LPST_TASK_SCHEDUL>  HASH_TASK_SCHEDUL;

/************************************************************************/
/* 3. 任务执行状态机                                                    */
/************************************************************************/
typedef enum _TASK_STATE
{
	_TASK_READ = 0,        // 任务准备
	_TASK_RUNNING,         // 任务执行中
	_TASK_STOP,            // 任务停止（执行结束）
	_TASK_INTERRUPTIBLE,   // 任务中断（挂起）
	_TASK_UNINTERRUPTIBLE, // 任务不中断（等待）
	_TASK_DEAD,            // 任务结束/僵死
}EM_TASK_STATE;

#endif //TASK_DEF_H

