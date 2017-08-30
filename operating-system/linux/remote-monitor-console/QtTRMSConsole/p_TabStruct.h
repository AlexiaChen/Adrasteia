/********************************************************
���ݿ��༭��2.19.2
�ļ��������ڣ�2017��07��22�� 12ʱ27��07��
���ݱ������3
********************************************************/
#ifndef BN_PC_TRMSH
#define BN_PC_TRMSH

#define CLIENT_LOGIN_LEN                4150
#define CLIENT_STATE_LEN                1084
#define SHUTDOWNCMD_LEN                 1024
/********************************************************/
/*�����ƣ�Client_Login	�ͻ��˵�¼*/

typedef struct _CLIENT_LOGIN{
    char        IPAddress[17];                  /*�ͻ���IP��ַ*/
    char        MAC[19];                        /*�ͻ��� MAC��ַ�������ص�ǰIP�� MAC*/
    char        OSBootTime[21];                 /*����ϵͳ����ʱ�䷵��������ʱ����*/
    char        BaseInfo[4097];                 /*�ͻ��˵���Ӳ��������Ϣ����������ϵͳ��Ϣ*/
}TCLIENT_LOGIN;

/********************************************************/
/*�����ƣ�Client_State	�ͻ���״̬����*/

typedef struct _CLIENT_STATE{
    char        IPAddress[17];                  /*�ͻ���IP��ַ*/
    char        MAC[19];                        /*�ͻ��� MAC��ַ�������ص�ǰIP�� MAC*/
    char        OSBootTime[21];                 /*����ϵͳ����ʱ�䷵��������ʱ����*/
    char        UseCPU[4];                      /*CPUռ����*/
    char        UserMemory[4];                  /*�ڴ�ռ����*/
    char        DiskSpaceInfo[1025];            /*���̿ռ�ռ����Ϣ��һ�����̻�Ŀ¼һ�����ݣ�����һ�𷵻�*/
}TCLIENT_STATE;

/********************************************************/
/*�����ƣ�ShutDownCmd	�ػ��ű�*/

typedef struct _SHUTDOWNCMD{
    char        ShutDownCmd[1025];              /*�ػ��ű���������Ҫ������Ҫ�ػ�ִ�еĽű����ͻ���ִ�нű����йػ�*/
}TSHUTDOWNCMD;

#endif