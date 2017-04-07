/*TCP client*/
#include <stdio.h>
#include "initsock.h"



int main(){
	
	CInitSock init_sock;
	//create the socket
	SOCKET s = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	if(s == INVALID_SOCKET){
		printf("socket() failed\n");
		return 0;
	}

	sockaddr_in sock_in;

	sock_in.sin_family = AF_INET;
	sock_in.sin_port = htons(4567);
	sock_in.sin_addr.S_un.S_addr = INADDR_ANY;

	//bind this socket to local addr
	if(bind(s,(sockaddr*)&sock_in,sizeof(sock_in)) == SOCKET_ERROR){

		printf("bind() failed\n");
		exit(1);
	}

    // receive data

	char buff[1024];

	sockaddr_in addr;

	int nLen = sizeof(addr);

    while(true){

		int nRev = recvfrom(s,buff,1024,0,(sockaddr*)&addr,&nLen);

		if(nRev > 0){

			buff[nRev] = '\0';
			printf("Received data (%s):%s",inet_ntoa(addr.sin_addr),buff);
		}

	}

	closesocket(s);

	
	return 0;
}