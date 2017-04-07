#ifndef INITWND_H
#define INITWND_H

#include <QDialog>
#include "ui_InitWnd.h"

class InitWnd : public QDialog
{
	Q_OBJECT

public:
	InitWnd(QWidget *parent = 0);
	~InitWnd();

	void setLonAndLat(QString &lon1, QString &lon2, QString &lat1, QString &lat2);

public slots:
	void openCfgPath();
	void okInitDlg();
	void cancelInitDlg();

public:
	QString getCfgPath() const;
	void getLonAndLat(QString &lon1, QString &lon2, QString &lat1, QString &lat2);

private:
	void setNationalLonLat();

private:
	QString cfgPath;
	Ui::InitWnd ui;
};

#endif // INITWND_H
