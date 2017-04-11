/*The program enable hide a file into bmp image file, NOTE:
* the size of the file you want to hide must  less than the size(width*height) of the image,
* It simply write the bytes of you want to hide into Alpha channel of the image
*/

//其前54个字节为文件头信息，从第55个字节开始到结束的所有数据
#include <cstdio>
#include <cstdlib>
#include <winsock.h>
#include <windows.h>


char * GetFileContent(char * filename, DWORD * filesize)
{
	HANDLE hfile = CreateFileA(filename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
	if (hfile == INVALID_HANDLE_VALUE)
	{
		printf("Can't open %s\n", filename);
		return NULL;
	}
	
	DWORD dwRead;
	DWORD dwSize = GetFileSize(hfile, &dwRead);
	*filesize = dwSize;
	
	char * cBuf = new char[dwSize];
	RtlZeroMemory(cBuf, sizeof(cBuf));
	ReadFile(hfile, cBuf, dwSize, &dwRead, 0);
	if (dwRead != dwSize)
	{
		printf("Read %s failed\n", filename);
		return NULL;
	}
	
	CloseHandle(hfile);
	return cBuf;
}

bool SaveFile(char * buf, int len, char * filename)
{
	HANDLE hfile = CreateFileA(filename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_ALWAYS, NULL, NULL);
	if (hfile == INVALID_HANDLE_VALUE)
	{
		printf("Can't open %s\n", filename);
		return false;
	}
	SetFilePointer(hfile, 0, 0, FILE_BEGIN);
	DWORD dwWritten;
	WriteFile(hfile, buf, len, &dwWritten, 0);
	CloseHandle(hfile);
	return	 true;
}

bool Hide(char * bmpFileName, char * secretFileName)
{
	DWORD dwBMPSize, dwSecretSize;
	char * lpBMP = GetFileContent(bmpFileName, &dwBMPSize);
	char * lpSecret = GetFileContent(secretFileName, &dwSecretSize);
	DWORD * lpFirstPoint = (DWORD *)(lpBMP + 10);
	printf("First point offset : %d\n", *lpFirstPoint);
	char *  lpCurrentBMP = lpBMP + *lpFirstPoint + 3;// Alpha byte of first pixel (R,G,B,A   R G B A)
	char * lpCurrentSecret = lpSecret;
	//第一个像素点保存Secret文件大小
	*((DWORD*)lpCurrentBMP) = dwSecretSize;
	lpCurrentBMP += 6;
	for (; lpCurrentBMP < (lpBMP + dwBMPSize) && lpCurrentSecret < (lpSecret + dwSecretSize); lpCurrentBMP += 6)
	{
		*lpCurrentBMP = *lpCurrentSecret;
		*(lpCurrentBMP + 1) = *(lpCurrentSecret + 1);
		*(lpCurrentBMP + 2) = *(lpCurrentSecret + 2);
		lpCurrentSecret += 3;
	}

	SaveFile(lpBMP, dwBMPSize, bmpFileName);
	delete[] lpBMP;
	delete[] lpSecret;
	return true;
}

bool Recovery(char * bmpFileName, char * secretFileName)
{
	DWORD dwBMPSize;
	
	char * lpBMP = GetFileContent(bmpFileName, &dwBMPSize);
	
	DWORD * lpFirstPoint = (DWORD *)(lpBMP + 10);
	
	printf("First point offset : %d\n", *lpFirstPoint);
	
	DWORD dwSecretSize = *(DWORD *)(lpBMP + *lpFirstPoint + 3);
	
	printf("Secret file size : %d\n", dwSecretSize);
	
	char * SecretBuf = new char[dwSecretSize];
	char *  lpCurrentBMP = lpBMP + *lpFirstPoint + 3 + 6;
	
	for (int i = 0; lpCurrentBMP < (lpBMP + dwBMPSize) && i < dwSecretSize; lpCurrentBMP += 6)
	{
		SecretBuf[i] = *lpCurrentBMP;
		SecretBuf[i + 1] = *(lpCurrentBMP + 1);
		SecretBuf[i + 2] = *(lpCurrentBMP + 2);
		i += 3;
	}
	
	SaveFile(SecretBuf, dwSecretSize, secretFileName);
	
	delete[] SecretBuf;
	delete[] lpBMP;
	
	return true;
}


void help(char* argv[])
{
	printf("Usage : %s Encrypt [secret_file] [BMP_file] \n", argv[0]);
	printf("Usage : %s Decrypt [BMP_file] [secret_file] \n", argv[0]);
}


int main(int argc, char* argv[])
{
	if (argc < 3 || strcmp(argv[1], "--help") == 0)
	{
		help(argv);
		return 0;
	}

	if (strcmp(argv[1], "Encrypt") == 0)
		Hide(argv[3], argv[2]);
	else if (strcmp(argv[1], "Decrypt") == 0)
		Recovery(argv[2], argv[3]);
	else
		printf("Invalid parameter\n");

	printf("Done\n");
	
	return 0;
}

