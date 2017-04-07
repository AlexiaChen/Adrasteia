#ifndef UPDATEDLG_H
#define UPDATEDLG_H

#include <QDialog>
#include "ui_updatedlg.h"
#include "ProductCfg.h"
#include "PublisherConfig.h"

class updateProductDlg : public QDialog
{
	Q_OBJECT

public:
	updateProductDlg(QWidget *parent = 0);
	~updateProductDlg();

public:
	void initProduct(ProductList& raw, int rawIndex, ProductList& merge, int mergeIndex);
	void initPublisherHas(PublisherConfig::PublisherHas & pub);
	Ui::updateDlg getUpdateDlgUI();

signals:
	void updateDlgSaveProduct(int raw_index, int merge_index);


public slots:
	void okProduct();
	void cancelEdit();
	void mergeState(int state);
	void fileMergeState(int state);

private:
	void disableMergeConfig();
	void enableMergeConfig();
	void disableFileMergeCfg();
	void enableFileMergeCfg();

private:
	Ui::updateDlg ui;
	int m_raw_index;
	int m_merge_index;
	Product m_product;
	ProductList m_mergeList;
	PublisherConfig::PublisherHas m_publisherHas;
};

#endif // UPDATEDLG_H
