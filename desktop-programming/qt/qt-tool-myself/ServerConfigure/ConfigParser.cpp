#include "ConfigParser.h"

#include "ServerCfg.h"
#include "ProductCfg.h"
#include "ClientCfg.h"
#include "StationCfg.h"
#include "PublisherConfig.h"
#include "TypeConfig.h"
#include "DataType.h"

ConfigParser::ConfigParser(QString &path, QObject *parent)
	: QObject(parent)
{
	m_servercCfg = new ServerCfg(path,parent);
	m_servercCfg->openFile();
	
	m_productCfg = new ProductCfg(path,parent);
	
	
	m_clientCfg = new ClientCfg(path,parent);
	m_clientCfg->openFile();

	m_stationCfg = new StationCfg(path,parent);
	m_stationCfg->openFile();

	////////////////////////////
	m_typeCfg = new TypeConfig(parent);
	m_typeCfg->openFile();
	
	m_publisherCfg = new PublisherConfig(parent);
	m_publisherCfg->openFile();

	m_dataTypeCfg = new DataTypeConfig(parent);
	m_dataTypeCfg->openFile();

}

ConfigParser::~ConfigParser()
{
	if (m_servercCfg) delete m_servercCfg;
	if (m_productCfg) delete m_productCfg;
	if (m_clientCfg) delete m_clientCfg;
	if (m_stationCfg) delete m_stationCfg;
	if (m_typeCfg) delete m_typeCfg;
	if (m_publisherCfg) delete m_publisherCfg;
	if (m_dataTypeCfg) delete m_dataTypeCfg;
}


ServerCfg* ConfigParser::getServerConfig() const
{
	return m_servercCfg;
}

ProductCfg* ConfigParser::getProductConfig() const
{
	
	m_productCfg->openFile();
	return m_productCfg;
}

ClientCfg* ConfigParser::getClientConfig() const
{
	m_clientCfg->openFile();
	return m_clientCfg;
}

StationCfg* ConfigParser::getStationConfig() const
{
	m_stationCfg->openFile();
	return m_stationCfg;
}

TypeConfig* ConfigParser::getTypeConfig() const
{
	return m_typeCfg;
}

PublisherConfig* ConfigParser::getPublisherConfig() const
{
	return m_publisherCfg;
}

DataTypeConfig* ConfigParser::getDataTypeConfig() const
{
	return m_dataTypeCfg;
}
