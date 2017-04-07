#include "HandleDB.h"
#include <QSqlQuery>
#include <QVariant>

/************************************************************************/
/* ���캯��                                                             */
/************************************************************************/
HandleDB::HandleDB()
{
}


/************************************************************************/
/* ��������                                                             */
/************************************************************************/
HandleDB::~HandleDB()
{
}

/************************************************************************/
/* ����                                                                 */
/************************************************************************/
HandleDB & HandleDB::getClass()
{
	static HandleDB objHandleDB;
	return objHandleDB;
}

/************************************************************************/
/* ��ʼ�����ݿ��еĲ�Ʒ��Ϣ                                             */
/************************************************************************/
int HandleDB::InitProduct(HASH_PRODUCT hasProduct)
{
	QString strSQL;          // SQL���
	QSqlDatabase dbConnect;  // ���ݿ�����
	int nConnectID;          // ���ӹ���ID

	// ʹ�����ӳ��ػ��������л�ȡһ�����ݿ�����
	DBConnGuard  dbConnectGuard(dbConnect, nConnectID);
	if (nConnectID == -1)
	{
		// ���ӹ���IDΪ-1����ʾû�д����ӳسɹ���ȡ������
		return -1;
	}

	// 1�����֮ǰ����Ϣ
	strSQL = QString("delete from Tbl_Prouct");
	QSqlQuery sqlQuery(dbConnect);
	if (!sqlQuery.exec(strSQL))
	{
		// ִ��ʧ��
		return -2;
	}

	// 2�������Ʒ��Ϣ
	HASH_PRODUCT::iterator iter;
	for (iter = hasProduct.begin(); iter != hasProduct.end(); iter++)
	{
		ST_PRODUCT stProduct = iter.value();
		if (stProduct.strRole != "raw")
		{
			continue;
		}

		// 2������Ϣ���뵽���ݿ���
		strSQL = QString("insert into Tbl_Product(ProductKey,ProductName,Type,Publisher,Lon1,Lon2,Lat1,Lat2,Di,Dj,TimeRange,MaxForecastTime,BusinessStatus,IsMerge) values('%1', '%2', '%3', '%4', %5, %6, %7, %8, %9, %10, %11, %12,'%13', 0)")
			.arg(stProduct.strKey).arg(stProduct.strName).arg(stProduct.strType).arg(stProduct.strPublisher).arg(stProduct.fLon1).arg(stProduct.fLon2).arg(stProduct.fLat1).arg(stProduct.fLat2)
			.arg(stProduct.fDi).arg(stProduct.fDj).arg(stProduct.nTimeRange).arg(stProduct.nMaxForecastTime).arg(stProduct.strBusinessStatus);

		if (!sqlQuery.exec(strSQL))
		{
			// ִ��ʧ��
			continue;
		}
	}

	// 3������ƴͼ��Ϣ
	for (iter = hasProduct.begin(); iter != hasProduct.end(); iter++)
	{
		ST_PRODUCT stProduct = iter.value();
		if (stProduct.strRole != "merge")
		{
			continue;
		}

		// 2������Ϣ���뵽���ݿ���
		strSQL = QString("update Tbl_Product set IsMerge = 1 where ProductKey='%1'").arg(stProduct.strRelatedKey);
		if (!sqlQuery.exec(strSQL))
		{
			// ִ��ʧ��
			continue;
		}
	}

	return 0;
}

