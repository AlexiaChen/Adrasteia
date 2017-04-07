/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* �ļ���: DataManager.h
* ��  ��: zhangl		�汾��1.0		��  �ڣ�2015/11/20
* ��  �������ݹ�����������ͬ������վ�����ݣ�
* ��  ����
* �����б�:
* �޸���־��
*   No. ����		����		�޸�����
*
*************************************************************************/
#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include "DataManagerDef.h"
#include "NwfdClientDef.h"
#include "ProductDef.h"
#include "TaskBase.h"
#include "CachedImpl.h"
#include "TaskTcpServer.h"
#include "TaskStationMsg.h"
#include <QString>
#include <QMutex>
#include <QMutexLocker>

/************************************************************************/
/* �� ���� DataManager                                                  */
/* �� �ࣺ                                                              */
/* ˵ ���� ���ݹ�����                                                   */
/* �� ����                                                              */
/************************************************************************/
class DataManager
{
private:
	DataManager();

public:
	~DataManager();
	static DataManager & getClass();

public:
	HASH_STATION_INFO &GetStationList();
	QString  GetForecastType();
	bool IsStation2Grid();

	/* ��ʼ�� */
	int Init(HASH_PRODUCT hasProduct, ST_CACHED stCached, ST_STATION_MAG stStationMag);
	int Grid2Station(QString strType, int nTimeRange, QString strProductKey, float * fGridData, float fStartLon, float fStartLat, float fDi, float fDj, int nNi, int nNj, int nYear, int nMonth, int nDay, int nHour, int nForecastTime, bool bFirst = false);
	int Station2Grid(QString strStationNo,int nTimeRange, float * fStationData, int nDataLen, int nYear, int nMonth, int nDay, int nHour, int nForecastTime, QString &strCityType, QString &strForecastType, int &nStationRange, bool & bIsUpdate);
	int MultiStation2Grid(QString strDataType, LIST_STATION_VALUE  lstStationDiffVal, float fStartLon, float fStartLat, float fEndLon, float fEndLat, int nYear, int nMonth, int nDay, int nHour, int nTimeRange, int nForecastTime);

	int SaveMultiStation(QString strType, int nYear, int nMonth, int nDay, int nHour, int nTimeRange, int nMaxForecastTime);

	int SetStationData(QString strType, int nTimeRange, int nYear, int nMonth, int nDay, int nHour, int nForecastTime, QString strStationNo, float fData);
	int GetStationData(QString strType, int nTimeRange, int nYear, int nMonth, int nDay, int nHour, int nForecastTime, QString strStationNo, float &fData);

	int flushStationInfoToMemcached();
	int flushStationInfoToLocal();

private:
	bool InitCached();
	int  InitStationList();
	bool GetGridRange(float fStationLon, float fStationLat, float fRadius, float fStartLon, float fStartLat, float fDi, float fDj, int nNi, int nNj, float &fLon1, float &fLon2, float &fLat1, float &fLat2);
	bool GetStationList(float fLon1, float fLon2, float fLat1, float fLat2, float fRadius, LIST_STATION_VALUE &lstStation);
	
	void SetStationProduct(QString strType, int nTimeRange, QString strProductKey);
	bool GetProduct(QString strType, int nTimeRange, ST_PRODUCT &stProduct);

	void ClearStationData();

private:
	ST_CACHED         m_stCached;        // ����������Ϣ
	CachedImpl *      m_pCachedImpl;     // ����ҵ����
	ST_STATION_MAG    m_stStationMag;    // վ��-��㴦������

	HASH_PRODUCT      m_hasProduct;      // ����Ʒ�б�
	HASH_STATION_INFO m_hasStaitonInfo;  // վ���б�

	HASH_STATION_DATA m_hasStationData;  // �洢���в�ֵ��վ������
	QMutex            m_mutexStationData;// ɢ��վ����

	HASH_STATION_PRODUCT m_hasStationProduct; // վ���Ʒƥ���

};

#endif // DATA_MANAGER_H


