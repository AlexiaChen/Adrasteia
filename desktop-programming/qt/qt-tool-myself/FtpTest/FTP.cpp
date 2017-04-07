#include "FTP.h"

static int SplitString( std::string strSrc, std::list<std::string> &strArray , std::string strFlag)
{
	int pos = 1; 

	while((pos = (int)strSrc.find_first_of(strFlag.c_str())) > 0) 
	{
		strArray.insert(strArray.end(), strSrc.substr(0 , pos));
		strSrc = strSrc.substr(pos + 1, strSrc.length() - pos - 1); 
	}

	strArray.insert(strArray.end(), strSrc.substr(0, strSrc.length()));

	return 0; 
}

CFTP::CFTP(void) : m_bLogin(false)
{
	m_cmdSocket = socket(AF_INET, SOCK_STREAM, 0);

    
	
}

CFTP::~CFTP(void)
{
	std::string strCmdLine = parseCommand(FTP_COMMAND_QUIT, "");

	Send(m_cmdSocket, strCmdLine.c_str());

#ifdef WIN32
	closesocket(m_cmdSocket);
#else
	close(m_cmdSocket);
#endif

	m_bLogin = false;
}

bool CFTP::IsLogin()
{
	return m_bLogin;
}

FTP_API CFTP::login2Server(const std::string &serverIP)
{
	std::string strPort;
	int pos = serverIP.find_first_of(":");

	if (pos > 0)
	{
		strPort = serverIP.substr(pos + 1, serverIP.length() - pos);
	}
	else
	{
		pos = serverIP.length();
		strPort = FTP_DEFAULT_PORT;
	}

	m_strServerIP = serverIP.substr(0, pos);
	m_nServerPort = atol(strPort.c_str());

	trace("IP: %s port: %d\n", m_strServerIP.c_str(), m_nServerPort);

	if (Connect(m_cmdSocket, m_strServerIP, m_nServerPort) < 0)
	{
		
		return -1;
	}
	
	m_strResponse = serverResponse(m_cmdSocket);
	printf("@@@@Response: %s", m_strResponse.c_str());

	return	parseResponse(m_strResponse);
}

FTP_API CFTP::inputUserName(const std::string &userName)
{
	std::string strCommandLine = parseCommand(FTP_COMMAND_USERNAME, userName);

	m_strUserName = userName;

	if (Send(m_cmdSocket, strCommandLine) < 0)
	{
		return -1;
	}

	m_strResponse = serverResponse(m_cmdSocket);
	printf("Response: %s\n", m_strResponse.c_str());

	return parseResponse(m_strResponse);
}

FTP_API CFTP::inputPassWord(const std::string &password)
{
	std::string strCmdLine = parseCommand(FTP_COMMAND_PASSWORD, password);

	m_strPassWord = password;
	if (Send(m_cmdSocket, strCmdLine) < 0)
	{
		return -1;
	}
	else
	{
		m_bLogin = true;

		m_strResponse = serverResponse(m_cmdSocket);
		printf("Response: %s\n", m_strResponse.c_str());

		return parseResponse(m_strResponse);
	}
}

FTP_API CFTP::quitServer(void)
{
	std::string strCmdLine = parseCommand(FTP_COMMAND_QUIT, "");
	if (Send(m_cmdSocket, strCmdLine) < 0)
	{
		return -1;
	}
	else
	{
		m_strResponse = serverResponse(m_cmdSocket);
		printf("Response: %s\n", m_strResponse.c_str());

		return parseResponse(m_strResponse);
	}

}

const std::string CFTP::PWD()
{
	std::string strCmdLine = parseCommand(FTP_COMMAND_CURRENT_PATH, "");

	if (Send(m_cmdSocket, strCmdLine.c_str()) < 0)
	{
		return "";
	}
	else
	{
		return serverResponse(m_cmdSocket);
	}
}


