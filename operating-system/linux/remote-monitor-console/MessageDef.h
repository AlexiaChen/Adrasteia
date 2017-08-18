#ifndef MESSAGE_DEF_H
#define MESSAGE_DEF_H

#include <string>
#include <cstdint>
#include <vector>
                                    //数据类型 数据宽度 备注说明
const int RECV_HEAD = 0;	        //Integer	1		0x81	数据包前导符
const int RECV_SENDER = 1;	        //Integer	1		发送者，管理终端为0x01,Windwos服务器为0x02，Linux服务器为0x3
const int RECV_RECEIVER = 2;	    //Integer	1		接收者，管理终端为0x01,Windwos服务器为0x02，Linux服务器为0x3
const int RECV_SENDTIME = 3;	    //DWORD		4		时间戳，用于发送者记录本机发送的时间值，精确到毫秒
const int RECV_TYPE = 4;	        //Integer	6		子包类型，值为协议标识符
const int RECV_LENGTH = 5;           //DWORD    4		子包长度
const int RECV_TRUE_LENGTH = 6;     //DWORD		4		子包压缩前长度
const int RECV_FLAG = 7;	       //DWORD		4		0	第一位成功失败标志，0成功，1失败 第三位 压缩标志 0未压缩 1 已压缩  第四位 加密解密标志 0未加密 1已加密
const int RECV_DATA = 8;	       //数据包

const int SYS_HEAD_LEN = 25;	//头包的长度25个字节
const int SENDER_MANGER = 0x01; //发送者是管理终端
const int SENDER_WINDOWS = 0x02;	//发送者是Windows客户端
const int SENDER_LINUX = 0x03;	//发送者是Linux客户端

const int START_ZIP_LENGTH = 10240;

class QTcpSocket;

typedef struct _TCP_CLIENT_INFO 
{
    QTcpSocket          *custom_socket;
    std::string			LoginTime;		//上线时间
    uint32_t       		LastSendTime;   //上次发送报文时间戳
    uint32_t     	 	SendTime;       //本次接收报文的时间戳
    int         		PackageType;    //数据包类型
    int         		CUR_STEP;       //当前报文处理环节
    int         		CUR_IDX;        //当前字段接收位置
    uint8_t        		bFieldBuf[16];  //字段缓冲区
    int32_t       		iPackageFLAG;   //报文标志
    int         		iLength;        //报文长度
    int         		iTrue_Length;   //压缩前报文长度
    std::vector<uint8_t> msRecv;   		//数据接收流
    std::string	        IPAddress;      /*客户端IP地址*/
    std::string	        MAC;            /*客户端 MAC地址，仅返回当前IP的 MAC*/
    std::string	        OSBootTime;     /*操作系统启动时间返回年月日时分秒*/
    std::string	        BaseInfo;       /*客户端的软硬件环境信息，包含操作系统信息*/
    std::string	        DiskSpaceInfo;  /*磁盘空间占用信息，一个磁盘或目录一条数据，多条一起返回*/
    //TListItem*			listItem;		//客户端在ListView上的位置

}TCP_CLIENT_INFO;

typedef struct _RecvData {               //数据接收缓冲结构
    QTcpSocket         *Socket; 		//客户端Socket
    uint8_t			   *DataBuffer;		//接收数据缓冲区
    size_t			   iBufLength;		//缓冲区数据长度
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
