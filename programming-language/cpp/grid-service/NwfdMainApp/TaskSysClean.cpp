#include "log.h" // log日志
#include "TaskSysClean.h"
#include "HandleCommon.h"
#include <QDir>
#include <QFileInfoList>
#include <QStringList>

/************************************************************************/
/* 构造函数                                                             */
/************************************************************************/
TaskSysClean::TaskSysClean()
{
	LOG_(LOGID_DEBUG, LOG_F("Entry TaskSysClean()"));
	m_strTaskName = "系统清理维护任务";
	SetTaskType(TASK_CLEAN); // 任务类型

	m_pCachedImpl = NULL;
	m_bReady = true;
	m_bFirst = true;

	m_emTaskState = _TASK_READ; 

	LOG_(LOGID_DEBUG, LOG_F("Leave TaskSysClean()"));
}

/************************************************************************/
/* 析构函数                                                             */
/************************************************************************/
TaskSysClean::~TaskSysClean()
{
	LOG_(LOGID_DEBUG, LOG_F("Leave ~TaskSysClean()"));
	// 停止线程执行
	LOG_(LOGID_DEBUG, LOG_F("[TaskSysClean]开始停止线程执行."));

	m_bReady = false;
	wait();

	// 删除缓存连接
	if (m_pCachedImpl)
	{
		delete m_pCachedImpl;
		m_pCachedImpl = NULL;
	}

	LOG_(LOGID_DEBUG, LOG_F("~缓存维护任务（TaskSysClean）析构."));

	LOG_(LOGID_DEBUG, LOG_F("Leave ~TaskSysClean()"));
}

/************************************************************************/
/* 设置产品列表                                                         */
/************************************************************************/
void TaskSysClean::Init(ST_CACHED stCached, ST_SYSCLEAN stSysClean, HASH_PRODUCT hasProduct, HASH_CLIENT hasClient, LIST_STATION_CFG lstStationCfg, ST_STATION_MAG stStationMag)
{
	m_stCached = stCached;
	m_stSysClean = stSysClean;
	m_hasProduct = hasProduct;
	m_hasClient = hasClient;
	m_lstStationCfg = lstStationCfg;
	m_stStationMag = stStationMag;

	if (!m_stCached.bDisabled && m_hasProduct.size() > 0)
	{
		// 任务已准备就绪，状态设置为准备状态
		m_emTaskState = _TASK_READ;

		LOG_(LOGID_DEBUG, LOG_F("[TaskSysClean]任务初始化完毕，准备就绪..."));
	}
}

// 设置
void TaskSysClean::setRootPath(QString strRootPath)
{
	m_strRoot = strRootPath;
}
void TaskSysClean::setCCCC(QString strCCCC)
{
	m_strCccc = strCCCC;
}

/************************************************************************/
/* 任务处理执行方法                                                     */
/************************************************************************/
void TaskSysClean::run()
{
	LOG_(LOGID_INFO, LOG_F("[TaskSysClean] 系统清理任务开始执行."));

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
			if (m_stCached.bDisabled)
			{
				// 状态变为开始执行
				m_emTaskState = _TASK_RUNNING;
			}
			else
			{
				// 缓存功能启动，先进行缓存连接
				// 缓存连接
				if (m_pCachedImpl == NULL)  
				{
					// 创建连接
					m_pCachedImpl = new CachedImpl(m_stCached);
					if (m_pCachedImpl == NULL)
					{
						// 结束本次任务
						m_emTaskState = _TASK_STOP;
					}
				}
				else if (m_pCachedImpl->ConnectStatus() != SUCCESS)
				{
					// 连接状态异常，进行重连
					m_emTaskState = _TASK_UNINTERRUPTIBLE;
					m_nReConTryNum = 0;
				}
				else
				{
					// 状态变为开始执行
					m_emTaskState = _TASK_RUNNING;   
				}
			}
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
					LOG_(LOGID_INFO, LOG_F("[系统清理] 执行本此任务成功"));
				}
				else
				{
					LOG_(LOGID_INFO, LOG_F("[系统清理] 执行本此任务失败"));
				}
			}
			catch (...)
			{
				LOG_(LOGID_ERROR, LOG_F("[系统清理]执行扫描处理任务异常"));
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
		{
			if(m_pCachedImpl->ConnectStatus() == SUCCESS)
			{
				// 连接状态正常，准备执行
				m_emTaskState = _TASK_READ;
			}
			else
			{
				// 等待10秒后，重新连接
				sleep(10);
				m_pCachedImpl->ReConnect();
				m_nReConTryNum++;

				// 尝试次数判断，超过尝试次数，不进行缓存清理
				if (m_nReConTryNum >= MAX_RECONNECT_NUM)
				{
					LOG_(LOGID_INFO, LOG_F("[TaskSysClean] 缓存连接异常，已超过重连次数，放弃缓存清理，开始其他清理."));

					m_emTaskState = _TASK_RUNNING;
				}
			}

			break;
		}
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

	LOG_(LOGID_INFO, LOG_F("[TaskSysClean] 系统清理任务执行结束"));
}

