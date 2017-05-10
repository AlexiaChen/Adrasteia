#ifndef GPO_MANAGER_H
#define GPO_MANAGER_H

#include <exception>
#include <string>
#include <utility>

#include <comdef.h>
#include <windows.h>
#include <gpedit.h>

struct IGroupPolicyObject;

class GPOException : public std::exception
{
public:
    explicit GPOException(const char* msg) : m_msg(msg){}
    explicit GPOException(const std::string& msg) :m_msg(msg.c_str()){}
    virtual ~GPOException() throw () {};

    virtual const char* what() const throw () { return m_msg.c_str(); }
private:
    std::string m_msg;
};

class GPOManager
{
public:
    GPOManager();
    ~GPOManager();

    enum GPOType
    {
        USB_DEVICE_REMOVE, /*Prevent Windows from installing removable devices*/
    };

    void applyPolicy(GPOType type, bool enable);
private:
    std::pair<std::string, std::string> polictyType(GPOType type);
private:
    HRESULT m_hr;
    HRESULT m_hrComInit;
    IGroupPolicyObject* m_pLGPO;
    HKEY m_machine_key;
private:
    static const IID my_IID_IGroupPolicyObject;
    static const IID my_CLSID_GroupPolicyObject;
    static const GUID ext_guid;
    // This next one can be any GUID you want
    static const GUID snap_guid;
};

#endif