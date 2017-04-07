#include "log.h"
#include "NwfdErrorCode.h"
#include "HandleInterpolation.h"
#include "Common.h"
#include "HandleCommon.h"

/************************************************************************/
/* 构造函数，析构函数，单例调用接口                                     */
/************************************************************************/
HandleInterpolation::HandleInterpolation()
{

}

HandleInterpolation::~HandleInterpolation()
{
}

HandleInterpolation & HandleInterpolation::getClass()
{
	static HandleInterpolation objInterpolation;
	return objInterpolation;
}

/************************************************************************/
/* 双线形插值法：获取某站点的值                                         */
/************************************************************************/
float HandleInterpolation::biInterpolation(float * fGridData, float fStartLon, float fStartLat, float fDi, float fDj, int nNi, int nNj, float fStationLon, float fStationLat)
{
	float fStaitonVal = 999.9f;
	int gi, gj; //要插值格点在原来格点场的位置--整数，使用该点及其右侧、下放的格点插值
	float u, v, w;
	float a1, a2;

	// 计算格点终止经纬度
	float fEndLon = fStartLon + fDi * (nNi-1);
	float fEndLat = fStartLat + fDj * (nNj - 1);

	// 站点坐标点位置判断
	// 经度判断
	if (fDi > 0)
	{
		if (fStationLon < fStartLon || fStationLon > fEndLon)
		{
			return fStaitonVal;
		}
	}
	else
	{
		if (fStationLon > fStartLon || fStationLon < fEndLon)
		{
			return fStaitonVal;
		}
	}
	// 纬度判断
	if (fDj > 0)
	{
		if (fStationLat < fStartLat || fStationLat > fEndLat)
		{
			return fStaitonVal;
		}
	}
	else
	{
		if (fStationLat > fStartLat || fStationLat < fEndLat)
		{
			return fStaitonVal;
		}
	}

	//根据站点经纬度，查找原格点数据中该点周围四个点
	gi = (int)((fStationLon - fStartLon) / fDi + 0.1);
	gj = (int)((fStationLat - fStartLat) / fDj + 0.1);
	if ((gj * nNi + gi) < 0 || ((gj + 1) * nNi + gi + 1) > nNi * nNj)
	{
		// 超出格点范围
		fStaitonVal = 999.9f; // 缺测值
	}
	else
	{
		//插值得到新点的值。--要处理边界的情况！
		u = fStationLon - (fStartLon + fDi * gi);
		v = fStationLat - (fStartLat + fDj * gj);

		u = u / fDi;
		v = v / fDj;
		if (u != 0)
		{
			a2 = (u)* fGridData[(gj + 1)* nNi + (gi + 1)] + (1 - u) * fGridData[gj * nNi + (gi + 1)];
			a1 = (u)* fGridData[(gj + 1)* nNi + gi] + (1 - u) * fGridData[gj* nNi + gi];
		}
		else
		{
			a1 = fGridData[gj * nNi + gi];
			a2 = fGridData[gj * nNi + (gi + 1)];
		}
		w = (v)* a2 + (1 - v) * a1;

		fStaitonVal = w;//
	}

	return fStaitonVal;
}

