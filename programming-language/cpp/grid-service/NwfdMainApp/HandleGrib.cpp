#include "log.h"
#include "NwfdErrorCode.h"
#include "HandleGrib.h"
#include "Common.h"
#include "HandleCommon.h"
#include <QTextStream>

#pragma execution_character_set("utf-8")

HandleGrib::HandleGrib()
{

}

HandleGrib::~HandleGrib()
{
}

HandleGrib & HandleGrib::getClass()
{
	static HandleGrib objGribDir;
	return objGribDir;
}

/************************************************************************/
/* 获取Grib申请内存存储空间的估算大小                                   */
/************************************************************************/
unsigned int HandleGrib::GetGribMemSize(int nNi, int nNj, int nCnt)
{
	if (nNi < 1 || nNj < 1 || nCnt < 1)
	{
		return 0;
	}

	unsigned int nMemSize = 0;

	int nGridSum = nNi * nNj;

	// 内存大小，按照格点数据的内存长度进行 // 500是用于保存grib信息
	nMemSize = 500 + nGridSum * sizeof(float);

	return nMemSize * nCnt;
}

/************************************************************************/
/* 获取Grib数据并保存到文件中                                           */
/************************************************************************/
int HandleGrib::ProcessGribData(float *fData, int nCnt, ST_PRODUCT stProduct,float fDi, float fDj, int nNi, int nNj, 
	int nYear, int nMonth, int  nDay, int  nHour, int  nForecasttime, int nTimeRange, QString strGribFile)
{
	if (fData == NULL || nCnt < 1)
	{
		// 传人参数不正确
		return -1;
	}

	// 获取Grib2内存申请空间估算
	unsigned int nMemSize = GetGribMemSize(nNi, nNj, nCnt);
	if (nMemSize == 0)
	{
		// 估算内存空间失败
		return -2;
	}

	unsigned char * cgrib = NULL;
	try
	{
		// 申请内存空间
		cgrib  = (unsigned char*)malloc(nMemSize);
		if (cgrib == NULL)
		{
			// 申请内存空间失败
			return -3;
		}

		//  获取Grib2数据
		long lLen;
		lLen = HandleNwfdLib::getClass().nwfd_datset2grib(cgrib, stProduct.nCategory, stProduct.nElement[0], stProduct.nStatistical,
			stProduct.nStatus, nYear, nMonth, nDay, nHour, 0, 0, nForecasttime, nTimeRange, nCnt,
			stProduct.fLon1, stProduct.fLon2, stProduct.fLat1, stProduct.fLat2, fDi, fDj, nNi, nNj, fData);
		if (lLen <= 0)
		{
			// 获取Grib2数据失败
			free(cgrib);
			cgrib = NULL;

			return -4;
		}

		// 保存数据到Grib2文件（不检测目录，传人前检验）
		long lRet = HandleNwfdLib::getClass().nwfd_savetofile(cgrib, lLen, strGribFile.toLocal8Bit().data());
		if (lRet != 0)
		{
			// 保存数据到Grib2文件失败
			free(cgrib);

			return -5;
		}

		// 释放存储空间
		free(cgrib);
	}
	catch (...)
	{
		if (cgrib)
		{
			free(cgrib);
		}

		// 其他异常抛出
		return -6;
	}

	// 返回正确
	return 0;
}

