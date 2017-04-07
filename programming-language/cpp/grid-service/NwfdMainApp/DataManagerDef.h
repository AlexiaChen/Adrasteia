/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: DataManagerDef.h
* 作  者: zhangl		版本：1.0		日  期：2015/12/22
* 描  述：数据管理宏定义
* 其  他：
* 功能列表:
* 修改日志：
*   No. 日期		作者		修改内容
*
*************************************************************************/
#ifndef DATA_MANAGER_DEF_H
#define DATA_MANAGER_DEF_H


#include <QHash>
#include <QList>

// 插值后的站点数据存储
typedef QHash<QString, float> HASH_STATION_TIME;              // Key: Time
typedef QHash<QString, HASH_STATION_TIME*>  HASH_STATION_NO;  // Key: StationNo
typedef QHash<QString, HASH_STATION_NO*>  HASH_STATION_DATA;  // Key: Type_Range

// 站点类型与格点产品匹配
typedef QHash<QString, QString> HASH_STATION_PRODUCT;

#endif //DATA_MANAGER_DEF_H
