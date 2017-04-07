/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: ZBase64.h
* 作  者: zhangl		版本：1.0		日  期：2015/04/
* 描  述：数据Base64格式的编解码
* 其  他：
* 功能列表:
*   1.
* 修改日志：
*   No. 日期		作者		     修改内容
*************************************************************************/
#ifndef ZBASE_64_H
#define ZBASE_64_H

#include <string>
using namespace std;

/************************************************************************/
/* 类 名： ZBase64                                                      */
/* 父 类： -                                                            */
/* 说 明： Base64格式编解码处理类                                       */
/* 描 述： 编解码使用静态方法处理                                       */
/************************************************************************/
class ZBase64
{
public:
	ZBase64();
	~ZBase64();

	/* 编码 */
	static string Encode(const unsigned char* Data, int DataByte);

	/* 解码 */
	static string Decode(const char* Data, int DataByte, int& OutByte);
};

#endif // ZBASE_64_H