/************************************************************************/
/* 获取文件数据                                                         */
/************************************************************************/
float * HandleGrib::GetNwfdData(QString strFile, float fOffSet, nwfd_grib2_field* &stDataField, int & nCnt)
{
	// 文件路径进行处理
	// windows下将路径中的 \ 统一替换为 / ; window/Linux都支持 “/”
	strFile.replace("\\", "/");

	float *fNwfdData = NULL;
	grdpts stGrid;

	// 判断文件数据格式
	QString strDataFormat = GetDataFormat(strFile);

	// 根据不同的文件数据类型进行不同的处理
	if (strDataFormat == FORMAT_MICAPS4)
	{
		fNwfdData =  HandleNwfdLib::getClass().nwfd_openm4file2(strFile.toLocal8Bit().data(), &stGrid, fOffSet);
		nCnt = 1;
		stDataField = (nwfd_grib2_field*)malloc(sizeof(nwfd_grib2_field));
		if (stDataField)
		{
			stDataField->Ni = stGrid.Ni;
			stDataField->Nj = stGrid.Nj;
			stDataField->lon1 = stGrid.lon1;
			stDataField->lon2 = stGrid.lon2;
			stDataField->lat1 = stGrid.lat1;
			stDataField->lat2 = stGrid.lat2;
			stDataField->incrementi = stGrid.nlon;
			stDataField->incrementj = stGrid.nlat;

			// 其他的先不管，遇到再说 todo
		}
	}
	else if (strDataFormat == FORMAT_MICAPS11)
	{
		fNwfdData =  HandleNwfdLib::getClass().nwfd_openm11file(strFile.toLocal8Bit().data(), &stGrid, fOffSet);
		nCnt = 2;
		stDataField = (nwfd_grib2_field*)malloc(sizeof(nwfd_grib2_field));
		if (stDataField)
		{
			stDataField->Ni = stGrid.Ni;
			stDataField->Nj = stGrid.Nj;
			stDataField->lon1 = stGrid.lon1;
			stDataField->lon2 = stGrid.lon2;
			stDataField->lat1 = stGrid.lat1;
			stDataField->lat2 = stGrid.lat2;
			stDataField->incrementi = stGrid.nlon;
			stDataField->incrementj = stGrid.nlat;
			// Micaps11的处理先简单处理 todo
		}
	}
	else if (strDataFormat == FORMAT_GRIB2)
	{
		fNwfdData = nwfd_opengrib2file(strFile, stDataField, nCnt);
	}
	else
	{
		// 类型不支持
	}

	return fNwfdData;
}

/************************************************************************/
/* 获取文件数据-根据时间，时次准确查找                                  */
/************************************************************************/
int HandleGrib::GetNwfdData(QString strFile, float fOffSet, int nYear, int nMonth, int nDay, int nHour, int nForecasttime, float* &fData, nwfd_grib2_field* &stDataField, int & nCnt)
{
	// 文件路径进行处理
	// windows下将路径中的 \ 统一替换为 / ; window/Linux都支持 “/”
	strFile.replace("\\", "/");

	int nErr = 0;
	fData = NULL;
	grdpts stGrid;

	// 验证文件是否存在
	if (!HandleCommon::Exists(strFile))
	{
		return FILE_ISNOT_EXISTS;
	}

	// 判断文件数据格式
	QString strDataFormat = GetDataFormat(strFile);

	// 根据不同的文件数据类型进行不同的处理
	if (strDataFormat == FORMAT_MICAPS4)
	{
		fData = HandleNwfdLib::getClass().nwfd_openm4file2(strFile.toLocal8Bit().data(), &stGrid, fOffSet);
		nCnt = 1;
		stDataField = (nwfd_grib2_field*)malloc(sizeof(nwfd_grib2_field));
		if (stDataField)
		{
			stDataField->Ni = stGrid.Ni;
			stDataField->Nj = stGrid.Nj;
			stDataField->lon1 = stGrid.lon1;
			stDataField->lon2 = stGrid.lon2;
			stDataField->lat1 = stGrid.lat1;
			stDataField->lat2 = stGrid.lat2;
			stDataField->incrementi = stGrid.nlon;
			stDataField->incrementj = stGrid.nlat;

			// 其他的先不管，遇到再说 todo
		}
	}
	else if (strDataFormat == FORMAT_MICAPS11)
	{
		fData = HandleNwfdLib::getClass().nwfd_openm11file(strFile.toLocal8Bit().data(), &stGrid, fOffSet);
		nCnt = 2;
		stDataField = (nwfd_grib2_field*)malloc(sizeof(nwfd_grib2_field) * 2);
		if (stDataField)
		{
			stDataField[1].Ni =  stDataField[0].Ni = stGrid.Ni;
			stDataField[1].Nj =  stDataField[0].Nj = stGrid.Nj;
			stDataField[1].lon1 = stDataField[0].lon1 = stGrid.lon1;
			stDataField[1].lon2 = stDataField[0].lon2 = stGrid.lon2;
			stDataField[1].lat1 = stDataField[0].lat1 = stGrid.lat1;
			stDataField[1].lat2 = stDataField[0].lat2 = stGrid.lat2;
			stDataField[1].incrementi = stDataField[0].incrementi = stGrid.nlon;
			stDataField[1].incrementj = stDataField[0].incrementj = stGrid.nlat;
			// micaps11 只针对风
			stDataField[1].category = stDataField[0].category = 2; 
			stDataField[0].element = 2;
			stDataField[1].element = 3;
			// Micaps11的处理先简单处理 todo
		}
		
	}
	else if (strDataFormat == FORMAT_GRIB2)
	{
		nErr = nwfd_opengrib2file(strFile,nYear, nMonth, nDay, nHour, nForecasttime,fData, stDataField, nCnt);
	}
	else
	{
		// 类型不支持
		nErr = FILE_FORMAT_UNKOWN;
	}

	return nErr;
}