/************************************************************************/
/* �洢������Ϣ                                                         */
/************************************************************************/
int HandleDB::SaveDDSFileInfo(ST_DDSFILE stDDSFile, int nFileSize, QDateTime tFileCreate, ST_DDS stDDS)
{
	QString strSQL;          // SQL���
	QSqlDatabase dbConnect;  // ���ݿ�����
	int nConnectID;          // ���ӹ���ID

	// ʹ�����ӳ��ػ��������л�ȡһ�����ݿ�����
	DBConnGuard  dbConnectGuard(dbConnect, nConnectID);
	if (nConnectID == -1)
	{
		// ���ӹ���IDΪ-1����ʾû�д����ӳسɹ���ȡ������
		return -1;
	}

	QSqlQuery sqlQuery(dbConnect);

	// ���ǿ�Ʒ��ͣ�����Ҫ�����ļ��Ƿ��Ѿ����͹�
	if (!stDDSFile.bForceFlg)
	{
		// 1����ѯ�Ƿ��Ѿ����ڴ�����Ϣ
		strSQL = QString("select jobID from NwfdDDSDetail where productKey='%1' and fileName='%2' and ddsType='%3' and remoteIP='%4' and remotePath='%5'")
			.arg(stDDSFile.strProductKey).arg(stDDSFile.strFileName).arg(stDDS.strDDSType).arg(stDDS.strRemoteIP).arg(stDDS.strRmotePath);
		if (!sqlQuery.exec(strSQL))
		{
			// ִ��ʧ��
			return -2;
		}
		if (sqlQuery.next())
		{
			// �Ѿ����ڣ����ٲ���
			return 0;
		}
	}

	// 2������Ϣ���뵽���ݿ���
	strSQL = QString("insert into NwfdDDSDetail(jobStatus,productKey,fileName,filePath,fileSize,createTime,ddsType,remoteIP,remotePath,remoteUser,remotePassword,remoteFileName) values('%1', '%2', '%3', '%4', %5, '%6', '%7', '%8', '%9', '%10', '%11', '%12')")
		.arg(WAITTING).arg(stDDSFile.strProductKey).arg(stDDSFile.strFileName).arg(stDDSFile.strFilePath).arg(nFileSize).arg(tFileCreate.toString("yyyy-MM-dd hh:mm:ss.zzz"))
		.arg(stDDS.strDDSType).arg(stDDS.strRemoteIP).arg(stDDS.strRmotePath).arg(stDDS.strUserName).arg(stDDS.strPassword).arg(stDDSFile.strFileName);
	
	if (!sqlQuery.exec(strSQL))
	{
		return -3;
	}

	return 0;
}

/************************************************************************/
/* ��ȡδ���������                                                     */
/************************************************************************/
int HandleDB::GetDDSJob(MAP_JOB & mapJob)
{
	QString strSQL;          // SQL���
	QSqlDatabase dbConnect;  // ���ݿ�����
	int nConnectID;          // ���ӹ���ID

	// ʹ�����ӳ��ػ��������л�ȡһ�����ݿ�����
	DBConnGuard  dbConnectGuard(dbConnect, nConnectID);
	if (nConnectID == -1)
	{
		// ���ӹ���IDΪ-1����ʾû�д����ӳسɹ���ȡ������
		return -1;
	}

	// �����ݿ��в�ѯ�ȴ����������
	strSQL = QString("select jobID,productKey,fileName,filePath,ddsType,remoteIP,remotePath,remoteUser,remotePassword,remoteFileName from NwfdDDSDetail where jobStatus ='WaitProcess'");
	QSqlQuery sqlQuery(dbConnect);
	if (!sqlQuery.exec(strSQL))
	{
		// ִ��ʧ��
		return -2;
	}
	
	while (sqlQuery.next())
	{
		ST_JOB stJob;
		stJob.nJobID = sqlQuery.value(0).toInt();
		stJob.strProductKey = sqlQuery.value(1).toString();
		stJob.strFileName = sqlQuery.value(2).toString();
		stJob.strFilePath = sqlQuery.value(3).toString();
		stJob.strDdsType = sqlQuery.value(4).toString();
		stJob.strRemoteIP = sqlQuery.value(5).toString();
		stJob.strRemotePath = sqlQuery.value(6).toString();
		stJob.strRemoteUser = sqlQuery.value(7).toString();
		stJob.strRemotePassword = sqlQuery.value(8).toString();
		stJob.strRemoteFileName = sqlQuery.value(9).toString();

		// ������ݵ��б���
		QString strKey = QString("%1_%2").arg(stJob.strDdsType).arg(stJob.strRemoteIP);
		if (mapJob.contains(strKey))
		{
			LIST_JOB  *lstJob = mapJob.value(strKey);
			lstJob->push_back(stJob);
		}
		else
		{
			LIST_JOB*  lstJob = new LIST_JOB; 
			lstJob->push_back(stJob);

			mapJob.insert(strKey, lstJob);
		}
	}

	return 0;
}

