#include "log.h"
#include "HandleNwfdLib.h"

using namespace std;

HandleNwfdLib::HandleNwfdLib()
{
	// 初始化
	hDll = NULL;
	m_funNwfd_openm4file  = NULL;
	m_funNwfd_openm4file2 = NULL;
	m_funNwfd_datset2grib = NULL;
	m_funNwfd_m4file2grib = NULL;
	m_funNwfd_savetofile  = NULL;
	m_funNwfd_freefld     = NULL;
    m_funNwfd_addfield_simpled48 = NULL;
	m_funNwfd_create     = NULL;
	m_funNwfd_addgrid    = NULL;
	m_funNwfd_end        = NULL;

	// 加载动态库，获取动态库接口方法
	hDll = Lib_Open(Lib_Str(QString(NWFD_GRIB_LIB)));
	if (hDll)
	{
		this->m_funNwfd_openm4file  = (lpNwfd_openm4file)Lib_GetFun(hDll,  "nwfd_openm4file");
		this->m_funNwfd_openm4file2 = (lpNwfd_openm4file2)Lib_GetFun(hDll, "nwfd_openm4file2");
		this->m_funNwfd_openm11file = (lpNwfd_openm11file)Lib_GetFun(hDll, "nwfd_openm11file");
		this->m_funNwfd_datset2grib = (lpNwfd_datset2grib)Lib_GetFun(hDll, "nwfd_datset2grib");
		this->m_funNwfd_m4file2grib = (lpNwfd_m4file2grib)Lib_GetFun(hDll, "nwfd_m4file2grib");
		this->m_funNwfd_savetofile  = (lpNwfd_savetofile)Lib_GetFun(hDll,  "nwfd_savetofile");
		this->m_funNwfd_freefld     = (lpNwfd_freefld)Lib_GetFun(hDll,     "nwfd_freefld");
		this->m_funNwfd_addfield_simpled48 = (lpNwfd_addfield_simpled48)Lib_GetFun(hDll, "nwfd_addfield_simpled48");
		this->m_funNwfd_create      = (lpNwfd_create)Lib_GetFun(hDll, "nwfd_create");
		this->m_funNwfd_addgrid     = (lpNwfd_addgrid)Lib_GetFun(hDll, "nwfd_addgrid");
		this->m_funNwfd_end         = (lpNwfd_end)Lib_GetFun(hDll, "nwfd_end");
		this->m_funNwfd_gribinfo    = (lpNwfd_gribinfo)Lib_GetFun(hDll, "nwfd_gribinfo");
		this->m_funNwfd_gribfieldinfo = (lpNwfd_gribfieldinfo)Lib_GetFun(hDll, "nwfd_gribfieldinfo");
		this->m_funNwfd_gribfield   = (lpNwfd_gribfield)Lib_GetFun(hDll, "nwfd_gribfield");
	}
	else
	{
		LOG_(LOGID_ERROR, LOG_F("打开dll %s失败，错误编码=%d"), NWFD_GRIB_LIB, Lib_GetErr());
		// todo
	}
}

HandleNwfdLib::~HandleNwfdLib()
{
	if (hDll)
	{
		Lib_Close(hDll);

		hDll = NULL;
		m_funNwfd_openm4file = NULL;
		m_funNwfd_openm4file2 = NULL;
		m_funNwfd_datset2grib = NULL;
		m_funNwfd_m4file2grib = NULL;
		m_funNwfd_savetofile = NULL;
		m_funNwfd_freefld = NULL;
		m_funNwfd_addfield_simpled48 = NULL;
		m_funNwfd_create = NULL;
		m_funNwfd_addgrid = NULL;
		m_funNwfd_end = NULL;
	}
}

HandleNwfdLib & HandleNwfdLib::getClass()
{
	static HandleNwfdLib objNwfdLib;
	return objNwfdLib;
}

/************************************************************************/
/* 读取micaps4文件                                                      */
/************************************************************************/
/**
* @brief nwfd_openm4file
* @param micaps
* @param grid
* @param t
* @param fld
* @return
*/
float * HandleNwfdLib::nwfd_openm4file(char *micaps, grdpts *grid)
{
	if (m_funNwfd_openm4file == NULL)
	{
		return  NULL;
	}

	return m_funNwfd_openm4file(micaps, grid);
}

float * HandleNwfdLib::nwfd_openm4file2(char *micaps, grdpts *grid, float fOffSet)
{
	if (m_funNwfd_openm4file2 == NULL)
	{
		return  NULL;
	}

	return m_funNwfd_openm4file2(micaps, grid, fOffSet);
}

