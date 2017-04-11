#ifndef MainWnd_h__
#define MainWnd_h__

#include <map>
#include <ShellAPI.h>
#include   <time.h>
#include <list>
#include <Dbt.h>
//#include <direct.h>
#include "Utils\WinImplBase.h"

using namespace std;

typedef enum{
	cmdKey,cmdLButton,cmdRButton
}CmdHead;
typedef union{
	POINT mousePoint;
	TCHAR key;
}CmdContent;
typedef struct{
	CmdHead cmdHead;
	CmdContent cmdContent;
}MyCmd,*pMyCmd;
#pragma once
class CMainWnd : public WindowImplBase
{
public:
	CMainWnd(void);
	~CMainWnd(void);

public:
	CDuiString GetSkinFolder(){ return _T(""); };
	CDuiString GetSkinFile(){ return CDuiString(_T("MainSkin.xml")); };
	LPCTSTR GetWindowClassName() const{return _T("QQ");};
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void InitWindow();
	void Notify(TNotifyUI& msg);

	
private:
	CButtonUI * pCloseBtn;
	CButtonUI * pCleanBtn;
	CButtonUI * pLButtonBtn;
	CButtonUI * pRButtonBtn;
	CButtonUI * pKeyBtn;
	CButtonUI * pExecuteBtn;
	CEditUI *	pX;
	CEditUI *	pY;
	CEditUI *	pKey;
	CRichEditUI *	pCmd;
	CLabelUI *	pTips;
private:
	bool OnCleanBtnClick(TNotifyUI* pTNotifyUI, LPARAM lParam, WPARAM wParam);
	bool OnLButtonBtnClick(TNotifyUI* pTNotifyUI, LPARAM lParam, WPARAM wParam);
	bool OnRButtonBtnClick(TNotifyUI* pTNotifyUI, LPARAM lParam, WPARAM wParam);
	bool OnKeyBtnClick(TNotifyUI* pTNotifyUI, LPARAM lParam, WPARAM wParam);
	bool OnExecuteBtnClick(TNotifyUI* pTNotifyUI, LPARAM lParam, WPARAM wParam);
	bool OnCloseBtnClick(TNotifyUI* pTNotifyUI, LPARAM lParam, WPARAM wParam);
	void OnHotKey(WPARAM wParam);
	void UpdateCmdListUI();
private:
	WORD	HotKeyMousePointID;
	WORD	HotKeyExecID;
	list<pMyCmd> m_CmdList;
};

#endif // MainWnd_h__
