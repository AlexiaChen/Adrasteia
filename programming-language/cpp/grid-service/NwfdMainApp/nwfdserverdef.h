/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: nwfdserverdef.h
* 作  者: zhangl  		版本：1.0		日  期：2015/07/13
* 描  述：网络服务定义
* 其  他：
* 功能列表:
* 修改日志：
*   No.   日期		  作者		       修改内容
* ------------------------------------------------------------------------
*************************************************************************/
#ifndef NWFD_SERVER_DEF_H
#define NWFD_SERVER_DEF_H

/************************************************************************/
/* TCPServer配置结构体定义                                              */
/************************************************************************/
typedef struct _TCPSERVER
{
	bool bDisabled;      // TCPServer是否不生效
	int  nListenPort;    // TCPServer侦听端口
	int  nMaxConnection; // TCPServer最大连接数
}ST_TCPSERVER;


#endif // NWFD_SERVER_DEF_H