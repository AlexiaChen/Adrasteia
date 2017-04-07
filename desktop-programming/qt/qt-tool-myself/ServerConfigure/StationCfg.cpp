#include "StationCfg.h"

#include <QDir>
#include <QDomNode>
#include <QDomText>
#include <QTextStream>

StationCfg::StationCfg(QString& path, QObject *parent)
	: cfgPath(path),QObject(parent),m_isInvalid(false)
{

}

StationCfg::~StationCfg()
{

}

bool StationCfg::isInvalid() const
{
	return m_isInvalid;
}

void StationCfg::saveToFile()
{
	m_xmlFile.resize(0);
	QTextStream out(&m_xmlFile);
	out.setCodec("UTF-8");
	m_doc.save(out, 4, QDomNode::EncodingFromTextStream);

	m_xmlFile.close();
	m_isInvalid = true;

	openFile();
}


void StationCfg::openFile()
{
	QString xmlPath = cfgPath + QString("/StationConfig.xml");
	m_xmlFile.setFileName(xmlPath);

	if (!m_xmlFile.open(QFile::ReadWrite | QFile::Text)) {
		printf("open server config file failed\n");
		m_isInvalid = true;
	}

	QString         strError;
	int             errLin = 0, errCol = 0;
	if (!m_doc.setContent(&m_xmlFile, false, &strError, &errLin, &errCol)) {
		printf("parse file failed at line %d column %d, error: %s !\n", errLin, errCol, strError);
		m_isInvalid = true;
	}

	if (m_doc.isNull()) {
		printf("document is null !\n");
		m_isInvalid = true;
	}

	m_root = m_doc.documentElement();
	if (m_root.tagName().compare(QString("config"), Qt::CaseSensitive) != 0)
	{
		m_isInvalid = true;
	}
}


void StationCfg::getStationList(StationList& raw, StationList& merge)
{
	QDomElement stationEle = m_root.firstChildElement("Stations").firstChildElement("Station");

	while (!stationEle.isNull())
	{
		
		
		if (stationEle.attribute("role") == "raw")
		{
			
			STStation station;
			//header
			station.role = stationEle.attribute("role");
			station.symbol = stationEle.attribute("Symbol");
			station.cityType = stationEle.attribute("cityType");
			station.timeRange = stationEle.attribute("TimeRange");
			station.maxForecastTime = stationEle.attribute("MaxForecastTime");

			// scantime
			QDomElement scanTimeEle = stationEle.firstChildElement("scantime");
			if (!scanTimeEle.isNull())
			{
				station.scanTime.interval = scanTimeEle.attribute("interval");

				QDomElement scanTimechildEle = scanTimeEle.firstChildElement("time");
				while (!scanTimechildEle.isNull())
				{
					Time time;
					time.hour = scanTimechildEle.attribute("hour");
					time.min = scanTimechildEle.attribute("min");
					time.second = scanTimechildEle.attribute("second");

					station.scanTime.timeList.push_back(time);
					scanTimechildEle = scanTimechildEle.nextSiblingElement();
				}
			}

			//Times
			QDomElement timeEle = stationEle.firstChildElement("Times").firstChildElement("time");
			while (!timeEle.isNull())
			{
				STTime stTime;
				stTime.reportHour = timeEle.attribute("reportHour");
				stTime.stationList = timeEle.attribute("stationlist");
				stTime.rule = timeEle.attribute("rule");
				stTime.saveType = timeEle.attribute("saveType");

				QDomElement savefileEle = timeEle.firstChildElement("savefile");
				if (!savefileEle.isNull())
				{
					stTime.saveFile.bDisable = savefileEle.attribute("disabled").toLower() == "yes";
					stTime.saveFile.savepath = savefileEle.attribute("savepath");
					stTime.saveFile.filename = savefileEle.attribute("filename");
					stTime.saveFile.dds = savefileEle.attribute("dds");

				}
				else
				{
					stTime.saveFile.bDisable = true;
				}

				station.times.timeList.push_back(stTime);

				timeEle = timeEle.nextSiblingElement();
			}

			//data Src
			QDomElement itemEle = stationEle.firstChildElement("datasrc").firstChildElement("item");
			while (!itemEle.isNull())
			{
				StationDataSrcItem item_t;
				item_t.dataType = itemEle.attribute("dataType");
				item_t.symbol = itemEle.attribute("Symbol");
				item_t.Description = itemEle.attribute("Description");
				item_t.srcfolder = itemEle.attribute("srcfolder");
				item_t.srcfileformat = itemEle.attribute("srcfileformat");

				station.dataSrc.dataSrcItemList.push_back(item_t);
				itemEle = itemEle.nextSiblingElement();
			}

			//save cached

			QDomElement saveCachedEle = stationEle.firstChildElement("savecached");
			if (!saveCachedEle.isNull())
			{
				station.savecached.bDisable = saveCachedEle.attribute("disabled").toLower() == "yes";
				station.savecached.validtime = saveCachedEle.attribute("validtime");
			}
			else
			{
				station.savecached.bDisable = true;
			}



			raw.push_back(station);
		}
		else if (stationEle.attribute("role") == "merge")
		{
			STStation station;
			//header
			station.role = stationEle.attribute("role");
			station.symbol = stationEle.attribute("Symbol");
			station.cityType = stationEle.attribute("cityType");
			station.timeRange = stationEle.attribute("TimeRange");
			station.maxForecastTime = stationEle.attribute("MaxForecastTime");

			// scantime
			QDomElement scanTimeEle = stationEle.firstChildElement("scantime");
			if (!scanTimeEle.isNull())
			{
				station.scanTime.interval = scanTimeEle.attribute("interval");

				QDomElement scanTimechildEle = scanTimeEle.firstChildElement("time");
				while (!scanTimechildEle.isNull())
				{
					Time time;
					time.hour = scanTimechildEle.attribute("hour");
					time.min = scanTimechildEle.attribute("min");
					time.second = scanTimechildEle.attribute("second");

					station.scanTime.timeList.push_back(time);
					scanTimechildEle = scanTimechildEle.nextSiblingElement();
				}
			}

			//Times
			QDomElement timeEle = stationEle.firstChildElement("Times").firstChildElement("time");
			while (!timeEle.isNull())
			{
				STTime stTime;
				stTime.reportHour = timeEle.attribute("reportHour");
				stTime.stationList = timeEle.attribute("stationlist");
				stTime.rule = timeEle.attribute("rule");
				stTime.saveType = timeEle.attribute("saveType");

				QDomElement savefileEle = timeEle.firstChildElement("savefile");
				if (!savefileEle.isNull())
				{
					stTime.saveFile.bDisable = savefileEle.attribute("disabled").toLower() == "yes";
					stTime.saveFile.savepath = savefileEle.attribute("savepath");
					stTime.saveFile.filename = savefileEle.attribute("filename");
					stTime.saveFile.dds = savefileEle.attribute("dds");

				}
				else
				{
					stTime.saveFile.bDisable = true;
				}

				station.times.timeList.push_back(stTime);

				timeEle = timeEle.nextSiblingElement();
			}

			//data Src
			QDomElement itemEle = stationEle.firstChildElement("datasrc").firstChildElement("item");
			while (!itemEle.isNull())
			{
				StationDataSrcItem item_t;
				item_t.dataType = itemEle.attribute("dataType");
				item_t.symbol = itemEle.attribute("Symbol");
				item_t.Description = itemEle.attribute("Description");
				item_t.srcfolder = itemEle.attribute("srcfolder");
				item_t.srcfileformat = itemEle.attribute("srcfileformat");

				station.dataSrc.dataSrcItemList.push_back(item_t);
				itemEle = itemEle.nextSiblingElement();
			}

			//save cached

			QDomElement saveCachedEle = stationEle.firstChildElement("savecached");
			if (!saveCachedEle.isNull())
			{
				station.savecached.bDisable = saveCachedEle.attribute("disabled").toLower() == "yes";
				station.savecached.validtime = saveCachedEle.attribute("validtime");
			}
			else
			{
				station.savecached.bDisable = true;
			}



			merge.push_back(station);
		}
		
		

		stationEle = stationEle.nextSiblingElement();
	}
}

