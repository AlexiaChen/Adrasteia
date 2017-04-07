#include "DataProcesser.h"
#include "stdio.h"
#include "string.h"
#include "malloc.h"
#include <cstdlib>

DataProcesser::DataProcesser()
{
	// �������ͣ���ȡ���ݿ�
	SetType(SCALE);
}


DataProcesser::~DataProcesser()
{
}

/************************************************************************/
/* ���ݲ�ֵ������(ֻ������������ֵ)                                   */
/* ����ֵ  int=0 ��ʾ��������ȷ                                       */
/************************************************************************/
int DataProcesser::DataProcess(float * fInData, nwfd_data_field * stInDataFieldInfo, float * &fOutData, nwfd_data_field * &stOutDataFieldInfo, int& nInOutCnt)
//int DataProcesser::DataProcess(float * fInData, int nInNi, int nInNj, float * & fOutData, int &nOutNi, int &nOutNj, int& nInOutCnt)
{
	return 500;
}


int DataProcesser::DataProcess(char * strParams, float * fInData, nwfd_data_field * stInDataFieldInfo, float * &fOutData, nwfd_data_field * &stOutDataFieldInfo, int& nInOutCnt, float fMissingVal)
{
    fOutData = NULL;
    stOutDataFieldInfo = NULL;

	// 1����������ж�
	if (strParams == NULL || fInData == NULL || stInDataFieldInfo == NULL || nInOutCnt <= 0)
	{
		// ��������
		return -1;
	}


	// 2�����ò�������������ʽ "lon1_lon2_lat1_lat2" ,todo �˴�����Ż�
	StringList lstParams;
	split(lstParams, strParams, ",");
	if (lstParams.size() != 5)
	{
		// ���õĲ�������
		return -2;
	}

	// ��������
	float di;
	float dj;
	float outdi;
	float outdj;
	bool  bIsWarning;
	StringList::iterator iter;
	int nNum = 0;
	for (iter = lstParams.begin(), nNum = 0; iter != lstParams.end(); iter++, nNum++)
	{
		string strData = *iter;

		switch (nNum)
		{
		case 0:
			di = (float)std::atof(strData.c_str()); break;
		case 1:
			dj = (float)std::atof(strData.c_str()); break;
		case 2:
			outdi = (float)std::atof(strData.c_str()); break;
		case 3:
			outdj = (float)std::atof(strData.c_str()); break;
		case 4:
			bIsWarning = !(std::atoi(strData.c_str()) == 0); break;
		default:
			break;
		};
	}


	// ��������ֱ����Ƿ���������
	if (di > outdi)
	{
		if (!IS_EQUAL((int)(di / outdi + 0.0001) * outdi, di))
		{
			return -3;
		}
	}
	else
	{
		if (!IS_EQUAL((int)(outdi / di + 0.0001) * di, outdi))
		{
			return -3;
		}
	}

	if (dj > outdj)
	{
		if (!IS_EQUAL((int)(dj / outdj + 0.0001) * outdj, dj))
		{
			return -3;
		}
	}
	else
	{
		if (!IS_EQUAL((int)(outdj / dj + 0.0001) * dj, outdj))
		{
			return -3;
		}
	}


	float fInLon1, fInLon2, fInDi, fInLat1, fInLat2, fInDj;
	int nInNi, nInNj;
	unsigned int nDataLenSum = 0; // �����ܳ���
    unsigned int nOutDataLenSum = 0; // �����ܳ���
	unsigned int nOff = 0;        // �����α�

	float * fInDataTemp = NULL;
    float * fOutDataTemp = NULL;
	unsigned int nInDataLenSum = 0; // ���������ܳ���
	// 3. ���ݼӹ�����
	for (int n = 0; n < nInOutCnt; n++)
	{
		fInLon1 = stInDataFieldInfo[n].lon1;
		fInLon2 = stInDataFieldInfo[n].lon2;
		fInDi = stInDataFieldInfo[n].Di;
		nInNi = stInDataFieldInfo[n].Ni;
		fInLat1 = stInDataFieldInfo[n].lat1;
		fInLat2 = stInDataFieldInfo[n].lat2;
		fInDj = stInDataFieldInfo[n].Dj;
		nInNj = stInDataFieldInfo[n].Nj;

        // ����ÿ�����ݵ���ʼλ��
		nInDataLenSum += nInNi * nInNj;
		fInDataTemp = fInData + nInDataLenSum - nInNi * nInNj;
        
       
        if (!IS_EQUAL(fInDi, di) || !IS_EQUAL(fInDj, dj))
        {
            continue;
        }

        fOutDataTemp = NULL;
        nwfd_data_field tmpOutField;
        //Scale����
        int nRet = proccess_di_dj(fInDataTemp, stInDataFieldInfo[n], fInDi, fInDj, outdi, outdj, fOutDataTemp, tmpOutField, bIsWarning, fMissingVal);
        if (nRet != 0 || fOutDataTemp == NULL)
        {
            if (fOutDataTemp) free(fOutDataTemp);
            if(fOutData) free(fOutData);
            if(stOutDataFieldInfo) free(stOutDataFieldInfo);

            return nRet;
         }
        
        if (n == 0)
        {
            // �����ʼ�ռ�
            fOutData = (float *)malloc(tmpOutField.Ni*tmpOutField.Nj*sizeof(float));
            if (fOutData == NULL)
            {
                return -3;
            }

            // ��������������Ϣ�ռ�
            stOutDataFieldInfo = (nwfd_data_field *)calloc(nInOutCnt, sizeof(nwfd_data_field));
            if (stOutDataFieldInfo == NULL)
            {
                free(fOutData);
                return -4;
            }
        }
        else
        {
            // ���·���ռ�
            fOutData = (float *)realloc(fOutData, tmpOutField.Ni*tmpOutField.Nj*sizeof(float)*(n + 1));
            if (fOutData == NULL)
            {
                free(stOutDataFieldInfo);
                return -3;
            }
        }

        int outLen = tmpOutField.Ni*tmpOutField.Nj;
        nOutDataLenSum += outLen;
        memcpy(fOutData + nOutDataLenSum - outLen, fOutDataTemp, tmpOutField.Ni*tmpOutField.Nj*sizeof(float));

        // ��Ʒ��Ϣ
        stOutDataFieldInfo[n].category = tmpOutField.category;
        stOutDataFieldInfo[n].element = tmpOutField.element;
        stOutDataFieldInfo[n].statistical = tmpOutField.statistical;
        stOutDataFieldInfo[n].status = tmpOutField.status;
        stOutDataFieldInfo[n].lon1 = tmpOutField.lon1;
        stOutDataFieldInfo[n].lon2 = tmpOutField.lon2;
        stOutDataFieldInfo[n].lat1 = tmpOutField.lat1;
        stOutDataFieldInfo[n].lat2 = tmpOutField.lat2;
        stOutDataFieldInfo[n].Ni = tmpOutField.Ni;
        stOutDataFieldInfo[n].Nj = tmpOutField.Nj;
        stOutDataFieldInfo[n].Di = tmpOutField.Di;
        stOutDataFieldInfo[n].Dj = tmpOutField.Dj;
        stOutDataFieldInfo[n].year = tmpOutField.year;
        stOutDataFieldInfo[n].month = tmpOutField.month;
        stOutDataFieldInfo[n].day = tmpOutField.day;
        stOutDataFieldInfo[n].hour = tmpOutField.hour;
        stOutDataFieldInfo[n].minute = tmpOutField.minute;
        stOutDataFieldInfo[n].second = tmpOutField.second;
        stOutDataFieldInfo[n].forecast = tmpOutField.forecast;
        stOutDataFieldInfo[n].timerange = tmpOutField.timerange;

		/////////////////////////////////////////////////////
	}

	return 0;
}

