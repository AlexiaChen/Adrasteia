/*Get local host ip*/

#include <stdio.h>
#include "initsock.h"



int main(){
	
	char host_name[512];
	CInitSock cinit_sock;
	//get host name
	gethostname(host_name,256);
	//get the infor of address by host name
	hostent *pHost = gethostbyname(host_name);

	in_addr addr;

	for(int i = 0;;i++){
		char* p = pHost ->h_addr_list[i];

		if(NULL == p) break;
		memcpy(&addr.S_un.S_addr,p,pHost ->h_length);
		char *ip = inet_ntoa(addr);

		printf("local host's ip is:%s\n",ip);
	}
	return 0;
}