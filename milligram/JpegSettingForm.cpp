#include "stdafx.h"
#include "JpegSettingForm.h"

CJpegSettingForm *JpegSettingForm;

INT_PTR CALLBACK JpegSettingFormWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	bool CallDefault = false;
	LRESULT result;
	result = JpegSettingForm->ProcessMessages(hWnd, message, wParam, lParam, CallDefault);
	if (CallDefault)
	{
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return result;
}

void CJpegSettingForm::SetData(std::wstring atitile, std::wstring ames, int * Data)
{
	Title = atitile;
	Message = ames;
	for (int i = 0; i < 4; i++)JCR[i] = Data[i];
}

void CJpegSettingForm::GetData(int * Data)
{
	for (int i = 0; i < 4; i++)Data[i] = JCR[i];
}

INT_PTR CJpegSettingForm::ShowDialog(HINSTANCE appInstance, HWND hWnd)
{
	return (DialogBox(appInstance, TEXT("IDD_JPEGSETTINGFORM"), hWnd, JpegSettingFormWndProc));
}

void CJpegSettingForm::Init(HWND hWnd)
{
	SetHWND(hWnd);
	hJCREdit = GetDlgItem(handle, ID_JPEGF_EDIT);
	hStaticText = GetDlgItem(handle, ID_JPEGF_STATIC);
	hSlider = GetDlgItem(handle, ID_JPEGF_SLIDER);
	Button[0] = GetDlgItem(handle, ID_JPEGF_BUTTON1);
	Button[1] = GetDlgItem(handle, ID_JPEGF_BUTTON2);
	Button[2] = GetDlgItem(handle, ID_JPEGF_BUTTON3);

	JCREdit.SetHWND(hJCREdit);
	JCRString.SetIntegerMode(JCR[0], false, true, true, 0, 100);

	Slider.SetHWND(hSlider);
	Slider.Init(0, 100, 10, JCR[0]);

	SetWindowText(handle, Title.c_str()); // ƒ^ƒCƒgƒ‹
	SetWindowText(hStaticText, Message.c_str());
	SetWindowText(hJCREdit, std::to_wstring(JCR[0]).c_str());
	SetWindowText(Button[0], std::to_wstring(JCR[1]).c_str());
	SetWindowText(Button[1], std::to_wstring(JCR[2]).c_str());
	SetWindowText(Button[2], std::to_wstring(JCR[3]).c_str());

	SuspendUpdate = false;

}

BOOL CJpegSettingForm::ProcessMessages(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool & CallDefault)
{
	CallDefault = false;
	switch (message)
	{
	case WM_INITDIALOG:
		Init(hWnd);
		break;

	case WM_CLOSE:
		Visible = false;
		EndDialog(hWnd, IDCANCEL);
		return(TRUE);

	case WM_SHOWWINDOW:
		Visible = true;
		break;
	
	case WM_HSCROLL:
		if (SuspendUpdate == false)
			Slider_Move();
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hWnd, IDOK);
			return(TRUE);
		case IDCANCEL:
			EndDialog(hWnd, IDCANCEL);
			return(TRUE);
		case ID_JPEGF_EDIT:
			switch (HIWORD(wParam))
			{
			case EN_UPDATE:
				if(SuspendUpdate == false)
					JCREdit_Change();
				break;
			}
			break;
		case ID_JPEGF_BUTTON1:
			ButtonN_Click(0);
			break;
		case ID_JPEGF_BUTTON2:
			ButtonN_Click(1);
			break;
		case ID_JPEGF_BUTTON3:
			ButtonN_Click(2);
			break;
		}
		break;

	}
	CallDefault = true;
	return(FALSE);
}


void CJpegSettingForm::ButtonN_Click(int index)
{
	if (((GetAsyncKeyState(VK_LCONTROL) & 0x8000) == 0 && (GetAsyncKeyState(VK_RCONTROL) & 0x8000)) == 0)
	{
		JCR[0] = JCR[index + 1];

		SuspendUpdate = true;
		Slider.SetSliderValue(JCR[index + 1]);
		SetWindowText(hJCREdit, std::to_wstring(JCR[0]).c_str());
		SuspendUpdate = false;
	}
	else
	{
		JCR[index + 1] = JCR[0];
		SetWindowText(Button[index], std::to_wstring(JCR[index + 1]).c_str());
	}
}

void CJpegSettingForm::JCREdit_Change(void)
{
	std::wstring temp = acfc::GetWindowString(hJCREdit);
	size_t CurPos = HIWORD(SendMessage(hJCREdit, EM_GETSEL, 0, 0));

	temp = JCRString.CheckString(temp, CurPos);

	JCR[0] = JCRString.GetIntegerValue();

	SuspendUpdate = true;
	SetWindowText(hJCREdit, std::to_wstring(JCR[0]).c_str());
	Slider.SetSliderValue(JCR[0]);
	JCR[0] = JCRString.GetIntegerValue();
	SuspendUpdate = false;

	SendMessage(hJCREdit, EM_SETSEL, CurPos, CurPos);

}

void CJpegSettingForm::Slider_Move(void)
{
	JCR[0] = Slider.Value;

	SuspendUpdate = true;
	SetWindowText(hJCREdit, std::to_wstring(JCR[0]).c_str());
	SuspendUpdate = false;
}

