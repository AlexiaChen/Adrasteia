/*select function example

listen port 4567 and accept the require of client ,then , print the data from 
client
*/
#include <stdio.h>
#include "initsock.h"



int main(int argc, char* argv[]){
	
	
	CInitSock init_sock;
	//create the socket
	SOCKET s_listen = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(s_listen == INVALID_SOCKET){
		printf("socket() failed\n");
		return 0;
	}

	sockaddr_in sin_t;
	sin_t.sin_family = AF_INET;
	sin_t.sin_port = htons(4567); // this server listen port number
	sin_t.sin_addr.S_un.S_addr = INADDR_ANY;

    //bind the socket to local host
	if(bind(s_listen,(sockaddr*)&sin_t,sizeof(sin_t)) == SOCKET_ERROR){
	  printf("bind failed!\n");
	  exit(1);
	}

	// start listen mode
	listen(s_listen,5);

	//the process of select model
	//initialize a set of sockets,add the handle of listen socket to the set
	fd_set fdSocket;
	FD_ZERO(&fdSocket);
	FD_SET(s_listen,&fdSocket);

    while(true){
		//copy fdSocket to fdRead and pass it to select function
		fd_set fdRead = fdSocket;
		//when the Event happens,select function will remove pending I/O handle of sockets and return

		int nRet = select(0,&fdRead,NULL,NULL,NULL);

		if(nRet > 0){
           // compare fdRead processed by select and fdSocket 

			for(int i = 0; i < (int)fdSocket.fd_count;i++){
				//check the param 1 if exists in the set of param 2
				if(FD_ISSET(fdSocket.fd_array[i],&fdRead)){ // if return true , param 1 exists

					if(fdSocket.fd_array[i] == s_listen){

						if(fdSocket.fd_count < FD_SETSIZE){
							sockaddr_in addrRemote;
							int nAddrLen = sizeof(addrRemote);
							
							SOCKET sNew = accept(s_listen,(sockaddr*)&addrRemote,&nAddrLen);
							FD_SET(sNew,&fdSocket);

							printf("Received Connection (%s)\n",inet_ntoa(addrRemote.sin_addr));

						}else{

							printf("Too much connections!\n");
							continue;
						}
					}else{
						char szText[256];
						int nRev = recv(fdSocket.fd_array[i],szText,strlen(szText),0);

						if(nRev > 0){                  //readable
							szText[nRev] = '\0';
							printf("Received data: %s\n",szText);
						}else{                    //close connection and reboot, interrupt
							closesocket(fdSocket.fd_array[i]);
							FD_CLR(fdSocket.fd_array[i],&fdSocket);
						}
					}
				}
			}

		}else{

			printf("failed select!\n");
			break;
		}

	}

    

	
	

	
	return 0;
}