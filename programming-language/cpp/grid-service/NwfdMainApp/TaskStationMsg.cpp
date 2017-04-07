#include "log.h" // log日志
#include "TaskStationMsg.h"
#include "Common.h"
#include "HandleCommon.h"
#include "HandleStationLib.h"
#include "DDSManager.h"
#include "NwfdErrorCode.h"
#include "NetworkManager.h"
#include "DataManager.h"
#include <QDir>
#include <QDateTime>
#include <QFileInfoList>

/************************************************************************/
/* 构造函数&析构函数                                                    */
/************************************************************************/
TaskStationMsg::TaskStationMsg()
{
	LOG_(LOGID_DEBUG, LOG_F("Entry TaskStationMsg()"));
	// 初始化状态为僵死，不初始化不予执行任务
	m_strTaskName = "站点实时处理任务";
	m_emTaskState = _TASK_DEAD;
	m_pCachedImpl = NULL;
	LOG_(LOGID_DEBUG, LOG_F("Leave TaskStationMsg()"));
}

TaskStationMsg::~TaskStationMsg()
{
	LOG_(LOGID_DEBUG, LOG_F("Entry ~TaskStationMsg()"));
	// 停止线程执行
	m_bReady = false;
	wait();
	LOG_(LOGID_DEBUG, LOG_F("Leave ~TaskStationMsg()"));
}

