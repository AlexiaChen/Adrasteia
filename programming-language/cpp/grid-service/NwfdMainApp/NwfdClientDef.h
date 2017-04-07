/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* �ļ���: NwfdClientDef.h
* ��  ��: zhangl  		�汾��1.0		��  �ڣ�2015/07/13
* ��  �����ͻ������ö���
* ��  ����
* �����б�:
* �޸���־��
*   No.   ����		  ����		       �޸�����
* ------------------------------------------------------------------------
*************************************************************************/
#ifndef NWFD_CLIENT_DEF_H
#define NWFD_CLIENT_DEF_H

#include <QString>
#include <QHash>
#include <QList>
#include <QDateTime>

/************************************************************************/
/* �ͻ���Ȩ������                                                       */
/************************************************************************/
typedef struct _CLIENT
{
	int nClientID;       // �ͻ��˱��
	QString strName;     // ����
	QString strClientIP; // �ͻ���IP
	QString strUserName; // ��֤�û���
	QString strPassWord; // ��֤����
	bool bAllowUpload;   // ��֤Ȩ�ޣ��Ƿ������ϴ�

	QString strGridSavePath; // ������ݱ���Ŀ¼
	QString strGridSaveFile; // ������ݱ�������
	QString strStationSavePath; // վ�����ݱ���Ŀ¼
	QString strStationSaveFile; // վ�����ݱ�������

	int nCachedValidTime;// ���������ݱ�����Ч��-���
}ST_CLIENT_INFO;

// �ͻ���Ȩ�������б�
typedef QHash <int, ST_CLIENT_INFO>  HASH_CLIENT;

/************************************************************************/
/* �ͻ����ϴ����ݣ����ݿ�����                                           */
/************************************************************************/
// ����״̬
//#define MERGE_WAITTING    "WaitProcess"     // �ȴ�����
//#define MERGE_PROCESSING  "processing"      // ������
//#define MERGE_COMPLETE    "complete"        // ��ɴ���
//#define MERGE_ABNORMAL    "abnormal"        // �쳣 

typedef struct _MergeData
{
	int     nMergeID;
	QString strMergeStatus;
	QString strMergeType;  // ���ͣ�G2S,RAINPROC,MERGE
	int     nClientID;
	QString strProductKey;
	QString strRelatedProductKey;
	int     nYear;
	int     nMonth;
	int     nDay;
	int     nHour;
	int     nMinute;
	int     nSecond;
	int     nStartForecastTime;
	int     nForecastTime;
	int     nTimeRange;
	float   fLon1;
	float   fLon2;
	float   fLat1;
	float   fLat2;
	QDateTime tCreateTime;
	QDateTime tStartTime;
	QDateTime tFinishTime;
	QString strErrReason;
	QString strErrLevel;
}ST_MergeData;
//  ���ݷַ������б�
typedef QList<ST_MergeData>  LIST_MERGEDATA;


#endif //NWFD_CLIENT_DEF_H