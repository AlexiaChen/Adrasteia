#include "log.h" // log日志
#include "nwfdmainapp.h"
#include "TaskSysClean.h"
#include "TaskProductRaw.h"
#include "TaskProductMerge.h"
#include "TaskProductClientMerge.h"
#include "TaskDDS.h"
#include "TaskTcpServer.h"
#include "TaskStationRaw.h"
#include "TaskStationMerge.h"
#include "TaskStationMsg.h"
#include "TaskHttpRecver.h"
#include "TaskWebMonitor.h"
#include "TaskFtpRecver.h"
#include "TaskFtpSender.h"
#include "DataManager.h"
#include "ProductTaskManager.h"
#include <QDateTime>

/************************************************************************/
/* 构造函数                                                             */
/************************************************************************/
NwfdMainApp::NwfdMainApp()
{
	m_pTaskManager = NULL;
	m_pNetworkManager = NULL;
	m_tcpServer = NULL;
	m_pDBConnPool = NULL;
	m_pDDSManager = NULL;
	m_pMask = NULL;
	m_pWebMonitorTask = NULL;
	m_pTaskCheckup = NULL;

	// 初始化日志
	InitLog();

	// 执行主处理
	StartMain();
}

/************************************************************************/
/* 析构函数                                                             */
/************************************************************************/
NwfdMainApp::~NwfdMainApp()
{
	// 执行关闭处理
	Close();
}

/************************************************************************/
/* 初始化日志                                                           */
/************************************************************************/
void NwfdMainApp::InitLog()
{
	// 日志初始化（call YAOLOG_INIT at the app entry point）
	YAOLOG_INIT;

	// 创建日志模块
	YAOLOG_CREATE(LOGID_INFO,    true, YaoUtil::LOG_TYPE_TEXT); // 主要运行日志
	YAOLOG_CREATE(LOGID_ERROR,   true, YaoUtil::LOG_TYPE_TEXT); // 错误异常日志
	YAOLOG_CREATE(LOGID_DEBUG,   true, YaoUtil::LOG_TYPE_TEXT); // debug日志
	YAOLOG_CREATE(LOGID_PRODUCT, true, YaoUtil::LOG_TYPE_TEXT); // 产品处理记录日志
	YAOLOG_CREATE(LOGID_DOWNLOAD,true, YaoUtil::LOG_TYPE_TEXT); // 下载
	YAOLOG_CREATE(LOGID_UPLOAD,  true, YaoUtil::LOG_TYPE_TEXT); // 上传
	YAOLOG_CREATE(LOGID_DDS,     true, YaoUtil::LOG_TYPE_TEXT); // 数据分发日志

	// 从配置文件进行初始化
	YAOLOG_SET_ATTR_FROM_CONFIG_FILE(LOGID_INFO,    LOG_CONFIG_FILE);
	YAOLOG_SET_ATTR_FROM_CONFIG_FILE(LOGID_ERROR,   LOG_CONFIG_FILE);
	YAOLOG_SET_ATTR_FROM_CONFIG_FILE(LOGID_DEBUG,   LOG_CONFIG_FILE);
	YAOLOG_SET_ATTR_FROM_CONFIG_FILE(LOGID_PRODUCT, LOG_CONFIG_FILE);
	YAOLOG_SET_ATTR_FROM_CONFIG_FILE(LOGID_DDS,     LOG_CONFIG_FILE);
	YAOLOG_SET_ATTR_FROM_CONFIG_FILE(LOGID_DOWNLOAD,LOG_CONFIG_FILE);
	YAOLOG_SET_ATTR_FROM_CONFIG_FILE(LOGID_UPLOAD,  LOG_CONFIG_FILE);

	// log
	LOG_(LOGID_INFO, LOG_F("Nwfd服务程序开始运行（Version: %s.%d） ^ ^"), LOG_STR(QString(VERSION)), BUILED);
}

/************************************************************************/
/* 系统关闭处理                                                         */
/************************************************************************/
void NwfdMainApp::Close()
{
	// 关闭TCP Server
	try
	{
		if (m_tcpServer)
		{
			m_tcpServer->close();
			delete m_tcpServer;  // ?
			m_tcpServer = NULL;
		}
	}
	catch (...)
	{
		LOG_(LOGID_ERROR, LOG_F("关闭TCP Server时发生异常."));
	}

	// 停止任务调度管理器
	if (m_pTaskManager)
	{
		LOG_(LOGID_DEBUG, LOG_F("停止任务调度管理器."));
		m_pTaskManager->Stop();
		// 延时500毫秒
		delay(500);
	}

	// 停止关闭任务
	try
	{
		TaskBase * pTask = NULL;
		for (HASH_TASK::iterator iterTask = m_hasTask.begin(); iterTask != m_hasTask.end(); iterTask++)
		{
			// 删除创建的任务
			pTask = iterTask.value();
			if (pTask)
			{
				LOG_(LOGID_DEBUG, LOG_STR(QString("删除任务[TaskPID:%1 TaskName:%2]").arg(pTask->GetTaskPID()).arg(pTask->GetTaskName())));
				delete pTask;
				pTask = NULL;
			}
		}
		// 清空任务列表
		m_hasTask.clear();
	}
	catch (...)
	{
		LOG_(LOGID_ERROR, LOG_F("清空任务列表时发生异常."));
	}

	// log
	LOG_(LOGID_INFO, LOG_F("Nwfd服务程序结束运行 - -"));

	// 日志退出（call YAOLOG_EXIT before app exit ）
	YAOLOG_EXIT;
}

