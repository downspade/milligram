#pragma once

#include <windows.h>
#include "resource.h"
#include "acfc.h"

class CInputForm : public acfc::CBaseWindow
{
public:
	void SetData(std::wstring atitile, std::wstring ames, std::wstring atext);
	INT_PTR ShowDialog(HINSTANCE appInstance, HWND hWnd);
	std::wstring Result;

	BOOL ProcessMessages(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool &CallDefault);
private:
	std::wstring Title;
	std::wstring Message;
	std::wstring Text;

	void Init(HWND hWnd);
	void ButtonOK_Click(void);

	HWND hStaticText;
	HWND hEdit;
};