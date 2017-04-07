#include "HandleCommon.h"

/************************************************************************/
/* 缓存产品时间名称                                                     */
/************************************************************************/
QString HandleCommon::GetCachedProductTime(int nYear, int nMonth, int nDay, int nHour, int nForecastTime, int nTimeRange)
{
    // 格式: YYYYMMDDHH0000.FFF_TT
    QString strProductTime = QString("").sprintf("%04d%02d%02d%02d0000.%03d_%02d", nYear, nMonth, nDay, nHour, nForecastTime, nTimeRange);

    return strProductTime;
}

/************************************************************************/
/* 解析缓存产品时间                                                     */
/************************************************************************/
bool HandleCommon::ParseProductTime(const char * szProductTime, int &nYear, int &nMonth, int &nDay, int &nHour, int &nMinute, int &nSecond, int &nForecastTime, int &nTimeRange)
{
    if (szProductTime == NULL)
    {
        return false;
    }

    if (strlen(szProductTime) < 21)
    {
        return false;
    }

    char szValue[10];
    memset(szValue, 0x00, sizeof(szValue));
    memcpy(szValue, szProductTime + 0, 4);
    nYear = atoi(szValue);
    memset(szValue, 0x00, sizeof(szValue));
    memcpy(szValue, szProductTime + 4, 2);
    nMonth = atoi(szValue);
    memset(szValue, 0x00, sizeof(szValue));
    memcpy(szValue, szProductTime + 6, 2);
    nDay = atoi(szValue);
    memset(szValue, 0x00, sizeof(szValue));
    memcpy(szValue, szProductTime + 8, 2);
    nHour = atoi(szValue);
    memset(szValue, 0x00, sizeof(szValue));
    memcpy(szValue, szProductTime + 10, 2);
    nMinute = atoi(szValue);
    memset(szValue, 0x00, sizeof(szValue));
    memcpy(szValue, szProductTime + 12, 2);
    nSecond = atoi(szValue);
    memset(szValue, 0x00, sizeof(szValue));
    memcpy(szValue, szProductTime + 15, 3);
    nForecastTime = atoi(szValue);
    memset(szValue, 0x00, sizeof(szValue));
    memcpy(szValue, szProductTime + 19, 2);
    nTimeRange = atoi(szValue);

    return true;
}

/************************************************************************/
/* 解析缓存产品时间-格式2                                               */
/************************************************************************/
bool HandleCommon::ParseProductTime(const char * szProductTime, int &nYear, int &nMonth, int &nDay, int &nHour, int &nForecastTime)
{
    if (szProductTime == NULL)
    {
        return false;
    }

    if (strlen(szProductTime) < 15)
    {
        return false;
    }

    char szValue[10];
    memset(szValue, 0x00, sizeof(szValue));
    memcpy(szValue, szProductTime + 0, 4);
    nYear = atoi(szValue);
    memset(szValue, 0x00, sizeof(szValue));
    memcpy(szValue, szProductTime + 4, 2);
    nMonth = atoi(szValue);
    memset(szValue, 0x00, sizeof(szValue));
    memcpy(szValue, szProductTime + 6, 2);
    nDay = atoi(szValue);
    memset(szValue, 0x00, sizeof(szValue));
    memcpy(szValue, szProductTime + 8, 2);
    nHour = atoi(szValue);
    memset(szValue, 0x00, sizeof(szValue));
    memcpy(szValue, szProductTime + 10, 2);
    memset(szValue, 0x00, sizeof(szValue));
    memcpy(szValue, szProductTime + 12, 3);
    nForecastTime = atoi(szValue);

    return true;
}

/************************************************************************/
/* 处理保存目录                                                         */
/************************************************************************/
QString HandleCommon::GetFolderPath(QString strSrcPath, int nYear, int nMonth, int nDay)
{
    // 替换文件名中的时间：{YYYYMMDDHH}
    QString strDateTime = QString("").sprintf("%04d%02d%02d", nYear, nMonth, nDay);

    QString strNewPath = strSrcPath;

    // 新替换符
    strNewPath.replace("[YYYYMMDD]", strDateTime);
    strNewPath.replace("[YYYY]", QString("").sprintf("%04d", nYear));
    strNewPath.replace("[YY]", QString("").sprintf("%02d", nYear % 100));
    strNewPath.replace("[MM]", QString("").sprintf("%02d", nMonth));
    strNewPath.replace("[DD]", QString("").sprintf("%02d", nDay));

    return strNewPath;
}

