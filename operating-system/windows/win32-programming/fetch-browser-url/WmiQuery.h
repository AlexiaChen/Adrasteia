#ifndef WMI_QUERY_H
#define WMI_QUERY_H

#include <string>
#include <exception>

#include <comdef.h>
#include <Wbemidl.h>

class WmiException : std::exception
{
public:
	explicit WmiException(const char* msg): m_msg(msg){}
	explicit WmiException(const std::string& msg) :m_msg(msg){}
	virtual ~WmiException(){};

	const char* what() { return m_msg.c_str(); }

private:
	std::string m_msg;
};

class WmiQuery
{
public:
	WmiQuery();
	~WmiQuery();
	
	enum WmiObj
	{
		OperatingSystem,

	};
	std::string execQuery(WmiObj obj, const std::string& field);

private:
	HRESULT m_hr;
	IWbemLocator *m_pLoc;
	IWbemServices *m_pSvc;

private:
	std::string wmiObjType(WmiObj obj);
};



#endif