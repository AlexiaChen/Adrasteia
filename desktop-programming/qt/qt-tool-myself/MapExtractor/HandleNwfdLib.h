/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: HandleGrib.h
* 作  者: zhangl  		版本：1.0		日  期：2015/04/
* 描  述：Grib2处理动态库调用类
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
#ifndef HANDLE_NWFDLIB_H
#define HANDLE_NWFDLIB_H

/************************************************************************/
/* 头文件引用                                                           */
/************************************************************************/
#include "NWFD-g2clib.h"
#include "LibraryDef.h"
#include <string.h>
#include <QString>

/************************************************************************/
/* micaps4 to grib2 格式转换动态库调用函数声明                          */
/************************************************************************/
typedef float* (*lpNwfd_openm4file)(char *, grdpts *);
typedef float* (*lpNwfd_openm4file2)(char *, grdpts *, float);
typedef float* (*lpNwfd_openm11file)(char *, grdpts *, float);
typedef long(*lpNwfd_datset2grib)(unsigned char*, g2int, g2int, g2int, g2int,g2int, g2int, g2int, g2int, g2int, g2int, g2int, g2int, g2int, g2float, g2float, g2float, g2float, g2float, g2float, g2int, g2int, g2float *);
typedef long(*lpNwfd_m4file2grib)(unsigned char *, char * [], g2int, g2int, g2int, g2int, g2int, g2int, g2int, g2int, g2int, g2int, g2int, g2int, g2int);
typedef long(*lpNwfd_savetofile)(unsigned char *, g2int, char *);
typedef long(*lpNwfd_freefld)(float *);
typedef long(*lpNwfd_addfield_simpled48)(unsigned char *, g2int, g2int, g2int, g2int, g2int, g2int, g2int, g2int, g2int, g2int, g2int, g2float *, g2int);
typedef long(*lpNwfd_create)(unsigned char*, g2int, g2int, g2int, g2int, g2int, g2int, g2int);
typedef long(*lpNwfd_addgrid)(unsigned char *, g2float, g2float, g2float, g2float, g2float, g2float, g2int, g2int);
typedef long(*lpNwfd_end)(unsigned char *);
typedef long(*lpNwfd_gribinfo)(unsigned char *, nwfd_grib2_info *);
typedef long(*lpNwfd_gribfieldinfo)(unsigned char *, g2int, nwfd_grib2_field *);
typedef float* (*lpNwfd_gribfield)(unsigned char *, g2int, nwfd_grib2_field *);
/************************************************************************/
/* 类 名： HandleGribDir                                                */
/* 父 类： -                                                            */
/* 说 明： Grib2动态库调用类                                            */
/* 描 述：                                                              */
/************************************************************************/
class HandleNwfdLib
{
private:
	HandleNwfdLib();

public:
	static HandleNwfdLib & getClass();

public:
	~HandleNwfdLib();

	// 功能外部调用方法
	float * nwfd_openm4file(char *micaps, grdpts *grid);
	float * nwfd_openm4file2(char *micaps, grdpts *grid, float fOffSet = 0);
	float * nwfd_openm11file(char *micaps, grdpts *grid, float fOffSet = 0);
	long nwfd_datset2grib(unsigned char* cgrib, g2int category, g2int element, g2int statistical, g2int status, g2int year, g2int month, g2int day, g2int hour, g2int minute, g2int second,g2int forecasttime, g2int timerange, g2int cnt, g2float lon1, g2float lon2, g2float lat1, g2float lat2, g2float incrementi, g2float incrementj, g2int Ni, g2int Nj, g2float *flds);
	long nwfd_m4file2grib(unsigned char* cgrib, char* micaps[], g2int cnt,g2int category, g2int element, g2int statistical, g2int status, g2int year, g2int month, g2int day, g2int hour, g2int minute, g2int second, g2int forecasttime, g2int timerange);
	long nwfd_savetofile(unsigned char *cgrib, g2int len, char *filename);
	long nwfd_freefld(float * fld);

	long nwfd_addfield_simpled48(unsigned char *cgrib, g2int category, g2int element, g2int statistical, g2int year, g2int month, g2int day, g2int hour, g2int minute, g2int second, g2int forecasttime, g2int timerange, g2float *fld, g2int ngrdpts);
	long nwfd_create(unsigned char* cgrib, g2int year, g2int month, g2int day, g2int hour, g2int minute, g2int second, g2int status);
	long nwfd_addgrid(unsigned char *cgrib, g2float lon1, g2float lon2, g2float lat1, g2float lat2, g2float incrementi, g2float incrementj, g2int Ni, g2int Nj);
	long nwfd_end(unsigned char *cgrib);

	long nwfd_gribinfo(unsigned char *cgrib, nwfd_grib2_info * info);
	long nwfd_gribfieldinfo(unsigned char *cgrib, g2int num, nwfd_grib2_field * field);
	float* nwfd_gribfield(unsigned char *cgrib, g2int num, nwfd_grib2_field * field);

private:
	HMODULE   hDll;
	lpNwfd_openm4file  m_funNwfd_openm4file ;  // 读取micaps4文件
	lpNwfd_openm4file2 m_funNwfd_openm4file2;  // 读取micaps4文件,方法2，传人偏移量值
	lpNwfd_openm11file m_funNwfd_openm11file;  // 读取micaps11文件
	lpNwfd_datset2grib m_funNwfd_datset2grib;  // 将g2float数据转成Grib2格式
	lpNwfd_m4file2grib m_funNwfd_m4file2grib;  // 将micaps4文件转成Grib2格式
	lpNwfd_savetofile  m_funNwfd_savetofile;   // 保存Grib2数据到文件中
	lpNwfd_freefld     m_funNwfd_freefld;      // 释放DLL中创建的空间

	lpNwfd_addfield_simpled48 m_funNwfd_addfield_simpled48;
	lpNwfd_create             m_funNwfd_create;
	lpNwfd_addgrid            m_funNwfd_addgrid;
	lpNwfd_end                m_funNwfd_end;

	lpNwfd_gribinfo        m_funNwfd_gribinfo;
	lpNwfd_gribfieldinfo   m_funNwfd_gribfieldinfo;
	lpNwfd_gribfield       m_funNwfd_gribfield;
};

#endif // HANDLE_NWFDLIB_H
