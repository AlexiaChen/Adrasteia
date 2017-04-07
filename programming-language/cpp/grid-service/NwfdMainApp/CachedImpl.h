/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: CachedImpl.h
* 作  者: Jeff		版本：1.0		日  期：2015/04/
* 描  述：缓存业务处理类
* 其  他：
* 功能列表:
*   1.
* 修改日志：
*   No. 日期		作者		修改内容
*   1  2015/05/04  zhangl    完善缓存的业务处理
*   2  2015/05/08  zhangl    增加ProviderBase类，改变调用方式
*   3  2015/05/15  zhangl    调整处理方法，以适应Redis和Memcached
*   4  2015/05/19  zhangl    添加memcached对Nwfd数据存储的管理（因slabs的1M限制）
*   5  2015/08/29  zhangl    修改缓存处理代码基于STL，以达到客户端代码统一，方便后期维护
*************************************************************************/
#ifndef CACHEDIMPL_H
#define CACHEDIMPL_H

//#define CLIENT_USE  // 客户端使用此条件

/************************************************************************/
/* 头文件引用                                                           */
/************************************************************************/
#include "CachedDef.h"
#include "LibraryDef.h"
#include "NwfdErrorCode.h"
#include "ProductDef.h"
#include "StationDef.h"
#include "ProviderBase.h"
#include "Common.h"
#ifndef CLIENT_USE
#include <QString>  // 主要用于加载dll名称宽字符处理
#endif
#include <string>
using namespace std;

/************************************************************************/
/* 缓存处理动态库调用接口声明                                           */
/************************************************************************/
typedef ProviderBase * (*lpGetProvider)(void);

/************************************************************************/
/* 类 名： CachedImpl                                                   */
/* 父 类： -                                                            */
/* 说 明： 缓存业务类                                                   */
/* 描 述： 该类缓存存储和提取的业务层                                   */
/************************************************************************/
class CachedImpl
{
public:
	CachedImpl(ST_CACHED stCached);
	~CachedImpl();
	int GetProvider();

private:
	int LoadImplLibrary(string strImplFileName);

public:
	/* 1. 基本操作 */
	int ConnectStatus();
	int ReConnect();
	int DisConnect();
	int FlushDB();

	/* 缓存标记 */
	bool SetChachedFlg(const char * strFlg);
	char * GetCachedFlg();

	/* 2. 数据产品缓存 Product */
	// 2-1 保存产品信息&产品数据
	int SaveProductKey(const char * strProductKey);
	int SaveProductInfo(ST_PRODUCT stProduct);
	int SaveProductTime(const char * strProductKey, const char * strTimeValue);
	int SaveProductData(const char * strProductKey, const char * strTimeValue, float * fData, int nDataSize, int nExpirationTime = 0);

	// 2-2 获取数据
	int GetAllProductKey(StringList &lstProductKey, bool bSortFlg = false);
	int GetProductInfo(const char * strProductKey, ST_PRODUCT &stProduct);
	int GetProductTime(const char * strProductKey, StringList &lstProductTime, bool bSortFlg = false);
	int GetProductData(const char * strProductKey, const char *  strProductTime, float* &fData, int &nDataLen);

	//  2-3 删除数据
	int DeleteProduct(const char * strProductKey);
	int DeleteProductData(const char * strProductKey, const char * strProductTime);

	/* 3. 数据合并缓存 Merge */
	// 3-1 保存数据
	int SaveMergeKey(const char * strMergeKey);
	int SaveMergeTime(const char * strMergeKey, const char * strTimeValue);
	int SaveMergeData(const char * strMergeKey, const char * strTimeValue, float * fData, int nDataSize, int nExpirationTime = 0);

	// 3-2 获取数据
	int GetAllMergeKey(StringList &lstMergeKey, bool bSortFlg = false);
	int GetMergeTime(const char * strMergeKey, StringList &lstMergeTime, bool bSortFlg = false);
	int GetMergeData(const char * strMergeKey, const char * strMergeTime, float* &fData, int &nDataLen);

	// 3-3 删除数据
	int DeleteMerge(const char * strProductKey);
	int DeleteMergeData(const char * strProductKey, const char * strProductTime);

