#include "CachedImpl.h"
#include "Common.h"
#include "JSON.h"

/************************************************************************/
/* 保存KEY公共方法                                                      */
/************************************************************************/
int CachedImpl::SaveKey(const char * strRootKeyLocker, const char * strRootKeyName, const char * strValueKeyName)
{
	// 1. 缓存对象判断
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	// 2. 加锁
	int nDelayCount = 0;
	do
	{
		// 加锁成功
		if (m_pCached->LockKey(strRootKeyLocker))
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

	// 3. 获取原ROOT中的数据
	string strValue;
	char *pValue = NULL;
	int  nValueSize = 0;
	pValue = m_pCached->GetValue(strRootKeyName, nValueSize);
	if (pValue == NULL)
	{
		// 没有这个key(LastError=16)
		if (m_pCached->GetLastError() == MEMCACHE_KEY_NOT_FOUND)
		{
			// 3. 重新组装数据
			strValue = string(strValueKeyName);
		}
		else
		{
			// 异常，返回
			m_pCached->UnLockKey(strRootKeyLocker);
			return CACHED_GETVALUE_FAIL;
		}
	}
	else
	{
		strValue = string(pValue);

		// 3. 查找是否已经存在
		if (FindValueFromString(strValue, strValueKeyName))
		{
			// 3.1 已经存在，不需要再保存
			m_pCached->UnLockKey(strRootKeyLocker);
			return SUCCESS;
		}

		// 3.2 数据不存在，重新组装数据(使用","分割)
		strValue = strValue + string(",") + string(strValueKeyName);
	}

	// 4. 保存数据到memcached中
	bool bRet = m_pCached->SetValue(strRootKeyName, strValue.data(), strValue.size());

	// 5. 解锁
	m_pCached->UnLockKey(strRootKeyLocker);

#ifndef DEBUG
	// 释放申请的空间
	if (pValue)
	{
		free(pValue);
		pValue = NULL;
	}
#endif

	if (!bRet)
	{
		return CACHED_SETVALUE_FAIL;
	}

	return SUCCESS;
}

/************************************************************************/
/* 保存信息公共方法                                                     */
/************************************************************************/
int CachedImpl::SaveInfo(const char * strInfoKeyLocker, const char * strInfoKeyName, ST_PRODUCT stProduct)
{
	// 1. 缓存对象判断
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	// 2. 加锁
	int nDelayCount = 0;
	do
	{
		// 加锁成功
		if (m_pCached->LockKey(strInfoKeyLocker))
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

	// 3. 拼接Value信息
	char strValue[2048];
	char strElement[50];
	char strWarning[20];

	memset(strValue, 0x00, sizeof(strValue));
	memset(strElement, 0x00, sizeof(strElement));
	memset(strWarning, 0x00, sizeof(strWarning));

	sprintf(strElement, "%d,%d", stProduct.nElement[0], stProduct.nElement[1]);
	if (stProduct.bIsWarning)
		strcpy(strWarning, "yes");
	else
		strcpy(strWarning, "no");

	// 产品信息格式使用JSON格式
#ifdef CLIENT_USE
	sprintf(strValue, "{\"Name\":\"%s\",\"Role\":\"%s\",\"Type\":\"%s\",\"Publisher\":\"%s\",\"Category\":\"%d\",\"Element\":\"%s\",\"Statistical\":\"%d\",\"Status\":\"%d\",\"OffSet\":\"%f\",\"Lon1\":\"%f\",\"Lon2\":\"%f\",\"Lat1\":\"%f\",\"Lat2\":\"%f\",\"Di\":\"%f\",\"Dj\":\"%f\",\"TimeRange\":\"%d\",\"MaxForecastTime\":\"%d\",\"BusinessStatus\":\"%s\",\"Warning\":\"%s\"}",
		stProduct.strName.data(), stProduct.strRole.data(), stProduct.strType.data(), stProduct.strPublisher.data(),
		stProduct.nCategory, strElement, stProduct.nStatistical, stProduct.nStatus, stProduct.fOffSet,
		stProduct.fLon1, stProduct.fLon2, stProduct.fLat1, stProduct.fLat2, stProduct.fDi, stProduct.fDj,
		stProduct.nTimeRange, stProduct.nMaxForecastTime, stProduct.strBusinessStatus.data(), strWarning);
#else
	sprintf(strValue, "{\"Name\":\"%s\",\"Role\":\"%s\",\"Type\":\"%s\",\"Publisher\":\"%s\",\"Category\":\"%d\",\"Element\":\"%s\",\"Statistical\":\"%d\",\"Status\":\"%d\",\"OffSet\":\"%f\",\"Lon1\":\"%f\",\"Lon2\":\"%f\",\"Lat1\":\"%f\",\"Lat2\":\"%f\",\"Di\":\"%f\",\"Dj\":\"%f\",\"TimeRange\":\"%d\",\"MaxForecastTime\":\"%d\",\"BusinessStatus\":\"%s\",\"Warning\":\"%s\"}",
		stProduct.strName.toLocal8Bit().data(), stProduct.strRole.toLocal8Bit().data(), stProduct.strType.toLocal8Bit().data(), stProduct.strPublisher.toLocal8Bit().data(),
		stProduct.nCategory, strElement, stProduct.nStatistical, stProduct.nStatus, stProduct.fOffSet,
		stProduct.fLon1, stProduct.fLon2, stProduct.fLat1, stProduct.fLat2, stProduct.fDi, stProduct.fDj,
		stProduct.nTimeRange, stProduct.nMaxForecastTime, stProduct.strBusinessStatus.toLocal8Bit().data(), strWarning);
#endif

	// 4. 保存到缓存中
	bool bRtn = m_pCached->SetValue(strInfoKeyName, strValue, strlen(strValue));

	// 5. 解锁
	m_pCached->UnLockKey(strInfoKeyLocker);

	// 6. 返回结果
	if (!bRtn)
	{
		return CACHED_SAVEPRODUCTINFO_FAIL;
	}

	return SUCCESS;
}

/************************************************************************/
/* 保存信息公共方法                                                     */
/************************************************************************/
int CachedImpl::SaveInfo(const char * strInfoKeyLocker, const char * strInfoKeyName, const char * strInfoValue)
{
	// 1. 缓存对象判断
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	// 2. 加锁
	int nDelayCount = 0;
	do
	{
		// 加锁成功
		if (m_pCached->LockKey(strInfoKeyLocker))
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

	// 4. 保存到缓存中
	bool bRtn = m_pCached->SetValue(strInfoKeyName, strInfoValue, strlen(strInfoValue));

	// 5. 解锁
	m_pCached->UnLockKey(strInfoKeyLocker);

	// 6. 返回结果
	if (!bRtn)
	{
		return CACHED_SAVEPRODUCTINFO_FAIL;
	}

	return SUCCESS;
}

/************************************************************************/
/* 保存时间公共方法                                                     */
/************************************************************************/
int CachedImpl::SaveTime(const char * strTimeKeyLocker, const char * strTimeKeyName, const char * strTimeValue)
{
	// 1. 缓存对象判断
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	// 2. 加锁
	int nDelayCount = 0;
	do
	{
		// 加锁成功
		if (m_pCached->LockKey(strTimeKeyLocker))
		{
			break;
		}

		// 延时5毫秒等待处理
		delay(5);
	} while (nDelayCount++ < 100);

	// 等待500mse超时，返回
	if (nDelayCount >= 100)
	{
		// 加锁，等待超时返回
		return CACHED_LOCK_TIMEOUT;
	}

	// 3. 获取原数据
	string strValue;
	char *pValue = NULL;
	int  nValueSize = 0;
	pValue = m_pCached->GetValue(strTimeKeyName, nValueSize);
	if (pValue == NULL)
	{
		// 没有这个key(LastError=16)
		if (m_pCached->GetLastError() != MEMCACHE_KEY_NOT_FOUND)
		{
			// 异常，返回
			m_pCached->UnLockKey(strTimeKeyLocker);
			return CACHED_GETVALUE_FAIL;
		}

		// 3. 重新组装数据
		strValue = string(strTimeValue);
	}
	else
	{
		strValue = string(pValue);

		// 3. 查找是否已经存在
		if (FindValueFromString(strValue, strTimeValue))
		{
			// 3.1 已经存在，不需要再保存
			m_pCached->UnLockKey(strTimeKeyLocker);
			return SUCCESS;
		}

		// 3.2 数据不存在，重新组装数据(使用","分割)
		strValue = strValue + string(",") + string(strTimeValue);
	}

	// 4. 保存数据到memcached中
	bool bRet = m_pCached->SetValue(strTimeKeyName, strValue.data(), strValue.size());

	// 5. 解锁
	m_pCached->UnLockKey(strTimeKeyLocker);

#ifndef DEBUG
	// 释放申请的空间
	if (pValue)
	{
		free(pValue);
		pValue = NULL;
	}
#endif

	// 6. 返回结果
	if (!bRet)
	{
		return CACHED_SAVEPRODUCTDATA_FAIL;
	}
	return SUCCESS;
}

/************************************************************************/
/* 保存数据公共方法                                                     */
/************************************************************************/
int CachedImpl::SaveData(const char * strDataKeyLocker, const char * strDataKeyName, const char * strTimeValue, const char * szData, int nDataSize, int nExpirationTime)
{
	
	// 1. 缓存对象判断
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	// 2. 加锁
	int nDelayCount = 0;
	do
	{
		// 加锁成功
		if (m_pCached->LockKey(strDataKeyLocker))
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

	//  针对memcached中Slabs的内存限制为1M，因此对数据fData进行散列存储处理
	//  PRODUCT_MEM_DATA中存储数据总长度 nDataSize
	//  多个PRODUCT_MEM_HASH_DATA中分散存储数据，Key的个数有nDataSize和POWER_BLOCK决定

	// 3. 存储总长度
	char szDataSize[50];
	memset(szDataSize, 0x00, sizeof(szDataSize));
	sprintf(szDataSize, "%d", nDataSize);
	bool bRet = m_pCached->SetValue(strDataKeyName, (char *)szDataSize, strlen(szDataSize), nExpirationTime);
	if (!bRet)
	{
		// 存储失败，异常返回
		m_pCached->UnLockKey(strDataKeyLocker);
		return CACHED_SETVALUE_FAIL;
	}

	// 4. 存储散列数据
	char strProductDataHashKey[256];
	char  *pData = (char *)szData;   // 存储数据
	int nRemainLength = nDataSize;   // 剩余数据长度
	int nStorageLength = 0;          // 存储长度
	int nKeyCount = 0;               // 存储个数
	int nOff = 0;                    // 数据指针游标位置（起始位置为0）

	do
	{
		// 存储长度默认为最大值
		nStorageLength = POWER_BLOCK;

		// 剩余数据长度（剩余长度-存储长度）
		nRemainLength -= nStorageLength;
		if (nRemainLength < 0)
		{
			// 如果剩余长度不超过最大存储长度，则将剩余数据全部存储
			nStorageLength += nRemainLength;
		}

		// Key Key=ProductData_p1_20150515080000.012_Num
		nKeyCount++;
		sprintf(strProductDataHashKey, "%s_%d", strDataKeyName, nKeyCount);

		// 存储数据
		bRet = m_pCached->SetValue(strProductDataHashKey, pData + nOff, nStorageLength, nExpirationTime);
		if (!bRet)
		{
			// 如果有一次存储失败，则真个数据将不完整，存储即失败
			m_pCached->UnLockKey(strDataKeyLocker);
			return CACHED_SETVALUE_FAIL;
		}

		//  数据指针游标下次存储位置
		nOff += nStorageLength;

	} while (nRemainLength > 0); // 还有未存储的数据，继续操作

	// 5. 解锁
	m_pCached->UnLockKey(strDataKeyLocker);

	// 6. 返回结果
	return SUCCESS;
}

/************************************************************************/
/* 保存数据公共方法                                                     */
/************************************************************************/
int CachedImpl::SaveData(const char * strDataKeyLocker, const char * strDataKeyName, const char * strTimeValue, float * fData, int nDataSize, int nExpirationTime)
{

	// 1. 缓存对象判断
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	// 2. 加锁
	int nDelayCount = 0;
	do
	{
		// 加锁成功
		if (m_pCached->LockKey(strDataKeyLocker))
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

	//  针对memcached中Slabs的内存限制为1M，因此对数据fData进行散列存储处理
	//  PRODUCT_MEM_DATA中存储数据总长度 nDataSize
	//  多个PRODUCT_MEM_HASH_DATA中分散存储数据，Key的个数有nDataSize和POWER_BLOCK决定

	// 3. 存储总长度
	char szDataSize[50];
	memset(szDataSize, 0x00, sizeof(szDataSize));
	nDataSize = nDataSize * sizeof(float);  // 修改float数组长度为字符长度
	sprintf(szDataSize, "%d", nDataSize );
	bool bRet = m_pCached->SetValue(strDataKeyName, (char *)szDataSize, strlen(szDataSize), nExpirationTime);
	if (!bRet)
	{
		// 存储失败，异常返回
		m_pCached->UnLockKey(strDataKeyLocker);
		return CACHED_SETVALUE_FAIL;
	}

	// 4. 存储散列数据
	char strProductDataHashKey[256];
	char  *pData = (char *)fData;    // 存储数据
	int nRemainLength = nDataSize;   // 剩余数据长度
	int nStorageLength = 0;          // 存储长度
	int nKeyCount = 0;               // 存储个数
	int nOff = 0;                    // 数据指针游标位置（起始位置为0）

	do
	{
		// 存储长度默认为最大值
		nStorageLength = POWER_BLOCK;

		// 剩余数据长度（剩余长度-存储长度）
		nRemainLength -= nStorageLength;
		if (nRemainLength < 0)
		{
			// 如果剩余长度不超过最大存储长度，则将剩余数据全部存储
			nStorageLength += nRemainLength;
		}

		// Key Key=ProductData_p1_20150515080000.012_Num
		nKeyCount++;
		sprintf(strProductDataHashKey, "%s_%d", strDataKeyName, nKeyCount);

		// 存储数据
		bRet = m_pCached->SetValue(strProductDataHashKey, pData + nOff, nStorageLength, nExpirationTime);
		if (!bRet)
		{
			// 如果有一次存储失败，则真个数据将不完整，存储即失败
			m_pCached->UnLockKey(strDataKeyLocker);
			return CACHED_SETVALUE_FAIL;
		}

		//  数据指针游标下次存储位置
		nOff += nStorageLength;

	} while (nRemainLength > 0); // 还有未存储的数据，继续操作

	// 5. 解锁
	m_pCached->UnLockKey(strDataKeyLocker);

	// 6. 返回结果
	return SUCCESS;
}

/************************************************************************/
/* 保存数据公共方法-数据，数据长度，起止经纬度                          */
/************************************************************************/
int CachedImpl::SaveData(const char * strDataKeyLocker, const char * strDataKeyName, const char * strTimeValue, float * fData, int nDataSize, float fLon1, float fLon2, float fLat1, float fLat2, int nExpirationTime)
{

	// 1. 缓存对象判断
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	// 2. 加锁
	int nDelayCount = 0;
	do
	{
		// 加锁成功
		if (m_pCached->LockKey(strDataKeyLocker))
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

	//  针对memcached中Slabs的内存限制为1M，因此对数据fData进行散列存储处理
	//  PRODUCT_MEM_DATA中存储数据总长度 nDataSize
	//  多个PRODUCT_MEM_HASH_DATA中分散存储数据，Key的个数有nDataSize和POWER_BLOCK决定

	// 3. 存储总长度
	char szDataSize[300];
	memset(szDataSize, 0x00, sizeof(szDataSize));
	nDataSize = nDataSize * sizeof(float);  // 修改float数组长度为字符长度
	sprintf(szDataSize, "%d,%f,%f,%f,%f", nDataSize, fLon1, fLon2, fLat1, fLat2);
	bool bRet = m_pCached->SetValue(strDataKeyName, (char *)szDataSize, strlen(szDataSize), nExpirationTime);
	if (!bRet)
	{
		// 存储失败，异常返回
		m_pCached->UnLockKey(strDataKeyLocker);
		return CACHED_SETVALUE_FAIL;
	}

	// 4. 存储散列数据
	char strProductDataHashKey[256];
	char  *pData = (char *)fData;    // 存储数据
	int nRemainLength = nDataSize;   // 剩余数据长度
	int nStorageLength = 0;          // 存储长度
	int nKeyCount = 0;               // 存储个数
	int nOff = 0;                    // 数据指针游标位置（起始位置为0）

	do
	{
		// 存储长度默认为最大值
		nStorageLength = POWER_BLOCK;

		// 剩余数据长度（剩余长度-存储长度）
		nRemainLength -= nStorageLength;
		if (nRemainLength < 0)
		{
			// 如果剩余长度不超过最大存储长度，则将剩余数据全部存储
			nStorageLength += nRemainLength;
		}

		// Key Key=ProductData_p1_20150515080000.012_Num
		nKeyCount++;
		sprintf(strProductDataHashKey, "%s_%d", strDataKeyName, nKeyCount);

		// 存储数据
		bRet = m_pCached->SetValue(strProductDataHashKey, pData + nOff, nStorageLength, nExpirationTime);
		if (!bRet)
		{
			// 如果有一次存储失败，则真个数据将不完整，存储即失败
			m_pCached->UnLockKey(strDataKeyLocker);
			return CACHED_SETVALUE_FAIL;
		}

		//  数据指针游标下次存储位置
		nOff += nStorageLength;

	} while (nRemainLength > 0); // 还有未存储的数据，继续操作

	// 5. 解锁
	m_pCached->UnLockKey(strDataKeyLocker);

	// 6. 返回结果
	return SUCCESS;

}

/************************************************************************/
/* 保存站点数据公共方法                                                 */
/************************************************************************/
int CachedImpl::SaveData_s(const char * strDataKeyLocker, const char * strDataKeyName, const char * strTimeValue, float * fData, int nDataSize, int nExpirationTime)
{

	// 1. 缓存对象判断
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	// 2. 加锁
	int nDelayCount = 0;
	do
	{
		// 加锁成功
		if (m_pCached->LockKey(strDataKeyLocker))
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

	// 3. 存储数据
	nDataSize = nDataSize * sizeof(float);
	bool bRet = m_pCached->SetValue(strDataKeyName, (char *)fData, nDataSize, nExpirationTime);

	// 4. 解锁
	m_pCached->UnLockKey(strDataKeyLocker);

	// 5. 返回结果
	if (!bRet)
	{
		return CACHED_SETVALUE_FAIL;
	}

	return SUCCESS;
}

/************************************************************************/
/* 获取所有Key的方法                                                    */
/************************************************************************/
int CachedImpl::GetAllKey(const char * strRootKeyName, StringList &lstKeyName, bool bSortFlg)
{
	// 1. 缓存对象判断
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	// 2. 获取数据
	int  nValueSize = 0;
	char * pValue = m_pCached->GetValue(strRootKeyName, nValueSize);
	if (pValue == NULL)
	{
		if (m_pCached->GetLastError() != MEMCACHE_KEY_NOT_FOUND)
		{
			return CACHED_GETVALUE_FAIL;
		}
		else
		{
			return SUCCESS;
		}
	}

	// 3. 解析数据 
	string strValue = string(pValue);
	lstKeyName.clear();
	split(lstKeyName, strValue, ",");

	// 4. 排序
	if (bSortFlg)
	{
		// 不区分大小写
		lstKeyName.sort();
	}

#ifndef DEBUG
	// 释放申请的空间
	if (pValue)
	{
		free(pValue);
		pValue = NULL;
	}
#endif

	// 5. 返回结果
	return SUCCESS;
}

/************************************************************************/
/* 获取信息的方法                                                       */
/************************************************************************/
int CachedImpl::GetInfo(const char * strInfoKeyName, ST_PRODUCT &stProduct)
{
	// 1. 缓存对象判断
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	// 2. 获取数据
	int nValueSize = 0;
	char *pValue = m_pCached->GetValue(strInfoKeyName, nValueSize);
	if (pValue == NULL)
	{
		return CACHED_GETVALUE_FAIL;
	}

	// 3. JSON数据解析处理
	bool bRet = JSON::ParserProductInfo(pValue, stProduct);

#ifndef DEBUG
	// 释放申请的空间
	if (pValue)
	{
		free(pValue);
		pValue = NULL;
	}
#endif

	if (!bRet)
	{
		return CACHED_GETPRODUCTINFO_FAIL;
	}

	// 4. 返回成功结果
	return SUCCESS;
}

/************************************************************************/
/* 获取站点信息的方法                                                   */
/************************************************************************/
int CachedImpl::GetInfo(const char * strInfoKeyName, float &fLon, float &fLat, float &fHeight)
{
	// 1. 缓存对象判断
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	// 2. 获取数据
	int nValueSize = 0;
	char *pValue = m_pCached->GetValue(strInfoKeyName, nValueSize);
	if (pValue == NULL)
	{
		return CACHED_GETVALUE_FAIL;
	}

	// 3. JSON数据解析处理
	string strInfo = string(pValue);
	string::size_type nOff = 0;
	string separator = ",";
	string strData;
	strData = getdata(strInfo, separator, nOff);
	fLon = atof(strData.data());
	strData = getdata(strInfo, separator, nOff);
	fLat = atof(strData.data());
	strData = getdata(strInfo, separator, nOff);
	fHeight = atof(strData.data());
    // cols ..

#ifndef DEBUG
	// 释放申请的空间
	if (pValue)
	{
		free(pValue);
		pValue = NULL;
	}
#endif

	// 4. 返回成功结果
	return SUCCESS;
}

/************************************************************************/
/* 获取所有时间的方法                                                   */
/************************************************************************/
int CachedImpl::GetAllTime(const char * strTimeKeyName, StringList &lstTime, bool bSortFlg)
{
	// 1. 缓存对象判断
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	// 2. 获取数据
	int  nValueSize = 0;
	char * pValue = m_pCached->GetValue(strTimeKeyName, nValueSize);
	if (pValue == NULL)
	{
		if (m_pCached->GetLastError() != MEMCACHE_KEY_NOT_FOUND)
		{
			return CACHED_GETVALUE_FAIL;
		}
		else
		{
			return SUCCESS;
		}
	}

	// 3. 解析数据 
	string strValue = string(pValue);
	lstTime.clear();
	split(lstTime, strValue, ",");

	// 4. 排序
	if (bSortFlg)
	{
		// 不区分大小写
		lstTime.sort();
	}

#ifndef DEBUG
	// 释放申请的空间
	if (pValue)
	{
		free(pValue);
		pValue = NULL;
	}
#endif

	// 5. 返回结果
	return SUCCESS;
}

/************************************************************************/
/* 获取数据的方法                                                       */
/************************************************************************/
int CachedImpl::GetData(const char * strDataKeyName, char* &pData, int &nDataLen)
{
	// 1. 缓存对象判断
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	// 2. 获取数据总长度
	int nValueLen = 0;
	char * pValue = m_pCached->GetValue(strDataKeyName, nValueLen);
	if (pValue == NULL || nValueLen <= 0)
	{
		if (m_pCached->GetLastError() != MEMCACHE_KEY_NOT_FOUND)
		{
			return CACHED_GETVALUE_FAIL;
		}
		else
		{
			return SUCCESS;
		}
	}
	nDataLen = atoi(pValue);

#ifndef DEBUG
	// 释放申请的空间
	if (pValue)
	{
		free(pValue);
		pValue = NULL;
	}
#endif

	// 3. 创建数据存储空间
	pData = (char *)malloc(nDataLen+1);
	if (pData == NULL)
	{
		return ERR_MEM_MALLOC;
	}

	// 4. 计算存储个数
	int nKeyCount = (int)(nDataLen / POWER_BLOCK);
	if ((nDataLen % POWER_BLOCK) > 0)
	{
		nKeyCount += 1;
	}

	char strProductDataHashKey[256];
	int nOff = 0;  // 数据指针游标位置（起始位置为0）

	// 5. 获取全部数据
	for (int i = 0; i < nKeyCount; i++)
	{
		// key
		sprintf(strProductDataHashKey, "%s_%d", strDataKeyName, i + 1);
		pValue = m_pCached->GetValue(strProductDataHashKey, nValueLen);
		if (pValue == NULL || nValueLen <= 0)
		{
			// 获取失败一次，则整个数据将不完整
			return CACHED_GETVALUE_FAIL;
		}

		//  数据拷贝
		memcpy((char *)pData + nOff, pValue, nValueLen);

		// 游标指向下次存储的位置
		nOff += nValueLen;

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
	pData[nOff] = '\0';

	// 7. 返回成功
	return SUCCESS;
}

/************************************************************************/
/* 获取数据的方法                                                       */
/************************************************************************/
int CachedImpl::GetData(const char * strDataKeyName, float* &fData, int &nDataLen)
{
	// 1. 缓存对象判断
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	// 2. 获取数据总长度
	int nValueLen = 0;
	char * pValue = m_pCached->GetValue(strDataKeyName, nValueLen);
	if (pValue == NULL || nValueLen <= 0)
	{
		if (m_pCached->GetLastError() != MEMCACHE_KEY_NOT_FOUND)
		{
			return CACHED_GETVALUE_FAIL;
		}
		else
		{
			return SUCCESS;
		}
	}
	nDataLen = atoi(pValue);

#ifndef DEBUG
	// 释放申请的空间
	if (pValue)
	{
		free(pValue);
		pValue = NULL;
	}
#endif

	// 3. 创建数据存储空间
	fData = (float *)malloc(nDataLen);
	if (fData == NULL)
	{
		return ERR_MEM_MALLOC;
	}

	// 4. 计算存储个数
	int nKeyCount = (int)(nDataLen / POWER_BLOCK);
	if ((nDataLen % POWER_BLOCK) > 0)
	{
		nKeyCount += 1;
	}

	char strProductDataHashKey[256];
	int nOff = 0;  // 数据指针游标位置（起始位置为0）

	// 5. 获取全部数据
	for (int i = 0; i < nKeyCount; i++)
	{
		// key
		sprintf(strProductDataHashKey, "%s_%d", strDataKeyName, i + 1);
		pValue = m_pCached->GetValue(strProductDataHashKey, nValueLen);
		if (pValue == NULL || nValueLen <= 0)
		{
			// 获取失败一次，则整个数据将不完整
			return CACHED_GETVALUE_FAIL;
		}

		//  数据拷贝
		memcpy((char *)fData + nOff, pValue, nValueLen);

		// 游标指向下次存储的位置
		nOff += nValueLen;

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

	// 6. 长度转换为float长度
	nDataLen = nDataLen / sizeof(float);

	// 7. 返回成功
	return SUCCESS;
}

/************************************************************************/
/* 获取数据的方法 -返回数据，数据长度，起止经纬度                       */
/************************************************************************/
int CachedImpl::GetData(const char * strDataKeyName, float* &fData, int &nDataLen, float &fLon1, float &fLon2, float &fLat1, float &fLat2)
{
	// 1. 缓存对象判断
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	// 2. 获取数据总长度
	int nValueLen = 0;
	char * pValue = m_pCached->GetValue(strDataKeyName, nValueLen);
	if (pValue == NULL || nValueLen <= 0)
	{
		if (m_pCached->GetLastError() != MEMCACHE_KEY_NOT_FOUND)
		{
			return CACHED_GETVALUE_FAIL;
		}
		else
		{
			return SUCCESS;
		}
	}
	// 2.1 对获取数据进行解析 DataLen,Lon1,Lon2,Lat1,Lat2
	string strValue = string(pValue);
	string strData;
	string::size_type nDataOff = 0; 
	string separator = ",";
	// 数据长度
	strData = getdata(strValue, separator, nDataOff);
	nDataLen = atoi(strData.data());
	// 起始经度
	strData = getdata(strValue, separator, nDataOff);
	fLon1 = atof(strData.data());
	// 终止经度
	strData = getdata(strValue, separator, nDataOff);
	fLon2 = atof(strData.data());
	// 起始纬度
	strData = getdata(strValue, separator, nDataOff);
	fLat1 = atof(strData.data());
	// 终止纬度
	strData = getdata(strValue, separator, nDataOff);
	fLat2 = atof(strData.data());

#ifndef DEBUG
	// 释放申请的空间
	if (pValue)
	{
		free(pValue);
		pValue = NULL;
	}
#endif

	// 3. 创建数据存储空间
	fData = (float *)malloc(nDataLen);
	if (fData == NULL)
	{
		return ERR_MEM_MALLOC;
	}

	// 4. 计算存储个数
	int nKeyCount = (int)(nDataLen / POWER_BLOCK);
	if ((nDataLen % POWER_BLOCK) > 0)
	{
		nKeyCount += 1;
	}

	char strProductDataHashKey[256];
	int nOff = 0;  // 数据指针游标位置（起始位置为0）

	// 5. 获取全部数据
	for (int i = 0; i < nKeyCount; i++)
	{
		// key
		sprintf(strProductDataHashKey, "%s_%d", strDataKeyName, i + 1);
		pValue = m_pCached->GetValue(strProductDataHashKey, nValueLen);
		if (pValue == NULL || nValueLen <= 0)
		{
			// 获取失败一次，则整个数据将不完整
			return CACHED_GETVALUE_FAIL;
		}

		//  数据拷贝
		memcpy((char *)fData + nOff, pValue, nValueLen);

		// 游标指向下次存储的位置
		nOff += nValueLen;

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

	// 6. 长度转换为float长度
	nDataLen = nDataLen / sizeof(float);

	// 7. 返回成功
	return SUCCESS;
}

/************************************************************************/
/* 获取站点数据公共方法                                                    */
/************************************************************************/
int CachedImpl::GetData_s(const char * strDataKeyName, float* &fData, int &nDataLen)
{
	// 1. 缓存对象判断
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	// 2. 获取数据
	int nValueLen;
	char * pValue = m_pCached->GetValue(strDataKeyName, nValueLen);
	if (pValue == NULL || nValueLen <= 0)
	{
		return CACHED_GETVALUE_FAIL;
	}

	// 创建空间
	fData = (float *)malloc(nValueLen);
	if (fData == NULL)
	{
#ifndef DEBUG
		// 释放申请的空间
		if (pValue)
		{
			free(pValue);
			pValue = NULL;
		}
#endif
		return ERR_MEM_MALLOC;
	}

	//  数据拷贝
	memcpy((char *)fData, pValue, nValueLen);
	//fData = (float *)pValue;
	nDataLen = nValueLen / sizeof(float); // 数据长度

	// todo add zhangl 
#ifndef DEBUG
	// 释放申请的空间
	if (pValue)
	{
		free(pValue);
		pValue = NULL;
	}
#endif

	return SUCCESS;
}

/************************************************************************/
/* 删除Key的公共方法                                                    */
/************************************************************************/
int CachedImpl::DeleteKey(const char * strRootKeyLocker, const char * strRootKeyName, const char * strValueKeyName)
{
	// 1. 缓存对象判断
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	// 2. 加锁
	int nDelayCount = 0;
	do
	{
		// 加锁成功
		if (m_pCached->LockKey(strRootKeyLocker))
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

	// 3. 获取原数据
	string strValue;
	char *pValue = NULL;
	int  nValueSize = 0;
	pValue = m_pCached->GetValue(strRootKeyName, nValueSize);
	if (pValue == NULL)
	{
		// 解锁
		m_pCached->UnLockKey(strRootKeyLocker);

		// 没有这个key(LastError=16)
		if (m_pCached->GetLastError() == MEMCACHE_KEY_NOT_FOUND)
		{
			// 没有此Key，正常返回
			return SUCCESS;
		}
		else
		{
			// 异常，返回
			return CACHED_GETVALUE_FAIL;
		}
	}

	// 4. 从字符串中删除该数据后，重新组装数据
	strValue = string(pValue);
	strValue = DeleteValueFromString(strValue, strValueKeyName);

#ifndef DEBUG
	// 释放申请的空间
	if (pValue)
	{
		free(pValue);
		pValue = NULL;
	}
#endif

	// 5. 将新数据重新保存到ProductTime_PK 中
	bool bRet;
	if (strValue == "")
	{
		// 如果已经没有数据了，则删除此Key（注：无法写入一个空字符串到一个Key中）
		bRet = m_pCached->DelKey(strRootKeyName);
		if (!bRet)
		{
			// 忽略因为Key不存在而导致的失败
			if (m_pCached->GetLastError() == MEMCACHE_KEY_NOT_FOUND)
			{
				bRet = true;
			}
		}
	}
	else
	{
		// 重新写回
		bRet = m_pCached->SetValue(strRootKeyName, strValue.data(), strValue.size());
	}

	// 6. 解锁
	m_pCached->UnLockKey(strRootKeyLocker);

	// 7. 返回结果
	if (!bRet)
	{
		return CACHED_DELVALUE_FAIL;
	}

	return SUCCESS;
}

/************************************************************************/
/* 删除信息的公共方法                                                   */
/************************************************************************/
int CachedImpl::DeleteInfo(const char * strInfoKeyLocker, const char * strInfoKeyName)
{
	// 1. 缓存对象判断
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	// 2. 加锁
	int nDelayCount = 0;
	do
	{
		// 加锁成功
		if (m_pCached->LockKey(strInfoKeyLocker))
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

	// 3. 删除该key
	bool bRet = true;
	if (!m_pCached->DelKey(strInfoKeyName))
	{
		// 删除失败
		if (m_pCached->GetLastError() != MEMCACHE_KEY_NOT_FOUND)
		{
			bRet = false;
		}
	}

	// 4. 解锁
	m_pCached->UnLockKey(strInfoKeyLocker);

	// 5. 返回结果
	if (!bRet)
	{
		return CACHED_DELVALUE_FAIL;
	}
	return SUCCESS;
}

/************************************************************************/
/* 删除某时间的公共方法                                                 */
/************************************************************************/
int CachedImpl::DeleteTime(const char * strTimeKeyLocker, const char * strTimeKeyName, const char * strTimeValue)
{
	// 1. 缓存对象判断
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	// 2. 加锁
	int nDelayCount = 0;
	do
	{
		// 加锁成功
		if (m_pCached->LockKey(strTimeKeyLocker))
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

	// 3. 获取原数据
	string strValue;
	char *pValue = NULL;
	int  nValueSize = 0;
	pValue = m_pCached->GetValue(strTimeKeyName, nValueSize);
	if (pValue == NULL)
	{
		// 解锁
		m_pCached->UnLockKey(strTimeKeyLocker);

		// 没有这个key(LastError=16)
		if (m_pCached->GetLastError() == MEMCACHE_KEY_NOT_FOUND)
		{
			// 没有此Key，正常返回
			return SUCCESS;
		}
		else
		{
			// 异常，返回
			return CACHED_GETVALUE_FAIL;
		}
	}

	// 4. 从字符串中删除该数据后，重新组装数据
	strValue = string(pValue);
	strValue = DeleteValueFromString(strValue, strTimeValue);

#ifndef DEBUG
	// 释放申请的空间
	if (pValue)
	{
		free(pValue);
		pValue = NULL;
	}
#endif

	// 5. 将新数据重新保存到ProductTime_PK 中
	bool bRet;
	if (strValue == "")
	{
		// 如果已经没有数据了，则删除此Key（注：无法写入一个空字符串到一个Key中）
		bRet = m_pCached->DelKey(strTimeKeyName);
		if (!bRet)
		{
			// 忽略因为Key不存在而导致的失败
			if (m_pCached->GetLastError() == MEMCACHE_KEY_NOT_FOUND)
			{
				bRet = true;
			}
		}
	}
	else
	{
		// 重新写回
		bRet = m_pCached->SetValue(strTimeKeyName, strValue.data(), strValue.size());
	}

	// 6. 解锁
	m_pCached->UnLockKey(strTimeKeyLocker);

	// 7. 返回结果
	if (!bRet)
	{
		return CACHED_DELVALUE_FAIL;
	}

	return SUCCESS;
}

/************************************************************************/
/* 删除数据的公共方法                                                   */
/************************************************************************/
int CachedImpl::DeleteData(const char * strDataKeyLocker, const char * strDataKeyName)
{
	// 1. 缓存对象判断
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	// 2. 加锁
	int nDelayCount = 0;
	do
	{
		// 加锁成功
		if (m_pCached->LockKey(strDataKeyLocker))
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

	// 3. 获取数据总长度
	int nDataLen = 0;
	bool bRet = true;
	char *pValue = m_pCached->GetValue(strDataKeyName, nDataLen);
	if (pValue == NULL || nDataLen <= 0)
	{
		// 解锁
		m_pCached->UnLockKey(strDataKeyLocker);

		// 忽略因为Key不存在而导致的失败
		if (m_pCached->GetLastError() == MEMCACHE_KEY_NOT_FOUND)
		{
			// 表示该Key不存在，数据已经清除
			return SUCCESS;
		}
		else
		{
			return CACHED_DELVALUE_FAIL;
		}
	}
	string strValue = string(pValue);
	string strData;
	string::size_type nOff = 0;
	string separator = ",";
	// 数据长度
	strData = getdata(strValue, separator, nOff);
	nDataLen = atoi(strData.data());

#ifndef DEBUG
	// 释放申请的空间
	if (pValue)
	{
		free(pValue);
		pValue = NULL;
	}
#endif

	// 4. 计算存储个数
	int nKeyCount = (int)(nDataLen / POWER_BLOCK);
	if ((nDataLen % POWER_BLOCK) > 0)
	{
		nKeyCount += 1;
	}

	// 5. 遍历删除所有散列数据
	char strProductDataHashKey[256];
	for (int i = 0; i < nKeyCount; i++)
	{
		// key
		sprintf(strProductDataHashKey, "%s_%d", strDataKeyName, i + 1);
		// 删除key
		bRet = m_pCached->DelKey(strProductDataHashKey);
		if (!bRet)
		{
			// 忽略因为Key不存在而导致的失败
			// 删除失败一次，则整个删除失败
			if (m_pCached->GetLastError() != MEMCACHE_KEY_NOT_FOUND)
			{
				// 解锁
				m_pCached->UnLockKey(strDataKeyLocker);
				// 返回
				return CACHED_DELVALUE_FAIL;
			}
		}
	}

	// 6. 删除存储总长度的Key
	bRet = m_pCached->DelKey(strDataKeyName);
	if (!bRet)
	{
		// 忽略因为Key不存在而导致的失败
		if (m_pCached->GetLastError() != MEMCACHE_KEY_NOT_FOUND)
		{
			// 解锁
			m_pCached->UnLockKey(strDataKeyLocker);
			// 返回结果
			return CACHED_DELVALUE_FAIL;
		}
	}

	// 7. 解锁
	m_pCached->UnLockKey(strDataKeyLocker);

	// 8. 返回结果
	return SUCCESS;
}

/************************************************************************/
/* 删除站点数据的公共方法                                               */
/************************************************************************/
int CachedImpl::DeleteData_s(const char * strDataKeyLocker, const char * strDataKeyName)
{
	// 1. 缓存对象判断
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	// 2. 加锁
	int nDelayCount = 0;
	do
	{
		// 加锁成功
		if (m_pCached->LockKey(strDataKeyLocker))
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

	// 3. 删除数据
	bool bRet = m_pCached->DelKey(strDataKeyName);
	if (!bRet)
	{
		// 忽略因为Key不存在而导致的失败
		if (m_pCached->GetLastError() != MEMCACHE_KEY_NOT_FOUND)
		{
			// 解锁
			m_pCached->UnLockKey(strDataKeyLocker);
			// 返回结果
			return CACHED_DELVALUE_FAIL;
		}
	}

	// 4. 解锁
	m_pCached->UnLockKey(strDataKeyLocker);

	// 5. 返回结果
	return SUCCESS;
}

/************************************************************************/
/* 从获取的数据字符串中判断某KeyValue是否存在                           */
/************************************************************************/
bool CachedImpl::FindValueFromString(string strSrc, string strValue)
{
	bool bFindFlg = false;

	StringList lstSrc;
	split(lstSrc, strSrc, ",");
	StringList::iterator iter;
	for (iter = lstSrc.begin(); iter != lstSrc.end(); iter++)
	{
		string strValueTemp = *iter;
		if (strValueTemp == strValue)
		{
			bFindFlg = true;
			break;
		}
	}

	return bFindFlg;
}

/************************************************************************/
/* 从获取的数据字符串中删除某KeyValue后返回重新组装后的字符串           */
/************************************************************************/
string CachedImpl::DeleteValueFromString(string strSrc, string strValue)
{
	StringList lstSrc;
	split(lstSrc, strSrc, ",");
	if (lstSrc.size() < 1)
	{
		return strSrc;
	}

	//  删除数据
	lstSrc.remove(strValue);

	// 重新合成数据
	string strRetValue = "";

	StringList::iterator iter;
	for (iter = lstSrc.begin(); iter != lstSrc.end(); iter++)
	{
		string strValueTemp = *iter;
		
		if (strRetValue == "")
		{
			strRetValue = strValueTemp;
		}
		else
		{
			strRetValue = strRetValue + "," + strValueTemp;
		}
	}

	return strRetValue;
}
