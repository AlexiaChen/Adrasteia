#include "FtpTest.h"

#include <QDomDocument>
#include <QFile>
#include <QDateTime>
#include <QRegExp>
#include "HandleCommon.h"
#include "FTP.h"

#define  NWFDSYS_CONFIG_FILE    "NwfdServerConfig.xml"

FtpTest::FtpTest(QWidget *parent)
    : QDialog(parent), m_ftp(NULL)
{
    ui.setupUi(this);
 #ifdef WIN32
    WSADATA wsaData = { 0 };
   WSAStartup(MAKEWORD(2, 2), &wsaData);
 #endif

    initSlots();

    makeDownloadDir();
}

FtpTest::~FtpTest()
{

}

void FtpTest::initSlots()
{
    connect(ui.refreshCfg, SIGNAL(clicked()), this, SLOT(refreshCfg()));
    connect(ui.ConnectFtp, SIGNAL(clicked()), this, SLOT(connectFtp()));
    connect(ui.refreshList, SIGNAL(clicked()), this, SLOT(refreshDir()));
    connect(ui.downLoad, SIGNAL(clicked()), this, SLOT(downloadFile()));
}

void FtpTest::connectFtp()
{
    
    if (m_ftp)
    {
        delete m_ftp;
        m_ftp = NULL;
    }
    
    try{

        m_ftp = new  CFTP();
        if (m_ftp != NULL)
        {
            FTP_API ret;

            // 登录

            if (m_download.ip.isEmpty() || m_download.user.isEmpty() || m_download.password.isEmpty())
            {
                ui.cfgShow->appendPlainText(QStringLiteral("IP，用户名和密码为空,请刷新配置"));
                return;
            }

            ui.cfgShow->appendPlainText(QStringLiteral("正在连接FTP..."));
            
            ret = m_ftp->login2Server(m_download.ip.toStdString());
            ret = m_ftp->inputUserName(m_download.user.toStdString());
            ret = m_ftp->inputPassWord(m_download.password.toStdString());
            ret = m_ftp->setTransferMode(CFTP::binary);
        }

    }

    catch (...)
    {
        if (m_ftp)
        {
            delete m_ftp;
            m_ftp = NULL;

        }
    }

    

    if (m_ftp->IsLogin())
    {
        ui.cfgShow->appendPlainText(QStringLiteral("连接FTP成功"));
    }
    else
    {
        ui.cfgShow->appendPlainText(QStringLiteral("连接FTP失败"));
    }
}

void FtpTest::refreshDir()
{
    
    ui.remoteList->clear();
    
    QStringList files;
    ui.cfgShow->appendPlainText(QStringLiteral("正在获取远程文件列表..."));
    if (!getRemoteFiles(files, m_download.url))
    {
        ui.cfgShow->appendPlainText(QStringLiteral("获取远程文件列表失败"));
        return;
    }

    for (int i = 0; i < files.size(); ++i)
    {
        QString filename = files.at(i);

        QRegExp rx(m_download.filename.toStdString().c_str());
        rx.setPatternSyntax(QRegExp::Wildcard);
        if (rx.exactMatch(filename))
        {
            ui.remoteList->addItem(filename);
        }

       
    }
}

