// NwfdScaleLib.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "NwfdScaleLib.h"

/************************************************************************/
/*  DLL Ӧ�ó���ĵ�������  GetDataProcesser                            */
/************************************************************************/
DataProInterface * GetDataProcesser()
{
	DataProInterface * pProcesser = new DataProcesser();

	return pProcesser;
}

