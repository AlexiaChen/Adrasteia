// *****************************************************************
//
//  E-mail Client 
//  Version 1.0
//  Copyright (C) 2009 MathxH Chen
//  Contact:         brainfvck@foxmail.com  
//  GitHub:
//  Author:            MathxH Chen
//  
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
// *****************************************************************



#include<windows.h>
#include<WinSock.h>

#include<stdio.h>
#include<string.h>
#include<malloc.h>

#include"resource.h"
#pragma  comment(lib, "ws2_32.lib")

//Base64 encode transform table
char* const   pBASE64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void OpenSocket(struct sockaddr* address, HWND Hwnd, SOCKET* socket_t);

void ASCIItoBASE64(char* buffer);//Base64 encode function

BOOL CALLBACK DlgMain(HWND,UINT,WPARAM,LPARAM);

int WINAPI WinMain(
  HINSTANCE hInstance,  // handle to current instance
  HINSTANCE hPrevInstance,  // handle to previous instance
  LPSTR lpCmdLine,      // pointer to command line
  int nCmdShow          // show state of window
)
{
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgMain);
	return 0;
}


BOOL CALLBACK DlgMain(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{

	char MailAccount[100];
	char PassWord[100]; 
	WORD VersionRequested;
	char host_by_name[50] = "smtp.";
	char* pstr = NULL;
	struct hostent* hptr = NULL;
	char IPBuffer[50];
	DWORD  socket_ret;
	char read_buffer[1500];
	char Buffer[1500];
	char HELO_Buffer[50]="HELO"; 
	char MailFrom[100]="MAIL FROM:";
	char ToAddress[50];
	char RCPT_TO_ADDR[100]="RCPT TO:";
	char Subject[50];
	char MainText[500];
	char Body[3000];
	
	SOCKET socket_t = 0;
	
	int error_t = 0;
	WSADATA m_wsadata;
	struct sockaddr_in their_address = {0};
	
	switch(message)
    {
	case WM_INITDIALOG:
		  //cannot initialize win sock here
          
		  

		 

          
          
          
		    return TRUE;

    case WM_CLOSE:
        
            EndDialog(hwnd,0);
            
			return TRUE;
	
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		  
                  
		   case IDOK://Send

			     GetDlgItemText(hwnd,IDC_ACCOUNT,MailAccount,100);
                 GetDlgItemText(hwnd,IDC_PASSWORD,PassWord,100);
				 
				 VersionRequested = MAKEWORD(2,2);
		         WSAStartup(VersionRequested, &m_wsadata);
		         memset(&their_address, 0, sizeof(their_address));
                 their_address.sin_family = AF_INET;
		         their_address.sin_port = htons(25);//port 25
                 
				 pstr = strchr(MailAccount,'@');
				 
				 if(pstr == NULL)
				 {
                    MessageBox(hwnd, TEXT("Invalid Account！"), TEXT("Error！"), MB_OK);
					return FALSE;
				 }
				 
				 pstr++;
				 
				 strcat(host_by_name, pstr);
				
                 hptr  = gethostbyname(host_by_name);
				 
				
				 
				 memcpy(&their_address.sin_addr.S_un.S_addr, hptr->h_addr_list[0], hptr->h_length); 
                
                 memset(IPBuffer, 0, 50);
                
				 sprintf(IPBuffer, "%d.%d.%d.%d",
                         their_address.sin_addr.S_un.S_un_b.s_b1, 
                         their_address.sin_addr.S_un.S_un_b.s_b2, 
                         their_address.sin_addr.S_un.S_un_b.s_b3, 
                         their_address.sin_addr.S_un.S_un_b.s_b4);
				 
				 their_address.sin_addr.S_un.S_addr = inet_addr(IPBuffer);
				 
				 SetDlgItemText(hwnd,IDC_MESSAGE, IPBuffer);
                  
				 //Connect mail SERVER 
                 OpenSocket((struct sockaddr*)&their_address,hwnd,&socket_t);
				  
				  
				 memset(read_buffer, 0, sizeof(read_buffer));
                 recv(socket_t, read_buffer, 1500, 0) ;
				 
                 //if connect fail  Sleep 2000ms  and reconnect
				 while(recv(socket_ret, read_buffer, 1500, 0) == 0)
				  {
                     SetDlgItemText(hwnd, IDC_MESSAGE, TEXT("ReConnect..."));
					 Sleep(2000);
					  OpenSocket((struct sockaddr *)&their_address,hwnd,&socket_t);
					 memset(read_buffer, 0, sizeof(read_buffer));

				  }
                  
				  SetDlgItemText(hwnd, IDC_MESSAGE, TEXT("Connect compelet!"));
                  
                  //log in smtp server for Authentication 
				  memset(Buffer, 0, 1500);
				  strcat(HELO_Buffer, " ");
				  strcat(HELO_Buffer, host_by_name);
				  strcat(HELO_Buffer, "\r\n");
				  sprintf(Buffer, HELO_Buffer);
				  send(socket_t, (const char*)Buffer, strlen(Buffer), 0);
                 
                  memset(read_buffer, 0, 1500);
				  recv(socket_t, read_buffer, 1500, 0);
				
                  memset(Buffer, 0, 1500);
				  sprintf(Buffer, "AUTH LOGIN\r\n");
				  send(socket_t, Buffer, strlen(Buffer), 0);
				  memset(read_buffer, 0, 1500);
				  recv(socket_t, read_buffer, 1500, 0);
		
                  //send Account to server for Authentication
				  memset(Buffer, 0, 1500);
				
				  ASCIItoBASE64(MailAccount);
				  sprintf(Buffer,"%s\r\n", MailAccount);
                  send(socket_t, Buffer, strlen(Buffer), 0);
				  memset(read_buffer, 0, 1500);
				  recv(socket_t, read_buffer, 1500, 0);
			
                  //send Password to server for Authentication
				  memset(Buffer, 0, 1500);
				  ASCIItoBASE64(PassWord);
				  sprintf(Buffer, "%s\r\n", PassWord);
				  send(socket_t, Buffer, strlen(Buffer), 0);
				  memset(read_buffer, 0, 1500);
				  recv(socket_t, read_buffer, 1500, 0);
             
                  //Mail sent from 
				  memset(Buffer, 0, 1500);
				  strcat(MailFrom, " ");
				  strcat(MailFrom, "<");
				  GetDlgItemText(hwnd,IDC_ACCOUNT,MailAccount,100);
				  strcat(MailFrom, MailAccount);
				  strcat(MailFrom, ">");
				  sprintf(Buffer, "%s\r\n", MailFrom);
				  send(socket_t, Buffer, strlen(Buffer), 0);
				  memset(read_buffer, 0, 1500);
				  recv(socket_t, read_buffer, 1500, 0);
			
                  
                  SetDlgItemText(hwnd, IDC_MESSAGE, TEXT("successfully log in！"));
                  
			      GetDlgItemText(hwnd,IDC_TOADDRESS,ToAddress, 50);
			      GetDlgItemText(hwnd,IDC_SUBJECT, Subject, 50);
				  GetDlgItemText(hwnd,IDC_TEXT, MainText, 500);
				  
				  //To Address
				  strcat(RCPT_TO_ADDR, " ");
				  strcat(RCPT_TO_ADDR, "<");
				  strcat(RCPT_TO_ADDR, ToAddress);
				  strcat(RCPT_TO_ADDR, ">");
				  sprintf(Buffer, "%s\r\n", RCPT_TO_ADDR);
                  send(socket_t, Buffer, strlen(Buffer), 0);
				  memset(read_buffer, 0, 1500);
				  recv(socket_t, read_buffer, 1500, 0);
				
                  sprintf(Buffer, "DATA\r\n");
				  send(socket_t, Buffer,strlen(Buffer), 0);
				  memset(read_buffer, 0, 1500);
				  recv(socket_t, read_buffer, 1500, 0);
				 
                  memset(Body, 0, sizeof(Body));
                  strcat(Body, "Subject: ");
				  strcat(Body, Subject);
                  strcat(Body, "\r\n");
				  strcat(Body, "From: ");
				  strcat(Body, "\" \"");
				  strcat(Body, "<");
				  strcat(Body, MailAccount);
				  strcat(Body, ">\r\n");
                  strcat(Body, "To: ");
                  strcat(Body, "\" \"");
				  strcat(Body, "<");
				  strcat(Body, ToAddress);
				  strcat(Body, ">\r\n");
				  strcat(Body, "\r\n");
				  strcat(Body, MainText);
				  
                  // the Mark of the end of the mail conetxt
				  sprintf(Buffer, "%s\r\n.\r\n", Body);
				  send(socket_t, Buffer, strlen(Buffer), 0);
				  memset(read_buffer, 0, 1500);
				  recv(socket_t, read_buffer, 1500, 0);
			      
				  //QUIT
				  sprintf(Buffer, "QUIT\r\n");
                  send(socket_t, Buffer, strlen(Buffer), 0);
				  memset(read_buffer, 0, 1500);
				  recv(socket_t, read_buffer,1500,0);
				
                  //Clear
				  closesocket(socket_t);
				  WSACleanup();

			      MessageBox(hwnd, TEXT("Sent Successfully"),TEXT("Hint"), MB_OK);
				  
				  return TRUE;
		}//switch(LOWORD(wParam))

		
    }//switch(message)
    return FALSE;
}

