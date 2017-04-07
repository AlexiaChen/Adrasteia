#include "TaskFtpRecver.h"

#include "log.h"
#include "FTP.h"
#include "HandleCommon.h"
#include "ProductTaskManager.h"  // 产品扫描任务管理
#include <QString>
#include <QVector>
#include <QRegExp>
#include <QFile>
#include <QRegularExpression>
#include <QDir>

TaskFtpRecver::TaskFtpRecver() :m_ftp(NULL)
{
    LOG_(LOGID_DEBUG, LOG_F("Entry TaskFtpRecver()"));
    m_strTaskName = "Ftp下载任务";
    SetTaskType(TASK_DOWNLOAD); // 任务类型
    m_strClearHasFlg = QDateTime::currentDateTime().toString("yyyyMMdd");

    LOG_(LOGID_DEBUG, LOG_F("Leave TaskFtpRecver()"));
}

TaskFtpRecver::~TaskFtpRecver()
{
    LOG_(LOGID_DEBUG, LOG_F("Entry ~TaskFtpRecver()"));

    if (m_ftp) delete m_ftp;

    LOG_(LOGID_DEBUG, LOG_F("Leave ~TaskFtpRecver()"));
}

void TaskFtpRecver::run()
{
    if (m_emTaskState != _TASK_READ)
    {
        // 如果任务没有在准备状态，不予执行此次任务
        LOG_(LOGID_INFO, LOG_F("[TaskPID:%d][%s]任务没有完成准备工作，不能执行，任务结束（当前状态为：%d）."), m_nTaskPID, (int)m_emTaskState);
        return;
    }

    m_bReady = true;

    while (m_bReady)
    {
        switch (m_emTaskState)
        {
        case _TASK_READ:    // 任务执行准备中
        {
            m_emTaskState = _TASK_RUNNING;
            break;
        }

        case _TASK_RUNNING: // 任务执行中
        {
            //to do
            execTask();
            // 当前暂不判断执行结果，是否执行成功都将以正常状态结束（以准备下次执行）
            m_emTaskState = _TASK_STOP;
            break;
        }
        case _TASK_STOP: //  任务结束停止
        {
            // 结束本次执行
            m_bReady = false;


            // 任务正常结束，状态改变为_TASK_READ，为下次执行做准备
            m_emTaskState = _TASK_READ;
            break;
        }
        case _TASK_INTERRUPTIBLE:   // 任务中断挂起
            break;

        case _TASK_UNINTERRUPTIBLE: // 任务不中断（等待）
            break;

        case _TASK_DEAD:    // 任务僵死
        {
            // 结束线程执行
            m_bReady = false;
            break;
        }
        default:
            break;
        }
    }
}