/************************************************************************/
/* �������ݷַ�������                                                 */
/************************************************************************/
int HandleDB::UpdateJobResult(ST_JOB stJob)
{
	QString strSQL; // SQL���

	if (stJob.strJobStatus == ABNORMAL)
	{
		// ִ���쳣
		strSQL = QString("update NwfdDDSDetail set jobStatus='%1', errReason='%2' where jobID=%3")
			.arg(stJob.strJobStatus).arg(stJob.strErrReason).arg(stJob.nJobID);
	}
	else if (stJob.strJobStatus == COMPLETE)
	{
		// ִ�����
		strSQL = QString("update NwfdDDSDetail set jobStatus='%1', startTime='%2', finishTime='%3',remoteFileSize='%4' where jobID=%5")
			.arg(stJob.strJobStatus).arg(stJob.tStartTime.toString("yyyy-MM-dd hh:mm:ss.zzz")).arg(stJob.tFinishTime.toString("yyyy-MM-dd hh:mm:ss.zzz"))
			.arg(stJob.nRemoteFileSize).arg(stJob.nJobID);
	}
	else
	{
		// ������������
		return 1;
	}

	return ExecSQL(strSQL);
}

/************************************************************************/
/* ɾ�����ݷַ���������                                                 */
/************************************************************************/
int HandleDB::DeleteDDS(QString strDate)
{
	QString strSQL; // SQL���

	// ִ���쳣
	strSQL = QString("delete from NwfdDDSDetail where createTime < '%1'").arg(strDate);

	return ExecSQL(strSQL);
}

/*************************************************************************/
/*  ��  ��: HandleDB                                                     */
/*  ������: ExecSQL                                                      */
/*  ��  ��: ִ��SQL���                                                  */
/*  ����ֵ: �Ƿ�ִ�гɹ�;0:�ɹ�; other:���ɹ�                            */
/*  ��  ��: strSQL: SQL���                                              */
/*************************************************************************/
int HandleDB::ExecSQL(QString strSQL)
{
	QSqlDatabase dbConnect;  // ���ݿ�����
	int nConnectID;          // ���ӹ���ID

	// ʹ�����ӳ��ػ��������л�ȡһ�����ݿ�����
	DBConnGuard  dbConnectGuard(dbConnect, nConnectID);
	if (nConnectID == -1)
	{
		// ���ӹ���IDΪ-1����ʾû�д����ӳسɹ���ȡ������
		return -1;
	}

	// ִ��SQL
	QSqlQuery sqlQuery(dbConnect); // ��¼��
	bool bRtn = sqlQuery.exec(strSQL);
	if (!bRtn)
	{
		return -2;
	}

	return 0;
}

/************************************************************************/
/* ������ݺϲ���Ϣ                                                     */
/************************************************************************/
int HandleDB::SaveMergeDataInfo(int nClientID, QString strProductKey, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, 
	int nForecastTime, int nTimeRange, float fLon1, float fLon2, float fLat1, float fLat2)
{
	QString strSQL;          // SQL���
	QSqlDatabase dbConnect;  // ���ݿ�����
	int nConnectID;          // ���ӹ���ID

	// ʹ�����ӳ��ػ��������л�ȡһ�����ݿ�����
	DBConnGuard  dbConnectGuard(dbConnect, nConnectID);
	if (nConnectID == -1)
	{
		// ���ӹ���IDΪ-1����ʾû�д����ӳسɹ���ȡ������
		return -1;
	}

	// 2������Ϣ���뵽���ݿ���
	strSQL = QString("insert into NwfdMergeDetail(mergeStatus,clientID,productKey,year,month,day,hour,minute,second, forecasttime,timerange,createTime,lon1,lon2,lat1,lat2) values('%1',%2,'%3',%4,%5,%6,%7,%8,%9,%10,%11,'%12',%13,%14,%15,%16)")
		.arg(WAITTING).arg(nClientID).arg(strProductKey).arg(nYear).arg(nMonth).arg(nDay).arg(nHour).arg(nMinute).arg(nSecond)
		.arg(nForecastTime).arg(nTimeRange).arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")).arg(fLon1).arg(fLon2).arg(fLat1).arg(fLat2);

	QSqlQuery sqlQuery(dbConnect);
	if (!sqlQuery.exec(strSQL))
	{
		return -3;
	}

	return 0;
}

