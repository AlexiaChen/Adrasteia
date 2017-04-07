// NwfdScaleLib.cpp : 定义 DLL 应用程序的导出函数。
//

#include "NwfdScaleLib.h"

/************************************************************************/
/*  DLL 应用程序的导出函数  GetDataProcesser                            */
/************************************************************************/
DataProInterface * GetDataProcesser()
{
	DataProInterface * pProcesser = new DataProcesser();

	return pProcesser;
}