/************************************************************************/
/* 处理保存目录                                                         */
/************************************************************************/
QString HandleCommon::GetFolderPath(QString strSrcPath, int nYear, int nMonth, int nDay, QString strType)
{
    QString strNewPath = strSrcPath.replace("{TYPE}", strType);

    // 新替换符
    strNewPath.replace("[TYPE]", strType);

    // 处理时间
    return GetFolderPath(strNewPath, nYear, nMonth, nDay);
}

/************************************************************************/
/* 处理保存目录                                                         */
/************************************************************************/
QString HandleCommon::GetFolderPath(QString strSrcPath, int nYear, int nMonth, int nDay, int nClientID, QString strKey, QString strType)
{
    // 替换文件名中的时间： {CLIENTID},{TYPE}
    QString strPath = strSrcPath;

    // 客户端编号，产品类型
    strPath.replace("{CLIENTID}", QString::number(nClientID));   // 客户端编号
    strPath.replace("{KEY}", strKey);  // Key
    strPath.replace("{TYPE}", strType);  // 类型

    // 客户端编号，产品类型
    strPath.replace("[CLIENTID]", QString::number(nClientID));   // 客户端编号
    strPath.replace("[KEY]", strKey);  // Key
    strPath.replace("[TYPE]", strType);  // 类型

    // 处理时间
    return GetFolderPath(strPath, nYear, nMonth, nDay);
}

QString HandleCommon::GetFolderPath(QString strSrcPath, int nYear, int nMonth, int nDay, QString rootPath, QString strType, QString strCccc)
{
    QString strNewPath = strSrcPath;

    // 替换符
    strNewPath.replace("[YYYY]", QString("").sprintf("%04d", nYear));
    strNewPath.replace("[YY]", QString("").sprintf("%02d", nYear % 100));
    strNewPath.replace("[MM]", QString("").sprintf("%02d", nMonth));
    strNewPath.replace("[DD]", QString("").sprintf("%02d", nDay));

    strNewPath.replace("[ROOT]", rootPath);
    strNewPath.replace("[TYPE]", strType);
    strNewPath.replace("[CCCC]", strCccc);

    return strNewPath;
}

/************************************************************************/
/* 处理保存目录                                                         */
/************************************************************************/
bool HandleCommon::HandleGribFolder(QString strGribPath)
{
    QDir dGrib;
    // 检验当前存储目录是否存在
    if (dGrib.exists(strGribPath))
    {
        return true;
    }

    // 将路径中的 \ 统一替换为 / ; Windows/Linux都支持 /
    strGribPath.replace("\\", "/");

    QStringList lstPath = strGribPath.split(QString("/"), QString::SkipEmptyParts);  // 分割字符串（剔除空串）
    QStringList::iterator iter;
    QString strDir;
    QString strPath;
    for (iter = lstPath.begin(); iter != lstPath.end(); iter++)
    {
        // 获取目录名称
        strDir = *iter;

        // 从头开始创建的完整路径
        if (iter == lstPath.begin())
        {

            strPath = strDir;
#ifndef _WIN32 
            // Linux下如何路径是从根"/"开始的要补上根
            if (strGribPath.at(0).toLatin1() == '/')
            {
                strPath = QString("/%2").arg(strDir);
            }
#endif
            // 跳过根的检测（window下的盘符，Linux的根）
            continue;
        }

        strPath = QString("%1/%2").arg(strPath).arg(strDir);

        //  如果不存在，进行创建
        if (!dGrib.exists(strPath))
        {
            //  创建失败
            if (!dGrib.mkdir(strPath))
            {
                //qDebug() << "创建目录失败";
                return false;
            }
        }
    }

    return true;
}

/************************************************************************/
/* 创建目录                                                             */
/************************************************************************/
bool HandleCommon::CreatePath(QString strPath)
{
    QDir d;
    // 检验当前存储目录是否存在
    if (d.exists(strPath))
    {
        return true;
    }

    // 将路径中的 \ 统一替换为 / ; Windows/Linux都支持 /
    strPath.replace("\\", "/");

    // 创建路径
    return d.mkpath(strPath);
}

