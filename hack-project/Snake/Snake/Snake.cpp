// Snake.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "Snake.h"
#include <vector>
#include <random>

#define MAX_LOADSTRING 100
//������Ϸ�� �� ��������С
#define BOUND_SIZE 10
#define SNACK_SIZE 10
#define GAME_WIDTH  80
#define GAME_HEIGHT 60
#define INFO_WIDTH  30
#define INFO_HEIGHT GAME_HEIGHT

#define MAX_NODE  80 //�ߵ���󳤶� 80 ��

#define MY_TIMER 1  //��ʱ��ID
#define DEFAULT_INTERVAL 500 //����̰ʳ�ߵ�Ĭ���ƶ��ٶ� 500�����ƶ�һ��
#define PAUSE_ID 1

std::vector<POINT> vSnake;
UCHAR g_ucSnakeLen = 4;
UCHAR g_ucSnakeHead = 4; //vSnack[g_ucSnakeHead-1] ��ʾ��ͷ������
UCHAR g_ucSnakeTail = 0; //vSnack[g_ucSnakeTail] ��ʾ��β������
UINT32 g_uiInterval = DEFAULT_INTERVAL; //�ƶ��ٶ�
POINT  g_ptDirect = {1, 0}; //�ƶ�����ÿ��x+1, y����
POINT  g_ptFoodPos;

BOOL g_bState = TRUE; //��Ϸ�Ƿ����
BOOL g_bNeedFood = TRUE; //�Ƿ�ҪͶ��ʳ��


// ȫ�ֱ���: 
HINSTANCE hInst;								// ��ǰʵ��
TCHAR szTitle[MAX_LOADSTRING];					// �������ı�
TCHAR szWindowClass[MAX_LOADSTRING];			// ����������

// �˴���ģ���а����ĺ�����ǰ������: 
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

 	// TODO:  �ڴ˷��ô��롣
	MSG msg;
	HACCEL hAccelTable;

	// ��ʼ��ȫ���ַ���
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_SNAKE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// ִ��Ӧ�ó����ʼ��: 
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SNAKE));

	// ����Ϣѭ��: 
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
//  ����:  MyRegisterClass()
//
//  Ŀ��:  ע�ᴰ���ࡣ
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
//   ����:  InitInstance(HINSTANCE, int)
//
//   Ŀ��:  ����ʵ�����������������
//
//   ע��: 
//
//        �ڴ˺����У�������ȫ�ֱ����б���ʵ�������
//        ��������ʾ�����򴰿ڡ�
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // ��ʵ������洢��ȫ�ֱ�����

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

VOID InitSnack()  //��ʼ���ߣ��趨�ߵ���ʼλ��
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
		//��ʼ���ߵĸ����ڵ�
		vSnake[i].x = i;
		vSnake[i].y = 1;
	}
}

POINT &GetSnakeNode(int index) //��ȡ�߽ڵ�λ�ã������ڼ����ڵ�
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
		//����β��ʼ��
		ptNode = GetSnakeNode(i);
		Rectangle(hdc, ptNode.x * SNACK_SIZE + BOUND_SIZE,
			ptNode.y * SNACK_SIZE + BOUND_SIZE,
			(ptNode.x + 1) * SNACK_SIZE + BOUND_SIZE,
			(ptNode.y + 1) * SNACK_SIZE + BOUND_SIZE);
	}
}

//�ƶ�������
VOID RefreshSnake()
{
	//���÷�������ʽ ���磬vSnake[0], vSnake[1],vSnake[2] vSnake[3] ��ʾ�ߵĻ�
	//�ƶ�һ�ں�vSnake[1],vSnake[2] vSnake[3] vSnake[4] ��ʾ��
	//vSnake[MAX_NODE -1 ] ����ͷ���� vSnake[0]
	POINT ptNewHead; //�µ���ͷλ��
	POINT ptNode;
	int i;

	ptNewHead.x = GetSnakeNode(g_ucSnakeLen - 1).x + g_ptDirect.x;
	ptNewHead.y = GetSnakeNode(g_ucSnakeLen - 1).y + g_ptDirect.y;

	if (!g_bNeedFood && ptNewHead.x == g_ptFoodPos.x && ptNewHead.y == g_ptFoodPos.y)
	{
		//�Ե�ʳ����
		vSnake[g_ucSnakeHead] = ptNewHead;

		g_ucSnakeHead++;
		if (g_ucSnakeHead == MAX_NODE) g_ucSnakeHead = 0;

		g_ucSnakeLen++;

		if (g_ucSnakeLen == MAX_NODE)
		{
			//Ӯ�ˣ���ʵ�ϣ����ǲ�Ӧ�õȵ����ʱ����ж�Ӯ��
			g_bState = FALSE;
			return;
		}
		g_bNeedFood = TRUE;
		return;
	}

	if (ptNewHead.x < 0 || ptNewHead.x >= GAME_WIDTH || ptNewHead.y < 0 || ptNewHead.y >= GAME_HEIGHT)
	{
		//��ײǽ��
		g_bState = FALSE;
		return;
	}

	for (i = 1; i < g_ucSnakeLen; i++)
	{
		ptNode = GetSnakeNode(i);
		if (ptNode.x == ptNewHead.x && ptNode.y == ptNewHead.y)
		{
			//��ײ���Լ���
			g_bState = FALSE;
			return;
		}
	}
	

	vSnake[g_ucSnakeHead].x = ptNewHead.x;
	vSnake[g_ucSnakeHead].y = ptNewHead.y;//�µ���ͷ

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
	//��ȡ������꣬�������ߵ�λ��
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
		if (i == g_ucSnakeLen) //һֱû��break����ʾ���ظ�
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
//  ����:  WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  Ŀ��:    ���������ڵ���Ϣ��
//
//  WM_COMMAND	- ����Ӧ�ó���˵�
//  WM_PAINT	- ����������
//  WM_DESTROY	- �����˳���Ϣ������
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
	case WM_CREATE: //��������ʱ��ִ�еĴ���
		GetWindowRect(hWnd, &rect); //��ȡ���ڴ�С
		nWinX = rect.right - rect.left;
		nWinY = rect.bottom - rect.top;
		GetClientRect(hWnd, &rect); //�ͻ�����С
		nClientX = rect.right - rect.left;
		nClientY = rect.bottom - rect.top;

		//�޸Ĵ��ڴ�С �ͻ�����С + �߿��С ��nWinX-nClientX��
		MoveWindow(hWnd, 0, 0, 
			(GAME_WIDTH + INFO_WIDTH)*SNACK_SIZE + BOUND_SIZE * 3 + (nWinX - nClientX),
			GAME_HEIGHT*SNACK_SIZE + BOUND_SIZE * 2 + (nWinY - nClientY), TRUE);
		
		
		
		InitSnack();

		SetTimer(hWnd, MY_TIMER, g_uiInterval,NULL); //��һ����ʱ��

		break;
	case WM_TIMER: //��ʱ������
		//�ƶ���
		RefreshSnake();
		if (!g_bState)
		{
			KillTimer(hWnd, MY_TIMER); //ֹͣ��ʱ��
			MessageBox(NULL, TEXT("������"), TEXT("FAIL"), MB_OK);
			return 0;
		}
		InvalidateRect(hWnd, NULL, TRUE);
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// �����˵�ѡ��: 
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
		// TODO:  �ڴ���������ͼ����...

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
		case VK_UP: //���ڷ���ע��㣬ԭ�������ϻ������µĻ�����������
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

// �����ڡ������Ϣ�������
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
