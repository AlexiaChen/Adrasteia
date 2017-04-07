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

	// 还没有完成
	m_bComplete = false;

	// 通过第一条数据获取连接信息
	ST_JOB  stJobFirst = m_lstJob->first();
	QString strRemoteIP = stJobFirst.strRemoteIP;
	QString strUserName = stJobFirst.strRemoteUser;
	QString strPassword = stJobFirst.strRemotePassword;

	//初始化sender
	m_sender = new HttpSender();

	if (!m_sender->isValid())
	{
		//初始化失败
		delete m_sender;
		m_sender = NULL;
		return;
	}

	QString strLocalFile;
	QString strRemoteFile;
	// 开始传输数据
	LIST_JOB::iterator iter;

	for (iter = m_lstJob->begin(); iter != m_lstJob->end(); iter++)
	{
		ST_JOB  stJob = *iter;

		if (m_sender->isValid())
		{
			// 开始执行时间
			stJob.tStartTime = QDateTime::currentDateTime();

			// 文件路径
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
					LOG_(LOGID_DDS, LOG_F("[HTTP][%s][%s]数据分发成功"), LOG_STR(stJob.strProductKey), LOG_STR(stJob.strFileName));
				}
				else
				{
					stJob.nRemoteFileSize = errcode;
					stJob.strJobStatus = "abnormal";
					stJob.strErrReason = QString("upload file failed through http,errorcode= %1").arg(stJob.nRemoteFileSize);
				}

			}

			// 结束时间
			stJob.tFinishTime = QDateTime::currentDateTime();
		}

		//  将执行结果写入数据库
		int nErr = HandleDB::getClass().UpdateJobResult(stJob);
		if (nErr == 0)
		{
			// todo 保存处理结果到数据库成功
			LOG_(LOGID_DEBUG, LOG_F("[%s][%s]更新数据库状态成功[%s]"), LOG_STR(stJob.strProductKey), LOG_STR(stJob.strFileName), LOG_STR(stJob.strJobStatus));
		}
		else
		{
			// todo 保存处理结果到数据库失败
			LOG_(LOGID_DEBUG, LOG_F("[%s][%s]更新数据库状态成功[%s]"), LOG_STR(stJob.strProductKey), LOG_STR(stJob.strFileName), LOG_STR(stJob.strJobStatus));
		}
		
	}

	// 执行完成，断开ftp连接
	if (m_sender)
	{
		delete m_sender;
		m_sender = NULL;
	}

	// 处理完成
	m_bComplete = true;


}