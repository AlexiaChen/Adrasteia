#include "FWManager.h"

#include <sstream>
#include <netfw.h>
#include <comutil.h>
#include <atlcomcli.h>

#pragma comment( lib, "ole32.lib" )
#pragma comment( lib, "oleaut32.lib" )

FWManager::FWManager():
m_hrComInit(S_FALSE),
m_hr(S_FALSE),
m_pNetFwPolicy2(NULL)
{
    // Initialize COM.
    m_hrComInit = CoInitializeEx(
        0,
        COINIT_APARTMENTTHREADED
        );

    // Ignore RPC_E_CHANGED_MODE; this just means that COM has already been
    // initialized with a different mode. Since we don't care what the mode is,
    // we'll just use the existing mode.
    if (m_hrComInit != RPC_E_CHANGED_MODE)
    {
        if (FAILED(m_hrComInit))
        {
            std::ostringstream errorStream;
            errorStream << "CoInitializeEx failed: 0x" << std::hex << m_hrComInit << std::endl;
            throw FWException(errorStream.str());
        }
    }

    // Retrieve INetFwPolicy2
    m_hr = WFCOMInitialize(&m_pNetFwPolicy2);
    if (FAILED(m_hr))
    {
        std::ostringstream errorStream;
        errorStream << "WFCOMInitialize for INetFwPolicy2 failed : 0x" << std::hex << m_hr << std::endl;
        if (SUCCEEDED(m_hrComInit)) CoUninitialize();
        throw FWException(errorStream.str());
    }
}

FWManager::~FWManager()
{
    // Release INetFwPolicy2
    if (m_pNetFwPolicy2 != NULL)
    {
        m_pNetFwPolicy2->Release();
    }

    // Uninitialize COM.
    if (SUCCEEDED(m_hrComInit))
    {
        CoUninitialize();
    }
}

void FWManager::enableFirewall(bool enable)
{
    // enable Windows Firewall for the Domain profile
    m_hr = m_pNetFwPolicy2->put_FirewallEnabled(NET_FW_PROFILE2_DOMAIN, enable);
    if (FAILED(m_hr))
    {
        std::ostringstream errorStream;
        errorStream << "put_FirewallEnabled failed for domain: 0x" << std::hex << m_hr << std::endl;
        throw FWException(errorStream.str());
       
    }
    
    // enable Windows Firewall for the Private profile
    m_hr = m_pNetFwPolicy2->put_FirewallEnabled(NET_FW_PROFILE2_PRIVATE, enable);
    if (FAILED(m_hr))
    {
        std::ostringstream errorStream;
        errorStream << "put_FirewallEnabled failed for private: 0x" << std::hex << m_hr << std::endl;
        throw FWException(errorStream.str());
    }

    // enable Windows Firewall for the Public profile
    m_hr = m_pNetFwPolicy2->put_FirewallEnabled(NET_FW_PROFILE2_PUBLIC, enable);
    if (FAILED(m_hr))
    {
        std::ostringstream errorStream;
        errorStream << "put_FirewallEnabled failed for public: 0x" << std::hex << m_hr << std::endl;
        throw FWException(errorStream.str());
        
    }
}

bool FWManager::isFirewallEnabled()
{
    
    std::array<bool, 3> enableCheck{ { false } };
    VARIANT_BOOL enabled;
    m_hr = m_pNetFwPolicy2->get_FirewallEnabled(NET_FW_PROFILE2_DOMAIN, &enabled);
    if (FAILED(m_hr))
    {
        std::ostringstream errorStream;
        errorStream << "get_FirewallEnabled failed for domain: 0x" << std::hex << m_hr << std::endl;
        throw FWException(errorStream.str());
    }
    enableCheck[0] = enabled ? true : false;

    m_hr = m_pNetFwPolicy2->get_FirewallEnabled(NET_FW_PROFILE2_PRIVATE, &enabled);
    if (FAILED(m_hr))
    {
        std::ostringstream errorStream;
        errorStream << "get_FirewallEnabled failed for private: 0x" << std::hex << m_hr << std::endl;
        throw FWException(errorStream.str());
    }
    enableCheck[1] = enabled ? true : false;

    m_hr = m_pNetFwPolicy2->get_FirewallEnabled(NET_FW_PROFILE2_PUBLIC, &enabled);
    if (FAILED(m_hr))
    {
        std::ostringstream errorStream;
        errorStream << "get_FirewallEnabled failed for public: 0x" << std::hex << m_hr << std::endl;
        throw FWException(errorStream.str());
    }
    enableCheck[2] = enabled ? true : false;

    for (bool b : enableCheck)
    {
        if (!b) return false;
    }

    return true;
}

