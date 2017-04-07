#ifndef ADDDLG_H
#define ADDDLG_H

#include <QDialog>
#include "ui_addDlg.h"
#include "PublisherConfig.h"
#include "TypeConfig.h"

class addProductDlg : public QDialog
{
	Q_OBJECT

public:
	addProductDlg(PublisherConfig::PublisherHas & pub, TypeConfig::TypeHas & type,QWidget *parent = 0);
	~addProductDlg();



public slots:
	void cancelDlg();
	void okDlg();
signals:
	void okAddProduct(QString& type,QString& dij,QString& publisher,QString& timerange,QString& maxtime);

private:
	Ui::addDlg ui;
};

#endif // ADDDLG_H
