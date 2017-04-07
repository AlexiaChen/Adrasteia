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
	QString url;      // ȫ������url
	QString filename; // ȫ�����ص��ļ���ʽ
	QString savepath; // ȫ�����ر���·��
}ST_DOWNLOAD_ALL;

typedef struct _Server{
    QString type;   // Server����
	QString source; // ����Դ���ͣ����ڱ�ʶ��ͬ������Դ
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
	QString source; // ����Դ���ͣ����ڱ�ʶ��ͬ������Դ
	QString ip;
	QString username;
	QString password;
	QString root;
    ST_LOG      log;
	ST_SCANTIME scantime;
	ST_SEND send;
}ST_UPLOAD;

#endif