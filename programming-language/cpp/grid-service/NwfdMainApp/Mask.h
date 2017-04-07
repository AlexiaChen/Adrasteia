/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* �ļ���: NetworkManager.h
* ��  ��: zhangl		�汾��1.0		��  �ڣ�2015/07/16
* ��  �����������ݴ���
* ��  ����
* �����б�:
* �޸���־��
*   No. ����		����		�޸�����
*
*************************************************************************/
#ifndef MASK_H
#define MASK_H

#include "ProductDef.h"
#include <QHash>
/************************************************************************/
/* Mask�ļ��б����                                                     */
/************************************************************************/
typedef QHash<QString, ST_MASK>  HASH_MASK_INFO;      // mask�ļ��б�
typedef QHash<QString, float*>   HASH_MASK_DATA;      // mask��פ�ڴ�����

/************************************************************************/
/* �� ���� CMask                                                        */
/* �� �ࣺ                                                              */
/* ˵ ���� ���ݺϲ��õ�Mask���ݹ���                                     */
/* �� ����                                                              */
/************************************************************************/
class CMask
{
private:
	CMask();

public:
	~CMask();
	static CMask & getClass();

public:
	bool AddMaskInfo(ST_MASK stMask);
	float * GetMaskData(QString strMaskFile);
	bool InitMaskData();

private:
	float * ReadMaskDataFromFile(ST_MASK stMask);
	void ClearMaskData();

private:
	HASH_MASK_INFO  m_hasMaskInfo;
	HASH_MASK_DATA  m_hasMaskData;
};

#endif //MASK_H

