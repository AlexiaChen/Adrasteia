#include <Winsock2.h>

#pragma comment(lib,"WS2_32")

class CInitSock{
public:
	CInitSock(BYTE minorVer = 2, BYTE majorVer = 2){
		//Init WS2_32.DLL
		WSADATA wsaData;
		WORD sockVersion = MAKEWORD(minorVer,majorVer);
		if(WSAStartup(sockVersion,&wsaData)!=0){
			exit(0);
		}
	}

	~CInitSock(){
		WSACleanup();
	}
};