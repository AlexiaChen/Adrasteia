#ifndef UPDATECLIENTDLG_H
#define UPDATECLIENTDLG_H

#include <QDialog>
#include "ui_updateClientDlg.h"
#include "ClientCfg.h"

class updateClientDlg : public QDialog
{
	Q_OBJECT

public:
	updateClientDlg(QWidget *parent = 0);
	~updateClientDlg();

	void initClient(ClientList& list, int index);
	Ui::updateClientDlg getUpdateClientDlgUI() const;

signals:
	void updateClient(int index);

public slots:
	void okDlg();
	void cancelDlg();

private:
	Ui::updateClientDlg ui;
	int m_index;
};

#endif // UPDATECLIENTDLG_H
