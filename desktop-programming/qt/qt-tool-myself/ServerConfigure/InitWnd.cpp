#include "InitWnd.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>

InitWnd::InitWnd(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	connect(ui.initDlgOpenPath, SIGNAL(clicked()), this, SLOT(openCfgPath()));
	connect(ui.initDlgOk, SIGNAL(clicked()), this, SLOT(okInitDlg()));
	connect(ui.initDlgCancel, SIGNAL(clicked()), this, SLOT(cancelInitDlg()));
	

	
}

InitWnd::~InitWnd()
{

}

void InitWnd::openCfgPath()
{
	
	
	cfgPath = QFileDialog::getExistingDirectory(this, QStringLiteral("配置文件目录"));
	
	ui.cfgFilePath->setText(cfgPath);
}

void InitWnd::okInitDlg()
{
	
	if (cfgPath.isEmpty() || ui.initDlglon1->text().isEmpty() ||
		ui.initDlglon2->text().isEmpty() ||
		ui.initDlglat1->text().isEmpty() ||
		ui.initDlglat2->text().isEmpty())
	{
		QMessageBox::warning(this, QStringLiteral("警告"), QStringLiteral("配置不能为空"), QMessageBox::Ok);
		return;
	}


	//检查配置文件是否存在
	QFile product_xml(cfgPath + "/ProductConfig.xml"), client_xml(cfgPath + "/ClientConfig.xml"), 
		station_xml(cfgPath + "/StationConfig.xml");
	
	if (!product_xml.exists() || !client_xml.exists() || !station_xml.exists())
	{
		QMessageBox::warning(this, QStringLiteral("警告"), QStringLiteral("配置路径下不存在配置文件，请检查！"));
		return;
	}

	float lon1 = ui.initDlglon1->text().toFloat();
	float lon2 = ui.initDlglon2->text().toFloat();

	float lat1 = ui.initDlglat1->text().toFloat();
	float lat2 = ui.initDlglat2->text().toFloat();

	if (lon1 > lon2 || lat1 > lat2)
	{
		QMessageBox::warning(this, QStringLiteral("警告"), QStringLiteral("终止经纬度一定比起始经纬度大！"), QMessageBox::Ok);
		return;
	}

	
	this->close();
}

void InitWnd::cancelInitDlg()
{
	this->close();
}

QString InitWnd::getCfgPath() const
{
	return cfgPath;
}




void InitWnd::getLonAndLat(QString &lon1, QString &lon2, QString &lat1, QString &lat2)
{
	lon1 = ui.initDlglon1->text();
	lon2 = ui.initDlglon2->text();
	lat1 = ui.initDlglat1->text();
	lat2 = ui.initDlglat2->text();
}

void InitWnd::setLonAndLat(QString &lon1, QString &lon2, QString &lat1, QString &lat2)
{
	ui.initDlglon1->setText(lon1);
	ui.initDlglon2->setText(lon2);
	ui.initDlglat1->setText(lat1);
	ui.initDlglat2->setText(lat2);
}
