#include "UpdateStatDataSrc.h"

UpdateStatDataSrc::UpdateStatDataSrc(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	connect(ui.okDlg, SIGNAL(clicked()), this, SLOT(okDlg()));
	connect(ui.cancelDlg, SIGNAL(clicked()), this, SLOT(cancelDlg()));
}

UpdateStatDataSrc::~UpdateStatDataSrc()
{

}

void UpdateStatDataSrc::okDlg()
{
	QString srcfolder = ui.statSrcfolder->text();
	QString srcfileformat = ui.statSrcFileFormat->text();

	emit UpdateItemInfo(srcfolder, srcfileformat);
	
	this->close();
}

void UpdateStatDataSrc::cancelDlg()
{
	this->close();
}

void UpdateStatDataSrc::initStatDataSrc(QString& srcfolder, QString& srcfileformat)
{
	ui.statSrcfolder->setText(srcfolder);
	ui.statSrcFileFormat->setText(srcfileformat);
}
