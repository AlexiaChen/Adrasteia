#include "log.h" // log日志
#include "TaskProductClientMerge.h"
#include "HandleDB.h"
#include "HandleGrib.h"
#include "HandleCommon.h"
#include "NetworkManager.h"
#include "Mask.h"
#include "DataManager.h"
#include "CachedManager.h"
#include <QDir>
#include <QDateTime>
#include <QFileInfoList>

/************************************************************************/
/* 构造函数&析构函数                                                    */
/************************************************************************/
TaskProductClientMerge::TaskProductClientMerge()
{
	LOG_(LOGID_DEBUG, LOG_F("Entry TaskProductClientMerge()"));
	m_strTaskName = "客户端上传数据实时拼图处理任务";

	// 初始化状态为僵死，不初始化不予执行任务
	m_emTaskState = _TASK_DEAD;

	m_strClearHasFlg = "";
	m_pCachedImpl = NULL;

	LOG_(LOGID_DEBUG, LOG_F("Leave TaskProductClientMerge()"));

}

TaskProductClientMerge::~TaskProductClientMerge()
{
	LOG_(LOGID_DEBUG, LOG_F("Entry ~TaskProductClientMerge()"));

	// 停止线程执行
	m_bReady = false;
	wait();

	LOG_(LOGID_DEBUG, LOG_F("Leave ~TaskProductClientMerge()"));
}

