#include "DataProcesser.h"
#include "stdio.h"
#include "string.h"
#include "malloc.h"
#include "stdlib.h"

DataProcesser::DataProcesser()
{
	// 处理类型，获取数据块
	SetType(BLOCK);
}


DataProcesser::~DataProcesser()
{

}

/************************************************************************/
/* 区域数据获取处理                                                     */
/* 返回值  int=0 表示处理结果正确                                       */
/************************************************************************/
int DataProcesser::DataProcess(float * fInData, nwfd_data_field * stInDataFieldInfo, float * &fOutData, nwfd_data_field * &stOutDataFieldInfo, int& nInOutCnt)
{
	// 1、输入参数判断
	if (fInData == NULL || stInDataFieldInfo == NULL || nInOutCnt <= 0)
	{
		// 参数错误
		return -1;
	}

	// 2、配置参数解析处理：格式 "lon1_lon2_lat1_lat2" ,todo 此代码待优化
	StringList lstParams;
	split(lstParams, m_szParams, "_");
	if (lstParams.size() != 4)
	{
		// 配置的参数错误
		return -2;
	}
	
	// 解析参数
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
	unsigned int nDataLenSum = 0; // 数据总长度
	unsigned int nOff = 0;        // 数据游标

	float * fInDataTemp = NULL;
	unsigned int nInDataLenSum = 0; // 输入数据总长度

	// 3. 数据加工处理
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

		// 计算每组数据的起始位置
		nInDataLenSum += nInNi * nInNj;
		fInDataTemp = fInData + nInDataLenSum - nInNi * nInNj;

		// 3.1 重新计算边界，将获取边界限定在格点场中  // todo  考虑float误差的问题
		// 起始经度
		if (fStartLon < fInLon1)
		{
			// 起始经度太小
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

		// 终止经度
		if (fEndLon > fInLon2)
		{
			// 终止经度太大
			fEndLon = fInLon2;
		}
		nCount = (int)((fEndLon - fInLon1) / fInDi + 0.1);
		fPostion = fInLon1 + fInDi * nCount;
		fEndLon = fPostion;

		// 起始纬度
		if (fStartLat < fInLat1)
		{
			// 起始纬度太小
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

		// 终止纬度
		if (fEndLat > fInLat2)
		{
			// 终止纬度太大
			fEndLat = fInLat2;
		}
		nCount = (int)((fEndLat - fInLat1) / fInDj + 0.1);
		fPostion = fInLat1 + fInDj * nCount;
		fEndLat = fPostion;

		// 3.2 截取条件数据
		int nLonCount = (int)((fEndLon - fStartLon) / fInDi + 0.1) + 1;  // 经度格点数
		int nLatCount = (int)((fEndLat - fStartLat) / fInDj + 0.1) + 1;  // 纬度格点数

		// 本次数据长度
		unsigned int nDatalen = nLonCount * nLatCount * sizeof(float);
		nDataLenSum += nDatalen;  // 总长度

		if ( n == 0)
		{
			// 分配初始空间
			fOutData= (float *)malloc(nDatalen);
			if (fOutData == NULL)
			{
				return -3;
			}

			// 分配所有数据信息空间
			stOutDataFieldInfo = (nwfd_data_field *)calloc(nInOutCnt, sizeof(nwfd_data_field));
			if (stOutDataFieldInfo == NULL)
			{
				free(fOutData);
				return -4;
			}
		}
		else
		{
			// 重新分配空间
			fOutData = (float *)realloc(fOutData, nDataLenSum);
			if (fOutData == NULL)
			{
				free(stOutDataFieldInfo);
				return -3;
			}
		}

		// 获取区域块数据
		int nStarti = (int)((fStartLon - fInLon1) / fInDi + 0.1) * sizeof(float);
		int nStartj = (int)((fStartLat - fInLat1) / fInDj + 0.1); 

		for (int j = 0; j < nLatCount; j++)
		{
			memcpy((char*)fOutData + nOff, (char *)fInDataTemp + nStarti + nStartj * nInNi * sizeof(float), nLonCount * sizeof(float));
			nOff += nLonCount * sizeof(float);
			nStartj++;
		}

		// 产品信息
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
/* 区域数据获取处理                                                     */
/* 返回值  int=0 表示处理结果正确                                       */
/************************************************************************/
int DataProcesser::DataProcess(char * strParams, float * fInData, nwfd_data_field * stInDataFieldInfo, float * &fOutData, nwfd_data_field * &stOutDataFieldInfo, int& nInOutCnt, float fMissingVal)
{
	// 1、输入参数判断
	if (strParams == NULL || fInData == NULL || stInDataFieldInfo == NULL || nInOutCnt <= 0)
	{
		// 参数错误
		return -1;
	}

	// 2、配置参数解析处理：格式 "lon1_lon2_lat1_lat2" ,todo 此代码待优化
	StringList lstParams;
	split(lstParams, strParams, ",");
	if (lstParams.size() != 4)
	{
		// 配置的参数错误
		return -2;
	}

	// 解析参数
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

	// 经纬度范围判断，不是正方向不处理
	if (lon2 < lon1 || lat2 < lat1)
	{
		return -5;
	}

	unsigned int nOff = 0;        // 数据游标

	float * fInDataTemp = NULL;
	unsigned int nInDataLenSum = 0; // 输入数据总长度
	unsigned int nOutDataLenSum = 0; // 输出数据总长度

	// 3. 数据加工处理
	for (int n = 0; n < nInOutCnt; n++)
	{
		// 2、 修订获取数据的经纬度，要正好在格点中
		float fStartLon = (int)(lon1 / stInDataFieldInfo[n].Di) * stInDataFieldInfo[n].Di;
		float fStartLat = (int)(lat1 / stInDataFieldInfo[n].Dj) * stInDataFieldInfo[n].Dj;

		float fEndLonTmp = (int)(lon2 / stInDataFieldInfo[n].Di) * stInDataFieldInfo[n].Di;
		float fEndLon = IS_EQUAL(fEndLonTmp, lon2) ? fEndLonTmp : (fEndLonTmp + stInDataFieldInfo[n].Di);
		float fEndLatTmp = (int)(lat2 / stInDataFieldInfo[n].Dj) * stInDataFieldInfo[n].Dj;
		float fEndLat = IS_EQUAL(fEndLatTmp, lat2) ? fEndLatTmp: (fEndLatTmp + stInDataFieldInfo[n].Dj);

		// 3、 获取数据的个数
		int nLonCount = (int)((fEndLon - fStartLon) / stInDataFieldInfo[n].Di + 1.1);  // 经度格点数
		int nLatCount = (int)((fEndLat - fStartLat) / stInDataFieldInfo[n].Dj + 1.1);  // 纬度格点数

		nInDataLenSum += stInDataFieldInfo[n].Ni *stInDataFieldInfo[n].Nj;
		// 4、 申请数据存储空间
		// 本次数据长度
		unsigned int nDatalen = nLonCount * nLatCount;
		nOutDataLenSum += nDatalen;  // 总长度
		if (n == 0)
		{
			// 分配初始空间
			fOutData = (float *)malloc(nDatalen * sizeof(float));
			if (fOutData == NULL)
			{
				return -3;
			}

			// 分配所有数据信息空间
			stOutDataFieldInfo = (nwfd_data_field *)calloc(nInOutCnt, sizeof(nwfd_data_field));
			if (stOutDataFieldInfo == NULL)
			{
				free(fOutData);
				return -4;
			}
		}
		else
		{
			// 重新分配空间
			fOutData = (float *)realloc(fOutData, nOutDataLenSum * sizeof(float));
			if (fOutData == NULL)
			{
				free(stOutDataFieldInfo);
				return -3;
			}
		}

		// 5、获取区域块数据
		float *fOutDataTemp = fOutData + nOutDataLenSum - nDatalen;
		float *fInDataTemp = fInData + nInDataLenSum - stInDataFieldInfo[n].Ni *stInDataFieldInfo[n].Nj;
		float fLon, fLat;
		int nPos = 0;
		int nOff = 0;
		for (int j = 0; j < nLatCount; j++)
		{
			// 此点的纬度
			fLat = fStartLat + j * stInDataFieldInfo[n].Dj;
			for (int i = 0; i < nLonCount; i++)
			{
				// 此点的经度
				fLon = fStartLon + i * stInDataFieldInfo[n].Di;

				// 在原数据的位置
				int y = (int)((fLon - stInDataFieldInfo[n].lon1) / stInDataFieldInfo[n].Di + 0.0001);
				int x = (int)((fLat - stInDataFieldInfo[n].lat1) / stInDataFieldInfo[n].Dj + 0.0001);

				if (x < 0 || x >(stInDataFieldInfo[n].Nj - 1) || y < 0 || y >(stInDataFieldInfo[n].Ni - 1) || 
					(fLon < stInDataFieldInfo[n].lon1) || (fLat < stInDataFieldInfo[n].lat1))
				{
					// 没有该数据,用缺测数据填补
					fOutDataTemp[nOff++] = fMissingVal;
				}
				else
				{
					// 找到该数据
					nPos = x * stInDataFieldInfo[n].Ni + y;
					fOutDataTemp[nOff++] = fInDataTemp[nPos];
				}
			}
		}

		// 产品信息
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
/* 数据空间释放                                                         */
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
/* 分割字符串                                                           */
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


// 从字符串中获取数据的方法
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