FTP_API CFTP::setTransferMode(type mode)
{
	std::string strCmdLine;

	switch (mode)
	{
	case binary:
		strCmdLine = parseCommand(FTP_COMMAND_TYPE_MODE, "I");
		break;
	case ascii:
		strCmdLine = parseCommand(FTP_COMMAND_TYPE_MODE, "A");
		break;
	default:
		break;
	}

	if (Send(m_cmdSocket, strCmdLine.c_str()) < 0)
	{
		//assert(false);
	}
	else
	{	
		m_strResponse  = serverResponse(m_cmdSocket);
		printf("@@@@Response: %s", m_strResponse.c_str());

		return parseResponse(m_strResponse);
	}

	return 0;
}


const std::string CFTP::Pasv()
{
	std::string strCmdLine = parseCommand(FTP_COMMAND_PSAV_MODE, "");

	if (Send(m_cmdSocket, strCmdLine.c_str()) < 0)
	{
		return "";
	}
	else
	{
		m_strResponse = serverResponse(m_cmdSocket);

		return m_strResponse;
	}
}


const std::string CFTP::Dir(const std::string &path)
{
	int dataSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (createDataLink(dataSocket) < 0)
	{
		return "";
	}
	// 数据连接成功
	std::string strCmdLine = parseCommand(FTP_COMMAND_DIR, path);

	if (Send(m_cmdSocket, strCmdLine) < 0)
	{
		trace("@@@@Response: %s\n", serverResponse(m_cmdSocket).c_str());
		
#ifdef WIN32
		closesocket(dataSocket);
#else
		close(dataSocket);
#endif
		return "";
	}
	else
	{
		trace("@@@@Response: %s\n", serverResponse(m_cmdSocket).c_str());
		m_strResponse = serverResponse(dataSocket);

		trace("@@@@Response: \n%s\n", m_strResponse.c_str());
#ifdef WIN32
		closesocket(dataSocket);
#else
		close(dataSocket);
#endif

		return m_strResponse;
	}
	
}


FTP_API CFTP::CD(const std::string &path)
{
	assert(m_cmdSocket != INVALID_SOCKET);

	std::string strCmdLine = parseCommand(FTP_COMMAND_CHANGE_DIRECTORY, path);

	if (Send(m_cmdSocket, strCmdLine) < 0)
	{
		return -1;
	}
		
	m_strResponse = serverResponse(m_cmdSocket);
	
	trace("@@@@Response: %s\n", m_strResponse.c_str());
	return parseResponse(m_strResponse);
}

FTP_API CFTP::DeleteFile(const std::string &strRemoteFile)
{
	assert(m_cmdSocket != INVALID_SOCKET);

	std::string strCmdLine = parseCommand(FTP_COMMAND_DELETE_FILE, strRemoteFile);

	if (Send(m_cmdSocket, strCmdLine) < 0)
	{
		return -1;
	}

	m_strResponse = serverResponse(m_cmdSocket);
	printf("@@@@Response: %s\n", m_strResponse.c_str());
	return parseResponse(m_strResponse);
}

FTP_API CFTP::DeleteDirectory(const std::string &strRemoteDir)
{
	assert(m_cmdSocket != INVALID_SOCKET);

	std::string strCmdLine = parseCommand(FTP_COMMAND_DELETE_DIRECTORY, strRemoteDir);

	if (Send(m_cmdSocket, strCmdLine) < 0)
	{
		return -1;
	}
	
	m_strResponse = serverResponse(m_cmdSocket);

	trace("@@@@Response: %s\n", m_strResponse.c_str());
	return parseResponse(m_strResponse);
}

FTP_API CFTP::CreateDirectory(const std::string &strRemoteDir)
{
	assert(m_cmdSocket != INVALID_SOCKET);

	std::string strCmdLine = parseCommand(FTP_COMMAND_CREATE_DIRECTORY, strRemoteDir);

	if (Send(m_cmdSocket, strCmdLine) < 0)
	{
		return -1;
	}
	
	m_strResponse = serverResponse(m_cmdSocket);

	trace("@@@@Response: %s\n", m_strResponse.c_str());
	return parseResponse(m_strResponse);
}

