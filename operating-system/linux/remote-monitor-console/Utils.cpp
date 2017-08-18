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

QString Utils::execCmd(const QString& cmd)
{
    qDebug() << "execCmd entry"; 
#ifdef __linux__
    FILE *fcmd = popen(cmd.toStdString().c_str(), "r");
#endif
    char buf[4096] = {0};
#ifndef __linux__
    FILE *fcmd = NULL;
#endif
    QString result;
    while (fgets(buf, sizeof(buf), fcmd) != 0) {
        result.append(QString(buf));
    }
#ifdef __linux__
    pclose(fcmd);
#endif
    qDebug() << "execCmd leave";
    return result;
}

QString Utils::hardDiskUsage()
{
    return execCmd("df -h");
}


QString Utils::localIP()
{
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
        qDebug() << "Create socket failed";
        return QString();
    }
    ifc.ifc_len = sizeof(buf);
    ifc.ifc_req = buf;
    if (ioctl(sock_fd, SIOCGIFCONF, (char *)&ifc) < 0)
    {
        qDebug() << "Get a list of interface addresses failed";
        return QString();
    }

    interface_num = ifc.ifc_len / sizeof(struct ifreq);
    qDebug() << "The number of interfaces is " << interface_num;

    while (interface_num--)
    {
        qDebug() << "Net device: " << buf[interface_num].ifr_name;

        QString net_device(buf[interface_num].ifr_name);

        if(net_device.compare("lo",Qt::CaseInsensitive) == 0) continue;

        if (ioctl(sock_fd, SIOCGIFFLAGS, (char *)&buf[interface_num]) < 0)
        {
            qDebug() << "Get the active flag word of the device";
            continue;
        }
        if (buf[interface_num].ifr_flags & IFF_PROMISC)
           qDebug() << "Interface is in promiscuous mode";

        if (buf[interface_num].ifr_flags & IFF_UP)
        {
            qDebug() << "Interface is running";
        }
        else
        {
            qDebug() << "Interface is not running";
            continue;
        }

        if (ioctl(sock_fd, SIOCGIFADDR, (char *)&buf[interface_num]) < 0)
        {
            qDebug() << "Get interface address failed";
            continue;
        }
        QString address;
        addr = inet_ntoa(((struct sockaddr_in*)(&buf[interface_num].ifr_addr))->sin_addr);
        address = QString::fromLocal8Bit(addr);
        close(sock_fd);
        return address;
    }

    close(sock_fd);
#endif

    return QString();
}

QString Utils::localMAC()
{
    int sock_fd;
#ifdef __linux__
    struct ifreq buf[20];
    struct ifconf ifc;
#endif
    int interface_num;
#ifdef __linux__
    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        qDebug() << "Create socket failed";
        return QString();
    }
    ifc.ifc_len = sizeof(buf);
    ifc.ifc_req = buf;
    if (ioctl(sock_fd, SIOCGIFCONF, (char *)&ifc) < 0)
    {
        qDebug() << "Get a list of interface addresses failed";
        return QString();
    }
    interface_num = ifc.ifc_len / sizeof(struct ifreq);
    qDebug() << "The number of interfaces is " << interface_num;

    while (interface_num--)
    {
        qDebug() << "Net device: " << buf[interface_num].ifr_name;

        QString net_device(buf[interface_num].ifr_name);

        if(net_device.compare("lo",Qt::CaseInsensitive) == 0) continue;

        if (ioctl(sock_fd, SIOCGIFFLAGS, (char *)&buf[interface_num]) < 0)
        {
            qDebug() << "Get the active flag word of the device";
            continue;
        }
        if (buf[interface_num].ifr_flags & IFF_PROMISC)
            qDebug() << "Interface is in promiscuous mode";

        if (buf[interface_num].ifr_flags & IFF_UP)
        {
            qDebug() << "Interface is running";
        }
        else
        {
            qDebug() << "Interface is not running";
            continue;
        }

        if (ioctl(sock_fd, SIOCGIFHWADDR, (char *)&buf[interface_num]) < 0)
        {
            qDebug() << "Get the hardware address of a device failed";
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

        QString macAddr = QString::fromLocal8Bit(buffer);
        close(sock_fd);
        return macAddr;
    }

    close(sock_fd);
#endif
    return QString();
}

QString Utils::cpuUsage()
{
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

    return QString::number(cpuUsage);
}

QString Utils::lastStartupTime()
{
    return execCmd("who -b");
}

QString Utils::memUsage()
{
    memOccupy memstat;
    int memUsage;
    memset(&memstat, 0, sizeof(memOccupy));

    getMemOccupy(&memstat);
    if (memstat.total != 0)
        memUsage = ((memstat.total - memstat.free) * 100) / memstat.total;
    else
        memUsage = 0;


    return QString::number(memUsage);
}

QString Utils::processesInf()
{
    return execCmd("ps");
}