float * HandleNwfdLib::nwfd_openm11file(char *micaps, grdpts *grid, float fOffSet)
{
	if (m_funNwfd_openm11file == NULL)
	{
		return  NULL;
	}

	return m_funNwfd_openm11file(micaps, grid, fOffSet);
}
/************************************************************************/
/* 将g2float数据转成Grib2格式                                           */
/************************************************************************/
/**
* @brief nwfd_datset2grib 将一组数据写入到grib message中
* @param cgrib
* @param category
* @param element
* @param statistical
* @param status
* @param year
* @param month
* @param day
* @param hour
* @param forecasttime  首个预报是时间
* @param timerange     每个时次的预报范围
* @param cnt           一共多少个
* @param lon1
* @param lon2
* @param lat1
* @param lat2
* @param Ni
* @param Nj
* @param subdivisions
* @param flds
* @return 返回的是grib消息体的长度
*/
long HandleNwfdLib::nwfd_datset2grib(unsigned char* cgrib, g2int category, g2int element, g2int statistical, g2int status,
	g2int year, g2int month, g2int day, g2int hour, g2int minute, g2int second,
	g2int forecasttime, g2int timerange, g2int cnt,
	g2float lon1, g2float lon2, g2float lat1, g2float lat2, g2float incrementi, g2float incrementj, g2int Ni, g2int Nj, g2float *flds)
{
	if (m_funNwfd_datset2grib == NULL)
	{
		return 0;
	}

	return m_funNwfd_datset2grib(cgrib, category, element, statistical, status,year, month, day, hour, minute, second, forecasttime, 
		timerange, cnt, lon1, lon2, lat1, lat2, incrementi, incrementj, Ni, Nj, flds);
}

/************************************************************************/
/* 将micaps4文件转成Grib2格式                                           */
/************************************************************************/
/**
* @brief nwfd_micaps2grib
* @param grib
* @param micaps
* @param product
* @param status
* @param timerange
* @return
*/
long HandleNwfdLib::nwfd_m4file2grib(unsigned char* cgrib, char* micaps[], g2int cnt,
	g2int category, g2int element, g2int statistical, g2int status,
	g2int year, g2int month, g2int day, g2int hour, g2int minute, g2int second, g2int forecasttime, g2int timerange)
{
	if (m_funNwfd_m4file2grib == NULL)
	{
		return 0;
	}

	return m_funNwfd_m4file2grib(cgrib, micaps, cnt, category, element, statistical, status, year, month, day, hour, minute, second, forecasttime, timerange);
}

/************************************************************************/
/* 保存Grib2数据到文件中                                                */
/************************************************************************/
/**
* @brief g2_savetofile
* @param cgrib
* @param len
* @param filename
* @return
*/
long HandleNwfdLib::nwfd_savetofile(unsigned char *cgrib, g2int len, char *filename)
{
	if (m_funNwfd_savetofile == NULL)
	{
		return -1;
	}
	
	return m_funNwfd_savetofile(cgrib, len, filename);
}

/************************************************************************/
/* 释放DLL中创建的空间                                                  */
/************************************************************************/
long HandleNwfdLib::nwfd_freefld(float * fld)
{
	if (m_funNwfd_freefld == NULL)
	{
		return -1;
	}

	return m_funNwfd_freefld(fld);
}

/**
* @brief nwfd_addfield
* @param cgrib
* @param category
* @param element
* @param statistical
* @param year
* @param month
* @param day
* @param hour
* @param forecasttime
* @param timerange
* @param fld
* @param ngrdpts
* @return
*/
long HandleNwfdLib::nwfd_addfield_simpled48(unsigned char *cgrib, g2int category, g2int element, g2int statistical, g2int year, g2int month, g2int day, g2int hour, g2int minute, g2int second, g2int forecasttime, g2int timerange, g2float *fld, g2int ngrdpts)
{
	if (m_funNwfd_addfield_simpled48 == NULL)
	{
		return -1;
	}

	return m_funNwfd_addfield_simpled48(cgrib, category, element, statistical, year, month, day, hour, minute, second, forecasttime, timerange, fld, ngrdpts);
}

long HandleNwfdLib::nwfd_create(unsigned char* cgrib, g2int year, g2int month, g2int day, g2int hour, g2int minute, g2int second, g2int status)
{
	if (m_funNwfd_create == NULL)
	{
		return -1;
	}

	return m_funNwfd_create(cgrib, year, month, day, hour, minute, second, status);
}

long HandleNwfdLib::nwfd_addgrid(unsigned char *cgrib, g2float lon1, g2float lon2, g2float lat1, g2float lat2, g2float incrementi, g2float incrementj, g2int Ni, g2int Nj)
{
	if (m_funNwfd_addgrid == NULL)
	{
		return -1;
	}

	return m_funNwfd_addgrid(cgrib, lon1, lon2, lat1, lat2, incrementi, incrementj, Ni, Nj);
}

long HandleNwfdLib::nwfd_end(unsigned char *cgrib)
{
	if (m_funNwfd_end == NULL)
	{
		return -1;
	}

	return m_funNwfd_end(cgrib);
}

long HandleNwfdLib::nwfd_gribinfo(unsigned char *cgrib, nwfd_grib2_info * info)
{
	if (m_funNwfd_gribinfo == NULL)
	{
		return -1;
	}

	return m_funNwfd_gribinfo(cgrib, info);
}

long HandleNwfdLib::nwfd_gribfieldinfo(unsigned char *cgrib, g2int num, nwfd_grib2_field * field)
{
	if (m_funNwfd_gribfieldinfo == NULL)
	{
		return -1;
	}

	return m_funNwfd_gribfieldinfo(cgrib, num, field);
}

float* HandleNwfdLib::nwfd_gribfield(unsigned char *cgrib, g2int num, nwfd_grib2_field * field)
{
	if (m_funNwfd_gribfield == NULL)
	{
		return NULL;
	}

	return m_funNwfd_gribfield(cgrib, num, field);
}