/************************************************************************/
/* 获取文件数据的格式                                                   */
/* return:FORMAT_MICAPS4、FORMAT_MICAPS11、FORMAT_GRIB2、FORMAT_UNKNOWN */
/************************************************************************/
QString HandleGrib::GetDataFormat(QString strFile)
{
	QString  strDataFormat = FORMAT_UNKNOWN;

	// 打开文件获取文件头信息
	FILE *fp;
	fp = fopen(strFile.toLocal8Bit().data(), "rb");
	if (fp == NULL)
	{
		return strDataFormat;
	}

	char szBuf[100];
	fgets(szBuf, 100, fp);  //读取前20个字节
	fclose(fp);

	// 进行类型判断
	QString strBuf = QString::fromLocal8Bit(szBuf);
	// 进行信息修订-删除其中的回车，换行，空格
	strBuf.replace(" ", "");

	if (strBuf.mid(0, 4).toLower() == "grib")
	{
		strDataFormat = FORMAT_GRIB2;  // GRIB
	}
	else if (strBuf.mid(0, 7).toLower() == "diamond")
	{
		if (strBuf.mid(7, 1).toInt() == 4)
		{
			strDataFormat = FORMAT_MICAPS4;  // micaps4
		}
		else if (strBuf.mid(7, 2).toInt() == 11)
		{
			strDataFormat = FORMAT_MICAPS11;  // micaps11
		}
	}
	else
	{
		// 当前未知类型
		strDataFormat = FORMAT_UNKNOWN;
	}

	return strDataFormat;
}

/************************************************************************/
/* 获取Grib2文件数据                                                    */
/************************************************************************/
float * HandleGrib::nwfd_opengrib2file(QString strFile, nwfd_grib2_field* &stDataField, int & nCnt)
{
	nCnt = 0;

	// 获取文件内容
	unsigned char * cgrib = ReadGribFile(strFile);
	if (cgrib == NULL)
	{
		return NULL;
	}

	// 获取文件信息
	nwfd_grib2_info stNwfdInfo;
	int ret = HandleNwfdLib::getClass().nwfd_gribinfo(cgrib, &stNwfdInfo);
	if (ret < 0)
	{
		free(cgrib);
		return NULL;
	}

	// 循环每个field，读出所有的数据
	float * fNwfdData = NULL;
	unsigned int nDataLenSum = 0;
	int nDataLen;
	int nOff = 0;

	// 创建信息存储空间
	stDataField = (nwfd_grib2_field *)calloc(stNwfdInfo.numfields, sizeof(nwfd_grib2_field));
	if (stDataField == NULL)
	{
		return NULL;
	}

	// 获取的总个数
	if (stNwfdInfo.numfields > 24)
		nCnt = 24;
	else
		nCnt = stNwfdInfo.numfields;

	// 读取数据
	for (int i = 1; i <= nCnt; i++)
	{
		nwfd_grib2_field field;

		// 获取Grib数据
		g2float* fld = HandleNwfdLib::getClass().nwfd_gribfield(cgrib, i, &field);
		nDataLen = field.Ni * field.Nj * sizeof(float);

		nDataLenSum += nDataLen;
		if (i == 1)
		{
			// 创建存储空间
			fNwfdData = (float*)malloc(nDataLenSum);
		}
		else
		{
			// 重新分配存储空间
			fNwfdData = (float *)realloc(fNwfdData, nDataLenSum);
		}
		if (fNwfdData == NULL)
		{
			free(fld);
			free(cgrib);
			free(stDataField);
			return NULL;
		}

		// 数据拷贝
		memcpy((char *)fNwfdData + nDataLenSum - nDataLen, (char *)fld, nDataLen);
		// 信息赋值
		stDataField[i - 1] = field;

		free(fld);
	}

	free(cgrib);

	return fNwfdData;
}

