#include "DDSHttp.h"

#include "log.h"

#include "HandleDB.h"
#include <QDateTime>

CDDSHttp::CDDSHttp():
m_bReady(false), m_sender(NULL)
{
	m_bComplete = true;
}

CDDSHttp::~CDDSHttp()
{
	m_bReady = false;

	if (m_sender)
	{
		delete m_sender;
		m_sender = NULL;
	}
}

void CDDSHttp::run()
{
	if (m_lstJob == NULL || m_lstJob->size() == 0)
	{
		return;
	}

	// ��û�����
	m_bComplete = false;

	// ͨ����һ�����ݻ�ȡ������Ϣ
	ST_JOB  stJobFirst = m_lstJob->first();
	QString strRemoteIP = stJobFirst.strRemoteIP;
	QString strUserName = stJobFirst.strRemoteUser;
	QString strPassword = stJobFirst.strRemotePassword;

	//��ʼ��sender
	m_sender = new HttpSender();

	if (!m_sender->isValid())
	{
		//��ʼ��ʧ��
		delete m_sender;
		m_sender = NULL;
		return;
	}

	QString strLocalFile;
	QString strRemoteFile;
	// ��ʼ��������
	LIST_JOB::iterator iter;

	for (iter = m_lstJob->begin(); iter != m_lstJob->end(); iter++)
	{
		ST_JOB  stJob = *iter;

		if (m_sender->isValid())
		{
			// ��ʼִ��ʱ��
			stJob.tStartTime = QDateTime::currentDateTime();

			// �ļ�·��
			strLocalFile = QString("%1/%2").arg(stJob.strFilePath).arg(stJob.strFileName);
			strLocalFile.replace("\\", "/");

			if (stJob.strRemotePath.isEmpty())
			{
				strRemoteFile = stJob.strRemoteFileName;
			}
			else
			{
				QString Url = QString("http://") + stJob.strRemoteIP + QString("/") + stJob.strRemotePath;
				m_sender->setUrl(Url.toStdString().c_str());
				
				int errcode;
				if (m_sender->send(strLocalFile.toStdString(), errcode))
				{
					
					stJob.strJobStatus = "complete";
					stJob.nRemoteFileSize = errcode;
					LOG_(LOGID_DDS, LOG_F("[HTTP][%s][%s]���ݷַ��ɹ�"), LOG_STR(stJob.strProductKey), LOG_STR(stJob.strFileName));
				}
				else
				{
					stJob.nRemoteFileSize = errcode;
					stJob.strJobStatus = "abnormal";
					stJob.strErrReason = QString("upload file failed through http,errorcode= %1").arg(stJob.nRemoteFileSize);
				}

			}

			// ����ʱ��
			stJob.tFinishTime = QDateTime::currentDateTime();
		}

		//  ��ִ�н��д�����ݿ�
		int nErr = HandleDB::getClass().UpdateJobResult(stJob);
		if (nErr == 0)
		{
			// todo ���洦���������ݿ�ɹ�
			LOG_(LOGID_DEBUG, LOG_F("[%s][%s]�������ݿ�״̬�ɹ�[%s]"), LOG_STR(stJob.strProductKey), LOG_STR(stJob.strFileName), LOG_STR(stJob.strJobStatus));
		}
		else
		{
			// todo ���洦���������ݿ�ʧ��
			LOG_(LOGID_DEBUG, LOG_F("[%s][%s]�������ݿ�״̬�ɹ�[%s]"), LOG_STR(stJob.strProductKey), LOG_STR(stJob.strFileName), LOG_STR(stJob.strJobStatus));
		}
		
	}

	// ִ����ɣ��Ͽ�ftp����
	if (m_sender)
	{
		delete m_sender;
		m_sender = NULL;
	}

	// �������
	m_bComplete = true;


}