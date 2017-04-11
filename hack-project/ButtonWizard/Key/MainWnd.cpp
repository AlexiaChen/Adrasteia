#include "StdAfx.h"
#include "MainWnd.h"

CMainWnd::CMainWnd(void)
{
} 


CMainWnd::~CMainWnd(void)
{
}
void CMainWnd::OnHotKey(WPARAM wParam)
{
	if (wParam == HotKeyMousePointID)//判定一下，是否F6的热键ID
	{
		HDC displayDC = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
		POINT screenPoint;
		GetCursorPos(&screenPoint);
		DWORD colorRef = GetPixel(displayDC, screenPoint.x, screenPoint.y);
		DeleteDC(displayDC);
		CDuiString tt;
		tt.Format(_T("color - %d     Red:%d  -  Green:%d  -  Blue:%d"), colorRef, colorRef & 0xff, (colorRef >> 8) &0xff, (colorRef >> 16) &0xff);
		MessageBox(this->GetHWND(), tt, _T("Color"), MB_OK);

		POINT curPos;
		if (GetCursorPos(&curPos))
		{
			CDuiString tmp;
			tmp.Format(_T("%d"), curPos.x);
			pX->SetText(tmp);
			tmp.Format(_T("%d"), curPos.y);
			pY->SetText(tmp);
		}
		else
			MessageBox(this->GetHWND(), _T("获取当前鼠标位置失败"), _T("Info"), MB_OK);
	}
	else if (wParam == HotKeyExecID)//F8,执行所有的命令序列
	{
		list<pMyCmd>::iterator iter = m_CmdList.begin();
		for (; iter != m_CmdList.end(); iter++)
		{
			pMyCmd tmp = *iter;
			switch (tmp->cmdHead)
			{
			case cmdKey:
				keybd_event(tmp->cmdContent.key, 0, 0, 0);
				keybd_event(tmp->cmdContent.key, 0, KEYEVENTF_KEYUP, 0);
				break;
			case cmdLButton:
				SetCursorPos(tmp->cmdContent.mousePoint.x, tmp->cmdContent.mousePoint.y);
				mouse_event(MOUSEEVENTF_LEFTDOWN, tmp->cmdContent.mousePoint.x, tmp->cmdContent.mousePoint.y, 0, 0);
				mouse_event(MOUSEEVENTF_LEFTUP, tmp->cmdContent.mousePoint.x, tmp->cmdContent.mousePoint.y, 0, 0);
				break;
			case cmdRButton:
				SetCursorPos(tmp->cmdContent.mousePoint.x, tmp->cmdContent.mousePoint.y);
				mouse_event(MOUSEEVENTF_RIGHTDOWN, tmp->cmdContent.mousePoint.x, tmp->cmdContent.mousePoint.y, 0, 0);
				mouse_event(MOUSEEVENTF_RIGHTUP, tmp->cmdContent.mousePoint.x, tmp->cmdContent.mousePoint.y, 0, 0);
				break;
			default:
				break;
			}
			Sleep(10);
		}
	}

	return;
}
LRESULT CMainWnd::HandleMessage( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	try
	{
		if (uMsg == WM_NCLBUTTONDBLCLK)
			return 1;
		else if (uMsg == WM_CLOSE)
		{
			OnCloseBtnClick(NULL, 0, 0);
		}
		else if (uMsg == WM_HOTKEY)
		{
			OnHotKey(wParam);//wParam附带有hotkey的ID
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

		pCleanBtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("CLEANBTN")));
		pCleanBtn->OnNotify += MakeDelegate(this, &CMainWnd::OnCleanBtnClick, _T("click"));

		pLButtonBtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("LBUTTON")));
		pLButtonBtn->OnNotify += MakeDelegate(this, &CMainWnd::OnLButtonBtnClick, _T("click"));

		pRButtonBtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("RBUTTON")));
		pRButtonBtn->OnNotify += MakeDelegate(this, &CMainWnd::OnRButtonBtnClick, _T("click"));

		pKeyBtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("KEYBUTTON")));
		pKeyBtn->OnNotify += MakeDelegate(this, &CMainWnd::OnKeyBtnClick, _T("click"));

		pExecuteBtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("EXECUTEBUTTON")));
		pExecuteBtn->OnNotify += MakeDelegate(this, &CMainWnd::OnExecuteBtnClick, _T("click"));

		pTips = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("Tips")));
		
		pKey = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("KEY")));
		pX = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("POINTX")));
		pY = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("POINTY")));
		pCmd = static_cast<CRichEditUI*>(m_PaintManager.FindControl(_T("CMDLIST")));
		
		HotKeyMousePointID = GlobalAddAtom(_T("ZS GetMousePoint")) - 0xC00;
		if (!RegisterHotKey(this->GetHWND(), HotKeyMousePointID, 0, VK_F6))
			::MessageBox(this->GetHWND(), _T("注册系统热键F6失败"), _T("Error"), MB_OK);
		HotKeyExecID = GlobalAddAtom(_T("ZS ExeCommand")) - 0xC00;
		if (!RegisterHotKey(this->GetHWND(), HotKeyExecID, 0, VK_F8))
			::MessageBox(this->GetHWND(), _T("注册系统热键F8失败"), _T("Error"), MB_OK);
	}
	catch (...)
	{
		throw "CMainWnd::Init";
	}
}