/************************************************************************/
/* 查找文件的筛选器名称                                                 */
/************************************************************************/
QString HandleCommon::GetFileFilterName(QString strFileName, int nYear, int nMonth, int nDay, int nHour)
{
    // 替换文件名中的时间：
    QString strFileFilterName = strFileName;

    // 替换符 新旧两套兼容并存
    strFileFilterName.replace("[YYYY]", QString("").sprintf("%04d", nYear));
    strFileFilterName.replace("[YY]", QString("").sprintf("%02d", nYear % 100));
    strFileFilterName.replace("[MM]", QString("").sprintf("%02d", nMonth));
    strFileFilterName.replace("[DD]", QString("").sprintf("%02d", nDay));
    strFileFilterName.replace("[HH]", QString("").sprintf("%02d", nHour));
    strFileFilterName.replace("[FFF]", "*");
    strFileFilterName.replace("[TT]", "*");
    strFileFilterName.replace("[yyyymmddhhmiss]", "*");

    strFileFilterName.replace("YYYY", QString("").sprintf("%04d", nYear));
    strFileFilterName.replace("YY", QString("").sprintf("%02d", nYear % 100));
    strFileFilterName.replace("MM", QString("").sprintf("%02d", nMonth));
    strFileFilterName.replace("DD", QString("").sprintf("%02d", nDay));
    strFileFilterName.replace("HH", QString("").sprintf("%02d", nHour));
    strFileFilterName.replace("FFF", "*");
    strFileFilterName.replace("TT", "*");
    strFileFilterName.replace("yyyymmddhhmiss", "*");

    return strFileFilterName;
}

QString HandleCommon::GetFileFilterName(QString strFileName, int nYear, int nMonth, int nDay, int nHour, int nForecastTime)
{
    // 替换文件名中的时间：
    QString strFileFilterName = strFileName;

    // 替换符 新旧两套兼容并存
    strFileFilterName.replace("[YYYY]", QString("").sprintf("%04d", nYear));
    strFileFilterName.replace("[YY]", QString("").sprintf("%02d", nYear % 100));
    strFileFilterName.replace("[MM]", QString("").sprintf("%02d", nMonth));
    strFileFilterName.replace("[DD]", QString("").sprintf("%02d", nDay));
    strFileFilterName.replace("[HH]", QString("").sprintf("%02d", nHour));
    strFileFilterName.replace("[FFF]", QString("").sprintf("%03d", nForecastTime));
    strFileFilterName.replace("[TT]", "*");
    strFileFilterName.replace("[yyyymmddhhmiss]", "*");

    strFileFilterName.replace("YYYY", QString("").sprintf("%04d", nYear));
    strFileFilterName.replace("YY", QString("").sprintf("%02d", nYear % 100));
    strFileFilterName.replace("MM", QString("").sprintf("%02d", nMonth));
    strFileFilterName.replace("DD", QString("").sprintf("%02d", nDay));
    strFileFilterName.replace("HH", QString("").sprintf("%02d", nHour));
    strFileFilterName.replace("FFF", QString("").sprintf("%03d", nForecastTime));
    strFileFilterName.replace("TT", "*");
    strFileFilterName.replace("yyyymmddhhmiss", "*");

    return strFileFilterName;
}

