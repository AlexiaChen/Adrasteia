/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* �ļ���: CachedManager.h
* ��  ��: zhangl		�汾��1.0		��  �ڣ�2015/12/25
* ��  �����������ӹ���
* ��  ����
* �����б�:
*   1.
* �޸���־��
*   No. ����		����		�޸�����
*************************************************************************/
#ifndef CACHED_MANAGER_H
#define CACHED_MANAGER_H

#include "CachedImpl.h"
#include <list>
#include <map>
#include <QMutex>

using namespace std;
/************************************************************************/
/* ���Ͷ���                                                             */
/************************************************************************/
#define MIN_CACHED_CONNECT_NUM    2         // ��С�������Ӹ���

// �������ӽṹ��
typedef struct _CACHED_CONNECT
{
	int     nConnectID;      // ����ID
	int     nConnectState;   // ����״̬ 0:��������0:�쳣
	bool    bUseState;       // ����״̬ true:����ʹ�ã�false:δ��ʹ��
} ST_CACHED_CONNECT;

// ���ӹ����б�
typedef list<ST_CACHED_CONNECT> LIST_CACHED_CONNECT;

// �������Ӷ���
typedef map<int, CachedImpl *> MAP_CACHED;  

/************************************************************************/
/* �� ���� CachedManager                                                */
/* �� �ࣺ -                                                            */
/* ˵ ���� �������ӹ�����                                               */
/* �� ����                                                              */
/************************************************************************/
class CachedManager
{
public:
	CachedManager();

public:
	~CachedManager();
	static CachedManager & getClass();
    
	int Init(ST_CACHED stCached, int nConnNum);// ��ʼ��
	void DestroyAllConnect();                  // �ر���������

	int GetCachedStatus(); // ��ȡ����״̬
	int ReconnectCached(); // �������ӻ���

	int GetACachedImpl(CachedImpl * & cached);// ��ȡһ��δ�õ�����
	void RestoreACachedImpl(int nConnectID);  // �ͷ�һ��ʹ�õ�����
private:
	int  InitAllConnect();    // ��ʼ����������

	int InitCachedImpl(CachedImpl * &pCachedImpl);

private:
	ST_CACHED   m_stCached;    // ��������
	int         m_nConnNum;    // �������Ӹ���

	int         m_nUnUseCount;      // δ��ʹ�õ�������
	int         m_nCachedOpenCount; // �򿪳ɹ������ݿ����
	MAP_CACHED  m_mapCached;        // ���������б�
	LIST_CACHED_CONNECT m_lstCachedConnect; // �������ӹ����б�

	QMutex      m_MutexConnect;  // ������
};

/************************************************************************/
/* �� ���� CachedGuard                                                  */
/* �� �ࣺ -                                                            */
/* ˵ ���� ��������������--ʹ������ʽ����                               */
/* �� ���� ���ù��������������֤����ȡ���͹黹����ɶԣ���ֹ��Դй¶   */
/************************************************************************/
class CachedGuard
{
public:
	CachedGuard(CachedImpl * & cached)
	{
		m_nConnectID = CachedManager::getClass().GetACachedImpl(cached);
	}

	// ������������ID
	CachedGuard(CachedImpl * & cached, int &nConnectID)
	{
		nConnectID = CachedManager::getClass().GetACachedImpl(cached);
		m_nConnectID = nConnectID;
	}

	virtual ~CachedGuard()
	{
		// ͨ�����ӹ����Ž��й���
		CachedManager::getClass().RestoreACachedImpl(m_nConnectID);
	}

private:
	int m_nConnectID;  // ���ӹ���ID
};

#endif // CACHED_MANAGER_H

