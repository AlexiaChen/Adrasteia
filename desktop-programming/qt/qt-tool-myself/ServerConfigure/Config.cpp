#include "Config.h"

#include "ConfigParser.h"
#include "ServerCfg.h"
#include "ClientCfg.h"
#include "ProductCfg.h"
#include "StationCfg.h"
#include "PublisherConfig.h"
#include "TypeConfig.h"

#include <QVariant>
#include <QCheckBox>
#include <QMessageBox>

#pragma execution_character_set("utf-8")

Config::Config(QString &path,QWidget *parent)
	:QDialog(parent)
{
	ui.setupUi(this);
	
	
	m_parser = new ConfigParser(path,parent);
	
	//初始化产品类型，发布者类型,数据类型
	m_publisherHas = m_parser->getPublisherConfig()->publishers();
	m_typeHas = m_parser->getTypeConfig()->types();
	m_datatypeHas = m_parser->getDataTypeConfig()->datatypes();
	
	m_updateProductDlg = new updateProductDlg(parent);
	m_updateProductDlg->setWindowTitle(QStringLiteral("修改产品配置"));
	m_updateProductDlg->setModal(true);

	m_addDlg = new addProductDlg(m_publisherHas, m_typeHas,parent);
	m_addDlg->setWindowTitle(QStringLiteral("添加产品"));
	m_addDlg->setModal(true);

	m_updateStatDlg = new updateStatDlg(m_datatypeHas,parent);
	m_updateStatDlg->setWindowTitle(QStringLiteral("修改站点配置"));
	m_updateStatDlg->setModal(true);
	
	m_updateClientDlg = new updateClientDlg(parent);
	m_updateClientDlg->setWindowTitle(QStringLiteral("修改客户端用户名密码"));
	m_updateClientDlg->setModal(true);
	
	createConnectBind();
	
	init();

}

Config::~Config()
{
	delete m_parser;
	delete m_updateProductDlg;
	delete m_addDlg;
	delete m_updateStatDlg;
}











void Config::updateProductCfg(int raw_index, int merge_index)
{
	

	QString srcPath = m_updateProductDlg->getUpdateDlgUI().newProductSrcPath->text();
	QString fileFormat = m_updateProductDlg->getUpdateDlgUI().newProductFileFormat->text();

	m_rawProductList[raw_index].srcfolder = srcPath;
	m_rawProductList[raw_index].srcfileformat = fileFormat;
	
	if (m_updateProductDlg->getUpdateDlgUI().grid2StatChkb->checkState() == Qt::CheckState::Checked)
	{
		m_rawProductList[raw_index].isGridtoStat = true;
	}
	else if (m_updateProductDlg->getUpdateDlgUI().grid2StatChkb->checkState() == Qt::CheckState::Unchecked)
	{
		m_rawProductList[raw_index].isGridtoStat = false;
	}

	if (m_updateProductDlg->getUpdateDlgUI().saveCachedChkb->checkState() == Qt::CheckState::Checked)
	{
		m_rawProductList[raw_index].savecached.bDisable = false;
	}
	else if (m_updateProductDlg->getUpdateDlgUI().saveCachedChkb->checkState() == Qt::CheckState::Unchecked)
	{
		m_rawProductList[raw_index].savecached.bDisable = true;
	}

	if (merge_index != -1)
	{
		//拼图合并配置是否check
		if (m_updateProductDlg->getUpdateDlgUI().mergeChkb->checkState() == Qt::CheckState::Checked)
		{
			//拼图合并配置的格点转站点是否check
			if (m_updateProductDlg->getUpdateDlgUI().grid2StatChkb->checkState() == Qt::CheckState::Checked)
			{
				m_mergeProductList[merge_index].isGridtoStat = true;
			}
			else if (m_updateProductDlg->getUpdateDlgUI().grid2StatChkb->checkState() == Qt::CheckState::Unchecked)
			{
				m_mergeProductList[merge_index].isGridtoStat = false;
			}

			//拼图合并配置的扫描时间只做显示，不能修改，跳过


			//文件合并filemerge是否被check
			if (m_updateProductDlg->getUpdateDlgUI().fileMergeChkb->checkState() == Qt::CheckState::Checked)
			{
				QString mergeSavePath = m_updateProductDlg->getUpdateDlgUI().fileMergeSavePath->text();
				QString fileMergeFileFormat = m_updateProductDlg->getUpdateDlgUI().fileMergeFileFormat->text();

				m_mergeProductList[merge_index].fileMerge.bDisable = false;
				m_mergeProductList[merge_index].fileMerge.savepath = mergeSavePath;
				m_mergeProductList[merge_index].fileMerge.filename = fileMergeFileFormat;
			}
			else if (m_updateProductDlg->getUpdateDlgUI().fileMergeChkb->checkState() == Qt::CheckState::Unchecked)
			{
				//如果filemerge没被check，不做任何修改，并删除filemerge配置
				m_mergeProductList[merge_index].fileMerge.bDisable = true;
			}


		}
		else if (m_updateProductDlg->getUpdateDlgUI().mergeChkb->checkState() == Qt::CheckState::Unchecked)
		{
			//如果没被check，拼图合并不做任何修改，并删除拼图合并配置
			m_mergeProductList.remove(merge_index);
		}
	}
	else
	{
		//如果产品没有对应merge，但是勾选了，就创建对应merge
		if (m_updateProductDlg->getUpdateDlgUI().mergeChkb->checkState() == Qt::CheckState::Checked)
		{
			Product newMerge;
			createNewMergeProduct(newMerge,m_rawProductList.value(raw_index));

			m_mergeProductList.push_back(newMerge);
		}
	}

	


	
}

