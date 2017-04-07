#include "addProductDlg.h"

addProductDlg::addProductDlg(PublisherConfig::PublisherHas & pub, TypeConfig::TypeHas & type, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	connect(ui.okAddProduct, SIGNAL(clicked()), this, SLOT(okDlg()));
	connect(ui.cancelAddProduct, SIGNAL(clicked()), this, SLOT(cancelDlg()));

	//插入产品类型和发布者类型
	ui.addProductPublisher->clear();
	auto key_list = pub.keys();

	for (int i = 0; i < key_list.size(); ++i)
	{
		ui.addProductPublisher->addItem(key_list.at(i));
	}

	key_list = type.keys();

	for (int i = 0; i < key_list.size(); ++i)
	{
		ui.addProductType->addItem(key_list.at(i));
	}
}

addProductDlg::~addProductDlg()
{

}

void addProductDlg::cancelDlg()
{
	this->close();
}

void addProductDlg::okDlg()
{

	QString type = ui.addProductType->currentText();
	QString dij = ui.addProductDij->currentText();
	QString publisher = ui.addProductPublisher->currentText();
	QString timerange = ui.addProductTimeRange->text();
	QString maxtime = ui.addProductMaxTime->text();
	
	
	emit okAddProduct(type, dij, publisher, timerange, maxtime);
	this->close();
}
