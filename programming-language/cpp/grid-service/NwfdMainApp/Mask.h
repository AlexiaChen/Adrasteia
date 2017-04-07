/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: NetworkManager.h
* 作  者: zhangl		版本：1.0		日  期：2015/07/16
* 描  述：网络数据处理
* 其  他：
* 功能列表:
* 修改日志：
*   No. 日期		作者		修改内容
*
*************************************************************************/
#ifndef MASK_H
#define MASK_H

#include "ProductDef.h"
#include <QHash>
/************************************************************************/
/* Mask文件列表管理                                                     */
/************************************************************************/
typedef QHash<QString, ST_MASK>  HASH_MASK_INFO;      // mask文件列表
typedef QHash<QString, float*>   HASH_MASK_DATA;      // mask常驻内存数据

/************************************************************************/
/* 类 名： CMask                                                        */
/* 父 类：                                                              */
/* 说 明： 数据合并用的Mask数据管理                                     */
/* 描 述：                                                              */
/************************************************************************/
class CMask
{
private:
	CMask();

public:
	~CMask();
	static CMask & getClass();

public:
	bool AddMaskInfo(ST_MASK stMask);
	float * GetMaskData(QString strMaskFile);
	bool InitMaskData();

private:
	float * ReadMaskDataFromFile(ST_MASK stMask);
	void ClearMaskData();

private:
	HASH_MASK_INFO  m_hasMaskInfo;
	HASH_MASK_DATA  m_hasMaskData;
};

#endif //MASK_H