/************************************************************************/
/* ������ݺϲ���Ϣ                                                     */
/************************************************************************/
int HandleDB::GetMergeData(QString strProductKey, LIST_MERGEDATA & listMergeData)
{
	QString strSQL;          // SQL���
	QSqlDatabase dbConnect;  // ���ݿ�����
	int nConnectID;          // ���ӹ���ID

	// ʹ�����ӳ��ػ��������л�ȡһ�����ݿ�����
	DBConnGuard  dbConnectGuard(dbConnect, nConnectID);
	if (nConnectID == -1)
	{
		// ���ӹ���IDΪ-1����ʾû�д����ӳسɹ���ȡ������
		return -1;
	}

	// �����ݿ��в�ѯ�ȴ����������
	strSQL = QString("select mergeID, mergeStatus,clientID,productKey,relatedProductKey,messageType,year,month,day,hour, minute,second,startForecastTime,forecasttime,timerange,lon1,lon2,lat1,lat2 from NwfdMergeDetail where mergeStatus ='WaitProcess' and productKey='%1' order by mergeID ASC").arg(strProductKey);
	QSqlQuery sqlQuery(dbConnect);
	if (!sqlQuery.exec(strSQL))
	{
		// ִ��ʧ��
		return -2;
	}

	while (sqlQuery.next())
	{
		ST_MergeData stMerge;
		stMerge.nMergeID = sqlQuery.value(0).toInt();
		stMerge.strMergeStatus = sqlQuery.value(1).toString();
		stMerge.nClientID = sqlQuery.value(2).toInt();
		stMerge.strProductKey = sqlQuery.value(3).toString();
		stMerge.strRelatedProductKey = sqlQuery.value(4).toString();
		stMerge.strMergeType = sqlQuery.value(5).toString();
		stMerge.nYear = sqlQuery.value(6).toInt();
		stMerge.nMonth = sqlQuery.value(7).toInt();
		stMerge.nDay = sqlQuery.value(8).toInt();
		stMerge.nHour = sqlQuery.value(9).toInt();
		stMerge.nMinute = sqlQuery.value(10).toInt();
		stMerge.nSecond = sqlQuery.value(11).toInt();
		stMerge.nStartForecastTime = sqlQuery.value(12).toInt();
		stMerge.nForecastTime = sqlQuery.value(13).toInt();
		stMerge.nTimeRange = sqlQuery.value(14).toInt();
		stMerge.fLon1 = sqlQuery.value(15).toFloat();
		stMerge.fLon2 = sqlQuery.value(16).toFloat();
		stMerge.fLat1 = sqlQuery.value(17).toFloat();
		stMerge.fLat2 = sqlQuery.value(18).toFloat();

		// ������ݵ��б���
		listMergeData.push_back(stMerge);
	}

	return 0;
}