/************************************************************************/
/* 获取Grib2文件数据                                                    */
/************************************************************************/
int HandleGrib::nwfd_opengrib2file(QString strFile, int nYear, int nMonth, int nDay, int nHour, int nForecasttime, float* &fData, nwfd_grib2_field* &stDataField, int & nCnt)
{
	nCnt = 0;
	fData = NULL;
	try {
		// 获取文件内容
		unsigned char * cgrib = ReadGribFile(strFile);
		if (cgrib == NULL)
		{
			// 获取文件内容失败
			return READ_FILE_FAILED;
		}

		// 获取文件信息
		nwfd_grib2_info stNwfdInfo;
		int ret = HandleNwfdLib::getClass().nwfd_gribinfo(cgrib, &stNwfdInfo);
		if (ret < 0)
		{
			// 获取文件信息失败
			free(cgrib);
			return GET_GRIBINFO_FAILED;
		}
	
		//if (stNwfdInfo.year != nYear || stNwfdInfo.month != nMonth || stNwfdInfo.day != nDay || stNwfdInfo.hour != nHour)
		//{
		//	 时间不匹配
		//	free(cgrib);
		//	return 1;
		//}

		// 循环每个field，读出符合时间条件的所有数据
		unsigned int nDataLenSum = 0;
		int nDataLen;
		int nOff = 0;

		// 读取数据
		for (int i = 1; i <= stNwfdInfo.numfields; i++)
		{
			nwfd_grib2_field field;

			// 获取Grib信息
			HandleNwfdLib::getClass().nwfd_gribfieldinfo(cgrib, i, &field);

			// 判断时效
			if (field.forecast != nForecasttime || field.year != nYear || field.month != nMonth || field.day != nDay || field.hour != nHour)
			{
				// 跳过
				continue;
			}

			// 获取Grib数据
			g2float* fld = HandleNwfdLib::getClass().nwfd_gribfield(cgrib, i, &field);
			if (fld == NULL)
			{
				// 获取数据失败
				continue;
			}

			// 数据个数
			nCnt++;
			// 数据长度
			nDataLen = field.Ni * field.Nj;
			// 返回数据总长度
			nDataLenSum += nDataLen;

			if (nCnt == 1)
			{
				// 创建存储空间
				fData = (float*)malloc(nDataLenSum * sizeof(float));
				stDataField = (nwfd_grib2_field *)malloc(sizeof(nwfd_grib2_field));
			}
			else
			{
				// 重新分配存储空间
				fData = (float *)realloc(fData, nDataLenSum * sizeof(float));
				stDataField = (nwfd_grib2_field *)realloc(stDataField, nCnt * sizeof(nwfd_grib2_field));
			}

			if (fData == NULL || stDataField == NULL)
			{
				HandleNwfdLib::getClass().nwfd_freefld(fld);
				free(cgrib);
				if (fData) free(fData);
				if (stDataField) free(stDataField);
			
				// 申请空间失败
				return ERR_MEM_MALLOC;
			}

			// 数据拷贝
			memcpy(fData + nDataLenSum - nDataLen, fld, nDataLen * sizeof(float));

			// 信息赋值
			stDataField[nCnt - 1] = field;

			HandleNwfdLib::getClass().nwfd_freefld(fld);
		}

		free(cgrib);

	}
	catch (...)
	{
		return -1000;
	}

	return 0;
}

