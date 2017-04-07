#ifndef UPDATESTATDATASRC_H
#define UPDATESTATDATASRC_H

#include <QDialog>
#include "ui_UpdateStatDataSrc.h"

class UpdateStatDataSrc : public QDialog
{
	Q_OBJECT

public:
	UpdateStatDataSrc(QWidget *parent = 0);
	~UpdateStatDataSrc();

	void initStatDataSrc(QString& srcfolder, QString& srcfileformat);

signals:
	void UpdateItemInfo(QString& srcfolder, QString& srcfileformat);

public slots:
	void okDlg();
	void cancelDlg();

private:
	Ui::UpdateStatDataSrc ui;
};

#endif // UPDATESTATDATASRC_H
