#include "log.h" // log日志
#include "TaskCheckup.h"
#include "HandleCommon.h"
#include "Common.h"
#include "TaskFtpRecver.h"
#include "TaskFtpSender.h"
#include "TaskProductRaw.h"
#include "TaskSysClean.h"
#include <QDir>
#include <QTextStream>

/************************************************************************/
/* 构造函数                                                             */
/************************************************************************/
TaskCheckup::TaskCheckup()
{
	LOG_(LOGID_DEBUG, LOG_F("Entry TaskSysClean()"));
	m_strTaskName = "交互配置检查任务";

	m_bReady = true;
	m_bFirst = true;

	m_strRoot ="";  // 数据存储的根路径
	m_strCCCC ="";  // 省数据编码
	m_fLon1 = 70;   // 省数据起始经度
	m_fLon2 = 140;  // 省数据终止经度
	m_fLat1 = 0;    // 省数据起始纬度
	m_fLat2 = 60;   // 省数据终止纬度

	m_emTaskState = _TASK_DEAD; 

	LOG_(LOGID_DEBUG, LOG_F("Leave TaskSysClean()"));
}

/************************************************************************/
/* 析构函数                                                             */
/************************************************************************/
TaskCheckup::~TaskCheckup()
{
	// 停止线程执行
	LOG_(LOGID_DEBUG, LOG_F("[TaskSysClean]开始停止线程执行."));

	m_bReady = false;
	wait();

	LOG_(LOGID_DEBUG, LOG_F("~交互配置检查任务（TaskCheckup）析构."));
}

/************************************************************************/
/* 初始化                                                               */
/************************************************************************/
void TaskCheckup::Init(ST_CHECKUP stCheckup)
{
	m_stCheckup = stCheckup;

	m_emTaskState = _TASK_READ;
}

/************************************************************************/
/* 任务处理执行方法                                                     */
/************************************************************************/
void TaskCheckup::run()
{
	LOG_(LOGID_INFO, LOG_F("[交互配置检查任务开始执行."));

	if (m_emTaskState != _TASK_READ)
	{
		// 如果任务没有在准备状态，不予执行此次任务
		LOG_(LOGID_INFO, LOG_F("【Error】 任务没有完成准备工作，不能执行，任务结束（当前状态为：%d）."), (int)m_emTaskState);
		return;
	}

	m_bReady = true;

	while (m_bReady)
	{
		// 任务状态机
		switch (m_emTaskState)
		{
		case _TASK_READ:    // 任务执行准备中
		{
			// 状态变为开始执行
			m_emTaskState = _TASK_RUNNING;
			break;
		}
		case _TASK_RUNNING: // 任务执行中
		{
			try
			{
				// 执行缓存维护任务
				bool bRet = ExecTask();
				if (bRet)
				{
					LOG_(LOGID_INFO, LOG_F("[配置检查任务] 执行成功"));
				}
				else
				{
					LOG_(LOGID_INFO, LOG_F("[配置检查任务] 执行失败"));
				}
			}
			catch (...)
			{
				LOG_(LOGID_ERROR, LOG_F("[配置检查任务]执行异常"));
			}

			// 当前暂不判断执行结果，是否执行成功都将以正常状态结束（以准备下次执行）
			m_emTaskState = _TASK_STOP;
			break;
		}
		case _TASK_STOP: //  任务结束停止
		{
			// 结束本次执行
			m_bReady = false;  

			// 任务正常结束，状态改变为_TASK_READ，为下次执行做准备
			m_emTaskState = _TASK_READ;   

			break;
		}
		case _TASK_INTERRUPTIBLE:   // 任务中断挂起
			break;
		case _TASK_UNINTERRUPTIBLE: // 任务不中断（等待）
			break;

		case _TASK_DEAD:    // 任务僵死
		{
			// 结束线程执行
			m_bReady = false;  
			break;
		}
		default:
			break;
		}
	}

	LOG_(LOGID_INFO, LOG_F("交互配置检查任务执行结束"));
}

