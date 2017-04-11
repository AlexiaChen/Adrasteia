// Win32Project1.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "Win32Project1.h"
// Tetris
#define MAX_LOADSTRING 100

#define BOUND_SIZE  10
#define TETRIS_SIZE 30
#define GAME_X 10
#define GAME_Y 20
#define INFO_X 6
#define INFO_Y GAME_Y

#define MY_TIMER 1
#define DEFAULT_INTERVAL 500 //默认下降速度，500毫秒下降一格

BOOL g_astTetris[][4][4] = 
{
	{ { 1, 1, 1, 1 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ { 1, 1, 1, 0 }, { 1, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ { 1, 1, 1, 0 }, { 0, 0, 1, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ { 1, 1, 1, 0 }, { 0, 1, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ { 1, 1, 0, 0 }, { 1, 1, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ { 0, 1, 1, 0 }, { 1, 1, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ { 1, 1, 0, 0 }, { 0, 1, 1, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } }
};
#define TETRIS_CNT (sizeof(g_astTetris)/sizeof(g_astTetris[0]))

BOOL g_CurTetris[4][4];
BOOL g_NextTetris[4][4];
BOOL g_stGame[GAME_X][GAME_Y]; //记录哪些地方方块

UINT TetrisX; //方块左上角的坐标
UINT TeTrisY;
UINT g_uiInterval;
UINT g_uiScore = 0;

UINT g_uiMySeed = 0xffff;

// 全局变量: 
HINSTANCE hInst;								// 当前实例
TCHAR szTitle[MAX_LOADSTRING];					// 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];			// 主窗口类名

// 此代码模块中包含的函数的前向声明: 
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO:  在此放置代码。
	MSG msg;
	HACCEL hAccelTable;

	// 初始化全局字符串
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_WIN32PROJECT1, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 执行应用程序初始化: 
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WIN32PROJECT1));

	// 主消息循环: 
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  函数:  MyRegisterClass()
//
//  目的:  注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WIN32PROJECT1));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_WIN32PROJECT1);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   函数:  InitInstance(HINSTANCE, int)
//
//   目的:  保存实例句柄并创建主窗口
//
//   注释: 
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // 将实例句柄存储在全局变量中

   hWnd = CreateWindow(szWindowClass, szTitle, WS_MINIMIZEBOX | WS_SYSMENU,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

int GetRandNum(int iMin, int iMax)
{
	std::random_device rd;
	return iMin + rd() % (iMax - iMin);
}

VOID DrawBackGround(HDC hdc)
{
	int x, y;
	HPEN hPen = (HPEN)GetStockObject(NULL_PEN);
	HBRUSH hBrush = (HBRUSH)GetStockObject(GRAY_BRUSH);

	HBRUSH hBrush1 = (HBRUSH)GetStockObject(BLACK_BRUSH);

	Rectangle(hdc, BOUND_SIZE, BOUND_SIZE, 
		BOUND_SIZE + GAME_X * TETRIS_SIZE, 
		BOUND_SIZE + GAME_Y * TETRIS_SIZE);
	SelectObject(hdc, hPen);
	for (x = 0; x < GAME_X; x++)
	{
		for (y = 0; y < GAME_Y; y++)
		{
			if (g_stGame[x][y])
			{
				SelectObject(hdc, hBrush1);
			}
			else
			{				
				SelectObject(hdc, hBrush);
			}
			Rectangle(hdc, BOUND_SIZE + x * TETRIS_SIZE,
				BOUND_SIZE + y * TETRIS_SIZE,
				BOUND_SIZE + (x + 1) * TETRIS_SIZE,
				BOUND_SIZE + (y + 1) * TETRIS_SIZE);
		}
	}
}

VOID DrawInfo(HDC hdc)
{
	int x, y;
	int nStartX, nStartY;
	HPEN hPen = (HPEN)GetStockObject(BLACK_PEN);
	HBRUSH hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
	HBRUSH hBrush1 = (HBRUSH)GetStockObject(GRAY_BRUSH);
	RECT rect;
	TCHAR szBuf[100] = {};

	SelectObject(hdc, hPen);
	SelectObject(hdc, hBrush);
	Rectangle(hdc, 
		BOUND_SIZE * 2 + GAME_X * TETRIS_SIZE, BOUND_SIZE,
		BOUND_SIZE * 2 + (GAME_X+INFO_X) * TETRIS_SIZE,
		BOUND_SIZE + INFO_Y * TETRIS_SIZE);

	for (x = 0; x < 4; x++)
	{
		for (y = 0; y < 4; y++)
		{
			nStartX = BOUND_SIZE * 2 + GAME_X*TETRIS_SIZE + (y + 1)*TETRIS_SIZE;
			nStartY = BOUND_SIZE + (x + 1)*TETRIS_SIZE;
			if (g_NextTetris[x][y])
			{
				SelectObject(hdc, hBrush);
			}
			else
			{
				SelectObject(hdc, hBrush1);
			}
			Rectangle(hdc, nStartX, nStartY, nStartX + TETRIS_SIZE, nStartY + TETRIS_SIZE);
		}
	}

	nStartX = BOUND_SIZE * 2 + GAME_X*TETRIS_SIZE;
	nStartY = BOUND_SIZE;

	rect.left = nStartX + TETRIS_SIZE;
	rect.right = nStartX + TETRIS_SIZE * (INFO_X - 1);
	rect.top = nStartY + TETRIS_SIZE * 6;
	rect.bottom = nStartY + TETRIS_SIZE * 7;
	wsprintf(szBuf, L"得分：%d", g_uiScore);
	DrawText(hdc, szBuf, wcslen(szBuf), &rect, DT_CENTER | DT_VCENTER);


}

VOID DrawTetris(HDC hdc, int nStartX, int nStartY, BOOL bTetris[4][4])
{
	int i, j;
	HPEN hPen = (HPEN)GetStockObject(BLACK_PEN);
	HBRUSH hBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);

	SelectObject(hdc, hPen);
	SelectObject(hdc, hBrush);
	for (i = 0; i < 4; i++)
	{			
		for (j = 0; j < 4; j++)
		{
			//j 是 X 方向的坐标偏移
			if (bTetris[i][j])
			{
				Rectangle(hdc,
					BOUND_SIZE + (nStartX + j) * TETRIS_SIZE,
					BOUND_SIZE + (nStartY + i) * TETRIS_SIZE,
					BOUND_SIZE + (nStartX + j + 1) * TETRIS_SIZE,
					BOUND_SIZE + (nStartY + i + 1) * TETRIS_SIZE);
			}
		}
	}
}

VOID RotateTetris(BOOL bTetris[4][4])
{
	BOOL bNewTetris[4][4] = {};
	int x, y;
	int xPos, yPos;
	BOOL bFlag; //靠近左上角

	//顺时针：从上往下，从左往右
	for (x = 0,xPos = 0; x < 4; x++)
	{
		bFlag = FALSE;
		for (y = 0; y < 4; y++)
		{
			bNewTetris[xPos][y] = bTetris[3 - y][x];
			//bNewTetris[x][y] = bTetris[y][3 - x]; //逆时针
			if (bNewTetris[xPos][y])
			{
				bFlag = TRUE;// 这一行有数据
			}
 		}
		if (bFlag) 
		{
			xPos++;
		}
	}

	memset(bTetris, 0, sizeof(bNewTetris));
	//靠左
	for (y = 0, yPos = 0; y < 4; y++)
	{
		bFlag = FALSE;
		for (x = 0; x < 4; x++)
		{
			bTetris[x][yPos] = bNewTetris[x][y];
			if (bTetris[x][yPos])
			{
				bFlag = TRUE;
			}
		}
		if (bFlag)
		{
			yPos++;
		}
	}

	return;
}

BOOL CheckTetris(int nStartX, int nStartY, BOOL bTetris[4][4], BOOL bGame[GAME_X][GAME_Y])
{
	int x, y;
	
	//碰左墙
	if (nStartX < 0)
	{
		return FALSE;
	}

	for (x = 0; x < 4; x++)
	{
		for (y = 0; y < 4; y++)
		{
			if (bTetris[x][y])
			{
				//碰右墙
				if (nStartX + y >= GAME_X)
				{
					return FALSE;
				}

				//碰下墙
				if (nStartY + x >= GAME_Y)
				{
					return FALSE;
				}

				//碰到已有方块
				if (bGame[nStartX + y][nStartY + x])
				{
					return FALSE;
				}
			}
		}
	}
	return TRUE;
}

VOID RefreshTetris(int nStartX, int nStartY, BOOL bTetris[4][4], BOOL bGame[GAME_X][GAME_Y])
{
	int x, y;
	BOOL bFlag;
	int newY;
	int iFullLine = 0;  //消去慢行的格子，这里记录行数，用于记分

	//合并 bTetris 到 bGame
	for (x = 0; x < 4; x++)
	{
		for (y = 0; y < 4; y++)
		{
			if (bTetris[x][y]) bGame[nStartX + y][nStartY + x] = TRUE;
		}
	}

	for (y = GAME_Y, newY = GAME_Y; y >= 0; y--)
	{
		bFlag = FALSE;
		for (x = 0; x < GAME_X; x++)
		{
			bGame[x][newY] = bGame[x][y];
			if (!bGame[x][y]) //不满格
			{
				bFlag = TRUE; 
			}
		}
		if (bFlag)
		{
			newY--;
		}
		else
		{
			//满格的化，用上一行替换这一行
			iFullLine++;
		}
	}

	if (iFullLine)
	{
		g_uiScore += (iFullLine * 100);
	}

	memcpy(g_CurTetris, g_NextTetris, sizeof(g_CurTetris));	
	memcpy(g_NextTetris, g_astTetris[GetRandNum(0, TETRIS_CNT)], sizeof(g_NextTetris));

	TetrisX = (GAME_X - 4) / 2;
	TeTrisY = 0;
}


VOID InitGame()
{
	int iTmp;

	TetrisX = (GAME_X - 4) / 2;
	TeTrisY = 0;
	g_uiInterval = DEFAULT_INTERVAL;
	g_uiScore = 0;

	iTmp = GetRandNum(0, TETRIS_CNT);
	memcpy(g_CurTetris, g_astTetris[iTmp], sizeof(g_CurTetris));

	iTmp = GetRandNum(0, TETRIS_CNT);
	memcpy(g_NextTetris, g_astTetris[iTmp], sizeof(g_NextTetris));

	memset(g_stGame, 0, sizeof(g_stGame));
}


//
//  函数:  WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:    处理主窗口的消息。
//
//  WM_COMMAND	- 处理应用程序菜单
//  WM_PAINT	- 绘制主窗口
//  WM_DESTROY	- 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	int nWinX, nWinY, nClientX, nClientY;
	RECT rect;
	BOOL bTmpTetris[4][4];

	switch (message)
	{
	case WM_CREATE:
		//获取窗口大小
		GetWindowRect(hWnd, &rect);
		nWinX = rect.right - rect.left;
		nWinY = rect.bottom - rect.top;
		//获取客户区大小
		GetClientRect(hWnd, &rect);
		nClientX = rect.right - rect.left;
		nClientY = rect.bottom - rect.top;

		MoveWindow(hWnd, 0, 0, 
			3 * BOUND_SIZE + (GAME_X + INFO_X)* TETRIS_SIZE + (nWinX - nClientX),
			2 * BOUND_SIZE + GAME_Y * TETRIS_SIZE + (nWinY - nClientY), TRUE);
		InitGame();
		SetTimer(hWnd, MY_TIMER, g_uiInterval, NULL);
		break;
	case WM_TIMER:
		if (CheckTetris(TetrisX, TeTrisY + 1, g_CurTetris, g_stGame))
		{
			TeTrisY++;
		}
		else
		{
			if (TeTrisY == 0)
			{
				KillTimer(hWnd, MY_TIMER);
				MessageBox(NULL, L"失败", L"FAIL", MB_OK);				

			}
			RefreshTetris(TetrisX, TeTrisY, g_CurTetris, g_stGame);	

		}
		
		InvalidateRect(hWnd, NULL, TRUE);
		break;
	case WM_LBUTTONDOWN:
		RotateTetris(g_CurTetris);
		InvalidateRect(hWnd, NULL, TRUE);
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_LEFT: //左方向键
			if (CheckTetris(TetrisX - 1, TeTrisY, g_CurTetris, g_stGame))
			{
				TetrisX--;
				InvalidateRect(hWnd, NULL, TRUE);
			}
			else
			{
				MessageBeep(0);
			}
			break;
		case VK_RIGHT:
			if (CheckTetris(TetrisX + 1, TeTrisY, g_CurTetris, g_stGame))
			{
				TetrisX++;
				InvalidateRect(hWnd, NULL, TRUE);
			}
			break;
		case VK_UP:
			memcpy(bTmpTetris, g_CurTetris, sizeof (bTmpTetris));	
			RotateTetris(bTmpTetris);
			if (CheckTetris(TetrisX, TeTrisY, bTmpTetris, g_stGame))
			{
				memcpy(g_CurTetris, bTmpTetris, sizeof (bTmpTetris));
				InvalidateRect(hWnd, NULL, TRUE);
			}
			break;
		case VK_DOWN:
			while (CheckTetris(TetrisX, TeTrisY + 1, g_CurTetris, g_stGame))
			{
				TeTrisY++;
			}
			RefreshTetris(TetrisX, TeTrisY, g_CurTetris, g_stGame);
			InvalidateRect(hWnd, NULL, TRUE);
			break;
		default:
		break;
		}
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// 分析菜单选择: 
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO:  在此添加任意绘图代码...
		DrawBackGround(hdc);
		DrawInfo(hdc);
		DrawTetris(hdc, TetrisX, TeTrisY, g_CurTetris);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		KillTimer(hWnd, MY_TIMER);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