FTP_API CFTP::Rename(const std::string &strRemoteFile, const std::string &strNewFile)
{
	assert(m_cmdSocket != INVALID_SOCKET);

	std::string strCmdLine = parseCommand(FTP_COMMAND_RENAME_BEGIN, strRemoteFile);
	Send(m_cmdSocket, strCmdLine);
	trace("@@@@Response: %s\n", serverResponse(m_cmdSocket).c_str());

	Send(m_cmdSocket, parseCommand(FTP_COMMAND_RENAME_END, strNewFile));

	m_strResponse = serverResponse(m_cmdSocket);
	trace("@@@@Response: %s\n", m_strResponse.c_str());
	return parseResponse(m_strResponse);
}

long CFTP::getFileLength(const std::string &strRemoteFile)
{
	assert(m_cmdSocket != INVALID_SOCKET);

	std::string strCmdLine = parseCommand(FTP_COMMAND_FILE_SIZE, strRemoteFile);

	if (Send(m_cmdSocket, strCmdLine) < 0)
	{
		return -1;
	}

	m_strResponse = serverResponse(m_cmdSocket);

	trace("@@@@Response: %s\n", m_strResponse.c_str());

	std::string strData = m_strResponse.substr(0, 3);
	unsigned long val = atol(strData.c_str());

	if (val == 213)
	{
		strData = m_strResponse.substr(4);
		trace("strData: %s\n", strData.c_str());
		val = atol(strData.c_str());

		return val;
	}

	return -1;
}


void CFTP::Close(int sock)
{
#ifdef WIN32
	closesocket(sock);
#else
	shutdown(sock, SHUT_RDWR);
	close(sock);
#endif
	sock = INVALID_SOCKET;
}

FTP_API CFTP::Get(const std::string &strRemoteFile, const std::string &strLocalFile)
{
	return downLoad(strRemoteFile, strLocalFile);
}

/************************************************************************/
/* 上传数据                                                             */
/************************************************************************/
FTP_API CFTP::Put(const std::string &strRemoteFile, const std::string &strLocalFile)
{
	std::string strCmdLine;
	const unsigned long dataLen = FTP_DEFAULT_BUFFER;
	char strBuf[dataLen] = {0};
	unsigned long nLen = 0;

	// 上传后文件的临时文件名
	std::string strRemoteFileTmp = strRemoteFile + ".tmp";

	// 打开本地文件
	FILE *pFile = fopen(strLocalFile.c_str(), "rb");  // 以只读方式打开  且文件必须存在
	if (pFile == NULL)
	{
		return -1;
	}
	// 创建传输连接
	int data_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (data_fd == -1)
	{
		return -2;
	}
	if (createDataLink(data_fd) < 0)
	{
		return -3;
	}

	// 删除远程同名文件
	DeleteFile(strRemoteFileTmp);

	// 设置模式为上传文件
	strCmdLine = parseCommand(FTP_COMMAND_UPLOAD_FILE, strRemoteFileTmp);
	if (Send(m_cmdSocket, strCmdLine) < 0)
	{
		Close(data_fd);
		return -4;
	}

	trace("@@@@Response: %s\n", serverResponse(m_cmdSocket).c_str());

	// 开始传送数据
	fseek(pFile, 0, SEEK_SET);
	while (!feof(pFile))
	{
		nLen = fread(strBuf, 1, dataLen, pFile);
		if (nLen <= 0)
		{
			break;
		}

		if (Send(data_fd, strBuf, nLen) < 0)
		{
			// 传输数据失败
			Close(data_fd);
			return -5;
		}
	}
	trace("@@@@Response: %s\n", serverResponse(data_fd).c_str());
	Close(data_fd);
	trace("@@@@Response: %s\n", serverResponse(m_cmdSocket).c_str());
	fclose(pFile);

	//return 1;

	// 传输数据完毕，获取传送数据大小
	unsigned long nSize = getFileLength(strRemoteFileTmp);

	// 删除远程同名文件
	//DeleteFile(strRemoteFile);

	// 更改文件名
	if (Rename(strRemoteFileTmp, strRemoteFile) != 250)
	{
		// 重命名失败，删除刚上传的文件
		DeleteFile(strRemoteFileTmp);
		return -6;
	}

	return nSize;
}

