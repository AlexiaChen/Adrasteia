#include "TypeConfig.h"
#include <QDir>
#include <QTextStream>
TypeConfig::TypeConfig(QObject *parent)
	: QObject(parent)
{

}

TypeConfig::~TypeConfig()
{

}

TypeConfig::TypeHas TypeConfig::types() const
{
	return m_typeHas;
}

void TypeConfig::openFile()
{
	QString cfg = QDir::currentPath() + QString("/type.cfg");

	m_file.setFileName(cfg);

	if (!m_file.open(QFile::ReadOnly | QFile::Text)) {
		printf("open type config file failed\n");

	}

	QTextStream text(&m_file);

	while (!text.atEnd())
	{
		QString line = text.readLine();
		QString key = line.split(QRegExp("\\s+"))[0];
		QString value = line.split(QRegExp("\\s+"))[1];
		m_typeHas.insert(key, value);
	}
}
