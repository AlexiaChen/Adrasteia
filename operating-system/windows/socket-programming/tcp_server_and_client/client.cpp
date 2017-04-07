/*TCP client*/
#include <stdio.h>
#include "initsock.h"



int main(){
	
	CInitSock init_sock;
	//create the socket
	SOCKET s = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(s == INVALID_SOCKET){
		printf("socket() failed\n");
		return 0;
	}

	//fill up with struct
	sockaddr_in server_addr;
	server_addr.sin_family				= AF_INET;
	server_addr.sin_port				= htons(4567);
	server_addr.sin_addr.S_un.S_addr   = inet_addr("10.9.1.104"); // the parameter need to be your local host ip,so you need to get ip dynamically

	//bind the socket to the address of local
	if(connect(s,(LPSOCKADDR)&server_addr,sizeof(server_addr)) == SOCKET_ERROR){
		printf("connect() failed\n");
		return 0;
	}

	//receive data
	char buffer[256];
	int receive_n = recv(s,buffer,256,0);

	if(receive_n > 0){

		buffer[receive_n] = '\0';
		printf("received data is %s\n",buffer);
	}

	//close the socket
	closesocket(s);

	return 0;
}