/************************************************************************/
/* 执行主处理                                                           */
/************************************************************************/ 
void NwfdMainApp::StartMain()
{
	/* 1、读取配置文件 */
 	LOG_(LOGID_INFO, LOG_F("开始读取配置."));
	m_strRoot = XmlConfig::getClass().GetRoot();   // 数据存储根目录
	m_strCccc = XmlConfig::getClass().GetCCCC();   // CCCC4位省编码
	m_fLon1 = XmlConfig::getClass().GetLon1();     // 起始经度
	m_fLon2 = XmlConfig::getClass().GetLon2();     // 终止经度
	m_fLat1 = XmlConfig::getClass().GetLat1();     // 起始纬度
	m_fLat2 = XmlConfig::getClass().GetLat2();     // 终止纬度
	m_stCheckup = XmlConfig::getClass().GetCheckupCfg();   // 配置检测
	m_stDownload = XmlConfig::getClass().GetDownLoadCfg(); // 下载配置
	m_stUpload = XmlConfig::getClass().GetUpLoadCfg();     // 上传配置
	m_stSysClean = XmlConfig::getClass().GetSysClean();    // 系统清理配置
	m_hasProduct = XmlConfig::getClass().GetProductList(); // 产品信息列表


	m_stTcpServer = XmlConfig::getClass().GetTcpServer();
	m_stCached = XmlConfig::getClass().GetCachedConfig();
	m_stDDSCfg = XmlConfig::getClass().GetDDSConfig();
	m_stConnPool = XmlConfig::getClass().GetConnPool();
	m_hasClient = XmlConfig::getClass().GetClientList();
	m_lstStationCfg = XmlConfig::getClass().GetStationCfg();
	m_stStationMag = XmlConfig::getClass().GetStationMag();
	m_stHttp = XmlConfig::getClass().GetHttpCfg();
	bool bMonitor = XmlConfig::getClass().IsMonitor();

	/* 2、初始化任务调度管理 */
	LOG_(LOGID_INFO, LOG_F("开始创建调度任务."));
	m_pTaskManager = TaskManager::getInstance();

	/* 2、初始化网络服务 */ // Not start 
	InitNetworkService();

	/* 3、初始化数据库连接 */ // Not start 
	InitDataBase();

	/* 4、初始化数据分发服务 */ // Not start 
	InitDDS();
	
	/* 5、初始化http接收任务 */ // Not start 
	InitHttpRecver();

	///////////////////////////// start //////////////////////////
	/* 初始化配置检测任务 */
	InitCheckupTask();

	// log
	LOG_(LOGID_INFO, LOG_F("[设置] 本地存储根路径：%s ; 省中心编码：%s ; 裁剪经纬度范围：（%0.2f,%0.2f,%0.2f,%0.2f）."),
		LOG_STR(m_strRoot),LOG_STR(m_strCccc), m_fLon1, m_fLon2, m_fLat1, m_fLat2);

	/* 初始化下载任务 */
	InitDownLoadTask();

	/*初始化上传任务*/
	InitUpLoadTask();

	/* 6、初始化系统清理检查任务 */
	InitSysCheckup();

	/* 7、初始化产品任务  */
	InitProductTask();

	// 任务监控
	//if (bMonitor)
	//{
	//	InitTaskMonitor();
	//}

	/* 8 初始化Mask数据 */
	//CMask::getClass().InitMaskData();

	/* 9、初始化站点任务 */
	//InitStationTask();

	/* 10、初始化站点消息任务  */
	//InitStationMsgTask();

	/* 10、启动任务管理器，执行定时任务 */
	LOG_(LOGID_INFO, LOG_F("启动任务调度管理器TaskManager"));
	m_pTaskManager->start();
}

/************************************************************************/
/* 初始化网络服务                                                       */
/************************************************************************/
void NwfdMainApp::InitNetworkService()
{
	// 1. 判断功能是否禁用
	if (m_stTcpServer.bDisabled)
	{
		// 网络服务功能禁用
		LOG_(LOGID_INFO, LOG_F("[Notice]根据配置，网络服务功能禁用."));
		return;
	}

	int nTaskPID;
	int nTaskSchedulID;
	bool bTaskFlg = true;
	TaskTcpServer *pTaskTcpServer = NULL;

	// 3.1 创建任务  
	pTaskTcpServer = new TaskTcpServer();
	if (pTaskTcpServer)
	{
		nTaskPID = m_pTaskManager->GetTaskPID();
		LOG_(LOGID_INFO, LOG_F("网络任务[TaskPID=%d]：%s"), nTaskPID, LOG_STR(pTaskTcpServer->GetTaskName()));

		// 3.2 初始化任务
		pTaskTcpServer->SetTaskPID(nTaskPID);
		pTaskTcpServer->Init(m_stTcpServer.nListenPort, m_stTcpServer.nMaxConnection);

		// 3.3 将任务添加到任务调度管理器中
		if (m_pTaskManager->AddTask(nTaskPID, pTaskTcpServer))
		{
			LOG_(LOGID_DEBUG, LOG_F("（TaskPID:%d）TCP服务任务已添加到TaskManager中."), nTaskPID);
		}
		else
		{
			// 如果添加失败
			LOG_(LOGID_DEBUG, LOG_F("（TaskPID:%d）TCP服务任务向TaskManager中添加任务失败."), nTaskPID);
			bTaskFlg = false;
		}

		// 3.4 创建任务调度
		nTaskSchedulID = m_pTaskManager->GetTaskSchedulID();
		// 向任务调度管理器中添加调度(一次性任务，5秒后执行)
		if (m_pTaskManager->AddTaskSchedul(nTaskSchedulID, nTaskPID, 0, false, 5, false))
		{
			LOG_(LOGID_DEBUG, LOG_F("（TaskPID:%d;SchedulID:%d）任务调度添加成功."), nTaskPID, nTaskSchedulID);
		}
		else
		{
			LOG_(LOGID_DEBUG, LOG_F("（TaskPID:%d;SchedulID:%d）任务调度添加失败."), nTaskPID, nTaskSchedulID);
			bTaskFlg = false;
		}
		if (bTaskFlg)
		{
			LOG_(LOGID_INFO, LOG_F("（TaskPID:%d）添加任务调度成功."), nTaskPID);
		}
		else
		{
			LOG_(LOGID_INFO, LOG_F("（TaskPID:%d）添加任务调度失败."), nTaskPID);
		}
		// 3.5 添加任务到列表中
		m_hasTask.insert(nTaskPID, pTaskTcpServer);
	}
	else
	{
		LOG_(LOGID_ERROR, LOG_F("new TaskTcpServer() 返回为 NULL"));
		LOG_(LOGID_INFO, LOG_F("[ERROR] TCP服务任务创建失败"));
	}

	// 3. 创建网络服务管理器
	m_pNetworkManager = &NetworkManager::getClass();

	// 3.1 初始化，添加管理客户端信息
	m_pNetworkManager->Init(m_hasProduct, m_hasClient, m_stCached);
	m_pNetworkManager->SetTcpServer(pTaskTcpServer);
	m_pNetworkManager->SetIsPublish(XmlConfig::getClass().IsPublish());

}

