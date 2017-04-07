/*************************************************************************
* Copyright (C), 2016,
* XXXXXXXXXXX Co.
* �ļ���: nwd-dataproc-win32.h
* ��  ��: zhangl  		�汾��1.0		��  �ڣ�
* ��  �������ݴ���ӿ�
* ��  ����
* �����б�:
*   1. ��㵽վ���ֵ����
*   2. վ���ֵ�����Ķ�������
*   3. ��ˮһ���Դ���
*   4. �ֱ��������߶ȴ���
*   5.
* �޸���־��
*   No.   ����		  ����		       �޸�����
* ------------------------------------------------------------------------
*************************************************************************/
#ifndef NWFD_DATAPROC_WIN32
#define NWFD_DATAPROC_WIN32

#include "nwfd-dataproc-def.h"

// ������־
//#define USE_LOG

#ifdef __cplusplus
extern "C" {
#endif

	/* 1. ��㵽վ���ֵ���� */
	int nwfd_get_grid2station_data(float * fGridData, float fStartLon, float fStartLat, float fEndLon, float fEndLat, float fDi, float fDj, ST_STATION_VALUE* &stStation, int nStationNum);
	//int nwfd_get_grid2station_data(float * fGridData, float fStartLon, float fStartLat, float fEndLon, float fEndLat, float fDi, float fDj, ST_STATION_VALUE* &stInStation, ST_STATION_VALUE* &stOutStation, int nStationNum);
	//void nwfd_free_staion(ST_STATION_VALUE* &stStation);
	
	/* 2. վ���ֵ�����Ķ������� */
	int nwfd_get_station2grid_data(ST_STATION_VALUE* stStationDiff, int nStationNum, float fRadius, float* &fGridData, float fStartLon, float fStartLat, float fEndLon, float fEndLat, float fDi, float fDj);

	/* 3. ��ˮһ���Դ��� */
	int nwfd_get_rain_data(float * fOrigRainData, int nOrigDataCnt, int nOrigRange, float* &fDestRainData, int nDestDataCnt, int nDestRange);

	int HandleRainCorrection_Downward(float * fOrigRainData, int nOrigDataCnt, int nOrigRange, float* &fDestRainData, int nDestDataCnt, int nDestRange);
	int HandleRainCorrection_Upward(float * fOrigRainData, int nOrigDataCnt, int nOrigRange, float* &fDestRainData, int nDestDataCnt, int nDestRange);

	/* 4. �ֱ��������߶ȴ��� */
	int proccess_di_dj(float *fInData, nwfd_grib2_field &stInField, float fInDi, float fInDj, float fOutDi, float fOutDj,
		float * &fOutData, nwfd_grib2_field &stOutField, bool warningdata, float fMissingVal);
	int proccess_di_dj_down(float *fInData, nwfd_grib2_field &stInField, float fInDi, float fInDj, float fOutDi, float fOutDj,
		float * &fOutData, nwfd_grib2_field &stOutField, bool warningdata, float fMissingVal);
	int proccess_di_dj_up(float *fInData, nwfd_grib2_field &stInField, float fInDi, float fInDj, float fOutDi, float fOutDj,
		float * &fOutData, nwfd_grib2_field &stOutField, float fMissingVal);

	/* 5. ��ȡĳ�������ݵĴ��� */
	int proccess_lon_lat(float *fInData, nwfd_grib2_field &stInField, float fStartLon, float fEndLon, float fStartLat, float fEndLat,
		float * &fOutData, nwfd_grib2_field &stOutFiled, float fMissingVal);

#ifdef __cplusplus
}
#endif



#endif // NWFD_DATAPROC_WIN32

