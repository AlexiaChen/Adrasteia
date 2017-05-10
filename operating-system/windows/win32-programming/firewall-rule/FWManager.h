#ifndef FW_MANAGER_H
#define FW_MANAGER_H

#include <exception>
#include <string>
#include <vector>
#include <array>

#include <windows.h>

struct INetFwPolicy2;
struct INetFwRule;

class FWException : public std::exception
{
public:
    explicit FWException(const char* msg) : m_msg(msg){}
    explicit FWException(const std::string& msg) :m_msg(msg.c_str()){}
    virtual ~FWException() throw () {};

    virtual const char* what() const throw () { return m_msg.c_str(); }
private:
    std::string m_msg;
};

class FWManager
{
public:
    FWManager();
    ~FWManager();

    typedef struct _FW_RULE{
        std::string name;
        std::string description;
        std::string appname;
        std::string servicename;
        std::string ip_protocal;
        std::string local_ports;
        std::string remote_ports;
        std::string icmp_type_code;
        std::string local_addr;
        std::string remote_addr;
        std::array<std::string, 3> profiles;
        std::string flow_direction;
        std::string action;
        std::vector<std::string> interfaces;
        std::string interface_types;
        bool enabled;
        std::string grouping;
        bool edge_traversal;
    }FW_RULE;

    typedef struct _NEW_RULE{
        std::string name;
        std::string description;
        std::string appname;
        std::string servicename;
        std::string ip_protocal;
        std::string local_ports;
        std::string remote_ports;
        std::string local_addr;
        std::string remote_addr;
        std::string flow_direction;
        std::string action;
        bool edge_traversal;
        bool enabled;
    }NEW_FW_RULE;

    enum PROFILE{
        DOMAIN_PROFILE,
        PUBLIC_PROFILE,
        PRIVATE_PROFILE,
        ALL_PROFILE
    };

    void enableFirewall(bool enable);
    bool isFirewallEnabled();
    void enumFirewallRules(std::vector<FW_RULE> &rules, const std::string group="");
    void enbaleRuleGroup(const std::string& group, PROFILE profile, bool enable);
    bool isRuleGroupEnable(const std::string& group, PROFILE profile);
    //************************************
    // Method:    addRule
    // Returns:   void
    // Qualifier: 添加新的规则
    // Parameter: const NEW_FW_RULE & rule  要添加的规则描述结构体
    // Parameter: PROFILE profile   该参数暂时保留
    // Parameter: const std::string & to_group  规则添加到的目标组
    //************************************
    void addRule(const NEW_FW_RULE& rule, PROFILE profile, const std::string& to_group);
    //************************************
    // Method:    deleteRule
    // Returns:   void
    // Qualifier:  删除规则
    // Parameter: const std::string & name  规则名称
    //************************************
    void deleteRule(const std::string &name);
   
private:
    HRESULT m_hrComInit;
    HRESULT m_hr;
    INetFwPolicy2 *m_pNetFwPolicy2;
private:
    // Forward declarations
    HRESULT  WFCOMInitialize(INetFwPolicy2** ppNetFwPolicy2);
    void DumpFWRulesInCollection(INetFwRule* FwRule, std::vector<FW_RULE> &rules, const std::string& group);
};

#endif