void StationCfg::getStationManage(STStationManage &mng)
{
	QDomElement stationMngEle = m_root.firstChildElement("stationmange");
	mng.stationListPath = stationMngEle.attribute("stationlist");
	mng.cityType = stationMngEle.attribute("cityType");
	mng.forecastType = stationMngEle.attribute("forecastType");
	mng.radius = stationMngEle.attribute("radius");
	mng.station2gridDisbale = stationMngEle.attribute("station2grid") == "yes" ? false : true;
	mng.publishDisable = stationMngEle.attribute("publish") == "yes" ? false : true;
	mng.validTime = stationMngEle.attribute("validtime");


}


void StationCfg::setStationList(StationList& raw, StationList& merge)
{
	//É¾³ýDom Tree
	QDomElement stationmangeEle = m_root.firstChildElement("stationmange");
	m_root.removeChild(stationmangeEle);
	QDomElement stationsEle = m_root.firstChildElement("Stations");
	m_root.removeChild(stationsEle);

	stationmangeEle = m_doc.createElement("stationmange");
	QDomText text = m_doc.createTextNode("");
	stationmangeEle.appendChild(text);
	m_root.appendChild(stationmangeEle);
	
	stationsEle = m_doc.createElement("Stations");
	text = m_doc.createTextNode("");
	stationsEle.appendChild(text);
	m_root.appendChild(stationsEle);

	//ÖØ½¨Dom Tree
	
	//add manager
	addStationMngToTree(stationmangeEle, m_mng);
	//add raw
	addRawStationsToTree(stationsEle, raw);
	//add merge
	addMergeStationsToTree(stationsEle, merge);
}

void StationCfg::setStationManage(STStationManage &mng)
{
	m_mng = mng;
}

