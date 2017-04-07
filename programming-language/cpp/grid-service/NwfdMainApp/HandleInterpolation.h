/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: HandleInterpolation.h
* 作  者: zhangl  		版本：1.0		日  期：2015/11/19
* 描  述：格点，站点插值处理
* 其  他：
* 功能列表:
*   1. 格点到站点的双线形插值法
* 
* 修改日志：
*   No.   日期		  作者		       修改内容
* ------------------------------------------------------------------------
*************************************************************************/
#ifndef HANDLE_INTERPOLATION_H
#define HANDLE_INTERPOLATION_H

#include "StationDef.h"

/************************************************************************/
/* 类 名： HandleInterpolation                                          */
/* 父 类： -                                                            */
/* 说 明： 站点，格点转换处理                                           */
/* 描 述：                                                              */
/************************************************************************/
class HandleInterpolation
{
private:
	HandleInterpolation();

public:
	~HandleInterpolation();
	static HandleInterpolation & getClass();

public:
	// 格点到站点插值处理
	float biInterpolation(float * fGridData, float fStartLon, float fStartLat, float fDi, float fDj, int nNi, int nNj, float fStationLon, float fStationLat);

	// 站点到格点的调整处理
	//bool AdjustmentGrid(float fStationData, float fStationLon, float fStationLat, int nRadius, float * fGridData, float fStartLon, float fStartLat, float fDi, float fDj, int nNi, int nNj);
	
	// cressman插值算法
	bool cressman(LIST_STATION_VALUE lstStationDiff, float fRadius, float * fGridData, float fStartLon, float fStartLat, float fDi, float fDj, int nNi, int nNj);

};

#endif //HANDLE_INTERPOLATION_H
