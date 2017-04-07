#ifndef MAPEXTRACTOR_H
#define MAPEXTRACTOR_H

#include <QtWidgets/QDialog>
#include "ui_mapextractor.h"

#include <QHash>

class MapExtractor : public QDialog
{
	Q_OBJECT

public:
	MapExtractor(QWidget *parent = 0);
	~MapExtractor();

public:
	void initSlot();

public slots:
	void openDialog();
	void process();

private:
	typedef struct _CityInfo{
		int clientID;
		float startLon;
		float endLon;
		float startLat;
		float endLat;
	}CityInfo;

	QHash<int,CityInfo> cityInfoHash;

private:
	Ui::MapExtractorClass ui;
};

#endif // MAPEXTRACTOR_H
