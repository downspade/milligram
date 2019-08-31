
// milligram.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "milligram.h"
#include "MainForm.h"
#include "InputForm.h"
#include "FormSizeForm.h"
#include "JpegSettingForm.h"
#include "ProgressForm.h"

#pragma comment(lib, "winmm.lib")

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance


// Forward declarations of functions included in this code module:

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	Gdiplus::GdiplusStartupInput gdiplusStartupInput; // = {1, NULL, FALSE, FALSE};
	gdiplusStartupInput.GdiplusVersion = 1;
	gdiplusStartupInput.DebugEventCallback = NULL;
	gdiplusStartupInput.SuppressBackgroundThread = FALSE;
	gdiplusStartupInput.SuppressExternalCodecs = FALSE;

	ULONG_PTR GdiplusToken = NULL;
	Gdiplus::GdiplusStartup(&GdiplusToken, &gdiplusStartupInput, NULL);

	MainForm = new CMainForm();
	InputForm = new CInputForm();
	FormSizeForm = new CFormSizeForm();
	JpegSettingForm = new CJpegSettingForm();
	ProgressForm = new CProgressForm();

	MainForm->Initialize(hInstance, nCmdShow, lpCmdLine);

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MILLIGRAM));

	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	delete ProgressForm;
	delete JpegSettingForm;
	delete FormSizeForm;
	delete InputForm;
	delete MainForm;

	Gdiplus::GdiplusShutdown(GdiplusToken);

	return (int)msg.wParam;
}





// Message handler for about box.
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



