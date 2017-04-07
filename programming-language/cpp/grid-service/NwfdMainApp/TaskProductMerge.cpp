#include "log.h" // log日志
#include "TaskProductMerge.h"
#include "HandleDB.h"
#include "HandleGrib.h"
#include "HandleCommon.h"
#include "Mask.h"
#include "DDSManager.h"
#include <QDir>
#include <QDateTime>
#include <QFileInfoList>

/************************************************************************/
/* 构造函数&析构函数                                                    */
/************************************************************************/
TaskProductMerge::TaskProductMerge()
{
	LOG_(LOGID_DEBUG, LOG_F("Entry TaskProductMerge()"));
	m_strTaskName = "产品拼图处理任务";

	// 初始化状态为僵死，不初始化不予执行任务
	m_emTaskState = _TASK_DEAD;

	m_strClearHasFlg = "";
	m_pCachedImpl = NULL;
	LOG_(LOGID_DEBUG, LOG_F("Leave TaskProductMerge()"));
}

TaskProductMerge::~TaskProductMerge()
{
	LOG_(LOGID_DEBUG, LOG_F("Entry ~TaskProductMerge()"));
	// 停止线程执行
	m_bReady = false;
	wait();
	LOG_(LOGID_DEBUG, LOG_F("Leave ~TaskProductMerge()"));
}

/************************************************************************/
/* 任务处理执行方法                                                     */
/************************************************************************/
void TaskProductMerge::run()
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
			ExecMergeTask();

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
void TaskProductMerge::Init(ST_PRODUCT stProduct, ST_CACHED stCached)
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

/************************************************************************/
/* 初始化缓存数据                                                       */
/************************************************************************/
bool TaskProductMerge::InitCached()
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
bool TaskProductMerge::ExecMergeTask()
{
	// 处理标记
	HandleClearHash();

	// 优先处理客户端数据， 对客户端发来的数据进行合并  
	// HandleClientData();

	// 扫描本地目录，进行数据合并  
	//HandleLocalData();

	// 对拼图处理进行调整，拼图处理：定点处理，将结果返回国家局
	// 直接从缓存的拼图块中取出数据写入文件中即可
	// 可以根据配置是写入单个Grib2文件还是，多个Grib2文件合并写入
	// 割客户端上传的数据，已经在实时拼图（TaskProductClientMerge）处理完成
	if (m_stProduct.strScanMode.isEmpty())
	{
		// 未设置扫描模式，从缓存中直接读取
		HandleMergeData();
	}
	else
	{
		// 设置缓存模式，从文件中提取数据进行拼图
		HandleLocalDataFromMode();
	}

	return true;
}

/************************************************************************/
/* 对客户端发来的数据进行合并                                           */
/************************************************************************/
bool TaskProductMerge::HandleClientData()
{
	// 获取要合并的基本信息，从数据库中（可能会多条）
	// 注意：合并的数据必须为等间距
	LIST_MERGEDATA lstMergeData;  // 从数据库读取的合并数据列表
	int    nFailedNum = 0;        // 数据库获取数据失败次数
	float* fMaskData = NULL;      // Mask数据
	float* fSrcData = NULL;       // 源数据
	float* fClientData = NULL;    // 客户端数据
	int    nSrcDataLen;           // 源数据长度
	int    nClientDataLen;        // 客户端数据长度
	bool   bExecResult = true;    // 处理结果

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

	while (true)
	{
		// 获取要合并的数据
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

		// 进行数据合并处理
		LIST_MERGEDATA::iterator iter;
		for (iter = lstMergeData.begin(); iter != lstMergeData.end(); iter++)
		{
			ST_MergeData stMergeData = *iter;

			stMergeData.tStartTime = QDateTime::currentDateTime(); // 开始时间

			// 客户端格点数和数据长度
			int nClientNi = (int)((stMergeData.fLon2 - stMergeData.fLon1) / m_stProduct.fDi + 0.1) + 1;
			int nClientNj = (int)((stMergeData.fLat2 - stMergeData.fLat1) / m_stProduct.fDj + 0.1) + 1;
			nClientDataLen = nClientNi * nClientNj;

			//// 判断是否对此客户端的数据进行合并
			//if (!m_stProduct.lstMergeItem.contains(stMergeData.nClientID))
			//{
			//	LOG_(LOGID_INFO, LOG_F("[TaskPID:%d][Key:%s] 此数据根据配置不进行合并（ClientID:%d,%s）."), m_nTaskPID, LOG_STR(m_stProduct.strKey), stMergeData.nClientID, LOG_STR(stMergeData.strProductKey));
			//	
			//	// 不对此客户端的数据合并
			//	stMergeData.strMergeStatus = MERGE_ABNORMAL;
			//	stMergeData.strErrReason = "do not process this client data ";

			//	//// 删除客户端上传的缓存数据
			//	//nErr = DelelteCachedClientData(stMergeData.nClientID, stMergeData.strProductKey);
			//	//if (nErr != 0)
			//	//{
			//	//	LOG_(LOGID_ERROR, LOG_F("[TaskPID:%d][Key:%s] 删除客户端缓存数据失败（ClientID=%d,%s）,ErrCode=%d."), m_nTaskPID, LOG_STR(m_stProduct.strKey), stMergeData.nClientID, stMergeData.strProductKey, nErr);
			//	//	stMergeData.strErrReason += ",delete calched client data failed";
			//	//}

			//	stMergeData.tFinishTime = QDateTime::currentDateTime(); // 结束时间

			//	// 将处理结果存储数据库
			//	nErr = HandleDB::getClass().UpdateMergeResult(stMergeData);
			//	if (nErr != 0)
			//	{
			//		LOG_(LOGID_ERROR, LOG_F("[TaskPID:%d][Key:%s] 更新数据库结果失败（MergeID=%d）,ErrCode=%d."), m_nTaskPID, LOG_STR(m_stProduct.strKey), stMergeData.nMergeID, nErr);
			//	}

			//	// 跳过下面的处理，执行下一条
			//	continue;
			//}
			
			// 该数据为符合条件的数据，开始执行数据合并处理
			// 获取原始数据，模版数据
			nwfd_grib2_field * stDataField = NULL;
			int nCnt = 0; // 客户端发送来的数据肯定为1
			fSrcData = GetSrcData(stMergeData.nYear, stMergeData.nMonth, stMergeData.nDay, stMergeData.nHour, stMergeData.nMinute,
				stMergeData.nSecond, stMergeData.nForecastTime, stMergeData.nTimeRange, stDataField, nCnt);
			if (fSrcData == NULL)
			{
				LOG_(LOGID_ERROR, LOG_F("[TaskPID:%d][Key:%s] 获取原始数据失败（Clinet:%d，%s）."), m_nTaskPID, LOG_STR(m_stProduct.strKey), stMergeData.nMergeID, LOG_STR(stMergeData.strProductKey));
				
				// 原始数据获取失败，无法进行数据合并处理
				stMergeData.strMergeStatus = ABNORMAL;
				stMergeData.strErrReason = "get src Data failed";

				//// 删除客户端上传的缓存数据
				//nErr = DelelteCachedClientData(stMergeData.nClientID, stMergeData.strProductKey);
				//if (nErr != 0)
				//{
				//	LOG_(LOGID_ERROR, LOG_F("[TaskPID:%d][Key:%s] 删除客户端缓存数据失败（ClientID:%d,%s）,ErrCode=%d."), m_nTaskPID, LOG_STR(m_stProduct.strKey), stMergeData.nClientID, LOG_STR(stMergeData.strProductKey), nErr);
				//	stMergeData.strErrReason += ",delete calched client data failed";
				//}

				stMergeData.tFinishTime = QDateTime::currentDateTime(); // 结束时间

				// 将处理结果存储数据库
				nErr = HandleDB::getClass().UpdateMergeResult(stMergeData);
				if (nErr != 0)
				{
					LOG_(LOGID_ERROR, LOG_F("[TaskPID:%d][Key:%s] 更新数据库结果失败（MergeID=%d）,ErrCode=%d."), m_nTaskPID, LOG_STR(m_stProduct.strKey), stMergeData.nMergeID, nErr);
				}

				// 跳过下面的处理，执行下一条
				continue;
			}

			// 获取要合并的客户端数据
			fClientData = GetClientData(stMergeData.nClientID, stMergeData.strProductKey);
			if (fClientData == NULL)
			{
				LOG_(LOGID_ERROR, LOG_F("[TaskPID:%d][Key:%s] 获取原始数据失败（Clinet:%d，%s）."), m_nTaskPID, LOG_STR(m_stProduct.strKey), stMergeData.nMergeID, LOG_STR(stMergeData.strProductKey));

				// 客户端缓存数据获取失败，无法进行数据合并处理
				stMergeData.strMergeStatus = ABNORMAL;
				stMergeData.strErrReason = "Get Client Cached data failed";

				//// 删除客户端上传的缓存数据
				//nErr = DelelteCachedClientData(stMergeData.nClientID, stMergeData.strProductKey);
				//if (nErr != 0)
				//{
				//	LOG_(LOGID_ERROR, LOG_F("[TaskPID:%d][Key:%s] 删除客户端缓存数据失败（ClientID:%d,%s）,ErrCode=%d."), m_nTaskPID, LOG_STR(m_stProduct.strKey), stMergeData.nClientID, stMergeData.strProductKey, nErr);
				//	stMergeData.strErrReason += ",delete calched client data failed";
				//}

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
				// 跳过下面的处理，执行下一条
				continue;
			}

			// 执行合并处理
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

				if (nSrcMergePos < nSrcDataLen && nClientMergePos < nClientDataLen)
				{
					// 数据合并
					fSrcData[nSrcMergePos] = fClientData[nClientMergePos];
				}
				else
				{
					// 该点越界 todo
					LOG_(LOGID_ERROR, LOG_F("[TaskPID:%d][Key:%s] 数据合并时该点（%f,%f）数据导致元数据或客户端数据越界"), m_nTaskPID, LOG_STR(m_stProduct.strKey), fMaskLon, fMaskLat);
				}
			}

			stMergeData.tFinishTime = QDateTime::currentDateTime();  // 结束时间

			// 合并完成后，记录数据库
			stMergeData.strMergeStatus = COMPLETE;

			//// 删除客户端上传的缓存数据
			//nErr = DelelteCachedClientData(stMergeData.nClientID, stMergeData.strProductKey);
			//if (nErr != 0)
			//{
			//	LOG_(LOGID_ERROR, LOG_F("[TaskPID:%d][Key:%s] 删除客户端缓存数据失败（ClientID:%d,%s）,ErrCode=%d."), m_nTaskPID, LOG_STR(m_stProduct.strKey), stMergeData.nClientID, LOG_STR(stMergeData.strProductKey), nErr);
			//	stMergeData.strErrReason += ",delete calched client data failed";
			//}

			// 将处理结果存储数据库
			nErr = HandleDB::getClass().UpdateMergeResult(stMergeData);
			if (nErr != 0)
			{
				LOG_(LOGID_ERROR, LOG_F("[TaskPID:%d][Key:%s] 更新数据库结果失败（MergeID=%d）,ErrCode=%d."), m_nTaskPID, LOG_STR(m_stProduct.strKey), stMergeData.nMergeID, nErr);
			}

			// 数据写入Cached中，并保存到文件中
			HandleComplete(fSrcData, nSrcDataLen, 1, stMergeData.nYear, stMergeData.nMonth, stMergeData.nDay, stMergeData.nHour, stMergeData.nMinute,
				stMergeData.nSecond, stMergeData.nForecastTime, stMergeData.nTimeRange, 3);

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
		}

		// 清空处理列表
		lstMergeData.clear();
	}

	return true;
}

