#include "log.h"
#include "DataManager.h"
#include "Common.h"
#include "HandleCommon.h"
#include "HandleNwfdLib.h"  // grib2文件
#include "CachedImpl.h"     // cached
#include "HandleGrib.h"
#include "HandleDB.h"
#include "HandleInterpolation.h"
#include "NetworkManager.h"

#include <QFile>
#include <QTextStream>
#include <QVector>
/************************************************************************/
/* 构造函数                                                             */
/************************************************************************/
DataManager::DataManager()
{
	LOG_(LOGID_DEBUG, LOG_F("Entry DataManager()"));
	
	m_pCachedImpl = NULL;
	m_stStationMag.bDisabled = true;  // 默认功能禁用

	LOG_(LOGID_DEBUG, LOG_F("Leave DataManager()"));
}

/************************************************************************/
/* 析构函数                                                             */
/************************************************************************/
DataManager::~DataManager()
{
	LOG_(LOGID_DEBUG, LOG_F("Entry ~DataManager()"));
	
	if (m_pCachedImpl)
	{
		delete m_pCachedImpl;
		m_pCachedImpl = NULL;
	}

	ClearStationData();

	LOG_(LOGID_DEBUG, LOG_F("Leave ~DataManager()"));
}

/************************************************************************/
/* 单例                                                                 */
/************************************************************************/
DataManager & DataManager::getClass()
{
	static DataManager objDataManager;
	return objDataManager;
}

/************************************************************************/
/* 初始化                                                               */
/************************************************************************/
int DataManager::Init(HASH_PRODUCT hasProduct, ST_CACHED stCached, ST_STATION_MAG stStationMag)
{
	m_hasProduct = hasProduct; // 产品列表
	m_stCached = stCached;     // 缓存配置
	m_stStationMag = stStationMag; // 站点-格点处理配置

	// 初始化缓存连接
	InitCached();

	// 初始化站点列表
	InitStationList();

	return 0;
}

/************************************************************************/
/* 获取站点列表                                                         */
/************************************************************************/
HASH_STATION_INFO &DataManager::GetStationList()
{
	return m_hasStaitonInfo;
}

/************************************************************************/
/* 获取站点列表                                                         */
/************************************************************************/
QString DataManager::GetForecastType()
{
	return m_stStationMag.strForecastType;
}

/************************************************************************/
/* 是否自动进行站点到格点的处理                                         */
/************************************************************************/
bool DataManager::IsStation2Grid()
{
	return m_stStationMag.bIsStation2Grid;
}

/************************************************************************/
/* 初始化缓存数据                                                       */
/************************************************************************/
bool DataManager::InitCached()
{
	// 建立缓存连接
	if (m_pCachedImpl == NULL)
	{
		m_pCachedImpl = new CachedImpl(m_stCached);
	}

	if (m_pCachedImpl == NULL)
	{
		// 创建失败
		LOG_(LOGID_ERROR, LOG_F("[DataManager]Cached连接创建失败"));

		return false;
	}

	// 测试连接状态
	int nErr = m_pCachedImpl->ConnectStatus();
	QString strConnectStatus = (nErr == SUCCESS) ? QString("连接") : QString("断开");
	LOG_(LOGID_INFO, LOG_F("[DataManager] Cached连接状态: %s"), LOG_STR(strConnectStatus));

	if (nErr != SUCCESS)
	{
		return false;
	}

	return true;
}

/************************************************************************/
/* 初始化站点列表                                                       */
/************************************************************************/
int DataManager::InitStationList()
{
	if (m_stStationMag.bDisabled)
	{
		// 功能禁用
		return 105;
	}

	// 打开文件
	FILE *fp;
	fp = fopen(m_stStationMag.strStationFile.toLocal8Bit().data(), "r");
	if (fp == NULL)
	{
		// 文件打开失败
		return -2;
	}

	char szBuf[512];
	string strBuf;
	string strData;
	string::size_type nOff = 0;  // 字符串游标位置
	string separator = " \t\r\n";

	m_hasStaitonInfo.clear();
	ST_STATION_INFO stStationInfo;

	// 按行读取文件内容
	while (!feof(fp))
	{
		fgets(szBuf, sizeof(szBuf), fp);
		strBuf = string(szBuf);
		nOff = 0;

		// 跳过空行
		if (strBuf.empty() || strBuf == "")
		{
			continue;
		}

		// 站号
		strData = getdata(strBuf, separator, nOff);
		if (strData == "")
		{
			strData = getdata(strBuf, separator, nOff);
		}
		memset(stStationInfo.stationNo, 0x00, sizeof(stStationInfo.stationNo));
		strcpy(stStationInfo.stationNo, strData.data());

		// 纬度
		strData = getdata(strBuf, separator, nOff);
		stStationInfo.fLat = atof(strData.data());

		// 经度
		strData = getdata(strBuf, separator, nOff);
		stStationInfo.fLon = atof(strData.data());

		// 海拔
		strData = getdata(strBuf, separator, nOff);
		stStationInfo.fHeight = atof(strData.data());

		//站点名称
		strData = getdata(strBuf, separator, nOff);
		memset(stStationInfo.stationName, 0x00, sizeof(stStationInfo.stationName));
		strcpy(stStationInfo.stationName, strData.data());

		// 添加到列表中
		m_hasStaitonInfo.insert(stStationInfo.stationNo, stStationInfo);
	}

	// 关闭文件
	fclose(fp);


	this->flushStationInfoToMemcached();


	return SUCCESS;
}

