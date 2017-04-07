#include "TaskFtpSender.h"

#include "log.h"
#include "FTP.h"
#include "HandleCommon.h"

#include <QString>
#include <QVector>
#include <QRegExp>
#include <QFile>
#include <QRegularExpression>
#include <QDir>

TaskFtpSender::TaskFtpSender() :m_ftp(NULL)
{
	LOG_(LOGID_DEBUG, LOG_F("Entry TaskFtpSender()"));
	m_strTaskName = "Ftp�ϴ�����";
	SetTaskType(TASK_UPLOAD); // ��������
  
	LOG_(LOGID_DEBUG, LOG_F("Leave TaskFtpSender()"));
}

TaskFtpSender::~TaskFtpSender()
{
	LOG_(LOGID_DEBUG, LOG_F("Entry ~TaskFtpSender()"));

	if (m_ftp) delete m_ftp;

   

	LOG_(LOGID_DEBUG, LOG_F("Leave ~TaskFtpSender()"));
}

void TaskFtpSender::login(const QString& ip, const QString &user, const QString &password)
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

void TaskFtpSender::init(const ST_UPLOAD& upload)
{
	m_upload = upload;
}

void TaskFtpSender::setRootPath(QString strRootPath)
{
	m_upload.root = strRootPath;
}
void TaskFtpSender::setCCCC(QString strCCCC)
{
	m_strCCCC = strCCCC;
}
void TaskFtpSender::setServer(QString strServerIp, QString strUser, QString strPasswd)
{
	m_upload.ip = strServerIp;
	m_upload.username = strUser;
	m_upload.password = strPasswd;
}

QString TaskFtpSender::getSourceType()
{
	return m_upload.source;
}

bool TaskFtpSender::isLogin()
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

void TaskFtpSender::run()
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

void TaskFtpSender::execTask()
{
    if (!reConnect())
    {
        return;
    }
    
    //�õ���ǰʱ��
    QDateTime date;
    date = QDateTime::currentDateTime();
    int year = date.date().year();
    int month = date.date().month();
    int day = date.date().day();

    //�滻·���ļ���
	QString srcPath = HandleCommon::GetFolderPath(m_upload.send.src, year, month, day, m_upload.root, "", m_strCCCC);
	QString url = HandleCommon::GetFolderPath(m_upload.send.url, year, month, day, "", "",m_strCCCC);

	//ɨ��Ŀ¼�µ��ļ�
    // Ŀ¼����
    QDir srcDir;
    srcDir.setPath(srcPath);
    srcDir.setFilter(QDir::Files);   // �г�Ŀ¼�е��ļ�
    srcDir.setSorting(QDir::Name);   // �����ļ������� 

    // ��ȡ��Ʒ�ļ�
    QFileInfoList files = srcDir.entryInfoList();
    
	uploadFiles(files, url, m_upload.send.filename, year, month, day);
}


void TaskFtpSender::uploadFiles(const QFileInfoList& files, const QString& url, const QString& fileNamePattern, int nYear, int nMonth, int nDay)
{
    
    for (int i = 0; i < files.size(); ++i)
    {
        QString localfile = files.at(i).absoluteFilePath();
        QString fileName = files.at(i).fileName();

        if (!fileNamePattern.isEmpty())
        {
            QRegExp rx(fileNamePattern);
            rx.setPatternSyntax(QRegExp::Wildcard);

            if (!rx.exactMatch(fileName))
            {
                continue;
            }
        }

        if (uploadFile(localfile, url, "", nYear, nMonth, nDay))
        {

            LOG_(LOGID_UPLOAD, LOG_F("�ļ�[%s]�ϴ���url:[%s]�ɹ�"), LOG_STR(localfile), LOG_STR(url));

            //�ϴ��ɹ����ļ���д��һ���ض�����־
            QDateTime time = QDateTime::currentDateTime();
            writeLog(QString("").sprintf("%s,%d-%d-%d %d:%d:%d", fileName.toStdString().c_str(), time.date().year(),
                time.date().month(), time.date().day(), time.time().hour(), time.time().minute(), time.time().second()));

            //ɾ���ļ�
            QFile file(localfile);
            file.remove();
        }
        else
        {
            LOG_(LOGID_UPLOAD, LOG_F("�ļ�[%s]�ϴ���url:[%s]ʧ��"), LOG_STR(localfile), LOG_STR(url));
        }


    }
}

bool TaskFtpSender::uploadFile(const QString& file, const QString& url, const QString& remoteFileName, int nYear, int nMonth, int nDay)
{
   
    // to do create mkdir
    //m_ftp->CreateDirectoryW(url.toStdString());
    
    m_ftp->CD(url.toStdString());

   QString remoteFile = remoteFileName;

   if (remoteFile.isEmpty())
   {
       remoteFile = file.toStdString().substr(file.toStdString().find_last_of("/\\") + 1).c_str();
   }
   remoteFile = HandleCommon::GetFileName(remoteFile, nYear, nMonth,nDay,"");

   int transfer_size = m_ftp->Put(remoteFile.toStdString(), file.toStdString());
   if (transfer_size < 0)
   {
      return false;
   }

   return true;
}

void TaskFtpSender::writeLog(const QString& text)
{
    //�õ���ǰ������
    QDateTime date = QDateTime::currentDateTime();

    int nYear = date.date().year();
    int nMonth = date.date().month();
    int nDay = date.date().day();

	QString logPath = HandleCommon::GetFolderPath(m_upload.log.path, nYear, nMonth, nDay, m_upload.root, "", m_strCCCC);
	QString logfileName = HandleCommon::GetFileName(m_upload.log.logfile, nYear, nMonth, nDay, "");

    QDir logDir(logPath);
    if (!logDir.exists())
    {
        logDir.mkpath(logPath);
    }

    QString logfile = logPath + QString("/") + logfileName;
    QFile logFile;
    QTextStream textOut;
    logFile.setFileName(logfile);

    logFile.open(QIODevice::Append | QIODevice::WriteOnly | QIODevice::Text);

    textOut.setDevice(&logFile);
    
    //д��־
    textOut << text << "\n";

    //����־ˢ�����
    textOut.flush();
    logFile.close();
}

bool TaskFtpSender::reConnect()
{
	// û��ServerIP��ַ������ʧ��
	if (m_upload.ip.isEmpty())
	{
		return false;
	}

    if (m_ftp)
    {
        delete m_ftp;
        m_ftp = NULL;
    }
    
    //���Ӳ���¼(login������ֱ�Ӿ͵���Connect������)
	this->login(m_upload.ip, m_upload.username, m_upload.password);

    int login_count = 3;
    while (!this->isLogin() && login_count > 0)
    {
        LOG_(LOGID_INFO, LOG_F("[ERROR] Ftp Server ��¼ʧ��,�������µ�¼..."));
        msleep(500);
        this->login(QString(m_upload.ip), QString(m_upload.username), QString(m_upload.password));
        login_count--;
    }

    if (!this->isLogin())
    {
        LOG_(LOGID_INFO, LOG_F("[ERROR] Ftp Server ��¼���ʧ��.��������"));
        return false;
    }

    return true;
}
