// UiLib_Demos.cpp : 定义应用程序的入口点。
//
#include "stdafx.h"
#include "MainWnd.h"
#include "resource.h"
#include <GdiPlus.h>
#pragma comment( lib, "GdiPlus.lib" )
#pragma comment(lib,"./Static_Debug_2010/UiLib_d.lib")
using namespace Gdiplus;
CMainWnd* pFrame;
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int nCmdShow)
{
	CPaintManagerUI::SetInstance(hInstance);
#ifndef _DEBUG
	CPaintManagerUI::SetResourceZip(_T("Res.zip"));
#endif

	GdiplusStartupInput   gdiplusStartupInput;
	ULONG_PTR             gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	HRESULT Hr = ::CoInitialize(NULL);
	if (FAILED(Hr)) return 0;

	if (::LoadLibrary(_T("d3d9.dll")) == NULL)
		::MessageBox(NULL, _T("加载 d3d9.dll 失败，一些特效可能无法显示！"), _T("信息提示"), MB_OK | MB_ICONWARNING);

	pFrame = new CMainWnd();
	if (pFrame == NULL)
		return 0;

	pFrame->Create(NULL, _T("UiLib Demos"), UI_WNDSTYLE_FRAME, 0L, 0, 0, 900, 600);
	pFrame->CenterWindow();

	::ShowWindow(*pFrame, SW_SHOW);

	CPaintManagerUI::MessageLoop();

	::CoUninitialize();
	GdiplusShutdown(gdiplusToken);

	_CrtDumpMemoryLeaks();
	return 0;
}