/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: JSON.h
* 作  者: zhangl		版本：1.0		日  期：2015/05/18
* 描  述：JSON数据解析类
* 其  他：使用开源CJSON进行处理
* 功能列表:
*   1.
* 修改日志：
*   1  2015/08/28  zhangl    去除QT类型，使用STL基本类型
*************************************************************************/
#ifndef JSON_H
#define JSON_H

#include "ProductDef.h"
#include "Common.h"
#include <string>

using namespace std;

//#define CLIENT_USE  // 客户端使用此条件
#ifdef CLIENT_USE
#define JSTR(str) string(str)
#else
#define JSTR(str) QString::fromLocal8Bit(str);
#endif

/************************************************************************/
/* 类 名： JSON                                                         */
/* 父 类： -                                                            */
/* 说 明： JSON数据解析类                                               */
/* 描 述： 使用静态方法进行处理,单线程执行                              */
/************************************************************************/
class JSON
{
public:
	JSON(){}
	~JSON(){}

public:
	static char * CreateProductInfoJson(ST_PRODUCT stProduct);
	static bool ParserProductInfo(char * strProductInfo, ST_PRODUCT & stProduct);
};

#endif // JSON_H
