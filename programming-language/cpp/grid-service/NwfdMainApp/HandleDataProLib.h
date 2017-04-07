/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: HandleDataProLib.h
* 作  者: zhangl  		版本：1.0		日  期：2015/05/26
* 描  述：数据加工处理动态库维护类
* 其  他：
* 功能列表:
*   1.
* 修改日志：
*   No.   日期		  作者		       修改内容
* ------------------------------------------------------------------------
*************************************************************************/
#ifndef HANDLE_DATAPROLIB_H
#define HANDLE_DATAPROLIB_H

#include "DataProInterface.h"
#include "LibraryDef.h"
#include <QList>  
#include <QHash>
#include <QString>

/************************************************************************/
/* 数据处理列表                                                         */
/************************************************************************/
typedef QList<DataProInterface *>  LIST_DATAPROC; // 一条数据加工处理链
typedef QHash<int, LIST_DATAPROC>  HASH_DATAPROC_GROUP; // 数据加工处理链组
/************************************************************************/
/* 数据处理动态库调用接口声明定义                                       */
/************************************************************************/
typedef DataProInterface * (*lpGetDataProcesser)(void);

/************************************************************************/
/* 动态库句柄列表                                                       */
/************************************************************************/
typedef QList<LIB_HANDLE>  LIST_HINSTANCE;

/************************************************************************/
/* 类 名： HandleDataProLib                                             */
/* 父 类： -                                                            */
/* 说 明： 数据加工处理动态库维护类                                     */
/* 描 述： 单例                                                         */
/************************************************************************/
class HandleDataProLib
{
private:
	HandleDataProLib();

public:
	~HandleDataProLib();

public:
	static HandleDataProLib & getClass();

	DataProInterface * GetDataProcesser(QString strLibName);

	void ClearListHinstance();
private:
	LIST_HINSTANCE  m_lstHinstance;
};

#endif //HANDLE_DATAPROLIB_H
