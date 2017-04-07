#include "log.h"
#include "NetworkManager.h"
#include "HandleNwfdLib.h"  // grib2文件
#include "CachedImpl.h"     // cached
#include "HandleGrib.h"
#include "HandleDB.h"
#include "Mask.h"
#include "ProtocolDef.h"
#include "DataManager.h"
#include "StationDef.h"

#include "CachedManager.h"

#include "TaskProductClientMerge.h"

#pragma  execution_character_set("utf-8")

/************************************************************************/
/* 构造函数                                                             */
/************************************************************************/
NetworkManager::NetworkManager()
{
	LOG_(LOGID_DEBUG, LOG_F("Entry NetworkManager()"));
	
	m_pTaskTcpServer = NULL;
	m_pTaskStationMsg = NULL;
	m_bIsPublish = true;

	LOG_(LOGID_DEBUG, LOG_F("Leave NetworkManager()"));
}

/************************************************************************/
/* 析构函数                                                             */
/************************************************************************/
NetworkManager::~NetworkManager()
{
	LOG_(LOGID_DEBUG, LOG_F("Entry ~NetworkManager()"));
	
	CachedManager::getClass().DestroyAllConnect();

	LOG_(LOGID_DEBUG, LOG_F("Leave ~NetworkManager()"));
}

/************************************************************************/
/* 单例                                                                 */
/************************************************************************/
NetworkManager & NetworkManager::getClass()
{
	static NetworkManager objNetworkManager;
	return objNetworkManager;
}

/************************************************************************/
/* 初始化                                                               */
/************************************************************************/
int NetworkManager::Init(HASH_PRODUCT hasProduct, HASH_CLIENT  hasClient, ST_CACHED stCached)
{
	m_hasProduct = hasProduct; // 产品列表
	m_hasClient = hasClient;   // 客户端列表
	m_stCached = stCached;     // 缓存配置

	// 初始化缓存连接
	InitCached();

	return 0;
}

/************************************************************************/
/* 设置服务器监听任务                                                   */
/************************************************************************/
void NetworkManager::SetTcpServer(TaskTcpServer*  pTaskTcpServer)
{
	m_pTaskTcpServer = pTaskTcpServer;
}

/************************************************************************/
/* 设置站点消息处理任务                                                 */
/************************************************************************/
void NetworkManager::SetStationMsg(TaskStationMsg* pTaskStationMsg)
{
	m_pTaskStationMsg = pTaskStationMsg;
}

/************************************************************************/
/* 设置是否进行发布                                                     */
/************************************************************************/
void NetworkManager::SetIsPublish(bool bIsPublish)
{
	m_bIsPublish = bIsPublish;
}

/************************************************************************/
/* 添加管理产品                                                         */
/************************************************************************/
bool NetworkManager::AddProduct(ST_PRODUCT  stProduct)
{
	// 此列表中的key为关联的ProductKey，而非配置的Key,每个ProductKey只有一个Key对应，否则错误
	if (FindProduct(stProduct.strRelatedKey))
	{
		// 已存在
		return false;
	}

	// 添加到任务列表中
	QMutexLocker locker(&m_mutexProduct);
	m_hasProduct.insert(stProduct.strRelatedKey, stProduct);

	return true;
}

/************************************************************************/
/* 查找管理产品                                                         */
/************************************************************************/
bool NetworkManager::FindProduct(QString strProductKey)
{
	QMutexLocker locker(&m_mutexProduct);

	if (m_hasProduct.contains(strProductKey))
	{
		return true;
	}

	return false;
}

/************************************************************************/
/* 移除管理产品                                                         */
/************************************************************************/
bool NetworkManager::RemoveProduct(QString strProductKey)
{
	if (FindProduct(strProductKey))
	{
		m_mutexProduct.lock();
		m_hasProduct.remove(strProductKey);
		m_mutexProduct.unlock();
	}

	return true;
}

/************************************************************************/
/* 获取某产品                                                           */
/************************************************************************/
bool NetworkManager::GetProduct(QString strProductKey, ST_PRODUCT &stProduct)
{
	if (!m_hasProduct.contains(strProductKey))
	{
		return false;
	}

	stProduct = m_hasProduct.value(strProductKey);

	return true;
}

/************************************************************************/
/* 添加任务                                                             */
/************************************************************************/
bool NetworkManager::AddTask(QString strProductKey, TaskBase * pTask)
{
	if (pTask == NULL)
	{
		return false;
	}

	TaskBase * pOrigTask = FindTask(strProductKey);
	if (pOrigTask)
	{
		// 已经存在
		LOG_(LOGID_DEBUG, LOG_F("任务已存在，添加失败（Key=%s）."), LOG_STR(strProductKey));

		return false;
	}

	// 添加到任务列表中
	QMutexLocker locker(&m_mutexTask);
	m_hasProductTask.insert(strProductKey, pTask);

	return true;
}

/************************************************************************/
/* 查找任务                                                             */
/************************************************************************/
TaskBase * NetworkManager::FindTask(QString strProductKey)
{
	QMutexLocker locker(&m_mutexTask);

	TaskBase * pTask = NULL;

	if (m_hasProductTask.contains(strProductKey))
	{
		pTask = m_hasProductTask[strProductKey];
	}

	return pTask;
}

/************************************************************************/
/* 删除任务                                                             */
/************************************************************************/
void NetworkManager::RemoveTask(QString strProductKey)
{
	TaskBase * pTask = FindTask(strProductKey);

	if (pTask)
	{
		m_mutexTask.lock();
		m_hasProductTask.remove(strProductKey);
		m_mutexTask.unlock();

		delete pTask;
		pTask = NULL;
	}
}

/************************************************************************/
/* 是否为合法客户端                                                     */
/************************************************************************/
bool NetworkManager::IsLegalClient(int nClientID, QString strClientIP)
{
	if (!m_hasClient.contains(nClientID))
	{
		// 该客户端编号不在列表中
		LOG_(LOGID_NETWORK, LOG_F("[NetworkManager]客户端[ClientID:%d]不在配置列表中"), nClientID);
		return false;
	}

	// 比较IP地址是否一致,当配置的IP地址为空时，不判断IP绑定
	ST_CLIENT_INFO stClientInfo = m_hasClient.value(nClientID);
	if (stClientInfo.strClientIP.isEmpty())
	{
		return true;
	}

	if (strClientIP == stClientInfo.strClientIP)
	{
		return true;
	}
	else
	{
		LOG_(LOGID_NETWORK, LOG_F("[NetworkManager]客户端[ClientID:%d][IP=%s]IP与配置绑定的不符"), nClientID, LOG_STR(strClientIP));
	}

	return false;
}

