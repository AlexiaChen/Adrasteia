#ifndef NWFD_DATAPROC_DEF
#define NWFD_DATAPROC_DEF

#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include <list>
using namespace std;

/************************************************************************/
/*  վ�㵥����ֵ                                                        */
/************************************************************************/
typedef struct _STATION_VALUE
{
	//char  staion[10]; // վ��
	float lon;        // ����
	float lat;        // γ��
	float height;     // �߶�
	float fValue;     // ֵ
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
/* 0ֵ���㶨��,���ڸ������ıȽ�                                         */
/************************************************************************/
#define  ZERO               0.0001   //  0 ����4λС��Ϊ��׼
#define  NA_ZERO           -0.0001   // -0
#define  IS_EQUAL_ZERO(x)  ((x) > NA_ZERO && (x) < ZERO)  // �жϸ������Ƿ�=0
#define  IS_EQUAL(x,y)     IS_EQUAL_ZERO(x-y)             // �ж������������Ƿ����




#endif // NWFD_DATAPROC_DEF