void CMainWnd::Notify( TNotifyUI& msg )
{
	WindowImplBase::Notify(msg);
}

bool CMainWnd::OnCloseBtnClick(TNotifyUI* pTNotifyUI, LPARAM lParam, WPARAM wParam)
{
	UnregisterHotKey(this->GetHWND(), HotKeyMousePointID);
	UnregisterHotKey(this->GetHWND(), HotKeyExecID);
	PostQuitMessage(0);
	return true;
}
//按钮 - 清除
bool CMainWnd::OnCleanBtnClick(TNotifyUI* pTNotifyUI, LPARAM lParam, WPARAM wParam)
{
	m_CmdList.clear();
	pCmd->SetText(_T(""));
	CDuiString tips;
	tips.Format(_T("请按F6进行鼠标位置捕获，或按F8进行指令集运行"));
	pTips->SetText(tips);
	return true;
}
//按钮 - 插入鼠标左键消息 
bool CMainWnd::OnLButtonBtnClick(TNotifyUI* pTNotifyUI, LPARAM lParam, WPARAM wParam)
{
	int x = _wtoi(pX->GetText());
	int y = _wtoi(pY->GetText());
	if (x && y)
	{
		pMyCmd tmp = new MyCmd();
		tmp->cmdHead = cmdLButton;
		tmp->cmdContent.mousePoint.x = x;
		tmp->cmdContent.mousePoint.y = y;
		m_CmdList.push_back(tmp);
		UpdateCmdListUI();
	}
	else
		MessageBox(this->GetHWND(), _T("请先设置鼠标坐标"), _T("Info"), MB_OK);
	return true;
}
//鼠标右键消息 
bool CMainWnd::OnRButtonBtnClick(TNotifyUI* pTNotifyUI, LPARAM lParam, WPARAM wParam)
{
	int x = _wtoi(pX->GetText());
	int y = _wtoi(pY->GetText());
	if (x && y)
	{
		pMyCmd tmp = new MyCmd();
		tmp->cmdHead = cmdRButton;
		tmp->cmdContent.mousePoint.x = x;
		tmp->cmdContent.mousePoint.y = y;
		m_CmdList.push_back(tmp);
		UpdateCmdListUI();
	}
	else
		MessageBox(this->GetHWND(), _T("请先设置鼠标坐标"), _T("Info"), MB_OK);
	return true;
}

bool CMainWnd::OnKeyBtnClick(TNotifyUI* pTNotifyUI, LPARAM lParam, WPARAM wParam)
{
	
	CDuiString sTmp = pKey->GetText();
	sTmp.MakeUpper();
	WCHAR vk = sTmp.GetAt(0);
	if (vk)
	{
		pMyCmd tmp = new MyCmd();
		tmp->cmdHead = cmdKey; 
		tmp->cmdContent.key = vk;
		m_CmdList.push_back(tmp);
		UpdateCmdListUI();
	}
	else
		MessageBox(this->GetHWND(), _T("请先设置鼠标坐标"), _T("Info"), MB_OK);
	return true;
}

bool CMainWnd::OnExecuteBtnClick(TNotifyUI* pTNotifyUI, LPARAM lParam, WPARAM wParam)
{
	OnHotKey(HotKeyExecID);
	return true;
}

void CMainWnd::UpdateCmdListUI()
{
	CDuiString tips;
	tips.Format(_T("当前已经存储 %d 条指令"), m_CmdList.size());
	pTips->SetText(tips);

	CDuiString cmdUI;
	list<pMyCmd>::iterator iter = m_CmdList.begin();
	for (; iter != m_CmdList.end();iter++)
	{
		pMyCmd tmp = *iter;
		CDuiString csTmp;
		switch (tmp->cmdHead)
		{
		case cmdKey:
			csTmp.Format(_T("按键 - %c\r\n"), tmp->cmdContent.key);
			break;
		case cmdLButton:
			csTmp.Format(_T("在坐标X: %d Y:%d 点击鼠标左键\r\n"), tmp->cmdContent.mousePoint.x, tmp->cmdContent.mousePoint.y);
			break;
		case cmdRButton:
			csTmp.Format(_T("在坐标X: %d Y:%d 点击鼠标右键\r\n"), tmp->cmdContent.mousePoint.x, tmp->cmdContent.mousePoint.y);
			break;
		default:
			break;
		}
		cmdUI += csTmp;
	}
	pCmd->SetText(cmdUI);
	return;
}