/************************************************************************/
/* 执行维护任务                                                         */
/************************************************************************/
bool TaskSysClean::ExecTask()
{
	// 检查日志
	CheckLog();

	// 检查文件
	CheckFile();

	return true;


	//// 如果是首次，先进行缓存产品Key清理工作
	//if (m_bFirst)
	//{
	//	// 检查Product和Merge下面的Key
	//	if (CheckProductKey() && CheckMergeKey())
	//	{
	//		// 执行成功后将不再执行
	//		m_bFirst = false;
	//	}
	//}

	//// 检查产品
	//bool bRet = CheckProduct();

	//// 检查拼图
	//bRet = CheckMerge() && bRet;

	//// 检查客户端
	//bRet = CheckClient() && bRet;

	//// 检查站点缓存
	//bRet = CheckStation() && bRet;

	//// 检查格点站点同步缓存
	//bRet = CheckStationManage() && bRet;

	//// 检查日志
	//CheckLog();

	//// 检查数据库
	//CheckDataBase();

	//// 检查文件
	//CheckFile();

	//return bRet;
}

/************************************************************************/
/* 检查产品Key                                                          */
/************************************************************************/
bool TaskSysClean::CheckProductKey()
{
	if (m_pCachedImpl == NULL)
	{
		return false;
	}

	StringList lstProductKey;

	//  获取所有的Key
	if (m_pCachedImpl->GetAllProductKey(lstProductKey, true) != SUCCESS)
	{
		return false;
	}

	// 检验产品Key
	string strTemp;
	QString strProductKey;
	StringList::iterator iter;
	for (iter = lstProductKey.begin(); iter != lstProductKey.end(); iter++)
	{
		strTemp = *iter;
		strProductKey = QString::fromLocal8Bit(strTemp.data());
		if (FindProduct(strProductKey))
		{
			// 如果缓存中原有产品在当前维护列表中，跳过下面的处理
			continue;
		}

		// 如果该产品已经不在维护队列中，删除该产品的所有内容
		if (m_pCachedImpl->DeleteProduct(strProductKey.toLocal8Bit().data()) == SUCCESS)
		{
			// todo 删除成功
			LOG_(LOGID_INFO, LOG_F("[TaskSysClean] 删除产品缓存成功（Key=%s）."), LOG_STR(strProductKey));
		}
		else
		{
			// todo 删除失败
			LOG_(LOGID_INFO, LOG_F("[TaskSysClean] 删除产品缓存失败（Key=%s）."), LOG_STR(strProductKey));
		}
	}

	return true;
}

/************************************************************************/
/* 检查拼图Key                                                          */
/************************************************************************/
bool TaskSysClean::CheckMergeKey()
{
	if (m_pCachedImpl == NULL)
	{
		return false;
	}

	StringList lstMergeKey;

	//  获取所有的Key
	if (m_pCachedImpl->GetAllMergeKey(lstMergeKey, true) != SUCCESS)
	{
		return false;
	}

	// 检验产品Key
	string strTemp;
	QString strMergeKey;
	StringList::iterator iter;
	for (iter = lstMergeKey.begin(); iter != lstMergeKey.end(); iter++)
	{
		strTemp = *iter;
		strMergeKey = QString::fromLocal8Bit(strTemp.data());
		if (FindProduct(strMergeKey))
		{
			// 如果缓存中原有产品在当前维护列表中，跳过下面的处理
			continue;
		}

		// 如果该产品已经不在维护队列中，删除该产品的所有内容
		if (m_pCachedImpl->DeleteMerge(strMergeKey.toLocal8Bit().data()) == SUCCESS)
		{
			// todo 删除成功
			LOG_(LOGID_INFO, LOG_F("[TaskSysClean] 删除拼图缓存成功（Key=%s）."), LOG_STR(strMergeKey));
		}
		else
		{
			// todo 删除失败
			LOG_(LOGID_INFO, LOG_F("[TaskSysClean] 删除拼图缓存失败（Key=%s）."), LOG_STR(strMergeKey));
		}
	}

	return true;
}