/************************************************************************/
/* 获取模版数据                                                         */
/************************************************************************/
float * TaskProductMerge::GetSrcData(int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nForecastTime, int nTimeRange, nwfd_grib2_field* &stDataField, int &nCnt)
{
	float *fSrcData = NULL;
	int  nDataLen;

	// 产品缓存时间
	QString strProductTime = HandleCommon::GetCachedProductTime(nYear, nMonth, nDay, nHour, nForecastTime, nTimeRange);
	
	// 1、获取缓存Merge的最新数据
	if (m_pCachedImpl)
	{
		int nErr = m_pCachedImpl->GetMergeData(m_stProduct.strRelatedKey.toLocal8Bit().data(), strProductTime.toLocal8Bit().data(), fSrcData, nDataLen);
		if (nErr == SUCCESS && fSrcData != NULL)
		{
			return fSrcData;
		}
	}

	// 2、根据模版类型定义，进行数据构造
	if (m_stProduct.stTemplate.strTemplateType.toLower() == "blank" || m_stProduct.stTemplate.strTemplateType.toLower() == "empty")
	{
		// 数据模版类型-缺测数据
		nDataLen = m_stProduct.nNi * m_stProduct.nNj;
		float * fTemplateData = (float *)malloc(nDataLen * sizeof(float));
		if (fTemplateData == NULL)
		{
			return NULL;
		}

		// 数据模版类型为空白数据-构造数据
		for (int i = 0; i < m_stProduct.nNi * m_stProduct.nNj; i++)
		{
			fTemplateData[i] = m_stProduct.stTemplate.fBlankValue; // 默认缺测值
		}

		return fTemplateData;
	}
	else
	{
		// 数据模版类型-非空白数据-获取指导报数据
		if (m_pCachedImpl)
		{
			int nErr = m_pCachedImpl->GetProductData(m_stProduct.strRelatedKey.toLocal8Bit().data(), strProductTime.toLocal8Bit().data(), fSrcData, nDataLen);
			if (nErr == SUCCESS && fSrcData != NULL)
			{
				return fSrcData;
			}
		}
	}

	// 补充 - 获取原数据失败，使用缺测值填充
	if (fSrcData == NULL)
	{
		// 数据模版类型-缺测数据
		nDataLen = m_stProduct.nNi * m_stProduct.nNj;
		fSrcData = (float *)malloc(nDataLen * sizeof(float));
		if (fSrcData == NULL)
		{
			return NULL;
		}

		// 数据模版类型为空白数据-构造数据
		for (int i = 0; i < m_stProduct.nNi * m_stProduct.nNj; i++)
		{
			fSrcData[i] = m_stProduct.stTemplate.fBlankValue; // 默认缺测值
		}

		return fSrcData;
	}

	return NULL;
}

/************************************************************************/
/* 获取客户端数据                                                       */
/************************************************************************/
float * TaskProductMerge::GetClientData(int nClientID, QString strProductKey)
{
	//float * fClientData = NULL;
	//int  nDataLen;
	//if (m_pCachedImpl)
	//{
	//	// error todo
	//	int nErr = m_pCachedImpl->GetClientData(nClientID, strProductKey.toLocal8Bit().data(), "", fClientData,  nDataLen);
	//	if (nErr == SUCCESS)
	//	{
	//		return fClientData;
	//	}
	//}

	return NULL;
}

/************************************************************************/
/* 获取空白数据                                                         */
/************************************************************************/
float* TaskProductMerge::GetBlankData()
{
	float * fData = (float *)malloc(m_stProduct.nNi * m_stProduct.nNj * sizeof(float));
	if (fData == NULL)
	{
		return NULL;
	}

	// 数据模版类型为空白数据-构造数据
	for (int i = 0; i < m_stProduct.nNi * m_stProduct.nNj; i++)
	{
		fData[i] = m_stProduct.stTemplate.fBlankValue; // 默认缺测值
	}

	return fData;
}

/************************************************************************/
/* 获取Mask某个点的坐标                                                 */
/************************************************************************/
bool TaskProductMerge::GetMaskLonLat(ST_MASK stMask, int nPos, float &fLon, float &fLat)
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
bool TaskProductMerge::GetLonLat(float fLon1, float fLat1, float fDi, float fDj, int nNi, int nNj, int nPos, float &fLon, float &fLat)
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
int TaskProductMerge::GetMergerPos(float fMaskLon, float fMaskLat, float fStartLon, float fStartLat, int nNi)
{
	int nRow = (int)((fMaskLat - fStartLat) / m_stProduct.fDj + 0.1);
	int nCol = (int)((fMaskLon - fStartLon) / m_stProduct.fDi + 0.1);

	int nPos = nRow * nNi + nCol;

	return nPos;
}
//
///************************************************************************/
///* 删除缓存中客户端上传数据                                             */
///************************************************************************/
//int TaskProductMerge::DelelteCachedClientData(int nClientID, QString strDataKey)
//{
//	// 缓存连接判断
//	if (m_pCachedImpl == NULL)
//	{
//		if (!InitCached())
//		{
//			// 初始化缓存连接失败
//			return -1;
//		}
//	}
//
//	// 判断连接状态
//	if (m_pCachedImpl->ConnectStatus() != SUCCESS)
//	{
//		// 连接状态断开，重新进行连接
//		m_pCachedImpl->ReConnect();
//
//		// 再次判断连接状态
//		if (m_pCachedImpl->ConnectStatus() != SUCCESS)
//		{
//			LOG_(LOGID_ERROR, LOG_F("[TaskPID:%d][%s]缓存重连失败，连接状态中断）."), m_nTaskPID, LOG_STR(m_stProduct.strKey));
//			return -2;
//		}
//	}
//
//	// 执行删除操作
//	// error todo
//	return m_pCachedImpl->DeleteClientData(nClientID, strDataKey.toLocal8Bit().data(), "");
//}

