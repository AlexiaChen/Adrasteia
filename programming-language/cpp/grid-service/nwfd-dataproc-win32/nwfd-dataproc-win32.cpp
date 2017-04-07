// nwfd-dataproc-win32.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "nwfd-dataproc-win32.h"

/* 1. ��㵽վ���ֵ���� */
//int nwfd_get_grid2station_data(float * fGridData, float fStartLon, float fStartLat, float fEndLon, float fEndLat, float fDi, float fDj, ST_STATION_VALUE* &stStation, int nStationNum)
//{
//	if (fGridData == NULL || stStation == NULL)
//	{
//		return -2;
//	}
//
//	float fStaitonVal = 999.9f;
//	int gi, gj; //Ҫ��ֵ�����ԭ����㳡��λ��--������ʹ�øõ㼰���Ҳࡢ�·ŵĸ���ֵ
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
//		// վ�������λ���ж�
//		// �����ж�
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
//		// γ���ж�
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
//		//����վ�㾭γ�ȣ�����ԭ��������иõ���Χ�ĸ���
//		gi = (int)((fStationLon - fStartLon) / fDi + 0.1);
//		gj = (int)((fStationLat - fStartLat) / fDj + 0.1);
//		if ((gj * nNi + gi) < 0 || ((gj + 1) * nNi + gi + 1) > nNi * nNj)
//		{
//			// ������㷶Χ
//			fStaitonVal = 999.9f; // ȱ��ֵ
//		}
//		else
//		{
//			//��ֵ�õ��µ��ֵ��--Ҫ����߽�������
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
//		// ��ֵ����õ���վ��ֵ
//		stStation[n].fValue = fStaitonVal;
//	}
//
//	return 0;
//}

