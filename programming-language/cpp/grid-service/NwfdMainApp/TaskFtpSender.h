#ifndef TASK_FTP_SENDER_H
#define TASK_FTP_SENDER_H

#include "TaskBase.h"
#include <string>
#include <vector>
#include <QFile>
#include <QTextStream>
#include <QFileInfoList>
#include "FtpDef.h"


class CFTP;
class TaskFtpSender : public TaskBase
{
	Q_OBJECT

public:
	TaskFtpSender();
	~TaskFtpSender();

	void login(const QString& ip, const QString &user, const QString &password);
	void init(const ST_UPLOAD& upload);
	bool isLogin();

	// …Ë÷√
	void setRootPath(QString strRootPath);
	void setCCCC(QString strCCCC);
	void setServer(QString strServerIp, QString strUser, QString strPasswd);
	QString getSourceType();

protected:
	void run();

private:
	void execTask();
    void uploadFiles(const QFileInfoList& files, const QString& url, const QString& fileNamePattern, int nYear, int nMonth, int nDay);
    bool uploadFile(const QString& file, const QString& url, const QString& remoteFileName, int nYear, int nMonth, int nDay);
    void writeLog(const QString& text);
   
    bool reConnect();

private:
	ST_UPLOAD m_upload;
	bool m_bReady;
	CFTP *m_ftp;
    QString m_strCCCC;
};

#endif