/************************************************************************/
/* ������ݺϲ���Ϣ                                                     */
/************************************************************************/
int HandleDB::UpdateMergeResult(ST_MergeData stMergeData)
{
	QString strSQL; // SQL���

	if (stMergeData.strMergeStatus == ABNORMAL)
	{
		// ִ���쳣
		strSQL = QString("update NwfdMergeDetail set mergeStatus='%1', errReason='%2' where mergeID=%3")
			.arg(stMergeData.strMergeStatus).arg(stMergeData.strErrReason).arg(stMergeData.nMergeID);
	}
	else if (stMergeData.strMergeStatus == COMPLETE)
	{
		// ִ�����
		strSQL = QString("update NwfdMergeDetail set mergeStatus='%1', startTime='%2', finishTime='%3' where mergeID=%4")
			.arg(stMergeData.strMergeStatus).arg(stMergeData.tStartTime.toString("yyyy-MM-dd hh:mm:ss.zzz")).arg(stMergeData.tFinishTime.toString("yyyy-MM-dd hh:mm:ss.zzz"))
			.arg(stMergeData.nMergeID);
	}
	else
	{
		// ������������
		return 1;
	}

	return ExecSQL(strSQL);
}

int HandleDB::UpdateMergeStatus(QString status,int mergeID)
{
	

		QString strSQL;          // SQL���
	QSqlDatabase dbConnect;  // ���ݿ�����
	int nConnectID;          // ���ӹ���ID

	// ʹ�����ӳ��ػ��������л�ȡһ�����ݿ�����
	DBConnGuard  dbConnectGuard(dbConnect, nConnectID);
	if (nConnectID == -1)
	{
		// ���ӹ���IDΪ-1����ʾû�д����ӳسɹ���ȡ������
		return -1;
	}

	// 2������Ϣ���뵽���ݿ���
	strSQL = QString("UPDATE NwfdMergeDetail SET mergeStatus='%1' WHERE mergeID=%2")
		.arg(status).arg(mergeID);

	QSqlQuery sqlQuery(dbConnect);
	if (!sqlQuery.exec(strSQL))
	{
		return -3;
	}

	return 0;
}

/************************************************************************/
/* ����ͻ������ݼ�¼�����ݿ�                                           */
/************************************************************************/
int HandleDB::SaveClientDataInfo(int nClientID, const QString&strProductKey, const QString &strRelatedProductKey, const QString &messageType, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond,
	int nStartForecastTime, int nForecastTime, int nTimeRange, float fLon1, float fLon2, float fLat1, float fLat2)
{
	QString strSQL;          // SQL���
	QSqlDatabase dbConnect;  // ���ݿ�����
	int nConnectID;          // ���ӹ���ID

	// ʹ�����ӳ��ػ��������л�ȡһ�����ݿ�����
	DBConnGuard  dbConnectGuard(dbConnect, nConnectID);
	if (nConnectID == -1)
	{
		// ���ӹ���IDΪ-1����ʾû�д����ӳسɹ���ȡ������
		return -1;
	}

	// 2������Ϣ���뵽���ݿ���
	strSQL = QString("insert into NwfdMergeDetail(mergeStatus,clientID,productKey,relatedProductKey,messageType,year,month,day,hour,minute,second, startForecastTime,forecasttime,timerange,createTime,lon1,lon2,lat1,lat2) values('%1',%2,'%3','%4','%5',%6,%7,%8,%9,%10,%11,%12,%13,%14,'%15',%16,%17,%18,%19)")
		.arg(WAITTING).arg(nClientID).arg(strProductKey).arg(strRelatedProductKey).arg(messageType).arg(nYear).arg(nMonth).arg(nDay).arg(nHour).arg(nMinute).arg(nSecond)
		.arg(nStartForecastTime).arg(nForecastTime).arg(nTimeRange).arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")).arg(fLon1).arg(fLon2).arg(fLat1).arg(fLat2);

	QSqlQuery sqlQuery(dbConnect);
	if (!sqlQuery.exec(strSQL))
	{
		return -3;
	}

	return 0;
}

