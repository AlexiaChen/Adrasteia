/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* �ļ���: DataProcesser.h
* ��  ��: zhangl		�汾��1.0		��  �ڣ�2015/07/09
* ��  �����������ݳ���ȡ��
* ��  ����
* �����б�:
*   1.
* �޸���־��
*   No. ����		����		     �޸�����
*************************************************************************/
#ifndef DATAPROCESSER_H
#define DATAPROCESSER_H

#include "DataProInterface.h"
#include <string>
#include <list>
using namespace std;

/************************************************************************/
/* 0ֵ���㶨��,���ڸ������ıȽ�                                         */
/************************************************************************/
#define  ZERO               0.0001   //  0 ����4λС��Ϊ��׼
#define  NA_ZERO           -0.0001   // -0
#define  IS_EQUAL_ZERO(x)  ((x) > NA_ZERO && (x) < ZERO)  // �жϸ������Ƿ�=0
#define  IS_EQUAL(x,y)     IS_EQUAL_ZERO(x-y)                // �ж������������Ƿ����

// �ַ����б���
typedef list<string> StringList;

/************************************************************************/
/* �� ���� DataProcesser                                                */
/* �� �ࣺ DataProBase                                                  */
/* ˵ ���� �������ݳ���ȡ��                                             */
/* �� ����                                                              */
/************************************************************************/
class DataProcesser : public DataProInterface
{
public:
	DataProcesser();
	~DataProcesser();

public:

	// ���ݼӹ�����-������������ݣ�����������Ϣ�������� ���������������ݣ�����������Ϣ������
	int DataProcess(float * fInData, nwfd_data_field * stInDataFieldInfo, float * &fOutData, nwfd_data_field * &stOutDataFieldInfo, int& nInOutCnt);

	int DataProcess(char * strParams, float * fInData, nwfd_data_field * stInDataFieldInfo, float * &fOutData, nwfd_data_field * &stOutDataFieldInfo, int& nInOutCnt, float fMissingVal);

	// ���ݿռ��ͷŽӿ�
	void FreeData(float * fData, nwfd_data_field * stDataFieldInfo);

private:
	// �ַ����ָ�
	void split(StringList& stringlist, const string& source, const string& separator);
	string getdata(const string &str, const string &separator, string::size_type &nOff);
};

#endif //DATAPROCESSER_H

