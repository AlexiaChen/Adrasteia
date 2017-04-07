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

	// fill remote addr information
	sockaddr_in addr;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(4567);
	addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	//bind this socket to local addr
	if(bind(s,(sockaddr*)&addr,sizeof(addr)) == SOCKET_ERROR){

		printf("bind() failed\n");
		exit(1);
	}

    // send data

	char buff[] = "TCP C/S Demo!\r\n";

	//if you call sendto first before creating socket,there is no need 
	//to bind this socket to local addr,it would bind automatically.on the contrary 
	//revcfrom would fail,it need to bind first
	sendto(s,buff,strlen(buff),0,(sockaddr*)&addr,sizeof(addr));

	closesocket(s);

	
	return 0;
}