/************************************************************************/
/* 任务处理执行方法                                                     */
/************************************************************************/
void TaskProductClientMerge::run()
{
	if (m_emTaskState != _TASK_READ)
	{
		// 如果任务没有在准备状态，不予执行此次任务
		LOG_(LOGID_INFO, LOG_F("[%s]任务没有完成准备工作，不能执行，任务结束（当前状态为：%d）."), LOG_STR(m_stProduct.strKey),(int)m_emTaskState);
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
void TaskProductClientMerge::Init(ST_PRODUCT stProduct, ST_CACHED stCached)
{
	m_stProduct = stProduct;
	m_stCached = stCached;

	// 初始化缓存
	if (InitCached())
	{
		LOG_(LOGID_INFO, LOG_F("[TaskPID:%d][%s]初始化缓存成功）."), m_nTaskPID, LOG_STR(m_stProduct.strKey));
		if (m_pCachedImpl->SaveMergeKey(m_stProduct.strRelatedKey.toLocal8Bit().data()) == SUCCESS)
		{
			LOG_(LOGID_INFO, LOG_F("[TaskPID:%d][Key:%s] 保存产品缓存信息成功"), m_nTaskPID, LOG_STR(m_stProduct.strKey));
		}
		else
		{
			// 如果没有处理成功
			LOG_(LOGID_INFO, LOG_F("[TaskPID:%d][Key:%s] 保存产品缓存信息失败"), m_nTaskPID, LOG_STR(m_stProduct.strKey));
		}
	}
	else
	{
		LOG_(LOGID_INFO, LOG_F("[TaskPID:%d][%s]初始化缓存失败"), m_nTaskPID, LOG_STR(m_stProduct.strKey));
	}

	// 任务准备完毕，等待执行
	m_emTaskState = _TASK_READ;

	LOG_(LOGID_INFO, LOG_F("初始化完成，任务做好执行准备..."));
}

/*
void TaskProductClientMerge::InitProduct(ST_PRODUCT stProduct, HASH_PRODUCT& hasProduct)
{
	m_stProduct = stProduct;
	m_hasProduct = hasProduct;
}
*/

/************************************************************************/
/* 初始化缓存数据                                                       */
/************************************************************************/
bool TaskProductClientMerge::InitCached()
{
	// 建立缓存连接
	if (m_pCachedImpl == NULL)
	{
		m_pCachedImpl = new CachedImpl(m_stCached);
	}

	if (m_pCachedImpl == NULL)
	{
		// 创建失败
		LOG_(LOGID_ERROR, LOG_F("[TaskPID:%d][%s]Cached连接创建失败"), m_nTaskPID,LOG_STR(m_stProduct.strKey));
		LOG_(LOGID_INFO, LOG_F("[TaskPID:%d][Error]Cached连接创建失败"), m_nTaskPID);

		return false;
	}

	// 测试连接状态
	int nErr = m_pCachedImpl->ConnectStatus();
	QString strConnectStatus = (nErr == SUCCESS) ? QString("连接") : QString("断开");
	LOG_(LOGID_INFO, LOG_F("[TaskPID:%d][Key:%s] Cached连接状态: %s"), m_nTaskPID, LOG_STR(m_stProduct.strKey), LOG_STR(strConnectStatus));

	if (nErr != SUCCESS)
	{
		return false;
	}

	return true;
}

/************************************************************************/
/* 执行产品扫描处理任务                                                 */
/************************************************************************/
bool TaskProductClientMerge::ExecTask()
{
	LIST_MERGEDATA lstMergeData;  // 从数据库读取的合并数据列表
	int    nFailedNum = 0;        // 数据库获取数据失败次数
	bool   bExecResult = true;    // 处理结果

	/*
	int n_err = HandleDB::getClass().GetFirstClientDataInfo(&m_firstItem);
	if (n_err != SUCCESS)
	{
		return false;
	}

	bool bRet;
	if (m_firstItem.messageType == "G2S")
	{
		bRet = HandleGrib2Station(m_firstItem.mergeID);
	}
	else if (m_firstItem.messageType == "RAINPROC")
	{
		bRet = HandleRainProc(m_firstItem.mergeID);
	}
	else
	{
		bRet = HandleClientData();
	}
	return bRet;
	*/

	while (true)
	{
		// 获取要数据库内容
		int nErr = HandleDB::getClass().GetMergeData(m_stProduct.strRelatedKey, lstMergeData);
		if (nErr != 0)
		{
			// 读取数据库内容失败
			if (++nFailedNum > 3)
			{
				// 失败次数已经超过3次，结束本次任务
				LOG_(LOGID_ERROR, LOG_F("[TaskPID:%d][Key:%s] 获取数据库内容失败次数达到3次，结束本次任务."), m_nTaskPID, LOG_STR(m_stProduct.strKey));
				bExecResult = false;
				break;
			}

			// 延时2秒*失败次数，后继续执行
			sleep(2 * nFailedNum);
		}
		if (lstMergeData.size() == 0)
		{
			// 没有要处理的数据，结束本次任务
			bExecResult = true;
			break;
		}
		nFailedNum = 0;

		// 进行数据处理
		LIST_MERGEDATA::iterator iter;
		for (iter = lstMergeData.begin(); iter != lstMergeData.end(); iter++)
		{
			ST_MergeData stMergeData = *iter;

			if (stMergeData.strMergeType == "G2S")
			{
				// 格点转站点处理
				bExecResult = HandleGrib2Station(stMergeData);
			}
			else if (stMergeData.strMergeType == "RAINPROC")
			{
				// 降水请求处理
				bExecResult = HandleRainProc(stMergeData);
			}
			else if (stMergeData.strMergeType == "MERGE")
			{
				// 上传格点的拼图处理
				bExecResult = HandleClientData(stMergeData);
			}
			else
			{
				// 未知类型
			}

			// 延时处理
			msleep(50);
		}

		// 清空处理列表
		lstMergeData.clear();
	}

	// 任务结束返回
	return bExecResult;
}

/************************************************************************/
/* 对客户端发来的数据进行合并                                           */
/************************************************************************/
bool TaskProductClientMerge::HandleClientData(ST_MergeData stMergeData)
{
	LOG_(LOGID_INFO, LOG_F("[TaskPID:%d][Key:%s] 进行客户端（Client:%d,%s）的拼图处理."), m_nTaskPID, LOG_STR(m_stProduct.strKey), stMergeData.nClientID, LOG_STR(stMergeData.strProductKey));

	// 获取要合并的基本信息，从数据库中（可能会多条）
	// 注意：合并的数据必须为等间距
	int    nFailedNum = 0;        // 数据库获取数据失败次数
	float* fMaskData = NULL;      // Mask数据
	float* fSrcData = NULL;       // 源数据
	float* fClientData = NULL;    // 客户端数据
	int    nSrcDataLen;           // 源数据长度
	int    nClientDataLen;        // 客户端数据长度
	bool   bExecResult = true;    // 处理结果
	int    nErr = 0;              // 调用函数返回结果

	float fMaskLon, fMaskLat;     // 要合并数据的坐标点位置
	int nSrcMergePos, nClientMergePos; // 元数据和客户端数据合并位置点

	// 获取Mask数据
	fMaskData = CMask::getClass().GetMaskData(m_stProduct.stMask.strMaskFile);
	if (fMaskData == NULL)
	{
		// todo Mask数据获取失败，无法进行数据合并处理
		return false;
	}

	// 元数据长度固定
	nSrcDataLen = m_stProduct.nNi * m_stProduct.nNj;

	// 进行数据合并处理
	stMergeData.tStartTime = QDateTime::currentDateTime(); // 开始时间

	// 验证是否配置中允许合并
	if (!IsAllowMerge(stMergeData.nClientID))
	{
		// 如果不允许，修改数据库状态，跳过此条处理
		stMergeData.strMergeStatus = ABNORMAL;
		stMergeData.strErrReason = "Not in the configuration list";

		stMergeData.tFinishTime = QDateTime::currentDateTime(); // 结束时间

		// 将处理结果存储数据库
		nErr = HandleDB::getClass().UpdateMergeResult(stMergeData);
		if (nErr != 0)
		{
			LOG_(LOGID_ERROR, LOG_F("[TaskPID:%d][Key:%s] 更新数据库结果失败（MergeID=%d）,ErrCode=%d."), m_nTaskPID, LOG_STR(m_stProduct.strKey), stMergeData.nMergeID, nErr);
		}

		return false;
	}

	// 客户端格点数和数据长度
	int nClientNi = (int)((stMergeData.fLon2 - stMergeData.fLon1) / m_stProduct.fDi + 0.1) + 1;
	int nClientNj = (int)((stMergeData.fLat2 - stMergeData.fLat1) / m_stProduct.fDj + 0.1) + 1;
	nClientDataLen = nClientNi * nClientNj;

	// 该数据为符合条件的数据，开始执行数据合并处理
	// 获取原始数据，模版数据
	nwfd_grib2_field * stDataField = NULL;
	int nCnt = 0; // 客户端发送来的数据肯定为1
	fSrcData = GetSrcData(stMergeData.nYear, stMergeData.nMonth, stMergeData.nDay, stMergeData.nHour, stMergeData.nMinute,
		stMergeData.nSecond, stMergeData.nForecastTime, stMergeData.nTimeRange, stDataField, nCnt);
	if (fSrcData == NULL)
	{
		LOG_(LOGID_ERROR, LOG_F("[TaskPID:%d][Key:%s] 获取原始数据失败（Clinet:%d,%s）."), m_nTaskPID, LOG_STR(m_stProduct.strKey), stMergeData.nClientID, LOG_STR(stMergeData.strProductKey));
				
		// 原始数据获取失败，无法进行数据合并处理
		stMergeData.strMergeStatus = ABNORMAL;
		stMergeData.strErrReason = "get src Data failed";

		stMergeData.tFinishTime = QDateTime::currentDateTime(); // 结束时间

		// 将处理结果存储数据库
		nErr = HandleDB::getClass().UpdateMergeResult(stMergeData);
		if (nErr != 0)
		{
			LOG_(LOGID_ERROR, LOG_F("[TaskPID:%d][Key:%s] 更新数据库结果失败（MergeID=%d）,ErrCode=%d."), m_nTaskPID, LOG_STR(m_stProduct.strKey), stMergeData.nMergeID, nErr);
		}

		// 返回
		if (stDataField)
		{
			free(stDataField);
			stDataField = NULL;
		}

		return false;
	}

	// 获取要合并的客户端数据
	fClientData = GetClientData(stMergeData.nClientID, stMergeData.strProductKey, stMergeData.nYear, stMergeData.nMonth, stMergeData.nDay, stMergeData.nHour, stMergeData.nMinute,
		stMergeData.nSecond, stMergeData.nForecastTime, stMergeData.nTimeRange);
	if (fClientData == NULL)
	{
		LOG_(LOGID_ERROR, LOG_F("[TaskPID:%d][Key:%s] 获取原始数据失败（Clinet:%d，%s）."), m_nTaskPID, LOG_STR(m_stProduct.strKey), stMergeData.nMergeID, LOG_STR(stMergeData.strProductKey));

		// 客户端缓存数据获取失败，无法进行数据合并处理
		stMergeData.strMergeStatus = ABNORMAL;
		stMergeData.strErrReason = "Get Client Cached data failed";

		stMergeData.tFinishTime = QDateTime::currentDateTime(); // 结束时间

		// 将处理结果存储数据库
		nErr = HandleDB::getClass().UpdateMergeResult(stMergeData);
		if (nErr != 0)
		{
			LOG_(LOGID_ERROR, LOG_F("[TaskPID:%d][Key:%s] 更新数据库结果失败（MergeID=%d）,ErrCode=%d."), m_nTaskPID, LOG_STR(m_stProduct.strKey), stMergeData.nMergeID, nErr);
		}

		// 删除元数据
		if (fSrcData)
		{
			free(fSrcData);
			fSrcData = NULL;
		}
		if (stDataField)
		{
			free(stDataField);
			stDataField = NULL;
		}

		// 返回
		return false;
	}

	// 执行合并处理
	if (stMergeData.nClientID % 100 == 0)
	{
		// 特殊处理，该客户端为省级客户端，不按照Mask进行拼图，而是将数据覆盖到模版数据上，此客户端要配置到list最开始

		for (int k = 0; k < nCnt; ++k)
		{
			
			int nOffset = k* m_stProduct.nNi * m_stProduct.nNj;
			for (int nPos = 0; nPos < m_stProduct.nNi * m_stProduct.nNj; nPos++)
			{
				// 此位置为要合并的数据，计算该点的经纬度
				GetLonLat(m_stProduct.fLon1, m_stProduct.fLat1, m_stProduct.fDi, m_stProduct.fDj, m_stProduct.nNi, m_stProduct.nNj, nPos, fMaskLon, fMaskLat);

				// 通过经纬度计算客户端的坐标
				nClientMergePos = GetMergerPos(fMaskLon, fMaskLat, stMergeData.fLon1, stMergeData.fLat1, nClientNi);

				if (nClientMergePos >= 0 && nClientMergePos < nClientDataLen)
				{
					// 数据合并
					fSrcData[nOffset + nPos] = fClientData[nOffset + nClientMergePos];
				}
			}
		}

	
	}
	else
	{
		// 执行合并处理

		for (int k = 0; k < nCnt; ++k)
		{
			
			int nOffset = k*m_stProduct.stMask.nNi * m_stProduct.stMask.nNj;
			for (int nPos = 0; nPos < m_stProduct.stMask.nNi * m_stProduct.stMask.nNj; nPos++)
			{
				// 查找要合并的位置点： stMergeData.nClientID 与 Mask文件中MaskID一致
				if ((int)(fMaskData[nPos] + 0.1) != stMergeData.nClientID)
				{
					continue;
				}

				// 此位置为要合并的数据，计算该点的经纬度
				GetMaskLonLat(m_stProduct.stMask, nPos, fMaskLon, fMaskLat);

				// 通过经纬度获取要合并数据的位置点
				nSrcMergePos = GetMergerPos(fMaskLon, fMaskLat, m_stProduct.fLon1, m_stProduct.fLat1, m_stProduct.nNi);
				nClientMergePos = GetMergerPos(fMaskLon, fMaskLat, stMergeData.fLon1, stMergeData.fLat1, nClientNi);

				if ((nSrcMergePos >= 0 && nSrcMergePos < nSrcDataLen) && (nClientMergePos >= 0 && nClientMergePos < nClientDataLen))
				{
					// 数据合并
					fSrcData[nOffset + nSrcMergePos] = fClientData[nOffset + nClientMergePos];
				}
				else
				{
					// 该点越界 todo
					LOG_(LOGID_ERROR, LOG_F("[TaskPID:%d][Key:%s] 客户端长传数据合并时该点（%f,%f）数据导致元数据或客户端数据越界"), m_nTaskPID, LOG_STR(m_stProduct.strKey), fMaskLon, fMaskLat);
				}
			}
		}


		
	}
	stMergeData.tFinishTime = QDateTime::currentDateTime();  // 结束时间

	// 合并完成后，记录数据库
	stMergeData.strMergeStatus = COMPLETE;

	// 将处理结果存储数据库
	nErr = HandleDB::getClass().UpdateMergeResult(stMergeData);
	if (nErr != 0)
	{
		LOG_(LOGID_ERROR, LOG_F("[TaskPID:%d][Key:%s] 更新数据库结果失败（MergeID=%d）,ErrCode=%d."), m_nTaskPID, LOG_STR(m_stProduct.strKey), stMergeData.nMergeID, nErr);
	}

	// 数据写入Cached中(不进行文件保存)
	bool bRet = HandleComplete(fSrcData, nSrcDataLen*m_stProduct.nCnt, m_stProduct.nCnt, stMergeData.nYear, stMergeData.nMonth, stMergeData.nDay, stMergeData.nHour, stMergeData.nMinute,
		stMergeData.nSecond, stMergeData.nForecastTime, stMergeData.nTimeRange, 2);
	if (bRet)
	{
		// 缓存保存成功，向各个客户端进行广播，发布拼图处理消息
		NetworkManager::getClass().PublishGrid(stMergeData.strProductKey, stMergeData.nYear, stMergeData.nMonth, stMergeData.nDay, stMergeData.nHour, stMergeData.nMinute,
			stMergeData.nSecond, stMergeData.nForecastTime, stMergeData.nTimeRange);

		// 如果配置需要插值成站点，则要进行插值处理
		if (!m_stProduct.stStationProc.bDisabled)
		{
			// 再插值站点
			nErr = DataManager::getClass().Grid2Station(m_stProduct.strType, m_stProduct.nTimeRange, stMergeData.strProductKey, fSrcData, m_stProduct.fLon1, m_stProduct.fLat1, m_stProduct.fDi, m_stProduct.fDj,
				m_stProduct.nNi, m_stProduct.nNj, stMergeData.nYear, stMergeData.nMonth, stMergeData.nDay, stMergeData.nHour, stMergeData.nForecastTime);
			if (nErr == SUCCESS)
			{
				// 站点转换成功
				LOG_(LOGID_DEBUG, LOG_F("[拼图][Key:%s]格点-站点同步成功."),  LOG_STR(stMergeData.strProductKey));
			}
			else
			{
				// 站点转化失败
				LOG_(LOGID_DEBUG, LOG_F("[拼图][Key:%s]格点-站点同步失败，错误编码：%d."), LOG_STR(stMergeData.strProductKey), nErr);
			}

			// 保存多站点数据
			nErr = DataManager::getClass().SaveMultiStation(m_stProduct.strType, stMergeData.nYear, stMergeData.nMonth, stMergeData.nDay, stMergeData.nHour, m_stProduct.nTimeRange, m_stProduct.nMaxForecastTime);
			if (nErr == SUCCESS)
			{
				// todo 处理成功
				LOG_(LOGID_DEBUG, LOG_F("[拼图][Key:%s]保存多站点数据成功."), LOG_STR(stMergeData.strProductKey));
			}
			else
			{
				// todo 处理失败
				LOG_(LOGID_DEBUG, LOG_F("[拼图][Key:%s]保存多站点数据失败，错误编码：%d."), LOG_STR(stMergeData.strProductKey), nErr);
			}
		}

	}

	// 降水处理，如：24小时降水到3小时降水
	if (m_stProduct.lstRainProc.size() != 0 )
	{
		// 24小时降水，且配置了3小时降水的Key,才进行以下操作
		HandleRainProc(fSrcData, stMergeData.nYear, stMergeData.nMonth, stMergeData.nDay, stMergeData.nHour, stMergeData.nMinute, stMergeData.nSecond, stMergeData.nForecastTime);
	}

	// 释放获取的数据空间
	if (fSrcData)
	{
		free(fSrcData);
		fSrcData = NULL;
	}
	if (fClientData)
	{
		free(fClientData);
		fClientData = NULL;
	}
	if (stDataField)
	{
		free(stDataField);
		stDataField = NULL;
	}

	LOG_(LOGID_DEBUG, LOG_F("数据合并成功：[Key:%s][ProductKey:%s][ClientID:%d]"), LOG_STR(m_stProduct.strKey), LOG_STR(m_stProduct.strRelatedKey), stMergeData.nClientID);

	return true;
}

/************************************************************************/
/* 降水处理                                                             */
/************************************************************************/
bool TaskProductClientMerge::HandleRainProc(ST_MergeData stMergeData)
{
	// 缓存连接判断
	if (m_pCachedImpl == NULL)
	{
		// 客户端缓存数据获取失败，无法进行数据合并处理
		stMergeData.strMergeStatus = ABNORMAL;
		stMergeData.strErrReason = "CachedImpl is null";

		// 将处理结果存储数据库
		HandleDB::getClass().UpdateMergeResult(stMergeData);

		//HandleDB::getClass().UpdateMergeStatus(ABNORMAL, stMergeData.nMergeID);
		return false;
	}

	// 降水关联处理
	QStringList lstRelatedProduct = stMergeData.strRelatedProductKey.split(",");
	for (QStringList::iterator iter = lstRelatedProduct.begin(); iter != lstRelatedProduct.end(); iter++)
	{
		QString strRelatedKey = *iter;

    	ST_PRODUCT stRelatedProduct;
		if (!NetworkManager::getClass().GetProduct(strRelatedKey, stRelatedProduct))
		{
			// 没有此产品
			continue;
		}

		if (stRelatedProduct.nTimeRange < stMergeData.nTimeRange)
		{
			// 向下处理-拆分
			for (int nForecast = stMergeData.nStartForecastTime; nForecast <= stMergeData.nForecastTime; nForecast += stRelatedProduct.nTimeRange)
			{
				HandleRainCorrection_Downward(m_pCachedImpl, m_stProduct, stRelatedProduct, stMergeData.nYear, stMergeData.nMonth, stMergeData.nDay,
					stMergeData.nHour, stMergeData.nMinute, stMergeData.nSecond, nForecast);
			}
		}
		else if (stRelatedProduct.nTimeRange > stMergeData.nTimeRange)
		{
			// 向上处理-合并
			int nRelatedStartForecast = ((stMergeData.nStartForecastTime - stMergeData.nTimeRange) / stRelatedProduct.nTimeRange) * stRelatedProduct.nTimeRange;
			int nRelatedEndForecast = ((stMergeData.nForecastTime - stMergeData.nTimeRange) / stRelatedProduct.nTimeRange) * stRelatedProduct.nTimeRange;

			for (int nForecast = nRelatedStartForecast; nForecast <= nRelatedEndForecast; nForecast += stRelatedProduct.nTimeRange)
			{
				HandleRainCorrection_Upward(m_pCachedImpl, m_stProduct, stRelatedProduct, stMergeData.nYear, stMergeData.nMonth, stMergeData.nDay,
					stMergeData.nHour, stMergeData.nMinute, stMergeData.nSecond, nForecast);
			}
		}
	}

	// 保存结果
	int nerr = HandleDB::getClass().UpdateMergeStatus(COMPLETE, stMergeData.nMergeID);
	if (nerr != SUCCESS)
	{
		return false;
	}
	
	return true;
}

/************************************************************************/
/* 格点转站点处理                                                       */
/************************************************************************/
bool TaskProductClientMerge::HandleGrib2Station(ST_MergeData stMergeData)
{
	// 缓存连接判断
	if (m_pCachedImpl == NULL)
	{
		// 客户端缓存数据获取失败，无法进行数据合并处理
		stMergeData.strMergeStatus = ABNORMAL;
		stMergeData.strErrReason = "CachedImpl is null";

		// 将处理结果存储数据库
		HandleDB::getClass().UpdateMergeResult(stMergeData);

		//HandleDB::getClass().UpdateMergeStatus(ABNORMAL, stMergeData.nMergeID);
		return false;
	}
	
	float * fData = NULL;
	int nDataLen = 0;
	char szTime[50];
	int nErr;

	// 按照预报时效进行处理
	for (int nForecastTime = stMergeData.nStartForecastTime; nForecastTime <= stMergeData.nForecastTime; nForecastTime += stMergeData.nTimeRange)
	{
		memset(szTime, 0x00, sizeof(szTime));
		sprintf(szTime, CACHED_TIME_FORMAT, stMergeData.nYear, stMergeData.nMonth, stMergeData.nDay, stMergeData.nHour, nForecastTime, stMergeData.nTimeRange);

		// 获取最新的拼图数据
		nErr = m_pCachedImpl->GetMergeData(stMergeData.strProductKey.toLocal8Bit().data(), szTime, fData, nDataLen);
		if (nErr != SUCCESS || fData == NULL)
		{
			// 获取数据失败
			continue;
		}

	    // 执行格点到站点的插值
		nErr = DataManager::getClass().Grid2Station(m_stProduct.strType, stMergeData.nTimeRange, stMergeData.strProductKey, fData, stMergeData.fLon1, stMergeData.fLat1, m_stProduct.fDi, m_stProduct.fDj, m_stProduct.nNi, m_stProduct.nNj,
			stMergeData.nYear, stMergeData.nMonth, stMergeData.nDay, stMergeData.nHour, nForecastTime);
		if (nErr != SUCCESS)
		{
			// 更新数据库状态
			HandleDB::getClass().UpdateMergeStatus(ABNORMAL, stMergeData.nMergeID);
			return false;
		}

	    // 释放数据空间
		if (fData)
		{
			free(fData);
			fData = NULL;
		}
	}

	// 保存多站点数据到缓存
	nErr = DataManager::getClass().SaveMultiStation(m_stProduct.strType, stMergeData.nYear, stMergeData.nMonth, stMergeData.nDay, stMergeData.nHour, 
	stMergeData.nTimeRange, m_stProduct.nMaxForecastTime);
	if (nErr != SUCCESS)
	{
		HandleDB::getClass().UpdateMergeStatus(ABNORMAL, stMergeData.nMergeID);
		return false;
	}

	//处理完任务，更新数据库任务队列
	nErr = HandleDB::getClass().UpdateMergeStatus(COMPLETE, stMergeData.nMergeID);

	if (nErr != SUCCESS)
	{
		return false;
	}
	
	return true;
}

/************************************************************************/
/* 获取模版数据                                                         */
/************************************************************************/
float * TaskProductClientMerge::GetSrcData(int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nForecastTime, int nTimeRange, nwfd_grib2_field* &stDataField, int &nCnt)
{
	float *fSrcData = NULL;
	int  nDataLen;
	nCnt = 1;
	// 产品缓存时间
	QString strProductTime = HandleCommon::GetCachedProductTime(nYear, nMonth, nDay, nHour, nForecastTime, nTimeRange);
	
	// 1、获取缓存Merge的最新数据
	if (m_pCachedImpl)
	{
		ST_PRODUCT stProduct;
		int nErr = m_pCachedImpl->GetMergeData(m_stProduct.strRelatedKey.toLocal8Bit().data(), strProductTime.toLocal8Bit().data(), fSrcData, nDataLen);
		int nErr2 = m_pCachedImpl->GetProductInfo(m_stProduct.strRelatedKey.toLocal8Bit().data(), stProduct);
		
		if (nErr2 == SUCCESS && nErr == SUCCESS && fSrcData != NULL)
		{
			//判断是否是风，风有2个维度
			nCnt = (stProduct.nElement[1] != -1) ? 2 : 1;
			return fSrcData;
		}

		if (nErr2 == SUCCESS)
		{
			
			nCnt = (stProduct.nElement[1] != -1) ? 2 : 1;
		}
	}

	//// 2、缓存中没有，读取上次保存文件的数据
	//QString strGrib2FileName = HandleCommon::GetFileName(m_stProduct.stFileSave.strFileName, nYear, nMonth, nDay, nHour, nForecastTime, nTimeRange);
	//QString strSrcFile = QString("%1/%2").arg(m_stProduct.stFileSave.strSavePath).arg(strGrib2FileName);
	//fSrcData = HandleGrib::getClass().GetNwfdData(strSrcFile, 0, stDataField, nCnt);
	//if (fSrcData != NULL)
	//{
	//	return fSrcData;
	//}

	// 3、根据模版类型定义，进行数据构造
	if (m_stProduct.stTemplate.strTemplateType.toLower() == "blank")
	{
		// 数据模版类型-缺测数据
		nDataLen = m_stProduct.nNi * m_stProduct.nNj*nCnt;
		float * fTemplateData = (float *)malloc(nDataLen * sizeof(float));
		if (fTemplateData == NULL)
		{
			return NULL;
		}

		// 数据模版类型为空白数据-构造数据
		
		for (int k = 0; k < nCnt; ++k)
		{
			int nOff = 0;
			for (int i = 0; i < m_stProduct.nNi; i++)
			{
				for (int j = 0; j < m_stProduct.nNj; j++)
				{
					fTemplateData[nOff++] = m_stProduct.stTemplate.fBlankValue; // 默认缺测值
				}
			}
		}
		
		
		

		return fTemplateData;
	}
	else
	{
		// 数据模版类型-非空白数据-获取指导报数据
		if (m_pCachedImpl)
		{
			ST_PRODUCT stProduct;
			int nErr = m_pCachedImpl->GetProductData(m_stProduct.strRelatedKey.toLocal8Bit().data(), strProductTime.toLocal8Bit().data(), fSrcData, nDataLen);
			int nErr2 = m_pCachedImpl->GetProductInfo(m_stProduct.strRelatedKey.toLocal8Bit().data(), stProduct);

			if (nErr2 == SUCCESS && nErr == SUCCESS && fSrcData != NULL)
			{

				//判断是否是风，风有2个维度
				nCnt = (stProduct.nElement[1] != -1) ? 2 : 1;
				return fSrcData;
			}
		}
	}

	// 补充 - 获取原数据失败，使用缺测值填充
	if (fSrcData == NULL)
	{
		// 数据模版类型-缺测数据
		nDataLen = m_stProduct.nNi * m_stProduct.nNj*nCnt;
		fSrcData = (float *)malloc(nDataLen * sizeof(float));
		if (fSrcData == NULL)
		{
			return NULL;
		}

		// 数据模版类型为空白数据-构造数据

		for (int k = 0; k < nCnt; ++k)
		{
			int nOff = 0;
			for (int i = 0; i < m_stProduct.nNi; i++)
			{
				for (int j = 0; j < m_stProduct.nNj; j++)
				{
					fSrcData[nOff++] = m_stProduct.stTemplate.fBlankValue; // 默认缺测值
				}
			}
		}
	

		return fSrcData;
	}

	return NULL;
}

/************************************************************************/
/* 获取客户端数据                                                       */
/************************************************************************/
float * TaskProductClientMerge::GetClientData(int nClientID, QString strProductKey,
	int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nForecastTime, int nTimeRange)
{
	// 5. 检查缓存连接
	if (m_pCachedImpl == NULL)
	{
		return NULL;
	}
	if (m_pCachedImpl->ConnectStatus() != SUCCESS)
	{
		m_pCachedImpl->ReConnect();
	}

	// 4. 获取缓存数据
	int nDataSize;
	float fLon1tmp, fLon2tmp, fLat1tmp, fLat2tmp;
	float * fData = NULL;
	char szTime[256];
	memset(szTime, 0x00, sizeof(szTime));
	sprintf(szTime, CACHED_TIME_FORMAT, nYear, nMonth, nDay, nHour, nForecastTime, nTimeRange);
	int nErr = m_pCachedImpl->GetClientData(nClientID, strProductKey.toLocal8Bit().data(), szTime, fData, nDataSize, fLon1tmp, fLon2tmp, fLat1tmp, fLat2tmp);
	if (nErr != SUCCESS || fData == NULL)
	{
		return NULL;
	}

	return fData;
}

/************************************************************************/
/* 获取Mask某个点的坐标                                                 */
/************************************************************************/
bool TaskProductClientMerge::GetMaskLonLat(ST_MASK stMask, int nPos, float &fLon, float &fLat)
{
	// 计算得到格点中的位置
	int nRow = nPos / stMask.nNi;
	int nCol = nPos % stMask.nNi;

	// 计算该位置的经纬度坐标
	fLon = stMask.fLon1 + nCol * stMask.fDi;
	fLat = stMask.fLat1 + nRow * stMask.fDj;

	return true;
}

/************************************************************************/
/* 获取Mask某个点的坐标                                                 */
/************************************************************************/
bool TaskProductClientMerge::GetLonLat(float fLon1, float fLat1, float fDi, float fDj, int nNi, int nNj, int nPos, float &fLon, float &fLat)
{
	// 计算得到格点中的位置
	int nRow = nPos / nNi;
	int nCol = nPos % nNi;

	// 计算该位置的经纬度坐标
	fLon = fLon1 + nCol * fDi;
	fLat = fLat1 + nRow * fDj;

	return true;
}

/************************************************************************/
/* 获取某坐标点的位置                                                   */
/************************************************************************/
int TaskProductClientMerge::GetMergerPos(float fMaskLon, float fMaskLat, float fStartLon, float fStartLat, int nNi)
{
	int nRow = (int)((fMaskLat - fStartLat) / m_stProduct.fDj + 0.1);
	int nCol = (int)((fMaskLon - fStartLon) / m_stProduct.fDi + 0.1);

	int nPos = nRow * nNi + nCol;

	return nPos;
}

/************************************************************************/
/* 处理完成的结果-存储到文件，存储到缓存中                              */
/************************************************************************/
bool TaskProductClientMerge::HandleComplete(float *fData, int nDataLen, int nCnt, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nForecastTime, int nTimeRange, int nFlg)
{
	// nFlg = 1 : 只保存到文件中
	// nFlg = 2 : 只保存到缓存中
	// nFlg = 3 : 保存到文件和缓存中

	if (nFlg == 1 || nFlg == 3)
	{
		///* 1、将数据保存到Grib2文件中 */
		//if (m_stProduct.stFileSave.bDisabled)
		//{
		//	// 不进行文件存储
		//}
		//else
		//{
		//	QString strGrib2FileName = HandleCommon::GetFileName(m_stProduct.stFileSave.strFileName, nYear, nMonth, nDay, nHour, nForecastTime, nTimeRange);
		//	QString strGrib2FilePath = m_stProduct.stFileSave.strSavePath.replace("\\", "/");
		//	QString strGrib2File = QString("%1/%2").arg(strGrib2FilePath).arg(strGrib2FileName);
		//	QString strGrib2File_tmp = QString("%1.tmp").arg(strGrib2File);

		//	if (HandleCommon::HandleGribFolder(strGrib2FilePath))
		//	{
		//		int nErr = SaveData2GribFile(fData, nCnt, m_stProduct, nYear, nMonth, nDay, nHour, nForecastTime, nTimeRange, strGrib2File_tmp);
		//		if (nErr == 0)
		//		{
		//			if (HandleCommon::Rename(strGrib2File_tmp, strGrib2File))
		//			{
		//				LOG_(LOGID_DEBUG, LOG_F("生成Grib2文件成功：%s"), LOG_STR(strGrib2File));
		//			}
		//		}
		//		else
		//		{
		//			LOG_(LOGID_DEBUG, LOG_F("[WARNING]生成Grib2文件失败,错误编码=%d"), nErr);
		//		}
		//	}
		//	else
		//	{
		//		// 创建目录失败
		//	}
		//}
	}

	if (nFlg == 2 || nFlg == 3)
	{
		/* 2、进行缓存存储 */
		if (m_stProduct.stCachedSave.bDisabled)
		{
			// 不存储
			LOG_(LOGID_DEBUG, LOG_F("[Notice]根据配置文件中的配置，该产品不保存到缓存中."));
		}
		else if (m_pCachedImpl == NULL)
		{
			LOG_(LOGID_DEBUG, LOG_F("[Error][TaskPID:%d]未创建Cached连接"), m_nTaskPID);
			return false;
		}
		else
		{
			// 连接状态断开
			int nErr = m_pCachedImpl->ConnectStatus();
			if (nErr != SUCCESS)
			{
				// 重新建立连接
				m_pCachedImpl->ReConnect();
				// 获取连接状态
				nErr = m_pCachedImpl->ConnectStatus();
			}

			if (nErr == SUCCESS)
			{
				// 将nwfd数据存储到缓存中
				QString strMergeTime = HandleCommon::GetCachedProductTime(nYear, nMonth, nDay, nHour, nForecastTime, nTimeRange);  //QString("").sprintf("%04d%02d%02d%02d0000.%03d_%02d", );

				// 数据长度
				nErr = m_pCachedImpl->SaveMergeData(m_stProduct.strRelatedKey.toLocal8Bit().data(), strMergeTime.toLocal8Bit().data(), fData, nDataLen);

				if (nErr == SUCCESS)
				{
					// 存储成功
					LOG_(LOGID_DEBUG, LOG_F("[TaskPID:%d]数据缓存存储成功：（ProductTime_%s）."), m_nTaskPID, LOG_STR(strMergeTime));
				}
				else
				{
					// 存储失败
					LOG_(LOGID_DEBUG, LOG_F("[WARNING][TaskPID:%d][%s]数据缓存存储失败：（ProductTime_%s）."), m_nTaskPID,LOG_STR(m_stProduct.strKey), LOG_STR(strMergeTime));
					return false;
				}
			}
			else
			{
				LOG_(LOGID_DEBUG, LOG_F("[Error][TaskPID:%d]Cached连接状态断开"), m_nTaskPID);
				return false;
			}
		}
	}

	return true;
}

/************************************************************************/
/* 是否允许合并                                                         */
/************************************************************************/
bool TaskProductClientMerge::IsAllowMerge(int nClientID)
{
	LIST_MERGE_ITEM::iterator iter;
	for (iter = m_stProduct.lstMergeItem.begin(); iter != m_stProduct.lstMergeItem.end(); iter++)
	{
		ST_MERGE_ITEM item = *iter;
		if (item.nClientID == nClientID)
		{
			return true;
		}
	}

	return false;
}

/************************************************************************/
/* 24小时降水到3小时降水的调整                                          */
/************************************************************************/
bool TaskProductClientMerge::HandleRainProc(float *fData, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nForecastTime)
{
	LIST_RAIN_PROC::iterator iter;
	for (iter = m_stProduct.lstRainProc.begin(); iter != m_stProduct.lstRainProc.end(); iter++)
	{
		ST_RAIN_PROC stRainPoc = *iter;
		if (stRainPoc.nRange > m_stProduct.nTimeRange)
		{
			// 向上进行降水处理(累加)，eg. 3小时->24小时
			HandleRainCorrection_Upward(stRainPoc, fData, nYear, nMonth, nDay, nHour, nMinute, nSecond, nForecastTime);
		}
		else if (stRainPoc.nRange < m_stProduct.nTimeRange)
		{
			// 向下进行降水处理(细分)，eg. 24小时->3小时
			HandleRainCorrection_Downward(stRainPoc, fData, nYear, nMonth, nDay, nHour, nMinute, nSecond, nForecastTime);
		}
		else
		{
			continue;
		}
	}

	return true;
}

/************************************************************************/
/* 24小时降水到3小时降水的调整                                          */
/************************************************************************/
bool TaskProductClientMerge::HandleRainCorrection_Downward(ST_RAIN_PROC stRainPoc, float *fData, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nForecastTime)
{
	// 该方法是通过24降水对3小时指导报的数据进行调整
	// 假定24降水的格点数,经纬度范围与3小时的是一致的

	if (stRainPoc.strRelatedKey.isEmpty())
	{
		return false;
	}

	// 处理数据的个数，如24到3小时降水，就是8个
	int nTimeRange = stRainPoc.nRange;
	int nCnt = m_stProduct.nTimeRange / nTimeRange;
	int forecast = nTimeRange;

	//float * fDataRain[8];  // 3小时降水1天的数据
	float ** fDataRain = (float **)malloc(nCnt * sizeof(float *));
	if (fDataRain == NULL)
	{
		// 空间申请失败
		return false;
	}

	char szTime[50];
	int nDataLen;
	int nErr;
	bool bFlg = true;

	// 获取3小时降水数据
	int n = 0;
	for (n = 0; n < nCnt; n++)
	{
		fDataRain[n] = NULL;

		// 获取数据
		forecast = (nForecastTime - m_stProduct.nTimeRange) + n * nTimeRange + nTimeRange;
		memset(szTime, 0x00, sizeof(szTime));
		sprintf(szTime, CACHED_TIME_FORMAT, nYear, nMonth, nDay, nHour, forecast, nTimeRange);
		nErr = m_pCachedImpl->GetProductData(stRainPoc.strRelatedKey.toLocal8Bit().data(), szTime, fDataRain[n], nDataLen);
		if (nErr != SUCCESS || fDataRain[n] == NULL)
		{
			fDataRain[n] = (float *)malloc(m_stProduct.nNi * m_stProduct.nNj * sizeof(float));
			if (fDataRain[n] == NULL)
			{
				bFlg = false;
				break;
			}
			// 未取得数据，构造一个空数据
			for (int i = 0; i < m_stProduct.nNi * m_stProduct.nNj; i++)
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
	for (int i = 0; i < m_stProduct.nNi * m_stProduct.nNj; i++)
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
				fDataRain[n][i] *= fData[i] / fRainSum ;
			}
		}
	}

	// 处理结束，将3小时降水保存到缓存中
	for (n = 0; n < nCnt; n++)
	{
		// 时间
		forecast = (nForecastTime - m_stProduct.nTimeRange) + n * nTimeRange + nTimeRange;
		memset(szTime, 0x00, sizeof(szTime));
		sprintf(szTime, CACHED_TIME_FORMAT, nYear, nMonth, nDay, nHour, forecast, nTimeRange);

		// 数据长度
		nErr = m_pCachedImpl->SaveMergeData(stRainPoc.strRelatedKey.toLocal8Bit().data(), szTime, fDataRain[n], m_stProduct.nNi * m_stProduct.nNj);
		if (nErr == SUCCESS)
		{
			// 存储成功
			LOG_(LOGID_DEBUG, LOG_F("[%s]数据缓存存储成功：（ProductTime_%s）."), LOG_STR(stRainPoc.strRelatedKey), LOG_STR(QString::fromLocal8Bit(szTime)));

			// 缓存保存成功，向各个客户端进行广播，发布3小时降水拼图处理消息
			// ############# todo ##############
			NetworkManager::getClass().PublishGrid(stRainPoc.strRelatedKey, nYear, nMonth, nDay, nHour, 0, 0, forecast, nTimeRange);
		}
		else
		{
			// 存储失败
			LOG_(LOGID_DEBUG, LOG_F("[WARNING][%s]数据缓存存储失败：（ProductTime_%s）."), LOG_STR(stRainPoc.strRelatedKey), LOG_STR(QString::fromLocal8Bit(szTime)));
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

	return true;
}

/************************************************************************/
/* 3小时降水到24小时降水的调整                                          */
/************************************************************************/
bool TaskProductClientMerge::HandleRainCorrection_Upward(ST_RAIN_PROC stRainPoc, float *fData, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nForecastTime)
{
	// 假定24降水的格点数,经纬度范围与3小时的是一致的
	if (stRainPoc.strRelatedKey.isEmpty())
	{
		return false;
	}

	// 处理数据的个数，如3到24小时降水，就是8个
	int nCnt = stRainPoc.nRange / m_stProduct.nTimeRange;
	int nTimeRange = m_stProduct.nTimeRange;
	int forecast = m_stProduct.nTimeRange;

	// 保存当前处理Key的元数据
	float ** fDataRain = (float **)malloc(nCnt * sizeof(float *));
	if (fDataRain == NULL)
	{
		// 空间申请失败
		return false;
	}
	
	// 保存合并数据
	float * fUpDataRain = (float *)malloc(m_stProduct.nNi * m_stProduct.nNj * sizeof(float));
	if (fUpDataRain == NULL)
	{
		free(fDataRain);
		// 空间申请失败
		return false;
	}

	char szTime[50];
	int nErr;
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
		fDataRain[n] = GetSrcData(nYear, nMonth, nDay, nHour, nMinute, nSecond, forecast, nTimeRange, field, nDataCnt);
		if (field)
		{
			free(field);
		}
	}

	// 累加数据
	float fRainSum;
	for (int i = 0; i < m_stProduct.nNi * m_stProduct.nNj; i++)
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
			if (fDataRain[n][i] > 9998)
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
	sprintf(szTime, CACHED_TIME_FORMAT, nYear, nMonth, nDay, nHour, forecast, stRainPoc.nRange);

	// 数据长度
	nErr = m_pCachedImpl->SaveMergeData(stRainPoc.strRelatedKey.toLocal8Bit().data(), szTime, fDataRain[n], m_stProduct.nNi * m_stProduct.nNj);
	if (nErr == SUCCESS)
	{
		// 存储成功
		LOG_(LOGID_DEBUG, LOG_F("[%s]数据缓存存储成功：（ProductTime_%s）."), LOG_STR(stRainPoc.strRelatedKey), LOG_STR(QString::fromLocal8Bit(szTime)));

		// 缓存保存成功，向各个客户端进行广播，发布24小时降水拼图处理消息
		// ############# todo ##############
		NetworkManager::getClass().PublishGrid(stRainPoc.strRelatedKey, nYear, nMonth, nDay, nHour, 0, 0, forecast, stRainPoc.nRange);
	}
	else
	{
		// 存储失败
		LOG_(LOGID_DEBUG, LOG_F("[WARNING][%s]数据缓存存储失败：（ProductTime_%s）."), LOG_STR(stRainPoc.strRelatedKey), LOG_STR(QString::fromLocal8Bit(szTime)));
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

bool TaskProductClientMerge::HandleRainCorrection_Upward(CachedImpl * pCachedImpl, ST_PRODUCT stProduct, ST_PRODUCT stRelatedProduct, int nYear, int nMonth,
	int nDay, int nHour, int nMinute, int nSecond, int nForecastTime)
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
			if (IS_EQUAL(fDataRain[n][i], 9999.0))
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

bool TaskProductClientMerge::HandleRainCorrection_Downward(CachedImpl * pCachedImpl, ST_PRODUCT stProduct, ST_PRODUCT stRelatedProduct, int nYear, int nMonth,
	int nDay, int nHour, int nMinute, int nSecond, int nForecastTime)
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
