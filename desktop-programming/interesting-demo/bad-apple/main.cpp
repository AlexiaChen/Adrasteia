#include<windows.h>
#include<stdio.h>
#include<string.h>
#include<time.h>
#include<iostream>
using namespace std;
#pragma comment(lib, "winmm.lib")


//返回标准的输入的句柄
HANDLE hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);

typedef int (WINAPI* SETCONSOLEFONT)(HANDLE, DWORD);

SETCONSOLEFONT SetConsoleFont;

void goto_xy(int x, int y);

int main(int argc, char* argv[])
{
    int k = 0, var_8 = 0, var_C = 0;
	
	double time_t ;
	
	SYSTEMTIME SystemTime;
   
    GetLocalTime(&SystemTime);
    
	time_t = time(NULL) + SystemTime.wMilliseconds/1000.0;
	
	HMODULE hModule = GetModuleHandle(L"kernel32");
    SetConsoleFont   = (SETCONSOLEFONT)GetProcAddress(hModule, "SetConsoleFont");

	SetConsoleFont(hConsoleOutput, 1);

	MCI_OPEN_PARMS mciOpen; 

	mciOpen.lpstrDeviceType = L"mpegvideo";
    mciOpen.lpstrElementName = L"apple.mp3";
	
    mciSendCommand(0,MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT, (DWORD)&mciOpen);


    UINT mciID;

	mciID = mciOpen.wDeviceID;

	MCI_PLAY_PARMS mciPlay; 

	mciSendCommand(mciID, MCI_PLAY, 0, (DWORD)&mciPlay);

    system("mode con cols=101 lines=44");
//////////////////////////////////////////////////////////////////////////////////////////////
	FILE *fp;
    char a;
	fp = fopen("apple.txt", "r");

	if(fp == NULL){
		fprintf(stderr,"connot open the file.\n");
		exit(1);
	}
    
    char Format[13100] = {0};
	char Command[10] = {"color f0"};

	system(Command);
    
	while(!feof(fp))
	{
        a = fgetc(fp);
        if(Format[k] != a)
		{
			Format[k] = a;
		}

		++k;

		if(Format[k-1] == 'n' && Format[k-2] == 'e' && Format[k-3] == 'h' && Format[k-4] == 'C')
		{
			k = 0;
            
			a = fgetc(fp);
            
			goto_xy(0,0);

			printf(Format);

			while(time(NULL) + SystemTime.wMilliseconds/1000.0 - time_t < 0.032)
			{
				Sleep(1);
				GetLocalTime(&SystemTime);
			}
            
			GetLocalTime(&SystemTime);

			time_t = time(NULL) + SystemTime.wMilliseconds/1000.0;
		}
	}

	
    
	 return 0;
}

void goto_xy(int x, int y)
{
	COORD coord = {x, y};
    //设置光标位置
	SetConsoleCursorPosition(hConsoleOutput, coord);

	
}