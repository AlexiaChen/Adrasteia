#ifndef FTP_DEF_H
#define FTP_DEF_H

#include <string>
#include <vector>
#include "ProductDef.h"

typedef  struct _DownloadProduct{
	QString type;
	
    QString range;
	QString url;
	QString filename;
	QString savepath;
	QString savefilename;
}ST_DOWNLOADPROD;

typedef  struct _DownloadAll{
	bool bDisabled;
	QString url;      // 全部下载url
	QString filename; // 全部下载的文件格式
	QString savepath; // 全部下载保存路径
}ST_DOWNLOAD_ALL;

typedef struct _Server{
    QString type;   // Server类型
	QString source; // 数据源类型，用于标识不同的下载源
    QString ip;
    QString username;
    QString password;
    ST_SCANTIME scantime;
	ST_DOWNLOAD_ALL stDownloadAll; 
    std::vector<ST_DOWNLOADPROD> products;
}ST_SERVER;

typedef struct _DownLoad{
	bool bDisabled;
    bool autoDisable;
    std::vector<ST_SERVER> servers;
}ST_DOWNLOAD;

typedef struct _Send{
	QString src;
    QString range;
	QString url;
	QString filename;
}ST_SEND;

typedef struct _Log{
    QString path;
    QString logfile;
}ST_LOG;

typedef struct _UpLoad{
	bool bDisabled;
	QString type;
	QString source; // 数据源类型，用于标识不同的下载源
	QString ip;
	QString username;
	QString password;
	QString root;
    ST_LOG      log;
	ST_SCANTIME scantime;
	ST_SEND send;
}ST_UPLOAD;

#endif