void Config::saveClientCfg()
{
	ClientCfg* cfg = m_parser->getClientConfig();
	
	cfg->setClientList(m_clientList);
	cfg->saveToFile();

	QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("客户端配置保存成功"), QMessageBox::Ok);
}

void Config::saveStationCfg()
{
	StationCfg* cfg = m_parser->getStationConfig();

	STStationManage mng;
	cfg->getStationManage(mng);
	cfg->setStationManage(mng);
	
	cfg->setStationList(m_rawStationlist, m_mergeStationlist);
	
	cfg->saveToFile();

	QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("站点配置保存成功"), QMessageBox::Ok);
}

void Config::selectProductOneRow(int row, int column)
{
	ui.productsTab->selectRow(row);
}

void  Config::selectStationOneRow(int row, int column)
{
	ui.stationsTab->selectRow(row);
}

void Config::addProductInfo(QString&type, QString&dij, QString&publisher, QString& timerange,
	QString&maxtime)
{
	
	//检查添加的产品是否重复

	for (int i = 0; i < ui.productsTab->rowCount(); ++i)
	{
		QString current_type, current_dij, current_publisher, current_timerange,current_maxtime;
		getProductInfoByRow(i, current_type, current_dij, current_publisher, current_timerange, current_maxtime);

		if (current_type == type && current_dij == dij && current_publisher == publisher && current_timerange == timerange)
		{
			QMessageBox::warning(this, QStringLiteral("警告"), QStringLiteral("不能添加重复的产品！请确认后再次添加"));
			return;
		}
	}
	
	
	insertProductRow(ui.productsTab->rowCount(), type, dij, publisher, timerange, maxtime);

	QString Type;
	QString Dij;
	QString Publisher;
	QString Timerang;
	QString Maxtime;

	getProductInfoByRow(ui.productsTab->rowCount() - 1, Type, Dij, Publisher, Timerang, Maxtime);

	Type = m_typeHas.value(Type);
	Publisher = m_publisherHas.value(Publisher);
	
	Product raw;
	createNewRawProduct(raw, Type, Dij, Publisher, Timerang, Maxtime);
	m_rawProductList.push_back(raw);

	int raw_index = Config::findProductIndex(m_rawProductList, Type, Dij, Publisher, Timerang, Maxtime);
	int merge_index = Config::findProductIndex(m_mergeProductList, Type, Dij, Publisher, Timerang, Maxtime);
	m_updateProductDlg->initProduct(m_rawProductList, raw_index, m_mergeProductList, merge_index);
	m_updateProductDlg->show();

}

void Config::addProduct()
{
	
	m_addDlg->show();
	
}

