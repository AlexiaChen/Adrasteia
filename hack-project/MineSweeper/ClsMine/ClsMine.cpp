// ClsMine.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "ClsMine.h"
#include<random>

#define MAX_LOADSTRING 100

#define MINE_SIZE 30  //ÿ������ռ�õĴ�С
#define DIV_SIZE  9   //�����ķ�Χ DIV_SIZE * DIV_SIZE

#define DIV_SIZE_MAX 50 //���������Χ

#define MINE_NUM  10  //���������׵�����

int g_iDivSize = DIV_SIZE;
int g_iMineNum = MINE_NUM;

int g_iResDiv;

BOOL g_arrMine[DIV_SIZE_MAX][DIV_SIZE_MAX] = { 0 }; //����λ���Ƿ�����
int g_arrRoundMine[DIV_SIZE_MAX][DIV_SIZE_MAX] = { 0 };//����λ����Χ�ж�����
//0 ��ʾû�б�������� 1 ��ʾ�������� 2 ��ʾ�Ҽ�����
int g_arrDivState[DIV_SIZE_MAX][DIV_SIZE_MAX] = { 0 };

// ȫ�ֱ���: 
HINSTANCE hInst;								// ��ǰʵ��
TCHAR szTitle[MAX_LOADSTRING];					// �������ı�
TCHAR szWindowClass[MAX_LOADSTRING];			// ����������

// �˴���ģ���а����ĺ�����ǰ������: 
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

 	// TODO:  �ڴ˷��ô��롣
	MSG msg;
	HACCEL hAccelTable;

	// ��ʼ��ȫ���ַ���
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_CLSMINE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// ִ��Ӧ�ó����ʼ��: 
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CLSMINE));

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
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CLSMINE));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_CLSMINE);
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
	int nWinX, nWinY; //���ڴ�С
	int nClientX, nClientY; //�ͻ�����С
	int nScreenX, nScreenY;
	int x, y;
	static int xPos, yPos;  //�洢��갴�µ�λ��
	static BOOL bMouse = FALSE;
	static BOOL bResult = FALSE;
	TCHAR szBuf[4] = {0};

	switch (message)
	{
	case WM_CREATE:
		//��ȡ���ڴ�С
		GetWindowRect(hWnd, &rect);
		nWinX = rect.right - rect.left;
		nWinY = rect.bottom - rect.top;
		//��ȡ�ͻ�����С
		GetClientRect(hWnd, &rect);
		nClientX = rect.right - rect.left;
		nClientY = rect.bottom - rect.top;

		//��ȡ��Ļ�ĳ��ȿ��
		nScreenX = GetSystemMetrics(SM_CXSCREEN); 
		nScreenY = GetSystemMetrics(SM_CYSCREEN);

		//���ھ���
		x = (nScreenX - MINE_SIZE*g_iDivSize - nWinX + nClientX) / 2;
		y = (nScreenY - MINE_SIZE*g_iDivSize - nWinY + nClientY) / 2;
		//HWND_TOP �ö�
		//MINE_SIZE*g_iDivSize + (nWinX - nClientX) ���ڴ�С=�ͻ�����С+�ǿͻ�����С
		//MINE_SIZE*g_iDivSize + (nWinY - nClientY  ���ڴ�С=�ͻ�����С+�ǿͻ�����С
		SetWindowPos(hWnd, HWND_TOP, x, y, MINE_SIZE*g_iDivSize + (nWinX - nClientX),
			MINE_SIZE*g_iDivSize + (nWinY - nClientY), SWP_SHOWWINDOW);
		InitMineArea();
		//bResult = TRUE;
		break;
	case WM_LBUTTONDOWN:
		xPos = LOWORD(lParam) / MINE_SIZE; //���x�������Ǹ���������
		yPos = HIWORD(lParam) / MINE_SIZE; //���Y�������Ǹ���������
		
		if (g_arrDivState[xPos][yPos] == 0)
		{
			if (g_arrMine[xPos][yPos])
			{
				//��������ط����ף� ������Ϸ
				MessageBox(NULL, TEXT("ɨ��ʧ��"), TEXT("Fail"), MB_OK);
				//TODO ��ʼ����Ϸ
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
				InvalidateRect(hWnd, &rect, TRUE); //ˢ�¾��ο�

				g_iResDiv--;
				if (g_iResDiv == 0)
				{
					MessageBox(NULL, TEXT("ɨ�׳ɹ�"), TEXT("Success"), MB_OK);
					//TODO ��ʼ����Ϸ
					InitMineArea();
					InvalidateRect(hWnd, NULL, TRUE);
				}
			}			
		}
		break;
	case WM_RBUTTONDOWN:
		xPos = LOWORD(lParam) / MINE_SIZE; //���x�������Ǹ���������
		yPos = HIWORD(lParam) / MINE_SIZE; //���Y�������Ǹ���������
		if (g_arrDivState[xPos][yPos] == 0 || g_arrDivState[xPos][yPos] == 2)
		{
			//�Ҽ���������־����
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
			InvalidateRect(hWnd, &rect, TRUE); //ˢ�¾��ο�
		}
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
		for (x = 0; x < g_iDivSize; x++)
		{
			for (y = 0; y < g_iDivSize; y++)
			{
				//���ɻ�ɫ
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
			else if (g_arrDivState[xPos][yPos] == 0) //��ԭ���ο�
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
			if (MessageBox(NULL, TEXT("���¿�ʼ��"), TEXT("Retry"), MB_YESNO) == IDYES)
			{
				InitMineArea();
				InvalidateRect(hWnd, NULL, TRUE);
			}
			else
			{
				MessageBox(NULL, L"�˳�", L"exit", MB_OK);
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
	int iFlag[DIV_SIZE_MAX * DIV_SIZE_MAX] = { 0 }; // iFlag[i] != 0, ��ʾ������� i ���ظ���
	int iCnt;
	
	//��յ���
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

	//���� g_iMineNum �����ף�����  g_iDivSize * g_iDivSize ����������
	// ���� g_iMineNum ��������� ������ķ�ΧΪ 0~g_iDivSize * g_iDivSize-1
	// ����������ظ�
	std::random_device rd;
	iCnt = g_iMineNum;
	while (iCnt)
	{
		iRandNum = rd() % (g_iDivSize * g_iDivSize);
		if (iFlag[iRandNum] == 0) //��֤��������ظ�
		{
			i = iRandNum / g_iDivSize;
			j = iRandNum % g_iDivSize;
			g_arrMine[i][j] = TRUE; //���λ��������
			RefreshRoundMine(i, j);	

			iFlag[iRandNum] = 1;
			iCnt--;
		}
	}
}