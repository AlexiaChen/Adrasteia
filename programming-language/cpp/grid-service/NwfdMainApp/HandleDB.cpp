#include "HandleDB.h"
#include <QSqlQuery>
#include <QVariant>

/************************************************************************/
/* 构造函数                                                             */
/************************************************************************/
HandleDB::HandleDB()
{
}


/************************************************************************/
/* 析构函数                                                             */
/************************************************************************/
HandleDB::~HandleDB()
{
}

/************************************************************************/
/* 单例                                                                 */
/************************************************************************/
HandleDB & HandleDB::getClass()
{
	static HandleDB objHandleDB;
	return objHandleDB;
}

/************************************************************************/
/* 初始化数据库中的产品信息                                             */
/************************************************************************/
int HandleDB::InitProduct(HASH_PRODUCT hasProduct)
{
	QString strSQL;          // SQL语句
	QSqlDatabase dbConnect;  // 数据库连接
	int nConnectID;          // 连接管理ID

	// 使用连接池守护从连接中获取一个数据库连接
	DBConnGuard  dbConnectGuard(dbConnect, nConnectID);
	if (nConnectID == -1)
	{
		// 连接管理ID为-1，表示没有从连接池成功获取到连接
		return -1;
	}

	// 1、清空之前的信息
	strSQL = QString("delete from Tbl_Prouct");
	QSqlQuery sqlQuery(dbConnect);
	if (!sqlQuery.exec(strSQL))
	{
		// 执行失败
		return -2;
	}

	// 2、插入产品信息
	HASH_PRODUCT::iterator iter;
	for (iter = hasProduct.begin(); iter != hasProduct.end(); iter++)
	{
		ST_PRODUCT stProduct = iter.value();
		if (stProduct.strRole != "raw")
		{
			continue;
		}

		// 2、将信息插入到数据库中
		strSQL = QString("insert into Tbl_Product(ProductKey,ProductName,Type,Publisher,Lon1,Lon2,Lat1,Lat2,Di,Dj,TimeRange,MaxForecastTime,BusinessStatus,IsMerge) values('%1', '%2', '%3', '%4', %5, %6, %7, %8, %9, %10, %11, %12,'%13', 0)")
			.arg(stProduct.strKey).arg(stProduct.strName).arg(stProduct.strType).arg(stProduct.strPublisher).arg(stProduct.fLon1).arg(stProduct.fLon2).arg(stProduct.fLat1).arg(stProduct.fLat2)
			.arg(stProduct.fDi).arg(stProduct.fDj).arg(stProduct.nTimeRange).arg(stProduct.nMaxForecastTime).arg(stProduct.strBusinessStatus);

		if (!sqlQuery.exec(strSQL))
		{
			// 执行失败
			continue;
		}
	}

	// 3、更新拼图信息
	for (iter = hasProduct.begin(); iter != hasProduct.end(); iter++)
	{
		ST_PRODUCT stProduct = iter.value();
		if (stProduct.strRole != "merge")
		{
			continue;
		}

		// 2、将信息插入到数据库中
		strSQL = QString("update Tbl_Product set IsMerge = 1 where ProductKey='%1'").arg(stProduct.strRelatedKey);
		if (!sqlQuery.exec(strSQL))
		{
			// 执行失败
			continue;
		}
	}

	return 0;
}

