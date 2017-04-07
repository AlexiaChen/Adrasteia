#ifndef NWFD_DATAPROC_DEF
#define NWFD_DATAPROC_DEF

#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include <list>
using namespace std;

/************************************************************************/
/*  站点单数据值                                                        */
/************************************************************************/
typedef struct _STATION_VALUE
{
	//char  staion[10]; // 站号
	float lon;        // 经度
	float lat;        // 纬度
	float height;     // 高度
	float fValue;     // 值
}ST_STATION_VALUE;
typedef list<ST_STATION_VALUE *> LIST_STATION_VALUE;

struct nwfd_grib2_field{
	int category;
	int element;
	int statistical;
	int status;

	float lon1;
	float lon2;
	float lat1;
	float lat2;
	float incrementi;
	float incrementj;
	int Ni;
	int Nj;

	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
	int forecast;
	int timerange;
};

/************************************************************************/
/* 0值浮点定义,用于浮点数的比较                                         */
/************************************************************************/
#define  ZERO               0.0001   //  0 ，以4位小数为标准
#define  NA_ZERO           -0.0001   // -0
#define  IS_EQUAL_ZERO(x)  ((x) > NA_ZERO && (x) < ZERO)  // 判断浮点数是否=0
#define  IS_EQUAL(x,y)     IS_EQUAL_ZERO(x-y)             // 判断两个浮点数是否相等




#endif // NWFD_DATAPROC_DEF

