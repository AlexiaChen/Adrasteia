#include "stdio.h"
#include "DataProInterface.h"
#include "dlfcn.h"
#include "malloc.h"
#include "string.h"

typedef DataProInterface* (*lpGetDataProcesser)(void);

int main()
{
    void * handle = dlopen("./libNwfdRegionalField.so", RTLD_LAZY);
    if(!handle)
    {
        printf("Load Error = %s\n", dlerror());
        return 1;
    }
	printf("** 1 **  dlopen success\n");
	
    lpGetDataProcesser GetDataProcesser = (lpGetDataProcesser)dlsym(handle, "GetDataProcesser");
    if(GetDataProcesser == NULL)
    {
        printf("GetDataProcesser Error = %s\n", dlerror());
		dlclose(handle);
		
		return 1;
    }
	printf("** 2 **  dlsym success\n");
	printf("********* start Test ***************\n");
	// test
	DataProInterface * pDataProcesser = GetDataProcesser();
	if( pDataProcesser == NULL)
	{
		printf("DataProInterface GetDataProcesser error\n");
		dlclose(handle);
		
		return 1;
	}
	printf("** 3 **  GetDataProcesser success\n");
	
	char m_szParams[256];
	float fInVal[24];
	int  nInNi = 4;
	int  nInNj = 3;
	int  nInOutCnt = 2;
	int nOff = 0;
	//  ************ 1 **************
	fInVal[nOff++] = 1;
	fInVal[nOff++] = 1;
	fInVal[nOff++] = 1;
	fInVal[nOff++] = 1;

	fInVal[nOff++] = 5;
	fInVal[nOff++] = 6;
	fInVal[nOff++] = 7;
	fInVal[nOff++] = 8;

	fInVal[nOff++] = 4;
	fInVal[nOff++] = 3;
	fInVal[nOff++] = 2;
	fInVal[nOff++] = 1;
	//  ************ 2 **************
	fInVal[nOff++] = 0;
	fInVal[nOff++] = 1;
	fInVal[nOff++] = 2;
	fInVal[nOff++] = 3;

	fInVal[nOff++] = 4;
	fInVal[nOff++] = 5;
	fInVal[nOff++] = 6;
	fInVal[nOff++] = 7;

	fInVal[nOff++] = 8;
	fInVal[nOff++] = 9;
	fInVal[nOff++] = 10;
	fInVal[nOff++] = 11;

	// 2倍插值
	strcpy(m_szParams, "2_2_2_2");
	int nRet = pDataProcesser->SetParams(m_szParams);
	if(nRet == 0)
	{
		printf("** 4 **  set params success (params=%s)\n",m_szParams);
	}
	else
	{
		printf("** 4 **  set params failed  ret =%d\n",nRet);
	}
	
	printf("***************** IN **********************\n");
	printf("InNi = %d    InNj = %d \n", nInNi, nInNj);
	printf("\n");
	float *fInValTmp = NULL;
	for (int p = 0; p < nInOutCnt; p++)
	{
		printf("<%d>:\n", p+1);
		fInValTmp = fInVal + (nInNi * nInNj)*p;

		for (int j = 0; j < nInNj; j++)
		{
			for (int i = 0; i < nInNi; i++)
			{

				printf("%-14.2f", fInValTmp[j*nInNi + i]);
			}

			printf("\n");
		}
	}
	printf("\n");
	printf("***************** OUT **********************\n");
	int nOutNi;
	int nOutNj;
	float *fOutVal = NULL;

	nRet = pDataProcesser->DataProcess(fInVal, nInNi, nInNj, fOutVal, nOutNi, nOutNj, nInOutCnt);
	if (nRet != 0)
	{
		printf("DataProcess failed, ErroCode=%d\n", nRet);
	}

	printf("OutNi = %d    OutNj = %d \n", nOutNi, nOutNj);
	printf("\n");
	float *fOutValTmep = NULL;
	for (int p = 0; p < nInOutCnt; p++)
	{
		printf("<%d>:\n", p + 1);
		fOutValTmep = fOutVal + (nOutNi*nOutNj)*p;

		for (int j = 0; j < nOutNj; j++)
		{
			for (int i = 0; i < nOutNi; i++)
			{

				printf("%-7.2f", fOutValTmep[j*nOutNi + i]);
			}

			printf("\n");
		}
	}
	
	printf("********* Test End ***************\n");
	// end
    dlclose(handle);

    return 0;
}

