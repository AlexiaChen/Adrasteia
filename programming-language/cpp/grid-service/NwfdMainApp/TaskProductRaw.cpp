#include "log.h" // log日志
#include "TaskProductRaw.h"
#include "HandleCommon.h"
#include "HandleGrib.h"
#include "HandleNwfdLib.h"
#include "DDSManager.h"
#include "DataManager.h"

#include <QDir>
#include <QDateTime>
#include <QDebug>

/************************************************************************/
/* 构造函数                                                             */
/************************************************************************/
TaskProductRaw::TaskProductRaw()
{
	m_strTaskName = "产品扫描处理任务";
	SetTaskType(TASK_RAW); // 任务类型

	// 初始化状态为僵死，不初始化不予执行任务
	m_emTaskState = _TASK_DEAD;

	// 文件删选标记,默认进行筛选
	m_strClearHasFlg = "";
	m_hasCompleteProduct_today.clear();    // 已处理产品标记_今天
	m_hasCompleteBatch_today.clear();      // 已处理过的产品批次_今天
	m_hasCompleteProduct_yesterday.clear();// 已处理产品标记_昨天
	m_hasCompleteBatch_yesterday.clear();  // 已处理过的产品批次_昨天

	m_pCachedImpl = NULL;
}
/************************************************************************/
/* 析构函数                                                             */
/************************************************************************/
TaskProductRaw::~TaskProductRaw()
{
	// 停止线程执行
	LOG_(LOGID_DEBUG, LOG_F("[TaskPID:%d]开始停止线程执行."), m_nTaskPID);
	m_bReady = false;
	wait();

	// 删除缓存连接
	if (m_pCachedImpl)
	{
		delete m_pCachedImpl;
		m_pCachedImpl = NULL;
	}

	// 清空数据处理列表
	ClearChainsGroup();
}

/************************************************************************/
/* 初始化                                                               */
/************************************************************************/
void TaskProductRaw::Init(ST_PRODUCT  stProduct, ST_CACHED stCached)
{
	// 1、赋值：产品信息，缓存配置
	m_stProduct = stProduct;
	m_stCached = stCached;

	// 2、初始化数据处理列表
	if (InitChains())
	{
		LOG_(LOGID_INFO, LOG_F("[TaskPID:%d][%s]初始化数据加工处理chains成功）."), m_nTaskPID, LOG_STR(m_stProduct.strKey));
	}
	else
	{
		LOG_(LOGID_INFO, LOG_F("[TaskPID:%d][%s]初始化数据加工处理chains失败"), m_nTaskPID, LOG_STR(m_stProduct.strKey));
	}

	// 3、初始化缓存数据
	// 如果不存储，则不进行创建缓存连接
	if (m_stProduct.stCachedSave.bDisabled)
	{
		LOG_(LOGID_INFO, LOG_F("[TaskPID:%d][%s]该任务不进行缓存存储"), m_nTaskPID, LOG_STR(m_stProduct.strKey));
	}
	else if (InitCached())
	{
		// 保存Key和Info信息 ｛Product, ProductInfo_Pkey｝
		if (m_pCachedImpl->SaveProductInfo(m_stProduct) == SUCCESS)
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
		LOG_(LOGID_INFO, LOG_F("[TaskPID:%d]初始化缓存失败."), m_nTaskPID);
	}

	// 4、任务准备完毕，等待执行
	m_emTaskState = _TASK_READ;

	LOG_(LOGID_DEBUG, LOG_F("[TaskPID:%d]任务初始化完毕，准备就绪..."), m_nTaskPID);
}

/************************************************************************/
/* 设置                                                                 */
/************************************************************************/
void TaskProductRaw::setRootPath(QString strRootPath)
{
	m_strRoot = strRootPath;
}
void TaskProductRaw::setCCCC(QString strCCCC)
{
	m_strCccc = strCCCC;
}
void TaskProductRaw::setGridRange(float lon1, float lon2, float lat1, float lat2)
{
	m_fLon1 = lon1;
	m_fLon2 = lon2;
	m_fLat1 = lat1;
	m_fLat2 = lat2;
}

/************************************************************************/
/* 初始化数据处理列表                                                   */
/************************************************************************/
bool TaskProductRaw::InitChains()
{
	// 清空数据处理列表组
	ClearChainsGroup();

	bool bInitChainsFlg = true;
	int  nRet = 0;

	// 创建数据处理对象
	HASH_CHAINS_GROUP::iterator iterGroup;
	for (iterGroup = m_stProduct.hasDataProc.begin(); iterGroup != m_stProduct.hasDataProc.end(); iterGroup++)
	{
		ST_DATAPROC stDataProc = iterGroup.value();
		if (stDataProc.stFileSave.bDisabled)
		{
			// 该数据加工处理链已禁用
			continue;
		}

		LIST_DATAPROC lstDataProc;

		// 动态库加载
		LIST_CHAINS::Iterator iter;
		for (iter = stDataProc.lstChains.begin(); iter != stDataProc.lstChains.end(); iter++)
		{
			ST_CHAIN stChain = *iter;
			QString  strLibName = stChain.strLibName;
			QString  strParams = stChain.strParams;

			DataProInterface * pDataPro = HandleDataProLib::getClass().GetDataProcesser(strLibName);
			if (pDataPro)
			{
				// 设置参数
				nRet = pDataPro->SetParams(strParams.toLocal8Bit().data());

				// nRet=0,添加成功
				if (nRet != 0)
				{
					LOG_(LOGID_ERROR, LOG_F("[TaskPID:%d][Key:%s]数据加工处理初始化，设置参数失败（动态库：%s;参数：%s;错误编码：%d）."), m_nTaskPID, LOG_STR(m_stProduct.strKey), LOG_STR(strLibName), LOG_STR(strParams), nRet);
				}

				// 添加到列表中
				lstDataProc.push_back(pDataPro);
			}
			else
			{
				LOG_(LOGID_ERROR, LOG_F("[TaskPID:%d][Key:%s]数据处理用动态库 %s 获取失败"), m_nTaskPID, LOG_STR(m_stProduct.strKey), LOG_STR(strLibName));

				// 因为动态库是顺序执行，一旦有没有获取成功的数据处理对象，则不对其处理
				// 清理掉该列表中的数据处理对象
				ClearChains(lstDataProc);

				bInitChainsFlg = false;

				break;
			}
		}

		// 该数据加工链处理完成，添加到数据加工处理组中
		if (lstDataProc.size() != 0)
		{
			m_hasDataProcGroup.insert(stDataProc.nID,lstDataProc);
		}
	}

	if (m_hasDataProcGroup.size() == 0 && !bInitChainsFlg)
	{
		bInitChainsFlg = false;
	}
	else
	{
		bInitChainsFlg = true;  // 表示所有的数据加工处理都被禁用
	}

	return bInitChainsFlg;
}

/************************************************************************/
/* 清空数据处理列表组                                                   */
/************************************************************************/
void TaskProductRaw::ClearChainsGroup()
{
	HASH_DATAPROC_GROUP::Iterator iter;
	for (iter = m_hasDataProcGroup.begin(); iter != m_hasDataProcGroup.end(); iter++)
	{
		LIST_DATAPROC lstDataProc = iter.value();
		ClearChains(lstDataProc);
	}

	m_hasDataProcGroup.clear();
}

/************************************************************************/
/* 清空数据处理列表                                                     */
/************************************************************************/
void TaskProductRaw::ClearChains(LIST_DATAPROC lstDataProc)
{
	LIST_DATAPROC::Iterator iter;
	for (iter = lstDataProc.begin(); iter != lstDataProc.end(); iter++)
	{
		DataProInterface * pDataProc = *iter;
		if (pDataProc)
		{
			delete pDataProc;
			pDataProc = NULL;
		}
	}

	lstDataProc.clear();
}

/************************************************************************/
/* 初始化缓存数据                                                       */
/************************************************************************/
bool TaskProductRaw::InitCached()
{
	// 建立缓存连接
	if (m_pCachedImpl == NULL)
	{
		m_pCachedImpl = new CachedImpl(m_stCached);
	}

	if (m_pCachedImpl == NULL)
	{
		// 创建失败
		LOG_(LOGID_ERROR, LOG_F("[TaskPID:%d][Key:%s]Cached连接创建失败"), m_nTaskPID, LOG_STR(m_stProduct.strKey));

		return false;
	}

	// 测试连接状态
	int nErr = m_pCachedImpl->ConnectStatus();
	QString strConnectStatus = (nErr == SUCCESS) ? "连接" : "断开";
	LOG_(LOGID_INFO, LOG_F("[TaskPID:%d][Key:%s] Cached连接状态: %s"), m_nTaskPID, LOG_STR(m_stProduct.strKey), LOG_STR(strConnectStatus));

	if (nErr != SUCCESS)
	{
		return false;
	}

	return true;
}

