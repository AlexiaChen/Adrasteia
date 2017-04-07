/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* �ļ���: StationDef.h
* ��  ��: zhangl		�汾��1.0		��  �ڣ�2015/08/04
* ��  ����վ����
* ��  ����
* �����б�:
*   1.
* �޸���־��
*   No. ����		����		     �޸�����
*************************************************************************/
#ifndef STATION_DEF_H
#define STATION_DEF_H

#include "ProductDef.h"
#include <QDateTime>
#include <QList>

/************************************************************************/
/* 1. վ�����ݶ���                                                      */
/************************************************************************/
typedef struct _station
{
	char   stationNo[10]; // վ��
	float  fLon;      // ����
	float  fLat;      // γ��
	float  fHeight;   // �߶�
	int    nRow;      // �и���
	int    nCol;      // �и���
	float* fData;     // ����
}st_station;

/************************************************************************/
/* 2.1  Ԫ�����б���                                                  */
/************************************************************************/
typedef struct _SRC_ITEM
{
	QString strDataType;     // �������ͣ� SCMOC ����ָ����; SPVT ʡָ����; SCMOU Ԥ��ԱԤ��; SPCC ʡ����Ԥ��
	QString strSymbol;       // ��ʶ����
	QString strDescription;  // ����
	QString strSrcFolder;    // Դ�ļ����Ŀ¼
	QString strSrcFileName;  // Դ�ļ�����ʽ
	ST_FILE_FORMAT stFileNameFormat; // Դ�ļ�������
	QStringList lstDDS;   // ���ݷַ��������б�-����ֱ��ת���ļ������
}ST_SRC_ITEM;

// վ��ϲ�˳���б�
typedef QList<ST_SRC_ITEM>  LIST_SRC_ITEM;

/************************************************************************/
/* 2.2  ��ʱ�䶨��ͺϲ�ʱ�䶨��                                      */
/************************************************************************/
typedef struct _REPORTTIME
{
	int  nReportTime;   // ��ʱ��
	int  nSaveHour;     // �ļ�����-ʱ
	int  nSaveMinute;   // �ļ�����-��
	QString strSaveStation;  // �ļ������վ���б�-�����õ��ļ��ж�ȡվ����Ϣ
	QString strSaveRule;     // �ļ��������
	QString strSaveType;     // �������� SCMOC��SPCC
	ST_FILESAVE stSaveFile;  // �ļ�����Ŀ¼������
}ST_REPORTTIME;

typedef QList<ST_REPORTTIME> LIST_REPORT_TIMES; // ʱ�ζ���

/************************************************************************/
/* 2.3 վ�����ö���                                                     */
/************************************************************************/
typedef struct _STATION_CFG
{
	QString  strRole;          // ����ʽ
	QString  strIDSymbol;      // ����ID���
	QString  strCityType;      // �������ͣ����� town; ����� bigcity
	int      nTimeRange;       // ʱ���� - �ļ������ݼ��
	int      nMaxForecastTime; // ���Ԥ��ʱЧ - �ļ������Ԥ��ʱЧ
									 
	ST_SCANTIME    stScanTime;       // ɨ��ʱ��
	LIST_SRC_ITEM  lstSrcItem;       // Ԫ�����б�
	ST_CACHEDSAVE  stCachedSave;     // ����洢����
	LIST_REPORT_TIMES lstReportTimes;// ʱ�δ����б�

}ST_STATION_CFG;

typedef QList<ST_STATION_CFG> LIST_STATION_CFG;  // վ�㴦��������Ϣ�б�

/************************************************************************/
/* 3 վ����Ϣ                                                           */
/************************************************************************/
typedef struct _STATION_INFO
{
	char   stationNo[10];// վ��
	char   stationName[256]; //վ������
	float  fLon;      // ����
	float  fLat;      // γ��
	float  fHeight;   // �߶�
}ST_STATION_INFO;

typedef QList<ST_STATION_INFO> LIST_STATION_INFO; // վ����Ϣ�б�
typedef QHash<QString, ST_STATION_INFO> HASH_STATION_INFO; // վ����Ϣ�б�

/************************************************************************/
/* 4 �ͻ������ݻ�ȡ�ṹ��                                               */
/************************************************************************/
struct nwfd_station_data
{
	char  type[32];      // �������� SCMOC��SNWFD, SPVT, SPCC��SCMOU...
	char  stationNo[10]; // վ��
	float lon;     // ����
	float lat;     // γ��
	float height;  // �߶�
	int   length;  // �������������ݳ��ȣ�
					    