void TaskFtpRecver::login(const QString& ip, const QString &user, const QString &password)
{
    try{

        m_ftp = new  CFTP();
        if (m_ftp != NULL)
        {
            FTP_API ret;

            // 登录
            ret = m_ftp->login2Server(ip.toStdString());
            ret = m_ftp->inputUserName(user.toStdString());
            ret = m_ftp->inputPassWord(password.toStdString());
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

}

bool TaskFtpRecver::isLogin()
{
    if (m_ftp)
    {
        return m_ftp->IsLogin();
    }
    else
    {
        return false;
    }
}

void TaskFtpRecver::setRootPath(QString strRootPath)
{
    m_strRoot = strRootPath;
}
void TaskFtpRecver::setCCCC(QString strCCCC)
{
    m_strCCCC = strCCCC;
}

void TaskFtpRecver::setServer(QString strServerIp, QString strUser, QString strPasswd)
{
    m_server.ip = strServerIp;
    m_server.username = strUser;
    m_server.password = strPasswd;
}

QString TaskFtpRecver::getSourceType()
{
    return m_server.source;
}

void TaskFtpRecver::init(const ST_SERVER& server)
{
    m_server = server;
}

void TaskFtpRecver::execTask()
{
    //清除下载HashFlag
    HanleClearHash();

    //重新连接
    if (!reConnect())
    {
        return;
    }

    if (!m_server.stDownloadAll.bDisabled)
    {
        // 如果下载全部数据功能没有禁用，就执行下载所有
        getAllRemoteFiles();
    }
    else
    {
        //遍历产品列表然后下载
        for (int i = 0; i < m_server.products.size(); ++i)
        {
            ST_DOWNLOADPROD product = m_server.products.at(i);
            //得到当前时间
            QDateTime date;
            date = QDateTime::currentDateTime();
            int year = date.date().year();
            int month = date.date().month();
            int day = date.date().day();
            int  range = product.range.toInt();

            QString url = HandleCommon::GetFolderPath(product.url, year, month, day, "", product.type, m_strCCCC);
            QString filename = HandleCommon::GetFileNameWildCard(product.filename, year, month, day, product.type, range);
            QString savepath = HandleCommon::GetFolderPath(product.savepath, year, month, day, m_strRoot, product.type, m_strCCCC);
            QString savefilename = HandleCommon::GetFileName(product.savefilename, year, month, day, 0, 0, range, product.type, m_strCCCC);

            //获得远程URL下的文件列表
            QStringList files;
            if (getRemoteFiles(files, url))
            {
                download(files, url, filename, savepath, savefilename, year, month, day, product.type, range);
            }
            else
            {
                LOG_(LOGID_DOWNLOAD, LOG_F("产品类型为:%s 获取URL[%s]下的文件列表失败"), LOG_STR(product.type), LOG_STR(product.url));
            }
        }
    }
}

// 下载所有文件
void TaskFtpRecver::getAllRemoteFiles()
{
    // 得到当前时间
    QDateTime date = QDateTime::currentDateTime();
    int year = date.date().year();
    int month = date.date().month();
    int day = date.date().day();

    QString url = m_server.stDownloadAll.url;
    url = HandleCommon::GetFolderPath(url, year, month, day, "", "", m_strCCCC);


    //获得远程URL下的文件列表
    QStringList files;
    if (!getRemoteFiles(files, url))
    {
        LOG_(LOGID_DOWNLOAD, LOG_F("获取URL[%s]下的文件列表失败"), LOG_STR(url));
        return;
    }

    // 下载文件
    for (int i = 0; i < files.size(); ++i)
    {
        QString remoteFileName = files.at(i);

        // 文件名格式判断
        if (!m_server.stDownloadAll.filename.isEmpty())
        {
            QString filepattern = m_server.stDownloadAll.filename;

            filepattern.replace("[YYYY]", QString("").sprintf("%04d", year));
            filepattern.replace("[YY]", QString("").sprintf("%02d", year % 100));
            filepattern.replace("[MM]", QString("").sprintf("%02d", month));
            filepattern.replace("[DD]", QString("").sprintf("%02d", day));

            QRegExp rx(filepattern);
            rx.setPatternSyntax(QRegExp::Wildcard);
            if (!rx.exactMatch(remoteFileName))
            {
                // 不符合下载名称规则，不再下载
                continue;
            }
        }

        // 判断是否下载过
        if (IsDownload(remoteFileName))
        {
            // 已经成功下载过，不再下载
            continue;
        }

        // 解析文件名 Z_NWGD_C_BABJ_*_P_RFFC_SCMOC-[TYPE]_[YYYY][MM][DD][HH]00_*[TT].GRB2
        QString strType;
        int nTimeRange;
        if (!ParseFileName(remoteFileName, strType, nTimeRange))
        {
            // 解析文件名失败
            continue;
        }

        QString strSavePath = m_server.stDownloadAll.savepath;
        strSavePath = HandleCommon::GetFolderPath(strSavePath, year, month, day, m_strRoot, strType, m_strCCCC);
        QString savefilename = remoteFileName;

        if (downloadFile2(strSavePath, savefilename, remoteFileName, url))
        {
            LOG_(LOGID_DOWNLOAD, LOG_F("文件[%s]下载成功"), LOG_STR(remoteFileName), LOG_STR(savefilename));

            m_downloadedHas[remoteFileName] = true;

            if (!m_autoDisable)
            {
                // 下载完成后自动解码没有禁用，启动解码任务
                ProductTaskManager::getClass().StartTask(strType, nTimeRange);
            }
        }
        else
        {
            LOG_(LOGID_DOWNLOAD, LOG_F("文件[%s]下载失败"), LOG_STR(remoteFileName), LOG_STR(savefilename));
            m_downloadedHas[remoteFileName] = false;
        }
    }
}

bool TaskFtpRecver::getRemoteFiles(QStringList& list, const QString &remotePath)
{
    if (!m_ftp)
    {
        return false;
    }

    std::string lists = m_ftp->Dir(remotePath.toStdString());

    list = parseFileList(lists);

    return true;
}

QStringList TaskFtpRecver::parseFileList(const std::string& filelist)
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

void TaskFtpRecver::download(const QStringList& files, const QString& url, const QString& filename, const QString& savePath, const QString& saveFileName, int nYear, int nMonth, int nDay, const QString& strType, int nRange)
{
    for (int i = 0; i < files.size(); ++i)
    {
        QString remoteFileName = files.at(i);
        QRegExp rx(filename.toStdString().c_str());
        rx.setPatternSyntax(QRegExp::Wildcard);
        //符合条件的文件名才进行处理
        if (rx.exactMatch(remoteFileName))
        {
            if (!m_downloadedHas.contains(remoteFileName) ||
                (m_downloadedHas.contains(remoteFileName) && !m_downloadedHas.value(remoteFileName)))
            {
                if (downloadFile2(savePath, saveFileName, remoteFileName, url))
                {
                    LOG_(LOGID_DOWNLOAD, LOG_F("文件[%s]下载成功"), LOG_STR(remoteFileName));

                    m_downloadedHas[remoteFileName] = true;

                    if (!m_autoDisable)
                    {
                        ProductTaskManager::getClass().StartTask(strType, nRange);
                    }
                }
                else
                {
                    LOG_(LOGID_DOWNLOAD, LOG_F("文件[%s]下载失败"), LOG_STR(remoteFileName));
                    m_downloadedHas[remoteFileName] = false;
                }
            }
        }
    }
}

bool TaskFtpRecver::downloadFile(const QString &localSavePath, const QString& localFileName, const QString& remoteFileName, const QString& remotePath)
{
    if (!m_ftp)
    {
        return false;
    }

    QString tmp_save_file_name;
    QString saved_file_name;
    if (localFileName.isEmpty())
    {
        // 如果设置的下载后的文件名为空，则与服务器上的文件名一致
        saved_file_name = localSavePath + "/" + remoteFileName;
    }
    else
    {
        saved_file_name = localSavePath + "/" + localFileName;
    }
    tmp_save_file_name = saved_file_name + ".tmp";

    // 检测该文件是否已经在本地存在，如果已经存在，则不再下载，并且标记为成功
    if (HandleCommon::Exists(saved_file_name))
    {
        // 已经存在
        LOG_(LOGID_DOWNLOAD, LOG_F("文件[%s]已经存在，不再下载"), LOG_STR(localFileName));
        return true;
    }

    //local save path create
    if (!HandleCommon::CreatePath(localSavePath))
    {
        // 已经存在
        LOG_(LOGID_ERROR, LOG_F("创建目录失败"), LOG_STR(localSavePath));
        return false;
    }

    m_ftp->CD(remotePath.toStdString());

    long file_size = m_ftp->getFileLength(remoteFileName.toStdString());
    long recvSize = m_ftp->Get(remoteFileName.toStdString(), tmp_save_file_name.toStdString());

    if (recvSize < 0 || recvSize != file_size)
    {
        return false;
    }

    bool ret = QFile::rename(tmp_save_file_name, saved_file_name);

    return ret;
}

void TaskFtpRecver::HanleClearHash()
{
    QString strNewClearHasFlg = QDateTime::currentDateTime().toString("yyyyMMdd");
    if (strNewClearHasFlg != m_strClearHasFlg)
    {
        m_downloadedHas.clear();
        // 重新赋值
        m_strClearHasFlg = strNewClearHasFlg;
    }
}

bool TaskFtpRecver::IsDownload(QString file)
{
    if (!m_downloadedHas.contains(file))
    {
        return false;
    }

    return m_downloadedHas.value(file);
}

bool TaskFtpRecver::reConnect()
{
    // 没有ServerIP地址，连接失败
    if (m_server.ip.isEmpty())
    {
        return false;
    }

    if (m_ftp)
    {
        delete m_ftp;
        m_ftp = NULL;
    }

    //连接并登录(login函数里直接就调用Connect连接了)
    this->login(m_server.ip, m_server.username, m_server.password);

    int login_count = 3;
    while (!this->isLogin() && login_count > 0)
    {
        LOG_(LOGID_INFO, LOG_F("[ERROR] Ftp Server 登录失败,尝试重新登录..."));
        msleep(500);
        this->login(QString(m_server.ip), QString(m_server.username), QString(m_server.password));
        login_count--;
    }

    if (!this->isLogin())
    {
        LOG_(LOGID_INFO, LOG_F("[ERROR] Ftp Server 登录多次失败.结束任务"));
        return false;
    }

    return true;
}


bool TaskFtpRecver::ParseFileName(QString strFile, QString& strType, int& nTimeRange)
{
    // 解析文件名 Z_NWGD_C_BABJ_yyyymmddhhmiss_P_RFFC_SCMOC-[TYPE]_[YYYY][MM][DD][HH]00_[FFF][TT].GRB2
    QStringList lst = strFile.split("_");
    if (lst.size() != 10)
    {
        // 格式不正确
        return false;
    }
    QString strTmp = lst.at(7);
    QStringList lstType = strTmp.split("-");
    if (lstType.size() != 2)
    {
        return false;
    }
    // 产品类型
    strType = lstType.at(1);

    // 时间间隔
    strTmp = lst.at(9);
    if (strTmp.length() < 5)
    {
        return false;
    }
    nTimeRange = strTmp.mid(3, 2).toInt();

    return true;
}

void TaskFtpRecver::setAutoDisable(bool bDisable)
{
    m_autoDisable = bDisable;
}

bool TaskFtpRecver::downloadFile2(const QString &localSavePath, const QString& localFileName, const QString& remoteFileName, const QString& remotePath)
{
    if (!m_ftp)
    {
        return false;
    }

    QString tmp_save_file_name;
    QString saved_file_name;
    if (localFileName.isEmpty())
    {
        // 如果设置的下载后的文件名为空，则与服务器上的文件名一致
        saved_file_name = localSavePath + "/" + remoteFileName;
    }
    else
    {
        saved_file_name = localSavePath + "/" + localFileName;
    }
    tmp_save_file_name = saved_file_name + ".tmp";

    // 检测该文件是否已经在本地存在，如果已经存在，则不再下载，并且标记为成功
    if (HandleCommon::Exists(saved_file_name))
    {
        // 已经存在
        LOG_(LOGID_DOWNLOAD, LOG_F("文件[%s]已经存在，不再下载"), LOG_STR(localFileName));
        return true;
    }

    //local save path create
    if (!HandleCommon::CreatePath(localSavePath))
    {
        // 已经存在
        LOG_(LOGID_ERROR, LOG_F("创建目录失败"), LOG_STR(localSavePath));
        return false;
    }

    m_ftp->CD(remotePath.toStdString());

    long  local_file_size = 0;

    if (HandleCommon::Exists(tmp_save_file_name))
    {
        local_file_size = getLocalFileSize(tmp_save_file_name);
    }

    long remote_file_size = m_ftp->getFileLength(remoteFileName.toStdString());
    long recvSize = 0;

    if (remote_file_size == -1 )
    {
        return false;
    }

    int conn_count = 10;
    
    while (conn_count != 0 && local_file_size != remote_file_size)
    {
        recvSize = m_ftp->Get2(remoteFileName.toStdString(), tmp_save_file_name.toStdString(), local_file_size);
        if (recvSize <= 0)
        {
            msleep(50);
            if (!reConnect())
            {
                break;
            }
            conn_count--;
        }
       
        local_file_size = getLocalFileSize(tmp_save_file_name);
    }

    bool ret = false;

    if (local_file_size == remote_file_size)
    {
        ret = QFile::rename(tmp_save_file_name, saved_file_name);
    }

    return ret;
}

long TaskFtpRecver::getLocalFileSize(const QString& filename)
{
    QFileInfo tmpFileInfo(filename);

    return tmpFileInfo.size();
}
