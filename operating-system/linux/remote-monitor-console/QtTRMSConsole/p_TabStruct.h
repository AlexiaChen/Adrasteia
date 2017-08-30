/********************************************************
数据库表编辑器2.19.2
文件创建日期：2017年07月22日 12时27分07秒
数据表个数：3
********************************************************/
#ifndef BN_PC_TRMSH
#define BN_PC_TRMSH

#define CLIENT_LOGIN_LEN                4150
#define CLIENT_STATE_LEN                1084
#define SHUTDOWNCMD_LEN                 1024
/********************************************************/
/*包名称：Client_Login	客户端登录*/

typedef struct _CLIENT_LOGIN{
    char        IPAddress[17];                  /*客户端IP地址*/
    char        MAC[19];                        /*客户端 MAC地址，仅返回当前IP的 MAC*/
    char        OSBootTime[21];                 /*操作系统启动时间返回年月日时分秒*/
    char        BaseInfo[4097];                 /*客户端的软硬件环境信息，包含操作系统信息*/
}TCLIENT_LOGIN;

/********************************************************/
/*包名称：Client_State	客户端状态回送*/

typedef struct _CLIENT_STATE{
    char        IPAddress[17];                  /*客户端IP地址*/
    char        MAC[19];                        /*客户端 MAC地址，仅返回当前IP的 MAC*/
    char        OSBootTime[21];                 /*操作系统启动时间返回年月日时分秒*/
    char        UseCPU[4];                      /*CPU占用率*/
    char        UserMemory[4];                  /*内存占用率*/
    char        DiskSpaceInfo[1025];            /*磁盘空间占用信息，一个磁盘或目录一条数据，多条一起返回*/
}TCLIENT_STATE;

/********************************************************/
/*包名称：ShutDownCmd	关机脚本*/

typedef struct _SHUTDOWNCMD{
    char        ShutDownCmd[1025];              /*关机脚本，根据需要定制需要关机执行的脚本，客户端执行脚本进行关机*/
}TSHUTDOWNCMD;

#endif