void Config::deleteProduct()
{
	int currentRow = ui.productsTab->currentRow();
	
	if (currentRow == -1)
	{
		return;
	}
	
	QString type;
	QString dij ;
	QString publisher;
	QString timerange ;
	QString Maxtime;
	
	getProductInfoByRow(currentRow, type, dij, publisher, timerange, Maxtime);

	type = m_typeHas.value(type);
	publisher = m_publisherHas.value(publisher);

	//删除相应raw的产品列表信息
	int index = Config::findProductIndex(m_rawProductList, type, dij, publisher, timerange, Maxtime);
	if (index != -1)
	{
		m_rawProductList.remove(index);
	}
	

	//删除相应的merge产品列表
	index = Config::findProductIndex(m_mergeProductList, type, dij, publisher, timerange, Maxtime);
	if (index != -1)
	{
		m_mergeProductList.remove(index);
	}


	
	
	ui.productsTab->removeRow(currentRow);
}

void Config::refreshProductCfg()
{
	ProductCfg* cfg = m_parser->getProductConfig();
	
	//重新载入客户端xml，用户配置merge产品的datasrc
	m_parser->getClientConfig()->getClientList(m_clientList);

	ProductList rawlist, mergeList;
	cfg->getProductList(rawlist, mergeList);

	m_rawProductList = rawlist;
	m_mergeProductList = mergeList;


	while (ui.productsTab->rowCount() > 0)
	{
		ui.productsTab->removeRow(0);
	}

	int rowNum = 0;
	for (int i = 0; i < m_rawProductList.size(); ++i)
	{

		Product prod = m_rawProductList.at(i);

		//插入产品行

		if (prod.publisher.isEmpty())
		{
			prod.publisher = "none";
		}
		
		insertProductRow(rowNum, m_typeHas.key(prod.type), prod.di, m_publisherHas.key(prod.publisher), prod.TimeRange, prod.MaxForecastTime);

		rowNum++;


	}
}

void Config::refreshStationCfg()
{
	StationCfg *cfg = m_parser->getStationConfig();

	StationList rawlist, mergelist;

	cfg->getStationList(rawlist, mergelist);
	m_rawStationlist = rawlist;
	m_mergeStationlist = mergelist;

	while (ui.stationsTab->rowCount() > 0)
	{
		ui.stationsTab->removeRow(0);
	}

	//站点表格
	int rowNum = 0;
	for (int i = 0; i < m_rawStationlist.size(); ++i)
	{

		STStation state = m_rawStationlist.at(i);

		//插入站点行
		insertStationRow(rowNum, state.cityType, state.timeRange);
		rowNum++;


	}

}

void Config::updateProduct()
{
	
	int currentRow = ui.productsTab->currentRow();

	if (currentRow != -1)
	{
		QString type;
		QString dij;
		QString publisher;
		QString timerange;
		QString Maxtime;

		getProductInfoByRow(currentRow, type, dij, publisher, timerange, Maxtime);

		type = m_typeHas.value(type);
		publisher = m_publisherHas.value(publisher);

		int rawIndex = findProductIndex(m_rawProductList, type, dij, publisher, timerange, Maxtime);
		int mergeIndex = findProductIndex(m_mergeProductList, type, dij, publisher, timerange, Maxtime);

		m_updateProductDlg->initProduct(m_rawProductList, rawIndex, m_mergeProductList, mergeIndex);
		m_updateProductDlg->initPublisherHas(m_publisherHas);
		m_updateProductDlg->show();
	}
	
	

}


void Config::addStation()
{
	QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("该功能目前暂不支持"));
}

void Config::deleteStation()
{
	QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("该功能目前暂不支持"));
}

void Config::updateStation()
{
	int currentRow = ui.stationsTab->currentRow();

	if (currentRow == -1)
	{
		return;
	}

	QString cityType;
	QString timerange;
	
	getStationInfoByRow(currentRow, cityType, timerange);

	int index = Config::findStationIndex(m_rawStationlist, cityType, timerange);

	m_updateStatDlg->initStation(m_rawStationlist, index);

	m_updateStatDlg->show();
}