/************************************************************************/
/* 格点转站点，并将站点数据保存到缓存中                                 */
/************************************************************************/
//int DataManager::Grid2Station(QString strProductKey, float * fGridData, float fStartLon, float fStartLat, float fDi, float fDj, int nNi, int nNj, int nYear, int nMonth, int nDay, int nHour, int nForecastTime, bool bFirst)
//{
//	if (m_stStationMag.bDisabled)
//	{
//		// 功能禁用
//		return 105;
//	}
//
//	// 该转换，当前只考虑24小时降水，24小时最高温，24小时最低温
//	// 1、参数判断
//	if (fGridData == NULL)
//	{
//		return ERR_PARAM_NULL;
//	}
//
//	// 2、查找key相应的信息
//	if (!m_hasProduct.contains(strProductKey))
//	{
//		// 不再列表范围内
//		return 100;
//	}
//
//	ST_PRODUCT stProduct = m_hasProduct[strProductKey];
//	int nTimeRange = stProduct.nTimeRange;  // 时间间隔使用格点间隔
//
//	// 3、 数据类型判断
//	int nPos = -1;
//	if (stProduct.strType == "TMP") // 温度 ：泛指 3小时间隔温度
//	{
//		nPos = 0;
//	}
//	else if (stProduct.strType == "ER24" || stProduct.strType == "ER12" || stProduct.strType == "ER06" || stProduct.strType == "ER03") // 降水
//	{
//		nPos = 5;
//	}
//	else if (stProduct.strType == "TMX") // 最高温度
//	{
//		nPos = 10;
//	}
//	else if (stProduct.strType == "TMI") // 最低温度
//	{
//		nPos = 11;
//	}
//	else
//	{
//		// 非处理类型
//		return 101;
//	}
//
//	// 4、缓存连接判断
//	if (m_pCachedImpl == NULL)
//	{
//		return CACHED_INIT_FAIL;
//	}
//
//	int nErr = m_pCachedImpl->ConnectStatus();
//	if (nErr != SUCCESS)
//	{
//		// 重新建立连接
//		m_pCachedImpl->ReConnect();
//		// 获取连接状态
//		nErr = m_pCachedImpl->ConnectStatus();
//		if (nErr != SUCCESS)
//		{
//			return CACHED_CONNECT_ABNORMAL;
//		}
//	}
//
//	// 5、时间
//	QString strTime = HandleCommon::GetCachedProductTime(nYear, nMonth, nDay, nHour, nForecastTime, nTimeRange);
//
//	// 6、处理所有站点
//	bool bIsUpdate;
//	HASH_STATION_INFO::iterator iter;
//	for (iter = m_hasStaitonInfo.begin(); iter != m_hasStaitonInfo.end(); iter++)
//	{
//		ST_STATION_INFO stStation = iter.value();
//
//		// 计算该站点的值
//		float fValue = HandleInterpolation::getClass().biInterpolation(fGridData, fStartLon, fStartLat, fDi, fDj, nNi, nNj, stStation.fLon, stStation.fLat);
//
//		// 保存到缓存中
//		int nErr = m_pCachedImpl->UpdateStationData(m_stStationMag.strCityType.toLocal8Bit().data(), m_stStationMag.strForecastType.toLocal8Bit().data(), nTimeRange,
//			stStation.stationNo, strTime.toLocal8Bit().data(), fValue, nPos, bIsUpdate, bFirst);
//		if (nErr != SUCCESS)
//		{
//			// 保存站点数据错误
//			LOG_(LOGID_DEBUG, LOG_F("[DataManger]格点（%s）转站点（%s）保存失败（错误编码=%d）."), LOG_STR(strProductKey), LOG_STR(QString::fromLocal8Bit(stStation.stationNo)), nErr);
//		}
//		else
//		{
//			if (bIsUpdate && !bFirst)
//			{
//				// 站点值已发生变化，并更新成功，发布消息
//				nErr = NetworkManager::getClass().PublishStation(m_stStationMag.strCityType, m_stStationMag.strForecastType, stStation.stationNo, nYear, nMonth, nDay, nHour, 0, 0, nForecastTime, nTimeRange);
//				if (nErr != SUCCESS)
//				{
//					// 发布消息失败
//				}
//			}
//		}
//	}
//
//	return SUCCESS;
//}