void FWManager::enumFirewallRules(std::vector<FW_RULE> &rules, const std::string group)
{
    INetFwRules *pFwRules = NULL;
    // Retrieve INetFwRules
    m_hr = m_pNetFwPolicy2->get_Rules(&pFwRules);
    if (FAILED(m_hr))
    {
        std::ostringstream errorStream;
        errorStream << "get_Rules failed in enumFirewallRules: 0x" << std::hex << m_hr << std::endl;
        throw FWException(errorStream.str());
    }

    // Obtain the number of Firewall rules
    long fwRuleCount = 0;
    m_hr = pFwRules->get_Count(&fwRuleCount);
    if (FAILED(m_hr))
    {
        std::ostringstream errorStream;
        errorStream << "get_Count failed in enumFirewallRules: 0x" << std::hex << m_hr << std::endl;
        throw FWException(errorStream.str());
    }

    // Iterate through all of the rules in pFwRules
    IUnknown *pEnumerator = NULL;
    IEnumVARIANT* pVariant = NULL;
    pFwRules->get__NewEnum(&pEnumerator);

    if (pEnumerator)
    {
        m_hr = pEnumerator->QueryInterface(__uuidof(IEnumVARIANT), reinterpret_cast<void**>(&pVariant));
    }

    CComVariant var;
    ULONG cFetched = 0;
    INetFwRule *pFwRule = NULL;
    while (SUCCEEDED(m_hr) && m_hr != S_FALSE)
    {
        var.Clear();
        m_hr = pVariant->Next(1, &var, &cFetched);

        if (S_FALSE != m_hr)
        {
            if (SUCCEEDED(m_hr))
            {
                m_hr = var.ChangeType(VT_DISPATCH);
            }
            if (SUCCEEDED(m_hr))
            {
                m_hr = (V_DISPATCH(&var))->QueryInterface(__uuidof(INetFwRule), reinterpret_cast<void**>(&pFwRule));
            }

            if (SUCCEEDED(m_hr))
            {
                // Output the properties of this rule
                DumpFWRulesInCollection(pFwRule,rules, group);
            }
        }
    }

}

void FWManager::enbaleRuleGroup(const std::string& group, FWManager::PROFILE profile, bool enable)
{
    BSTR bstrRuleGroup = SysAllocString(std::wstring(group.begin(),group.end()).c_str());
    
    switch (profile)
    {
    case FWManager::PROFILE::DOMAIN_PROFILE:
        {
            m_hr = m_pNetFwPolicy2->EnableRuleGroup(NET_FW_PROFILE2_DOMAIN, bstrRuleGroup, enable);
            if (FAILED(m_hr))
            {
                std::ostringstream errorStream;
                errorStream << "EnableRuleGroup failed for domain in enbaleRuleGroup: 0x" << std::hex << m_hr << std::endl;
                SysFreeString(bstrRuleGroup);
                throw FWException(errorStream.str());

            }
        }
    break;
    case FWManager::PROFILE::PUBLIC_PROFILE:
        {
            m_hr = m_pNetFwPolicy2->EnableRuleGroup(NET_FW_PROFILE2_PUBLIC, bstrRuleGroup, enable);
            if (FAILED(m_hr))
            {
                std::ostringstream errorStream;
                errorStream << "EnableRuleGroup failed for public in enbaleRuleGroup: 0x" << std::hex << m_hr << std::endl;
                SysFreeString(bstrRuleGroup);
                throw FWException(errorStream.str());

            }
        }
    break;
    case FWManager::PROFILE::PRIVATE_PROFILE:
        {
            m_hr = m_pNetFwPolicy2->EnableRuleGroup(NET_FW_PROFILE2_PRIVATE, bstrRuleGroup, enable);
            if (FAILED(m_hr))
            {
                std::ostringstream errorStream;
                errorStream << "EnableRuleGroup failed for private in enbaleRuleGroup: 0x" << std::hex << m_hr << std::endl;
                SysFreeString(bstrRuleGroup);
                throw FWException(errorStream.str());

            }
        }
    break;
    default:
        break;
    }
    
    SysFreeString(bstrRuleGroup);

}

