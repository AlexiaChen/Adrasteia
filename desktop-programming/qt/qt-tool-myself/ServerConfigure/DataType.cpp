#include "DataType.h"

#include <QDir>
#include <QTextStream>

DataTypeConfig::DataTypeConfig(QObject *parent)
	: QObject(parent)
{

}

DataTypeConfig::~DataTypeConfig()
{

}

DataTypeConfig::DataTypeHas DataTypeConfig::datatypes() const
{
	return m_dataTypeHas;
}

void DataTypeConfig::openFile()
{
	QString cfg = QDir::currentPath() + QString("/datatype.cfg");

	m_file.setFileName(cfg);

	if (!m_file.open(QFile::ReadOnly | QFile::Text)) {
		printf("open data type config file failed\n");

	}

	QTextStream text(&m_file);

	while (!text.atEnd())
	{
		QString line = text.readLine();
		QString key = line.split(QRegExp("\\s+"))[0];
		QString value = line.split(QRegExp("\\s+"))[1];
		m_dataTypeHas.insert(key, value);
	}
}