int HandleDB::GetFirstClientDataInfo(ST_DB_MSG_ITEM* outItem)
{
	QString strSQL;          // SQL���
	QSqlDatabase dbConnect;  // ���ݿ�����
	int nConnectID;          // ���ӹ���ID

	// ʹ�����ӳ��ػ��������л�ȡһ�����ݿ�����
	DBConnGuard  dbConnectGuard(dbConnect, nConnectID);
	if (nConnectID == -1)
	{
		// ���ӹ���IDΪ-1����ʾû�д����ӳسɹ���ȡ������
		return -1;
	}
	
	
	// 2������Ϣ�����ݿ��������
	strSQL = QString("SELECT mergeID,clientID, productKey, relatedProductKey, messageType,   \
	year,month,day, hour, minute, second,startForecastTime,forecasttime,timerange, lon1,lon2, lat1,lat2 FROM       \
	NwfdMergeDetail  WHERE mergeStatus='%1' ORDER BY mergeID ASC").arg(WAITTING);
		

	QSqlQuery sqlQuery(dbConnect);
	if (!sqlQuery.exec(strSQL))
	{
		return -3;
	}

	if (sqlQuery.next())
	{
		
		outItem->mergeID = sqlQuery.value(0).toInt();
		outItem->clientID = sqlQuery.value(1).toInt();
		outItem->productKey = sqlQuery.value(2).toString();
		outItem->relatedProductKey = sqlQuery.value(3).toString();
		outItem->messageType = sqlQuery.value(4).toString();
		
		outItem->year = sqlQuery.value(5).toInt();
		outItem->month = sqlQuery.value(6).toInt();
		outItem->day = sqlQuery.value(7).toInt();
		
		outItem->hour = sqlQuery.value(8).toInt();
		outItem->min = sqlQuery.value(9).toInt();
		outItem->sec = sqlQuery.value(10).toInt();

		outItem->startForecastTime = sqlQuery.value(11).toInt();
		outItem->ForecastTime = sqlQuery.value(12).toInt();

		outItem->timeRange = sqlQuery.value(13).toInt();

		outItem->lon1 = sqlQuery.value(14).toFloat();
		outItem->lon2 = sqlQuery.value(15).toFloat();
		outItem->lat1 = sqlQuery.value(16).toFloat();
		outItem->lat2 = sqlQuery.value(17).toFloat();

		return 0;
	}



	return -3;


}

/************************************************************************/
/* ɾ���ͻ�������                                                       */
/************************************************************************/
int HandleDB::DeleteClientData(QString strDate)
{
	QString strSQL; // SQL���

	// ִ���쳣
	strSQL = QString("delete from NwfdMergeDetail where createTime < '%1'").arg(strDate);

	return ExecSQL(strSQL);
}

/************************************************************************/
/* ɾ������������                                                     */
/************************************************************************/
int HandleDB::DeleteTaskData(QString strDate)
{
	QString strSQL; // SQL���

	// ִ���쳣
	strSQL = QString("delete from Tbl_Task where startTime < '%1'").arg(strDate);

	return ExecSQL(strSQL);
}

