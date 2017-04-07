#ifndef CLIENTCFG_H
#define CLIENTCFG_H

#include <QObject>
#include <QString>
#include <QDomDocument>
#include <QDomElement>
#include <QVector>
#include <QFile>

typedef struct _Grid{
	QString savePath;
	QString fileName;
}Grid;


typedef struct _Station{
	QString savePath;
	QString fileName;
}Station;



typedef struct _Client{
	QString ID;
	QString name;
	QString ip;
	QString userName;
	QString passWord;
	bool isUpLoad;
	QString cacheValidTime;
	Grid grid;
	Station station;
}Client;


typedef QVector<Client> ClientList;

class ClientCfg : public QObject
{
	Q_OBJECT

public:
	ClientCfg(QString& path, QObject *parent);
	~ClientCfg();

	bool isInvalid() const;
	void saveToFile();
	void openFile();

	//getter
	void getClientList(ClientList &list);

	//setter
	void setClientList(ClientList &list);

private:
	void addClientsToTree(ClientList& list, QDomElement& clients);
	void addClientHeader(QDomElement& clientEle, Client& client);
	void addGrid(QDomElement& clientEle, Client& client);

private:
	QString cfgPath;
	bool m_isInvalid;
	QDomDocument m_doc;
	QDomElement m_root;
	QFile m_xmlFile;
};

#endif // CLIENTCFG_H
