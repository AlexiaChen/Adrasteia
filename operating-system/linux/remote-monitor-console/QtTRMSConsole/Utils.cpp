#include "Utils.h"

#ifdef __linux__
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#endif

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string.h>

#include "Public.h"

typedef struct _cpuOccupy {
    char name[20];
    unsigned int user;
    unsigned int nice;
    unsigned int system;
    unsigned int idle;
    unsigned int iowait;
    unsigned int irq;
    unsigned int softirq;
} cpuOccupy;

typedef struct _memOccupy {
    char name[20];
    unsigned long total;
    unsigned long free;
} memOccupy;

static void getMemOccupy(memOccupy *memst) {
    FILE *fp;
    char buf[256];
    memOccupy *memOcc;
    memOcc = memst;

    fp = fopen("/proc/meminfo", "r");
    if (!fp) {
        fprintf(stderr, "open /proc/meminfo!\n");
        return;
    }
    fgets(buf, sizeof(buf), fp);
    sscanf(buf, "%s %lu %s", memOcc->name, &memOcc->total, memOcc->name);
    fgets(buf, sizeof(buf), fp);
    sscanf(buf, "%s %lu %s", memOcc->name, &memOcc->free, memOcc->name);

    fclose(fp);
}

static void getCpuOccupy(cpuOccupy *cpust) {
    FILE *fp;
    char buf[256];
    cpuOccupy *cpuOcc;
    cpuOcc = cpust;

    fp = fopen("/proc/stat", "r");
    if (!fp) {
        fprintf(stderr, "open /proc/stat error!\n");
        return;
    }
    fgets(buf, sizeof(buf), fp);
    sscanf(buf, "%s %u %u %u %u %u %u %u", cpuOcc->name, &cpuOcc->user, &cpuOcc->nice, &cpuOcc->system, &cpuOcc->idle, &cpuOcc->iowait
        , &cpuOcc->irq, &cpuOcc->softirq);
    fclose(fp);
}

static int calCpuOccupy(cpuOccupy *c1, cpuOccupy *c2) {
    unsigned long total1, total2, id, sd, xd;
    int cpuUsage = 0;

    total1 = c1->user + c1->nice + c1->system + c1->idle + c1->iowait + c1->irq + c1->softirq;
    total2 = c2->user + c2->nice + c2->system + c2->idle + c2->iowait + c2->irq + c2->softirq;

    id = c2->user - c1->user;
    sd = c2->system - c1->system;
    xd = c2->nice - c1->nice;

    if ((total2 - total1) != 0)
        cpuUsage = ((sd + id + xd) * 100) / (total2 - total1);
    else
        cpuUsage = 0;
    return cpuUsage;
}


Utils::Utils()
{
}


Utils::~Utils()
{
}

void Utils::shutDown()
{
#ifdef __linux__
    system("shutdown -P now");
#endif
}

void Utils::reboot()
{
#ifdef __linux__
    system("shutdown -r now");
#endif
}

std::string Utils::execCmd(const std::string& cmd)
{
#ifdef __linux__
    FILE *fcmd = popen(cmd.c_str(), "r");
#endif
    char buf[1024*5] = {0};
#ifndef __linux__
    FILE *fcmd = NULL;
#endif
    std::string result;
    while (fgets(buf, sizeof(buf), fcmd) != 0) {
        result.append(buf);
    }
#ifdef __linux__
    pclose(fcmd);
#endif
    
    return result;
}

std::string Utils::hardDiskUsage()
{
#ifdef WIN32
    return "60";
#endif

#ifdef __linux__
    return execCmd("df -h");
#endif
}


std::string Utils::localIP()
{
#ifdef WIN32
    return "192.168.3.11";
#endif

    int sock_fd;
#ifdef __linux__
    struct ifreq buf[20];
    struct ifconf ifc;
#endif
    int interface_num;
    char *addr;//[ADDR_LEN];
#ifdef __linux__
    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        //std::cout << "Create socket failed" << std::endl;
        return std::string();
    }
    ifc.ifc_len = sizeof(buf);
    ifc.ifc_req = buf;
    if (ioctl(sock_fd, SIOCGIFCONF, (char *)&ifc) < 0)
    {
        //std::cout << "Get a list of interface addresses failed" << std::endl;
        close(sock_fd);
        return std::string();
    }

    interface_num = ifc.ifc_len / sizeof(struct ifreq);
    //std::cout << "The number of interfaces is " << interface_num <<std::endl;

    while (interface_num--)
    {
        //std::cout << "Net device: " << buf[interface_num].ifr_name <<std::endl;

        std::string net_device(buf[interface_num].ifr_name);

        
        if(net_device.find("e") == std::string::npos)
        {
            continue;
        }

        if (ioctl(sock_fd, SIOCGIFFLAGS, (char *)&buf[interface_num]) < 0)
        {
            //std::cout << "Get the active flag word of the device" << std::endl;
            continue;
        }
        if (buf[interface_num].ifr_flags & IFF_PROMISC)
           //std::cout << "Interface is in promiscuous mode" << std::endl;

        if (buf[interface_num].ifr_flags & IFF_UP)
        {
            //std::cout << "Interface is running" << std::endl;
        }
        else
        {
            //std::cout << "Interface is not running" << std::endl;
            continue;
        }

        if (ioctl(sock_fd, SIOCGIFADDR, (char *)&buf[interface_num]) < 0)
        {
            //std::cout << "Get interface address failed" << std::endl;
            continue;
        }
       
        //QString address;
        addr = inet_ntoa(((struct sockaddr_in*)(&buf[interface_num].ifr_addr))->sin_addr);
        std::string address(addr);
        close(sock_fd);
        return address;
    }

    close(sock_fd);
#endif

    return std::string();
}

