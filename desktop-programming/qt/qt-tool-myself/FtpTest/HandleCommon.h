/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* �ļ���: HandleCommon.h
* ��  ��: zhangl  		�汾��1.0		��  �ڣ�2015/08/20
* ��  ����������������
* ��  ����
* �����б�:
*   1. �ļ����ƴ���
*   2. �ļ�·������
*
* �޸���־��
*   No.   ����		  ����		       �޸�����
* ------------------------------------------------------------------------
*************************************************************************/
#ifndef HANDLE_COMMON_H
#define HANDLE_COMMON_H

#include <QString>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>

/************************************************************************/
/* �� ���� CDDSBase                                                     */
/* �� �ࣺ QThread                                                      */
/* ˵ ���� ����������                                                 */
/* �� ����                                                              */
/************************************************************************/
class HandleCommon
{
public:
	HandleCommon(){}
	~HandleCommon(){}

public:
	// ������ش���
	static QString GetCachedProductTime(int nYear, int nMonth, int nDay, int nHour, int nForecastTime, int nTimeRange);
	static bool ParseProductTime(const char * szProductTime, int &nYear, int &nMonth, int &nDay, int &nHour, int &nMinute, int &nSecond, int &nForecastTime, int &nTimeRange);
	static bool ParseProductTime(const char * szProductTime, int &nYear, int &nMonth, int &nDay, int &nHour, int &nForecastTime);

	// Ŀ¼��ش���
	static QString GetFolderPath(QString strSrcPath, int nYear, int nMonth, int nDay);
	static QString GetFolderPath(QString strSrcPath, int nYear, int nMonth, int nDay, QString strType);
	static QString GetFolderPath(QString strSrcPath, int nYear, int nMonth, int nDay, QString rootPath, QString strType, QString strCccc);  // for use
	static QString GetFolderPath(QString strSrcPath, int nYear, int nMonth, int nDay, int nClientID, QString strKey, QString strType);

	static bool HandleGribFolder(QString strGribPath);
	static bool CreatePath(QString strPath);

	// �����ļ�ɸѡ
	static QString GetFileFilterName(QString strFileName, int nYear, int nMonth, int nDay, int nHour);
	static QString GetFileFilterName(QString strFileName, int nYear, int nMonth, int nDay, int nHour, int nForecastTime);
	static QString GetFileFilterName(QString strFileName, int nYear, int nMonth, int nDay, int nHour, int nForecastTime, int nTimeRange);
	
	static QString GetFileFilterName(QString strFileName, int nYear, int nMonth, int nDay, int nHour, int nForecastTime, int nTimeRange, QString strType, QString strCccc); // for use
	static QString GetFileFilterName(QString strFileName, int nYear, int nMonth, int nDay, QString strType, QString strCccc); // for use

	// �ļ�����ش���
	static QString GetFileName(QString strSrcFileName, int nYear, int nMonth, int nDay, int nHour);
	static QString GetFileName(QString strSrcFileName, int nYear, int nMonth, int nDay, QString strType);
	static QString GetFileName(QString strSrcFileName, int nYear, int nMonth, int nDay, int nHour, int nForecasttime, int nTimeRange);
	static QString GetFileName(QString strSrcFileName, int nYear, int nMonth, int nDay, int nHour, int nForecasttime, int nTimeRange, int nClientID, QString strType);
	static QString GetFileName(QString strSrcFileName, int nYear, int nMonth, int nDay, int nHour, int nForecasttime, int nTimeRange, QString strType, QString strCccc); // for use
    static QString GetFileNameWildCard(QString strSrcFileName, int nYear, int nMonth, int nDay, QString strType, QString strRange);
	
    static bool Exists(QString strFile);
	static bool Rename(QString strOldFileName, QString strNewFileName);


	// ʱ����ش���
	static QDateTime GetYesterday();
	static QDateTime GetUTC(int nYear, int nMonth, int nDay, int nHour, int nMinute,int nSecond);
	static QDateTime GetUTC(QDateTime tBjTime);
	static QDateTime GetBJT(int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond);
	static QDateTime GetBJT(QDateTime tUTime);

	// ����ɨ��ģʽ,��ȡ��ֹʱ��
	static bool GetDateTimeFromScanMode(QString strScanMode, QDateTime& tStartDay, QDateTime& tEndDay);

	// ��ȡ�ļ�����ʱ��
	static QString GetFileCreatedTime(QString strFile);
};


#endif //HANDLE_COMMON_H