/************************************************************************/
/* ����ͻ����ϴ���վ������                                             */
/************************************************************************/
int HandleDB::SaveStationData(int nClientID, QString strCityType, QString strDataType, QString strStationNo, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond,
	int nForecastTime, int nTimeRange, float * fData, int nDataLen)
{
	QString strSQL;          // SQL���
	QSqlDatabase dbConnect;  // ���ݿ�����
	int nConnectID;          // ���ӹ���ID

	// ʹ�����ӳ��ػ��������л�ȡһ�����ݿ�����
	DBConnGuard  dbConnectGuard(dbConnect, nConnectID);
	if (nConnectID == -1)
	{
		// ���ӹ���IDΪ-1����ʾû�д����ӳسɹ���ȡ������
		return -1;
	}

	// 2������Ϣ���뵽���ݿ���
	// վ���������ݱ�������
	float temperature = -1;
	float humidity = -1;
	float windDirection = -1;
	float windSpeed = -1;
	float airPressure = -1;
	float precipitation = -1;
	float cloud = -1;
	float lCloud = -1;
	float weather = -1;
	float visibility = -1;
	float tMax = -1;
	float tMin = -1;
	float hMax = -1;
	float hMin = -1;
	float precipitation24H = -1;
	float rProbability12H = -1;
	float cloud12H = -1;
	float lCloud12H = -1;
	float weather12H = -1;
	float windDirection12H = -1;
	float windSpeed12H = -1;

	int nOff = 0;

	// ���������ж�
	if (strCityType == "bigcity")
	{
		weather = fData[nOff++];
		tMax = fData[nOff++];
		tMin = fData[nOff++];
		windDirection = fData[nOff++];
		windSpeed = fData[nOff++];
		precipitation = fData[nOff++];
	}
	else
	{
		temperature = fData[nOff++];
		humidity = fData[nOff++];
		windDirection = fData[nOff++];
		windSpeed = fData[nOff++];
		airPressure = fData[nOff++];
		precipitation = fData[nOff++];
		cloud = fData[nOff++];
		lCloud = fData[nOff++];
		weather = fData[nOff++];
		visibility = fData[nOff++];
		tMax = fData[nOff++];
		tMin = fData[nOff++];
		hMax = fData[nOff++];
		hMin = fData[nOff++];
		precipitation24H = fData[nOff++];
		rProbability12H = fData[nOff++];
		cloud12H = fData[nOff++];
		lCloud12H = fData[nOff++];
		weather12H = fData[nOff++];
		windDirection12H = fData[nOff++];
		windSpeed12H = fData[nOff++];
	}

	strSQL = QString("insert into NwfdStationSend(status,dataType,cityType,year,month,day,hour,minute,second,forecasttime,timerange,station,temperature,humidity,windDirection,windSpeed,airPressure,precipitation,cloud,lCloud,weather,visibility,tMax,tMin,hMax,hMin,precipitation24H,rProbability12H,cloud12H,lCloud12H,weather12H,windDirection12H,windSpeed12H,createTime,clientID) values('%1','%2','%3',%4,%5,%6,%7,%8,%9,%10,%11,'%12',%13,%14,%15,%16,%17,%18,%19,%20,%21,%22,%23,%24,%25,%26,%27,%28,%29,%30,%31,%32,%33,'%34',%35)")
		.arg(WAITTING).arg(strDataType).arg(strCityType).arg(nYear).arg(nMonth).arg(nDay).arg(nHour).arg(nMinute).arg(nSecond).arg(nForecastTime).arg(nTimeRange).arg(strStationNo)
		.arg(temperature).arg(humidity).arg(windDirection).arg(windSpeed).arg(airPressure).arg(precipitation).arg(cloud).arg(lCloud).arg(weather).arg(visibility)
		.arg(tMax).arg(tMin).arg(hMax).arg(hMin).arg(precipitation24H).arg(rProbability12H).arg(cloud12H).arg(lCloud12H).arg(weather12H).arg(windDirection12H).arg(windSpeed12H)
		.arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")).arg(nClientID);

	QSqlQuery sqlQuery(dbConnect);
	if (!sqlQuery.exec(strSQL))
	{
		return -3;
	}

	return 0;
}

int  HandleDB::GetTaskReprocessMessage(ST_DB_TASK_MSG* msg)
{
	QString strSQL;          // SQL���
	QSqlDatabase dbConnect;  // ���ݿ�����
	int nConnectID;          // ���ӹ���ID

	// ʹ�����ӳ��ػ��������л�ȡһ�����ݿ�����
	DBConnGuard  dbConnectGuard(dbConnect, nConnectID);
	if (nConnectID == -1)
	{
		// ���ӹ���IDΪ-1����ʾû�д����ӳسɹ���ȡ������
		return -1;
	}

	//ֻ��ȡ�������Ϣ
	QDateTime currTime = QDateTime::currentDateTime();
	int year = currTime.toString("yyyy").toInt();
	int month = currTime.toString("MM").toInt();
	int day = currTime.toString("dd").toInt();

		
	// 2������Ϣ�����ݿ��������
	strSQL = QString("SELECT id,productkey,role,status FROM  Tbl_Task  WHERE status='%1' and year=%2 \
					 					 		and month=%3 and day=%4").arg(REPROCESS).arg(year).arg(month).arg(day);


	QSqlQuery sqlQuery(dbConnect);
	if (!sqlQuery.exec(strSQL))
	{
		return -3;
	}

	if (sqlQuery.next())
	{
		msg->id = sqlQuery.value(0).toInt();
		msg->productKey = sqlQuery.value(1).toString();
		msg->productRole = sqlQuery.value(2).toString();
		msg->taskStatus = sqlQuery.value(3).toString();
		msg->errReason = sqlQuery.value(4).toString();

		return 0;
	}

	return -3;
}

