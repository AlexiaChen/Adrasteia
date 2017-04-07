#include "CachedManager.h"

/************************************************************************/
/* ���캯��                                                             */
/************************************************************************/
CachedManager::CachedManager()
{
	m_nUnUseCount = 0;
	m_nCachedOpenCount = 0;
}

/************************************************************************/
/* ��������                                                             */
/************************************************************************/
CachedManager::~CachedManager()
{
	DestroyAllConnect();
}

/************************************************************************/
/* ������ȡ����                                                         */
/************************************************************************/
CachedManager & CachedManager::getClass()
{
	static CachedManager objManager;
	return objManager;
}

/************************************************************************/
/* ��ʼ��                                                               */
/************************************************************************/
int CachedManager::Init(ST_CACHED stCached, int nConnNum)
{
	// ������Ϣ
	m_stCached = stCached;
	m_nConnNum = nConnNum;
	if (m_nConnNum < MIN_CACHED_CONNECT_NUM)
	{
		m_nConnNum = MIN_CACHED_CONNECT_NUM;
	}

	// ��ʼ����������
	InitAllConnect();
	if (m_nCachedOpenCount == 0)
	{
		// û�п�������
		return CACHED_INIT_FAIL;
	}

	return SUCCESS;
}

/************************************************************************/
/* ��ʼ����������                                                       */
/************************************************************************/
int CachedManager::InitCachedImpl(CachedImpl * &pCachedImpl)
{
	pCachedImpl = new CachedImpl(m_stCached);
	if (pCachedImpl == NULL)
	{
		return ERR_CREATE_CACHED;
	}

	// ��ȡ��������
	int nErrCode = pCachedImpl->GetProvider();
	if (nErrCode != SUCCESS)
	{
		return nErrCode;
	}

	return SUCCESS;
}

/************************************************************************/
/* ��ʼ����������                                                       */
/************************************************************************/
int CachedManager::InitAllConnect()
{
	// ��ʼ����������
	m_nCachedOpenCount = 0;// �򿪳ɹ������ݿ����
	m_nUnUseCount = 0;     // δ��ʹ�õ����Ӹ���

	for (int nConnectID = 1; nConnectID <= m_nConnNum; nConnectID++)
	{
		ST_CACHED_CONNECT stCachedConnect;

		// ����һ������
		CachedImpl * pCachedImpl = NULL;
		int nCachedState = InitCachedImpl(pCachedImpl);
		if (nCachedState == SUCCESS)
		{
			// �������ӵĸ���
			m_nCachedOpenCount++;
		}

		stCachedConnect.nConnectID = nConnectID;      // ���ӹ���ID
		stCachedConnect.nConnectState = nCachedState; // ����״̬����
		stCachedConnect.bUseState = false;            // ʹ��״̬δ��ʹ��

		// δ��ʹ�õ����ݿ�������
		m_nUnUseCount++;

		// ���������
		m_mapCached[nConnectID] = pCachedImpl;

		// ���뵽���ݿ����ӹ����б���
		m_lstCachedConnect.push_back(stCachedConnect);
	}

	return SUCCESS;
}

/************************************************************************/
/* �ر���������                                                         */
/************************************************************************/
void CachedManager::DestroyAllConnect()
{
	// ɾ����������
	MAP_CACHED::iterator iter;
	for (iter = m_mapCached.begin(); iter != m_mapCached.end(); iter++)
	{
		CachedImpl * pCachedImpl = iter->second;
		if (pCachedImpl != NULL)
		{
			delete pCachedImpl;
			pCachedImpl = NULL;
		}
	}
	m_mapCached.clear();

	// ɾ���������
	m_lstCachedConnect.clear();

	m_nCachedOpenCount = 0;
	m_nUnUseCount = 0;
}

