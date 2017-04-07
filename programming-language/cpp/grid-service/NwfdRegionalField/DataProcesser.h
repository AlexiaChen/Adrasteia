/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: DataProcesser.h
* 作  者: zhangl		版本：1.0		日  期：2015/07/09
* 描  述：区域数据场获取类
* 其  他：
* 功能列表:
*   1.
* 修改日志：
*   No. 日期		作者		     修改内容
*************************************************************************/
#ifndef DATAPROCESSER_H
#define DATAPROCESSER_H

#include "DataProInterface.h"
#include <string>
#include <list>
using namespace std;

/************************************************************************/
/* 0值浮点定义,用于浮点数的比较                                         */
/************************************************************************/
#define  ZERO               0.0001   //  0 ，以4位小数为标准
#define  NA_ZERO           -0.0001   // -0
#define  IS_EQUAL_ZERO(x)  ((x) > NA_ZERO && (x) < ZERO)  // 判断浮点数是否=0
#define  IS_EQUAL(x,y)     IS_EQUAL_ZERO(x-y)                // 判断两个浮点数是否相等

// 字符串列表定义
typedef list<string> StringList;

/************************************************************************/
/* 类 名： DataProcesser                                                */
/* 父 类： DataProBase                                                  */
/* 说 明： 区域数据场获取类                                             */
/* 描 述：                                                              */
/************************************************************************/
class DataProcesser : public DataProInterface
{
public:
	DataProcesser();
	~DataProcesser();

public:

	// 数据加工处理-传入参数：数据，数据描述信息，个数； 传出：处理后的数据，数据描述信息，个数
	int DataProcess(float * fInData, nwfd_data_field * stInDataFieldInfo, float * &fOutData, nwfd_data_field * &stOutDataFieldInfo, int& nInOutCnt);

	int DataProcess(char * strParams, float * fInData, nwfd_data_field * stInDataFieldInfo, float * &fOutData, nwfd_data_field * &stOutDataFieldInfo, int& nInOutCnt, float fMissingVal);

	// 数据空间释放接口
	void FreeData(float * fData, nwfd_data_field * stDataFieldInfo);

private:
	// 字符串分割
	void split(StringList& stringlist, const string& source, const string& separator);
	string getdata(const string &str, const string &separator, string::size_type &nOff);
};

#endif //DATAPROCESSER_H

