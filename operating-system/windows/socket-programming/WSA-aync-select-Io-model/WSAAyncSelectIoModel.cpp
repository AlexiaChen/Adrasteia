/*WSAAyncSelect I/O model

pros: it could simply develop net program with GUI 

simple TCP server ,listen port 4567 and accept the require of client ,then , print the data from 
client
*/
#include <stdio.h>
#include "initsock.h"

typedef enum MyMessage{

	WM_SOCKET = 0x0401

}MyMessage;

LRESULT CALLBACK WindowPro(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);

//in order to use WSAAyncSelect I/O model, this proram create an invisible Window
int main(int argc, char* argv[]){
	
	wchar_t szClass[] = L"MainWClass";

	WNDCLASSEX wndclass;

	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WindowPro;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = NULL;
	wndclass.hIcon = LoadIcon(NULL,IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL,IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szClass;
	wndclass.hIconSm = NULL;

	RegisterClassEx(&wndclass);

	HWND hwnd = CreateWindowEx(
		0,
		szClass,
		L"",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		NULL,
		NULL
		);

	if(hwnd == NULL){
		MessageBox(NULL,L"window error!",L"error",MB_OK);
		return -1;
	}
	CInitSock cinit;

	SOCKET s_listen = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	sockaddr_in sin_t;
	
	sin_t.sin_family = AF_INET;
	sin_t.sin_port = htons(4567);
	sin_t.sin_addr.S_un.S_addr = INADDR_ANY;

	if(bind(s_listen,(sockaddr*)&sin_t,sizeof(sin_t)) ==  SOCKET_ERROR){

		printf("bind failed!\n");
		return -1;
	}

	//set the socket to the type of window message
	WSAAsyncSelect(s_listen,hwnd,WM_SOCKET,FD_ACCEPT | FD_CLOSE);

	listen(s_listen,5);

	MSG msg;

	while(GetMessage(&msg,NULL,0,0)){
		TranslateMessage(&msg);    
		DispatchMessage(&msg);
	}

	return msg.wParam;
	
	
}


LRESULT CALLBACK WindowPro(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam){

	switch(uMsg){
	case WM_SOCKET:
		{
		   SOCKET s = wParam;//get the socket which the event happened
		   //check if error
		   if(WSAGETSELECTERROR(lParam)){
			   closesocket(s);
			   return 0;
		   }

		   //handle the event

		   switch(WSAGETSELECTEVENT(lParam)){
		   case FD_ACCEPT:  //check a new connection in a set of listening sockets
			   {
				   SOCKET client = accept(s,NULL,NULL);
			   WSAAsyncSelect(client,hwnd,WM_SOCKET,FD_READ | FD_WRITE | FD_CLOSE);

			   
			   }
			   break;
		   case FD_WRITE:
			   {
			      
			   }
			   break;
		   case FD_READ:
			   {
				   char szText[1024] ={0};
				   if(recv(s,szText,1024,0) == -1 ) closesocket(s);
				   else printf("Recv data is:%s\n",szText);
			   }
			   
			   break;
		   case FD_CLOSE:
			   {
				   closesocket(s);
			   }
			   
			   break;
		   }
		}
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd,uMsg, wParam,lParam);
}