/************************************************************************/
/* 是否登录成功                                                         */
/************************************************************************/
bool NetworkManager::IsLogin(int nClientID, QString strUserName, QString strPasswd, bool &bAllowUpdate)
{
	bAllowUpdate = false;

	if (!m_hasClient.contains(nClientID))
	{
		// 该客户端编号不在列表中
		LOG_(LOGID_NETWORK, LOG_F("[NetworkManager]客户端[ClientID:%d]该编号不在配置列表中"), nClientID);
		return false;
	}

	// 验证用户名和密码
	ST_CLIENT_INFO stClientInfo = m_hasClient.value(nClientID);
	if (strUserName == stClientInfo.strUserName && strPasswd == stClientInfo.strPassWord)
	{
		// 是否允许客户端上传数据
		bAllowUpdate = stClientInfo.bAllowUpload;

		return true;
	}
	else
	{
		LOG_(LOGID_NETWORK, LOG_F("[NetworkManager]客户端[ClientID:%d][UserName:%s,PassWord:%s]用户名密码与配置不符，无法通过验证"), nClientID, LOG_STR(strUserName), LOG_STR(strPasswd));
	}

	return false;
}

/************************************************************************/
/* 处理客户端发送来的数据                                               */
/************************************************************************/
int NetworkManager::HandleGridData(int nClientID, QString strProductKey, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond,
	int nForecastTime, int nTimeRange,float fLon1, float fLon2, float fLat1, float fLat2)
{
	// 20150901 zhangl  修改处理过程  收到数据后根据配置保存数据 并保存结果到数据库
	// 保存数据到本地
	int nErr = SaveClientData(nClientID, strProductKey, nYear, nMonth, nDay, nHour, nMinute, nSecond, nForecastTime, nTimeRange, fLon1, fLon2, fLat1, fLat2);
	if (nErr != SUCCESS)
	{
		// todo 添加数据库失败
		LOG_(LOGID_NETWORK, LOG_F("[ERROR]保存客户端[ClientID:%d]上传数据到本地文件失败(err=%d)(Key:%s)."), nClientID, nErr, LOG_STR(strProductKey));
	}

	// 将处理结果保存到数据库
	nErr = HandleDB::getClass().SaveClientDataInfo(nClientID, strProductKey, "","MERGE", nYear, nMonth, nDay, nHour, nMinute, nSecond, 0, nForecastTime, nTimeRange, fLon1, fLon2, fLat1, fLat2);
	if (nErr != 0)
	{
		// todo 添加数据库失败
		LOG_(LOGID_NETWORK, LOG_F("[ERROR][NetworkManager]客户端[ClientID:%d]接收信息保存数据库失败"), nClientID, nErr);
		return nErr;
	}
	else
	{
		// 启动相关的客户端上传数据合并任务线程，异步处理
		if (!m_hasProductTask.contains(strProductKey))
		{
			// 没有创建相应的合并处理任务，初始化错误
			LOG_(LOGID_NETWORK, LOG_F("[ERROR][NetworkManager]客户端[ClientID:%d]不允许该客户端对该产品（%s）进行合并."), nClientID, LOG_STR(strProductKey));
			return -3;
		}
		TaskBase * pTask = m_hasProductTask.value(strProductKey);
		if (pTask == NULL)
		{
			LOG_(LOGID_NETWORK, LOG_F("[ERROR][NetworkManager]客户端[ClientID:%d]没有找到相应处理任务（%s）."), nClientID, LOG_STR(strProductKey));
			// 任务错误
			return -4;
		}

		// 启动任务
		pTask->start();
	}

	return SUCCESS;

	//// 确认该产品是否允许该客户端进行数据合并 : strProductKey为要合并的Key
	//if (!m_hasProduct.contains(strProductKey))
	//{
	//	// 没有相关的合并配置，结束
	//	// TODO,删除客户端缓存数据
	//	DeleteClientData(nClientID, strProductKey);

	//	return -1;
	//}

	// 获取产品信息
	//ST_PRODUCT stProduct = m_hasProduct.value(strProductKey);

	//// 如果允许，验证缓存中是否已经上传数据
	//if (!CheckClientData(nClientID, strProductKey))
	//{
	//	// 数据没有上传成功
	//	return -2;
	//}

	// 验证数据成功，添加信息到数据库中，
	//int nErr = HandleDB::getClass().SaveMergeDataInfo(nClientID, strProductKey, nYear, nMonth, nDay, nHour, nMinute, nSecond, nForecastTime, nTimeRange, fLon1, fLon2, fLat1, fLat2);
	//if (nErr != 0)
	//{
	//	// todo 添加数据库失败
	//	return -5;
	//}

	//// 启动相关的数据合并任务线程，异步处理
	//if (!m_hasProductTask.contains(strProductKey))
	//{
	//	// 没有创建相应的合并处理任务，初始化错误
	//	return -3;
	//}
	//TaskBase * pTask = m_hasProductTask.value(strProductKey);
	//if (pTask == NULL)
	//{
	//	// 任务错误
	//	return -4;
	//}
	//// 启动任务
	//pTask->start();

	//return 0;
}