	int year;      // ��
	int month;     // ��
	int day;       // ��
	int hour;      // ʱ
	int minute;    // ��
	int second;    // ��
	int forecast;  // Ԥ��ʱ��
	int timerange; // ʱ����
};

/************************************************************************/
/* 5 ���ݿ��Ӧ��ṹ                                                   */
/************************************************************************/
typedef struct _StationData
{
	int     nID;
	QString strStatus;
	QString strDataType;
	QString strCityType;

	int     nClientID;    // �ͻ��˱��
	int     nStationType; // 1: ����վ��  2��վ���ļ�

	int     nYear;
	int     nMonth;
	int     nDay;
	int     nHour;
	int     nMinute;
	int     nSecond;
	int     nForecastTime;
	int     nTimeRange;

	QString stationNo;      // վ����
	float temperature;      // �¶�
	float humidity;         // ���ʪ��
	float windDirection;    // ����
	float windSpeed; 		// ����
	float airPressure;		// ��ѹ
	float precipitation;	// ��ˮ��
	float cloud;			// ������
	float lCloud;			// ������
	float weather;			// ��������
	float visibility;		// �ܼ���
	float tMax;				// �������
	float tMin;				// �������
	float hMax;				// ������ʪ��
	float hMin;				// ��С���ʪ��
	float precipitation24H;	// 24Сʱ�ۼƽ�ˮ��
	float rProbability12H;	// 12Сʱ�ۼƽ�ˮ��
	float cloud12H;			// 12Сʱ������
	float lCloud12H;		// 12Сʱ������
	float weather12H;		// 12Сʱ��������
	float windDirection12H;	// 12Сʱ����
	float windSpeed12H;		// 12Сʱ����

	QDateTime tCreateTime;	
	QDateTime tStartTime;	
	QDateTime tFinishTime;	
	QString strErrReason;	
	QString strErrLevel;	
}ST_StationData;

//  ���ݷַ������б�
typedef QList<ST_StationData>  LIST_STATIONDATA;

/************************************************************************/
/* 6 վ����Ϣ                                                           */
/************************************************************************/
#define STATION_TYPE_SINGLE     1   // ����վ��
#define STATION_TYPE_FILE       2   // վ���ļ�
#define STATION_TYPE_MULTI      3   // ��վ������
#define STATION_TYPE_MULTI_DIFF 4   //��վ���ֵ����
typedef struct _StationMsg
{
	QString strCityType;
	QString strStationNo;
	QString strDataType;
	int     nClientID;    // �ͻ��˱��
	int     nStationType; // 1: ����վ��  2��վ���ļ�
	int     nYear;
	int     nMonth;
	int     nDay;
	int     nHour;
	int     nMinute;
	int     nSecond;
	int     nStartForecast;  // ��ʼԤ��ʱ��
	int     nForecastTime;   // Ԥ��ʱЧ �� ��ֹԤ��ʱЧ
	int     nTimeRange;
}ST_StationMsg;

//  ���ݷַ������б�
typedef QList<ST_StationMsg>  LIST_STATION_MSG;

/************************************************************************/
/* 7 վ����ת������                                                   */
/************************************************************************/
typedef struct _STATION_MAG
{
	bool bDisabled;         // �����Ƿ����
	QString strStationFile; // վ���ļ�
	QString strCityType;    // ת����վ��ĳ������� town
	QString strForecastType;// ת�����Ԥ������  SPVT
	int nRadius;            // վ��-��㣬Ӱ��뾶 1
	int nValidTime;         // ����������Чʱ��
	bool bIsPublish;        // �Ƿ񷢲�
	bool bIsStation2Grid;   // �Ƿ��Զ�����վ�㵽���Ĵ���
}ST_STATION_MAG;

/************************************************************************/
/* 8 վ�㵥����ֵ                                                       */
/************************************************************************/
typedef struct _STATION_VALUE
{
	QString strStationNo; // վ��
	float lon;     // ����
	float lat;     // γ��
	float height;  // �߶�
	float fValue;  // ֵ
}ST_STATION_VALUE;
typedef QList<ST_STATION_VALUE *> LIST_STATION_VALUE;

#endif // STATION_DEF_H