void Config::createConnectBind()
{
	connect(ui.selectConfig, SIGNAL(currentChanged(int)), this, SLOT(loadCurrentChange(int)));
	

	//bind tableWidget
	connect(ui.productsTab, SIGNAL(cellClicked(int ,int)), this, SLOT(selectProductOneRow(int,int)));
	connect(ui.stationsTab, SIGNAL(cellClicked(int, int)), this, SLOT(selectStationOneRow(int, int)));
	connect(ui.clientsTab, SIGNAL(cellClicked(int, int)), this, SLOT(selectClientOneRow(int, int)));
	
	//CRUD for product
	connect(ui.addProduct, SIGNAL(clicked()), this, SLOT(addProduct()));
	connect(ui.deleteProduct, SIGNAL(clicked()), this, SLOT(deleteProduct()));
	connect(ui.updateProduct, SIGNAL(clicked()), this, SLOT(updateProduct()));
	connect(ui.saveProductCfg, SIGNAL(clicked()), this, SLOT(saveProductCfg()));
	connect(ui.refreshProductCfg, SIGNAL(clicked()), this, SLOT(refreshProductCfg()));
	
	connect(m_updateProductDlg, SIGNAL(updateDlgSaveProduct(int,int)), this, SLOT(updateProductCfg(int,int)));
	connect(m_addDlg, SIGNAL(okAddProduct(QString& , QString& , QString& , QString& , QString& )),
		this, SLOT(addProductInfo(QString&, QString&, QString&, QString& , QString&)));

	//CRUD for station
	connect(ui.refreshStationCfg, SIGNAL(clicked()), this, SLOT(refreshStationCfg()));

	connect(ui.addStation, SIGNAL(clicked()), this, SLOT(addStation()));
	connect(ui.deleteStation, SIGNAL(clicked()), this, SLOT(deleteStation()));
	connect(ui.updateStation, SIGNAL(clicked()), this, SLOT(updateStation()));
	connect(ui.saveStationCfg, SIGNAL(clicked()), this, SLOT(saveStationCfg()));
	connect(m_updateStatDlg, SIGNAL(updateStation(int)), this, SLOT(updateStationCfg(int)));

	//CRUD for client
	connect(ui.refreshClientCfg, SIGNAL(clicked()), this, SLOT(refreshClientCfg()));
	connect(ui.updateClientPwd, SIGNAL(clicked()), this, SLOT(updateClient()));
	connect(m_updateClientDlg, SIGNAL(updateClient(int)), this, SLOT(updateClientCfg(int)));
	connect(ui.clientCfgSave, SIGNAL(clicked()), this, SLOT(saveClientCfg()));
}

void Config::loadCurrentChange(int index)
{
	QString currentTabText = ui.selectConfig->tabText(index);

	
	if (currentTabText.toLower() == "客户端配置")
	{
		loadClientCfg();
	}
	else if (currentTabText.toLower() == "站点配置")
	{
		loadStationCfg();
	}
	else if (currentTabText.toLower() == "产品配置")
	{
		loadProductCfg();
	}
}



void Config::insertProductRow(int row, const QString &type, const QString &dij, const QString &publisher,
	const QString &timerange, const QString &maxtime)
{
	ui.productsTab->insertRow(row);



	QTableWidgetItem *item = new QTableWidgetItem(type);
	item->setFlags(item->flags() ^ Qt::ItemIsEditable); //set item not editable
	ui.productsTab->setItem(row, 0, item);

	item = new QTableWidgetItem(dij);
	item->setFlags(item->flags() ^ Qt::ItemIsEditable);
	ui.productsTab->setItem(row, 1, item);

	item = new QTableWidgetItem(publisher);
	item->setFlags(item->flags() ^ Qt::ItemIsEditable);
	ui.productsTab->setItem(row, 2, item);

	item = new QTableWidgetItem(timerange);
	item->setFlags(item->flags() ^ Qt::ItemIsEditable);
	ui.productsTab->setItem(row, 3, item);


	item = new QTableWidgetItem(maxtime);
	item->setFlags(item->flags() ^ Qt::ItemIsEditable);
	ui.productsTab->setItem(row, 4, item);

	
}

