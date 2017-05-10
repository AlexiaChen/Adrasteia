#include "GPOManager.h"

#include <sstream>
#include <Userenv.h>
#include <objbase.h>

#pragma comment(lib,"Userenv.lib")

const IID GPOManager::my_IID_IGroupPolicyObject = { 0xea502723, 0xa23d, 0x11d1, { 0xa7, 0xd3, 0x0, 0x0, 0xf8, 0x75, 0x71, 0xe3 } };
const IID GPOManager::my_CLSID_GroupPolicyObject = { 0xea502722, 0xa23d, 0x11d1, { 0xa7, 0xd3, 0x0, 0x0, 0xf8, 0x75, 0x71, 0xe3 } };
const GUID GPOManager::ext_guid = REGISTRY_EXTENSION_GUID ;
const GUID GPOManager::snap_guid = { 0x3d271cfc, 0x2bc6, 0x4ac2, { 0xb6, 0x33, 0x3b, 0xdf, 0xf5, 0xbd, 0xab, 0x2a } };

GPOManager::GPOManager():
m_pLGPO(nullptr),
m_machine_key(NULL),
m_hr(S_FALSE),
m_hrComInit(S_FALSE)
{
#ifndef GOP_DLL_USE
    m_hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    m_hrComInit = m_hr;
    if (FAILED(m_hr))
    {
        std::ostringstream errorStream;
        errorStream << "Failed to initialize COM library. Error code = 0x" << std::hex << m_hr << std::endl;
        throw GPOException(errorStream.str());
    }
#endif

    // Create an instance of the IGroupPolicyObject class
    m_hr = CoCreateInstance(my_CLSID_GroupPolicyObject, NULL, CLSCTX_INPROC_SERVER,
        my_IID_IGroupPolicyObject, (LPVOID*)&m_pLGPO);
    if (FAILED(m_hr))
    {
        std::ostringstream errorStream;
        errorStream << "Failed to create IGroupPolicy object." << " Err code = 0x" << std::hex << m_hr << std::endl;
#ifndef GOP_DLL_USE
        if (SUCCEEDED(m_hrComInit)) CoUninitialize();
#endif
        throw GPOException(errorStream.str());
    }

    // We need the machine LGPO
    m_hr = m_pLGPO->OpenLocalMachineGPO(GPO_OPEN_LOAD_REGISTRY);
    if (FAILED(m_hr))
    {
        std::ostringstream errorStream;
        errorStream << "Failed to open Local Machine LGPO" << " Err code = 0x" << std::hex << m_hr << std::endl;
        m_pLGPO->Release();
#ifndef GOP_DLL_USE
        if (SUCCEEDED(m_hrComInit)) CoUninitialize();
#endif
        throw GPOException(errorStream.str());
    }

    m_hr = m_pLGPO->GetRegistryKey(GPO_SECTION_MACHINE, &m_machine_key);
    if (FAILED(m_hr))
    {
        std::ostringstream errorStream;
        errorStream << "Failed to Get Registry Key" << " Err code = 0x" << std::hex << m_hr << std::endl;
        m_pLGPO->Release();
#ifndef GOP_DLL_USE
        if (SUCCEEDED(m_hrComInit)) CoUninitialize();
#endif
        throw GPOException(errorStream.str());
    }

}

GPOManager::~GPOManager()
{
   if (m_machine_key != NULL) RegCloseKey(m_machine_key);
    
   if(m_pLGPO != NULL) m_pLGPO->Release();
#ifndef GPO_DLL_USE
   if (SUCCEEDED(m_hrComInit)) CoUninitialize();
#endif
}

void GPOManager::applyPolicy(GPOType type, bool enable)
{
    std::string keyPath = polictyType(type).first;
    std::string keyName = polictyType(type).second;
    if (keyPath.empty() || keyName.empty())
    {
        throw GPOException("you  are applying invalid GPO Type");
    }
    
    HKEY dsrkey = NULL;
    LONG ret = RegCreateKeyEx(m_machine_key, keyPath.c_str(),
        0, NULL, 0, KEY_SET_VALUE | KEY_QUERY_VALUE, NULL, &dsrkey, NULL);

    if (ret != ERROR_SUCCESS)
    {
        std::ostringstream errorStream;
        errorStream << "Failed to create reg key" << std::hex << ret << std::endl;
        throw GPOException(errorStream.str());
    }

    // Create the value
    DWORD val = enable ? 1 : 0;
    ret = RegSetKeyValue(dsrkey, NULL, keyName.c_str(), REG_DWORD, &val, sizeof(val));
    if (ret != ERROR_SUCCESS)
    {
        std::ostringstream errorStream;
        errorStream << "Failed to set reg key value" << std::hex << ret << std::endl;
        RegCloseKey(dsrkey);
        throw GPOException(errorStream.str());
    }
    
    RegCloseKey(dsrkey);
    
    // Apply policy and free resources
    m_hr = m_pLGPO->Save(TRUE, TRUE, const_cast<GUID*>(&ext_guid), const_cast<GUID*>(&snap_guid));
    if (FAILED(m_hr))
    {
        std::ostringstream errorStream;
        errorStream << "Failed to save policy" << std::hex << m_hr << std::endl;
        throw GPOException(errorStream.str());
    }

    RefreshPolicyEx(TRUE, RP_FORCE);
}

std::pair<std::string, std::string> GPOManager::polictyType(GPOType type)
{
    switch (type)
    {
    case GPOManager::USB_DEVICE_REMOVE:
        return std::make_pair(std::string("SOFTWARE\\Policies\\Microsoft\\Windows\\DeviceInstall\\Restrictions"), 
            std::string("DenyRemovableDevices"));
    default:
        return std::make_pair(std::string(),std::string());
    }
}