/************************************************************************/
/* 处理客户端发送来的单站点数据                                         */
/************************************************************************/
int NetworkManager::HandleStationData(int nClientID, QString strCityType, QString strStationNo, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond,
	int nForecastTime, int nTimeRange, float fLat, float fLon, float fHeight)
{
	int nErr;
	float * fData = NULL;
	int nDataSize = 0;

	CachedImpl * cachedImp = NULL;
	
	int nConnectID = CachedManager::getClass().GetACachedImpl(cachedImp);

	if (cachedImp == NULL)
	{
		return -3;
	}

	// 2. 获取缓存数据
	char szTime[256];
	memset(szTime, 0x00, sizeof(szTime));
	sprintf(szTime, CACHED_TIME_FORMAT, nYear, nMonth, nDay, nHour, nForecastTime, nTimeRange);

	char szDataType[25];
	memset(szDataType, 0x00, sizeof(szDataType));
	sprintf(szDataType, CLIENT_STATION, nClientID);

	nErr = cachedImp->GetStationData(strCityType.toLocal8Bit().data(), szDataType, nTimeRange, strStationNo.toLocal8Bit().data(), szTime, fData, nDataSize);
	if (nErr != SUCCESS || fData == NULL)
	{
		
		CachedManager::getClass().RestoreACachedImpl(nConnectID);
		return -4;
	}

	// 大城市的站点数据以原流程执行
	if (strCityType == "bigcity")
	{
		// 3、保存数据到SPCC中 - 最新的订正数据
		nErr = cachedImp->SaveStationData(strCityType.toLocal8Bit().data(), "SPCC", nTimeRange, strStationNo.toLocal8Bit().data(), szTime, fData, nDataSize);
		if (nErr != SUCCESS)
		{
			CachedManager::getClass().RestoreACachedImpl(nConnectID);
			free(fData);
			return -5;
		}

		// 4、将数据保存到数据库
		nErr = HandleDB::getClass().SaveStationData(nClientID, strCityType, "SPCC", strStationNo, nYear, nMonth, nDay, nHour, nMinute, nSecond, nForecastTime, nTimeRange, fData, nDataSize);
		//if (nErr != SUCCESS)
		//{
		//	free(fData);
		//	return -6;
		//}

		// 5、发布站点消息
		nErr = PublishStation("SPCC", strCityType, strStationNo, nYear, nMonth, nDay, nHour, nMinute, nSecond, nForecastTime, nTimeRange);
		if (nErr != SUCCESS)
		{
		// 发布站点消息失败
		}
	}
	else
	{
		QString strPubCityType, strPubForecastType;
		int nPubStationRange;
		bool bIsUpdate;
		nErr = DataManager::getClass().Station2Grid(strStationNo, nTimeRange, fData, nDataSize, nYear, nMonth, nDay, nHour, nForecastTime, strPubCityType, strPubForecastType, nPubStationRange, bIsUpdate);
		if (nErr != SUCCESS)
		{
			// 站点转格点失败
		}
		else
		{
			if (bIsUpdate)
			{
				nErr = PublishStation(strPubForecastType, strPubCityType, strStationNo, nYear, nMonth, nDay, nHour, nMinute, nSecond, nForecastTime, nPubStationRange);
				if (nErr != SUCCESS)
				{
					// 发布站点消息失败
				}
			}
		}
	}

	// 释放数据空间
	CachedManager::getClass().RestoreACachedImpl(nConnectID);
	free(fData);

	return SUCCESS;
}

/************************************************************************/
/* 处理客户端发送来的站点文件数据                                       */
/************************************************************************/
int NetworkManager::HandleStationFile(int nClientID, QString strCityType, int nTimeRange, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond)
{
	// 信息
	LOG_(LOGID_NETWORK, LOG_F("客户端[ClientID:%d]发送多个站点数据: %s,%d, %04d-%02d-%02d %02d:%02d:%02d"), nClientID, LOG_STR(strCityType), nTimeRange, nYear, nMonth, nDay, nHour, nMinute, nSecond);

	// 保存到文件
	// todo

	// 保存结果到数据库
	// todo

	// 调用处理任务
	ST_StationMsg stStationMsg;
	stStationMsg.nStationType = STATION_TYPE_FILE;
	stStationMsg.nClientID = nClientID;
	stStationMsg.strCityType = strCityType;
	stStationMsg.nTimeRange = nTimeRange;
	stStationMsg.nYear = nYear;
	stStationMsg.nMonth = nMonth;
	stStationMsg.nDay  = nDay;
	stStationMsg.nHour = nHour;
	stStationMsg.nMinute = nMinute;
	stStationMsg.nSecond = nSecond;
	stStationMsg.nForecastTime = 0;
	if (m_pTaskStationMsg)
	{
		// 添加消息到任务中
		m_pTaskStationMsg->AddMsg(stStationMsg);
		// 启动处理
		m_pTaskStationMsg->start();
	}

	return SUCCESS;
} 

/************************************************************************/
/* 处理客户端发送来的站点文件数据                                       */
/************************************************************************/
int NetworkManager::HandleMultiStationData(int nClientID, QString strDataType, int nTimeRange, int nEndForcast, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond)
{
	// 信息
	LOG_(LOGID_NETWORK, LOG_F("客户端[ClientID:%d]发送多站点数据: %s,%d, %04d-%02d-%02d %02d:%02d:%02d"), nClientID, LOG_STR(strDataType), nTimeRange, nYear, nMonth, nDay, nHour, nMinute, nSecond);

	// 保存结果到数据库
	// todo

	// 调用处理任务
	ST_StationMsg stStationMsg;
	stStationMsg.nStationType = STATION_TYPE_MULTI;
	stStationMsg.nClientID = nClientID;
	stStationMsg.strCityType = "";
	stStationMsg.strDataType = strDataType;
	stStationMsg.nTimeRange = nTimeRange;
	stStationMsg.nYear = nYear;
	stStationMsg.nMonth = nMonth;
	stStationMsg.nDay = nDay;
	stStationMsg.nHour = nHour;
	stStationMsg.nMinute = nMinute;
	stStationMsg.nSecond = nSecond;
	stStationMsg.nForecastTime = nEndForcast;
	if (m_pTaskStationMsg)
	{
		// 添加消息到任务中
		m_pTaskStationMsg->AddMsg(stStationMsg);
		// 启动处理
		m_pTaskStationMsg->start();
	}

	return SUCCESS;
}

