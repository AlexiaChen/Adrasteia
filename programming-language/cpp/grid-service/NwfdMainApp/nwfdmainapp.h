/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: nwfdmainapp.h
* 作  者: Jeff  		版本：1.0		日  期：
* 描  述：主处理
* 其  他：
* 功能列表:
*   1. 初始化（日志，配置，任务，调度）
*   2. 启动调度，执行主处理
*   3. 界面监控
* 修改日志：
*   No.   日期		  作者		       修改内容
* ------------------------------------------------------------------------
*   1  2015/04/29	zhangl		优化主处理，以任务调度方式进行处理
*   2  2015/05/27   zhangl      调整主处理代码，添加日志
*************************************************************************/
#ifndef NWFDMAINAPP_H
#define NWFDMAINAPP_H

/************************************************************************/
/* 头文件引用                                                           */
/************************************************************************/
#include "XmlConfig.h"
#include "TaskManager.h"
#include "NetworkManager.h"
#include "nwfdtcpserver.h"
#include "DBConnPool.h"
#include "DDSManager.h"
#include "CachedImpl.h"
#include "TaskBase.h"
#include "TaskWebMonitor.h"
#include "Mask.h"

/************************************************************************/
/* 版本定义                                                             */
/************************************************************************/
#define VERSION  "2.0"       // 当前系统版本号
#define BUILED    1          

/************************************************************************/
/* 类 名： NwfdMainApp                                                  */
/* 父 类： QMainWindow                                                  */
/* 说 明： 主处理类                                                     */
/* 描 述：                                                              */
/************************************************************************/
class NwfdMainApp
{
public:
	NwfdMainApp();
	~NwfdMainApp();

private:
	/* 初始化，关闭处理 */
	void InitLog();
	void Close();

	/* 主处理 */
	void StartMain();
	void InitNetworkService();
	void InitDataBase();
	void InitDDS();
	void InitSysCheckup();
	void InitProductTask();
	void InitTaskMonitor();
	void InitStationTask();
	void InitStationMsgTask();
	void InitHttpRecver();
	void InitDownLoadTask();
	void InitUpLoadTask();
	void InitCheckupTask();

	bool AddTaskManager(TaskBase * pTask, ST_SCANTIME stScanTime, bool bFirstExecFlg = true);
	int  GetBeginTime(int nHour, int nMin, int nSecond);

private:
	/* 配置信息 */
	QString      m_strRoot;     // 数据存储根目录
	QString      m_strCccc;     // CCCC4位省编码
	float        m_fLon1;       // 起始经度
	float        m_fLon2;       // 终止经度
	float        m_fLat1;       // 起始纬度
	float        m_fLat2;       // 终止纬度
	HASH_SERVER_CFG m_hasServer;// FTP服务器配置
	ST_CHECKUP   m_stCheckup;   // 配置检测
	ST_DOWNLOAD  m_stDownload;  // 下载配置
	ST_UPLOAD    m_stUpload;    // 上传配置
	ST_SYSCLEAN  m_stSysClean;  // 系统清理配置
	HASH_PRODUCT m_hasProduct;  // 产品信息列表


	ST_TCPSERVER m_stTcpServer;     // TCP Server配置
	ST_CONNPOOL  m_stConnPool;      // 连接池配置
	ST_DDS_CFG   m_stDDSCfg;        // 数据分发配置
	ST_CACHED    m_stCached;        // 缓存配置
	ST_HTTP      m_stHttp;          // Http配置
	HASH_CLIENT  m_hasClient;       // 客户端配置列表

	/* 任务&调度 */
	TaskManager * m_pTaskManager;   // 任务调度管理
	HASH_TASK     m_hasTask;        // 任务列表

	/* 配置检测任务 */
	TaskCheckup*  m_pTaskCheckup;
	
	/* 数据库连接 */
	CDBConnPool * m_pDBConnPool;    // 数据库连接池

	/* 数据分发 */
	CDDSManager * m_pDDSManager;    // 数据分发管理器

	/* 网络管理 */
	NetworkManager *m_pNetworkManager; // 网络管理器
	NwfdTcpServer  *m_tcpServer;       // TCPServer
	CMask          *m_pMask;           // Mask常驻内存信息

	/* 站点处理 */
	LIST_STATION_CFG m_lstStationCfg;    // 站点配置列表
	ST_STATION_MAG   m_stStationMag;     // 站点管理：用于格点-站点转换

	/* 发布开关 */
	
	/*任务监控*/
	TaskWebMonitor *m_pWebMonitorTask;
};

#endif // NWFDMAINAPP_H
