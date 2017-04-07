#include "TaskWebMonitor.h"

#include "log.h" // log��־
#include "ProductDef.h"
#include "HandleDB.h"
#include "NwfdErrorCode.h"

TaskWebMonitor::TaskWebMonitor()
{
	LOG_(LOGID_DEBUG, LOG_F("Entry TaskWebMonitor()"));
	
	m_emTaskState = _TASK_READ;

	
	LOG_(LOGID_DEBUG, LOG_F("Leave TaskWebMonitor()"));
}

TaskWebMonitor::~TaskWebMonitor()
{
	LOG_(LOGID_DEBUG, LOG_F("Entry ~TaskWebMonitor()"));
	// ֹͣ�߳�ִ��
	m_bReady = false;
	wait();
	LOG_(LOGID_DEBUG, LOG_F("Leave ~TaskWebMonitor()"));
}


bool TaskWebMonitor::AddTask(QString strProductKey, TaskBase * pTask)
{
	if (pTask == NULL)
	{
		return false;
	}

	TaskBase * pOrigTask = FindTask(strProductKey);
	if (pOrigTask)
	{
		// �Ѿ�����
		LOG_(LOGID_DEBUG, LOG_F("�����Ѵ��ڣ����ʧ�ܣ�Key=%s��."), LOG_STR(strProductKey));

		return false;
	}

	// ��ӵ������б���
	QMutexLocker locker(&m_mutexTask);
	m_hasProductTask.insert(strProductKey, pTask);

	return true;
}

void TaskWebMonitor::run()
{
	if (m_emTaskState != _TASK_READ)
	{
		// �������û����׼��״̬������ִ�д˴�����
		LOG_(LOGID_INFO, LOG_F("����û�����׼������������ִ�У������������ǰ״̬Ϊ��%d��."), (int)m_emTaskState);
		return;
	}

	m_bReady = true;

	while (m_bReady)
	{
		// ����״̬��״̬ת������
		switch (m_emTaskState)
		{
		case _TASK_READ:    // ����ִ��׼����
		{
			m_emTaskState = _TASK_RUNNING;
			break;
		}
		case _TASK_RUNNING: // ����ִ����
		{
			// ִ������
			ExecMonitorTask();

			// ��ǰ�ݲ��ж�ִ�н�����Ƿ�ִ�гɹ�����������״̬��������׼���´�ִ�У�
			m_emTaskState = _TASK_STOP;
			break;
		}
		case _TASK_STOP: //  �������ֹͣ
		{
			// ��������ִ��
			m_bReady = false;

			// ��������������״̬�ı�Ϊ_TASK_READ��Ϊ�´�ִ����׼��
			m_emTaskState = _TASK_READ;
			break;
		}
		case _TASK_INTERRUPTIBLE:   // �����жϹ���
			break;

		case _TASK_UNINTERRUPTIBLE: // �����жϣ��ȴ���
			break;

		case _TASK_DEAD:    // ������
			m_bReady = false;  // �����߳�ִ��
			break;

		default:
			break;
		}
	}
}

bool TaskWebMonitor::ExecMonitorTask()
{
	

		ST_DB_TASK_MSG msg;
		if (HandleDB::getClass().GetTaskReprocessMessage(&msg) != SUCCESS)
		{
			return false;
		}
	

		TaskBase* pTask = NULL; 
		pTask = FindTask(msg.productKey);
		if (pTask == NULL)
		{
			return false;
		}

		pTask->start();
	
	
	return true;
}

TaskBase* TaskWebMonitor::FindTask(QString strProductKey)
{
	QMutexLocker locker(&m_mutexTask);

	TaskBase * pTask = NULL;

	if (m_hasProductTask.contains(strProductKey))
	{
		pTask = m_hasProductTask[strProductKey];
	}

	return pTask;
}