/************************************************************************/
/* 初始化数据库连接                                                     */
/************************************************************************/
void NwfdMainApp::InitDataBase()
{
	// 1. 判断功能是否禁用
	if (m_stConnPool.bDisabled)
	{
		// 数据库连接功能禁用
		LOG_(LOGID_INFO, LOG_F("[Init]根据配置，数据库库功能禁用."));
		return;
	}

	// 2. 创建数据库连接池
	m_pDBConnPool = CDBConnPool::GetInstance();

	// 2.1 初始化数据库连接
	m_pDBConnPool->Init(m_stConnPool.stDataBase, m_stConnPool.nMinConnectNum, m_stConnPool.nMaxConnectNum);

	// 2.2 启动连接池，创建数据库连接
	m_pDBConnPool->start();

	LOG_(LOGID_INFO, LOG_F("[Init]数据库功能启动."));

	// 延时等待2000mes
	delay(2000);

	// 初始化产品信息
	//int nErr = HandleDB::getClass().InitProduct(m_hasProduct);
	//if (nErr == SUCCESS)
	//{
	//	LOG_(LOGID_INFO, LOG_F("[Init]初始化数据库产品信息成功."));
	//}
	//else
	//{
	//	LOG_(LOGID_ERROR, LOG_F("[Init]初始化数据库产品信息失败(err=%d)."), nErr);
	//}

}

/************************************************************************/
/* Http接收任务                                                         */
/************************************************************************/
void  NwfdMainApp::InitHttpRecver()
{
	if (m_stHttp.bDisabled)
	{
		LOG_(LOGID_INFO, LOG_F("[Init]根据配置，Http功能禁用."));
		return;
	}

	int nTaskPID;
	bool bTaskFlg;
	HttpRecver *pTaskHttpRecver = NULL;
	bTaskFlg = true;

	pTaskHttpRecver = new HttpRecver();
	if (pTaskHttpRecver == NULL)
	{
		// 创建失败
		LOG_(LOGID_ERROR, LOG_F("new HttpRecver() 返回为 NULL"));
		LOG_(LOGID_INFO, LOG_F("[ERROR] HttpRecver任务创建失败"));
		return;
	}
	
	if (!pTaskHttpRecver->isValid())
	{
		LOG_(LOGID_ERROR, LOG_F("LibCurl 初始化失败"));
		LOG_(LOGID_INFO, LOG_F("[ERROR] HttpRecver任务创建失败"));
		return;
	}

	nTaskPID = m_pTaskManager->GetTaskPID();

	LOG_(LOGID_INFO, LOG_F("任务[TaskPID=%d]：%s"), nTaskPID, LOG_STR(pTaskHttpRecver->GetTaskName()));

	pTaskHttpRecver->SetTaskPID(nTaskPID);
	//pTaskHttpRecver->parseProductDownPath(m_stHttp.url);
	pTaskHttpRecver->initDownLoadReportTimes(m_stHttp.reportTimes);
	pTaskHttpRecver->setXmlUrl(m_stHttp.url);
	pTaskHttpRecver->setDownLoadRootPath(m_stHttp.savePath);

	if (AddTaskManager(pTaskHttpRecver, m_stHttp.scanTime, false))
	{
		bTaskFlg = true;
	}
	else
	{
		
		bTaskFlg = false;
	}
	

	if (bTaskFlg)
	{
		LOG_(LOGID_INFO, LOG_F("（TaskPID:%d）添加任务调度成功."), nTaskPID);
	}
	else
	{
		LOG_(LOGID_INFO, LOG_F("（TaskPID:%d）添加任务调度失败."), nTaskPID);
	}

	// 3.5 添加任务到列表中
	m_hasTask.insert(nTaskPID, pTaskHttpRecver);
}