/************************************************************************/
/* 维护产品数据                                                         */
/************************************************************************/
bool TaskSysClean::CheckProduct()
{
	LOG_(LOGID_DEBUG, LOG_F("[TaskSysClean]检测产品数据时效开始."));

	if (m_pCachedImpl == NULL)
	{
		return false;
	}

	bool bRet = true;

	// 检查所有的产品数据是否已经超过时效
	HASH_PRODUCT::iterator iter;
	QString strProductKey;
	for (iter = m_hasProduct.begin(); iter != m_hasProduct.end(); iter++)
	{
		strProductKey = iter.key();
		ST_PRODUCT stProduct = iter.value();
		if (stProduct.strRole != "raw")
		{
			continue;
		}
		bool bResult = CheckProductData(strProductKey, stProduct.stCachedSave.nValidTime);
		bRet = bRet && bResult;
	}

	//LOG_(LOGID_DEBUG, LOG_F("[TaskPID:%d]检测产品数据时效结束，结果=%d(0:失败，1：成功)"), m_nTaskPID, bRet);

	return bRet;
}

/************************************************************************/
/* 检测某一产品                                                         */
/************************************************************************/
bool TaskSysClean::CheckProductData(QString strProductKey, int nValidTime)
{
	//LOG_(LOGID_DEBUG, LOG_F("[TaskPID:%d][Key=%s]开始检查产品,有效时间=%d"), m_nTaskPID, LOG_STR(strProductKey), nValidTime);

	if (m_pCachedImpl == NULL)
	{
		return false;
	}

	StringList lstProductTime;
	bool bRet = true;

	//  获取所有的Time
	if (m_pCachedImpl->GetProductTime(strProductKey.toLocal8Bit().data(), lstProductTime, true) != SUCCESS)
	{
		return false;
	}

	// 检验产品Time
	string strTmp;
	QString strProductTime;
	StringList::iterator iter;
	for (iter = lstProductTime.begin(); iter != lstProductTime.end(); iter++)
	{
		strTmp = *iter;
		strProductTime = QString::fromLocal8Bit(strTmp.data());

		//LOG_(LOGID_DEBUG, LOG_F("[TaskPID:%d][Key=%s]判断产品时间（ProductTime=%s）"), m_nTaskPID, LOG_STR(strProductKey), LOG_STR(strProductTime));

		if (IsValidTime(strProductTime, nValidTime))
		{
			// 产品时间有效，跳过下面的删除操作
			continue;
		}

		// 如果该产品已经不在维护队列中，删除该产品的所有内容
		if (m_pCachedImpl->DeleteProductData(strProductKey.toLocal8Bit().data(), strProductTime.toLocal8Bit().data()) == SUCCESS)
		{
			// 删除成功
			LOG_(LOGID_DEBUG, LOG_F("[TaskSysClean][Key=%s]产品数据已到期，删除产品数据成功（ProductTime=%s）."), LOG_STR(strProductKey), LOG_STR(strProductTime));
		}
		else
		{
			// todo 删除失败
			LOG_(LOGID_DEBUG, LOG_F("[TaskSysClean][Key=%s]产品数据已到期，删除产品数据失败（ProductTime=%s）."), LOG_STR(strProductKey), LOG_STR(strProductTime));
			bRet = false;
		}
	}

	//LOG_(LOGID_DEBUG, LOG_F("[TaskPID:%d][Key=%s]检查产品结束"), m_nTaskPID, LOG_STR(strProductKey), nValidTime);

	return bRet;
}

