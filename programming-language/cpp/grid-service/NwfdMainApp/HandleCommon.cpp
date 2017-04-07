#include "HandleCommon.h"

/************************************************************************/
/* �����Ʒʱ������                                                     */
/************************************************************************/
QString HandleCommon::GetCachedProductTime(int nYear, int nMonth, int nDay, int nHour, int nForecastTime, int nTimeRange)
{
	// ��ʽ: YYYYMMDDHH0000.FFF_TT
	QString strProductTime = QString("").sprintf("%04d%02d%02d%02d0000.%03d_%02d", nYear, nMonth, nDay, nHour, nForecastTime, nTimeRange);

	return strProductTime;
}

/************************************************************************/
/* ���������Ʒʱ��                                                     */
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
/* ���������Ʒʱ��-��ʽ2                                               */
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
/* ������Ŀ¼                                                         */
/************************************************************************/
QString HandleCommon::GetFolderPath(QString strSrcPath, int nYear, int nMonth, int nDay)
{
	// �滻�ļ����е�ʱ�䣺{YYYYMMDDHH}
	QString strDateTime = QString("").sprintf("%04d%02d%02d",nYear, nMonth, nDay);

	QString strNewPath = strSrcPath;
    
	// ���滻��
	strNewPath.replace("[YYYYMMDD]", strDateTime);
	strNewPath.replace("[YYYY]", QString("").sprintf("%04d", nYear));
	strNewPath.replace("[YY]", QString("").sprintf("%02d", nYear % 100));
	strNewPath.replace("[MM]", QString("").sprintf("%02d", nMonth));
	strNewPath.replace("[DD]", QString("").sprintf("%02d", nDay));

	return strNewPath;
}

/************************************************************************/
/* ������Ŀ¼                                                         */
/************************************************************************/
QString HandleCommon::GetFolderPath(QString strSrcPath, int nYear, int nMonth, int nDay, QString strType)
{
	QString strNewPath = strSrcPath.replace("{TYPE}", strType);

	// ���滻��
	strNewPath.replace("[TYPE]", strType);

	// ����ʱ��
	return GetFolderPath(strNewPath, nYear, nMonth, nDay);
}

/************************************************************************/
/* ������Ŀ¼                                                         */
/************************************************************************/
QString HandleCommon::GetFolderPath(QString strSrcPath, int nYear, int nMonth, int nDay, int nClientID, QString strKey, QString strType)
{
	// �滻�ļ����е�ʱ�䣺 {CLIENTID},{TYPE}
	QString strPath = strSrcPath;

	// �ͻ��˱�ţ���Ʒ����
	strPath.replace("{CLIENTID}", QString::number(nClientID));   // �ͻ��˱��
	strPath.replace("{KEY}", strKey);  // Key
	strPath.replace("{TYPE}", strType);  // ����

	// �ͻ��˱�ţ���Ʒ����
	strPath.replace("[CLIENTID]", QString::number(nClientID));   // �ͻ��˱��
	strPath.replace("[KEY]", strKey);  // Key
	strPath.replace("[TYPE]", strType);  // ����

	// ����ʱ��
	return GetFolderPath(strPath, nYear, nMonth, nDay);
}

QString HandleCommon::GetFolderPath(QString strSrcPath, int nYear, int nMonth, int nDay, QString rootPath, QString strType, QString strCccc)
{
	QString strNewPath = strSrcPath;

	// �滻��
	strNewPath.replace("[YYYY]", QString("").sprintf("%04d", nYear));
	strNewPath.replace("[YY]", QString("").sprintf("%02d", nYear % 100));
	strNewPath.replace("[MM]", QString("").sprintf("%02d", nMonth));
	strNewPath.replace("[DD]", QString("").sprintf("%02d", nDay));

	strNewPath.replace("[ROOT]",rootPath);
	strNewPath.replace("[TYPE]", strType);
	strNewPath.replace("[CCCC]", strCccc);

	return strNewPath;
}