/************************************************************************/
/* 读取Grib文件,获取全部文件内容                                        */
/************************************************************************/
unsigned char * HandleGrib::ReadGribFile(QString strFile)
{
	FILE *fp;

	fp = fopen(strFile.toLocal8Bit().data(), "rb");
	if (fp == NULL)
	{
		return NULL;
	}

	fseek(fp, 0, SEEK_END);

	long dwFileSize = ftell(fp);

	rewind(fp);

	unsigned char * pchBuf = (unsigned char*)malloc(dwFileSize + 1);
	if (pchBuf == NULL)
	{
		fclose(fp);
		return NULL;
	}
	memset(pchBuf, 0, dwFileSize);

	fread(pchBuf, 1, dwFileSize, fp);
	pchBuf[dwFileSize] = '\0';

	fclose(fp);

	return pchBuf;
}

unsigned char * HandleGrib::ReadGribFile(QString strFile, unsigned int &size)
{
	FILE *fp;

	fp = fopen(strFile.toLocal8Bit().data(), "rb");
	if (fp == NULL)
	{
		return NULL;
	}

	fseek(fp, 0, SEEK_END);

	long dwFileSize = ftell(fp);

	rewind(fp);

	unsigned char * pchBuf = (unsigned char*)malloc(dwFileSize + 1);
	if (pchBuf == NULL)
	{
		fclose(fp);
		return NULL;
	}
	memset(pchBuf, 0, dwFileSize);

	fread(pchBuf, 1, dwFileSize, fp);
	pchBuf[dwFileSize] = '\0';

	// 返回长度：
	size = dwFileSize;

	fclose(fp);

	return pchBuf;
}

/************************************************************************/
/* 根据偏移量重新计算数据                                               */
/************************************************************************/
bool HandleGrib::CacalNwfdData(float* &fData, int nNi, int nNj, int nCnt, float fOffSet)
{
	if (fData == NULL)
	{
		return false;
	}

	// 偏移量为0，不需要重新计算
	if (IS_EQUAL_ZERO(fOffSet))
	{
		return true;
	}

	for (int i = 0; i < nNi * nNj * nCnt; i ++)
	{
		// 剔除缺测值 9999
		if (fData[i] > 9990)
		{
			continue;
		}

		fData[i] += fOffSet;
	}

	return true;
}

/************************************************************************/
/* 保存数据到Grib文件中                                                 */
/************************************************************************/
int HandleGrib::SaveData2GribFile(float *fData, int nCnt, int nCategory, int nElement, int nStatistical, int nStatus,
	float fLon1, float fLon2, float fLat1, float fLat2, float fDi, float fDj,
	int nYear, int nMonth, int  nDay, int  nHour, int  nForecasttime, int nTimeRange, int nClientID, QString strKey, QString strType, QString strPath, QString strFileName)
{
	// 1、替换目录中的可变字符
	QString strSavePath = HandleCommon::GetFolderPath(strPath, nYear, nMonth, nDay, nClientID, strKey, strType);
	QString strFile = HandleCommon::GetFileName(strFileName, nYear, nMonth, nDay, nHour, nForecasttime, nTimeRange, nClientID, strType);
	
	return SaveData2GribFile(fData, nCnt, nCategory, nElement, nStatistical, nStatus, fLon1, fLon2,  fLat1,  fLat2,  fDi, fDj,
		nYear, nMonth, nDay, nHour, nForecasttime, nTimeRange, strSavePath, strFile);
}

/************************************************************************/
/* 保存数据到Grib文件中                                                 */
/************************************************************************/
int HandleGrib::SaveData2GribFile(float *fData, int nCnt, int nCategory, int nElement, int nStatistical, int nStatus,
	float fLon1, float fLon2, float fLat1, float fLat2, float fDi, float fDj, 
	int nYear, int nMonth, int  nDay, int  nHour, int  nForecastTime, int nTimeRange, QString strPath, QString strFileName)
{
	// 1、替换目录中的可变字符
	QString strSavePath = HandleCommon::GetFolderPath(strPath,nYear, nMonth, nDay);

	// 2、创建存储目录
	if (!HandleCommon::HandleGribFolder(strSavePath))
	{
		// 创建失败
		return -10;
	}

	// 文件名
	QString strGrib2FileName = HandleCommon::GetFileName(strFileName, nYear, nMonth, nDay, nHour, nForecastTime, nTimeRange);
	QString	strGrib2File = QString("%1/%2").arg(strSavePath).arg(strGrib2FileName);
	QString strGrib2File_tmp = QString("%1.tmp").arg(strGrib2File);

	// 保存数据到grib2文件中
	int nRet = SaveData2GribFile(fData, nCnt, nCategory,nElement,nStatistical, nStatus,
		fLon1, fLon2, fLat1, fLat2, fDi, fDj, nYear, nMonth, nDay, nHour, nForecastTime, nTimeRange, strGrib2File_tmp);
	if (nRet != 0)
	{
		return nRet;
	}

	// 文件重命名
	if (!HandleCommon::Rename(strGrib2File_tmp, strGrib2File))
	{
		return -11;
	}

	return 0;
}

