/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* �ļ���: NetworkManager.h
* ��  ��: zhangl		�汾��1.0		��  �ڣ�2015/07/16
* ��  �����������ݴ���
* ��  ����
* �����б�:
* �޸���־��
*   No. ����		����		�޸�����
*
*************************************************************************/
#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

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
/* �� ���� NetworkManager                                               */
/* �� �ࣺ                                                              */
/* ˵ ���� �������ݽ���������                                           */
/* �� ����                                                              */
/************************************************************************/
class NetworkManager
{
private:
	NetworkManager();

public:
	~NetworkManager();
	static NetworkManager & getClass();

public:
	/* ��ʼ�� */
	int Init(HASH_PRODUCT hasProduct, HASH_CLIENT  hasClient, ST_CACHED stCached);
	void SetTcpServer(TaskTcpServer*  pTaskTcpServer);
	void SetStationMsg(TaskStationMsg* pTaskStationMsg);
	void SetIsPublish(bool bIsPublish);

	/* ��Ʒ���� */
	bool AddProduct(ST_PRODUCT stProduct);
	bool FindProduct(QString strProductKey);
	bool RemoveProduct(QString strProductKey);
	bool GetProduct(QString strProductKey, ST_PRODUCT &stProduct);

	/* ������� */
	bool AddTask(QString strProductKey, TaskBase * pTask);
	TaskBase * FindTask(QString strProductKey);
	void RemoveTask(QString strProductKey);

	/* �ͻ����������� */
	bool IsLegalClient(int nClientID, QString strClientIP);
	bool IsLogin(int nClientID, QString strUserName, QString strPasswd, bool &bAllowUpdate);
	int HandleGridData(int nClientID, QString strProductKey, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, 
		int nForecastTime, int nTimeRange, float fLon1, float fLon2, float fLat1, float fLat2);
	int HandleStationData(int nClientID, QString strCityType, QString strStationNo, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond,
		int nForecastTime, int nTimeRange, float fLat, float fLon, float fHeight);

	int HandleStationFile(int nClientID, QString strCityType, int nTimeRange, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond);
	int HandleMultiStationData(int nClientID, QString strDataType, int nTimeRange,int nEndForcast, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond);
	
	int HandleGrid2Station(int nClientID, QString strProductKey, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nStartForecast, int nEndForecast);
	int HandleStation2Grid(int nClientID, QString strDataType, int nTimeRange, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nStartForecast, int nEndForecast);
	int HandleRainProcess(int nClientID, QString strProductKey, QString strRelatedProductKey, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nStartForecast, int nEndForecast);

	int HandleUploadStationCfg(int nClientID);

	/* ƴͼ������� */
	int PublishGrid(QString strProductKey, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nForecastTime, int nTimeRange);
	int PublishStation(QString strDataType, QString strCityType, QString strStationNo, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nForecastTime, int nTimeRange);
	int PublishMultiStation(QString strDataType,  int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nTimeRange);

private:
	bool InitCached();
	int  SaveClientData(int nClientID, QString strProductKey, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond,
		int nForecastTime, int nTimeRange, float fLon1, float fLon2, float fLat1, float fLat2);

private:
	/*�����ϴ�վ�����õĶ�Ӧ������*/
	int HandleAdd(const QString& stationNum, const QString& stationName,float lat,float lon,float height);
	int HandleDelete(const QString& stationNum, const QString& stationName, float lat, float lon, float height);
	int HandleModify(const QString& stationNum, const QString& stationName, float lat, float lon, float height);

	/* ��ˮ���� */
	bool HandleRainCorrection_Upward(CachedImpl * pCachedImpl, ST_PRODUCT stProduct, ST_PRODUCT stRelatedProduct, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nForecastTime);
	bool HandleRainCorrection_Downward(CachedImpl * pCachedImpl, ST_PRODUCT stProduct, ST_PRODUCT stRelatedProduct, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nForecastTime);

private:
	HASH_CLIENT       m_hasClient;       // �ͻ��������б�
	HASH_PRODUCT      m_hasProduct;      // ����Ĳ�Ʒ�б�: ע�˴���Key����Ҫ�ϲ��Ĺ���Key
	QMutex            m_mutexProduct;    // ��
	HASH_PRODUCT_TASK m_hasProductTask;  // �����б� : ע�˴���Key����Ҫ�ϲ��Ĺ���Key
	QMutex            m_mutexTask;       // ������
	ST_CACHED         m_stCached;        // ����������Ϣ

	TaskTcpServer*    m_pTaskTcpServer;  // socketServer
	TaskStationMsg*   m_pTaskStationMsg; // վ����Ϣ����
	bool              m_bIsPublish;      // �����ܿ���

	QMutex  m_mutexStationCfg;//վ��������

};


#endif //NETWORK_MANAGER_H