/************************************************************************/
/* ������Ŀ¼                                                         */
/************************************************************************/
bool HandleCommon::HandleGribFolder(QString strGribPath)
{
	QDir dGrib;
	// ���鵱ǰ�洢Ŀ¼�Ƿ����
	if (dGrib.exists(strGribPath))
	{
		return true;
	}

	// ��·���е� \ ͳһ�滻Ϊ / ; Windows/Linux��֧�� /
	strGribPath.replace("\\", "/");

	QStringList lstPath = strGribPath.split(QString("/"), QString::SkipEmptyParts);  // �ָ��ַ������޳��մ���
	QStringList::iterator iter;
	QString strDir;
	QString strPath;
	for (iter = lstPath.begin(); iter != lstPath.end(); iter++)
	{
		// ��ȡĿ¼����
		strDir = *iter;

		// ��ͷ��ʼ����������·��
		if (iter == lstPath.begin())
		{

			strPath = strDir;
#ifndef _WIN32 
			// Linux�����·���ǴӸ�"/"��ʼ��Ҫ���ϸ�
			if (strGribPath.at(0).toLatin1() == '/')
			{
				strPath = QString("/%2").arg(strDir);
			}
#endif
			// �������ļ�⣨window�µ��̷���Linux�ĸ���
			continue;
		}

		strPath = QString("%1/%2").arg(strPath).arg(strDir);

		//  ��������ڣ����д���
		if (!dGrib.exists(strPath))
		{
			//  ����ʧ��
			if (!dGrib.mkdir(strPath))
			{
				//qDebug() << "����Ŀ¼ʧ��";
				return false;
			}
		}
	}

	return true;
}

/************************************************************************/
/* ����Ŀ¼                                                             */
/************************************************************************/
bool HandleCommon::CreatePath(QString strPath)
{
	QDir d;
	// ���鵱ǰ�洢Ŀ¼�Ƿ����
	if (d.exists(strPath))
	{
		return true;
	}

	// ��·���е� \ ͳһ�滻Ϊ / ; Windows/Linux��֧�� /
	strPath.replace("\\", "/");

	// ����·��
	return d.mkpath(strPath);
}

