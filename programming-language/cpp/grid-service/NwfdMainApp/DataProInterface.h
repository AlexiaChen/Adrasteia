/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: DataProInterface.h
* 作  者: zhangl  		版本：1.0		日  期：2015/05/25
* 描  述：格点化数据处理接口
* 其  他：
* 功能列表:
*   1.
* 修改日志：
*   No.   日期		  作者		       修改内容
* ------------------------------------------------------------------------
*   1  2015-07-10    zhangl       修改数据加工处理接口
*************************************************************************/
#ifndef DATAPROINTERFACE_H
#define DATAPROINTERFACE_H

/************************************************************************/
/* 定义处理类型                                                         */
/************************************************************************/
#define SCALE  1 // 升降尺度处理
#define BLOCK  2 // 获取数据块处理

/************************************************************************/
/* 数据信息定义                                                         */
/************************************************************************/
struct nwfd_data_field
{
	int category;     // 类别码
	int element;      // 元素
	int statistical;  // 统计码
	int status;       // 状态码
	float lon1;       // 起始经度
	float lon2;       // 终止经度
	float lat1;       // 起始纬度
	float lat2;       // 终止纬度
	float Di;         // 经度格距
	float Dj;         // 纬度格距
	int Ni;           // 经度格点数
	int Nj;           // 纬度格点数
	int year;         // 年
	int month;        // 月
	int day;          // 日
	int hour;         // 时
	int minute;       // 分
	int second;       // 秒
	int forecast;     // 预报时间
	int timerange;    // 时间间隔
};

/************************************************************************/
/* 类 名： DataProInterface                                             */
/* 父 类： -                                                            */
/* 说 明： 格点化数据处理接口类                                         */
/* 描 述：                                                              */
/************************************************************************/
class DataProInterface
{
public:
	DataProInterface();
	virtual ~DataProInterface();

public:
	// 初始化参数
	virtual int SetParams(char * strParams);
	virtual char* GetParams();

	virtual void SetType(int nType);
	virtual int GetType();

	// 数据加工处理-传入参数：数据，数据描述信息，个数； 传出：处理后的数据，数据描述信息，个数
	virtual int DataProcess(float * fInData, nwfd_data_field * stInDataFieldInfo, float * &fOutData, nwfd_data_field * &stOutDataFieldInfo, int& nInOutCnt) = 0;

	// 数据加工处理-传入参数：处理参数，数据，数据描述信息，个数； 传出：处理后的数据，数据描述信息，个数， 缺测值
	virtual int DataProcess(char * strParams, float * fInData, nwfd_data_field * stInDataFieldInfo, float * &fOutData, nwfd_data_field * &stOutDataFieldInfo, int& nInOutCnt, float fMissingVal) = 0;

	// 数据空间释放接口
	virtual void FreeData(float * fData, nwfd_data_field * stDataFieldInfo) = 0;

public:
	char m_szParams[256];
	int  m_nType;
};

#endif //DATAPROINTERFACE_H