const std::string CFTP::parseCommand(const unsigned int command, const std::string &strParam)
{
	if (command < FTP_COMMAND_BASE || command > FTP_COMMAND_END)
	{
		return "";
	}

	std::string strCommandLine;

	m_nCurrentCommand = command;
	m_commandStr.clear();

	switch (command)
	{
	case FTP_COMMAND_USERNAME:
		strCommandLine = "USER ";
		break;
	case FTP_COMMAND_PASSWORD:
		strCommandLine = "PASS ";
		break;
	case FTP_COMMAND_QUIT:
		strCommandLine = "QUIT ";
		break;
	case FTP_COMMAND_CURRENT_PATH:
		strCommandLine = "PWD ";
		break;
	case FTP_COMMAND_TYPE_MODE:
		strCommandLine = "TYPE ";
		break;
	case FTP_COMMAND_PSAV_MODE:
		strCommandLine = "PASV ";
		break;
	case FTP_COMMAND_DIR:
		strCommandLine = "LIST ";
		break;
	case FTP_COMMAND_CHANGE_DIRECTORY:
		strCommandLine = "CWD ";
		break;
	case FTP_COMMAND_DELETE_FILE:
		strCommandLine = "DELE ";
		break;
	case FTP_COMMAND_DELETE_DIRECTORY:
		strCommandLine = "RMD ";
		break;
	case FTP_COMMAND_CREATE_DIRECTORY:
		strCommandLine = "MKD ";
		break;
	case FTP_COMMAND_RENAME_BEGIN:
		strCommandLine = "RNFR ";
		break;
	case FTP_COMMAND_RENAME_END:
		strCommandLine = "RNTO ";
		break;
	case FTP_COMMAND_FILE_SIZE:
		strCommandLine = "SIZE ";
		break;
	case FTP_COMMAND_DOWNLOAD_FILE:
		strCommandLine = "RETR ";
		break;
	case FTP_COMMAND_DOWNLOAD_POS:
		strCommandLine = "REST ";
		break;
	case FTP_COMMAND_UPLOAD_FILE:
		strCommandLine = "STOR ";
		break;
	case FTP_COMMAND_APPEND_FILE:
		strCommandLine = "APPE ";
		break;
	default :
		break;
	}

	strCommandLine += strParam;
	strCommandLine += "\r\n";

	m_commandStr = strCommandLine;
	trace("parseCommand: %s\n", m_commandStr.c_str());

	return m_commandStr;
}

