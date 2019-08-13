#pragma once

#include <windows.h>
#include "resource.h"
#include "acfc.h"

class CJpegSettingForm : public acfc::CBaseWindow
{
public:
	void SetData(std::wstring atitile, std::wstring ames, int *Data);
	void GetData(int *Data);
	int ShowDialog(HINSTANCE appInstance, HWND hWnd);
	BOOL ProcessMessages(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool &CallDefault);

private:
	std::wstring Title;
	std::wstring Message;
	int JCR[4];

	void Init(HWND hWnd);
	void ButtonN_Click(int index);
	void JCREdit_Change(void);
	void Slider_Move(void);

	acfc::CEditBox JCREdit;
	acfc::CNumberStr JCRString;
	acfc::CSlider Slider;
	
	HWND hJCREdit;
	HWND hStaticText;
	HWND hSlider;
	HWND Button[3];

	bool SuspendUpdate;

};