/************************************************************************/
/* 格点到站点的插值处理                                                 */
/************************************************************************/
int NetworkManager::HandleGrid2Station(int nClientID, QString strProductKey, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nStartForecast, int nEndForecast)
{
	

	// 产品
	ST_PRODUCT stProduct;
	if (!m_hasProduct.contains(strProductKey))
	{
		// 产品Key错误，没有此产品
		return -1;
	}
	stProduct = m_hasProduct.value(strProductKey);

	
	int nErr = HandleDB::getClass().SaveClientDataInfo(nClientID, strProductKey, "", "G2S", nYear, nMonth, nDay, nHour, nMinute, nSecond, nStartForecast, nEndForecast, stProduct.nTimeRange, 
		stProduct.fLon1, stProduct.fLon2, stProduct.fLat1, stProduct.fLat2);
	if (nErr != SUCCESS)
	{
		// todo 添加数据库失败
		LOG_(LOGID_NETWORK, LOG_F("[ERROR][NetworkManager]客户端[ClientID:%d]接收信息保存数据库失败"), nClientID, nErr);
		return nErr;
	}
	else
	{
		
		if (!m_hasProductTask.contains(strProductKey))
		{
			
			LOG_(LOGID_NETWORK, LOG_F("[ERROR][NetworkManager]客户端[ClientID:%d]不允许该客户端对该产品（%s）进行合并."), nClientID, LOG_STR(strProductKey));
			return -3;
		}
		TaskBase * pTask = m_hasProductTask.value(strProductKey);
		if (pTask == NULL)
		{
			LOG_(LOGID_NETWORK, LOG_F("[ERROR][NetworkManager]客户端[ClientID:%d]没有找到相应处理任务（%s）."), nClientID, LOG_STR(strProductKey));
			// 任务错误
			return -4;
		}

		// 启动任务
		pTask->start();
	}

	return SUCCESS;
}

/************************************************************************/
/* 站点到格点的插值处理                                                 */
/************************************************************************/
int NetworkManager::HandleStation2Grid(int nClientID, QString strDataType, int nTimeRange, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nStartForecast, int nEndForecast)
{
	// 信息
	LOG_(LOGID_NETWORK, LOG_F("客户端[ClientID:%d]站点插值格点请求: %s,%d, %04d-%02d-%02d %02d:%02d:%02d"), nClientID, LOG_STR(strDataType), nTimeRange, nYear, nMonth, nDay, nHour, nMinute, nSecond);

	// 保存结果到数据库
	// todo

	// 调用处理任务
	ST_StationMsg stStationMsg;
	stStationMsg.nStationType = STATION_TYPE_MULTI_DIFF;
	stStationMsg.nClientID = nClientID;
	stStationMsg.strCityType = "";
	stStationMsg.strDataType = strDataType;
	stStationMsg.nTimeRange = nTimeRange;
	stStationMsg.nYear = nYear;
	stStationMsg.nMonth = nMonth;
	stStationMsg.nDay = nDay;
	stStationMsg.nHour = nHour;
	stStationMsg.nMinute = nMinute;
	stStationMsg.nSecond = nSecond;
	stStationMsg.nStartForecast = nStartForecast;
	stStationMsg.nForecastTime = nEndForecast;
	if (m_pTaskStationMsg)
	{
		// 添加消息到任务中
		m_pTaskStationMsg->AddMsg(stStationMsg);
		// 启动处理
		m_pTaskStationMsg->start();
	}

	return SUCCESS;
}

/************************************************************************/
/* 降水一致性处理                                                       */
/************************************************************************/
int NetworkManager::HandleRainProcess(int nClientID, QString strProductKey, QString strRelatedProductKey, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nStartForecast, int nEndForecast)
{
	// 查找原产品
	if (!m_hasProduct.contains(strProductKey))
	{
		// 没有原产品
		return -1;
	}
	ST_PRODUCT stProduct = m_hasProduct.value(strProductKey);

	
	
	int nErr = HandleDB::getClass().SaveClientDataInfo(nClientID, strProductKey, strRelatedProductKey, "RAINPROC", nYear, nMonth, nDay, nHour, nMinute, nSecond, nStartForecast, nEndForecast, stProduct.nTimeRange,
		stProduct.fLon1, stProduct.fLon2, stProduct.fLat1, stProduct.fLat2);
	if (nErr != SUCCESS)
	{
		// todo 添加数据库失败
		LOG_(LOGID_NETWORK, LOG_F("[ERROR][NetworkManager]客户端[ClientID:%d]接收信息保存数据库失败"), nClientID, nErr);
		return nErr;
	}
	else
	{

		if (!m_hasProductTask.contains(strProductKey))
		{

			LOG_(LOGID_NETWORK, LOG_F("[ERROR][NetworkManager]客户端[ClientID:%d]不允许该客户端对该产品（%s）进行合并."), nClientID, LOG_STR(strProductKey));
			return -3;
		}
		TaskBase * pTask = m_hasProductTask.value(strProductKey);
		if (pTask == NULL)
		{
			LOG_(LOGID_NETWORK, LOG_F("[ERROR][NetworkManager]客户端[ClientID:%d]没有找到相应处理任务（%s）."), nClientID, LOG_STR(strProductKey));
			// 任务错误
			return -4;
		}

		TaskProductClientMerge *pclientMerge = dynamic_cast<TaskProductClientMerge *>(pTask);

		if (pclientMerge != NULL)
		{
			// 启动任务
			//pclientMerge->InitProduct(stProduct, m_hasProduct);
			pclientMerge->start();
		}
		else
		{
			LOG_(LOGID_NETWORK, LOG_F("[ERROR][NetworkManager]客户端[ClientID:%d]没有找到相应处理任务（%s）."), nClientID, LOG_STR(strProductKey));
			return -4;
		}
		
	}

	return SUCCESS;
}

/************************************************************************/
/* 站点配置，添加站点                                                   */
/************************************************************************/
int NetworkManager::HandleAdd(const QString& stationNum, const QString& stationName, float lat,
	float lon, float height)
{
	HASH_STATION_INFO &hasStationInfo = DataManager::getClass().GetStationList();
	ST_STATION_INFO stationInfo;
	
	memset(&stationInfo, 0, sizeof(ST_STATION_INFO));
	
	memcpy(&stationInfo.stationNo, stationNum.toLocal8Bit().data(), stationNum.toLocal8Bit().size());
	memcpy(&stationInfo.stationName, stationName.toLocal8Bit().data(), stationName.toLocal8Bit().size());
	
	stationInfo.fHeight = height;
	stationInfo.fLat = lat;
	stationInfo.fLon = lon;
	
	QMutexLocker locker(&m_mutexStationCfg);
	
	if(hasStationInfo.contains(stationNum))
	{
		hasStationInfo[stationNum] = stationInfo;
	}
	else
	{
		hasStationInfo.insert(stationNum, stationInfo);
	}

	return SUCCESS;
}