/************************************************************************/
/* 格点转站点，并将站点数据保存到缓存中                                 */
/************************************************************************/
int DataManager::Grid2Station(QString strType, int nTimeRange, QString strProductKey, float * fGridData, float fStartLon, float fStartLat, float fDi, float fDj, int nNi, int nNj, int nYear, int nMonth, int nDay, int nHour, int nForecastTime, bool bFirst)
{
	// 1、是否禁用判断
	if (m_stStationMag.bDisabled)
	{
		// 功能禁用
		return FUNCTION_DISABLED;
	}

	// 2、参数判断
	if (fGridData == NULL)
	{
		return ERR_PARAM_NULL;
	}

	// 保存站点类型与格点产品Key的匹配
	if (bFirst)
	{
		// 第一次处理，产品Product执行raw时作为初始化格点-站点的依据
		SetStationProduct(strType, nTimeRange, strProductKey);
	}

	// 3、处理所有站点
	HASH_STATION_INFO::iterator iter;
	for (iter = m_hasStaitonInfo.begin(); iter != m_hasStaitonInfo.end(); iter++)
	{
		ST_STATION_INFO stStation = iter.value();

		// 计算该站点的值
		float fValue = HandleInterpolation::getClass().biInterpolation(fGridData, fStartLon, fStartLat, fDi, fDj, nNi, nNj, stStation.fLon, stStation.fLat);
		if (!IS_EQUAL(fValue, 999.9) && (strType == "TMP" || strType == "ETM" || strType == "ETN"))
		{
			fValue -= 273.15f;
		}

		// 保存设置该值
		int nErr = SetStationData(strType, nTimeRange, nYear, nMonth, nDay, nHour, nForecastTime, QString::fromLocal8Bit(stStation.stationNo), fValue);
		if (nErr != 0)
		{
			// 处理失败

		}
	}

	return SUCCESS;
}

