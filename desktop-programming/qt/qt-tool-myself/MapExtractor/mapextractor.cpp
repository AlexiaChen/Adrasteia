#include "mapextractor.h"

#include <QFileDialog>
#include <QString>
#include <cmath>

#include "HandleNwfdLib.h"

#pragma execution_character_set("utf-8")

bool AreSame(float a, float b) {
    return std::fabs(a - b) < std::numeric_limits<float>::epsilon();
}

MapExtractor::MapExtractor(QWidget* parent)
    : QDialog(parent) {
    ui.setupUi(this);

    initSlot();
}

MapExtractor::~MapExtractor() {

}

void MapExtractor::initSlot() {
    connect(ui.selectFile, SIGNAL(clicked()), this, SLOT(openDialog()));
    connect(ui.process, SIGNAL(clicked()), this, SLOT(process()));
}

void MapExtractor::openDialog() {
    QString path = QFileDialog::getOpenFileName(this, tr("Open Micaps"), ".", tr("Micaps Files(*.*)"));
    if (!path.isEmpty()) {
        ui.filePath->setText(path);
    }

}

void MapExtractor::process() {
    QString micapsFile = ui.filePath->text();

    
	ui.showInfo->clear();
	cityInfoHash.clear();
	
	ui.showInfo->append(QString("正在解析Micaps4文件，请稍等（file=%1）").arg(micapsFile));

    // 检测文件存不存在？
    QFileInfo fMicaps(micapsFile);
    if (!fMicaps.exists()) {
        ui.showInfo->append(QString("Micaps4文件不存在[%1]").arg(micapsFile));
        return;
    }

    QString strMicapsFileName = fMicaps.fileName();
    ui.showInfo->append(QString("文件名=%1").arg(strMicapsFileName));

    float* fNwfdData = NULL;
    grdpts stGrid;           // micaps4格式化数据
    // 打开Micaps4文件，获取数据和Micaps4格式数据
    fNwfdData = HandleNwfdLib::getClass().nwfd_openm4file(micapsFile.toLocal8Bit().data(), &stGrid);
    //i is lon , j lat
    int NLat = stGrid.Nj;
    int NLon = stGrid.Ni;
    int nLen = NLat*NLon;

    float DLon = stGrid.nlon;
    float DLat = stGrid.nlat;

    float start_lon = stGrid.lon1;
    float end_lon = stGrid.lon2;

    float start_lat = stGrid.lat1;
    float end_lat = stGrid.lat2;


	

    for (int k = 0; k < nLen; ++k)
    {
        if (!AreSame(fNwfdData[k], 9999.0f))
        {

            int client_id = static_cast<int>(fNwfdData[k]);

            int Lat_i = k / NLon;
            int Lon_i = k % NLon;

			float lon = start_lon + Lon_i*DLon;
            float lat = start_lat + Lat_i*DLat;

			if (!cityInfoHash.contains(client_id))
            {
                CityInfo inf;

				inf.clientID = client_id;
				inf.startLon = lon;
                inf.endLon = lon;
                inf.startLat = lat;
                inf.endLat = lat;

                cityInfoHash.insert(client_id, inf);
			}
			else
			{
				if (cityInfoHash[client_id].startLon > lon)
				{
					cityInfoHash[client_id].startLon = lon;
				}

				if (cityInfoHash[client_id].endLon < lon)
				{
					cityInfoHash[client_id].endLon = lon;
				}

				if (cityInfoHash[client_id].startLat > lat)
				{
					cityInfoHash[client_id].startLat = lat;
				}

				if (cityInfoHash[client_id].endLat < lat)
				{
					cityInfoHash[client_id].endLat = lat;
				}
				
			}









        }
    }




    //处理结束
    if (fNwfdData) {
        HandleNwfdLib::getClass().nwfd_freefld(fNwfdData);
        fNwfdData = NULL;
    }

	ui.showInfo->append("Micaps4文件解析结束，获得数据");

	QString provinceBuff = QString("整个省的起始经纬度: startLon:%1 ; endLon:%2 ; startLat:%3 ; endLat:%4").arg(start_lon)
		.arg(end_lon).arg(start_lat).arg(end_lat);
	
	ui.showInfo->append(provinceBuff);

	for (auto iter = cityInfoHash.begin(); iter != cityInfoHash.end(); ++iter)
	{
		
		float lon1 = AreSame(fmodf(iter->startLon, DLon), 0.0f) ? iter->startLon - DLon : (int)(iter->startLon / DLon) * DLon - DLon;
		float lon2 = AreSame(fmodf(iter->endLon, DLon), 0.0f) ? iter->endLon + DLon : ((int)(iter->endLon / DLon) + 1) * DLon + DLon;
		float lat1 = AreSame(fmodf(iter->startLat, DLat), 0.0f) ? iter->startLat - DLat : (int)(iter->startLat / DLat) * DLat - DLat;
		float lat2 = AreSame(fmodf(iter->endLat, DLat), 0.0f) ? iter->endLat + DLat : ((int)(iter->endLat / DLat) + 1) * DLat + DLat;


		
		QString buffer = QString("client ID: %1 ; startLon:%2 ; endLon:%3 ; startLat:%4 ; \
			endLat:%5 ;").arg(iter->clientID).arg(QString::number(lon1, 'f', 2)).arg(QString::number(lon2, 'f', 2)).
arg(QString::number(lat1, 'f', 2)).arg(QString::number(lat2, 'f', 2));

		ui.showInfo->append(buffer);
	}






}
