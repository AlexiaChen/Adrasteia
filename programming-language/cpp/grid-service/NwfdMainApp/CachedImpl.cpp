#include "log.h"
#include "CachedImpl.h"
#include "ZBase64.h"
#include "JSON.h"

/************************************************************************/
/* 构造函数                                                             */
/************************************************************************/
CachedImpl::CachedImpl(ST_CACHED stCached)
{
	hDll = NULL;
	m_funGetProvider = NULL;
	m_pCached = NULL;

	// 缓存配置信息赋值
	m_stCached = stCached;

	// 获取缓存处理对象
	GetProvider();
}

/************************************************************************/
/* 析构函数                                                             */
/************************************************************************/
CachedImpl::~CachedImpl()
{
	// 释放缓存处理对象
	if (m_pCached)
	{
		delete m_pCached;
		m_pCached = NULL;
	}

	// 释放动态库句柄
	if (hDll)
	{
		Lib_Close(hDll);
	}
}

/************************************************************************/
/* 获取缓存处理对象                                                     */
/************************************************************************/
int CachedImpl::GetProvider()
{
	int nErrCode; 
	string strImplFileName = "";

	// 判断cached类型
	switch (m_stCached.emType)
	{
	case _CACHED_UNKNOW:
		strImplFileName = "";
		break;

	case _CACHED_REDIS:  // redis
		strImplFileName = NWFD_REDISIMPL;
		break;

	case _CACHED_MEMCACHED:    // memcached
		strImplFileName = NWFD_MEMCACHEIMPL;
		break;

	default:
		break;
	}

	// 获取缓存操作对象
	nErrCode = LoadImplLibrary(strImplFileName);
	if (nErrCode != SUCCESS)
	{
		// 返回加载动态库错误编码
		return nErrCode;
	}

	if (m_funGetProvider)
	{
		m_pCached = m_funGetProvider();
		if (m_pCached == NULL)
		{
			return GET_PROVIDER_FAIL;
		}
	}

	// 进行缓存连接
	if (m_pCached)
	{
		if (!m_pCached->Connect(m_stCached.strSvrAddr, m_stCached.nPortNo))
		{
			// todo 连接失败
			return CACHED_CONNECT_FAIL;
		}
	}

	return SUCCESS;
}

/************************************************************************/
/* 加载缓存处理的动态库                                                 */
/************************************************************************/
int CachedImpl::LoadImplLibrary(string strImplFileName)
{
	// 动态库文件名称判断
	if (strImplFileName == "")
	{
		return ERR_CACHED_TYPE;
	}

	// 加载动态库
#ifdef CLIENT_USE
	hDll = Lib_Open(strImplFileName.data());
#else
	hDll = Lib_Open(Lib_Str(QString(strImplFileName.data())));
#endif
	if (hDll)
	{
		this->m_funGetProvider = (lpGetProvider)Lib_GetFun(hDll, "GetProvider");
		if (this->m_funGetProvider == NULL)
		{
			// 获取接口方法失败
			return GET_LIB_FUN_FAIL;
		}
	}
	else
	{
		// 加载动态库失败
		return LOAD_LIB_FAIL;
	}

	return SUCCESS;
}

/************************************************************************/
/* 获取连接状态                                                         */
/************************************************************************/
int CachedImpl::ConnectStatus()
{
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	bool bRet = m_pCached->ConnectStatus();
	if (!bRet)
	{
		return CACHED_CONNECT_ABNORMAL;
	}

	return SUCCESS;
}

/************************************************************************/
/* 重新连接缓存                                                         */
/************************************************************************/
int CachedImpl::ReConnect()
{
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	bool bRet = m_pCached->ReConnect();
	if (!bRet)
	{
		return CACHED_CONNECT_FAIL;
	}

	return SUCCESS;
}

/************************************************************************/
/* 断开连接                                                             */
/************************************************************************/
int CachedImpl::DisConnect()
{
	if (m_pCached)
	{
		m_pCached->DisConnect();
	}

	return SUCCESS;
}
/************************************************************************/
/* 清空数据库                                                           */
/************************************************************************/
int CachedImpl::FlushDB()
{
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	return m_pCached->FlushDB();
}
/************************************************************************/
/* 清空数据库                                                           */
/************************************************************************/
bool CachedImpl::SetChachedFlg(const char * strFlg)
{
	string strValue = string(strFlg);
	return m_pCached->SetValue("CachedFlg", strValue.data(), strValue.size());
}

/************************************************************************/
/* 清空数据库                                                           */
/************************************************************************/
char * CachedImpl::GetCachedFlg()
{
	int nValueSize;
	return m_pCached->GetValue("CachedFlg", nValueSize);
}

/************************************************************************/
/* 1.1 保存产品KEY                                                      */
/************************************************************************/
int CachedImpl::SaveProductKey(const char * strProductKey)
{
	char strProductRoot[256];
	char strProductLocker[256];

	memset(strProductRoot, 0x00, sizeof(strProductRoot));
	memset(strProductLocker, 0x00, sizeof(strProductLocker));

	strcpy(strProductRoot, PRODUCT_ROOT);        // 根
	strcpy(strProductLocker, PRODUCT_ROOT_LOCK); // 锁

	// 调用保存Key公共方法
	return SaveKey(strProductLocker, strProductRoot, strProductKey);
}

/************************************************************************/
/* 1.2 保存产品信息                                                     */
/************************************************************************/
int CachedImpl::SaveProductInfo(ST_PRODUCT stProduct)
{
	// 保存产品Key
	int nErr;
#ifdef CLIENT_USE
	char szProductkey[256];
	memset(szProductkey, 0x00, sizeof(szProductkey));
	strcpy(szProductkey, stProduct.strKey.data());
	nErr = SaveProductKey(szProductkey);
#else
	nErr = SaveProductKey(stProduct.strKey.toLocal8Bit().data());
#endif
	if (nErr != SUCCESS)
	{
		return nErr;
	}

	// 保存产品信息
	char szProductInfoKey[256];
	char szInfoKeyLocker[256];

	memset(szProductInfoKey, 0x00, sizeof(szProductInfoKey));
	memset(szInfoKeyLocker, 0x00, sizeof(szInfoKeyLocker));

#ifdef CLIENT_USE
	sprintf(szProductInfoKey, PRODUCT_INFO, stProduct.strKey.data());
	sprintf(szInfoKeyLocker,  PRODUCT_INFO_LOCK, stProduct.strKey.data());
#else
	sprintf(szProductInfoKey, PRODUCT_INFO, stProduct.strKey.toLocal8Bit().data());
	sprintf(szInfoKeyLocker, PRODUCT_INFO_LOCK, stProduct.strKey.toLocal8Bit().data());
#endif

	// 调用保存Key公共方法
	return SaveInfo(szInfoKeyLocker, szProductInfoKey, stProduct);
}

