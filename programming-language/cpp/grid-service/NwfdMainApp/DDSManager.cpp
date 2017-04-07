#include "log.h"
#include "DDSManager.h"
#include "HandleDB.h"
#include <QFileInfo>
#include <QDateTime>

/************************************************************************/
/* 构造函数                                                             */
/************************************************************************/
CDDSManager::CDDSManager()
{
	LOG_(LOGID_DEBUG, LOG_F("Entry CDDSManager()"));
	m_pDBConnPool = NULL;
	m_pTaskDDS = NULL;
	m_bDisabled = true;  // 禁用

	LOG_(LOGID_DEBUG, LOG_F("Leave CDDSManager()"));
}

/************************************************************************/
/* 析构函数                                                             */
/************************************************************************/
CDDSManager::~CDDSManager()
{
	LOG_(LOGID_DEBUG, LOG_F("Entry ~CDDSManager()"));
	// 线程停止
	m_bReady = false;

	wait();
	LOG_(LOGID_DEBUG, LOG_F("Leave ~CDDSManager()"));
}

/************************************************************************/
/* 单例                                                                 */
/************************************************************************/
CDDSManager & CDDSManager::getClass()
{
	static CDDSManager objDDSManager;
	return objDDSManager;
}

/************************************************************************/
/* 初始化                                                               */
/************************************************************************/
void CDDSManager::Init(HASH_DDS  hasDDS)
{
	// 1. 保存初始化参数
	m_hasDDS = hasDDS;

	// 2. 功能启用
	m_bDisabled = false;

	m_pDBConnPool = CDBConnPool::GetInstance();

	// 3. 启动该线程
	this->start();
}

/************************************************************************/
/* 设置数据分发任务                                                     */
/************************************************************************/
void CDDSManager::SetDDSTask(TaskDDS* pTaskDDS)
{
	m_pTaskDDS = pTaskDDS;
}

/************************************************************************/
/* 数据分发管理线程外部停止方法                                         */
/************************************************************************/
void CDDSManager::Stop()
{
	// 线程停止
	m_bReady = false;
}

/************************************************************************/
/* 数据分发管理线程外部停止方法                                         */
/************************************************************************/
bool CDDSManager::IsDisabled()
{
	return m_bDisabled;
}

/************************************************************************/
/* 数据文件分发                                                         */
/************************************************************************/
int CDDSManager::DataDistribution(QString strKey, QStringList lstDDS, QString strFileName, QString strPath, bool bForceFlg)
{
	if (IsDisabled())
	{
		// 数据分发管理功能禁用
		return 0;
	}

	// 添加到队列中
	ST_DDSFILE stDDSFile;
	stDDSFile.strProductKey = strKey;
	stDDSFile.strFileName = strFileName;
	stDDSFile.strFilePath = strPath;
	stDDSFile.nFailedNum = 0;
	stDDSFile.bForceFlg = bForceFlg;

	int nErr;
	QStringList::iterator iterDDSID;
	for (iterDDSID = lstDDS.begin(); iterDDSID != lstDDS.end(); iterDDSID++)
	{
		stDDSFile.strDDSID = *iterDDSID;

		// 添加到分发队列中
		nErr = AddDDSFile(stDDSFile);
		if (nErr == 0)
		{
			// todo 添加成功
		}
		else
		{
			// todo 添加失败
		}
	}

	return 0;
}

/************************************************************************/
/* 添加文件到处理列表中                                                 */
/************************************************************************/
int CDDSManager::AddDDSFile(ST_DDSFILE stDDSFile)
{
	// 功能禁用
	if (m_bDisabled)
	{
		return 1;
	}

	QMutexLocker locker(&m_mutexList);

	if (m_lstDDSFile.size()  >= MAX_DDS_FILELIST)
	{
		// 未处理个数超过最大限制 todo
		return -1;
	}

	// 添加到处理队列中
	m_lstDDSFile.push_back(stDDSFile);

	return 0;
}

