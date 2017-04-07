#include "TaskFtpRecver.h"

#include "log.h"
#include "FTP.h"
#include "HandleCommon.h"
#include "ProductTaskManager.h"  // ��Ʒɨ���������
#include <QString>
#include <QVector>
#include <QRegExp>
#include <QFile>
#include <QRegularExpression>
#include <QDir>

TaskFtpRecver::TaskFtpRecver() :m_ftp(NULL)
{
    LOG_(LOGID_DEBUG, LOG_F("Entry TaskFtpRecver()"));
    m_strTaskName = "Ftp��������";
    SetTaskType(TASK_DOWNLOAD); // ��������
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
        // �������û����׼��״̬������ִ�д˴�����
        LOG_(LOGID_INFO, LOG_F("[TaskPID:%d][%s]����û�����׼������������ִ�У������������ǰ״̬Ϊ��%d��."), m_nTaskPID, (int)m_emTaskState);
        return;
    }

    m_bReady = true;

    while (m_bReady)
    {
        switch (m_emTaskState)
        {
        case _TASK_READ:    // ����ִ��׼����
        {
            m_emTaskState = _TASK_RUNNING;
            break;
        }

        case _TASK_RUNNING: // ����ִ����
        {
            //to do
            execTask();
            // ��ǰ�ݲ��ж�ִ�н�����Ƿ�ִ�гɹ�����������״̬��������׼���´�ִ�У�
            m_emTaskState = _TASK_STOP;
            break;
        }
        case _TASK_STOP: //  �������ֹͣ
        {
            // ��������ִ��
            m_bReady = false;


            // ��������������״̬�ı�Ϊ_TASK_READ��Ϊ�´�ִ����׼��
            m_emTaskState = _TASK_READ;
            break;
        }
        case _TASK_INTERRUPTIBLE:   // �����жϹ���
            break;

        case _TASK_UNINTERRUPTIBLE: // �����жϣ��ȴ���
            break;

        case _TASK_DEAD:    // ������
        {
            // �����߳�ִ��
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

            // ��¼
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
    //�������HashFlag
    HanleClearHash();

    //��������
    if (!reConnect())
    {
        return;
    }

    if (!m_server.stDownloadAll.bDisabled)
    {
        // �������ȫ�����ݹ���û�н��ã���ִ����������
        getAllRemoteFiles();
    }
    else
    {
        //������Ʒ�б�Ȼ������
        for (int i = 0; i < m_server.products.size(); ++i)
        {
            ST_DOWNLOADPROD product = m_server.products.at(i);
            //�õ���ǰʱ��
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

            //���Զ��URL�µ��ļ��б�
            QStringList files;
            if (getRemoteFiles(files, url))
            {
                download(files, url, filename, savepath, savefilename, year, month, day, product.type, range);
            }
            else
            {
                LOG_(LOGID_DOWNLOAD, LOG_F("��Ʒ����Ϊ:%s ��ȡURL[%s]�µ��ļ��б�ʧ��"), LOG_STR(product.type), LOG_STR(product.url));
            }
        }
    }
}

// ���������ļ�
void TaskFtpRecver::getAllRemoteFiles()
{
    // �õ���ǰʱ��
    QDateTime date = QDateTime::currentDateTime();
    int year = date.date().year();
    int month = date.date().month();
    int day = date.date().day();

    QString url = m_server.stDownloadAll.url;
    url = HandleCommon::GetFolderPath(url, year, month, day, "", "", m_strCCCC);


    //���Զ��URL�µ��ļ��б�
    QStringList files;
    if (!getRemoteFiles(files, url))
    {
        LOG_(LOGID_DOWNLOAD, LOG_F("��ȡURL[%s]�µ��ļ��б�ʧ��"), LOG_STR(url));
        return;
    }

    // �����ļ�
    for (int i = 0; i < files.size(); ++i)
    {
        QString remoteFileName = files.at(i);

        // �ļ�����ʽ�ж�
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
                // �������������ƹ��򣬲�������
                continue;
            }
        }

        // �ж��Ƿ����ع�
        if (IsDownload(remoteFileName))
        {
            // �Ѿ��ɹ����ع�����������
            continue;
        }

        // �����ļ��� Z_NWGD_C_BABJ_*_P_RFFC_SCMOC-[TYPE]_[YYYY][MM][DD][HH]00_*[TT].GRB2
        QString strType;
        int nTimeRange;
        if (!ParseFileName(remoteFileName, strType, nTimeRange))
        {
            // �����ļ���ʧ��
            continue;
        }

        QString strSavePath = m_server.stDownloadAll.savepath;
        strSavePath = HandleCommon::GetFolderPath(strSavePath, year, month, day, m_strRoot, strType, m_strCCCC);
        QString savefilename = remoteFileName;

        if (downloadFile2(strSavePath, savefilename, remoteFileName, url))
        {
            LOG_(LOGID_DOWNLOAD, LOG_F("�ļ�[%s]���سɹ�"), LOG_STR(remoteFileName), LOG_STR(savefilename));

            m_downloadedHas[remoteFileName] = true;

            if (!m_autoDisable)
            {
                // ������ɺ��Զ�����û�н��ã�������������
                ProductTaskManager::getClass().StartTask(strType, nTimeRange);
            }
        }
        else
        {
            LOG_(LOGID_DOWNLOAD, LOG_F("�ļ�[%s]����ʧ��"), LOG_STR(remoteFileName), LOG_STR(savefilename));
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
        //�����������ļ����Ž��д���
        if (rx.exactMatch(remoteFileName))
        {
            if (!m_downloadedHas.contains(remoteFileName) ||
                (m_downloadedHas.contains(remoteFileName) && !m_downloadedHas.value(remoteFileName)))
            {
                if (downloadFile2(savePath, saveFileName, remoteFileName, url))
                {
                    LOG_(LOGID_DOWNLOAD, LOG_F("�ļ�[%s]���سɹ�"), LOG_STR(remoteFileName));

                    m_downloadedHas[remoteFileName] = true;

                    if (!m_autoDisable)
                    {
                        ProductTaskManager::getClass().StartTask(strType, nRange);
                    }
                }
                else
                {
                    LOG_(LOGID_DOWNLOAD, LOG_F("�ļ�[%s]����ʧ��"), LOG_STR(remoteFileName));
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
        // ������õ����غ���ļ���Ϊ�գ�����������ϵ��ļ���һ��
        saved_file_name = localSavePath + "/" + remoteFileName;
    }
    else
    {
        saved_file_name = localSavePath + "/" + localFileName;
    }
    tmp_save_file_name = saved_file_name + ".tmp";

    // �����ļ��Ƿ��Ѿ��ڱ��ش��ڣ�����Ѿ����ڣ��������أ����ұ��Ϊ�ɹ�
    if (HandleCommon::Exists(saved_file_name))
    {
        // �Ѿ�����
        LOG_(LOGID_DOWNLOAD, LOG_F("�ļ�[%s]�Ѿ����ڣ���������"), LOG_STR(localFileName));
        return true;
    }

    //local save path create
    if (!HandleCommon::CreatePath(localSavePath))
    {
        // �Ѿ�����
        LOG_(LOGID_ERROR, LOG_F("����Ŀ¼ʧ��"), LOG_STR(localSavePath));
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
        // ���¸�ֵ
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
    // û��ServerIP��ַ������ʧ��
    if (m_server.ip.isEmpty())
    {
        return false;
    }

    if (m_ftp)
    {
        delete m_ftp;
        m_ftp = NULL;
    }

    //���Ӳ���¼(login������ֱ�Ӿ͵���Connect������)
    this->login(m_server.ip, m_server.username, m_server.password);

    int login_count = 3;
    while (!this->isLogin() && login_count > 0)
    {
        LOG_(LOGID_INFO, LOG_F("[ERROR] Ftp Server ��¼ʧ��,�������µ�¼..."));
        msleep(500);
        this->login(QString(m_server.ip), QString(m_server.username), QString(m_server.password));
        login_count--;
    }

    if (!this->isLogin())
    {
        LOG_(LOGID_INFO, LOG_F("[ERROR] Ftp Server ��¼���ʧ��.��������"));
        return false;
    }

    return true;
}


bool TaskFtpRecver::ParseFileName(QString strFile, QString& strType, int& nTimeRange)
{
    // �����ļ��� Z_NWGD_C_BABJ_yyyymmddhhmiss_P_RFFC_SCMOC-[TYPE]_[YYYY][MM][DD][HH]00_[FFF][TT].GRB2
    QStringList lst = strFile.split("_");
    if (lst.size() != 10)
    {
        // ��ʽ����ȷ
        return false;
    }
    QString strTmp = lst.at(7);
    QStringList lstType = strTmp.split("-");
    if (lstType.size() != 2)
    {
        return false;
    }
    // ��Ʒ����
    strType = lstType.at(1);

    // ʱ����
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
        // ������õ����غ���ļ���Ϊ�գ�����������ϵ��ļ���һ��
        saved_file_name = localSavePath + "/" + remoteFileName;
    }
    else
    {
        saved_file_name = localSavePath + "/" + localFileName;
    }
    tmp_save_file_name = saved_file_name + ".tmp";

    // �����ļ��Ƿ��Ѿ��ڱ��ش��ڣ�����Ѿ����ڣ��������أ����ұ��Ϊ�ɹ�
    if (HandleCommon::Exists(saved_file_name))
    {
        // �Ѿ�����
        LOG_(LOGID_DOWNLOAD, LOG_F("�ļ�[%s]�Ѿ����ڣ���������"), LOG_STR(localFileName));
        return true;
    }

    //local save path create
    if (!HandleCommon::CreatePath(localSavePath))
    {
        // �Ѿ�����
        LOG_(LOGID_ERROR, LOG_F("����Ŀ¼ʧ��"), LOG_STR(localSavePath));
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