/************************************************************************/
/* 1.3 保存产品时间                                                     */
/************************************************************************/
int CachedImpl::SaveProductTime(const char * strProductKey, const char * strTimeValue)
{
	char szProductTimeKey[256];
	char szTimeKeyLocker[256];

	memset(szProductTimeKey, 0x00, sizeof(szProductTimeKey));
	memset(szTimeKeyLocker, 0x00, sizeof(szTimeKeyLocker));

	sprintf(szProductTimeKey, PRODUCT_TIME, strProductKey);
	sprintf(szTimeKeyLocker,  PRODUCT_TIME_LOCK, strProductKey);

	// 调用保存Key公共方法
	return SaveTime(szTimeKeyLocker, szProductTimeKey, strTimeValue);
}

/************************************************************************/
/* 1.4 保存产品数据                                                     */
/************************************************************************/
int CachedImpl::SaveProductData(const char * strProductKey, const char * strTimeValue, float * fData, int nDataSize, int nExpirationTime)
{
	// 保存时间
	int nErr = SaveProductTime(strProductKey, strTimeValue);
	if (nErr != SUCCESS)
	{
		return nErr;
	}

	// 保存数据
	char szProductDataKey[256];
	char szDataKeyLocker[256];

	memset(szProductDataKey, 0x00, sizeof(szProductDataKey));
	memset(szDataKeyLocker, 0x00, sizeof(szDataKeyLocker));

	sprintf(szProductDataKey, PRODUCT_MEM_DATA, strProductKey, strTimeValue);
	sprintf(szDataKeyLocker, PRODUCT_MEM_DATA_LOCK, strProductKey, strTimeValue);

	// 调用保存Key公共方法
	return SaveData(szDataKeyLocker, szProductDataKey, strTimeValue, fData, nDataSize, nExpirationTime);
}

/************************************************************************/
/* 1.5 获取所有的产品KEY                                                */
/************************************************************************/
int CachedImpl::GetAllProductKey(StringList &lstProductKey, bool bSortFlg)
{
	char strProductRoot[256];
	memset(strProductRoot, 0x00, sizeof(strProductRoot));
	strcpy(strProductRoot, PRODUCT_ROOT); 

	return GetAllKey(strProductRoot, lstProductKey, bSortFlg);
}

/************************************************************************/
/* 1.6 获取产品信息                                                    */
/************************************************************************/
int CachedImpl::GetProductInfo(const char * strProductKey, ST_PRODUCT &stProduct)
{
	char szProductInfoKey[256];
	memset(szProductInfoKey, 0x00, sizeof(szProductInfoKey));
	sprintf(szProductInfoKey, PRODUCT_INFO, strProductKey);

	return GetInfo(szProductInfoKey, stProduct);
}

/************************************************************************/
/* 1.7 获取所有的产品时间                                               */
/************************************************************************/
int CachedImpl::GetProductTime(const char * strProductKey, StringList &lstProductTime, bool bSortFlg)
{
	char szProductTimeKey[256];
	memset(szProductTimeKey, 0x00, sizeof(szProductTimeKey));
	sprintf(szProductTimeKey, PRODUCT_TIME, strProductKey);

	return GetAllTime(szProductTimeKey, lstProductTime, bSortFlg);
}

/************************************************************************/
/* 1.8 获取产品数据                                                     */
/************************************************************************/
int CachedImpl::GetProductData(const char * strProductKey, const char * strProductTime, float* &fData, int &nDataLen)
{
	char szProductDataKey[256];
	memset(szProductDataKey, 0x00, sizeof(szProductDataKey));
	sprintf(szProductDataKey, PRODUCT_MEM_DATA, strProductKey, strProductTime);

	return GetData(szProductDataKey, fData, nDataLen);
}
/************************************************************************/
/* 1.9 删除某产品                                                       */
/************************************************************************/
int CachedImpl::DeleteProduct(const char * strProductKey)
{
	int nErr;

	// 1. 获取整个时间列表
	StringList  lstProductTime;
	nErr = GetProductTime(strProductKey, lstProductTime);
	if (nErr != SUCCESS)
	{
		return nErr;
	}

	// 遍历删除产品数据
	StringList::iterator iter;
	string strTime;
	char szTime[256];
	string strProductDataKey;
	for (iter = lstProductTime.begin(); iter != lstProductTime.end(); iter++)
	{
		strTime = *iter;
		memset(szTime, 0x00, sizeof(szTime));
		strcpy(szTime, strTime.data());

		// 删除数据
		nErr = DeleteProductData(strProductKey, szTime);
		if ( nErr != SUCCESS)
		{
			return nErr;
		}
	}

	// 删除时间数据
	char szProductTimeKey[256];
	memset(szProductTimeKey, 0x00, sizeof(szProductTimeKey));
	sprintf(szProductTimeKey, PRODUCT_TIME, strProductKey);
	bool bRet = m_pCached->DelKey(szProductTimeKey);
	if (!bRet)
	{
		// 因为Key不存在而失败，忽略
		if (m_pCached->GetLastError() != MEMCACHE_KEY_NOT_FOUND)
		{
			return CACHED_DELVALUE_FAIL;
		}
	}

	// 删除信息数据
	char szProductInfoKey[256];
	char szInfoKeyLocker[256];
	memset(szProductInfoKey, 0x00, sizeof(szProductInfoKey));
	memset(szInfoKeyLocker, 0x00, sizeof(szInfoKeyLocker));
	sprintf(szProductInfoKey, PRODUCT_INFO, strProductKey);
	sprintf(szInfoKeyLocker, PRODUCT_INFO_LOCK, strProductKey);

	nErr = DeleteInfo(szInfoKeyLocker, szProductInfoKey);
	if (nErr != SUCCESS)
	{
		return nErr;
	}

	// 删除产品Key ( 从Product中移除该Key) - 锁
	char strProductRoot[256];
	char strProductLocker[256];
	memset(strProductRoot, 0x00, sizeof(strProductRoot));
	memset(strProductLocker, 0x00, sizeof(strProductLocker));
	strcpy(strProductRoot, PRODUCT_ROOT);        // 根
	strcpy(strProductLocker, PRODUCT_ROOT_LOCK); // 锁

	nErr = DeleteKey(strProductLocker, strProductRoot, strProductKey);
	if (nErr != SUCCESS)
	{
		return nErr;
	}

	return SUCCESS;
}

/************************************************************************/
/* 1.10 删除产品数据                                                    */
/************************************************************************/
int CachedImpl::DeleteProductData(const char * strProductKey, const char * strProductTime)
{
	// 删除数据
	char szProductDataKey[256];
	char szDataKeyLocker[256];

	memset(szProductDataKey, 0x00, sizeof(szProductDataKey));
	memset(szDataKeyLocker, 0x00, sizeof(szDataKeyLocker));

	sprintf(szProductDataKey, PRODUCT_MEM_DATA, strProductKey, strProductTime);
	sprintf(szDataKeyLocker, PRODUCT_MEM_DATA_LOCK, strProductKey, strProductTime);

	int nErr = DeleteData(szDataKeyLocker, szProductDataKey);
	if (nErr != SUCCESS)
	{
		return nErr;
	}

	// 删除时间
	char szProductTimeKey[256];
	char szTimeKeyLocker[256];

	memset(szProductTimeKey, 0x00, sizeof(szProductTimeKey));
	memset(szTimeKeyLocker, 0x00, sizeof(szTimeKeyLocker));

	sprintf(szProductTimeKey, PRODUCT_TIME, strProductKey);
	sprintf(szTimeKeyLocker, PRODUCT_TIME_LOCK, strProductKey);

	return DeleteTime(szTimeKeyLocker, szProductTimeKey, strProductTime);
}