/************************************************************************/
/* 维护拼图数据                                                         */
/************************************************************************/
bool TaskSysClean::CheckMerge()
{
	LOG_(LOGID_DEBUG, LOG_F("[TaskSysClean]检测拼图数据时效开始."));

	if (m_pCachedImpl == NULL)
	{
		return false;
	}

	bool bRet = true;

	// 检查所有的产品数据是否已经超过时效
	HASH_PRODUCT::iterator iter;
	QString strMergeKey;
	for (iter = m_hasProduct.begin(); iter != m_hasProduct.end(); iter++)
	{
		strMergeKey = iter.key();
		ST_PRODUCT stProduct = iter.value();
		if (stProduct.strRole != "merge")
		{
			continue;
		}

		bool bResult = CheckMergeData(stProduct.strRelatedKey, stProduct.stCachedSave.nValidTime);
		bRet = bRet && bResult;
	}

	//LOG_(LOGID_DEBUG, LOG_F("[TaskPID:%d]检测产品数据时效结束，结果=%d(0:失败，1：成功)"), m_nTaskPID, bRet);

	return bRet;
}

/************************************************************************/
/* 检测某一拼图数据                                                     */
/************************************************************************/
bool TaskSysClean::CheckMergeData(QString strProductKey, int nValidTime)
{
	if (m_pCachedImpl == NULL)
	{
		return false;
	}

	StringList lstMergeTime;
	bool bRet = true;

	//  获取所有的Time
	if (m_pCachedImpl->GetMergeTime(strProductKey.toLocal8Bit().data(), lstMergeTime, true) != SUCCESS)
	{
		return false;
	}

	// 检验产品Time
	string strTmp;
	QString strMergeTime;
	StringList::iterator iter;
	for (iter = lstMergeTime.begin(); iter != lstMergeTime.end(); iter++)
	{
		strTmp = *iter;
		strMergeTime = QString::fromLocal8Bit(strTmp.data());

		if (IsValidTime(strMergeTime, nValidTime))
		{
			// 产品时间有效，跳过下面的删除操作
			continue;
		}

		// 如果该产品已经不在维护队列中，删除该产品的所有内容
		if (m_pCachedImpl->DeleteMergeData(strProductKey.toLocal8Bit().data(), strMergeTime.toLocal8Bit().data()) == SUCCESS)
		{
			// 删除成功
			LOG_(LOGID_DEBUG, LOG_F("[TaskSysClean][Key=%s]产品数据已到期，删除拼图数据成功（ProductTime=%s）."), LOG_STR(strProductKey), LOG_STR(strMergeTime));
		}
		else
		{
			// todo 删除失败
			LOG_(LOGID_DEBUG, LOG_F("[TaskSysClean][Key=%s]产品数据已到期，删除拼图数据失败（ProductTime=%s）."), LOG_STR(strProductKey), LOG_STR(strMergeTime));
			bRet = false;
		}
	}

	return bRet;
}

/************************************************************************/
/* 维护客户端数据                                                       */
/************************************************************************/
bool TaskSysClean::CheckClient()
{
	LOG_(LOGID_DEBUG, LOG_F("[TaskSysClean]检测拼图数据时效开始."));

	if (m_pCachedImpl == NULL)
	{
		return false;
	}

	bool bRet = true;

	// 检查所有的产品数据是否已经超过时效
	HASH_CLIENT::iterator iter;
	for (iter = m_hasClient.begin(); iter != m_hasClient.end(); iter++)
	{
		ST_CLIENT_INFO stClient = iter.value();

		// 获取该客户端下面的所有Key
		StringList lstClientKey;
		if (m_pCachedImpl->GetAllClientKey(stClient.nClientID, lstClientKey, true) != SUCCESS)
		{
			continue;
		}

		// 检验产品Key
		string strTemp;
		QString strProductKey;
		StringList::iterator iter;
		for (iter = lstClientKey.begin(); iter != lstClientKey.end(); iter++)
		{
			strTemp = *iter;
			strProductKey = QString::fromLocal8Bit(strTemp.data());
			bool bResult = CheckClientData(stClient.nClientID, strProductKey, stClient.nCachedValidTime);
			bRet = bRet && bResult;
		}
	}

	return bRet;
}

