/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: NwfdInterpolation.h
* 作  者: zhangl		版本：1.0		日  期：2015/05/25
* 描  述：插值处理dll 外部调用定义
* 其  他：
* 功能列表:
*   1.
* 修改日志：
*   No. 日期		作者		     修改内容
*************************************************************************/
#ifndef NWFD_SCALE_LIB_H
#define NWFD_SCALE_LIB_H

#include "DataProcesser.h"

/************************************************************************/
/* 提供统一外部调用接口  GetDataProcesser，返回DataProBase的子类对象指针*/
/************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

	DataProInterface * GetDataProcesser();

#ifdef __cplusplus
}
#endif

#endif // NWFD_SCALE_LIB_H
