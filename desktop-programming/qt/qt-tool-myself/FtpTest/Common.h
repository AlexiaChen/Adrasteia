/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* �ļ���: Common.h
* ��  ��: zhangl  		�汾��1.0		��  �ڣ�2015/08/28
* ��  ��������������������ʹ�û���STL��
* ��  ����
* �����б�:
*
* �޸���־��
*   No.   ����		  ����		       �޸�����
* ------------------------------------------------------------------------
*************************************************************************/
#ifndef COMMON_H
#define COMMON_H

#include "memory.h"
#include <list>
#include <string>
using namespace std;

/************************************************************************/
/* 1. ��ʱ��������(������ʱ)                                            */
/************************************************************************/
#ifdef WIN32
#define delay(mse)   Sleep(mse)
#else
#include "unistd.h"
#define delay(mse)   usleep(mse*1000)
#endif

/************************************************************************/
/* 2. �ַ����б��ַ����ָ�                                            */
/************************************************************************/
// �ַ����б���
typedef list<string> StringList;

// �ַ����ָ�
void split(StringList& stringlist, const string& source, const string& separator);

// ���ַ�����˳���ȡ����
string getdata(const string &str, const string &separator, string::size_type &nOff);

/************************************************************************/
/* 6. ���紫������ת��                                                  */
/************************************************************************/
void RxF4(float &v, unsigned char *pBuf, int &nOff);
void TxF4(float v, unsigned char *pBuf, int &nOff);
void RxI2(unsigned short &v, unsigned char *pBuf, int &nOff);
void TxI2(short v, unsigned char *pBuf, int &nOff);
void RxI4(int &v, unsigned char *pBuf, int &nOff);
void TxI4(int v, unsigned char *pBuf, int &nOff);

/************************************************************************/
/* 0ֵ���㶨��,���ڸ������ıȽ�                                         */
/************************************************************************/
#define  ZERO               0.00001   //  0
#define  NA_ZERO           -0.00001   // -0
#define  IS_EQUAL_ZERO(x)  ((x) > NA_ZERO && (x) < ZERO)  // �жϸ������Ƿ�=0
#define  IS_EQUAL(x,y)     IS_EQUAL_ZERO(x-y)             // �ж������������Ƿ����

#endif

