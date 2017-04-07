#ifndef CONFIGPARSER_H
#define CONFIGPARSER_H

#include <QObject>

class ServerCfg;
class ProductCfg;
class ClientCfg;
class StationCfg;
class PublisherConfig;
class TypeConfig;
class DataTypeConfig;

class ConfigParser : public QObject
{
	Q_OBJECT

public:
	ConfigParser(QString &path,QObject *parent);
	~ConfigParser();

	ServerCfg* getServerConfig() const;
	ProductCfg* getProductConfig() const;
	ClientCfg* getClientConfig() const;
	StationCfg* getStationConfig() const;
	//////////////////////
	TypeConfig* getTypeConfig() const;
	PublisherConfig* getPublisherConfig() const;
	DataTypeConfig* getDataTypeConfig() const;

private:
	QString cfgPath;
	ServerCfg* m_servercCfg;
	ProductCfg* m_productCfg;
	ClientCfg* m_clientCfg;
	StationCfg* m_stationCfg;

	////////////////
	PublisherConfig* m_publisherCfg;
	TypeConfig*      m_typeCfg;
	DataTypeConfig* m_dataTypeCfg;
};

#endif // CONFIGPARSER_H
