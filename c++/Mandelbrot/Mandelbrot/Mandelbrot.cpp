#include "stdafx.h"
#include <string.h>
#include <stdio.h>
#include "Mandelbrot.h"
#include "opencl.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);


HDC imageDC;
HBITMAP image;
Mandelbrot* mandelbrot = NULL;
Mandelbrot* mandelbrot_cpu = NULL;
Mandelbrot* mandelbrot_gpu = NULL;

int double_fp = 1;
double cx = -0.5;
double cy = 0;
double radio = 1.5;

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;
	HACCEL hAccelTable;

	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_MANDELBROT, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MANDELBROT));

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

void RenderImage(HWND hWnd)
{
	long long a, b, c;
	int timespan;
	char buf[1024];
	int len;
	void* bmp;

	HDC hdc = GetDC(hWnd);
	QueryPerformanceCounter((LARGE_INTEGER*)&a);
	if(double_fp)
		bmp = mandelbrot->Render(cx, cy, radio);
	else
		bmp = mandelbrot->Render32((float)cx, (float)cy, (float)radio);

	BITMAPINFO bmpInfo;  
    bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);  
    bmpInfo.bmiHeader.biWidth = 512;  
    bmpInfo.bmiHeader.biHeight = -512;  
    bmpInfo.bmiHeader.biPlanes = 1;
    bmpInfo.bmiHeader.biBitCount = 32;  
    bmpInfo.bmiHeader.biCompression = 0;  
    bmpInfo.bmiHeader.biSizeImage = 0;  
    bmpInfo.bmiHeader.biXPelsPerMeter = 3000;  
    bmpInfo.bmiHeader.biYPelsPerMeter = 3000;  
    bmpInfo.bmiHeader.biClrUsed = 0;  
    bmpInfo.bmiHeader.biClrImportant = 0;
	DWORD re = SetDIBitsToDevice(imageDC, 0, 0, 512, 512, 0, 0, 0, 512, bmp, &bmpInfo, DIB_RGB_COLORS);
	BitBlt(hdc, 0, 0, 512, 512, imageDC, 0, 0, SRCCOPY);

	QueryPerformanceCounter((LARGE_INTEGER*)&b);
	QueryPerformanceFrequency((LARGE_INTEGER*)&c);
	timespan = (int)((b-a)*1000/c);

	len = _snprintf(buf, 1024, "Rending Time: %d ms    ", timespan);
	TextOut(hdc, 520, 8, buf, len);
	ReleaseDC(hWnd, hdc);
}

