#ifndef PUBLISHERCONFIG_H
#define PUBLISHERCONFIG_H

#include <QObject>
#include <QFile>
#include <QHash>

class PublisherConfig : public QObject
{
	Q_OBJECT

	
public:
	typedef QHash<QString, QString> PublisherHas;
	
	PublisherConfig(QObject *parent);
	~PublisherConfig();

	PublisherHas publishers() const;
	void openFile();

private:
	PublisherHas m_pubHas;
	QFile m_file;

};

#endif // PUBLISHERCONFIG_H
