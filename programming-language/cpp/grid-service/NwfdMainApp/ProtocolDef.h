/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* �ļ���: ProtocolDef.h
* ��  ��: zhangl		�汾��1.0		��  �ڣ�2015/06/26
* ��  ����Э����Ϣ����
* ��  ����
* �����б�:
* �޸���־��
*************************************************************************/
#ifndef PROTOCOL_DEF_H
#define PROTOCOL_DEF_H

/************************************************************************/
/* 1. Э�������Ϣͷ                                                    */
/************************************************************************/
typedef struct _STD_MSG_HEAD
{
	unsigned char 	VER;    // �汾��        1Byte
	int         	ADR;    // �ͻ��˱�ʶ    4Byte
	unsigned char 	CMD;    // ������        1Byte
	unsigned char 	SEQ;    // ���ݰ����    1Byte
	unsigned char 	TYPE;   // ���ݸ�ʽ���  1Byte
	int          	LEN;    // ���ݳ���      4Bytes
} STD_MSG_HEAD, *LPSTD_MSG_HEAD;

/************************************************************************/
/* �汾�Ŷ��� & �㲥��ַ����                                            */
/************************************************************************/
#define  MSG_VERSION         0x10   // ��1.0
#define  BROADCAST_ADR       -1     // �㲥��ַ

/************************************************************************/
/* 2. Э��ͷ&β��ʼ��־λ                                               */
/************************************************************************/
#define STD_SOI           0x7E  // ��ʼ��־λ
#define STD_EOI           0x0D  // ��ֹ��־λ

/************************************************************************/
/* 3. Э�������붨��                                                    */
/************************************************************************/
#define CMD_HEARTBEAT             0x01  // ����
#define CMD_HEARTBEAT_ACK         0x81  // ����ȷ��
#define CMD_LOGIN                 0x02  // ��¼ע��
#define CMD_LOGIN_ACK             0x82  // ��¼ע��
#define CMD_SENDDATA              0x03  // ��������
#define CMD_SENDDATA_ACK          0x83  // ����ȷ��Ӧ��
#define CMD_SENDSTATION           0x04  // ����վ������
#define CMD_SENDSTATION_ACK       0x84  // վ������ȷ��Ӧ��
#define CMD_SENDSTATION_FILE      0x05  // ����վ���ļ�
#define CMD_SENDSTATION_FILE_ACK  0x85  // վ������ȷ��Ӧ��
#define CMD_SEND_MULT_STATION     0x06  // ����վ������,��վ�㣬��ʱЧ
#define CMD_SEND_MULT_STATION_ACK 0x86  // ���վ������ȷ��Ӧ��

#define CMD_GRID2STATION          0x07  // ���תվ������
#define CMD_GRID2STATION_ACK      0x87  // ���תվ��ȷ��Ӧ��
#define CMD_STATION2GRID          0x08  // վ��ת�������
#define CMD_STATION2GRID_ACK      0x88  // վ��ת���ȷ��Ӧ��
#define CMD_RAIN_PROCESS          0x09  // ��ˮһ���Դ�������
#define CMD_RAIN_PROCESS_ACK      0x89  // ��ˮһ���Դ���ȷ��Ӧ��
#define CMD_UP_STATIONCFG         0x0A  // ���ͻ���վ�����������������ͬ���ϴ�
#define CMD_UP_STATIONCFG_ACK     0x8A  // վ������ȷ��Ӧ��

#define CMD_MERGE_UPDATE          0x10  // ���������͵�ƴͼ������Ϣ
#define CMD_MERGE_UPDATE_ACK      0x90  // ���������͵�ƴͼ������Ϣȷ��			
#define CMD_STATION_UPDATE        0x11  // ���������͵�վ�������Ϣ
#define CMD_STATION_UPDATE_ACK    0x91  // ���������͵�վ�������Ϣȷ��			
#define CMD_MULTI_STATION_UPDATE     0x12  // ���������͵Ķ�վ�������Ϣ
#define CMD_MULTI_STATION_UPDATE_ACK 0x92  // ���������͵Ķ�վ�������Ϣȷ��		

#define CMD_ERROR                 0x8f  // ��������Ӧ-�յ������ݴ���

/************************************************************************/
/* 4. �������Ͷ���                                                      */
/************************************************************************/
#define TYPE_FORMAT_1     0x01  // ��һ�����ݸ�ʽ
#define TYPE_FORMAT_2     0x02  // �ڶ������ݸ�ʽ
#define TYPE_FORMAT_3     0x03  // ���������ݸ�ʽ
#define TYPE_FORMAT_4     0x04  // ���������ݸ�ʽ
#define TYPE_FORMAT_5     0x05  // ���������ݸ�ʽ-վ������
#define TYPE_FORMAT_6     0x06  // ���������ݸ�ʽ-ƴͼ
#define TYPE_FORMAT_7     0x07  // ���������ݸ�ʽ-վ��

/************************************************************************/
/* 5. ������Ϣ�е����Ͷ���                                              */
/************************************************************************/
#define DATA_TYPE_1       0x01  // ԭʼ����    float
#define DATA_TYPE_2       0x02  // �����е�Key string
#define DATA_TYPE_3       0x03  // ����zipѹ���������  char
#define DATA_TYPE_4       0x04  // Grib2��ʽ������  char

/************************************************************************/
/* 6. ���ݹ̶�����                                                      */
/************************************************************************/
#define MSG_MIN_LENGHET               14  // Э����̳���
#define MSG_LONGIN_USERNAME           25  // ��¼��Ϣ-�û�������
#define MSG_LONGIN_PASSWORD           25  // ��¼��Ϣ-���볤��
#define MSG_LONGIN_LENGHET            50  // ��¼��Ϣ����
#define TYPE_FORMAT_1_BASELENGHTH     38  // ��һ�����ݸ�ʽ
#define TYPE_FORMAT_2_BASELENGHTH     5   // �ڶ������ݸ�ʽ
#define TYPE_FORMAT_3_BASELENGHTH     43  // ���������ݸ�ʽ
#define TYPE_FORMAT_5_BASELENGHTH     41  // ���������ݸ�ʽ
#define TYPE_FORMAT_6_BASELENGHTH     13  // ���������ݸ�ʽ
#define TYPE_FORMAT_7_BASELENGHTH     39  // ���������ݸ�ʽ

/************************************************************************/
/* 7. ���ݹ̶�����                                                      */
/************************************************************************/
#define MSG_LOGIN_FAILED    0x90     // ��¼ʧ��
#define RIGHT_WRITE         0x01     // ��дȨ��
#define RIGHT_NOTWRITE      0x00     // û��дȨ��


#endif //PROTOCOL_DEF_H


