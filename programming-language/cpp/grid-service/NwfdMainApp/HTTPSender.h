#ifndef HTTP_SENDER_H
#define HTTP_SENDER_H


#include <string>
#include <curl/curl.h>

class HttpSender {

public:
	HttpSender();
	~HttpSender();

	bool isValid() const;
	void setUrl(const std::string& url);
	bool send(const std::string &file,int& err_code); 

private:
	std::string getFileNameFromPath(const std::string& path);


private:
	bool m_bReady;
	CURL* m_hCurl;
	std::string m_url;
	bool m_isValid;
};


#endif