bool FWManager::isRuleGroupEnable(const std::string& group,FWManager::PROFILE profile)
{
    BSTR bstrRuleGroup = SysAllocString(std::wstring(group.begin(), group.end()).c_str());
    VARIANT_BOOL bIsEnabled = FALSE;

    switch (profile)
    {
    case FWManager::PROFILE::DOMAIN_PROFILE:
        {
            m_hr = m_pNetFwPolicy2->IsRuleGroupEnabled(NET_FW_PROFILE2_DOMAIN, bstrRuleGroup, &bIsEnabled);
            if (FAILED(m_hr))
            {
                std::ostringstream errorStream;
                errorStream << "IsRuleGroupEnabled failed for domain in isRuleGroupEnable: 0x" << std::hex << m_hr << std::endl;
                SysFreeString(bstrRuleGroup);
                throw FWException(errorStream.str());
            }
        }
        break;
    case FWManager::PROFILE::PUBLIC_PROFILE:
        {
            m_hr = m_pNetFwPolicy2->IsRuleGroupEnabled(NET_FW_PROFILE2_PUBLIC, bstrRuleGroup, &bIsEnabled);
            if (FAILED(m_hr))
            {
                std::ostringstream errorStream;
                errorStream << "IsRuleGroupEnabled failed for public in isRuleGroupEnable: 0x" << std::hex << m_hr << std::endl;
                SysFreeString(bstrRuleGroup);
                throw FWException(errorStream.str());
            }
        }
        break;
    case FWManager::PROFILE::PRIVATE_PROFILE:
        {
            m_hr = m_pNetFwPolicy2->IsRuleGroupEnabled(NET_FW_PROFILE2_PRIVATE, bstrRuleGroup, &bIsEnabled);
            if (FAILED(m_hr))
            {
                std::ostringstream errorStream;
                errorStream << "IsRuleGroupEnabled failed for private in isRuleGroupEnable: 0x" << std::hex << m_hr << std::endl;
                SysFreeString(bstrRuleGroup);
                throw FWException(errorStream.str());
            }
        }
        break;
    default:
        break;
    }
    SysFreeString(bstrRuleGroup);
    return bIsEnabled ? true : false;
}