/************************************************************************/
/* 保存数据到Grib文件中                                                 */
/************************************************************************/
int HandleGrib::SaveData2GribFile(float *fData, int nCnt, int nCategory, int nElement, int nStatistical, int nStatus,
	float fLon1, float fLon2, float fLat1, float fLat2, float fDi, float fDj,
	int nYear, int nMonth, int  nDay, int  nHour, int  nForecasttime, int nTimeRange, QString strGribFile)
{
	// 参数判断
	if (fData == NULL || nCnt < 1)
	{
		// 传人参数不正确
		return -1;
	}

	// 格点数计算
	int nNi = (int)((fLon2 - fLon1) / fDi + 0.1) + 1;
	int nNj = (int)((fLat2 - fLat1) / fDj + 0.1) + 1;

	// 获取Grib2内存申请空间估算
	unsigned int nMemSize = GetGribMemSize(nNi, nNj, nCnt);
	if (nMemSize == 0)
	{
		// 估算内存空间失败
		return -2;
	}

	unsigned char * cgrib = NULL;
	try
	{
		// 申请内存空间
		cgrib = (unsigned char*)malloc(nMemSize);
		if (cgrib == NULL)
		{
			// 申请内存空间失败
			return -3;
		}

		//  获取Grib2数据
		long lLen;
		lLen = HandleNwfdLib::getClass().nwfd_datset2grib(cgrib, nCategory, nElement, nStatistical,
			nStatus, nYear, nMonth, nDay, nHour, 0, 0, nForecasttime, nTimeRange, nCnt,
			fLon1, fLon2, fLat1, fLat2, fDi, fDj, nNi, nNj, fData);
		if (lLen <= 0)
		{
			// 获取Grib2数据失败
			free(cgrib);
			cgrib = NULL;

			return -4;
		}

		// 保存数据到Grib2文件（不检测目录，传人前检验）
		long lRet = HandleNwfdLib::getClass().nwfd_savetofile(cgrib, lLen, strGribFile.toLocal8Bit().data());
		if (lRet != 0)
		{
			// 保存数据到Grib2文件失败
			free(cgrib);

			return -5;
		}

		// 释放存储空间
		free(cgrib);
	}
	catch (...)
	{
		if (cgrib)
		{
			free(cgrib);
		}

		// 其他异常抛出
		return -6;
	}

	// 返回正确
	return 0;
}

