/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: HandleCommon.h
* 作  者: zhangl  		版本：1.0		日  期：2015/08/20
* 描  述：公共处理方法类
* 其  他：
* 功能列表:
*   1. 文件名称处理
*   2. 文件路径处理
*
* 修改日志：
*   No.   日期		  作者		       修改内容
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
/* 类 名： CDDSBase                                                     */
/* 父 类： QThread                                                      */
/* 说 明： 公共处理方法                                                 */
/* 描 述：                                                              */
/************************************************************************/
class HandleCommon
{
public:
    HandleCommon(){}
    ~HandleCommon(){}

public:
    // 缓存相关处理
    static QString GetCachedProductTime(int nYear, int nMonth, int nDay, int nHour, int nForecastTime, int nTimeRange);
    static bool ParseProductTime(const char * szProductTime, int &nYear, int &nMonth, int &nDay, int &nHour, int &nMinute, int &nSecond, int &nForecastTime, int &nTimeRange);
    static bool ParseProductTime(const char * szProductTime, int &nYear, int &nMonth, int &nDay, int &nHour, int &nForecastTime);

    // 目录相关处理
    static QString GetFolderPath(QString strSrcPath, int nYear, int nMonth, int nDay);
    static QString GetFolderPath(QString strSrcPath, int nYear, int nMonth, int nDay, QString strType);
    static QString GetFolderPath(QString strSrcPath, int nYear, int nMonth, int nDay, QString rootPath, QString strType, QString strCccc);  // for use
    static QString GetFolderPath(QString strSrcPath, int nYear, int nMonth, int nDay, int nClientID, QString strKey, QString strType);

    static bool HandleGribFolder(QString strGribPath);
    static bool CreatePath(QString strPath);

    // 查找文件筛选
    static QString GetFileFilterName(QString strFileName, int nYear, int nMonth, int nDay, int nHour);
    static QString GetFileFilterName(QString strFileName, int nYear, int nMonth, int nDay, int nHour, int nForecastTime);
    static QString GetFileFilterName(QString strFileName, int nYear, int nMonth, int nDay, int nHour, int nForecastTime, int nTimeRange);

    static QString GetFileFilterName(QString strFileName, int nYear, int nMonth, int nDay, int nHour, int nForecastTime, int nTimeRange, QString strType, QString strCccc); // for use
    static QString GetFileFilterName(QString strFileName, int nYear, int nMonth, int nDay, QString strType, QString strCccc); // for use

    // 文件名相关处理
    static QString GetFileName(QString strSrcFileName, int nYear, int nMonth, int nDay, int nHour);
    static QString GetFileName(QString strSrcFileName, int nYear, int nMonth, int nDay, QString strType);
    static QString GetFileName(QString strSrcFileName, int nYear, int nMonth, int nDay, int nHour, int nForecasttime, int nTimeRange);
    static QString GetFileName(QString strSrcFileName, int nYear, int nMonth, int nDay, int nHour, int nForecasttime, int nTimeRange, int nClientID, QString strType);
    static QString GetFileName(QString strSrcFileName, int nYear, int nMonth, int nDay, int nHour, int nForecasttime, int nTimeRange, QString strType, QString strCccc); // for use
    static QString GetFileNameWildCard(QString strSrcFileName, int nYear, int nMonth, int nDay, QString strType, QString strRange);

    static bool Exists(QString strFile);
    static bool Rename(QString strOldFileName, QString strNewFileName);

    // 时间相关处理
    static QDateTime GetYesterday();
    static QDateTime GetUTC(int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond);
    static QDateTime GetUTC(QDateTime tBjTime);
    static QDateTime GetBJT(int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond);
    static QDateTime GetBJT(QDateTime tUTime);

    // 解析扫描模式,获取起止时间
    static bool GetDateTimeFromScanMode(QString strScanMode, QDateTime& tStartDay, QDateTime& tEndDay);

    // 获取文件创建时间
    static QString GetFileCreatedTime(QString strFile);
};


#endif //HANDLE_COMMON_H