/************************************************************************/
/* 初始化数据分发服务                                                   */
/************************************************************************/
void NwfdMainApp::InitDDS()
{
	// 1. 判断功能是否禁用
	if (m_stDDSCfg.bDisabled)
	{
		// 数据库连接功能禁用
		LOG_(LOGID_INFO, LOG_F("[Init]根据配置，数据分发功能禁用."));
		return;
	}
	
	// 2. 创建数据分发服务管理器
	m_pDDSManager = &CDDSManager::getClass();
	m_pDDSManager->Init(m_stDDSCfg.hasDDS);

	// 2.2 启动数据分发管理器
	m_pDDSManager->start();

	// 3. 创建数据分发处理任务
	int nTaskPID;
	int nTaskSchedulID;
	bool bTaskFlg;
	TaskDDS *pTaskDDS = NULL;
	bTaskFlg = true;

	// 3.1 创建任务
	pTaskDDS = new TaskDDS();
	if (pTaskDDS == NULL)
	{
		// 创建失败
		LOG_(LOGID_ERROR, LOG_F("[Init] 数据分发任务创建失败"));
		return;
	}

	nTaskPID = m_pTaskManager->GetTaskPID();
	LOG_(LOGID_INFO, LOG_F("任务[TaskPID=%d]：%s"), nTaskPID, LOG_STR(pTaskDDS->GetTaskName()));

	// 3.2 初始化任务
	pTaskDDS->SetTaskPID(nTaskPID);
	pTaskDDS->Init(m_stDDSCfg.hasDDS);

	// 将任务添加到数据分发服务管理器中
	m_pDDSManager->SetDDSTask(pTaskDDS);

	// 3.3 将任务添加到任务调度管理器中
	if (m_pTaskManager->AddTask(nTaskPID, pTaskDDS))
	{
		LOG_(LOGID_DEBUG, LOG_F("（TaskPID:%d）数据分发任务已添加到TaskManager中."), nTaskPID);
	}
	else
	{
		LOG_(LOGID_DEBUG, LOG_F("（TaskPID:%d）数据分发任务向TaskManager中添加任务失败."), nTaskPID);
		bTaskFlg = false;
	}

	// 3.4 创建任务调度
	nTaskSchedulID = m_pTaskManager->GetTaskSchedulID();

	// 向任务调度管理器中添加调度
	if (m_pTaskManager->AddTaskSchedul(nTaskSchedulID, nTaskPID, 0, true, m_stDDSCfg.nInterval, false))
	{
		LOG_(LOGID_DEBUG, LOG_F("（TaskPID:%d;SchedulID:%d）任务调度添加成功."), nTaskPID, nTaskSchedulID);
	}
	else
	{
		LOG_(LOGID_DEBUG, LOG_F("（TaskPID:%d;SchedulID:%d）任务调度添加失败."), nTaskPID, nTaskSchedulID);
		bTaskFlg = false;
	}

	if (bTaskFlg)
	{
		LOG_(LOGID_INFO, LOG_F("（TaskPID:%d）数据分发任务,添加任务调度成功."), nTaskPID);
	}
	else
	{
		LOG_(LOGID_ERROR, LOG_F("（TaskPID:%d）数据分发任务,添加任务调度失败."), nTaskPID);
	}

	// 3.5 添加任务到列表中
	m_hasTask.insert(nTaskPID, pTaskDDS);
}

/************************************************************************/
/* 初始化缓存维护服务                                                   */
/************************************************************************/
void NwfdMainApp::InitSysCheckup()
{
	// 1. 判断功能是否禁用
	if (m_stSysClean.bDisabled)
	{
		// 数据库连接功能禁用
		LOG_(LOGID_INFO, LOG_F("[Init]根据配置，系统清理维护功能禁用."));
		return;
	}

	// 2. 创建缓存维护任务
	int nTaskPID;
	int nTaskSchedulID;
	bool bTaskFlg;
	TaskSysClean *pTaskSysClean = NULL;
	bTaskFlg = true;

	// 2.1 创建任务
	pTaskSysClean = new TaskSysClean();
	if (pTaskSysClean == NULL)
	{
		// 创建缓存维护任务失败
		LOG_(LOGID_ERROR, LOG_F("[ERROR] 系统清理维护任务创建失败"));
		return;
	}

	nTaskPID = m_pTaskManager->GetTaskPID();
	LOG_(LOGID_INFO, LOG_F("任务[TaskPID=%d]：%s"), nTaskPID, LOG_STR(pTaskSysClean->GetTaskName()));

	// 2.2 初始化任务
	pTaskSysClean->SetTaskPID(nTaskPID);
	pTaskSysClean->Init(m_stCached, m_stSysClean, m_hasProduct, m_hasClient, m_lstStationCfg, m_stStationMag);
	// 设置
	pTaskSysClean->setRootPath(m_strRoot);
	pTaskSysClean->setCCCC(m_strCccc);

	// 2.3 将任务添加到任务调度管理器中
	if (m_pTaskManager->AddTask(nTaskPID, pTaskSysClean))
	{
		LOG_(LOGID_DEBUG, LOG_F("（TaskPID:%d）缓存维护任务已添加到TaskManager中."), nTaskPID);
	}
	else
	{
		LOG_(LOGID_DEBUG, LOG_F("（TaskPID:%d）缓存维护任务向TaskManager中添加任务失败."), nTaskPID);
		bTaskFlg = false;
	}

	// 2.4 创建任务调度
	nTaskSchedulID = m_pTaskManager->GetTaskSchedulID();

	// 向任务调度管理器中添加调度-首次不执行
	if (m_pTaskManager->AddTaskSchedul(nTaskSchedulID, nTaskPID, 0, true, m_stSysClean.nInterval, false))
	{
		LOG_(LOGID_DEBUG, LOG_F("（TaskPID:%d;SchedulID:%d）任务调度添加成功."), nTaskPID, nTaskSchedulID);
	}
	else
	{
		LOG_(LOGID_DEBUG, LOG_F("（TaskPID:%d;SchedulID:%d）任务调度添加失败."), nTaskPID, nTaskSchedulID);
		bTaskFlg = false;
	}

	if (bTaskFlg)
	{
		LOG_(LOGID_INFO, LOG_F("[TaskPID:%d]系统清理维护任务，添加任务调度成功."), nTaskPID);
	}
	else
	{
		LOG_(LOGID_ERROR, LOG_F("[TaskPID:%d]系统清理维护任务，添加任务调度失败."), nTaskPID);
	}

	// 2.5 添加任务到列表中
	m_hasTask.insert(nTaskPID, pTaskSysClean);

	// 添加到配置检查关联表中
	if (m_pTaskCheckup)
	{
		m_pTaskCheckup->AddTask(pTaskSysClean);
	}
}