/************************************************************************/
/* 执行维护任务                                                         */
/************************************************************************/
bool TaskCheckup::ExecTask()
{
	QString strRoot;
	QString strCCCC;
	float fLon1, fLon2, fLat1, fLat2;

	int nErr = ReadConfigFile(strRoot, strCCCC, fLon1, fLon2, fLat1, fLat2); 
	if (nErr < 0)
	{
		// 读取文件失败
		return false;
	}

	// 发现有与之前读取的不一致的，对相关任务进行设置
	if (strRoot != m_strRoot || strCCCC != m_strCCCC || !IS_EQUAL(fLon1,m_fLon1)
		|| !IS_EQUAL(fLon2, m_fLon2) || !IS_EQUAL(fLat1, m_fLat1) || !IS_EQUAL(fLat2, m_fLat2) ||
		m_bIsServerUpdate)
	{
		// 更新最新的值
		if (nErr >= 0)
		{
			m_strRoot = strRoot;
			m_strCCCC = strCCCC;
		}
		if (nErr == 0)
		{
			m_fLon1 = fLon1;
			m_fLon2 = fLon2;
			m_fLat1 = fLat1;
			m_fLat2 = fLat2;
		}

		LOG_(LOGID_INFO, LOG_F("[编辑平台修改配置][最新设置] 本地存储根路径：%s ; 省中心编码：%s ; 裁剪经纬度范围：（%0.2f,%0.2f,%0.2f,%0.2f）."),
			LOG_STR(m_strRoot), LOG_STR(m_strCCCC), m_fLon1, m_fLon2, m_fLat1, m_fLat2);

		QString strSouceType;
		ST_SERVER_CFG stServer;
		// 重置其他任务的值
		HASH_TASK::iterator iter;
		for (iter = m_hasTask.begin(); iter != m_hasTask.end(); iter++)
		{
			TaskBase* pTask = iter.value();
			if (pTask == NULL)
			{
				continue;
			}
			switch (pTask->GetTaskType())
			{
			case TASK_DOWNLOAD:
				((TaskFtpRecver*)pTask)->setRootPath(m_strRoot);
				((TaskFtpRecver*)pTask)->setCCCC(strCCCC);
				strSouceType = ((TaskFtpRecver*)pTask)->getSourceType();
				if (!strSouceType.isEmpty() && m_hasServerCfg.contains(strSouceType))
				{
					stServer = m_hasServerCfg.value(strSouceType);
					((TaskFtpRecver*)pTask)->setServer(stServer.strServerIP, stServer.strUserName, stServer.strPassword);
				}
				break;

			case TASK_UPLOAD:
				((TaskFtpSender*)pTask)->setRootPath(m_strRoot);
				((TaskFtpSender*)pTask)->setCCCC(strCCCC);
				strSouceType = ((TaskFtpSender*)pTask)->getSourceType();
				if (!strSouceType.isEmpty() && m_hasServerCfg.contains(strSouceType))
				{
					stServer = m_hasServerCfg.value(strSouceType);
					((TaskFtpSender*)pTask)->setServer(stServer.strServerIP, stServer.strUserName, stServer.strPassword);
				}
				break;

			case TASK_RAW:
				((TaskProductRaw*)pTask)->setRootPath(m_strRoot);
				((TaskProductRaw*)pTask)->setCCCC(strCCCC);
				if (nErr == 0)
				{
					((TaskProductRaw*)pTask)->setGridRange(m_fLon1, m_fLon2, m_fLat1, m_fLat2);
				}
				break;
			case TASK_CLEAN:
				((TaskSysClean*)pTask)->setRootPath(m_strRoot);
				((TaskSysClean*)pTask)->setCCCC(strCCCC);
				break;
			}
		}
	}

	return true;
}

/************************************************************************/
/* 读取配置文件，获取配置文件内容                                       */
/************************************************************************/
int TaskCheckup::GetConfigParam(QString &strRoot, QString &strCCCC, float &fLon1, float &fLon2, float &fLat1, float &fLat2, HASH_SERVER_CFG &hasServer)
{
	int nErr = ReadConfigFile(strRoot, strCCCC, fLon1, fLon2, fLat1, fLat2);

	// 更新最新的值
	if (nErr >= 0 )
	{
		m_strRoot = strRoot;
		m_strCCCC = strCCCC;
	}
	if (nErr == 0)
	{
		m_fLon1 = fLon1;
		m_fLon2 = fLon2;
		m_fLat1 = fLat1;
		m_fLat2 = fLat2;
	}

	hasServer = m_hasServerCfg;

	return nErr;
}

/************************************************************************/
/* 读取配置文件，获取配置文件内容                                       */
/************************************************************************/
int TaskCheckup::ReadConfigFile(QString &strRoot, QString &strCCCC, float &fLon1, float &fLon2, float &fLat1, float &fLat2)
{
	int ret = 0;

	QString strFile = QString("%1/%2").arg(m_stCheckup.strPath).arg(m_stCheckup.strCfgFileName);

	// 打开配置文件
	QFile file(strFile);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		// 打开文件失败
		// todo 写入日志
		return -1;
	}

	// 按行读取
	QTextStream in(&file);

	// 第一行：数据存储根路径
	strRoot = in.readLine();

	// 第二行：省编码
	strCCCC = in.readLine();

	// 第三行：经纬度范围
	QString strLonLat = in.readLine(); 
	QStringList lstLonLat = strLonLat.split(" ", QString::SkipEmptyParts);
	if (lstLonLat.size() != 4)
	{
		// 数据错误
		ret = 1;
	}
	else
	{
		fLon1 = lstLonLat.at(0).toFloat();
		fLon2= lstLonLat.at(1).toFloat();
		fLat1 = lstLonLat.at(2).toFloat();
		fLat2 = lstLonLat.at(3).toFloat();
	}

	// 后面行，是FTPServer配置
	m_bIsServerUpdate = false;
	QString strServer;
	do
	{
		strServer = in.readLine();
		if (strServer.isEmpty())
		{
			break;
		}

		QStringList lstServer = strServer.split(" ", QString::SkipEmptyParts);
		if (lstServer.size() != 4)
		{
			// 数据错误
			continue;
		}
		ST_SERVER_CFG server;
		server.strSourceType = lstServer.at(0);
		server.strServerIP = lstServer.at(1);
		server.strUserName = lstServer.at(2);
		server.strPassword = lstServer.at(3);

		if (!m_hasServerCfg.contains(server.strSourceType))
		{
			m_hasServerCfg.insert(server.strSourceType,server);
			m_bIsServerUpdate = true;
		}
		else
		{
			ST_SERVER_CFG serverOld = m_hasServerCfg.value(server.strSourceType);
			if (server.strServerIP != serverOld.strServerIP || server.strUserName != serverOld.strUserName ||
				server.strPassword != serverOld.strPassword)
			{
				m_hasServerCfg[server.strSourceType]=server;
				m_bIsServerUpdate = true;
			}
		}

	} while (!strServer.isEmpty());

	// 关闭文件
	file.close();

	return ret;
}

/************************************************************************/
/* 添加任务                                                             */
/************************************************************************/
void TaskCheckup::AddTask(TaskBase * pTask)
{
	m_hasTask.insert(pTask->GetTaskPID(), pTask);
}

