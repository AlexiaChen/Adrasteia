#include "WmiQuery.h"

#include <iostream>
#include <sstream>

#pragma comment(lib, "wbemuuid.lib")

WmiQuery::WmiQuery() :m_hr(NULL),
m_pLoc(NULL),
m_pSvc(NULL)
{
	m_hr = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (FAILED(m_hr))
	{
		std::ostringstream errorStream;
		errorStream << "Failed to initialize COM library. Error code = 0x" << std::hex << m_hr << std::endl;
		throw WmiException(errorStream.str());
	}

	m_hr = CoInitializeSecurity(
		NULL,                        // Security descriptor    
		-1,                          // COM negotiates authentication service
		NULL,                        // Authentication services
		NULL,                        // Reserved
		RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication level for proxies
		RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation level for proxies
		NULL,                        // Authentication info
		EOAC_NONE,                   // Additional capabilities of the client or server
		NULL);                       // Reserved

	if (FAILED(m_hr))
	{
		std::ostringstream errorStream;
		errorStream << "Failed to initialize security. Error code = 0x" << std::hex << m_hr <<std::endl;
		CoUninitialize();
		throw WmiException(errorStream.str());
	}
	
	m_hr = CoCreateInstance(
		CLSID_WbemLocator,
		0,
		CLSCTX_INPROC_SERVER,
		IID_IWbemLocator, (LPVOID *)&m_pLoc);

	if (FAILED(m_hr))
	{
		std::ostringstream errorStream;
		errorStream << "Failed to create IWbemLocator object." << " Err code = 0x" << std::hex << m_hr << std::endl;
		CoUninitialize();
		throw WmiException(errorStream.str());
	}


	// Connect to the root\default namespace with the current user.
	m_hr = m_pLoc->ConnectServer(
		BSTR(L"ROOT\\CIMV2"),  //namespace  ROOT\\DEFAULT
		NULL,       // User name 
		NULL,       // User password
		0,         // Locale 
		NULL,     // Security flags
		0,         // Authority 
		0,        // Context object 
		&m_pSvc);   // IWbemServices proxy


	if (FAILED(m_hr))
	{
		std::ostringstream errorStream;
		errorStream << "Could not connect. Error code = 0x" << std::hex << m_hr << std::endl;
		m_pLoc->Release();
		CoUninitialize();
		throw WmiException(errorStream.str());
	}

	// Set the proxy so that impersonation of the client occurs.
	m_hr = CoSetProxyBlanket(m_pSvc,
		RPC_C_AUTHN_WINNT,
		RPC_C_AUTHZ_NONE,
		NULL,
		RPC_C_AUTHN_LEVEL_CALL,
		RPC_C_IMP_LEVEL_IMPERSONATE,
		NULL,
		EOAC_NONE
		);

	if (FAILED(m_hr))
	{
		std::ostringstream errorStream;
		errorStream << "Could not set proxy blanket. Error code = 0x" << std::hex << m_hr << std::endl;
		m_pSvc->Release();
		m_pLoc->Release();
		CoUninitialize();
		throw WmiException(errorStream.str());
	}



}

WmiQuery::~WmiQuery()
{
	m_pSvc->Release();
	m_pLoc->Release();
	CoUninitialize();
}

std::string WmiQuery::execQuery(WmiObj obj, const std::string& field) 
{
	
	std::string wqlQuery =
		"SELECT * FROM " + wmiObjType(obj);
	
	IEnumWbemClassObject* pEnumerator = NULL;
	m_hr = m_pSvc->ExecQuery(
		bstr_t("WQL"),
		bstr_t(wqlQuery.c_str()),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		NULL,
		&pEnumerator);

	if (FAILED(m_hr))
	{
		std::ostringstream errorStream;
		errorStream << "Query for operating system name failed." <<  " Error code = 0x" <<std::hex << m_hr << std::endl;
		m_pSvc->Release();
		m_pLoc->Release();
		CoUninitialize();
		throw WmiException(errorStream.str());
	}

	IWbemClassObject *pclsObj = NULL;
	ULONG uReturn = 0;

	while (pEnumerator)
	{
		HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
			&pclsObj, &uReturn);

		if (0 == uReturn)
		{
			break;
		}

		VARIANT vtProp;

		// Get the value of the Name property
		std::wstring fieldStr(field.begin(),field.end());
		
		hr = pclsObj->Get(fieldStr.c_str(), 0, &vtProp, 0, 0);
		
		std::wstring result_wstr(vtProp.bstrVal);
		std::string result(result_wstr.begin(), result_wstr.end());

		VariantClear(&vtProp);
		pclsObj->Release();

		return result;
	}

	return std::string("");
}

std::string WmiQuery::wmiObjType(WmiObj obj)
{
	switch (obj)
	{
	case WmiQuery::OperatingSystem: return std::string("Win32_OperatingSystem");
	default: throw WmiException("Not a valid Wmi Object!");
	}
}