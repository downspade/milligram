#include "stdafx.h"
#include "FormSizeForm.h"

CFormSizeForm *FormSizeForm;

BOOL CALLBACK FormSizeFormWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	bool CallDefault = false;
	LRESULT result;
	result = FormSizeForm->ProcessMessages(hWnd, message, wParam, lParam, CallDefault);
	if (CallDefault)
	{
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return result;
}

CFormSizeForm::CFormSizeForm(void)
{
}


void CFormSizeForm::SetData(std::vector<std::wstring>& Str, int w, int h)
{
	std::copy(Str.begin(), Str.end(), std::back_inserter(StrData));
	WidthData = w;
	HeightData = h;
	Ratio = (double)w / h;
}

int CFormSizeForm::ShowDialog(HINSTANCE appInstance, HWND hWnd)
{
	return (DialogBox(appInstance, TEXT("IDD_FORMSIZEFORM"), hWnd, FormSizeFormWndProc));
}

void CFormSizeForm::GetData(int & w, int & h)
{
	w = WidthData;
	h = HeightData;
}


void CFormSizeForm::Init(HWND hWnd)
{
	SetHWND(hWnd);
	hWidthEdit = GetDlgItem(handle, ID_FORMSIZEF_EDIT_WIDTH);
	hHeightEdit = GetDlgItem(handle, ID_FORMSIZEF_EDIT_HEIGHT);
	hWidthText = GetDlgItem(handle, ID_FORMSIZEF_STATIC_WIDTH);
	hHeightText = GetDlgItem(handle, ID_FORMSIZEF_STATIC_HEIGHT);

	SetWindowText(handle, StrData[0].c_str()); // ƒ^ƒCƒgƒ‹
	SetWindowText(hWidthText, StrData[1].c_str());
	SetWindowText(hHeightText, StrData[2].c_str());

	SetWindowText(hWidthEdit, std::to_wstring(WidthData).c_str());
	SetWindowText(hHeightEdit, std::to_wstring(HeightData).c_str());

	WidthEdit.SetHWND(hWidthEdit);
	HeightEdit.SetHWND(hHeightEdit);

	WidthStr.SetIntegerMode(WidthData, false, true, true, 30, 99999);
	HeightStr.SetIntegerMode(HeightData, false, true, true, 30, 99999);

	SuspendUpdate = false;
}

BOOL CFormSizeForm::ProcessMessages(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool & CallDefault)
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
		switch (LOWORD(wParam))
		{
		case IDOK:
			ButtonOK_Click();
			EndDialog(hWnd, IDOK);
			return(TRUE);
		case IDCANCEL:
			EndDialog(hWnd, IDCANCEL);
			return(TRUE);
		case ID_FORMSIZEF_EDIT_WIDTH:
			switch (HIWORD(wParam))
			{
			case EN_UPDATE:
				if(SuspendUpdate == false)
					WidthEdit_Changed();
				break;
			case EN_KILLFOCUS:
				WidthEdit_Leave();
				break;
			}
			break;
		case ID_FORMSIZEF_EDIT_HEIGHT:
			switch (HIWORD(wParam))
			{
			case EN_UPDATE:
				if (SuspendUpdate == false)
					HeightEdit_Changed();
				break;
			case EN_KILLFOCUS:
				HeightEdit_Leave();
				break;
			}
			break;

		}
		break;

	}
	CallDefault = true;
	return(FALSE);
}

void CFormSizeForm::WidthEdit_Changed(void)
{
	std::wstring temp = acfc::GetWindowString(hWidthEdit);
	int CurPos = HIWORD(SendMessage(hWidthEdit, EM_GETSEL, 0, 0));

	temp = WidthStr.CheckString(temp, CurPos);

	WidthData = WidthStr.GetIntegerValue();
	HeightData = (int)((WidthData + Ratio / 2) / Ratio);

	SuspendUpdate = true;
	SetWindowText(hHeightEdit, std::to_wstring(HeightData).c_str());
	SuspendUpdate = false;

	HeightStr.SetIntegerValue(HeightData);
}

void CFormSizeForm::HeightEdit_Changed(void)
{
	std::wstring temp = acfc::GetWindowString(hWidthEdit);
	int CurPos = HIWORD(SendMessage(hWidthEdit, EM_GETSEL, 0, 0));

	temp = WidthStr.CheckString(temp, CurPos);

	HeightData = HeightStr.GetIntegerValue();
	WidthData = (int)((HeightData + 0.5) * Ratio);

	SuspendUpdate = true;
	SetWindowText(hWidthEdit, std::to_wstring(WidthData).c_str());
	SuspendUpdate = false;

	WidthStr.SetIntegerValue(WidthData);
}

void CFormSizeForm::WidthEdit_Leave(void)
{
	SetWindowText(hWidthEdit, std::to_wstring(WidthData).c_str());
}

void CFormSizeForm::HeightEdit_Leave(void)
{
	SetWindowText(hHeightEdit, std::to_wstring(HeightData).c_str());
}

void CFormSizeForm::ButtonOK_Click(void)
{
	WidthData = WidthStr.GetIntegerValue();
	HeightData = HeightStr.GetIntegerValue();
}
