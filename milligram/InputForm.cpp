#include "stdafx.h"
#include "InputForm.h"

CInputForm *InputForm;

// フォームイベントプロシージャ
BOOL CALLBACK InputFormWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	bool CallDefault = false;
	LRESULT result;
	result = InputForm->ProcessMessages(hWnd, message, wParam, lParam, CallDefault);
	if (CallDefault)
	{
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return result;
}

void CInputForm::SetData(std::wstring atitle, std::wstring ames, std::wstring atext)
{
	Title = atitle;
	Message = ames;
	Text = atext;
}

int CInputForm::ShowDialog(HINSTANCE appInstance, HWND hWnd)
{
	return (DialogBox(appInstance, TEXT("IDD_INPUTFORM"), hWnd, InputFormWndProc));
}


BOOL CInputForm::ProcessMessages(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool &CallDefault)
{
	CallDefault = false;
	switch (message)
	{
	case WM_INITDIALOG:
		Init(hWnd);
		break;
	case WM_CLOSE:
		EndDialog(hWnd, IDCANCEL);
		return(TRUE);

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			ButtonOK_Click();
			EndDialog(hWnd, IDOK);
			return(TRUE);
		case IDCANCEL:
			EndDialog(hWnd, IDCANCEL);
			return(TRUE);
		}
		break;

	}
	CallDefault = true;
	return(FALSE);
}

void CInputForm::Init(HWND hWnd)
{
	SetHWND(hWnd);
	hStaticText = GetDlgItem(handle, ID_INPUTF_STATIC);
	hEdit = GetDlgItem(handle, ID_INPUTF_EDIT);

	SetWindowText(handle, Title.c_str());
	SetWindowText(hStaticText, Message.c_str());
	SetWindowText(hEdit, Text.c_str());
}

void CInputForm::ButtonOK_Click(void)
{
	int Length = GetWindowTextLength(hEdit) + 1;
	TCHAR *buf = new TCHAR[Length];
	GetWindowText(hEdit, buf, Length);
	Result = buf;
	delete[] buf;
}


