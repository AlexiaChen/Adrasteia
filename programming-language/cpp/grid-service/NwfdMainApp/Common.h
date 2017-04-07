/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: Common.h
* 作  者: zhangl  		版本：1.0		日  期：2015/08/28
* 描  述：公共处理方法函数（使用基本STL）
* 其  他：
* 功能列表:
*
* 修改日志：
*   No.   日期		  作者		       修改内容
* ------------------------------------------------------------------------
*************************************************************************/
#ifndef COMMON_H
#define COMMON_H

#include "memory.h"
#include <list>
#include <string>
using namespace std;

/************************************************************************/
/* 1. 延时函数定义(毫秒延时)                                            */
/************************************************************************/
#ifdef WIN32
#define delay(mse)   Sleep(mse)
#else
#include "unistd.h"
#define delay(mse)   usleep(mse*1000)
#endif

/************************************************************************/
/* 2. 字符串列表及字符串分割                                            */
/************************************************************************/
// 字符串列表定义
typedef list<string> StringList;

// 字符串分割
void split(StringList& stringlist, const string& source, const string& separator);

// 从字符串中顺序读取数据
string getdata(const string &str, const string &separator, string::size_type &nOff);

/************************************************************************/
/* 6. 网络传输类型转换                                                  */
/************************************************************************/
void RxF4(float &v, unsigned char *pBuf, int &nOff);
void TxF4(float v, unsigned char *pBuf, int &nOff);
void RxI2(unsigned short &v, unsigned char *pBuf, int &nOff);
void TxI2(short v, unsigned char *pBuf, int &nOff);
void RxI4(int &v, unsigned char *pBuf, int &nOff);
void TxI4(int v, unsigned char *pBuf, int &nOff);

/************************************************************************/
/* 0值浮点定义,用于浮点数的比较                                         */
/************************************************************************/
#define  ZERO               0.0001   //  0 ，以4位小数为标准
#define  NA_ZERO           -0.0001   // -0
#define  IS_EQUAL_ZERO(x)  ((x) > NA_ZERO && (x) < ZERO)  // 判断浮点数是否=0
#define  IS_EQUAL(x,y)     IS_EQUAL_ZERO(x-y)             // 判断两个浮点数是否相等

#endif