/************************************************************************/
/* 站点到格点的调整处理                                                 */
/************************************************************************/
/*
bool HandleInterpolation::AdjustmentGrid(float fStationData, float fStationLon, float fStationLat,  int nRadius, float * fGridData, float fStartLon, float fStartLat, float fDi, float fDj, int nNi, int nNj)
{
	// 参数判断
	if (fGridData == NULL)
	{
		return false;
	}
	if (nRadius < 1)
	{
		return false;
	}

	// 计算格点终止经纬度
	float fEndLon = fStartLon + fDi * (nNi - 1);
	float fEndLat = fStartLat + fDj * (nNj - 1);

	// 以正方向为准
	// 调整方法，以站点坐标为中心，以影响个数*该方向的格距为半径，修改其在矩形框中的格点坐标上的值（=站点值）
	float fLon1, fLon2, fLat1, fLat2; // 所要影响的经纬度范围
	fLon1 = fStationLon - nRadius * fDi; 
	fLon2 = fStationLon + nRadius * fDi;
	fLat1 = fStationLat - nRadius * fDj;
	fLat2 = fStationLat + nRadius * fDj;

	// 经度判断，如果矩形框与格点范围没有交集，则不处理
	if (fDi > 0)
	{
		if (fLon2 < fStartLon || fLon1 > fEndLon)
		{
			return false;
		}
	}
	else
	{
		if (fLon1 > fStartLon || fLon2 < fEndLon)
		{
			return false;
		}
	}
	// 纬度判断
	if (fDj > 0)
	{
		if (fLat2 < fStartLat || fLat1 > fEndLat)
		{
			return false;
		}
	}
	else
	{
		if (fLat1 > fStartLat || fLat2 < fEndLat)
		{
			return false;
		}
	}

	// 调整经纬度，以保证落在格点坐标内
	int  nCount;
	float fPostion;
	// 起始经度
	if (fLon1 < fStartLon)
	{
		// 起始经度太小
		fLon1 = fStartLon;
	}
	nCount = (int)((fLon1 - fStartLon) / fDi + 0.1);
	fPostion = fStartLon + fDi * nCount;
	if (IS_EQUAL(fLon1, fPostion))
	{
		fLon1 = fPostion;
	}
	else
	{
		fLon1 = fPostion + fDi;
	}

	// 终止经度
	if (fLon2> fEndLon)
	{
		// 终止经度太大
		fLon2 = fEndLon;
	}
	nCount = (int)((fLon2 - fStartLon) / fDi + 0.1);
	fPostion = fStartLon + fDi * nCount;
	fLon2 = fPostion;

	// 起始纬度
	if (fLat1 < fStartLat)
	{
		// 起始纬度太小
		fLat1 = fStartLat;
	}
	nCount = (int)((fLat1 - fStartLat) / fDj + 0.1);
	fPostion = fStartLat + fDj * nCount;
	if (IS_EQUAL(fLat1, fPostion))
	{
		fLat1 = fPostion;
	}
	else
	{
		fLat1 = fPostion + fDj;
	}

	// 终止纬度
	if (fLon2 > fEndLat)
	{
		// 终止纬度太大
		fLon2 = fEndLat;
	}
	nCount = (int)((fLon2 - fStartLat) / fDj + 0.1);
	fPostion = fStartLat + fDj * nCount;
	fLon2 = fPostion;

	// 3.2 截取条件数据
	int nLonCount = (int)((fLon2 - fLon1) / fDi + 0.1) + 1;  // 经度格点数
	int nLatCount = (int)((fLat2 - fLat1) / fDj + 0.1) + 1;  // 纬度格点数

	// 影响区域的起始坐标
	int nStarti = (int)((fLon1 - fStartLon) / fDi + 0.1);
	int nStartj = (int)((fLat1 - fStartLat) / fDj + 0.1);

	for (int i = nStarti; i < nStarti + nLonCount; i++)
	{
		for (int j = nStartj; j < nStartj + nLatCount; j++)
		{
			fGridData[j * nNi + i] = fStationData;
		}
	}

	return true;
}
*/
/************************************************************************/
/* cressman插值算法                                                     */
/************************************************************************/
bool HandleInterpolation::cressman(LIST_STATION_VALUE lstStationDiff, float fRadius, float * fGridData, float fStartLon, float fStartLat, float fDi, float fDj, int nNi, int nNj)
{
	// 站点值是订正值与原值的差值
	float fRadius2 = fRadius * fRadius; // 半径的平方

	float fWeightSum; // 权重之和
	float fWeightValSum; 

	// 构造一个差值格点场
	//float * fDiffData = (float *)malloc(nNi * nNj * sizeof(float));
	//if (fDiffData == NULL)
	//{
	//	return false;
	//}
	float fDiffVal = 0;
	for (int i = 0; i < nNi; i++)
	{
		for (int j = 0; j < nNj; j++)
		{
			// 格点插值 初始化为0
			//fDiffData[j* nNi + i] = 0;
			fDiffVal = 0;  

			// 该格点的经度
			float fLon = fStartLon + i * fDi;

			// 该格点的纬度
			float fLat = fStartLat + j * fDj;

			fWeightSum = 0;
			fWeightValSum = 0;

			// 查找半径影响范围内的站点
			LIST_STATION_VALUE::iterator iter;
			for (iter = lstStationDiff.begin(); iter != lstStationDiff.end(); iter++)
			{
				ST_STATION_VALUE * stStationDiff = *iter;

				// 该站点到该格点坐标点的距离的平方
				float fLonDis = stStationDiff->lon - fLon;
				float fLatDis = stStationDiff->lat - fLat;
				float fDistance2 = fLonDis * fLonDis + fLatDis * fLatDis;
				if (fDistance2 > fRadius2)
				{
					// 如果在半径范围意外，跳过改点
					continue;
				}

				float fWeight = (fRadius2 - fDistance2) / (fRadius2 + fDistance2);
				fWeightSum += fWeight; // 权重之和
				fWeightValSum += fWeight * fWeight *  stStationDiff->fValue;
			}

			// 如果权重和为0，则没有找到影响的站点
			if (IS_EQUAL_ZERO(fWeightSum))
			{
				continue;
			}

			// 计算格点差值
			fDiffVal = fWeightValSum / fWeightSum;

			// 计算新的格点值
			fGridData[j* nNi + i] += fDiffVal;

		}
	}

	//// 订正格点数据
	//for (int i = 0; i < nNi * nNj; i++)
	//{
	//	fGridData[i] += fDiffData[i];
	//}
	//
	//// 释放空间
	//if (fDiffData)
	//{
	//	free(fDiffData);
	//	fDiffData = NULL;
	//}

	return true;
}