/************************************************************************/
/* 2.1 保存拼图KEY                                                      */
/************************************************************************/
int CachedImpl::SaveMergeKey(const char * strMergeKey)
{
	char strMergeRoot[256];
	char strMergeLocker[256];

	memset(strMergeRoot, 0x00, sizeof(strMergeRoot));
	memset(strMergeLocker, 0x00, sizeof(strMergeLocker));

	strcpy(strMergeRoot, MERGE_ROOT);        // 根
	strcpy(strMergeLocker, MERGE_ROOT_LOCK); // 锁

	// 调用保存Key公共方法
	return SaveKey(strMergeLocker, strMergeRoot, strMergeKey);
}

/************************************************************************/
/* 2.3 保存产品时间                                                     */
/************************************************************************/
int CachedImpl::SaveMergeTime(const char * strMergeKey, const char * strTimeValue)
{
	char szMergeTime[256];
	char szTimeKeyLocker[256];

	memset(szMergeTime, 0x00, sizeof(szMergeTime));
	memset(szTimeKeyLocker, 0x00, sizeof(szTimeKeyLocker));

	sprintf(szMergeTime, MERGE_TIME, strMergeKey);
	sprintf(szTimeKeyLocker, MERGE_TIME_LOCK, strMergeKey);

	// 调用保存Key公共方法
	return SaveTime(szTimeKeyLocker, szMergeTime, strTimeValue);
}

/************************************************************************/
/* 2.4 保存拼图数据                                                     */
/************************************************************************/
int CachedImpl::SaveMergeData(const char * strMergeKey, const char * strTimeValue, float * fData, int nDataSize, int nExpirationTime)
{
	// 保存时间
	int nErr = SaveMergeTime(strMergeKey, strTimeValue);
	if (nErr != SUCCESS)
	{
		return nErr;
	}

	// 保存数据
	char szMergeDataKey[256];
	char szDataKeyLocker[256];

	memset(szMergeDataKey, 0x00, sizeof(szMergeDataKey));
	memset(szDataKeyLocker, 0x00, sizeof(szDataKeyLocker));

	sprintf(szMergeDataKey, MERGE_MEM_DATA, strMergeKey, strTimeValue);
	sprintf(szDataKeyLocker, MERGE_MEM_DATA_LOCK, strMergeKey, strTimeValue);

	// 调用保存Key公共方法
	return SaveData(szDataKeyLocker, szMergeDataKey, strTimeValue, fData, nDataSize, nExpirationTime);
}

/************************************************************************/
/* 2.5 获取所有的拼图KEY                                                */
/************************************************************************/
int CachedImpl::GetAllMergeKey(StringList &lstMergeKey, bool bSortFlg)
{
	char strMergeRoot[256];
	memset(strMergeRoot, 0x00, sizeof(strMergeRoot));
	strcpy(strMergeRoot, MERGE_ROOT);

	return GetAllKey(strMergeRoot, lstMergeKey, bSortFlg);
}

/************************************************************************/
/* 2.7 获取所有的拼图时间                                               */
/************************************************************************/
int CachedImpl::GetMergeTime(const char * strMergeKey, StringList &lstMergeTime, bool bSortFlg)
{
	char szMergeTimeKey[256];
	memset(szMergeTimeKey, 0x00, sizeof(szMergeTimeKey));
	sprintf(szMergeTimeKey, MERGE_TIME, strMergeKey);

	return GetAllTime(szMergeTimeKey, lstMergeTime, bSortFlg);
}

/************************************************************************/
/* 2.8 获取拼图数据                                                     */
/************************************************************************/
int CachedImpl::GetMergeData(const char * strMergeKey, const char * strMergeTime, float* &fData, int &nDataLen)
{
	char szMergeDataKey[256];
	memset(szMergeDataKey, 0x00, sizeof(szMergeDataKey));
	sprintf(szMergeDataKey, MERGE_MEM_DATA, strMergeKey, strMergeTime);

	return GetData(szMergeDataKey, fData, nDataLen);
}

/************************************************************************/
/* 2.9 删除某产品                                                       */
/************************************************************************/
int CachedImpl::DeleteMerge(const char * strProductKey)
{
	int nErr;

	// 1. 获取整个时间列表
	StringList  lstProductTime;
	nErr = GetMergeTime(strProductKey, lstProductTime);
	if (nErr != SUCCESS)
	{
		return nErr;
	}

	// 遍历删除产品数据
	StringList::iterator iter;
	string strTime;
	char szTime[256];
	string strProductDataKey;
	for (iter = lstProductTime.begin(); iter != lstProductTime.end(); iter++)
	{
		strTime = *iter;
		memset(szTime, 0x00, sizeof(szTime));
		strcpy(szTime, strTime.data());

		// 删除数据
		nErr = DeleteMergeData(strProductKey, szTime);
		if (nErr != SUCCESS)
		{
			return nErr;
		}
	}

	// 删除时间数据
	char szProductTimeKey[256];
	memset(szProductTimeKey, 0x00, sizeof(szProductTimeKey));
	sprintf(szProductTimeKey, MERGE_TIME, strProductKey);
	bool bRet = m_pCached->DelKey(szProductTimeKey);
	if (!bRet)
	{
		// 因为Key不存在而失败，忽略
		if (m_pCached->GetLastError() != MEMCACHE_KEY_NOT_FOUND)
		{
			return CACHED_DELVALUE_FAIL;
		}
	}

	// 删除信息数据
	//char szProductInfoKey[256];
	//char szInfoKeyLocker[256];
	//memset(szProductInfoKey, 0x00, sizeof(szProductInfoKey));
	//memset(szInfoKeyLocker, 0x00, sizeof(szInfoKeyLocker));
	//sprintf(szProductInfoKey, MERGE_INFO, strProductKey);
	//sprintf(szInfoKeyLocker, MERGE_INFO_LOCK, strProductKey);

	//nErr = DeleteInfo(szInfoKeyLocker, szProductInfoKey);
	//if (nErr != NULL)
	//{
	//	return nErr;
	//}

	// 删除产品Key ( 从Product中移除该Key) - 锁
	char strProductRoot[256];
	char strProductLocker[256];
	memset(strProductRoot, 0x00, sizeof(strProductRoot));
	memset(strProductLocker, 0x00, sizeof(strProductLocker));
	strcpy(strProductRoot, MERGE_ROOT);        // 根
	strcpy(strProductLocker, MERGE_ROOT_LOCK); // 锁

	nErr = DeleteKey(strProductLocker, strProductRoot, strProductKey);
	if (nErr != SUCCESS)
	{
		return nErr;
	}

	return SUCCESS;
}