/************************************************************************/
/* 处理完成的结果-存储到文件，存储到缓存中                              */
/************************************************************************/
bool TaskProductMerge::HandleComplete(float *fData, int nDataLen, int nCnt, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nForecastTime, int nTimeRange, int nFlg)
{
	// nFlg = 1 : 只保存到文件中
	// nFlg = 2 : 只保存到缓存中
	// nFlg = 3 : 保存到文件和缓存中

	if (nFlg == 1 || nFlg == 3)
	{
		/* 1、将数据保存到Grib2文件中 */
		if (m_stProduct.stFileSave.bDisabled)
		{
			// 不进行文件存储
		}
		else
		{
			QString strGrib2FileName = HandleCommon::GetFileName(m_stProduct.stFileSave.strFileName, nYear, nMonth, nDay, nHour, nForecastTime, nTimeRange);
			QString strGrib2FilePath = m_stProduct.stFileSave.strSavePath.replace("\\", "/");
			strGrib2FilePath = HandleCommon::GetFolderPath(m_stProduct.stFileSave.strSavePath, nYear, nMonth, nDay);
			QString strGrib2File = QString("%1/%2").arg(strGrib2FilePath).arg(strGrib2FileName);
			QString strGrib2File_tmp = QString("%1.tmp").arg(strGrib2File);

			if (HandleCommon::CreatePath(strGrib2FilePath))
			{
				int nErr = SaveData2GribFile(fData, nCnt, m_stProduct, nYear, nMonth, nDay, nHour, nForecastTime, nTimeRange, strGrib2File_tmp,false);
				if (nErr == 0)
				{
					if (HandleCommon::Rename(strGrib2File_tmp, strGrib2File))
					{
						// 数据分发处理
						DataDistribution(m_stProduct.stFileSave.lstDDS, strGrib2FileName, m_stProduct.stFileSave.strSavePath);

						LOG_(LOGID_DEBUG, LOG_F("生成Grib2文件成功：%s"), LOG_STR(strGrib2File));
					}
				}
				else
				{
					LOG_(LOGID_DEBUG, LOG_F("[WARNING]生成Grib2文件失败,错误编码=%d"), nErr);
				}
			}
			else
			{
				// 创建目录失败
				LOG_(LOGID_DEBUG, LOG_F("[WARNING]创建目录失败（%s）"), LOG_STR(strGrib2FilePath));
			}
		}
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
/* 写数据操作                                                           */
/************************************************************************/
int TaskProductMerge::WriteData(QString strSavePath, QString strSaveFileName, float *fData, int nDataLen, int nCnt, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nForecastTime, int nTimeRange, QStringList lstDDS, bool bIsMergeFile)
{
	QString strGrib2FileName = HandleCommon::GetFileName(strSaveFileName, nYear, nMonth, nDay, nHour, nForecastTime, nTimeRange);
	QString strGrib2FilePath = strSavePath.replace("\\", "/");
	QString strGrib2File = QString("%1/%2").arg(strGrib2FilePath).arg(strGrib2FileName);
	QString strGrib2File_tmp = QString("%1.tmp").arg(strGrib2File);

	int nErr = SUCCESS;

	// 创建目录
	if (HandleCommon::CreatePath(strGrib2FilePath))
	{
		// 保存数据到文件中
		nErr = SaveData2GribFile(fData, nCnt, m_stProduct, nYear, nMonth, nDay, nHour, nForecastTime, nTimeRange, strGrib2File_tmp, bIsMergeFile);
		if (nErr == SUCCESS)
		{
			if (HandleCommon::Rename(strGrib2File_tmp, strGrib2File))
			{
				// 数据分发处理
				DataDistribution(lstDDS, strGrib2FileName, strGrib2FilePath);

				LOG_(LOGID_DEBUG, LOG_F("生成Grib2文件成功：%s"), LOG_STR(strGrib2File));
			}
			else
			{
				nErr = -6;
			}
		}
		else
		{
			LOG_(LOGID_DEBUG, LOG_F("[WARNING]生成Grib2文件失败,错误编码=%d"), nErr);
		}
	}
	else
	{
		// 创建目录失败
		return 200;
	}

	return nErr;
}

/************************************************************************/
/* 写数据操作                                                           */
/************************************************************************/
int TaskProductMerge::WriteData2Micaps(QString strSavePath, QString strSaveFileName, float *fData, int nYear, int nMonth, int nDay, int nHour, int nForecastTime, QStringList lstDDS)
{
	strSavePath = strSavePath.replace("\\", "/");
	QString strFile = QString("%1/%2").arg(strSavePath).arg(strSaveFileName);
	QString strFile_tmp = QString("%1.tmp").arg(strFile);

	int nErr = SUCCESS;
	float fOffSet = 0;
	if (m_stProduct.strType == "TMP" || m_stProduct.strType == "ETM" || m_stProduct.strType == "ETN")
	{
		// 温度相关数据要进行单位转换
		fOffSet = -273.15f;
	}

	// 创建目录
	if (HandleCommon::CreatePath(strSavePath))
	{
		// 保存数据到Micaps文件中
		nErr = HandleGrib::getClass().SaveData2MicapsFile(fData, m_stProduct.nCnt, m_stProduct.fLon1, m_stProduct.fLon2, m_stProduct.fLat1, m_stProduct.fLat2, 
			m_stProduct.fDi, m_stProduct.fDj, m_stProduct.nNi, m_stProduct.nNj, nYear, nMonth, nDay, nHour, nForecastTime, 
			m_stProduct.strName, m_stProduct.strLineVal, fOffSet, m_stProduct.fMissingVal, strFile_tmp);
		if (nErr == SUCCESS)
		{
			if (HandleCommon::Rename(strFile_tmp, strFile))
			{
				// 数据分发处理
				DataDistribution(lstDDS, strSaveFileName, strSavePath);
			}
			else
			{
				// 重命名失败
				nErr = -6;
			}
		}
	}
	else
	{
		// 创建目录失败
		return 200;
	}

	return SUCCESS;
}
/************************************************************************/
/* 按照扫描模式进行处理                                                 */
/************************************************************************/
bool TaskProductMerge::HandleLocalDataFromMode()
{
	LOG_(LOGID_INFO, LOG_F("[Key:%s] 本地文件数据拼图任务开始执行."), LOG_STR(m_stProduct.strKey));

	bool  bResult = true;
	int nErr = 0;
	int nYear, nMonth, nDay, nHour;
	QString strMergeName;

	// 根据转换模式确定目录和文件
	QDateTime tToday = QDateTime::currentDateTime();
	int nNowHour = tToday.time().hour(); // 当前时

	QString  strStartDay;
	QString  strEndDay;
	QDateTime tStartDay;
	QDateTime tEndDay;

	if (m_stProduct.strScanMode == "day")
	{
		strStartDay = QString("").sprintf("%04d%02d%02d %02d:%02d:%02d", tToday.date().year(), tToday.date().month(), tToday.date().day(), 0, 0, 0);
		strEndDay = QString("").sprintf("%04d%02d%02d %02d:%02d:%02d", tToday.date().year(), tToday.date().month(), tToday.date().day(), 23, 59, 59);
	}
	else
	{
		QStringList lstDate = m_stProduct.strScanMode.split("-");
		if (lstDate.size() != 2)
		{
			LOG_(LOGID_ERROR, LOG_F("[Key:%s] 扫描模式设置错误."), LOG_STR(m_stProduct.strKey));
			LOG_(LOGID_INFO, LOG_F("[Key:%s] 本地文件数据拼图任务执行结束."), LOG_STR(m_stProduct.strKey));
			return false;
		}
		QString strStart = lstDate.at(0);
		QString strEnd = lstDate.at(1);
		if (strStart.size() != 8 || strEnd.size() != 8)
		{
			LOG_(LOGID_ERROR, LOG_F("[Key:%s] 扫描模式设置错误."), LOG_STR(m_stProduct.strKey));
			LOG_(LOGID_INFO, LOG_F("[Key:%s] 本地文件数据拼图任务执行结束."), LOG_STR(m_stProduct.strKey));
			return false;
		}

		strStartDay = QString("").sprintf("%s %02d:%02d:%02d", strStart.toLocal8Bit().data(), 0, 0, 0);
		strEndDay = QString("").sprintf("%s %02d:%02d:%02d", strEnd.toLocal8Bit().data(), 23, 59, 59);
	}

	tStartDay = QDateTime::fromString(strStartDay, "yyyyMMdd hh:mm:ss");
	tEndDay = QDateTime::fromString(strEndDay, "yyyyMMdd hh:mm:ss");

	// 按照配置的扫描模式的时间进行遍历
	for (QDateTime tDay = tStartDay; tDay < tEndDay; tDay = tDay.addDays(1))
	{
		nYear = tDay.date().year();
		nMonth = tDay.date().month();
		nDay = tDay.date().day();

		// 遍历起报时间：小时
		// 配置的起报时间
		HASH_FORECAST::iterator iter;
		for (iter = m_stProduct.hasForecast.begin(); iter != m_stProduct.hasForecast.end(); iter++)
		{
			ST_FORECAST stForecast = *iter;
			nHour = stForecast.nReportTime; // 起报时间

			// 如果当前时间，不在配置的执行时间范围内，跳过本次合并处理
			if ((stForecast.nStartTime != 0 && stForecast.nEndTime != 0) && (nNowHour < stForecast.nStartTime || nNowHour > stForecast.nEndTime))
			{
				continue;
			}

			// 执行该时次的本地文件拼图合并
			ProcessLocalMerge(nYear, nMonth, nDay, nHour);

			//// 按照时间间隔和最大配置时效进行处理
			//for (nForecastTime = m_stProduct.nTimeRange; nForecastTime <= m_stProduct.nMaxForecastTime; nForecastTime += m_stProduct.nTimeRange)
			//{
			//	// 处理标记名称
			//	//strMergeName = QString("").sprintf("%04d%02d%02d%02d.%03d", nYear, nMonth, nDay, nHour, nForecastTime);
			//	//if (IsHasProcessed(strMergeName))
			//	//{
			//	//	// 已经处理过
			//	//	continue;
			//	//}
			//	// 该预报时间的拼图处理
			//	nErr = ProcessLocalMerge(nYear, nMonth, nDay, nHour, nForecastTime, m_stProduct.nTimeRange);
			//	if (nErr == 0)
			//	{
			//		// 处理成功
			//		//m_hasCompleteProduct_today[strMergeName] = true;
			//		LOG_(LOGID_MERGE, LOG_F("[Merge][%s][%s]文件合并处理成功."), LOG_STR(m_stProduct.strKey), LOG_STR(strMergeName));
			//	}
			//	else
			//	{
			//		// 处理失败
			//		//m_hasCompleteProduct_today[strMergeName] = false;
			//		LOG_(LOGID_MERGE, LOG_F("[Merge][%s][%s]文件合并处理失败，ErrCode=%d."), LOG_STR(m_stProduct.strKey), LOG_STR(strMergeName), nErr);
			//	}
			//}
		}
	}

	LOG_(LOGID_INFO, LOG_F("[Key:%s] 本地文件数据拼图任务执行结束."), LOG_STR(m_stProduct.strKey));

	return bResult;
}

/************************************************************************/
/* 扫描本地目录，进行数据合并                                           */
/************************************************************************/
bool TaskProductMerge::HandleLocalData()
{
	int nErr = 0;
	int nYear, nMonth, nDay, nHour, nForecastTime;
	QString strMergeName;

	// 当前日期
	QDateTime tNow = QDateTime::currentDateTime();
	nYear = tNow.date().year();
	nMonth = tNow.date().month();
	nDay = tNow.date().day();

	// 配置的起报时间
	HASH_FORECAST::iterator iter;
	for (iter = m_stProduct.hasForecast.begin(); iter != m_stProduct.hasForecast.end(); iter++)
	{
		ST_FORECAST stForecast = *iter;

		// 起报时间
		nHour = stForecast.nReportTime;

		// 按照时间间隔和最大配置时效进行处理
		for (nForecastTime = m_stProduct.nTimeRange; nForecastTime <= m_stProduct.nMaxForecastTime; nForecastTime += m_stProduct.nTimeRange)
		{
			// 处理标记名称
			strMergeName = QString("").sprintf("%04d%02d%02d%02d.%03d", nYear, nMonth, nDay, nHour, nForecastTime);
			//if (IsHasProcessed(strMergeName))
			//{
			//	// 已经处理过
			//	continue;
			//}

			// 该预报时间的合并处理
			nErr = ProcessLocalMerge(nYear, nMonth, nDay, nHour, nForecastTime, m_stProduct.nTimeRange);
			if (nErr == 0)
			{
				// 处理成功
				m_hasCompleteProduct_today[strMergeName] = true;
				LOG_(LOGID_MERGE, LOG_F("[Merge][%s][%s]文件合并处理成功."), LOG_STR(m_stProduct.strKey), LOG_STR(strMergeName));
			}
			else
			{
				// 处理失败
				m_hasCompleteProduct_today[strMergeName] = false;
				LOG_(LOGID_MERGE, LOG_F("[Merge][%s][%s]文件合并处理失败，ErrCode=%d."), LOG_STR(m_stProduct.strKey), LOG_STR(strMergeName), nErr);
			}
		}
	}

	return true;
}

/************************************************************************/
/* 保存拼图数据到文件中                                                 */
/************************************************************************/
bool TaskProductMerge::HandleMergeData()
{
	int nErr = 0;
	int nYear, nMonth, nDay, nHour, nForecastTime, nNowHour;
	QString strMergeName;

	
	//任务开始时间戳
	QString startTimeStamp = QDateTime::currentDateTime().toString("yyyyMMddHHmmss");
	
	
	// 当前日期
	QDateTime tNow = QDateTime::currentDateTime();
	nYear = tNow.date().year();
	nMonth = tNow.date().month();
	nDay = tNow.date().day();
	nNowHour = tNow.time().hour(); // 当前小时

	// 每个时次总共生成的拼图数据个数
	int nDataTotalCount = m_stProduct.nMaxForecastTime / m_stProduct.nTimeRange; 
	float * fMergeData = NULL;
	int nDataLenSum = 0;

	// 配置的起报时间：08时，20时
	HASH_FORECAST::iterator iter;
	for (iter = m_stProduct.hasForecast.begin(); iter != m_stProduct.hasForecast.end(); iter++)
	{
		ST_FORECAST stForecast = *iter;
		// 起报时间
		nHour = stForecast.nReportTime;

		// 如果当前时间，不在配置的执行时间范围内，跳过本次合并处理
		if ((stForecast.nStartTime != 0 && stForecast.nEndTime != 0) && (nNowHour < stForecast.nStartTime || nNowHour > stForecast.nEndTime))
		{
			continue;
		}

		// 一个时次的合并文件存储申请
		if (!m_stProduct.stMergeFileSave.bDisabled)
		{
			if (fMergeData)
			{
				free(fMergeData);
				fMergeData = NULL;
			}

			fMergeData = (float *)malloc(m_stProduct.nNi * m_stProduct.nNj * sizeof(float) * nDataTotalCount);
			if (fMergeData == NULL)
			{
				// 合并数据空间申请失败
				// todo
			}
		}
		// 重新开始一个时次
		nDataLenSum = 0;

		// 按照时间间隔和最大配置时效进行处理
		for (nForecastTime = m_stProduct.nTimeRange; nForecastTime <= m_stProduct.nMaxForecastTime; nForecastTime += m_stProduct.nTimeRange)
		{
			// 处理标记名称
			strMergeName = QString("").sprintf("%04d%02d%02d%02d.%03d", nYear, nMonth, nDay, nHour, nForecastTime);

			float *fData = NULL;
			nwfd_grib2_field* stDataField = NULL;
			int nDataCnt;
			int nDataLen;

			nDataCnt = m_stProduct.nCnt;
			nDataLen = m_stProduct.nNi * m_stProduct.nNj;
			nDataLenSum += nDataLen;
			
			// 获取拼图数据
			fData = GetSrcData(nYear, nMonth, nDay, nHour, 0, 0, nForecastTime, m_stProduct.nTimeRange, stDataField, nDataCnt);
			if (fData == NULL)
			{
				// 获取数据失败
				LOG_(LOGID_MERGE, LOG_F("[Merge][%s][%s]拼图数据获取失败."), LOG_STR(m_stProduct.strRelatedKey), LOG_STR(strMergeName));
				continue;
			}

			// 写入单个文件
			if (!m_stProduct.stFileSave.bDisabled)
			{
				// 处理完成
				int nErr = WriteData(m_stProduct.stFileSave.strSavePath, m_stProduct.stFileSave.strFileName, fData, nDataLen, nDataCnt, nYear, nMonth, nDay, nHour, 0, 0, nForecastTime, m_stProduct.nTimeRange, m_stProduct.stFileSave.lstDDS,false);
				if (nErr == SUCCESS)
				{
					LOG_(LOGID_MERGE, LOG_F("[Merge][%s][%s]拼图数据单时次文件处理成功."), LOG_STR(m_stProduct.strKey), LOG_STR(strMergeName));
					
				}
				else
				{
					LOG_(LOGID_MERGE, LOG_F("[Merge][%s][%s]拼图数据单时次文件处理失败（err=%d）."), LOG_STR(m_stProduct.strKey), LOG_STR(strMergeName), nErr);
				}
			}

			// 一个时次的合并文件
			if (!m_stProduct.stMergeFileSave.bDisabled)
			{
				if (fMergeData)
				{
					memcpy(fMergeData + nDataLenSum - nDataLen, fData, nDataLen * sizeof(float));
				}
			}

			// 释放数据
			if (fData)
			{
				free(fData);
				fData = NULL;
			}
			if (stDataField)
			{
				free(stDataField);
				stDataField = NULL;
			}
			// 延时处理
			msleep(20);
		}

		// 一个时次的写入一个合并文件中
		if (!m_stProduct.stMergeFileSave.bDisabled)
		{
			if (fMergeData)
			{
				int nErr = WriteData(m_stProduct.stMergeFileSave.strSavePath, m_stProduct.stMergeFileSave.strFileName, fMergeData, nDataLenSum, nDataTotalCount, nYear, nMonth, nDay, nHour, 0, 0, m_stProduct.nMaxForecastTime, m_stProduct.nTimeRange, m_stProduct.stMergeFileSave.lstDDS,true);
				if (nErr == SUCCESS)
				{
					LOG_(LOGID_MERGE, LOG_F("[Merge][%s][%s]拼图多时次合并文件处理成功."), LOG_STR(m_stProduct.strKey), LOG_STR(strMergeName));
					//把任务执行结果存入数据库
					ST_DB_TASK_MSG msg;
					msg.productKey = m_stProduct.strRelatedKey;
					msg.productRole = m_stProduct.strRole;
					msg.year = nYear;
					msg.month = nMonth;
					msg.day = nDay;
					msg.reporttime = nHour;
					msg.forecasttime = m_stProduct.nMaxForecastTime;
					msg.folder = m_stProduct.stMergeFileSave.strSavePath;
					msg.filename = HandleCommon::GetFileName(m_stProduct.stMergeFileSave.strFileName, nYear, nMonth, nDay, nHour, 
						m_stProduct.nMaxForecastTime, m_stProduct.nTimeRange);
					msg.taskStatus = COMPLETE;
					msg.errReason = "文件生成成功";

					msg.starttime = startTimeStamp;
					
					//任务结束时间戳
					QString endTimeStamp = QDateTime::currentDateTime().toString("yyyyMMddHHmmss");
					
					msg.finishtime = endTimeStamp;
					HandleDB::getClass().UpdateTaskMessage(&msg);
				}
				else
				{
					// todo
					LOG_(LOGID_MERGE, LOG_F("[Merge][%s][%s]拼图多时次合并文件处理失败（err=%d）."), LOG_STR(m_stProduct.strKey), LOG_STR(strMergeName),nErr);

					//把任务执行结果存入数据库
					ST_DB_TASK_MSG msg;
					msg.productKey = m_stProduct.strKey;
					msg.productRole = m_stProduct.strRole;
					msg.year = nYear;
					msg.month = nMonth;
					msg.day = nDay;
					msg.reporttime = nHour;
					msg.forecasttime = m_stProduct.nMaxForecastTime;
					msg.folder = m_stProduct.stMergeFileSave.strSavePath;
					msg.filename = HandleCommon::GetFileName(m_stProduct.stMergeFileSave.strFileName, nYear, nMonth, nDay, nHour,
						m_stProduct.nMaxForecastTime, m_stProduct.nTimeRange);
					msg.taskStatus = ABNORMAL;
					msg.errReason = "文件生成失败";

					msg.starttime = startTimeStamp;

					//任务结束时间戳
					QString endTimeStamp = QDateTime::currentDateTime().toString("yyyyMMddHHmmss");

					msg.finishtime = endTimeStamp;
					HandleDB::getClass().UpdateTaskMessage(&msg);
				}

				// 释放空间
				free(fMergeData);
				fMergeData = NULL;
			}
		}
		else
		{
			//没打开file merge 开关
			ST_DB_TASK_MSG msg;
			msg.productKey = m_stProduct.strKey;
			msg.productRole = m_stProduct.strRole;
			msg.year = nYear;
			msg.month = nMonth;
			msg.day = nDay;
			msg.reporttime = nHour;
			msg.forecasttime = m_stProduct.nMaxForecastTime;
			msg.folder = m_stProduct.stMergeFileSave.strSavePath;
			msg.filename = HandleCommon::GetFileName(m_stProduct.stMergeFileSave.strFileName, nYear, nMonth, nDay, nHour,
				m_stProduct.nMaxForecastTime, m_stProduct.nTimeRange);
			msg.taskStatus = ABNORMAL;
			msg.errReason = "文件合并选项没有打开";

			msg.starttime = startTimeStamp;

			//任务结束时间戳
			QString endTimeStamp = QDateTime::currentDateTime().toString("yyyyMMddHHmmss");

			msg.finishtime = endTimeStamp;
			HandleDB::getClass().UpdateTaskMessage(&msg);
		}

	}

	return true;
}

/************************************************************************/
/* 已处理产品Hash标记维护处理                                           */
/************************************************************************/
void TaskProductMerge::HandleClearHash()
{
	// 按天进行标记判断处理
	QString strNewClearHasFlg = QDateTime::currentDateTime().toString("yyyyMMdd");
	if (strNewClearHasFlg != m_strClearHasFlg)
	{
		// 日期已经切换，结果判断列表清空，防止无限增长
		// 清除昨天的数据
		m_hasCompleteProduct_yesterday.clear();
		// 今天的数据成为昨天的数据
		m_hasCompleteProduct_yesterday = m_hasCompleteProduct_today;
		// 清除今天的数据
		m_hasCompleteProduct_today.clear();

		// 清空今天的合并的数据
		m_hasMergeData.clear();

		// 重新赋值
		m_strClearHasFlg = strNewClearHasFlg;
	}
}

/************************************************************************/
/* 是否已经处理判断方法                                                 */
/************************************************************************/
bool TaskProductMerge::IsHasProcessed(QString strName)
{
	// 查看今天处理的数据
	if (m_hasCompleteProduct_today.contains(strName))
	{
		return 	m_hasCompleteProduct_today[strName];
	}

	// 查看昨天处理的数据
	if (m_hasCompleteProduct_yesterday.contains(strName))
	{
		return 	m_hasCompleteProduct_yesterday[strName];
	}

	return false;
}

/************************************************************************/
/* 获取产品文件列表                                                     */
/************************************************************************/
/*
bool TaskProductMerge::GetProductFileList(QString strSrcFolder, QString strSrcFile, QFileInfoList& lstFile)
{
	QDir dDir;

	// 删选日期
	QDateTime tToday = QDateTime::currentDateTime();
	//QDateTime tYesterday = HandleCommon::GetYesterday();

	// 替换路径中的时间
	strSrcFolder = HandleCommon::GetFolderPath(strSrcFolder, tToday.date().year(), tToday.date().month(), tToday.date().day());
	strSrcFolder.replace("\\", "/");

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
	strFileFormat.replace("FFF", "*");
	strFileFormat.replace("TT", "*");
	strFileFormat.replace("HH", "*");
	strFileFormat.replace("yyyymmddhhmiss", "*");

	// 筛选名称
	QString strTodayFilterName = strFileFormat;
	//QString strYesterdayFilterName = strFileFormat;
	if (m_stProduct.stFileNameFormat.stYear.iLen == 4) // 4位年份
	{
		strTodayFilterName.replace("YYYYMMDD", tToday.toString("yyyyMMdd"));
		//strYesterdayFilterName.replace("YYYYMMDD", tYesterday.toString("yyyyMMdd"));
	}
	else if (m_stProduct.stFileNameFormat.stYear.iLen == 2) // 2位年份
	{
		strTodayFilterName.replace("YYMMDD", tToday.toString("yyMMdd"));
		//strYesterdayFilterName.replace("YYMMDD", tYesterday.toString("yyMMdd"));
	}

	QStringList lstfileFilter;
	lstfileFilter << strTodayFilterName; //<< strYesterdayFilterName;

	dDir.setNameFilters(lstfileFilter); // 设置文件筛选

	// 获取产品文件
	lstFile = dDir.entryInfoList();

	return true;
}
*/

bool TaskProductMerge::GetProductFileList(QString strSrcPath, QString strFilterName, QFileInfoList& lstFile)
{
	QDir dDir;

	// 检验产品目录是否存在
	if (!dDir.exists(strSrcPath))
	{
		return false;
	}

	// 目录设置
	dDir.setPath(strSrcPath);      // 设置目录名称
	dDir.setFilter(QDir::Files);   // 列出目录中的文件
	dDir.setSorting(QDir::Name);   // 按照文件名排序

	QStringList lstfileFilter;
	lstfileFilter << strFilterName;

	dDir.setNameFilters(lstfileFilter); // 设置文件筛选

	// 获取产品文件
	lstFile = dDir.entryInfoList();

	return true;
}

/************************************************************************/
/* 本地文件合并处理                                                     */
/************************************************************************/
int TaskProductMerge::ProcessLocalMerge(int nYear, int nMonth, int nDay, int nHour)
{
	/* 1. 变量定义 */
	// Mask数据
	float *fMaskData = NULL; 
	// 原数据(模版数据)
	float *fSrcData = NULL;    
	nwfd_grib2_field* stSrcDataField = NULL;
	int nSrcDataCnt;
	int nSrcDataLen;
	int nSrcMergePos;
	// 客户端数据
	float *fClientData = NULL; 
	nwfd_grib2_field* stClientDataField = NULL;
	int nClinetCnt;
	int nClientDataLen;
	int nClientMergePos;

	// 每个时次总共生成的拼图数据个数
	float * fMergeData = NULL;
	int nDataTotalCount = m_stProduct.nMaxForecastTime / m_stProduct.nTimeRange;
	int nDataLenSum = 0;

	float fMaskLon, fMaskLat;
	int nForecastTime;
	int nTimeRange = m_stProduct.nTimeRange;
	bool bExecResult = true;
	int nErr = SUCCESS;

	// 原数据长度
	nSrcDataLen = m_stProduct.nNi * m_stProduct.nNj;

	/* 2. 获取Mask数据 */
	fMaskData = CMask::getClass().GetMaskData(m_stProduct.stMask.strMaskFile);
	if (fMaskData == NULL)
	{
		// todo Mask数据获取失败，无法进行数据合并处理
		LOG_(LOGID_MERGE, LOG_F("[%s][%s] 获取Mask数据(%s)失败，拼图结束."), LOG_STR(m_stProduct.strKey), LOG_STR(m_stProduct.strName), LOG_STR(m_stProduct.stMask.strMaskFile));
		return -1;
	}

	/* 3. 一个时次的合并文件存储申请 */
	if (!m_stProduct.stMergeFileSave.bDisabled)
	{
		fMergeData = (float *)malloc(nSrcDataLen * sizeof(float) * nDataTotalCount);
		if (fMergeData == NULL)
		{
			// 合并数据空间申请失败
			LOG_(LOGID_MERGE, LOG_F("[%s][%s] 拼图一个时次的合并数据空间申请失败."), LOG_STR(m_stProduct.strKey), LOG_STR(m_stProduct.strName));
		}
	}
	nDataLenSum = 0;

	bool bGetSrcDataFailed = false;
	bool bNewMergeDataFlg = false;

	/* 4. 根据每个预报时效进行拼图处理 */
	for (nForecastTime = nTimeRange; nForecastTime <= m_stProduct.nMaxForecastTime; nForecastTime += nTimeRange)
	{
		LOG_(LOGID_MERGE, LOG_F("[%s][%s][%04d%02d%02d%02d.%03d] 拼图开始 >>>."), LOG_STR(m_stProduct.strKey), LOG_STR(m_stProduct.strName), nYear, nMonth, nDay, nHour, nForecastTime);
		bool bMergeFlg = false;

		// 模版数据:原拼图数据/指导报数据/缺测值
		fSrcData = GetSrcData(nYear, nMonth, nDay, nHour, 0, 0, nForecastTime, nTimeRange, stSrcDataField, nSrcDataCnt);
		if (fSrcData == NULL)
		{
			// 获取模版数据异常
			LOG_(LOGID_MERGE, LOG_F(">>[%s][%s][%04d%02d%02d%02d.%03d] 获取模版数据失败."), LOG_STR(m_stProduct.strKey), LOG_STR(m_stProduct.strName), nYear, nMonth, nDay, nHour, nForecastTime);
			
			// 获取模版数据失败，导致如果进行一个时次的文件合并时，数据将不再准确
			bGetSrcDataFailed = true;
			continue;
		}
		
		// 按照配置的合并列表顺序进行合并
		LIST_MERGE_ITEM::iterator iter;
		for (iter = m_stProduct.lstMergeItem.begin(); iter != m_stProduct.lstMergeItem.end(); iter++)
		{
			ST_MERGE_ITEM stMergeItem = *iter;
			QFileInfoList  lstFile;

			// 如果该客户端配置的目录为空，则跳过本产品
			if (stMergeItem.strSrcFolder.isEmpty() || stMergeItem.strSrcFileName.isEmpty())
			{
				LOG_(LOGID_MERGE, LOG_F(">>[%s][%04d%02d%02d%02d.%03d][%d] 客户端配置为空-跳过."), LOG_STR(m_stProduct.strKey), nYear, nMonth, nDay, nHour, nForecastTime, stMergeItem.nClientID);
				continue;
			}

			// 获取文件列表
			QString strItemSrcFolder = HandleCommon::GetFolderPath(stMergeItem.strSrcFolder, nYear, nMonth, nDay, m_stProduct.strType); //目录
			QString strItemFilterName = HandleCommon::GetFileFilterName(stMergeItem.strSrcFileName, nYear, nMonth, nDay, nHour, nForecastTime);        // 文件名
			if (!GetProductFileList(strItemSrcFolder, strItemFilterName, lstFile))
			{
				// 获取失败
				LOG_(LOGID_DEBUG, LOG_F("[TaskPID:%d][%s] 获取目录文件失败（path=%s）"), m_nTaskPID, LOG_STR(m_stProduct.strKey), LOG_STR(strItemSrcFolder));
				LOG_(LOGID_MERGE, LOG_F(">>[%s][%04d%02d%02d%02d.%03d][%d] 该客户端没有符合条件的文件(%s)."), LOG_STR(m_stProduct.strKey), nYear, nMonth, nDay, nHour, nForecastTime, stMergeItem.nClientID, LOG_STR(strItemFilterName));
				continue;
			}

			// 查找文件列表中的最新文件
			QFileInfoList::iterator iterFile;
			for (iterFile = lstFile.begin(); iterFile != lstFile.end(); iterFile++)
			{
				// 文件名
				QString strFileName = (*iterFile).fileName();

				// 跳过以tmp结尾的数据文件
				if (strFileName.endsWith(".tmp", Qt::CaseInsensitive))
				{
					continue;
				}

				// 检查该文件，该时次的是否处理过
				QString strFlg = QString("%1_%2").arg(strFileName).arg(nForecastTime);
				if (IsHasProcessed(strFlg))
				{
					// 已处理过，跳过
					continue;
				}

				// 获取要合并的客户端数据
				int nErr = GetClientLocalData(strItemSrcFolder, strFileName, nYear, nMonth, nDay, nHour, nForecastTime, fClientData, stClientDataField, nClinetCnt);
				if (nErr != 0 || fClientData == NULL)
				{
					// 客户端数据获取失败
					LOG_(LOGID_MERGE, LOG_F(">>[%s][%04d%02d%02d%02d.%03d][%d] 获取该客户端文件数据失败（%s）,错误编码=%d."), LOG_STR(m_stProduct.strKey), nYear, nMonth, nDay, nHour, nForecastTime, stMergeItem.nClientID, LOG_STR(strFileName),nErr);
					continue;
				}

				LOG_(LOGID_DEBUG, LOG_F("[Key:%s][%04d-%02d-%02d] ClientID=%d,FileName=%s:文件信息(%f,%f,%f,%f,%f,%f,%d,%d) "), LOG_STR(m_stProduct.strKey), nYear, nMonth, nDay,
					stMergeItem.nClientID, LOG_STR(strFileName), stClientDataField[0].lon1, stClientDataField[0].lon2,
					stClientDataField[0].lat1, stClientDataField[0].lat2, stClientDataField[0].incrementi, stClientDataField[0].incrementj, stClientDataField[0].Ni, stClientDataField[0].Nj);

				// 格距判断
				if (!IS_EQUAL(stClientDataField[0].incrementi, m_stProduct.fDi) || !IS_EQUAL(stClientDataField[0].incrementj, m_stProduct.fDj))
				{
					LOG_(LOGID_MERGE, LOG_F(">>[%s][%04d%02d%02d%02d.%03d][%d]该客户端文件格距(%0.2f,%0.2f)与配置不一致(File=%s)."), LOG_STR(m_stProduct.strKey), nYear, nMonth, nDay, nHour, nForecastTime, stMergeItem.nClientID, stClientDataField[0].incrementi, stClientDataField[0].incrementj, LOG_STR(strFileName));

					// 格距不相等，不能进行合并
					if (fClientData)
					{
						free(fClientData);
						fClientData = NULL;
					}
					if (stClientDataField)
					{
						free(stClientDataField);
						stClientDataField = NULL;
					}

					continue;
				}

				// 开始合并处理
				LOG_(LOGID_DEBUG, LOG_F("[Key:%s] 进行[%04d-%02d-%02d]拼图:ClientID=%d,FileName=%s."), LOG_STR(m_stProduct.strKey), nYear, nMonth, nDay, stMergeItem.nClientID, LOG_STR(strFileName));
				LOG_(LOGID_MERGE, LOG_F(">>[%s][%04d%02d%02d%02d.%03d][%d][File=%s] 获取该客户端文件数据成功."), LOG_STR(m_stProduct.strKey), nYear, nMonth, nDay, nHour, nForecastTime, stMergeItem.nClientID, LOG_STR(strFileName));

				// 不使用配置的经纬度，使用文件中读取的经纬度
				nClientDataLen = stClientDataField[0].Ni * stClientDataField[0].Nj;

				// 执行合并处理
				if (stMergeItem.nClientID % 100 == 0)
				{
					// 特殊处理，该客户端为省级客户端，不按照Mask进行拼图，而是将数据覆盖到模版数据上，此客户端要配置到list最开始 
					for (int nPos = 0; nPos < m_stProduct.nNi * m_stProduct.nNj; nPos++)
					{
						// 此位置为要合并的数据，计算该点的经纬度
						GetLonLat(m_stProduct.fLon1, m_stProduct.fLat1, m_stProduct.fDi, m_stProduct.fDj, m_stProduct.nNi, m_stProduct.nNj, nPos, fMaskLon, fMaskLat);

						// 通过经纬度计算客户端的坐标
						nClientMergePos = GetMergerPos(fMaskLon, fMaskLat, stClientDataField[0].lon1, stClientDataField[0].lat1, stClientDataField[0].Ni);

						if (nClientMergePos >= 0 && nClientMergePos < nClientDataLen)
						{
							// 数据合并
							fSrcData[nPos] = fClientData[nClientMergePos];
						}
					}
				}
				else
				{
					// 其他客户端按照Mask文件进行匹配
					for (int nPos = 0; nPos < m_stProduct.stMask.nNi * m_stProduct.stMask.nNj; nPos++)
					{
						// 查找要合并的位置点： stMergeData.nClientID 与 Mask文件中MaskID一致
						if ((int)(fMaskData[nPos] + 0.1) != stMergeItem.nClientID)
						{
							continue;
						}

						// 此位置为要合并的数据，计算该点的经纬度
						GetMaskLonLat(m_stProduct.stMask, nPos, fMaskLon, fMaskLat);

						// 通过经纬度获取要合并数据的位置点
						nSrcMergePos = GetMergerPos(fMaskLon, fMaskLat, m_stProduct.fLon1, m_stProduct.fLat1, m_stProduct.nNi);
						nClientMergePos = GetMergerPos(fMaskLon, fMaskLat, stClientDataField[0].lon1, stClientDataField[0].lat1, stClientDataField[0].Ni);

						if ((nSrcMergePos >= 0 && nSrcMergePos < nSrcDataLen) && (nClientMergePos >= 0 && nClientMergePos < nClientDataLen))
						{
							// 数据合并
							fSrcData[nSrcMergePos] = fClientData[nClientMergePos];
						}
						else
						{
							// 该点越界 todo
							LOG_(LOGID_ERROR, LOG_F("[Key:%s] 进行文件合并时该点（%f,%f）数据导致元数据或客户端数据越界"), LOG_STR(m_stProduct.strKey), fMaskLon, fMaskLat);
						}
					}
				}

				// 本文件数据处理完成，释放客户端空间
				if (fClientData)
				{
					free(fClientData);
					fClientData = NULL;
				}
				if (stClientDataField)
				{
					free(stClientDataField);
					stClientDataField = NULL;
				}

				// 有新的数据进行拼图数据
				m_hasCompleteProduct_today[strFlg] = true;
				bMergeFlg = true;
				bNewMergeDataFlg = true;
			}
		}

		// 该时次有新数据添加到拼图数据中
		if (bMergeFlg)
		{
			// 处理完成，保存到缓存中
			bExecResult = HandleComplete(fSrcData, nSrcDataLen, 1, nYear, nMonth, nDay, nHour, 0, 0, nForecastTime, nTimeRange, 2);
			if (bExecResult)
			{
				LOG_(LOGID_MERGE, LOG_F("[%s][%04d%02d%02d%02d.%03d] 该时次拼图保存缓存成功."), LOG_STR(m_stProduct.strKey), nYear, nMonth, nDay, nHour, nForecastTime);
			}
			else
			{
				LOG_(LOGID_MERGE, LOG_F("[%s][%04d%02d%02d%02d.%03d] 该时次拼图保存缓存失败."), LOG_STR(m_stProduct.strKey), nYear, nMonth, nDay, nHour, nForecastTime);
			}
		}

		// 保存到单个文件中
		if (!m_stProduct.stFileSave.bDisabled)
		{
			QString strSavePath = HandleCommon::GetFolderPath(m_stProduct.stFileSave.strSavePath, nYear, nMonth, nDay, m_stProduct.strType);
			QString strSaveFileName = HandleCommon::GetFileName(m_stProduct.stFileSave.strFileName, nYear, nMonth, nDay, nHour, nForecastTime, nTimeRange);
				
			// 进行最后生成时间的处理判断，只针对没有生成过的
			if (bHandleLastTime(nHour))
			{
				// 已到最后处理时间
				if (!IsHasProcessed(strSaveFileName))
				{
					// 如果之前该文件没有生成成功过，则强制生成该数据文件,忽略是否有新数据
					bMergeFlg = true;
				}
			}

			if (bMergeFlg)
			{
				if (m_stProduct.stFileSave.strFileFormat == "grib")
				{
					// 保存到grib2文件中
					nErr = WriteData(strSavePath, strSaveFileName, fSrcData, nSrcDataLen, 1, nYear, nMonth, nDay, nHour, 0, 0, nForecastTime, nTimeRange, m_stProduct.stFileSave.lstDDS, false);
				}
				else if (m_stProduct.stFileSave.strFileFormat == "micaps")
				{
					// 保存到micaps4文件中
					nErr = WriteData2Micaps(strSavePath, strSaveFileName, fSrcData, nYear, nMonth, nDay, nHour, nForecastTime, m_stProduct.stFileSave.lstDDS);
				}

				if (nErr == SUCCESS)
				{
					// 生成文件成功
					m_hasCompleteProduct_today[strSaveFileName] = true;
					LOG_(LOGID_MERGE, LOG_F("[%s][%04d%02d%02d%02d.%03d] 生成该时次的拼图文件成功[File=%s]."), LOG_STR(m_stProduct.strKey), nYear, nMonth, nDay, nHour, nForecastTime, LOG_STR(strSaveFileName));
				}
				else
				{
					LOG_(LOGID_MERGE, LOG_F("[%s][%04d%02d%02d%02d.%03d] 生成该时次的拼图文件失败[File=%s]，错误编码=%d."), LOG_STR(m_stProduct.strKey), nYear, nMonth, nDay, nHour, nForecastTime, LOG_STR(strSaveFileName), nErr);
				}
			}
		}
		else
		{
			LOG_(LOGID_MERGE, LOG_F("[%s][%04d%02d%02d%02d.%03d] 该时次拼图结束，没有新数据加入."), LOG_STR(m_stProduct.strKey), nYear, nMonth, nDay, nHour, nForecastTime);
		}

		// 一个时次的合并文件
		nDataLenSum += nSrcDataLen;
		if (!m_stProduct.stMergeFileSave.bDisabled)
		{
			if (fMergeData)
			{
				memcpy(fMergeData + nDataLenSum - nSrcDataLen, fSrcData, nSrcDataLen * sizeof(float));
			}
		}

		// 释放获取的数据空间
		if (fSrcData)
		{
			free(fSrcData);
			fSrcData = NULL;
		}
		if (stSrcDataField)
		{
			free(stSrcDataField);
			stSrcDataField = NULL;
		}
	}

	/* 5. 一个时次的文件合并后的文件生成 */
	if (!m_stProduct.stMergeFileSave.bDisabled && fMergeData != NULL && !bGetSrcDataFailed)
	{
		// 保存目录&文件名
		QString strSavePath = HandleCommon::GetFolderPath(m_stProduct.stMergeFileSave.strSavePath, nYear, nMonth, nDay, m_stProduct.strType);
		QString strSaveFileName = HandleCommon::GetFileName(m_stProduct.stMergeFileSave.strFileName, nYear, nMonth, nDay, nHour, m_stProduct.nMaxForecastTime, nTimeRange);
		QString strSaveMergeFileFlg = QString("MERGE_%1").arg(strSaveFileName);

		// todo进行最后生成时间的处理判断，只针对没有生成过的
		if (bHandleLastTime(nHour))
		{
			// 已到最后处理时间
			if (!IsHasProcessed(strSaveMergeFileFlg))
			{
				// 如果之前该文件没有生成成功过，则强制生成该数据文件,忽略是否有新数据
				bNewMergeDataFlg = true;
			}
		}

		// 有新数据
		if (bNewMergeDataFlg)
		{
			// 保存到文件中
			nErr = WriteData(strSavePath, strSaveFileName, fMergeData, nDataLenSum, nDataTotalCount,
				nYear, nMonth, nDay, nHour, 0, 0, m_stProduct.nMaxForecastTime, m_stProduct.nTimeRange, m_stProduct.stMergeFileSave.lstDDS, true);
			if (nErr == SUCCESS)
			{
				// 生成文件成功
				m_hasCompleteProduct_today[strSaveMergeFileFlg] = true;
				LOG_(LOGID_MERGE, LOG_F("[%s][%04d%02d%02d%02d.%03d] 生成一个时次的拼图合并文件成功[File=%s]."), LOG_STR(m_stProduct.strKey), nYear, nMonth, nDay, nHour, nForecastTime, LOG_STR(strSaveFileName));
			}
			else
			{
				LOG_(LOGID_MERGE, LOG_F("[%s][%04d%02d%02d%02d.%03d] 生成一个时次的拼图合并文件失败[File=%s]，错误编码=%d."), LOG_STR(m_stProduct.strKey), nYear, nMonth, nDay, nHour, nForecastTime, LOG_STR(strSaveFileName), nErr);
			}
		}
		else
		{
			LOG_(LOGID_MERGE, LOG_F("[%s][%04d%02d%02d%02d] 没有新数据加入,所以生成拼图文件."), LOG_STR(m_stProduct.strKey), nYear, nMonth, nDay, nHour);
		}
	}
	else
	{
		LOG_(LOGID_MERGE, LOG_F("[%s][%s] 生成一个时次的拼图合并数据失败."), LOG_STR(m_stProduct.strKey), LOG_STR(m_stProduct.strName));
	}
	// 释放空间
	if (fMergeData)
	{
		free(fMergeData);
		fMergeData = NULL;
	}

	return SUCCESS;
}

/************************************************************************/
/* 本地文件合并处理                                                     */
/************************************************************************/
int TaskProductMerge::ProcessLocalMerge(int nYear, int nMonth, int nDay, int nHour, int nForecastTime, int nTimeRange)
{
	float *fMaskData = NULL;

	float *fSrcData = NULL;
	nwfd_grib2_field* stSrcDataField = NULL;
	int nSrcDataCnt;
	int nSrcDataLen;
	int nSrcMergePos;

	float *fClientData = NULL;
	nwfd_grib2_field* stClientDataField = NULL;
	int nClinetCnt;
	int nClientDataLen;
	int nClientMergePos;

	float fMaskLon, fMaskLat;
	bool bExecResult = true;

	// 获取Mask数据
	fMaskData = CMask::getClass().GetMaskData(m_stProduct.stMask.strMaskFile);
	if (fMaskData == NULL)
	{
		// todo Mask数据获取失败，无法进行数据合并处理
		return -1;
	}

	// 模版数据:原拼图数据/指导报数据/缺测值
	fSrcData = GetSrcData(nYear, nMonth, nDay, nHour, 0, 0, nForecastTime, nTimeRange, stSrcDataField, nSrcDataCnt);
	if (fSrcData == NULL)
	{
		// 获取模版数据异常
		return -2;
	}
	nSrcDataLen = m_stProduct.nNi * m_stProduct.nNj;

	LOG_(LOGID_MERGE, LOG_F("[%s][%s][%04d%02d%02d%02d.%03d] 拼图开始 >>>."), LOG_STR(m_stProduct.strKey), LOG_STR(m_stProduct.strName), nYear, nMonth, nDay, nHour, nForecastTime);
	bool bMergeFlg = false;
	// 按照配置的合并列表顺序进行合并
	LIST_MERGE_ITEM::iterator iter;
	for (iter = m_stProduct.lstMergeItem.begin(); iter != m_stProduct.lstMergeItem.end(); iter++)
	{
		ST_MERGE_ITEM stMergeItem = *iter;
		QFileInfoList  lstFile;

		// 如果该客户端配置的目录为空，则跳过本产品
		if (stMergeItem.strSrcFolder.isEmpty() || stMergeItem.strSrcFileName.isEmpty())
		{
			LOG_(LOGID_MERGE, LOG_F(">>[%d]该客户端配置为空-跳过."), stMergeItem.nClientID);
			continue;
		}

		// 获取文件列表
		QString strItemSrcFolder = HandleCommon::GetFolderPath(stMergeItem.strSrcFolder, nYear, nMonth, nDay, m_stProduct.strType); //目录
		QString strItemFilterName = HandleCommon::GetFileFilterName(stMergeItem.strSrcFileName, nYear, nMonth, nDay,nHour);         // 文件名
		if (!GetProductFileList(strItemSrcFolder, strItemFilterName, lstFile))
		{
			// 获取失败
			LOG_(LOGID_DEBUG, LOG_F("[TaskPID:%d][%s] 获取目录文件失败（path=%s）"), m_nTaskPID, LOG_STR(m_stProduct.strKey), LOG_STR(strItemSrcFolder));
			LOG_(LOGID_MERGE, LOG_F(">>[%d]该客户端没有符合条件的文件(%s)."), stMergeItem.nClientID, LOG_STR(strItemFilterName));
			continue;
		}

		// 查找文件列表中的最新文件
		QFileInfoList::iterator iterFile;
		for (iterFile = lstFile.begin(); iterFile != lstFile.end(); iterFile++)
		{
			// 文件名
			QString strFileName = (*iterFile).fileName();

			// 跳过以tmp结尾的数据文件
			if (strFileName.endsWith(".tmp", Qt::CaseInsensitive))
			{
				continue;
			}

			// 检查该文件，该时次的是否处理过
			QString strFlg = QString("%1_%2").arg(strFileName).arg(nForecastTime);
			if (IsHasProcessed(strFlg))
			{
				// 已处理过，跳过
				continue;
			}

			// 解析文件名确认产品
			//if (HandleCommon::ParseFileName(strFileNametmp, stMergeItem.stFileNameFormat, year, month, day, hour, forecasttime))
			//{
			//	// 精确查找，本方法只针对单个时间的文件
			//	//if (year == nYear && month == nMonth && day == nDay && hour == nHour && forecasttime == nForecastTime)
			//	//{
			//	//	strFileName = strFileNametmp;
			//	//}
			//	if (year != nYear || month != nMonth || day != nDay || hour != nHour || forecasttime != nForecastTime)
			//	{
			//		continue;
			//	}
			//	strFileName = strFileNametmp;
			//}
			//else
			//{
			//	continue;
			//}
						///////----------------------------------------------------------------------------------------------------
			// 所有文件进行拼图
			// 没有找到合适的客户端数据文件，跳过本客户端本预报时效的处理

			//////////////////////////////////////// Merge 处理 ////////////////////////////////////////
			// 获取要合并的客户端数据
			int nErr = GetClientLocalData(strItemSrcFolder, strFileName, nYear,nMonth, nDay, nHour, nForecastTime, fClientData, stClientDataField, nClinetCnt);
			if (nErr != 0 || fClientData == NULL)
			{
				// 客户端数据获取失败
				LOG_(LOGID_MERGE, LOG_F(">>[%d]该客户端读取文件数据失败（%s）,错误编码."), stMergeItem.nClientID, LOG_STR(strFileName));
				continue;
			}

			LOG_(LOGID_DEBUG, LOG_F("[Key:%s][%04d-%02d-%02d] ClientID=%d,FileName=%s:文件信息(%f,%f,%f,%f,%f,%f,%d,%d) "), LOG_STR(m_stProduct.strKey), nYear, nMonth, nDay, 
				stMergeItem.nClientID, LOG_STR(strFileName), stClientDataField[0].lon1, stClientDataField[0].lon2, 
				stClientDataField[0].lat1, stClientDataField[0].lat2, stClientDataField[0].incrementi, stClientDataField[0].incrementj, stClientDataField[0].Ni, stClientDataField[0].Nj);

			// 格距判断
			if (!IS_EQUAL(stClientDataField[0].incrementi, m_stProduct.fDi) || !IS_EQUAL(stClientDataField[0].incrementj, m_stProduct.fDj))
			{
				LOG_(LOGID_DEBUG, LOG_F("[Key:%s][%04d-%02d-%02d]拼图:ClientID=%d,FileName=%s.格距(%0.2f,%0.2f)与配置不一致，不进行拼图处理"), LOG_STR(m_stProduct.strKey), nYear, nMonth, nDay, stMergeItem.nClientID, LOG_STR(strFileName), stClientDataField[0].incrementi, stClientDataField[0].incrementj);
				LOG_(LOGID_MERGE, LOG_F(">>[%d]该客户端文件格距(%0.2f,%0.2f)与配置不一致."), stMergeItem.nClientID, stClientDataField[0].incrementi, stClientDataField[0].incrementj);

				// 格距不相等，不能进行合并
				if (fClientData)
				{
					free(fClientData);
					fClientData = NULL;
				}
				if (stClientDataField)
				{
					free(stClientDataField);
					stClientDataField = NULL;
				}

				continue;
			}

			// 开始合并处理
			LOG_(LOGID_DEBUG, LOG_F("[Key:%s] 进行[%04d-%02d-%02d]拼图:ClientID=%d,FileName=%s."), LOG_STR(m_stProduct.strKey),nYear,nMonth,nDay,stMergeItem.nClientID, LOG_STR(strFileName));

			// todo 如何含有多个，进行甄别，暂不处理
			// 计算本地客户端数据格点数和数据长度，等间距处理
			//int nClientNi = (int)((stMergeItem.fLon2 - stMergeItem.fLon1) / m_stProduct.fDi + 0.1) + 1;
			//int nClientNj = (int)((stMergeItem.fLat2 - stMergeItem.fLat1) / m_stProduct.fDj + 0.1) + 1;
			//nClientDataLen = nClientNi * nClientNj;
			// 不使用配置的经纬度，使用文件中读取的经纬度
			nClientDataLen = stClientDataField[0].Ni * stClientDataField[0].Nj;

			// 执行合并处理
			if (stMergeItem.nClientID % 100 == 0)
			{
				// 特殊处理，该客户端为省级客户端，不按照Mask进行拼图，而是将数据覆盖到模版数据上，此客户端要配置到list最开始 
				for (int nPos = 0; nPos < m_stProduct.nNi * m_stProduct.nNj; nPos++)
				{
					// 此位置为要合并的数据，计算该点的经纬度
					GetLonLat(m_stProduct.fLon1, m_stProduct.fLat1, m_stProduct.fDi, m_stProduct.fDj, m_stProduct.nNi, m_stProduct.nNj, nPos, fMaskLon, fMaskLat);

					// 通过经纬度计算客户端的坐标
					nClientMergePos = GetMergerPos(fMaskLon, fMaskLat, stClientDataField[0].lon1, stClientDataField[0].lat1, stClientDataField[0].Ni);

					if (nClientMergePos >= 0 && nClientMergePos < nClientDataLen)
					{
						// 数据合并
						fSrcData[nPos] = fClientData[nClientMergePos];
					}
				}
			}
			else
			{
				// 其他客户端按照Mask文件进行匹配
				for (int nPos = 0; nPos < m_stProduct.stMask.nNi * m_stProduct.stMask.nNj; nPos++)
				{
					// 查找要合并的位置点： stMergeData.nClientID 与 Mask文件中MaskID一致
					if ((int)(fMaskData[nPos] + 0.1) != stMergeItem.nClientID)
					{
						continue;
					}

					// 此位置为要合并的数据，计算该点的经纬度
					GetMaskLonLat(m_stProduct.stMask, nPos, fMaskLon, fMaskLat);

					// 通过经纬度获取要合并数据的位置点
					nSrcMergePos = GetMergerPos(fMaskLon, fMaskLat, m_stProduct.fLon1, m_stProduct.fLat1, m_stProduct.nNi);
					//nClientMergePos = GetMergerPos(fMaskLon, fMaskLat, stMergeItem.fLon1, stMergeItem.fLat1, nClientNi);
					nClientMergePos = GetMergerPos(fMaskLon, fMaskLat, stClientDataField[0].lon1, stClientDataField[0].lat1, stClientDataField[0].Ni);

					if ((nSrcMergePos >= 0 && nSrcMergePos < nSrcDataLen) && (nClientMergePos >= 0 && nClientMergePos < nClientDataLen))
					{
						// 数据合并
						fSrcData[nSrcMergePos] = fClientData[nClientMergePos];
					}
					else
					{
						// 该点越界 todo
						LOG_(LOGID_ERROR, LOG_F("[TaskPID:%d][Key:%s] 进行文件合并时该点（%f,%f）数据导致元数据或客户端数据越界"), m_nTaskPID, LOG_STR(m_stProduct.strKey), fMaskLon, fMaskLat);
					}
				}
			}
			if (fClientData)
			{
				free(fClientData);
				fClientData = NULL;
			}
			if (stClientDataField)
			{
				free(stClientDataField);
				stClientDataField = NULL;
			}

			// 有新的数据进行拼图数据
			bMergeFlg = true;
		}
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	}

	// 有新拼图数据
	if (bMergeFlg)
	{
		// 处理完成，
		bExecResult = HandleComplete(fSrcData, nSrcDataLen, 1, nYear, nMonth, nDay, nHour, 0, 0, nForecastTime, nTimeRange, 1);
	}
	else
	{
		LOG_(LOGID_MERGE, LOG_F("[%s][%s][%04d%02d%02d%02d.%03d] 拼图开始 >>>."), LOG_STR(m_stProduct.strKey), LOG_STR(m_stProduct.strName), nYear, nMonth, nDay, nHour, nForecastTime);
	}

	// 释放获取的数据空间
	if (fSrcData)
	{
		free(fSrcData);
		fSrcData = NULL;
	}
	if (stSrcDataField)
	{
		free(stSrcDataField);
		stSrcDataField = NULL;
	}

	if (!bExecResult)
	{
		return -3;
	}

	return SUCCESS;

	//// 进行文件合并处理
	//int nErr = ProcessLocalMerge(stMergeItem, strFileName);
	//if (nErr == 0)
	//{
	//	// 处理成功
	//	LOG_(LOGID_PRODUCT, LOG_F("[Merge][%s][ClientID=%d][FileName:%s]文件合并处理成功."), LOG_STR(m_stProduct.strKey), stMergeItem.nClientID, LOG_STR(strFileName));
	//}
	//else
	//{
	//	// 处理失败
	//	LOG_(LOGID_PRODUCT, LOG_F("[Merge][%s][ClientID=%d][FileName:%s]文件合并处理失败，ErrCode=%d."), LOG_STR(m_stProduct.strKey), stMergeItem.nClientID, LOG_STR(strFileName), nErr);
	//}
}

/************************************************************************/
/* 本地文件合并处理                                                     */
/************************************************************************/
int TaskProductMerge::ProcessLocalMerge(ST_MERGE_ITEM stMergeItem, QString strFileName)
{
	// 按照多个数据压到一个grib2文件中进行处理
	float *fMaskData = NULL;
	float *fSrcData = NULL;
	float *fClientData = NULL;
	int nYear, nMonth, nDay, nHour, nMinute, nSecond, nForecastTime, nTimeRange;
	int nSrcDataLen, nClientDataLen;
	int nClinetCnt;
	int nSrcDataLenSum = 0;
	bool bExecResult = true;
	float fMaskLon, fMaskLat;
	int nSrcMergePos, nClientMergePos;

	nMinute = 0;
	nSecond = 0;

	// 解析文件名，获取基本信息
	if (!HandleCommon::ParseFileName(strFileName, stMergeItem.stFileNameFormat, nYear, nMonth, nDay, nHour, nForecastTime, nTimeRange))
	{
		return -1;
	}

	// 获取Mask数据
	fMaskData = CMask::getClass().GetMaskData(m_stProduct.stMask.strMaskFile);
	if (fMaskData == NULL)
	{
		// todo Mask数据获取失败，无法进行数据合并处理
		return -2;
	}

	// 计算本地客户端数据格点数和数据长度，等间距处理
	int nClientNi = (int)((stMergeItem.fLon2 - stMergeItem.fLon1) / m_stProduct.fDi + 0.1) + 1;
	int nClientNj = (int)((stMergeItem.fLat2 - stMergeItem.fLat1) / m_stProduct.fDj + 0.1) + 1;
	nClientDataLen = nClientNi * nClientNj;
	nSrcDataLen = m_stProduct.nNi * m_stProduct.nNj;

	// 获取客户端数据
	float *fClientDataTemp = NULL;
	nwfd_grib2_field* stClientDataField = NULL;
	fClientData = GetClientLocalData(stMergeItem.strSrcFolder, strFileName, stClientDataField, nClinetCnt);
	if (fClientData == NULL)
	{
		// 客户端数据获取失败
		return -4;
	}

	for (int n = 0; n < nClinetCnt; n++)
	{
		// 获取合并模版数据
		nwfd_grib2_field* stSrcDataField = NULL;
		int nSrcDataCnt;  
		// 以下处理按照元数据为1个处理 ***NOTICE***
		float * fSrcDataTemp = GetSrcData(nYear, nMonth, nDay, nHour, nMinute, nSecond, stClientDataField[n].forecast, stClientDataField[n].timerange, stSrcDataField, nSrcDataCnt);
		if (fSrcDataTemp == NULL)
		{
			// 原始数据获取失败，无法进行数据合并处理
			return -3;
		}
		if (fSrcData == NULL)
		{
			nSrcDataLenSum += nSrcDataLen;
			fSrcData = (float *)malloc(nSrcDataLen * sizeof(float));
			memcpy((char*)fSrcData, (char*)fSrcDataTemp, nSrcDataLen * sizeof(float));
			free(fSrcDataTemp);
			fSrcDataTemp = NULL;
		}
		else
		{
			nSrcDataLenSum += nSrcDataLen;
			fSrcData = (float *)realloc(fSrcData, nSrcDataLenSum * sizeof(float));
			memcpy((char*)fSrcData + (nSrcDataLenSum - nSrcDataLen), (char*)fSrcDataTemp, nSrcDataLen * sizeof(float));
			free(fSrcDataTemp);
			fSrcDataTemp = NULL;
		}

		// 元数据起始地址
		fSrcDataTemp = fSrcData + m_stProduct.nNi * m_stProduct.nNj * n;

		// 客户端数据起始地址
		fClientDataTemp = fClientData + nClientNi * nClientNj * n;

		// 执行合并处理
		for (int nPos = 0; nPos < m_stProduct.stMask.nNi * m_stProduct.stMask.nNj; nPos++)
		{
			// 查找要合并的位置点： stMergeData.nClientID 与 Mask文件中MaskID一致
			if ((int)(fMaskData[nPos] + 0.1) != stMergeItem.nClientID)
			{
				continue;
			}

			// 此位置为要合并的数据，计算该点的经纬度
			GetMaskLonLat(m_stProduct.stMask, nPos, fMaskLon, fMaskLat);

			// 通过经纬度获取要合并数据的位置点
			nSrcMergePos = GetMergerPos(fMaskLon, fMaskLat, m_stProduct.fLon1, m_stProduct.fLat1, m_stProduct.nNi);
			nClientMergePos = GetMergerPos(fMaskLon, fMaskLat, stMergeItem.fLon1, stMergeItem.fLat1, nClientNi);

			if (nSrcMergePos < nSrcDataLen && nClientMergePos < nClientDataLen)
			{
				// 数据合并
				fSrcDataTemp[nSrcMergePos] = fClientDataTemp[nClientMergePos];
			}
			else
			{
				// 该点越界 todo
				LOG_(LOGID_ERROR, LOG_F("[TaskPID:%d][Key:%s] 进行文件合并时该点（%f,%f）数据导致元数据或客户端数据越界"), m_nTaskPID, LOG_STR(m_stProduct.strKey), fMaskLon, fMaskLat);
			}
		}

		// 只存储数据到缓存
		HandleComplete(fSrcDataTemp, nSrcDataLen, 1, nYear, nMonth, nDay, nHour, nMinute, nSecond, stClientDataField[n].forecast, stClientDataField[n].timerange, 2);

		if (stSrcDataField)
		{
			free(stSrcDataField);
			stSrcDataField = NULL;
		}
	}

	// 只保存数据到文件中
	HandleComplete(fSrcData, nSrcDataLen, nClinetCnt, nYear, nMonth, nDay, nHour, nMinute, nSecond, nForecastTime, nTimeRange, 1);

	// 释放获取的数据空间
	if (fSrcData)
	{
		free( fSrcData);
		fSrcData = NULL;
	}
	if (fClientData)
	{
		free(fClientData);
		fClientData = NULL;
	}
	if (stClientDataField)
	{
		free(stClientDataField);
		stClientDataField = NULL;
	}
	return 0;
}

/************************************************************************/
/* 获取客户端数据                                                       */
/************************************************************************/
float * TaskProductMerge::GetClientLocalData(QString strPath, QString strFileName, nwfd_grib2_field* &stDataField, int &nCnt)
{
	QDateTime tToday = QDateTime::currentDateTime();
	// 替换路径中的时间
	strPath = HandleCommon::GetFolderPath(strPath, tToday.date().year(), tToday.date().month(), tToday.date().day());

	QString strFile = QString("%1/%2").arg(strPath).arg(strFileName);

	float *fLocalData = NULL;

	fLocalData = HandleGrib::getClass().GetNwfdData(strFile, 0, stDataField, nCnt);

	return fLocalData;
}

/************************************************************************/
/* 获取客户端数据                                                       */
/************************************************************************/
int TaskProductMerge::GetClientLocalData(QString strPath, QString strFileName, int nYear, int nMonth, int nDay, int nHour, int nForecastTime, float* & fData, nwfd_grib2_field* &stDataField, int &nCnt)
{
	QString strFile = QString("%1/%2").arg(strPath).arg(strFileName);

	int nErr = HandleGrib::getClass().GetNwfdData(strFile, m_stProduct.fOffSet, nYear, nMonth, nDay, nHour, nForecastTime, fData, stDataField, nCnt);

	return nErr;
}

/************************************************************************/
/* 保存数据到Grib2文件                                                  */
/************************************************************************/
int TaskProductMerge::SaveData2GribFile(float *fData, int nCnt, ST_PRODUCT stProduct, int nYear, int nMonth, int  nDay, int  nHour, int  nForecasttime, int nTimeRange, QString strGribFile, bool bIsMergeFile)
{
	if (fData == NULL || nCnt < 1)
	{
		// 传人参数不正确
		return -1;
	}

	// 获取Grib2内存申请空间估算
	unsigned int nMemSize = HandleGrib::getClass().GetGribMemSize(stProduct.nNi, stProduct.nNj, nCnt);
	if (nMemSize == 0)
	{
		// 估算内存空间失败
		return -2;
	}

	unsigned char * cgrib = NULL;
	try
	{
		// 申请内存空间
		cgrib = (unsigned char*)malloc(nMemSize * m_stProduct.nCnt);
		if (cgrib == NULL)
		{
			// 申请内存空间失败
			return -3;
		}
		
		int forecasttime;

		int nOff = 0;

		// 开始创建Grib2数据
		long lLen = HandleNwfdLib::getClass().nwfd_create(cgrib, nYear, nMonth, nDay, nHour, 0, 0, m_stProduct.nStatus);
		lLen = HandleNwfdLib::getClass().nwfd_addgrid(cgrib, stProduct.fLon1, stProduct.fLon2, stProduct.fLat1, stProduct.fLat2, stProduct.fDi, stProduct.fDj, stProduct.nNi, stProduct.nNj);
		
		for (int i = 0; i < nCnt; i++)
		{
			if (bIsMergeFile)
			{
				forecasttime =  (i + 1) * nTimeRange;
			}
			else
			{
				forecasttime = nForecasttime;
			}

			for (int j = 0; j < m_stProduct.nCnt; j++)
			{
				lLen = HandleNwfdLib::getClass().nwfd_addfield_simpled48(cgrib, stProduct.nCategory, stProduct.nElement[j], stProduct.nStatistical,
					nYear, nMonth, nDay, nHour, 0, 0, forecasttime, nTimeRange, fData + nOff, stProduct.nNi * stProduct.nNj);

				nOff += m_stProduct.nNi * m_stProduct.nNj;
			}
		}

		lLen = HandleNwfdLib::getClass().nwfd_end(cgrib);

		if (lLen <= 0)
		{
			// 获取Grib2数据失败
			free(cgrib);
			cgrib = NULL;

			return -4;
		}

		// 保存数据到Grib2文件（不检测目录，传人前检验）
		long lRet = HandleNwfdLib::getClass().nwfd_savetofile(cgrib, lLen, strGribFile.toLocal8Bit().data());
		if (lRet != 0)
		{
			// 保存数据到Grib2文件失败
			free(cgrib);

			return -5;
		}

		// 释放存储空间
		free(cgrib);
	}
	catch (...)
	{
		if (cgrib)
		{
			free(cgrib);
		}

		// 其他异常抛出
		return -6;
	}

	// 返回正确
	return 0;
}

/************************************************************************/
/* 数据分发处理                                                         */
/************************************************************************/
int TaskProductMerge::DataDistribution(QStringList lstDDS, QString strFileName, QString strPath)
{
	if (CDDSManager::getClass().IsDisabled())
	{
		// 数据分发管理功能禁用
		return 0;
	}

	// 添加到队列中
	ST_DDSFILE stDDSFile;
	stDDSFile.strProductKey = m_stProduct.strKey;
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

/************************************************************************/
/* 是否是最后处理时间                                                   */
/************************************************************************/
bool TaskProductMerge::bHandleLastTime(int nHour)
{
	if (!m_stProduct.hasForecast.contains(nHour))
	{
		return false;
	}

	ST_FORECAST stForcast = m_stProduct.hasForecast.value(nHour);
	if (stForcast.nLastHour != -1 && stForcast.nLastMinute != -1)
	{
		QDateTime tCurrentTime = QDateTime::currentDateTime();
		QString   strLatestTime = QString("").sprintf("%04d-%02d-%02d %02d:%02d:%02d", tCurrentTime.date().year(), tCurrentTime.date().month(), tCurrentTime.date().day(), stForcast.nLastHour, stForcast.nLastMinute, 0);
		QDateTime tLatestTime = QDateTime::fromString(strLatestTime, "yyyy-MM-dd hh:mm:ss");
		if (tCurrentTime.toTime_t() >= tLatestTime.toTime_t())
		{
			// 当前时间已超过最后处理时间
			return true;
		}
	}

	return false;
}
