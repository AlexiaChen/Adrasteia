#include "AddStatDataSrc.h"
#include <QMessageBox>

AddStatDataSrc::AddStatDataSrc(DataTypeConfig::DataTypeHas & has, QWidget *parent /*= 0*/) : QDialog(parent)
{
	ui.setupUi(this);

	ui.addDataTypesCmbox->clear();

	for (int i = 0; i < has.keys().size(); ++i)
	{
		ui.addDataTypesCmbox->addItem(has.keys().at(i));
	}
	
	connect(ui.cancelDlg, SIGNAL(clicked()), this, SLOT(cancelDlg()));
	connect(ui.okDlg, SIGNAL(clicked()), this, SLOT(okDlg()));
	
}

AddStatDataSrc::~AddStatDataSrc()
{

}

void AddStatDataSrc::okDlg()
{
	QString current_data_type = ui.addDataTypesCmbox->currentText();
	QString current_srcfoler = ui.statSrcfolder->text();
	QString current_srcfileformat = ui.statSrcFileFormat->text();

	if (current_srcfoler.isEmpty() || current_srcfileformat.isEmpty())
	{
		QMessageBox::warning(this, QStringLiteral("警告"), QStringLiteral("配置不能为空"));
		return;
	}
	
	emit currentDataType(current_data_type,current_srcfoler,current_srcfileformat);
	this->close();
}

void AddStatDataSrc::cancelDlg()
{
	this->close();
}