/* 1. ��㵽վ���ֵ���� */
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
	int gi, gj; //Ҫ��ֵ�����ԭ����㳡��λ��--������ʹ�øõ㼰���Ҳࡢ�·ŵĸ���ֵ
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

		// վ�������λ���ж�
		// �����ж�
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

		// γ���ж�
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

		//����վ�㾭γ�ȣ�����ԭ��������иõ���Χ�ĸ���
		gi = (int)((fStationLon - fStartLon) / fDi + 0.1);
		gj = (int)((fStationLat - fStartLat) / fDj + 0.1);
		if ((gj * nNi + gi) < 0 || ((gj + 1) * nNi + gi + 1) > nNi * nNj)
		{
			// ������㷶Χ
			fStaitonVal = 999.9f; // ȱ��ֵ
		}
		else
		{
			//��ֵ�õ��µ��ֵ��--Ҫ����߽�������
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

		// ��ֵ����õ���վ��ֵ
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

/* 2. վ���ֵ�����Ķ������� */
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

	float fRadius2 = fRadius * fRadius; // �뾶��ƽ��

	float fWeightSum; // Ȩ��֮��
	float fWeightValSum;

	float fDiffVal = 0;

	int nNi = (int)((fEndLon - fStartLon) / fDi + 1.1);
	int nNj = (int)((fEndLat - fStartLat) / fDj + 1.1);

	for (int i = 0; i < nNi; i++)
	{
		for (int j = 0; j < nNj; j++)
		{
			// ����ֵ ��ʼ��Ϊ0
			fDiffVal = 0;

			// �ø��ľ���
			float fLon = fStartLon + i * fDi;

			// �ø���γ��
			float fLat = fStartLat + j * fDj;

			fWeightSum = 0;
			fWeightValSum = 0;

			// ���Ұ뾶Ӱ�췶Χ�ڵ�վ��
			for (int n = 0; n < nStationNum; n++)
			{
				// �޳�ȱ��ֵ
				if (IS_EQUAL(stStationDiff[n].fValue, 999.9))
				{
					continue;
				}

				// ��վ�㵽�ø�������ľ����ƽ��
				float fLonDis = stStationDiff[n].lon - fLon;
				float fLatDis = stStationDiff[n].lat - fLat;
				float fDistance2 = fLonDis * fLonDis + fLatDis * fLatDis;
				if (fDistance2 > fRadius2)
				{
					// ����ڰ뾶��Χ���⣬�����ĵ�
					continue;
				}

				float fWeight = (fRadius2 - fDistance2) / (fRadius2 + fDistance2);
				fWeightSum += fWeight; // Ȩ��֮��
				fWeightValSum += fWeight * fWeight *  stStationDiff[n].fValue;
			}

			// ���Ȩ�غ�Ϊ0����û���ҵ�Ӱ���վ��
			if (IS_EQUAL_ZERO(fWeightSum))
			{
				continue;
			}

			// �������ֵ
			fDiffVal = fWeightValSum / fWeightSum;

			// �����µĸ��ֵ
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

/* 3. ��ˮһ���Դ��� */
int nwfd_get_rain_data(float * fOrigRainData, int nOrigDataCnt, int nOrigRange, float* &fDestRainData, int nDestDataCnt, int nDestRange)
{
	// �����ж�
	if (fOrigRainData == NULL || fDestRainData == NULL)
	{
		return -2;
	}

	// �����ж�
	if (nOrigRange > nDestRange)
	{
		// ���²��
		return HandleRainCorrection_Downward(fOrigRainData, nOrigDataCnt, nOrigRange, fDestRainData, nDestDataCnt, nDestRange);
	}
	else if (nOrigRange < nDestRange)
	{
		// ���Ϻϲ�
		return HandleRainCorrection_Upward(fOrigRainData, nOrigDataCnt, nOrigRange, fDestRainData, nDestDataCnt, nDestRange);
	}
	else
	{
		return 0;
	}

	return 0;
}

/************************************************************************/
/* ��ˮ����,���²��                                                    */
/************************************************************************/
int HandleRainCorrection_Downward(float * fOrigRainData, int nOrigDataCnt, int nOrigRange, float* &fDestRainData, int nDestDataCnt, int nDestRange)
{
	// ����ж������ݳ����ж�
	int nCnt = nOrigRange / nDestRange;
	int nDataCnt = nDestDataCnt / nOrigDataCnt;
	if (nCnt != nDataCnt)
	{
		return -10;
	}

	// ��������
	float fRainSum = 0;
	float fRainVal = 0;
	for (int i = 0; i < nOrigDataCnt; i++)
	{
		// ����ս�ˮ��С��0.8���򲻽��е���
		if (fOrigRainData[i] < 0)  // if (fOrigRainData[i] < 0.8)
		{
			continue;
		}
		else if (IS_EQUAL(fOrigRainData[i], 9999.0))
		{
			// ȱ��ֵ�����⴦��
			for (int n = 0; n < nCnt; n++)
			{
				fDestRainData[n * nOrigDataCnt + i] = fOrigRainData[i];
			}

			continue;
		}

		// �����ۼ�1���ڵ�3Сʱָ����
		fRainSum = 0;
		for (int n = 0; n < nCnt; n++)
		{
			fRainSum += fDestRainData[n * nOrigDataCnt + i];
		}

		// ����ۼ���=0
		if (IS_EQUAL_ZERO(fRainSum))
		{
			// ƽ���ֲ���ˮ��
			fRainVal = fOrigRainData[i] / nCnt;
			for (int n = 0; n < nCnt; n++)
			{
				fDestRainData[n * nOrigDataCnt + i] = fRainVal;
			}
		}
		else
		{
			// ������������ˮ��
			for (int n = 0; n < nCnt; n++)
			{
				fDestRainData[n * nOrigDataCnt + i] *= fOrigRainData[i] / fRainSum;
			}
		}
	}

	return 0;
}

/************************************************************************/
/* ��ˮ����,���Ϻϲ�                                                    */
/************************************************************************/
int HandleRainCorrection_Upward(float * fOrigRainData, int nOrigDataCnt, int nOrigRange, float* &fDestRainData, int nDestDataCnt, int nDestRange)
{
	// ����ж������ݳ����ж�
	int nCnt = nDestRange / nOrigRange;
	int nDataCnt = nOrigDataCnt / nDestDataCnt;
	if (nCnt != nDataCnt)
	{
		return -10;
	}

	for (int i = 0; i < nDestDataCnt; i++)
	{
		// �ۼ�1���ڵ�3Сʱָ����
		float fRainSum = -1;
		for (int n = 0; n < nCnt; n++)
		{
			if (IS_EQUAL(fOrigRainData[n * nDestDataCnt + i], 9999.0))
			{
				// ȱ��ֵ
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

		// �ۼ�ֵ�ϲ�
		if (fRainSum < 0)
		{
			fDestRainData[i] = 9999.0;   // ȱ��
		}
		else
		{
			fDestRainData[i] = fRainSum; // �ۼ�ֵ
		}
	}

	return 0;
}

/************************************************************************/
/* ��γ�ȷ�Χ����                                                       */
/************************************************************************/
int proccess_lon_lat(float *fInData, nwfd_grib2_field &stInField, float fStartLon, float fEndLon, float fStartLat, float fEndLat,
	float * &fOutData, nwfd_grib2_field &stOutFiled, float fMissingVal)
{
	// 1����������ж�
	if (fInData == NULL)
	{
		// ��������
		return -1;
	}

	// 2�� �޶���ȡ���ݵľ�γ�ȣ�Ҫ�����ڸ����
	fStartLon = (int)(fStartLon / stInField.incrementi) * stInField.incrementi;
	fStartLat = (int)(fStartLat / stInField.incrementj) * stInField.incrementj;

	float fEndLonTmp = (int)(fEndLon / stInField.incrementi) * stInField.incrementi;
	fEndLon = (fEndLonTmp < fEndLon) ? (fEndLonTmp + stInField.incrementi) : fEndLonTmp;
	float fEndLatTmp = (int)(fEndLat / stInField.incrementj) * stInField.incrementj;
	fEndLat = (fEndLatTmp < fEndLat) ? (fEndLatTmp + stInField.incrementj) : fEndLatTmp;

	// 3�� ��ȡ���ݵĸ���
	int nLonCount = (int)((fEndLon - fStartLon) / stInField.incrementi + 1.1);  // ���ȸ����
	int nLatCount = (int)((fEndLat - fStartLat) / stInField.incrementj + 1.1);  // γ�ȸ����

	// 4�� �������ݴ洢�ռ�
	fOutData = (float *)malloc(nLonCount * nLatCount * sizeof(float));
	if (fOutData == NULL)
	{
		return -2;
	}

	// 5����ȡ���������
	float fLon, fLat;
	int nPos = 0;
	int nOff = 0;
	for (int j = 0; j < nLatCount; j++)
	{
		// �˵��γ��
		fLat = fStartLat + j * stInField.incrementj;
		for (int i = 0; i < nLonCount; i++)
		{
			// �˵�ľ���
			fLon = fStartLon + i * stInField.incrementi;

			// ��ԭ���ݵ�λ��
			int y = (int)((fLon - stInField.lon1) / stInField.incrementi + 0.0001);
			int x = (int)((fLat - stInField.lat1) / stInField.incrementj + 0.0001);

			if (x < 0 || x >(stInField.Nj - 1) || y < 0 || y >(stInField.Ni - 1) || (fLon < stInField.lon1) || (fLat < stInField.lat1))
			{
				// û�и�����,��ȱ�������
				fOutData[nOff++] = fMissingVal;
			}
			else
			{
				// �ҵ�������
				nPos = x * stInField.Ni + y;
				fOutData[nOff++] = fInData[nPos];
			}
		}
	}

	// ����������Ϣ��������γ�ȷ�Χ�͸���
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
/* �ֱ��ʴ���                                                           */
/************************************************************************/
int proccess_di_dj(float *fInData, nwfd_grib2_field &stInField, float fInDi, float fInDj, float fOutDi, float fOutDj,
	float * &fOutData, nwfd_grib2_field &stOutField, bool warningdata, float fMissingVal)
{
	// �����ж�
	if (fInData == NULL)
	{
		return -1;
	}

	if (IS_EQUAL(fInDi, fOutDi) && IS_EQUAL(fInDj, fOutDj))
	{
		// ����Ҫ�仯,��������
		fOutData = (float *)malloc(stInField.Ni * stInField.Nj * sizeof(float));
		if (fOutData)
		{
			memcpy(fOutData, fInData, stInField.Ni * stInField.Nj * sizeof(float));
		}

		stOutField = stInField;
	}
	else if (fInDi > fOutDi && fInDj > fOutDj)
	{
		// ���߶ȴ���
		return proccess_di_dj_down(fInData, stInField, fInDi, fInDj, fOutDi, fOutDj, fOutData, stOutField, warningdata, fMissingVal);
	}
	else if (fInDi < fOutDi && fInDj < fOutDj)
	{
		// ���߶ȴ���
		return proccess_di_dj_up(fInData, stInField, fInDi, fInDj, fOutDi, fOutDj, fOutData, stOutField, fMissingVal);
	}
	else
	{
		return -2;
	}

	return 0;
}

/************************************************************************/
/* �ֱ��ʴ���-���߶�                                                    */
/************************************************************************/
int proccess_di_dj_down(float *fInData, nwfd_grib2_field &stInField, float fInDi, float fInDj, float fOutDi, float fOutDj,
	float * &fOutData, nwfd_grib2_field &stOutField, bool warningdata, float fMissingVal)
{
	if (fInData == NULL)
	{
		return -1;
	}

	// ������������֮ǰ��֤�����Բ���У��
	int n = (int)(fInDi / fOutDi + 0.1); // ���ȷ���
	int m = (int)(fInDj / fOutDj + 0.1); // γ�ȷ���

	// ������������
	int nOutNi = (stInField.Ni - 1) * n + 1;
	int nOutNj = (stInField.Nj - 1) * m + 1;

	// ���߶ȴ���ľ�γ��Χ��ԭ�ȵ�һ�£����Բ������¼���

	// ����������ݴ洢�ռ�
	fOutData = (float *)malloc(nOutNi * nOutNj * sizeof(float));
	if (fOutData == NULL)
	{
		// ����ռ�ʧ��
		return -2;
	}

	// ���߶ȴ���
	int i, j;     // Ԫ���ݵ������
	int x, y;     // ��ֵ��������
	int s, t;
	float fA, fB, fC, fD; // Ԫ���ݵ��ĸ����ֵ
	float fVal;           // ��ֵ������ֵ
	int pos;

	// ����һ�����ݣ�����ɨ��
	for (j = 0; j < stInField.Nj - 1; j++)  // ԭ���� γ�ȷ���
	{
		for (i = 0; i < stInField.Ni - 1; i++) // ԭ���� ���ȷ���
		{
			// ���ֵ
			fA = fInData[j * stInField.Ni + i];
			fB = fInData[j * stInField.Ni + i + 1];
			fC = fInData[(j + 1) * stInField.Ni + i];
			fD = fInData[(j + 1) * stInField.Ni + i + 1];

			// ��ֵ����
			for (s = 0; s <= n; s++) // ���ȷ���
			{
				for (t = 0; t <= m; t++) // γ�ȷ���
				{
					// ��ֵ��������
					x = i * n + s;
					y = j * m + t;
					pos = y * nOutNi + x;

					if (warningdata)
					{
						// Ԥ�����ݣ������в�ֵ����ֻ���ݾ���ȡ�����ֵ
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
						// ��ֵ����
						fVal = fA + (fC - fA) / m * t + (fB + (fD - fB) / m * t - fA - (fC - fA) / m * t) / n * s;

						// ��ֵ
						fOutData[pos] = fVal;

						// ȱ��ֵ�ж����½���ȱ��ֵ��ֵ
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

	// ������Ϣ��ֵ
	stOutField = stInField;
	stOutField.incrementi = fOutDi;  // �ֱ���
	stOutField.incrementj = fOutDj;
	stOutField.Ni = nOutNi;          // ����
	stOutField.Nj = nOutNj;

	return 0;
}

/************************************************************************/
/* �ֱ��ʴ���-���߶�                                                    */
/************************************************************************/
int proccess_di_dj_up(float *fInData, nwfd_grib2_field &stInField, float fInDi, float fInDj, float fOutDi, float fOutDj,
	float * &fOutData, nwfd_grib2_field &stOutField, float fMissingVal)
{
	if (fInData == NULL)
	{
		return -1;
	}

	// ������������֮ǰ��֤�����Բ���У��
	int n = (int)(fOutDi / fInDi + 0.1); // ���ȷ���
	int m = (int)(fOutDj / fInDj + 0.1); // γ�ȷ���

	// ������������
	int nOutNi = (stInField.Ni - 1) / n + 1;
	nOutNi = ((stInField.Ni - 1) % n == 0) ? nOutNi : nOutNi + 1;
	int nOutNj = (stInField.Nj - 1) / m + 1;
	nOutNj = ((stInField.Nj - 1) % m == 0) ? nOutNj : nOutNj + 1;

	// �����γ�ȼ���
	float fOutLon1 = stInField.lon1;
	float fOutLon2 = fOutLon1 + (nOutNi - 1) * fOutDi;
	float fOutLat1 = stInField.lat1;
	float fOutLat2 = fOutLat1 + (nOutNj - 1) * fOutDj;

	// ����������ݴ洢�ռ�
	fOutData = (float *)malloc(nOutNi * nOutNj * sizeof(float));
	if (fOutData == NULL)
	{
		// ����ռ�ʧ��
		return -2;
	}

	float fLat, fLon;
	int nOff = 0;
	int nPos;
	// ����һ�����ݣ�����ɨ��
	for (int j = 0; j < nOutNj; j++)
	{
		fLat = fOutLat1 + j * fOutDj;
		for (int i = 0; i < nOutNi; i++)
		{
			fLon = fOutLon1 + i * fOutDi;

			// ��ԭ���ݵ�λ��
			int y = (int)((fLon - stInField.lon1) / stInField.incrementi + 0.0001);
			int x = (int)((fLat - stInField.lat1) / stInField.incrementj + 0.0001);

			if (x < 0 || x >(stInField.Nj - 1) || y < 0 || y >(stInField.Ni - 1) || (fLon < stInField.lon1) || (fLat < stInField.lat1))
			{
				// û�и�����,��ȱ�������
				fOutData[nOff++] = fMissingVal;
			}
			else
			{
				// �ҵ�������
				nPos = x * stInField.Ni + y;
				fOutData[nOff++] = fInData[nPos];
			}

		}
	}

	// ������Ϣ��ֵ
	stOutField = stInField;
	stOutField.incrementi = fOutDi;  // �ֱ���
	stOutField.incrementj = fOutDj;
	stOutField.Ni = nOutNi;          // ����
	stOutField.Nj = nOutNj;
	stOutField.lon1 = fOutLon1;      // ��γ�Ȼ����΢����
	stOutField.lon2 = fOutLon2;
	stOutField.lat1 = fOutLat1;
	stOutField.lat2 = fOutLat2;

	return 0;
}
