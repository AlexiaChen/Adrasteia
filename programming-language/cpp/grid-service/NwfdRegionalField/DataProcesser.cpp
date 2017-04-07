#include "DataProcesser.h"
#include "stdio.h"
#include "string.h"
#include "malloc.h"
#include "stdlib.h"

DataProcesser::DataProcesser()
{
	// �������ͣ���ȡ���ݿ�
	SetType(BLOCK);
}


DataProcesser::~DataProcesser()
{

}

/************************************************************************/
/* �������ݻ�ȡ����                                                     */
/* ����ֵ  int=0 ��ʾ��������ȷ                                       */
/************************************************************************/
int DataProcesser::DataProcess(float * fInData, nwfd_data_field * stInDataFieldInfo, float * &fOutData, nwfd_data_field * &stOutDataFieldInfo, int& nInOutCnt)
{
	// 1����������ж�
	if (fInData == NULL || stInDataFieldInfo == NULL || nInOutCnt <= 0)
	{
		// ��������
		return -1;
	}

	// 2�����ò�������������ʽ "lon1_lon2_lat1_lat2" ,todo �˴�����Ż�
	StringList lstParams;
	split(lstParams, m_szParams, "_");
	if (lstParams.size() != 4)
	{
		// ���õĲ�������
		return -2;
	}
	
	// ��������
	float lon1;
	float lon2;
	float lat1;
	float lat2;

	StringList::iterator iter;
	int nNum = 0;
	for (iter = lstParams.begin(),nNum = 0; iter != lstParams.end(); iter++, nNum++)
	{
		string strData = *iter;

		switch (nNum)
		{
		case 0:
			lon1 = (float)atof(strData.c_str()); break;
		case 1:
			lon2 = (float)atof(strData.c_str()); break;
		case 2:
			lat1 = (float)atof(strData.c_str()); break;
		case 3:
			lat2 = (float)atof(strData.c_str()); break;
		default:
			break;
		};
	}

	float fInLon1, fInLon2, fInDi, fInLat1, fInLat2, fInDj;
	int nInNi, nInNj;
	int   nCount;
	float fPostion;
	unsigned int nDataLenSum = 0; // �����ܳ���
	unsigned int nOff = 0;        // �����α�

	float * fInDataTemp = NULL;
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

		float fStartLon = lon1;
		float fEndLon = lon2;
		float fStartLat = lat1;
		float fEndLat = lat2;

		// ����ÿ�����ݵ���ʼλ��
		nInDataLenSum += nInNi * nInNj;
		fInDataTemp = fInData + nInDataLenSum - nInNi * nInNj;

		// 3.1 ���¼���߽磬����ȡ�߽��޶��ڸ�㳡��  // todo  ����float��������
		// ��ʼ����
		if (fStartLon < fInLon1)
		{
			// ��ʼ����̫С
			fStartLon = fInLon1;
		}
		nCount = (int)((fStartLon - fInLon1) / fInDi + 0.1);
		fPostion = fInLon1 + fInDi * nCount;
		if (IS_EQUAL(fStartLon, fPostion))
		{
			fStartLon = fPostion;
		}
		else
		{
			fStartLon = fPostion + fInDi;
		}

		// ��ֹ����
		if (fEndLon > fInLon2)
		{
			// ��ֹ����̫��
			fEndLon = fInLon2;
		}
		nCount = (int)((fEndLon - fInLon1) / fInDi + 0.1);
		fPostion = fInLon1 + fInDi * nCount;
		fEndLon = fPostion;

		// ��ʼγ��
		if (fStartLat < fInLat1)
		{
			// ��ʼγ��̫С
			fStartLat = fInLat1;
		}
		nCount = (int)((fStartLat - fInLat1) / fInDj + 0.1);
		fPostion = fInLat1 + fInDj * nCount;
		if (IS_EQUAL(fStartLat, fPostion))
		{
			fStartLat = fPostion;
		}
		else
		{
			fStartLat = fPostion + fInDj;
		}

		// ��ֹγ��
		if (fEndLat > fInLat2)
		{
			// ��ֹγ��̫��
			fEndLat = fInLat2;
		}
		nCount = (int)((fEndLat - fInLat1) / fInDj + 0.1);
		fPostion = fInLat1 + fInDj * nCount;
		fEndLat = fPostion;

		// 3.2 ��ȡ��������
		int nLonCount = (int)((fEndLon - fStartLon) / fInDi + 0.1) + 1;  // ���ȸ����
		int nLatCount = (int)((fEndLat - fStartLat) / fInDj + 0.1) + 1;  // γ�ȸ����

		// �������ݳ���
		unsigned int nDatalen = nLonCount * nLatCount * sizeof(float);
		nDataLenSum += nDatalen;  // �ܳ���

		if ( n == 0)
		{
			// �����ʼ�ռ�
			fOutData= (float *)malloc(nDatalen);
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
			fOutData = (float *)realloc(fOutData, nDataLenSum);
			if (fOutData == NULL)
			{
				free(stOutDataFieldInfo);
				return -3;
			}
		}

		// ��ȡ���������
		int nStarti = (int)((fStartLon - fInLon1) / fInDi + 0.1) * sizeof(float);
		int nStartj = (int)((fStartLat - fInLat1) / fInDj + 0.1); 

		for (int j = 0; j < nLatCount; j++)
		{
			memcpy((char*)fOutData + nOff, (char *)fInDataTemp + nStarti + nStartj * nInNi * sizeof(float), nLonCount * sizeof(float));
			nOff += nLonCount * sizeof(float);
			nStartj++;
		}

		// ��Ʒ��Ϣ
		stOutDataFieldInfo[n].category = stInDataFieldInfo[n].category;
		stOutDataFieldInfo[n].element = stInDataFieldInfo[n].element;
		stOutDataFieldInfo[n].statistical = stInDataFieldInfo[n].statistical;
		stOutDataFieldInfo[n].status = stInDataFieldInfo[n].status;
		stOutDataFieldInfo[n].lon1 = fStartLon;
		stOutDataFieldInfo[n].lon2 = fEndLon;
		stOutDataFieldInfo[n].lat1 = fStartLat;
		stOutDataFieldInfo[n].lat2 = fEndLat;
		stOutDataFieldInfo[n].Ni = nLonCount;
		stOutDataFieldInfo[n].Nj = nLatCount;
		stOutDataFieldInfo[n].Di = stInDataFieldInfo[n].Di;
		stOutDataFieldInfo[n].Dj = stInDataFieldInfo[n].Dj;
		stOutDataFieldInfo[n].year = stInDataFieldInfo[n].year;
		stOutDataFieldInfo[n].month = stInDataFieldInfo[n].month;
		stOutDataFieldInfo[n].day = stInDataFieldInfo[n].day;
		stOutDataFieldInfo[n].hour = stInDataFieldInfo[n].hour;
		stOutDataFieldInfo[n].minute = stInDataFieldInfo[n].minute;
		stOutDataFieldInfo[n].second = stInDataFieldInfo[n].second;
		stOutDataFieldInfo[n].forecast = stInDataFieldInfo[n].forecast;
		stOutDataFieldInfo[n].timerange = stInDataFieldInfo[n].timerange;

		/////////////////////////////////////////////////////
	}

	return 0;
}

