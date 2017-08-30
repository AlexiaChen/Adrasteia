#ifndef GEN_TAB_H
#define GEN_TAB_H

#include <string>
#include <vector>

#include "p_TabStruct.h"
#include "TCPSocket.h"

//�������� ���ݿ�� ��ע˵��
#define RECV_HEAD          0	//Integer	1		0x81	���ݰ�ǰ����
#define RECV_SENDER        1	//Integer	1		�����ߣ������ն�Ϊ0x01,Windwos������Ϊ0x02��Linux������Ϊ0x3
#define RECV_RECEIVER      2	//Integer	1		�����ߣ������ն�Ϊ0x01,Windwos������Ϊ0x02��Linux������Ϊ0x3
#define RECV_SENDTIME      3	//DWORD		4		ʱ��������ڷ����߼�¼�������͵�ʱ��ֵ����ȷ������
#define RECV_TYPE          4	//Integer	6		�Ӱ����ͣ�ֵΪЭ���ʶ��
#define RECV_LENGTH        5    //DWORD		4		�Ӱ�����
#define RECV_TRUE_LENGTH   6    //DWORD		4		�Ӱ�ѹ��ǰ����
#define RECV_FLAG		   7	//DWORD		4		0	��һλ�ɹ�ʧ�ܱ�־��0�ɹ���1ʧ�� ����λ ѹ����־ 0δѹ�� 1 ��ѹ��  ����λ ���ܽ��ܱ�־ 0δ���� 1�Ѽ���
#define RECV_DATA          8	//���ݰ�

#define SYS_HEAD_LEN	   25	//ͷ���ĳ���25���ֽ�
#define SENDER_MANGER	   0x01 //�������ǹ����ն�
#define SENDER_WINDOWS	   0x02	//��������Windows�ͻ���
#define SENDER_LINUX	   0x03	//��������Linux�ͻ���

#define MY_WM_RESTORE   WM_USER+1024
#define START_ZIP_LENGTH	10240

typedef struct tru_Client {             //�ͻ��˽ṹ
    // QTcpSocket *	Socket;         //�ͻ���Socket
    unsigned int		LoginTime;		//����ʱ��
    unsigned int        	LastSendTime;   //�ϴη��ͱ���ʱ���
    unsigned char      	SendTime;       //���ν��ձ��ĵ�ʱ���
    int         		PackageType;    //���ݰ�����
    int         		CUR_STEP;       //��ǰ���Ĵ�����
    int         		CUR_IDX;        //��ǰ�ֶν���λ��
    unsigned char        		bFieldBuf[16];  //�ֶλ�����
    unsigned int       		iPackageFLAG;   //���ı�־
    int         		iLength;        //���ĳ���
    int         		iTrue_Length;   //ѹ��ǰ���ĳ���
    // TMemoryStream * 	msRecv;   		//���ݽ�����
    unsigned char       *msRecv;    
    int                 msRecvLength;
    //std::vector<unsigned char>   msRecv;

    /*������Socket���ݽ������ */
    /*�����ǿͻ��������Ϣ */
    std::string	        IPAddress;      /*�ͻ���IP��ַ*/
    std::string	        MAC;            /*�ͻ��� MAC��ַ�������ص�ǰIP�� MAC*/
    std::string	        OSBootTime;     /*����ϵͳ����ʱ�䷵��������ʱ����*/
    std::string	        BaseInfo;       /*�ͻ��˵���Ӳ��������Ϣ����������ϵͳ��Ϣ*/
    std::string	        DiskSpaceInfo;  /*���̿ռ�ռ����Ϣ��һ�����̻�Ŀ¼һ�����ݣ�����һ�𷵻�*/
    int					CPU;            //CPUռ����
    int					Memory;         //�ڴ�ռ����
    //TListItem*			listItem;		//�ͻ�����ListView�ϵ�λ��
} TCLIENT_INFO;

typedef struct tru_Recv {               //���ݽ��ջ���ṹ
    // QTcpSocket *	Socket; 		//�ͻ���Socket
    unsigned char			 *	DataBuffer;		//�������ݻ�����
    int					iBufLength;		//���������ݳ���
} TRECV_DATA;



/////////p_InitTab////////////////
void p_InitTab_Client_Login(TCLIENT_LOGIN *P);
void p_InitTab_Client_State(TCLIENT_STATE *P);
void p_InitTab_ShutDownCmd(TSHUTDOWNCMD *P);

/////////////////p_TabStruct//////////////////////
int p_TabPack_Client_Login(char *cBuf, int iBufLen, TCLIENT_LOGIN *client_login);
int p_TabUnPack_Client_Login(char *cBuf, TCLIENT_LOGIN* client_login);
int p_TabPack_Client_State(char *cBuf, int iBufLen, TCLIENT_STATE *client_state);
int p_TabUnPack_Client_State(char *cBuf, TCLIENT_STATE* client_state);
int p_TabPack_ShutDownCmd(char *cBuf, int iBufLen, TSHUTDOWNCMD *shutdowncmd);
int p_TabUnPack_ShutDownCmd(char *cBuf, TSHUTDOWNCMD* shutdowncmd);

//////////////////////////////////////////////////////////////////////
void AnaData(unsigned char * bBuffer, int iBufLength,
    TCLIENT_INFO *CLIENT_INFO, bool &bRecvEnd);
void AnaPackage(Client* client_tcp, TCLIENT_INFO *serviceInfo);
void GetPackageFLAG(unsigned int dwFlag, bool &bError, bool &bCompress, bool &bCrypt);
//���ð�ͷ��Ϣ
void SetHeadPackage(unsigned char * bHeadBuffer, char * cType,
    int iSender, int iRecver, int iLength, int iTrue_Length, bool bCompressed);
//����һ�����ݰ�
void CreatPackage(char * cType, char *cSendBuf,
    int &iSendBufLen, unsigned char * bPackageData, int &iPackageLength);

//��������
void SendData(Client* client_tcp,unsigned char * bBuffer, int iDataLength);


#endif