/************************************************************************/
/* 站点配置，删除站点                                                   */
/************************************************************************/
int NetworkManager::HandleDelete(const QString& stationNum, const QString& stationName,float lat,
	float lon,float height)
{
	HASH_STATION_INFO &hasStationInfo = DataManager::getClass().GetStationList();

	QMutexLocker locker(&m_mutexStationCfg);
	
	if (hasStationInfo.contains(stationNum))
	{
		hasStationInfo.remove(stationNum);
	}

	return SUCCESS;
}

/************************************************************************/
/* 站点配置，修改站点                                                   */
/************************************************************************/
int NetworkManager::HandleModify(const QString& stationNum, const QString& stationName, const float lat,
	float lon, float height)
{
	HASH_STATION_INFO &hasStationInfo = DataManager::getClass().GetStationList();
	ST_STATION_INFO stationInfo;

	memset(&stationInfo, 0, sizeof(ST_STATION_INFO));
	
	memcpy(&stationInfo.stationNo, stationNum.toLocal8Bit().data(), stationNum.toLocal8Bit().size());
	memcpy(&stationInfo.stationName, stationName.toLocal8Bit().data(), stationName.toLocal8Bit().size());

	stationInfo.fHeight = height;
	stationInfo.fLat = lat;
	stationInfo.fLon = lon;

	
	QMutexLocker locker(&m_mutexStationCfg);
	
	if (hasStationInfo.contains(stationNum))
	{
		hasStationInfo[stationNum] = stationInfo;
	}
	else
	{
		hasStationInfo.insert(stationNum, stationInfo);
	}

	return SUCCESS;
}

/************************************************************************/
/* 站点配置                                                             */
/************************************************************************/
int NetworkManager::HandleUploadStationCfg(int nClientID)
{
	
	CachedImpl * cachedImp = NULL;

	int nConnectID = CachedManager::getClass().GetACachedImpl(cachedImp);

	if (cachedImp == NULL)
	{
		return -3;
	}
	
	char *stationCfgData = NULL;
	int cfgDataLen;
	
	int nErr = cachedImp->GetStationCfgData(stationCfgData, cfgDataLen, nClientID);

	if (nErr != SUCCESS || stationCfgData == NULL)
	{
		CachedManager::getClass().RestoreACachedImpl(nConnectID);
		return -4;
	}

	CachedManager::getClass().RestoreACachedImpl(nConnectID);


	//解析客户端站点配置数据格式
	QString strBuf = QString::fromLocal8Bit(stationCfgData);//[544123(type,name,lon,lat,height); 544123(type,name,lon,lat,height);...]
	int nPosStart = strBuf.indexOf("[");
	int nPosEnd = strBuf.indexOf("]");

	QString strData = strBuf.mid(nPosStart + 1, nPosEnd - nPosStart - 1);
	QStringList lstData = strData.split(";");
	int nStationCnt = lstData.size();
	
	QStringList::iterator iter;
	
	for (iter = lstData.begin(); iter != lstData.end(); iter++)
	{
		QString strTmp = *iter; // stationNumber(optype,name, latitude,longitude,height)  optype: 'M' 'A' 'D' 对应改，增，删
		
		if (strTmp.isEmpty())
		{
			break;
		}
		
		int nDataPosStart = strTmp.indexOf("(");
		int nDataPosEnd = strTmp.indexOf(")");

		// 站号
		QString strStationNo = strTmp.mid(0, nDataPosStart);
		QString strValueBuf = strTmp.mid(nDataPosStart + 1, nDataPosEnd - nDataPosStart - 1);
		QStringList lstValue = strValueBuf.split(",");

		QString opType = lstValue[0]; //得到操作type
		QString stationName = lstValue[1];
		
		float latitude = lstValue[2].toFloat();
		float longitude = lstValue[3].toFloat();
		float height = lstValue[4].toFloat();
		
		if (opType == "A")
		{
			HandleAdd(strStationNo, stationName, latitude, longitude, height);
		}
		else if (opType == "D")
		{
			HandleDelete(strStationNo, stationName, latitude, longitude, height);
		}
		else if (opType == "M")
		{
			HandleModify(strStationNo, stationName, latitude, longitude, height);
		}
		else
		{
			LOG_(LOGID_NETWORK, LOG_F("客户端[ClientID:%d]发送上传站点配置的未知操作码。"), nClientID);
		}

	}

	//把更改的数据刷新同步至缓存并同步写入文件
	DataManager::getClass().flushStationInfoToMemcached();
	DataManager::getClass().flushStationInfoToLocal();
	
	
	
	// 释放数据空间
	free(stationCfgData);
	
	return SUCCESS;
}

/************************************************************************/
/* 初始化缓存数据                                                       */
/************************************************************************/
bool NetworkManager::InitCached()
{
	int nErr = CachedManager::getClass().Init(m_stCached, 8);

	if (nErr != SUCCESS)
	{
		return false;
	}

	return true;
}

/************************************************************************/
/* 存储客户端上传数据                                                   */
/************************************************************************/
int NetworkManager::SaveClientData(int nClientID, QString strProductKey, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond,
	int nForecastTime, int nTimeRange, float fLon1, float fLon2, float fLat1, float fLat2)
{
	// 1. 检查存储产品Key
	if (!m_hasProduct.contains(strProductKey))
	{
		return -1;
	}
	// 2. 获取产品信息
	ST_PRODUCT stProduct = m_hasProduct[strProductKey];

	// 3. 检查客户端配置
	if (!m_hasClient.contains(nClientID))
	{
		return -2;
	}

	// 4. 获取客户端配置
	ST_CLIENT_INFO stClientInfo = m_hasClient[nClientID];

	CachedImpl * cachedImp = NULL;
	
	int nConnectID = CachedManager::getClass().GetACachedImpl(cachedImp);

	if (cachedImp == NULL)
	{
		return -3;
	}

	// 4. 获取缓存数据
	int nDataSize;
	float fLon1tmp, fLon2tmp, fLat1tmp, fLat2tmp;
	float * fData = NULL;
	char szTime[256];
	memset(szTime, 0x00, sizeof(szTime));
	sprintf(szTime, CACHED_TIME_FORMAT, nYear, nMonth, nDay, nHour, nForecastTime, nTimeRange);
	int nErr = cachedImp->GetClientData(nClientID, strProductKey.toLocal8Bit().data(), szTime, fData, nDataSize, fLon1tmp, fLon2tmp, fLat1tmp, fLat2tmp);
	if (nErr != SUCCESS || fData == NULL)
	{
		CachedManager::getClass().RestoreACachedImpl(nConnectID);
		return -4;
	}

	CachedManager::getClass().RestoreACachedImpl(nConnectID);

	// 5. 保存数据到grib2文件中
	nErr = HandleGrib::getClass().SaveData2GribFile(fData, 1, stProduct.nCategory, stProduct.nElement[0], stProduct.nStatistical, stProduct.nStatus,
		fLon1, fLon2, fLat1, fLat2, stProduct.fDi, stProduct.fDj, nYear, nMonth, nDay, nHour, nForecastTime, nTimeRange, nClientID, strProductKey, stProduct.strType, stClientInfo.strGridSavePath, stClientInfo.strGridSaveFile);

	// 6. 释放data空间
	free(fData);

	// 7. 返回结果
	return nErr;
}