/************************************************************************/
/* 初始化产品任务                                                       */
/************************************************************************/
void NwfdMainApp::InitProductTask()
{
	int nTaskPID;

	HASH_PRODUCT::iterator iter;
	for (iter = m_hasProduct.begin(); iter != m_hasProduct.end(); iter++)
	{
		ST_PRODUCT stProduct = *iter;

		// 5.1 产品扫描存储处理业务
		if (stProduct.strRole.toLower() == "raw")
		{
			// 5.1.1 创建任务
			TaskProductRaw *pTask = new TaskProductRaw();
			if (pTask == NULL)
			{
				// 创建任务失败
				LOG_(LOGID_ERROR, LOG_F("[ERROR] 产品扫描任务创建失败（key：%s）."), LOG_STR(stProduct.strKey));

				continue;
			}

			nTaskPID = m_pTaskManager->GetTaskPID();
			LOG_(LOGID_INFO, LOG_F("产品扫描任务[TaskPID=%d]：(Key=%s)"), nTaskPID, LOG_STR(stProduct.strKey));

			// 5.1.2 初始化任务
			pTask->SetTaskPID(nTaskPID);
			pTask->Init(stProduct, m_stCached);
			pTask->setRootPath(m_strRoot);
			pTask->setCCCC(m_strCccc);
			pTask->setGridRange(m_fLon1, m_fLon2, m_fLat1, m_fLat2);

			// 5.1.3 添加任务和调度
			if (AddTaskManager(pTask, stProduct.stScanTime))
			{
				LOG_(LOGID_INFO, LOG_F("[TaskPID:%d]产品扫描任务(Key=%s)，添加任务调度成功"), nTaskPID, LOG_STR(stProduct.strKey));
			}
			else
			{
				LOG_(LOGID_ERROR, LOG_F("[TaskPID:%d]产品扫描任务(Key=%s)，添加任务调度失败"), nTaskPID, LOG_STR(stProduct.strKey));
			}

			// 5.1.4 将此任务添加到任务列表中
			m_hasTask.insert(nTaskPID, pTask);

			// 添加到配置检查关联表中
			if (m_pTaskCheckup)
			{
				m_pTaskCheckup->AddTask(pTask);
			}

			// 添加到产品任务管理中
			ProductTaskManager::getClass().AddTask(stProduct.strType, stProduct.nTimeRange, pTask);
		}
		else if (stProduct.strRole.toLower() == "merge")
		{
			// 数据合并任务
			TaskProductMerge *pTask = new TaskProductMerge();
			if (pTask == NULL)
			{
				// 创建任务失败
				LOG_(LOGID_ERROR, LOG_F("[ERROR] 产品拼图任务创建失败（key：%s）."), LOG_STR(stProduct.strKey));

				continue;
			}

			// 添加定时扫描任务-添加任务和调度
			nTaskPID = m_pTaskManager->GetTaskPID();
			LOG_(LOGID_INFO, LOG_F("产品拼图任务[TaskPID=%d]：(Key=%s: %s)"), nTaskPID, LOG_STR(stProduct.strKey),LOG_STR(stProduct.strRelatedKey));
			
			// 初始化任务：注 数据合并任务的启动由网络服务管理器控制
			pTask->SetTaskPID(nTaskPID);
			pTask->Init(stProduct, m_stCached);

			// 添加Mask文件到Mask管理中
			CMask::getClass().AddMaskInfo(stProduct.stMask);

			if (AddTaskManager(pTask, stProduct.stScanTime,false))
			{
				LOG_(LOGID_INFO, LOG_F("[TaskPID:%d]产品拼图任务(Key=%s: %s)，添加任务调度成功"), nTaskPID, LOG_STR(stProduct.strKey), LOG_STR(stProduct.strRelatedKey));
			}
			else
			{
				LOG_(LOGID_ERROR, LOG_F("[TaskPID:%d]产品拼图任务(Key=%s: %s)，添加任务调度失败"), nTaskPID, LOG_STR(stProduct.strKey), LOG_STR(stProduct.strRelatedKey));
			}

			// 将此任务添加到任务列表中
			m_hasTask.insert(nTaskPID, pTask);

			if (m_pWebMonitorTask)
			{
				m_pWebMonitorTask->AddTask(stProduct.strRelatedKey, pTask);
			}
			
			//////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			// 创建用于客户端上传数据合并的任务
			TaskProductClientMerge *pClientTask = new TaskProductClientMerge();
			if (pClientTask == NULL)
			{
				// 创建任务失败
				LOG_(LOGID_ERROR, LOG_F("客户端上传数据合并任务创建失败（key：%s）."), LOG_STR(stProduct.strKey));

				continue;
			}

			// 初始化任务：注 客户端上传数据合并任务的启动由网络服务管理器控制，非定时器控制
			nTaskPID = m_pTaskManager->GetTaskPID();
			pClientTask->SetTaskPID(nTaskPID);
			pClientTask->Init(stProduct, m_stCached);

			// 添加到网络服务管理器中
			if (m_pNetworkManager)
			{
				// 添加任务
				m_pNetworkManager->AddTask(stProduct.strRelatedKey, pClientTask);
			}

			// 将此任务添加到任务列表中
			m_hasTask.insert(nTaskPID, pClientTask);
		}
		else
		{
			// todo  暂未处理
			LOG_(LOGID_INFO, LOG_F("[WARNING][Key:%s] %s 类型的业务暂未处理，无法为此创建任务"), LOG_STR(stProduct.strKey), LOG_STR(stProduct.strRole));
		}
	}
	
}


/************************************************************************/
/* 初始化任务监控任务                                                      */
/************************************************************************/
void NwfdMainApp::InitTaskMonitor()
{
	int nTaskPID;

	m_pWebMonitorTask = new TaskWebMonitor();
	if (m_pWebMonitorTask == NULL)
	{
		// 创建任务失败
		LOG_(LOGID_ERROR, LOG_F("[ERROR] 任务监控任务创建失败."));
		return;
	}

	nTaskPID = m_pTaskManager->GetTaskPID();
	LOG_(LOGID_INFO, LOG_F("任务监控任务[TaskPID=%d]"), nTaskPID);

	m_pWebMonitorTask->SetTaskPID(nTaskPID);

	//添加WebMonitorTask到任务管理器，每5秒执行一次
	ST_SCANTIME scanTime;
	scanTime.nInterval = 5;
	if (AddTaskManager(m_pWebMonitorTask, scanTime, true))
	{
		LOG_(LOGID_INFO, LOG_F("[TaskPID:%d]产品拼图任务，添加任务调度成功"), nTaskPID);
	}
	else
	{
		LOG_(LOGID_ERROR, LOG_F("[TaskPID:%d]产品拼图任务，添加任务调度失败"), nTaskPID);
	}

	// 将此任务添加到任务列表中
	m_hasTask.insert(nTaskPID, m_pWebMonitorTask);
}

