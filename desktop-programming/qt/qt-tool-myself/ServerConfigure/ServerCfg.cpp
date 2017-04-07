#include "ServerCfg.h"

#include <QDir>
#include <QDomNode>
#include <QDomText>
#include <QTextStream>

ServerCfg::ServerCfg(QString& path, QObject *parent)
	: cfgPath(path),QObject(parent), m_isInvalid(false)
{
	

}

ServerCfg::~ServerCfg()
{

}

bool ServerCfg::isInvalid() const
{
	return m_isInvalid;
}

void ServerCfg::saveToFile()
{

	m_xmlFile.resize(0);
	QTextStream out(&m_xmlFile);
	out.setCodec("UTF-8");
	m_doc.save(out,4, QDomNode::EncodingFromTextStream);

	m_xmlFile.close();
	m_isInvalid = true;

	openFile();

}

void  ServerCfg::openFile()
{
	QString xmlPath = cfgPath + QString("/NwfdServerConfig.xml");
	m_xmlFile.setFileName(xmlPath);

	if (!m_xmlFile.open(QFile::ReadWrite | QFile::Text)) {
		printf("open server config file failed\n");
		m_isInvalid = true;
	}

	QString         strError;
	int             errLin = 0, errCol = 0;
	if (!m_doc.setContent(&m_xmlFile, false, &strError, &errLin, &errCol)) {
		printf("parse file failed at line %d column %d, error: %s !\n", errLin, errCol, strError);
		m_isInvalid = true;
	}

	if (m_doc.isNull()) {
		printf("document is null !\n");
		m_isInvalid = true;
	}

	m_root = m_doc.documentElement();
	if (m_root.tagName().compare(QString("config"), Qt::CaseSensitive) != 0)
	{
		m_isInvalid = true;
	}
}


void ServerCfg::getServerSocketListenerInfo(ServerSocketListenerInfo* cfg)
{
	
		QDomElement listener = m_root.firstChildElement("listener");
		
		if (!listener.isNull())
		{
			cfg->bDisable = listener.attribute("disabled").toLower() == "yes";
			cfg->portNum = listener.firstChildElement("port").text();
			cfg->maxConNum = listener.firstChildElement("maxConnections").text();
		}
			
}

void ServerCfg::getCacheConfig(CacheConfig* cfg)
{
	QDomElement provider = m_root.firstChildElement("cached").firstChildElement("provider");

	if (!provider.isNull())
	{
		cfg->bDisable = false;
		QString  strType = provider.attribute("type");

		if (strType.toLower() == "redis")
		{
			cfg->providerType = ProviderType::REDIS;
		}
		else if (strType.toLower() == "memcached")
		{
			cfg->providerType = ProviderType::MEMCACHED;
		}
		else
		{
			cfg->providerType = ProviderType::UNKNOWN;
		}

		cfg->cacheIP = provider.firstChildElement("address").text();
		cfg->cachePort = provider.firstChildElement("port").text();
	}
	else
	{
		cfg->bDisable = true;
	}
			
}

void ServerCfg::getDBConfig(DBConfig* cfg)
{
	

	QDomElement database = m_root.firstChildElement("database");

	if (!database.isNull())
	{
		cfg->bDisable = database.attribute("disabled").toLower() == "yes";
		cfg->minConnectNum = database.attribute("minCon");
		cfg->maxConnectNum = database.attribute("maxCon");
	}

	QDomElement db = database.firstChildElement("db");
	QString strDBType = db.attribute("type");
	if (strDBType == "sqlite")
	{
		cfg->dbType = DBType::DB_SQLITE;
	}
	else if (strDBType == "mysql")
	{
		cfg->dbType = DBType::DB_MYSQL;
	}
	else if (strDBType == "sqlserver")
	{
		cfg->dbType = DBType::DB_SQLSERVER;
	}
	else if (strDBType == "oracle")
	{
		cfg->dbType = DBType::DB_ORACLE;
	}
	else
	{
		cfg->dbType = DBType::DB_UNKNOWN;
	}

	cfg->dbIP = db.attribute("address");
	cfg->dbPort = db.attribute("port");
	cfg->userName = db.attribute("username");
	cfg->password = db.attribute("password");
	cfg->dbName = db.attribute("databasename");
		
}

void ServerCfg::getDDSConfig(DDSConfig *cfg)
{
	QDomElement dds = m_root.firstChildElement("dds");
			
	if (!dds.isNull())
	{
		cfg->bDisable = dds.attribute("disabled").toLower() == "yes";
		cfg->scanInterval = dds.attribute("interval");
				
		QDomElement ddsserver = dds.firstChildElement("server");
		while (!ddsserver.isNull())
		{
			DDSServer server;
					
			server.ID = ddsserver.attribute("id");

			QString strType = ddsserver.attribute("type");
			if (strType == "mq")
			{
				server.ddsType = DDSType::MQ;
			}
			else
			{
				server.ddsType = DDSType::UNKNOWN;
			}

			server.serverIP = ddsserver.attribute("address");
			server.userName = ddsserver.attribute("username");
			server.password = ddsserver.attribute("passwd");
			server.ftpFileRootPath = ddsserver.attribute("path");

			//²åÈëDDS ServerÁ´±í 
			cfg->serverList.push_back(server);
			
					
			ddsserver = ddsserver.nextSiblingElement();
		}
	}
			
}

