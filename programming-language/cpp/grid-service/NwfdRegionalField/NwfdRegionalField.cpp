// NwfdInterpolation.cpp : ���� DLL Ӧ�ó���ĵ���������
#include "NwfdRegionalField.h"

/************************************************************************/
/*  DLL Ӧ�ó���ĵ�������  GetDataProcesser                            */
/************************************************************************/
DataProInterface * GetDataProcesser()
{
	DataProInterface * pProcesser = new DataProcesser();

	return pProcesser;
}