/************************************************************************/
/* 2.10 删除产品数据                                                    */
/************************************************************************/
int CachedImpl::DeleteMergeData(const char * strMergeKey, const char * strMergeTime)
{
	// 删除数据
	char szMergeDataKey[256];
	char szDataKeyLocker[256];

	memset(szMergeDataKey, 0x00, sizeof(szMergeDataKey));
	memset(szDataKeyLocker, 0x00, sizeof(szDataKeyLocker));

	sprintf(szMergeDataKey, MERGE_MEM_DATA, strMergeKey, strMergeTime);
	sprintf(szDataKeyLocker, MERGE_MEM_DATA_LOCK, strMergeKey, strMergeTime);

	int nErr = DeleteData(szDataKeyLocker, szMergeDataKey);
	if (nErr != SUCCESS)
	{
		return nErr;
	}

	// 删除时间
	char szMergeTimeKey[256];
	char szTimeKeyLocker[256];

	memset(szMergeTimeKey, 0x00, sizeof(szMergeTimeKey));
	memset(szTimeKeyLocker, 0x00, sizeof(szTimeKeyLocker));

	sprintf(szMergeTimeKey, MERGE_TIME, strMergeKey);
	sprintf(szTimeKeyLocker, MERGE_TIME_LOCK, strMergeKey);

	return DeleteTime(szTimeKeyLocker, szMergeTimeKey, strMergeTime);
}

/************************************************************************/
/* 3.1 保存客户端KEY                                                    */
/************************************************************************/
int CachedImpl::SaveClientKey(int nClientID, const char * strProductKey)
{
	char szClientRoot[256];
	char szClientLocker[256];

	memset(szClientRoot, 0x00, sizeof(szClientRoot));
	memset(szClientLocker, 0x00, sizeof(szClientLocker));

	sprintf(szClientRoot, CLIENT_ROOT, nClientID);        // 根
	sprintf(szClientLocker, CLIENT_ROOT_LOCK, nClientID); // 锁

	// 调用保存Key公共方法
	return SaveKey(szClientLocker, szClientRoot, strProductKey);
}

/************************************************************************/
/* 3.3 保存客户端时间                                                   */
/************************************************************************/
int CachedImpl::SaveClientTime(int nClientID, const char * strProductKey, const char * strTimeValue)
{
	char szClientTime[256];
	char szTimeKeyLocker[256];

	memset(szClientTime, 0x00, sizeof(szClientTime));
	memset(szTimeKeyLocker, 0x00, sizeof(szTimeKeyLocker));

	sprintf(szClientTime, CLIENT_TIME, nClientID, strProductKey);
	sprintf(szTimeKeyLocker, CLIENT_TIME_LOCK, nClientID, strProductKey);

	// 调用保存Key公共方法
	return SaveTime(szTimeKeyLocker, szClientTime, strTimeValue);
}

/************************************************************************/
/* 3.4 保存客户端数据                                                   */
/************************************************************************/
int CachedImpl::SaveClientData(int nClientID, const char * strProductKey, const char * strTimeValue, float * fData, int nDataSize, float fLon1, float fLon2, float fLat1, float fLat2, int nExpirationTime)
{
	// 保存时间
	int nErr = SaveClientTime(nClientID, strProductKey, strTimeValue);
	if (nErr != SUCCESS)
	{
		return nErr;
	}

	// 保存数据
	char szClientDataKey[256];
	char szDataKeyLocker[256];

	memset(szClientDataKey, 0x00, sizeof(szClientDataKey));
	memset(szDataKeyLocker, 0x00, sizeof(szDataKeyLocker));

	sprintf(szClientDataKey, CLIENT_MEM_DATA, nClientID, strProductKey, strTimeValue);
	sprintf(szDataKeyLocker, CLIENT_MEM_DATA_LOCK, nClientID, strProductKey, strTimeValue);

	// 调用保存Key公共方法
	return SaveData(szDataKeyLocker, szClientDataKey, strTimeValue, fData, nDataSize, fLon1, fLon2, fLat1, fLat2, nExpirationTime);
}

/************************************************************************/
/* 3.5 获取所有的客户端KEY                                              */
/************************************************************************/
int CachedImpl::GetAllClientKey(int nClientID, StringList &lstProductKey,  bool bSortFlg)
{
	char strClientRoot[256];
	memset(strClientRoot, 0x00, sizeof(strClientRoot));
	sprintf(strClientRoot, CLIENT_ROOT, nClientID);

	return GetAllKey(strClientRoot, lstProductKey, bSortFlg);
}

/************************************************************************/
/* 3.7 获取客户端时间列表                                               */
/************************************************************************/
int CachedImpl::GetClientTime(int nClientID, const char * strProductKey, StringList &lstClientTime, bool bSortFlg)
{
	char szClientTime[256];
	memset(szClientTime, 0x00, sizeof(szClientTime));
	sprintf(szClientTime, CLIENT_TIME, nClientID, strProductKey);

	// 调用保存Key公共方法
	return GetAllTime(szClientTime, lstClientTime, bSortFlg);
}

/************************************************************************/
/* 3.8 获取客户端数据                                                   */
/************************************************************************/
int CachedImpl::GetClientData(int nClientID, const char * strProductKey, const char * strClientTime, float* &fData, int &nDataLen, float &fLon1, float &fLon2, float &fLat1, float &fLat2)
{
	char szClientDataKey[256];
	memset(szClientDataKey, 0x00, sizeof(szClientDataKey));
	sprintf(szClientDataKey, CLIENT_MEM_DATA, nClientID, strProductKey, strClientTime);

	return GetData(szClientDataKey, fData, nDataLen, fLon1, fLon2, fLat1, fLat2);
}

/************************************************************************/
/* 3.9 删除某产品                                                       */
/************************************************************************/
int CachedImpl::DeleteClient(int nClientID, char * strProductKey)
{
	int nErr;

	// 1. 获取整个时间列表
	StringList  lstProductTime;
	nErr = GetClientTime(nClientID, strProductKey, lstProductTime);
	if (nErr != SUCCESS)
	{
		return nErr;
	}

	// 遍历删除产品数据
	StringList::iterator iter;
	string strTime;
	char szTime[256];
	string strProductDataKey;
	for (iter = lstProductTime.begin(); iter != lstProductTime.end(); iter++)
	{
		strTime = *iter;
		memset(szTime, 0x00, sizeof(szTime));
		strcpy(szTime, strTime.data());

		// 删除数据
		nErr = DeleteClientData(nClientID, strProductKey, szTime);
		if (nErr != SUCCESS)
		{
			return nErr;
		}
	}

	// 删除时间数据
	char szProductTimeKey[256];
	memset(szProductTimeKey, 0x00, sizeof(szProductTimeKey));
	sprintf(szProductTimeKey, CLIENT_TIME, nClientID, strProductKey);
	bool bRet = m_pCached->DelKey(szProductTimeKey);
	if (!bRet)
	{
		// 因为Key不存在而失败，忽略
		if (m_pCached->GetLastError() != MEMCACHE_KEY_NOT_FOUND)
		{
			return CACHED_DELVALUE_FAIL;
		}
	}

	// 删除信息数据
	//char szProductInfoKey[256];
	//char szInfoKeyLocker[256];
	//memset(szProductInfoKey, 0x00, sizeof(szProductInfoKey));
	//memset(szInfoKeyLocker, 0x00, sizeof(szInfoKeyLocker));
	//sprintf(szProductInfoKey, MERGE_INFO, strProductKey);
	//sprintf(szInfoKeyLocker, MERGE_INFO_LOCK, strProductKey);

	//nErr = DeleteInfo(szInfoKeyLocker, szProductInfoKey);
	//if (nErr != NULL)
	//{
	//	return nErr;
	//}

	// 删除产品Key ( 从Client中移除该Key)
	char strProductRoot[256];
	char strProductLocker[256];
	memset(strProductRoot, 0x00, sizeof(strProductRoot));
	memset(strProductLocker, 0x00, sizeof(strProductLocker));
	sprintf(strProductRoot, CLIENT_ROOT, nClientID);        // 根
	sprintf(strProductLocker, CLIENT_ROOT_LOCK, nClientID); // 锁

	nErr = DeleteKey(strProductLocker, strProductRoot, strProductKey);
	if (nErr != SUCCESS)
	{
		return nErr;
	}

	return SUCCESS;
}