/************************************************************************/
/* 初始化站点任务                                                       */
/************************************************************************/
void NwfdMainApp::InitStationTask()
{
	// 站点管理,用于站点-格点同步
	if (!m_stStationMag.bDisabled)
	{
		DataManager::getClass().Init(m_hasProduct, m_stCached, m_stStationMag);
		LOG_(LOGID_INFO, LOG_F("[Init]格点-站点同步管理功能启用."));
	}
	else
	{
		LOG_(LOGID_INFO, LOG_F("[Init]根据配置，格点-站点同步管理功能禁用."));
	}

	// 1. 判断功能是否禁用
	if (m_lstStationCfg.size() == 0)
	{
		// 没有站点配置信息，则不进行处理
		LOG_(LOGID_INFO, LOG_F("[Init]根据配置，站点处理功能禁用."));
		return;
	}

	// 2. 创建任务
	int nTaskPID;


	LIST_STATION_CFG::iterator iter;
	for (iter = m_lstStationCfg.begin(); iter != m_lstStationCfg.end(); iter++)
	{
		ST_STATION_CFG stStationCfg = *iter;

		// 2.1 创建任务
		if (stStationCfg.strRole == "raw")
		{
			TaskStationRaw* pTask = new TaskStationRaw();

			if (pTask == NULL)
			{
				// 创建任务失败
				LOG_(LOGID_ERROR, LOG_F("[ERROR] 创建站点扫描任务失败."));

				return;
			}

			nTaskPID = m_pTaskManager->GetTaskPID();
			LOG_(LOGID_INFO, LOG_F("任务[TaskPID=%d]：站点处理任务"), nTaskPID);

			// 2.2 初始化任务
			pTask->SetTaskPID(nTaskPID);
			pTask->Init(stStationCfg, m_stCached);

			// 2.3 添加任务和调度
			if (AddTaskManager(pTask, stStationCfg.stScanTime))
			{
				LOG_(LOGID_INFO, LOG_F("[TaskPID:%d]站点扫描任务，添加任务调度成功"), nTaskPID);
			}
			else
			{
				LOG_(LOGID_ERROR, LOG_F("[TaskPID:%d]站点扫描任务，添加任务调度失败"), nTaskPID);
			}

			// 2.4 将此任务添加到任务列表中
			m_hasTask.insert(nTaskPID, pTask);
		}

		else if (stStationCfg.strRole == "merge")
		{
			TaskStationMerge *pTask = new TaskStationMerge();

			if (pTask == NULL)
			{
				// 创建任务失败
				LOG_(LOGID_ERROR, LOG_F("[ERROR] 创建站点合并任务失败."));

				return;
			}

			nTaskPID = m_pTaskManager->GetTaskPID();
			LOG_(LOGID_INFO, LOG_F("任务[TaskPID=%d]：站点合并处理任务"), nTaskPID);

			// 2.2 初始化任务
			pTask->SetTaskPID(nTaskPID);
			pTask->Init(stStationCfg, m_stCached);

			// 2.3 添加任务和调度
			if (AddTaskManager(pTask, stStationCfg.stScanTime, false))
			{
				LOG_(LOGID_INFO, LOG_F("[TaskPID:%d]站点合并处理任务,添加任务调度成功"), nTaskPID);
			}
			else
			{
				LOG_(LOGID_ERROR, LOG_F("[TaskPID:%d]站点合并处理任务,添加任务调度失败"), nTaskPID);
			}

			// 2.4 将此任务添加到任务列表中
			m_hasTask.insert(nTaskPID, pTask);
		}
		else
		{
			continue;
		}

	}
}

/************************************************************************/
/* 初始化站点消息任务                                                   */
/************************************************************************/
void NwfdMainApp::InitStationMsgTask()
{
	int nTaskPID;

	// 1.创建站点消息处理任务
	TaskStationMsg* pTask = new TaskStationMsg();
	if (pTask == NULL)
	{
		// 创建任务失败
		LOG_(LOGID_ERROR, LOG_F("[ERROR] 创建站点消息任务失败."));

		return;
	}

	nTaskPID = m_pTaskManager->GetTaskPID();
	LOG_(LOGID_INFO, LOG_F("任务[TaskPID=%d]：站点消息处理任务"), nTaskPID);

	// 2. 初始化任务
	pTask->SetTaskPID(nTaskPID);
	pTask->Init(m_stCached);

	// 3. 将此任务添加到网格管理器中
	NetworkManager::getClass().SetStationMsg(pTask);

	// 4. 将此任务添加到任务列表中
	m_hasTask.insert(nTaskPID, pTask);
}

