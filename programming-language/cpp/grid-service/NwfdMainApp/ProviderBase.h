/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: ProviderBase.h
* 作  者: zhangl		版本：1.0		日  期：2015/05/08
* 描  述：缓存处理基类（接口）
* 其  他：
* 功能列表:
*   1. 定义缓存处理的基本操作方法
*
* 修改日志：
*   No.   日期		作者		修改内容
*   1  2015/05/13  zhangl    添加Key-Value读写方法（for MemCached/redis）
*   2  2015/05/15  zhangl    添加memcached的特殊锁处理
*************************************************************************/
#ifndef PROVIDER_BASE_H
#define PROVIDER_BASE_H

#include "stdio.h"

/************************************************************************/
/* 类 名： ProviderBase                                                 */
/* 父 类： -                                                            */
/* 说 明： 缓存处理基类                                                 */
/* 描 述： 所有类型的缓存都要继承此类，实现其接口定义的方法，操作缓存   */
/************************************************************************/
class ProviderBase
{
public:
	ProviderBase();
	virtual ~ProviderBase();

public:
	// 基本内容
	void SetProviderName(const char *strProviderName);

	// 连接
	virtual bool SetConnectParam(const char *strServerIP, int nPortNo);
	virtual bool Connect(const char *strServerIP, int nPortNo);
	virtual bool Connect();
	virtual void DisConnect();
	virtual bool ReConnect();
	virtual bool ConnectStatus();

	// 增删改查操作
	//  1、 基本操作   
	virtual bool FlushDB();
	virtual bool DelKey(const char * strKey);
	virtual bool IsExistsKey(const char * strKey);

	//  2、Key-Value 数据写/读(使用于Redis和Memcached)
	virtual bool SetValue(const char * strKey, const char * szValue, int nValueSize, int nExpirationTime = 0);
	virtual bool AppendValue(const char * strKey, const char * szValue, int nValueSize);
	virtual char *GetValue(const char * strKey, int & nValueSize);

	//  3、Redis Set类型
	virtual bool AddSMember(const char * strKey, const char * strValue);
	virtual bool RemoveSMember(const char * strKey, const char * strValue);
	virtual bool GetSMembers(const char * strKey, char(*pValArray)[256], int &nCount);
	virtual bool IsSMember(const char * strKey, const char * strValue);

	//  4、Redis hash类型
	virtual bool SetHValue(const char * strKey, const char * strField, const char * strValue);
	virtual bool DelHField(const char * strKey, const char * strField);
	virtual bool GetHValue(const char * strKey, const char * strField, char * strValue);
	virtual char * GetHValue(const char * strKey, const char * strField);

	// 5、特殊处理（for memcached）
	virtual bool LockKey(const char * strKey, int nExpirationTime = 1);
	virtual bool UnLockKey(const char *strKey);
	virtual int  GetLastError();

public:
	char   m_strProviderName[50];  // 名称
	char   m_strServerIP[32];      // 缓存 Server IP地址
	int    m_nPortNo;              // 缓存 Server 端口
	bool   m_bConStatus;           // 连接状态
};

#endif //PROVIDER_BASE_H