/************************************************************************/
/* 3.10 删除产品数据                                                    */
/************************************************************************/
int CachedImpl::DeleteClientData(int nClientID, char * strProductKey, char * strClientTime)
{
	// 删除数据
	char szClientDataKey[256];
	char szDataKeyLocker[256];

	memset(szClientDataKey, 0x00, sizeof(szClientDataKey));
	memset(szDataKeyLocker, 0x00, sizeof(szDataKeyLocker));

	sprintf(szClientDataKey, CLIENT_MEM_DATA, nClientID, strProductKey, strClientTime);
	sprintf(szDataKeyLocker, CLIENT_MEM_DATA_LOCK, nClientID, strProductKey, strClientTime);

	int nErr = DeleteData(szDataKeyLocker, szClientDataKey);
	if (nErr != SUCCESS)
	{
		return nErr;
	}

	// 删除时间
	char szClientTimeKey[256];
	char szTimeKeyLocker[256];

	memset(szClientTimeKey, 0x00, sizeof(szClientTimeKey));
	memset(szTimeKeyLocker, 0x00, sizeof(szTimeKeyLocker));

	sprintf(szClientTimeKey, CLIENT_TIME, nClientID, strProductKey);
	sprintf(szTimeKeyLocker, CLIENT_TIME_LOCK, nClientID, strProductKey);

	return DeleteTime(szTimeKeyLocker, szClientTimeKey, strClientTime);
}

/************************************************************************/
/* 4.1 保存站点                                                         */
/************************************************************************/
int CachedImpl::SaveStation(const char * strCityType, const char * strDataType, int nTimeRange, const char * strStationNo)
{
	char strStationRoot[256];
	char strStationLocker[256];

	memset(strStationRoot, 0x00, sizeof(strStationRoot));
	memset(strStationLocker, 0x00, sizeof(strStationLocker));

	sprintf(strStationRoot, STATION_ROOT, strCityType, strDataType, nTimeRange);        // 根
	sprintf(strStationLocker, STATION_ROOT_LOCK, strCityType, strDataType, nTimeRange); // 锁

	// 调用保存Key公共方法
	return SaveKey(strStationLocker, strStationRoot, strStationNo);
}

/************************************************************************/
/* 4.2 保存站点信息                                                     */
/************************************************************************/
int CachedImpl::SaveStationInfo(const char * strCityType, const char * strDataType, int nTimeRange, const char * strStationNo, float fLon, float fLat, float fHeight, int nCols)
{
	// 保存产品Key
	int nErr;
	nErr = SaveStation(strCityType, strDataType, nTimeRange, strStationNo);
	if (nErr != SUCCESS)
	{
		return nErr;
	}

	// 保存产品信息
	char szStationInfoKey[256];
	char szInfoKeyLocker[256];
	char szInfoValue[1024];

	memset(szStationInfoKey, 0x00, sizeof(szStationInfoKey));
	memset(szInfoKeyLocker, 0x00, sizeof(szInfoKeyLocker));
	memset(szInfoValue, 0x00, sizeof(szInfoValue));

	sprintf(szStationInfoKey, STATION_INFO, strCityType, strDataType, nTimeRange, strStationNo);
	sprintf(szInfoKeyLocker, STATION_INFO_LOCK, strCityType, strDataType, nTimeRange, strStationNo);
	sprintf(szInfoValue, "%f,%f,%f,%d", fLon, fLat, fHeight, nCols);

	// 调用保存Key公共方法
	return SaveInfo(szInfoKeyLocker, szStationInfoKey, szInfoValue);
}

/************************************************************************/
/* 4.3 保存站点时间                                                     */
/************************************************************************/
int CachedImpl::SaveStationTime(const char * strCityType, const char * strDataType, int nTimeRange, const char * strStationNo, const char * strTimeValue)
{
	char sStationTimeKey[256];
	char szTimeKeyLocker[256];

	memset(sStationTimeKey, 0x00, sizeof(sStationTimeKey));
	memset(szTimeKeyLocker, 0x00, sizeof(szTimeKeyLocker));

	sprintf(sStationTimeKey, STATION_TIME, strCityType, strDataType, nTimeRange, strStationNo);
	sprintf(szTimeKeyLocker, STATION_TIME_LOCK, strCityType, strDataType, nTimeRange, strStationNo);

	// 调用保存Key公共方法
	return SaveTime(szTimeKeyLocker, sStationTimeKey, strTimeValue);
}

/************************************************************************/
/* 4.4 保存站点数据                                                     */
/************************************************************************/
int CachedImpl::SaveStationData(const char * strCityType, const char * strDataType, int nTimeRange, const char * strStationNo, const char * strTimeValue, float * fData, int nDataSize, int nExpirationTime)
{
	// 保存时间
	int nErr = SaveStationTime(strCityType, strDataType, nTimeRange, strStationNo, strTimeValue);
	if (nErr != SUCCESS)
	{
		return nErr;
	}

	// 保存数据
	char szStationDataKey[256];
	char szDataKeyLocker[256];

	memset(szStationDataKey, 0x00, sizeof(szStationDataKey));
	memset(szDataKeyLocker, 0x00, sizeof(szDataKeyLocker));

	sprintf(szStationDataKey, STATION_DATA, strCityType, strDataType, nTimeRange, strStationNo, strTimeValue);
	sprintf(szDataKeyLocker, STATION_DATA_LOCK, strCityType, strDataType, nTimeRange, strStationNo, strTimeValue);

	// 调用保存站点数据的公共方法
	return SaveData_s(szDataKeyLocker, szStationDataKey, strTimeValue, fData, nDataSize, nExpirationTime);
}

