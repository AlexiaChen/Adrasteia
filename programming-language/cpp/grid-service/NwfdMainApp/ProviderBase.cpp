#include "ProviderBase.h"
#include "string.h"

ProviderBase::ProviderBase()
{
#ifdef _WIN32
	strcpy_s(m_strProviderName, "Provider Base");
	strcpy_s(m_strServerIP, "127.0.0.1");
#else
	strcpy(m_strProviderName, "Provider Base");
	strcpy(m_strServerIP, "127.0.0.1");
#endif

	m_nPortNo = 6379;
	m_bConStatus = false;
}


ProviderBase::~ProviderBase()
{
}

// 基本内容
void ProviderBase::SetProviderName(const char * strProviderName)
{
#ifdef _WIN32
	strcpy_s(m_strProviderName, strProviderName);
#else
	strcpy(m_strProviderName, strProviderName);
#endif
}

// 连接
bool ProviderBase::SetConnectParam(const char *strServerIP, int nPortNo)
{
	return false;
}

bool ProviderBase::Connect(const char *strServerIP, int nPortNo)
{
	return false;
}

bool ProviderBase::Connect()
{
	return false;
}
void ProviderBase::DisConnect()
{
	return;
}
bool ProviderBase::ReConnect()
{
	return false;
}
bool ProviderBase::ConnectStatus()
{
	return false;
}

// 增删改查操作
//  1、 基本操作   
bool ProviderBase::FlushDB()
{
	return false;
}
bool ProviderBase::DelKey(const char * strKey)
{
	return false;
}
bool ProviderBase::IsExistsKey(const char * strKey)
{
	return false;
}
//  2、Key-Value 数据写/读(使用于Redis和Memcached)
bool ProviderBase::SetValue(const char * strKey, const char * szValue, int nValueSize, int nExpirationTime)
{
	return false;
}
bool ProviderBase::AppendValue(const char * strKey, const char * szValue, int nValueSize)
{
	return false;
}
char * ProviderBase::GetValue(const char * strKey, int & nValueSize)
{
	return NULL;
}

//  3、Redis Set类型
bool ProviderBase::AddSMember(const char * strKey, const char * strValue)
{
	return false;
}
bool ProviderBase::RemoveSMember(const char * strKey, const char * strValue)
{
	return false;
}
bool ProviderBase::GetSMembers(const char * strKey, char(*pValArray)[256], int &nCount)
{
	return false;
}
bool ProviderBase::IsSMember(const char * strKey, const  char * strValue)
{
	return false;
}

//  4、Redis hash类型
bool ProviderBase::SetHValue(const char * strKey, const char * strField, const char * strValue)
{
	return false;
}
bool ProviderBase::DelHField(const char * strKey, const char * strField)
{
	return false;
}
bool ProviderBase::GetHValue(const char * strKey, const char * strField, char * strValue)
{
	return false;
}
char * ProviderBase::GetHValue(const char * strKey, const char * strField)
{
	return NULL;
}

bool ProviderBase::LockKey(const char * strKey, int nExpirationTime)
{
	return false;
}
bool ProviderBase::UnLockKey(const char *strKey)
{
	return false;
}
int ProviderBase::GetLastError()
{
	return 0;
}
