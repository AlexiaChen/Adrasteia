/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: DateBaseSQLDef.h
* 作  者: zhangl		版本：1.0		日  期：2015/07/06
* 描  述: 数据库SQL操作语句定义
* 其  他:
* 功能列表:
*   1.
* 修改日志：
*   No. 日期		作者		修改内容
*
*************************************************************************/

#ifndef DATEBASESQLDEF_H
#define DATEBASESQLDEF_H

// 检测通信状态使用的语句
#define CHECK_CONNECT_STATE  "SELECT WS_NO FROM Tbl_WorkStation"

// WaitProcess   processing   complete  abnormal

// 数据分发详情
//CREATE TABLE NwfdDDSDetail
//(
//jobID INTEGER  primary key,
//jobStatus  TEXT,
//productKey TEXT,
//fileName   TEXT,
//filePath   TEXT,
//fileSize   INTEGER,
//createTime TEXT,
//startTime  TEXT,
//finishTime TEXT,
//
//ddsType     TEXT,
//remoteIP    TEXT,
//remotePath  TEXT,
//remoteUser  TEXT,
//remotePassword TEXT,
//remoteFileName TEXT,
//remoteFileSize INTEGER,
//
//errReason TEXT,
//errLevel  TEXT
//);

//insert into NwfdDDSDetail values
//(1, "未处理", "fsol_10km_ttt03", "abc.grb2", "/data/nwfd/fsol", 100, 0, 0, 0, "ftp", "10.10.10.10", "/", "user", "password", "edf.grb2", 100, "", "");

// 客户端上传详情
//CREATE TABLE NwfdMergeDetail
//(
//mergeID INTEGER  primary key,
//mergeStatus  TEXT,
//clientID    INTEGER,
//productKey  TEXT,
//year        INTEGER,
//month       INTEGER,
//day         INTEGER,
//hour        INTEGER,
//minute      INTEGER,
//second      INTEGER,
//forecasttime INTEGER,
//timerange    INTEGER,
//lon1 real,
//lon2 real,
//lat1 real,
//lat2 real,
//createTime TEXT,
//startTime  TEXT,
//finishTime TEXT,
//
//errReason TEXT,
//errLevel  TEXT
//);


// 客户端收到数据
//CREATE TABLE NwfdStationSend
//(
//	id INTEGER  primary key,
//	status   TEXT,
//	dataType TEXT,
//	cityType TEXT,
//	year        INTEGER,
//	month       INTEGER,
//	day         INTEGER,
//	hour        INTEGER,
//	minute      INTEGER,
//	second      INTEGER,
//	forecasttime INTEGER,
//	timerange    INTEGER,
//	station	TEXT,
//	temperature	REAL,
//	humidity		REAL,
//	windDirection	REAL,
//	windSpeed		REAL,
//	airPressure	REAL,
//	precipitation	REAL,
//	cloud			REAL,
//	lCloud		REAL,
//	weather		REAL,
//	visibility	REAL,
//	tMax		REAL,
//	tMin		REAL,
//	hMax		REAL,
//	hMin		REAL,
//	precipitation24H	REAL,
//	rProbability12H	REAL,
//	cloud12H			REAL,
//	lCloud12H			REAL,
//	weather12H		REAL,
//	windDirection12H	REAL,
//	windSpeed12H		REAL,
//
//	createTime TEXT,
//	startTime  TEXT,
//	finishTime TEXT,
//	errReason TEXT,
//	errLevel  TEXT
//);


//
//CREATE TABLE NwfdStationRecv
//(
//	id INTEGER  primary key,
//	status   TEXT,
//	dataType TEXT,
//	cityType TEXT,
//	year        INTEGER,
//	month       INTEGER,
//	day         INTEGER,
//	hour        INTEGER,
//	minute      INTEGER,
//	second      INTEGER,
//	forecasttime INTEGER,
//	timerange    INTEGER,
//	station	TEXT,
//	temperature	REAL,
//	humidity		REAL,
//	windDirection	REAL,
//	windSpeed		REAL,
//	airPressure	REAL,
//	precipitation	REAL,
//	cloud			REAL,
//	lCloud		REAL,
//	weather		REAL,
//	visibility	REAL,
//	tMax		REAL,
//	tMin		REAL,
//	hMax		REAL,
//	hMin		REAL,
//	precipitation24H	REAL,
//	rProbability12H	REAL,
//	cloud12H			REAL,
//	lCloud12H			REAL,
//	weather12H		REAL,
//	windDirection12H	REAL,
//	windSpeed12H		REAL,
//
//	clientID   INTEGER,
//	createTime TEXT,
//	startTime  TEXT,
//	finishTime TEXT,
//	errReason TEXT,
//	errLevel  TEXT
//);


// 产品信息表
//CREATE TABLE Tbl_Product
//(
//ProductKey  TEXT primary key,
//ProductName TEXT,
//Type        TEXT,
//Publisher   TEXT,
//
//Lon1  REAL,
//Lon2  REAL,
//Lat1  REAL,
//Lat2  REAL,
//Di    REAL,
//Dj    REAL,
//
//TimeRange INTEGER,
//MaxForecastTime INTEGER,
//BusinessStatus TEXT,
//IsMerge INTEGER
//);

#endif // DATEBASESQLDEF_H