/************************************************************************/
/* 保存数据到Micaps4文件中                                              */
/************************************************************************/
int HandleGrib::SaveData2MicapsFile(float * fData, int nCnt,
	float fLon1, float fLon2, float fLat1, float fLat2, float fDi, float fDj, int nNi, int nNj,
	int nYear, int nMonth, int  nDay, int  nHour, int  nForecasttime,
	QString strName, QString strLineVal, float fOffSet, float fMissingVal, QString strFile)
{
	// 参数判断
	if (nCnt != 1 && nCnt != 2)
	{
		return -1;
	}

	try
	{
		QFile file;
		file.setFileName(strFile);
		if (file.exists())
		{
			// 删除之前的文件
			file.remove();
		}

		if (!file.open(QIODevice::ReadWrite | QIODevice::Text))
		{
			// 打开文件失败
			return -1;
		}
		QTextStream textStream(&file);
		textStream.setCodec("GBK");

		// 写入数据
	    // micaps类型
		int nMicapsType = (nCnt == 1) ? 4 : 11;

		// head1
		QString m_strHead1 = QString("").sprintf("diamond %d %04d%02d%02d%02d_%03d时效%s预报", nMicapsType, nYear, nMonth, nDay, nHour, nForecasttime, strName.toLocal8Bit().data());
		textStream << m_strHead1 << "\n";

		// head2
		QString m_strHead2 = QString("").sprintf("%02d %02d %02d %02d %03d 999 %0.2f %0.2f %0.2f %0.2f %0.2f %0.2f %d %d",
			nYear, nMonth, nDay, nHour, nForecasttime, fDi, fDj, fLon1, fLon2, fLat1, fLat2, nNi, nNj);
		if (nMicapsType == 4)
		{
			// 添加等值线参数
			if (strLineVal.isEmpty())
			{
				m_strHead2 = m_strHead2 + " 4 20 50 2 0.00";
			}
			else
			{
				m_strHead2 = m_strHead2 + " " + strLineVal;
			}
		}

		textStream << m_strHead2 << "\n";

		// data
		int nCount = 0;
		QString strData = "";
		float  fVal;
		float *fDataTmp = NULL;
		for (int n = 0; n < nCnt; n++)
		{
			// 确定数据的起始位置
			fDataTmp = fData + n * nNi* nNj;
			nCount = 0;

			for (int i = 0; i < nNj; i++)  // 行-纬度
			{
				for (int j = 0; j < nNi; j++)  // 列-经度
				{
					fVal = fDataTmp[i * nNi + j];
					if (!IS_EQUAL(fVal, fMissingVal))  // 根据传入参数确定缺测值 // 9999 为缺测值
					{
						fVal += fOffSet;
					}

					// 拼接字符串
					strData = strData + "  " + QString("").sprintf("%6.2f", fVal);

					// 每10个数据写入一行
					nCount++;
					if (nCount % 10 == 0)
					{
						textStream << strData << "\n";
						strData = "";
						// 刷新缓存
						// file.flush();
					}
				}
			}
			if (strData != "")
			{
				textStream << strData << "\n";
				strData = "";
			}

			// 写完一组数据换行
			// textStream << "\n";
		}

		// 刷新缓存
		file.flush();
		// 关闭文件
		file.close();
	}
	catch (...)
	{
		// 异常错误
		return -2;
	}

	return 0;
}

/************************************************************************/
/* 保存数据到Grib2文件中公共方法                                        */
/************************************************************************/
int HandleGrib::SaveData2File(float *fData, int nDataCnt,  /* 数据 */
	float fLon1, float fLon2, float fLat1, float fLat2, float fDi, float fDj, int nNi, int nNj, /* 经纬度范围和个数 */
	int nYear, int nMonth, int nDay, int nHour, int nForecastTime, int nTimeRange, /* 时间 */
	int nCategory, int nElement[2], int nStatistical, int nStatus, int nCnt, /* grib信息 */
	QString strName, QString strLineVal, float fOffSet, float fMissingVal,  /* micpas 存储信息 */
	QString strPath, QString strFileName, QString strFileFormat, bool bIsMergeFile/* 文件存储信息 */
	)
{
	// nDataCnt: 为总传入数据的个数，一个完整的数据为1个
	// nCnt:为单个数据的个数，micaps4为1，micaps11为2

	// 1、参数检查
	if (fData == NULL || nCnt < 1)
	{
		// 传人参数不正确
		return -1;
	}
	if (strFileFormat == "micaps" && nDataCnt != 1)
	{
		// 传输参数与实际情况不匹配
		return -1;
	}

	// 2、创建存储目录
	if (!HandleCommon::CreatePath(strPath))
	{
		// 创建失败
		return -2;
	}

	// 文件名
	QString	strFile = QString("%1/%2").arg(strPath).arg(strFileName);
	QString strFile_tmp = QString("%1.tmp").arg(strFile);

	// 存储处理
	int nErr = 0;
	if (strFileFormat == "grib")
	{
		// grib文件
		nErr = SaveData2GribFile(fData, nDataCnt, fLon1, fLon2, fLat1, fLat2, fDi, fDj, nNi, nNj,
			nYear, nMonth, nDay, nHour, nForecastTime, nTimeRange,
			nCategory, nElement, nStatistical, nStatus, nCnt, strFile_tmp, bIsMergeFile);

	}
	else if (strFileFormat == "micaps")
	{
		// micaps4 | micaps11
		nErr = SaveData2MicapsFile(fData, nCnt, fLon1, fLon2, fLat1, fLat2, fDi, fDj, nNi, nNj,
			nYear, nMonth, nDay, nHour, nForecastTime, strName, strLineVal, fOffSet, fMissingVal, strFile_tmp);
	}
	else
	{
		// 保存格式不对
		return -4;
	}

	// 文件保存失败
	if (nErr != 0)
	{
		return nErr;
	}

	// 数据保存成功，文件重命名
	if (!HandleCommon::Rename(strFile_tmp, strFile))
	{
		return -5;
	}

	return 0;
}

