
#include "GPOManager.h"
#include "FWManager.h"
#include <iostream>

#include <stdio.h>

#include <windows.h>
#include <netfw.h>
#include <Ntsecapi.h>


#pragma comment( lib, "ole32.lib" )
#pragma comment(lib,"Advapi32.lib")

static bool disableAutoSharedServer()
{
    HKEY hSharesKey = NULL;
    char *sRoot = "SYSTEM\\CurrentControlSet\\Services\\LanmanServer\\Parameters";

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, sRoot, 0, KEY_READ | KEY_WRITE | KEY_SET_VALUE, &hSharesKey) != ERROR_SUCCESS)
    {
        return false;
    }

    DWORD value = 0;
    RegSetValueEx(hSharesKey, "AutoShareServer", 0, REG_DWORD, reinterpret_cast<BYTE*>(&value), sizeof(value));
    RegSetValueEx(hSharesKey, "AutoShareWKS ", 0, REG_DWORD, reinterpret_cast<BYTE*>(&value), sizeof(value));

    return true;

}


static void preventAccessControl()
{
    //¹Ø±ÕÄ¬ÈÏ¹²Ïí
    system("net share ipc$ /delete");
    system("net share admin$ /delete");
    system("net share c$ /delete");
    system("net share d$ /delete");
    system("net share e$ / delete");
    system("net share f$ /delete");

    disableAutoSharedServer();
}

static void disableNeedlessServices()
{
    ShellExecute(NULL, "runas", "sc.exe", "stop Alerter", NULL, SW_HIDE);
    ShellExecute(NULL, "runas", "sc.exe", "config Alerter start= disabled", NULL, SW_HIDE);
    
    ShellExecute(NULL, "runas", "sc.exe", "stop Clipbook", NULL, SW_HIDE);
    ShellExecute(NULL, "runas", "sc.exe", "config Clipbook start= disabled", NULL, SW_HIDE);
    
    ShellExecute(NULL, "runas", "sc.exe", "stop Browser", NULL, SW_HIDE);
    ShellExecute(NULL, "runas", "sc.exe", "config Browser start= disabled", NULL, SW_HIDE);

    ShellExecute(NULL, "runas", "sc.exe", "stop Messenger", NULL, SW_HIDE);
    ShellExecute(NULL, "runas", "sc.exe", "config Messenger start= disabled", NULL, SW_HIDE);

    ShellExecute(NULL, "runas", "sc.exe", "stop Remote Registry", NULL, SW_HIDE);
    ShellExecute(NULL, "runas", "sc.exe", "config Remote Registry start= disabled", NULL, SW_HIDE);

    ShellExecute(NULL, "runas", "sc.exe", "stop Routing and Remote Access", NULL, SW_HIDE);
    ShellExecute(NULL, "runas", "sc.exe", "config Routing and Remote Access start= disabled", NULL, SW_HIDE);

    ShellExecute(NULL, "runas", "sc.exe", "stop Simple Mail Trasfer Protocol(SMTP)", NULL, SW_HIDE);
    ShellExecute(NULL, "runas", "sc.exe", "config Simple Mail Trasfer Protocol(SMTP) start= disabled", NULL, SW_HIDE);

    ShellExecute(NULL, "runas", "sc.exe", "stop Simple Network Management Protocol(SNMP) Service", NULL, SW_HIDE);
    ShellExecute(NULL, "runas", "sc.exe", "config Simple Network Management Protocol(SNMP) Service start= disabled", NULL, SW_HIDE);

    ShellExecute(NULL, "runas", "sc.exe", "stop Simple Network Management Protocol(SNMP) Trap", NULL, SW_HIDE);
    ShellExecute(NULL, "runas", "sc.exe", "config Simple Network Management Protocol(SNMP) Trap start= disabled", NULL, SW_HIDE);

    ShellExecute(NULL, "runas", "sc.exe", "stop Telnet", NULL, SW_HIDE);
    ShellExecute(NULL, "runas", "sc.exe", "config Telnet start= disabled", NULL, SW_HIDE);

    ShellExecute(NULL, "runas", "sc.exe", "stop World Wide Web Publishing Service", NULL, SW_HIDE);
    ShellExecute(NULL, "runas", "sc.exe", "config World Wide Web Publishing Service start= disabled", NULL, SW_HIDE);

    ShellExecute(NULL, "runas", "sc.exe", "stop Spooler", NULL, SW_HIDE);
    ShellExecute(NULL, "runas", "sc.exe", "config Spooler start= disabled", NULL, SW_HIDE);

    ShellExecute(NULL, "runas", "sc.exe", "stop Automatic Updates", NULL, SW_HIDE);
    ShellExecute(NULL, "runas", "sc.exe", "config Automatic Updates start= disabled", NULL, SW_HIDE);

    ShellExecute(NULL, "runas", "sc.exe", "stop Terminal Service", NULL, SW_HIDE);
    ShellExecute(NULL, "runas", "sc.exe", "config Terminal Service start= disabled", NULL, SW_HIDE);
}



LSA_HANDLE GetPolicyHandle()
{
    LSA_OBJECT_ATTRIBUTES ObjectAttributes;
    USHORT SystemNameLength;
    LSA_UNICODE_STRING lusSystemName;
    NTSTATUS ntsResult;
    LSA_HANDLE lsahPolicyHandle;

    // Object attributes are reserved, so initialize to zeros.
    ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes));


    // Get a handle to the Policy object.
    ntsResult = LsaOpenPolicy(
        NULL,    //Name of the target system.
        &ObjectAttributes, //Object attributes.
        POLICY_ALL_ACCESS, //Desired access permissions.
        &lsahPolicyHandle  //Receives the policy handle.
        );

    if (ntsResult != 0)
    {
        // An error occurred. Display it as a win32 error code.
        wprintf(L"OpenPolicy returned %lu\n",
            LsaNtStatusToWinError(ntsResult));
        return NULL;
    }
    return lsahPolicyHandle;
}



int main()
{
   
   // preventAccessControl();

   // GetPolicyHandle();

   // disableNeedlessServices();
    
    /* try{
        GPOManager manager;
        manager.applyPolicy(GPOManager::GPOType::USB_DEVICE_REMOVE, false);
    }
    catch (GPOException& e)
    {
        std::cerr << e.what() << std::endl;
        int d;
        std::cin >> d;
        exit(-1);
    }
    
    std::cout << "apply a policy success" << std::endl;

    int d;
    std::cin >> d;*/
   
   /* try{
        FWManager fw_manager;
        fw_manager.enableFirewall(true);
        std::vector<FWManager::FW_RULE> rules;
        fw_manager.enumFirewallRules(rules);

        FWManager::NEW_FW_RULE rule;
        rule.name.assign("Port Rstrict OUT");
        rule.description.assign("Port 139 restrict");
        rule.edge_traversal = false;
        rule.ip_protocal.assign("*");
        rule.flow_direction.assign("OUT");
        rule.action.assign("BLOCK");
        rule.local_ports.assign("139");
        rule.local_addr.assign("*");
        rule.enabled = true;
        fw_manager.addRule(rule, FWManager::ALL_PROFILE, "my_fw_group");

        
        rules.clear();
        fw_manager.enumFirewallRules(rules, "my_fw_group");
        for (auto const & item : rules)
        {
            fw_manager.deleteRule(item.name);
        }

       

    }
    catch (FWException& e)
    {
        std::cerr << e.what() << std::endl;
        exit(-1);
    }

    std::cout << "enable firewall success" << std::endl;*/


   



    
   
    return 0;

}



