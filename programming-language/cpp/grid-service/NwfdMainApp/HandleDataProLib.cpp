#include "log.h"
#include "HandleDataProLib.h"

/************************************************************************/
/* 构造函数                                                             */
/************************************************************************/
HandleDataProLib::HandleDataProLib()
{
}

/************************************************************************/
/* 析构函数                                                             */
/************************************************************************/
HandleDataProLib::~HandleDataProLib()
{
	// 清空动态库句柄列表
	ClearListHinstance();
}

/************************************************************************/
/* 单例调用接口                                                         */
/************************************************************************/
HandleDataProLib & HandleDataProLib::getClass()
{
	static HandleDataProLib objDataProLib;

	return objDataProLib;
}

/************************************************************************/
/* 获取数据处理对象                                                     */
/************************************************************************/
DataProInterface * HandleDataProLib::GetDataProcesser(QString strLibName)
{
	// 动态库文件名称判断
	if (strLibName.isEmpty())
	{
		return NULL;
	}

	LIB_HANDLE hDll  = NULL;
	lpGetDataProcesser funGetDataProcesser = NULL;
	DataProInterface * pDataPro = NULL;

	// 加载动态库
	hDll = Lib_Open(Lib_Str(strLibName));
	if (!hDll)
	{
		// todo
		LOG_(LOGID_ERROR, LOG_F("加载 %s 失败, LastError=%d"), LOG_STR(strLibName), Lib_GetErr());

		return NULL;
	}

	// 添加句柄到列表中
	m_lstHinstance.push_back(hDll);

	// 获取接口函数
	funGetDataProcesser = (lpGetDataProcesser)Lib_GetFun(hDll, "GetDataProcesser");
	if (funGetDataProcesser == NULL)
	{
		// 获取接口函数失败
		return NULL;
	}

	//  获取接口对象
	pDataPro = funGetDataProcesser();

	return pDataPro;
}

/************************************************************************/
/* 清空动态库句柄                                                       */
/************************************************************************/
void HandleDataProLib::ClearListHinstance()
{
	LIST_HINSTANCE::Iterator iter;
	for (iter = m_lstHinstance.begin(); iter != m_lstHinstance.end(); iter++)
	{
		LIB_HANDLE hDll = *iter;
		if (hDll)
		{
			Lib_Close(hDll);
			hDll = NULL;
		}
	}

	m_lstHinstance.clear();
}

