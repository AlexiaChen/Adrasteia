#include <windows.h>
#define cryptFlag 445

int ReadDisk(int Id, int num, unsigned char *buffer);
int WriteDisk(int Id, int num, unsigned char* buffer);

// this program need to run as admin , in order to open hard disk driver correctly

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	char keys[256] = { 0 };		//buffer,缓冲区，用来保存ini配置文件里面输入的密码
	unsigned char mbr[512] = { 0 };	//用于保存从硬盘MBR里面读取出来的原始的MBR信息的
	unsigned char lock_disk[512] = { 0 };
	unsigned char len;//255 
	int i = 0;
	
	//硬盘锁的密码
	GetPrivateProfileStringA("Locker", "psw", "left", keys, 256, ".\\Config.ini");
	len = strlen(keys);
	if (len == 0 || len >= 18)
	{
		MessageBoxA(NULL, "Error!password length should be 0 ~ 18 ..", "Error", MB_OK | MB_ICONERROR);
		exit(-1);
	}
	
	byte XResult = 0;				//密码异或值
	for (i = 0; i < len; ++i)
		XResult ^= keys[i];

	if (ReadDisk(0, 1, mbr) == 0)
	{
		MessageBoxA(NULL, "Read MBR failed!", "Error", MB_OK | MB_ICONERROR);
		exit(-1);
	}

	if (mbr[cryptFlag] == 0x16)
	{
		MessageBoxA(NULL, "Error!hard disk is already locked, cannot lock repeat..", "Error", MB_OK | MB_ICONERROR);
		exit(-1);
	}
	mbr[cryptFlag] = 0x16;
	for (i = 0; i < 64; ++i)
		mbr[446 + i] ^= XResult;
	WriteDisk(2, 1, mbr);//将原始引导程序保存到第3扇区
	memcpy(lock_disk + cryptFlag, mbr + cryptFlag, 67);
	WriteDisk(0, 1, lock_disk);//写入加密的MBR
	MessageBoxA(NULL,"Congratulation! lock Hard Disk success !!!\n Do not lock repeat!\n","Success",MB_OK | MB_ICONINFORMATION);
	return 0;
}
int ReadDisk(int Id, int num, unsigned char *buffer)
{
	/*
	读取扇区
	id = ID号
	num = 读取数量
	成功返回读取字节数
	*/
	HANDLE hFile = NULL;
	int offset = 0;
	int ReadSize = 0;
	DWORD Readed = 0;
	offset = Id * 512;
	ReadSize = num * 512;
	if (buffer == NULL)
	{
		return ReadSize;
	}
	
	//打开硬盘物理驱动器
	hFile = CreateFileA("\\\\.\\\\physicaldrive0",
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		MessageBoxA(NULL, "cannot open \\\\.\\\\physicaldrive0..", "Error", MB_OK | MB_ICONERROR);
		return 0;
	}
	SetFilePointer(hFile, offset, 0, 0);
	ReadFile(hFile, buffer, ReadSize, &Readed, NULL);
	CloseHandle(hFile);
	return Readed;
}
int WriteDisk(int Id, int num, unsigned char* buffer)
{
	/*
	写扇区
	*/
	HANDLE hFile = NULL;
	int WriteSize = 0;
	int offset = 0;
	DWORD Writed = 0;
	offset = Id * 512;
	WriteSize = num * 512;
	if (buffer == NULL)
	{
		return WriteSize;
	}
	//Open it
	
	hFile = CreateFileA("\\\\.\\\\physicaldrive0", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		MessageBoxA(NULL, "cannot open \\\\.\\\\physicaldrive0..", "Error", MB_OK | MB_ICONERROR);
		return 0;
	}
	SetFilePointer(hFile, offset, 0, 0);
	WriteFile(hFile, buffer, WriteSize, &Writed, 0);
	CloseHandle(hFile);
	return WriteSize;
}