/************************************************************************/
/* 添加任务和调度                                                       */
/************************************************************************/
bool NwfdMainApp::AddTaskManager(TaskBase * pTask, ST_SCANTIME stScanTime, bool bFirstExecFlg)
{
	int nTaskPID = pTask->GetTaskPID();
	int nTaskSchedulID;
	bool bTaskFlg = true;

	// 将任务添加到任务调度管理器中
	if (m_pTaskManager->AddTask(nTaskPID, pTask))
	{
		LOG_(LOGID_DEBUG, LOG_F("（TaskPID:%d）任务已添加到TaskManager中."), nTaskPID);
	}
	else
	{
		LOG_(LOGID_DEBUG, LOG_F("（TaskPID:%d）向TaskManager中添加任务失败."), nTaskPID);

		return false;
	}

	// 创建任务调度
	int  nBeginTime;
	int  nInterval;
	bool bFirstExec;

	LOG_(LOGID_DEBUG, LOG_F("（TaskPID:%d）添加任务调度."), nTaskPID);
	LIST_PROTIME::iterator iter;
	for (iter = stScanTime.lstProTime.begin(); iter != stScanTime.lstProTime.end(); iter++)
	{
		ST_PROTIME stProTime = *iter;

		// 是否首次执行
		if (!bFirstExecFlg)
		{
			bFirstExec = false;
		}
		else
		{
			bFirstExec = (iter == stScanTime.lstProTime.begin()) ? true : false;
		}
		
		// 计算执行时间
		nBeginTime = GetBeginTime(stProTime.nHour, stProTime.nMin, stProTime.nSecond);

		// 时间间隔
		if (stProTime.nInterval <= 0)
		{
			// 如果配置获得的间隔时间小于等于0，则默认间隔为1天
			nInterval = 24 * 60 * 60;
		}
		else
		{
			nInterval = stProTime.nInterval;
		}

		// 任务调度管理ID
		nTaskSchedulID = m_pTaskManager->GetTaskSchedulID();

		// 向任务调度管理器中添加调度
		if (m_pTaskManager->AddTaskSchedul(nTaskSchedulID, nTaskPID, nBeginTime, true, nInterval, bFirstExec))
		{
			LOG_(LOGID_DEBUG, LOG_F("任务调度添加成功（BeginTime=%d(%02d:%02d:%02d);Interval=%d）."), nBeginTime, stProTime.nHour, stProTime.nMin, stProTime.nSecond, nInterval);
		}
		else
		{
			LOG_(LOGID_DEBUG, LOG_F("任务调度添加失败（BeginTime=%d(%02d:%02d:%02d);Interval=%d）."), nBeginTime, stProTime.nHour, stProTime.nMin, stProTime.nSecond, nInterval);

			bTaskFlg = false;
		}
	}
	// 当没有配置扫描时间列表时，执行扫描间隔处理
	if (stScanTime.lstProTime.size() == 0 && stScanTime.nInterval != 0)
	{
		// 是否首次执行
		bFirstExec = true;

		// 计算起始时间
		nBeginTime = 0;  // 0：添加调度时会自动根据执行间隔确定下次执行时间
 
		// 时间间隔
		nInterval = stScanTime.nInterval;

		// 任务调度管理ID
		nTaskSchedulID = m_pTaskManager->GetTaskSchedulID();

		// 向任务调度管理器中添加调度
		if (m_pTaskManager->AddTaskSchedul(nTaskSchedulID, nTaskPID, nBeginTime, true, nInterval, bFirstExec))
		{
			LOG_(LOGID_DEBUG, LOG_F("任务调度添加成功（BeginTime=%d;Interval=%d）."), nBeginTime, nInterval);
		}
		else
		{
			LOG_(LOGID_DEBUG, LOG_F("任务调度添加失败（BeginTime=%d;Interval=%d）."), nBeginTime, nInterval);

			bTaskFlg = false;
		}
	}

	return bTaskFlg;
}

/************************************************************************/
/* 获取任务开始执行时间                                                 */
/************************************************************************/
int NwfdMainApp::GetBeginTime(int nHour, int nMin, int nSecond)
{
	// 当前时间
	QDateTime tNow = QDateTime::currentDateTime();

	// 任务开始执行时间
	QString  strBeginTime = QString("").sprintf("%04d-%02d-%02d %02d:%02d:%02d", tNow.date().year(), tNow.date().month(), tNow.date().day(), nHour, nMin, nSecond);
	QDateTime tBegin = QDateTime::fromString(strBeginTime, "yyyy-MM-dd hh:mm:ss");

	// 时间判断
	if (tNow > tBegin)
	{
		// 当前时间已经超过执行时间，执行时间+1天
		tBegin = tBegin.addDays(1);
	}

	// 返回秒数
	return tBegin.toTime_t();
}

/************************************************************************/
/* 初始化下载任务                                                       */
/************************************************************************/
void NwfdMainApp::InitDownLoadTask()
{
	if (m_stDownload.bDisabled)
	{
		LOG_(LOGID_INFO, LOG_F("[Init]根据配置，DownLoad功能禁用."));
		return;
	}

	// 按照配置的Server个数创建下载任务
    for (int i = 0; i < m_stDownload.servers.size(); ++i)
    {
        ST_SERVER server = m_stDownload.servers.at(i);

		// 判断Server类型
		if (server.type.toLower() == "ftp")
        {
            int nTaskPID;

            TaskFtpRecver *pTaskFtpRecver = NULL;
            pTaskFtpRecver = new TaskFtpRecver();
            if (pTaskFtpRecver == NULL)
            {
                // 创建失败
				LOG_(LOGID_ERROR, LOG_F("FTP下载任务创建失败（%s）."), LOG_STR(server.ip));
                LOG_(LOGID_INFO, LOG_F("[ERROR] FtpRecver任务创建失败"));

                continue;
            }

			// 初始化
            pTaskFtpRecver->init(server);
            // 设置
            pTaskFtpRecver->setAutoDisable(m_stDownload.autoDisable);
            pTaskFtpRecver->setRootPath(m_strRoot);
            pTaskFtpRecver->setCCCC(m_strCccc);
			if (!server.source.isEmpty() && m_hasServer.contains(server.source))
			{
				ST_SERVER_CFG stServerCfg = m_hasServer.value(server.source);
				pTaskFtpRecver->setServer(stServerCfg.strServerIP, stServerCfg.strUserName, stServerCfg.strPassword);
			}

            nTaskPID = m_pTaskManager->GetTaskPID();

            LOG_(LOGID_INFO, LOG_F("任务[TaskPID=%d]：%s"), nTaskPID, LOG_STR(pTaskFtpRecver->GetTaskName()));

            pTaskFtpRecver->SetTaskPID(nTaskPID);

            if (AddTaskManager(pTaskFtpRecver, server.scantime, false))
            {
				LOG_(LOGID_INFO, LOG_F("（TaskPID:%d）添加任务调度成功."), nTaskPID);
            }
            else
            {
				LOG_(LOGID_INFO, LOG_F("（TaskPID:%d）添加任务调度失败."), nTaskPID);
            }

            // 3.5 添加任务到列表中
            m_hasTask.insert(nTaskPID, pTaskFtpRecver);

            // 添加到配置检查关联表中
            if (m_pTaskCheckup)
            {
                m_pTaskCheckup->AddTask(pTaskFtpRecver);
            }
        }
		else
		{
			// 其他下载类型不支持
			LOG_(LOGID_ERROR, LOG_F("不支持该类型的下载任务（%s）."), LOG_STR(server.type));
		}
    }
}
/************************************************************************/
/* 初始化上传任务                                                       */
/************************************************************************/
void NwfdMainApp::InitUpLoadTask()
{
	if (m_stUpload.bDisabled)
	{
		LOG_(LOGID_INFO, LOG_F("[Init]根据配置，UpLoad功能禁用."));
		return;
	}

	if (m_stUpload.type.toLower() == "ftp")
	{
		int nTaskPID;

		TaskFtpSender *pTaskFtpSender = NULL;

		pTaskFtpSender = new TaskFtpSender();
		if (pTaskFtpSender == NULL)
		{
			// 创建失败
			LOG_(LOGID_ERROR, LOG_F("new TaskFtpRecver() 返回为 NULL"));
			LOG_(LOGID_INFO, LOG_F("[ERROR] FtpRecver任务创建失败"));
			return;
		}

		pTaskFtpSender->init(m_stUpload);
		// 设置
		pTaskFtpSender->setRootPath(m_strRoot);
		pTaskFtpSender->setCCCC(m_strCccc);
		if (!m_stUpload.source.isEmpty() && m_hasServer.contains(m_stUpload.source))
		{
			ST_SERVER_CFG stServerCfg = m_hasServer.value(m_stUpload.source);
			pTaskFtpSender->setServer(stServerCfg.strServerIP, stServerCfg.strUserName, stServerCfg.strPassword);
		}

		nTaskPID = m_pTaskManager->GetTaskPID();

		LOG_(LOGID_INFO, LOG_F("任务[TaskPID=%d]：%s"), nTaskPID, LOG_STR(pTaskFtpSender->GetTaskName()));

		pTaskFtpSender->SetTaskPID(nTaskPID);


		if (AddTaskManager(pTaskFtpSender, m_stUpload.scantime, false))
		{
			LOG_(LOGID_INFO, LOG_F("（TaskPID:%d）添加任务调度成功."), nTaskPID);
		}
		else
		{
			LOG_(LOGID_INFO, LOG_F("（TaskPID:%d）添加任务调度失败."), nTaskPID);
		}

		// 3.5 添加任务到列表中
		m_hasTask.insert(nTaskPID, pTaskFtpSender);

		// 添加到配置检查关联表中
		if (m_pTaskCheckup)
		{
			m_pTaskCheckup->AddTask(pTaskFtpSender);
		}
	}
	else
	{
		//to do s
	}
}

