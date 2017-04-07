#include "log.h" // log日志
#include "XmlConfig.h"
#include <QStringList>

#include "HttpDef.h"
#include "FtpDef.h"

#pragma execution_character_set("utf-8")

/************************************************************************/
/* XML构造函数                                                          */
/************************************************************************/
XmlConfig::XmlConfig()
{
	LOG_(LOGID_DEBUG, LOG_F("Entry XmlConfig()"));

	// 默认初始化
	m_strRoot = "";     // 数据存储根目录
	m_strCccc = "";     // CCCC4位省编码
	m_fLon1 = -1;       // 起始经度
	m_fLon2 = -1;       // 终止经度
	m_fLat1 = -1;       // 起始纬度
	m_fLat2 = -1;       // 终止纬度

	// 读取配置
	if (ReadConfig())
	{
		LOG_(LOGID_INFO, LOG_F("[配置]读取成功"));
	}
	else
	{
		LOG_(LOGID_INFO, LOG_F("[配置]读取失败"));
	}
	LOG_(LOGID_DEBUG, LOG_F("Leave XmlConfig()"));
}

/************************************************************************/
/* 析构函数                                                             */
/************************************************************************/
XmlConfig::~XmlConfig()
{
	LOG_(LOGID_DEBUG, LOG_F("Entry ~XmlConfig()"));
	LOG_(LOGID_DEBUG, LOG_F("Leave ~XmlConfig()"));
}

/************************************************************************/
/* XML配置读取类单例调用方法                                            */
/************************************************************************/
XmlConfig & XmlConfig::getClass()
{
	static XmlConfig clazz;
	return clazz;
}

/************************************************************************/
/* 读取配置                                                             */
/************************************************************************/
bool XmlConfig::ReadConfig()
{
	// 读取基本配置信息
	int nNwfdSysErr = ReadNwfdSysConfig();

	// 读取客户端配置
	//int nClientErr = ReadClientConfig();

	// 读取格点配置信息
	int nProductErr = ReadProductConfig();

	// 读取站点配置信息
	//int nStationErr = ReadStationConfig();

	// 返回
	//return ((nNwfdSysErr + nClientErr + nProductErr + nStationErr) == 0);
	return ((nNwfdSysErr + nProductErr) == 0);
}

