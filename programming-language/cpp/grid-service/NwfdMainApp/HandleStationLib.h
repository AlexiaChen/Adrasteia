/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: HandleStationLib.h
* 作  者: zhangl  		版本：1.0		日  期：2015/08/04
* 描  述：站点数据处理动态库调用类
* 其  他：
* 功能列表:
*
* 修改日志：
*   No.   日期		  作者		       修改内容
* ------------------------------------------------------------------------
*************************************************************************/
#ifndef HANDLE_STATIONLIB_H
#define HANDLE_STATIONLIB_H

/************************************************************************/
/* 头文件引用                                                           */
/************************************************************************/
#include "StationDef.h"
#include "LibraryDef.h"
#include <QString>

/************************************************************************/
/* micaps4 to grib2 格式转换动态库调用函数声明                          */
/************************************************************************/
typedef int(*lpNwfd_openstationfile)(const char *, char *, st_station *&, int &);
typedef void (*lpNwfd_freestationdata)(st_station * &, int );
typedef int(*lpNwfd_savedata2file)(const char *, const char *, const char *, char*, st_station *, int, int, int, int, int, int, int);

/************************************************************************/
/* 类 名： HandleGribDir                                                */
/* 父 类： -                                                            */
/* 说 明： Grib2动态库调用类                                            */
/* 描 述：                                                              */
/************************************************************************/
class HandleStationLib
{
private:
	HandleStationLib();

public:
	static HandleStationLib & getClass();

public:
	~HandleStationLib();

	// 1、打开站点文件获取站点数据
	int nwfd_openstationfile(const char * szType, char* file, st_station * &pstStation, int &Cnt);

	// 2、释放站点数据申请的空间
	void nwfd_freestationdata(st_station * &pstStation, int cnt);

	// 3、保存站点数据到文件中
	int nwfd_savedata2file(const char * szType, const char * szIDSymbol, const char * szDataType, char* file, st_station * pstStation, int cnt, int year, int month, int day, int hour, int minute, int second);

private:
	LIB_HANDLE   hDll ;
	lpNwfd_openstationfile  m_funNwfd_openstationfile; 
	lpNwfd_freestationdata  m_funNwfd_freestationdata;
	lpNwfd_savedata2file    m_funNwfd_savedata2file;
};

#endif // HANDLE_NWFDLIB_H


