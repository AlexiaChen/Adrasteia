#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QDebug>
class Utils
{
public:
    Utils();
    ~Utils();
public:
    static void shutDown();
    static void reboot();
    static QString execCmd(const QString& cmd);
    static void execCmd2(const QString& cmd);
    static QString hardDiskUsage();
    static QString processesInf();
    static QString localIP();
    static QString localMAC();
    static QString cpuUsage();
    static QString memUsage();
    static QString lastStartupTime();

};
#endif