/************************************************************************/
/* 存储基本信息                                                         */
/************************************************************************/
int HandleDB::SaveDDSFileInfo(ST_DDSFILE stDDSFile, int nFileSize, QDateTime tFileCreate, ST_DDS stDDS)
{
	QString strSQL;          // SQL语句
	QSqlDatabase dbConnect;  // 数据库连接
	int nConnectID;          // 连接管理ID

	// 使用连接池守护从连接中获取一个数据库连接
	DBConnGuard  dbConnectGuard(dbConnect, nConnectID);
	if (nConnectID == -1)
	{
		// 连接管理ID为-1，表示没有从连接池成功获取到连接
		return -1;
	}

	QSqlQuery sqlQuery(dbConnect);

	// 如果强制发送，不需要检查此文件是否已经发送过
	if (!stDDSFile.bForceFlg)
	{
		// 1、查询是否已经存在此条信息
		strSQL = QString("select jobID from NwfdDDSDetail where productKey='%1' and fileName='%2' and ddsType='%3' and remoteIP='%4' and remotePath='%5'")
			.arg(stDDSFile.strProductKey).arg(stDDSFile.strFileName).arg(stDDS.strDDSType).arg(stDDS.strRemoteIP).arg(stDDS.strRmotePath);
		if (!sqlQuery.exec(strSQL))
		{
			// 执行失败
			return -2;
		}
		if (sqlQuery.next())
		{
			// 已经存在，不再插入
			return 0;
		}
	}

	// 2、将信息插入到数据库中
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
/* 获取未处理的数据                                                     */
/************************************************************************/
int HandleDB::GetDDSJob(MAP_JOB & mapJob)
{
	QString strSQL;          // SQL语句
	QSqlDatabase dbConnect;  // 数据库连接
	int nConnectID;          // 连接管理ID

	// 使用连接池守护从连接中获取一个数据库连接
	DBConnGuard  dbConnectGuard(dbConnect, nConnectID);
	if (nConnectID == -1)
	{
		// 连接管理ID为-1，表示没有从连接池成功获取到连接
		return -1;
	}

	// 从数据库中查询等待处理的数据
	strSQL = QString("select jobID,productKey,fileName,filePath,ddsType,remoteIP,remotePath,remoteUser,remotePassword,remoteFileName from NwfdDDSDetail where jobStatus ='WaitProcess'");
	QSqlQuery sqlQuery(dbConnect);
	if (!sqlQuery.exec(strSQL))
	{
		// 执行失败
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

		// 添加数据到列表中
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
/* 更新数据分发处理结果                                                 */
/************************************************************************/
int HandleDB::UpdateJobResult(ST_JOB stJob)
{
	QString strSQL; // SQL语句

	if (stJob.strJobStatus == ABNORMAL)
	{
		// 执行异常
		strSQL = QString("update NwfdDDSDetail set jobStatus='%1', errReason='%2' where jobID=%3")
			.arg(stJob.strJobStatus).arg(stJob.strErrReason).arg(stJob.nJobID);
	}
	else if (stJob.strJobStatus == COMPLETE)
	{
		// 执行完成
		strSQL = QString("update NwfdDDSDetail set jobStatus='%1', startTime='%2', finishTime='%3',remoteFileSize='%4' where jobID=%5")
			.arg(stJob.strJobStatus).arg(stJob.tStartTime.toString("yyyy-MM-dd hh:mm:ss.zzz")).arg(stJob.tFinishTime.toString("yyyy-MM-dd hh:mm:ss.zzz"))
			.arg(stJob.nRemoteFileSize).arg(stJob.nJobID);
	}
	else
	{
		// 其他，不处理
		return 1;
	}

	return ExecSQL(strSQL);
}

/************************************************************************/
/* 删除数据分发过期数据                                                 */
/************************************************************************/
int HandleDB::DeleteDDS(QString strDate)
{
	QString strSQL; // SQL语句

	// 执行异常
	strSQL = QString("delete from NwfdDDSDetail where createTime < '%1'").arg(strDate);

	return ExecSQL(strSQL);
}

/*************************************************************************/
/*  类  名: HandleDB                                                     */
/*  方法名: ExecSQL                                                      */
/*  功  能: 执行SQL语句                                                  */
/*  返回值: 是否执行成功;0:成功; other:不成功                            */
/*  参  数: strSQL: SQL语句                                              */
/*************************************************************************/
int HandleDB::ExecSQL(QString strSQL)
{
	QSqlDatabase dbConnect;  // 数据库连接
	int nConnectID;          // 连接管理ID

	// 使用连接池守护从连接中获取一个数据库连接
	DBConnGuard  dbConnectGuard(dbConnect, nConnectID);
	if (nConnectID == -1)
	{
		// 连接管理ID为-1，表示没有从连接池成功获取到连接
		return -1;
	}

	// 执行SQL
	QSqlQuery sqlQuery(dbConnect); // 记录集
	bool bRtn = sqlQuery.exec(strSQL);
	if (!bRtn)
	{
		return -2;
	}

	return 0;
}

/************************************************************************/
/* 添加数据合并信息                                                     */
/************************************************************************/
int HandleDB::SaveMergeDataInfo(int nClientID, QString strProductKey, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, 
	int nForecastTime, int nTimeRange, float fLon1, float fLon2, float fLat1, float fLat2)
{
	QString strSQL;          // SQL语句
	QSqlDatabase dbConnect;  // 数据库连接
	int nConnectID;          // 连接管理ID

	// 使用连接池守护从连接中获取一个数据库连接
	DBConnGuard  dbConnectGuard(dbConnect, nConnectID);
	if (nConnectID == -1)
	{
		// 连接管理ID为-1，表示没有从连接池成功获取到连接
		return -1;
	}

	// 2、将信息插入到数据库中
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
/* 添加数据合并信息                                                     */
/************************************************************************/
int HandleDB::GetMergeData(QString strProductKey, LIST_MERGEDATA & listMergeData)
{
	QString strSQL;          // SQL语句
	QSqlDatabase dbConnect;  // 数据库连接
	int nConnectID;          // 连接管理ID

	// 使用连接池守护从连接中获取一个数据库连接
	DBConnGuard  dbConnectGuard(dbConnect, nConnectID);
	if (nConnectID == -1)
	{
		// 连接管理ID为-1，表示没有从连接池成功获取到连接
		return -1;
	}

	// 从数据库中查询等待处理的数据
	strSQL = QString("select mergeID, mergeStatus,clientID,productKey,relatedProductKey,messageType,year,month,day,hour, minute,second,startForecastTime,forecasttime,timerange,lon1,lon2,lat1,lat2 from NwfdMergeDetail where mergeStatus ='WaitProcess' and productKey='%1' order by mergeID ASC").arg(strProductKey);
	QSqlQuery sqlQuery(dbConnect);
	if (!sqlQuery.exec(strSQL))
	{
		// 执行失败
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

		// 添加数据到列表中
		listMergeData.push_back(stMerge);
	}

	return 0;
}


/************************************************************************/
/* 添加数据合并信息                                                     */
/************************************************************************/
int HandleDB::UpdateMergeResult(ST_MergeData stMergeData)
{
	QString strSQL; // SQL语句

	if (stMergeData.strMergeStatus == ABNORMAL)
	{
		// 执行异常
		strSQL = QString("update NwfdMergeDetail set mergeStatus='%1', errReason='%2' where mergeID=%3")
			.arg(stMergeData.strMergeStatus).arg(stMergeData.strErrReason).arg(stMergeData.nMergeID);
	}
	else if (stMergeData.strMergeStatus == COMPLETE)
	{
		// 执行完成
		strSQL = QString("update NwfdMergeDetail set mergeStatus='%1', startTime='%2', finishTime='%3' where mergeID=%4")
			.arg(stMergeData.strMergeStatus).arg(stMergeData.tStartTime.toString("yyyy-MM-dd hh:mm:ss.zzz")).arg(stMergeData.tFinishTime.toString("yyyy-MM-dd hh:mm:ss.zzz"))
			.arg(stMergeData.nMergeID);
	}
	else
	{
		// 其他，不处理
		return 1;
	}

	return ExecSQL(strSQL);
}

int HandleDB::UpdateMergeStatus(QString status,int mergeID)
{
	

		QString strSQL;          // SQL语句
	QSqlDatabase dbConnect;  // 数据库连接
	int nConnectID;          // 连接管理ID

	// 使用连接池守护从连接中获取一个数据库连接
	DBConnGuard  dbConnectGuard(dbConnect, nConnectID);
	if (nConnectID == -1)
	{
		// 连接管理ID为-1，表示没有从连接池成功获取到连接
		return -1;
	}

	// 2、将信息插入到数据库中
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
/* 保存客户端数据记录到数据库                                           */
/************************************************************************/
int HandleDB::SaveClientDataInfo(int nClientID, const QString&strProductKey, const QString &strRelatedProductKey, const QString &messageType, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond,
	int nStartForecastTime, int nForecastTime, int nTimeRange, float fLon1, float fLon2, float fLat1, float fLat2)
{
	QString strSQL;          // SQL语句
	QSqlDatabase dbConnect;  // 数据库连接
	int nConnectID;          // 连接管理ID

	// 使用连接池守护从连接中获取一个数据库连接
	DBConnGuard  dbConnectGuard(dbConnect, nConnectID);
	if (nConnectID == -1)
	{
		// 连接管理ID为-1，表示没有从连接池成功获取到连接
		return -1;
	}

	// 2、将信息插入到数据库中
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
	QString strSQL;          // SQL语句
	QSqlDatabase dbConnect;  // 数据库连接
	int nConnectID;          // 连接管理ID

	// 使用连接池守护从连接中获取一个数据库连接
	DBConnGuard  dbConnectGuard(dbConnect, nConnectID);
	if (nConnectID == -1)
	{
		// 连接管理ID为-1，表示没有从连接池成功获取到连接
		return -1;
	}
	
	
	// 2、将信息从数据库升序读出
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
/* 删除客户端数据                                                       */
/************************************************************************/
int HandleDB::DeleteClientData(QString strDate)
{
	QString strSQL; // SQL语句

	// 执行异常
	strSQL = QString("delete from NwfdMergeDetail where createTime < '%1'").arg(strDate);

	return ExecSQL(strSQL);
}

/************************************************************************/
/* 删除任务监控数据                                                     */
/************************************************************************/
int HandleDB::DeleteTaskData(QString strDate)
{
	QString strSQL; // SQL语句

	// 执行异常
	strSQL = QString("delete from Tbl_Task where startTime < '%1'").arg(strDate);

	return ExecSQL(strSQL);
}

/************************************************************************/
/* 保存客户端上传的站点数据                                             */
/************************************************************************/
int HandleDB::SaveStationData(int nClientID, QString strCityType, QString strDataType, QString strStationNo, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond,
	int nForecastTime, int nTimeRange, float * fData, int nDataLen)
{
	QString strSQL;          // SQL语句
	QSqlDatabase dbConnect;  // 数据库连接
	int nConnectID;          // 连接管理ID

	// 使用连接池守护从连接中获取一个数据库连接
	DBConnGuard  dbConnectGuard(dbConnect, nConnectID);
	if (nConnectID == -1)
	{
		// 连接管理ID为-1，表示没有从连接池成功获取到连接
		return -1;
	}

	// 2、将信息插入到数据库中
	// 站点气象数据变量定义
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

	// 城市类型判断
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
	QString strSQL;          // SQL语句
	QSqlDatabase dbConnect;  // 数据库连接
	int nConnectID;          // 连接管理ID

	// 使用连接池守护从连接中获取一个数据库连接
	DBConnGuard  dbConnectGuard(dbConnect, nConnectID);
	if (nConnectID == -1)
	{
		// 连接管理ID为-1，表示没有从连接池成功获取到连接
		return -1;
	}

	//只获取今天的消息
	QDateTime currTime = QDateTime::currentDateTime();
	int year = currTime.toString("yyyy").toInt();
	int month = currTime.toString("MM").toInt();
	int day = currTime.toString("dd").toInt();

		
	// 2、将信息从数据库升序读出
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
	

	QString strSQL;          // SQL语句
	QSqlDatabase dbConnect;  // 数据库连接
	int nConnectID;          // 连接管理ID

	// 使用连接池守护从连接中获取一个数据库连接
	DBConnGuard  dbConnectGuard(dbConnect, nConnectID);
	if (nConnectID == -1)
	{
		// 连接管理ID为-1，表示没有从连接池成功获取到连接
		return -1;
	}

	
	strSQL = QString("SELECT id FROM  Tbl_Task  WHERE  productkey='%1' and role='%2' and year=%3 \
		  and month=%4 and day=%5 and reporttime=%6 and status='%7'").arg(msg->productKey).arg(msg->productRole).arg(msg->year).arg(msg->month).arg(msg->day).arg(msg->reporttime).arg(REPROCESS);


	QSqlQuery sqlQuery(dbConnect);
	if (!sqlQuery.exec(strSQL))
	{
		return -3;
	}
	
	//如果有记录
	if (sqlQuery.next())
	{
		//只做更新
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
		//重新插入新的一条
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
	QString strSQL; // SQL语句

	
	strSQL = QString("delete from Tbl_MqRecv where createTime < '%1'").arg(strDate);

	return ExecSQL(strSQL);
}

int HandleDB::DeleteMqSendData(QString strDate)
{
	QString strSQL; // SQL语句


	strSQL = QString("delete from Tbl_MqSend where startTime < '%1'").arg(strDate);

	return ExecSQL(strSQL);
}

int HandleDB::DeleteStationSendData(QString strDate)
{
	QString strSQL; // SQL语句


	strSQL = QString("delete from NwfdStationSend where startTime < '%1'").arg(strDate);

	return ExecSQL(strSQL);
}

