#include "log.h"
#include "DDSMq.h"
#include "HandleDB.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>

/************************************************************************/
/* ���캯��                                                             */
/************************************************************************/
CDDSMq::CDDSMq()
{
	m_bReady = false;
	m_bComplete = true;
}

/************************************************************************/
/* ��������                                                             */
/************************************************************************/
CDDSMq::~CDDSMq()
{
	m_bReady = true;
}

/************************************************************************/
/* ���ݴ�������ִ��                                                     */
/************************************************************************/
void CDDSMq::run()
{
	// �ж��б�
	if (m_lstJob == NULL || m_lstJob->size() == 0)
	{
		return;
	}

	// ���ñ�־-δ���
	m_bComplete = false;

	// ͨ����һ�����ݻ�ȡ������Ϣ
	ST_JOB  stJobFirst = m_lstJob->first();

	// ��ǰmqģʽ�ڱ���-����Զ�˵�ַ���û���������
	QString strRemoteIP = stJobFirst.strRemoteIP; 
	QString strUserName = stJobFirst.strRemoteUser;
	QString strPassword = stJobFirst.strRemotePassword;

	QString strLocalFile;
	QString strRemoteFile;

	// ��ʼ��������
	LIST_JOB::iterator iter;
	for (iter = m_lstJob->begin(); iter != m_lstJob->end(); iter++)
	{
		ST_JOB  stJob = *iter;

		if (stJob.strRemotePath.isEmpty())
		{
			// ��ǰmqģʽ����Ŀ¼Ϊ���쳣
			// ���û�е�¼�ɹ���ִ��ʧ�ܣ���ʧ��ԭ��д�����ݿ�
			stJob.strJobStatus = "abnormal";
			stJob.strErrReason = "remote path is empty";
		}
		else
		{
			// ��ʼִ��ʱ��
			stJob.tStartTime = QDateTime::currentDateTime();

			// �ļ�·��
			strLocalFile = QString("%1/%2").arg(stJob.strFilePath).arg(stJob.strFileName);
			strLocalFile.replace("\\", "/");

			// Զ��·��
			strRemoteFile = QString("%1/%2").arg(stJob.strRemotePath).arg(stJob.strRemoteFileName);
			strRemoteFile.replace("\\", "/");

			// ����ת��
			stJob.nRemoteFileSize = TransferFile(strLocalFile, strRemoteFile);

			// ����ʱ��
			stJob.tFinishTime = QDateTime::currentDateTime();

			if (stJob.nRemoteFileSize <= 0)
			{
				// ����ʧ��
				stJob.strJobStatus = "abnormal";
				stJob.strErrReason = QString("Transfer file failed,errorcode= %1").arg(stJob.nRemoteFileSize);
				LOG_(LOGID_DDS, LOG_F("[MQ][%s][%s]���ݷַ�ʧ��[errorcode=%d]"), LOG_STR(stJob.strProductKey), LOG_STR(stJob.strFileName), LOG_STR(stJob.strJobStatus), stJob.nRemoteFileSize);
			}
			else
			{
				stJob.strJobStatus = "complete";
				LOG_(LOGID_DDS, LOG_F("[MQ][%s][%s]���ݷַ��ɹ���remotefilesize=%d��"), LOG_STR(stJob.strProductKey), LOG_STR(stJob.strFileName), stJob.nRemoteFileSize);
			}
		}

		//  ��ִ�н��д�����ݿ�
		int nErr = HandleDB::getClass().UpdateJobResult(stJob);
		if (nErr == 0)
		{
			// todo ���洦���������ݿ�ɹ�
			LOG_(LOGID_DEBUG, LOG_F("[%s][%s]MQ�������ݿ�״̬�ɹ�[%s]"), LOG_STR(stJob.strProductKey), LOG_STR(stJob.strFileName), LOG_STR(stJob.strJobStatus));
		}
		else
		{
			// todo ���洦���������ݿ�ʧ��
			LOG_(LOGID_DEBUG, LOG_F("[%s][%s]MQ�������ݿ�״̬�ɹ�[%s]"), LOG_STR(stJob.strProductKey), LOG_STR(stJob.strFileName), LOG_STR(stJob.strJobStatus));
		}
	}

	// ���ñ�־-���
	m_bComplete = true;
}

/************************************************************************/
/* ת���ļ�                                                             */
/************************************************************************/
int CDDSMq::TransferFile(QString strLocalFile, QString strRemoteFile)
{
	// ����ļ���ʱ�ļ�
	QString strRemoteFile_tmp = QString("%1.tmp").arg(strRemoteFile);

	// ���ԭʼ�ļ��Ƿ����
	QFileInfo fileInfo;
	 
	if (!fileInfo.exists(strLocalFile))
	{
		// ������
		return -1;
	}

	// �������ļ��Ƿ����
	if (fileInfo.exists(strRemoteFile_tmp))
	{
		// �Ѿ����ڣ�ɾ���ļ�
		if (!QFile::remove(strRemoteFile_tmp))
		{
			// ɾ��ʧ��
			return -2;
		}
	}

	// �ļ����
	if (!QFile::copy(strLocalFile, strRemoteFile_tmp))
	{
		return -3;
	}

	// �ļ�������
	if (fileInfo.exists(strRemoteFile))
	{
		// �Ѿ����ڣ�ɾ���ļ�
		if (!QFile::remove(strRemoteFile))
		{
			// ɾ��ʧ��
			return -4;
		}
	}
	if (!QFile::rename(strRemoteFile_tmp, strRemoteFile))
	{
		return -5;
	}

	// ��ȡ�ļ���С
	QFileInfo remoteFileInfo(strRemoteFile);
	int nFileSize = remoteFileInfo.size();  

	return nFileSize;
}


