#include "updateProductdlg.h"
#include "Config.h"

#include <QTime>
#include <QVector>
#include <QMessageBox>

updateProductDlg::updateProductDlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	connect(ui.okUpdateProduct, SIGNAL(clicked()), this, SLOT(okProduct()));
	connect(ui.cancelUpdateEdit, SIGNAL(clicked()), this, SLOT(cancelEdit()));
	connect(ui.mergeChkb, SIGNAL(stateChanged(int)), this, SLOT(mergeState(int )));
	connect(ui.fileMergeChkb, SIGNAL(stateChanged(int)), this, SLOT(fileMergeState(int)));
}

updateProductDlg::~updateProductDlg()
{

}

void updateProductDlg::okProduct()
{
	emit updateDlgSaveProduct(m_raw_index,m_merge_index);
	this->close();
}

void updateProductDlg::cancelEdit()
{
	this->close();
}

void  updateProductDlg::mergeState(int state)
{
	
	QVector<QString> publisherList;


	for (int i = 0; i < m_publisherHas.values().size(); ++i)
	{
		QString publisher = m_publisherHas.values().at(i);

		publisherList.push_back(publisher);
	}

	
	if (state == Qt::CheckState::Checked)
	{
		QString dij = m_product.di;
		QString time_range = m_product.TimeRange;
		QString product_type = m_product.type;
		QString publisher = m_product.publisher;

		int publisher_index = publisherList.indexOf(publisher);

		if (publisher_index != -1)
		{
			publisherList.remove(publisher_index);
		}

		for (int i = 0; i < publisherList.size(); ++i)
		{
			int index = Config::findProductIndex2(m_mergeList, product_type, dij,publisherList.value(i), time_range);

			if (index != -1)
			{
				QMessageBox::warning(this, QStringLiteral("警告"), QStringLiteral("该类型产品已经有拼图，不能再添加拼图"));
				ui.mergeChkb->setCheckState(Qt::CheckState::Unchecked);
				
				return;
			}
		}

		
		enableMergeConfig();
	}
	else if (state == Qt::CheckState::Unchecked)
	{
		disableMergeConfig();
	}
}

void updateProductDlg::fileMergeState(int state)
{
	if (state == Qt::CheckState::Checked)
	{
		enableFileMergeCfg();
	}
	else if (state == Qt::CheckState::Unchecked)
	{
		disableFileMergeCfg();
	}
}

void updateProductDlg::initProduct(ProductList& raw, int rawIndex, ProductList& merge, int mergeIndex)
{
	m_raw_index = rawIndex;
	m_merge_index = mergeIndex;

	Product prod = raw.value(m_raw_index);
	m_product = prod;
	m_mergeList = merge;

	ui.newProductSrcPath->setText(prod.srcfolder);
	ui.newProductFileFormat->setText(prod.srcfileformat);

	prod.isGridtoStat ? ui.grid2StatChkb->setCheckState(Qt::CheckState::Checked) :
		ui.grid2StatChkb->setCheckState(Qt::CheckState::Unchecked);

	prod.savecached.bDisable ? ui.saveCachedChkb->setCheckState(Qt::CheckState::Unchecked):
		ui.saveCachedChkb->setCheckState(Qt::CheckState::Checked);

	//如果没有对应的merge配置
	if (m_merge_index == -1)
	{
		disableMergeConfig();
		ui.mergeChkb->setCheckState(Qt::CheckState::Unchecked);
	}
	else
	{
		enableMergeConfig();

		ui.mergeChkb->setCheckState(Qt::CheckState::Checked);

		Product mergeProd = merge.value(m_merge_index);

		ui.mergeScanTime->setTime(QTime(mergeProd.scanTime.timeList.first().hour.toInt(),
			mergeProd.scanTime.timeList.first().min.toInt(),
			mergeProd.scanTime.timeList.first().second.toInt()
			));


		if (mergeProd.fileMerge.bDisable)
		{
			ui.fileMergeChkb->setCheckState(Qt::CheckState::Unchecked);
			disableFileMergeCfg();
		}
		else
		{
			ui.fileMergeChkb->setCheckState(Qt::CheckState::Checked);
			enableMergeConfig();

			ui.fileMergeSavePath->setText(mergeProd.fileMerge.savepath);
			ui.fileMergeFileFormat->setText(mergeProd.fileMerge.filename);
		}   
			


	}

}

Ui::updateDlg updateProductDlg::getUpdateDlgUI() 
{
	return ui;
}


void updateProductDlg::disableMergeConfig()
{
	ui.mergeScanTime->setEnabled(false);
	ui.fileMergeChkb->setEnabled(false);
	ui.fileMergeSavePath->setEnabled(false);
	ui.fileMergeFileFormat->setEnabled(false);
}

void updateProductDlg::enableMergeConfig()
{
	ui.mergeScanTime->setEnabled(true);
	ui.mergeScanTime->setReadOnly(true);
	ui.fileMergeChkb->setEnabled(true);
	ui.fileMergeSavePath->setEnabled(true);
	ui.fileMergeFileFormat->setEnabled(true);
}

void updateProductDlg::disableFileMergeCfg()
{
	
	ui.fileMergeSavePath->setEnabled(false);
	ui.fileMergeFileFormat->setEnabled(false);
}

void updateProductDlg::enableFileMergeCfg()
{
	ui.fileMergeSavePath->setEnabled(true);
	ui.fileMergeFileFormat->setEnabled(true);
}

void updateProductDlg::initPublisherHas(PublisherConfig::PublisherHas & pub)
{
	m_publisherHas = pub;
}
