/*TCP server*/
#include <stdio.h>
#include "initsock.h"



int main(){
	
	CInitSock init_sock;
	//create the socket
	SOCKET slisten = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(slisten == INVALID_SOCKET){
		printf("socket() failed\n");
		return 0;
	}

	//fill up with struct
	sockaddr_in s_in;
	s_in.sin_family				= AF_INET;
	s_in.sin_port				= htons(4567);
	s_in.sin_addr.S_un.S_addr   = INADDR_ANY;

	//bind the socket to the address of local
	if(bind(slisten,(LPSOCKADDR)&s_in,sizeof(s_in)) == SOCKET_ERROR){
		printf("bind() failed\n");
		return 0;
	}

	// enter listen mode
	if(listen(slisten,2) == SOCKET_ERROR){
		printf("listen() failed\n");
		return 0;
	}

	//circularly Accept the request from client 
	sockaddr_in remote_addr;
	int addr_len = sizeof(remote_addr);

	SOCKET client;
	char text[] = "TCP server Demo";

	while(true){
		//accept new connection
		client = accept(slisten,(SOCKADDR*)&remote_addr,&addr_len);
		if(client == INVALID_SOCKET){
			printf("accept() failed\n");
			continue;
		}

		printf("Accept a connection:%s\r\n",inet_ntoa(remote_addr.sin_addr));
        // send data to client
		send(client,text,strlen(text),0);
		//close the connection of client
		closesocket(client);

	}

	closesocket(slisten);
	return 0;
}