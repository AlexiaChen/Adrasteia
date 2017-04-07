#include "updateStatDlg.h"

#include <QMessageBox>

updateStatDlg::updateStatDlg(DataTypeConfig::DataTypeHas & has,QWidget *parent /*= 0*/) : QDialog(parent)
{
	ui.setupUi(this);
	
	m_dataTypeHas = has;

	m_addDataSrcDlg = new AddStatDataSrc(m_dataTypeHas,this);
	m_addDataSrcDlg->setWindowTitle(QStringLiteral("添加"));
	m_addDataSrcDlg->setModal(true);

	m_updateDataSrcDlg = new UpdateStatDataSrc(this);
	m_updateDataSrcDlg->setModal(true);

	ui.dataSrcTab->horizontalHeader()->setStretchLastSection(true);
	
	connect(ui.addDataSrcItem, SIGNAL(clicked()), this, SLOT(addItem()));
	connect(ui.deleteDataSrcItem, SIGNAL(clicked()), this, SLOT(deleteItem()));
	connect(ui.updateDataSrcItem, SIGNAL(clicked()), this, SLOT(updateItem()));
	connect(ui.updateStatDlgok, SIGNAL(clicked()), this, SLOT(okDlg()));
	connect(ui.updateStatDlgCancel, SIGNAL(clicked()), this, SLOT(cancelDlg()));
	connect(ui.dataSrcTab, SIGNAL(cellClicked(int, int)), this, SLOT(selectItemOneRow(int, int)));
	connect(m_addDataSrcDlg, SIGNAL(currentDataType(QString&,QString&,QString&)), this, SLOT(addItem(QString&,QString&,QString&)));
	
	connect(m_updateDataSrcDlg, SIGNAL(UpdateItemInfo(QString&,QString&)), this, SLOT(updateItemMsg(QString&, QString&)));
	
	
}

updateStatDlg::~updateStatDlg()
{
	if (m_addDataSrcDlg) delete m_addDataSrcDlg;
	if (m_updateDataSrcDlg) delete m_updateDataSrcDlg;
	
}


void updateStatDlg::initStation(StationList& raw, int rawIndex)
{
	if (rawIndex != -1)
	{
		
		m_rawIndex = rawIndex;
		m_rawlist = raw;
		STStation station = raw.at(rawIndex);

		
		//clear data src table
		clearDataSrcTab();


		//add data src
		for (int i = 0; i < station.dataSrc.dataSrcItemList.size(); ++i)
		{
			
			StationDataSrcItem item = station.dataSrc.dataSrcItemList.at(i);
			
			QString datatype = m_dataTypeHas.key(item.dataType);
			insertDataSrcItem(datatype, item.srcfolder, item.srcfileformat);
		}

		ui.dataSrcTab->resizeColumnsToContents();
	}
}

Ui::updateStatDlg updateStatDlg::getUpdateStatDlgUI()
{
	return ui;
}

void updateStatDlg::addItem()
{
	
	m_addDataSrcDlg->show();
	
}

void updateStatDlg::addItem(QString& dataType, QString& srcfolder, QString& srcfileformat)
{
	auto item_list = ui.dataSrcTab->findItems(dataType, Qt::MatchCaseSensitive);

	if (!item_list.isEmpty())
	{
		QMessageBox::warning(this, QStringLiteral("警告"), QStringLiteral("添加的数据类型重复"));
		return;
	}

	

	insertDataSrcItem(dataType, srcfolder, srcfileformat);
}

void updateStatDlg::deleteItem()
{
	int currentRow = ui.dataSrcTab->currentRow();
	if (currentRow == -1)
	{
		return;
	}

	ui.dataSrcTab->removeRow(currentRow);
}

void updateStatDlg::okDlg()
{
	
	emit updateStation(m_rawIndex);
	this->close();
}

void updateStatDlg::cancelDlg()
{

	this->close();
}

void  updateStatDlg::clearDataSrcTab()
{
	while (ui.dataSrcTab->rowCount())
	{
		ui.dataSrcTab->removeRow(0);
	}


}



void updateStatDlg::insertDataSrcItem(QString& dataType, QString &srcfolder, QString& fileformat)
{
	
	int rowNum = ui.dataSrcTab->rowCount();
	
	ui.dataSrcTab->insertRow(rowNum);

	QTableWidgetItem *item = new QTableWidgetItem(dataType);
	item->setFlags(item->flags() ^ Qt::ItemIsEditable); //set item not editable
	ui.dataSrcTab->setItem(rowNum, 0, item);

	item = new QTableWidgetItem(srcfolder);
	item->setFlags(item->flags() ^ Qt::ItemIsEditable); //set item not editable
	ui.dataSrcTab->setItem(rowNum, 1, item);

	item = new QTableWidgetItem(fileformat);
	item->setFlags(item->flags() ^ Qt::ItemIsEditable); //set item not editable
	ui.dataSrcTab->setItem(rowNum, 2, item);

}

void updateStatDlg::selectItemOneRow(int row,int column)
{
	ui.dataSrcTab->selectRow(row);
}

void updateStatDlg::updateItem()
{
	

	QString name,srcfolder, srcfileformat;
	int current_row = ui.dataSrcTab->currentRow();
	if (current_row != -1)
	{
		getDataSrcItemByRow(current_row, name, srcfolder, srcfileformat);
		
		m_updateDataSrcDlg->initStatDataSrc(srcfolder, srcfileformat);
		m_updateDataSrcDlg->setWindowTitle(name + QStringLiteral("配置修改"));
		m_updateDataSrcDlg->show();
		
	}
	

}

void updateStatDlg::updateItemMsg(QString& srcfolder, QString& srcfileformat)
{
	setDataSrcItemByRow(ui.dataSrcTab->currentRow(), srcfolder, srcfileformat);
}

void updateStatDlg::getDataSrcItemByRow(int row, QString& name, QString& srcfolder, QString& fileformat)
{
	
	name = ui.dataSrcTab->item(row, 0)->text();
	srcfolder = ui.dataSrcTab->item(row, 1)->text();
	fileformat = ui.dataSrcTab->item(row, 2)->text();
}

void updateStatDlg::setDataSrcItemByRow(int row, QString& srcfolder, QString& fileformat)
{
	ui.dataSrcTab->item(row, 1)->setText(srcfolder);
	ui.dataSrcTab->item(row, 2)->setText(fileformat);
}


