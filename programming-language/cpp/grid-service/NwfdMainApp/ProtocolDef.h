/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: ProtocolDef.h
* 作  者: zhangl		版本：1.0		日  期：2015/06/26
* 描  述：协议信息定义
* 其  他：
* 功能列表:
* 修改日志：
*************************************************************************/
#ifndef PROTOCOL_DEF_H
#define PROTOCOL_DEF_H

/************************************************************************/
/* 1. 协议基本信息头                                                    */
/************************************************************************/
typedef struct _STD_MSG_HEAD
{
	unsigned char 	VER;    // 版本号        1Byte
	int         	ADR;    // 客户端标识    4Byte
	unsigned char 	CMD;    // 命令码        1Byte
	unsigned char 	SEQ;    // 数据包序号    1Byte
	unsigned char 	TYPE;   // 数据格式类别  1Byte
	int          	LEN;    // 数据长度      4Bytes
} STD_MSG_HEAD, *LPSTD_MSG_HEAD;

/************************************************************************/
/* 版本号定义 & 广播地址定义                                            */
/************************************************************************/
#define  MSG_VERSION         0x10   // 即1.0
#define  BROADCAST_ADR       -1     // 广播地址

/************************************************************************/
/* 2. 协议头&尾起始标志位                                               */
/************************************************************************/
#define STD_SOI           0x7E  // 起始标志位
#define STD_EOI           0x0D  // 终止标志位

/************************************************************************/
/* 3. 协议命令码定义                                                    */
/************************************************************************/
#define CMD_HEARTBEAT             0x01  // 心跳
#define CMD_HEARTBEAT_ACK         0x81  // 心跳确认
#define CMD_LOGIN                 0x02  // 登录注册
#define CMD_LOGIN_ACK             0x82  // 登录注册
#define CMD_SENDDATA              0x03  // 发送数据
#define CMD_SENDDATA_ACK          0x83  // 数据确认应答
#define CMD_SENDSTATION           0x04  // 发送站点数据
#define CMD_SENDSTATION_ACK       0x84  // 站点数据确认应答
#define CMD_SENDSTATION_FILE      0x05  // 发送站点文件
#define CMD_SENDSTATION_FILE_ACK  0x85  // 站点数据确认应答
#define CMD_SEND_MULT_STATION     0x06  // 发送站点数据,多站点，多时效
#define CMD_SEND_MULT_STATION_ACK 0x86  // 多个站点数据确认应答

#define CMD_GRID2STATION          0x07  // 格点转站点请求
#define CMD_GRID2STATION_ACK      0x87  // 格点转站点确认应答
#define CMD_STATION2GRID          0x08  // 站点转格点请求
#define CMD_STATION2GRID_ACK      0x88  // 站点转格点确认应答
#define CMD_RAIN_PROCESS          0x09  // 降水一致性处理请求
#define CMD_RAIN_PROCESS_ACK      0x89  // 降水一致性处理确认应答
#define CMD_UP_STATIONCFG         0x0A  // 将客户端站点配置与服务器进行同步上传
#define CMD_UP_STATIONCFG_ACK     0x8A  // 站点配置确认应答

#define CMD_MERGE_UPDATE          0x10  // 服务器发送的拼图更新消息
#define CMD_MERGE_UPDATE_ACK      0x90  // 服务器发送的拼图更新消息确认			
#define CMD_STATION_UPDATE        0x11  // 服务器发送的站点更新消息
#define CMD_STATION_UPDATE_ACK    0x91  // 服务器发送的站点更新消息确认			
#define CMD_MULTI_STATION_UPDATE     0x12  // 服务器发送的多站点更新消息
#define CMD_MULTI_STATION_UPDATE_ACK 0x92  // 服务器发送的多站点更新消息确认		

#define CMD_ERROR                 0x8f  // 服务器响应-收到的数据错误

/************************************************************************/
/* 4. 数据类型定义                                                      */
/************************************************************************/
#define TYPE_FORMAT_1     0x01  // 第一类数据格式
#define TYPE_FORMAT_2     0x02  // 第二类数据格式
#define TYPE_FORMAT_3     0x03  // 第三类数据格式
#define TYPE_FORMAT_4     0x04  // 第四类数据格式
#define TYPE_FORMAT_5     0x05  // 第五类数据格式-站点数据
#define TYPE_FORMAT_6     0x06  // 第六类数据格式-拼图
#define TYPE_FORMAT_7     0x07  // 第七类数据格式-站点

/************************************************************************/
/* 5. 数据信息中的类型定义                                              */
/************************************************************************/
#define DATA_TYPE_1       0x01  // 原始数据    float
#define DATA_TYPE_2       0x02  // 缓存中的Key string
#define DATA_TYPE_3       0x03  // 采用zip压缩后的数据  char
#define DATA_TYPE_4       0x04  // Grib2格式的数据  char

/************************************************************************/
/* 6. 数据固定长度                                                      */
/************************************************************************/
#define MSG_MIN_LENGHET               14  // 协议最短长度
#define MSG_LONGIN_USERNAME           25  // 登录信息-用户名长度
#define MSG_LONGIN_PASSWORD           25  // 登录信息-密码长度
#define MSG_LONGIN_LENGHET            50  // 登录信息长度
#define TYPE_FORMAT_1_BASELENGHTH     38  // 第一类数据格式
#define TYPE_FORMAT_2_BASELENGHTH     5   // 第二类数据格式
#define TYPE_FORMAT_3_BASELENGHTH     43  // 第三类数据格式
#define TYPE_FORMAT_5_BASELENGHTH     41  // 第五类数据格式
#define TYPE_FORMAT_6_BASELENGHTH     13  // 第六类数据格式
#define TYPE_FORMAT_7_BASELENGHTH     39  // 第六类数据格式

/************************************************************************/
/* 7. 数据固定长度                                                      */
/************************************************************************/
#define MSG_LOGIN_FAILED    0x90     // 登录失败
#define RIGHT_WRITE         0x01     // 有写权限
#define RIGHT_NOTWRITE      0x00     // 没有写权限


#endif //PROTOCOL_DEF_H


