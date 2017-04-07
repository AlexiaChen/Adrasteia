#include "log.h"
#include "HandleStationLib.h"
#include "Common.h"
#include "stdio.h"

/************************************************************************/
/* 构造函数                                                             */
/************************************************************************/
HandleStationLib::HandleStationLib()
{
	// 初始化
	hDll = NULL;
	m_funNwfd_openstationfile = NULL;

	//// 加载动态库，获取动态库接口方法
	//hDll = Lib_Open(Lib_Str(QString(NWFD_STATION_LIB)));
	//if (hDll)
	//{
	//	this->m_funNwfd_openstationfile = (lpNwfd_openstationfile)Lib_GetFun(hDll, "nwfd_openstationfile");
	//	this->m_funNwfd_freestationdata = (lpNwfd_freestationdata)Lib_GetFun(hDll, "nwfd_freestationdata");
	//	this->m_funNwfd_savedata2file   = (lpNwfd_savedata2file)Lib_GetFun(hDll,    "nwfd_savedata2file");
	//}
	//else
	//{
	//	LOG_(LOGID_ERROR, LOG_F("打开dll %s失败，错误编码=%d"), NWFD_STATION_LIB, Lib_GetErr());
	//	// todo
	//}
}

/************************************************************************/
/* 析构函数                                                             */
/************************************************************************/
HandleStationLib::~HandleStationLib()
{
	if (hDll)
	{
		Lib_Close(hDll);
	}
}
 
/************************************************************************/
/* 单例接口                                                             */
/************************************************************************/
HandleStationLib & HandleStationLib::getClass()
{
	static HandleStationLib objStationLib;
	return objStationLib;
}
//
///************************************************************************/
///* 读取站点文件，获取站点数据                                           */
///************************************************************************/
//int HandleStationLib::nwfd_openstationfile(const char * szType, char* file, st_station * &pstStation, int &Cnt)
//{
//	if (m_funNwfd_openstationfile == NULL)
//	{
//		return  -100;
//	}
//
//	return m_funNwfd_openstationfile(szType, file, pstStation, Cnt);
//}
//
///************************************************************************/
///* 释放站点数据申请的空间                                               */
///************************************************************************/
//void HandleStationLib::nwfd_freestationdata(st_station * &pstStation, int cnt)
//{
//	if (m_funNwfd_freestationdata == NULL)
//	{
//		return;
//	}
//
//	return m_funNwfd_freestationdata(pstStation, cnt);
//}
//
///************************************************************************/
///* 保存站点数据到文件中                                                 */
///************************************************************************/
//int HandleStationLib::nwfd_savedata2file(const char * szType, const char * szIDSymbol, const char * szDataType, char* file, st_station * pstStation, int cnt, int year, int month, int day, int hour, int minute, int second)
//{
//	if (m_funNwfd_savedata2file == NULL)
//	{
//		return  -100;
//	}
//
//	return m_funNwfd_savedata2file(szType, szIDSymbol, szDataType, file, pstStation, cnt, year, month, day, hour, minute, second);
//}
//