void Config::insertStationRow(int row, const QString& citytype, const QString &timerange)
{
	ui.stationsTab->insertRow(row);

	QTableWidgetItem *item;
	
	if (citytype == "town")
	{
		item = new QTableWidgetItem(QStringLiteral("城镇"));
		item->setFlags(item->flags() ^ Qt::ItemIsEditable); //set item not editable
		ui.stationsTab->setItem(row, 0, item);
	}
	else if (citytype == "bigcity")
	{
		item = new QTableWidgetItem(QStringLiteral("大城市"));
		item->setFlags(item->flags() ^ Qt::ItemIsEditable); //set item not editable
		ui.stationsTab->setItem(row, 0, item);
	}
	
	

	item = new QTableWidgetItem(timerange);
	item->setFlags(item->flags() ^ Qt::ItemIsEditable);
	ui.stationsTab->setItem(row, 1, item);

}

void Config::getStationInfoByRow(int row, QString &citytype,
	QString &timerange)
{
	
	
	if (ui.stationsTab->item(row, 0)->text() == QStringLiteral("大城市"))
	{
		citytype = "bigcity";
	}
	else if (ui.stationsTab->item(row, 0)->text() == QStringLiteral("城镇"))
	{
		citytype = "town";
	}
	
	timerange = ui.stationsTab->item(row, 1)->text();	
}

void Config::getProductInfoByRow(int row, QString &type, QString &dij, QString &publisher,
	QString &timerange, QString &maxtime)
{
	type = ui.productsTab->item(row,0)->text();
	dij = ui.productsTab->item(row,1)->text();
	publisher = ui.productsTab->item(row,2)->text();
	timerange = ui.productsTab->item(row,3)->text();
	 maxtime = ui.productsTab->item(row,4)->text();
}


void  Config::createMergeProductHeader(Product& merge, Product& raw)
{
	merge.key = QString("merge_") + raw.key;
	merge.name = raw.name;
	merge.role = "merge";
	merge.lon1 = raw.lon1;
	merge.lon2 = raw.lon1;
	merge.lat1 = raw.lat1;
	merge.lat2 = raw.lat2;
	merge.offset = raw.offset;
	merge.status = raw.status;
	merge.element = raw.element;
	merge.statistical = raw.statistical;
	merge.BusinessStatus = raw.BusinessStatus;
	merge.isWarning = raw.isWarning;
	merge.publisher = raw.publisher;
	merge.di = raw.di;
	merge.dj = raw.dj;
	merge.TimeRange = raw.TimeRange;
	merge.type = raw.type;
	merge.MaxForecastTime = raw.MaxForecastTime;
	merge.category = raw.category;
}

void Config::createRawProductHeader(Product &raw, QString &type, QString& dij, QString& publisher,
	QString& timerange, QString &maxtime)
{
	
	raw.name = m_typeHas.key(type) + m_publisherHas.key(publisher) + QString("指导报");
	raw.role = "raw";

	if (publisher == "province")
	{
		raw.lon1 = QString::number(m_lon1);
		raw.lon2 = QString::number(m_lon2);
		raw.lat1 = QString::number(m_lat1);
		raw.lat2 = QString::number(m_lat2);
	}
	else if (publisher == "national" || publisher == "none")
	{
		raw.lon1 = "70";
		raw.lon2 = "140";
		raw.lat1 = "0";
		raw.lat2 = "60";
	}
	
	
	raw.offset = "0";
	raw.status = "0";
	raw.element = "0";
	raw.statistical = "0";
	raw.BusinessStatus = "";
	raw.isWarning = "no";
	

	
	raw.publisher = publisher;
	
	raw.di = dij;
	raw.dj = dij;
	raw.TimeRange = timerange;
	raw.type = type;
	raw.MaxForecastTime = maxtime;
	raw.category = "0";

	QString d;

	if (dij == "0.05")
	{
		d = "05km";
	}
	else if (dij == "0.1")
	{
		d = "10km";
	}

	
	raw.key = publisher + "_" + d + "_" + type;
	
	
	
}

void Config::addMergeProductScanTime(Product& merge)
{
	Time tmpTime;
	tmpTime.hour = "09";
	tmpTime.min = "50";
	
	merge.scanTime.timeList.push_back(tmpTime);

}