/************************************************************************/
/* 检查缓存连接状态                                                     */
/************************************************************************/
bool TaskProductRaw::CheckCachedStatus()
{
	if (m_pCachedImpl == NULL)
	{
		// 缓存连接没有创建，进行初始化
		InitCached();
	}

	// 检查缓存连接状态
	int nErr = m_pCachedImpl->ConnectStatus();
	if (nErr != SUCCESS)
	{
		// 重新建立连接
		m_pCachedImpl->ReConnect();

		// 获取连接状态
		nErr = m_pCachedImpl->ConnectStatus();
	}

	// 返回结果
	return (nErr == SUCCESS);
}

/************************************************************************/
/* 任务处理执行方法                                                     */
/************************************************************************/
void TaskProductRaw::run()
{
	LOG_(LOGID_INFO, LOG_F("[TaskPID:%d][Key:%s] 产品扫描处理任务（raw）开始执行."), m_nTaskPID, LOG_STR(m_stProduct.strKey));

	if (m_emTaskState != _TASK_READ)
	{
		// 如果任务没有在准备状态，不予执行此次任务
		LOG_(LOGID_INFO, LOG_F("[TaskPID:%d][Key:%s] [Notice]任务没有完成准备工作，不能执行，任务结束（当前状态为：%d）."), m_nTaskPID, LOG_STR(m_stProduct.strKey), (int)m_emTaskState);
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
			try
			{
				// 执行任务
				ExecRawTask();
			}
			catch (...)
			{
				LOG_(LOGID_ERROR, LOG_F("[TaskPID:%d]执行扫描处理任务异常"), m_nTaskPID);
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

	LOG_(LOGID_INFO, LOG_F("[TaskPID:%d][Key:%s] 产品扫描处理任务（raw）执行结束."), m_nTaskPID, LOG_STR(m_stProduct.strKey));
}

/************************************************************************/
/* 执行产品扫描处理任务                                                 */
/************************************************************************/
bool TaskProductRaw::ExecRawTask()
{
	bool  bResult = true;
	bool  bBatchResult = true;

	// 1、标记列表处理
	HandleClearHash();

	// raw任务 扫描模式默认设置-没有配置默认1天
	if (m_stProduct.strScanMode.isEmpty()) 
		m_stProduct.strScanMode = "day";

	LOG_(LOGID_INFO, LOG_F("[Key:%s] 扫描任务开始执行（扫描模式：%s）."), LOG_STR(m_stProduct.strKey), LOG_STR(m_stProduct.strScanMode));

	// 2、按照扫描模式进行处理
	// 当前统一使用此模式处理，对于没有设置扫描模式的，
	HandleLocalDataFromMode();

	LOG_(LOGID_INFO, LOG_F("[Key:%s] 扫描任务执行结束."), LOG_STR(m_stProduct.strKey));

	return true;
}

/************************************************************************/
/* 按照扫描模式进行处理：当前修改为统一使用此模式处理                   */
/************************************************************************/
bool TaskProductRaw::HandleLocalDataFromMode()
{
	int nErr = 0;
	int nYear, nMonth, nDay, nHour;
	QDateTime tStartDay;
	QDateTime tEndDay;

	// 1、根据扫描模式，获取起止时间
	if (!HandleCommon::GetDateTimeFromScanMode(m_stProduct.strScanMode, tStartDay, tEndDay))
	{
		LOG_(LOGID_ERROR, LOG_F("[Key:%s] 扫描模式设置错误."), LOG_STR(m_stProduct.strKey));

		return false;
	}

	// 2、按照时间进行处理单个文件和合并文件
	for (QDateTime tDay = tStartDay; tDay < tEndDay; tDay = tDay.addDays(1))
	{
		// 获取要处理的时期
		nYear = tDay.date().year();
		nMonth = tDay.date().month();
		nDay = tDay.date().day();

		if (m_stProduct.strDecMode == "auto")
		{
			// 如果解码是自动解码模式，则直接获取文件列表进行解码（只处理GRIB数据）
			HandleAutoProcess(nYear, nMonth, nDay);
		}
		else
		{
			// 遍历配置的起报时间
			HASH_FORECAST::iterator iter;
			for (iter = m_stProduct.hasForecast.begin(); iter != m_stProduct.hasForecast.end(); iter++)
			{
				ST_FORECAST stForecast = *iter;
				nHour = stForecast.nReportTime; // 起报时间

				// 执行该时次的扫描处理
				HandleProcess(nYear, nMonth, nDay, nHour);
			}
		}
	}

	return true;
}

/************************************************************************/
/* 处理某天的数据                                                       */
/************************************************************************/
int TaskProductRaw::HandleProcess(int nYear, int nMonth, int nDay, int nHour)
{
	// 1. 得到准确的src folder目录  目录可能包含[YYYY]/[MM]/[DD]/[TYPE]
	QString strSrcFolder = HandleCommon::GetFolderPath(m_stProduct.strSrcFolder, nYear, nMonth, nDay,m_strRoot,m_stProduct.strType,m_strCccc);

	LOG_(LOGID_DEBUG, LOG_F("[Key:%s]开始扫描产品文件目录：%s"), LOG_STR(m_stProduct.strKey), LOG_STR(strSrcFolder));

	// 2. 处理单个文件的逻辑
	if (!m_stProduct.stFileSave.bDisabled || !m_stProduct.stCachedSave.bDisabled ||
		m_hasDataProcGroup.size() > 0 || !m_stProduct.stStationProc.bDisabled)
	{
		// 符合单个文件的处理条件，生成单个文件/存储缓存/数据加工处理/格点到站点转换处理
		HandleProduct(strSrcFolder, nYear, nMonth, nDay, nHour);
	}

	//3. 处理合并文件的逻辑
	if (!m_stProduct.stMergeFileSave.bDisabled)
	{
		// 需要生成合并后的文件
		HandleProductBatch(strSrcFolder, nYear, nMonth, nDay, nHour);
	}

	return SUCCESS;
}

/************************************************************************/
/* 自动解码处理                                                         */
/************************************************************************/
int  TaskProductRaw::HandleAutoProcess(int nYear, int nMonth, int nDay)
{
	// 扫描文件目录
	QString strSrcFolder = HandleCommon::GetFolderPath(m_stProduct.strSrcFolder, nYear, nMonth, nDay, m_strRoot, m_stProduct.strType, m_strCccc);
	// 获取文件名格式
	QString strFileNameFilter = HandleCommon::GetFileFilterName(m_stProduct.strSrcFileName, nYear, nMonth, nDay, m_stProduct.strType, m_strCccc);

	LOG_(LOGID_DEBUG, LOG_F("[Key:%s]开始扫描产品文件目录：%s(文件名格式：%s)"), LOG_STR(m_stProduct.strKey), LOG_STR(strSrcFolder), LOG_STR(strFileNameFilter));

	// 检索符合条件的文件，得到文件列表
	QFileInfoList lstFile;
	if (!GetProductFileList(strSrcFolder, strFileNameFilter, lstFile))
	{
		// 获取文件列表失败
		return -1;
	}

	LOG_(LOGID_DEBUG, LOG_F("[Key:%s]获取该格式的文件个数为：%d."), LOG_STR(m_stProduct.strKey), lstFile.size());

	// 遍历文件，对文件进行处理
	for (int i = 0; i < lstFile.size(); ++i)
	{
		// 文件名
		QString strFileName = lstFile.value(i).fileName();

		// 如果已经处理成功，则不再处理
		if (IsHasProcessed(strFileName))
		{
			LOG_(LOGID_DEBUG, LOG_F("[Key:%s]文件（%s）已经处理过."), LOG_STR(m_stProduct.strKey), LOG_STR(strFileName));
			continue;
		}

		LOG_(LOGID_DEBUG, LOG_F("[Key:%s]开始处理产品文件：%s"), LOG_STR(m_stProduct.strKey), LOG_STR(strFileName));

		// 处理该时效的文件产品
		int nErr = AutoProcess(strSrcFolder, strFileName, nYear, nMonth, nDay);
		if (nErr == SUCCESS)
		{
			// 处理成功
			m_hasCompleteProduct_today[strFileName] = true;
			LOG_(LOGID_INFO, LOG_F("[Key:%s][FileName:%s]文件处理成功"), LOG_STR(m_stProduct.strKey), LOG_STR(strFileName));
		}
		else
		{
			// 其他错误
			m_hasCompleteProduct_today[strFileName] = false;
			LOG_(LOGID_INFO, LOG_F("[WARNING][Key:%s][FileName:%s]文件处理失败，错误编码=%d."), LOG_STR(m_stProduct.strKey), LOG_STR(strFileName),nErr);
		}

		// 每处理完一次，延时1毫秒
		msleep(1);

	} // end FileInfo List for loop

	return 0;
}
/************************************************************************/
/* 对某天某起报时的单个产品进行处理                                     */
/************************************************************************/
bool TaskProductRaw::HandleProduct(QString strSrcFolder, int nYear, int nMonth, int nDay, int nHour)
{
	LOG_(LOGID_DEBUG, LOG_F("[TaskPID:%d][Key:%s]单个产品处理开始."), m_nTaskPID, LOG_STR(m_stProduct.strKey));

	bool bResult = true;
	bool bRet = true;
	bool bHasProcess = false;

	// 按照配置的预报时效处理，不在配置范围内的不做处理
	for (int forecasttime = m_stProduct.nTimeRange; forecasttime <= m_stProduct.nMaxForecastTime; forecasttime += m_stProduct.nTimeRange)
	{
		// 得到文件筛选的表达式
		// 如果配置的文件名格式中有[FFF],表示此文件为单个数据文件
		// 如果是合并的GRIB文件，在进行配置时，可以用通配符*表示
		QString strFileNameFilter = HandleCommon::GetFileFilterName(m_stProduct.strSrcFileName, nYear, nMonth, nDay, nHour,
			forecasttime, m_stProduct.nTimeRange,m_stProduct.strType, m_strCccc);

		// 检索符合条件的文件，得到文件列表
		QFileInfoList lstFile;
		if (!GetProductFileList(strSrcFolder, strFileNameFilter, lstFile))
		{
			// 获取文件列表失败
			continue;
		}

		// 遍历文件，对文件进行处理
		for (int i = 0; i < lstFile.size(); ++i)
		{
			// 文件名
			QString strFileName = lstFile.value(i).fileName();

			// 处理标签: 文件名_预报时效
			QString strProcessedFlg = QString("%1_%2").arg(strFileName).arg(forecasttime);

			//如果已经处理成功，则不再处理
			if (IsHasProcessed(strProcessedFlg))
			{
				continue;
			}

			LOG_(LOGID_DEBUG, LOG_F("[TaskPID:%d][Key:%s][%04d%02d%02d%02d.%03d]开始处理产品：%s"), m_nTaskPID, LOG_STR(m_stProduct.strKey),
				nYear, nMonth, nDay, nHour, forecasttime, LOG_STR(strProcessedFlg));

			bHasProcess = true; // 有新的处理

			// 处理该时效的文件产品
			int nErr = Process(strSrcFolder, strFileName, nYear, nMonth, nDay, nHour, forecasttime);
			if (nErr == SUCCESS)
			{
				// 处理成功（如果配置不存储，也表示成功）
				m_hasCompleteProduct_today[strProcessedFlg] = true;
				LOG_(LOGID_INFO, LOG_F("[TaskPID:%d][Key:%s][FileName:%s]文件处理成功"), m_nTaskPID, LOG_STR(m_stProduct.strKey), LOG_STR(strFileName));
			}
			else
			{
				// 其他错误
				bRet = false;
				m_hasCompleteProduct_today[strProcessedFlg] = false;
				LOG_(LOGID_INFO, LOG_F("[WARNING][TaskPID:%d][Key:%s][FileName:%s]文件处理失败，错误编码=%d."), m_nTaskPID, LOG_STR(m_stProduct.strKey), LOG_STR(strFileName), nErr);
			}

			// 每处理完一次，延时1毫秒
			msleep(1);

		} // end FileInfo List for loop

	} // end forecast for loop

	// 格点到站点一个起报时间的处理
	if (!m_stProduct.stStationProc.bDisabled && bHasProcess)
	{
		// 将最新的插值后的站点数据写入到缓存中
		int nErr = DataManager::getClass().SaveMultiStation(m_stProduct.strType, nYear, nMonth, nDay, nHour, m_stProduct.nTimeRange, m_stProduct.nMaxForecastTime);
		if (nErr == SUCCESS)
		{
			// todo 处理成功
			LOG_(LOGID_INFO, LOG_F("[Key:%s][%04d%02d%02d%02d]保存插值后的所有站点数据成功"), LOG_STR(m_stProduct.strKey), 
				nYear, nMonth, nDay, nHour);
		}
		else
		{
			// todo 处理失败
			LOG_(LOGID_INFO, LOG_F("[Key:%s][%04d%02d%02d%02d]保存插值后的所有站点数据失败，错误编码=%d"), LOG_STR(m_stProduct.strKey),
				nYear, nMonth, nDay, nHour, nErr);
		}
	}

	LOG_(LOGID_DEBUG, LOG_F("[TaskPID:%d][Key:%s]单个产品处理结束"), m_nTaskPID, LOG_STR(m_stProduct.strKey));

	return bRet;
}

/************************************************************************/
/* 对某预报时间的数据进行处理                                           */
/************************************************************************/
int TaskProductRaw::Process(QString strPath, QString strFileName, int nYear, int nMonth, int nDay, int nHour, int nForecastTime)
{
	int  nErr = SUCCESS;
	bool bDataProcFlg = true;
	bool bSaveFileFlg = true;
	bool bSaveCachedFlg = true;
	bool bGrid2StatFlg = true;

	float* fData = NULL;
	nwfd_grib2_field* stDataField = NULL;
	int nCnt = 0;

   // 1.读取文件获取数据
	QString strFile = QString("%1/%2").arg(strPath).arg(strFileName);
	nErr = HandleGrib::getClass().GetNwfdData(strFile, m_stProduct.fOffSet, nYear, nMonth, nDay, nHour, nForecastTime, fData, stDataField, nCnt);
    
    if (nErr != SUCCESS)
	{
		// 获取数据失败
		return nErr;
	}
	if (fData == NULL)
	{
		// 获取的数据为空
		if (stDataField) free(stDataField);
       
		return GET_DATA_IS_NULL;
	}
	if (stDataField == NULL)
	{
		// 获取的数据为空
		if (fData) free(fData);

		return GET_DATAINFO_IS_NULL;
	}
	if (nCnt != m_stProduct.nCnt)
	{
		// 获取的数据个数与配置不一致,将无法正确处理
		if (fData) free(fData);
		if (stDataField) free(stDataField);
       
		return GET_DATA_CNT_ERROR;
	}

	if (stDataField[0].Ni != m_stProduct.nNi || stDataField[0].Nj != m_stProduct.nNj)
	{
		// 数据个数与配置的不一致
		if (fData) free(fData);
		if (stDataField) free(stDataField);

		return GET_DATA_NINJ_ERROR;
	}
	
	// 考虑风的情况，当element与配置的顺序不一致的时候，进行U,V数据块对调
	if (nCnt == 2)
	{
		QString strDataFormat = HandleGrib::getClass().GetDataFormat(strFile);
		if (strDataFormat == FORMAT_GRIB2 && stDataField[0].element != m_stProduct.nElement[0])
		{
			int tmpDataLen = stDataField[0].Ni*stDataField[0].Nj;

			float *tmpExchangeBuff = (float*)malloc(tmpDataLen*sizeof(float));
			if (tmpExchangeBuff)
			{
				memcpy(tmpExchangeBuff, fData, tmpDataLen * sizeof(float));
				memcpy(fData, fData + tmpDataLen, tmpDataLen * sizeof(float));
				memcpy(fData + tmpDataLen, tmpExchangeBuff, tmpDataLen * sizeof(float));

				free(tmpExchangeBuff);
			}
			else
			{
				// 申请临时空间失败
				if (fData) free(fData);
				if (stDataField) free(stDataField);

				return ERR_MEMTEMP_FAILED;
			}
		}
	}

	// 2. 数据加工处理
	bDataProcFlg = DataProcess(strFileName, fData, nYear, nMonth, nDay, nHour, nForecastTime, m_stProduct.nTimeRange);

	// 3. 单个文件保存处理
	bSaveFileFlg = SaveFile(strFileName, fData, nYear, nMonth, nDay, nHour, nForecastTime, m_stProduct.nTimeRange);
   

	// 4. 数据保存缓存处理
	//bSaveCachedFlg = SaveCached(strFileName, fData, nYear, nMonth, nDay, nHour, nForecastTime, m_stProduct.nTimeRange);

	// 5. 格点插值站点处理
	//bGrid2StatFlg = Grid2Station(strFileName, fData, nYear, nMonth, nDay, nHour, nForecastTime, m_stProduct.nTimeRange);

	// 6. 释放之前读取文件分配的资源
	if (fData) free(fData);
	if (stDataField) free(stDataField);

  
	// 7.处理结果
	return (bDataProcFlg & bSaveFileFlg) ? SUCCESS : 164;
}

/************************************************************************/
/* 针对GRIB数据的自动处理                                               */
/************************************************************************/
int TaskProductRaw::AutoProcess(QString strPath, QString strFileName, int nYear, int nMonth, int nDay)
{
	bool bRet = true;

	try{
		// 文件
		QString strFile = QString("%1/%2").arg(strPath).arg(strFileName);

		// 获取文件内容
		unsigned char * cgrib = HandleGrib::getClass().ReadGribFile(strFile);
		if (cgrib == NULL)
		{
			// 读取文件失败
			return 161;
		}
		// 判断文件类型
		if (memcmp(cgrib, "GRIB", 4) != 0 )
		{
			// 文件格式不符合条件
			free(cgrib);
			return 162;
		}

		// 获取文件信息
		nwfd_grib2_info stNwfdInfo;
		int ret = HandleNwfdLib::getClass().nwfd_gribinfo(cgrib, &stNwfdInfo);
		if (ret < 0)
		{
			// 解析文件信息失败
			free(cgrib);
			return 163;
		}

		// 循环每个field，读出所有的数据
		for (int i = 1; i <= stNwfdInfo.numfields; i += m_stProduct.nCnt)
		{
			nwfd_grib2_field field;

			// 获取Grib数据
			g2float* fld = HandleNwfdLib::getClass().nwfd_gribfield(cgrib, i, &field);
			if (fld == NULL)
			{
				continue;
			}

			// 判断数据是否符合处理标准
			if (field.Ni != m_stProduct.nNi || field.Nj != m_stProduct.nNj)
			{
				// 不符合大小
				HandleNwfdLib::getClass().nwfd_freefld(fld);
				continue;
			}

			float * fData = fld;
			if (m_stProduct.nCnt == 2)
			{
				// 风产品
				nwfd_grib2_field field2;
				g2float* fld2 = HandleNwfdLib::getClass().nwfd_gribfield(cgrib, i+1, &field2);
				if (fld2 == NULL)
				{
					HandleNwfdLib::getClass().nwfd_freefld(fld);
					continue;
				}
				// 判断UV顺序
				if (field.element != 2 || field2.element != 3)
				{
					HandleNwfdLib::getClass().nwfd_freefld(fld);
					HandleNwfdLib::getClass().nwfd_freefld(fld2);
					continue;
				}
				// 申请空间
				fData = (float *)malloc(m_stProduct.nNi * m_stProduct.nNj * 2 * sizeof(float));
				if (fData == NULL)
				{
					HandleNwfdLib::getClass().nwfd_freefld(fld);
					HandleNwfdLib::getClass().nwfd_freefld(fld2);
					continue;
				}

				memcpy(fData, fld, m_stProduct.nNi * m_stProduct.nNj * sizeof(float));
				memcpy(fData + m_stProduct.nNi * m_stProduct.nNj, fld2, m_stProduct.nNi * m_stProduct.nNj * sizeof(float));

				HandleNwfdLib::getClass().nwfd_freefld(fld2);
			}

			// 2. 数据加工处理
			if (!DataProcess(strFileName, fData, field.year, field.month, field.day, field.hour, field.forecast, m_stProduct.nTimeRange))
			{
				bRet = false;
			}

			// 3. 单个文件保存处理
			if (!SaveFile(strFileName, fData, field.year, field.month, field.day, field.hour, field.forecast, m_stProduct.nTimeRange))
			{
				bRet = false;
			}

			// 处理结束		
			if (m_stProduct.nCnt == 2)
			{
				// 风产品释放申请空间
				if (fData) free(fData);
			}

			HandleNwfdLib::getClass().nwfd_freefld(fld);
		}

		free(cgrib);
	}
	catch (...)
	{
		return 165;
	}

	// 164表示有处理失败的情况
	return bRet ? SUCCESS : 164;
}

/************************************************************************/
/* 保存文件处理                                                         */
/************************************************************************/
bool TaskProductRaw::SaveFile(QString strsSrcFileName, float *fData, int nYear, int nMonth, int nDay, int nHour, int nForecastTime, int nTimeRange)
{
	if (m_stProduct.stFileSave.bDisabled)
	{
		// 功能禁用
		return true;
	}

	// 处理标记
	QString strSaveFileFlg = QString("SaveFile_%1_%2").arg(strsSrcFileName).arg(nForecastTime);

	if (IsHasProcessed(strSaveFileFlg))
	{
		// 已经处理成功过
		return true;
	}

	QString strFilePath = HandleCommon::GetFolderPath(m_stProduct.stFileSave.strSavePath, nYear, nMonth, nDay, m_strRoot, m_stProduct.strType, m_strCccc);
	QString strFileName = HandleCommon::GetFileName(m_stProduct.stFileSave.strFileName, nYear, nMonth, nDay, nHour, nForecastTime, nTimeRange,m_stProduct.strType, m_strCccc);

	// 保存数据到文件中
	int nErr = HandleGrib::getClass().SaveData2File(fData, 1, m_stProduct.fLon1, m_stProduct.fLon2, m_stProduct.fLat1,
		m_stProduct.fLat2, m_stProduct.fDi, m_stProduct.fDj, m_stProduct.nNi, m_stProduct.nNj,
		nYear, nMonth, nDay, nHour, nForecastTime, m_stProduct.nTimeRange,
		m_stProduct.nCategory, m_stProduct.nElement, m_stProduct.nStatistical, m_stProduct.nStatus, m_stProduct.nCnt,
		m_stProduct.strName, m_stProduct.strLineVal, m_stProduct.fOffSet, m_stProduct.fMissingVal, strFilePath, strFileName, m_stProduct.stFileSave.strFileFormat, false);
	if (nErr == SUCCESS)
	{
		// 保存文件成功，进行数据分发
		CDDSManager::getClass().DataDistribution(m_stProduct.strKey, m_stProduct.stFileSave.lstDDS, strFileName, strFilePath);

		// 处理成功
		m_hasCompleteProduct_today[strSaveFileFlg] = true;
		LOG_(LOGID_PRODUCT, LOG_F("[Key:%s][生成文件成功][FileName:%s]"), LOG_STR(m_stProduct.strKey), LOG_STR(strFileName));
	}
	else
	{
		// 处理失败
		if (!m_hasCompleteProduct_today.contains(strSaveFileFlg))
		{
			// 之前没有处理过，写入日志
			LOG_(LOGID_PRODUCT, LOG_F("[Key:%s][生成文件失败][FileName:%s]错误代码=%d"), LOG_STR(m_stProduct.strKey), LOG_STR(strFileName), nErr);
		}

		m_hasCompleteProduct_today[strSaveFileFlg] = false;

		return false;
	}

	return true;
}

/************************************************************************/
/* 保存缓存处理                                                         */
/************************************************************************/
bool TaskProductRaw::SaveCached(QString strsSrcFileName, float *fData, int nYear, int nMonth, int nDay, int nHour, int nForecastTime, int nTimeRange)
{
	if (m_stProduct.stCachedSave.bDisabled)
	{
		// 功能禁用
		return true;
	}

	if (m_stCached.bDisabled)
	{
		// 缓存连接禁止启动, 单个数据缓存存储将不再进行
		return true;
	}

	// 缓存状态
	if (!CheckCachedStatus())
	{
		LOG_(LOGID_ERROR, LOG_F("[Key:%s] 缓存连接状态异常."), LOG_STR(m_stProduct.strKey));
		return false;
	}

	// 存储时效判断，是否该数据已经超过保存的时效
	if (!IsProductTimeValid(nYear, nMonth, nDay, nHour, m_stProduct.stCachedSave.nValidTime))
	{
		// 该数据已经超出缓存保留时效，将不在保存
		return true;
	}
	
	// 处理标记
	QString strSaveCachedFlg = QString("SaveCached_%1_%2").arg(strsSrcFileName).arg(nForecastTime);
	if (IsHasProcessed(strSaveCachedFlg))
	{
		// 已经处理成功过
		return true;
	}

	// 将nwfd数据存储到缓存中 [product]
	QString strProductTime = HandleCommon::GetCachedProductTime(nYear, nMonth, nDay, nHour, nForecastTime, nTimeRange);
	int nDataSize = m_stProduct.nNi * m_stProduct.nNj * m_stProduct.nCnt;
	int nErr = m_pCachedImpl->SaveProductData(m_stProduct.strKey.toLocal8Bit().data(), strProductTime.toLocal8Bit().data(), fData, nDataSize);
	if (nErr == SUCCESS)
	{
		// 处理成功
		LOG_(LOGID_PRODUCT, LOG_F("[Key:%s][存储缓存成功][ProductTime:%s]"), LOG_STR(m_stProduct.strKey), LOG_STR(strProductTime));
		m_hasCompleteProduct_today[strSaveCachedFlg] = true;
	}
	else
	{
		// 处理失败
		if (!m_hasCompleteProduct_today.contains(strSaveCachedFlg))
		{
			// 之前没有处理过，写入日志
			LOG_(LOGID_PRODUCT, LOG_F("[Key:%s][存储缓存失败][ProductTime:%s] 错误代码=%d"), LOG_STR(m_stProduct.strKey), LOG_STR(strProductTime), nErr);
		}

		m_hasCompleteProduct_today[strSaveCachedFlg] = false;

		return false;
	}

	return true;
}
/************************************************************************/
/* 格点插值站点处理                                                     */
/************************************************************************/
bool TaskProductRaw::Grid2Station(QString strsSrcFileName, float *fData, int nYear, int nMonth, int nDay, int nHour, int nForecastTime, int nTimeRange)
{
	if (m_stProduct.stStationProc.bDisabled)
	{
		// 功能禁用
		return true;
	}

	// 缓存状态
	if (!CheckCachedStatus())
	{
		LOG_(LOGID_ERROR, LOG_F("[Key:%s] 缓存连接状态异常."), LOG_STR(m_stProduct.strKey));
		return false;
	}

	// 处理标记
	QString strGrid2StaionFlg = QString("Grid2Stat_%1_%2").arg(strsSrcFileName).arg(nForecastTime);
	if (IsHasProcessed(strGrid2StaionFlg))
	{
		// 已经处理成功过
		return true;
	}

	// 要先存储格点merge数据，作为格点-站点一致的原始数据 [merge]
	QString strMergeTime = HandleCommon::GetCachedProductTime(nYear, nMonth, nDay, nHour, nForecastTime, nTimeRange);
	int DataSize = m_stProduct.nNi * m_stProduct.nNj * m_stProduct.nCnt;
	int nErr = m_pCachedImpl->SaveMergeData(m_stProduct.strKey.toLocal8Bit().data(), strMergeTime.toLocal8Bit().data(), fData, DataSize);
	if (nErr == SUCCESS)
	{
		// 存储成功
		LOG_(LOGID_DEBUG, LOG_F("[%s][%s]站点插值前Merge数据缓存存储成功."), LOG_STR(m_stProduct.strKey), LOG_STR(strMergeTime));
	}
	else
	{
		// 存储失败
		LOG_(LOGID_DEBUG, LOG_F("[%s][%s]站点插值前Merge数据缓存存储失败（错误编码：%d）."), LOG_STR(m_stProduct.strKey), LOG_STR(strMergeTime), nErr);
	}

	// 再插值站点
	nErr = DataManager::getClass().Grid2Station(m_stProduct.strType, nTimeRange, m_stProduct.strKey, fData, m_stProduct.fLon1, m_stProduct.fLat1, 
		m_stProduct.fDi, m_stProduct.fDj, m_stProduct.nNi, m_stProduct.nNj, nYear, nMonth, nDay, nHour, nForecastTime, true);
	if (nErr == SUCCESS)
	{
		// 处理成功
		LOG_(LOGID_PRODUCT, LOG_F("[Key:%s][格点插值站点成功][ProductTime:%s]"), LOG_STR(m_stProduct.strKey), LOG_STR(strMergeTime));
		m_hasCompleteProduct_today[strGrid2StaionFlg] = true;
	}
	else
	{
		// 处理失败
		if (!m_hasCompleteProduct_today.contains(strGrid2StaionFlg))
		{
			// 之前没有处理过，写入日志
			LOG_(LOGID_PRODUCT, LOG_F("[Key:%s][格点插值站点失败][ProductTime:%s] 错误代码=%d"), LOG_STR(m_stProduct.strKey), LOG_STR(strMergeTime), nErr);
		}

		m_hasCompleteProduct_today[strGrid2StaionFlg] = false;

		return false;
	}

	return true;
}

/************************************************************************/
/* 数据加工处理                                                         */
/************************************************************************/
bool TaskProductRaw::DataProcess(QString strsSrcFileName, float *fData, int nYear, int nMonth, int nDay, int nHour, int nForecastTime, int nTimeRange)
{
	bool bRet = true;
	QString strDataProcFlg;

	HASH_DATAPROC_GROUP::Iterator iterDataProc;
	for (iterDataProc = m_hasDataProcGroup.begin(); iterDataProc != m_hasDataProcGroup.end(); iterDataProc++)
	{
		int nDataProcID = iterDataProc.key();

		strDataProcFlg = QString("DataProc_%1_%2_%3").arg(nDataProcID).arg(strsSrcFileName).arg(nForecastTime);
		if (IsHasProcessed(strDataProcFlg))
		{
			// 已经处理成功过,跳过
			continue;
		}

		LIST_DATAPROC lstDataProc = iterDataProc.value();
		ST_DATAPROC stDataProc;
		if (!FindDataProcInfo(nDataProcID, stDataProc))
		{
			// 中间过程出现异常，一般不会出现
			LOG_(LOGID_ERROR, LOG_F("[Key:%s]没有找到ID=%d的数据加工处理信息"), LOG_STR(m_stProduct.strKey), nDataProcID);
			continue;
		}

		// 基本信息
		nwfd_data_field *stDataFieldInfo = (nwfd_data_field *)malloc(m_stProduct.nCnt * sizeof(nwfd_data_field));
		if (stDataFieldInfo == NULL)
		{
			// 跳过本次数据加工处理
			continue;
		}
		for (int n = 0; n < m_stProduct.nCnt; n++)
		{
			stDataFieldInfo[n].category = m_stProduct.nCategory;
			stDataFieldInfo[n].element = m_stProduct.nElement[n];
			stDataFieldInfo[n].statistical = m_stProduct.nStatistical;
			stDataFieldInfo[n].status = m_stProduct.nStatus;
			stDataFieldInfo[n].lon1 = m_stProduct.fLon1;
			stDataFieldInfo[n].lon2 = m_stProduct.fLon2;
			stDataFieldInfo[n].lat1 = m_stProduct.fLat1;
			stDataFieldInfo[n].lat2 = m_stProduct.fLat2;
			stDataFieldInfo[n].Ni = m_stProduct.nNi;  // 赋值 经度格点数
			stDataFieldInfo[n].Nj = m_stProduct.nNj;  // 赋值 纬度格点数
			stDataFieldInfo[n].Di = m_stProduct.fDi;
			stDataFieldInfo[n].Dj = m_stProduct.fDj;
			stDataFieldInfo[n].year = nYear;
			stDataFieldInfo[n].month = nMonth;
			stDataFieldInfo[n].day = nDay;
			stDataFieldInfo[n].hour = nHour;
			stDataFieldInfo[n].minute = 0;
			stDataFieldInfo[n].second = 0;
			stDataFieldInfo[n].forecast = nForecastTime;
			stDataFieldInfo[n].timerange = nTimeRange;
		}

		// 数据加工处理
		int nErr = DataProcess(stDataProc, lstDataProc, fData, stDataFieldInfo, m_stProduct.nCnt);
		if (nErr == SUCCESS)
		{
			LOG_(LOGID_PRODUCT, LOG_F("[Key:%s][数据加工处理成功][处理编号:%d][%04d%02d%02d%02d.%03d]"), LOG_STR(m_stProduct.strKey), nDataProcID, nYear, nMonth, nDay, nHour, nForecastTime);

			m_hasCompleteProduct_today[strDataProcFlg] = true;
		}
		else
		{
			bRet = false;
			if (!m_hasCompleteProduct_today.contains(strDataProcFlg))
			{
				LOG_(LOGID_PRODUCT, LOG_F("[Key:%s][数据加工处理失败][处理编号:%d][%04d%02d%02d%02d.%03d]错误代码=%d"), LOG_STR(m_stProduct.strKey), nDataProcID, nYear, nMonth, nDay, nHour, nForecastTime, nErr);
			}
			
			m_hasCompleteProduct_today[strDataProcFlg] = false;
		}

		// 释放申请的数据信息空间
		if (stDataFieldInfo)
		{
			free(stDataFieldInfo);
		}
	}

	return bRet;
}

/************************************************************************/
/* 数据加工处理                                                         */
/************************************************************************/
int TaskProductRaw::DataProcess(ST_DATAPROC stDataProc, LIST_DATAPROC lstDataProc, float *fData, nwfd_data_field *stDataFieldInfo, int  nCnt)
{
	if (fData == NULL || stDataFieldInfo == NULL || nCnt <= 0)
	{
		// 参数错误
		LOG_(LOGID_DEBUG, LOG_F("[TaskPID:%d]数据加工处理，参数错误."), m_nTaskPID);

		return ERR_PARAM;
	}
	if (lstDataProc.size() == 0)
	{
		// 没有数据加工对象
		LOG_(LOGID_DEBUG, LOG_F("[TaskPID:%d]没有数据加工对象"), m_nTaskPID);

		return NO_DATAPROC_CHAINS;
	}

	// 输入数据
	float * fInData = NULL;
	nwfd_data_field *stInDataFieldInfo = NULL;

	// 输出数据（经过加工处理后最终要处理的数据）
	float * fOutData = fData;
	nwfd_data_field *stOutDataFieldInfo = stDataFieldInfo;

	// 数据个数
	int nInOutCnt = nCnt;
	unsigned int nDataLen = 0;
	unsigned int nDataLenSum = 0;

	// 返回值，参数
	int nRet = 0;
	QString strParams;

	/* 1、 顺序数据加工 */
	LIST_DATAPROC::Iterator iter;
	DataProInterface * pDataPro = NULL;
	for (iter = lstDataProc.begin(); iter != lstDataProc.end(); iter++)
	{
		// 获取数据处理指针
		pDataPro = *iter;
		if (pDataPro == NULL)
		{
			// 加工处理对象异常
			LOG_(LOGID_DEBUG, LOG_F("[TaskPID:%d]数据加工处理对象指针为空"), m_nTaskPID);
			return DATAPROC_IS_NULL;
		}

		// 输出变成输入，以备下次处理
		fInData = fOutData;
		stInDataFieldInfo = stOutDataFieldInfo;

		// 调用接口进行处理(nInOutCnt为数组个数，即为输入，也为输出，处理时不关心其变化，直接扔给下一条处理)
		if (pDataPro->GetType() == BLOCK && strlen(pDataPro->GetParams()) == 0)
		{
			// 获取数据块处理
			QString strParams = QString("%1,%2,%3,%4").arg(m_fLon1).arg(m_fLon2).arg(m_fLat1).arg(m_fLat2);
			
            nRet = pDataPro->DataProcess(strParams.toLocal8Bit().data(), fInData, stInDataFieldInfo, fOutData, stOutDataFieldInfo, nInOutCnt, m_stProduct.fMissingVal);
		}
		else if (pDataPro->GetType() == SCALE)
		{
			// 升降尺度处理
			QString strParams = QString::fromLocal8Bit(pDataPro->GetParams());
			if (strParams.isEmpty())
			{
				// 参数设置错误
				break;
			}
			strParams = QString("%1,%2").arg(strParams).arg(m_stProduct.bIsWarning);
			nRet = pDataPro->DataProcess(strParams.toLocal8Bit().data(), fInData, stInDataFieldInfo, fOutData, stOutDataFieldInfo, nInOutCnt, m_stProduct.fMissingVal);
		}
		else
		{
			nRet = pDataPro->DataProcess(fInData, stInDataFieldInfo, fOutData, stOutDataFieldInfo, nInOutCnt);
		}
		if (nRet != 0 || fOutData == NULL || stOutDataFieldInfo == NULL || nInOutCnt <= 0)
		{
			// 处理失败，结束循环
			break;
		}

		// 释放上次处理的存储空间（除第一次处理外）
		if (fInData != NULL && fInData != fData)
		{
			free(fInData);
		}
		if (stInDataFieldInfo != NULL && stInDataFieldInfo != stDataFieldInfo)
		{
			free(stInDataFieldInfo);
		}

		// 创建新的存储空间，用于存储返回的数据和数据信息
		nwfd_data_field * stDataFieldInfoTemp = (nwfd_data_field *)malloc(nInOutCnt * sizeof(nwfd_data_field));
		if (stDataFieldInfoTemp == NULL)
		{
			nRet = ERR_MEM_MALLOC;
			break;
		}
		nDataLenSum = 0;
		for (int n = 0; n < nInOutCnt; n++)
		{
			stDataFieldInfoTemp[n] = stOutDataFieldInfo[n];
			nDataLen = stDataFieldInfoTemp[n].Ni * stDataFieldInfoTemp[n].Nj;  // 数据长度
			nDataLenSum += nDataLen;

		}

		float * fTemp = (float *)malloc(nDataLenSum * sizeof(float));
		if (fTemp == NULL)
		{
			// 创建存储空间失败，结束循环
			nRet = ERR_MEM_MALLOC;
			break;
		}
		memcpy(fTemp, fOutData, nDataLenSum * sizeof(float));

		// 调用dll接口释放返回的数据
		pDataPro->FreeData(fOutData, stOutDataFieldInfo);
		// 重新赋值输出数据
		fOutData = fTemp;
		stOutDataFieldInfo = stDataFieldInfoTemp;
	}

	// 处理链中有处理失败的情况发生,没有完全处理完
	if (iter != lstDataProc.end())
	{
		// 释放空间
		if (fOutData != NULL && fOutData != fData)
		{
			free(fOutData);
		}
		if (stOutDataFieldInfo != NULL && stOutDataFieldInfo != stDataFieldInfo)
		{
			free(stOutDataFieldInfo);
		}


		return nRet;
	}

	/* 2、 获取Grib2数据并存储到文件中 */
	QString strFilePath = HandleCommon::GetFolderPath(stDataProc.stFileSave.strSavePath, stOutDataFieldInfo[0].year,
		stOutDataFieldInfo[0].month, stOutDataFieldInfo[0].day, m_strRoot, m_stProduct.strType, m_strCccc);
	QString strFileName = HandleCommon::GetFileName(stDataProc.stFileSave.strFileName, stOutDataFieldInfo[0].year,
		stOutDataFieldInfo[0].month, stOutDataFieldInfo[0].day, stOutDataFieldInfo[0].hour, stOutDataFieldInfo[0].forecast, 
		m_stProduct.nTimeRange, m_stProduct.strType, m_strCccc);

	// 保存数据到文件中
   
	int nErr = HandleGrib::getClass().SaveData2File(fOutData, 1, stOutDataFieldInfo[0].lon1, stOutDataFieldInfo[0].lon2,
		stOutDataFieldInfo[0].lat1, stOutDataFieldInfo[0].lat2, stOutDataFieldInfo[0].Di, stOutDataFieldInfo[0].Dj, stOutDataFieldInfo[0].Ni, stOutDataFieldInfo[0].Nj,
		stOutDataFieldInfo[0].year, stOutDataFieldInfo[0].month, stOutDataFieldInfo[0].day,
		stOutDataFieldInfo[0].hour, stOutDataFieldInfo[0].forecast, m_stProduct.nTimeRange,
		m_stProduct.nCategory, m_stProduct.nElement, m_stProduct.nStatistical, m_stProduct.nStatus, m_stProduct.nCnt,
		m_stProduct.strName, m_stProduct.strLineVal, m_stProduct.fOffSet, m_stProduct.fMissingVal, strFilePath, strFileName, stDataProc.stFileSave.strFileFormat, false);
	if (nErr == SUCCESS)
	{
		// 保存文件成功，进行数据分发
		CDDSManager::getClass().DataDistribution(m_stProduct.strKey, m_stProduct.stFileSave.lstDDS, strFileName, strFilePath);
	}

	// 释放空间
	if (fOutData != NULL && fOutData != fData)
	{
		free(fOutData);
	}
	if (stOutDataFieldInfo != NULL && stOutDataFieldInfo != stDataFieldInfo)
	{
		free(stOutDataFieldInfo);
	}

	return nErr;
}

/************************************************************************/
/* 文件合并处理                                                        */
/************************************************************************/
bool TaskProductRaw::HandleProductBatch(QString strSrcFolder, int nYear, int nMonth, int nDay, int nHour)
{
	bool bRet = true;
	bool bFileTimeUpdate = false;  // 文件时间是否有更新

	// 得到文件批次
	QString strBatchName = QString("").sprintf("%04d%02d%02d%02d", nYear, nMonth, nDay, nHour);

	// 如果该文件批次已经处理过，那么就跳过本次处理
	if (IsHasProcessedBatch(strBatchName))
	{
		// 20160530 检测原文件时间有没有变化
		// 如果变化表示文件有更新，虽然已经成功生成过，但是还是需要重新生成该文件
		if (IsFileTimeUpdate(strBatchName))
		{
			// 文件时间有更新
			bFileTimeUpdate = true;
		}
		else
		{
			// 文件没有更新，并且也已经成功处理过， 所以
			return true;
		}
	}

	int nErr;
	nErr = ProcessBatch(strSrcFolder, nYear, nMonth, nDay, nHour, bFileTimeUpdate);

	if (nErr == SUCCESS) // 处理成功
	{
		m_hasCompleteBatch_today[strBatchName] = true;
		LOG_(LOGID_INFO, LOG_F("[Key:%s][BatchName:%s]该时效批次文件处理成功"), LOG_STR(m_stProduct.strKey), LOG_STR(strBatchName));
	}
	else if (nErr == 111)
	{
		bRet = false;
		QString strBatchNameErrFlg = QString("%1_%2").arg(strBatchName).arg(nErr);
		if (!m_hasCompleteBatch_today.contains(strBatchNameErrFlg))
		{
			// 防止多次写入,同一个批次的处理，同一个错误只写入一次
			LOG_(LOGID_PRODUCT, LOG_F("[Key:%s][BatchName:%s]该时效批次文件处理失败,错误代码：%d(原文件缺失)."), LOG_STR(m_stProduct.strKey), LOG_STR(strBatchName), nErr);
			m_hasCompleteBatch_today[strBatchNameErrFlg] = false;
		}

		m_hasCompleteBatch_today[strBatchName] = false;
	}
	else
	{
		bRet = false;
		QString strBatchNameErrFlg = QString("%1_%2").arg(strBatchName).arg(nErr);
		if (!m_hasCompleteBatch_today.contains(strBatchNameErrFlg))
		{
			// 防止多次写入,同一个批次的处理，同一个错误只写入一次
			LOG_(LOGID_PRODUCT, LOG_F("[Key:%s][BatchName:%s]该时效批次文件处理失败,错误代码：%d."), LOG_STR(m_stProduct.strKey), LOG_STR(strBatchName), nErr);
			m_hasCompleteBatch_today[strBatchNameErrFlg] = false;
		}

		m_hasCompleteBatch_today[strBatchName] = false;
	}

	return bRet;
}

/************************************************************************/
/* 文件合并处理                                                         */
/************************************************************************/
int TaskProductRaw::ProcessBatch(QString strSrcFolder, int nYear, int nMonth, int nDay, int nHour, bool bFileTimeUpdate)
{
	int nErr = SUCCESS;
	QString strBatchName = QString("").sprintf("%04d%02d%02d%02d", nYear, nMonth, nDay, nHour); // 合并批次

	// 1、确定起始终止预报时效
	// forecast 配置中是否包含该时次，如果没有则不合并
	if (!m_stProduct.hasForecast.contains(nHour))
	{
		return NO_REPORTHOUR_CFG;
	}

	// 获取forecast 配置
	ST_FORECAST stForcast = m_stProduct.hasForecast.value(nHour);
	int startForecast = stForcast.nStartForecast;
	int endForecast = stForcast.nEndForecast;
	const int rangetime = m_stProduct.nTimeRange;

	// 如果其中start 或 end 不配置,那么就按照默认的
	if (startForecast == 0 || endForecast == 0)
	{
		startForecast = m_stProduct.nTimeRange;
		endForecast = m_stProduct.nMaxForecastTime;
	}

	// 检查时效，end必须大于等于start 同时也必须整除start，不然时间段就不正确
	if (endForecast < startForecast || endForecast % startForecast != 0)
	{
		LOG_(LOGID_DEBUG, LOG_F("[TaskPID:%d][ProductKey:%s] [BatchName:%s] 的forecast配置有误"), m_nTaskPID, LOG_STR(m_stProduct.strKey), LOG_STR(strBatchName));
		return FORECAT_RANGE_CFG_ERROR;
	}

	int fileCount; // 数据个数
	float * fNwfdData = NULL; // 合并后数据
	int nOff = 0; // 合并数据偏移量
	int nDataLen; // 单个数据长度
	bool bIsNotWhole = false; // 是否是不完整数据

	try{
		// 数据个数
		fileCount = endForecast / startForecast;
		nDataLen = m_stProduct.nNi * m_stProduct.nNj * m_stProduct.nCnt;

		// 申请存储空间
		fNwfdData = (float*)malloc(fileCount * nDataLen * sizeof(float));
		if (fNwfdData == NULL)
		{
			LOG_(LOGID_DEBUG, LOG_F("[Key:%s]批次合并时申请存储空间失败（个数=%d）."), LOG_STR(m_stProduct.strKey), fileCount);

			return ERR_MEM_MALLOC;
		}

		// 开始获取数据处理
		for (int forecasttime = startForecast; forecasttime <= endForecast; forecasttime += rangetime)
		{
			// 得到准确的文件名
			QString strFileName = HandleCommon::GetFileName(m_stProduct.strSrcFileName, nYear, nMonth, nDay, nHour, forecasttime, rangetime);
			QString strFile = QString("%1/%2").arg(strSrcFolder).arg(strFileName);

			LOG_(LOGID_DEBUG, LOG_F("[Key:%s][BatchName:%s]开始处理文件%s."), LOG_STR(m_stProduct.strKey), LOG_STR(strBatchName), LOG_STR(strFile));

			// 获取数据
			float * fData = NULL;
			nwfd_grib2_field* stDataField = NULL;
			int nCnt = 0;
			nErr = HandleGrib::getClass().GetNwfdData(strFile, m_stProduct.fOffSet, nYear, nMonth, nDay, nHour, forecasttime, fData, stDataField, nCnt);

			// 获取数据失败 
			if (nErr != SUCCESS || fData == NULL || stDataField == NULL || nCnt != m_stProduct.nCnt ||
				m_stProduct.nNi != stDataField[0].Ni || m_stProduct.nNj != stDataField[0].Nj)
			{
				LOG_(LOGID_DEBUG, LOG_F("[Key:%s][BatchName:%s] 获取数据失败，错误编码=%d."), LOG_STR(m_stProduct.strKey), LOG_STR(strBatchName), nErr);

				// 检查是否到最后处理时间，或者已经生成过一次不完整的数据
				if (!IsLastTimeReached(nHour) || IshasNotWholeBatchProcessed(strBatchName))
				{
					// 如果没到最后处理时间，或者没有配置最后处理时间，结束本次合并处理
					// 或者该批次的不完整数据已经生成过一次，则不需要再生成了
					if (fData)  free(fData);
					if (stDataField) free(stDataField);
					if (fNwfdData) free(fNwfdData);

					LOG_(LOGID_DEBUG, LOG_F("[Key:%s][BatchName:%s] 获取数据失败，不再处理，返回."), LOG_STR(m_stProduct.strKey), LOG_STR(strBatchName));

					// 获取数据失败，结束合并处理
					return (nErr != SUCCESS) ? nErr : GET_DATA_FAILED;
				}
				else
				{
					// 到最后处理时间，填充缺测数据
					fData = (float *)malloc(nDataLen * sizeof(float));
					if (fData == NULL)
					{
						// 申请空间失败，停止本次处理
						if (stDataField) free(stDataField);
						if (fNwfdData) free(fNwfdData);

						return ERR_MEMTEMP_FAILED;
					}

					// 填充缺测数据
					//std::fill_n(fData, nDataLen, 9999.0f);
					for (int i = 0; i < nDataLen; i++)
					{
						fData[i] = 9999.0f;
					}

					// 本次生成的数据为不完整数据
					bIsNotWhole = true;
				}
			}

			// 20160530 记录文件创建时间
			QString strCreatedTime = HandleCommon::GetFileCreatedTime(strFile);
			SetFileCreatedTime(strBatchName, strFile, strCreatedTime);

			// 获取数据成功，或者填充数据成功，复制数据
			memcpy(fNwfdData + nOff, fData, nDataLen*sizeof(float));
			nOff += nDataLen;

			// 释放本次数据申请空间
			if (fData)  free(fData);
			if (stDataField) free(stDataField);
		}

		// 存储数据到GRIB文件中
		QString strFilePath = HandleCommon::GetFolderPath(m_stProduct.stMergeFileSave.strSavePath, nYear, nMonth, nDay);
		QString strFileName = HandleCommon::GetFileName(m_stProduct.stMergeFileSave.strFileName, nYear, nMonth, nDay, nHour, endForecast, rangetime);
		
		nErr = HandleGrib::getClass().SaveData2File(fNwfdData, fileCount, m_stProduct.fLon1, m_stProduct.fLon2, m_stProduct.fLat1,
			m_stProduct.fLat2, m_stProduct.fDi, m_stProduct.fDj, m_stProduct.nNi, m_stProduct.nNj,
			nYear, nMonth, nDay, nHour, startForecast, m_stProduct.nTimeRange,
			m_stProduct.nCategory, m_stProduct.nElement, m_stProduct.nStatistical, m_stProduct.nStatus, m_stProduct.nCnt,
			m_stProduct.strName, m_stProduct.strLineVal, m_stProduct.fOffSet, m_stProduct.fMissingVal, strFilePath, strFileName, "grib", true);

		if (nErr == SUCCESS)
		{
			// 数据完整性处理标记,如果该标记为TRUE,则表示不完整数据已经生成过一次
			m_hasNotWholeBatch_today[strBatchName] = bIsNotWhole;

			// 保存文件成功，进行数据分发
			// 如果文件时间有更新，重新生成后要强制发送，即不管之前有没有发送过此文件
			CDDSManager::getClass().DataDistribution(m_stProduct.strKey, m_stProduct.stMergeFileSave.lstDDS, strFileName, strFilePath, bFileTimeUpdate);

			// 写入产品处理日志
			LOG_(LOGID_PRODUCT, LOG_F("[Key:%s][BatchName:%s]该时效批次文件处理成功(%s)."), LOG_STR(m_stProduct.strKey), LOG_STR(strBatchName), LOG_STR(strFileName));
		}
	}
	catch (...)
	{
		LOG_(LOGID_ERROR, LOG_F("[Key:%s]合并一个时次文件时发生异常."), LOG_STR(m_stProduct.strKey));
		return ERR_CATCH;
	}

	return nErr;
}

/************************************************************************/
/* 获取产品文件列表                                                     */
/************************************************************************/
bool TaskProductRaw::GetProductFileList(QString strSrcFolder, QString strFilterName, QFileInfoList& list)
{
	QDir dDir;

	// 检验产品目录是否存在
	if (!dDir.exists(strSrcFolder))
	{
		return false;
	}

	// 目录设置
	dDir.setPath(strSrcFolder);    // 设置目录名称
	dDir.setFilter(QDir::Files);   // 列出目录中的文件
	dDir.setSorting(QDir::Name);   // 按照文件名排序

	QStringList lstfileFilter;
	lstfileFilter << strFilterName;

	dDir.setNameFilters(lstfileFilter); // 设置文件筛选

	// 获取产品文件
	list = dDir.entryInfoList();

	return true;
}

/************************************************************************/
/* 已处理产品Hash标记维护处理                                           */
/************************************************************************/
void TaskProductRaw::HandleClearHash()
{
	// 按天进行标记判断处理，保留两天历史记录
	QString strNewClearHasFlg = QDateTime::currentDateTime().toString("yyyyMMdd");
	if (strNewClearHasFlg != m_strClearHasFlg)
	{
		// 日期已经切换，结果判断列表清空，防止无限增长
		// 清除昨天的数据
		m_hasCompleteProduct_yesterday.clear();
		m_hasCompleteBatch_yesterday.clear();

		// 今天的数据成为昨天的数据
		m_hasCompleteProduct_yesterday = m_hasCompleteProduct_today;
		m_hasCompleteBatch_yesterday = m_hasCompleteBatch_today;
		m_hasNotWholeBatch_yesterday = m_hasNotWholeBatch_today;

		// 清除今天的数据
		m_hasCompleteProduct_today.clear();
		m_hasCompleteBatch_today.clear();
		m_hasNotWholeBatch_today.clear();
		// 清除当日时间列表
		ClearBatchFile();

		// 重新赋值
		m_strClearHasFlg = strNewClearHasFlg;
	}
}

/************************************************************************/
/* 设置处理结果                                                         */
/************************************************************************/
void TaskProductRaw::SetProcessedResult(QString strFlg, bool bResult)
{
	m_hasCompleteProduct_today[strFlg] = bResult;
}

/************************************************************************/
/* 是否已经处理判断方法                                                 */
/************************************************************************/
bool TaskProductRaw::IsHasProcessed(QString strFlg)
{
	// 查看今天处理的数据
	if (m_hasCompleteProduct_today.contains(strFlg))
	{
		return 	m_hasCompleteProduct_today[strFlg];
	}

	// 查看昨天处理的数据
	if (m_hasCompleteProduct_yesterday.contains(strFlg))
	{
		return 	m_hasCompleteProduct_yesterday[strFlg];
	}

	return false;
}

/************************************************************************/
/* 判断某时效批次的文件是否已经处理过                                   */
/************************************************************************/
bool TaskProductRaw::IsHasProcessedBatch(QString strFlg)
{
	// 查看今天处理的数据
	if (m_hasCompleteBatch_today.contains(strFlg))
	{
		return 	m_hasCompleteBatch_today[strFlg];
	}

	// 查看昨天处理的数据
	if (m_hasCompleteBatch_yesterday.contains(strFlg))
	{
		return 	m_hasCompleteBatch_yesterday[strFlg];
	}

	return false;
}

/************************************************************************/
/* 是否有不完整数据批次已经处理过                                       */
/************************************************************************/
bool TaskProductRaw::IshasNotWholeBatchProcessed(QString strBatchName)
{
	// 查看今天处理的数据
	if (m_hasNotWholeBatch_today.contains(strBatchName))
	{
		return 	m_hasNotWholeBatch_today[strBatchName];
	}

	// 查看昨天处理的数据
	if (m_hasNotWholeBatch_yesterday.contains(strBatchName))
	{
		return 	m_hasNotWholeBatch_today[strBatchName];
	}

	return false;
}

/************************************************************************/
/* 判断是否到最后处理时间                                               */
/************************************************************************/
bool TaskProductRaw::IsLastTimeReached(int nReportTime)
{
	ST_FORECAST stForcast = m_stProduct.hasForecast.value(nReportTime);

	// 配置时不能配置：同为-1表示不进行检测时间处理
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

/************************************************************************/
/* 产品时间是否是有效时间                                               */
/************************************************************************/
bool TaskProductRaw::IsProductTimeValid(int nYear, int nMonth, int nDay, int nHour, int nValidTime)
{
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
/* 查找某编号的数据加工处理信息                                         */
/************************************************************************/
bool TaskProductRaw::FindDataProcInfo(int nID, ST_DATAPROC & stDataProc)
{
	if (!m_stProduct.hasDataProc.contains(nID))
	{
		return false;
	}

	stDataProc = m_stProduct.hasDataProc.value(nID);

	return true;
}

/************************************************************************/
/* 清空列表                                                             */
/************************************************************************/
void TaskProductRaw::ClearBatchFile()
{
	Hash_BatchTime::iterator iter;
	for (iter = m_hasBatchFileTime.begin(); iter != m_hasBatchFileTime.end(); iter++)
	{
		Hash_FileTime * hasFile = iter.value();
		if (hasFile)
		{
			hasFile->clear();
			delete hasFile;
			hasFile = NULL;
		}
	}

	m_hasBatchFileTime.clear();
}

/************************************************************************/
/* 设置文件创建时间                                                     */
/************************************************************************/
void TaskProductRaw::SetFileCreatedTime(QString strBatchName, QString strFile, QString strCreatedTime)
{
	Hash_FileTime * hasFile = NULL;
	if (m_hasBatchFileTime.contains(strBatchName))
	{
		hasFile = m_hasBatchFileTime.value(strBatchName);
	}
	else
	{
		hasFile = new Hash_FileTime();
		m_hasBatchFileTime.insert(strBatchName, hasFile);
	}

	// 添加到队列中
	if (hasFile)
	{
		hasFile->insert(strFile, strCreatedTime);
	}
}

/************************************************************************/
/* 检测文件有无更新                                                     */
/************************************************************************/
bool TaskProductRaw::IsFileTimeUpdate(QString strBatchName)
{
	// 无法判断文件是否有更新的情况下，认为没有更新
	if (!m_hasBatchFileTime.contains(strBatchName))
	{
		return false;
	}

	Hash_FileTime * hasFile = m_hasBatchFileTime.value(strBatchName);
	if (hasFile == NULL)
	{
		return false;
	}

	Hash_FileTime::Iterator iter;
	for (iter = hasFile->begin(); iter != hasFile->end(); iter++)
	{
		QString strFile = iter.key();
		QString strTime = iter.value();

		// 重新检查文件时间
		QString strFileTime = HandleCommon::GetFileCreatedTime(strFile);
		if (!strFileTime.isEmpty() && strFileTime != strTime)
		{
			// 与原先的时间不相同? 还是判断时间比之前的文件新？
			return true;
		}
	}

	return false;
}