/************************************************************************/
/* 发布拼图结果                                                         */
/************************************************************************/
int NetworkManager::PublishGrid(QString strProductKey, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nForecastTime, int nTimeRange)
{
	if (!m_bIsPublish)
	{
		LOG_(LOGID_DEBUG, LOG_F("发布关闭"));
		return -1;
	}

	// TCP Server 设置失败
	if (m_pTaskTcpServer == NULL)
	{
		return -1;
	}

	LOG_(LOGID_DEBUG, LOG_F("发布格点订阅消息,Key=%s,Time=%04d-%02d-%02d %02d:%02d-%02d-%02d"), LOG_STR(strProductKey),  nYear, nMonth, nDay, nHour, nMinute, nSecond, nForecastTime, nTimeRange);

	// 创建消息
	char szSendMsg[1024];
	memset(szSendMsg, 0x00, sizeof(szSendMsg));

	int nDataLen = strProductKey.length(); // Key长度

	int nOff = 0;
	// 信息头
	szSendMsg[nOff++] = STD_SOI;        // 起始标志位
	szSendMsg[nOff++] = MSG_VERSION;    // 版本号
	TxI4(BROADCAST_ADR, (unsigned char *)szSendMsg, nOff); // 客户端标识
	szSendMsg[nOff++] = CMD_MERGE_UPDATE; // 命令码：发送拼图更新数据
	szSendMsg[nOff++] = 0;                // 数据包序号
	szSendMsg[nOff++] = TYPE_FORMAT_6;    // 数据格式类别:第6类数据
	TxI4(TYPE_FORMAT_6_BASELENGHTH + nDataLen, (unsigned char *)szSendMsg, nOff);// 数据内容长度

	// 信息内容
	TxI2(nYear, (unsigned char *)szSendMsg, nOff); // 年
	szSendMsg[nOff++] = nMonth;
	szSendMsg[nOff++] = nDay;
	szSendMsg[nOff++] = nHour;
	szSendMsg[nOff++] = nMinute;
	szSendMsg[nOff++] = nSecond;
	szSendMsg[nOff++] = nForecastTime;
	szSendMsg[nOff++] = nTimeRange;
	TxI4(nDataLen, (unsigned char *)szSendMsg, nOff);// Key长度
	memcpy(szSendMsg + nOff, strProductKey.toLocal8Bit().data(), nDataLen);  // Key
	nOff += nDataLen;

	// 信息尾
	szSendMsg[nOff++] = STD_EOI;       // 起始标志位

	// 广播数据
	m_pTaskTcpServer->Broadcast(szSendMsg, nOff);

	return SUCCESS;
}

/************************************************************************/
/* 发布站点消息                                                         */
/************************************************************************/
int NetworkManager::PublishStation(QString strDataType, QString strCityType, QString strStationNo, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nForecastTime, int nTimeRange)
{
	if (!m_bIsPublish)
	{
		LOG_(LOGID_DEBUG, LOG_F("发布关闭"));
		return -1;
	}

	// TCP Server 设置失败
	if (m_pTaskTcpServer == NULL)
	{
		return -1;
	}

	// 创建消息
	char szSendMsg[1024];
	memset(szSendMsg, 0x00, sizeof(szSendMsg));

	int nOff = 0;
	// 信息头
	szSendMsg[nOff++] = STD_SOI;        // 起始标志位
	szSendMsg[nOff++] = MSG_VERSION;    // 版本号
	TxI4(BROADCAST_ADR, (unsigned char *)szSendMsg, nOff); // 客户端标识
	szSendMsg[nOff++] = CMD_STATION_UPDATE;// 命令码：发送站点更新数据
	szSendMsg[nOff++] = 0;                // 数据包序号
	szSendMsg[nOff++] = TYPE_FORMAT_7;    // 数据格式类别:第6类数据
	TxI4(TYPE_FORMAT_7_BASELENGHTH, (unsigned char *)szSendMsg, nOff);// 数据内容长度

	// 信息内容
	TxI2(nYear, (unsigned char *)szSendMsg, nOff); // 年
	szSendMsg[nOff++] = nMonth;
	szSendMsg[nOff++] = nDay;
	szSendMsg[nOff++] = nHour;
	szSendMsg[nOff++] = nMinute;
	szSendMsg[nOff++] = nSecond;
	szSendMsg[nOff++] = nForecastTime;
	szSendMsg[nOff++] = nTimeRange;

	memcpy(szSendMsg + nOff, strDataType.toLocal8Bit().data(), strDataType.length());  // 数据类型
	nOff += 10;
	memcpy(szSendMsg + nOff, strCityType.toLocal8Bit().data(), strCityType.length());  // 城市类型
	nOff += 10;
	memcpy(szSendMsg + nOff, strStationNo.toLocal8Bit().data(), strStationNo.length()); // 站号
	nOff += 10;

	// 信息尾
	szSendMsg[nOff++] = STD_EOI;       // 起始标志位

	// 广播数据
	m_pTaskTcpServer->Broadcast(szSendMsg, nOff);

	return SUCCESS;
}