void Config::addProductSaveCached(Product &prod)
{
	prod.savecached.bDisable = false;
	prod.savecached.validtime = "86400";
}

void Config::addMergeProductFileMerge(Product& merge)
{
	merge.fileMerge.bDisable = false;
	merge.fileMerge.dds = QString("1");
	merge.fileMerge.savepath = m_updateProductDlg->getUpdateDlgUI().fileMergeSavePath->text();
	merge.fileMerge.filename = m_updateProductDlg->getUpdateDlgUI().fileMergeFileFormat->text();
}

void Config::addMergeProductTemplate(Product& merge)
{

	merge.template_t.type = "data";
	merge.template_t.value = "9999.0";
}

void  Config::addMergeProductMask(Product& merge)
{
	merge.mask.di = merge.di;
	merge.mask.dj = merge.dj;
	merge.mask.filePath = "./mask/mask.005";
}

void  Config::addMergeProductDataSrc(Product& merge)
{
	for (int i = 0; i < m_clientList.size(); ++i)
	{
		Item item;
		item.clientID = m_clientList.at(i).ID;

		merge.dataSrc.itemList.push_back(item);
	}
	
}

void Config::addRawProductScanTime(Product& raw)
{

	raw.scanTime.interval = "600";

}

void Config::addRawProductForeCfg(Product& raw)
{
	ForeCast frc08,frc20;
	
	frc08.hourTime = "08";
	frc08.RangeTime = "3";
	
	frc20.hourTime = "20";
	frc20.RangeTime = "3";
	
	raw.forecastCfg.forecastList.push_back(frc08);
	raw.forecastCfg.forecastList.push_back(frc20);
}

void Config::addRawProductDataProc(Product& raw)
{
	Chains chs;
	chs.bDisable = false;
	chs.filename = "{YYYYMMDDHH}.{FFF}.GRIB2";
	chs.ID = "1";
	chs.name = "省数据";
	chs.savepath = "/data/lwfd/gribdata/" + raw.type;
	
	Chain chn;
	chn.libName = "libNwfdRegionalField.so";

	//起止经纬度需要往外扩大
	float DLon = raw.di.toFloat();
	float DLat = raw.dj.toFloat();
	

	float lon1 = isEqual(fmodf(m_lon1, DLon), 0.0f) ? m_lon1 - DLon : (int)(m_lon1 / DLon) * DLon - DLon;
	float lon2 = isEqual(fmodf(m_lon2, DLon), 0.0f) ? m_lon2 + DLon : ((int)(m_lon2 / DLon) + 1) * DLon + DLon;
	float lat1 = isEqual(fmodf(m_lat1, DLat), 0.0f) ? m_lat1 - DLat : (int)(m_lat1 / DLat) * DLat - DLat;
	float lat2 = isEqual(fmodf(m_lat2, DLat), 0.0f) ? m_lat2 + DLat : ((int)(m_lat2 / DLat) + 1) * DLat + DLat;

	chn.params = QString("%1_%2_%3_%4").arg(lon1).arg(lon2).arg(lat1).arg(lat2);
	chs.chainList.push_back(chn);
	
	raw.dataProc.chainsList.push_back(chs);
}


void Config::addMergeProductForeCfg(Product& merge)
{
	
	ForeCast frc;
	frc.hourTime = "08";
	frc.latestTime = "00:00";
	
	merge.forecastCfg.forecastList.push_back(frc);
}



void Config::createNewMergeProduct(Product& merge, Product& raw)
{
	//header
	createMergeProductHeader(merge, raw);

	//scanTime
	addMergeProductScanTime(merge);

	//forecast Cfg
	addMergeProductForeCfg(merge);

	//save cached
	addProductSaveCached(merge);

	// file merge
	if (m_updateProductDlg->getUpdateDlgUI().fileMergeChkb->checkState() == Qt::CheckState::Checked)
	{
		addMergeProductFileMerge(merge);
	}
	else if (m_updateProductDlg->getUpdateDlgUI().fileMergeChkb->checkState() == Qt::CheckState::Unchecked)
	{
		merge.fileMerge.bDisable = true;
	}

	//realted product
	merge.relatedProduct = raw.key;

	//template
	addMergeProductTemplate(merge);

	//mask
	addMergeProductMask(merge);
	
	//data src
	addMergeProductDataSrc(merge);
	
	//grid2state
	if (m_updateProductDlg->getUpdateDlgUI().grid2StatChkb->checkState() == Qt::CheckState::Checked)
	{
		merge.isGridtoStat = true;
	}
	else if (m_updateProductDlg->getUpdateDlgUI().grid2StatChkb->checkState() == Qt::CheckState::Unchecked)
	{
		merge.isGridtoStat = false;
	}


}