void OpenSocket(struct sockaddr* address, HWND Hwnd,SOCKET* socket_t)
{
	
	int connect_ret = 0;
	int connect_count = 0;
	BOOL bConnected = FALSE;
	
ReOpen:	
	*socket_t = socket(AF_INET, SOCK_STREAM, 0);
	                                             
    if(*socket_t == INVALID_SOCKET)
	{
		SetDlgItemText(Hwnd, IDC_MESSAGE, TEXT("Open socket_ret(TCP) error! Waiting 3000ms and reopen the socket..."));
		Sleep(3000);
		
        goto ReOpen;
	}
    
	
	
ReConnect:	
    connect_ret = connect(*socket_t, address, sizeof(struct sockaddr));
          
	if(connect_ret == SOCKET_ERROR)
    {
			 
	   MessageBox(Hwnd, TEXT("Connect Error,Waiting 3000ms and ReConnect the Socket..."),TEXT("Hint"), MB_OK);
           Sleep(3000);
	   goto ReConnect;
			  
    }
		   
}

void ASCIItoBASE64(char* buffer)
{
    int length = strlen(buffer);
	int temp_length = 0;
	int i = 0, j = 0;
    char* temp_buffer = (char*)malloc(length*2);
   
	memset(temp_buffer, 0, length*2);
    
	for(; i < length-2; i += 3, j += 4)
	{
		 //将3个字符编码成4个字符
         int n = (buffer[i] << 16) + (buffer[i+1] << 8) + buffer[i+2];
		 int m = (n << 6) & 0xff000000;
		 n &= 0x3ffff;
		 m |= (n << 4) & 0x00ff0000;
		 n &= 0xfff;
		 m |= (n << 2) & 0x0000ff00;
		 n &= 0x3f;
         m |= n;

         temp_buffer[j+3] = pBASE64[m & 0xff];//取第1字节
	     temp_buffer[j+2] = pBASE64[m >> 8 & 0xff];//取第2字节
		 temp_buffer[j+1] = pBASE64[m >> 16 & 0xff];//取第3字节
		 temp_buffer[j]   = pBASE64[m >> 24 & 0xff];//取第4字节

         
         
		 
	}
   
    //如果要编码的字节数不能被3整除
	//先使用0字节值在末尾补足，使其能够被3整除，然后再进行base64的编码。
	//在编码后的base64文本后加上一个或两个'='号，代表补足的字节数。也就是说，
	//当最后剩余一个八位字节(一个byte)时，最后一个6位的base64字节块有四位是0值，
	//最后附加上两个等号；如果最后剩余两个八位字节(2个byte)时，最后一个6位的base
	//字节块有两位是0值，最后附加一个等号
	if(i != length)
	{
		
		int n = ((length - i) == 1) ? (buffer[i] << 16) /*剩1字节时*/: (buffer[i] << 16) + (buffer[i+1] << 8)/*剩2字节时*/;
        
		//重复以前的方式,因为扩展了被
        int m = (n << 6) & 0xff000000;
        n &= 0x3ffff;
        m |= (n << 4) & 0x00ff0000;
        n &= 0xfff;
        m |= (n << 2) & 0x0000ff00;
        n &= 0x3f;
        m |= n;


        if(length - i == 1)
		{
            temp_buffer[j+1] = pBASE64[ m >> 16 & 0xff];//同上
			temp_buffer[j]   = pBASE64[m >> 24];//同上
			strcat(temp_buffer, "==");
		}
		else
		{
			temp_buffer[j+2] = pBASE64[m >> 8 & 0xff];
			temp_buffer[j+1] = pBASE64[m >> 16 & 0xff];
			temp_buffer[j]   = pBASE64[m >> 24 &0xff];
			strcat(temp_buffer, "=");
		}


	}
    strcpy(buffer, temp_buffer);

	free(temp_buffer);
	

}