void StationCfg::addStationMngToTree(QDomElement& mngEle, STStationManage& mng)
{
	mngEle.setAttribute("stationlist", mng.stationListPath);
	mngEle.setAttribute("cityType", mng.cityType);
	mngEle.setAttribute("forecastType", mng.forecastType);
	mngEle.setAttribute("radius", mng.radius);
	if (!mng.station2gridDisbale) mngEle.setAttribute("station2grid", "yes");
	if (!mng.publishDisable)      mngEle.setAttribute("publish", "yes");
	mngEle.setAttribute("validtime", mng.validTime);
	 
}

void StationCfg::addRawStationsToTree(QDomElement& stations, StationList& raw)
{
	for (int i = 0; i < raw.size(); ++i)
	{
		QDomElement station = m_doc.createElement("Station");
		STStation stat = raw.at(i);
		
		//add header
		addStationHeader(station, stat);

		//add scan time
		addRawScanTime(station, stat);

		//add data src
		addDataSrc(station, stat);

		if (!stat.savecached.bDisable)
		{
			addSaveCached(station, stat);
		}

		stations.appendChild(station);
	}
}

void StationCfg::addMergeStationsToTree(QDomElement& stations, StationList& merge)
{
	for (int i = 0; i < merge.size(); ++i)
	{
		QDomElement station = m_doc.createElement("Station");
		STStation stat = merge.at(i);
		
		//add header
		addStationHeader(station, stat);
		//add san time
		addMergeScanTime(station, stat);

		//add times
		addTimes(station, stat);

		stations.appendChild(station);
	}
}

void StationCfg::addStationHeader(QDomElement& station, STStation &stat)
{
	station.setAttribute("role", stat.role);
	station.setAttribute("Symbol", stat.symbol);
	station.setAttribute("cityType", stat.cityType);
	station.setAttribute("TimeRange", stat.timeRange);
	station.setAttribute("MaxForecastTime", stat.maxForecastTime);
}

void StationCfg::addRawScanTime(QDomElement& station, STStation& stat)
{
	QDomElement scantimeEle = m_doc.createElement("scantime");

	scantimeEle.setAttribute("interval", stat.scanTime.interval);

	station.appendChild(scantimeEle);
}

void StationCfg::addMergeScanTime(QDomElement& station, STStation& stat)
{
	QDomElement scantimeEle = m_doc.createElement("scantime");

	for (int i = 0; i < stat.scanTime.timeList.size(); ++i)
	{
		QDomElement timeEle = m_doc.createElement("time");
		timeEle.setAttribute("hour", stat.scanTime.timeList.value(i).hour);
		timeEle.setAttribute("min", stat.scanTime.timeList.value(i).min);
		timeEle.setAttribute("second", stat.scanTime.timeList.value(i).second);
		scantimeEle.appendChild(timeEle);
	}

	station.appendChild(scantimeEle);
}


void StationCfg::addSaveCached(QDomElement& station, STStation& stat)
{
	QDomElement savecachedEle = m_doc.createElement("savecached");
	savecachedEle.setAttribute("validtime", stat.savecached.validtime);
	station.appendChild(savecachedEle);
}

void StationCfg::addDataSrc(QDomElement& station, STStation& stat)
{
	QDomElement dataSrcEle = m_doc.createElement("datasrc");

	for (int i = 0; i < stat.dataSrc.dataSrcItemList.size(); ++i)
	{
		QDomElement itemEle = m_doc.createElement("item");

		itemEle.setAttribute("dataType", stat.dataSrc.dataSrcItemList.value(i).dataType);
		itemEle.setAttribute("Symbol", stat.dataSrc.dataSrcItemList.value(i).symbol);
		itemEle.setAttribute("Description", stat.dataSrc.dataSrcItemList.value(i).Description);
		itemEle.setAttribute("srcfolder", stat.dataSrc.dataSrcItemList.value(i).srcfolder);
		itemEle.setAttribute("srcfileformat", stat.dataSrc.dataSrcItemList.value(i).srcfileformat);
		
		dataSrcEle.appendChild(itemEle);
	}


	station.appendChild(dataSrcEle);
}

void StationCfg::addTimes(QDomElement& station, STStation& stat)
{
	QDomElement timesEle = m_doc.createElement("Times");

	for (int i = 0; i < stat.times.timeList.size(); ++i)
	{

		QDomElement timeEle = m_doc.createElement("time");
		timeEle.setAttribute("reportHour", stat.times.timeList.value(i).reportHour);
		timeEle.setAttribute("stationlist", stat.times.timeList.value(i).stationList);
		timeEle.setAttribute("rule", stat.times.timeList.value(i).rule);
		timeEle.setAttribute("saveType", stat.times.timeList.value(i).saveType);
		
		//add save file
		if (!stat.times.timeList.value(i).saveFile.bDisable)
		{
			addSaveFile(timeEle, stat.times.timeList.value(i).saveFile);
		}
		
		
		timesEle.appendChild(timeEle);
	}

	station.appendChild(timesEle);
}

void StationCfg::addSaveFile(QDomElement& time, SaveFile& savefile)
{
	QDomElement savefileEle = m_doc.createElement("savefile");

	savefileEle.setAttribute("savepath", savefile.savepath);
	savefileEle.setAttribute("filename",savefile.filename);
	savefileEle.setAttribute("dds", savefile.dds);
	
	time.appendChild(savefileEle);
}