QString HandleCommon::GetFileFilterName(QString strFileName, int nYear, int nMonth, int nDay, int nHour, int nForecastTime, int nTimeRange)
{
    // 替换文件名中的时间：
    QString strFileFilterName = strFileName;

    // 替换符 新旧两套兼容并存
    strFileFilterName.replace("[YYYY]", QString("").sprintf("%04d", nYear));
    strFileFilterName.replace("[YY]", QString("").sprintf("%02d", nYear % 100));
    strFileFilterName.replace("[MM]", QString("").sprintf("%02d", nMonth));
    strFileFilterName.replace("[DD]", QString("").sprintf("%02d", nDay));
    strFileFilterName.replace("[HH]", QString("").sprintf("%02d", nHour));
    strFileFilterName.replace("[FFF]", QString("").sprintf("%03d", nForecastTime));
    strFileFilterName.replace("[TT]", QString("").sprintf("%02d", nTimeRange));
    strFileFilterName.replace("[yyyymmddhhmiss]", "*");

    strFileFilterName.replace("YYYY", QString("").sprintf("%04d", nYear));
    strFileFilterName.replace("YY", QString("").sprintf("%02d", nYear % 100));
    strFileFilterName.replace("MM", QString("").sprintf("%02d", nMonth));
    strFileFilterName.replace("DD", QString("").sprintf("%02d", nDay));
    strFileFilterName.replace("HH", QString("").sprintf("%02d", nHour));
    strFileFilterName.replace("FFF", QString("").sprintf("%03d", nForecastTime));
    strFileFilterName.replace("TT", QString("").sprintf("%02d", nTimeRange));
    strFileFilterName.replace("yyyymmddhhmiss", "*");

    return strFileFilterName;
}
QString HandleCommon::GetFileFilterName(QString strFileName, int nYear, int nMonth, int nDay, int nHour, int nForecastTime, int nTimeRange, QString strType, QString strCccc)
{
    // 替换文件名中的时间：
    QString strFileFilterName = strFileName;

    // 替换符 新旧两套兼容并存
    strFileFilterName.replace("[YYYY]", QString("").sprintf("%04d", nYear));
    strFileFilterName.replace("[YY]", QString("").sprintf("%02d", nYear % 100));
    strFileFilterName.replace("[MM]", QString("").sprintf("%02d", nMonth));
    strFileFilterName.replace("[DD]", QString("").sprintf("%02d", nDay));
    strFileFilterName.replace("[HH]", QString("").sprintf("%02d", nHour));
    strFileFilterName.replace("[FFF]", QString("").sprintf("%03d", nForecastTime));
    strFileFilterName.replace("[TT]", QString("").sprintf("%02d", nTimeRange));
    strFileFilterName.replace("[TYPE]", strType);
    strFileFilterName.replace("[CCCC]", strCccc);

    strFileFilterName.replace("[yyyymmddhhmiss]", "*");

    return strFileFilterName;
}

QString HandleCommon::GetFileFilterName(QString strFileName, int nYear, int nMonth, int nDay, QString strType, QString strCccc)
{
    // 替换文件名中的时间：
    QString strFileFilterName = strFileName;

    // 替换符 新旧两套兼容并存
    strFileFilterName.replace("[YYYY]", QString("").sprintf("%04d", nYear));
    strFileFilterName.replace("[YY]", QString("").sprintf("%02d", nYear % 100));
    strFileFilterName.replace("[MM]", QString("").sprintf("%02d", nMonth));
    strFileFilterName.replace("[DD]", QString("").sprintf("%02d", nDay));
    strFileFilterName.replace("[HH]", "*");
    strFileFilterName.replace("[FFF]", "*");
    strFileFilterName.replace("[TT]", "*");
    strFileFilterName.replace("[TYPE]", strType);
    strFileFilterName.replace("[CCCC]", strCccc);

    strFileFilterName.replace("[yyyymmddhhmiss]", "*");

    return strFileFilterName;
}


