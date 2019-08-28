#pragma once

#include <windows.h>
#include "resource.h"
#include "acfc.h"
#include "spiplugin.h"
#include <set>
using namespace milligram;

class CProgressForm : public acfc::CBaseWindow
{
public:
	void SetData(std::wstring path, bool subFolder, std::wstring fileMaskString);
	void GetData(std::vector<CImageInfo> &Data);
	INT_PTR ShowDialog(HINSTANCE appInstance, HWND hWnd);
	void Start(void);
	void UpdateText(void);
	void End(void);
	void Search(void);
	void SearchFolder(std::wstring folder);
	BOOL ProcessMessages(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool &CallDefault);
	void Init(HWND hWnd);

private:
	std::wstring RootPath;
	bool SubFolder;
	std::wstring FileMaskString;

	std::set<std::wstring>SearchedFolder;
	std::set<std::wstring>FoundFiles;

	std::vector<CImageInfo> Dest;

	std::wstring SearchingFile;

	DWORD ThreadID;
	CRITICAL_SECTION CriticalSection = {};
	HANDLE hThread;
	bool SearchEnd;

	HWND hStaticText;

	acfc::CTimer LoopTimer;
};