/************************************************************************/
/* 线程执行方法                                                         */
/************************************************************************/
void CDDSManager::run()
{
	//  开始启动数据分发管理线程
	m_bReady = true;
	while (m_bReady)
	{
		// 1. 数据库连接初始化成功，开始执行任务
		try
		{
			// 执行任务
			ExecTask();  

			// 每次处理完等待30秒
			sleep(30);
		}
		catch (...)
		{
			// 任务执行异常
		}
	}
}

/************************************************************************/
/* 线程执行方法                                                         */
/************************************************************************/
int CDDSManager::ExecTask()
{
	QMutexLocker locker(&m_mutexList);

	bool bNewJobFlg = false;

	// 开始处理队列中的文件信息
	int nSize = m_lstDDSFile.size();
	while(nSize > 0)
	{
		// 取出第一个进行处理
		ST_DDSFILE stDDSFile = m_lstDDSFile.first();
		if (stDDSFile.nFailedNum >= MAX_EXEC_COUNT)
		{
			// 超过最大处理次数,记录日志，先不处理此文件,
			// todo
			LOG_(LOGID_DEBUG, LOG_F("[DDSManager][Key:%s][File:%s]添加到数据库超过失败次数，不再处理."), LOG_STR(stDDSFile.strProductKey), LOG_STR(stDDSFile.strFileName));
		}
		else
		{
			// 处理
			int nErr = Proccess(stDDSFile);
			if (nErr == 0)
			{
				// 处理成功
				LOG_(LOGID_DEBUG, LOG_F("[DDSManager][Key:%s][File:%s]添加到数据库成功."), LOG_STR(stDDSFile.strProductKey), LOG_STR(stDDSFile.strFileName));
				// 有新任务
				bNewJobFlg = true;
			}
			else if (nErr == -1 )
			{
				// 因为硬伤而处理失败，直接抛弃该条数据
				// todo
				LOG_(LOGID_DEBUG, LOG_F("[DDSManager][Key:%s][File:%s]添加到数据库失败(配置对此文件不进行分发处理，抛弃)"), LOG_STR(stDDSFile.strProductKey), LOG_STR(stDDSFile.strFileName));
			}
			else
			{
				LOG_(LOGID_DEBUG, LOG_F("[DDSManager][Key:%s][File:%s]添加到数据库失败 errorcode=%d."), LOG_STR(stDDSFile.strProductKey), LOG_STR(stDDSFile.strFileName), nErr);

				// 其他处理失败情况，
				stDDSFile.nFailedNum++;
				// 将此文件添加到队尾，等待下次执行时处理
				m_lstDDSFile.push_back(stDDSFile);
			}
		}

		// 移除队列
		m_lstDDSFile.pop_front();
		nSize--;
	}

	// 如果有新的插入，则直接执行数据分发任务
	if (bNewJobFlg)
	{
		if (m_pTaskDDS)
		{
			m_pTaskDDS->start();
		}
	}
	
	return 0;
}

/************************************************************************/
/* 线程执行方法                                                         */
/************************************************************************/
int CDDSManager::Proccess(ST_DDSFILE stDDSFile)
{
	// 1. 获取数据分发目的地信息
	if (!m_hasDDS.contains(stDDSFile.strDDSID))
	{
		// 数据分发目的地列表中没有此信息
		return -1;
	}
	ST_DDS stDDS = m_hasDDS.value(stDDSFile.strDDSID);

	// 2. 验证文件是否存在, 获取文件信息
	QString strFile = QString("%1/%2").arg(stDDSFile.strFilePath).arg(stDDSFile.strFileName);
	strFile = strFile.replace("\\", "/");

	QFileInfo fileInfo(strFile);
	if (!fileInfo.exists())
	{
		return -2;
	}

	// 获取文件信息 
	int nFileSize = fileInfo.size();         // 文件大小
	QDateTime tFileCreated = fileInfo.created(); // 文件创建时间

	// 3. 存储该条信息到数据库中 
	int nErr = HandleDB::getClass().SaveDDSFileInfo(stDDSFile, nFileSize, tFileCreated, stDDS);
	if (nErr == 0)
	{
		// todo 添加到数据库成功
	}  
	else
	{
		// todo 添加到数据库失败
	}

	return nErr;
}