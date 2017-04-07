/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: DDSBase.h
* 作  者: zhangl  		版本：1.0		日  期：2015-07-14
* 描  述：数据分发器基类
* 其  他：
* 功能列表:
* 修改日志：
*   No.   日期		  作者		       修改内容
* ------------------------------------------------------------------------
*************************************************************************/
#ifndef DDS_BASE_H
#define DDS_BASE_H

#include "DDSDef.h"
#include <QThread>

/************************************************************************/
/* 类 名： CDDSBase                                                     */
/* 父 类： QThread                                                      */
/* 说 明： 数据分发器基类(接口类)                                       */
/* 描 述：                                                              */
/************************************************************************/
class CDDSBase : public QThread
{
	Q_OBJECT

public:
	CDDSBase();
	virtual ~CDDSBase();

	virtual void SetJobList(LIST_JOB  *lstJob);
	virtual bool IsComplete();
protected:
	virtual void run() = 0;

protected:
	LIST_JOB  *m_lstJob;
	bool       m_bComplete;
};

#endif //DDS_BASE_H
