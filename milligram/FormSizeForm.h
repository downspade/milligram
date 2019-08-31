#pragma once

#include <windows.h>
#include "resource.h"
#include "acfc.h"

class CFormSizeForm : public acfc::CBaseWindow
{
public:
	CFormSizeForm(void);

	void SetData(std::vector<std::wstring> &Str, int w, int h);
	INT_PTR ShowDialog(HINSTANCE appInstance, HWND hWnd);
	void GetData(int &w, int &h);

	void Init(HWND hWnd);

	std::vector<std::wstring> StrData;
	int WidthData;
	int HeightData;

	BOOL ProcessMessages(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool &CallDefault);

	void WidthEdit_Changed(void);
	void HeightEdit_Changed(void);
	void WidthEdit_Leave(void);
	void HeightEdit_Leave(void);

	void ButtonOK_Click(void);

private:
	HWND hWidthEdit;
	HWND hHeightEdit;

	WNDPROC hWidthNEOrgProc;
	WNDPROC hHeightNEOrgProc;
	
	HWND hWidthText;
	HWND hHeightText;

	acfc::CEditBox WidthEdit;
	acfc::CEditBox HeightEdit;
	acfc::CNumberStr WidthStr;
	acfc::CNumberStr HeightStr;

	double Ratio;

	bool SuspendUpdate;

};

extern CFormSizeForm *FormSizeForm;