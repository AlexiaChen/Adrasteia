// ClsMine.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "ClsMine.h"
#include<random>

#define MAX_LOADSTRING 100

#define MINE_SIZE 30  //每个雷区占用的大小
#define DIV_SIZE  9   //雷区的范围 DIV_SIZE * DIV_SIZE

#define DIV_SIZE_MAX 50 //雷区的最大范围

#define MINE_NUM  10  //雷区里面雷的数量

int g_iDivSize = DIV_SIZE;
int g_iMineNum = MINE_NUM;

int g_iResDiv;

BOOL g_arrMine[DIV_SIZE_MAX][DIV_SIZE_MAX] = { 0 }; //各个位置是否有雷
int g_arrRoundMine[DIV_SIZE_MAX][DIV_SIZE_MAX] = { 0 };//各个位置周围有多少雷
//0 表示没有被点击过， 1 表示左键点击， 2 表示右键单击
int g_arrDivState[DIV_SIZE_MAX][DIV_SIZE_MAX] = { 0 };

// 全局变量: 
HINSTANCE hInst;								// 当前实例
TCHAR szTitle[MAX_LOADSTRING];					// 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];			// 主窗口类名

// 此代码模块中包含的函数的前向声明: 
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
VOID  InitMineArea();

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
	LoadString(hInstance, IDC_CLSMINE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 执行应用程序初始化: 
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CLSMINE));

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
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CLSMINE));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_CLSMINE);
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

   hWnd = CreateWindow(szWindowClass, szTitle, WS_SYSMENU,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
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
	RECT rect;
	int nWinX, nWinY; //窗口大小
	int nClientX, nClientY; //客户区大小
	int nScreenX, nScreenY;
	int x, y;
	static int xPos, yPos;  //存储鼠标按下的位置
	static BOOL bMouse = FALSE;
	static BOOL bResult = FALSE;
	TCHAR szBuf[4] = {0};

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

		//获取屏幕的长度宽度
		nScreenX = GetSystemMetrics(SM_CXSCREEN); 
		nScreenY = GetSystemMetrics(SM_CYSCREEN);

		//窗口居中
		x = (nScreenX - MINE_SIZE*g_iDivSize - nWinX + nClientX) / 2;
		y = (nScreenY - MINE_SIZE*g_iDivSize - nWinY + nClientY) / 2;
		//HWND_TOP 置顶
		//MINE_SIZE*g_iDivSize + (nWinX - nClientX) 窗口大小=客户区大小+非客户区大小
		//MINE_SIZE*g_iDivSize + (nWinY - nClientY  窗口大小=客户区大小+非客户区大小
		SetWindowPos(hWnd, HWND_TOP, x, y, MINE_SIZE*g_iDivSize + (nWinX - nClientX),
			MINE_SIZE*g_iDivSize + (nWinY - nClientY), SWP_SHOWWINDOW);
		InitMineArea();
		//bResult = TRUE;
		break;
	case WM_LBUTTONDOWN:
		xPos = LOWORD(lParam) / MINE_SIZE; //鼠标x坐标在那个格子里面
		yPos = HIWORD(lParam) / MINE_SIZE; //鼠标Y坐标在那个格子里面
		
		if (g_arrDivState[xPos][yPos] == 0)
		{
			if (g_arrMine[xPos][yPos])
			{
				//假如这个地方有雷， 结束游戏
				MessageBox(NULL, TEXT("扫雷失败"), TEXT("Fail"), MB_OK);
				//TODO 开始新游戏
				bResult = TRUE;
				InvalidateRect(hWnd, NULL, TRUE);
			}
			else
			{
				bMouse = TRUE;
				g_arrDivState[xPos][yPos] = 1;				
				rect.left = xPos * MINE_SIZE;
				rect.right = (xPos + 1) * MINE_SIZE;
				rect.top = yPos * MINE_SIZE;
				rect.bottom = (yPos + 1)*MINE_SIZE;
				InvalidateRect(hWnd, &rect, TRUE); //刷新矩形框

				g_iResDiv--;
				if (g_iResDiv == 0)
				{
					MessageBox(NULL, TEXT("扫雷成功"), TEXT("Success"), MB_OK);
					//TODO 开始新游戏
					InitMineArea();
					InvalidateRect(hWnd, NULL, TRUE);
				}
			}			
		}
		break;
	case WM_RBUTTONDOWN:
		xPos = LOWORD(lParam) / MINE_SIZE; //鼠标x坐标在那个格子里面
		yPos = HIWORD(lParam) / MINE_SIZE; //鼠标Y坐标在那个格子里面
		if (g_arrDivState[xPos][yPos] == 0 || g_arrDivState[xPos][yPos] == 2)
		{
			//右键单击，标志有雷
			if (g_arrDivState[xPos][yPos] == 0)
			{
				g_arrDivState[xPos][yPos] = 2;
			}
			else
			{
				g_arrDivState[xPos][yPos] = 0;
			}
			bMouse = TRUE;
			rect.left = xPos * MINE_SIZE;
			rect.right = (xPos + 1) * MINE_SIZE;
			rect.top = yPos * MINE_SIZE;
			rect.bottom = (yPos + 1)*MINE_SIZE;
			InvalidateRect(hWnd, &rect, TRUE); //刷新矩形框
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
		for (x = 0; x < g_iDivSize; x++)
		{
			for (y = 0; y < g_iDivSize; y++)
			{
				//调成灰色
				HBRUSH hBrush = (HBRUSH)GetStockObject(GRAY_BRUSH);
				SelectObject(hdc, hBrush);
				Rectangle(hdc, x*MINE_SIZE, y*MINE_SIZE, (x + 1)*MINE_SIZE, (y + 1)*MINE_SIZE);
			}
		}
		if (bMouse)
		{
			bMouse = FALSE;
			if (g_arrDivState[xPos][yPos] == 1)
			{
				HBRUSH hBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
				SelectObject(hdc, hBrush);
				Rectangle(hdc, xPos*MINE_SIZE, yPos*MINE_SIZE, (xPos + 1)*MINE_SIZE, (yPos + 1)*MINE_SIZE);

				rect.left = xPos * MINE_SIZE;
				rect.right = (xPos + 1) * MINE_SIZE;
				rect.top = yPos * MINE_SIZE;
				rect.bottom = (yPos + 1)*MINE_SIZE;
				SetBkMode(hdc, TRANSPARENT);
				wsprintf(szBuf, TEXT("%d"), g_arrRoundMine[xPos][yPos]);
				DrawText(hdc, szBuf, wcslen(szBuf), &rect, DT_CENTER | DT_VCENTER);
			}
			else if (g_arrDivState[xPos][yPos] == 0) //还原矩形框
			{
				HBRUSH hBrush = (HBRUSH)GetStockObject(GRAY_BRUSH);
				SelectObject(hdc, hBrush);
				Rectangle(hdc, xPos*MINE_SIZE, yPos*MINE_SIZE, (xPos + 1)*MINE_SIZE, (yPos + 1)*MINE_SIZE);
			}
			else
			{
				HBRUSH hBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
				SelectObject(hdc, hBrush);
				Ellipse(hdc, xPos*MINE_SIZE, yPos*MINE_SIZE, (xPos + 1)*MINE_SIZE, (yPos + 1)*MINE_SIZE);
			}
		}
		if (bResult)
		{
			bResult = FALSE;
			for (x = 0; x < g_iDivSize; x++)
			{
				for (y = 0; y < g_iDivSize; y++)
				{
					if (g_arrMine[x][y])
					{
						HBRUSH hBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
						SelectObject(hdc, hBrush);
						Ellipse(hdc, x*MINE_SIZE, y*MINE_SIZE, (x + 1)*MINE_SIZE, (y + 1)*MINE_SIZE);
					}
					else
					{
						HBRUSH hBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
						SelectObject(hdc, hBrush);						
						Rectangle(hdc, x*MINE_SIZE, y*MINE_SIZE, (x + 1)*MINE_SIZE, (y + 1)*MINE_SIZE);

						rect.left = x * MINE_SIZE;
						rect.right = (x + 1) * MINE_SIZE;
						rect.top = y * MINE_SIZE;
						rect.bottom = (y + 1)*MINE_SIZE;
						SetBkMode(hdc, TRANSPARENT);
						wsprintf(szBuf, TEXT("%d"), g_arrRoundMine[x][y]);
						DrawText(hdc, szBuf, wcslen(szBuf), &rect, DT_CENTER | DT_VCENTER);
					}
				}
			}
			if (MessageBox(NULL, TEXT("重新开始？"), TEXT("Retry"), MB_YESNO) == IDYES)
			{
				InitMineArea();
				InvalidateRect(hWnd, NULL, TRUE);
			}
			else
			{
				MessageBox(NULL, L"退出", L"exit", MB_OK);
			}
		}

		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
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

VOID RefreshRoundMine(int x, int y)
{
	
	if (y > 0)          g_arrRoundMine[x][y - 1]++;
	if (y < g_iDivSize) g_arrRoundMine[x][y + 1]++;
	if (x > 0)          g_arrRoundMine[x - 1][y]++;
	if (x < g_iDivSize) g_arrRoundMine[x + 1][y]++;

	if (x > 0 && y > 0)          g_arrRoundMine[x - 1][y - 1]++;
	if (x > 0 && y < g_iDivSize) g_arrRoundMine[x - 1][y + 1]++;
	if (x < g_iDivSize && y > 0) g_arrRoundMine[x + 1][y - 1]++;
	if (x < g_iDivSize && y < g_iDivSize) g_arrRoundMine[x + 1][y + 1]++;
}

VOID InitMineArea()
{
	int i;
	int j;
	int iRandNum;
	int iFlag[DIV_SIZE_MAX * DIV_SIZE_MAX] = { 0 }; // iFlag[i] != 0, 表示该随机数 i 是重复的
	int iCnt;
	
	//清空地雷
	for (i = 0; i < g_iDivSize; i++)
	{
		for (j = 0; j < g_iDivSize; j++)
		{
			g_arrMine[i][j] = FALSE;
			g_arrRoundMine[i][j] = 0;
			g_arrDivState[i][j] = 0;
		}
	}
	g_iResDiv = g_iDivSize * g_iDivSize - g_iMineNum;

	//生成 g_iMineNum 个地雷，放入  g_iDivSize * g_iDivSize 个格子里面
	// 生成 g_iMineNum 个随机数， 随机数的范围为 0~g_iDivSize * g_iDivSize-1
	// 随机数不能重复
	std::random_device rd;
	iCnt = g_iMineNum;
	while (iCnt)
	{
		iRandNum = rd() % (g_iDivSize * g_iDivSize);
		if (iFlag[iRandNum] == 0) //保证随机数不重复
		{
			i = iRandNum / g_iDivSize;
			j = iRandNum % g_iDivSize;
			g_arrMine[i][j] = TRUE; //这个位置上有雷
			RefreshRoundMine(i, j);	

			iFlag[iRandNum] = 1;
			iCnt--;
		}
	}
}