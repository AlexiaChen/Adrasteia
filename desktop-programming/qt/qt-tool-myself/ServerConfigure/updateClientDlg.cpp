#include "updateClientDlg.h"

updateClientDlg::updateClientDlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	connect(ui.okDlg, SIGNAL(clicked()), this, SLOT(okDlg()));
	connect(ui.cancelDlg, SIGNAL(clicked()), this, SLOT(cancelDlg()));
}

updateClientDlg::~updateClientDlg()
{

}

void updateClientDlg::initClient(ClientList& list, int index)
{
	if (index != -1)
	{
		m_index = index;
		Client client = list.value(index);
		ui.clientUserName->setText(client.userName);
		ui.clientPassWord->setText(client.passWord);
	}
}

void updateClientDlg::okDlg()
{
	emit updateClient(m_index);
	this->close();
}

void updateClientDlg::cancelDlg()
{
	this->close();
}

Ui::updateClientDlg updateClientDlg::getUpdateClientDlgUI() const
{
	return ui;
}
