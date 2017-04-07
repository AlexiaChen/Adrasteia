/*************************************************************************
* Copyright (C), 2016,
* XXXXXXXXXXX Co.
* 文件名: nwd-dataproc-win32.h
* 作  者: zhangl  		版本：1.0		日  期：
* 描  述：数据处理接口
* 其  他：
* 功能列表:
*   1. 格点到站点插值处理
*   2. 站点差值到格点的订正处理
*   3. 降水一致性处理
*   4. 分辨率升降尺度处理
*   5.
* 修改日志：
*   No.   日期		  作者		       修改内容
* ------------------------------------------------------------------------
*************************************************************************/
#ifndef NWFD_DATAPROC_WIN32
#define NWFD_DATAPROC_WIN32

#include "nwfd-dataproc-def.h"

// 调试日志
//#define USE_LOG

#ifdef __cplusplus
extern "C" {
#endif

	/* 1. 格点到站点插值处理 */
	int nwfd_get_grid2station_data(float * fGridData, float fStartLon, float fStartLat, float fEndLon, float fEndLat, float fDi, float fDj, ST_STATION_VALUE* &stStation, int nStationNum);
	//int nwfd_get_grid2station_data(float * fGridData, float fStartLon, float fStartLat, float fEndLon, float fEndLat, float fDi, float fDj, ST_STATION_VALUE* &stInStation, ST_STATION_VALUE* &stOutStation, int nStationNum);
	//void nwfd_free_staion(ST_STATION_VALUE* &stStation);
	
	/* 2. 站点差值到格点的订正处理 */
	int nwfd_get_station2grid_data(ST_STATION_VALUE* stStationDiff, int nStationNum, float fRadius, float* &fGridData, float fStartLon, float fStartLat, float fEndLon, float fEndLat, float fDi, float fDj);

	/* 3. 降水一致性处理 */
	int nwfd_get_rain_data(float * fOrigRainData, int nOrigDataCnt, int nOrigRange, float* &fDestRainData, int nDestDataCnt, int nDestRange);

	int HandleRainCorrection_Downward(float * fOrigRainData, int nOrigDataCnt, int nOrigRange, float* &fDestRainData, int nDestDataCnt, int nDestRange);
	int HandleRainCorrection_Upward(float * fOrigRainData, int nOrigDataCnt, int nOrigRange, float* &fDestRainData, int nDestDataCnt, int nDestRange);

	/* 4. 分辨率升降尺度处理 */
	int proccess_di_dj(float *fInData, nwfd_grib2_field &stInField, float fInDi, float fInDj, float fOutDi, float fOutDj,
		float * &fOutData, nwfd_grib2_field &stOutField, bool warningdata, float fMissingVal);
	int proccess_di_dj_down(float *fInData, nwfd_grib2_field &stInField, float fInDi, float fInDj, float fOutDi, float fOutDj,
		float * &fOutData, nwfd_grib2_field &stOutField, bool warningdata, float fMissingVal);
	int proccess_di_dj_up(float *fInData, nwfd_grib2_field &stInField, float fInDi, float fInDj, float fOutDi, float fOutDj,
		float * &fOutData, nwfd_grib2_field &stOutField, float fMissingVal);

	/* 5. 获取某区域数据的处理 */
	int proccess_lon_lat(float *fInData, nwfd_grib2_field &stInField, float fStartLon, float fEndLon, float fStartLat, float fEndLat,
		float * &fOutData, nwfd_grib2_field &stOutFiled, float fMissingVal);

#ifdef __cplusplus
}
#endif



#endif // NWFD_DATAPROC_WIN32

