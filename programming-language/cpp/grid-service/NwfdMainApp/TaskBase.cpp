#include "TaskBase.h"

TaskBase::TaskBase()
{
	m_nTaskPID    = 0;
	m_emTaskState = _TASK_READ;
	m_strTaskName = "任务接口";
}

TaskBase::~TaskBase()
{

}

/************************************************************************/
/* 任务基类的基本设置获取方法                                           */
/************************************************************************/
void  TaskBase::SetTaskPID(int nTaskPID)
{
	m_nTaskPID = nTaskPID;
}

int TaskBase::GetTaskPID()
{
	return m_nTaskPID;
}

void  TaskBase::SetTaskState(EM_TASK_STATE emTaskState)
{
	m_emTaskState = emTaskState;
}

EM_TASK_STATE TaskBase::GetTaskState()
{
	return m_emTaskState;
}

void  TaskBase::SetTaskName(QString strTaskName)
{
	m_strTaskName = strTaskName;
}

QString TaskBase::GetTaskName()
{
	return m_strTaskName;
}

void TaskBase::SetTaskType(int nType)
{
	m_nType = nType;
}

int  TaskBase::GetTaskType()
{
	return m_nType;
}