int  HandleDB::UpdateTaskMessage(ST_DB_TASK_MSG* msg)
{
	

	QString strSQL;          // SQL���
	QSqlDatabase dbConnect;  // ���ݿ�����
	int nConnectID;          // ���ӹ���ID

	// ʹ�����ӳ��ػ��������л�ȡһ�����ݿ�����
	DBConnGuard  dbConnectGuard(dbConnect, nConnectID);
	if (nConnectID == -1)
	{
		// ���ӹ���IDΪ-1����ʾû�д����ӳسɹ���ȡ������
		return -1;
	}

	
	strSQL = QString("SELECT id FROM  Tbl_Task  WHERE  productkey='%1' and role='%2' and year=%3 \
		  and month=%4 and day=%5 and reporttime=%6 and status='%7'").arg(msg->productKey).arg(msg->productRole).arg(msg->year).arg(msg->month).arg(msg->day).arg(msg->reporttime).arg(REPROCESS);


	QSqlQuery sqlQuery(dbConnect);
	if (!sqlQuery.exec(strSQL))
	{
		return -3;
	}
	
	//����м�¼
	if (sqlQuery.next())
	{
		//ֻ������
		int id = sqlQuery.value(0).toInt();
		QString updateSql = QString("UPDATE Tbl_Task SET status='%1', forecasttime=%2, folder='%3', filename='%4', startTime='%5',\
		finishTime='%6', errReason='%7'	WHERE id=%8").arg(msg->taskStatus).arg(msg->forecasttime).arg(msg->folder).arg(msg->filename).arg(msg->starttime).arg(msg->finishtime).arg(msg->errReason).arg(id);

		QSqlQuery sqlupdate(dbConnect);
		if (!sqlupdate.exec(updateSql))
		{
			return -3;
		}
		
	}
	else
	{
		//���²����µ�һ��
		QString insertSql = QString("INSERT INTO Tbl_Task(productkey,role,status,year,month,day,reporttime,forecasttime,folder,  \
		filename,startTime,finishTime,errReason) VALUES('%1','%2','%3',%4,%5,%6,%7,%8,'%9','%10','%11','%12','%13')").arg(msg->productKey)  \
		.arg(msg->productRole).arg(msg->taskStatus).arg(msg->year).arg(msg->month).arg(msg->day).arg(msg->reporttime).arg(msg->forecasttime)  \
		.arg(msg->folder).arg(msg->filename).arg(msg->starttime).arg(msg->finishtime).arg(msg->errReason);


		QSqlQuery sqlinsert(dbConnect);
		if (!sqlinsert.exec(insertSql))
		{
			return -3;
		}
	}
	
	
	return 0;
}

int HandleDB::DeleteStationRecvData(QString strDate)
{
	QString strSQL; 

	strSQL = QString("delete from NwfdStationRecv where createTime < '%1'").arg(strDate);
	
	return ExecSQL(strSQL);
}

int HandleDB::DeleteMqRecvData(QString strDate)
{
	QString strSQL; // SQL���

	
	strSQL = QString("delete from Tbl_MqRecv where createTime < '%1'").arg(strDate);

	return ExecSQL(strSQL);
}

int HandleDB::DeleteMqSendData(QString strDate)
{
	QString strSQL; // SQL���


	strSQL = QString("delete from Tbl_MqSend where startTime < '%1'").arg(strDate);

	return ExecSQL(strSQL);
}

int HandleDB::DeleteStationSendData(QString strDate)
{
	QString strSQL; // SQL���


	strSQL = QString("delete from NwfdStationSend where startTime < '%1'").arg(strDate);

	return ExecSQL(strSQL);
}

