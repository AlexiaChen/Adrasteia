/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* �ļ���: NwfdInterpolation.h
* ��  ��: zhangl		�汾��1.0		��  �ڣ�2015/05/25
* ��  ������ֵ����dll �ⲿ���ö���
* ��  ����
* �����б�:
*   1.
* �޸���־��
*   No. ����		����		     �޸�����
*************************************************************************/
#ifndef NWFD_SCALE_LIB_H
#define NWFD_SCALE_LIB_H

#include "DataProcesser.h"

/************************************************************************/
/* �ṩͳһ�ⲿ���ýӿ�  GetDataProcesser������DataProBase���������ָ��*/
/************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

	DataProInterface * GetDataProcesser();

#ifdef __cplusplus
}
#endif

#endif // NWFD_SCALE_LIB_H