/************************************************************************/
/* 站点修改格点，并将站点数据保存到缓存中                               */
/************************************************************************/
int DataManager::Station2Grid(QString strStationNo, int nTimeRange, float * fStationData, int nStationDataLen, int nYear, int nMonth, int nDay, int nHour, int nForecastTime, QString &strCityType, QString &strForecastType, int &nStationRange, bool & bIsUpdate)
{
	if (m_stStationMag.bDisabled)
	{
		// 功能禁用
		return FUNCTION_DISABLED;
	}

	// 该转换，当前只考虑24小时降水，24小时最高温，24小时最低温
	// 返回值初始化
	strCityType = m_stStationMag.strCityType;
	strForecastType = m_stStationMag.strForecastType;
	nStationRange = nTimeRange;
	bIsUpdate = false;

	// 检查该站号是否在列表中
	if (!m_hasStaitonInfo.contains(strStationNo))
	{
		// 不再处理列表中
		return 100;
	}
	ST_STATION_INFO stStation = m_hasStaitonInfo[strStationNo];

	// 缓存判断
	if (m_pCachedImpl == NULL)
	{
		return CACHED_INIT_FAIL;
	}
	
	//  时间
	QString strTime = HandleCommon::GetCachedProductTime(nYear, nMonth, nDay, nHour, nForecastTime, nTimeRange);

	// 获取缓存中的原数据
	float * fData = NULL;
	int nDataLen = 0;
	int nErr = m_pCachedImpl->GetStationData(m_stStationMag.strCityType.toLocal8Bit().data(), m_stStationMag.strForecastType.toLocal8Bit().data(),
		nTimeRange, strStationNo.toLocal8Bit().data(), strTime.toLocal8Bit().data(), fData, nDataLen);
	if (nErr != SUCCESS)
	{
		// 获取原数据失败，将当前数据直接保存
		nErr = m_pCachedImpl->SaveStationData(m_stStationMag.strCityType.toLocal8Bit().data(), m_stStationMag.strForecastType.toLocal8Bit().data(),
			nTimeRange, strStationNo.toLocal8Bit().data(), strTime.toLocal8Bit().data(), fStationData, nStationDataLen);
		if (nErr == SUCCESS)
		{
			bIsUpdate = true;
		}

		// 暂不进行插值格点
		return nErr;
	}

	// 比较数据，看变化值
	if (nStationDataLen != nDataLen )
	{
		// 长度错误
		return 101;
	}

	QString strType;
	ST_PRODUCT stProduct;
	bool bSaveFlg = false;
	for (int i = 0; i < nDataLen; i++)
	{
		if (IS_EQUAL(fData[i], fStationData[i]))
		{
			// 值相同，跳过
			continue;
		}

		// 位置判断
		switch (i)
		{
		//case 0: strType = "TMP";  break; // 温度
		case 5: strType = "ER24";  break; // 降水
		case 10: strType = "TMX";  break; // 最高温度
		case 11: strType = "TMI";  break; // 最低温度
		default: strType = "";
		}
		
		if (!bSaveFlg)
		{

			// 保存数据
			nErr = m_pCachedImpl->SaveStationData(m_stStationMag.strCityType.toLocal8Bit().data(), m_stStationMag.strForecastType.toLocal8Bit().data(),
				nTimeRange, strStationNo.toLocal8Bit().data(), strTime.toLocal8Bit().data(), fStationData, nStationDataLen);
			if (nErr == SUCCESS)
			{
				bIsUpdate = true;
				bSaveFlg = true; // 保存成功
			}
		}

		// 值有变化，但不在站点转格点的处理范围内，跳过
		if (strType == "")
		{			
			continue;
		}

		// 没有找到相应的格点数据Key，不做处理
		if (!GetProduct(strType,nTimeRange, stProduct))
		{
			continue;
		}

		// 获取格点数据
		float * fGridData = NULL;
		int nGridDataLen = 0;
		nErr = m_pCachedImpl->GetMergeData(stProduct.strKey.toLocal8Bit().data(), strTime.toLocal8Bit().data(), fGridData, nGridDataLen);
		if (nErr != SUCCESS || fGridData == NULL)
		{
			// 获取格点数据失败，跳过
			// todo
			continue;
		}
		
		// 查找影响的格点经纬度范围
		float fLon1, fLon2, fLat1, fLat2;
		// 影响半径，经纬方向值不相等时取最大格距 
		float fRadius = m_stStationMag.nRadius * ((stProduct.fDi >= stProduct.fDj) ? stProduct.fDi : stProduct.fDj); 
		// 获取影响格点范围
		bool bRet = GetGridRange(stStation.fLon, stStation.fLat, fRadius, stProduct.fLon1, stProduct.fLat1, stProduct.fDi, stProduct.fDj, stProduct.nNi, stProduct.nNj, fLon1, fLon2, fLat1, fLat2);
		if (!bRet)
		{
			// 释放格点空间
			if (fGridData)
			{
				free(fGridData);
				fGridData = NULL;
			}
			continue;
		}

		// 找到该格点所影响的站点列表
		LIST_STATION_VALUE lstStation;
		GetStationList(fLon1, fLon2, fLat1, fLat2, fRadius, lstStation);
		if (lstStation.size() == 0)
		{
			// 释放格点空间
			if (fGridData)
			{
				free(fGridData);
				fGridData = NULL;
			}
			continue;
		}

		// 设置站点插值
		LIST_STATION_VALUE::iterator iter;
		for (iter = lstStation.begin(); iter != lstStation.end(); iter++)
		{
			ST_STATION_VALUE * stStationValue = *iter;
			if (stStationValue->strStationNo == strStationNo)
			{
				stStationValue->fValue = fStationData[i] - fData[i];
			}
		}

		// 调整格点数据
		int nLonCount = (int)((fLon1 - stProduct.fLon1) / stProduct.fDi + 0.1) + 1;  // 经度格点数
		int nLatCount = (int)((fLat1 - stProduct.fLat1) / stProduct.fDj + 0.1) + 1;  // 纬度格点数
		float * fGridDataTmp = fGridData + (nLatCount - 1) * stProduct.nNi + (nLonCount - 1);

		int nNi = (int)((fLon2 - fLon1) / stProduct.fDi + 0.1) + 1;  // 经度格点数
		int nNj = (int)((fLat2 - fLat1) / stProduct.fDj + 0.1) + 1;  // 纬度格点数

		bRet = HandleInterpolation::getClass().cressman(lstStation, fRadius, fGridDataTmp, fLon1, fLat1, stProduct.fDi, stProduct.fDj, nNi, nNj);
		if (bRet)
		{
			// 调成成功，保存格点数据到缓存中
			nErr = m_pCachedImpl->SaveMergeData(stProduct.strKey.toLocal8Bit().data(), strTime.toLocal8Bit().data(), fGridData, nGridDataLen);
			if (nErr == SUCCESS)
			{
				// 格点值已发生变化，并更新成功，发布消息
				nErr = NetworkManager::getClass().PublishGrid(stProduct.strKey, nYear, nMonth, nDay, nHour, 0, 0, nForecastTime, stProduct.nTimeRange);
				if (nErr != SUCCESS)
				{
					// 发布消息失败
				}
			}
		}

		// 删除站点
		for (iter = lstStation.begin(); iter != lstStation.end(); iter++)
		{
			ST_STATION_VALUE * stStationValue = *iter;
			if (stStationValue)
			{
				delete stStationValue;
				stStationValue = NULL;
			}
		}

		//// 调整格点数据
		//bool bResult = HandleInterpolation::getClass().AdjustmentGrid(fStationData[i], stStation.fLon, stStation.fLat, m_stStationMag.nRadius, fGridData, stProduct.fLon1, stProduct.fLat1, stProduct.fDi, stProduct.fDj, stProduct.nNi, stProduct.nNj);

		// 释放格点空间
		if (fGridData)
		{
			free(fGridData);
			fGridData = NULL;
		}
	}

	// 释放站点数据内存
	if (fData)
	{
		free(fData);
		fData = NULL;
	}

	return SUCCESS;
}

