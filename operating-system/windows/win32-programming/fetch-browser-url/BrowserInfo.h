#ifndef BROWSER_INFO_H
#define BROWSER_INFO_H

#include <string>
#include <windows.h>
#include <ddeml.h>

class ActiveTab
{
public:
	enum BrowserType
	{
		FIRE_FOX,
		INTERNET_EXPLORER,
		CHROME,
		OPERA
	};

	static std::string url(BrowserType type);
	
private:
	static std::string ie_url();
	static std::string commen_url(const std::string& type);
	
	static BOOL CALLBACK BrowserIEEnumProc(HWND hwndChild, LPARAM lParam);
	static BOOL CALLBACK BrowserIEChildEnumProc(HWND hwndChild, LPARAM lParam);
	static HDDEDATA CALLBACK DdeCallback(UINT uType, UINT uFmt, HCONV hconv,
		HSZ hsz1, HSZ hsz2, HDDEDATA hdata,
		DWORD dwData1, DWORD dwData2);
	
	static std::string DDERequest(DWORD idInst, HCONV hConv);
	static bool valid_url(const std::string& url);


	

};


#endif