// nwfd-dataproc-win32.cpp : 定义 DLL 应用程序的导出函数。
//

#include "nwfd-dataproc-win32.h"

/* 1. 格点到站点插值处理 */
//int nwfd_get_grid2station_data(float * fGridData, float fStartLon, float fStartLat, float fEndLon, float fEndLat, float fDi, float fDj, ST_STATION_VALUE* &stStation, int nStationNum)
//{
//	if (fGridData == NULL || stStation == NULL)
//	{
//		return -2;
//	}
//
//	float fStaitonVal = 999.9f;
//	int gi, gj; //要插值格点在原来格点场的位置--整数，使用该点及其右侧、下放的格点插值
//	float u, v, w;
//	float a1, a2;
//
//	int nNi = (int)((fEndLon - fStartLon) / fDi + 1.1);
//	int nNj = (int)((fEndLat - fEndLat) / fDj + 1.1);
//
//	for (int n = 0; n < nStationNum; n++)
//	{
//		float fStationLon = stStation[n].lon;
//		float fStationLat = stStation[n].lat;
//
//		// 站点坐标点位置判断
//		// 经度判断
//		if (fDi > 0)
//		{
//			if (fStationLon < fStartLon || fStationLon > fEndLon)
//			{
//				stStation[n].fValue = 999.9f;
//				continue;
//			}
//		}
//		else
//		{
//			if (fStationLon > fStartLon || fStationLon < fEndLon)
//			{
//				stStation[n].fValue = 999.9f;
//				continue;
//			}
//		}
//
//		// 纬度判断
//		if (fDj > 0)
//		{
//			if (fStationLat < fStartLat || fStationLat > fEndLat)
//			{
//				stStation[n].fValue = 999.9f;
//				continue;
//			}
//		}
//		else
//		{
//			if (fStationLat > fStartLat || fStationLat < fEndLat)
//			{
//				stStation[n].fValue = 999.9f;
//				continue;
//			}
//		}
//
//		//根据站点经纬度，查找原格点数据中该点周围四个点
//		gi = (int)((fStationLon - fStartLon) / fDi + 0.1);
//		gj = (int)((fStationLat - fStartLat) / fDj + 0.1);
//		if ((gj * nNi + gi) < 0 || ((gj + 1) * nNi + gi + 1) > nNi * nNj)
//		{
//			// 超出格点范围
//			fStaitonVal = 999.9f; // 缺测值
//		}
//		else
//		{
//			//插值得到新点的值。--要处理边界的情况！
//			u = fStationLon - (fStartLon + fDi * gi);
//			v = fStationLat - (fStartLat + fDj * gj);
//
//			u = u / fDi;
//			v = v / fDj;
//			if (u != 0)
//			{
//				a2 = (u)* fGridData[(gj + 1)* nNi + (gi + 1)] + (1 - u) * fGridData[gj * nNi + (gi + 1)];
//				a1 = (u)* fGridData[(gj + 1)* nNi + gi] + (1 - u) * fGridData[gj* nNi + gi];
//			}
//			else
//			{
//				a1 = fGridData[gj * nNi + gi];
//				a2 = fGridData[gj * nNi + (gi + 1)];
//			}
//			w = (v)* a2 + (1 - v) * a1;
//
//			fStaitonVal = w;
//		}
//
//		// 插值计算得到的站点值
//		stStation[n].fValue = fStaitonVal;
//	}
//
//	return 0;
//}