/************************************************************************/
/* ��ȡ����״̬                                                         */
/************************************************************************/
int CachedManager::GetCachedStatus()
{
	int nConnectID;
	CachedImpl * pCachedImpl = NULL;
	int nCachedStatusCnt = 0;

	for (LIST_CACHED_CONNECT::iterator iter = m_lstCachedConnect.begin(); iter != m_lstCachedConnect.end(); iter++)
	{
		nConnectID = (*iter).nConnectID; // ���ݿ����ӵ�ID��
		pCachedImpl = m_mapCached[nConnectID]; // ��������

		// ֮ǰ�����������
		if ((*iter).nConnectState != SUCCESS)
		{
			if (pCachedImpl)
			{
				delete pCachedImpl;
				pCachedImpl = NULL;
			}

			// ���´���
			(*iter).nConnectState = InitCachedImpl(pCachedImpl);
			m_mapCached[nConnectID] = pCachedImpl;
		}

		// ����״̬�ж�
		if (pCachedImpl)
		{
			if (pCachedImpl->ConnectStatus() == SUCCESS)
			{
				nCachedStatusCnt++;
			}
		}
	}

	if (nCachedStatusCnt == 0)
	{
		return CACHED_CONNECT_ABNORMAL;
	}

	return SUCCESS;
}

/************************************************************************/
/* �������ӻ���                                                         */
/************************************************************************/
int CachedManager::ReconnectCached()
{
	int nConnectID;
	CachedImpl * pCachedImpl = NULL;
	for (LIST_CACHED_CONNECT::iterator iter = m_lstCachedConnect.begin(); iter != m_lstCachedConnect.end(); iter++)
	{
		if ((*iter).bUseState)
		{
			continue;
		}

		nConnectID = (*iter).nConnectID; // ���ݿ����ӵ�ID��
		pCachedImpl = m_mapCached[nConnectID]; // ��������

		// ֮ǰ�����������
		if ((*iter).nConnectState != SUCCESS)
		{
			if (pCachedImpl)
			{
				delete pCachedImpl;
				pCachedImpl = NULL;
			}

			// ���´���,�滻ԭ��������
			(*iter).nConnectState = InitCachedImpl(pCachedImpl);
			m_mapCached[nConnectID] = pCachedImpl;
		}
		else
		{
			pCachedImpl->ReConnect();
		}
	}

	return SUCCESS;
}

/************************************************************************/
/* ��ȡһ��δ�õ�����                                                   */
/************************************************************************/
int CachedManager::GetACachedImpl(CachedImpl * & pCachedImpl)
{
	int nConnectID;
	nConnectID = -1;

	int nCount = 0;

	// δ��������=0 ����ʱ�ȴ��ͷ�
	while (m_nUnUseCount == 0)
	{
		// ��ʱ
		delay(50);         // 100����
		if (nCount++ == 10) // ���ȴ�500��
		{
			break;
		}
	}

	QMutexLocker lock(&m_MutexConnect);

	// �����ӹ����б����ҵ�һ��δ��ʹ�õ�����
	for (LIST_CACHED_CONNECT::iterator iter = m_lstCachedConnect.begin(); iter != m_lstCachedConnect.end(); iter++)
	{
		if (!(*iter).bUseState &&( (*iter).nConnectState == SUCCESS))
		{
			nConnectID = (*iter).nConnectID; // ���ݿ����ӵ�ID��
			pCachedImpl = m_mapCached[nConnectID]; // ��������

			(*iter).bUseState = true;        // ���ı��Ϊʹ��
			m_nUnUseCount--;                 // δ��������
			break;
		}
	}
	 
	return nConnectID;
}

/************************************************************************/
/* �ͷ�һ��ʹ�õ�����                                                   */
/************************************************************************/
void CachedManager::RestoreACachedImpl(int nConnectID)
{
	QMutexLocker lock(&m_MutexConnect);

	// �����ӹ����б����ҵ�һ��δ��ʹ�õ�����
	LIST_CACHED_CONNECT::iterator iter = m_lstCachedConnect.begin();
	for (; iter != m_lstCachedConnect.end(); iter++)
	{
		if ((*iter).nConnectID == nConnectID)
		{
			(*iter).bUseState = false; // ���ı��Ϊδ������
			m_nUnUseCount++;           // δ��������
			break;
		}
	}
}


