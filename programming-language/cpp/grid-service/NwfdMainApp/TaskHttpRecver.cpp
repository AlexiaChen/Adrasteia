#include "TaskHttpRecver.h"

#include "log.h"
#include <QString>
#include <QFile>
#include <QDomDocument>
#include <QTextStream>
#include <QDateTime>
#include <QDir>
#include <QList>
#include "HandleCommon.h"

HttpRecver::HttpRecver() :
m_isValid(false), m_hCurl(NULL), m_fp(NULL)
{
	LOG_(LOGID_DEBUG, LOG_F("Entry HttpRecver()"));
	m_strTaskName = "HTTP下载任务";

	m_strClearHasFlg = "";
	m_savePath = "";
	curl_global_init(CURL_GLOBAL_ALL);
	m_hCurl = curl_easy_init();

	if (m_hCurl)
	{
		m_isValid = true;
	}
	
	LOG_(LOGID_DEBUG, LOG_F("Entry HttpRecver()"));
}

HttpRecver::~HttpRecver()
{
	LOG_(LOGID_DEBUG, LOG_F("Entry ~HttpRecver()"));
	
	if (m_hCurl)
	{
		curl_easy_cleanup(m_hCurl);
	}

	curl_global_cleanup();

	LOG_(LOGID_DEBUG, LOG_F("Leave ~HttpRecver()"));
}

bool HttpRecver::isValid() const
{
	return m_isValid;
}

void HttpRecver::setDownLoadRootPath(const std::string &root)
{
	QDir tmpPath(QString(root.c_str()));

	if (tmpPath.exists())
	{
		m_rootPath = root;
	}
	else
	{
		tmpPath.mkpath(QString(root.c_str()));
		m_rootPath = root;
	}
}
void HttpRecver::setXmlUrl(std::string xmlurl)
{
	m_strXmlUrl = xmlurl;
}

void HttpRecver::parseProductDownPath(const std::string &xmlurl)
{
	setUrl(xmlurl);
	
	QDir dir = QDir::currentPath();

	setSavePath(dir.absolutePath().toStdString());

	if (recv())
	{
		LOG_(LOGID_DEBUG, LOG_F("fetch product download path's xml success"));
		
		QString xmlFilePath = QString(m_savePath.c_str()) + QString("/") + QString(getFileNameFromPath(xmlurl).c_str());
		QFile xmlFile;
		xmlFile.setFileName(xmlFilePath);


		if (!xmlFile.open(QFile::ReadWrite | QFile::Text)) {
			LOG_(LOGID_DEBUG, LOG_F("open download product xml failed"));
			m_isValid = false;
		}

		QString         strError;
		int             errLin = 0, errCol = 0;
		QDomDocument doc;
		if (!doc.setContent(&xmlFile, false, &strError, &errLin, &errCol)) {
			printf("parse file failed at line %d column %d, error: %s !\n", errLin, errCol, strError);
			m_isValid = false;
		}

		if (doc.isNull()) {
			printf("document is null !\n");
			m_isValid = false;
		}

		QDomElement root = doc.documentElement();
		QDomElement ndgd = root.firstChildElement("ndgd");
		if (!ndgd.isNull())
		{
			m_productPathList.clear();
		}
		while (!ndgd.isNull())
		{
			HttpDownLoad download;

			download.ele = ndgd.attribute("ele");
			download.url = ndgd.attribute("url");
			download.fileformat = ndgd.attribute("format");
			m_productPathList.push_back(download);
			
			ndgd = ndgd.nextSiblingElement();
		}

	}
	else
	{
		LOG_(LOGID_DEBUG, LOG_F("Cannot fetch the XML Config for product download path"));
	}
}

void HttpRecver::clearYesterdayData()
{
	QDateTime yesterday = HandleCommon::GetYesterday();
	std::string date = yesterday.toString("yyyyMMdd").toStdString();
	//通过文件名上的时间戳找出昨天的Hash的Key
	QList<QString> allKeys = m_downLoadedFlag.keys();

	for (int i = 0; i < allKeys.count(); ++i)
	{
		std::string key = allKeys.at(i).toStdString();
		
		if (key.find_last_of(date) != std::string::npos)
		{
			m_downLoadedFlag.remove(QString(key.c_str()));
		}
	}

}

void HttpRecver::HandleClearHash()
{
	// 按天进行标记判断处理
	QString strNewClearHasFlg = QDateTime::currentDateTime().toString("yyyyMMdd");
	if (strNewClearHasFlg != m_strClearHasFlg)
	{
		// 日期已经切换，结果判断列表清空，防止无限增长
		// 清除昨天的记录
		m_downLoadedFlag.clear();

		// 重新赋值
		m_strClearHasFlg = strNewClearHasFlg;
	}
}

bool HttpRecver::isDownLoaded(const std::string &filename)
{
	if (m_downLoadedFlag.contains(QString(filename.c_str())))
	{
		return m_downLoadedFlag[QString(filename.c_str())];
	}

	return false;
}