/* 1. 格点到站点插值处理 */
int nwfd_get_grid2station_data(float * fGridData, float fStartLon, float fStartLat, float fEndLon, float fEndLat, float fDi, float fDj, ST_STATION_VALUE* &stStation, int nStationNum)
{
#ifdef USE_LOG
	FILE *m_fp = NULL;
	fopen_s(&m_fp,"./testdata.log", "wt");
#endif

#ifdef USE_LOG
	int n = (int)((fEndLon - fStartLon) / fDi + 1.1);
	int m = (int)((fEndLat - fStartLat) / fDj + 1.1);
	int off = 0;
	fprintf(m_fp, "%0.2f  %0.2f   %0.2f   %0.2f (%d,%d) \n", fStartLon, fStartLat, fEndLon, fEndLat, n,m);

	//for (int i = 0; i < n; i++)
	//{
	//	for (int j = 0; j < m; j++)
	//	{
	//		fprintf(m_fp, "%0.2f \n", n, fGridData[off++]);
	//	}
	//	fprintf(m_fp, "\n");
	//	fflush(m_fp);
	//}

	//for (int i = 0; i < n * m; i++)
	//{
	//	fprintf(m_fp, "[%04d] %0.2f \n", n, fGridData[i]);
	//}
	//fflush(m_fp);
#endif

	if (fGridData == NULL || stStation == NULL || nStationNum <= 0)
	{
		return -1;
	}

	float fStaitonVal = 999.9f;
	int gi, gj; //要插值格点在原来格点场的位置--整数，使用该点及其右侧、下放的格点插值
	float u, v, w;
	float a1, a2;

	int nNi = (int)((fEndLon - fStartLon) / fDi + 1.1);
	int nNj = (int)((fEndLat - fStartLat) / fDj + 1.1);

	for (int n = 0; n < nStationNum; n++)
	{
#ifdef USE_LOG
		if (m_fp)
		{
			//fprintf(m_fp, "[%d] In  >> %0.2f  %0.2f  %0.2f \n", n,  stInStation[n].lon, stInStation[n].lat, stInStation[n].height);
			fprintf(m_fp, "[%d] get >> %0.2f  %0.2f  %0.2f \n", n,  stStation[n].lon, stStation[n].lat, stStation[n].height);
			fflush(m_fp);
		}
#endif
		
		float fStationLon = stStation[n].lon;
		float fStationLat = stStation[n].lat;

		// 站点坐标点位置判断
		// 经度判断
		if (fDi > 0)
		{
			if (fStationLon < fStartLon || fStationLon > fEndLon)
			{
				stStation[n].fValue = 999.9f;
				continue;
			}
		}
		else
		{
			if (fStationLon > fStartLon || fStationLon < fEndLon)
			{
				stStation[n].fValue = 999.9f;
				continue;
			}
		}

		// 纬度判断
		if (fDj > 0)
		{
			if (fStationLat < fStartLat || fStationLat > fEndLat)
			{
				stStation[n].fValue = 999.9f;
				continue;
			}
		}
		else
		{
			if (fStationLat > fStartLat || fStationLat < fEndLat)
			{
				stStation[n].fValue = 999.9f;
				continue;
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

			fStaitonVal = w;
		}

		// 插值计算得到的站点值
		stStation[n].fValue = fStaitonVal;

#ifdef USE_LOG
		if (m_fp)
		{
			fprintf(m_fp, "[%d] Value  >>  %0.2f \n",n, stStation[n].fValue);
			fflush(m_fp);
		}
#endif
	}

#ifdef USE_LOG
	if (m_fp)
	{
		fclose(m_fp);
	}
#endif

	return 0;
}


void nwfd_free_staion(ST_STATION_VALUE* &stStation)
{
	if (stStation)
	{
		free(stStation);
		stStation = NULL;
	}
}

/* 2. 站点差值到格点的订正处理 */
int nwfd_get_station2grid_data(ST_STATION_VALUE* stStationDiff, int nStationNum, float fRadius, float* &fGridData, float fStartLon, float fStartLat, float fEndLon, float fEndLat, float fDi, float fDj)
{
#ifdef USE_LOG
	FILE *m_fp = NULL;
	fopen_s(&m_fp, "./testdata.log", "wt");
#endif

#ifdef USE_LOG
	fprintf(m_fp, "nStationNum=%d  fRadius=%0.2f   (%0.2f  %0.2f  %0.2f  %0.2f) (%0.2f,%0.2f) \n", nStationNum, fRadius, fStartLon, fStartLat, fEndLon, fEndLat, fDi, fDj);

	for (int n = 0; n < nStationNum; n++)
	{
		fprintf(m_fp, "[%d] get >> (%0.2f  %0.2f)  Val=%0.2f \n", n, stStationDiff[n].lon, stStationDiff[n].lat, stStationDiff[n].fValue);
	}

	fflush(m_fp);
#endif

	if (stStationDiff == NULL || fGridData == NULL)
	{
		return -2;
	}

	float fRadius2 = fRadius * fRadius; // 半径的平方

	float fWeightSum; // 权重之和
	float fWeightValSum;

	float fDiffVal = 0;

	int nNi = (int)((fEndLon - fStartLon) / fDi + 1.1);
	int nNj = (int)((fEndLat - fStartLat) / fDj + 1.1);

	for (int i = 0; i < nNi; i++)
	{
		for (int j = 0; j < nNj; j++)
		{
			// 格点插值 初始化为0
			fDiffVal = 0;

			// 该格点的经度
			float fLon = fStartLon + i * fDi;

			// 该格点的纬度
			float fLat = fStartLat + j * fDj;

			fWeightSum = 0;
			fWeightValSum = 0;

			// 查找半径影响范围内的站点
			for (int n = 0; n < nStationNum; n++)
			{
				// 剔除缺测值
				if (IS_EQUAL(stStationDiff[n].fValue, 999.9))
				{
					continue;
				}

				// 该站点到该格点坐标点的距离的平方
				float fLonDis = stStationDiff[n].lon - fLon;
				float fLatDis = stStationDiff[n].lat - fLat;
				float fDistance2 = fLonDis * fLonDis + fLatDis * fLatDis;
				if (fDistance2 > fRadius2)
				{
					// 如果在半径范围意外，跳过改点
					continue;
				}

				float fWeight = (fRadius2 - fDistance2) / (fRadius2 + fDistance2);
				fWeightSum += fWeight; // 权重之和
				fWeightValSum += fWeight * fWeight *  stStationDiff[n].fValue;
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

#ifdef USE_LOG
			if (m_fp)
			{
				fprintf(m_fp, "SET >> (%0.2f  %0.2f)  Val=%0.2f   pos=%d\n", fLon, fLat, fDiffVal, j* nNi + i);
				fflush(m_fp);
			}
#endif

		}
	}

#ifdef USE_LOG
	if (m_fp)
	{
		fclose(m_fp);
	}
#endif

	return 0;
}

/* 3. 降水一致性处理 */
int nwfd_get_rain_data(float * fOrigRainData, int nOrigDataCnt, int nOrigRange, float* &fDestRainData, int nDestDataCnt, int nDestRange)
{
	// 参数判断
	if (fOrigRainData == NULL || fDestRainData == NULL)
	{
		return -2;
	}

	// 处理判断
	if (nOrigRange > nDestRange)
	{
		// 向下拆分
		return HandleRainCorrection_Downward(fOrigRainData, nOrigDataCnt, nOrigRange, fDestRainData, nDestDataCnt, nDestRange);
	}
	else if (nOrigRange < nDestRange)
	{
		// 向上合并
		return HandleRainCorrection_Upward(fOrigRainData, nOrigDataCnt, nOrigRange, fDestRainData, nDestDataCnt, nDestRange);
	}
	else
	{
		return 0;
	}

	return 0;
}

/************************************************************************/
/* 降水处理,向下拆分                                                    */
/************************************************************************/
int HandleRainCorrection_Downward(float * fOrigRainData, int nOrigDataCnt, int nOrigRange, float* &fDestRainData, int nDestDataCnt, int nDestRange)
{
	// 间隔判断与数据长度判断
	int nCnt = nOrigRange / nDestRange;
	int nDataCnt = nDestDataCnt / nOrigDataCnt;
	if (nCnt != nDataCnt)
	{
		return -10;
	}

	// 调整数据
	float fRainSum = 0;
	float fRainVal = 0;
	for (int i = 0; i < nOrigDataCnt; i++)
	{
		// 如果日降水量小于0.8，则不进行调整
		if (fOrigRainData[i] < 0)  // if (fOrigRainData[i] < 0.8)
		{
			continue;
		}
		else if (IS_EQUAL(fOrigRainData[i], 9999.0))
		{
			// 缺测值，特殊处理
			for (int n = 0; n < nCnt; n++)
			{
				fDestRainData[n * nOrigDataCnt + i] = fOrigRainData[i];
			}

			continue;
		}

		// 否则，累加1日内的3小时指导报
		fRainSum = 0;
		for (int n = 0; n < nCnt; n++)
		{
			fRainSum += fDestRainData[n * nOrigDataCnt + i];
		}

		// 如果累计量=0
		if (IS_EQUAL_ZERO(fRainSum))
		{
			// 平均分布降水量
			fRainVal = fOrigRainData[i] / nCnt;
			for (int n = 0; n < nCnt; n++)
			{
				fDestRainData[n * nOrigDataCnt + i] = fRainVal;
			}
		}
		else
		{
			// 按比例调整降水量
			for (int n = 0; n < nCnt; n++)
			{
				fDestRainData[n * nOrigDataCnt + i] *= fOrigRainData[i] / fRainSum;
			}
		}
	}

	return 0;
}

/************************************************************************/
/* 降水处理,向上合并                                                    */
/************************************************************************/
int HandleRainCorrection_Upward(float * fOrigRainData, int nOrigDataCnt, int nOrigRange, float* &fDestRainData, int nDestDataCnt, int nDestRange)
{
	// 间隔判断与数据长度判断
	int nCnt = nDestRange / nOrigRange;
	int nDataCnt = nOrigDataCnt / nDestDataCnt;
	if (nCnt != nDataCnt)
	{
		return -10;
	}

	for (int i = 0; i < nDestDataCnt; i++)
	{
		// 累加1日内的3小时指导报
		float fRainSum = -1;
		for (int n = 0; n < nCnt; n++)
		{
			if (IS_EQUAL(fOrigRainData[n * nDestDataCnt + i], 9999.0))
			{
				// 缺测值
				continue;
			}

			if (fRainSum < 0)
			{
				fRainSum = fOrigRainData[n * nDestDataCnt + i];
			}
			else
			{
				fRainSum += fOrigRainData[n * nDestDataCnt + i];
			}
		}

		// 累加值合并
		if (fRainSum < 0)
		{
			fDestRainData[i] = 9999.0;   // 缺测
		}
		else
		{
			fDestRainData[i] = fRainSum; // 累加值
		}
	}

	return 0;
}

/************************************************************************/
/* 经纬度范围处理                                                       */
/************************************************************************/
int proccess_lon_lat(float *fInData, nwfd_grib2_field &stInField, float fStartLon, float fEndLon, float fStartLat, float fEndLat,
	float * &fOutData, nwfd_grib2_field &stOutFiled, float fMissingVal)
{
	// 1、输入参数判断
	if (fInData == NULL)
	{
		// 参数错误
		return -1;
	}

	// 2、 修订获取数据的经纬度，要正好在格点中
	fStartLon = (int)(fStartLon / stInField.incrementi) * stInField.incrementi;
	fStartLat = (int)(fStartLat / stInField.incrementj) * stInField.incrementj;

	float fEndLonTmp = (int)(fEndLon / stInField.incrementi) * stInField.incrementi;
	fEndLon = (fEndLonTmp < fEndLon) ? (fEndLonTmp + stInField.incrementi) : fEndLonTmp;
	float fEndLatTmp = (int)(fEndLat / stInField.incrementj) * stInField.incrementj;
	fEndLat = (fEndLatTmp < fEndLat) ? (fEndLatTmp + stInField.incrementj) : fEndLatTmp;

	// 3、 获取数据的个数
	int nLonCount = (int)((fEndLon - fStartLon) / stInField.incrementi + 1.1);  // 经度格点数
	int nLatCount = (int)((fEndLat - fStartLat) / stInField.incrementj + 1.1);  // 纬度格点数

	// 4、 申请数据存储空间
	fOutData = (float *)malloc(nLonCount * nLatCount * sizeof(float));
	if (fOutData == NULL)
	{
		return -2;
	}

	// 5、获取区域块数据
	float fLon, fLat;
	int nPos = 0;
	int nOff = 0;
	for (int j = 0; j < nLatCount; j++)
	{
		// 此点的纬度
		fLat = fStartLat + j * stInField.incrementj;
		for (int i = 0; i < nLonCount; i++)
		{
			// 此点的经度
			fLon = fStartLon + i * stInField.incrementi;

			// 在原数据的位置
			int y = (int)((fLon - stInField.lon1) / stInField.incrementi + 0.0001);
			int x = (int)((fLat - stInField.lat1) / stInField.incrementj + 0.0001);

			if (x < 0 || x >(stInField.Nj - 1) || y < 0 || y >(stInField.Ni - 1) || (fLon < stInField.lon1) || (fLat < stInField.lat1))
			{
				// 没有该数据,用缺测数据填补
				fOutData[nOff++] = fMissingVal;
			}
			else
			{
				// 找到该数据
				nPos = x * stInField.Ni + y;
				fOutData[nOff++] = fInData[nPos];
			}
		}
	}

	// 返回数据信息，修正经纬度范围和个数
	stOutFiled = stInField;
	stOutFiled.lon1 = fStartLon;
	stOutFiled.lon2 = fEndLon;
	stOutFiled.lat1 = fStartLat;
	stOutFiled.lat2 = fEndLat;
	stOutFiled.Ni = nLonCount;
	stOutFiled.Nj = nLatCount;

	return 0;
}

/************************************************************************/
/* 分辨率处理                                                           */
/************************************************************************/
int proccess_di_dj(float *fInData, nwfd_grib2_field &stInField, float fInDi, float fInDj, float fOutDi, float fOutDj,
	float * &fOutData, nwfd_grib2_field &stOutField, bool warningdata, float fMissingVal)
{
	// 参数判断
	if (fInData == NULL)
	{
		return -1;
	}

	if (IS_EQUAL(fInDi, fOutDi) && IS_EQUAL(fInDj, fOutDj))
	{
		// 不需要变化,正常返回
		fOutData = (float *)malloc(stInField.Ni * stInField.Nj * sizeof(float));
		if (fOutData)
		{
			memcpy(fOutData, fInData, stInField.Ni * stInField.Nj * sizeof(float));
		}

		stOutField = stInField;
	}
	else if (fInDi > fOutDi && fInDj > fOutDj)
	{
		// 降尺度处理
		return proccess_di_dj_down(fInData, stInField, fInDi, fInDj, fOutDi, fOutDj, fOutData, stOutField, warningdata, fMissingVal);
	}
	else if (fInDi < fOutDi && fInDj < fOutDj)
	{
		// 升尺度处理
		return proccess_di_dj_up(fInData, stInField, fInDi, fInDj, fOutDi, fOutDj, fOutData, stOutField, fMissingVal);
	}
	else
	{
		return -2;
	}

	return 0;
}

/************************************************************************/
/* 分辨率处理-降尺度                                                    */
/************************************************************************/
int proccess_di_dj_down(float *fInData, nwfd_grib2_field &stInField, float fInDi, float fInDj, float fOutDi, float fOutDj,
	float * &fOutData, nwfd_grib2_field &stOutField, bool warningdata, float fMissingVal)
{
	if (fInData == NULL)
	{
		return -1;
	}

	// 整倍处理已在之前验证，所以不再校验
	int n = (int)(fInDi / fOutDi + 0.1); // 经度方向
	int m = (int)(fInDj / fOutDj + 0.1); // 纬度方向

	// 输出格点数计算
	int nOutNi = (stInField.Ni - 1) * n + 1;
	int nOutNj = (stInField.Nj - 1) * m + 1;

	// 降尺度处理的经纬范围与原先的一致，所以不用重新计算

	// 创建输出数据存储空间
	fOutData = (float *)malloc(nOutNi * nOutNj * sizeof(float));
	if (fOutData == NULL)
	{
		// 申请空间失败
		return -2;
	}

	// 降尺度处理
	int i, j;     // 元数据的坐标点
	int x, y;     // 插值后的坐标点
	int s, t;
	float fA, fB, fC, fD; // 元数据的四个格点值
	float fVal;           // 插值计算后的值
	int pos;

	// 处理一组数据，按行扫描
	for (j = 0; j < stInField.Nj - 1; j++)  // 原数据 纬度方向
	{
		for (i = 0; i < stInField.Ni - 1; i++) // 原数据 经度方向
		{
			// 格点值
			fA = fInData[j * stInField.Ni + i];
			fB = fInData[j * stInField.Ni + i + 1];
			fC = fInData[(j + 1) * stInField.Ni + i];
			fD = fInData[(j + 1) * stInField.Ni + i + 1];

			// 插值处理
			for (s = 0; s <= n; s++) // 经度方向
			{
				for (t = 0; t <= m; t++) // 纬度方向
				{
					// 插值后的坐标点
					x = i * n + s;
					y = j * m + t;
					pos = y * nOutNi + x;

					if (warningdata)
					{
						// 预警数据，不进行插值处理，只根据距离取最近的值
						if (s <= n / 2)
						{
							if (t <= m / 2)
								fOutData[pos] = fA;
							else
								fOutData[pos] = fC;
						}
						else
						{
							if (t <= m / 2)
								fOutData[pos] = fB;
							else
								fOutData[pos] = fD;
						}
					}
					else
					{
						// 数值计算
						fVal = fA + (fC - fA) / m * t + (fB + (fD - fB) / m * t - fA - (fC - fA) / m * t) / n * s;

						// 赋值
						fOutData[pos] = fVal;

						// 缺测值判断重新进行缺测值赋值
						if (IS_EQUAL(fA, fMissingVal))
						{
							if ((s >= 0 && s < n) && (t >= 0 && t < m))
							{
								fOutData[pos] = fMissingVal;
							}
						}
						if (IS_EQUAL(fB, fMissingVal))
						{
							if ((s >= 1 && s <= n) && (t >= 0 && t < m))
							{
								fOutData[pos] = fMissingVal;
							}
						}
						if (IS_EQUAL(fC, fMissingVal))
						{
							if ((s >= 0 && s < n) && (t >= 1 && t <= m))
							{
								fOutData[pos] = fMissingVal;
							}
						}
						if (IS_EQUAL(fD, fMissingVal))
						{
							if ((s >= 1 && s <= n) && (t >= 1 && t <= m))
							{
								fOutData[pos] = fMissingVal;
							}
						}
					}
				}
			}
		}
	}

	// 返回信息赋值
	stOutField = stInField;
	stOutField.incrementi = fOutDi;  // 分辨率
	stOutField.incrementj = fOutDj;
	stOutField.Ni = nOutNi;          // 个数
	stOutField.Nj = nOutNj;

	return 0;
}

/************************************************************************/
/* 分辨率处理-升尺度                                                    */
/************************************************************************/
int proccess_di_dj_up(float *fInData, nwfd_grib2_field &stInField, float fInDi, float fInDj, float fOutDi, float fOutDj,
	float * &fOutData, nwfd_grib2_field &stOutField, float fMissingVal)
{
	if (fInData == NULL)
	{
		return -1;
	}

	// 整倍处理已在之前验证，所以不再校验
	int n = (int)(fOutDi / fInDi + 0.1); // 经度方向
	int m = (int)(fOutDj / fInDj + 0.1); // 纬度方向

	// 输出格点数计算
	int nOutNi = (stInField.Ni - 1) / n + 1;
	nOutNi = ((stInField.Ni - 1) % n == 0) ? nOutNi : nOutNi + 1;
	int nOutNj = (stInField.Nj - 1) / m + 1;
	nOutNj = ((stInField.Nj - 1) % m == 0) ? nOutNj : nOutNj + 1;

	// 输出经纬度计算
	float fOutLon1 = stInField.lon1;
	float fOutLon2 = fOutLon1 + (nOutNi - 1) * fOutDi;
	float fOutLat1 = stInField.lat1;
	float fOutLat2 = fOutLat1 + (nOutNj - 1) * fOutDj;

	// 创建输出数据存储空间
	fOutData = (float *)malloc(nOutNi * nOutNj * sizeof(float));
	if (fOutData == NULL)
	{
		// 申请空间失败
		return -2;
	}

	float fLat, fLon;
	int nOff = 0;
	int nPos;
	// 处理一组数据，按行扫描
	for (int j = 0; j < nOutNj; j++)
	{
		fLat = fOutLat1 + j * fOutDj;
		for (int i = 0; i < nOutNi; i++)
		{
			fLon = fOutLon1 + i * fOutDi;

			// 在原数据的位置
			int y = (int)((fLon - stInField.lon1) / stInField.incrementi + 0.0001);
			int x = (int)((fLat - stInField.lat1) / stInField.incrementj + 0.0001);

			if (x < 0 || x >(stInField.Nj - 1) || y < 0 || y >(stInField.Ni - 1) || (fLon < stInField.lon1) || (fLat < stInField.lat1))
			{
				// 没有该数据,用缺测数据填补
				fOutData[nOff++] = fMissingVal;
			}
			else
			{
				// 找到该数据
				nPos = x * stInField.Ni + y;
				fOutData[nOff++] = fInData[nPos];
			}

		}
	}

	// 返回信息赋值
	stOutField = stInField;
	stOutField.incrementi = fOutDi;  // 分辨率
	stOutField.incrementj = fOutDj;
	stOutField.Ni = nOutNi;          // 个数
	stOutField.Nj = nOutNj;
	stOutField.lon1 = fOutLon1;      // 经纬度会进行微调整
	stOutField.lon2 = fOutLon2;
	stOutField.lat1 = fOutLat1;
	stOutField.lat2 = fOutLat2;

	return 0;
}
