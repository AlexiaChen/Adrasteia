#include "PublisherConfig.h"
#include <QDir>
#include <QTextStream>
#include <QRegExp>
PublisherConfig::PublisherConfig(QObject *parent)
	: QObject(parent)
{

}

PublisherConfig::~PublisherConfig()
{

}

PublisherConfig::PublisherHas PublisherConfig::publishers() const
{

	return m_pubHas;
}

void PublisherConfig::openFile()
{
	QString cfg = QDir::currentPath() + QString("/publisher.cfg");
	
	m_file.setFileName(cfg);

	if (!m_file.open(QFile::ReadOnly | QFile::Text)) {
		printf("open publisher config file failed\n");
		
	}

	QTextStream text(&m_file);
	
	while (!text.atEnd())
	{
		QString line = text.readLine();
		QString key = line.split(QRegExp("\\s+"))[0];
		QString value = line.split(QRegExp("\\s+"))[1];
		m_pubHas.insert(key, value);
	}
}