/************************************************************************/
/* 打开站点文件获取站点数据                                             */
/* @return:                                                             */
/*  -1: 打开文件失败                                                    */
/*  -2: 文件类型错误                                                    */
/*  -4: 创建空间失败                                                    */
/*   0: 处理正确                                                        */
/************************************************************************/
int HandleStationLib::nwfd_openstationfile(const char * szType, char* file, st_station * &pstStation, int &Cnt)
{
	// 1、打开文件
	FILE *fp;

#ifdef _WIN32
	fopen_s(&fp, file, "rb");
#else
	fp = fopen(file, "rb");
#endif

	if (fp == NULL) {
		return -1;
	}

	fseek(fp, 0, SEEK_END);
	long dwFileSize = ftell(fp);
	rewind(fp);

	char* pchBuf = (char*)malloc(dwFileSize + 1);
	memset(pchBuf, 0, dwFileSize);

	fread(pchBuf, 1, dwFileSize, fp);
	pchBuf[dwFileSize] = '\0';

	// 关闭文件
	fclose(fp);

	// 使用string类型处理数据
	string strFileBuf(pchBuf);
	free(pchBuf);
	string strData;
	string::size_type nOff = 0;  // 字符串游标位置
	string separator = " \r\n";

	// 获取数据
	strData = getdata(strFileBuf, separator, nOff);
	if (strData == "")
	{
		// 容错处理：剔除文件中第一个空格
		getdata(strFileBuf, separator, nOff);
	}

	// 判断文件类型
	if (strData != "ZCZC")
	{
		return -2;
	}

	getdata(strFileBuf, separator, nOff);
	getdata(strFileBuf, separator, nOff);
	getdata(strFileBuf, separator, nOff);
	
	strData = getdata(strFileBuf, separator, nOff);
	if (strData == "CCA")
	{
		getdata(strFileBuf, separator, nOff);//skip CCA field
	}
	
	getdata(strFileBuf, separator, nOff);
	getdata(strFileBuf, separator, nOff);
	if (strcmp(szType, "bigcity") == 0)  // 大城市
	{
		getdata(strFileBuf, separator, nOff);
		getdata(strFileBuf, separator, nOff);
		getdata(strFileBuf, separator, nOff);
		getdata(strFileBuf, separator, nOff);
		getdata(strFileBuf, separator, nOff);
		getdata(strFileBuf, separator, nOff);
		getdata(strFileBuf, separator, nOff);
	}

	// 站点个数
	strData = getdata(strFileBuf, separator, nOff);
	int nStationNum = atoi(strData.data());
	if (nStationNum <= 0 || nStationNum > 9999999)
	{
		return -3;
	}
	// 创建空间
	pstStation = (st_station *)calloc(nStationNum, sizeof(st_station));
	if (pstStation == NULL)
	{
		return -4;
	}

	// 读取数据
	for (int n = 0; n < nStationNum; n++)
	{
		// 站号
		strData = getdata(strFileBuf, separator, nOff);
		//pstStation[n].nStationNo = atoi(strData.data());
		memset(pstStation[n].stationNo, 0x00, sizeof(pstStation[n].stationNo));
		strcpy(pstStation[n].stationNo, strData.data());

		// 经度
		strData = getdata(strFileBuf, separator, nOff);
		pstStation[n].fLon = (float)atof(strData.data());

		// 纬度
		strData = getdata(strFileBuf, separator, nOff);
		pstStation[n].fLat = (float)atof(strData.data());

		// 高度
		strData = getdata(strFileBuf, separator, nOff);
		pstStation[n].fHeight = (float)atof(strData.data());

		// 行数
		strData = getdata(strFileBuf, separator, nOff);
		pstStation[n].nRow = atoi(strData.data());

		// 列数
		if (strcmp(szType, "bigcity") == 0)  // 大城市
		{
			pstStation[n].nCol = 6; // 大城市固定列
		}
		else  // 城镇
		{
			strData = getdata(strFileBuf, separator, nOff);
			pstStation[n].nCol = atoi(strData.data()); // 未包含首列是forecasttime
		}

		// 数据
		pstStation[n].fData = (float*)calloc(pstStation[n].nRow*(pstStation[n].nCol + 1), sizeof(float));
		if (pstStation[n].fData == NULL)
		{
			// 释放之前的空间
			nwfd_freestationdata(pstStation, n);
			return -4;
		}

		int nDataOff = 0;
		for (int i = 0; i < pstStation[n].nRow; i++)
		{
			for (int j = 0; j < pstStation[n].nCol + 1; j++)
			{
				strData = getdata(strFileBuf, separator, nOff);
				pstStation[n].fData[nDataOff++] = (float)atof(strData.data());
			}
		}

	}

	// 终结标志判断
	strData = getdata(strFileBuf, separator, nOff);
	if (strData != "NNNN")
	{
		// 终结符读取异常，可能中间格式不对
		// 释放之前的空间
		nwfd_freestationdata(pstStation, nStationNum);
		return -5;
	}

	// 返回个数
	Cnt = nStationNum;

	// 返回正确处理结果
	return 0;
}

/************************************************************************/
/* 释放申请的内存空间                                                   */
/************************************************************************/
void HandleStationLib::nwfd_freestationdata(st_station * &pstStation, int cnt)
{
	for (int n = 0; n < cnt; n++)
	{
		if (pstStation[n].fData)
		{
			free(pstStation[n].fData);
			pstStation[n].fData = NULL;
		}
	}

	free(pstStation);
	pstStation = NULL;
}

/************************************************************************/
/* 保存站点数据到文件中                                                 */
/************************************************************************/
int HandleStationLib::nwfd_savedata2file(const char * szType, const char * szIDSymbol, const char * szDataType, char* file, st_station * pstStation, int cnt, int year, int month, int day, int hour, int minute, int second)
{
	FILE *fp;

	// 打开文件
#ifdef _WIN32
	fopen_s(&fp, file, "wb");
#else
	fp = fopen(file, "wb");
#endif
	if (fp == NULL)
	{
		return -1;
	}

	// 写入数据
	// 头部信息
	fputs("ZCZC\n", fp);
	fputs("FSCI50 ", fp);
	fprintf(fp, "%s ", szIDSymbol);
	fprintf(fp, "%02d%02d%02d\n", day, hour, minute);
	fprintf(fp, "%04d%02d%02d%02d时站点预报\n", year, month, day, hour);

	fputs(szDataType, fp);  // 类型 SCMOC SPCC

	fprintf(fp, "%04d%02d%02d%02d\n", year, month, day, hour);

	if (strcmp(szType, "bigcity") == 0)  // 大城市
	{
		fprintf(fp, "006\n 621 622 623 624 625 626\n");
	}

	// 站个数
	fprintf(fp, "%d\n", cnt);
	// 每个站信息
	for (int n = 0; n < cnt; n++)
	{
		if (strcmp(szType, "bigcity") == 0)  // 大城市
		{
			fprintf(fp, "%-6s %8.2f %8.2f %8.2f %3d\n", pstStation[n].stationNo, pstStation[n].fLon, pstStation[n].fLat, pstStation[n].fHeight, pstStation[n].nRow);
		}
		else
		{
			// 城镇
			fprintf(fp, "%-6s %8.2f %8.2f %8.2f %3d %3d\n", pstStation[n].stationNo, pstStation[n].fLon, pstStation[n].fLat, pstStation[n].fHeight, pstStation[n].nRow, pstStation[n].nCol);
		}
		int nOff = 0;
		for (int i = 0; i < pstStation[n].nRow; i++)
		{
			for (int j = 0; j < pstStation[n].nCol + 1; j++)
			{
				if (j == 0)
				{
					fprintf(fp, "%3d", (int)pstStation[n].fData[nOff++]);
				}
				else
				{
					fprintf(fp, "%7.1f", pstStation[n].fData[nOff++]);
				}
			}
			fputs("\n", fp);
		}
	}

	// 结束符
	fputs("NNNN\n", fp);

	// 关闭文件
	fclose(fp);

	return 0;
}
