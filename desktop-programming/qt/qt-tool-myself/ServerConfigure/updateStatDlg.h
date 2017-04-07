#ifndef UPDATESTATDLG_H
#define UPDATESTATDLG_H

#include <QDialog>
#include "ui_updateStatDlg.h"
#include "StationCfg.h"
#include "DataType.h"
#include "qobjectdefs.h"
#include "AddStatDataSrc.h"
#include "UpdateStatDataSrc.h"

class updateStatDlg : public QDialog
{
	Q_OBJECT

public:
	updateStatDlg(DataTypeConfig::DataTypeHas & has,QWidget *parent = 0);

	~updateStatDlg();

public:
	void initStation(StationList& raw, int rawIndex);
	Ui::updateStatDlg getUpdateStatDlgUI();


signals:
	void updateStation(int raw_index);
	
public slots:
	void addItem();
	void addItem(QString& dataType,QString& srcfolder,QString& srcfileformat);
	void deleteItem();
	void updateItem();
	void updateItemMsg(QString& srcfolder, QString& srcfileformat);
	void okDlg();
	void cancelDlg();
	void selectItemOneRow(int row,int column);

private:
	void clearDataSrcTab();
	
	void insertDataSrcItem(QString& dataType, QString &srcfolder, QString& fileformat);
	void getDataSrcItemByRow(int row, QString& name, QString& srcfolder, QString& fileformat);
	void setDataSrcItemByRow(int row, QString& srcfolder, QString& fileformat);

private:
	Ui::updateStatDlg ui;
	DataTypeConfig::DataTypeHas m_dataTypeHas;
	AddStatDataSrc *m_addDataSrcDlg;
	UpdateStatDataSrc* m_updateDataSrcDlg;

	int m_rawIndex;
	StationList m_rawlist;
};

#endif // UPDATESTATDLG_H
