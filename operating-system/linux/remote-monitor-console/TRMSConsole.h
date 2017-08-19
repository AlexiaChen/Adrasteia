#ifndef TRMS_CONSOLE_H
#define TRMS_CONSOLE_H

#include <string>
#include <vector>

#include <QObject>
#include<QTcpSocket>
#include <QTimer>

#include <cstdint>

#include "p_TabStruct.h"

//数据类型 数据宽度 备注说明
#define RECV_HEAD          0	//Integer	1		0x81	数据包前导符
#define RECV_SENDER        1	//Integer	1		发送者，管理终端为0x01,Windwos服务器为0x02，Linux服务器为0x3
#define RECV_RECEIVER      2	//Integer	1		接收者，管理终端为0x01,Windwos服务器为0x02，Linux服务器为0x3
#define RECV_SENDTIME      3	//DWORD		4		时间戳，用于发送者记录本机发送的时间值，精确到毫秒
#define RECV_TYPE          4	//Integer	6		子包类型，值为协议标识符
#define RECV_LENGTH        5    //DWORD		4		子包长度
#define RECV_TRUE_LENGTH   6    //DWORD		4		子包压缩前长度
#define RECV_FLAG		   7	//DWORD		4		0	第一位成功失败标志，0成功，1失败 第三位 压缩标志 0未压缩 1 已压缩  第四位 加密解密标志 0未加密 1已加密
#define RECV_DATA          8	//数据包

#define SYS_HEAD_LEN	   25	//头包的长度25个字节
#define SENDER_MANGER	   0x01 //发送者是管理终端
#define SENDER_WINDOWS	   0x02	//发送者是Windows客户端
#define SENDER_LINUX	   0x03	//发送者是Linux客户端

#define MY_WM_RESTORE   WM_USER+1024
#define START_ZIP_LENGTH	10240

typedef struct tru_Client {             //客户端结构
    QTcpSocket *	Socket;         //客户端Socket
    uint32_t		LoginTime;		//上线时间
    uint32_t       	LastSendTime;   //上次发送报文时间戳
    uint8_t      	SendTime;       //本次接收报文的时间戳
    int         		PackageType;    //数据包类型
    int         		CUR_STEP;       //当前报文处理环节
    int         		CUR_IDX;        //当前字段接收位置
    uint8_t        		bFieldBuf[16];  //字段缓冲区
    uint32_t       		iPackageFLAG;   //报文标志
    int         		iLength;        //报文长度
    int         		iTrue_Length;   //压缩前报文长度
    // TMemoryStream * 	msRecv;   		//数据接收流
    std::vector<uint8_t>   msRecv;

    /*以上是Socket数据接收相关 */
    /*以下是客户端相关信息 */
    std::string	        IPAddress;      /*客户端IP地址*/
    std::string	        MAC;            /*客户端 MAC地址，仅返回当前IP的 MAC*/
    std::string	        OSBootTime;     /*操作系统启动时间返回年月日时分秒*/
    std::string	        BaseInfo;       /*客户端的软硬件环境信息，包含操作系统信息*/
    std::string	        DiskSpaceInfo;  /*磁盘空间占用信息，一个磁盘或目录一条数据，多条一起返回*/
    int					CPU;            //CPU占用率
    int					Memory;         //内存占用率
    //TListItem*			listItem;		//客户端在ListView上的位置
} TCLIENT_INFO;

typedef struct tru_Recv {               //数据接收缓冲结构
    QTcpSocket *	Socket; 		//客户端Socket
    uint8_t			 *	DataBuffer;		//接收数据缓冲区
    int					iBufLength;		//缓冲区数据长度
} TRECV_DATA;


class TRMSConsole : public QObject
{
    Q_OBJECT

public:
    TRMSConsole(QObject *parent = NULL);
    ~TRMSConsole();

    void login();
signals:
    void logined();
    void remoteShutdown(const QString& script);
    void clientState();
    void loginOut();

private slots:

    void revData(); 
    void sendTask();

    void displayError(QAbstractSocket::SocketError);
    void slotConnected();
    void slotDisConnected();

    void slotLogined();
    void slotRemoteShutDown(const QString& script);
    void slotClientState();
    void slotLoginOut();

private:
    void sendClientState();

private:
    
    void initSiganlSlot();
    void disconnectSinalSlot();
    //   自动生成的打包解包代码

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
    void AnaData(uint8_t * bBuffer, int iBufLength,
        TCLIENT_INFO *CLIENT_INFO, bool &bRecvEnd);
    void  AnaPackage(TCLIENT_INFO *serviceInfo);
    void GetPackageFLAG(uint32_t dwFlag, bool &bError, bool &bCompress, bool &bCrypt);
    //设置包头信息
    void SetHeadPackage(uint8_t * bHeadBuffer, char * cType,
        int iSender, int iRecver, int iLength, int iTrue_Length, bool bCompressed);
    //创建一个数据包
    void CreatPackage(char * cType, char *cSendBuf,
        int &iSendBufLen, uint8_t ** bPackageData, int &iPackageLength);

    //发送数据
    int SendData(QTcpSocket *Socket, uint8_t * bBuffer, int iDataLength);


private:
    QTcpSocket *m_tcpSocket;
    QTimer m_sendTaskTimer;
    TCLIENT_INFO m_client_info;
    bool m_isRecvEnd;
};

#endif