void FtpTest::refreshCfg()
{
    ui.cfgShow->clear();
    
    //1.读取XML
    QFile file(NWFDSYS_CONFIG_FILE);
    if (!file.open(QIODevice::ReadOnly))
    {
        ui.cfgShow->appendPlainText(QStringLiteral("打开配置文件失败"));
        return ;
    }

    QDomDocument domDocument;
    if (!domDocument.setContent(&file))
    {
      
        file.close();
        return ;
    }
    file.close();

    QDomElement root = domDocument.documentElement();

    m_download.rootPath = root.firstChildElement("root").text();
    m_download.CCCC = root.firstChildElement("cccc").text();

    //2.读取Ftp下载的配置
    QDomElement downloadEle = root.firstChildElement("download");

    if (!downloadEle.isNull())
    {
        QString serverIp = downloadEle.attribute("ip");
        QString userName = downloadEle.attribute("user");
        QString passWord = downloadEle.attribute("password");

        m_download.ip = serverIp;
        m_download.user = userName;
        m_download.password = passWord;

        ui.cfgShow->appendPlainText(QStringLiteral("服务器IP：%1\n用户名:%2\n密码:%3\n").arg(serverIp).arg(userName).arg(passWord));


        //2. all cfg
        QDomElement allEle = downloadEle.firstChildElement("all");
        if (!allEle.isNull())
        {
            QString url = allEle.attribute("url");
            QString filename = allEle.attribute("filename");
           
            QDateTime date = QDateTime::currentDateTime();
            int nYear = date.date().year();
            int nMonth = date.date().month();
            int nDay = date.date().day();
            m_download.url = HandleCommon::GetFolderPath(url, nYear, nMonth, nDay);
            m_download.filename = filename;

            ui.cfgShow->appendPlainText(QStringLiteral("URL：%1\n文件匹配模式:%2\n").arg(m_download.url).arg(m_download.filename));
        }
        else
        {
            ui.cfgShow->appendPlainText(QStringLiteral("配置文件中没有all配置"));
        }
        
    }
    else
    {
        ui.cfgShow->appendPlainText(QStringLiteral("配置文件中没有下载配置"));
    }

}

void FtpTest::downloadFile()
{
    //1.得到文件列表中当前选择的文件
    QString remoteFileName = ui.remoteList->currentItem()->text();
    if (!remoteFileName.isEmpty())
    {
        if (downloadFile(m_savePath, remoteFileName, remoteFileName, m_download.url))
        {
            ui.cfgShow->appendPlainText(QStringLiteral("文件[%1]下载到[%2]成功").arg(remoteFileName).arg(m_savePath));
        }
        else
        {
            ui.cfgShow->appendPlainText(QStringLiteral("文件[%1]下载到[%2]失败").arg(remoteFileName).arg(m_savePath));
        }


    }
}

bool FtpTest::getRemoteFiles(QStringList& list, const QString &remotePath)
{
    if (!m_ftp)
    {
        return false;
    }

    std::string lists = m_ftp->Dir(remotePath.toStdString());

    list = parseFileList(lists);

    return true;
}

QStringList FtpTest::parseFileList(const std::string& filelist)
{
    QStringList file_lines, files;
    QString fileList = filelist.c_str();

    file_lines = fileList.split("\r\n");

    for (int i = 0; i < file_lines.size(); ++i)
    {
        QString file_line = file_lines.at(i);

        if (!file_line.isEmpty())
        {
            QString file_name = file_line.split(QRegularExpression("\\s+"))[8];
            files.push_back(file_name);
        }


    }

    return files;
}

void FtpTest::makeDownloadDir()
{
    QString currentPath = QDir::currentPath();
    QString saveFilePath = currentPath + QString("/") + QString("download");
    QDir saveFileDir(saveFilePath);
    if (!saveFileDir.exists())
    {
        saveFileDir.mkpath(saveFilePath);
    }

    m_savePath = saveFilePath;
}

bool FtpTest::downloadFile(const QString &localSavePath, const QString& localFileName, const QString& remoteFileName, const QString& remotePath)
{
    if (!m_ftp)
    {
        return false;
    }

    QString tmp_save_file_name;
    QString saved_file_name;
    if (localFileName.isEmpty())
    {
        tmp_save_file_name = localSavePath + "/" + remoteFileName + ".tmp";
        saved_file_name = localSavePath + "/" + remoteFileName;
    }
    else
    {
        tmp_save_file_name = localSavePath + "/" + localFileName + ".tmp";
        saved_file_name = localSavePath + "/" + localFileName;
    }

  
    m_ftp->CD(remotePath.toStdString());

    if (m_ftp->Get(remoteFileName.toStdString(), tmp_save_file_name.toStdString()) != 0)
    {
        return false;
    }

    bool ret = QFile::rename(tmp_save_file_name, saved_file_name) ? true : false;

    return ret;
}