/************************************************************************/
/* 多个站点，订正其格点数据                                             */
/************************************************************************/
int DataManager::MultiStation2Grid(QString strDataType, LIST_STATION_VALUE  lstStationDiffVal, float fStartLon, float fStartLat, float fEndLon, float fEndLat, int nYear, int nMonth, int nDay, int nHour, int nTimeRange, int nForecastTime)
{
	// 获取相应的格点数据
	ST_PRODUCT stProduct;
	if (!GetProduct(strDataType, nTimeRange, stProduct))
	{
		return -1;
	}
	
	//  时间
	QString strTime = HandleCommon::GetCachedProductTime(nYear, nMonth, nDay, nHour, nForecastTime, nTimeRange);

	float * fGridData = NULL;
	int nGridDataLen = 0;
	int nErr = m_pCachedImpl->GetMergeData(stProduct.strKey.toLocal8Bit().data(), strTime.toLocal8Bit().data(), fGridData, nGridDataLen);
	if (nErr != SUCCESS || fGridData == NULL)
	{
		// 获取格点数据失败，跳过
		return -2;
	}

	// 影响半径
	float fRadius = m_stStationMag.nRadius * ((stProduct.fDi >= stProduct.fDj) ? stProduct.fDi : stProduct.fDj);

	// 站点到格点订正插值处理
	bool bRet = HandleInterpolation::getClass().cressman(lstStationDiffVal, fRadius, fGridData, stProduct.fLon1, stProduct.fLat1, stProduct.fDi, stProduct.fDj, stProduct.nNi, stProduct.nNj);
	if (bRet)
	{
		// 调成成功，保存格点数据到缓存中
		nErr = m_pCachedImpl->SaveMergeData(stProduct.strKey.toLocal8Bit().data(), strTime.toLocal8Bit().data(), fGridData, nGridDataLen);
		if (nErr == SUCCESS)
		{
			// 格点值已发生变化，并更新成功，发布消息
			nErr = NetworkManager::getClass().PublishGrid(stProduct.strKey, nYear, nMonth, nDay, nHour, 0, 0, nForecastTime, stProduct.nTimeRange);
			if (nErr != SUCCESS)
			{
				// 发布消息失败
			}
		}
	}

	// 释放空间
	if (fGridData)
	{
		free(fGridData);
		fGridData = NULL;
	}

	if (!bRet || nErr != SUCCESS)
	{
		// 处理失败
		return -3;
	}

	return 0;
}