/************************************************************************/
/* �����ļ���ɸѡ������                                                 */
/************************************************************************/
QString HandleCommon::GetFileFilterName(QString strFileName, int nYear, int nMonth, int nDay, int nHour)
{
	// �滻�ļ����е�ʱ�䣺
	QString strFileFilterName = strFileName;

	// �滻�� �¾����׼��ݲ���
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
	// �滻�ļ����е�ʱ�䣺
	QString strFileFilterName = strFileName;

	// �滻�� �¾����׼��ݲ���
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
	// �滻�ļ����е�ʱ�䣺
	QString strFileFilterName = strFileName;

	// �滻�� �¾����׼��ݲ���
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
	// �滻�ļ����е�ʱ�䣺
	QString strFileFilterName = strFileName;

	// �滻�� �¾����׼��ݲ���
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
	// �滻�ļ����е�ʱ�䣺
	QString strFileFilterName = strFileName;

	// �滻�� �¾����׼��ݲ���
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
/* �����ļ�����                                                         */
/************************************************************************/
QString HandleCommon::GetFileName(QString strSrcFileName, int nYear, int nMonth, int nDay, int nHour)
{
	// �滻�ļ����е�ʱ�䣺{YYYYMMDDHH}
	QString strDateTime;
	strDateTime.sprintf("%04d%02d%02d%02d", nYear, nMonth, nDay, nHour);  // YYYYMMDDHH

	QString strFileName = strSrcFileName;
	strFileName.replace("{YYYYMMDDHH}", strDateTime);
	strFileName.replace("{YYYY}", QString("").sprintf("%04d",nYear));
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

	// �滻�ļ��е�{CCC}
	QString strDateTimeNow = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
	strFileName.replace("{TIME}", strDateTimeNow);
	strFileName.replace("{CCC}", strDateTimeNow);

	strFileName.replace("[TIME]", strDateTimeNow);
	strFileName.replace("[CCC]", strDateTimeNow);

	// �滻�ļ��е�{UUU} // ����ʱ
	QDateTime tUTime = GetUTC(QDateTime::currentDateTime());
	QString strDateTimeUT = tUTime.toString("yyyyMMddhhmmss");
	strFileName.replace("{UTC}", strDateTimeUT);
	strFileName.replace("[UTC]", strDateTimeUT);

	return strFileName;
}
/************************************************************************/
/* �����ļ�����                                                         */
/************************************************************************/
QString HandleCommon::GetFileName(QString strSrcFileName, int nYear, int nMonth, int nDay, int nHour, int nForecasttime, int nTimeRange)
{
	// �滻�ļ����е�ʱ�䣺{YYYYMMDDHH},{FFF},{TT}
	QString strForecast;
	QString strTimeRange;
	QString strFileName = strSrcFileName;

	// Ԥ��ʱ�䣬ʱ��������
	strForecast.sprintf("%03d", nForecasttime);  // FFF
	strTimeRange.sprintf("%02d", nTimeRange);    // TT

	strFileName.replace("{FFF}", strForecast);   // Ԥ��ʱ��
	strFileName.replace("{TT}", strTimeRange);   // ʱ����

	strFileName.replace("[FFF]", strForecast);   // Ԥ��ʱ��
	strFileName.replace("[TT]", strTimeRange);   // ʱ����

	// ����ʱ��
	return GetFileName(strFileName, nYear, nMonth, nDay, nHour);
}

/************************************************************************/
/* �����ļ�����                                                         */
/************************************************************************/
QString HandleCommon::GetFileName(QString strSrcFileName, int nYear, int nMonth, int nDay, int nHour, int nForecasttime, int nTimeRange, int nClientID, QString strType)
{
	// �滻�ļ����е�ʱ�䣺 {CLIENTID},{TYPE}
	QString strFileName = strSrcFileName;

	// �ͻ��˱�ţ���Ʒ����
	strFileName.replace("{CLIENTID}", QString::number(nClientID));   // �ͻ��˱��
	strFileName.replace("{TYPE}", strType);  // ����

	strFileName.replace("[CLIENTID]", QString::number(nClientID));   // �ͻ��˱��
	strFileName.replace("[TYPE]", strType);  // ����

	// ����ʱ��
	return GetFileName(strFileName, nYear, nMonth, nDay, nHour, nForecasttime, nTimeRange);
}

QString HandleCommon::GetFileName(QString strSrcFileName, int nYear, int nMonth, int nDay, QString strType)
{
    // �滻�ļ��е�{CCC}
    QString strDateTimeNow = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
	QString strFileName = strSrcFileName;
    strFileName.replace("[TIME]", strDateTimeNow);
    strFileName.replace("[TYPE]",strType);

	strFileName.replace("[YYYY]", QString("").sprintf("%04d", nYear));
	strFileName.replace("[YY]", QString("").sprintf("%02d", nYear % 100));
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
/* �ж��ļ��Ƿ����                                                           */
/************************************************************************/
bool HandleCommon::Exists(QString strFile)
{
	QDir d;

	// �ж��ļ��Ƿ����
	return d.exists(strFile);
}

/************************************************************************/
/* �ļ�������                                                           */
/************************************************************************/
bool HandleCommon::Rename(QString strOldFileName, QString strNewFileName)
{
	QDir d;
	bool bOK;

	// �ж����ļ������ļ��Ƿ����
	if (d.exists(strNewFileName))
	{
		// ɾ�����ļ������ļ�
		bOK = d.remove(strNewFileName);
	}

	// ���ļ���������Ϊ���ļ���
	bOK = d.rename(strOldFileName, strNewFileName);

	return bOK;
}
/************************************************************************/
/* �����ļ����ƣ��õ���Ӧ��Ϣ                                           */
/************************************************************************/
bool HandleCommon::ParseFileName(QString strFileName, ST_FILE_FORMAT stFileNameFormat, int &nYear, int &nMonth, int &nDay, int &nHour)
{
	// filename demo 2015041008.012
	if (strFileName.isNull() || strFileName.isEmpty())
	{
		return false;
	}

	// �ļ��������ж�
	if (stFileNameFormat.nLength != 0)
	{
		if (strFileName.length() != stFileNameFormat.nLength)
		{
			return false;
		}
	}

	bool bOK;
	// ��
	nYear = strFileName.mid(stFileNameFormat.stYear.iStart, stFileNameFormat.stYear.iLen).toInt(&bOK);
	if (!bOK)
	{
		return false;
	}
	if (stFileNameFormat.stYear.iLen == 2)
	{
		nYear += 2000; // 2λĬ��Ϊ2000���Ժ�
	}

	// ��
	nMonth = strFileName.mid(stFileNameFormat.stMonth.iStart, stFileNameFormat.stMonth.iLen).toInt(&bOK);
	if (!bOK)
	{
		return false;
	}
	// ��
	nDay = strFileName.mid(stFileNameFormat.stDay.iStart, stFileNameFormat.stDay.iLen).toInt(&bOK);
	if (!bOK)
	{
		return false;
	}
	// ʱ
	nHour = strFileName.mid(stFileNameFormat.stHour.iStart, stFileNameFormat.stHour.iLen).toInt(&bOK);
	if (!bOK)
	{
		return false;
	}

	return true;
}

/************************************************************************/
/* �����ļ����ƣ��õ���Ӧ��Ϣ                                           */
/************************************************************************/
bool HandleCommon::ParseFileName(QString strFileName, ST_FILE_FORMAT stFileNameFormat, int &nYear, int &nMonth, int &nDay, int &nHour, int &nForecastTime)
{
	bool bFlg = ParseFileName(strFileName, stFileNameFormat, nYear, nMonth, nDay, nHour);
	if (!bFlg)
	{
		return false;
	}

	bool bOK;
	// Ԥ��ʱ��
	nForecastTime = strFileName.mid(stFileNameFormat.stForcast.iStart, stFileNameFormat.stForcast.iLen).toInt(&bOK);
	if (!bOK)
	{
		return false;
	}

	return true;
}

/************************************************************************/
/* �����ļ����ƣ��õ���Ӧ��Ϣ                                           */
/************************************************************************/
bool HandleCommon::ParseFileName(QString strFileName, ST_FILE_FORMAT stFileNameFormat, int &nYear, int &nMonth, int &nDay, int &nHour, int &nForecastTime, int &nTimeRange)
{
	bool bFlg = ParseFileName(strFileName, stFileNameFormat, nYear, nMonth, nDay, nHour, nForecastTime);
	if (!bFlg)
	{
		return false;
	}

	bool bOK;
	// ʱ����
	nTimeRange = strFileName.mid(stFileNameFormat.stRange.iStart, stFileNameFormat.stRange.iLen).toInt(&bOK);
	if (!bOK)
	{
		return false;
	}

	return true;
}

/************************************************************************/
/* ��ȡ��������                                                         */
/************************************************************************/
QDateTime HandleCommon::GetYesterday()
{
	QDateTime tToday = QDateTime::currentDateTime();
	QDateTime tYesterDaty = tToday.addDays(-1);

	return tYesterDaty;
}
/************************************************************************/
/* ��ȡ����ʱ�����˲���Ϊ����ʱ�䣩                                     */
/************************************************************************/
QDateTime HandleCommon::GetUTC(int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond)
{
	QString  strBjTime = QString("").sprintf("%04d-%02d-%02d %02d:%02d:%02d", nYear, nMonth, nDay, nHour, nMinute, nSecond);
	QDateTime tBjTime = QDateTime::fromString(strBjTime, "yyyy-MM-dd hh:mm:ss");

	// ����ʱ=����ʱ��-8Сʱ
	return tBjTime.addSecs(-28800);
}

/************************************************************************/
/* ��ȡ����ʱ�����˲���Ϊ����ʱ�䣩                                     */
/************************************************************************/
QDateTime HandleCommon::GetUTC(QDateTime tBjTime)
{
	// ����ʱ=����ʱ��-8Сʱ
	return tBjTime.addSecs(-28800);
}

/************************************************************************/
/* ��ȡ����ʱ�����˲���Ϊ����ʱ�䣩                                     */
/************************************************************************/
QDateTime HandleCommon::GetBJT(int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond)
{
	QString  strUTime = QString("").sprintf("%04d-%02d-%02d %02d:%02d:%02d", nYear, nMonth, nDay, nHour, nMinute, nSecond);
	QDateTime tUTime = QDateTime::fromString(strUTime, "yyyy-MM-dd hh:mm:ss");

	// ����ʱ��=����ʱ+8Сʱ
	return tUTime.addSecs(28800);
}

/************************************************************************/
/* ��ȡ����ʱ�����˲���Ϊ����ʱ�䣩                                     */
/************************************************************************/
QDateTime HandleCommon::GetBJT(QDateTime tUTime)
{
	// ����ʱ��=����ʱ+8Сʱ
	return tUTime.addSecs(28800);
}


/************************************************************************/
/* ����ɨ��ģʽ,��ȡ��ֹʱ��                                            */
/************************************************************************/
bool HandleCommon::GetDateTimeFromScanMode(QString strScanMode, QDateTime& tStartDay, QDateTime& tEndDay)
{
	QString  strStartDay;
	QString  strEndDay;

	// ��ǰʱ��
	QDateTime tToday = QDateTime::currentDateTime();

	// ɨ��ģʽ
	if (strScanMode == "day")
	{
		// ����
		strStartDay = QString("").sprintf("%04d%02d%02d %02d:%02d:%02d", tToday.date().year(), tToday.date().month(), tToday.date().day(), 0, 0, 0);
		strEndDay = QString("").sprintf("%04d%02d%02d %02d:%02d:%02d", tToday.date().year(), tToday.date().month(), tToday.date().day(), 23, 59, 59);
	}
	else if (strScanMode == "2day")
	{
		// 2��
		QDateTime tYesterday = tToday.addDays(-1);
		strStartDay = QString("").sprintf("%04d%02d%02d %02d:%02d:%02d", tYesterday.date().year(), tYesterday.date().month(), tYesterday.date().day(), 0, 0, 0);
		strEndDay = QString("").sprintf("%04d%02d%02d %02d:%02d:%02d", tToday.date().year(), tToday.date().month(), tToday.date().day(), 23, 59, 59);
	}
	else
	{
		// ָ�����ڼ�� ��ʽ��20160101-20160405
		QStringList lstDate = strScanMode.split("-");
		if (lstDate.size() != 2)
		{
			// ��ʽ����
			return false;
		}
		QString strStart = lstDate.at(0);
		QString strEnd = lstDate.at(1);
		if (strStart.size() != 8 || strEnd.size() != 8)
		{
			// ��ʽ����
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
/* ��ȡ�ļ�����ʱ��                                                     */
/************************************************************************/
QString HandleCommon::GetFileCreatedTime(QString strFile)
{
	QFileInfo fileInfo(strFile);
	if (!fileInfo.exists())
	{
		return "";
	}

	// ��ȡ�ļ���Ϣ 
	// int nFileSize = fileInfo.size();         // �ļ���С
	QDateTime tFileCreated = fileInfo.created(); // �ļ�����ʱ��
	QString strCreatedTime = tFileCreated.toString("yyyy-MM-dd hh:mm:ss");
	
	return strCreatedTime;
}

/************************************************************************/
/* ��ȡ�ļ�����ɸѡ��                                                   */
/************************************************************************/
QString HandleCommon::GetFileNameWildCard(QString strSrcFileName, int nYear, int nMonth, int nDay, QString strType, int nTimeRange)
{
    // �滻�ļ���
    QString strFileName = strSrcFileName;

    strFileName.replace("[YYYY]", QString("").sprintf("%04d", nYear));
	strFileName.replace("[YY]", QString("").sprintf("%02d", nYear % 100));

    strFileName.replace("[MM]", QString("").sprintf("%02d", nMonth));
    strFileName.replace("[DD]", QString("").sprintf("%02d", nDay));
    strFileName.replace("[HH]", QString("").sprintf("%s", "*"));

	strFileName.replace("[TYPE]", strType);
	strFileName.replace("[TT]", QString("").sprintf("%02d", nTimeRange));

    return strFileName;
}