/************************************************************************/
/* 检测某一拼图数据                                                     */
/************************************************************************/
bool TaskSysClean::CheckClientData(int nClientID, QString strProductKey, int nValidTime)
{
	if (m_pCachedImpl == NULL)
	{
		return false;
	}

	StringList lstClientTime;
	bool bRet = true;

	//  获取所有的Time
	if (m_pCachedImpl->GetClientTime(nClientID, strProductKey.toLocal8Bit().data(), lstClientTime, true) != SUCCESS)
	{
		return false;
	}

	// 检验产品Time
	string strTmp;
	QString strClientTime;
	StringList::iterator iter;
	for (iter = lstClientTime.begin(); iter != lstClientTime.end(); iter++)
	{
		strTmp = *iter;
		strClientTime = QString::fromLocal8Bit(strTmp.data());

		if (IsValidTime(strClientTime, nValidTime))
		{
			// 产品时间有效，跳过下面的删除操作
			continue;
		}

		// 如果该产品已经不在维护队列中，删除该产品的所有内容
		if (m_pCachedImpl->DeleteClientData(nClientID, strProductKey.toLocal8Bit().data(), strClientTime.toLocal8Bit().data()) == SUCCESS)
		{
			// 删除成功
			LOG_(LOGID_DEBUG, LOG_F("[TaskSysClean][ClientID=%d]产品数据已到期，删除客户端数据成功（Key=%s,ProductTime=%s）."), nClientID, LOG_STR(strProductKey), LOG_STR(strClientTime));
		}
		else
		{
			// todo 删除失败
			LOG_(LOGID_DEBUG, LOG_F("[TaskSysClean][ClientID=%d]产品数据已到期，删除客户端数据失败（Key=%s,ProductTime=%s）."), nClientID, LOG_STR(strProductKey), LOG_STR(strClientTime));
			bRet = false;
		}
	}

	return bRet;
}

/************************************************************************/
/* 维护站点数据                                                         */
/************************************************************************/
bool TaskSysClean::CheckStation()
{
	LOG_(LOGID_DEBUG, LOG_F("[TaskSysClean]检测站点数据时效开始."));

	if (m_pCachedImpl == NULL)
	{
		return false;
	}

	bool bRet = true;

	QStringList lstDateType;
	lstDateType.push_back("SCMOC");
	lstDateType.push_back("SPCC");
	lstDateType.push_back("SPVT");
	lstDateType.push_back("SNWFD");
	lstDateType.push_back("SFJC");
	lstDateType.push_back("SCMOU");

	int nValidTime;
	// 检查所有的站点数据是否已经超过时效
	LIST_STATION_CFG::iterator iter;
	for (iter = m_lstStationCfg.begin(); iter != m_lstStationCfg.end(); iter++)
	{
		ST_STATION_CFG stStation = *iter;
		if (stStation.strRole != "raw")
		{
			continue;
		}

		QStringList::iterator iterType;
		for (iterType = lstDateType.begin(); iterType != lstDateType.end(); iterType++)
		{
			QString strDataType = *iterType;

			// 存储缓存的有效时间
			if (stStation.stCachedSave.bDisabled)
			{
				nValidTime = 86400;
			}
			else
			{
				nValidTime = stStation.stCachedSave.nValidTime;
			}

			// 获取该站点下面的所有站号
			StringList lstStationNo;
			if (m_pCachedImpl->GetAllStationNo(stStation.strCityType.toLocal8Bit().data(), strDataType.toLocal8Bit().data(), stStation.nTimeRange, lstStationNo, true) != SUCCESS)
			{
				continue;
			}

			// 检验产品Key
			string strStationNo;
			StringList::iterator iter;
			for (iter = lstStationNo.begin(); iter != lstStationNo.end(); iter++)
			{
				strStationNo = *iter;
				bool bResult = CheckStationData(stStation.strCityType, strDataType, stStation.nTimeRange, strStationNo.data(), nValidTime);
				bRet = bRet && bResult;
			}
		}
	}

	return bRet;
}

