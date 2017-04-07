#include "log.h" // log日志
#include "TaskStationRaw.h"
#include "Common.h"
#include "HandleCommon.h"
#include "HandleStationLib.h"
#include "DDSManager.h"
#include "NwfdErrorCode.h"
#include <QDir>
#include <QDateTime>
#include <QFileInfoList>

/************************************************************************/
/* 构造函数&析构函数                                                    */
/************************************************************************/
TaskStationRaw::TaskStationRaw()
{
	LOG_(LOGID_DEBUG, LOG_F("Entry TaskStationRaw()"));
	// 初始化状态为僵死，不初始化不予执行任务
	m_strTaskName = "站点扫描处理任务";
	m_emTaskState = _TASK_DEAD;
	m_pCachedImpl = NULL;
	m_strClearHasFlg = "";
	LOG_(LOGID_DEBUG, LOG_F("Leave TaskStationRaw()"));
}

TaskStationRaw::~TaskStationRaw()
{
	LOG_(LOGID_DEBUG, LOG_F("Entry ~TaskStationRaw()"));
	// 停止线程执行
	m_bReady = false;
	wait();
	LOG_(LOGID_DEBUG, LOG_F("Leave ~TaskStationRaw()"));
}

/************************************************************************/
/* 任务处理执行方法                                                     */
/************************************************************************/
void TaskStationRaw::run()
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
void TaskStationRaw::Init(ST_STATION_CFG stStationCfg, ST_CACHED stCached)
{
	m_stStationCfg = stStationCfg;
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
bool TaskStationRaw::InitCached()
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
/* 执行产品扫描处理任务                                                 */
/************************************************************************/
bool TaskStationRaw::ExecTask()
{
	// 执行扫描处理
	RawProcess();

	return true;
}

/************************************************************************/
/* 执行扫描处理                                                         */
/************************************************************************/
bool TaskStationRaw::RawProcess()
{
	int nYear, nMonth, nDay;
	int nErr = 0;

	// 当前日期
	QDateTime tToday = QDateTime::currentDateTime();
	nYear = tToday.date().year();
	nMonth = tToday.date().month();
	nDay = tToday.date().day();

	// 遍历每个项目
	LIST_SRC_ITEM::iterator iter;
	for (iter = m_stStationCfg.lstSrcItem.begin(); iter != m_stStationCfg.lstSrcItem.end(); iter++)
	{
		ST_SRC_ITEM stSrcItem = *iter;

		// 路径
		QString strSrcFolder = HandleCommon::GetFolderPath(stSrcItem.strSrcFolder, nYear, nMonth, nDay);
		strSrcFolder.replace("\\", "/");

		QFileInfoList lstFile;
		if (!GetProductFileList(strSrcFolder, stSrcItem.strSrcFileName, stSrcItem.stFileNameFormat, lstFile))
		{
			// 查找文件失败
			continue;
		}

		// 遍历文件，进行存储处理
		QFileInfoList::iterator iterFile;
		for (iterFile = lstFile.begin(); iterFile != lstFile.end(); iterFile++)
		{
			// 文件名
			QString strFileName = (*iterFile).fileName();
			QString strFile = QString("%1/%2").arg(strSrcFolder).arg(strFileName);

			// 判断是否已经成功处理过
			if (IsHasProcessed(strFile))
			{
				continue;
			}

			// 解析文件名确认产品
			int year, month, day, hour;
			
			if (!HandleCommon::ParseFileName(strFileName, stSrcItem.stFileNameFormat, year, month, day, hour))
			{
				// 解析文件名称失败 todo
				
				continue;
			}

			// * 起报时国际时-北京时转换
			//hour += 8;
			QDateTime tBjTime = HandleCommon::GetBJT(year, month, day, hour, 0, 0);
			year = tBjTime.date().year();
			month = tBjTime.date().month();
			day = tBjTime.date().day();
			hour = tBjTime.time().hour();			

			// 处理该文件
			st_station * pStation = NULL;
			int nStationCnt = 0;
			nErr = HandleStationLib::getClass().nwfd_openstationfile(m_stStationCfg.strCityType.toLocal8Bit().data(), strFile.toLocal8Bit().data(), pStation, nStationCnt);
			if (nErr != SUCCESS || pStation == NULL)
			{
				// 解析文件失败
				// todo			
				continue;
			}

			// 存储到缓存中
			nErr = SaveData2Cached(stSrcItem, year, month, day, hour, pStation, nStationCnt);
			if (nErr != SUCCESS)
			{
				// 存储失败
				// todo
				m_hasCompleteProduct_today[strFile] = false;
			}
			else
			{
				// 处理成功,设置标记位
				m_hasCompleteProduct_today[strFile] = true;
			}

			// 释放获取数据空间
			HandleStationLib::getClass().nwfd_freestationdata(pStation, nStationCnt);

			// 文件数据分发
			// 为已经做好的SPCC文件直接转发而做
			DataDistribution(stSrcItem.lstDDS, strFileName, strSrcFolder);
		}
	}

	return true;
}

/************************************************************************/
/* 获取产品文件列表                                                     */
/************************************************************************/
bool TaskStationRaw::GetProductFileList(QString strSrcFolder, QString strSrcFile, ST_FILE_FORMAT stFileNameFormat, QFileInfoList& lstFile)
{
	QDir dDir;
	// 检验产品目录是否存在
	if (!dDir.exists(strSrcFolder))
	{
		return false;
	}

	// 目录设置
	dDir.setPath(strSrcFolder); // 设置目录名称
	dDir.setFilter(QDir::Files);            // 列出目录中的文件
	dDir.setSorting(QDir::Name);            // 按照文件名排序

	// 文件筛选
	QString strFileFormat = strSrcFile;
	strFileFormat.replace("yyyymmddhhmiss", "*");
	strFileFormat.replace("FFF", "*");
	strFileFormat.replace("TT", "*");

	// 筛选日期
	QDateTime tToday = QDateTime::currentDateTime();
	QString strTodayFilterName = strFileFormat;
	// 年月日
	if (stFileNameFormat.stYear.iLen == 4) // 4位年份
	{
		strTodayFilterName.replace("YYYYMMDD", tToday.toString("yyyyMMdd"));
	}
	else if (stFileNameFormat.stYear.iLen == 2) // 2位年份
	{
		strTodayFilterName.replace("YYMMDD", tToday.toString("yyMMdd"));
	}

	// 时
	strTodayFilterName.replace("HH", "*");

	QStringList lstfileFilter;
	lstfileFilter << strTodayFilterName;

	dDir.setNameFilters(lstfileFilter); // 设置文件筛选

	// 获取产品文件
	lstFile = dDir.entryInfoList();

	return true;
}

/************************************************************************/
/* 存储数据到缓存中                                                     */
/************************************************************************/
int TaskStationRaw::SaveData2Cached(ST_SRC_ITEM stSrcItem, int nYear, int nMonth, int nDay, int nHour, st_station * pStation, int nStationCnt)
{
	// 检查缓存连接
	if (m_pCachedImpl == NULL)
	{
		return -1;
	}

	int nErr;

	// 将所有站点数据存储到缓存中
	for (int n = 0; n < nStationCnt; n++)
	{
		// 保存站点信息
		nErr = m_pCachedImpl->SaveStationInfo(m_stStationCfg.strCityType.toLocal8Bit().data(), stSrcItem.strDataType.toLocal8Bit().data(), m_stStationCfg.nTimeRange, 
			pStation[n].stationNo, pStation[n].fLon, pStation[n].fLat, pStation[n].fHeight, pStation[n].nCol);
		if (nErr != SUCCESS)
		{
			// 保存信息失败
			// todo
			continue;
		}

		// 保存站点数据
		int nOff = 0;
		for (int i = 0; i < pStation[n].nRow; i++)
		{
			int nForecastTime = (int)pStation[n].fData[nOff++];
			float * fData = pStation[n].fData + nOff;
			int nDataCnt = pStation[n].nCol;
			nOff += nDataCnt;

			// 保存该时效数据
			QString strTime = QString("").sprintf(CACHED_TIME_FORMAT, nYear, nMonth, nDay, nHour, nForecastTime, m_stStationCfg.nTimeRange);
			nErr = m_pCachedImpl->SaveStationData(m_stStationCfg.strCityType.toLocal8Bit().data(), stSrcItem.strDataType.toLocal8Bit().data(), m_stStationCfg.nTimeRange, pStation[n].stationNo, strTime.toLocal8Bit().data(), fData, nDataCnt);
			if (nErr != SUCCESS)
			{
				// 保存数据失败
				// todo
				LOG_(LOGID_STATION, LOG_F("[缓存]保存站点[%s]数据失败"), pStation[n].stationNo);
			}
		}
	}

	return 0;
}

/************************************************************************/
/* 保存数据处理                                                         */
/************************************************************************/
bool TaskStationRaw::MergeProcess()
{
	int nYear, nMonth, nDay;
	// 当前日期
	QDateTime tToday = QDateTime::currentDateTime();
	nYear = tToday.date().year();
	nMonth = tToday.date().month();
	nDay = tToday.date().day();

	// 判断处理时间是否到
	LIST_REPORT_TIMES::iterator iter;
	for (iter = m_stStationCfg.lstReportTimes.begin(); iter != m_stStationCfg.lstReportTimes.end(); iter++)
	{
		ST_REPORTTIME stReportTime = *iter;
		QString strTimeFlg = QString("").sprintf("%04d_%02d_%02d_%02d", nYear, nMonth, nDay, stReportTime.nReportTime);

		if (IsHasSaved(strTimeFlg))
		{
			// 已经处理过
			continue;
		}

		// 时间判断，是否到处理时间
		if (!IsTimeOut(stReportTime.nSaveHour, stReportTime.nSaveMinute))
		{
			//计划时间未到
			continue;
		}
		
		// 处理今日-该时次的数据
		int nErr = SaveProcess(stReportTime, nYear, nMonth, nDay, stReportTime.nReportTime);
		if (nErr == SUCCESS)
		{
			// 处理成功
			m_hasCompleteSave_today[strTimeFlg] = true;
		}
		else
		{
			// 处理失败 todo
			m_hasCompleteSave_today[strTimeFlg] = false;
		}

	}

	return false;
}

/************************************************************************/
/* 是否时间已到                                                         */
/************************************************************************/
bool TaskStationRaw::IsTimeOut(int nHour, int nMinute)
{
	QDateTime tCurrentTime = QDateTime::currentDateTime();
	QString   strPlanTime = QString("").sprintf("%04d-%02d-%02d %02d:%02d:%02d", tCurrentTime.date().year(), tCurrentTime.date().month(), tCurrentTime.date().day(), nHour, nMinute, 0);
	QDateTime tPlanTime = QDateTime::fromString(strPlanTime, "yyyy-MM-dd hh:mm:ss");
	if (tCurrentTime.toTime_t() >= tPlanTime.toTime_t())
	{
		// 计划时间到，进行处理
		return true;
	}

	// 计划时间未到
	return false;
}


/************************************************************************/
/* 保存数据处理                                                         */
/************************************************************************/
int TaskStationRaw::SaveProcess(ST_REPORTTIME stForecast, int nYear, int nMonth, int nDay, int nHour)
{
	// 1、判断配置 - 站点列表文件，获取规则，是否存储文件
	if (stForecast.strSaveStation.isEmpty() || stForecast.strSaveRule.isEmpty() || stForecast.stSaveFile.bDisabled == true)
	{
		return -1;
	}

	// 2、获取站点文件数据
	LIST_STATION_INFO lstStationInfo;
	int nErr = GetStationList(stForecast.strSaveStation, lstStationInfo);
	if (nErr != SUCCESS)
	{
		return nErr;
	}

	// 所要存储的站个数为0，不进行处理
	if (lstStationInfo.size() == 0)
	{
		return -2;
	}

	// 3、按照站点获取每个站的数据
	st_station * pstStation = (st_station *)calloc(lstStationInfo.size(), sizeof(st_station));
	if (pstStation == NULL)
	{
		return -3;
	}

	int nStationCnt = 0;
	LIST_STATION_INFO::Iterator iter;
	for (iter = lstStationInfo.begin(); iter != lstStationInfo.end(); iter++)
	{
		ST_STATION_INFO stStationInfo = *iter;
		float * fData = NULL;
		int nRow = 0;
		int nCol = 0;

		nErr = GetStationData(stStationInfo.stationNo, stForecast.strSaveRule, nYear, nMonth, nDay, nHour, 0, 0, fData, nRow, nCol);
		if (nErr != SUCCESS || fData == NULL || nRow == 0 || nCol == 0)
		{
			// 获取数据失败
			// todo

		}
		else
		{
			strcpy(pstStation[nStationCnt].stationNo, stStationInfo.stationNo);
			pstStation[nStationCnt].fLon = stStationInfo.fLon;
			pstStation[nStationCnt].fLat = stStationInfo.fLat;
			pstStation[nStationCnt].fHeight = stStationInfo.fHeight;
			pstStation[nStationCnt].fData = fData;
			pstStation[nStationCnt].nRow = nRow;
			pstStation[nStationCnt].nCol = nCol;

			nStationCnt++;
		}
	}

	// 个数判断
	if (nStationCnt != lstStationInfo.size())
	{
		// 有获取失败的数据
	}

	// 北京时-国际时 转换
	nHour -= 8;

	// 4、保存数据到文件中
	QString strPath = HandleCommon::GetFolderPath(stForecast.stSaveFile.strSavePath, nYear, nMonth, nDay);
	
	QString strFileName = HandleCommon::GetFileName(stForecast.stSaveFile.strFileName, nYear, nMonth, nDay, nHour);
	QString strFile = QString("%1/%2").arg(strPath).arg(strFileName);
	if (HandleCommon::HandleGribFolder(strPath))
	{
		nErr = HandleStationLib::getClass().nwfd_savedata2file(m_stStationCfg.strCityType.toLocal8Bit().data(), m_stStationCfg.strIDSymbol.toLocal8Bit().data(), 
			stForecast.strSaveType.toLocal8Bit().data(), strFile.toLocal8Bit().data(), pstStation, nStationCnt, nYear, nMonth, nDay, nHour, 0, 0);
		if (nErr == SUCCESS)
		{
			// 5、 数据分发
		}
	}


	// 6、 释放存储空间
	for (int n = 0; n < nStationCnt; n++)
	{
		if (pstStation[n].fData)
		{
			free(pstStation[n].fData);
			pstStation[n].fData = NULL;
		}
	}
	free(pstStation);
	pstStation = NULL;
	
	return 0;
}

/************************************************************************/
/* 获取要保存的站点信息                                                 */
/************************************************************************/
int TaskStationRaw::GetStationData(char * strStationNo, QString strSaveRule, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, float * &fData, int &nRow, int &nCol)
{
	// 1、 初始化
	fData = NULL;
	nRow = 0;
	nCol = 0;

	int nErr = 0;

	// 2、站号
	if (strStationNo == NULL)
	{
		return -1;
	}

	// 3、按照配置的时间间隔和最大预报时效进行获取
	int nForecastTime, year, month, day, hour;
	nwfd_station_data * stDataInfo = NULL;
	float * fForecastData = NULL;
	int nCnt = 0;
	int nOff = 0;
	unsigned int nDataLenSum = 0;
	for (nForecastTime = m_stStationCfg.nTimeRange; nForecastTime <= m_stStationCfg.nMaxForecastTime; nForecastTime += m_stStationCfg.nTimeRange)
	{
		// 计算获取数据时间
		GetForecastTime(nYear, nMonth, nDay, nHour, nForecastTime, year, month, day, hour);

		// 获取数据
		nErr = nwfd_get_stationdata_forecasttime(m_stStationCfg.strCityType.toLocal8Bit().data(), strSaveRule.toLocal8Bit().data(), m_stStationCfg.nTimeRange,
			strStationNo, year, month, day, hour, 0, 0, fForecastData, stDataInfo, nCnt);

		if (nErr != SUCCESS || fForecastData == NULL || stDataInfo == NULL || nCnt != 1)
		{
			continue;
		}

		// 总长度
		nDataLenSum += (stDataInfo[0].length + 1) * sizeof(float);

		// 分配空间
		if (nForecastTime == m_stStationCfg.nTimeRange)
		{
			// 分配空间
			fData = (float *)malloc(nDataLenSum);
		}
		else
		{
			// 重新分配空间
			fData = (float *)realloc(fData, nDataLenSum);
		}
		if (fData == NULL)
		{
			return ERR_MEM_MALLOC;
		}

		// 赋值
		fData[nOff++] = (float)nForecastTime;  // 首列为预报时效
		memcpy((char *)fData + nOff * sizeof(float), (char *)fForecastData, stDataInfo[0].length * sizeof(float));
		nOff += stDataInfo[0].length;

		// 列数判断
		if (nCol == 0)
		{
			nCol = stDataInfo[0].length;
		}
		else if (nCol != stDataInfo[0].length)
		{
			//返回的列数不整齐
			// todo
		}

		// 行数个数加1
		nRow++;

		// 释放空间
		free(fForecastData);
		fForecastData = NULL;
		free(stDataInfo);
		stDataInfo = NULL;
	}

	return SUCCESS;
}

/************************************************************************/
/* 从缓存中获取站点数据方法                                             */
/************************************************************************/
int TaskStationRaw::nwfd_get_stationdata_forecasttime(const char * strCityType, const char * szRule, int nTimeRange, const char * szStationNo,
	int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, float * &fData, nwfd_station_data *& stDataInfo, int &nCnt)
{
	// 参数判断
	if (strCityType == NULL || szRule == NULL || szStationNo == NULL)
	{
		return ERR_PARAM_NULL;
	}

	// 缓存连接判断
	if (m_pCachedImpl == NULL)
	{
		return -1;
	}
	// 返回值初始化
	fData = NULL;
	stDataInfo = NULL;
	nCnt = 0;

	// 解析出站点列表和取数据规则
	StringList lstStationNo;
	StringList lstRule; 
	split(lstStationNo, szStationNo, ",");
	split(lstRule, szRule, ",");

	int nErrCode;
	int nStationCnt = 0;  // 最终获取的数据总个数
	int nDataLenSum = 0;  //  最终获取的数据总长度

	StringList::iterator iterStation;
	for (iterStation = lstStationNo.begin(); iterStation != lstStationNo.end(); iterStation++)
	{
		string strStationNo = (*iterStation);

		StringList::iterator iterRule;
		for (iterRule = lstRule.begin(); iterRule != lstRule.end(); iterRule++)
		{
			string strRule = *iterRule;

			// 获取时间列表
			StringList lstTime;
			nErrCode = m_pCachedImpl->GetStationTime(strCityType, strRule.data(), nTimeRange, strStationNo.data(), lstTime, true);
			if (nErrCode != SUCCESS)
			{
				// 获取失败,根据下一条规则获取时间
				continue;
			}

			// 遍历时间，进行反序查找
			StringList::reverse_iterator iterTime;
			bool bIsFoundFlg = false;
			string strTime;
			int year, month, day, hour, minute, second, forecast, range;
			for (iterTime = lstTime.rbegin(); iterTime != lstTime.rend(); iterTime++)
			{
				strTime = *iterTime;

				// 解析时间
				if (!HandleCommon::ParseProductTime(strTime.data(), year, month, day, hour, minute, second, forecast, range))
				{
					// 解析失败
					// todo
					continue;
				}

				// 时间判断
				time_t tSearchTime = GetTime(nYear, nMonth, nDay, nHour, nMinute, nSecond);
				time_t tForcastTime = GetTime(year, month, day, hour, minute, second, forecast);

				if (tSearchTime == tForcastTime)
				{
					bIsFoundFlg = true;
					break;
				}
			}

			// 没有找到，进行下一规则的查找
			if (!bIsFoundFlg)
			{
				continue;
			}

			// 找到时间，获取信息和数据
			float *fStationData = NULL;
			int nDataLen = 0;
			float fLon = -1, fLat = -1, fHeight = -1;
			
			// 获取数据
			nErrCode = m_pCachedImpl->GetStationData(strCityType, strRule.data(), nTimeRange, strStationNo.data(), strTime.data(), fStationData, nDataLen);
			if (nErrCode != SUCCESS || fStationData == NULL)
			{
				// 获取数据失败，跳过
				continue;
			}

			// 城镇数据：高温，低温交换
			if (strcmp(strCityType,"town") == 0 &&  (nHour != hour))
			{
				// 起报时间不同时，即一个08时，一个20时起报，此时高温，低温是相反的
				float fTempValue = fStationData[10];
				fStationData[10] = fStationData[11];
				fStationData[11] = fTempValue;
			}

			// 创建返回数据空间
			nDataLen *= sizeof(float); // 数据占内存长度
			nDataLenSum += nDataLen;   // 数据总长度
			if (nStationCnt == 0)
			{
				// 第一个数据
				fData = (float *)malloc(nDataLenSum);
				if (fData == NULL)
				{
					free(fStationData);
					return ERR_MEM_MALLOC;
				}

				// 分配块信息空间
				stDataInfo = (nwfd_station_data *)calloc(1, sizeof(nwfd_station_data));
				if (stDataInfo == NULL)
				{
					free(fStationData);
					free(fData);
					fData = NULL;
					return ERR_MEM_MALLOC;
				}
			}
			else
			{
				// 重新分配空间
				fData = (float *)realloc(fData, nDataLenSum);
				if (fData == NULL)
				{
					free(fStationData);
					free(fData);
					fData = NULL;
					free(stDataInfo);
					stDataInfo = NULL;

					return ERR_MEM_MALLOC;
				}
				stDataInfo = (nwfd_station_data *)realloc(stDataInfo, (nStationCnt + 1) * sizeof(nwfd_station_data));
				if (stDataInfo == NULL)
				{
					free(fStationData);

					free(fData);
					fData = NULL;

					return ERR_MEM_MALLOC;
				}
			}

			// 返回数据赋值
			memcpy((char *)fData + nDataLenSum - nDataLen, (char *)fStationData, nDataLen);
			strcpy(stDataInfo[nStationCnt].type, strRule.data());  // 数据类型 SCMOC，SPVT, SPCC，SCMOU...
			stDataInfo[nStationCnt].lon = fLon;       // 经度
			stDataInfo[nStationCnt].lat = fLat;       // 纬度
			stDataInfo[nStationCnt].height = fHeight;    // 高度
			stDataInfo[nStationCnt].length = nDataLen / sizeof(float);    // 数据列数（数据长度）
			stDataInfo[nStationCnt].year = year;         // 年
			stDataInfo[nStationCnt].month = month;        // 月
			stDataInfo[nStationCnt].day = day ;          // 日
			stDataInfo[nStationCnt].hour = hour;         // 时
			stDataInfo[nStationCnt].minute = minute;       // 分
			stDataInfo[nStationCnt].second = second;       // 秒
			stDataInfo[nStationCnt].forecast = forecast;     // 预报时间
			stDataInfo[nStationCnt].timerange = range;    // 时间间隔

			nStationCnt += 1;

			// 本规则数据已经找到，结束本站号的查找
			break;
		}
	}

	// 返回数据个数
	nCnt = nStationCnt;

	return SUCCESS;
}

/************************************************************************/
/* 计算预报时间                                                         */
/************************************************************************/
time_t TaskStationRaw::GetTime(int year, int month, int day, int hour, int minute, int second, int forecast)
{
	QString strTime = QString("").sprintf("%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, minute, second);
	QDateTime tTime = QDateTime::fromString(strTime, "yyyy-MM-dd hh:mm:ss");
	QDateTime tForecastTime = tTime;
	if (forecast > 0)
	{
		tForecastTime = tTime.addSecs(forecast * 3600);
	}

	return tForecastTime.toTime_t();
}

/************************************************************************/
/* 获取预报时间                                                         */
/************************************************************************/
void TaskStationRaw::GetForecastTime(int nYear, int nMonth, int nDay, int nHour, int nForecastTime, int &year, int &month, int &day, int &hour)
{
	QString strTime = QString("").sprintf("%04d-%02d-%02d %02d:%02d:%02d", nYear, nMonth, nDay, nHour, 0, 0);
	QDateTime tTime = QDateTime::fromString(strTime, "yyyy-MM-dd hh:mm:ss");
	QDateTime tNewTime = tTime.addSecs(nForecastTime*3600);

	// 获取新的时间
	year = tNewTime.date().year();
	month = tNewTime.date().month();
	day = tNewTime.date().day();
	hour = tNewTime.time().hour();
}

/************************************************************************/
/* 获取要保存的站点信息                                                 */
/************************************************************************/
int TaskStationRaw::GetStationList(QString strSaveStation, LIST_STATION_INFO &lstStationInfo)
{
	// 打开文件
	FILE *fp;
	fp = fopen(strSaveStation.toLocal8Bit().data(), "r");
	if (fp == NULL)
	{
		// 文件打开失败
		return -1;
	}

	char szBuf[512];
	string strBuf;
	string strData;
	string::size_type nOff = 0;  // 字符串游标位置
	string separator = " \t\r\n";

	ST_STATION_INFO stStationInfo;
	lstStationInfo.clear();

	// 按行读取文件内容
	while (!feof(fp))
	{
		fgets(szBuf, sizeof(szBuf), fp);
		strBuf = string(szBuf);
		nOff = 0;

		// 站号
		strData = getdata(strBuf, separator, nOff);
		if (strData == "")
		{
			strData = getdata(strBuf, separator, nOff);
		}
		strcpy(stStationInfo.stationNo ,strData.data());

		// 经度
		strData = getdata(strBuf, separator, nOff);
		stStationInfo.fLon = atof(strData.data());

		// 纬度
		strData = getdata(strBuf, separator, nOff);
		stStationInfo.fLat = atof(strData.data());

		// 海拔
		strData = getdata(strBuf, separator, nOff);
		stStationInfo.fHeight = atof(strData.data());

		// 添加到列表中
		lstStationInfo.push_back(stStationInfo);
	}

	// 关闭文件
	fclose(fp);

	return 0;
}

/************************************************************************/
/* 是否已经处理判断方法                                                 */
/************************************************************************/
bool TaskStationRaw::IsHasProcessed(QString strFile)
{
	// 查看今天处理的数据
	if (m_hasCompleteProduct_today.contains(strFile))
	{
		return 	m_hasCompleteProduct_today[strFile];
	}

	// 查看昨天处理的数据
	if (m_hasCompleteProduct_yesterday.contains(strFile))
	{
		return 	m_hasCompleteProduct_yesterday[strFile];
	}

	return false;
}

/************************************************************************/
/* 是否已经保存过的时效                                                 */
/************************************************************************/
bool TaskStationRaw::IsHasSaved(QString strTime)
{
	// 查看今天处理的数据
	if (m_hasCompleteSave_today.contains(strTime))
	{
		return 	m_hasCompleteSave_today[strTime];
	}

	// 查看昨天处理的数据
	if (m_hasCompleteSave_yesterday.contains(strTime))
	{
		return 	m_hasCompleteSave_yesterday[strTime];
	}

	return false;
}

/************************************************************************/
/* 已处理产品Hash标记维护处理                                           */
/************************************************************************/
void TaskStationRaw::HandleClearHash()
{
	// 按天进行标记判断处理
	QString strNewClearHasFlg = QDateTime::currentDateTime().toString("yyyyMMdd");
	if (strNewClearHasFlg != m_strClearHasFlg)
	{
		// 日期已经切换，结果判断列表清空，防止无限增长
		// 清除昨天的数据
		m_hasCompleteProduct_yesterday.clear();
		m_hasCompleteSave_yesterday.clear();
		// 今天的数据成为昨天的数据
		m_hasCompleteProduct_yesterday = m_hasCompleteProduct_today;
		m_hasCompleteSave_yesterday = m_hasCompleteSave_today;

		// 清除今天的数据
		m_hasCompleteProduct_today.clear();
		m_hasCompleteSave_today.clear();

		// 重新赋值
		m_strClearHasFlg = strNewClearHasFlg;
	}
}

/************************************************************************/
/* 数据分发处理                                                         */
/************************************************************************/
int TaskStationRaw::DataDistribution(QStringList lstDDS, QString strFileName, QString strPath)
{
	if (CDDSManager::getClass().IsDisabled())
	{
		// 数据分发管理功能禁用
		return 1;
	}

	// 添加到队列中
	ST_DDSFILE stDDSFile;
	stDDSFile.strProductKey = m_stStationCfg.strCityType;
	stDDSFile.strFileName = strFileName;
	stDDSFile.strFilePath = strPath;
	stDDSFile.nFailedNum = 0;

	int nErr;
	QStringList::iterator iterDDSID;
	for (iterDDSID = lstDDS.begin(); iterDDSID != lstDDS.end(); iterDDSID++)
	{
		stDDSFile.strDDSID = *iterDDSID;
		nErr = CDDSManager::getClass().AddDDSFile(stDDSFile);
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
