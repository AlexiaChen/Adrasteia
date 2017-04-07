/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: logDef.h
* 作  者: zhangl  		版本：1.0		日  期：2015/05/05
* 描  述：日志公共调用头文件
* 其  他：
* 功能列表:
*   1.
* 修改日志：
*   No.   日期		  作者		       修改内容
* ------------------------------------------------------------------------
*  
*************************************************************************/
#ifndef LOG_DEF_H
#define LOG_DEF_H

/************************************************************************/
/* Log配置文件                                                          */
/************************************************************************/
#define LOG_CONFIG_FILE  "logconfig.ini"

/************************************************************************/
/* Log名称宏定义                                                        */
/************************************************************************/
#define LOGID_INFO      "info"      // 系统运行状态
#define LOGID_ERROR     "error"     // 错误日志，会导致系统无法正常运行
#define LOGID_DEBUG     "debug"     // 重要调试日志
#define LOGID_PRODUCT   "product"   // 产品处理记录日志
#define LOGID_DOWNLOAD  "download"  // 下载日志
#define LOGID_UPLOAD    "upload"    // 上传日志
#define LOGID_DDS       "dds"       // 数据分发日志
#define LOGID_NETWORK   "network"   // 网络
#define LOGID_STATION   "station"   // 站点
#define LOGID_MERGE     "merge"    // 拼图
/************************************************************************/
/* 定义Log字符串转换  QString类型到log用的字符类型                      */
/************************************************************************/
#ifdef UNICODE
#define LOG_F(str)    QString(str).toStdWString().data()   //  for format
#define LOG_STR(str)  str.toStdWString().data()            //  for ...
#else
#define LOG_F(str)    str
#define LOG_STR(str)  str.toStdString().data()
#endif

/************************************************************************/
/* Log输出 Windows，Linux兼容性定义                                     */
/************************************************************************/
#ifdef _WIN32 // for Windows
#ifdef UNICODE
#define  LOG_         LOGW__ 
#else
#define  LOG_         LOGA__ 
#endif
#else    // for Linux
#define  LOG_         LOG__ 
#endif

/************************************************************************/
/* 中文乱码，定义字符集编码                                             */
/************************************************************************/
#pragma execution_character_set("utf-8")

/************************************************************************************************
* 日志使用说明：
*  1、使用顺序： 初始化 - 创建日志 - 初始化日志配置 - 使用日志 - 日志退出
*  2、日志保存位置和命名：（dir\log\logID_time.log）在程序目录的 log目录下，一个logId一个文件
*     YAOLOG_SET_LOGFILE_ATTR(LOGID_I, false, false, false,"c:\\logfile", "tt.log");
*
*  3、日志的创建：LOG_TYPE_TEXT、LOG_TYPE_BIN、LOG_TYPE_FORMATTED_BIN 三种类型的文件
*     YAOLOG_CREATE(LOGID_A, true, YaoUtil::LOG_TYPE_TEXT);
*     YAOLOG_CREATE(LOGID_B, true, YaoUtil::LOG_TYPE_BIN);            // LOGID_B is a binary logger
*     YAOLOG_CREATE(LOGID_FB, true, YaoUtil::LOG_TYPE_FORMATTED_BIN); // LOGID_FB is a formatted binary logger
*
*  4、日志配置： 可以通过配置文件，也可以直接配置
*
*  5、使用：LOGA__：非中文;   LOGW__: 可以有中文
*     LOGA__(LOGID_I, "My name is %s! I'm %d!", "ford", 29);
*     LOGW__(LOGID_I, L"我的名字是： %s! I'm %d!", L"ford", 29);
*
*     YAOLOG_DISABLE_ALL(true);
*
*	// output both to file and http server
*	YAOLOG_SET_LOG_ATTR( LOGID_I, true, YaoUtil::OUT_FLAG_FILE | YaoUtil::OUT_FLAG_REMOTE, true, true, true, true,
*	"http://192.168.1.195/default.aspx");
*	YAOLOG_SET_LOGFILE_ATTR(LOGID_I, false, false, false, "c:\\logfile", "tt.log");
*	LOG__(LOGID_I, _T("My name is %s! I'm %d!"), _T("neil"), 29);
*
*	// log binary data
*	YAOLOG_SET_ATTR_FROM_CONFIG_FILE(LOGID_B, "logconfig.ini");
*	char bin[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
*	LOGBIN__(LOGID_B, bin, 10);
*
*	// log formatted binary data
*	YAOLOG_SET_ATTR_FROM_CONFIG_FILE(LOGID_FB, "logconfig.ini");
*	char data1[20] = { 0x52, 0x55, 0x4F, 0x4B, 4, 5, 6, 7, 8, 9, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
*	char data2[4] = { 0x4F, 0x4B, 0x00, 0x01 };
*	LOGBIN_F__(LOGID_FB, "send", data1, 20);
*	LOGBIN_F__(LOGID_FB, "recv", data2, 4);
*************************************************************************************************/

#endif //LOG_DEF_H