void Config::createNewRawProduct(Product &raw, QString &type, QString& dij, QString& publisher,
	QString& timerange, QString &maxtime)
{
	// header
	createRawProductHeader(raw,type,dij,publisher,timerange,maxtime);

	//scantime
	addRawProductScanTime(raw);

	//src 
	raw.srcfolder = "";
	raw.srcfileformat = "";

	// fore cfg
	addRawProductForeCfg(raw);

	// data proc
	if (publisher == "none")
	{
		addRawProductDataProc(raw);
	}
	

	//save cached
	addProductSaveCached(raw);
	
	//grid2stat
	raw.isGridtoStat = false;

}



int  Config::findProductIndex(ProductList& list,  QString &type, QString &dij, QString &publisher,
	QString &timerange, QString &maxtime)
{
	for (int i = 0; i < list.size(); ++i)
	{
		Product Prod = list.at(i);

		if ( Prod.type == type && Prod.di == dij
			&& Prod.TimeRange == timerange && Prod.MaxForecastTime == maxtime
			&& Prod.publisher == publisher)
		{
			return i;
		}
	}

	return -1;
}

int Config::findProductIndex2(ProductList& list, QString &type, QString &dij, QString &publisher, QString &timerange)
{
	for (int i = 0; i < list.size(); ++i)
	{
		Product Prod = list.at(i);

		if (Prod.type == type && Prod.di == dij
			&& Prod.TimeRange == timerange 
			&& Prod.publisher == publisher)
		{
			return i;
		}
	}

	return -1;
}

int Config::findStationIndex(StationList& list, QString &citytype,
	QString &timerange)
{
	for (int i = 0; i < list.size(); ++i)
	{
		STStation stat = list.at(i);

		if (stat.cityType == citytype && stat.timeRange == timerange )
		{
			return i;
		}
	}

	return -1;
}




void Config::loadClientCfg()
{
	
	
	refreshClientCfg();

}

void  Config::loadStationCfg()
{
	refreshStationCfg();
}

void Config::loadProductCfg()
{
	refreshProductCfg();
}

void Config::init()
{
	loadClientCfg();
}

void Config::updateStationCfg(int raw_index)
{

	m_rawStationlist[raw_index].savecached.bDisable = false;
	

	//遍历修改的data src
	m_rawStationlist[raw_index].dataSrc.dataSrcItemList.clear();

	for (int i = 0; i < m_updateStatDlg->getUpdateStatDlgUI().dataSrcTab->rowCount(); ++i)
	{
		QString datatype = m_updateStatDlg->getUpdateStatDlgUI().dataSrcTab->item(i,0)->text();
		QString srcfolder = m_updateStatDlg->getUpdateStatDlgUI().dataSrcTab->item(i, 1)->text();
		QString srcfileformat = m_updateStatDlg->getUpdateStatDlgUI().dataSrcTab->item(i, 2)->text();
		
		addDataSrcItem(m_rawStationlist[raw_index].dataSrc.dataSrcItemList, datatype, srcfolder, srcfileformat, 
			m_rawStationlist[raw_index].cityType, 
			m_rawStationlist[raw_index].timeRange);
	}
}

void Config::saveProductCfg()
{
	ProductCfg* cfg = m_parser->getProductConfig();

	
	// for data proc params
	fillDataProcParamsWithLonLat();
	
	
	cfg->setProductListNew(m_rawProductList, m_mergeProductList);
	cfg->saveToFile();

	QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("产品配置保存成功"), QMessageBox::Ok);
}