/************************************************************************/
/* 维护站点-格点同步数据                                                */
/************************************************************************/
bool TaskSysClean::CheckStationManage()
{
	LOG_(LOGID_DEBUG, LOG_F("[TaskSysClean]检测站点管理数据时效开始."));
	if (m_stStationMag.nValidTime == 0)
	{
		return true;
	}

	if (m_pCachedImpl == NULL)
	{
		return false;
	}
	bool bRet = true;

	// 检查所有的站点数据是否已经超过时效
	StringList lstStationFileType;
	if (m_pCachedImpl->GetAllMultiStationType(lstStationFileType) != SUCCESS)
	{
		return false;
	}

	for (StringList::iterator iterType = lstStationFileType.begin(); iterType != lstStationFileType.end(); iterType++)
	{
		string strStationType = *iterType;
		string::size_type nOff = 0;
		int nClientID = -1;
		int nTimeRange;
		string strDataType;

		string strData1 = getdata(strStationType, "_", nOff);
		if (strData1.size() > 3 && memcmp(strData1.data(), "CLT", 3) == 0)
		{
			char szClientID[10];
			memset(szClientID, 0x00, sizeof(szClientID));
			memcpy(szClientID, strData1.data() + 3, strData1.length() - 3);
			nClientID = atoi(szClientID);

			strDataType = getdata(strStationType, "_", nOff);
		}
		else
		{
			strDataType = strData1;
		}
		string strRange = getdata(strStationType, "_", nOff);
		nTimeRange = atoi(strRange.data());

		if (nClientID != -1)
		{
			strDataType = strData1 + "_" + strDataType;
		}

		StringList lstProductTime;
		//  获取所有的Time
		if (m_pCachedImpl->GetMultiStationTime(strDataType.data(), nTimeRange, lstProductTime, true) != SUCCESS)
		{
			continue;
		}

		// 检验产品Time
		string strTmp;
		QString strProductTime;
		StringList::iterator iter;
		for (iter = lstProductTime.begin(); iter != lstProductTime.end(); iter++)
		{
			strTmp = *iter;
			strProductTime = QString::fromLocal8Bit(strTmp.data());

			//LOG_(LOGID_DEBUG, LOG_F("[TaskPID:%d][Key=%s]判断产品时间（ProductTime=%s）"), m_nTaskPID, LOG_STR(strProductKey), LOG_STR(strProductTime));

			if (IsValidTime(strProductTime, m_stStationMag.nValidTime))
			{
				// 产品时间有效，跳过下面的删除操作
				continue;
			}

			// 如果该产品已经不在维护队列中，删除该产品的所有内容
			if (m_pCachedImpl->DeleteMultiStationData(strDataType.data(), nTimeRange, strTmp.data()) == SUCCESS)
			{
				// 删除成功
				LOG_(LOGID_DEBUG, LOG_F("[TaskSysClean][格点同步数据]数据已到期，删除产品数据成功（ProductTime=%s）."), LOG_STR(strProductTime));
			}
			else
			{
				// todo 删除失败
				LOG_(LOGID_DEBUG, LOG_F("[TaskSysClean][格点同步数据]数据已到期，删除产品数据失败（ProductTime=%s）."), LOG_STR(strProductTime));
				bRet = false;
			}
		}
	}

	return bRet;
}

/************************************************************************/
/* 检测某一站点数据                                                     */
/************************************************************************/
bool TaskSysClean::CheckStationData(QString strCityType, QString strDataType, int nTimeRange, const char * szStationNo, int nValidTime)
{
	if (m_pCachedImpl == NULL)
	{
		return false;
	}

	StringList lstStationTime;
	bool bRet = true;

	//  获取所有的Time
	if (m_pCachedImpl->GetStationTime(strCityType.toLocal8Bit().data(), strDataType.toLocal8Bit().data(), nTimeRange, szStationNo, lstStationTime, true) != SUCCESS)
	{
		return false;
	}

	// 检验产品Time
	string strTmp;
	QString strStationTime;
	StringList::iterator iter;
	for (iter = lstStationTime.begin(); iter != lstStationTime.end(); iter++)
	{
		strTmp = *iter;
		strStationTime = QString::fromLocal8Bit(strTmp.data());

		if (IsValidTime(strStationTime, nValidTime))
		{
			// 产品时间有效，跳过下面的删除操作
			continue;
		}

		// 如果该产品已经不在维护队列中，删除该产品的所有内容
		if (m_pCachedImpl->DeleteStationData(strCityType.toLocal8Bit().data(), strDataType.toLocal8Bit().data(), nTimeRange, szStationNo, strStationTime.toLocal8Bit().data()) == SUCCESS)
		{
			// 删除成功
			LOG_(LOGID_DEBUG, LOG_F("[TaskSysClean][%s,%s,%d,%s,%s]产品数据已到期，删除站点数据成功."), LOG_STR(strCityType), LOG_STR(strDataType), nTimeRange, szStationNo, LOG_STR(strStationTime));
		}
		else
		{
			// todo 删除失败
			LOG_(LOGID_DEBUG, LOG_F("[TaskSysClean][%s,%s,%d,%s,%s]产品数据已到期，删除站点数据失败."), LOG_STR(strCityType), LOG_STR(strDataType), nTimeRange, szStationNo, LOG_STR(strStationTime));
			bRet = false;
		}
	}

	return bRet;
}

