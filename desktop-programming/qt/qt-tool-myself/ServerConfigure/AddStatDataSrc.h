#ifndef ADDSTATDATASRC_H
#define ADDSTATDATASRC_H

#include <QDialog>
#include "ui_AddStatDataSrc.h"
#include "DataType.h"

class AddStatDataSrc : public QDialog
{
	Q_OBJECT

public:
	AddStatDataSrc(DataTypeConfig::DataTypeHas & has, QWidget *parent = 0);
	~AddStatDataSrc();

	
signals:
	void currentDataType(QString& datatype,QString& srcfoler,QString &srcfileformat);
	
public slots:
	void okDlg();
	void cancelDlg();

private:
	Ui::AddStatDataSrc ui;
};

#endif // ADDSTATDATASRC_H