/************************************************************************/
/* 多个站点，订正其格点数据                                             */
/************************************************************************/
int DataManager::SaveMultiStation(QString strType, int nYear, int nMonth, int nDay, int nHour, int nTimeRange, int nMaxForecastTime)
{
	// 1、功能是否功能判定
	if (m_stStationMag.bDisabled)
	{
		// 功能禁用
		return FUNCTION_DISABLED;
	}

	// 2、缓存连接判断
	if (m_pCachedImpl == NULL)
	{
		return CACHED_INIT_FAIL;
	}
	
	int nErr = m_pCachedImpl->ConnectStatus();
	if (nErr != SUCCESS)
	{
		// 重新建立连接
		m_pCachedImpl->ReConnect();
		// 获取连接状态
		nErr = m_pCachedImpl->ConnectStatus();
		if (nErr != SUCCESS)
		{
			return CACHED_CONNECT_ABNORMAL;
		}
	}

	// 格式转换
	QString strKey = QString("%1_%2").arg(strType).arg(nTimeRange);

	HASH_STATION_NO * hasStation = NULL;
	if (!m_hasStationData.contains(strKey))
	{
		// 没有可用数据
		return NO_AVAILABLE_DATA;
	}
	hasStation = m_hasStationData.value(strKey);
	
	QString strCachedValue = "";

	HASH_STATION_NO::iterator iter;
	for (iter = hasStation->begin(); iter != hasStation->end(); iter ++)
	{
		QString strStationNo = iter.key();
		HASH_STATION_TIME * hasStaData = iter.value();

		//  首个
		if (iter == hasStation->begin())
		{
			strCachedValue = QString("[%1").arg(strStationNo);
		}
		else
		{
			strCachedValue = QString("%1;%2").arg(strCachedValue).arg(strStationNo);
		}

		for (int n = nTimeRange; n <= nMaxForecastTime; n += nTimeRange)
		{
			QString strTime = QString("").sprintf("%04d%02d%02d%02d.%03d", nYear, nMonth, nDay, nHour, n);
			float fData = 999.9f;

			if (hasStaData->contains(strTime))
			{
				fData = hasStaData->value(strTime);
			}

			if (n == nTimeRange)
			{
				strCachedValue = QString("%1(%2").arg(strCachedValue).arg(fData);
			}
			else
			{
				strCachedValue = QString("%1,%2").arg(strCachedValue).arg(fData);
			}
		}
		strCachedValue = QString("%1)").arg(strCachedValue);
	}

	if (strCachedValue.isEmpty())
	{
		// 没有合适数据
		return NO_AVAILABLE_DATA;
	}

	// 添加结束字符
	strCachedValue = QString("%1]").arg(strCachedValue);
	
	// 保存到缓存中
	QString strStationTime = HandleCommon::GetCachedProductTime(nYear, nMonth, nDay, nHour, 0, nTimeRange);
	nErr = m_pCachedImpl->SaveMultiStationData(strType.toLocal8Bit().data(), nTimeRange, strStationTime.toLocal8Bit().data(), strCachedValue.toLocal8Bit().data(), strCachedValue.length());
	if (nErr == SUCCESS)
	{
		// 发布数据
		if (m_stStationMag.bIsPublish)
		{
			int nRet = NetworkManager::getClass().PublishMultiStation(strType, nYear, nMonth, nDay, nHour, 0, 0, nTimeRange);
			if (nRet == 0)
			{
				// 发布成功
			}
			else
			{
				// 发布失败
			}
		}
	}

	return nErr;
}

/************************************************************************/
/* 找到站点所影响的格点经纬度范围                                       */
/************************************************************************/
bool DataManager::GetGridRange(float fStationLon, float fStationLat, float fRadius, float fStartLon, float fStartLat, float fDi, float fDj, int nNi, int nNj, float &fLon1, float &fLon2, float &fLat1, float &fLat2)
{
	// 初始化经纬度范围
	fLon1 = fStationLon - fRadius;
	fLon2 = fStationLon + fRadius;
	fLat1 = fStationLat - fRadius;
	fLat2 = fStationLat + fRadius;

	// 计算格点终止经纬度
	float fEndLon = fStartLon + fDi * (nNi - 1);
	float fEndLat = fStartLat + fDj * (nNj - 1);

	// 经度判断，如果矩形框与格点范围没有交集，则不处理
	if (fDi > 0)
	{
		if (fLon2 < fStartLon || fLon1 > fEndLon)
		{
			return false;
		}
	}
	else
	{
		if (fLon1 > fStartLon || fLon2 < fEndLon)
		{
			return false;
		}
	}
	// 纬度判断
	if (fDj > 0)
	{
		if (fLat2 < fStartLat || fLat1 > fEndLat)
		{
			return false;
		}
	}
	else
	{
		if (fLat1 > fStartLat || fLat2 < fEndLat)
		{
			return false;
		}
	}

	// 调整经纬度，以保证落在格点坐标内
	int  nCount;
	float fPostion;
	// 起始经度
	if (fLon1 < fStartLon)
	{
		// 起始经度太小
		fLon1 = fStartLon;
	}
	nCount = (int)((fLon1 - fStartLon) / fDi + 0.1);
	fPostion = fStartLon + fDi * nCount;
	if (IS_EQUAL(fLon1, fPostion))
	{
		fLon1 = fPostion;
	}
	else
	{
		fLon1 = fPostion + fDi;
	}

	// 终止经度
	if (fLon2> fEndLon)
	{
		// 终止经度太大
		fLon2 = fEndLon;
	}
	nCount = (int)((fLon2 - fStartLon) / fDi + 0.1);
	fPostion = fStartLon + fDi * nCount;
	fLon2 = fPostion;

	// 起始纬度
	if (fLat1 < fStartLat)
	{
		// 起始纬度太小
		fLat1 = fStartLat;
	}
	nCount = (int)((fLat1 - fStartLat) / fDj + 0.1);
	fPostion = fStartLat + fDj * nCount;
	if (IS_EQUAL(fLat1, fPostion))
	{
		fLat1 = fPostion;
	}
	else
	{
		fLat1 = fPostion + fDj;
	}

	// 终止纬度
	if (fLon2 > fEndLat)
	{
		// 终止纬度太大
		fLon2 = fEndLat;
	}
	nCount = (int)((fLon2 - fStartLat) / fDj + 0.1);
	fPostion = fStartLat + fDj * nCount;
	fLon2 = fPostion;

	return true;
}

