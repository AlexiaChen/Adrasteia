#include "Config.h"
#include "InitWnd.h"
#include <QtWidgets/QApplication>
#include <QTextCodec>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QRegExp>

//历史记录
static bool read_history_lonlat(QString& lon1, QString&lon2,QString& lat1,QString &lat2){

	QString history_path = QDir::currentPath() + QString("/history.cfg");
	QFile history_file;
	history_file.setFileName(history_path);

	if (!history_file.open(QFile::ReadOnly | QFile::Text)) {
		printf("oepn history file failed\n");
		history_file.close();
		return false;
	}

	QTextStream history_stream(&history_file);

	while (!history_stream.atEnd())
	{
		QString lonlat_line = history_stream.readLine();
		
		if (lonlat_line.isEmpty())
		{
			history_file.close();
			return false;
		}
		
		lon1 = lonlat_line.split(QRegExp("\\s+"))[0];
		lon2 = lonlat_line.split(QRegExp("\\s+"))[1];
		lat1 = lonlat_line.split(QRegExp("\\s+"))[2];
		lat2 = lonlat_line.split(QRegExp("\\s+"))[3];

	}

	return true;
}


static void save_history_lonlat(const QString&lon1,const QString&lon2,const QString& lat1, const QString&lat2){

	QString history_path = QDir::currentPath() + QString("/history.cfg");
	QFile history_file;
	history_file.setFileName(history_path);

	if (!history_file.open(QFile::WriteOnly | QFile::Text)) {
		printf("oepn history file failed\n");
		history_file.close();
		
	}

	QTextStream history_stream(&history_file);

	history_stream << lon1 << "  "  << lon2 << "  "<< lat1 << " " << lat2;

	history_file.flush();
	history_file.close();
}

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	
	
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
	
	
	QString lon1, lon2, lat1, lat2;
	read_history_lonlat(lon1, lon2, lat1, lat2);
	
	InitWnd initwnd;
	initwnd.setWindowTitle(QStringLiteral("软件初始化设置"));
	initwnd.setLonAndLat(lon1, lon2, lat1, lat2);
	initwnd.exec();
	
	QString cfgPath = initwnd.getCfgPath();
	
	
	
	initwnd.getLonAndLat(lon1, lon2, lat1, lat2);
	
	if (cfgPath.isEmpty() || lon1.isEmpty() || lon2.isEmpty() || lat1.isEmpty() || lat2.isEmpty())
	{
		return 0;
	}

	save_history_lonlat(lon1, lon2, lat1, lat2);

	Config w(cfgPath);
	w.setWindowTitle(QStringLiteral("Lwfd配置主界面"));
	w.initLanLat(lon1.toFloat(), lon2.toFloat(), lat1.toFloat(), lat2.toFloat());
	w.show();
	
	return a.exec();
}
