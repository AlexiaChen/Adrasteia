#ifndef WUA_MANAGER_H
#define WUA_MANAGER_H

#include <exception>
#include <string>
#include <vector>
#include <windows.h>

struct IUpdateSession;
struct IUpdateSession3;
struct IUpdateHistoryEntryCollection;
struct IUpdateHistoryEntry;

class WUAException : public std::exception 
{
public:
    explicit WUAException(const char* msg) : m_msg(msg){}
    explicit WUAException(const std::string& msg) :m_msg(msg.c_str()){}
    virtual ~WUAException() throw () {};

    virtual const char* what() const throw () { return m_msg.c_str(); }
private:
    std::string m_msg;
};

class WUAManager
{
public:
    WUAManager();
    ~WUAManager();
public:
    typedef struct
    {
        std::string id;
        std::string url;
        std::string description;
        std::string date;
    }st_history_update;

    void getHistoryUpdates(std::vector<st_history_update> &updates);
private:
    void iterHistoryCollection(IUpdateHistoryEntryCollection* pUpdateCollect, int count, std::vector<st_history_update> &updates);
    bool getHistoryItem(IUpdateHistoryEntryCollection* pUpdateCollect, int index, st_history_update &update);
    bool isInstalledItem(IUpdateHistoryEntry* pItem);
    bool hasValidID(IUpdateHistoryEntry* pItem,std::string& id);
private:
    HRESULT m_hrComInit;
    HRESULT m_hr;
    IUpdateSession *m_pUpdateSession;
    IUpdateSession3* m_pUpdateSession3;
};
#endif // !WUA_MANAGER_H