	/* 4. 客户端缓存 Client */
	// 4-1 保存数据
	int SaveClientKey(int nClientID, const char * strProductKey);
	int SaveClientTime(int nClientID, const char * strProductKey, const char * strTimeValue);
	int SaveClientData(int nClientID, const char * strProductKey, const char * strTimeValue, float * fData, int nDataSize, float fLon1, float fLon2, float fLat1, float fLat2, int nExpirationTime = 0);

	// 4-2 获取数据
	int GetAllClientKey(int nClientID, StringList &lstProductKey, bool bSortFlg = false);
	int GetClientTime(int nClientID, const char * strProductKey, StringList &lstClientTime, bool bSortFlg = false);
	int GetClientData(int nClientID, const char * strProductKey, const char * strClientTime, float* &fData, int &nDataLen, float &fLon1, float &fLon2, float &fLat1, float &fLat2);

	// 4-3 删除数据
	int DeleteClient(int nClientID, char * strProductKey);
	int DeleteClientData(int nClientID, char * strProductKey, char * strClientTime);

	/* 5. 站点缓存 Station */
	// 5-1 保存站点信息&站点数据
	int SaveStation(const char * strCityType, const char * strDataType, int nTimeRange, const char * strStationNo);
	int SaveStationInfo(const char * strCityType, const char * strDataType, int nTimeRange, const char * strStationNo, float fLon, float fLat, float fHeight, int nCols);
	int SaveStationTime(const char * strCityType, const char * strDataType, int nTimeRange, const char * strStationNo, const char * strTimeValue);
	int SaveStationData(const char * strCityType, const char * strDataType, int nTimeRange, const char * strStationNo, const char * strTimeValue, float * fData, int nDataSize, int nExpirationTime = 0);
	int UpdateStationData(const char * strCityType, const char * strDataType, int nTimeRange, const char * strStationNo, const char * strTimeValue, float fValue, int nPos, bool &bIsUpdate, bool bSaveStationNo = false, int nExpirationTime = 0);

	// 5-2 获取数据
	int GetAllStationNo(const char * strCityType, const char * strDataType, int nTimeRange, StringList &lstStationNo, bool bSortFlg = false);
	int GeStationInfo(const char * strCityType, const char * strDataType, int nTimeRange, const char * strStationNo, float &fLon, float &fLat, float &fHeight);
	int GetStationTime(const char * strCityType, const char * strDataType, int nTimeRange, const char * strStationNo, StringList &lstStationTime, bool bSortFlg = false);
	int GetStationData(const char * strCityType, const char * strDataType, int nTimeRange, const char * strStationNo, const char * strStationTime, float* &fData, int &nDataLen);

	// 5-3 删除数据
	int DeleteStation(const char * strCityType, const char * strDataType, int nTimeRange, const char * strStationNo);
	int DeleteStationData(const char * strCityType, const char * strDataType, int nTimeRange, const char * strStationNo, const char * strStationTime);

	/* 6. 站点文件缓存 */
	// 6-1 保存站点信息&站点数据
	int SaveStationFile(const char * strCityType, const char * strDataType, int nTimeRange);
	int SaveStationFileTime(const char * strCityType, const char * strDataType, int nTimeRange, const char * strTimeValue);
	int SaveStationFileData(const char * strCityType, const char * strDataType, int nTimeRange, const char * strTimeValue, const char * pFileData, int nDataSize, int nExpirationTime = 0);

	// 6-2 获取数据
	int GetAllStationFileType(StringList &lstStationFileType, bool bSortFlg = false);
	int GetStationFileTime(const char * strCityType, const char * strDataType, int nTimeRange, StringList &lstStationTime, bool bSortFlg = false);
	int GetStationFileData(const char * strCityType, const char * strDataType, int nTimeRange, const char * strStationTime, char* &pFileData, int &nDataLen);

	// 6-3 删除数据
	int DeleteStationFile(const char * strCityType, const char * strDataType, int nTimeRange);
	int DeleteStationFileData(const char * strCityType, const char * strDataType, int nTimeRange, const char * strStationTime);

