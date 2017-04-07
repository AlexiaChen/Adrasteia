/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* �ļ���: DataManagerDef.h
* ��  ��: zhangl		�汾��1.0		��  �ڣ�2015/12/22
* ��  �������ݹ���궨��
* ��  ����
* �����б�:
* �޸���־��
*   No. ����		����		�޸�����
*
*************************************************************************/
#ifndef DATA_MANAGER_DEF_H
#define DATA_MANAGER_DEF_H


#include <QHash>
#include <QList>

// ��ֵ���վ�����ݴ洢
typedef QHash<QString, float> HASH_STATION_TIME;              // Key: Time
typedef QHash<QString, HASH_STATION_TIME*>  HASH_STATION_NO;  // Key: StationNo
typedef QHash<QString, HASH_STATION_NO*>  HASH_STATION_DATA;  // Key: Type_Range

// վ�����������Ʒƥ��
typedef QHash<QString, QString> HASH_STATION_PRODUCT;

#endif //DATA_MANAGER_DEF_H
