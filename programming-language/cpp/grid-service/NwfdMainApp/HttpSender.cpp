#include "HTTPSender.h"

#include <QString>
#include "log.h"

HttpSender::HttpSender() :
m_hCurl(NULL), m_isValid(false)
{

	LOG_(LOGID_DEBUG, LOG_F("Entry HttpSender()"));

	curl_global_init(CURL_GLOBAL_ALL);
	m_hCurl = curl_easy_init();

	if (m_hCurl)
	{
		m_isValid = true;
	}

	LOG_(LOGID_DEBUG, LOG_F("Leave HttpSender()"));
}

HttpSender::~HttpSender()
{

	LOG_(LOGID_DEBUG, LOG_F("Entry ~HttpSender()"));

	if (m_hCurl)
	{
		curl_easy_cleanup(m_hCurl);
	}

	curl_global_cleanup();

	LOG_(LOGID_DEBUG, LOG_F("Leave ~HttpSender()"));
}

bool HttpSender::isValid() const
{
	return m_isValid;
}


void HttpSender::setUrl(const std::string& url)
{
	m_url = url;
}

bool HttpSender::send(const std::string &file, int& err_code)
{

	curl_slist* pOptionList = NULL;
	pOptionList = curl_slist_append(pOptionList, "Expect:");
	curl_easy_setopt(m_hCurl, CURLOPT_HTTPHEADER, pOptionList);

	curl_httppost* pFormPost = NULL;
	curl_httppost* pLastElem = NULL;

	//上传文件，指定本地文件完整路径
	curl_formadd(&pFormPost, &pLastElem, CURLFORM_COPYNAME, "sendfile",
		CURLFORM_FILE, file.c_str(), CURLFORM_CONTENTTYPE,
		"application/octet-stream", CURLFORM_END);

	curl_formadd(&pFormPost, &pLastElem,
		CURLFORM_COPYNAME, "filename",
		CURLFORM_COPYCONTENTS, getFileNameFromPath(file).c_str(),
		CURLFORM_END);
	//不加一个结束的hfs服务端无法写入文件，一般不存在这种问题，这里加入只是为了测试.
	curl_formadd(&pFormPost, &pLastElem, CURLFORM_COPYNAME, "end", CURLFORM_COPYCONTENTS, "end", CURLFORM_END);
	curl_easy_setopt(m_hCurl, CURLOPT_HTTPPOST, pFormPost);
	curl_easy_setopt(m_hCurl, CURLOPT_URL, m_url.c_str());

	CURLcode res = curl_easy_perform(m_hCurl);
	if (res != CURLE_OK)
	{
		err_code = -1;
		return false;
	}

	curl_formfree(pFormPost);

	err_code = 0;
	return true;
}

std::string HttpSender::getFileNameFromPath(const std::string& path)
{

	return path.substr(path.find_last_of("/\\") + 1);
}