void FWManager::addRule(const NEW_FW_RULE& rule, PROFILE profile, const std::string& to_group)
{
    BSTR bstrRuleName = SysAllocString(std::wstring(rule.name.begin(),rule.name.end()).c_str());
    BSTR bstrRuleDescription = SysAllocString(std::wstring(rule.description.begin(),rule.description.end()).c_str());
    BSTR bstrRuleGroup = SysAllocString(std::wstring(to_group.begin(),to_group.end()).c_str());
    BSTR bstrRuleApplication = SysAllocString(std::wstring(rule.appname.begin(),rule.appname.end()).c_str());
    BSTR bstrRuleLPorts = SysAllocString(std::wstring(rule.local_ports.begin(),rule.local_ports.end()).c_str());
    BSTR bstrRuleLAddr = SysAllocString(std::wstring(rule.local_addr.begin(), rule.local_addr.end()).c_str());
    BSTR bstrRuleServiceName = SysAllocString(std::wstring(rule.servicename.begin(), rule.servicename.end()).c_str());

    // Retrieve INetFwRules
    INetFwRules *pFwRules = NULL;
    m_hr = m_pNetFwPolicy2->get_Rules(&pFwRules);
    if (FAILED(m_hr))
    {
        std::ostringstream errorStream;
        errorStream << "get_Rules failed in addRule: 0x" << std::hex << m_hr << std::endl;
        SysFreeString(bstrRuleName);
        SysFreeString(bstrRuleDescription);
        SysFreeString(bstrRuleGroup);
        SysFreeString(bstrRuleApplication);
        SysFreeString(bstrRuleLPorts);
        SysFreeString(bstrRuleLAddr);
        SysFreeString(bstrRuleServiceName);
        throw FWException(errorStream.str());
    }

    // Retrieve Current Profiles bitmask
    long CurrentProfilesBitMask = 0;
    m_hr = m_pNetFwPolicy2->get_CurrentProfileTypes(&CurrentProfilesBitMask);
    if (FAILED(m_hr))
    {
        std::ostringstream errorStream;
        errorStream << "get_CurrentProfileTypes failed in addRule: 0x" << std::hex << m_hr << std::endl;
        if (pFwRules != NULL)    pFwRules->Release();
        SysFreeString(bstrRuleName);
        SysFreeString(bstrRuleDescription);
        SysFreeString(bstrRuleGroup);
        SysFreeString(bstrRuleApplication);
        SysFreeString(bstrRuleLPorts);
        SysFreeString(bstrRuleLAddr);
        SysFreeString(bstrRuleServiceName);
        throw FWException(errorStream.str());
    }

    CurrentProfilesBitMask ^= CurrentProfilesBitMask;
    if (profile == ALL_PROFILE)
    {
        CurrentProfilesBitMask = NET_FW_PROFILE2_PUBLIC | NET_FW_PROFILE2_PRIVATE | NET_FW_PROFILE2_DOMAIN;
    }
    else
    {
        switch (profile)
        {
        case FWManager::DOMAIN_PROFILE:
            CurrentProfilesBitMask = NET_FW_PROFILE2_DOMAIN;
            break;
        case FWManager::PUBLIC_PROFILE:
            CurrentProfilesBitMask = NET_FW_PROFILE2_PUBLIC;
            break;
        case FWManager::PRIVATE_PROFILE:
            CurrentProfilesBitMask = NET_FW_PROFILE2_PRIVATE;
            break;
        default:
            break;
        }
    }
    

    // Create a new Firewall Rule object.
    INetFwRule *pFwRule = NULL;
    m_hr = CoCreateInstance(
        __uuidof(NetFwRule),
        NULL,
        CLSCTX_INPROC_SERVER,
        __uuidof(INetFwRule),
        reinterpret_cast<void**>(&pFwRule));
    if (FAILED(m_hr))
    {
        std::ostringstream errorStream;
        errorStream << "CoCreateInstance for Firewall Rule failed in addRule: 0x" << std::hex << m_hr << std::endl;
        if (pFwRules != NULL)   pFwRules->Release();
        SysFreeString(bstrRuleName);
        SysFreeString(bstrRuleDescription);
        SysFreeString(bstrRuleGroup);
        SysFreeString(bstrRuleApplication);
        SysFreeString(bstrRuleLPorts);
        SysFreeString(bstrRuleLAddr);
        SysFreeString(bstrRuleServiceName);
        throw FWException(errorStream.str());
     }

    // Populate the Firewall Rule object
    pFwRule->put_Name(bstrRuleName);
    pFwRule->put_Description(bstrRuleDescription);
    
    if (!rule.appname.empty()) pFwRule->put_ApplicationName(bstrRuleApplication);
    if (!rule.servicename.empty()) pFwRule->put_ServiceName(bstrRuleServiceName);
    if (rule.ip_protocal.compare("TCP") == 0)
    {
        pFwRule->put_Protocol(NET_FW_IP_PROTOCOL_TCP);
    } 
    else if (rule.ip_protocal.compare("UDP") == 0)
    {
        pFwRule->put_Protocol(NET_FW_IP_PROTOCOL_UDP);
    }

    if (!rule.local_addr.empty()) pFwRule->put_LocalAddresses(bstrRuleLAddr);
    
    pFwRule->put_LocalPorts(bstrRuleLPorts);
    

    if (rule.flow_direction.compare("IN") == 0)
    {
        pFwRule->put_Direction(NET_FW_RULE_DIR_IN);
    }
    else if (rule.flow_direction.compare("OUT") == 0)
    {
        pFwRule->put_Direction(NET_FW_RULE_DIR_OUT);
    }
    
    pFwRule->put_Grouping(bstrRuleGroup);
    pFwRule->put_Profiles(CurrentProfilesBitMask);
    
    if (rule.action.compare("ALLOW") == 0)
    {
        pFwRule->put_Action(NET_FW_ACTION_ALLOW);
    }
    else if (rule.action.compare("BLOCK") == 0)
    {
        pFwRule->put_Action(NET_FW_ACTION_BLOCK);
    }

    rule.enabled ? pFwRule->put_Enabled(VARIANT_TRUE) : pFwRule->put_Enabled(VARIANT_FALSE);
    rule.edge_traversal ? pFwRule->put_EdgeTraversal(VARIANT_TRUE) : pFwRule->put_EdgeTraversal(VARIANT_FALSE);

    // Add the Firewall Rule
    m_hr = pFwRules->Add(pFwRule);
    if (FAILED(m_hr))
    {
        std::ostringstream errorStream;
        errorStream << "Firewall Rule Add failed in addRule: 0x" << std::hex << m_hr << std::endl;
        if (pFwRule != NULL)    pFwRule->Release();
        if (pFwRules != NULL)   pFwRules->Release();
        SysFreeString(bstrRuleName);
        SysFreeString(bstrRuleDescription);
        SysFreeString(bstrRuleGroup);
        SysFreeString(bstrRuleApplication);
        SysFreeString(bstrRuleLPorts);
        SysFreeString(bstrRuleLAddr);
        SysFreeString(bstrRuleServiceName);
        throw FWException(errorStream.str());
    }

    if (pFwRule != NULL)    pFwRule->Release();
    if (pFwRules != NULL)   pFwRules->Release();

    SysFreeString(bstrRuleName);
    SysFreeString(bstrRuleDescription);
    SysFreeString(bstrRuleGroup);
    SysFreeString(bstrRuleApplication);
    SysFreeString(bstrRuleLPorts);
    SysFreeString(bstrRuleLAddr);
    SysFreeString(bstrRuleServiceName);
}