void Config::addDataSrcItem(DataSrcItemList& list, QString& datatype, QString& srcfolder, QString& srcfileformat, QString& cityType, QString&timerange)
{
	StationDataSrcItem item;

	item.dataType = m_datatypeHas.value(datatype);
	item.symbol = "BABJ";
	item.Description = datatype;
	item.srcfolder = srcfolder;
	item.srcfileformat = srcfileformat;

	list.push_back(item);
}

void Config::fillDataProcParamsWithLonLat()
{
	//配置数据量（N*M）^2不大，
	for (int i = 0; i < m_rawProductList.size(); ++i)
	{
		if (m_rawProductList.at(i).publisher == "none")
		{
			
			for (int j = 0; j < m_rawProductList.at(i).dataProc.chainsList.size(); ++j)
			{
				
				m_rawProductList[i].dataProc.chainsList[j].chainList[0].params = 
					QString("%1_%2_%3_%4").arg(m_lon1).arg(m_lon2).arg(m_lat1).arg(m_lat2);
		
			}
		}
	}
}

void Config::refreshClientCfg()
{
	ClientCfg* cfg = m_parser->getClientConfig();

	ClientList list;
	
	cfg->getClientList(list);
	m_clientList = list;

	while (ui.clientsTab->rowCount() > 0)
	{
		ui.clientsTab->removeRow(0);
	}

	//客户端
	int rowNum = 0;
	for (int i = 0; i < m_clientList.size(); ++i)
	{

		Client client = m_clientList.at(i);

		//插入客户端行
		insertClientRow(rowNum, client.ID, client.name, client.userName, client.passWord);
		rowNum++;

	}
	 
}

void Config::insertClientRow(int row, const QString& id, const QString& name, const QString&username, const QString& password)
{
	ui.clientsTab->insertRow(row);

	QTableWidgetItem *item;

	
	item = new QTableWidgetItem(id);
	item->setFlags(item->flags() ^ Qt::ItemIsEditable); //set item not editable
	ui.clientsTab->setItem(row, 0, item);
	

	item = new QTableWidgetItem(name);
	item->setFlags(item->flags() ^ Qt::ItemIsEditable); //set item not editable
	ui.clientsTab->setItem(row, 1, item);


	item = new QTableWidgetItem(username);
	item->setFlags(item->flags() ^ Qt::ItemIsEditable); //set item not editable
	ui.clientsTab->setItem(row, 2, item);

	item = new QTableWidgetItem(password);
	item->setFlags(item->flags() ^ Qt::ItemIsEditable); //set item not editable
	ui.clientsTab->setItem(row, 3, item);
	
}

void Config::getClientInfoByRow(int row, QString& id, QString& name, QString&username, QString& password)
{
	id = ui.clientsTab->item(row, 0)->text();
	name = ui.clientsTab->item(row, 1)->text();
	username = ui.clientsTab->item(row, 2)->text();
	password = ui.clientsTab->item(row, 3)->text();
}

void Config::selectClientOneRow(int row, int column)
{
	ui.clientsTab->selectRow(row);
}

void Config::updateClient()
{
	int currentRow = ui.clientsTab->currentRow();

	if (currentRow == -1)
	{
		return;
	}

	QString id, name, username, password;
	getClientInfoByRow(currentRow, id, name, username, password);

	int index = Config::findClientIndex(m_clientList, id);
	m_updateClientDlg->setWindowTitle(name + QStringLiteral("用户名密码修改"));
	m_updateClientDlg->initClient(m_clientList, index);
	m_updateClientDlg->show();

}

int Config::findClientIndex(ClientList& list, QString& id)
{
	for (int i = 0; i < list.size(); ++i)
	{
		Client client = list.at(i);

		if (client.ID == id)
		{
			return i;
		}
	}

	return -1;
}

void Config::updateClientCfg(int index)
{
	QString username = m_updateClientDlg->getUpdateClientDlgUI().clientUserName->text();
	QString password = m_updateClientDlg->getUpdateClientDlgUI().clientPassWord->text();

	m_clientList[index].userName = username;
	m_clientList[index].passWord = password;


	setClientInfoByRow(index, username, password);
}

void Config::setClientInfoByRow(int row, const QString&username, const QString& password)
{
	ui.clientsTab->item(row, 2)->setText(username);
	ui.clientsTab->item(row, 3)->setText(password);
}

