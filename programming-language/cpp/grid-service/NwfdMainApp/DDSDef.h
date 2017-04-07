#ifndef DDS_DEF_H
#define DDS_DEF_H

#include <QString>
#include <QList>
#include <QMap>
#include <QHash>
#include <QDateTime>

/************************************************************************/
/* �궨��                                                               */
/************************************************************************/
#define  MAX_DDS_FILELIST   10000   // ���ݷַ��б����δ�������Ϊ10000��
#define  MAX_EXEC_COUNT     3       // ÿ���ļ���������
/************************************************************************/
/* 1. ���ݷַ���������                                                  */
/************************************************************************/
// 1.1 ���ݷַ�����������
typedef enum _DDS_TYPE
{
	_UNKNOWN = 0,
	_FTP     = 1,
	_MQ      = 2,
	_HTTP	 = 3
}EM_DDS_TYPE;

// 1.2 ���ݷַ�Ŀ�ĵض���
typedef struct _DDS
{
	QString  strID;          // ��ʶ��
	//EM_DDS_TYPE emType;    // ���ݷַ�����������
	QString strDDSType;      // ���ݷַ�����������
	QString strRemoteIP;     // ��������ַ
	QString strUserName;     // �û���
	QString strPassword;     // ����
	QString strRmotePath;    // �ļ��洢��Ŀ¼
}ST_DDS;

// 1.3  ���ݷַ�Ŀ�ĵ��б�
typedef QHash<QString, ST_DDS> HASH_DDS; // dds�б�

/************************************************************************/
/* 2. ���ݷַ���������                                                  */
/************************************************************************/
typedef struct _DDS_CFG
{
	bool bDisabled;  // �Ƿ����
	int  nInterval;  // ɨ��ʱ����
	HASH_DDS hasDDS; // ���ݷַ��б�
}ST_DDS_CFG;

/************************************************************************/
/* 3. ���ݷַ��ļ���Ϣ                                                  */
/************************************************************************/
typedef struct _DDSFILE
{
	QString strProductKey;  // ��ƷKey
	QString strFileName;    // �ļ���
	QString strFilePath;    // �ļ�·��
	QString strDDSID;       // ���ݷַ�Ŀ�ĵر��
	int     nFailedNum;     // ʧ�ܴ���
	bool    bForceFlg;      // ǿ�Ʒ��ͱ��
}ST_DDSFILE;

// ���ݷַ��ļ��б�
typedef QList<ST_DDSFILE>  LIST_DDSFILE;


/************************************************************************/
/* 4. �ַ������б�                                                      */
/************************************************************************/
// ����״̬
//#define JOB_WAITTING    "WaitProcess"     // �ȴ�����
//#define JOB_PROCESSING  "processing"  // ������
//#define JOB_COMPLETE    "complete"    // ��ɴ���
//#define JOB_ABNORMAL    "abnormal"    // �쳣 

typedef struct _JOB
{
	int  nJobID;
	QString strJobStatus;
	QString strProductKey;
	QString strFileName;
	QString strFilePath;
	int nFileSize;
	QDateTime tCreateTime;
	QDateTime tStartTime;
	QDateTime tFinishTime;
	QString strDdsType;
	QString strRemoteIP;
	QString strRemotePath;
	QString strRemoteUser;
	QString strRemotePassword;
	QString strRemoteFileName;
	int nRemoteFileSize;
	QString strErrReason;
	QString strErrLevel ;
}ST_JOB;
//  ���ݷַ������б�
typedef QList<ST_JOB>  LIST_JOB;

// �����б�
typedef QMap<QString, LIST_JOB*> MAP_JOB;  // ��������Ŀ�ĵط���


#endif //DDS_DEF_H
