#ifndef DATATYPE_H
#define DATATYPE_H

#include <QObject>
#include <QHash>
#include <QString>
#include <QFile>

class DataTypeConfig : public QObject
{
	Q_OBJECT

public:
	DataTypeConfig(QObject *parent);
	~DataTypeConfig();

public:
	typedef QHash<QString, QString> DataTypeHas;

	
	DataTypeHas datatypes() const;
	
	void openFile();


private:
	DataTypeHas m_dataTypeHas;
	QFile m_file;
};

#endif // DATATYPE_H
