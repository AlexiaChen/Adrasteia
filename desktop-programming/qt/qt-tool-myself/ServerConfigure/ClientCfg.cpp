#include "ClientCfg.h"

#include <QDir>
#include <QDomNode>
#include <QDomText>
#include <QTextStream>

ClientCfg::ClientCfg(QString& path, QObject *parent)
	: cfgPath(path),QObject(parent), m_isInvalid(false)
{

}

ClientCfg::~ClientCfg()
{

}

bool ClientCfg::isInvalid() const
{
	return m_isInvalid;
}

void ClientCfg::saveToFile()
{
	m_xmlFile.resize(0);
	QTextStream out(&m_xmlFile);
	out.setCodec("UTF-8");
	m_doc.save(out, 4, QDomNode::EncodingFromTextStream);

	m_xmlFile.close();
	m_isInvalid = true;

	openFile();
}

void ClientCfg::openFile()
{
	QString xmlPath = cfgPath + QString("/ClientConfig.xml");
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

void ClientCfg::getClientList(ClientList &list)
{
	QDomElement client = m_root.firstChildElement("clients").firstChildElement("client");
	while (!client.isNull())
	{
		Client stClient;
		stClient.ID = client.attribute("clientid");
		stClient.name = client.attribute("name");
		stClient.ip = client.attribute("ip");
		stClient.userName = client.attribute("username");
		stClient.passWord = client.attribute("password");
		stClient.isUpLoad = client.attribute("upload").toLower() == "yes" ? true: false;

		QDomElement clientGrid = client.firstChildElement("grid");
		if (!clientGrid.isNull())
		{
			stClient.grid.savePath = clientGrid.attribute("savepath");
			stClient.grid.fileName = clientGrid.attribute("filename");
		}
		QDomElement clientStation = client.firstChildElement("station");
		if (!clientStation.isNull())
		{
			stClient.station.savePath = clientStation.attribute("savepath");
			stClient.station.fileName = clientStation.attribute("filename");
		}

		stClient.cacheValidTime = client.attribute("cachedvalidtime");

		
		// 插入客户端配置列表中
		list.push_back(stClient);
		

		// 下一条客户端配置
		client = client.nextSiblingElement();
	}
}

void ClientCfg::setClientList(ClientList &list)
{
	//删除Dom Tree并重建
	QDomElement clients = m_root.firstChildElement("clients");
	m_root.removeChild(clients);

	clients = m_doc.createElement("clients");
	m_root.appendChild(clients);

	addClientsToTree(list, clients);
	
}

void ClientCfg::addClientsToTree(ClientList& list, QDomElement& clients)
{
	for (int i = 0; i < list.size(); ++i)
	{
		QDomElement clientEle = m_doc.createElement("client");

		Client client = list.at(i);

		addClientHeader(clientEle, client);
		addGrid(clientEle, client);
		
		clients.appendChild(clientEle);
	}
}

void ClientCfg::addClientHeader(QDomElement& clientEle, Client& client)
{
	clientEle.setAttribute("clientid",client.ID);
	clientEle.setAttribute("name", client.name);
	clientEle.setAttribute("ip", client.ip);
	clientEle.setAttribute("username", client.userName);
	clientEle.setAttribute("password", client.passWord);
	if (client.isUpLoad)  clientEle.setAttribute("upload", "yes");
	clientEle.setAttribute("cachedvalidtime", client.cacheValidTime);
}

void ClientCfg::addGrid(QDomElement& clientEle, Client& client)
{
	QDomElement gridEle = m_doc.createElement("grid");
	gridEle.setAttribute("savepath",client.grid.savePath);
	gridEle.setAttribute("filename", client.grid.fileName);
	clientEle.appendChild(gridEle);
}
