#ifndef TASK_HTTP_RECVER_H
#define TASK_HTTP_RECVER_H

#include "TaskBase.h"
#include <curl/curl.h>
#include <string>
#include <cstdio>
#include <QVector>
#include <QString>
#include <QStringList>
#include <QHash>

class HttpRecver :public TaskBase{

	Q_OBJECT

public:
	
	HttpRecver();
	~HttpRecver();

	typedef std::vector<std::string> ReportTimes;

	bool isValid() const;
	void initDownLoadReportTimes(const ReportTimes & list);
	
	void parseProductDownPath(const std::string &xmlurl);
	void setDownLoadRootPath(const std::string &root);
	void setXmlUrl(std::string xmlurl);

protected:
	void run();

private slots:
	void clearYesterdayData();

private:
	void downFilesFromURLPath(const std::string& path,const std::string &fileformat,const std::string& ele);
	void setUrl(const std::string& url);
	void setSavePath(const std::string &path);
	bool isDownLoaded(const std::string &filename);
	
	bool recv();
	bool isValidProductFile(const std::string &filename);
	//下载回调函数
	static size_t DownloadCallback(void* pBuffer, size_t nSize, size_t nMemByte, void* pParam);
	std::string getFileNameFromPath(const std::string& path);
	void HandleClearHash();

private:
	FILE *m_fp;
	CURL* m_hCurl;
	//QTimer m_timer;
	std::string m_strXmlUrl; 
	std::string m_url;
	std::string m_savePath;
	std::string m_filename;
	std::string m_rootPath;
	bool m_isValid;
	bool m_bReady;
	
	typedef struct _HttpDownLoad{
		QString ele;
		QString url;
		QString fileformat;
	}HttpDownLoad;
	ReportTimes m_reporttimes;
	
	typedef QVector<HttpDownLoad> ProductPathAndFormatList;
	ProductPathAndFormatList m_productPathList;
	QHash<QString, bool> m_downLoadedFlag;
	QHash<QString, QStringList> m_filesFromPath;

	QString  m_strClearHasFlg;  // 已处理产品Hash标记维护判断
};

#endif