/************************************************************************/
/* 查找是否存在该产品                                                   */
/************************************************************************/
bool TaskSysClean::FindProduct(QString strProductKey)
{
	if (m_hasProduct.contains(strProductKey))
	{
		return true;
	}

	return false;
}

/************************************************************************/
/* 产品时间是否有效                                                     */
/* 返回值： true表示在有效时间范围内，false表示已经超出时效             */
/************************************************************************/
bool TaskSysClean::IsValidTime(QString strTime, int nValidTime)
{
	int   nYear, nMonth, nDay, nHour;

	// 解析ProductTime字符串获取时间
	nYear = strTime.mid(0, 4).toInt();
	nMonth = strTime.mid(4, 2).toInt();
	nDay = strTime.mid(6, 2).toInt();
	nHour = strTime.mid(8, 2).toInt();

	// 获取产品时间
	QString  strProductime = QString("").sprintf("%04d-%02d-%02d %02d:%02d:%02d", nYear, nMonth, nDay, nHour, 0, 0);
	QDateTime tProductTime = QDateTime::fromString(strProductime, "yyyy-MM-dd hh:mm:ss");

	// 当前时间
	time_t tNow = time(NULL);

	// 比较当前时间是否超出ProdutTime的时效
	if ((tNow - tProductTime.toTime_t()) < nValidTime)
	{
		// 在有效范围内
		return true;
	}

	return false;
}


/************************************************************************/
/* 检查日志                                                             */
/************************************************************************/
bool TaskSysClean::CheckLog()
{
	QDir dDir;
	QFileInfoList lstFile;
	QString strLogDir = "./log";

	// 检验产品目录是否存在
	if (!dDir.exists(strLogDir))
	{
		return true;
	}

	// 保留天数
	int nLogKeeyDay = m_stSysClean.nLogKeepDay;
	if (nLogKeeyDay < MIN_LOG_KEEP_DAY)
	{
		nLogKeeyDay = MIN_LOG_KEEP_DAY;
	}

	// 删选日期
	QDateTime tToday = QDateTime::currentDateTime();

	// 获取日志文件
	dDir.setPath(strLogDir);       // 设置目录名称
	dDir.setFilter(QDir::Files);   // 列出目录中的文件
	lstFile = dDir.entryInfoList();

	// 遍历检测
	QFileInfoList::iterator iter;
	for (iter = lstFile.begin(); iter != lstFile.end(); iter++)
	{
		QFileInfo file = *iter;
		QString strFileName = file.fileName();
		QStringList lstFileName = strFileName.split("_");
		QString strDate = lstFileName.at(1);
		QDateTime tLogDate = QDateTime::fromString(strDate, "yyyyMMdd");
		QDateTime tDate = tLogDate.addDays(nLogKeeyDay);
		if (tDate < tToday)
		{
			// 删除文件
			QString strFile = QString("%1/%2").arg(strLogDir).arg(strFileName);
			if (QFile::remove(strFile))
			{
				LOG_(LOGID_DEBUG, LOG_F("[TaskSysClean]删除日志文件成功（%s）."), LOG_STR(strFileName));
			}
		}
	}

	return true;
}

