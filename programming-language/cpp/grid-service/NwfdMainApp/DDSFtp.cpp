#include "log.h"
#include "DDSFtp.h"
#include "HandleDB.h"
#include <QDateTime>

CDDSFtp::CDDSFtp()
{
	m_bReady = false;
	m_ftp = NULL;

	m_bComplete = true;
}

CDDSFtp::~CDDSFtp()
{
	m_bReady = false;

	if (m_ftp)
	{
		delete m_ftp;
		m_ftp = NULL;
	}
}

/************************************************************************/
/* 数据传输任务执行                                                     */
/************************************************************************/
void CDDSFtp::run()
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

	// 创建ftp连接
	if (!CreateFtpManager(strRemoteIP,  strUserName, strPassword))
	{
		// 创建连接失败
		return;
	}

	// 检测是否登录成功
	int  nTryNum = 0;
	bool bLoginFlg = m_ftp->IsLogin();
	while (!bLoginFlg)
	{
		// 尝试次数
		nTryNum++;
		if (nTryNum >= 3)
		{
			break;
		}

		// 延时 10*尝试次数 秒后，尝试重新登录
		sleep(10 * nTryNum);
		if (CreateFtpManager(strRemoteIP, strUserName, strPassword))
		{
			bLoginFlg = m_ftp->IsLogin();
		}
	}

	QString strLocalFile;
	QString strRemoteFile;
	// 开始传输数据
	LIST_JOB::iterator iter;
	for (iter = m_lstJob->begin(); iter != m_lstJob->end(); iter++)
	{
		ST_JOB  stJob = *iter;

		if (!bLoginFlg)
		{
			// 如果没有登录成功，执行失败，将失败原因写入数据库
			stJob.strJobStatus = "abnormal";
			stJob.strErrReason = "login ftp server failed";
		}
		else
		{
			// 开始执行时间
			stJob.tStartTime = QDateTime::currentDateTime();

			// 文件路径
			strLocalFile = QString("%1/%2").arg(stJob.strFilePath).arg(stJob.strFileName);
			strLocalFile.replace("\\","/");
			if (stJob.strRemotePath.isEmpty())
			{
				strRemoteFile = stJob.strRemoteFileName;
			}
			else
			{
				// 有目录的
				//strRemoteFile = QString("%1/%2").arg(stJob.strRemotePath).arg(stJob.strRemoteFileName);
				m_ftp->CD(stJob.strRemotePath.toLocal8Bit().data());
			}

			// 发送数据
			stJob.nRemoteFileSize = m_ftp->Put(strRemoteFile.toLocal8Bit().data(), strLocalFile.toLocal8Bit().data());
			// 结束时间
			stJob.tFinishTime = QDateTime::currentDateTime();

			if (stJob.nRemoteFileSize <= 0)
			{
				// 传送失败
				stJob.strJobStatus = "abnormal";
				stJob.strErrReason = QString("Put file failed,errorcode= %1").arg(stJob.nRemoteFileSize);
				LOG_(LOGID_DDS, LOG_F("[FTP][%s][%s]数据分发失败[errorcode=%d]"), LOG_STR(stJob.strProductKey), LOG_STR(stJob.strFileName), LOG_STR(stJob.strJobStatus), stJob.nRemoteFileSize);
				//LOG_(LOGID_INFO, LOG_F("[%s][%s]FTP数据分发失败[errorcode=%d]"), LOG_STR(stJob.strProductKey), LOG_STR(stJob.strFileName), LOG_STR(stJob.strJobStatus), stJob.nRemoteFileSize);
			}
			else
			{
				stJob.strJobStatus = "complete";
				LOG_(LOGID_DDS, LOG_F("[FTP][%s][%s]数据分发成功（remotefilesize=%d）"), LOG_STR(stJob.strProductKey), LOG_STR(stJob.strFileName), stJob.nRemoteFileSize);
				//LOG_(LOGID_INFO, LOG_F("[%s][%s]FTP数据分发成功（remotefilesize=%d）"), LOG_STR(stJob.strProductKey), LOG_STR(stJob.strFileName), stJob.nRemoteFileSize);
			}
			
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
	if (m_ftp)
	{
		delete m_ftp;
		m_ftp = NULL;
	}

	// 处理完成
	m_bComplete = true;
}

/************************************************************************/
/* 创建FTP连接                                                          */
/************************************************************************/
bool CDDSFtp::CreateFtpManager(QString strRemoteIP, QString strUserName, QString strPassword)
{
	try
	{
		// 删除原先的连接
		if (m_ftp)
		{
			delete m_ftp;
			m_ftp = NULL;
		}

		m_ftp = new  CFTP();
		if (m_ftp == NULL)
		{
			// 创建连接失败
			return false;
		}

		FTP_API ret;

		// 登录
		ret = m_ftp->login2Server(strRemoteIP.toLocal8Bit().data());
		ret = m_ftp->inputUserName(strUserName.toLocal8Bit().data());
		ret = m_ftp->inputPassWord(strPassword.toLocal8Bit().data());
		ret = m_ftp->setTransferMode(CFTP::binary);
	}
	catch (...)
	{
		if (m_ftp)
		{
			delete m_ftp;
			m_ftp = NULL;
		}

		return false;
	}

	return true;
}