/************************************************************************/
/* 初始化配置检测任务                                                   */
/************************************************************************/
void NwfdMainApp::InitCheckupTask()
{
	// 1. 判断功能是否禁用
	if (m_stCheckup.bDisabled)
	{
		LOG_(LOGID_INFO, LOG_F("[Init]根据配置，配置检测功能禁用."));
		return;
	}

	// 2. 创建配置检测任务
	int nTaskPID;
	int nTaskSchedulID;
	bool bTaskFlg;
	
	bTaskFlg = true;

	// 2.1 创建任务
	m_pTaskCheckup = new TaskCheckup();
	if (m_pTaskCheckup == NULL)
	{
		// 创建缓存维护任务失败
		LOG_(LOGID_ERROR, LOG_F("[ERROR] 配置检测任务创建失败"));
		return;
	}

	nTaskPID = m_pTaskManager->GetTaskPID();
	LOG_(LOGID_INFO, LOG_F("任务[TaskPID=%d]：%s"), nTaskPID, LOG_STR(m_pTaskCheckup->GetTaskName()));

	// 2.2 初始化任务
	m_pTaskCheckup->SetTaskPID(nTaskPID);
	m_pTaskCheckup->Init(m_stCheckup);
	// 获取与平台的交互配置
	QString strRoot, strCCCC;
	float fLon1, fLon2, fLat1, fLat2;
	int nErr = m_pTaskCheckup->GetConfigParam(strRoot, strCCCC, fLon1, fLon2, fLat1, fLat2, m_hasServer);
	// 更新最新的值
	if (nErr >= 0)
	{
		m_strRoot = strRoot;
		m_strCccc = strCCCC;
	}
	if (nErr == 0)
	{
		m_fLon1 = fLon1;
		m_fLon2 = fLon2;
		m_fLat1 = fLat1;
		m_fLat2 = fLat2;
	}

	// 2.3 将任务添加到任务调度管理器中
	if (m_pTaskManager->AddTask(nTaskPID, m_pTaskCheckup))
	{
		LOG_(LOGID_DEBUG, LOG_F("（TaskPID:%d）缓存维护任务已添加到TaskManager中."), nTaskPID);
	}
	else
	{
		LOG_(LOGID_DEBUG, LOG_F("（TaskPID:%d）配置检测任务向TaskManager中添加任务失败."), nTaskPID);
		bTaskFlg = false;
	}

	// 2.4 创建任务调度
	nTaskSchedulID = m_pTaskManager->GetTaskSchedulID();

	// 向任务调度管理器中添加调度-首次不执行
	if (m_pTaskManager->AddTaskSchedul(nTaskSchedulID, nTaskPID, 0, true, m_stCheckup.nInterval, false))
	{
		LOG_(LOGID_DEBUG, LOG_F("（TaskPID:%d;SchedulID:%d）任务调度添加成功."), nTaskPID, nTaskSchedulID);
	}
	else
	{
		LOG_(LOGID_DEBUG, LOG_F("（TaskPID:%d;SchedulID:%d）任务调度添加失败."), nTaskPID, nTaskSchedulID);
		bTaskFlg = false;
	}

	if (bTaskFlg)
	{
		LOG_(LOGID_INFO, LOG_F("[TaskPID:%d]配置检测任务，添加任务调度成功."), nTaskPID);
	}
	else
	{
		LOG_(LOGID_ERROR, LOG_F("[TaskPID:%d]配置检测任务，添加任务调度失败."), nTaskPID);
	}

	// 2.5 添加任务到列表中
	m_hasTask.insert(nTaskPID, m_pTaskCheckup);
}