/************************************************************************/
/* 发布多站点消息                                                       */
/************************************************************************/
int NetworkManager::PublishMultiStation(QString strDataType, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nTimeRange)
{
	if (!m_bIsPublish)
	{
		LOG_(LOGID_DEBUG, LOG_F("发布关闭"));
		return -1;
	}

	// TCP Server 设置失败
	if (m_pTaskTcpServer == NULL)
	{
		return -1;
	}

	// 创建消息
	char szSendMsg[1024];
	memset(szSendMsg, 0x00, sizeof(szSendMsg));

	int nOff = 0;
	// 信息头
	szSendMsg[nOff++] = STD_SOI;        // 起始标志位
	szSendMsg[nOff++] = MSG_VERSION;    // 版本号
	TxI4(BROADCAST_ADR, (unsigned char *)szSendMsg, nOff); // 客户端标识
	szSendMsg[nOff++] = CMD_MULTI_STATION_UPDATE;// 命令码：发送站点更新数据
	szSendMsg[nOff++] = 0;                // 数据包序号
	szSendMsg[nOff++] = TYPE_FORMAT_7;    // 数据格式类别:第6类数据
	TxI4(18, (unsigned char *)szSendMsg, nOff);// 数据内容长度

	// 信息内容
	TxI2(nYear, (unsigned char *)szSendMsg, nOff); // 年
	szSendMsg[nOff++] = nMonth;
	szSendMsg[nOff++] = nDay;
	szSendMsg[nOff++] = nHour;
	szSendMsg[nOff++] = nMinute;
	szSendMsg[nOff++] = nSecond;
	szSendMsg[nOff++] = nTimeRange;

	memcpy(szSendMsg + nOff, strDataType.toLocal8Bit().data(), strDataType.length());  // 数据类型
	nOff += 10;

	// 信息尾
	szSendMsg[nOff++] = STD_EOI;       // 起始标志位

	// 广播数据
	m_pTaskTcpServer->Broadcast(szSendMsg, nOff);

	return SUCCESS;
}


/************************************************************************/
/* 降水向下拆分处理                                                     */
/************************************************************************/
bool NetworkManager::HandleRainCorrection_Downward(CachedImpl * pCachedImpl, ST_PRODUCT stProduct, ST_PRODUCT stRelatedProduct, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nForecastTime)
{
	// 该方法是通过24降水对3小时指导报的数据进行调整
	// 假定24降水的格点数,经纬度范围与3小时的是一致的

	// 获取原降水数据
	int nErr;
	float *fData = NULL;
	int nDataLen = 0;
	char szTime[50];
	memset(szTime, 0x00, sizeof(szTime));
	sprintf(szTime, CACHED_TIME_FORMAT, nYear, nMonth, nDay, nHour, nForecastTime, stProduct.nTimeRange);

	nErr = pCachedImpl->GetMergeData(stProduct.strKey.toLocal8Bit().data(), szTime, fData, nDataLen);
	if (nErr != SUCCESS || fData == NULL)
	{
		// 获取原降水数据失败
		return false;
	}

	// 目标降水处理
	// 处理数据的个数，如24到3小时降水，就是8个
	int nTimeRange = stRelatedProduct.nTimeRange; // 目标降水的时间间隔
	int nCnt = stProduct.nTimeRange / nTimeRange;
	int forecast = nTimeRange;

	//float * fDataRain[8];  // 3小时降水1天的数据
	float ** fDataRain = (float **)malloc(nCnt * sizeof(float *));
	if (fDataRain == NULL)
	{
		// 空间申请失败
		return false;
	}

	bool bFlg = true;

	// 获取3小时降水数据
	int n = 0;
	for (n = 0; n < nCnt; n++)
	{
		fDataRain[n] = NULL;

		// 获取数据
		forecast = (nForecastTime - stProduct.nTimeRange) + n * nTimeRange + nTimeRange;
		memset(szTime, 0x00, sizeof(szTime));
		sprintf(szTime, CACHED_TIME_FORMAT, nYear, nMonth, nDay, nHour, forecast, nTimeRange);
		nErr = pCachedImpl->GetProductData(stRelatedProduct.strKey.toLocal8Bit().data(), szTime, fDataRain[n], nDataLen);
		if (nErr != SUCCESS || fDataRain[n] == NULL)
		{
			fDataRain[n] = (float *)malloc(stProduct.nNi * stProduct.nNj * sizeof(float));
			if (fDataRain[n] == NULL)
			{
				bFlg = false;
				break;
			}
			// 未取得数据，构造一个空数据
			for (int i = 0; i < stProduct.nNi * stProduct.nNj; i++)
			{
				fDataRain[n][i] = 0;
			}
		}
	}
	// 判断获取数据的情况
	if (!bFlg)
	{
		// 未取得数据成功
		// 释放数据空间
		for (int i = 0; i < n; i++)
		{
			if (fDataRain[i])
			{
				free(fDataRain[i]);
				fDataRain[i] = NULL;
			}
		}
		if (fDataRain)
		{
			free(fDataRain);
			fDataRain = NULL;
		}

		return false;
	}

	// 调整数据
	float fRainSum = 0;
	float fRainVal = 0;
	for (int i = 0; i < stProduct.nNi * stProduct.nNj; i++)
	{
		// 如果日降水量小于0.8，则不进行调整
		if (fData[i] < 0.8)
		{
			continue;
		}
		else if (fData[i] > 9998)
		{
			// 缺测值，特殊处理
			for (n = 0; n < nCnt; n++)
			{
				fDataRain[n][i] = fData[i];
			}

			continue;
		}

		// 否则，累加1日内的3小时指导报
		fRainSum = 0;
		for (n = 0; n < nCnt; n++)
		{
			fRainSum += fDataRain[n][i];
		}

		// 如果累计量=0
		if (fRainSum > -0.000001 && fRainSum < 0.000001)
		{
			// 平均分布降水量
			fRainVal = fData[i] / nCnt;
			for (n = 0; n < nCnt; n++)
			{
				fDataRain[n][i] = fRainVal;
			}
		}
		else
		{
			// 按比例调整降水量
			for (n = 0; n < nCnt; n++)
			{
				fDataRain[n][i] *= fData[i] / fRainSum;
			}
		}
	}

	// 处理结束，将3小时降水保存到缓存中
	for (n = 0; n < nCnt; n++)
	{
		// 时间
		forecast = (nForecastTime - stProduct.nTimeRange) + n * nTimeRange + nTimeRange;
		memset(szTime, 0x00, sizeof(szTime));
		sprintf(szTime, CACHED_TIME_FORMAT, nYear, nMonth, nDay, nHour, forecast, nTimeRange);

		// 数据长度
		nErr = pCachedImpl->SaveMergeData(stRelatedProduct.strKey.toLocal8Bit().data(), szTime, fDataRain[n], stProduct.nNi * stProduct.nNj);
		if (nErr == SUCCESS)
		{
			// 存储成功
			LOG_(LOGID_DEBUG, LOG_F("[%s]数据缓存存储成功：（ProductTime_%s）."), LOG_STR(stRelatedProduct.strKey), LOG_STR(QString::fromLocal8Bit(szTime)));

			// 缓存保存成功，向各个客户端进行广播，发布3小时降水拼图处理消息
			// ############# todo ##############
			NetworkManager::getClass().PublishGrid(stRelatedProduct.strKey, nYear, nMonth, nDay, nHour, 0, 0, forecast, nTimeRange);
		}
		else
		{
			// 存储失败
			LOG_(LOGID_DEBUG, LOG_F("[WARNING][%s]数据缓存存储失败：（ProductTime_%s）."), LOG_STR(stRelatedProduct.strKey), LOG_STR(QString::fromLocal8Bit(szTime)));
		}
	}

	// 处理结束，释放3小时降水内存空间
	for (int i = 0; i < nCnt; i++)
	{
		if (fDataRain[i])
		{
			free(fDataRain[i]);
			fDataRain[i] = NULL;
		}
	}
	if (fDataRain)
	{
		free(fDataRain);
		fDataRain = NULL;
	}
	if (fData)
	{
		free(fData);
		fData = NULL;
	}

	return true;
}

