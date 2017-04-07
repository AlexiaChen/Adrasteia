#ifndef TASKFTPRECVER_H
#define TASKFTPRECVER_H

#include "TaskBase.h"
#include <string>
#include <vector>
#include <QHash>
#include "FtpDef.h"

class CFTP;
class TaskFtpRecver : public TaskBase
{
   Q_OBJECT

public:
	TaskFtpRecver();
	~TaskFtpRecver();
	
	void login(const QString& ip, const QString &user, const QString &password);
	void init(const ST_SERVER& server);
	bool isLogin();

	// ����
    void setAutoDisable(bool bDisable);
	void setRootPath(QString strRootPath);
	void setCCCC(QString strCCCC);
	void setServer(QString strServerIp, QString strUser, QString strPasswd);
	QString getSourceType();

protected:
	void run();

private:
	void execTask();
	void getAllRemoteFiles();
	bool getRemoteFiles(QStringList& list, const QString &remotePath);
	QStringList parseFileList(const std::string& filelist);
	void download(const QStringList& files, const QString& url, const QString& filename, const QString& savePath, const QString& saveFileName, int nYear, int nMonth, int nDay, const QString& strType, int nRange);
	bool downloadFile(const QString &localSavePath, const QString& localFileName, const QString& remoteFileName, const QString& remotePath);
    
    /*֧�ֶϵ�����*/
    bool downloadFile2(const QString &localSavePath, const QString& localFileName, const QString& remoteFileName, const QString& remotePath);
    
    void HanleClearHash();
	bool IsDownload(QString file);
    bool reConnect();

	bool ParseFileName(QString strFile, QString& strType, int& nTimeRange);

    long getLocalFileSize(const QString& filename);

private:
	ST_SERVER m_server;
    bool m_autoDisable;
	bool m_bReady;
	CFTP *m_ftp;

	QString m_strRoot; // ��Ŀ¼
	QString m_strCCCC; // ʡ����
    QString m_strClearHasFlg;

	///////////////////////
	QHash<QString, bool> m_downloadedHas;
};

#endif // TASKFTPRECVER_H