/************************************************************************/
/* 读取系统基本配置                                                     */
/************************************************************************/
int XmlConfig::ReadNwfdSysConfig()
{
	LOG_(LOGID_DEBUG, LOG_F("开始读取基本配置[%s]"), LOG_F(NWFDSYS_CONFIG_FILE));

	/* 1、打开配置文件，获取配置内容 */
	QFile file(NWFDSYS_CONFIG_FILE);
	if (!file.open(QIODevice::ReadOnly))
	{
		LOG_(LOGID_ERROR, LOG_F("[配置]配置文件%s打开失败"), LOG_F(NWFDSYS_CONFIG_FILE));
		return -1;
	}

	QDomDocument domDocument;
	if (!domDocument.setContent(&file))
	{
		LOG_(LOGID_ERROR, LOG_F("[配置]获取配置文件%s内容失败"), LOG_F(NWFDSYS_CONFIG_FILE));
		file.close();
		return -2;
	}
	file.close();

	QDomElement root = domDocument.documentElement();

	/* 2、默认配置 */
	m_strRoot = root.firstChildElement("root").text();
	m_strCccc = root.firstChildElement("cccc").text();
	m_fLon1 = root.firstChildElement("gridrange").attribute("lon1").toFloat();
	m_fLon2 = root.firstChildElement("gridrange").attribute("lon2").toFloat();
	m_fLat1 = root.firstChildElement("gridrange").attribute("lat1").toFloat();
	m_fLat2 = root.firstChildElement("gridrange").attribute("lat2").toFloat();

	/* 3、平台配置检测 */
	QDomElement checkup = root.firstChildElement("checkup");
	if (!checkup.isNull())
	{
		m_stCheckup.bDisabled = false;
		m_stCheckup.nInterval = checkup.attribute("interval").toInt();
		m_stCheckup.strPath = checkup.firstChildElement("cfg").attribute("path");
		m_stCheckup.strCfgFileName = checkup.firstChildElement("cfg").attribute("file");
	}
	else
	{
		m_stCheckup.bDisabled = true;
	}

	/* 4、下载相关配置 */
	QDomElement downloadEle = root.firstChildElement("download");
	if (!downloadEle.isNull())
	{
		m_stDownLoad.bDisabled = false;
		
		// 下载完成后是否自动调用解码作业
        QDomElement autoEle = downloadEle.firstChildElement("auto");
        if (!autoEle.isNull())
        {
			// 如果有，则靠disabled的属性控制关闭
			m_stDownLoad.autoDisable = autoEle.attribute("disabled").toLower() == "yes";
        }
        else
        {
			// 没有设置表示默认执行自动下载
            m_stDownLoad.autoDisable = false;
        }
        
        // 默认下载路径
        QDomNodeList servers = downloadEle.elementsByTagName("server");

        for (int i = 0; i < servers.size(); ++i)
        {
            QDomElement serverEle = servers.at(i).toElement();

            ST_SERVER server;

            server.type = serverEle.attribute("type");
			server.source = serverEle.attribute("source");
            server.ip = serverEle.attribute("ip");
            server.username = serverEle.attribute("user");
            server.password = serverEle.attribute("password");

            // 执行时间
            QDomElement scantimeEle = serverEle.firstChildElement("scantime");
            if (!scantimeEle.isNull())
            {
                server.scantime.nInterval = scantimeEle.attribute("interval").toInt();
            }

			// 全部下载
			QDomElement productall = serverEle.firstChildElement("all");
			if (!productall.isNull())
			{
				server.stDownloadAll.bDisabled = false;
				server.stDownloadAll.url = productall.attribute("url");
				server.stDownloadAll.filename = productall.attribute("filename");
				server.stDownloadAll.savepath = productall.attribute("savepath");
			}
			else
			{
				server.stDownloadAll.bDisabled = true;
			}

            // 下载产品列表
            QDomElement productsEle = serverEle.firstChildElement("products");
            if (!productsEle.isNull())
            {
                QDomElement productEle = productsEle.firstChildElement("product");
                while (!productEle.isNull())
                {
                    ST_DOWNLOADPROD product;
                    product.type = productEle.attribute("type");
					
                    product.range = productEle.attribute("range");
                    product.url = productEle.attribute("url");
                    product.filename = productEle.attribute("filename");
                    product.savepath = productEle.attribute("savepath");
                    product.savefilename = productEle.attribute("savefilename");
                    server.products.push_back(product);

                    productEle = productEle.nextSiblingElement();
                }
            }

            m_stDownLoad.servers.push_back(server);
        }
	}
	else
	{
		m_stDownLoad.bDisabled = true;
	}

	/* 5、上传相关配置 */
	QDomElement uploadEle = root.firstChildElement("upload");
	if (!uploadEle.isNull())
	{
		m_stUpLoad.bDisabled = false;

		m_stUpLoad.type = uploadEle.attribute("type");
		m_stUpLoad.source = uploadEle.attribute("source");

		m_stUpLoad.ip = uploadEle.attribute("ip");
		m_stUpLoad.username = uploadEle.attribute("user");
		m_stUpLoad.password = uploadEle.attribute("password");

		QDomElement logEle = uploadEle.firstChildElement("log");
		if (!logEle.isNull())
		{
			m_stUpLoad.log.path = logEle.attribute("path");
			m_stUpLoad.log.logfile = logEle.text();
		}

		QDomElement scantimeEle = uploadEle.firstChildElement("scantime");
		if (!scantimeEle.isNull())
		{
			m_stUpLoad.scantime.nInterval = scantimeEle.attribute("interval").toInt();
		}

		//send
		QDomElement sendEle = uploadEle.firstChildElement("send");
		if (!sendEle.isNull())
		{
			m_stUpLoad.send.src = sendEle.attribute("src");
			m_stUpLoad.send.url = sendEle.attribute("url");
			m_stUpLoad.send.filename = sendEle.attribute("filename");
		}
	}
	else
	{
		m_stUpLoad.bDisabled = true;
	}

	/* 6、 系统清理 */
	QDomElement clean = root.firstChildElement("clean");
	if (!clean.isNull())
	{
		m_stSysClean.bDisabled = clean.attribute("disabled").toLower() == "yes";
		m_stSysClean.nInterval = clean.attribute("interval").toInt();
		m_stSysClean.nLogKeepDay = clean.firstChildElement("log").text().toInt();
		m_stSysClean.nDBKeepDay = clean.firstChildElement("database").text().toInt();
		m_stSysClean.nFileKeepDay = clean.firstChildElement("file").text().toInt();
		QString strFileDir = clean.firstChildElement("file").attribute("dir");
		m_stSysClean.lstFileDir = strFileDir.split(";");  // 以；分割多个目录
	}
	else
	{
		m_stSysClean.bDisabled = true;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/* 2、TCPServer服务配置 */
	QDomElement listener = root.firstChildElement("listener");
	if (!listener.isNull())
	{
		m_stTcpServer.bDisabled = listener.attribute("disabled").toLower() == "yes";
		m_stTcpServer.nListenPort = listener.firstChildElement("port").text().toInt();
		m_stTcpServer.nMaxConnection = listener.firstChildElement("maxConnections").text().toInt();
	}
	else
	{
		m_stTcpServer.bDisabled = true;
	}
	LOG_(LOGID_DEBUG, LOG_F("[TcpServer] disabled=%d;监听端口=%d;最大连接数=%d"), (int)m_stTcpServer.bDisabled, m_stTcpServer.nListenPort, m_stTcpServer.nMaxConnection);

	/* 3、缓存服务配置 */
	QDomElement provider = root.firstChildElement("cached").firstChildElement("provider");
	if (!provider.isNull())
	{
		m_stCached.bDisabled = false;
		QString  strType = provider.attribute("type");
		if (strType.toLower() == "redis")
		{
			m_stCached.emType = _CACHED_REDIS;
		}
		else if (strType.toLower() == "memcached")
		{
			m_stCached.emType = _CACHED_MEMCACHED;
		}
		else
		{
			m_stCached.emType = _CACHED_UNKNOW;
		}
		QString strSvrAddr = provider.firstChildElement("address").text();
		memset(m_stCached.strSvrAddr, 0x00, sizeof(m_stCached.strSvrAddr));
		strcpy(m_stCached.strSvrAddr, strSvrAddr.toLocal8Bit().data());
		m_stCached.nPortNo = provider.firstChildElement("port").text().toInt();

		LOG_(LOGID_DEBUG, LOG_F("[Cached] 类型=%s;Server=%s:%d"), LOG_STR(strType), m_stCached.strSvrAddr, m_stCached.nPortNo);
	}
	else
	{
		m_stCached.bDisabled = true;  // 缓存连接功能禁止
	}

	/*http配置读取*/
	QDomElement http = root.firstChildElement("http");
	if (!http.isNull())
	{
		m_stHttp.bDisabled = http.attribute("disabled") == "yes";
		m_stHttp.scanTime.nInterval = http.attribute("interval").toInt();
		
		QDomElement download = http.firstChildElement("download");
		if (!download.isNull())
		{
			m_stHttp.url = download.attribute("url").toStdString();
			m_stHttp.savePath = download.attribute("savepath").toStdString();
		}

		/*读取扫描时间*/
		QDomElement scant = http.firstChildElement("scantime").firstChildElement("time");

		while (!scant.isNull())
		{
			ST_PROTIME time;
			time.nHour = scant.attribute("hour").toShort();
			time.nMin = scant.attribute("min").toShort();
			time.nSecond = scant.attribute("second").toShort();
			time.nInterval = scant.text().toInt();
			
			m_stHttp.scanTime.lstProTime.append(time);

				
			scant = scant.nextSiblingElement();
		}
 
		// download report time
		QDomElement download_report_time = http.firstChildElement("downloadreporttimes");

		if (!download_report_time.isNull())
		{
			QString reporttimes = download_report_time.text();

			if (!reporttimes.isEmpty())
			{
				QStringList list = reporttimes.split(QRegExp(","));

				for (int i = 0; i < list.size(); ++i)
				{
					m_stHttp.reportTimes.push_back(list.at(i).toStdString());
				}
			}
			else
			{
				m_stHttp.bDisabled = true;
			}			
		}
		else
		{
			m_stHttp.bDisabled = true;
		}
	}
	else
	{
		m_stHttp.bDisabled = true;
	}

	/* 4、数据库配置 */
	QDomElement database = root.firstChildElement("database");
	if (!database.isNull())
	{
		m_stConnPool.bDisabled =  database.attribute("disabled").toLower() == "yes";
		m_stConnPool.nMinConnectNum = database.attribute("minCon").toInt();
		m_stConnPool.nMaxConnectNum = database.attribute("maxCon").toInt();

		QDomElement db = database.firstChildElement("db");  // 数据库配置
		QString strDBType = db.attribute("type");
		if (strDBType == DB_SQLITE)  m_stConnPool.stDataBase.emDBType = SQLITE;
		else if (strDBType == DB_MYSQL)  m_stConnPool.stDataBase.emDBType = MYSQL;
		else if (strDBType == DB_SQLSERVER)  m_stConnPool.stDataBase.emDBType = SQLSERVER;
		else if (strDBType == DB_ORACLE)  m_stConnPool.stDataBase.emDBType = ORACLE;
		else m_stConnPool.stDataBase.emDBType = NODEF;

		m_stConnPool.stDataBase.strServer = db.attribute("address");
		m_stConnPool.stDataBase.nPort = db.attribute("port").toInt();
		m_stConnPool.stDataBase.strUser = db.attribute("username");
		m_stConnPool.stDataBase.strPassword = db.attribute("password");
		m_stConnPool.stDataBase.strDBName = db.attribute("databasename");
	}
	else
	{
		m_stConnPool.bDisabled = true;
	}
 
	/* 5、 数据分发配置       */
	QDomElement dds = root.firstChildElement("dds");
	if (!dds.isNull())
	{
		m_stDDSCfg.bDisabled = dds.attribute("disabled").toLower() == "yes";
		m_stDDSCfg.nInterval = dds.attribute("interval").toInt();  // 扫描时间间隔
		QDomElement ddsserver = dds.firstChildElement("server");
		while (!ddsserver.isNull())
		{
			ST_DDS  stDDS;
			stDDS.strID = ddsserver.attribute("id");   // 标识号
			stDDS.strDDSType = ddsserver.attribute("type").toLower();   // 类型
			stDDS.strRemoteIP = ddsserver.attribute("address");         // 服务器地址
			stDDS.strUserName = ddsserver.attribute("username");        // 用户名
			stDDS.strPassword = ddsserver.attribute("passwd");          // 密码
			stDDS.strRmotePath = ddsserver.attribute("path");           // ftp文件存储根目录

			// 添加文件-分发服务器对应到列表中
			m_stDDSCfg.hasDDS.insert(stDDS.strID, stDDS);

			// 下一条数据分发配置
			ddsserver = ddsserver.nextSiblingElement();
		}
	}
	else
	{
		m_stDDSCfg.bDisabled = true;
	}


	/* 7、 拼图任务监控 */
	QDomElement monitor = root.firstChildElement("monitor");
	if (monitor.isNull())
	{
		m_bMonitor = false;
	}
	else
	{
		m_bMonitor = (monitor.text().toLower() == "yes");
	}

	/* 8、 发布总开关 */
	QDomElement publish = root.firstChildElement("publish");
	if (publish.isNull())
	{
		m_bPublish = false;
	}
	else
	{
		m_bPublish = (publish.text().toLower() == "yes");
	}

	LOG_(LOGID_DEBUG, LOG_F("读取基本配置结束"));

	return 0;
}
/************************************************************************/
/* 读取客户端配置                                                       */
/************************************************************************/
int XmlConfig::ReadClientConfig()
{
	LOG_(LOGID_DEBUG, LOG_F("开始读取客户端配置[%s]"), LOG_F(CLIENT_CONFIG_FILE));

	/* 1、打开配置文件，获取配置内容 */
	QFile file(CLIENT_CONFIG_FILE);
	if (!file.open(QIODevice::ReadOnly))
	{
		LOG_(LOGID_ERROR, LOG_F("[配置]配置文件%s打开失败"), LOG_F(CLIENT_CONFIG_FILE));
		return -1;
	}

	QDomDocument domDocument;
	if (!domDocument.setContent(&file))
	{
		LOG_(LOGID_ERROR, LOG_F("[配置]获取配置文件%s内容失败"), LOG_F(CLIENT_CONFIG_FILE));
		file.close();
		return -2;
	}
	file.close();

	QDomElement root = domDocument.documentElement();

	/* 6、客户端配置 */
	QDomElement client = root.firstChildElement("clients").firstChildElement("client");
	while (!client.isNull())
	{
		ST_CLIENT_INFO stClient;
		stClient.nClientID = client.attribute("clientid").toInt();
		stClient.strName = client.attribute("name");
		stClient.strClientIP = client.attribute("ip");
		stClient.strUserName = client.attribute("username");
		stClient.strPassWord = client.attribute("password");
		stClient.bAllowUpload = client.attribute("upload").toLower() == "yes";

		QDomElement clientGrid = client.firstChildElement("grid");
		if (!clientGrid.isNull())
		{
			stClient.strGridSavePath = clientGrid.attribute("savepath");
			stClient.strGridSaveFile = clientGrid.attribute("filename");
		}
		QDomElement clientStation = client.firstChildElement("station");
		if (!clientStation.isNull())
		{
			stClient.strStationSavePath = clientStation.attribute("savepath");
			stClient.strStationSaveFile = clientStation.attribute("filename");
		}

		stClient.nCachedValidTime = client.attribute("cachedvalidtime").toInt();

		if (m_hasClient.contains(stClient.nClientID))
		{
			//  已经存在
			LOG_(LOGID_ERROR, LOG_F("[配置][ClientCfg] ClientID=%d已经存在，请确认配置是否有误"), stClient.nClientID);
		}
		else
		{
			// 插入客户端配置列表中
			m_hasClient.insert(stClient.nClientID, stClient);
		}

		// 下一条客户端配置
		client = client.nextSiblingElement();
	}


	LOG_(LOGID_DEBUG, LOG_F("读取客户端配置结束"));

	return 0;
}

/************************************************************************/
/* 读取系统格点产品配置                                                 */
/************************************************************************/
int XmlConfig::ReadProductConfig()
{
	LOG_(LOGID_DEBUG, LOG_F("开始读取格点产品配置[%s]"), LOG_F(PRODUCT_CONFIG_FILE));

	/* 1、打开配置文件，获取配置内容 */
	QFile file(PRODUCT_CONFIG_FILE);
	if (!file.open(QIODevice::ReadOnly))
	{
		LOG_(LOGID_ERROR, LOG_F("[配置]配置文件%s打开失败"), LOG_F(PRODUCT_CONFIG_FILE));
		return -1;
	}

	QDomDocument domDocument;
	if (!domDocument.setContent(&file))
	{
		LOG_(LOGID_ERROR, LOG_F("[配置]获取配置文件%s内容失败"), LOG_F(PRODUCT_CONFIG_FILE));
		file.close();
		return -2;
	}
	file.close();

	QDomElement root = domDocument.documentElement();

	/* 2.6 产品配置 */
	m_hasProduct.clear();
	QDomNode products = root.namedItem("products");
	QDomElement product = products.firstChildElement("product");
	while (!product.isNull())
	{
		ST_PRODUCT stProduct;

		// 属性内容-基本信息
		stProduct.strKey = product.attribute("key");
		stProduct.strName = product.attribute("name");
		stProduct.strRole = product.attribute("role");
		stProduct.strType = product.attribute("type");
		stProduct.strPublisher = product.attribute("publisher");

		QString strLogContent = QString("Key=%1;Name=%2;Role=%3;Type=%4;Publisher=%5").arg(stProduct.strKey).arg(stProduct.strName)
			.arg(stProduct.strRole).arg(stProduct.strType).arg(stProduct.strPublisher);
		LOG_(LOGID_DEBUG, LOG_F("[Product Key=%s] %s"), LOG_STR(stProduct.strKey), LOG_STR(strLogContent));

		// 属性内容-类型
		stProduct.nCnt = 0; // 个数
		stProduct.nCategory = product.attribute("category").toInt();
		QString strElement = product.attribute("element");
		stProduct.nElement[0] = -1;
		stProduct.nElement[1] = -1;
		QStringList lstElement = strElement.split(",");
		for (int i = 0; i < lstElement.size(); i++)
		{
			if (i >= 2)
			{
				break;
			}
			stProduct.nElement[i] = lstElement.at(i).toInt();

			// 个数根据element的配置进行判断
			stProduct.nCnt++;
		}
		stProduct.nStatistical = product.attribute("statistical").toInt();
		stProduct.nStatus = product.attribute("status").toInt();

		strLogContent = QString("Category=%1;Element=%2,%3;Statistical=%4;Status=%5;Cnt=%6").
			arg(stProduct.nCategory).arg(stProduct.nElement[0]).arg(stProduct.nElement[1]).
			arg(stProduct.nStatistical).arg(stProduct.nStatus).arg(stProduct.nCnt);
		LOG_(LOGID_DEBUG, LOG_F("[Product Key=%s] %s"), LOG_STR(stProduct.strKey), LOG_STR(strLogContent));

		// 属性内容-经纬度信息
		float fLon1 = product.attribute("lon1").toFloat();
		float fLon2 = product.attribute("lon2").toFloat();
		float fLat1 = product.attribute("lat1").toFloat();
		float fLat2 = product.attribute("lat2").toFloat();
		if (fLon2 > fLon1)
		{
			stProduct.fLon1 = fLon1;
			stProduct.fLon2 = fLon2;
		}
		else
		{
			stProduct.fLon1 = fLon2;
			stProduct.fLon2 = fLon1;
		}

		if (fLat2 > fLat1)
		{
			stProduct.fLat1 = fLat1;
			stProduct.fLat2 = fLat2;
		}
		else
		{
			stProduct.fLat1 = fLat2;
			stProduct.fLat2 = fLat1;
		}

		float fDi = product.attribute("di").toFloat();
		float fDj = product.attribute("dj").toFloat();
		if (fDi > 0) stProduct.fDi = fDi;
		else stProduct.fDi = fDi * -1;
		
		if (fDj > 0) stProduct.fDj = fDj;
		else stProduct.fDj = fDj * -1;

		// 经纬度格点数计算
		stProduct.nNi = (int)((stProduct.fLon2 - stProduct.fLon1) / stProduct.fDi + 0.1) + 1;
		stProduct.nNj = (int)((stProduct.fLat2 - stProduct.fLat1) / stProduct.fDj + 0.1) + 1;
		stProduct.fOffSet = product.attribute("offset").toFloat();
		stProduct.nTimeRange = product.attribute("TimeRange").toInt();
		stProduct.nMaxForecastTime = product.attribute("MaxForecastTime").toInt();
		stProduct.strBusinessStatus = product.attribute("BusinessStatus");
		stProduct.bIsWarning = product.attribute("IsWarning").toLower() == "yes";
		QString strMissing = product.attribute("missingval");
		stProduct.fMissingVal = strMissing.isEmpty() ? 9999.0 : strMissing.toFloat();  // 缺测
		stProduct.strLineVal = product.attribute("lineval");
		QStringList lstTmp = stProduct.strLineVal.split(" ");
		if (lstTmp.size() != 5)  stProduct.strLineVal = "";

		strLogContent = QString("Lon1=%1;Lon2=%2;Lat1=%3;Lat2=%4;Di=%5;Dj=%6;offset=%7").arg(stProduct.fLon1)
			.arg(stProduct.fLon2).arg(stProduct.fLat1).arg(stProduct.fLat2).arg(stProduct.fDi).arg(stProduct.fDj).arg(stProduct.fOffSet);
		LOG_(LOGID_DEBUG, LOG_F("[Product Key=%s] %s"), LOG_STR(stProduct.strKey), LOG_STR(strLogContent));

		// 扫描执行时间
		stProduct.stScanTime.nInterval = product.firstChildElement("scantime").attribute("interval").toInt();
		QDomElement protime = product.firstChildElement("scantime").firstChildElement("time");
		while (!protime.isNull())
		{
			ST_PROTIME  stProTime;
			stProTime.nHour = protime.attribute("hour").toInt();
			stProTime.nMin = protime.attribute("min").toInt();
			stProTime.nSecond = protime.attribute("second").toInt();
			stProTime.nInterval = protime.text().toInt();

			strLogContent = QString("扫描时间=%1:%2:%3; 间隔=%4秒").arg(stProTime.nHour).arg(stProTime.nMin).arg(stProTime.nSecond).
				arg(stProTime.nInterval);
			LOG_(LOGID_DEBUG, LOG_F("[Product Key=%s] %s"), LOG_STR(stProduct.strKey), LOG_STR(strLogContent));

			// 添加到产品的时间列表中
			stProduct.stScanTime.lstProTime.push_back(stProTime);

			// 下一条扫描时间
			protime = protime.nextSiblingElement();
		}
		// 扫描模式
		stProduct.strScanMode = product.firstChildElement("scanmode").text().toLower();
		// 扫描模式默认设置-没有配置默认1天
		//if (stProduct.strScanMode.isEmpty()) 
		//	stProduct.strScanMode = "day";

		// 解码模式
		stProduct.strDecMode = product.firstChildElement("decmode").text().toLower();

		// raw处理特有
		if (stProduct.strRole == "raw")
		{
			// 扫描路径&文件格式
			stProduct.strSrcFolder = product.firstChildElement("srcfolder").text();
			stProduct.strSrcFileName = product.firstChildElement("srcfileformat").text();
			// 元文件名格式解析
			GetFileNameFormat(stProduct.strSrcFileName, stProduct.stFileNameFormat);
			LOG_(LOGID_DEBUG, LOG_F("[Product Key=%s] 产品目录：%s;文件格式：%s"), LOG_STR(stProduct.strKey), LOG_STR(stProduct.strSrcFolder), LOG_STR(stProduct.strSrcFileName));
		}

		// 预报配置-起报时间-文件个数-时间间隔-文件合并最后生成时间
		QDomElement forecast = product.firstChildElement("forecastCfg").firstChildElement("forecast");
		while (!forecast.isNull())
		{
			ST_FORECAST  stforcast;
			stforcast.nReportTime = forecast.attribute("hourtime").toInt();    // 起报时间
			stforcast.nFileCount = forecast.attribute("filecount").toInt();    // 文件总数 0：表示忽略文件个数

			stforcast.nStartForecast = forecast.attribute("startForecast").toInt();// 文件合并处理时-起始合并预报时-不设置表示不处理
			stforcast.nEndForecast = forecast.attribute("endForecast").toInt();    // 文件合并处理时-终止合并预报时
			stforcast.nStartTime = forecast.attribute("startHour").toInt();// 文件合并处理时-起始合并预报时-不设置表示不处理
			stforcast.nEndTime = forecast.attribute("endHour").toInt();    // 文件合并处理时-终止合并预报时

			QString strLatestTime = forecast.attribute("latestTime");          // 文件合并最后合并时间,格式：06:20
			stforcast.nLastHour = -1;  // 初始化 最迟合并时间
			stforcast.nLastMinute = -1;
			QStringList lstLatestTime = strLatestTime.split(":");
			if (lstLatestTime.size() >= 2)
			{
				stforcast.nLastHour = lstLatestTime.at(0).toInt();
				stforcast.nLastMinute = lstLatestTime.at(1).toInt();
			}

			// 不等间隔配置处理
			QDomElement file = forecast.firstChildElement("file");
			while (!file.isNull())
			{
				ST_RANGE  stRange;
				stRange.nForcastTime = file.attribute("forcasttime").toInt();
				stRange.nTimeRange = file.text().toInt();

				// 添加到时间范围列表中列表中
				stforcast.lstRange.push_back(stRange);

				// 下一条
				file = file.nextSiblingElement();
			}

			// 添加到预报时间间隔中
			stProduct.hasForecast.insert(stforcast.nReportTime, stforcast);

			// 下一条
			forecast = forecast.nextSiblingElement();
		}

		// 存储缓存
		QDomElement cachedsave = product.firstChildElement("savecached");
		if (!cachedsave.isNull())
		{
			stProduct.stCachedSave.bDisabled = cachedsave.attribute("disabled").toLower() == "yes";
			stProduct.stCachedSave.nValidTime = cachedsave.attribute("validtime").toInt();
		}
		else
		{
			stProduct.stCachedSave.bDisabled = true;
		}

		// 存储文件
		QDomElement filesave = product.firstChildElement("savefile");
		if (!filesave.isNull())
		{
			stProduct.stFileSave.bDisabled = filesave.attribute("disabled").toLower() == "yes";
			stProduct.stFileSave.strSavePath = filesave.attribute("savepath");
			stProduct.stFileSave.strFileName = filesave.attribute("filename");
			stProduct.stFileSave.strFileFormat = filesave.attribute("fileformat").toLower();  // 文件格式
			if (stProduct.stFileSave.strFileFormat.isEmpty())
			{
				stProduct.stFileSave.strFileFormat = "grib";
			}
			QString strDDS = filesave.attribute("dds");
			if (!strDDS.isEmpty())
			{
				stProduct.stFileSave.lstDDS = strDDS.split(",");
			}
		}
		else
		{
			stProduct.stFileSave.bDisabled = true;
		}


		// 文件合并处理
		QDomElement mergefilesave = product.firstChildElement("filemerge");
		if (!mergefilesave.isNull())
		{
			stProduct.stMergeFileSave.bDisabled = mergefilesave.attribute("disabled").toInt();
			stProduct.stMergeFileSave.strSavePath = mergefilesave.attribute("savepath");
			stProduct.stMergeFileSave.strFileName = mergefilesave.attribute("filename");
			stProduct.stMergeFileSave.strFileFormat = mergefilesave.attribute("fileformat").toLower();  // 文件格式
			if (stProduct.stMergeFileSave.strFileFormat.isEmpty())
			{
				stProduct.stMergeFileSave.strFileFormat = "grib";
			}
			QString strDDS = mergefilesave.attribute("dds");
			if (!strDDS.isEmpty())
			{
				stProduct.stMergeFileSave.lstDDS = strDDS.split(",");
			}
		}
		else
		{
			stProduct.stMergeFileSave.bDisabled = true;
		}

		if (stProduct.strRole == "raw")
		{
			// 数据加工处理
			QDomElement dataproc = product.firstChildElement("dataproc").firstChildElement("chains");
			while (!dataproc.isNull())
			{
				ST_DATAPROC stDataProc;
				stDataProc.nID = dataproc.attribute("id").toInt();
				stDataProc.strName = dataproc.attribute("name");
				stDataProc.stFileSave.bDisabled = dataproc.attribute("disabled").toLower() == "yes";
				stDataProc.stFileSave.strSavePath = dataproc.attribute("savepath");
				stDataProc.stFileSave.strFileName = dataproc.attribute("filename");
				stDataProc.stFileSave.strFileFormat = dataproc.attribute("fileformat").toLower();  // 文件格式
				if (stDataProc.stFileSave.strFileFormat.isEmpty())
				{
					stDataProc.stFileSave.strFileFormat = "grib";
				}
				QString strDDS = dataproc.attribute("dds");
				if (!strDDS.isEmpty())
				{
					stDataProc.stFileSave.lstDDS = strDDS.split(",");
				}

				QDomElement prochain = dataproc.firstChildElement("chain");
				while (!prochain.isNull())
				{
					ST_CHAIN  stChain;
					stChain.strParams = prochain.attribute("params");
					stChain.strLibName = prochain.text();

					strLogContent = QString("数据加工动态库：%1; 参数=%2").arg(stChain.strLibName).arg(stChain.strParams);
					LOG_(LOGID_DEBUG, LOG_F("[Product Key=%s] %s"), LOG_STR(stProduct.strKey), LOG_STR(strLogContent));

					// 添加到数据处理dll列表中
					stDataProc.lstChains.push_back(stChain);

					// 下一条数据处理dll
					prochain = prochain.nextSiblingElement();
				}

				// 添加到数据加工处理链列表中
				if (stProduct.hasDataProc.contains(stDataProc.nID))
				{
					strLogContent = QString("数据加工处理编号重复（id=%1），添加失败").arg(stDataProc.nID);
					LOG_(LOGID_ERROR, LOG_F("[配置][Product Key=%s] %s"), LOG_STR(stProduct.strKey), LOG_STR(strLogContent));
				}
				else
				{
					stProduct.hasDataProc.insert(stDataProc.nID, stDataProc);
				}

				// 下一条数据加工处理链
				dataproc = dataproc.nextSiblingElement();
			}
		}

		// 站点处理
		QDomElement stationproc = product.firstChildElement("grid2station");
		if (!stationproc.isNull())
		{
			stProduct.stStationProc.bDisabled = stationproc.attribute("disabled").toLower() == "yes";
			stProduct.stStationProc.strProcLib = stationproc.text();
			// todo 临时进行此使用
			stProduct.stStationProc.bDisabled = !(stationproc.text().toLower() == "yes");
		}
		else
		{
			stProduct.stStationProc.bDisabled = true;
		}


		/////////////////////////// merge //////////////////////////////////////
		// merge处理
		if (stProduct.strRole == "merge")
		{
			// 数据合并模版：空数据/指导报数据
			stProduct.strRelatedKey = product.firstChildElement("relatedproduct").text();
			stProduct.stTemplate.strTemplateType = product.firstChildElement("template").attribute("type");
			stProduct.stTemplate.fBlankValue = product.firstChildElement("template").text().toFloat();

			// MASK
			stProduct.stMask.strMaskFile = product.firstChildElement("mask").text();
			stProduct.stMask.fLon1 = product.firstChildElement("mask").attribute("lon1").toFloat();
			stProduct.stMask.fLon2 = product.firstChildElement("mask").attribute("lon2").toFloat();
			stProduct.stMask.fLat1 = product.firstChildElement("mask").attribute("lat1").toFloat();
			stProduct.stMask.fLat2 = product.firstChildElement("mask").attribute("lat2").toFloat();
			stProduct.stMask.fDi = product.firstChildElement("mask").attribute("di").toFloat();
			stProduct.stMask.fDj = product.firstChildElement("mask").attribute("dj").toFloat();
			stProduct.stMask.nNi = (int)((stProduct.stMask.fLon2 - stProduct.stMask.fLon1) / stProduct.stMask.fDi + 0.1) + 1;
			stProduct.stMask.nNj = (int)((stProduct.stMask.fLat2 - stProduct.stMask.fLat1) / stProduct.stMask.fDj + 0.1) + 1;
			
			// 降水处理
			QDomElement rain = product.firstChildElement("rainproc").firstChildElement("rainKey");
			while (!rain.isNull())
			{
				ST_RAIN_PROC stRainKey;
				stRainKey.nRange = rain.attribute("range").toInt();
				stRainKey.strRelatedKey = rain.text();
				stProduct.lstRainProc.push_back(stRainKey);

				rain = rain.nextSiblingElement();
			}
			// 合并方式
			stProduct.nMergeWay = MERGE_WAY_CACHED;  // 默认通过缓存

			// 合并数据源
			QDomElement mergeitem = product.firstChildElement("datasrc").firstChildElement("item");
			while (!mergeitem.isNull())
			{
				ST_MERGE_ITEM stMergeItem;

				stMergeItem.nClientID = mergeitem.attribute("clientid").toInt();
				stMergeItem.fLon1 = mergeitem.attribute("lon1").toFloat();
				stMergeItem.fLon2 = mergeitem.attribute("lon2").toFloat();
				stMergeItem.fLat1 = mergeitem.attribute("lat1").toFloat();
				stMergeItem.fLat2 = mergeitem.attribute("lat2").toFloat();
				stMergeItem.strSrcFolder = mergeitem.attribute("srcfolder");
				stMergeItem.strSrcFileName = mergeitem.attribute("srcfileformat");
				GetFileNameFormat(stMergeItem.strSrcFileName, stMergeItem.stFileNameFormat);

				if (!stMergeItem.strSrcFolder.isEmpty() && !stMergeItem.strSrcFileName.isEmpty())
				{
					// 只要合并数据源中有一个不是空，则通过文件合并
					stProduct.nMergeWay = MERGE_WAY_FILE; 
				}

				// 插入列表中
				stProduct.lstMergeItem.push_back(stMergeItem);

				// 下一条数据源
				mergeitem = mergeitem.nextSiblingElement();
			}
		}
		/////////////////////////// merge //////////////////////////////////////

		// 加入产品队列中
		if (!AddProductList(stProduct))
		{
			LOG_(LOGID_ERROR, LOG_F("[配置][Product Key=%s] Key重复，添加失败"), LOG_STR(stProduct.strKey));
		}

		// 下一个产品
		product = product.nextSiblingElement();
	}


	LOG_(LOGID_DEBUG, LOG_F("读取格点产品配置结束"));

	return 0;
}

/************************************************************************/
/* 读取系统站点产品配置                                                 */
/************************************************************************/
int XmlConfig::ReadStationConfig()
{
	LOG_(LOGID_DEBUG, LOG_F("开始读取站点产品配置[%s]"), LOG_F(STATION_CONFIG_FILE));

	/* 1、打开配置文件，获取配置内容 */
	QFile file(STATION_CONFIG_FILE);
	if (!file.open(QIODevice::ReadOnly))
	{
		LOG_(LOGID_ERROR, LOG_F("[配置]配置文件%s打开失败"), LOG_F(STATION_CONFIG_FILE));
		return -1;
	}

	QDomDocument domDocument;
	if (!domDocument.setContent(&file))
	{
		LOG_(LOGID_ERROR, LOG_F("[配置]获取配置文件%s内容失败"), LOG_F(STATION_CONFIG_FILE));
		file.close();
		return -2;
	}
	file.close();

	QDomElement root = domDocument.documentElement();

	/*2、读取站点管理配置 */
	QDomElement stationmag = root.firstChildElement("stationmange");
	if (!stationmag.isNull())
	{
		m_stStationMag.bDisabled = stationmag.attribute("disabled").toLower() == "yes";
		m_stStationMag.strStationFile = stationmag.attribute("stationlist");
		m_stStationMag.strCityType = stationmag.attribute("cityType").toLower();
		m_stStationMag.strForecastType = stationmag.attribute("forecastType");
		//m_stStationMag.nStationRange = stationmag.attribute("timeRange").toInt();
		m_stStationMag.nRadius = stationmag.attribute("radius").toInt();
		m_stStationMag.nValidTime = stationmag.attribute("validtime").toInt();
		m_stStationMag.bIsPublish = stationmag.attribute("publish").toLower() == "yes";
		m_stStationMag.bIsStation2Grid = stationmag.attribute("station2grid").toLower() == "yes";
	}
	else
	{
		m_stStationMag.bDisabled = true;
	}

	/*3、读取站点配置 */
	m_lstStationCfg.clear();
	QDomElement station = root.namedItem("Stations").firstChildElement("Station");
	while (!station.isNull())
	{
		ST_STATION_CFG stStationCfg;

		// 基本信息
		stStationCfg.strRole = station.attribute("role").toLower();
		stStationCfg.strIDSymbol = station.attribute("Symbol").toLower();
		stStationCfg.strCityType = station.attribute("cityType").toLower();
		stStationCfg.nTimeRange = station.attribute("TimeRange").toInt();
		stStationCfg.nMaxForecastTime = station.attribute("MaxForecastTime").toInt();

		// 扫描执行时间
		stStationCfg.stScanTime.nInterval = station.firstChildElement("scantime").attribute("interval").toInt();
		QDomElement protime = station.firstChildElement("scantime").firstChildElement("time");
		while (!protime.isNull())
		{
			ST_PROTIME  stProTime;
			stProTime.nHour = protime.attribute("hour").toInt();
			stProTime.nMin = protime.attribute("min").toInt();
			stProTime.nSecond = protime.attribute("second").toInt();
			stProTime.nInterval = protime.text().toInt();

			// 添加到产品的时间列表中
			stStationCfg.stScanTime.lstProTime.push_back(stProTime);

			// 下一条扫描时间
			protime = protime.nextSiblingElement();
		}

		// 数据源
		QDomElement item = station.firstChildElement("datasrc").firstChildElement("item");
		while (!item.isNull())
		{
			ST_SRC_ITEM stSrcItem;

			stSrcItem.strDataType = item.attribute("dataType");
			stSrcItem.strSymbol = item.attribute("Symbol");
			stSrcItem.strDescription = item.attribute("Description");
			stSrcItem.strSrcFolder = item.attribute("srcfolder");
			stSrcItem.strSrcFileName = item.attribute("srcfileformat");
			GetFileNameFormat(stSrcItem.strSrcFileName, stSrcItem.stFileNameFormat);
			QString strDDS = item.attribute("dds");
			if (!strDDS.isEmpty())
			{
				stSrcItem.lstDDS = strDDS.split(",");
			}

			// 插入列表中
			stStationCfg.lstSrcItem.push_back(stSrcItem);

			// 下一条数据源
			item = item.nextSiblingElement();
		}

		// 缓存存储
		QDomElement cachedsave = station.firstChildElement("savecached");
		if (!cachedsave.isNull())
		{
			stStationCfg.stCachedSave.bDisabled = cachedsave.attribute("disabled").toLower() == "yes";
			stStationCfg.stCachedSave.nValidTime = cachedsave.attribute("validtime").toInt();
		}
		else
		{
			stStationCfg.stCachedSave.bDisabled = true;
		}

		// 时次定义和保存文件的时间定义
		QDomElement reporttimes = station.firstChildElement("Times").firstChildElement("time");
		while (!reporttimes.isNull())
		{
			ST_REPORTTIME stTime;

			stTime.nReportTime = reporttimes.attribute("reportHour").toInt();
			//stTime.nSaveHour = reporttimes.attribute("saveHour").toInt();
			//stTime.nSaveMinute = reporttimes.attribute("saveMinute").toInt();
			stTime.strSaveStation = reporttimes.attribute("stationlist");
			stTime.strSaveRule = reporttimes.attribute("rule");
			stTime.strSaveType = reporttimes.attribute("saveType");

			// 文件保存
			QDomElement filesave = reporttimes.firstChildElement("savefile");
			if (!filesave.isNull())
			{
				// 文件保存
				stTime.stSaveFile.bDisabled = filesave.attribute("disabled").toLower() == "yes";
				stTime.stSaveFile.strSavePath = filesave.attribute("savepath");
				stTime.stSaveFile.strFileName = filesave.attribute("filename");
				QString strDDS = filesave.attribute("dds");
				if (!strDDS.isEmpty())
				{
					stTime.stSaveFile.lstDDS = strDDS.split(",");
				}
			}
			else
			{
				stTime.stSaveFile.bDisabled = true;
			}

			// 插入列表中
			stStationCfg.lstReportTimes.push_back(stTime);

			// 下一条数据源
			reporttimes = reporttimes.nextSiblingElement();
		}

		// 添加到站点配置列表中
		m_lstStationCfg.push_back(stStationCfg);

		// 下一个站点配置
		station = station.nextSiblingElement();
	}
	LOG_(LOGID_DEBUG, LOG_F("读取站点产品配置结束"));

	return 0;
}

/************************************************************************/
/* 查找是否已经存在某Key的产品                                          */
/************************************************************************/
bool XmlConfig::FindProduct(QString strKey)
{
	if (m_hasProduct.contains(strKey))
	{
		return true;
	}

	return false;
}

/************************************************************************/
/* 添加产品信息到产品列表中                                             */
/************************************************************************/
bool  XmlConfig::AddProductList(ST_PRODUCT stProduct)
{
	// 查找是否已经存在
	if (FindProduct(stProduct.strKey))
	{
		// todo 该Key的产品已经存储，无法进行添加
		return false;
	}

	// 添加到列表中
	m_hasProduct.insert(stProduct.strKey, stProduct);

	return true;
}

/************************************************************************/
/* 获取TCPServer的配置                                                  */
/************************************************************************/
ST_TCPSERVER XmlConfig::GetTcpServer()
{
	return m_stTcpServer;
}

/************************************************************************/
/* 获取缓存配置                                                         */
/************************************************************************/
ST_CACHED XmlConfig::GetCachedConfig()
{
	return m_stCached;
}

/************************************************************************/
/* 获取Http配置                                                         */
/************************************************************************/
ST_HTTP XmlConfig::GetHttpCfg()
{
	return m_stHttp;
}

/************************************************************************/
/* 获取监控                                                             */
/************************************************************************/
bool  XmlConfig::IsMonitor()
{
	return m_bMonitor;
}

/************************************************************************/
/* 发布开关                                                             */
/************************************************************************/
bool  XmlConfig::IsPublish()
{
	return m_bPublish;
}

/************************************************************************/
/* 获取系统清理配置                                                     */
/************************************************************************/
ST_SYSCLEAN XmlConfig::GetSysClean()
{
	return m_stSysClean;
}

/************************************************************************/
/* 获取数据分发配置                                                     */
/************************************************************************/
ST_DDS_CFG XmlConfig::GetDDSConfig()
{
	return m_stDDSCfg;
}

/************************************************************************/
/* 获取产品列表                                                         */
/************************************************************************/
HASH_PRODUCT  XmlConfig::GetProductList()
{
	return m_hasProduct;
}

/************************************************************************/
/* 获取客户端列表                                                       */
/************************************************************************/
HASH_CLIENT XmlConfig::GetClientList()
{
	return m_hasClient;
}

/************************************************************************/
/* 获取连接池配置                                                       */
/************************************************************************/
ST_CONNPOOL XmlConfig::GetConnPool()
{
	return m_stConnPool;
}

/************************************************************************/
/* 获取站点配置                                                         */
/************************************************************************/
LIST_STATION_CFG XmlConfig::GetStationCfg()
{
	return m_lstStationCfg;
}

/************************************************************************/
/* 获取站点管理配置                                                     */
/************************************************************************/
ST_STATION_MAG XmlConfig::GetStationMag()
{
	return m_stStationMag;
}

/************************************************************************/
/* 解析文件名格式                                                       */
/************************************************************************/
void XmlConfig::GetFileNameFormat(QString strFileName, ST_FILE_FORMAT &stFormat)
{
	// 默认设置
	stFormat.stYear.iStart = 0;
	stFormat.stYear.iLen = 4;
	stFormat.stMonth.iStart = 4;
	stFormat.stMonth.iLen = 2;
	stFormat.stDay.iStart = 6;
	stFormat.stDay.iLen = 2;
	stFormat.stHour.iStart = 8;
	stFormat.stHour.iLen = 2;
	stFormat.stForcast.iStart = 11;
	stFormat.stForcast.iLen = 3;

	// 文件长度 
	stFormat.nLength = strFileName.length();
	if (stFormat.nLength == 0)
	{
		return;
	}

	// 年
	short iYearStart = strFileName.indexOf("YYYY");
	if (iYearStart >= 0)
	{
		stFormat.stYear.iStart = iYearStart;
		stFormat.stYear.iLen = 4;
	}
	else
	{
		// 2位年份
		iYearStart = strFileName.indexOf("YY");
		if (iYearStart >= 0)
		{
			stFormat.stYear.iStart = iYearStart;
			stFormat.stYear.iLen = 2;
		}
	}

	// 月
	short iMonthStart = strFileName.indexOf("MM");
	if (iMonthStart >= 0)
	{
		stFormat.stMonth.iStart = iMonthStart;
		stFormat.stMonth.iLen = 2;
	}

	// 日
	short iDayStart = strFileName.indexOf("DD");
	if (iDayStart >= 0)
	{
		stFormat.stDay.iStart = iDayStart;
		stFormat.stDay.iLen = 2;
	}

	// 时
	short iHourStart = strFileName.indexOf("HH");
	if (iHourStart >= 0)
	{
		stFormat.stHour.iStart = iHourStart;
		stFormat.stHour.iLen = 2;
	}

	// 预报时间
	short iForcastStart = strFileName.indexOf("FFF");
	if (iForcastStart >= 0)
	{
		stFormat.stForcast.iStart = iForcastStart;
		stFormat.stForcast.iLen = 3;
	}

	// 时间间隔
	short iRangeStart = strFileName.indexOf("TT");
	if (iRangeStart >= 0)
	{
		stFormat.stRange.iStart = iRangeStart;
		stFormat.stRange.iLen = 2;
	}
}

/************************************************************************/
/* 配置检测                                                             */
/************************************************************************/
ST_CHECKUP XmlConfig::GetCheckupCfg()
{
	return m_stCheckup;
}

/************************************************************************/
/* 下载配置                                                             */
/************************************************************************/
ST_DOWNLOAD XmlConfig::GetDownLoadCfg()
{
	return m_stDownLoad;
}

/************************************************************************/
/* 上传配置                                                             */
/************************************************************************/
ST_UPLOAD XmlConfig::GetUpLoadCfg()
{
	return m_stUpLoad;
}

/************************************************************************/
/* 默认配置                                                             */
/************************************************************************/
QString  XmlConfig::GetRoot()
{
	return m_strRoot;
}
QString  XmlConfig::GetCCCC()
{
	return m_strCccc;
}
float XmlConfig::GetLon1()
{
	return m_fLon1;
}
float XmlConfig::GetLon2()
{
	return m_fLon2;
}
float XmlConfig::GetLat1()
{
	return m_fLat1;
}
float XmlConfig::GetLat2()
{
	return m_fLat2;
}