/************************************************************************/
/* 检查数据库                                                           */
/************************************************************************/
bool TaskSysClean::CheckDataBase()
{
	// 保留天数
	int nDBKeeyDay = m_stSysClean.nDBKeepDay;
	if (nDBKeeyDay < MIN_DB_KEEP_DAY)
	{
		nDBKeeyDay = MIN_DB_KEEP_DAY;
	}
	nDBKeeyDay *= -1;

	// 删选日期
	QDateTime tToday = QDateTime::currentDateTime();
	QDateTime tDate = tToday.addDays(nDBKeeyDay);
	QString strDate = tDate.toString("yyyy-MM-dd hh:mm:ss.zzz");

	// 删除DDS数据
	if (HandleDB::getClass().DeleteDDS(strDate) == 0)
	{
		LOG_(LOGID_DEBUG, LOG_F("[TaskSysClean]执行数据库DDS数据删除成功."));
	}

	// 删除客户端数据
	if (HandleDB::getClass().DeleteClientData(strDate) == 0)
	{
		LOG_(LOGID_DEBUG, LOG_F("[TaskSysClean]执行数据库客户端数据删除成功."));
	}

	// 删除任务数据
	if (HandleDB::getClass().DeleteTaskData(strDate) == 0)
	{
		LOG_(LOGID_DEBUG, LOG_F("[TaskSysClean]执行数据库任务监控数据删除成功."));
	}

	//删除站点收发数据
	if (HandleDB::getClass().DeleteStationRecvData(strDate) == 0)
	{
		LOG_(LOGID_DEBUG, LOG_F("[TaskSysClean]执行数据库站点接收数据删除成功."));
	}

	if (HandleDB::getClass().DeleteStationSendData(strDate) == 0)
	{
		LOG_(LOGID_DEBUG, LOG_F("[TaskSysClean]执行数据库站点发送数据删除成功."));
	}


	//删除MQ收发数据
	if (HandleDB::getClass().DeleteMqRecvData(strDate) == 0)
	{
		LOG_(LOGID_DEBUG, LOG_F("[TaskSysClean]执行数据库MQ接收数据删除成功."));
	}

	if (HandleDB::getClass().DeleteMqSendData(strDate) == 0)
	{
		LOG_(LOGID_DEBUG, LOG_F("[TaskSysClean]执行数据库MQ发送数据删除成功."));
	}


	return false;
}

/************************************************************************/
/* 检查文件                                                             */
/************************************************************************/
bool TaskSysClean::CheckFile()
{
	// 修改为通过配置根目录，递归遍历该目录下的文件和文件夹
	// 比较文件的创建日期是否符合配置条件，从而进行删除/保留处理
	QDateTime tToday = QDateTime::currentDateTime();
	int nYear = tToday.date().year();
	int nMonth = tToday.date().month();
	int nDay = tToday.date().day();

	QStringList::iterator iter;
	for (iter = m_stSysClean.lstFileDir.begin(); iter != m_stSysClean.lstFileDir.end(); iter++)
	{
		QString strDir = *iter;

		// 替换符号
		QString strPath = HandleCommon::GetFolderPath(strDir, nYear, nMonth, nDay, m_strRoot, "", m_strCccc);

		// 遍历检查该目录
		HandleFile(strPath);
	}

	return true;
}

/************************************************************************/
/* 文件检查判断处理 :通过创建日期进行判断                               */
/************************************************************************/
bool TaskSysClean::HandleFile(QString strPath)
{
	QDir dDir;

	// 检验目录是否存在
	if (!dDir.exists(strPath))
	{
		return false;
	}

	// 目录设置
	dDir.setPath(strPath); // 设置目录名称
	dDir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot); // 列出目录中的文件和目录
	//dDir.setSorting(QDir::Name);            // 按照文件名排序

	// 获取文件列表
	QFileInfoList lstFile = dDir.entryInfoList();
	
	// 当前时间
	QDateTime tNow = QDateTime::currentDateTime();

	// 遍历文件进行判断
	QFileInfoList::iterator iter;
	for (iter = lstFile.begin(); iter != lstFile.end(); iter++)
	{
		QFileInfo file = *iter;

		// 判断是文件还是文件夹，如果是文件夹则递归调用
		if (file.isDir())
		{
			QString strDir = file.filePath();
			HandleFile(strDir);
		}
		else
		{
			// 文件创建时间
			QDateTime tFileCreated = file.created();

			if ((tNow.toTime_t() - tFileCreated.toTime_t()) > (uint)m_stSysClean.nFileKeepDay * 24 * 3600)
			{
				// 删除文件
				QString strFile = QString("%1/%2").arg(strPath).arg(file.fileName());
				if (QFile::remove(strFile))
				{
					LOG_(LOGID_DEBUG, LOG_F("[TaskSysClean]删除历史文件成功（%s）."), LOG_STR(file.fileName()));
				}
			}
		}
	}

	return true;
}
