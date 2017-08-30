#ifndef UTILS_H
#define UTILS_H

//#include <QString>
//#include <QDebug>
#include <string>

class Utils
{
public:
    Utils();
    ~Utils();
public:
    static void shutDown();
    static void reboot();
    static std::string execCmd(const std::string& cmd);
    static void execCmd2(const std::string& cmd);
    static std::string hardDiskUsage();
    static std::string processesInf();
    static std::string localIP();
    static std::string localMAC();
    static std::string cpuUsage();
    static std::string memUsage();
    static std::string lastStartupTime();

};
#endif