/************************************************************************/
/* 找到格点所影响的站点列表                                            */
/************************************************************************/
bool DataManager::GetStationList(float fLon1, float fLon2, float fLat1, float fLat2, float fRadius, LIST_STATION_VALUE &lstStation)
{
	// 查找方法
	// 将经纬度范围按照影响半径扩大一圈，在该范围内找到所有的站点
	float fStartLon = fLon1 - fRadius;
	float fEndLon = fLon2 + fRadius;
	float fStartLat = fLat1 - fRadius;
	float fEndLat = fLat2 + fRadius;

	HASH_STATION_INFO::iterator iter;
	for (iter = m_hasStaitonInfo.begin(); iter != m_hasStaitonInfo.end(); iter++)
	{
		ST_STATION_INFO stStation = iter.value();
		if (stStation.fLon < fStartLon || stStation.fLon > fEndLon || stStation.fLat < fStartLat || stStation.fLat > fEndLat)
		{
			continue;
		}
		ST_STATION_VALUE * stStationValue = new ST_STATION_VALUE;
		if (stStationValue == NULL)
		{
			continue;
		}
		stStationValue->strStationNo = QString::fromLocal8Bit(stStation.stationNo);
		stStationValue->lon = stStation.fLon;
		stStationValue->lat = stStation.fLat;
		stStationValue->height = stStation.fHeight;
		stStationValue->fValue = 0;

		// 添加到列表中
		lstStation.push_back(stStationValue);
	}

	return true;
}

/************************************************************************/
/* 设置产品匹配                                                         */
/************************************************************************/
void DataManager::SetStationProduct(QString strType, int nTimeRange, QString strProductKey)
{
	QString strStationKey = QString("%1_%2").arg(strType).arg(nTimeRange);

	if (!m_hasStationProduct.contains(strStationKey))
	{
		m_hasStationProduct.insert(strStationKey, strProductKey);
	}
}

/************************************************************************/
/* 找到相应的格点                                                       */
/************************************************************************/
bool DataManager::GetProduct(QString strType, int nTimeRange, ST_PRODUCT &stProduct)
{
	/*
	HASH_PRODUCT::iterator iter;
	for (iter = m_hasProduct.begin(); iter != m_hasProduct.end(); iter++)
	{
		ST_PRODUCT stPro = iter.value();
		if (stPro.strType == strType && stPro.nTimeRange == nTimeRange && stPro.strRole == "raw" && !stPro.stStationProc.bDisabled)
		{
			stProduct = stPro;
			return true;
		}
	}

	return false;
	*/

	QString strStationKey = QString("%1_%2").arg(strType).arg(nTimeRange);

	if (!m_hasStationProduct.contains(strStationKey))
	{
		return false;
	}

	QString strProductKey = m_hasStationProduct.value(strStationKey);
	if (!m_hasProduct.contains(strProductKey))
	{
		return false;
	}
	stProduct = m_hasProduct.value(strProductKey);

	return true;
}

/************************************************************************/
/* 设置站点值                                                           */
/************************************************************************/
int DataManager::SetStationData(QString strType, int nTimeRange,int nYear, int nMonth, int nDay, int nHour, int nForecastTime, QString strStationNo, float fData)
{
	QMutexLocker locker(&m_mutexStationData);

	QString strKey = QString("%1_%2").arg(strType).arg(nTimeRange);
	// 查找类型
	HASH_STATION_NO * hasStation = NULL;
	if (!m_hasStationData.contains(strKey))
	{
		hasStation = new HASH_STATION_NO;
		// 处理失败
		if (hasStation == NULL)
		{
			return -1;
		}

		m_hasStationData.insert(strKey, hasStation);
	}
	else
	{
		hasStation = m_hasStationData.value(strKey);
	}

	// 查找站号
	HASH_STATION_TIME * hasStaData = NULL;
	if (!hasStation->contains(strStationNo))
	{
		hasStaData = new HASH_STATION_TIME;
		// 处理失败
		if (hasStaData == NULL)
		{
			return -2;
		}

		hasStation->insert(strStationNo, hasStaData);
	}
	else
	{
		hasStaData = hasStation->value(strStationNo);
	}

	// 时间
	QString strTime = QString("").sprintf("%04d%02d%02d%02d.%03d", nYear, nMonth, nDay, nHour, nForecastTime);
	// 赋站点值
	hasStaData->insert(strTime, fData);

	return 0;
}