/************************************************************************/
/* 4.4 保存站点数据                                                     */
/************************************************************************/
int CachedImpl::UpdateStationData(const char * strCityType, const char * strDataType, int nTimeRange, const char * strStationNo, const char * szTime, float fValue, int nPos, bool &bIsUpdate, bool bSaveStationNo, int nExpirationTime)
{
	// 默认没有更新
	bIsUpdate = false;

	// 1. 缓存对象判断
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	// 2、保存站号
	if (bSaveStationNo)
	{
		SaveStation(strCityType, strDataType, nTimeRange, strStationNo);
	}

	// 修改数据
	char szStationDataKey[256];
	char szDataKeyLocker[256];

	memset(szStationDataKey, 0x00, sizeof(szStationDataKey));
	memset(szDataKeyLocker, 0x00, sizeof(szDataKeyLocker));

	sprintf(szStationDataKey, STATION_DATA, strCityType, strDataType, nTimeRange, strStationNo, szTime);
	sprintf(szDataKeyLocker, STATION_DATA_LOCK, strCityType, strDataType, nTimeRange, strStationNo, szTime);

	// 4、 加锁
	int nDelayCount = 0;
	do
	{
		// 加锁成功
		if (m_pCached->LockKey(szDataKeyLocker))
		{
			break;
		}

		// 延时5毫秒等待处理
		delay(5);
	} while (nDelayCount++ < 100);

	if (nDelayCount >= 100)
	{
		// 加锁，等待超时返回
		return CACHED_LOCK_TIMEOUT;
	}

	// 保存时间
	SaveStationTime(strCityType, strDataType, nTimeRange, strStationNo, szTime);

	// 5、获取数据
	float fData[21];
	int nValueLen;
	char * pValue = m_pCached->GetValue(szStationDataKey, nValueLen);
	if (pValue == NULL || nValueLen <= 0)
	{
		for (int i = 0; i < 21; i++)
		{
			fData[i] = 999.9f;
		}
	}
	else if (nValueLen > 21 * sizeof(float))
	{
		// 长度异常，返回
#ifndef DEBUG
		// 释放申请的空间
		if (pValue)
		{
			free(pValue);
			pValue = NULL;
		}
#endif
		// 解锁
		m_pCached->UnLockKey(szDataKeyLocker);
		return -1;
	}
	else
	{
		//  数据拷贝
		memcpy((char *)fData, pValue, nValueLen);

		// todo add zhangl 
#ifndef DEBUG
		// 释放申请的空间
		if (pValue)
		{
			free(pValue);
			pValue = NULL;
		}
#endif
	}

	// 存储处理
	bool bRet = true;
	if (IS_EQUAL(fData[nPos], fValue))
	{
		// 与原先的值相等
		bIsUpdate = false;
	}
	else
	{
		// 不等
		fData[nPos] = fValue;

		// 8. 存储数据到缓存中
		int nDataSize = 21 * sizeof(float);
		bRet = m_pCached->SetValue(szStationDataKey, (char *)fData, nDataSize, nExpirationTime);
		if (bRet)
		{
			// 存储成功，值更新
			bIsUpdate = true;
		}
	}

	// 9. 解锁
	m_pCached->UnLockKey(szDataKeyLocker);

	// 10. 返回结果
	if (!bRet)
	{
		return CACHED_SETVALUE_FAIL;
	}

	return SUCCESS;
}

/************************************************************************/
/* 4.4 获取所有站点号                                                   */
/************************************************************************/
int CachedImpl::GetAllStationNo(const char * strCityType, const char * strDataType, int nTimeRange, StringList &lstStationNo, bool bSortFlg)
{
	char strStationRoot[256];
	memset(strStationRoot, 0x00, sizeof(strStationRoot));
	sprintf(strStationRoot, STATION_ROOT, strCityType, strDataType, nTimeRange);        // 根

	return GetAllKey(strStationRoot, lstStationNo, bSortFlg);
}

/************************************************************************/
/* 4.5 获取站点信息                                                     */
/************************************************************************/
int CachedImpl::GeStationInfo(const char * strCityType, const char * strDataType, int nTimeRange, const char * strStationNo, float &fLon, float &fLat, float &fHeight)
{
	// 保存产品信息
	char szStationInfoKey[256];
	memset(szStationInfoKey, 0x00, sizeof(szStationInfoKey));
	sprintf(szStationInfoKey, STATION_INFO, strCityType, strDataType, nTimeRange, strStationNo);

	return GetInfo(szStationInfoKey, fLon, fLat, fHeight);
}

/************************************************************************/
/* 4.6 获取某站所有时间                                                 */
/************************************************************************/
int CachedImpl::GetStationTime(const char * strCityType, const char * strDataType, int nTimeRange, const char * strStationNo, StringList &lstStationTime, bool bSortFlg)
{
	char szStationTime[256];
	memset(szStationTime, 0x00, sizeof(szStationTime));
	sprintf(szStationTime, STATION_TIME, strCityType, strDataType, nTimeRange, strStationNo);

	return GetAllTime(szStationTime, lstStationTime, bSortFlg);
}

/************************************************************************/
/* 4.7 获取某站某时间数据                                               */
/************************************************************************/
int CachedImpl::GetStationData(const char * strCityType, const char * strDataType, int nTimeRange, const char * strStationNo, const char * strStationTime, float* &fData, int &nDataLen)
{
	char szStationDataKey[256];
	memset(szStationDataKey, 0x00, sizeof(szStationDataKey));
	sprintf(szStationDataKey, STATION_DATA, strCityType, strDataType, nTimeRange, strStationNo, strStationTime);

	// 调用保存站点数据的公共方法
	return GetData_s(szStationDataKey, fData, nDataLen);
}

/************************************************************************/
/* 4.8 删除某站点                                                       */
/************************************************************************/
int CachedImpl::DeleteStation(const char * strCityType, const char * strDataType, int nTimeRange, const char * strStationNo)
{
	
	return -1;
}

/************************************************************************/
/* 4.9 删除某站点数据                                                   */
/************************************************************************/
int CachedImpl::DeleteStationData(const char * strCityType, const char * strDataType, int nTimeRange, const char * strStationNo, const char * strStationTime)
{
	// 删除数据
	char szStationDataKey[256];
	char szDataKeyLocker[256];

	memset(szStationDataKey, 0x00, sizeof(szStationDataKey));
	memset(szDataKeyLocker, 0x00, sizeof(szDataKeyLocker));

	sprintf(szStationDataKey, STATION_DATA, strCityType, strDataType, nTimeRange, strStationNo, strStationTime);
	sprintf(szDataKeyLocker, STATION_DATA_LOCK, strCityType, strDataType, nTimeRange, strStationNo, strStationTime);

	int nErr = DeleteData_s(szDataKeyLocker, szStationDataKey);
	if (nErr != SUCCESS)
	{
		return nErr;
	}

	// 删除时间
	char szStationTimeKey[256];
	char szTimeKeyLocker[256];

	memset(szStationTimeKey, 0x00, sizeof(szStationTimeKey));
	memset(szTimeKeyLocker, 0x00, sizeof(szTimeKeyLocker));

	sprintf(szStationTimeKey, STATION_TIME, strCityType, strDataType, nTimeRange, strStationNo);
	sprintf(szTimeKeyLocker, STATION_TIME_LOCK, strCityType, strDataType, nTimeRange, strStationNo);

	return DeleteTime(szTimeKeyLocker, szStationTimeKey, strStationTime);
}

