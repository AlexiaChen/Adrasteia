#ifndef SERVERCFG_H
#define SERVERCFG_H

#include <QObject>
#include <QString>
#include <QDomDocument>
#include <QDomElement>
#include <QVector>
#include <QFile>

typedef struct _ServerSocketListenerInfo{
	bool bDisable;
	QString portNum;
	QString maxConNum;

}ServerSocketListenerInfo;

typedef struct _HTTPConfig{
	bool bDisable;
	QString scanInterval;
	QString downloadURL;
	QString savePath;
}HttpConfig;

enum class ProviderType{
	REDIS,
	MEMCACHED,
	UNKNOWN
};
typedef struct _CacheConfig{
	bool bDisable;
	ProviderType providerType;
	QString cacheIP;
	QString cachePort;
}CacheConfig;


enum class DBType{
	DB_SQLITE,
	DB_MYSQL,
	DB_SQLSERVER,
	DB_ORACLE,
	DB_UNKNOWN
};
typedef struct _DBConfig{
	bool bDisable;
	QString minConnectNum;
	QString maxConnectNum;
	DBType dbType;
	QString dbIP;
	QString dbPort;
	QString userName;
	QString password;
	QString dbName;
}DBConfig;


enum class DDSType
{
	MQ,
	UNKNOWN
};


typedef struct _DDSServer{
	QString ID;
	DDSType ddsType;
	QString serverIP;
	QString userName;
	QString password;
	QString ftpFileRootPath;
}DDSServer;

typedef QVector<DDSServer> DDSServerList;

typedef struct _DDSConfig{
	bool bDisable;
	QString scanInterval;
	DDSServerList serverList;
}DDSConfig;

typedef struct _SystemMaintanConfig{
	bool bDisable;
	QString scanInterval;
	QString logKeepDay;
	QString dbKeepDay;
}SystemMaintanConfig;

class ServerCfg : public QObject
{
	Q_OBJECT

public:
	ServerCfg(QString& path,QObject *parent);
	~ServerCfg();
	
	bool isInvalid() const;
	void saveToFile();
	void openFile();
	//getter
	void getServerSocketListenerInfo(ServerSocketListenerInfo* cfg);
	void getCacheConfig(CacheConfig* cfg);
	void getDBConfig(DBConfig* cfg);
	void getDDSConfig(DDSConfig *cfg);
	void getSystemMaintanConfig(SystemMaintanConfig* cfg);

	//setter
	void setServerSocketListenerInfo(ServerSocketListenerInfo* cfg);
	void setCacheConfig(CacheConfig* cfg);
	void setDBConfig(DBConfig* cfg);
	void setDDSConfig(DDSConfig* cfg);
	void setSystemMaintanConfig(SystemMaintanConfig* cfg);

private:
	QString cfgPath;
	bool m_isInvalid;
	QDomDocument m_doc;
	QDomElement m_root;
	QFile m_xmlFile;
};

#endif // SERVERCFG_H
