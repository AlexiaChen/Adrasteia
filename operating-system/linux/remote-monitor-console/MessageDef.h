#ifndef MESSAGE_DEF_H
#define MESSAGE_DEF_H

#include <string>
#include <cstdint>
#include <vector>
                                    //�������� ���ݿ�� ��ע˵��
const int RECV_HEAD = 0;	        //Integer	1		0x81	���ݰ�ǰ����
const int RECV_SENDER = 1;	        //Integer	1		�����ߣ������ն�Ϊ0x01,Windwos������Ϊ0x02��Linux������Ϊ0x3
const int RECV_RECEIVER = 2;	    //Integer	1		�����ߣ������ն�Ϊ0x01,Windwos������Ϊ0x02��Linux������Ϊ0x3
const int RECV_SENDTIME = 3;	    //DWORD		4		ʱ��������ڷ����߼�¼�������͵�ʱ��ֵ����ȷ������
const int RECV_TYPE = 4;	        //Integer	6		�Ӱ����ͣ�ֵΪЭ���ʶ��
const int RECV_LENGTH = 5;           //DWORD    4		�Ӱ�����
const int RECV_TRUE_LENGTH = 6;     //DWORD		4		�Ӱ�ѹ��ǰ����
const int RECV_FLAG = 7;	       //DWORD		4		0	��һλ�ɹ�ʧ�ܱ�־��0�ɹ���1ʧ�� ����λ ѹ����־ 0δѹ�� 1 ��ѹ��  ����λ ���ܽ��ܱ�־ 0δ���� 1�Ѽ���
const int RECV_DATA = 8;	       //���ݰ�

const int SYS_HEAD_LEN = 25;	//ͷ���ĳ���25���ֽ�
const int SENDER_MANGER = 0x01; //�������ǹ����ն�
const int SENDER_WINDOWS = 0x02;	//��������Windows�ͻ���
const int SENDER_LINUX = 0x03;	//��������Linux�ͻ���

const int START_ZIP_LENGTH = 10240;

class QTcpSocket;

typedef struct _TCP_CLIENT_INFO 
{
    QTcpSocket          *custom_socket;
    std::string			LoginTime;		//����ʱ��
    uint32_t       		LastSendTime;   //�ϴη��ͱ���ʱ���
    uint32_t     	 	SendTime;       //���ν��ձ��ĵ�ʱ���
    int         		PackageType;    //���ݰ�����
    int         		CUR_STEP;       //��ǰ���Ĵ�����
    int         		CUR_IDX;        //��ǰ�ֶν���λ��
    uint8_t        		bFieldBuf[16];  //�ֶλ�����
    int32_t       		iPackageFLAG;   //���ı�־
    int         		iLength;        //���ĳ���
    int         		iTrue_Length;   //ѹ��ǰ���ĳ���
    std::vector<uint8_t> msRecv;   		//���ݽ�����
    std::string	        IPAddress;      /*�ͻ���IP��ַ*/
    std::string	        MAC;            /*�ͻ��� MAC��ַ�������ص�ǰIP�� MAC*/
    std::string	        OSBootTime;     /*����ϵͳ����ʱ�䷵��������ʱ����*/
    std::string	        BaseInfo;       /*�ͻ��˵���Ӳ��������Ϣ����������ϵͳ��Ϣ*/
    std::string	        DiskSpaceInfo;  /*���̿ռ�ռ����Ϣ��һ�����̻�Ŀ¼һ�����ݣ�����һ�𷵻�*/
    //TListItem*			listItem;		//�ͻ�����ListView�ϵ�λ��

}TCP_CLIENT_INFO;

typedef struct _RecvData {               //���ݽ��ջ���ṹ
    QTcpSocket         *Socket; 		//�ͻ���Socket
    uint8_t			   *DataBuffer;		//�������ݻ�����
    size_t			   iBufLength;		//���������ݳ���
} RECV_DATA_T;


typedef struct _MsgHeader
{
    uint8_t head;
    uint8_t sender;
    uint8_t receiver;
    uint32_t sendtime;
    uint8_t type[6];
    uint32_t length;
    uint32_t true_length;
    uint32_t flag;
}MsgHeader;

typedef struct SubMsg1001
{
    char ip[15];
    char mac[18];
    char os_boot_time[128];
    char base_info[4096];
}SubMsg1001;

typedef struct SubMsg1003
{
    char ip[15];
    char mac[18];
    char os_boot_time[128];
    char cpu_usage[3];
    char mem_usage[3];
    char disk_space[1024];
}SubMsg1003;

#endif