FTP_API CFTP::Connect(int socketfd, const std::string &serverIP, unsigned int nPort)
{
	if (socketfd == INVALID_SOCKET)
	{
		return -1;
	}

	unsigned int argp = 1;
	int error = -1;
	int len = sizeof(int);
	struct sockaddr_in  addr;
	bool ret = false;
	timeval stime;
	fd_set  set;

#ifdef _WIN32
	ioctlsocket(socketfd, FIONBIO, (u_long*)&argp); //设置为非阻塞模式
#else
	ioctl(socketfd, FIONBIO, &argp);  //设置为非阻塞模式
#endif

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port	= htons(nPort);
	addr.sin_addr.s_addr = inet_addr(serverIP.c_str());
#ifdef _WIN32
	memset(&(addr.sin_zero), 0x00, 8);
#else
	bzero(&(addr.sin_zero), 8);
#endif
	trace("Address: %s %d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
	
	if (connect(socketfd, (struct sockaddr*)&addr, sizeof(struct sockaddr)) == -1)   //若直接返回 则说明正在进行TCP三次握手
	{
		stime.tv_sec = 20;  //设置为1秒超时
		stime.tv_usec = 0;
		FD_ZERO(&set);
		FD_SET(socketfd, &set);

		if (select(socketfd + 1, NULL, &set, NULL, &stime) > 0)   ///在这边等待 阻塞 返回可以读的描述符 或者超时返回0  或者出错返回-1
		{
#ifdef WIN32
			getsockopt(socketfd, SOL_SOCKET, SO_ERROR, (char *)&error, &len);
#else
			getsockopt(socketfd, SOL_SOCKET, SO_ERROR, &error, (socklen_t*)&len);
#endif
			if (error == 0)
			{
				ret = true;
			}
			else
			{
				ret = false;
			}
		}
	}
	else
	{	trace("Connect Immediately!!!\n");
		ret = true;
	}

	argp = 0;

#ifdef _WIN32
	ioctlsocket(socketfd, FIONBIO, (u_long*)&argp);
#else
	ioctl(socketfd, FIONBIO, &argp);
#endif

	if (!ret)
	{
#ifdef WIN32
		closesocket(socketfd);
#else
		close(socketfd);
#endif
		fprintf(stderr, "cannot connect server!!\n");
		return -1;
	}

	//fprintf(stdout, "Connect!!!\n");

	return 0;
}


const std::string CFTP::serverResponse(int sockfd)
{
	if (sockfd == INVALID_SOCKET)
	{
		return "";
	}
	
	int nRet = -1;
	char buf[MAX_PATH+1] = {0};

	m_strResponse.clear();

	while ((nRet = getData(sockfd, buf, MAX_PATH)) > 0)
	{
		buf[MAX_PATH] = '\0';
		m_strResponse += buf;
	}

	return m_strResponse;
}

FTP_API CFTP::getData(int fd, char *strBuf, unsigned long length)
{
	assert(strBuf != NULL);

	if (fd == INVALID_SOCKET)
	{
		return -1;
	}

	memset(strBuf, 0, length);
	timeval stime;
	int nLen;

	stime.tv_sec = 1;
	stime.tv_usec = 0;

	fd_set	readfd;
	FD_ZERO( &readfd );
	FD_SET(fd, &readfd );

	if (select(fd + 1, &readfd, 0, 0, &stime) > 0)
	{
		if ((nLen = recv(fd, strBuf, length, 0)) > 0)
		{
			return nLen;
		}
		else
		{
			return -2;
		}
	}
	return 0;
}

FTP_API CFTP::Send(int fd, const std::string &cmd)
{
	if (fd == INVALID_SOCKET)
	{
		return -1;
	}

	return Send(fd, cmd.c_str(), cmd.length());
}

FTP_API CFTP::Send(int fd, const char *cmd, const size_t len)
{
	if((FTP_COMMAND_USERNAME != m_nCurrentCommand) 
		&&(FTP_COMMAND_PASSWORD != m_nCurrentCommand)
		&&(!m_bLogin))
	{
		return -1;
	}

	timeval timeout;
	timeout.tv_sec  = 1;
	timeout.tv_usec = 0;

	fd_set  writefd;
	FD_ZERO(&writefd);  
	FD_SET(fd, &writefd);

	if(select(fd + 1, 0, &writefd , 0 , &timeout) > 0)
	{
		size_t nlen  = len; 
		int nSendLen = 0; 
		while (nlen >0) 
		{
			nSendLen = send(fd, cmd , (int)nlen , 0);

			if(nSendLen == -1) 
				return -2; 

			nlen = nlen - nSendLen;
			cmd +=  nSendLen;
		}
		return 0;
	}
	return -1;
}


FTP_API CFTP::createDataLink(int data_fd)
{
	assert(data_fd != INVALID_SOCKET);

	std::string strData;
	unsigned long nPort = 0 ;
	std::string strServerIp ; 
	std::list<std::string> strArray ;

	std::string parseStr = Pasv();

	if (parseStr.size() <= 0)
	{
		return -1;
	}

	//trace("parseInfo: %s\n", parseStr.c_str());

	size_t nBegin = parseStr.find_first_of("(");
	size_t nEnd	  = parseStr.find_first_of(")");
	strData		  = parseStr.substr(nBegin + 1, nEnd - nBegin - 1);

	//trace("ParseAfter: %s\n", strData.c_str());
	if( SplitString( strData , strArray , "," ) <0)
		return -1;

	if( ParseString( strArray , nPort , strServerIp) < 0)
		return -1;

	//trace("nPort: %ld IP: %s\n", nPort, strServerIp.c_str());

	if (Connect(data_fd, strServerIp, nPort) < 0)
	{
		return -1;
	}

	return 0;

}

FTP_API CFTP::ParseString(std::list<std::string> strArray, unsigned long & nPort, std::string & strServerIp)
{
	if (strArray.size() < 6 )
		return -1 ;

	std::list<std::string>::iterator citor;
	citor = strArray.begin();
	strServerIp = *citor;
	strServerIp += ".";
	citor ++;
	strServerIp += *citor;
	strServerIp += ".";
	citor ++ ;
	strServerIp += *citor;
	strServerIp += ".";
	citor ++ ;
	strServerIp += *citor;
	citor = strArray.end();
	citor--;
	nPort = atol( (*citor).c_str());
	citor--;
	nPort += atol( (*(citor)).c_str()) * 256 ;
	return 0 ; 
}

FILE *CFTP::createLocalFile(const std::string &strLocalFile)
{
	return fopen(strLocalFile.c_str(), "w+b");
}

FTP_API CFTP::downLoad(const std::string &strRemoteFile, const std::string &strLocalFile, const int pos, const unsigned int length)
{
	assert(length >= 0);

	FILE *file = NULL;
	unsigned long nDataLen = FTP_DEFAULT_BUFFER;
	char strPos[MAX_PATH]  = {0};
	int data_fd = socket(AF_INET, SOCK_STREAM, 0);
	
	assert(data_fd != -1);

	if ((length != 0) && (length < nDataLen))
	{
		nDataLen = length;
	}
	char *dataBuf = new char[nDataLen];
	assert(dataBuf != NULL);

	sprintf(strPos, "%d", pos);

	if (createDataLink(data_fd) < 0)
	{
		trace("@@@@ Create Data Link error!!!\n");
		return -1;
	}

	std::string strCmdLine = parseCommand(FTP_COMMAND_DOWNLOAD_POS, strPos);
	if (Send(m_cmdSocket, strCmdLine) < 0)
	{
		return -1;
	}
	trace("@@@@Response: %s\n", serverResponse(m_cmdSocket).c_str());

	strCmdLine = parseCommand(FTP_COMMAND_DOWNLOAD_FILE, strRemoteFile);

	if (Send(m_cmdSocket, strCmdLine) < 0)
	{
		return -1;
	}
	trace("@@@@Response: %s\n", serverResponse(m_cmdSocket).c_str());

	file = createLocalFile(std::string(strLocalFile));
	assert(file != NULL);
	
	int len = 0;
	unsigned int nReceiveLen = 0;
	while ((len = getData(data_fd, dataBuf, nDataLen)) > 0)
	{
		nReceiveLen += len;

		int num = fwrite(dataBuf, 1, len, file);
		memset(dataBuf, 0, sizeof(dataBuf));
	
		//trace("%s", dataBuf);
		trace("Num:%d\n", num);
		if (nReceiveLen == length && length != 0)
			break;

		if ((nReceiveLen + nDataLen) > length  && length != 0)
		{
			delete []dataBuf;
			nDataLen = length - nReceiveLen;
			dataBuf = new char[nDataLen];
		}
	}

	Close(data_fd);
	fclose(file);
	delete []dataBuf;

	return 0;
}

FTP_API CFTP::parseResponse(const std::string &str)
{
	assert(!str.empty());

	std::string strData = str.substr(0, 3);
	unsigned int val = atoi(strData.c_str());

	return val;
}