std::string Utils::localMAC()
{
#ifdef WIN32
    return "12:23:A8:E6";
#endif
    int sock_fd;
#ifdef __linux__
    struct ifreq buf[20];
    struct ifconf ifc;
#endif
    int interface_num;
#ifdef __linux__
    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        //std::cout << "Create socket failed" << std::endl;
        return std::string();
    }
    ifc.ifc_len = sizeof(buf);
    ifc.ifc_req = buf;
    if (ioctl(sock_fd, SIOCGIFCONF, (char *)&ifc) < 0)
    {
       // std::cout << "Get a list of interface addresses failed" << std::endl;
        close(sock_fd);
        return std::string();
    }
    interface_num = ifc.ifc_len / sizeof(struct ifreq);
    //std::cout << "The number of interfaces is " << interface_num <<std::endl;

    while (interface_num--)
    {
       // std::cout << "Net device: " << buf[interface_num].ifr_name <<std::endl;

        std::string net_device(buf[interface_num].ifr_name);

        if(net_device.find("e") == std::string::npos)
        {
            continue;
        }

        if (ioctl(sock_fd, SIOCGIFFLAGS, (char *)&buf[interface_num]) < 0)
        {
            //std::cout << "Get the active flag word of the device" << std::endl;
            continue;
        }
        if (buf[interface_num].ifr_flags & IFF_PROMISC)
            //std::cout << "Interface is in promiscuous mode" << std::endl;

        if (buf[interface_num].ifr_flags & IFF_UP)
        {
            //std::cout << "Interface is running" << std::endl;
        }
        else
        {
            //std::cout << "Interface is not running" << std::endl;
            continue;
        }

        if (ioctl(sock_fd, SIOCGIFHWADDR, (char *)&buf[interface_num]) < 0)
        {
            //std::cout << "Get the hardware address of a device failed" <<std::endl;
            continue;
        }

        char buffer[256] = {0};
        
        sprintf(buffer,"%02X:%02X:%02X:%02X:%02X:%02X",
            (unsigned char)buf[interface_num].ifr_hwaddr.sa_data[0],
            (unsigned char)buf[interface_num].ifr_hwaddr.sa_data[1],
            (unsigned char)buf[interface_num].ifr_hwaddr.sa_data[2],
            (unsigned char)buf[interface_num].ifr_hwaddr.sa_data[3],
            (unsigned char)buf[interface_num].ifr_hwaddr.sa_data[4],
            (unsigned char)buf[interface_num].ifr_hwaddr.sa_data[5]);

        std::string macAddr(buffer);
        close(sock_fd);
        return macAddr;
    }

    close(sock_fd);
#endif
    return std::string();
}

std::string Utils::cpuUsage()
{
#ifdef WIN32
    return "30";
#endif

    cpuOccupy cpuStat1;
    cpuOccupy cpuStat2;
    int cpuUsage;

    memset(&cpuStat1, 0, sizeof(cpuOccupy));
    memset(&cpuStat2, 0, sizeof(cpuOccupy));

    getCpuOccupy(&cpuStat1);
#ifdef __linux__
    sleep(3);
#endif
    getCpuOccupy(&cpuStat2);
    cpuUsage = calCpuOccupy(&cpuStat1, &cpuStat2);

    char numStr[25] = {0};
    sprintf(numStr, "%d", cpuUsage);
    return std::string(numStr);
}

static std::string deleteChar(std::string& str)
{
    std::string ret;
    for (int i = 0; i < str.size(); ++i)
    {
        if (str[i] > 'a' && str[i] < 'z')
        {
           continue;
        }
        else if (str[i] > 'A' && str[i] < 'Z')
        {
            continue;
        }
        else
        {
            ret.push_back(str[i]);
        }
    }
    return ret;
}

std::string Utils::lastStartupTime()
{
#ifdef WIN32
    return "2017-8-16 Thu 14:36";
#endif

#ifdef __linux__
    std::string cmdRet = execCmd("who -b");
    std::string sOutput = deleteChar(cmdRet);
    return sOutput;
#endif
}

std::string Utils::memUsage()
{
#ifdef WIN32
    return "70";
#endif
    memOccupy memstat;
    int memUsage;
    memset(&memstat, 0, sizeof(memOccupy));

    getMemOccupy(&memstat);
    if (memstat.total != 0)
        memUsage = ((memstat.total - memstat.free) * 100) / memstat.total;
    else
        memUsage = 0;


    char numStr[25] = { 0 };
    sprintf(numStr, "%d", memUsage);
    return std::string(numStr);
}

std::string Utils::processesInf()
{
#ifdef WIN32
    return "/usr/local/bin/taste /usr/lcoal/bin/mem_cast";
#endif
#ifdef __linux__
    return execCmd("ps");
#endif
}

void Utils::execCmd2(const std::string& cmd)
{
   

    std::ofstream outBashFile("tmp_bash.sh");

    if (!outBashFile)
    {
        //std::cout << "Bash File open() fail" << std::endl;
        return;
    }

    outBashFile << "#!/usr/bin/env bash" << std::endl;
    outBashFile << cmd;
    
    outBashFile.close();

    char chmodStr[1024] = { 0 };

    sprintf(chmodStr, "chmod +x %s", "tmp_bash.sh");
    execCmd(std::string(chmodStr));

    char tmp_bash_path[1024] = { 0 };
    char tmp_bash_dir[256] = { 0 };
    strcpy(tmp_bash_dir, execCmd("pwd").c_str());
    CutBothEndsSpace(tmp_bash_dir);
   // std::cout << "Cut Space tmp bash dir" << tmp_bash_dir << std::endl;
    sprintf(tmp_bash_path, "%s/tmp_bash.sh", tmp_bash_dir);

    system(tmp_bash_path);

}
