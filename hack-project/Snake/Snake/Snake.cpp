// Snake.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "Snake.h"
#include <vector>
#include <random>

#define MAX_LOADSTRING 100
//定义游戏区 和 控制区大小
#define BOUND_SIZE 10
#define SNACK_SIZE 10
#define GAME_WIDTH  80
#define GAME_HEIGHT 60
#define INFO_WIDTH  30
#define INFO_HEIGHT GAME_HEIGHT

#define MAX_NODE  80 //蛇的最大长度 80 节

#define MY_TIMER 1  //定时器ID
#define DEFAULT_INTERVAL 500 //定义贪食蛇的默认移动速度 500毫秒移动一节
#define PAUSE_ID 1

std::vector<POINT> vSnake;
UCHAR g_ucSnakeLen = 4;
UCHAR g_ucSnakeHead = 4; //vSnack[g_ucSnakeHead-1] 表示蛇头的坐标
UCHAR g_ucSnakeTail = 0; //vSnack[g_ucSnakeTail] 表示蛇尾的坐标
UINT32 g_uiInterval = DEFAULT_INTERVAL; //移动速度
POINT  g_ptDirect = {1, 0}; //移动方向，每次x+1, y不变
POINT  g_ptFoodPos;

BOOL g_bState = TRUE; //游戏是否结束
BOOL g_bNeedFood = TRUE; //是否要投放食物


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
	LoadString(hInstance, IDC_SNAKE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 执行应用程序初始化: 
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SNAKE));

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
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SNAKE));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_SNAKE);
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

   hWnd = CreateWindow(szWindowClass, szTitle, WS_SYSMENU | WS_MINIMIZEBOX,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

VOID InitSnack()  //初始化蛇，设定蛇的起始位置
{
	int i;

	vSnake.clear();
	vSnake.resize(MAX_NODE);	

	g_ucSnakeTail = 0;
	g_ucSnakeHead = 4;
	g_ucSnakeLen = 4;
	g_uiInterval = DEFAULT_INTERVAL;

	for (i = 0; i < g_ucSnakeLen; i++)
	{
		//初始化蛇的各个节点
		vSnake[i].x = i;
		vSnake[i].y = 1;
	}
}

POINT &GetSnakeNode(int index) //获取蛇节点位置：倒数第几个节点
{
	int i = g_ucSnakeTail + index;

	if (i >= MAX_NODE)
	{
		i -= MAX_NODE;
	}

	return vSnake[i];
}

VOID DrawSnake(HDC hdc)
{
	int i;
	POINT ptNode;
	HBRUSH hBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
	SelectObject(hdc, hBrush);
	for (i = 0; i < g_ucSnakeLen; i++)
	{
		//从蛇尾开始画
		ptNode = GetSnakeNode(i);
		Rectangle(hdc, ptNode.x * SNACK_SIZE + BOUND_SIZE,
			ptNode.y * SNACK_SIZE + BOUND_SIZE,
			(ptNode.x + 1) * SNACK_SIZE + BOUND_SIZE,
			(ptNode.y + 1) * SNACK_SIZE + BOUND_SIZE);
	}
}

//移动蛇坐标
VOID RefreshSnake()
{
	//采用翻滚的形式 比如，vSnake[0], vSnake[1],vSnake[2] vSnake[3] 表示蛇的话
	//移动一节后，vSnake[1],vSnake[2] vSnake[3] vSnake[4] 表示蛇
	//vSnake[MAX_NODE -1 ] 后，蛇头存入 vSnake[0]
	POINT ptNewHead; //新的蛇头位置
	POINT ptNode;
	int i;

	ptNewHead.x = GetSnakeNode(g_ucSnakeLen - 1).x + g_ptDirect.x;
	ptNewHead.y = GetSnakeNode(g_ucSnakeLen - 1).y + g_ptDirect.y;

	if (!g_bNeedFood && ptNewHead.x == g_ptFoodPos.x && ptNewHead.y == g_ptFoodPos.y)
	{
		//吃到食物了
		vSnake[g_ucSnakeHead] = ptNewHead;

		g_ucSnakeHead++;
		if (g_ucSnakeHead == MAX_NODE) g_ucSnakeHead = 0;

		g_ucSnakeLen++;

		if (g_ucSnakeLen == MAX_NODE)
		{
			//赢了，事实上，我们不应该等到这个时候才判断赢了
			g_bState = FALSE;
			return;
		}
		g_bNeedFood = TRUE;
		return;
	}

	if (ptNewHead.x < 0 || ptNewHead.x >= GAME_WIDTH || ptNewHead.y < 0 || ptNewHead.y >= GAME_HEIGHT)
	{
		//蛇撞墙了
		g_bState = FALSE;
		return;
	}

	for (i = 1; i < g_ucSnakeLen; i++)
	{
		ptNode = GetSnakeNode(i);
		if (ptNode.x == ptNewHead.x && ptNode.y == ptNewHead.y)
		{
			//蛇撞到自己了
			g_bState = FALSE;
			return;
		}
	}
	

	vSnake[g_ucSnakeHead].x = ptNewHead.x;
	vSnake[g_ucSnakeHead].y = ptNewHead.y;//新的蛇头

	g_ucSnakeHead++;
	if (g_ucSnakeHead == MAX_NODE) g_ucSnakeHead = 0;
	
	g_ucSnakeTail++;
	if (g_ucSnakeTail == MAX_NODE) g_ucSnakeTail = 0;

	return;
}

VOID DrawFood(HDC hdc)
{
	int x, y;
	POINT ptNode; 
	int i;
	HBRUSH hBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);

	if (!g_bNeedFood)
	{
		SelectObject(hdc, hBrush);
		Ellipse(hdc, BOUND_SIZE + g_ptFoodPos.x * SNACK_SIZE, 
			BOUND_SIZE + g_ptFoodPos.y * SNACK_SIZE,
			BOUND_SIZE + (g_ptFoodPos.x + 1) * SNACK_SIZE, 
			BOUND_SIZE + (g_ptFoodPos.y + 1) * SNACK_SIZE);
		return;
	}

	std::random_device rd;
	//获取随机坐标，不能是蛇的位置
	while (1)
	{
		x = rd() % GAME_WIDTH;
		y = rd() % GAME_HEIGHT;

		for (i = 0; i < g_ucSnakeLen; i++)
		{
			ptNode = GetSnakeNode(i);
			if (ptNode.x == x && ptNode.y == y)
			{
				break;
			}
		}
		if (i == g_ucSnakeLen) //一直没有break，表示不重复
		{
			break;
		}
	}
	g_bNeedFood = FALSE;
	g_ptFoodPos.x = x;
	g_ptFoodPos.y = y;

	SelectObject(hdc, hBrush);
	Ellipse(hdc, BOUND_SIZE + x * SNACK_SIZE, BOUND_SIZE + y * SNACK_SIZE,
		BOUND_SIZE + (x + 1) * SNACK_SIZE, BOUND_SIZE + (y + 1) * SNACK_SIZE);

	return;
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
	int nWinX, nWinY, nClientX, nClientY;
	HBRUSH hBrush;

	switch (message)
	{
	case WM_CREATE: //创建窗口时候执行的代码
		GetWindowRect(hWnd, &rect); //获取窗口大小
		nWinX = rect.right - rect.left;
		nWinY = rect.bottom - rect.top;
		GetClientRect(hWnd, &rect); //客户区大小
		nClientX = rect.right - rect.left;
		nClientY = rect.bottom - rect.top;

		//修改窗口大小 客户区大小 + 边框大小 （nWinX-nClientX）
		MoveWindow(hWnd, 0, 0, 
			(GAME_WIDTH + INFO_WIDTH)*SNACK_SIZE + BOUND_SIZE * 3 + (nWinX - nClientX),
			GAME_HEIGHT*SNACK_SIZE + BOUND_SIZE * 2 + (nWinY - nClientY), TRUE);
		
		
		
		InitSnack();

		SetTimer(hWnd, MY_TIMER, g_uiInterval,NULL); //起一个定时器

		break;
	case WM_TIMER: //定时器到点
		//移动蛇
		RefreshSnake();
		if (!g_bState)
		{
			KillTimer(hWnd, MY_TIMER); //停止计时器
			MessageBox(NULL, TEXT("你输了"), TEXT("FAIL"), MB_OK);
			return 0;
		}
		InvalidateRect(hWnd, NULL, TRUE);
		break;
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

		hBrush = (HBRUSH)GetStockObject(GRAY_BRUSH);
		SelectObject(hdc, hBrush);

		Rectangle(hdc, BOUND_SIZE, BOUND_SIZE, 
			BOUND_SIZE + GAME_WIDTH*SNACK_SIZE, 
			BOUND_SIZE + GAME_HEIGHT*SNACK_SIZE);

		Rectangle(hdc, BOUND_SIZE * 2 + GAME_WIDTH*SNACK_SIZE  , BOUND_SIZE,
			BOUND_SIZE*2 + (GAME_WIDTH + INFO_WIDTH)*SNACK_SIZE,
			BOUND_SIZE + INFO_HEIGHT*SNACK_SIZE);

		DrawSnake(hdc);

		DrawFood(hdc);

		EndPaint(hWnd, &ps);
		break;
	case WM_KEYDOWN:
		if (!g_bState )
		{
			break;
		}
		switch (wParam)
		{
		case VK_UP: //调节方向：注意点，原来是往上或者往下的话，不做操作
			if (g_ptDirect.x != 0)
			{
				g_ptDirect.x = 0;
				g_ptDirect.y = -1;
			}
			break;
		case VK_DOWN:
			if (g_ptDirect.x != 0)
			{
				g_ptDirect.x = 0;
				g_ptDirect.y = 1;
			}
			break;
		case VK_LEFT:
			if (g_ptDirect.y != 0)
			{
				g_ptDirect.x = -1;
				g_ptDirect.y = 0;
			}
			break;
		case VK_RIGHT:
			if (g_ptDirect.y != 0)
			{
				g_ptDirect.x = 1;
				g_ptDirect.y = 0;
			}
			break;
		}
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
