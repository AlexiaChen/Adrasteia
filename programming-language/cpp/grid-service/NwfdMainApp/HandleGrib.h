/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: HandleGrib.h
* 作  者: zhangl  		版本：1.0		日  期：2015/04/14
* 描  述：Grib2相关公共处理调用方法(公共方法)
* 其  他：
* 功能列表:
*   1. 检测创建Grib存储目录
*   2. 获取Grib文件名
*   3. 获取Grib存储的存储空间大小
* 
* 修改日志：
*   No.   日期		  作者		       修改内容
* ------------------------------------------------------------------------
*   1  2015/05/27    zhangl    修改此类名和功能，改为针对girb相关的公共方法
*************************************************************************/

#ifndef HANDLE_GRIB_H
#define HANDLE_GRIB_H

#include <QString>
#include <QDir>
#include <QDateTime>
#include "ProductDef.h"
#include "HandleNwfdLib.h"

/************************************************************************/
/* 类 名： HandleGribDir                                                */
/* 父 类： -                                                            */
/* 说 明： Grib2存储目录处理类                                          */
/* 描 述： 检测创建Grib2的存储目录                                      */
/************************************************************************/
class HandleGrib
{
private:
	HandleGrib();

public:
	~HandleGrib();
	static HandleGrib & getClass();

public:
	/* Grib内存空间大小估算  */
	unsigned int GetGribMemSize(int nNi, int nNj, int nCnt);

	/* Grib数据处理 */
	// 获取文件数据
	float * GetNwfdData(QString strFile, float fOffSet, nwfd_grib2_field* &stDataField, int & nCnt);
	int GetNwfdData(QString strFile, float fOffSet, int nYear, int nMonth, int nDay, int nHour, int nForecasttime, float* &fData, nwfd_grib2_field* &stDataField, int & nCnt);

	QString GetDataFormat(QString strFile);
	float * nwfd_opengrib2file(QString strFile, nwfd_grib2_field* &stDataField, int & nCnt);
	int nwfd_opengrib2file(QString strFile, int nYear, int nMonth, int nDay, int nHour, int nForecasttime, float* &fData, nwfd_grib2_field* &stDataField, int & nCnt);

	unsigned char * ReadGribFile(QString strFile);
	unsigned char * ReadGribFile(QString strFile, unsigned int &size);

	// 根据偏移量重新计算数据
	bool  CacalNwfdData(float* &fData, int nNi, int nNj, int nCnt, float fOffSet);

	// 处理数据到文件
	int ProcessGribData(float *fData, int nCnt, ST_PRODUCT stProduct, float fDi, float fDj, int nNi, int nNj, 
		int nYear, int nMonth, int  nDay, int  nHour, int  nForecasttime, int nTimeRange, QString strGribFile);

	// 保存数据到grib文件中
	int SaveData2GribFile(float *fData, int nCnt, int nCategory, int nElement, int nStatistical, int nStatus,
		float fLon1, float fLon2, float fLat1, float fLat2, float fDi, float fDj,
		int nYear, int nMonth, int  nDay, int  nHour, int  nForecasttime, int nTimeRange, int nClientID, QString strKey, QString strType, QString strPath, QString strFileName);

	int SaveData2GribFile(float *fData, int nCnt, int nCategory, int nElement, int nStatistical, int nStatus, 
		float fLon1, float fLon2, float fLat1, float fLat2, float fDi, float fDj, 
		int nYear, int nMonth, int  nDay, int  nHour, int  nForecasttime, int nTimeRange, QString strPath, QString strFileName);

	int SaveData2GribFile(float *fData, int nCnt, int nCategory, int nElement, int nStatistical, int nStatus,
		float fLon1, float fLon2, float fLat1, float fLat2, float fDi, float fDj, 
		int nYear, int nMonth, int  nDay, int  nHour, int  nForecasttime, int nTimeRange, QString strGribFile);

	// 保存数据到文件中公共方法
	int SaveData2File(float *fData, int nDataCnt,  /* 数据 */
		float fLon1, float fLon2, float fLat1, float fLat2, float fDi, float fDj, int nNi, int nNj, /* 经纬度范围和个数 */
		int nYear, int nMonth, int nDay, int nHour, int nForecastTime, int nTimeRange, /* 时间 */
		int nCategory, int nElement[2], int nStatistical, int nStatus, int nCnt, /* grib信息 */
		QString strName, QString strLineVal, float fOffSet, float fMissingVal,  /* micpas 存储信息 */
		QString strPath, QString strFileName, QString strFileFormat = "grib", bool bIsMergeFile = false/* 文件存储信息 */
		);

	// 保存数据到grib文件中公共方法
	int SaveData2GribFile(float *fData, int nDataCnt,  /* 数据 */
		float fLon1, float fLon2, float fLat1, float fLat2, float fDi, float fDj, int nNi, int nNj, /* 经纬度范围和个数 */
		int nYear, int nMonth, int nDay, int nHour, int nForecastTime, int nTimeRange, /* 时间 */
		int nCategory, int nElement[2], int nStatistical, int nStatus, int nCnt, /* grib信息 */
		QString strFile, bool bIsMergeFile = false/* 文件存储信息 */
		);

	// 保存数据到micaps文件中(支持Micaps4，Micaps11)
	int SaveData2MicapsFile(float * fData, int nCnt, 
		float fLon1, float fLon2, float fLat1, float fLat2, float fDi, float fDj, int nNi, int nNj,
		int nYear, int nMonth, int  nDay, int  nHour, int  nForecasttime, 
		QString strName, QString strLineVal, float fOffSet, float fMissingVal, QString strFile);
};

#endif //HANDLE_GRIB_H
