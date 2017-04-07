#include "BrowserInfo.h"

#include <array>
#include <windows.h>

std::string ActiveTab::url(ActiveTab::BrowserType type)
{
	std::string url;
	switch (type)
	{
	case ActiveTab::FIRE_FOX:
		url = commen_url("FireFox");
		break;
	case ActiveTab::INTERNET_EXPLORER:
		url = ie_url();
		break;
	case ActiveTab::CHROME:
		url = commen_url("Chrome");
		break;
	case ActiveTab::OPERA:
		url = commen_url("Opera");
		break;
	default:
		break;
	}
	
	return url;
}

BOOL CALLBACK ActiveTab::BrowserIEChildEnumProc(HWND hwndChild, LPARAM lParam)
{
	

	std::array<char, 256> className{ { 0 } };
	GetClassName(hwndChild, className.data(), 256*sizeof(char));
		
	std::array<char, 4028> url{ { 0 } };

	std::string class_name(className.data());
	if (class_name.compare("Edit") == 0)
	{
		// 必须用SendMessage, 不然用GetWindowText会得不到，因为是远程进程
		SendMessage(hwndChild, WM_GETTEXT, 4028*sizeof(char), (LPARAM)url.data()); 
			
	}
	else if (class_name.compare("ComboBoxEx32") == 0)
	{
		SendMessage(hwndChild, WM_GETTEXT, 4028*sizeof(char), (LPARAM)url.data());

	}
	else if (class_name.compare("TabWindowClass") == 0)
	{
		SendMessage(hwndChild, WM_GETTEXT, 4028*sizeof(char), (LPARAM)url.data());
	}
	else
	{

	}

	if (valid_url(url.data()))
	{
		memcpy((void*)lParam, url.data(), 4028);
	}

	return TRUE;
}

BOOL CALLBACK ActiveTab::BrowserIEEnumProc(HWND hwndChild, LPARAM lParam)
{
	
		
	std::array<char, 256> className{ {0} };

	GetClassName(hwndChild, className.data(), 256*sizeof(char));

	std::string class_name(className.data());
		
	if (class_name.compare("IEFrame") == 0)
	{
		EnumChildWindows(hwndChild, ActiveTab::BrowserIEChildEnumProc, lParam);
	}
	else if (class_name.compare("Edit") == 0)
	{
			
		std::array<char, 4028> url{ { 0 } };
		SendMessage(hwndChild, WM_GETTEXT, 4028*sizeof(char), (LPARAM)url.data());
		if (valid_url(url.data()))
		{
			memcpy((void*)lParam, url.data(), 4028);
		}
			
	}
	else{ }
		


	return TRUE;
}

std::string ActiveTab::DDERequest(DWORD idInst, HCONV hConv)
{
	HSZ hszItem = DdeCreateStringHandle(idInst, "\\Doc", 0);

	HDDEDATA hData = DdeClientTransaction(NULL, 0, hConv, hszItem, CF_TEXT,
		XTYP_REQUEST, 5000, NULL); //5秒超时

	if (hData == NULL)
	{
		return std::string("");
	}
	else
	{
		char szResult[4028] = {0};
		DdeGetData(hData, (unsigned char *)szResult, 4028, 0);

		return std::string(szResult);
	}
}

std::string ActiveTab::ie_url()
{
	std::array<char,4028> url{ { 0 } };
	
	EnumChildWindows(NULL, ActiveTab::BrowserIEEnumProc, (LPARAM)url.data());

	return std::string(url.data());

}

HDDEDATA CALLBACK
ActiveTab::DdeCallback(UINT uType, UINT uFmt, HCONV hconv,
HSZ hsz1, HSZ hsz2, HDDEDATA hdata,
DWORD dwData1, DWORD dwData2)
{
	(void)uType;
	(void)uFmt;
	(void)hconv;
	(void)hsz1;
	(void)hsz2;
	(void)hdata;
	(void)dwData1;
	(void)dwData2;
	
	return ((HDDEDATA)NULL);
}

std::string ActiveTab::commen_url(const std::string& type)
{
	DWORD idDde = 0;
	HCONV HConversation;
	HSZ   Server = NULL;
	HSZ   Topic = NULL;

	DdeInitialize(&idDde, (PFNCALLBACK)ActiveTab::DdeCallback, APPCLASS_STANDARD | APPCMD_CLIENTONLY, 0);

	Server = DdeCreateStringHandle(idDde, type.c_str(), CP_WINANSI);
	Topic = DdeCreateStringHandle(idDde, "WWW_GetWindowInfo", CP_WINANSI);


	HConversation = DdeConnect(idDde, Server, Topic, NULL);

	if (HConversation == 0)
	{
		DWORD error_code = DdeGetLastError(idDde);
		(void)error_code;
		DdeUninitialize(idDde);
		return std::string("");
	}

	
	//请求数据
	std::string url = DDERequest(idDde, HConversation);

	//断开连接,释放资源
	DdeDisconnect(HConversation);
	DdeUninitialize(idDde);

	return url;
}

bool ActiveTab::valid_url(const std::string& url)
{
	if (url.empty()) return false;
	if (url.find("https") == std::string::npos || url.find("http") == std::string::npos) return false;
	return true;
}

