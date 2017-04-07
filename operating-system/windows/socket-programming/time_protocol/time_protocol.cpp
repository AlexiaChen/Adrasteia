/*Time Protocol RFC-868,Local time and network time proofreading, 
change the system time!*/
#include <stdio.h>
#include "initsock.h"

static void set_time_from_tp(unsigned long ulTime){
	
	//windows file time is 64-bit value,it's time interval from 1601.01.01 12:00(noon) to now
	// the unit is 1/1000 0000 seconds (100 nano-seconds)

	FILETIME ft;
	SYSTEMTIME st;

	//first, convert the base time (1900.01.01,00:00) to windows file time

	st.wYear = 1900;
	st.wMonth = 1;
	st.wDay = 1;
	st.wHour = 0;
	st.wMinute = 0;
	st.wSecond = 0;
	st.wMilliseconds = 0;

	SystemTimeToFileTime(&st,&ft);

	//then,add the base time of time protocol to the elapsed time (ulTime)
	long long *pll = (long long *)&ft;
	
	//watch,the unit of file time is 1/1000 0000 seconds
	*pll +=  (long long)10000000*ulTime;

	//convert file time back to system time and update system time
	FileTimeToSystemTime(&ft,&st);
	SetSystemTime(&st);
}

int main(int argc, char* argv[]){
	
	if(argc != 2){

		printf("argument fault!\n   \
			   usage: cmd [time server ip] \n \
			");
		exit(1);
	}
	CInitSock init_sock;
	//create the socket
	SOCKET s = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(s == INVALID_SOCKET){
		printf("socket() failed\n");
		return 0;
	}

	// fill remote addr information to connect time server
	//for more information http://tf.nist.gov/tf-cgi/servers.cgi
	sockaddr_in addr;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(37);
	addr.sin_addr.S_un.S_addr = inet_addr(argv[1]);

	if(connect(s,(sockaddr*)&addr,sizeof(addr)) == SOCKET_ERROR){
		printf("connect fail\n");
		exit(1);
	}

	unsigned long ulTime = 0;// this value is seconds from the midnight 1900.01.01 to now

	int nRev = recv(s,(char*)&ulTime,sizeof(ulTime),0);

	if(nRev > 0){
	   ulTime = ntohl(ulTime);
	   set_time_from_tp(ulTime);
	   printf("success  synchronize time server");
	}else{


		printf("cannot synchronize with Time server!\n");
	}

    

	
	closesocket(s);

	
	return 0;
}