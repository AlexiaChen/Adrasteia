#ifndef FTPTEST_H
#define FTPTEST_H

#include <QtWidgets/QDialog>
#include <QString>
#include <QStringList>
#include "ui_ftptest.h"

class CFTP;

class FtpTest : public QDialog
{
    Q_OBJECT

public:
    FtpTest(QWidget *parent = 0);
    ~FtpTest();

    void initSlots();
    void makeDownloadDir();

public slots:
    void refreshCfg();
    void connectFtp();
    void refreshDir();

    //to do
    void downloadFile();

private:
    bool getRemoteFiles(QStringList& list, const QString &remotePath);
    bool downloadFile(const QString &localSavePath, const QString& localFileName, const QString& remoteFileName, const QString& remotePath);
    QStringList parseFileList(const std::string& filelist);

private:
    Ui::FtpTestClass ui;
    CFTP* m_ftp;

    QString m_savePath;

    typedef struct _DownLoad{
        QString CCCC;
        QString rootPath;
        QString ip;
        QString user;
        QString password;
        QString url;
        QString filename;
    }ST_DOWNLOAD;

    ST_DOWNLOAD m_download;
};

#endif // FTPTEST_H
