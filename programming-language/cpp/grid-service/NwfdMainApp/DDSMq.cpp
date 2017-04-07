#include "log.h"
#include "DDSMq.h"
#include "HandleDB.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>

/************************************************************************/
/* 构造函数                                                             */
/************************************************************************/
CDDSMq::CDDSMq()
{
	m_bReady = false;
	m_bComplete = true;
}

/************************************************************************/
/* 析构函数                                                             */
/************************************************************************/
CDDSMq::~CDDSMq()
{
	m_bReady = true;
}

/************************************************************************/
/* 数据传输任务执行                                                     */
/************************************************************************/
void CDDSMq::run()
{
	// 判断列表
	if (m_lstJob == NULL || m_lstJob->size() == 0)
	{
		return;
	}

	// 设置标志-未完成
	m_bComplete = false;

	// 通过第一条数据获取连接信息
	ST_JOB  stJobFirst = m_lstJob->first();

	// 当前mq模式在本地-忽略远端地址，用户名，密码
	QString strRemoteIP = stJobFirst.strRemoteIP; 
	QString strUserName = stJobFirst.strRemoteUser;
	QString strPassword = stJobFirst.strRemotePassword;

	QString strLocalFile;
	QString strRemoteFile;

	// 开始传输数据
	LIST_JOB::iterator iter;
	for (iter = m_lstJob->begin(); iter != m_lstJob->end(); iter++)
	{
		ST_JOB  stJob = *iter;

		if (stJob.strRemotePath.isEmpty())
		{
			// 当前mq模式，此目录为空异常
			// 如果没有登录成功，执行失败，将失败原因写入数据库
			stJob.strJobStatus = "abnormal";
			stJob.strErrReason = "remote path is empty";
		}
		else
		{
			// 开始执行时间
			stJob.tStartTime = QDateTime::currentDateTime();

			// 文件路径
			strLocalFile = QString("%1/%2").arg(stJob.strFilePath).arg(stJob.strFileName);
			strLocalFile.replace("\\", "/");

			// 远端路径
			strRemoteFile = QString("%1/%2").arg(stJob.strRemotePath).arg(stJob.strRemoteFileName);
			strRemoteFile.replace("\\", "/");

			// 数据转发
			stJob.nRemoteFileSize = TransferFile(strLocalFile, strRemoteFile);

			// 结束时间
			stJob.tFinishTime = QDateTime::currentDateTime();

			if (stJob.nRemoteFileSize <= 0)
			{
				// 传送失败
				stJob.strJobStatus = "abnormal";
				stJob.strErrReason = QString("Transfer file failed,errorcode= %1").arg(stJob.nRemoteFileSize);
				LOG_(LOGID_DDS, LOG_F("[MQ][%s][%s]数据分发失败[errorcode=%d]"), LOG_STR(stJob.strProductKey), LOG_STR(stJob.strFileName), LOG_STR(stJob.strJobStatus), stJob.nRemoteFileSize);
			}
			else
			{
				stJob.strJobStatus = "complete";
				LOG_(LOGID_DDS, LOG_F("[MQ][%s][%s]数据分发成功（remotefilesize=%d）"), LOG_STR(stJob.strProductKey), LOG_STR(stJob.strFileName), stJob.nRemoteFileSize);
			}
		}

		//  将执行结果写入数据库
		int nErr = HandleDB::getClass().UpdateJobResult(stJob);
		if (nErr == 0)
		{
			// todo 保存处理结果到数据库成功
			LOG_(LOGID_DEBUG, LOG_F("[%s][%s]MQ更新数据库状态成功[%s]"), LOG_STR(stJob.strProductKey), LOG_STR(stJob.strFileName), LOG_STR(stJob.strJobStatus));
		}
		else
		{
			// todo 保存处理结果到数据库失败
			LOG_(LOGID_DEBUG, LOG_F("[%s][%s]MQ更新数据库状态成功[%s]"), LOG_STR(stJob.strProductKey), LOG_STR(stJob.strFileName), LOG_STR(stJob.strJobStatus));
		}
	}

	// 设置标志-完成
	m_bComplete = true;
}

/************************************************************************/
/* 转存文件                                                             */
/************************************************************************/
int CDDSMq::TransferFile(QString strLocalFile, QString strRemoteFile)
{
	// 另存文件临时文件
	QString strRemoteFile_tmp = QString("%1.tmp").arg(strRemoteFile);

	// 检查原始文件是否存在
	QFileInfo fileInfo;
	 
	if (!fileInfo.exists(strLocalFile))
	{
		// 不存在
		return -1;
	}

	// 检查另存文件是否存在
	if (fileInfo.exists(strRemoteFile_tmp))
	{
		// 已经存在，删除文件
		if (!QFile::remove(strRemoteFile_tmp))
		{
			// 删除失败
			return -2;
		}
	}

	// 文件另存
	if (!QFile::copy(strLocalFile, strRemoteFile_tmp))
	{
		return -3;
	}

	// 文件重命名
	if (fileInfo.exists(strRemoteFile))
	{
		// 已经存在，删除文件
		if (!QFile::remove(strRemoteFile))
		{
			// 删除失败
			return -4;
		}
	}
	if (!QFile::rename(strRemoteFile_tmp, strRemoteFile))
	{
		return -5;
	}

	// 获取文件大小
	QFileInfo remoteFileInfo(strRemoteFile);
	int nFileSize = remoteFileInfo.size();  

	return nFileSize;
}