/************************************************************************/
/* ���ݿռ��ͷ�                                                         */
/************************************************************************/
void DataProcesser::FreeData(float * fData, nwfd_data_field * stDataFieldInfo)
{
	if (fData)
	{
		free(fData);
		fData = NULL;
	}
	if (stDataFieldInfo)
	{
		free(stDataFieldInfo);
		stDataFieldInfo = NULL;
	}
}



/************************************************************************/
/* �ֱ��ʴ���                                                           */
/************************************************************************/
int DataProcesser::proccess_di_dj(float *fInData, nwfd_data_field &stInField, float fInDi, float fInDj, float fOutDi, float fOutDj,
	float * &fOutData, nwfd_data_field &stOutField, bool warningdata, float fMissingVal)
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
int DataProcesser::proccess_di_dj_down(float *fInData, nwfd_data_field &stInField, float fInDi, float fInDj, float fOutDi, float fOutDj,
	float * &fOutData, nwfd_data_field &stOutField, bool warningdata, float fMissingVal)
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
	stOutField.Di = fOutDi;  // �ֱ���
	stOutField.Dj = fOutDj;
	stOutField.Ni = nOutNi;          // ����
	stOutField.Nj = nOutNj;

	return 0;
}

/************************************************************************/
/* �ֱ��ʴ���-���߶�                                                    */
/************************************************************************/
int DataProcesser::proccess_di_dj_up(float *fInData, nwfd_data_field &stInField, float fInDi, float fInDj, float fOutDi, float fOutDj,
	float * &fOutData, nwfd_data_field &stOutField, float fMissingVal)
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
			int y = (int)((fLon - stInField.lon1) / stInField.Di + 0.0001);
			int x = (int)((fLat - stInField.lat1) / stInField.Dj + 0.0001);

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
	stOutField.Di = fOutDi;  // �ֱ���
	stOutField.Dj = fOutDj;
	stOutField.Ni = nOutNi;          // ����
	stOutField.Nj = nOutNj;
	stOutField.lon1 = fOutLon1;      // ��γ�Ȼ����΢����
	stOutField.lon2 = fOutLon2;
	stOutField.lat1 = fOutLat1;
	stOutField.lat2 = fOutLat2;

	return 0;
}


/************************************************************************/
/* �ָ��ַ���                                                           */
/************************************************************************/
void DataProcesser::split(StringList& stringlist, const string& source, const string& separator)
{
	int start = 0;
	int end = source.find_first_of(separator);

	while (end != string::npos)
	{
		stringlist.push_back(source.substr(start, end - start));
		start = end + 1;
		end = source.find(separator, start);
	}

	stringlist.push_back(source.substr(start, source.length() - start));
}


// ���ַ����л�ȡ���ݵķ���
string getdata(const string &str, const string &separator, string::size_type &nOff)
{
	string substring;
	string::size_type  index;

	index = str.find_first_of(separator, nOff);
	if (index != string::npos)
	{
		substring = str.substr(nOff, index - nOff);

		nOff = str.find_first_not_of(separator, index);
		//if (nOff == string::npos)
		//{
		//	return "";
		//}
	}
	else
	{
		substring = str.substr(nOff);
		//return "";
	}

	return substring;
}