/* 6. 站点文件缓存 StationFile */
// 保存站点文件根
int CachedImpl::SaveStationFile(const char * strCityType, const char * strDataType, int nTimeRange)
{
	char strStationRoot[256];
	char strStationLocker[256];
	char strStationValue[256];

	memset(strStationRoot, 0x00, sizeof(strStationRoot));
	memset(strStationLocker, 0x00, sizeof(strStationLocker));
	memset(strStationValue, 0x00, sizeof(strStationValue));

	sprintf(strStationRoot, STATION_FILE_ROOT);        // 根
	sprintf(strStationLocker, STATION_FILE_ROOT_LOCK); // 锁
	sprintf(strStationValue, STATION_FILE_ROOT_VAL, strCityType, strDataType, nTimeRange); // 值
	
	// 调用保存Key公共方法
	return SaveKey(STATION_FILE_ROOT_LOCK, STATION_FILE_ROOT, strStationValue);
}

// 保存站点文件时间
int CachedImpl::SaveStationFileTime(const char * strCityType, const char * strDataType, int nTimeRange, const char * strTimeValue)
{
	// 保存跟
	int nErr = SaveStationFile(strCityType, strDataType, nTimeRange);
	if (nErr != SUCCESS)
	{
		return nErr;
	}

	char szStationFileTimeKey[256];
	char szTimeKeyLocker[256];

	memset(szStationFileTimeKey, 0x00, sizeof(szStationFileTimeKey));
	memset(szTimeKeyLocker, 0x00, sizeof(szTimeKeyLocker));

	sprintf(szStationFileTimeKey, STATION_FILE_TIME, strCityType, strDataType, nTimeRange);
	sprintf(szTimeKeyLocker, STATION_FILE_TIME_LOCK, strCityType, strDataType, nTimeRange);

	// 调用保存Key公共方法
	return SaveTime(szTimeKeyLocker, szStationFileTimeKey, strTimeValue);
}

// 保存站点文件数据
int CachedImpl::SaveStationFileData(const char * strCityType, const char * strDataType, int nTimeRange, const char * strTimeValue, const char * pFileData, int nDataSize, int nExpirationTime)
{
	// 保存时间
	int nErr = SaveStationFileTime(strCityType, strDataType, nTimeRange, strTimeValue);
	if (nErr != SUCCESS)
	{
		return nErr;
	}

	// 保存数据
	char szStationFileDataKey[256];
	char szDataKeyLocker[256];

	memset(szStationFileDataKey, 0x00, sizeof(szStationFileDataKey));
	memset(szDataKeyLocker, 0x00, sizeof(szDataKeyLocker));

	sprintf(szStationFileDataKey, STATION_FILE_DATA, strCityType, strDataType, nTimeRange, strTimeValue);
	sprintf(szDataKeyLocker, STATION_FILE_DATA_LOCK, strCityType, strDataType, nTimeRange, strTimeValue);

	// 调用保存站点数据的公共方法
	return SaveData(szDataKeyLocker, szStationFileDataKey, strTimeValue, pFileData, nDataSize, nExpirationTime);
}

// 获取所有的站点文件类型
int  CachedImpl::GetAllStationFileType(StringList &lstStationFileType, bool bSortFlg)
{
	return GetAllKey(STATION_FILE_ROOT, lstStationFileType, bSortFlg);
}

// 获取某类型的所有的站点文件时间
int CachedImpl::GetStationFileTime(const char * strCityType, const char * strDataType, int nTimeRange, StringList &lstStationTime, bool bSortFlg)
{
	char strStationFileTime[256];
	memset(strStationFileTime, 0x00, sizeof(strStationFileTime));
	sprintf(strStationFileTime, STATION_FILE_TIME, strCityType, strDataType, nTimeRange);

	return GetAllTime(strStationFileTime, lstStationTime, bSortFlg);
}

// 获取某时间的站点文件内容数据
int CachedImpl::GetStationFileData(const char * strCityType, const char * strDataType, int nTimeRange, const char * strStationTime, char* &pFileData, int &nDataLen)
{
	char szStationFileDataKey[256];
	memset(szStationFileDataKey, 0x00, sizeof(szStationFileDataKey));
	sprintf(szStationFileDataKey, STATION_FILE_DATA, strCityType, strDataType, nTimeRange, strStationTime);

	// 调用保存站点数据的公共方法
	return GetData(szStationFileDataKey, pFileData, nDataLen);
}

// 删除站点文件根
int CachedImpl::DeleteStationFile(const char * strCityType, const char * strDataType, int nTimeRange)
{
	return -1;
}

// 删除站点文件数据
int CachedImpl::DeleteStationFileData(const char * strCityType, const char * strDataType, int nTimeRange, const char * strStationTime)
{
	// 删除数据
	char szStationFileDataKey[256];
	char szDataKeyLocker[256];

	memset(szStationFileDataKey, 0x00, sizeof(szStationFileDataKey));
	memset(szDataKeyLocker, 0x00, sizeof(szDataKeyLocker));

	sprintf(szStationFileDataKey, STATION_FILE_DATA, strCityType, strDataType, nTimeRange, strStationTime);
	sprintf(szDataKeyLocker, STATION_FILE_DATA_LOCK, strCityType, strDataType, nTimeRange, strStationTime);

	int nErr = DeleteData(szDataKeyLocker, szStationFileDataKey);
	if (nErr != SUCCESS)
	{
		return nErr;
	}

	// 删除时间
	char szStationFileTimeKey[256];
	char szTimeKeyLocker[256];

	memset(szStationFileTimeKey, 0x00, sizeof(szStationFileTimeKey));
	memset(szTimeKeyLocker, 0x00, sizeof(szTimeKeyLocker));

	sprintf(szStationFileTimeKey, STATION_FILE_TIME, strCityType, strDataType, nTimeRange);
	sprintf(szTimeKeyLocker, STATION_FILE_TIME_LOCK, strCityType, strDataType, nTimeRange);

	return DeleteTime(szTimeKeyLocker, szStationFileTimeKey, strStationTime);
}

/* 6. 站点文件缓存 StationFile */
// 保存站点文件根
int CachedImpl::SaveMultiStation(const char * strDataType, int nTimeRange)
{
	char strStationRoot[256];
	char strStationLocker[256];
	char strStationValue[256];

	memset(strStationRoot, 0x00, sizeof(strStationRoot));
	memset(strStationLocker, 0x00, sizeof(strStationLocker));
	memset(strStationValue, 0x00, sizeof(strStationValue));

	sprintf(strStationRoot, STATION_MULTI_ROOT);        // 根
	sprintf(strStationLocker, STATION_MULTI_ROOT_LOCK); // 锁
	sprintf(strStationValue, STATION_MULTI_ROOT_VAL, strDataType, nTimeRange); // 值

	// 调用保存Key公共方法
	return SaveKey(strStationLocker, strStationRoot, strStationValue);
}