/************************************************************************/
/* 降水向上合并处理                                                     */
/************************************************************************/
bool NetworkManager::HandleRainCorrection_Upward(CachedImpl * pCachedImpl, ST_PRODUCT stProduct, ST_PRODUCT stRelatedProduct, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nForecastTime)
{
	// 获取原降水数据
	int nErr;

	// 处理数据的个数，如3到24小时降水，就是8个
	int nTimeRange = stProduct.nTimeRange;
	int nCnt = stRelatedProduct.nTimeRange / nTimeRange;
	int forecast = stProduct.nTimeRange;

	// 保存当前处理Key的元数据
	float ** fDataRain = (float **)malloc(nCnt * sizeof(float *));
	if (fDataRain == NULL)
	{
		// 空间申请失败
		return false;
	}

	// 保存合并数据
	float * fUpDataRain = (float *)malloc(stProduct.nNi * stProduct.nNj * sizeof(float));
	if (fUpDataRain == NULL)
	{
		free(fDataRain);
		// 空间申请失败
		return false;
	}

	char szTime[50];
	bool bFlg = true;

	// 获取3小时降水数据
	int nBaseForecast = (nForecastTime / nTimeRange) * nTimeRange;
	int n = 0;
	for (n = 0; n < nCnt; n++)
	{
		fDataRain[n] = NULL;

		// 获取数据
		forecast = nBaseForecast + (n + 1) * nTimeRange;
		nwfd_grib2_field * field = NULL;
		int nDataCnt;
		
		// 获取拼图数据
		memset(szTime, 0x00, sizeof(szTime));
		sprintf(szTime, CACHED_TIME_FORMAT, nYear, nMonth, nDay, nHour, forecast, nTimeRange);

		nErr = pCachedImpl->GetMergeData(stProduct.strKey.toLocal8Bit().data(), szTime, fDataRain[n], nDataCnt);
		if (nErr != SUCCESS || fDataRain[n] == NULL)
		{
			// 获取原降水拼图数据失败
			continue;
		}

		if (field)
		{
			free(field);
		}
	}

	// 累加数据
	float fRainSum;
	for (int i = 0; i < stProduct.nNi * stProduct.nNj; i++)
	{
		// 累加1日内的3小时指导报
		fRainSum = -1;
		for (n = 0; n < nCnt; n++)
		{
			if (fDataRain[n] == NULL)
			{
				// 空指针
				continue;
			}
			if (IS_EQUAL(fDataRain[n][i] , 9999.0))
			{
				// 缺测值
				continue;
			}

			if (fRainSum < 0)
			{
				fRainSum = fDataRain[n][i];
			}
			else
			{
				fRainSum += fDataRain[n][i];
			}
		}

		// 累加值合并
		if (fRainSum < 0)
		{
			fUpDataRain[i] = 9999.0;  // 缺测
		}
		else
		{
			fUpDataRain[i] = fRainSum; // 累加值
		}
	}

	// 处理结束，将累加后的降水保存到缓存中
	// 时间
	forecast = nBaseForecast + nCnt * nTimeRange;
	memset(szTime, 0x00, sizeof(szTime));
	sprintf(szTime, CACHED_TIME_FORMAT, nYear, nMonth, nDay, nHour, forecast, stRelatedProduct.nTimeRange);

	// 数据长度
	nErr = pCachedImpl->SaveMergeData(stRelatedProduct.strKey.toLocal8Bit().data(), szTime, fDataRain[n], stProduct.nNi * stProduct.nNj);
	if (nErr == SUCCESS)
	{
		// 存储成功
		LOG_(LOGID_DEBUG, LOG_F("[%s]数据缓存存储成功：（ProductTime_%s）."), LOG_STR(stRelatedProduct.strKey), LOG_STR(QString::fromLocal8Bit(szTime)));

		// 缓存保存成功，向各个客户端进行广播，发布24小时降水拼图处理消息
		// ############# todo ##############
		NetworkManager::getClass().PublishGrid(stRelatedProduct.strKey, nYear, nMonth, nDay, nHour, 0, 0, forecast, stRelatedProduct.nTimeRange);
	}
	else
	{
		// 存储失败
		LOG_(LOGID_DEBUG, LOG_F("[WARNING][%s]数据缓存存储失败：（ProductTime_%s）."), LOG_STR(stRelatedProduct.strKey), LOG_STR(QString::fromLocal8Bit(szTime)));
	}

	// 处理结束，释放3小时降水内存空间
	for (int i = 0; i < nCnt; i++)
	{
		if (fDataRain[i])
		{
			free(fDataRain[i]);
			fDataRain[i] = NULL;
		}
	}
	if (fDataRain)
	{
		free(fDataRain);
		fDataRain = NULL;
	}

	return true;
}