void ServerCfg::getSystemMaintanConfig(SystemMaintanConfig* cfg)
{
	
	QDomElement checkup = m_root.firstChildElement("checkup");
		
	if (!checkup.isNull())
	{
		cfg->bDisable = checkup.attribute("disabled").toLower() == "yes";
		cfg->scanInterval = checkup.attribute("interval");
		cfg->logKeepDay = checkup.firstChildElement("log").text();
		cfg->dbKeepDay = checkup.firstChildElement("database").text();
	}
	else
	{
		cfg->bDisable = true;
	}

}


void ServerCfg::setServerSocketListenerInfo(ServerSocketListenerInfo* cfg)
{
	QDomElement listener = m_root.firstChildElement("listener");

	if (!listener.isNull())
	{
		QDomElement oldPort = listener.firstChildElement("port");
		QDomElement newPort = m_doc.createElement("port");
		QDomText newPortText = m_doc.createTextNode(cfg->portNum);
		newPort.appendChild(newPortText);

		listener.replaceChild(newPort, oldPort);

		QDomElement oldMaxCons = listener.firstChildElement("maxConnections");
		QDomElement newMaxCons = m_doc.createElement("maxConnections");
		QDomText newMaxConsText = m_doc.createTextNode(cfg->maxConNum);
		newMaxCons.appendChild(newMaxConsText);
		listener.replaceChild(newMaxCons, oldMaxCons);
	}
}

void ServerCfg::setCacheConfig(CacheConfig* cfg)
{
	QDomElement provider = m_root.firstChildElement("cached").firstChildElement("provider");

	if (!provider.isNull())
	{

		if (cfg->providerType == ProviderType::MEMCACHED)
		{
			provider.setAttribute("type", "memcached");
		}
		else if (cfg->providerType == ProviderType::REDIS)
		{
			provider.setAttribute("type", "redis");
		}
		
		QDomElement oldAddress = provider.firstChildElement("address");
		QDomElement newAddress = m_doc.createElement("address");
		QDomText newAddressText = m_doc.createTextNode(cfg->cacheIP);
		newAddress.appendChild(newAddressText);

		provider.replaceChild(newAddress, oldAddress);

		QDomElement oldPort = provider.firstChildElement("port");
		QDomElement newPort = m_doc.createElement("port");
		QDomText newPortText = m_doc.createTextNode(cfg->cachePort);
		newPort.appendChild(newPortText);
		provider.replaceChild(newPort, oldPort);
	}
}

void ServerCfg::setDBConfig(DBConfig* cfg)
{
	QDomElement database = m_root.firstChildElement("database");

	if (!database.isNull())
	{
		database.setAttribute("maxCon", cfg->maxConnectNum);
		database.setAttribute("minCon", cfg->minConnectNum);

		QDomElement db = database.firstChildElement("db");
		db.setAttribute("databasename", cfg->dbName);
		db.setAttribute("port", cfg->dbPort);
		db.setAttribute("address", cfg->dbIP);
		
		if (cfg->dbType == DBType::DB_SQLITE)
		{
			db.setAttribute("type", "sqlite");
		}
		else if (cfg->dbType == DBType::DB_MYSQL)
		{
			db.setAttribute("type", "mysql");
		}
		else if (cfg->dbType == DBType::DB_SQLSERVER)
		{
			db.setAttribute("type", "sqlserver");
		}
		else if (cfg->dbType == DBType::DB_ORACLE)
		{
			db.setAttribute("type", "oracle");
		}

		db.setAttribute("password", cfg->password);
		db.setAttribute("username", cfg->userName);
		
	}
}

void ServerCfg::setDDSConfig(DDSConfig* cfg)
{
	QDomElement dds = m_root.firstChildElement("dds");

	if (!dds.isNull())
	{
		dds.setAttribute("interval", cfg->scanInterval);
		
		for (int i = 0; i < cfg->serverList.count(); ++i)
		{
			
			QDomElement server = dds.firstChildElement("server");
			
			while (!server.isNull())
			{
				if (cfg->serverList.at(i).ID == server.attribute("id"))
				{
					server.setAttribute("username", cfg->serverList.at(i).userName);
					server.setAttribute("address", cfg->serverList.at(i).serverIP);
					server.setAttribute("id", cfg->serverList.at(i).ID);



					if (cfg->serverList.at(i).ddsType == DDSType::MQ)
					{
						server.setAttribute("type", "mq");
					}

					server.setAttribute("path", cfg->serverList.at(i).ftpFileRootPath);
					server.setAttribute("passwd", cfg->serverList.at(i).password);

				}



				server = server.nextSiblingElement();
			}
			
		}
	}
}

void ServerCfg::setSystemMaintanConfig(SystemMaintanConfig* cfg)
{
	QDomElement checkup = m_root.firstChildElement("checkup");

	if (!checkup.isNull())
	{
		checkup.setAttribute("interval", cfg->scanInterval);

		QDomElement oldLog = checkup.firstChildElement("log");
		QDomElement newLog = m_doc.createElement("log");
		QDomText newLogText = m_doc.createTextNode(cfg->logKeepDay);

		newLog.appendChild(newLogText);
		checkup.replaceChild(newLog, oldLog);

		QDomElement oldDatabase = checkup.firstChildElement("database");
		QDomElement newDatabase = m_doc.createElement("database");
		QDomText newdbText = m_doc.createTextNode(cfg->dbKeepDay);

		newDatabase.appendChild(newdbText);
		checkup.replaceChild(newDatabase, oldDatabase);
	}
}