void ShowLabel(HWND hWnd, int x, int y)
{
	char buf[1024];
	double dx = cx-radio+2*radio*x/512;
	double dy = cy+radio-2*radio*y/512;

	HDC hdc = GetDC(hWnd);

	int len = _snprintf(buf, 1024, "Mouse X: %.15lf    ", dx);
	TextOut(hdc, 520, 24, buf, len);

	len = _snprintf(buf, 1024, "Mouse Y: %.15lf    ", dy);
	TextOut(hdc, 520, 40, buf, len);

	len = _snprintf(buf, 1024, "Center X: %.15lf    ", cx);
	TextOut(hdc, 520, 56, buf, len);

	len = _snprintf(buf, 1024, "Center Y: %.15lf    ", cy);
	TextOut(hdc, 520, 72, buf, len);

	len = _snprintf(buf, 1024, "Radio: %.15lf    ", radio);
	TextOut(hdc, 520, 88, buf, len);

	ReleaseDC(hWnd, hdc);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int xPos;
	static int yPos;
	static int xLD;
	static int yLD;
	static double cxLD;
	static double cyLD;

	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case ID_FILE_CPU:
			{
				HMENU hmenu = GetMenu(hWnd);
				CheckMenuItem(hmenu, ID_FILE_CPU, MF_BYCOMMAND | MF_CHECKED);
				CheckMenuItem(hmenu, ID_FILE_GPU, MF_BYCOMMAND | MF_UNCHECKED);
				mandelbrot = mandelbrot_cpu;
				RenderImage(hWnd);
				break;
			}
		case ID_FILE_GPU:
			{
				HMENU hmenu = GetMenu(hWnd);
				CheckMenuItem(hmenu, ID_FILE_GPU, MF_BYCOMMAND | MF_CHECKED);
				CheckMenuItem(hmenu, ID_FILE_CPU, MF_BYCOMMAND | MF_UNCHECKED);
				mandelbrot = mandelbrot_gpu;
				RenderImage(hWnd);
				break;
			}
			break;
		case ID_FILE_DOUBLE:
			{
				HMENU hmenu = GetMenu(hWnd);
				CheckMenuItem(hmenu, ID_FILE_DOUBLE, MF_BYCOMMAND | MF_CHECKED);
				CheckMenuItem(hmenu, ID_FILE_SINGLE, MF_BYCOMMAND | MF_UNCHECKED);
				double_fp = 1;
				RenderImage(hWnd);
				break;
			}
		case ID_FILE_SINGLE:
			{
				HMENU hmenu = GetMenu(hWnd);
				CheckMenuItem(hmenu, ID_FILE_DOUBLE, MF_BYCOMMAND | MF_UNCHECKED);
				CheckMenuItem(hmenu, ID_FILE_SINGLE, MF_BYCOMMAND | MF_CHECKED);
				double_fp = 0;
				RenderImage(hWnd);
				break;
			}
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		BitBlt(hdc, 0, 0, 512, 512, imageDC, 0, 0, SRCCOPY);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_LBUTTONDOWN:
		{
			xLD = LOWORD(lParam);
			yLD = HIWORD(lParam);
			cxLD = cx;
			cyLD = cy;
		}
	case WM_MOUSEMOVE:
		{
			xPos = LOWORD(lParam);
			yPos = HIWORD(lParam);
			if(MK_LBUTTON & wParam)
			{
				double dx = radio*(xPos-xLD)/256;
				double dy = radio*(yPos-yLD)/256;
				cx = cxLD - dx;
				cy = cyLD + dy;
				RenderImage(hWnd);
			}
			ShowLabel(hWnd, xPos, yPos);
			break;
		}
	case WM_MOUSEWHEEL:
		{
			POINT pt;
			pt.x = LOWORD(lParam);
			pt.y = HIWORD(lParam);
			ScreenToClient(hWnd, &pt);
			xPos = pt.x;
			yPos = pt.y;
			int wheel = GET_WHEEL_DELTA_WPARAM(wParam);

			double dx = cx-radio+2*radio*xPos/512;
			double dy = cy+radio-2*radio*yPos/512;

			if(wheel > 0)
			{
				radio /= 2;
				cx = (cx + dx) / 2;
				cy = (cy + dy) / 2;
			}
			if(wheel < 0)
			{
				radio *= 2;
				cx = 2 * cx - dx;
				cy = 2 * cy - dy;
			}

			RenderImage(hWnd);
			ShowLabel(hWnd, xPos, yPos);
			break;
		}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

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

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MANDELBROT));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_MANDELBROT);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;
   RECT WindowRect;
   DWORD dwStyle = WS_OVERLAPPEDWINDOW;
   hInst = hInstance;

   WindowRect.left = 0;
   WindowRect.top = 0;
   WindowRect.bottom = 512;
   WindowRect.right = 750;
   AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, 0);
   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 
	   WindowRect.right-WindowRect.left, WindowRect.bottom-WindowRect.top, NULL, NULL, hInstance, NULL);

   if (!hWnd)
      return FALSE;

   HDC hdc = GetDC(hWnd);
   imageDC = CreateCompatibleDC(hdc);
   image = CreateCompatibleBitmap(hdc, 512, 512);
   SelectObject(imageDC, image);
   mandelbrot_cpu = new MandelbrotCPU();
   mandelbrot_gpu = new MandelbrotGPU();
   mandelbrot = mandelbrot_gpu;

   HMENU hmenu = GetMenu(hWnd);
   char name[1024];
   mandelbrot_cpu->DeviceName(name);
   ModifyMenu(hmenu, ID_FILE_CPU, MF_BYCOMMAND | MF_STRING | MF_UNCHECKED , ID_FILE_CPU, name);
   mandelbrot_gpu->DeviceName(name);
   ModifyMenu(hmenu, ID_FILE_GPU, MF_BYCOMMAND | MF_STRING | MF_CHECKED, ID_FILE_GPU, name);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
   RenderImage(hWnd);
   return TRUE;
}