/************************************************************************/
/* 构造文件名称                                                         */
/************************************************************************/
QString HandleCommon::GetFileName(QString strSrcFileName, int nYear, int nMonth, int nDay, int nHour)
{
    // 替换文件名中的时间：{YYYYMMDDHH}
    QString strDateTime;
    strDateTime.sprintf("%04d%02d%02d%02d", nYear, nMonth, nDay, nHour);  // YYYYMMDDHH

    QString strFileName = strSrcFileName;
    strFileName.replace("{YYYYMMDDHH}", strDateTime);
    strFileName.replace("{YYYY}", QString("").sprintf("%04d", nYear));
    strFileName.replace("{YY}", QString("").sprintf("%02d", nYear % 100));
    strFileName.replace("{MM}", QString("").sprintf("%02d", nMonth));
    strFileName.replace("{DD}", QString("").sprintf("%02d", nDay));
    strFileName.replace("{HH}", QString("").sprintf("%02d", nHour));

    strFileName.replace("[YYYYMMDDHH]", strDateTime);
    strFileName.replace("[YYYY]", QString("").sprintf("%04d", nYear));
    strFileName.replace("[YY]", QString("").sprintf("%02d", nYear % 100));
    strFileName.replace("[MM]", QString("").sprintf("%02d", nMonth));
    strFileName.replace("[DD]", QString("").sprintf("%02d", nDay));
    strFileName.replace("[HH]", QString("").sprintf("%02d", nHour));

    // 替换文件中的{CCC}
    QString strDateTimeNow = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
    strFileName.replace("{TIME}", strDateTimeNow);
    strFileName.replace("{CCC}", strDateTimeNow);

    strFileName.replace("[TIME]", strDateTimeNow);
    strFileName.replace("[CCC]", strDateTimeNow);

    // 替换文件中的{UUU} // 世界时
    QDateTime tUTime = GetUTC(QDateTime::currentDateTime());
    QString strDateTimeUT = tUTime.toString("yyyyMMddhhmmss");
    strFileName.replace("{UTC}", strDateTimeUT);
    strFileName.replace("[UTC]", strDateTimeUT);

    return strFileName;
}
/************************************************************************/
/* 构造文件名称                                                         */
/************************************************************************/
QString HandleCommon::GetFileName(QString strSrcFileName, int nYear, int nMonth, int nDay, int nHour, int nForecasttime, int nTimeRange)
{
    // 替换文件名中的时间：{YYYYMMDDHH},{FFF},{TT}
    QString strForecast;
    QString strTimeRange;
    QString strFileName = strSrcFileName;

    // 预报时间，时间间隔处理
    strForecast.sprintf("%03d", nForecasttime);  // FFF
    strTimeRange.sprintf("%02d", nTimeRange);    // TT

    strFileName.replace("{FFF}", strForecast);   // 预测时间
    strFileName.replace("{TT}", strTimeRange);   // 时间间隔

    strFileName.replace("[FFF]", strForecast);   // 预测时间
    strFileName.replace("[TT]", strTimeRange);   // 时间间隔

    // 处理时间
    return GetFileName(strFileName, nYear, nMonth, nDay, nHour);
}

/************************************************************************/
/* 构造文件名称                                                         */
/************************************************************************/
QString HandleCommon::GetFileName(QString strSrcFileName, int nYear, int nMonth, int nDay, int nHour, int nForecasttime, int nTimeRange, int nClientID, QString strType)
{
    // 替换文件名中的时间： {CLIENTID},{TYPE}
    QString strFileName = strSrcFileName;

    // 客户端编号，产品类型
    strFileName.replace("{CLIENTID}", QString::number(nClientID));   // 客户端编号
    strFileName.replace("{TYPE}", strType);  // 类型

    strFileName.replace("[CLIENTID]", QString::number(nClientID));   // 客户端编号
    strFileName.replace("[TYPE]", strType);  // 类型

    // 处理时间
    return GetFileName(strFileName, nYear, nMonth, nDay, nHour, nForecasttime, nTimeRange);
}

QString HandleCommon::GetFileName(QString strSrcFileName, int nYear, int nMonth, int nDay, QString strType)
{
    // 替换文件名中的时间：{YYYYMMDDHH}
    QString strDateTime;
    strDateTime.sprintf("%04d%02d%02d%s", nYear, nMonth, nDay, "*");  // YYYYMMDDHH

    QString strFileName = strSrcFileName;
    strFileName.replace("{YYYYMMDDHH}", strDateTime);
    strFileName.replace("{YYYY}", QString("").sprintf("%04d", nYear));
    strFileName.replace("{MM}", QString("").sprintf("%02d", nMonth));
    strFileName.replace("{DD}", QString("").sprintf("%02d", nDay));


    // 替换文件中的{CCC}
    QString strDateTimeNow = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
    strFileName.replace("{TIME}", strDateTimeNow);
    strFileName.replace("{CCC}", strDateTimeNow);

    strFileName.replace("[TIME]", strDateTimeNow);
    strFileName.replace("[CCC]", strDateTimeNow);


    strFileName.replace("[TYPE]", strType);
    strFileName.replace("[YYYYMMDDHH]", strDateTime);
    strFileName.replace("[YYYY]", QString("").sprintf("%04d", nYear));
    strFileName.replace("[MM]", QString("").sprintf("%02d", nMonth));
    strFileName.replace("[DD]", QString("").sprintf("%02d", nDay));

    return strFileName;
}

