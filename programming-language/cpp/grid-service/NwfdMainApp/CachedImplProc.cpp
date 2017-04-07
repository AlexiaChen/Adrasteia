#include "CachedImpl.h"
#include "Common.h"
#include "JSON.h"

/************************************************************************/
/* ����KEY��������                                                      */
/************************************************************************/
int CachedImpl::SaveKey(const char * strRootKeyLocker, const char * strRootKeyName, const char * strValueKeyName)
{
	// 1. ��������ж�
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	// 2. ����
	int nDelayCount = 0;
	do
	{
		// �����ɹ�
		if (m_pCached->LockKey(strRootKeyLocker))
		{
			break;
		}

		// ��ʱ5����ȴ�����
		delay(5);
	} while (nDelayCount++ < 100);

	if (nDelayCount >= 100)
	{
		// �������ȴ���ʱ����
		return CACHED_LOCK_TIMEOUT;
	}

	// 3. ��ȡԭROOT�е�����
	string strValue;
	char *pValue = NULL;
	int  nValueSize = 0;
	pValue = m_pCached->GetValue(strRootKeyName, nValueSize);
	if (pValue == NULL)
	{
		// û�����key(LastError=16)
		if (m_pCached->GetLastError() == MEMCACHE_KEY_NOT_FOUND)
		{
			// 3. ������װ����
			strValue = string(strValueKeyName);
		}
		else
		{
			// �쳣������
			m_pCached->UnLockKey(strRootKeyLocker);
			return CACHED_GETVALUE_FAIL;
		}
	}
	else
	{
		strValue = string(pValue);

		// 3. �����Ƿ��Ѿ�����
		if (FindValueFromString(strValue, strValueKeyName))
		{
			// 3.1 �Ѿ����ڣ�����Ҫ�ٱ���
			m_pCached->UnLockKey(strRootKeyLocker);
			return SUCCESS;
		}

		// 3.2 ���ݲ����ڣ�������װ����(ʹ��","�ָ�)
		strValue = strValue + string(",") + string(strValueKeyName);
	}

	// 4. �������ݵ�memcached��
	bool bRet = m_pCached->SetValue(strRootKeyName, strValue.data(), strValue.size());

	// 5. ����
	m_pCached->UnLockKey(strRootKeyLocker);

#ifndef DEBUG
	// �ͷ�����Ŀռ�
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
/* ������Ϣ��������                                                     */
/************************************************************************/
int CachedImpl::SaveInfo(const char * strInfoKeyLocker, const char * strInfoKeyName, ST_PRODUCT stProduct)
{
	// 1. ��������ж�
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	// 2. ����
	int nDelayCount = 0;
	do
	{
		// �����ɹ�
		if (m_pCached->LockKey(strInfoKeyLocker))
		{
			break;
		}

		// ��ʱ5����ȴ�����
		delay(5);
	} while (nDelayCount++ < 100);

	if (nDelayCount >= 100)
	{
		// �������ȴ���ʱ����
		return CACHED_LOCK_TIMEOUT;
	}

	// 3. ƴ��Value��Ϣ
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

	// ��Ʒ��Ϣ��ʽʹ��JSON��ʽ
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

	// 4. ���浽������
	bool bRtn = m_pCached->SetValue(strInfoKeyName, strValue, strlen(strValue));

	// 5. ����
	m_pCached->UnLockKey(strInfoKeyLocker);

	// 6. ���ؽ��
	if (!bRtn)
	{
		return CACHED_SAVEPRODUCTINFO_FAIL;
	}

	return SUCCESS;
}

/************************************************************************/
/* ������Ϣ��������                                                     */
/************************************************************************/
int CachedImpl::SaveInfo(const char * strInfoKeyLocker, const char * strInfoKeyName, const char * strInfoValue)
{
	// 1. ��������ж�
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	// 2. ����
	int nDelayCount = 0;
	do
	{
		// �����ɹ�
		if (m_pCached->LockKey(strInfoKeyLocker))
		{
			break;
		}

		// ��ʱ5����ȴ�����
		delay(5);
	} while (nDelayCount++ < 100);

	if (nDelayCount >= 100)
	{
		// �������ȴ���ʱ����
		return CACHED_LOCK_TIMEOUT;
	}

	// 4. ���浽������
	bool bRtn = m_pCached->SetValue(strInfoKeyName, strInfoValue, strlen(strInfoValue));

	// 5. ����
	m_pCached->UnLockKey(strInfoKeyLocker);

	// 6. ���ؽ��
	if (!bRtn)
	{
		return CACHED_SAVEPRODUCTINFO_FAIL;
	}

	return SUCCESS;
}

/************************************************************************/
/* ����ʱ�乫������                                                     */
/************************************************************************/
int CachedImpl::SaveTime(const char * strTimeKeyLocker, const char * strTimeKeyName, const char * strTimeValue)
{
	// 1. ��������ж�
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	// 2. ����
	int nDelayCount = 0;
	do
	{
		// �����ɹ�
		if (m_pCached->LockKey(strTimeKeyLocker))
		{
			break;
		}

		// ��ʱ5����ȴ�����
		delay(5);
	} while (nDelayCount++ < 100);

	// �ȴ�500mse��ʱ������
	if (nDelayCount >= 100)
	{
		// �������ȴ���ʱ����
		return CACHED_LOCK_TIMEOUT;
	}

	// 3. ��ȡԭ����
	string strValue;
	char *pValue = NULL;
	int  nValueSize = 0;
	pValue = m_pCached->GetValue(strTimeKeyName, nValueSize);
	if (pValue == NULL)
	{
		// û�����key(LastError=16)
		if (m_pCached->GetLastError() != MEMCACHE_KEY_NOT_FOUND)
		{
			// �쳣������
			m_pCached->UnLockKey(strTimeKeyLocker);
			return CACHED_GETVALUE_FAIL;
		}

		// 3. ������װ����
		strValue = string(strTimeValue);
	}
	else
	{
		strValue = string(pValue);

		// 3. �����Ƿ��Ѿ�����
		if (FindValueFromString(strValue, strTimeValue))
		{
			// 3.1 �Ѿ����ڣ�����Ҫ�ٱ���
			m_pCached->UnLockKey(strTimeKeyLocker);
			return SUCCESS;
		}

		// 3.2 ���ݲ����ڣ�������װ����(ʹ��","�ָ�)
		strValue = strValue + string(",") + string(strTimeValue);
	}

	// 4. �������ݵ�memcached��
	bool bRet = m_pCached->SetValue(strTimeKeyName, strValue.data(), strValue.size());

	// 5. ����
	m_pCached->UnLockKey(strTimeKeyLocker);

#ifndef DEBUG
	// �ͷ�����Ŀռ�
	if (pValue)
	{
		free(pValue);
		pValue = NULL;
	}
#endif

	// 6. ���ؽ��
	if (!bRet)
	{
		return CACHED_SAVEPRODUCTDATA_FAIL;
	}
	return SUCCESS;
}

/************************************************************************/
/* �������ݹ�������                                                     */
/************************************************************************/
int CachedImpl::SaveData(const char * strDataKeyLocker, const char * strDataKeyName, const char * strTimeValue, const char * szData, int nDataSize, int nExpirationTime)
{
	
	// 1. ��������ж�
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	// 2. ����
	int nDelayCount = 0;
	do
	{
		// �����ɹ�
		if (m_pCached->LockKey(strDataKeyLocker))
		{
			break;
		}

		// ��ʱ5����ȴ�����
		delay(5);
	} while (nDelayCount++ < 100);

	if (nDelayCount >= 100)
	{
		// �������ȴ���ʱ����
		return CACHED_LOCK_TIMEOUT;
	}

	//  ���memcached��Slabs���ڴ�����Ϊ1M����˶�����fData����ɢ�д洢����
	//  PRODUCT_MEM_DATA�д洢�����ܳ��� nDataSize
	//  ���PRODUCT_MEM_HASH_DATA�з�ɢ�洢���ݣ�Key�ĸ�����nDataSize��POWER_BLOCK����

	// 3. �洢�ܳ���
	char szDataSize[50];
	memset(szDataSize, 0x00, sizeof(szDataSize));
	sprintf(szDataSize, "%d", nDataSize);
	bool bRet = m_pCached->SetValue(strDataKeyName, (char *)szDataSize, strlen(szDataSize), nExpirationTime);
	if (!bRet)
	{
		// �洢ʧ�ܣ��쳣����
		m_pCached->UnLockKey(strDataKeyLocker);
		return CACHED_SETVALUE_FAIL;
	}

	// 4. �洢ɢ������
	char strProductDataHashKey[256];
	char  *pData = (char *)szData;   // �洢����
	int nRemainLength = nDataSize;   // ʣ�����ݳ���
	int nStorageLength = 0;          // �洢����
	int nKeyCount = 0;               // �洢����
	int nOff = 0;                    // ����ָ���α�λ�ã���ʼλ��Ϊ0��

	do
	{
		// �洢����Ĭ��Ϊ���ֵ
		nStorageLength = POWER_BLOCK;

		// ʣ�����ݳ��ȣ�ʣ�೤��-�洢���ȣ�
		nRemainLength -= nStorageLength;
		if (nRemainLength < 0)
		{
			// ���ʣ�೤�Ȳ��������洢���ȣ���ʣ������ȫ���洢
			nStorageLength += nRemainLength;
		}

		// Key Key=ProductData_p1_20150515080000.012_Num
		nKeyCount++;
		sprintf(strProductDataHashKey, "%s_%d", strDataKeyName, nKeyCount);

		// �洢����
		bRet = m_pCached->SetValue(strProductDataHashKey, pData + nOff, nStorageLength, nExpirationTime);
		if (!bRet)
		{
			// �����һ�δ洢ʧ�ܣ���������ݽ����������洢��ʧ��
			m_pCached->UnLockKey(strDataKeyLocker);
			return CACHED_SETVALUE_FAIL;
		}

		//  ����ָ���α��´δ洢λ��
		nOff += nStorageLength;

	} while (nRemainLength > 0); // ����δ�洢�����ݣ���������

	// 5. ����
	m_pCached->UnLockKey(strDataKeyLocker);

	// 6. ���ؽ��
	return SUCCESS;
}

/************************************************************************/
/* �������ݹ�������                                                     */
/************************************************************************/
int CachedImpl::SaveData(const char * strDataKeyLocker, const char * strDataKeyName, const char * strTimeValue, float * fData, int nDataSize, int nExpirationTime)
{

	// 1. ��������ж�
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	// 2. ����
	int nDelayCount = 0;
	do
	{
		// �����ɹ�
		if (m_pCached->LockKey(strDataKeyLocker))
		{
			break;
		}

		// ��ʱ5����ȴ�����
		delay(5);
	} while (nDelayCount++ < 100);

	if (nDelayCount >= 100)
	{
		// �������ȴ���ʱ����
		return CACHED_LOCK_TIMEOUT;
	}

	//  ���memcached��Slabs���ڴ�����Ϊ1M����˶�����fData����ɢ�д洢����
	//  PRODUCT_MEM_DATA�д洢�����ܳ��� nDataSize
	//  ���PRODUCT_MEM_HASH_DATA�з�ɢ�洢���ݣ�Key�ĸ�����nDataSize��POWER_BLOCK����

	// 3. �洢�ܳ���
	char szDataSize[50];
	memset(szDataSize, 0x00, sizeof(szDataSize));
	nDataSize = nDataSize * sizeof(float);  // �޸�float���鳤��Ϊ�ַ�����
	sprintf(szDataSize, "%d", nDataSize );
	bool bRet = m_pCached->SetValue(strDataKeyName, (char *)szDataSize, strlen(szDataSize), nExpirationTime);
	if (!bRet)
	{
		// �洢ʧ�ܣ��쳣����
		m_pCached->UnLockKey(strDataKeyLocker);
		return CACHED_SETVALUE_FAIL;
	}

	// 4. �洢ɢ������
	char strProductDataHashKey[256];
	char  *pData = (char *)fData;    // �洢����
	int nRemainLength = nDataSize;   // ʣ�����ݳ���
	int nStorageLength = 0;          // �洢����
	int nKeyCount = 0;               // �洢����
	int nOff = 0;                    // ����ָ���α�λ�ã���ʼλ��Ϊ0��

	do
	{
		// �洢����Ĭ��Ϊ���ֵ
		nStorageLength = POWER_BLOCK;

		// ʣ�����ݳ��ȣ�ʣ�೤��-�洢���ȣ�
		nRemainLength -= nStorageLength;
		if (nRemainLength < 0)
		{
			// ���ʣ�೤�Ȳ��������洢���ȣ���ʣ������ȫ���洢
			nStorageLength += nRemainLength;
		}

		// Key Key=ProductData_p1_20150515080000.012_Num
		nKeyCount++;
		sprintf(strProductDataHashKey, "%s_%d", strDataKeyName, nKeyCount);

		// �洢����
		bRet = m_pCached->SetValue(strProductDataHashKey, pData + nOff, nStorageLength, nExpirationTime);
		if (!bRet)
		{
			// �����һ�δ洢ʧ�ܣ���������ݽ����������洢��ʧ��
			m_pCached->UnLockKey(strDataKeyLocker);
			return CACHED_SETVALUE_FAIL;
		}

		//  ����ָ���α��´δ洢λ��
		nOff += nStorageLength;

	} while (nRemainLength > 0); // ����δ�洢�����ݣ���������

	// 5. ����
	m_pCached->UnLockKey(strDataKeyLocker);

	// 6. ���ؽ��
	return SUCCESS;
}

/************************************************************************/
/* �������ݹ�������-���ݣ����ݳ��ȣ���ֹ��γ��                          */
/************************************************************************/
int CachedImpl::SaveData(const char * strDataKeyLocker, const char * strDataKeyName, const char * strTimeValue, float * fData, int nDataSize, float fLon1, float fLon2, float fLat1, float fLat2, int nExpirationTime)
{

	// 1. ��������ж�
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	// 2. ����
	int nDelayCount = 0;
	do
	{
		// �����ɹ�
		if (m_pCached->LockKey(strDataKeyLocker))
		{
			break;
		}

		// ��ʱ5����ȴ�����
		delay(5);
	} while (nDelayCount++ < 100);

	if (nDelayCount >= 100)
	{
		// �������ȴ���ʱ����
		return CACHED_LOCK_TIMEOUT;
	}

	//  ���memcached��Slabs���ڴ�����Ϊ1M����˶�����fData����ɢ�д洢����
	//  PRODUCT_MEM_DATA�д洢�����ܳ��� nDataSize
	//  ���PRODUCT_MEM_HASH_DATA�з�ɢ�洢���ݣ�Key�ĸ�����nDataSize��POWER_BLOCK����

	// 3. �洢�ܳ���
	char szDataSize[300];
	memset(szDataSize, 0x00, sizeof(szDataSize));
	nDataSize = nDataSize * sizeof(float);  // �޸�float���鳤��Ϊ�ַ�����
	sprintf(szDataSize, "%d,%f,%f,%f,%f", nDataSize, fLon1, fLon2, fLat1, fLat2);
	bool bRet = m_pCached->SetValue(strDataKeyName, (char *)szDataSize, strlen(szDataSize), nExpirationTime);
	if (!bRet)
	{
		// �洢ʧ�ܣ��쳣����
		m_pCached->UnLockKey(strDataKeyLocker);
		return CACHED_SETVALUE_FAIL;
	}

	// 4. �洢ɢ������
	char strProductDataHashKey[256];
	char  *pData = (char *)fData;    // �洢����
	int nRemainLength = nDataSize;   // ʣ�����ݳ���
	int nStorageLength = 0;          // �洢����
	int nKeyCount = 0;               // �洢����
	int nOff = 0;                    // ����ָ���α�λ�ã���ʼλ��Ϊ0��

	do
	{
		// �洢����Ĭ��Ϊ���ֵ
		nStorageLength = POWER_BLOCK;

		// ʣ�����ݳ��ȣ�ʣ�೤��-�洢���ȣ�
		nRemainLength -= nStorageLength;
		if (nRemainLength < 0)
		{
			// ���ʣ�೤�Ȳ��������洢���ȣ���ʣ������ȫ���洢
			nStorageLength += nRemainLength;
		}

		// Key Key=ProductData_p1_20150515080000.012_Num
		nKeyCount++;
		sprintf(strProductDataHashKey, "%s_%d", strDataKeyName, nKeyCount);

		// �洢����
		bRet = m_pCached->SetValue(strProductDataHashKey, pData + nOff, nStorageLength, nExpirationTime);
		if (!bRet)
		{
			// �����һ�δ洢ʧ�ܣ���������ݽ����������洢��ʧ��
			m_pCached->UnLockKey(strDataKeyLocker);
			return CACHED_SETVALUE_FAIL;
		}

		//  ����ָ���α��´δ洢λ��
		nOff += nStorageLength;

	} while (nRemainLength > 0); // ����δ�洢�����ݣ���������

	// 5. ����
	m_pCached->UnLockKey(strDataKeyLocker);

	// 6. ���ؽ��
	return SUCCESS;

}

/************************************************************************/
/* ����վ�����ݹ�������                                                 */
/************************************************************************/
int CachedImpl::SaveData_s(const char * strDataKeyLocker, const char * strDataKeyName, const char * strTimeValue, float * fData, int nDataSize, int nExpirationTime)
{

	// 1. ��������ж�
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	// 2. ����
	int nDelayCount = 0;
	do
	{
		// �����ɹ�
		if (m_pCached->LockKey(strDataKeyLocker))
		{
			break;
		}

		// ��ʱ5����ȴ�����
		delay(5);
	} while (nDelayCount++ < 100);

	if (nDelayCount >= 100)
	{
		// �������ȴ���ʱ����
		return CACHED_LOCK_TIMEOUT;
	}

	// 3. �洢����
	nDataSize = nDataSize * sizeof(float);
	bool bRet = m_pCached->SetValue(strDataKeyName, (char *)fData, nDataSize, nExpirationTime);

	// 4. ����
	m_pCached->UnLockKey(strDataKeyLocker);

	// 5. ���ؽ��
	if (!bRet)
	{
		return CACHED_SETVALUE_FAIL;
	}

	return SUCCESS;
}

/************************************************************************/
/* ��ȡ����Key�ķ���                                                    */
/************************************************************************/
int CachedImpl::GetAllKey(const char * strRootKeyName, StringList &lstKeyName, bool bSortFlg)
{
	// 1. ��������ж�
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	// 2. ��ȡ����
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

	// 3. �������� 
	string strValue = string(pValue);
	lstKeyName.clear();
	split(lstKeyName, strValue, ",");

	// 4. ����
	if (bSortFlg)
	{
		// �����ִ�Сд
		lstKeyName.sort();
	}

#ifndef DEBUG
	// �ͷ�����Ŀռ�
	if (pValue)
	{
		free(pValue);
		pValue = NULL;
	}
#endif

	// 5. ���ؽ��
	return SUCCESS;
}

/************************************************************************/
/* ��ȡ��Ϣ�ķ���                                                       */
/************************************************************************/
int CachedImpl::GetInfo(const char * strInfoKeyName, ST_PRODUCT &stProduct)
{
	// 1. ��������ж�
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	// 2. ��ȡ����
	int nValueSize = 0;
	char *pValue = m_pCached->GetValue(strInfoKeyName, nValueSize);
	if (pValue == NULL)
	{
		return CACHED_GETVALUE_FAIL;
	}

	// 3. JSON���ݽ�������
	bool bRet = JSON::ParserProductInfo(pValue, stProduct);

#ifndef DEBUG
	// �ͷ�����Ŀռ�
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

	// 4. ���سɹ����
	return SUCCESS;
}

/************************************************************************/
/* ��ȡվ����Ϣ�ķ���                                                   */
/************************************************************************/
int CachedImpl::GetInfo(const char * strInfoKeyName, float &fLon, float &fLat, float &fHeight)
{
	// 1. ��������ж�
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	// 2. ��ȡ����
	int nValueSize = 0;
	char *pValue = m_pCached->GetValue(strInfoKeyName, nValueSize);
	if (pValue == NULL)
	{
		return CACHED_GETVALUE_FAIL;
	}

	// 3. JSON���ݽ�������
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
	// �ͷ�����Ŀռ�
	if (pValue)
	{
		free(pValue);
		pValue = NULL;
	}
#endif

	// 4. ���سɹ����
	return SUCCESS;
}

/************************************************************************/
/* ��ȡ����ʱ��ķ���                                                   */
/************************************************************************/
int CachedImpl::GetAllTime(const char * strTimeKeyName, StringList &lstTime, bool bSortFlg)
{
	// 1. ��������ж�
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	// 2. ��ȡ����
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

	// 3. �������� 
	string strValue = string(pValue);
	lstTime.clear();
	split(lstTime, strValue, ",");

	// 4. ����
	if (bSortFlg)
	{
		// �����ִ�Сд
		lstTime.sort();
	}

#ifndef DEBUG
	// �ͷ�����Ŀռ�
	if (pValue)
	{
		free(pValue);
		pValue = NULL;
	}
#endif

	// 5. ���ؽ��
	return SUCCESS;
}

/************************************************************************/
/* ��ȡ���ݵķ���                                                       */
/************************************************************************/
int CachedImpl::GetData(const char * strDataKeyName, char* &pData, int &nDataLen)
{
	// 1. ��������ж�
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	// 2. ��ȡ�����ܳ���
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
	// �ͷ�����Ŀռ�
	if (pValue)
	{
		free(pValue);
		pValue = NULL;
	}
#endif

	// 3. �������ݴ洢�ռ�
	pData = (char *)malloc(nDataLen+1);
	if (pData == NULL)
	{
		return ERR_MEM_MALLOC;
	}

	// 4. ����洢����
	int nKeyCount = (int)(nDataLen / POWER_BLOCK);
	if ((nDataLen % POWER_BLOCK) > 0)
	{
		nKeyCount += 1;
	}

	char strProductDataHashKey[256];
	int nOff = 0;  // ����ָ���α�λ�ã���ʼλ��Ϊ0��

	// 5. ��ȡȫ������
	for (int i = 0; i < nKeyCount; i++)
	{
		// key
		sprintf(strProductDataHashKey, "%s_%d", strDataKeyName, i + 1);
		pValue = m_pCached->GetValue(strProductDataHashKey, nValueLen);
		if (pValue == NULL || nValueLen <= 0)
		{
			// ��ȡʧ��һ�Σ����������ݽ�������
			return CACHED_GETVALUE_FAIL;
		}

		//  ���ݿ���
		memcpy((char *)pData + nOff, pValue, nValueLen);

		// �α�ָ���´δ洢��λ��
		nOff += nValueLen;

		// todo add zhangl 
#ifndef DEBUG
		// �ͷ�����Ŀռ�
		if (pValue)
		{
			free(pValue);
			pValue = NULL;
		}
#endif
	}
	pData[nOff] = '\0';

	// 7. ���سɹ�
	return SUCCESS;
}

/************************************************************************/
/* ��ȡ���ݵķ���                                                       */
/************************************************************************/
int CachedImpl::GetData(const char * strDataKeyName, float* &fData, int &nDataLen)
{
	// 1. ��������ж�
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	// 2. ��ȡ�����ܳ���
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
	// �ͷ�����Ŀռ�
	if (pValue)
	{
		free(pValue);
		pValue = NULL;
	}
#endif

	// 3. �������ݴ洢�ռ�
	fData = (float *)malloc(nDataLen);
	if (fData == NULL)
	{
		return ERR_MEM_MALLOC;
	}

	// 4. ����洢����
	int nKeyCount = (int)(nDataLen / POWER_BLOCK);
	if ((nDataLen % POWER_BLOCK) > 0)
	{
		nKeyCount += 1;
	}

	char strProductDataHashKey[256];
	int nOff = 0;  // ����ָ���α�λ�ã���ʼλ��Ϊ0��

	// 5. ��ȡȫ������
	for (int i = 0; i < nKeyCount; i++)
	{
		// key
		sprintf(strProductDataHashKey, "%s_%d", strDataKeyName, i + 1);
		pValue = m_pCached->GetValue(strProductDataHashKey, nValueLen);
		if (pValue == NULL || nValueLen <= 0)
		{
			// ��ȡʧ��һ�Σ����������ݽ�������
			return CACHED_GETVALUE_FAIL;
		}

		//  ���ݿ���
		memcpy((char *)fData + nOff, pValue, nValueLen);

		// �α�ָ���´δ洢��λ��
		nOff += nValueLen;

		// todo add zhangl 
#ifndef DEBUG
		// �ͷ�����Ŀռ�
		if (pValue)
		{
			free(pValue);
			pValue = NULL;
		}
#endif
	}

	// 6. ����ת��Ϊfloat����
	nDataLen = nDataLen / sizeof(float);

	// 7. ���سɹ�
	return SUCCESS;
}

/************************************************************************/
/* ��ȡ���ݵķ��� -�������ݣ����ݳ��ȣ���ֹ��γ��                       */
/************************************************************************/
int CachedImpl::GetData(const char * strDataKeyName, float* &fData, int &nDataLen, float &fLon1, float &fLon2, float &fLat1, float &fLat2)
{
	// 1. ��������ж�
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	// 2. ��ȡ�����ܳ���
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
	// 2.1 �Ի�ȡ���ݽ��н��� DataLen,Lon1,Lon2,Lat1,Lat2
	string strValue = string(pValue);
	string strData;
	string::size_type nDataOff = 0; 
	string separator = ",";
	// ���ݳ���
	strData = getdata(strValue, separator, nDataOff);
	nDataLen = atoi(strData.data());
	// ��ʼ����
	strData = getdata(strValue, separator, nDataOff);
	fLon1 = atof(strData.data());
	// ��ֹ����
	strData = getdata(strValue, separator, nDataOff);
	fLon2 = atof(strData.data());
	// ��ʼγ��
	strData = getdata(strValue, separator, nDataOff);
	fLat1 = atof(strData.data());
	// ��ֹγ��
	strData = getdata(strValue, separator, nDataOff);
	fLat2 = atof(strData.data());

#ifndef DEBUG
	// �ͷ�����Ŀռ�
	if (pValue)
	{
		free(pValue);
		pValue = NULL;
	}
#endif

	// 3. �������ݴ洢�ռ�
	fData = (float *)malloc(nDataLen);
	if (fData == NULL)
	{
		return ERR_MEM_MALLOC;
	}

	// 4. ����洢����
	int nKeyCount = (int)(nDataLen / POWER_BLOCK);
	if ((nDataLen % POWER_BLOCK) > 0)
	{
		nKeyCount += 1;
	}

	char strProductDataHashKey[256];
	int nOff = 0;  // ����ָ���α�λ�ã���ʼλ��Ϊ0��

	// 5. ��ȡȫ������
	for (int i = 0; i < nKeyCount; i++)
	{
		// key
		sprintf(strProductDataHashKey, "%s_%d", strDataKeyName, i + 1);
		pValue = m_pCached->GetValue(strProductDataHashKey, nValueLen);
		if (pValue == NULL || nValueLen <= 0)
		{
			// ��ȡʧ��һ�Σ����������ݽ�������
			return CACHED_GETVALUE_FAIL;
		}

		//  ���ݿ���
		memcpy((char *)fData + nOff, pValue, nValueLen);

		// �α�ָ���´δ洢��λ��
		nOff += nValueLen;

		// todo add zhangl 
#ifndef DEBUG
		// �ͷ�����Ŀռ�
		if (pValue)
		{
			free(pValue);
			pValue = NULL;
		}
#endif
	}

	// 6. ����ת��Ϊfloat����
	nDataLen = nDataLen / sizeof(float);

	// 7. ���سɹ�
	return SUCCESS;
}

/************************************************************************/
/* ��ȡվ�����ݹ�������                                                    */
/************************************************************************/
int CachedImpl::GetData_s(const char * strDataKeyName, float* &fData, int &nDataLen)
{
	// 1. ��������ж�
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	// 2. ��ȡ����
	int nValueLen;
	char * pValue = m_pCached->GetValue(strDataKeyName, nValueLen);
	if (pValue == NULL || nValueLen <= 0)
	{
		return CACHED_GETVALUE_FAIL;
	}

	// �����ռ�
	fData = (float *)malloc(nValueLen);
	if (fData == NULL)
	{
#ifndef DEBUG
		// �ͷ�����Ŀռ�
		if (pValue)
		{
			free(pValue);
			pValue = NULL;
		}
#endif
		return ERR_MEM_MALLOC;
	}

	//  ���ݿ���
	memcpy((char *)fData, pValue, nValueLen);
	//fData = (float *)pValue;
	nDataLen = nValueLen / sizeof(float); // ���ݳ���

	// todo add zhangl 
#ifndef DEBUG
	// �ͷ�����Ŀռ�
	if (pValue)
	{
		free(pValue);
		pValue = NULL;
	}
#endif

	return SUCCESS;
}

/************************************************************************/
/* ɾ��Key�Ĺ�������                                                    */
/************************************************************************/
int CachedImpl::DeleteKey(const char * strRootKeyLocker, const char * strRootKeyName, const char * strValueKeyName)
{
	// 1. ��������ж�
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	// 2. ����
	int nDelayCount = 0;
	do
	{
		// �����ɹ�
		if (m_pCached->LockKey(strRootKeyLocker))
		{
			break;
		}

		// ��ʱ5����ȴ�����
		delay(5);
	} while (nDelayCount++ < 100);

	if (nDelayCount >= 100)
	{
		// �������ȴ���ʱ����
		return CACHED_LOCK_TIMEOUT;
	}

	// 3. ��ȡԭ����
	string strValue;
	char *pValue = NULL;
	int  nValueSize = 0;
	pValue = m_pCached->GetValue(strRootKeyName, nValueSize);
	if (pValue == NULL)
	{
		// ����
		m_pCached->UnLockKey(strRootKeyLocker);

		// û�����key(LastError=16)
		if (m_pCached->GetLastError() == MEMCACHE_KEY_NOT_FOUND)
		{
			// û�д�Key����������
			return SUCCESS;
		}
		else
		{
			// �쳣������
			return CACHED_GETVALUE_FAIL;
		}
	}

	// 4. ���ַ�����ɾ�������ݺ�������װ����
	strValue = string(pValue);
	strValue = DeleteValueFromString(strValue, strValueKeyName);

#ifndef DEBUG
	// �ͷ�����Ŀռ�
	if (pValue)
	{
		free(pValue);
		pValue = NULL;
	}
#endif

	// 5. �����������±��浽ProductTime_PK ��
	bool bRet;
	if (strValue == "")
	{
		// ����Ѿ�û�������ˣ���ɾ����Key��ע���޷�д��һ�����ַ�����һ��Key�У�
		bRet = m_pCached->DelKey(strRootKeyName);
		if (!bRet)
		{
			// ������ΪKey�����ڶ����µ�ʧ��
			if (m_pCached->GetLastError() == MEMCACHE_KEY_NOT_FOUND)
			{
				bRet = true;
			}
		}
	}
	else
	{
		// ����д��
		bRet = m_pCached->SetValue(strRootKeyName, strValue.data(), strValue.size());
	}

	// 6. ����
	m_pCached->UnLockKey(strRootKeyLocker);

	// 7. ���ؽ��
	if (!bRet)
	{
		return CACHED_DELVALUE_FAIL;
	}

	return SUCCESS;
}

/************************************************************************/
/* ɾ����Ϣ�Ĺ�������                                                   */
/************************************************************************/
int CachedImpl::DeleteInfo(const char * strInfoKeyLocker, const char * strInfoKeyName)
{
	// 1. ��������ж�
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	// 2. ����
	int nDelayCount = 0;
	do
	{
		// �����ɹ�
		if (m_pCached->LockKey(strInfoKeyLocker))
		{
			break;
		}

		// ��ʱ5����ȴ�����
		delay(5);
	} while (nDelayCount++ < 100);

	if (nDelayCount >= 100)
	{
		// �������ȴ���ʱ����
		return CACHED_LOCK_TIMEOUT;
	}

	// 3. ɾ����key
	bool bRet = true;
	if (!m_pCached->DelKey(strInfoKeyName))
	{
		// ɾ��ʧ��
		if (m_pCached->GetLastError() != MEMCACHE_KEY_NOT_FOUND)
		{
			bRet = false;
		}
	}

	// 4. ����
	m_pCached->UnLockKey(strInfoKeyLocker);

	// 5. ���ؽ��
	if (!bRet)
	{
		return CACHED_DELVALUE_FAIL;
	}
	return SUCCESS;
}

/************************************************************************/
/* ɾ��ĳʱ��Ĺ�������                                                 */
/************************************************************************/
int CachedImpl::DeleteTime(const char * strTimeKeyLocker, const char * strTimeKeyName, const char * strTimeValue)
{
	// 1. ��������ж�
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	// 2. ����
	int nDelayCount = 0;
	do
	{
		// �����ɹ�
		if (m_pCached->LockKey(strTimeKeyLocker))
		{
			break;
		}

		// ��ʱ5����ȴ�����
		delay(5);
	} while (nDelayCount++ < 100);

	if (nDelayCount >= 100)
	{
		// �������ȴ���ʱ����
		return CACHED_LOCK_TIMEOUT;
	}

	// 3. ��ȡԭ����
	string strValue;
	char *pValue = NULL;
	int  nValueSize = 0;
	pValue = m_pCached->GetValue(strTimeKeyName, nValueSize);
	if (pValue == NULL)
	{
		// ����
		m_pCached->UnLockKey(strTimeKeyLocker);

		// û�����key(LastError=16)
		if (m_pCached->GetLastError() == MEMCACHE_KEY_NOT_FOUND)
		{
			// û�д�Key����������
			return SUCCESS;
		}
		else
		{
			// �쳣������
			return CACHED_GETVALUE_FAIL;
		}
	}

	// 4. ���ַ�����ɾ�������ݺ�������װ����
	strValue = string(pValue);
	strValue = DeleteValueFromString(strValue, strTimeValue);

#ifndef DEBUG
	// �ͷ�����Ŀռ�
	if (pValue)
	{
		free(pValue);
		pValue = NULL;
	}
#endif

	// 5. �����������±��浽ProductTime_PK ��
	bool bRet;
	if (strValue == "")
	{
		// ����Ѿ�û�������ˣ���ɾ����Key��ע���޷�д��һ�����ַ�����һ��Key�У�
		bRet = m_pCached->DelKey(strTimeKeyName);
		if (!bRet)
		{
			// ������ΪKey�����ڶ����µ�ʧ��
			if (m_pCached->GetLastError() == MEMCACHE_KEY_NOT_FOUND)
			{
				bRet = true;
			}
		}
	}
	else
	{
		// ����д��
		bRet = m_pCached->SetValue(strTimeKeyName, strValue.data(), strValue.size());
	}

	// 6. ����
	m_pCached->UnLockKey(strTimeKeyLocker);

	// 7. ���ؽ��
	if (!bRet)
	{
		return CACHED_DELVALUE_FAIL;
	}

	return SUCCESS;
}

/************************************************************************/
/* ɾ�����ݵĹ�������                                                   */
/************************************************************************/
int CachedImpl::DeleteData(const char * strDataKeyLocker, const char * strDataKeyName)
{
	// 1. ��������ж�
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	// 2. ����
	int nDelayCount = 0;
	do
	{
		// �����ɹ�
		if (m_pCached->LockKey(strDataKeyLocker))
		{
			break;
		}

		// ��ʱ5����ȴ�����
		delay(5);
	} while (nDelayCount++ < 100);

	if (nDelayCount >= 100)
	{
		// �������ȴ���ʱ����
		return CACHED_LOCK_TIMEOUT;
	}

	// 3. ��ȡ�����ܳ���
	int nDataLen = 0;
	bool bRet = true;
	char *pValue = m_pCached->GetValue(strDataKeyName, nDataLen);
	if (pValue == NULL || nDataLen <= 0)
	{
		// ����
		m_pCached->UnLockKey(strDataKeyLocker);

		// ������ΪKey�����ڶ����µ�ʧ��
		if (m_pCached->GetLastError() == MEMCACHE_KEY_NOT_FOUND)
		{
			// ��ʾ��Key�����ڣ������Ѿ����
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
	// ���ݳ���
	strData = getdata(strValue, separator, nOff);
	nDataLen = atoi(strData.data());

#ifndef DEBUG
	// �ͷ�����Ŀռ�
	if (pValue)
	{
		free(pValue);
		pValue = NULL;
	}
#endif

	// 4. ����洢����
	int nKeyCount = (int)(nDataLen / POWER_BLOCK);
	if ((nDataLen % POWER_BLOCK) > 0)
	{
		nKeyCount += 1;
	}

	// 5. ����ɾ������ɢ������
	char strProductDataHashKey[256];
	for (int i = 0; i < nKeyCount; i++)
	{
		// key
		sprintf(strProductDataHashKey, "%s_%d", strDataKeyName, i + 1);
		// ɾ��key
		bRet = m_pCached->DelKey(strProductDataHashKey);
		if (!bRet)
		{
			// ������ΪKey�����ڶ����µ�ʧ��
			// ɾ��ʧ��һ�Σ�������ɾ��ʧ��
			if (m_pCached->GetLastError() != MEMCACHE_KEY_NOT_FOUND)
			{
				// ����
				m_pCached->UnLockKey(strDataKeyLocker);
				// ����
				return CACHED_DELVALUE_FAIL;
			}
		}
	}

	// 6. ɾ���洢�ܳ��ȵ�Key
	bRet = m_pCached->DelKey(strDataKeyName);
	if (!bRet)
	{
		// ������ΪKey�����ڶ����µ�ʧ��
		if (m_pCached->GetLastError() != MEMCACHE_KEY_NOT_FOUND)
		{
			// ����
			m_pCached->UnLockKey(strDataKeyLocker);
			// ���ؽ��
			return CACHED_DELVALUE_FAIL;
		}
	}

	// 7. ����
	m_pCached->UnLockKey(strDataKeyLocker);

	// 8. ���ؽ��
	return SUCCESS;
}

/************************************************************************/
/* ɾ��վ�����ݵĹ�������                                               */
/************************************************************************/
int CachedImpl::DeleteData_s(const char * strDataKeyLocker, const char * strDataKeyName)
{
	// 1. ��������ж�
	if (m_pCached == NULL)
	{
		return GET_PROVIDER_FAIL;
	}

	// 2. ����
	int nDelayCount = 0;
	do
	{
		// �����ɹ�
		if (m_pCached->LockKey(strDataKeyLocker))
		{
			break;
		}

		// ��ʱ5����ȴ�����
		delay(5);
	} while (nDelayCount++ < 100);

	if (nDelayCount >= 100)
	{
		// �������ȴ���ʱ����
		return CACHED_LOCK_TIMEOUT;
	}

	// 3. ɾ������
	bool bRet = m_pCached->DelKey(strDataKeyName);
	if (!bRet)
	{
		// ������ΪKey�����ڶ����µ�ʧ��
		if (m_pCached->GetLastError() != MEMCACHE_KEY_NOT_FOUND)
		{
			// ����
			m_pCached->UnLockKey(strDataKeyLocker);
			// ���ؽ��
			return CACHED_DELVALUE_FAIL;
		}
	}

	// 4. ����
	m_pCached->UnLockKey(strDataKeyLocker);

	// 5. ���ؽ��
	return SUCCESS;
}

/************************************************************************/
/* �ӻ�ȡ�������ַ������ж�ĳKeyValue�Ƿ����                           */
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
/* �ӻ�ȡ�������ַ�����ɾ��ĳKeyValue�󷵻�������װ����ַ���           */
/************************************************************************/
string CachedImpl::DeleteValueFromString(string strSrc, string strValue)
{
	StringList lstSrc;
	split(lstSrc, strSrc, ",");
	if (lstSrc.size() < 1)
	{
		return strSrc;
	}

	//  ɾ������
	lstSrc.remove(strValue);

	// ���ºϳ�����
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