void FWManager::deleteRule(const std::string &name)
{
    BSTR bstrRuleName = SysAllocString(std::wstring(name.begin(), name.end()).c_str());

    // Retrieve INetFwRules
    INetFwRules *pFwRules = NULL;
    m_hr = m_pNetFwPolicy2->get_Rules(&pFwRules);
    if (FAILED(m_hr))
    {
        std::ostringstream errorStream;
        errorStream << "get_Rules failed in deleteRule: 0x" << std::hex << m_hr << std::endl;
        SysFreeString(bstrRuleName);
        throw FWException(errorStream.str());
    }

    pFwRules->Remove(bstrRuleName);
    if (pFwRules != NULL) pFwRules->Release();
    SysFreeString(bstrRuleName);

}

HRESULT FWManager::WFCOMInitialize(INetFwPolicy2** ppNetFwPolicy2)
{
    HRESULT hr = S_FALSE;

    hr = CoCreateInstance(
        __uuidof(NetFwPolicy2),
        NULL,
        CLSCTX_INPROC_SERVER,
        __uuidof(INetFwPolicy2),
        (void**)ppNetFwPolicy2);

    return hr;
}

void FWManager::DumpFWRulesInCollection(INetFwRule* FwRule, std::vector<FW_RULE> &rules, const std::string& group)
{
    struct ProfileMapElement
    {
        NET_FW_PROFILE_TYPE2 Id;
        LPCWSTR Name;
    };

    FW_RULE fw_rule;

    ProfileMapElement ProfileMap[3];
    ProfileMap[0].Id = NET_FW_PROFILE2_DOMAIN;
    ProfileMap[0].Name = L"Domain";
    ProfileMap[1].Id = NET_FW_PROFILE2_PRIVATE;
    ProfileMap[1].Name = L"Private";
    ProfileMap[2].Id = NET_FW_PROFILE2_PUBLIC;
    ProfileMap[2].Name = L"Public";

    BSTR bstrVal;
    if (SUCCEEDED(FwRule->get_Name(&bstrVal)) && bstrVal != NULL)
    {
        fw_rule.name.assign(_com_util::ConvertBSTRToString(bstrVal));
    }
    if (SUCCEEDED(FwRule->get_Description(&bstrVal)) && bstrVal != NULL)
    {
        fw_rule.description.assign(_com_util::ConvertBSTRToString(bstrVal));
    }
    if (SUCCEEDED(FwRule->get_ApplicationName(&bstrVal)) && bstrVal != NULL)
    {
        fw_rule.appname.assign(_com_util::ConvertBSTRToString(bstrVal));
    }
    if (SUCCEEDED(FwRule->get_ServiceName(&bstrVal)) && bstrVal != NULL)
    {
        fw_rule.servicename.assign(_com_util::ConvertBSTRToString(bstrVal));
    }

    long lVal = 0;
    if (SUCCEEDED(FwRule->get_Protocol(&lVal)))
    {
        switch (lVal)
        {
        case NET_FW_IP_PROTOCOL_TCP:
            fw_rule.ip_protocal.assign("TCP");
            break;
        case NET_FW_IP_PROTOCOL_UDP:
            fw_rule.ip_protocal.assign("UDP");
            break;
        default:
            fw_rule.ip_protocal.assign("UNKNOWN");
            break;
        }

        if (lVal != NET_FW_IP_VERSION_V4 && lVal != NET_FW_IP_VERSION_V6)
        {
            if (SUCCEEDED(FwRule->get_LocalPorts(&bstrVal)) && bstrVal != NULL)
            {
                fw_rule.local_ports.assign(_com_util::ConvertBSTRToString(bstrVal));
            }

            if (SUCCEEDED(FwRule->get_RemotePorts(&bstrVal)) && bstrVal != NULL)
            {
                fw_rule.remote_ports.assign(_com_util::ConvertBSTRToString(bstrVal));
            }
        }
        else
        {
            if (SUCCEEDED(FwRule->get_IcmpTypesAndCodes(&bstrVal)) && bstrVal != NULL)
            {
                fw_rule.icmp_type_code.assign(_com_util::ConvertBSTRToString(bstrVal));
            }
        }
    }// if get protocal

    if (SUCCEEDED(FwRule->get_LocalAddresses(&bstrVal)) && bstrVal != NULL)
    {
        fw_rule.local_addr.assign(_com_util::ConvertBSTRToString(bstrVal));
    }
    if (SUCCEEDED(FwRule->get_RemoteAddresses(&bstrVal)) && bstrVal != NULL)
    {
        fw_rule.remote_addr.assign(_com_util::ConvertBSTRToString(bstrVal));
    }

    long lProfileBitmask = 0;
    if (SUCCEEDED(FwRule->get_Profiles(&lProfileBitmask)))
    {
        // The returned bitmask can have more than 1 bit set if multiple profiles 
        //   are active or current at the same time

        for (int i = 0; i < 3; i++)
        {
            if (lProfileBitmask & ProfileMap[i].Id)
            {
                std::wstring temp_w(ProfileMap[i].Name);
                fw_rule.profiles[i] = std::string(temp_w.begin(),temp_w.end());
            }
            else
            {
                fw_rule.profiles[i].assign("");
            }
        }
    }// if get profiles

    NET_FW_RULE_DIRECTION fwDirection;
    if (SUCCEEDED(FwRule->get_Direction(&fwDirection)))
    {
        switch (fwDirection)
        {
        case NET_FW_RULE_DIR_IN:
            fw_rule.flow_direction.assign("IN");
            break;
        case NET_FW_RULE_DIR_OUT:
            fw_rule.flow_direction.assign("OUT");
            break;
        default:
            fw_rule.flow_direction.assign("UNKNOWN");
            break;
        }
    }

    NET_FW_ACTION fwAction;
    if (SUCCEEDED(FwRule->get_Action(&fwAction)))
    {
        switch (fwAction)
        {
        case NET_FW_ACTION_BLOCK:
            fw_rule.action.assign("BLOCK");
            break;
        case NET_FW_ACTION_ALLOW:
            fw_rule.action.assign("ALLOW");
            break;
        default:
            fw_rule.action.assign("UNKNOWN");
            break;
        }
    }

    variant_t InterfaceArray;
    variant_t InterfaceString;
    if (SUCCEEDED(FwRule->get_Interfaces(&InterfaceArray)))
    {
        if (InterfaceArray.vt != VT_EMPTY)
        {
            SAFEARRAY    *pSa = NULL;

            pSa = InterfaceArray.parray;

            for (long index = pSa->rgsabound->lLbound; index < (long)pSa->rgsabound->cElements; index++)
            {
                SafeArrayGetElement(pSa, &index, &InterfaceString);
                if (InterfaceString.bstrVal != NULL)
                {
                    fw_rule.interfaces.push_back(std::string(_com_util::ConvertBSTRToString(InterfaceString.bstrVal)));
                }
                
            }
        }
    }

    if (SUCCEEDED(FwRule->get_InterfaceTypes(&bstrVal)) && bstrVal != NULL)
    {
        fw_rule.interface_types.assign(_com_util::ConvertBSTRToString(bstrVal));
    }

    VARIANT_BOOL bEnabled;
    if (SUCCEEDED(FwRule->get_Enabled(&bEnabled)))
    {
        fw_rule.enabled = bEnabled ? true : false;
    }

    if (SUCCEEDED(FwRule->get_Grouping(&bstrVal)) && bstrVal != NULL)
    {
        fw_rule.grouping.assign(_com_util::ConvertBSTRToString(bstrVal));
    }

    if (SUCCEEDED(FwRule->get_EdgeTraversal(&bEnabled)))
    {
        fw_rule.edge_traversal = bEnabled ? true : false;
    }

    //只枚举所属组的规则
    if (!group.empty() && fw_rule.grouping.compare(group) == 0)
    {
        rules.push_back(fw_rule);
    }
    
    if (group.empty())
    {
        rules.push_back(fw_rule);
    }
    
    
   

}