QString HandleCommon::GetFileName(QString strSrcFileName, int nYear, int nMonth, int nDay, int nHour, int nForecasttime, int nTimeRange, QString strType, QString strCccc)
{
    QString strFileName = strSrcFileName;

    strFileName.replace("[YYYY]", QString("").sprintf("%04d", nYear));
    strFileName.replace("[YY]", QString("").sprintf("%02d", nYear % 100));
    strFileName.replace("[MM]", QString("").sprintf("%02d", nMonth));
    strFileName.replace("[DD]", QString("").sprintf("%02d", nDay));
    strFileName.replace("[HH]", QString("").sprintf("%02d", nHour));
    strFileName.replace("[FFF]", QString("").sprintf("%03d", nForecasttime));
    strFileName.replace("[TT]", QString("").sprintf("%02d", nTimeRange));

    strFileName.replace("[TYPE]", strType);
    strFileName.replace("[CCCC]", strCccc);

    QString strDateTimeNow = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
    strFileName.replace("[TIME]", strDateTimeNow);

    return strFileName;
}

/************************************************************************/
/* 判断文件是否存在                                                           */
/************************************************************************/
bool HandleCommon::Exists(QString strFile)
{
    QDir d;

    // 判断文件是否存在
    return d.exists(strFile);
}

/************************************************************************/
/* 文件重命名                                                           */
/************************************************************************/
bool HandleCommon::Rename(QString strOldFileName, QString strNewFileName)
{
    QDir d;
    bool bOK;

    // 判断新文件名的文件是否存在
    if (d.exists(strNewFileName))
    {
        // 删除新文件名的文件
        bOK = d.remove(strNewFileName);
    }

    // 老文件名重命名为新文件名
    bOK = d.rename(strOldFileName, strNewFileName);

    return bOK;
}


/************************************************************************/
/* 获取昨天日期                                                         */
/************************************************************************/
QDateTime HandleCommon::GetYesterday()
{
    QDateTime tToday = QDateTime::currentDateTime();
    QDateTime tYesterDaty = tToday.addDays(-1);

    return tYesterDaty;
}
/************************************************************************/
/* 获取世界时（传人参数为北京时间）                                     */
/************************************************************************/
QDateTime HandleCommon::GetUTC(int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond)
{
    QString  strBjTime = QString("").sprintf("%04d-%02d-%02d %02d:%02d:%02d", nYear, nMonth, nDay, nHour, nMinute, nSecond);
    QDateTime tBjTime = QDateTime::fromString(strBjTime, "yyyy-MM-dd hh:mm:ss");

    // 世界时=北京时间-8小时
    return tBjTime.addSecs(-28800);
}

/************************************************************************/
/* 获取世界时（传人参数为北京时间）                                     */
/************************************************************************/
QDateTime HandleCommon::GetUTC(QDateTime tBjTime)
{
    // 世界时=北京时间-8小时
    return tBjTime.addSecs(-28800);
}

/************************************************************************/
/* 获取北京时（传人参数为世界时间）                                     */
/************************************************************************/
QDateTime HandleCommon::GetBJT(int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond)
{
    QString  strUTime = QString("").sprintf("%04d-%02d-%02d %02d:%02d:%02d", nYear, nMonth, nDay, nHour, nMinute, nSecond);
    QDateTime tUTime = QDateTime::fromString(strUTime, "yyyy-MM-dd hh:mm:ss");

    // 北京时间=世界时+8小时
    return tUTime.addSecs(28800);
}

/************************************************************************/
/* 获取北京时（传人参数为世界时间）                                     */
/************************************************************************/
QDateTime HandleCommon::GetBJT(QDateTime tUTime)
{
    // 北京时间=世界时+8小时
    return tUTime.addSecs(28800);
}


