/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: XmlConfig.h
* 作  者: Jeff  		版本：1.0		日  期：2015/04/09
* 描  述：配置文件
* 其  他：
* 功能列表:
*   1.
* 修改日志：
*   No.   日期		  作者		       修改内容
* ------------------------------------------------------------------------
*   1  2015/04/14    zhangl    修改读取配置文件的方法
*   2  2015/04/29    zhangl    配置文件结构变更，修改读取方法
*   3  2015/05/08    zhangl    修改产品扫描时间的结构
*   4  2015/05/26    zhangl    修改数据加工处理chains的结构
*************************************************************************/
#ifndef XMLCONFIG_H
#define XMLCONFIG_H

/************************************************************************/
/* 头文件引用                                                           */
/************************************************************************/
#include "ProductDef.h"   // 产品定义头文件
#include "CachedDef.h"    // 缓存定义头文件
#include "DDSDef.h"       // 数据分发定义头文件
#include "nwfdserverdef.h"// 网络服务定义头文件
#include "NwfdClientDef.h"// 客户端配置定义头文件
#include "DataBaseDef.h"  // 数据库定义头文件 
#include "TaskSysClean.h"   // 系统清理维护头文件
#include "StationDef.h"   // 站点定义头文件
#include "TaskCheckup.h"
#include "HttpDef.h"
#include "FtpDef.h"
#include <QDomDocument>
#include <QFile>
#include <QString>

/************************************************************************/
/* 配置文件名称宏定义                                                   */
/************************************************************************/
#define  NWFDSYS_CONFIG_FILE    "NwfdServerConfig.xml"
#define  CLIENT_CONFIG_FILE     "ClientConfig.xml"
#define  PRODUCT_CONFIG_FILE    "ProductConfig.xml"
#define  MERGE_CONFIG_FILE      "MergeConfig.xml"
#define  STATION_CONFIG_FILE    "StationConfig.xml"
/************************************************************************/
/* 类 名： XmlConfig                                                    */
/* 父 类： -                                                            */
/* 说 明： XML配置读取类                                                */
/* 描 述： 单例                                                         */
/************************************************************************/
class XmlConfig
{
private:
	XmlConfig();

public:
	~XmlConfig();
	static XmlConfig & getClass();

public:
	ST_TCPSERVER GetTcpServer();
	ST_CACHED    GetCachedConfig();
	ST_SYSCLEAN  GetSysClean();    // 系统清理
	ST_DDS_CFG   GetDDSConfig();
	HASH_PRODUCT GetProductList();
	HASH_CLIENT  GetClientList();
	ST_CONNPOOL  GetConnPool();
	LIST_STATION_CFG GetStationCfg();
	ST_STATION_MAG GetStationMag();

	ST_HTTP GetHttpCfg();

	ST_DOWNLOAD GetDownLoadCfg(); // 下载
	ST_UPLOAD  GetUpLoadCfg();    // 上传
	ST_CHECKUP GetCheckupCfg();   // 配置检测
	QString  GetRoot();
	QString  GetCCCC();
	float GetLon1();
	float GetLon2();
	float GetLat1();
	float GetLat2();

	bool  IsMonitor();
	bool  IsPublish();

private:
	bool ReadConfig();

	int ReadNwfdSysConfig();  // 读取系统基本配置
	int ReadClientConfig();   // 读取客户端配置
	int ReadProductConfig();  // 读取产品扫描配置
	int ReadStationConfig();  // 读取站点配置

	bool FindProduct(QString strKey);
	bool AddProductList(ST_PRODUCT stProduct);
	void GetFileNameFormat(QString strFileName , ST_FILE_FORMAT &stFormat);

private:
	/* 配置信息 */
	QString      m_strRoot;     // 数据存储根目录
	QString      m_strCccc;     // CCCC4位省编码
	float        m_fLon1;       // 起始经度
	float        m_fLon2;       // 终止经度
	float        m_fLat1;       // 起始纬度
	float        m_fLat2;       // 终止纬度

	ST_CHECKUP  m_stCheckup;    // 配置检测
	ST_DOWNLOAD  m_stDownLoad;  // 下载配置
	ST_UPLOAD    m_stUpLoad;    // 上传配置
	ST_SYSCLEAN  m_stSysClean;  // 系统清理
	HASH_PRODUCT m_hasProduct;  // 产品列表配置

	/////////////////////////////////////////////////////////
	bool         m_bMonitor;    // 任务监控开关 -- 默认不开启
	bool         m_bPublish;    // 发布总开关 -- 默认开启

	ST_TCPSERVER m_stTcpServer; // TCP Server配置
	ST_CACHED    m_stCached;    // Cache配置
	ST_DDS_CFG   m_stDDSCfg;    // 数据分发配置
	HASH_CLIENT  m_hasClient;   // 客户端配置列表
	ST_CONNPOOL  m_stConnPool;  // 连接池配置
	ST_HTTP      m_stHttp;      // http配置

	LIST_STATION_CFG m_lstStationCfg; // 站点配置列表
	ST_STATION_MAG    m_stStationMag; // 站点管理：用于格点-站点转换
};


#endif // XMLCONFIG_H
