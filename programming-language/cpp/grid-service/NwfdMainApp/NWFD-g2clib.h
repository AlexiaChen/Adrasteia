#ifndef _nwfd_g2clib_H
#define _nwfd_g2clib_H

#ifdef __64BIT__
typedef int g2int;
typedef unsigned int g2intu;
#else
typedef long long g2int; // modify 为了修改数据因太大到导致异常的问题修改为long long(8个字节)
typedef unsigned long g2intu;
#endif

typedef float g2float;

struct grdpts{
    g2float lon1;
    g2float lon2;
    g2float lat1;
    g2float lat2;
    g2float nlon;
    g2float nlat;
    g2int Ni;
    g2int Nj;
};

struct nwfd_grib2_info{
    g2int year;
    g2int month;
    g2int day;
    g2int hour;
    g2int minute;
    g2int second;

    g2int numfields;
};

struct nwfd_grib2_field{
	g2int category;
	g2int element;
	g2int statistical;
	g2int status;

	g2float lon1;
	g2float lon2;
	g2float lat1;
	g2float lat2;
	g2float incrementi;
	g2float incrementj;
	g2int Ni;
	g2int Nj;

	g2int year;
	g2int month;
	g2int day;
	g2int hour;
	g2int minute;
	g2int second;
	g2int forecast;
	g2int timerange;
};

#endif