void HttpRecver::downFilesFromURLPath(const std::string& path, const std::string &fileformat, const std::string& ele)
{
	QStringList files;
	

	for (size_t i = 0; i < m_reporttimes.size(); ++i)
	{
		QString file = QString(fileformat.c_str());
		file.replace("HH", QString(m_reporttimes.at(i).c_str()));

		files.push_back(file);
	}
	

	std::string savePath = m_rootPath + std::string("/") + ele;
	
	//检查存储目录是否存在，如果不存在，就创建，创建成功true，失败fale，存在true
	if (HandleCommon::HandleGribFolder(QString(savePath.c_str())))
	{
		setSavePath(savePath);
	
		for (int i = 0; i < files.count(); ++i)
		{
			if (!isDownLoaded(files.at(i).toStdString()))
			{
				QString filepath = QString(path.c_str()) + files.at(i);

				setUrl(filepath.toStdString());

				if (recv())
				{
					m_downLoadedFlag[files.at(i)] = true;
				}
				else
				{
					m_downLoadedFlag[files.at(i)] = false;
				}

			}
			else
			{
				m_downLoadedFlag[files.at(i)] = true;
			}
		}
	
	}
	
	
	
	
}

void HttpRecver::setUrl(const std::string& url)
{
	m_url = url;
	m_filename = getFileNameFromPath(m_url);
}

void HttpRecver::setSavePath(const std::string &path)
{
	QDir tmpPath(QString(path.c_str()));

	if (tmpPath.exists())
	{
		m_savePath = path;
	}
	else
	{
		tmpPath.mkpath(QString(path.c_str()));
		m_savePath = path;
	}
}

std::string HttpRecver::getFileNameFromPath(const std::string& path)
{
	return path.substr(path.find_last_of("/\\") + 1);
}

bool HttpRecver::isValidProductFile(const std::string &filename)
{
	QFile file(filename.c_str());

	if (!file.open(QFile::ReadOnly | QFile::Text))
	{
		return false;
	}

	QTextStream out(&file);

	QString text = out.readAll();

	if (text.contains(QString("<html>"), Qt::CaseInsensitive))
	{
		file.close();
		return false;
	}

	file.close();
	return true;
}

bool HttpRecver::recv()
{
	curl_easy_setopt(m_hCurl, CURLOPT_URL, m_url.c_str());

	std::string oldfilePath = m_savePath + std::string("/") + m_filename + std::string(".tmp");
	std::string newfilePath = m_savePath + std::string("/") + m_filename;
	
	m_fp = fopen(oldfilePath.c_str(), "wb");

	if (!m_fp)
	{
		return false;
	}

	//设置接收数据的回调   
	curl_easy_setopt(m_hCurl, CURLOPT_WRITEFUNCTION, DownloadCallback);
	curl_easy_setopt(m_hCurl, CURLOPT_WRITEDATA, m_fp);
	curl_easy_setopt(m_hCurl, CURLOPT_MAXREDIRS, 5);
	curl_easy_setopt(m_hCurl, CURLOPT_FOLLOWLOCATION, 1);

	CURLcode retcCode = curl_easy_perform(m_hCurl);

	if (retcCode != CURLE_OK)
	{

		fclose(m_fp);
		m_fp = NULL;
		return false;
	}

	fclose(m_fp);
	m_fp = NULL;

	if (!isValidProductFile(oldfilePath))
	{
		QFile file(oldfilePath.c_str());
		file.remove();
		return false;
	}

	//判断new file是否存在于本地
	QFile file(newfilePath.c_str());

	if (file.exists())
	{
		file.remove();
	}

	if (!QFile::rename(QString(oldfilePath.c_str()), QString(newfilePath.c_str()))){
	
		return false;
	}
	
	return true;
}

size_t HttpRecver::DownloadCallback(void* pBuffer, size_t nSize, size_t nMemByte, void* pParam)
{
	FILE* fp = (FILE*)pParam;
	size_t nWrite = fwrite(pBuffer, nSize, nMemByte, fp);

	LOG_(LOGID_INFO, LOG_F("transfer bytes is :%d."), nWrite);
	
	return nWrite;
}

void HttpRecver::run()
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
			// 清除标记
			HandleClearHash();

			// 下载XML，获取下载产品
			parseProductDownPath(m_strXmlUrl);
			setDownLoadRootPath(m_rootPath);

			// 产品文件下载处理
			for (int i = 0; i < m_productPathList.count(); ++i)
			{
				QString url = m_productPathList.at(i).url;
				QString fileFormat = m_productPathList.at(i).fileformat;
				QString ele = m_productPathList.at(i).ele;

				QDateTime currentTime = QDateTime::currentDateTime();
				QString date = currentTime.toString("yyyy-MM-dd");
				QStringList ymd = date.split("-");

				url.replace("YYYY", ymd[0]);
				url.replace("MM", ymd[1]);
				url.replace("DD", ymd[2]);

				fileFormat.replace("YYYY", ymd[0]);
				fileFormat.replace("MM", ymd[1]);
				fileFormat.replace("DD", ymd[2]);
				
				downFilesFromURLPath(url.toStdString(),fileFormat.toStdString(),ele.toStdString());
			}
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

void HttpRecver::initDownLoadReportTimes(const ReportTimes & list)
{
	m_reporttimes = list;
}