	/* 7. 某类型多站点多时次合并数据 */
	// 7-1 保存
	int SaveMultiStation(const char * strDataType, int nTimeRange);
	int SaveMultiStationTime(const char * strDataType, int nTimeRange, const char * strTimeValue);
	int SaveMultiStationData(const char * strDataType, int nTimeRange, const char * strStationTime, const char * strData, int nDataLen, int nExpirationTime = 0);

	// 7-2 获取数据
	int GetAllMultiStationType(StringList &lstMultiStationType, bool bSortFlg = false);
	int GetMultiStationTime(const char * strDataType, int nTimeRange, StringList &lstStationTime, bool bSortFlg = false);
	int GetMultiStationData(const char * strDataType, int nTimeRange, const char * strStationTime, char* &pFileData, int &nDataLen);

	// 7-3 删除数据
	int DeleteMultiStation(const char * strDataType, int nTimeRange);
	int DeleteMultiStationData(const char * strDataType, int nTimeRange, const char * strStationTime);

	/* 8. 站点配置信息 */
	int GetStationCfgData(char * &strStationCfg, int &nDataLen, int nClientID = -1);
	int SaveStationCfgData(const char * strStationCfg, int nDataLen, int nClientID = -1);
	int DeleteStationCfgData(int nClientID = -1);

private:
	// 添加，获取，删除公共方法
	int SaveKey(const char * strRootKeyLocker, const char * strRootKeyName, const char * strValueKeyName);
	int SaveInfo(const char * strInfoKeyLocker, const char * strInfoKeyName, ST_PRODUCT stProduct);
	int SaveInfo(const char * strInfoKeyLocker, const char * strInfoKeyName, const char * strInfoValue);
	int SaveTime(const char * strTimeKeyLocker, const char * strTimeKeyName, const char * strTimeValue);
	int SaveData(const char * strDataKeyLocker, const char * strDataKeyName, const char * strTimeValue, const char * szData, int nDataSize, int nExpirationTime = 0);
	int SaveData(const char * strDataKeyLocker, const char * strDataKeyName, const char * strTimeValue, float * fData, int nDataSize, int nExpirationTime = 0);
	int SaveData(const char * strDataKeyLocker, const char * strDataKeyName, const char * strTimeValue, float * fData, int nDataSize, float fLon1, float fLon2, float fLat1, float fLat2, int nExpirationTime = 0);
	int SaveData_s(const char * strDataKeyLocker, const char * strDataKeyName, const char * strTimeValue, float * fData, int nDataSize, int nExpirationTime = 0);

	int GetAllKey(const char * strRootKeyName, StringList &lstKeyName, bool bSortFlg = false);
	int GetInfo(const char * strInfoKeyName, ST_PRODUCT &stProduct);
	int GetInfo(const char * strInfoKeyName, float &fLon, float &fLat, float &fHeight);
	int GetAllTime(const char * strTimeKeyName, StringList &lstTime, bool bSortFlg = false);
	int GetData(const char * strDataKeyName, char* &pData, int &nDataLen);
	int GetData(const char * strDataKeyName, float* &fData, int &nDataLen);
	int GetData(const char * strDataKeyName, float* &fData, int &nDataLen, float &fLon1, float &fLon2, float &fLat1, float &fLat2);
	int GetData_s(const char * strDataKeyName, float* &fData, int &nDataLen);

	int DeleteKey(const char * strRootKeyLocker, const char * strRootKeyName, const char * strValueKeyName);
	int DeleteInfo(const char * strInfoKeyLocker, const char * strInfoKeyName);
	int DeleteTime(const char * strTimeKeyLocker, const char * strTimeKeyName, const char * strTimeValue);
	int DeleteData(const char * strDataKeyLocker, const char * strDataKeyName);
	int DeleteData_s(const char * strDataKeyLocker, const char * strDataKeyName);

private:
	// 字符串判断拼接处理
	bool FindValueFromString(string strSrc, string strValue);
	string DeleteValueFromString(string strSrc, string strValue);

private:
	LIB_HANDLE    hDll;               // DLL调用句柄
	lpGetProvider m_funGetProvider;   // 获取实际类型处理对象指针
	ProviderBase * m_pCached;         // 缓存处理对象
	ST_CACHED      m_stCached;        // 缓存基本配置信息
};

#endif // CACHEDIMPL_H
