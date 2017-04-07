/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* �ļ���: DataProcesser.h
* ��  ��: zhangl		�汾��1.0		��  �ڣ�2015/05/25
* ��  ������ֵ���������
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
/* ˵ ����                                                              */
/* �� ����                                                              */
/************************************************************************/
class DataProcesser : public DataProInterface
{
public:
	DataProcesser();
	~DataProcesser();

public:
	// ���ݲ�ֵ������
	int DataProcess(float * fInData, nwfd_data_field * stInDataFieldInfo, float * &fOutData, nwfd_data_field * &stOutDataFieldInfo, int& nInOutCnt);

	int DataProcess(char * strParams, float * fInData, nwfd_data_field * stInDataFieldInfo, float * &fOutData, nwfd_data_field * &stOutDataFieldInfo, int& nInOutCnt, float fMissingVal);

	// ���ݿռ��ͷŽӿ�
	void FreeData(float * fData, nwfd_data_field * stDataFieldInfo);

private:
	/* �ֱ��������߶ȴ��� */
	int proccess_di_dj(float *fInData, nwfd_data_field &stInField, float fInDi, float fInDj, float fOutDi, float fOutDj,
		float * &fOutData, nwfd_data_field &stOutField, bool warningdata, float fMissingVal);
	int proccess_di_dj_down(float *fInData, nwfd_data_field &stInField, float fInDi, float fInDj, float fOutDi, float fOutDj,
		float * &fOutData, nwfd_data_field &stOutField, bool warningdata, float fMissingVal);
	int proccess_di_dj_up(float *fInData, nwfd_data_field &stInField, float fInDi, float fInDj, float fOutDi, float fOutDj,
		float * &fOutData, nwfd_data_field &stOutField, float fMissingVal);

private:
	// �ַ����ָ�
	void split(StringList& stringlist, const string& source, const string& separator);
	string getdata(const string &str, const string &separator, string::size_type &nOff);

};

#endif //DATAPROCESSER_H