/************************************************************************/
/* 保存数据到Grib2文件中公共方法                                        */
/************************************************************************/
int HandleGrib::SaveData2GribFile(float *fData, int nDataCnt,  /* 数据 */
	float fLon1, float fLon2, float fLat1, float fLat2, float fDi, float fDj, int nNi, int nNj, /* 经纬度范围和个数 */
	int nYear, int nMonth, int nDay, int nHour, int nForecastTime, int nTimeRange, /* 时间 */
	int nCategory, int nElement[2], int nStatistical, int nStatus, int nCnt, /* grib信息 */
	QString strFile, bool bIsMergeFile/* 文件存储信息 */)
{
	// 传入数据的nCnt表示数据组个数，每组数据的个数为 stProduct.nNi*stProduct.nNj*stProduct.nCnt
	// stProduct.nCnt的风产品为2，其他为1
	// 如果是数据合并的，必须是等时间间隔的数据
	// 如果是单个数据生成时，预报时效使用的Forecast为nEndForecast

	if (fData == NULL || nDataCnt < 1)
	{
		// 传人参数不正确
		return -1;
	}

	// 获取Grib2内存申请空间估算
	unsigned int nMemSize = HandleGrib::getClass().GetGribMemSize(nNi, nNj, nCnt * nDataCnt);
	if (nMemSize == 0)
	{
		// 估算内存空间失败
		return -2;
	}

	unsigned char * cgrib = NULL;
	try
	{
		// 申请内存空间
		cgrib = (unsigned char*)malloc(nMemSize);
		if (cgrib == NULL)
		{
			// 申请内存空间失败
			return -3;
		}

		int forecasttime; // 预报时效
		int nOff = 0;
		int nDataLen = nNi * nNj;

		// 开始创建Grib2数据
		long lLen = HandleNwfdLib::getClass().nwfd_create(cgrib, nYear, nMonth, nDay, nHour, 0, 0, nStatus);

		lLen = HandleNwfdLib::getClass().nwfd_addgrid(cgrib, fLon1, fLon2, fLat1, fLat2, fDi, fDj, nNi, nNj);

		for (int i = 0; i < nDataCnt; i++)
		{
			if (bIsMergeFile)
			{
				// 合并时重新计算预报时效
				forecasttime = nForecastTime + i * nTimeRange;
			}
			else
			{
				forecasttime = nForecastTime;
			}

			// 控制每个数据
			for (int j = 0; j < nCnt; j++)
			{
				// 使用simple压码
				lLen = HandleNwfdLib::getClass().nwfd_addfield_simpled48(cgrib, nCategory, nElement[j], nStatistical,
					nYear, nMonth, nDay, nHour, 0, 0, forecasttime, nTimeRange, fData + nOff, nDataLen);

				nOff += nDataLen;
			}
		}

		lLen = HandleNwfdLib::getClass().nwfd_end(cgrib);

		if (lLen <= 0)
		{
			// 获取Grib2数据失败
			free(cgrib);
			cgrib = NULL;

			return -4;
		}

		// 保存数据到Grib2文件（不检测目录，传人前检验）
		long lRet = HandleNwfdLib::getClass().nwfd_savetofile(cgrib, lLen, strFile.toLocal8Bit().data());
		if (lRet != 0)
		{
			// 保存数据到Grib2文件失败
			free(cgrib);

			return -5;
		}

		// 释放存储空间
		free(cgrib);
	}
	catch (...)
	{
		if (cgrib)
		{
			free(cgrib);
		}

		// 其他异常抛出
		return -6;
	}

	// 返回正确
	return 0;
}
