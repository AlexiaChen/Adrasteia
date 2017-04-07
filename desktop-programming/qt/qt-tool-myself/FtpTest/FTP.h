/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* �ļ���: FTP.h
* ��  ��: zhangl  		�汾��1.0		��  �ڣ�2015-07-14
* ��  ����Ftp������
* ��  ����
* �����б�:
* �޸���־��
*   No.   ����		  ����		       �޸�����
* ------------------------------------------------------------------------
*************************************************************************/
#ifndef FTP_H_
#define FTP_H_

#ifdef WIN32
#include <winsock.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#endif
#include <assert.h>
#include <string.h>
#include <string>
#include <list>

#ifndef _WIN32
#define INVALID_SOCKET				-1
#endif

#define FTP_API						int
#define MAX_PATH					260
#define trace						printf

#define FTP_PARAM_BASE
#define FTP_DEFAULT_PORT			"21"							//FTPĬ�϶˿ں�
#define FTP_DEFAULT_BUFFER			1024*4							//FTP���ػ���Ĭ�ϴ�С
#define FTP_DEFAULT_PATH			"/mnt/dvs/"						//FTPĬ�ϱ���·��
	
#define FTP_COMMAND_BASE			1000
#define FTP_COMMAND_END				FTP_COMMAND_BASE + 30
#define FTP_COMMAND_USERNAME		FTP_COMMAND_BASE + 1			//�û���
#define FTP_COMMAND_PASSWORD		FTP_COMMAND_BASE + 2			//����
#define FTP_COMMAND_QUIT			FTP_COMMAND_BASE + 3			//�˳�
#define FTP_COMMAND_CURRENT_PATH	FTP_COMMAND_BASE + 4			// ��ȡ�ļ�·��
#define FTP_COMMAND_TYPE_MODE		FTP_COMMAND_BASE + 5			// �ı䴫��ģʽ
#define FTP_COMMAND_PSAV_MODE		FTP_COMMAND_BASE + 6			// �����˿�ģʽ
#define FTP_COMMAND_DIR				FTP_COMMAND_BASE + 7			// ��ȡ�ļ��б�
#define FTP_COMMAND_CHANGE_DIRECTORY FTP_COMMAND_BASE + 8			// �ı�·��
#define FTP_COMMAND_DELETE_FILE		FTP_COMMAND_BASE + 9			// ɾ���ļ�
#define FTP_COMMAND_DELETE_DIRECTORY FTP_COMMAND_BASE + 10			// ɾ��Ŀ¼/�ļ���
#define FTP_COMMAND_CREATE_DIRECTORY FTP_COMMAND_BASE + 11			// ����Ŀ¼/�ļ���
#define FTP_COMMAND_RENAME_BEGIN    FTP_COMMAND_BASE  +12			// ��ʼ������
#define FTP_COMMAND_RENAME_END      FTP_COMMAND_BASE + 13			// ����������
#define FTP_COMMAND_FILE_SIZE		FTP_COMMAND_BASE + 14			// ��ȡ�ļ���С
#define FTP_COMMAND_DOWNLOAD_POS	FTP_COMMAND_BASE + 15			// �����ļ���ָ��λ�ÿ�ʼ
#define FTP_COMMAND_DOWNLOAD_FILE	FTP_COMMAND_BASE + 16			// �����ļ�
#define FTP_COMMAND_UPLOAD_FILE		FTP_COMMAND_BASE + 17			// �ϴ��ļ�
#define FTP_COMMAND_APPEND_FILE		FTP_COMMAND_BASE + 18			// ׷�������ļ�	

/*		  ��½����
		login2Server
			|
		inputUserName
			|
		inputPassWord
			|
		  �������
			|
		  quit
*/

class CFTP
{
public :
	
	enum type {
		binary = 0x31,
		ascii,
	};
	
	CFTP(void);

	virtual ~CFTP(void);
	
	bool IsLogin();

	// ! ��½������
	FTP_API login2Server(const std::string &serverIP);

	// !�����û���
	FTP_API inputUserName(const std::string &userName);

	// !��������
	FTP_API inputPassWord(const std::string &password);

	// !�˳�FTP
	FTP_API quitServer(void);

	// !��� PWD
	const std::string PWD();

	// !���ô����ʽ 2����  ����ascii��ʽ����
	FTP_API setTransferMode(type mode);

	// !����Ϊ����ģʽ
	const std::string Pasv();

	// ! ��� DIR
	const std::string Dir(const std::string &path);

	// !���� �� CD
	FTP_API CD(const std::string &path);

	// ��ɾ���ļ�
	FTP_API DeleteFile(const std::string &strRemoteFile);

	// ! ɾ���ļ���/Ŀ¼
	FTP_API DeleteDirectory(const std::string &strRemoteDir);

	// ! ����Ŀ¼/�ļ���
	FTP_API CreateDirectory(const std::string &strRemoteDir);

	// !������
	FTP_API Rename(const std::string &strRemoteFile, const std::string &strNewFile);

	// !��ȡ�ļ���С
	long getFileLength(const std::string &strRemoteFile);

	// !�ر�����
	void Close(int sock);

	// �����ļ�
	FTP_API Get(const std::string &strRemoteFile, const std::string &strLocalFile);

	// �����ļ�  ֧�ֶϵ紫�ͷ�ʽ
	FTP_API Put(const std::string &strRemoteFile, const std::string &strLocalFile);

private:
	// !�ϳɷ��͵�������������
	const std::string parseCommand(const unsigned int command, const std::string &strParam);

	// ! ��������
	FTP_API Connect(int socketfd, const std::string &serverIP, unsigned int nPort);

	// ! ���ط�������Ϣ
	const std::string serverResponse(int sockfd);

	// !��ȡ����������
	FTP_API getData(int fd, char *strBuf, unsigned long length);

	// !��������
	FTP_API Send(int fd, const std::string &cmd);

	// !��������
	FTP_API Send(int fd, const char *cmd, const size_t len);

	// !������������
	FTP_API createDataLink(int data_fd);

	// !����PASVģʽ���ص��ַ�����ȡFTP�˿ںź�FTP������IP
	FTP_API ParseString(std::list<std::string> strArray, unsigned long & nPort ,std::string & strServerIp);

	// �򿪱����ļ�
	FILE *createLocalFile(const std::string &strLocalFile);

	// �����ļ� 
	FTP_API downLoad(const std::string &strRemoteFile, const std::string &strLocalFile, const int pos = 0, const unsigned int length = 0);

	// ��������ftp�����ֵ
	FTP_API parseResponse(const std::string &str);

private:
	//�����������׽���
	int		m_cmdSocket;
	
	// !��ǰ�û���
	std::string m_strUserName;

	// !��ǰ�û�����
	std::string m_strPassWord;

	// !��������IP
	std::string m_strServerIP;

	// !������Port
	unsigned int m_nServerPort;

	// !��������Ӧ��Ϣ����
	std::string m_strResponse;

	// !�����������
	std::string m_commandStr;

	// ����ǰʹ�õ��������
	unsigned int m_nCurrentCommand;

	// !�Ƿ��½��־��
	bool	m_bLogin;
};

 
#endif // FTP_H_