/************************************************************************/
/* �������ݻ�ȡ����                                                     */
/* ����ֵ  int=0 ��ʾ��������ȷ                                       */
/************************************************************************/
int DataProcesser::DataProcess(char * strParams, float * fInData, nwfd_data_field * stInDataFieldInfo, float * &fOutData, nwfd_data_field * &stOutDataFieldInfo, int& nInOutCnt, float fMissingVal)
{
	// 1����������ж�
	if (strParams == NULL || fInData == NULL || stInDataFieldInfo == NULL || nInOutCnt <= 0)
	{
		// ��������
		return -1;
	}

	// 2�����ò�������������ʽ "lon1_lon2_lat1_lat2" ,todo �˴�����Ż�
	StringList lstParams;
	split(lstParams, strParams, ",");
	if (lstParams.size() != 4)
	{
		// ���õĲ�������
		return -2;
	}

	// ��������
	float lon1;
	float lon2;
	float lat1;
	float lat2;
	StringList::iterator iter;
	int nNum = 0;
	for (iter = lstParams.begin(), nNum = 0; iter != lstParams.end(); iter++, nNum++)
	{
		string strData = *iter;

		switch (nNum)
		{
		case 0:
			lon1 = (float)atof(strData.c_str()); break;
		case 1:
			lon2 = (float)atof(strData.c_str()); break;
		case 2:
			lat1 = (float)atof(strData.c_str()); break;
		case 3:
			lat2 = (float)atof(strData.c_str()); break;
		default:
			break;
		};
	}

	// ��γ�ȷ�Χ�жϣ����������򲻴���
	if (lon2 < lon1 || lat2 < lat1)
	{
		return -5;
	}

	unsigned int nOff = 0;        // �����α�

	float * fInDataTemp = NULL;
	unsigned int nInDataLenSum = 0; // ���������ܳ���
	unsigned int nOutDataLenSum = 0; // ��������ܳ���

	// 3. ���ݼӹ�����
	for (int n = 0; n < nInOutCnt; n++)
	{
		// 2�� �޶���ȡ���ݵľ�γ�ȣ�Ҫ�����ڸ����
		float fStartLon = (int)(lon1 / stInDataFieldInfo[n].Di) * stInDataFieldInfo[n].Di;
		float fStartLat = (int)(lat1 / stInDataFieldInfo[n].Dj) * stInDataFieldInfo[n].Dj;

		float fEndLonTmp = (int)(lon2 / stInDataFieldInfo[n].Di) * stInDataFieldInfo[n].Di;
		float fEndLon = IS_EQUAL(fEndLonTmp, lon2) ? fEndLonTmp : (fEndLonTmp + stInDataFieldInfo[n].Di);
		float fEndLatTmp = (int)(lat2 / stInDataFieldInfo[n].Dj) * stInDataFieldInfo[n].Dj;
		float fEndLat = IS_EQUAL(fEndLatTmp, lat2) ? fEndLatTmp: (fEndLatTmp + stInDataFieldInfo[n].Dj);

		// 3�� ��ȡ���ݵĸ���
		int nLonCount = (int)((fEndLon - fStartLon) / stInDataFieldInfo[n].Di + 1.1);  // ���ȸ����
		int nLatCount = (int)((fEndLat - fStartLat) / stInDataFieldInfo[n].Dj + 1.1);  // γ�ȸ����

		nInDataLenSum += stInDataFieldInfo[n].Ni *stInDataFieldInfo[n].Nj;
		// 4�� �������ݴ洢�ռ�
		// �������ݳ���
		unsigned int nDatalen = nLonCount * nLatCount;
		nOutDataLenSum += nDatalen;  // �ܳ���
		if (n == 0)
		{
			// �����ʼ�ռ�
			fOutData = (float *)malloc(nDatalen * sizeof(float));
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
			fOutData = (float *)realloc(fOutData, nOutDataLenSum * sizeof(float));
			if (fOutData == NULL)
			{
				free(stOutDataFieldInfo);
				return -3;
			}
		}

		// 5����ȡ���������
		float *fOutDataTemp = fOutData + nOutDataLenSum - nDatalen;
		float *fInDataTemp = fInData + nInDataLenSum - stInDataFieldInfo[n].Ni *stInDataFieldInfo[n].Nj;
		float fLon, fLat;
		int nPos = 0;
		int nOff = 0;
		for (int j = 0; j < nLatCount; j++)
		{
			// �˵��γ��
			fLat = fStartLat + j * stInDataFieldInfo[n].Dj;
			for (int i = 0; i < nLonCount; i++)
			{
				// �˵�ľ���
				fLon = fStartLon + i * stInDataFieldInfo[n].Di;

				// ��ԭ���ݵ�λ��
				int y = (int)((fLon - stInDataFieldInfo[n].lon1) / stInDataFieldInfo[n].Di + 0.0001);
				int x = (int)((fLat - stInDataFieldInfo[n].lat1) / stInDataFieldInfo[n].Dj + 0.0001);

				if (x < 0 || x >(stInDataFieldInfo[n].Nj - 1) || y < 0 || y >(stInDataFieldInfo[n].Ni - 1) || 
					(fLon < stInDataFieldInfo[n].lon1) || (fLat < stInDataFieldInfo[n].lat1))
				{
					// û�и�����,��ȱ�������
					fOutDataTemp[nOff++] = fMissingVal;
				}
				else
				{
					// �ҵ�������
					nPos = x * stInDataFieldInfo[n].Ni + y;
					fOutDataTemp[nOff++] = fInDataTemp[nPos];
				}
			}
		}

		// ��Ʒ��Ϣ
		stOutDataFieldInfo[n].category = stInDataFieldInfo[n].category;
		stOutDataFieldInfo[n].element = stInDataFieldInfo[n].element;
		stOutDataFieldInfo[n].statistical = stInDataFieldInfo[n].statistical;
		stOutDataFieldInfo[n].status = stInDataFieldInfo[n].status;
		stOutDataFieldInfo[n].lon1 = fStartLon;
		stOutDataFieldInfo[n].lon2 = fEndLon;
		stOutDataFieldInfo[n].lat1 = fStartLat;
		stOutDataFieldInfo[n].lat2 = fEndLat;
		stOutDataFieldInfo[n].Ni = nLonCount;
		stOutDataFieldInfo[n].Nj = nLatCount;
		stOutDataFieldInfo[n].Di = stInDataFieldInfo[n].Di;
		stOutDataFieldInfo[n].Dj = stInDataFieldInfo[n].Dj;
		stOutDataFieldInfo[n].year = stInDataFieldInfo[n].year;
		stOutDataFieldInfo[n].month = stInDataFieldInfo[n].month;
		stOutDataFieldInfo[n].day = stInDataFieldInfo[n].day;
		stOutDataFieldInfo[n].hour = stInDataFieldInfo[n].hour;
		stOutDataFieldInfo[n].minute = stInDataFieldInfo[n].minute;
		stOutDataFieldInfo[n].second = stInDataFieldInfo[n].second;
		stOutDataFieldInfo[n].forecast = stInDataFieldInfo[n].forecast;
		stOutDataFieldInfo[n].timerange = stInDataFieldInfo[n].timerange;

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