/************************************************************************/
/* 解析扫描模式,获取起止时间                                            */
/************************************************************************/
bool HandleCommon::GetDateTimeFromScanMode(QString strScanMode, QDateTime& tStartDay, QDateTime& tEndDay)
{
    QString  strStartDay;
    QString  strEndDay;

    // 当前时间
    QDateTime tToday = QDateTime::currentDateTime();

    // 扫描模式
    if (strScanMode == "day")
    {
        // 当天
        strStartDay = QString("").sprintf("%04d%02d%02d %02d:%02d:%02d", tToday.date().year(), tToday.date().month(), tToday.date().day(), 0, 0, 0);
        strEndDay = QString("").sprintf("%04d%02d%02d %02d:%02d:%02d", tToday.date().year(), tToday.date().month(), tToday.date().day(), 23, 59, 59);
    }
    else if (strScanMode == "2day")
    {
        // 2天
        QDateTime tYesterday = tToday.addDays(-1);
        strStartDay = QString("").sprintf("%04d%02d%02d %02d:%02d:%02d", tYesterday.date().year(), tYesterday.date().month(), tYesterday.date().day(), 0, 0, 0);
        strEndDay = QString("").sprintf("%04d%02d%02d %02d:%02d:%02d", tToday.date().year(), tToday.date().month(), tToday.date().day(), 23, 59, 59);
    }
    else
    {
        // 指定日期间隔 格式：20160101-20160405
        QStringList lstDate = strScanMode.split("-");
        if (lstDate.size() != 2)
        {
            // 格式错误
            return false;
        }
        QString strStart = lstDate.at(0);
        QString strEnd = lstDate.at(1);
        if (strStart.size() != 8 || strEnd.size() != 8)
        {
            // 格式错误
            return false;
        }

        strStartDay = QString("").sprintf("%s %02d:%02d:%02d", strStart.toLocal8Bit().data(), 0, 0, 0);
        strEndDay = QString("").sprintf("%s %02d:%02d:%02d", strEnd.toLocal8Bit().data(), 23, 59, 59);
    }

    tStartDay = QDateTime::fromString(strStartDay, "yyyyMMdd hh:mm:ss");
    tEndDay = QDateTime::fromString(strEndDay, "yyyyMMdd hh:mm:ss");

    return true;
}

/************************************************************************/
/* 获取文件创建时间                                                     */
/************************************************************************/
QString HandleCommon::GetFileCreatedTime(QString strFile)
{
    QFileInfo fileInfo(strFile);
    if (!fileInfo.exists())
    {
        return "";
    }

    // 获取文件信息 
    // int nFileSize = fileInfo.size();         // 文件大小
    QDateTime tFileCreated = fileInfo.created(); // 文件创建时间
    QString strCreatedTime = tFileCreated.toString("yyyyMMddhhmmss");

    return strCreatedTime;
}

QString HandleCommon::GetFileNameWildCard(QString strSrcFileName, int nYear, int nMonth, int nDay, QString strType, QString strRange)
{
    // 替换文件名中的时间：{YYYYMMDDHH}
    QString strDateTime;
    strDateTime.sprintf("%04d%02d%02d%s", nYear, nMonth, nDay, "*");  // YYYYMMDDHH

    QString strFileName = strSrcFileName;
    strFileName.replace("{YYYYMMDDHH}", strDateTime);
    strFileName.replace("{YYYY}", QString("").sprintf("%04d", nYear));
    strFileName.replace("{MM}", QString("").sprintf("%02d", nMonth));
    strFileName.replace("{DD}", QString("").sprintf("%02d", nDay));
    strFileName.replace("{HH}", QString("").sprintf("%s", "*"));
    strFileName.replace("{RANGE}", QString("").sprintf("%s", strRange));

    // 替换文件中的{CCC}
    QString strDateTimeNow = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
    strFileName.replace("{TIME}", strDateTimeNow);
    strFileName.replace("{CCC}", strDateTimeNow);

    strFileName.replace("[TIME]", strDateTimeNow);
    strFileName.replace("[CCC]", strDateTimeNow);


    strFileName.replace("[TYPE]", strType);
    strFileName.replace("[YYYYMMDDHH]", strDateTime);
    strFileName.replace("[YYYY]", QString("").sprintf("%04d", nYear));
    strFileName.replace("[MM]", QString("").sprintf("%02d", nMonth));
    strFileName.replace("[DD]", QString("").sprintf("%02d", nDay));
    strFileName.replace("[HH]", QString("").sprintf("%s", "*"));
    strFileName.replace("[RANGE]", QString("").sprintf("%s", strRange));

    return strFileName;
}