/************************************************************************/
/* 获取站点值                                                           */
/************************************************************************/
int DataManager::GetStationData(QString strType, int nTimeRange, int nYear, int nMonth, int nDay, int nHour, int nForecastTime, QString strStationNo, float &fData)
{
	QString strKey = QString("%1_%2").arg(strType).arg(nTimeRange);

	// 查找类型
	if (!m_hasStationData.contains(strKey))
	{
		return NO_AVAILABLE_DATA;
	}
	HASH_STATION_NO * hasStation = m_hasStationData.value(strKey);
	
	// 查找站号
	if (!hasStation->contains(strStationNo))
	{
		return NO_AVAILABLE_DATA;
	}
	HASH_STATION_TIME * hasStaData = hasStation->value(strStationNo);

	// 时间
	QString strTime = QString("").sprintf("%04d%02d%02d%02d.%03d", nYear, nMonth, nDay, nHour, nForecastTime);

	// 赋站点值
	fData = hasStaData->value(strTime);

	return 0;
}

/************************************************************************/
/* 清空所有的插值后的站点数据                                           */
/************************************************************************/
void DataManager::ClearStationData()
{
	QMutexLocker locker(&m_mutexStationData);

	HASH_STATION_DATA::iterator iter;
	for (iter = m_hasStationData.begin(); iter != m_hasStationData.end(); iter++)
	{
		HASH_STATION_NO * hasStation = iter.value();
		if (hasStation == NULL)
		{
			continue;
		}

		HASH_STATION_NO::iterator iterSta;
		for (iterSta = hasStation->begin(); iterSta != hasStation->end(); iterSta++)
		{
			HASH_STATION_TIME * hasStaData = iterSta.value();
			if (hasStaData)
			{
				hasStaData->clear();
				delete hasStaData;
				hasStaData = NULL;
			}
		}

		hasStation->clear();
		delete hasStation;
		hasStation = NULL;
	}

	m_hasStationData.clear();
}

int DataManager::flushStationInfoToMemcached()
{

	HASH_STATION_INFO::iterator iter;

	QString stationCfgBuf;

	
	//以特定格式刷新至缓存
	stationCfgBuf.append("[");

	for (iter = m_hasStaitonInfo.begin(); iter != m_hasStaitonInfo.end(); ++iter)
	{
		stationCfgBuf.append(iter->stationNo);
		stationCfgBuf.append("(");
		stationCfgBuf.append(iter->stationName);
		stationCfgBuf.append(",");
		stationCfgBuf.append(QString::number(iter->fLat));
		stationCfgBuf.append(",");
		stationCfgBuf.append(QString::number(iter->fLon));
		stationCfgBuf.append(",");
		stationCfgBuf.append(QString::number(iter->fHeight));
		stationCfgBuf.append(")");

		//最后一个元素不加分号
		if (iter != m_hasStaitonInfo.end() - 1)
		{
			stationCfgBuf.append(";");
		}
		
		
	}

	stationCfgBuf.append("]");

	
	if (m_pCachedImpl == NULL)
	{
		return CACHED_INIT_FAIL;
	}

	int nErr = m_pCachedImpl->ConnectStatus();
	if (nErr != SUCCESS)
	{
		// 重新建立连接
		m_pCachedImpl->ReConnect();
		// 获取连接状态
		nErr = m_pCachedImpl->ConnectStatus();
		if (nErr != SUCCESS)
		{
			return CACHED_CONNECT_ABNORMAL;
		}
	}
	
	
	int nerr = m_pCachedImpl->SaveStationCfgData(stationCfgBuf.toLocal8Bit(), stationCfgBuf.toLocal8Bit().size());

	if (nerr != SUCCESS)
	{
		return nerr;
	}
	
	return SUCCESS;

}


int DataManager::flushStationInfoToLocal()
{
	//保存致本地dat文件

	QVector<QString> stationCfgLines;
	HASH_STATION_INFO::iterator iter;
	
	for (iter = m_hasStaitonInfo.begin(); iter != m_hasStaitonInfo.end(); ++iter)
	{
		QString line;
		line.append(iter->stationNo);
		line.append("\t");
		line.append(QString::number(iter->fLat));
		line.append("\t");
		line.append(QString::number(iter->fLon));
		line.append("\t");
		line.append(QString::number(iter->fHeight));
		line.append("\t");
		line.append(iter->stationName);

		stationCfgLines.push_back(line);

	}

    QFile file(m_stStationMag.strStationFile);
	if (!file.open(QFile::WriteOnly | QFile::Text))
	{

		return -2;
	}

	QTextStream out(&file);

	for (int i = 0; i < stationCfgLines.size(); ++i)
	{
		out << stationCfgLines.at(i) << endl;
	}

	out.flush();

	file.close();

	return SUCCESS;
}