// 保存站点文件时间
int CachedImpl::SaveMultiStationTime(const char * strDataType, int nTimeRange, const char * strTimeValue)
{
	// 保存跟
	int nErr = SaveMultiStation(strDataType, nTimeRange);
	if (nErr != SUCCESS)
	{
		return nErr;
	}

	char szStationFileTimeKey[256];
	char szTimeKeyLocker[256];

	memset(szStationFileTimeKey, 0x00, sizeof(szStationFileTimeKey));
	memset(szTimeKeyLocker, 0x00, sizeof(szTimeKeyLocker));

	sprintf(szStationFileTimeKey, STATION_MULTI_TIME, strDataType, nTimeRange);
	sprintf(szTimeKeyLocker, STATION_MULTI_TIME_LOCK, strDataType, nTimeRange);

	// 调用保存Key公共方法
	return SaveTime(szTimeKeyLocker, szStationFileTimeKey, strTimeValue);
}

// 保存站点文件数据
int CachedImpl::SaveMultiStationData(const char * strDataType, int nTimeRange, const char * strTimeValue, const char * strData, int nDataSize, int nExpirationTime)
{
	// 保存时间
	int nErr = SaveMultiStationTime(strDataType, nTimeRange, strTimeValue);
	if (nErr != SUCCESS)
	{
		return nErr;
	}

	// 保存数据
	char szStationFileDataKey[256];
	char szDataKeyLocker[256];

	memset(szStationFileDataKey, 0x00, sizeof(szStationFileDataKey));
	memset(szDataKeyLocker, 0x00, sizeof(szDataKeyLocker));

	sprintf(szStationFileDataKey, STATION_MULTI_DATA, strDataType, nTimeRange, strTimeValue);
	sprintf(szDataKeyLocker, STATION_MULTI_DATA_LOCK, strDataType, nTimeRange, strTimeValue);

	// 调用保存站点数据的公共方法
	return SaveData(szDataKeyLocker, szStationFileDataKey, strTimeValue, strData, nDataSize, nExpirationTime);
}

// 获取所有的站点文件类型
int  CachedImpl::GetAllMultiStationType(StringList &lstStationFileType, bool bSortFlg)
{
	return GetAllKey(STATION_MULTI_ROOT, lstStationFileType, bSortFlg);
}

// 获取某类型的所有的站点文件时间
int CachedImpl::GetMultiStationTime(const char * strDataType, int nTimeRange, StringList &lstStationTime, bool bSortFlg)
{
	char strStationFileTime[256];
	memset(strStationFileTime, 0x00, sizeof(strStationFileTime));
	sprintf(strStationFileTime, STATION_MULTI_TIME, strDataType, nTimeRange);

	return GetAllTime(strStationFileTime, lstStationTime, bSortFlg);
}

// 获取某时间的站点文件内容数据
int CachedImpl::GetMultiStationData(const char * strDataType, int nTimeRange, const char * strStationTime, char* &pData, int &nDataLen)
{
	char szStationFileDataKey[256];
	memset(szStationFileDataKey, 0x00, sizeof(szStationFileDataKey));
	sprintf(szStationFileDataKey, STATION_MULTI_DATA, strDataType, nTimeRange, strStationTime);

	// 调用保存站点数据的公共方法
	return GetData(szStationFileDataKey, pData, nDataLen);
}

// 删除站点文件根
int CachedImpl::DeleteMultiStation(const char * strDataType, int nTimeRange)
{
	
	return -1;
}

// 删除站点文件数据
int CachedImpl::DeleteMultiStationData(const char * strDataType, int nTimeRange, const char * strStationTime)
{
	// 删除数据
	char szStationFileDataKey[256];
	char szDataKeyLocker[256];

	memset(szStationFileDataKey, 0x00, sizeof(szStationFileDataKey));
	memset(szDataKeyLocker, 0x00, sizeof(szDataKeyLocker));

	sprintf(szStationFileDataKey, STATION_MULTI_DATA, strDataType, nTimeRange, strStationTime);
	sprintf(szDataKeyLocker, STATION_MULTI_DATA_LOCK, strDataType, nTimeRange, strStationTime);

	int nErr = DeleteData(szDataKeyLocker, szStationFileDataKey);
	if (nErr != SUCCESS)
	{
		return nErr;
	}

	// 删除时间
	char szStationFileTimeKey[256];
	char szTimeKeyLocker[256];

	memset(szStationFileTimeKey, 0x00, sizeof(szStationFileTimeKey));
	memset(szTimeKeyLocker, 0x00, sizeof(szTimeKeyLocker));

	sprintf(szStationFileTimeKey, STATION_MULTI_TIME, strDataType, nTimeRange);
	sprintf(szTimeKeyLocker, STATION_MULTI_TIME_LOCK, strDataType, nTimeRange);

	return DeleteTime(szTimeKeyLocker, szStationFileTimeKey, strStationTime);
}

// 获取站点配置数据
int CachedImpl::GetStationCfgData(char * &strStationCfg, int &nDataLen, int nClientID)
{
	char szType[50];
	memset(szType, 0x00, sizeof(szType));
	if (nClientID == -1) // 缺省，表示服务器数据
	{
		sprintf(szType,"%s","SVR");
	}
	else // 否则为客户端数据
	{
		sprintf(szType, "CLT%d", nClientID);
	}

	char szStationCfgDataKey[256];
	memset(szStationCfgDataKey, 0x00, sizeof(szStationCfgDataKey));
	sprintf(szStationCfgDataKey, STATION_CFG_DATA, szType);

	// 调用保存站点数据的公共方法
	return GetData(szStationCfgDataKey, strStationCfg, nDataLen);
}

// 保存站点配置数据
int CachedImpl::SaveStationCfgData(const char * strStationCfg, int nDataLen, int nClientID)
{
	char szType[50];
	memset(szType, 0x00, sizeof(szType));
	if (nClientID == -1) // 缺省，表示服务器数据
	{
		sprintf(szType, "%s", "SVR");
	}
	else // 否则为客户端数据
	{
		sprintf(szType, "CLT%d", nClientID);
	}

	char szStationCfgDataKey[256];
	char szDataKeyLocker[256];

	memset(szStationCfgDataKey, 0x00, sizeof(szStationCfgDataKey));
	memset(szDataKeyLocker, 0x00, sizeof(szDataKeyLocker));

	sprintf(szStationCfgDataKey, STATION_CFG_DATA, szType);
	sprintf(szDataKeyLocker, STATION_CFG_DATA_LOCK, szType);

	return SaveData(szDataKeyLocker, szStationCfgDataKey, NULL, strStationCfg, nDataLen);
}

// 删除站点配置数据
int CachedImpl::DeleteStationCfgData(int nClientID)
{
	char szType[50];
	memset(szType, 0x00, sizeof(szType));
	if (nClientID == -1) // 缺省，表示服务器数据
	{
		sprintf(szType, "%s", "SVR");
	}
	else // 否则为客户端数据
	{
		sprintf(szType, "CLT%d", nClientID);
	}

	char szStationCfgDataKey[256];
	char szDataKeyLocker[256];
	
	memset(szStationCfgDataKey, 0x00, sizeof(szStationCfgDataKey));
	memset(szDataKeyLocker, 0x00, sizeof(szDataKeyLocker));

	sprintf(szStationCfgDataKey, STATION_CFG_DATA, szType);
	sprintf(szDataKeyLocker, STATION_CFG_DATA_LOCK, szType);

	return DeleteData(szDataKeyLocker, szStationCfgDataKey);
}



