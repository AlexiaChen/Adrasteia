#ifndef TYPECONFIG_H
#define TYPECONFIG_H

#include <QObject>
#include <QHash>
#include <QFile>

class TypeConfig : public QObject
{
	Q_OBJECT

public:
	typedef QHash<QString, QString> TypeHas;
	
	TypeConfig(QObject *parent);
	~TypeConfig();
	
	TypeHas types() const;
	void openFile();

private:
	TypeHas m_typeHas;
	QFile m_file;
};

#endif // TYPECONFIG_H
