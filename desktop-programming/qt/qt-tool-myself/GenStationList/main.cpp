
#include <QtCore/QCoreApplication>
#include <QString>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QStringList>
#include <QVector>
#include <QMap>
#include <QDebug>
#include <QTextCodec>

#pragma execution_character_set("utf-8")

QVector<QString> provinceName;
QVector<QString> stationAllLines;
QVector<QString> stationTimeAllLines;

QVector<QString> findSubListByProvinceName(const QVector<QString> &allLines, const QString& name)
{
	QVector<QString> newList;

	for (const QString& line : allLines)
	{
		if (line.contains(name))
		{
			newList.push_back(line);
		}
	}

	return newList;

}



QString findStationByStationNum(const QVector<QString> &allLines, int num)
{


	for (const QString& line : allLines)
	{
		
		// split with white spaces
		if (line.split(QRegExp("\\s+"))[0].toInt() == num)
		{
			return line;
		}
	}

	return "";

}

int getStationNumPerLine(const QString& line)
{
	return line.split(",")[0].toInt();

}

QVector<int> findStationNumListByTimesAndProvinceName(const QVector<QString> &allLines, const QString& name,
	const QString& time)
{
	QVector<int> newList;

	for (const QString& line : allLines)
	{
		
		
		if (line.contains(name) && line.split(",")[4] == time)
		{
			newList.push_back(getStationNumPerLine(line));
		}
	}

	return newList;

}

void saveToFile(const QString& filename,  QVector<QString> & qstream)
{
	QFile file(filename);
	if (!file.open(QFile::WriteOnly | QFile::Text))
	{

		return;
	}

	QTextStream out(&file);
	
	for (const QString& line : qstream)
	{
		out << line << endl;
	}

	out.flush();

	file.close();
}


void readStationList(const QString& filename)
{
	QFile file(filename);
	
	if (!file.open(QFile::ReadOnly | QFile::Text))
	{
		
		return;
	}
	
	QTextStream out(&file);
	
	QString currLine;
	
	
	while (!out.atEnd()){
	
		
		 currLine = out.readLine();

	
		 stationAllLines.push_back(currLine);
		 
		
	
	}

	qDebug() << "read station complete";
	
	file.close();

	
	QString output("C:/Users/MathxH/Desktop/output/");
	
	for (const QString &name : provinceName)
	{
		QVector<QString> text = findSubListByProvinceName(stationAllLines, name);

		saveToFile(output + name + "_station.dat", text);
	}

	
	
	
	qDebug() << "gen station complete";
}


void processTime(const QVector<int>& stationNum,const QString& province_name,int time_num)
{
	
	QString output("C:/Users/MathxH/Desktop/output/");
	
	QVector<QString> text;

	for (int num : stationNum)
	{
		
		QString station = findStationByStationNum(stationAllLines, num);
		
		if (station != "")
		{
			text.push_back(station);
		}
		else
		{
			
			//没有该时次的站点
			//text.push_back(QString(num) + QString(""));
		}
	}

	
	
	saveToFile(output + province_name + QString("_townstation") + QString::number(time_num, 10) +".dat", text);

}


void readStationTimes(const QString& filename)
{

	QFile file(filename);

	if (!file.open(QFile::ReadOnly | QFile::Text))
	{

		return;
	}

	QTextStream out(&file);

	QString currLine;


	while (!out.atEnd()){


		currLine = out.readLine();


		stationTimeAllLines.push_back(currLine);



	}

	qDebug() << "read station times complete";

	file.close();

	//处理每个省的5个不同时次

	

	
	QVector<int> time1;
	QVector<int> time2;
	QVector<int> time3;
	QVector<int> time4;
	QVector<int> time5;
	
	for (const QString &name : provinceName)
	{
		time1 = findStationNumListByTimesAndProvinceName(stationTimeAllLines, name, "1");
		time2 = findStationNumListByTimesAndProvinceName(stationTimeAllLines, name, "2");
		time3 = findStationNumListByTimesAndProvinceName(stationTimeAllLines, name, "3");
		time4 = findStationNumListByTimesAndProvinceName(stationTimeAllLines, name, "4");
		time5 = findStationNumListByTimesAndProvinceName(stationTimeAllLines, name, "5");


		processTime(time1,name ,1);
		processTime(time2,name ,2);
		processTime(time3,name, 3);
		processTime(time4,name ,4);
		processTime(time5,name, 5);
		
	}

	qDebug() << "gen station times complete";

}

void init()
{
	provinceName.push_back("北京");
	provinceName.push_back("天津");

	provinceName.push_back("河北");

	provinceName.push_back("山西");

	provinceName.push_back("内蒙古");
	provinceName.push_back("辽宁");

	provinceName.push_back("吉林");
	provinceName.push_back("黑龙江");
	provinceName.push_back("上海");

	provinceName.push_back("江苏");

	provinceName.push_back("浙江");

	provinceName.push_back("安徽");

	provinceName.push_back("福建");
	provinceName.push_back("江西");

	provinceName.push_back("山东");

	provinceName.push_back("河南");

	provinceName.push_back("湖北");

	provinceName.push_back("湖南");

	provinceName.push_back("广东");
	provinceName.push_back("广西");

	provinceName.push_back("海南");

	provinceName.push_back("重庆");

	provinceName.push_back("四川");

	provinceName.push_back("贵州");
		
	provinceName.push_back("云南");

	provinceName.push_back("西藏");

	provinceName.push_back("陕西");

	provinceName.push_back("甘肃");

	provinceName.push_back("青海");

	provinceName.push_back("宁夏");
	provinceName.push_back("新疆");

	


}


int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
	QString stationlist("C:/Users/MathxH/Desktop/STATIONS.DAT");
	QString stationtimes("C:/Users/MathxH/Desktop/nwfd站点.txt");

	init();
	
	readStationList(stationlist);
	readStationTimes(stationtimes);

	return a.exec();
}
