#ifndef STATIONCFG_H
#define STATIONCFG_H

#include <QObject>
#include <QString>
#include <QDomDocument>
#include <QDomElement>
#include <QVector>
#include <QFile>

#include "ProductCfg.h"

typedef struct _STTime{
	QString reportHour;
	QString stationList;
	QString rule;
	QString saveType;
	SaveFile saveFile;
}STTime;

typedef QVector<STTime> STTimeList;

typedef struct _Times{
	STTimeList timeList;
}Times;

typedef struct _StationDataSrcItem{
	int searchKey;
	QString dataType;
	QString symbol;
	QString Description;
	QString srcfolder;
	QString srcfileformat;
}StationDataSrcItem;

typedef QVector<StationDataSrcItem> DataSrcItemList;

typedef struct _StationDataSrc{
	DataSrcItemList dataSrcItemList;
}StationDataSrc;

typedef struct _StationCfg
{
	int searchKey;   //for change Station data
	QString role;
	QString symbol;
	QString cityType;
	QString timeRange;
	QString maxForecastTime;
	ScanTime scanTime;
	StationDataSrc dataSrc;
	CachedSave savecached;
	Times times;
}STStation;


typedef struct _StationManage{
	QString stationListPath;
	QString cityType;
	QString forecastType;
	QString radius;
	bool station2gridDisbale;
	bool publishDisable;
	QString validTime;
}STStationManage;

typedef QVector<STStation> StationList;

class StationCfg : public QObject
{
	Q_OBJECT

public:
	StationCfg(QString& path, QObject *parent);
	~StationCfg();

	bool isInvalid() const;
	void saveToFile();
	void openFile();

	//getter

	void getStationList(StationList& raw, StationList& merge);
	void getStationManage(STStationManage &mng);


	//setter
	void setStationList(StationList& raw, StationList& merge);
	void setStationManage(STStationManage &mng);

private:
	void addStationMngToTree(QDomElement& mngEle,STStationManage& mng);
	void addRawStationsToTree(QDomElement& stations, StationList& raw);
	void addMergeStationsToTree(QDomElement& stations, StationList& merge);

	/////////////////////////////
	void addStationHeader(QDomElement& station, STStation &stat);
	void addRawScanTime(QDomElement& station, STStation& stat);
	void addMergeScanTime(QDomElement& station, STStation& stat);
	void addSaveCached(QDomElement& station, STStation& stat);
	void addDataSrc(QDomElement& station, STStation& stat);
	void addTimes(QDomElement& station, STStation& stat);
	void addSaveFile(QDomElement& time, SaveFile& savefile);

private:
	QString cfgPath;
	bool m_isInvalid;
	QDomDocument m_doc;
	QDomElement m_root;
	QFile m_xmlFile;
	STStationManage m_mng;

};

#endif // STATIONCFG_H
