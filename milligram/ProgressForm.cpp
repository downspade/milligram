#include "stdafx.h"
#include "ProgressForm.h"
#define MIN_LOOP 50

CProgressForm *ProgressForm;

// フォームイベントプロシージャ
BOOL CALLBACK ProgressFormWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	bool CallDefault = false;
	LRESULT result;
	result = ProgressForm->ProcessMessages(hWnd, message, wParam, lParam, CallDefault);
	if (CallDefault)
	{
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return result;
}

BOOL CProgressForm::ProcessMessages(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool & CallDefault)
{
	CallDefault = false;
	switch (message)
	{
	case WM_INITDIALOG:
		Init(hWnd);
		break;

	case WM_CLOSE:
		EndDialog(hWnd, IDOK);
		return(TRUE);

	case WM_SHOWWINDOW:
		if(wParam)Start();
		break;

	case WM_TIMER:
		if (LoopTimer.IsThis(wParam))UpdateText();
		break;

	}
	CallDefault = true;
	return(FALSE);
}

void CProgressForm::Init(HWND hWnd)
{
	SetHWND(hWnd);
	hStaticText = GetDlgItem(handle, ID_PROGRESSF_STATIC);
	SearchEnd = true;
}

void CProgressForm::SetData(std::wstring path, bool subFolder, std::wstring fileMaskString)
{
	RootPath = path;
	SubFolder = subFolder;
	FileMaskString = fileMaskString;
}

void CProgressForm::GetData(std::vector<CImageInfo>& Data)
{
	std::copy(Dest.begin(), Dest.end(), std::back_inserter(Data));
}

int CProgressForm::ShowDialog(HINSTANCE appInstance, HWND hWnd)
{
	return (DialogBox(appInstance, TEXT("IDD_PROGRESSFORM"), hWnd, ProgressFormWndProc));
}

DWORD WINAPI ThreadSearch(LPVOID data)
{
	CProgressForm *ProgressForm = (CProgressForm *)data;
	ProgressForm->Search();
	ExitThread(TRUE);
}

void CProgressForm::Start(void)
{
	if (SearchEnd == false)return;
	Dest.clear();
	SearchedFolder.clear();
	FoundFiles.clear();
	SearchEnd = false;
	SetWindowText(hStaticText, RootPath.c_str());

	hThread = CreateThread(NULL, 0, ThreadSearch, this, 0, &ThreadID);
	InitializeCriticalSection(&CriticalSection);

	LoopTimer.Init(handle, MIN_LOOP);
	LoopTimer.Enabled(true);
	UpdateText();
}

void CProgressForm::UpdateText(void)
{
	DWORD dwParam;
	GetExitCodeThread(hThread, &dwParam);
	if (dwParam == STILL_ACTIVE)
	{
		EnterCriticalSection(&CriticalSection);
		SetWindowText(hStaticText, SearchingFile.c_str());
		SendMessage(handle, WM_PAINT, 0, 0);
		LeaveCriticalSection(&CriticalSection);
	}
	else
		End();
}

void CProgressForm::End(void)
{
	DeleteCriticalSection(&CriticalSection);
	CloseHandle(hThread);
	SendMessage(handle, WM_CLOSE, 0, 0);
}

void CProgressForm::Search(void)
{
	SearchingFile = RootPath;
	SearchFolder(RootPath);
	SearchEnd = true;
}

void CProgressForm::SearchFolder(std::wstring FolderPath)
{

	if (SearchEnd || SearchedFolder.find(FolderPath) != SearchedFolder.end()) return;

	if ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0){SearchEnd = true; return;}

	std::vector<std::wstring> Files;
	std::vector<std::wstring> Folders;

	SearchedFolder.insert(FolderPath);

	HANDLE hFind;
	WIN32_FIND_DATA win32fd;

	std::wstring SearchName = FolderPath + L"\\*.*";
	std::wstring Target;

	hFind = FindFirstFile(SearchName.c_str(), &win32fd);

	if (hFind == INVALID_HANDLE_VALUE)return;

	/* 指定のディレクトリ以下のファイル名をファイルがなくなるまで取得する */
	do
	{
		if (SearchEnd)return;
		if ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0) { SearchEnd = true; return; }

		if ((win32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		{
			Target = FolderPath + TEXT("\\") + win32fd.cFileName;

			/* ファイルが見つかったらVector配列に保存する */
			if (FoundFiles.find(Target) == FoundFiles.end())
			{
				FoundFiles.insert(Target);

				struct _stati64 st;
				_wstat64(Target.c_str(), &st);

				CImageInfo newII;
				newII.FileName = Target;
				newII.FileSize = (int)st.st_size;
				newII.Timestamp = st.st_ctime;
				newII.Rotate = -1;

				Dest.push_back(newII);

				if (Dest.size() % 10 == 0)
				{
					EnterCriticalSection(&CriticalSection);
					SearchingFile = Target;
					LeaveCriticalSection(&CriticalSection);
				}
			}
		}
		else if (SubFolder && (StrCmp(win32fd.cFileName, TEXT(".")) != 0 && StrCmp(win32fd.cFileName, TEXT("..")) != 0))
		{
			Folders.push_back(FolderPath + TEXT("\\") + win32fd.cFileName);
		}

	} while (FindNextFile(hFind, &win32fd));

	FindClose(hFind);

	if (SubFolder)
	{
		for (int i = 0; i < (int)Folders.size(); i++)
		{
			if (SearchEnd)return;
			if ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0) { SearchEnd = true; return; }

			SearchFolder(Folders[i]);
		}
	}
}