/************************************************************************/
/* 任务处理执行方法                                                     */
/************************************************************************/
void TaskStationMsg::run()
{
	if (m_emTaskState != _TASK_READ)
	{
		// 如果任务没有在准备状态，不予执行此次任务
		LOG_(LOGID_INFO, LOG_F("[Station]任务没有完成准备工作，不能执行，任务结束（当前状态为：%d）."),(int)m_emTaskState);
		return;
	}

	m_bReady = true;

	while (m_bReady)
	{
		// 任务状态机状态转换机制
		switch (m_emTaskState)
		{
		case _TASK_READ:    // 任务执行准备中
		{
			m_emTaskState = _TASK_RUNNING;
			break;
		}
		case _TASK_RUNNING: // 任务执行中
		{
			// 执行任务
			ExecTask();

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
			m_bReady = false;  // 结束线程执行
			break;

		default:
			break;
		}
	}
}

/************************************************************************/
/* 设置所管理的产品                                                     */
/************************************************************************/
void TaskStationMsg::Init(ST_CACHED stCached)
{
	m_stCached = stCached;

	// 初始化缓存
	if (InitCached())
	{
		LOG_(LOGID_INFO, LOG_F("[TaskPID:%d][Station]初始化缓存成功）."), m_nTaskPID);
	}
	else
	{
		LOG_(LOGID_INFO, LOG_F("[TaskPID:%d][Station]初始化缓存失败"), m_nTaskPID);
	}

	// 任务准备完毕，等待执行
	m_emTaskState = _TASK_READ;

	LOG_(LOGID_INFO, LOG_F("初始化完成，任务做好执行准备..."));
}

/************************************************************************/
/* 初始化缓存数据                                                       */
/************************************************************************/
bool TaskStationMsg::InitCached()
{
	// 建立缓存连接
	if (m_pCachedImpl == NULL)
	{
		m_pCachedImpl = new CachedImpl(m_stCached);
	}

	if (m_pCachedImpl == NULL)
	{
		// 创建失败
		LOG_(LOGID_ERROR, LOG_F("[TaskPID:%d][Station]Cached连接创建失败"), m_nTaskPID);
		LOG_(LOGID_INFO, LOG_F("[TaskPID:%d][Error]Cached连接创建失败"), m_nTaskPID);

		return false;
	}

	// 测试连接状态
	int nErr = m_pCachedImpl->ConnectStatus();
	QString strConnectStatus = (nErr == SUCCESS) ? QString("连接") : QString("断开");
	LOG_(LOGID_INFO, LOG_F("[TaskPID:%d][Station] Cached连接状态: %s"), m_nTaskPID, LOG_STR(strConnectStatus));

	if (nErr != SUCCESS)
	{
		return false;
	}

	return true;
}

/************************************************************************/
/* 添加消息到列表中                                                     */
/************************************************************************/
void TaskStationMsg::AddMsg(ST_StationMsg stStationMsg)
{
	QMutexLocker locker(&m_mutexMsg);
	m_lstStationMsg.push_back(stStationMsg);
}

/************************************************************************/
/* 执行产品扫描处理任务                                                 */
/************************************************************************/
bool TaskStationMsg::ExecTask()
{
	// 执行扫描处理
	Process();

	return true;
}

/************************************************************************/
/* 执行扫描处理                                                         */
/************************************************************************/
bool TaskStationMsg::Process()
{
	LIST_STATION_MSG::iterator iter;
	while (m_lstStationMsg.size() != 0)
	{
		// 获取队列中的第一条消息
		ST_StationMsg stStationMsg = m_lstStationMsg.first();
		
		if (stStationMsg.nStationType == STATION_TYPE_SINGLE)
		{
			// 处理单站点消息
			ProcessStation(stStationMsg);
		}
		else if (stStationMsg.nStationType == STATION_TYPE_FILE) 
		{
			// 处理站点文件消息
			ProcessStationFile(stStationMsg);
		}
		else if (stStationMsg.nStationType == STATION_TYPE_MULTI)
		{
			// 处理多站点数据
			ProcessMultiStation(stStationMsg);
		}
		else if (stStationMsg.nStationType == STATION_TYPE_MULTI_DIFF)
		{
			// 处理多站点数据
			ProcessMultiDiffStation(stStationMsg);
		}
		else // 其他类型
		{
			// 未知类型，不处理
		}

		m_mutexMsg.lock();
		// 从列表中移除第一个
		m_lstStationMsg.pop_front();
		m_mutexMsg.unlock();

		// 延时处理
		msleep(50);
	}

	return true;
}

/************************************************************************/
/* 处理站点文件消息                                                     */
/************************************************************************/
int TaskStationMsg::ProcessStationFile(ST_StationMsg stStationMsg)
{
	// 获取站点文件数据
	if (m_pCachedImpl == NULL)
	{
		InitCached();
	}
	// 缓存连接不正常，重连
	if (m_pCachedImpl->ConnectStatus() != SUCCESS)
	{
		m_pCachedImpl->ReConnect();
	}

	// 获取站点文件的缓存数据
	char szDataType[25];
	memset(szDataType, 0x00, sizeof(szDataType));
	sprintf(szDataType, CLIENT_STATION, stStationMsg.nClientID);

	char szTime[256];
	memset(szTime, 0x00, sizeof(szTime));
	sprintf(szTime, CACHED_TIME_FORMAT, stStationMsg.nYear, stStationMsg.nMonth, stStationMsg.nDay, stStationMsg.nHour, 0, stStationMsg.nTimeRange);

	char * pFileData = NULL;
	int nDataLen = 0;
	int nErr = m_pCachedImpl->GetStationFileData(stStationMsg.strCityType.toLocal8Bit().data(), szDataType, stStationMsg.nTimeRange, szTime, pFileData, nDataLen);
	if (nErr != SUCCESS || pFileData == NULL)
	{
		// 获取数据失败
		return -1;
	}

	try
	{
		// 数据解析
		string strFileBuf = string(pFileData);
		string strData;
		string::size_type nOff = 0;  // 字符串游标位置
		string separator = " \r\n";

		string strStationNo;
		float fLon, fLat, fHeight;
		int nRow, nCol;
		float fData[100];
		int nForecastTime;
		string strStationList = "";
		int nCnt = 0;

		// 获取数据
		strData = getdata(strFileBuf, separator, nOff);
		if (strData == "")
		{
			// 容错处理：剔除文件中第一个空格
			getdata(strFileBuf, separator, nOff);
		}

		// 格式判断
		if (strData != "ZCZC")
		{
			return 60;
		}
		getdata(strFileBuf, separator, nOff);
		getdata(strFileBuf, separator, nOff);
		getdata(strFileBuf, separator, nOff);
		getdata(strFileBuf, separator, nOff);
		getdata(strFileBuf, separator, nOff);
		getdata(strFileBuf, separator, nOff);
		//if (stStationMsg.strCityType == QString("bigcity"))  // 大城市
		if (stStationMsg.strCityType == "bigcity")
		{
			getdata(strFileBuf, separator, nOff);
			getdata(strFileBuf, separator, nOff);
			getdata(strFileBuf, separator, nOff);
			getdata(strFileBuf, separator, nOff);
			getdata(strFileBuf, separator, nOff);
			getdata(strFileBuf, separator, nOff);
			getdata(strFileBuf, separator, nOff);
		}

		// 站点个数
		strData = getdata(strFileBuf, separator, nOff);
		int nStationCnt = atoi(strData.data());
		if (nStationCnt <= 0 || nStationCnt > 9999999)
		{
			return 60;
		}

		// 读取数据
		for (int n = 0; n < nStationCnt; n++)
		{
			// 站号
			strStationNo = getdata(strFileBuf, separator, nOff);
			if (strStationNo == "")
			{
				return 60;
			}

			// 经度
			strData = getdata(strFileBuf, separator, nOff);
			fLon = (float)atof(strData.data());

			// 纬度
			strData = getdata(strFileBuf, separator, nOff);
			fLat = (float)atof(strData.data());

			// 高度
			strData = getdata(strFileBuf, separator, nOff);
			fHeight = (float)atof(strData.data());

			// 行数
			strData = getdata(strFileBuf, separator, nOff);
			nRow = atoi(strData.data());

			// 列数
			if (stStationMsg.strCityType == "bigcity")  // 大城市
			{
				nCol = 6; // 大城市固定列
			}
			else  // 城镇
			{
				strData = getdata(strFileBuf, separator, nOff);
				nCol = atoi(strData.data()); // 未包含首列是forecasttime
			}

			// 保存信息到缓存中
			nErr = m_pCachedImpl->SaveStationInfo(stStationMsg.strCityType.toLocal8Bit().data(), "SPCC", stStationMsg.nTimeRange, strStationNo.data(), fLon, fLat, fHeight, nCol);
			if (nErr != SUCCESS)
			{
				// 保存失败
				// todo
			}

			// 数据
			for (int i = 0; i < nRow; i++)
			{
				// 预报时间
				strData = getdata(strFileBuf, separator, nOff);
				nForecastTime = (int)atoi(strData.data());

				// 数据
				int nDataOff = 0;
				for (int j = 0; j < nCol; j++)
				{
					strData = getdata(strFileBuf, separator, nOff);
					fData[nDataOff++] = (float)atof(strData.data());
				}

				// 保存数据到缓存中
				memset(szTime, 0x00, sizeof(szTime));
				sprintf(szTime, CACHED_TIME_FORMAT, stStationMsg.nYear, stStationMsg.nMonth, stStationMsg.nDay, stStationMsg.nHour, nForecastTime, stStationMsg.nTimeRange);
				nErr = m_pCachedImpl->SaveStationData(stStationMsg.strCityType.toLocal8Bit().data(), "SPCC", stStationMsg.nTimeRange, strStationNo.data(), szTime, fData, nDataOff);
				if (nErr == SUCCESS)
				{
					// 发布消息
					NetworkManager::getClass().PublishStation("SPCC", stStationMsg.strCityType, strStationNo.data(), stStationMsg.nYear, stStationMsg.nMonth, stStationMsg.nDay, stStationMsg.nHour, stStationMsg.nMinute, stStationMsg.nSecond, nForecastTime, stStationMsg.nTimeRange);
				}
			}

			nCnt++;
		}


		// 释放空间
		if (pFileData)
		{
			free(pFileData);
			pFileData = NULL;
		}
	}
	catch (...)
	{

	}

	return SUCCESS;
}

/************************************************************************/
/* 处理单站点消息                                                       */
/************************************************************************/
int TaskStationMsg::ProcessStation(ST_StationMsg stStationMsg)
{
	return SUCCESS;
}


///************************************************************************/
///* 处理多站点消息                                                       */
///************************************************************************/
//int TaskStationMsg::ProcessMultiStation(ST_StationMsg stStationMsg)
//{
//	// 获取站点文件数据
//	if (m_pCachedImpl == NULL)
//	{
//		InitCached();
//	}
//	// 缓存连接不正常，重连
//	if (m_pCachedImpl->ConnectStatus() != SUCCESS)
//	{
//		m_pCachedImpl->ReConnect();
//	}
//
//	// 类型判断
//	int nPos = 0;
//	if (stStationMsg.strDataType == "TMP") // 温度 ：泛指 3小时间隔温度
//	{
//		nPos = 0;
//	}
//	else if (stStationMsg.strDataType == "ER24" || stStationMsg.strDataType == "ER12" || stStationMsg.strDataType == "ER06" || stStationMsg.strDataType == "ER03") // 降水
//	{
//		nPos = 5;
//	}
//	else if (stStationMsg.strDataType == "TMX") // 最高温度
//	{
//		nPos = 10;
//	}
//	else if (stStationMsg.strDataType == "TMI") // 最低温度
//	{
//		nPos = 11;
//	}
//	else
//	{
//		return -1;
//	}
//
//	// 获取站点文件的缓存数据
//	char szDataType[25];
//	memset(szDataType, 0x00, sizeof(szDataType));
//	sprintf(szDataType, CLIENT_STATION_TYPE, stStationMsg.nClientID, stStationMsg.strDataType.toLocal8Bit().data());
//
//	// 时间
//	char szTime[256];
//	memset(szTime, 0x00, sizeof(szTime));
//	sprintf(szTime, CACHED_TIME_FORMAT, stStationMsg.nYear, stStationMsg.nMonth, stStationMsg.nDay, stStationMsg.nHour, 0, stStationMsg.nTimeRange);
//
//	// 数据
//	char * pData = NULL;
//	int nDataLen = 0;
//	int nErr = m_pCachedImpl->GetStationFileData(stStationMsg.strCityType.toLocal8Bit().data(), szDataType, stStationMsg.nTimeRange, szTime, pData, nDataLen);
//	if (nErr != SUCCESS || pData == NULL)
//	{
//		// 获取数据失败
//		return -2;
//	}
//
//
//	// 数据处理
//	HASH_STATION_INFO hasStationInfo = DataManager::getClass().GetStationList();
//	QString strForecastType = DataManager::getClass().GetForecastType();
//
//	// 数据解析
//	QString strBuf = QString::fromLocal8Bit(pData);
//	int nPosStart = strBuf.indexOf("[");
//	int nPosEnd = strBuf.indexOf("]");
//	
//	QString strData = strBuf.mid(nPosStart, nPosEnd - nPosStart - 1);
//	QStringList lstData = strData.split(";");
//	int nStationCnt = lstData.size();
//
//	// 创建数据存储空间
//	st_station* pstStation = (st_station *)calloc(nStationCnt, sizeof(st_station));
//	if (pstStation == NULL)
//	{
//		// 创建存储空间失败
//		// 释放数据空间
//		if (pData)
//		{
//			free(pData);
//			pData = NULL;
//		}
//
//		return -3;
//	}
//
//	// 数据的列数固定
//	int nCol = stStationMsg.nForecastTime / stStationMsg.nTimeRange;
//
//	float fStartLon,fEndLon, fStartLat, fEndLat;
//
//	int nCnt = 0;
//	QStringList::iterator iter;
//	for (iter = lstData.begin(); iter != lstData.end(); iter++)
//	{
//		QString strTmp = *iter; // 544123(1.0,2.0,3.0,4.0)
//		int nDataPosStart = strTmp.indexOf("(");
//		int nDataPosEnd = strTmp.indexOf(")");
//
//		// 站号
//		QString strStationNo = strTmp.mid(0, nDataPosStart);
//		if (!hasStationInfo.contains(strStationNo))
//		{
//			// 配置的站点列表中没有该站点，因此没有办法知道经纬度，也就没办法进行转换处理
//			continue;
//		}
//		ST_STATION_INFO stStationInfo = hasStationInfo.value(strStationNo);
//
//		QString strValueBuf = strTmp.mid(nDataPosStart, nDataPosEnd - nDataPosStart - 1);
//		QStringList lstValue = strValueBuf.split(",");
//		if (lstValue.size()  < nCol)
//		{
//			// 数据个数不全，无法判定处理
//			continue;
//		}
//
//		// 数据
//		pstStation[nCnt].fData = (float *)malloc(nCol * sizeof(float));
//		if (pstStation[nCnt].fData == NULL)
//		{
//			// 创建数据空间失败，
//			continue;
//		}
//
//		// 赋值
//		memset(pstStation[nCnt].stationNo, 0x00, sizeof(pstStation[nCnt].stationNo));
//		strcpy(pstStation[nCnt].stationNo, strStationNo.toLocal8Bit().data());
//		pstStation[nCnt].fLon = stStationInfo.fLon;
//		pstStation[nCnt].fLat = stStationInfo.fLat;
//		pstStation[nCnt].fHeight = stStationInfo.fHeight;
//		pstStation[nCnt].nRow = 1;
//		pstStation[nCnt].nCol = nCol;
//		if (nCnt == 0)
//		{
//			fStartLon = pstStation[nCnt].fLon;
//			fEndLon = pstStation[nCnt].fLon;
//			fStartLat = pstStation[nCnt].fLat;
//			fEndLat = pstStation[nCnt].fLat;
//		}
//		else
//		{
//			if (pstStation[nCnt].fLon < fStartLon)
//			{
//				fStartLon = pstStation[nCnt].fLon;
//			}
//			if (pstStation[nCnt].fLon > fEndLon)
//			{
//				fEndLon = pstStation[nCnt].fLon;
//			}
//			if (pstStation[nCnt].fLat < fStartLat)
//			{
//				fStartLat = pstStation[nCnt].fLat;
//			}
//			if (pstStation[nCnt].fLat > fEndLat)
//			{
//				fEndLat = pstStation[nCnt].fLat;
//			}
//		}
//
//		for (int i = 0; i < nCol; i++)
//		{
//			pstStation[nCnt].fData[i] = lstValue.at(i).toFloat();
//		}
//
//		nCnt++;
//	}
//
//
//	// 站点-格点订正处理
//	for (int forecast = stStationMsg.nTimeRange; forecast <= stStationMsg.nForecastTime; forecast += stStationMsg.nTimeRange)
//	{
//		int i = forecast / stStationMsg.nTimeRange - 1;
//
//		QString strTime = HandleCommon::GetCachedProductTime(stStationMsg.nYear, stStationMsg.nMonth, stStationMsg.nDay, stStationMsg.nHour, forecast, stStationMsg.nTimeRange);
//
//		LIST_STATION_VALUE lstDiffVal;
//		// 计算差值
//		for (int n = 0; n < nCnt; n++)
//		{
//			// 获取
//			float * fStationData = NULL;
//			int nStationDataLen = 0;
//			int nErr = m_pCachedImpl->GetStationData(stStationMsg.strCityType.toLocal8Bit().data(), strForecastType.toLocal8Bit().data(),
//				stStationMsg.nTimeRange, pstStation[n].stationNo, strTime.toLocal8Bit().data(), fStationData, nStationDataLen);
//			if (fStationData == NULL)
//			{
//				// 不对此数据进行处理
//				continue;
//			}
//
//			// 创建差值
//			ST_STATION_VALUE * stStationValue = new ST_STATION_VALUE;
//			if (stStationValue == NULL)
//			{
//				// 释放空间
//				if (fStationData)
//				{
//					free(fStationData);
//					fStationData = NULL;
//				}
//				continue;
//			}
//
//			stStationValue->strStationNo = QString::fromLocal8Bit(pstStation[nCnt].stationNo);
//			stStationValue->lon = pstStation[n].fLon;
//			stStationValue->lat = pstStation[n].fLat;
//			stStationValue->height = pstStation[n].fHeight;
//			stStationValue->fValue = pstStation[n].fData[i] - fStationData[nPos];
//			// 插入列表中
//			lstDiffVal.push_back(stStationValue);
//
//			// 释放空间
//			if (fStationData)
//			{
//				free(fStationData);
//				fStationData = NULL;
//			}
//		}
//
//		// 格点到站点订正处理
//		int nErr = DataManager::getClass().MultiStation2Grid(stStationMsg.strDataType, lstDiffVal, fStartLon, fStartLat, fEndLon, fEndLat,
//			stStationMsg.nYear, stStationMsg.nMonth, stStationMsg.nDay, stStationMsg.nHour, stStationMsg.nTimeRange, forecast);
//		if (nErr != SUCCESS)
//		{ 
//			// 处理失败
//		}
//		else
//		{
//			// 处理成功
//		}
//
//		msleep(50);
//	}
//
//	// 释放数据空间
//	for (int n = 0; n < nCnt; n++)
//	{
//		if (pstStation[n].fData)
//		{
//			free(pstStation[n].fData);
//			pstStation[n].fData = NULL;
//		}
//	}
//	free(pstStation);
//	pstStation = NULL;
//	
//	if (pData)
//	{
//		free(pData);
//		pData = NULL;
//	}
//
//	return 0;
//}

/************************************************************************/
/* 处理多站点消息                                                       */
/************************************************************************/
int TaskStationMsg::ProcessMultiStation(ST_StationMsg stStationMsg)
{
	// 获取站点文件数据
	if (m_pCachedImpl == NULL)
	{
		InitCached();
	}
	// 缓存连接不正常，重连
	if (m_pCachedImpl->ConnectStatus() != SUCCESS)
	{
		m_pCachedImpl->ReConnect();
	}

	// 获取多站点缓存数据
	char szDataType[25];
	memset(szDataType, 0x00, sizeof(szDataType));
	sprintf(szDataType, CLIENT_STATION_TYPE, stStationMsg.nClientID, stStationMsg.strDataType.toLocal8Bit().data());

	// 时间
	char szTime[256];
	memset(szTime, 0x00, sizeof(szTime));
	sprintf(szTime, CACHED_TIME_FORMAT, stStationMsg.nYear, stStationMsg.nMonth, stStationMsg.nDay, stStationMsg.nHour, 0, stStationMsg.nTimeRange);

	// 数据
	char * pData = NULL;
	int nDataLen = 0;
	int nErr = m_pCachedImpl->GetMultiStationData(szDataType, stStationMsg.nTimeRange, szTime, pData, nDataLen);
	if (nErr != SUCCESS || pData == NULL)
	{
		// 获取数据失败
		return -2;
	}

	// 数据处理
	HASH_STATION_INFO hasStationInfo = DataManager::getClass().GetStationList();
	QString strForecastType = DataManager::getClass().GetForecastType();

	// 数据解析
	QString strBuf = QString::fromLocal8Bit(pData);
	int nPosStart = strBuf.indexOf("[");
	int nPosEnd = strBuf.indexOf("]");

	QString strData = strBuf.mid(nPosStart+1, nPosEnd - nPosStart - 1);
	QStringList lstData = strData.split(";");
	int nStationCnt = lstData.size();

	// 创建数据存储空间
	st_station* pstStation = (st_station *)calloc(nStationCnt, sizeof(st_station));
	if (pstStation == NULL)
	{
		// 创建存储空间失败
		// 释放数据空间
		if (pData)
		{
			free(pData);
			pData = NULL;
		}

		return -3;
	}

	// 数据的列数固定
	int nCol = stStationMsg.nForecastTime / stStationMsg.nTimeRange;

	float fStartLon, fEndLon, fStartLat, fEndLat;

	int nCnt = 0;
	QStringList::iterator iter;
	for (iter = lstData.begin(); iter != lstData.end(); iter++)
	{
		QString strTmp = *iter; // 544123(1.0,2.0,3.0,4.0)
		int nDataPosStart = strTmp.indexOf("(");
		int nDataPosEnd = strTmp.indexOf(")");

		// 站号
		QString strStationNo = strTmp.mid(0, nDataPosStart);
		if (!hasStationInfo.contains(strStationNo))
		{
			// 配置的站点列表中没有该站点，因此没有办法知道经纬度，也就没办法进行转换处理
			continue;
		}
		ST_STATION_INFO stStationInfo = hasStationInfo.value(strStationNo);

		QString strValueBuf = strTmp.mid(nDataPosStart+1, nDataPosEnd - nDataPosStart - 1);
		QStringList lstValue = strValueBuf.split(",");
		if (lstValue.size()  < nCol)
		{
			// 数据个数不全，无法判定处理
			continue;
		}

		// 数据
		pstStation[nCnt].fData = (float *)malloc(nCol * sizeof(float));
		if (pstStation[nCnt].fData == NULL)
		{
			// 创建数据空间失败，
			continue;
		}

		// 赋值
		memset(pstStation[nCnt].stationNo, 0x00, sizeof(pstStation[nCnt].stationNo));
		strcpy(pstStation[nCnt].stationNo, strStationNo.toLocal8Bit().data());
		pstStation[nCnt].fLon = stStationInfo.fLon;
		pstStation[nCnt].fLat = stStationInfo.fLat;
		pstStation[nCnt].fHeight = stStationInfo.fHeight;
		pstStation[nCnt].nRow = 1;
		pstStation[nCnt].nCol = nCol;
		if (nCnt == 0)
		{
			fStartLon = pstStation[nCnt].fLon;
			fEndLon = pstStation[nCnt].fLon;
			fStartLat = pstStation[nCnt].fLat;
			fEndLat = pstStation[nCnt].fLat;
		}
		else
		{
			if (pstStation[nCnt].fLon < fStartLon)
			{
				fStartLon = pstStation[nCnt].fLon;
			}
			if (pstStation[nCnt].fLon > fEndLon)
			{
				fEndLon = pstStation[nCnt].fLon;
			}
			if (pstStation[nCnt].fLat < fStartLat)
			{
				fStartLat = pstStation[nCnt].fLat;
			}
			if (pstStation[nCnt].fLat > fEndLat)
			{
				fEndLat = pstStation[nCnt].fLat;
			}
		}

		for (int i = 0; i < nCol; i++)
		{
			pstStation[nCnt].fData[i] = lstValue.at(i).toFloat();
		}

		nCnt++;
	}

	// 站点-格点订正处理
	if (DataManager::getClass().IsStation2Grid())
	{
		for (int forecast = stStationMsg.nTimeRange; forecast <= stStationMsg.nForecastTime; forecast += stStationMsg.nTimeRange)
		{
			int i = forecast / stStationMsg.nTimeRange - 1;

			QString strTime = HandleCommon::GetCachedProductTime(stStationMsg.nYear, stStationMsg.nMonth, stStationMsg.nDay, stStationMsg.nHour, forecast, stStationMsg.nTimeRange);

			LIST_STATION_VALUE lstDiffVal;
			bool bIsDiff = false;

			// 计算差值
			for (int n = 0; n < nCnt; n++)
			{
				// 获取
				QString strStationNo = QString::fromLocal8Bit(pstStation[n].stationNo);
				float fStationData;
				int nErr = DataManager::getClass().GetStationData(stStationMsg.strDataType, stStationMsg.nTimeRange, stStationMsg.nYear, stStationMsg.nMonth, stStationMsg.nDay, stStationMsg.nHour, forecast, strStationNo, fStationData);
				if (nErr != 0)
				{
					// 获取失败的该如何处理 ？？？

					// 获取失败，插入该值
					DataManager::getClass().SetStationData(stStationMsg.strDataType, stStationMsg.nTimeRange, stStationMsg.nYear, stStationMsg.nMonth, stStationMsg.nDay, stStationMsg.nHour, forecast, strStationNo, pstStation[n].fData[i]);
				
					// 该值不做计算
					continue;
				}

				if (IS_EQUAL(fStationData, 999.9))
				{
					// 如果该值是缺测值该如何处理 ？？？
					continue;
				}
				if (IS_EQUAL(pstStation[n].fData[i], 999.9))
				{
					// 如果该值是缺测值该如何处理 ？？？
					continue;
				}

				// 创建差值
				ST_STATION_VALUE * stStationValue = new ST_STATION_VALUE;
				if (stStationValue == NULL)
				{
					continue;
				}

				stStationValue->strStationNo = strStationNo;
				stStationValue->lon = pstStation[n].fLon;
				stStationValue->lat = pstStation[n].fLat;
				stStationValue->height = pstStation[n].fHeight;
				stStationValue->fValue = pstStation[n].fData[i] - fStationData;
				if (!IS_EQUAL_ZERO(stStationValue->fValue))
				{
					// 值发生变化，保存该值
					DataManager::getClass().SetStationData(stStationMsg.strDataType, stStationMsg.nTimeRange, stStationMsg.nYear, stStationMsg.nMonth, stStationMsg.nDay, stStationMsg.nHour, forecast, strStationNo, pstStation[n].fData[i]);
					bIsDiff = true;
				}

				// 插入列表中
				lstDiffVal.push_back(stStationValue);
			}

			// 格点到站点订正处理
			if (bIsDiff)
			{
				int nErr = DataManager::getClass().MultiStation2Grid(stStationMsg.strDataType, lstDiffVal, fStartLon, fStartLat, fEndLon, fEndLat,
					stStationMsg.nYear, stStationMsg.nMonth, stStationMsg.nDay, stStationMsg.nHour, stStationMsg.nTimeRange, forecast);
				if (nErr != SUCCESS)
				{
					// 处理失败
				}
				else
				{
					// 处理成功
				}
			}

			// 释放空间
			for (LIST_STATION_VALUE::iterator iterdiff = lstDiffVal.begin(); iterdiff != lstDiffVal.end(); iterdiff++)
			{
				ST_STATION_VALUE * stStationValue = *iterdiff;
				if (stStationValue)
				{
					delete stStationValue;
					stStationValue = NULL;
				}
			}
			lstDiffVal.clear();

			msleep(50);
		}
	}
	// 重新生成该类型的多站点数据
	nErr = DataManager::getClass().SaveMultiStation(stStationMsg.strDataType, stStationMsg.nYear, stStationMsg.nMonth, stStationMsg.nDay, stStationMsg.nHour, stStationMsg.nTimeRange, stStationMsg.nForecastTime);
	if (nErr == SUCCESS)
	{
		// 处理成功，发布？

	}
	else
	{
		// 处理失败

	}

	// 释放数据空间
	for (int n = 0; n < nCnt; n++)
	{
		if (pstStation[n].fData)
		{
			free(pstStation[n].fData);
			pstStation[n].fData = NULL;
		}
	}
	free(pstStation);
	pstStation = NULL;

	if (pData)
	{
		free(pData);
		pData = NULL;
	}

	return 0;
}


/************************************************************************/
/* 处理多站点差值消息-进行站点到格点插值处理请求                        */
/************************************************************************/
int TaskStationMsg::ProcessMultiDiffStation(ST_StationMsg stStationMsg)
{
	// 获取站点文件数据
	if (m_pCachedImpl == NULL)
	{
		InitCached();
	}
	// 缓存连接不正常，重连
	if (m_pCachedImpl->ConnectStatus() != SUCCESS)
	{
		m_pCachedImpl->ReConnect();
	}

	// 获取多站点缓存数据
	char szDataType[25];
	memset(szDataType, 0x00, sizeof(szDataType));
	sprintf(szDataType, CLIENT_STATION_TYPE_DIFF, stStationMsg.nClientID, stStationMsg.strDataType.toLocal8Bit().data());

	// 时间
	char szTime[256];
	memset(szTime, 0x00, sizeof(szTime));
	sprintf(szTime, CACHED_TIME_FORMAT, stStationMsg.nYear, stStationMsg.nMonth, stStationMsg.nDay, stStationMsg.nHour, 0, stStationMsg.nTimeRange);

	// 数据
	char * pData = NULL;
	int nDataLen = 0;
	int nErr = m_pCachedImpl->GetMultiStationData(szDataType, stStationMsg.nTimeRange, szTime, pData, nDataLen);
	if (nErr != SUCCESS || pData == NULL)
	{
		// 获取数据失败
		return -2;
	}

	// 数据处理
	HASH_STATION_INFO hasStationInfo = DataManager::getClass().GetStationList();

	// 数据解析
	QString strBuf = QString::fromLocal8Bit(pData);
	int nPosStart = strBuf.indexOf("[");
	int nPosEnd = strBuf.indexOf("]");

	QString strData = strBuf.mid(nPosStart + 1, nPosEnd - nPosStart - 1);
	QStringList lstData = strData.split(";");
	int nStationCnt = lstData.size();

	// 创建数据存储空间
	st_station* pstStation = (st_station *)calloc(nStationCnt, sizeof(st_station));
	if (pstStation == NULL)
	{
		// 创建存储空间失败
		// 释放数据空间
		if (pData)
		{
			free(pData);
			pData = NULL;
		}

		return -3;
	}

	// 数据的列数固定
	int nCol = (stStationMsg.nForecastTime - stStationMsg.nStartForecast) / stStationMsg.nTimeRange + 1;

	float fStartLon, fEndLon, fStartLat, fEndLat;

	int nCnt = 0;
	QStringList::iterator iter;
	for (iter = lstData.begin(); iter != lstData.end(); iter++)
	{
		QString strTmp = *iter; // 544123(1.0,2.0,3.0,4.0)
		int nDataPosStart = strTmp.indexOf("(");
		int nDataPosEnd = strTmp.indexOf(")");

		// 站号
		QString strStationNo = strTmp.mid(0, nDataPosStart);
		if (!hasStationInfo.contains(strStationNo))
		{
			// 配置的站点列表中没有该站点，因此没有办法知道经纬度，也就没办法进行转换处理
			continue;
		}
		ST_STATION_INFO stStationInfo = hasStationInfo.value(strStationNo);

		QString strValueBuf = strTmp.mid(nDataPosStart + 1, nDataPosEnd - nDataPosStart - 1);
		QStringList lstValue = strValueBuf.split(",");
		if (lstValue.size()  < nCol)
		{
			// 数据个数不全，无法判定处理
			continue;
		}

		// 数据
		pstStation[nCnt].fData = (float *)malloc(nCol * sizeof(float));
		if (pstStation[nCnt].fData == NULL)
		{
			// 创建数据空间失败，
			continue;
		}

		// 赋值
		memset(pstStation[nCnt].stationNo, 0x00, sizeof(pstStation[nCnt].stationNo));
		strcpy(pstStation[nCnt].stationNo, strStationNo.toLocal8Bit().data());
		pstStation[nCnt].fLon = stStationInfo.fLon;
		pstStation[nCnt].fLat = stStationInfo.fLat;
		pstStation[nCnt].fHeight = stStationInfo.fHeight;
		pstStation[nCnt].nRow = 1;
		pstStation[nCnt].nCol = nCol;
		if (nCnt == 0)
		{
			fStartLon = pstStation[nCnt].fLon;
			fEndLon = pstStation[nCnt].fLon;
			fStartLat = pstStation[nCnt].fLat;
			fEndLat = pstStation[nCnt].fLat;
		}
		else
		{
			if (pstStation[nCnt].fLon < fStartLon)
			{
				fStartLon = pstStation[nCnt].fLon;
			}
			if (pstStation[nCnt].fLon > fEndLon)
			{
				fEndLon = pstStation[nCnt].fLon;
			}
			if (pstStation[nCnt].fLat < fStartLat)
			{
				fStartLat = pstStation[nCnt].fLat;
			}
			if (pstStation[nCnt].fLat > fEndLat)
			{
				fEndLat = pstStation[nCnt].fLat;
			}
		}

		for (int i = 0; i < nCol; i++)
		{
			pstStation[nCnt].fData[i] = lstValue.at(i).toFloat();
		}

		nCnt++;
	}

	// 站点-格点订正处理
	for (int forecast = stStationMsg.nStartForecast; forecast <= stStationMsg.nForecastTime; forecast += stStationMsg.nTimeRange)
	{
		int i = forecast / stStationMsg.nTimeRange - 1;

		QString strTime = HandleCommon::GetCachedProductTime(stStationMsg.nYear, stStationMsg.nMonth, stStationMsg.nDay, stStationMsg.nHour, forecast, stStationMsg.nTimeRange);

		LIST_STATION_VALUE lstDiffVal;
		bool bIsDiff = false;

		// 构造该时次的站点差值列表
		for (int n = 0; n < nCnt; n++)
		{
			if (IS_EQUAL_ZERO(pstStation[n].fData[i]))
			{
				// 差值为0，没有变化
				continue;
			}

			// 创建差值
			ST_STATION_VALUE * stStationValue = new ST_STATION_VALUE;
			if (stStationValue == NULL)
			{
				continue;
			}

			stStationValue->strStationNo = QString::fromLocal8Bit(pstStation[n].stationNo);
			stStationValue->lon = pstStation[n].fLon;
			stStationValue->lat = pstStation[n].fLat;
			stStationValue->height = pstStation[n].fHeight;
			stStationValue->fValue = pstStation[n].fData[i];

			// 插入列表中
			lstDiffVal.push_back(stStationValue);
			bIsDiff = true;
		}

		// 格点到站点订正处理
		if (bIsDiff)
		{
			int nErr = DataManager::getClass().MultiStation2Grid(stStationMsg.strDataType, lstDiffVal, fStartLon, fStartLat, fEndLon, fEndLat,
				stStationMsg.nYear, stStationMsg.nMonth, stStationMsg.nDay, stStationMsg.nHour, stStationMsg.nTimeRange, forecast);
			if (nErr != SUCCESS)
			{
				// 处理失败
			}
			else
			{
				// 处理成功
			}
		}

		// 释放空间
		for (LIST_STATION_VALUE::iterator iterdiff = lstDiffVal.begin(); iterdiff != lstDiffVal.end(); iterdiff++)
		{
			ST_STATION_VALUE * stStationValue = *iterdiff;
			if (stStationValue)
			{
				delete stStationValue;
				stStationValue = NULL;
			}
		}
		lstDiffVal.clear();

		msleep(50);
	}

	// 释放数据空间
	for (int n = 0; n < nCnt; n++)
	{
		if (pstStation[n].fData)
		{
			free(pstStation[n].fData);
			pstStation[n].fData = NULL;
		}
	}
	free(pstStation);
	pstStation = NULL;

	if (pData)
	{
		free(pData);
		pData = NULL;
	}

	return 0;
}

