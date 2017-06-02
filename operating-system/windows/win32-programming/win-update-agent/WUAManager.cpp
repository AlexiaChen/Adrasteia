#include "WUAManager.h"

#include <sstream>
#include <ctime>
#include <regex>
#include <unordered_map>
#include <wuapi.h>
#include <comutil.h>

#pragma comment(lib,"Wuguid.lib")

WUAManager::WUAManager():
m_hrComInit(S_FALSE),
m_hr(S_FALSE),
m_pUpdateSession(NULL),
m_pUpdateSession3(NULL)
{
    // Initialize COM.
#ifdef ENABLE_COM_INIT 
    m_hrComInit = CoInitializeEx(
        0,
        COINIT_APARTMENTTHREADED
        );

    if (m_hrComInit != RPC_E_CHANGED_MODE)
    {
        if (FAILED(m_hrComInit))
        {
            std::ostringstream errorStream;
            errorStream << "CoInitializeEx failed: 0x" << std::hex << m_hrComInit << std::endl;
            throw WUAException(errorStream.str());
        }
    }
#endif

    m_hr = CoCreateInstance(
        CLSID_UpdateSession,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IUpdateSession3,
        (LPVOID*)&m_pUpdateSession);

    if (FAILED(m_hr))
    {
        std::ostringstream errorStream;
        errorStream << "CoCreateInstance for IUpdateSession failed : 0x" << std::hex << m_hr << std::endl;
#ifdef ENABLE_COM_INIT 
        if (SUCCEEDED(m_hrComInit)) CoUninitialize();
#endif
        throw WUAException(errorStream.str());
    }

}

WUAManager::~WUAManager()
{
    if (m_pUpdateSession) m_pUpdateSession->Release();
    if (m_pUpdateSession3) m_pUpdateSession3->Release();
    
#ifdef ENABLE_COM_INIT
    // Uninitialize COM.
    if (SUCCEEDED(m_hrComInit))
    {
        CoUninitialize();
    }
#endif
}

void WUAManager::getHistoryUpdates(std::vector<st_history_update> &updates)
{
    IUpdateSearcher* searcher;
    m_hr = m_pUpdateSession->CreateUpdateSearcher(&searcher);
    if (FAILED(m_hr))
    {
        std::ostringstream errorStream;
        errorStream << "CreateUpdateSearcher failed : 0x" << std::hex << m_hr << std::endl;
        throw WUAException(errorStream.str());
    }
    LONG count;
    searcher->GetTotalHistoryCount(&count);
    IUpdateHistoryEntryCollection* pUpdateHisrtoryEntryCollection;
    searcher->QueryHistory(0, count, &pUpdateHisrtoryEntryCollection);
    std::vector<st_history_update> temp_updates;
    iterHistoryCollection(pUpdateHisrtoryEntryCollection, count, temp_updates);

    std::unordered_map<std::string, st_history_update> updatesMap;
    for (const auto& update : temp_updates)
    {
        if (updatesMap.find(update.id) != updatesMap.end())
        {
            st_history_update compara_update = updatesMap[update.id];
            if (compara_update.date < update.date)
            {
                updatesMap[update.id] = update;
            }
        }
        else
        {
            updatesMap[update.id]= update;
        }
    }

    for (const auto& update : updatesMap)
    {
        updates.push_back(update.second);
    }

    pUpdateHisrtoryEntryCollection->Release();
    searcher->Release();
}

void WUAManager::iterHistoryCollection(IUpdateHistoryEntryCollection* pUpdateCollect, int count, std::vector<st_history_update> &updates)
{
    for (int i = 0; i < count; ++i)
    {
        st_history_update update;
        if (getHistoryItem(pUpdateCollect, i, update))
        {
            updates.push_back(update);
        }

    }// for count
}

bool WUAManager::getHistoryItem(IUpdateHistoryEntryCollection* pUpdateCollect, int index, st_history_update &update)
{
    IUpdateHistoryEntry *pHistoryEntry = NULL;
    pUpdateCollect->get_Item(index, &pHistoryEntry);
    if (pHistoryEntry == NULL) return false;
    if (!isInstalledItem(pHistoryEntry))
    {
        pHistoryEntry->Release();
        return false;
    }
    
    std::string id;
    if (!hasValidID(pHistoryEntry, id))
    {
        pHistoryEntry->Release();
        return false;
    }

    BSTR url, description;
    DATE date;

    pHistoryEntry->get_SupportUrl(&url);
    pHistoryEntry->get_Description(&description);
    pHistoryEntry->get_Date(&date);

    update.id.assign(id);
    update.url.assign(_com_util::ConvertBSTRToString(url));
    update.description.assign(_com_util::ConvertBSTRToString(description));

    SYSTEMTIME sys_date;
    VariantTimeToSystemTime(date, &sys_date);
    std::string str_date = std::to_string(sys_date.wYear) +
        std::string("/") + std::to_string(sys_date.wMonth) +
        std::string("/") + std::to_string(sys_date.wDay);

    update.date.assign(str_date);
    
    SysFreeString(url);
    SysFreeString(description);
    pHistoryEntry->Release();
    return true;
}

bool WUAManager::isInstalledItem(IUpdateHistoryEntry* pItem)
{
    UpdateOperation op;
    pItem->get_Operation(&op);
    return op == uoInstallation ? true : false;
}

bool WUAManager::hasValidID(IUpdateHistoryEntry* pItem, std::string& id)
{
    BSTR title;
    pItem->get_Title(&title);
    std::smatch match;
    std::regex exp_id("KB[0-9]{7}");
    std::string temp_title = _com_util::ConvertBSTRToString(title);
    if (std::regex_search(temp_title, match, exp_id))
    {
        id = match[0];
        SysFreeString(title);
        return true;
        
    }
    SysFreeString(title);
    return false;
}

