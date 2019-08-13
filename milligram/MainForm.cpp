#include "stdafx.h"
#include "MainForm.h"
#include "InputForm.h"
#include "FormSizeForm.h"
#include "JpegSettingForm.h"
#include "ProgressForm.h"
#include "resource.h"

#define RECT_W(src) ((src).right - (src).left)
#define RECT_H(src) ((src).bottom - (src).top)
//---------------------------------------------------------------------------------------------------

CMainForm *MainForm = nullptr;
extern CInputForm *InputForm;
extern CFormSizeForm *FormSizeForm;
extern CJpegSettingForm *JpegSettingForm;
extern CProgressForm *ProgressForm;

//---------------------------------------------------------------------------------------------------

// フォームイベントプロシージャ
LRESULT CALLBACK MainFormWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	bool CallDefault = false;
	LRESULT result;
	result = MainForm->ProcessMessages(hWnd, message, wParam, lParam, CallDefault);
	if (CallDefault)
	{
		return (DefWindowProc(hWnd, message, wParam, lParam));
	}
	return (result);
}

// リストボックスイベントプロシージャ
LRESULT WINAPI ListBoxProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	bool CallDefault = false;
	LRESULT result;
	result = MainForm->ProcessMessagesListBox(hWnd, message, wParam, lParam, CallDefault);
	if (CallDefault)
	{
		return (DefWindowProc(hWnd, message, wParam, lParam));
	}
	return (result);
}

//---------------------------------------------------------------------------------------------------

LRESULT CALLBACK ProgressCallback(int nNum, int nDenom, long lData)
{
	return(MainForm->SpiProgressCallBack(nNum, nDenom, lData));
}

//---------------------------------------------------------------------------------------------------

unsigned __stdcall ThreadGIFAnime(void *data)
{
	CMainForm *MainForm = (CMainForm *)data;
	MainForm->GifAnimeTimer();
	ExitThread(TRUE);
}


LRESULT CMainForm::SpiProgressCallBack(int nNum, int nDenom, long lData)
{
	time_t Now = timeGetTime();
	if (SlideShow == false)
	{
		switch (LoadState)
		{
		case 0:
			if (nNum == 0) break;
			if (nNum > 0 && nNum < nDenom)
			{
				if (Now - LoadStart * nDenom / nNum > 500)
				{
					StartnNum = nNum;
					LoadState = 1;
				}
				else if (Now - LoadStart > 100) LoadState = 2;
			}
			break;

		case 1:
		{
			double P = (double)(nNum - StartnNum) / (nDenom - StartnNum);

			if (P - ProgressRatio > 0.05)
			{
				ProgressRatio = P;
				if (LoadState == 1)
				{
					Gdiplus::Graphics g(hWindow);
					SolidBrush brush(DrawColor);

					int barWidth = (int)(ProgressRatio * WWidth);
					Point offset{ 0, 0 };
					if (FullScreen == false)
					{
						if (WLeft >= FrameWidth)
							offset.X += FrameWidth;
						else
							offset.X += WLeft;

						if (WTop >= FrameWidth)
							offset.Y += FrameWidth;
						else
							offset.Y += WTop;


						g.FillRectangle(&brush, offset.X, WHeight - 2 + offset.Y, barWidth, 2);
					}
					else
					{
						g.FillRectangle(&brush, WLeft - MLeft, WTop + WHeight - 2 - MLeft, barWidth, 2);
					}

					DeleteObject(&brush);
				}

			}
		}
		break;

		case 2:
			break;
		}
	}

	if ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0)
	{
		while ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0);
		return (1);
	}

	return (0);
}


CMainForm::CMainForm(void)
{
}

CMainForm::~CMainForm(void)
{
}

void CMainForm::Close(void)
{
	SendMessage(hWindow, WM_CLOSE, 0, 0);
}

bool CMainForm::Initialize(HINSTANCE hInstance, int nCmdShow, LPWSTR lpCmdLine)
{
	if (Initialized == true) return(true);
	Initialized = false;

	appInstance = hInstance;

	LoadString(appInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING); // タイトルの読み込み
	LoadString(appInstance, IDC_MILLIGRAM, szWindowClass, MAX_LOADSTRING);

	// TODO:マニフェストのほうが安定する
	//HRESULT hres = SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);

	MyRegisterClass(); // フォームを作る準備

	GetDesktopRect(); // デスクトップ矩形の計算

	LoadResource(); // リソースの読み込み

	std::vector<std::wstring> ExeParam;
	std::vector<std::wstring> DropLists;
	std::map<std::wstring, std::wstring> IniMap;

	int i;
	bool CmdLine = false;
	std::wstring Temp;

	IniFolderName = acfc::GetSpecialFolderPath(CSIDL_LOCAL_APPDATA); // IniFolder の場所を得ておく

	// 引数を読み込む
	// 実行ファイル名も取得
	TCHAR* lpCommandLine = GetCommandLine();
	acfc::ParseCommanLine(ExeParam, lpCommandLine);

	ExeFileName = ExeParam[0]; // 実行ファイル名の設定
	
	// ダイアログの準備
	PrepareDialog();

	// Susie 関係の準備
	WheelPos = WheelSensitivity / 2;

	// 引数のチェック
	if (ExeParam.size() > 1)
	{
		int Len;
		for (i = 1; i < (int)ExeParam.size(); i++)
		{
			if (ExeParam[i][0] == TEXT('-'))
			{
				SetInstanceMode(ExeParam[i].substr(1, ExeParam[i].length() - 1));
			}
			else
			{
				Temp = ExeParam[i];
				Len = Temp.length();
				if (Len > 5)
					if (Temp.substr(Len - 4, 4) == TEXT(".lnk"))
						Temp = acfc::GetFileFromLink(Temp);

				DropLists.push_back(Temp);
				CmdLine = true;
			}
		}
	}

	// ------------- ini ファイルの読み込み --------------
	LoadIni(IniParamName, CmdLine);

	if (!CreateForm(nCmdShow))return (false); // 実際にフォームを作成

	Susie.Init(hWindow, (FARPROC)ProgressCallback);
	Susie.FormHandle = hWindow;
	Susie.InitSize(200, 200);

	CorrectWindow();

	if (CmdLine == true) // 幅と高さが決まったのでここで場所を設定
	{
		POINT CurPos;
		GetCursorPos(&CurPos);
		Left = CurPos.x - Width / 2;
		Top = CurPos.y - Height / 2;
		SetCenter(CurPos.x, CurPos.y);
	}
	GetMonitorParameter();

	// ファイルヒストリの処理
	ConvertHistoryMenu();

	int OLeft = WLeft, OTop = WTop;

	SetTrayIcon(ESetTrayIconMode_ADD);

	if (InstanceMode == false)
		SetMenuEnabled(hPopupMenu, ID_OPTION_DELETEINSTANCE, false);

	if (SpiPathes.size() == 0)
		SpiPathes.push_back(acfc::GetFolderName(ExeFileName));

	Susie.SetSpiPathes(SpiPathes);      // SPI path set

	if (CmdLine)
	{
		AddHistoryList(DropLists);
		OpenFiles(DropLists);
	}

	if (FileList.size() > 0) // 前回のファイルを読み込んでいるかコマンドライン
	{
		if (ShowIndex >= 0 && CmdLine == false)
		{
			ReloadMode = true;

			if (ShowIndexBack >= 0) // アーカイブファイル中のファイルを表示する場合
			{
				if (ShowIndexBack >= (int)FileList.size() || acfc::FileExists(FileList[ShowIndexBack].FileName) == false)
				{
					ShowIndexBack = -1;
				}
			}

			if (ShowIndexBack < 0)
			{
				ShowAbsoluteImage(ShowIndex, 1);
			}
			else
			{
				ShowAbsoluteImage(ShowIndexBack, ShowIndex, 1, true);
			}

			if (ReloadMode)
			{
				WLeft = OLeft;
				WTop = OTop;
				SyncWindow();
			}
			ReloadMode = false;
		}
		if (ShowingList == true) SetFileList();
	}
	else
	{
		ShowIndex = -1;
		ShowIndexBack = -1;
	}

	if (ShowIndex < 0)
	{
		CheckMonitorIni();

		if (FixSizeRatio == true)
		{
			WWidth = (int)(WWidth * SizeRatio);
			WHeight = (int)(WHeight * SizeRatio);
		}
		else
			SizeRatio = 1;

		if (CenterX - WWidth / 2 < MLeft)
			WLeft = MLeft;
		else if (CenterX + WWidth / 2 > MRight)
			WLeft = MRight - WWidth;
		else
			WLeft = CenterX - WWidth / 2;

		if (CenterY - WHeight / 2 < MTop)
			WTop = MTop;
		else if (CenterY + WHeight / 2 > MBottom)
			WTop = MBottom - WHeight;
		else
			WTop = CenterY - WHeight / 2;

		SetCenter(WLeft + WWidth / 2, WTop + WHeight / 2);
		SyncWindow();
	}

	// ini ファイルを読み込んだり各種初期化をする

	DrawColor = GetDrawColor(FullFillColor);

	// メニューを修正
	SyncMenuChecked();

	// ウィンドウをここで表示
	ShowWindow(hWindow, nCmdShow);
	UpdateWindow(hWindow);
	acfc::SetAbsoluteForegroundWindow(hWindow);

	DragAcceptFiles(hWindow, true);

	CursorTimer.Enabled(true);

	Initialized = true;
	return(true);
}

// フォームの初期化など
bool CMainForm::CreateForm(int nCmdShow)
{
	if (HideTaskButton == true)
	{
		hWindow = CreateWindowEx(WS_EX_TOOLWINDOW, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, appInstance, nullptr);
	}
	else
	{
		hWindow = CreateWindowEx(WS_EX_LAYERED, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, appInstance, nullptr);
	}

	SyncWindow();

	if (!hWindow)return (false);

	SetWindowLong(hWindow, GWL_STYLE, WS_POPUP);
	SetWindowPos(hWindow, NULL, 0, 0, Width, Height, (SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOMOVE));

	return (true);
}

ATOM CMainForm::MyRegisterClass(void)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc = MainFormWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = appInstance;
	wcex.hIcon = LoadIcon(appInstance, MAKEINTRESOURCE(IDI_MILLIGRAM));
	wcex.hCursor = nullptr;
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, (TEXT("IDI_ICON")));

	return RegisterClassExW(&wcex);
}

// Dialog を準備する
void CMainForm::PrepareDialog(void)
{
	OpenFileDialog.Filter = LoadStringResource(IDS_RES_2001);
	OpenFileDialog.Title = LoadStringResource(IDS_RES_2002);
	OpenFileDialog.ofn.Flags |= OFN_ALLOWMULTISELECT;

	OpenPluginDialog.Filter = LoadStringResource(IDS_RES_2003);
	OpenPluginDialog.Title = LoadStringResource(IDS_RES_2004);

	SavemflDialog.Filter = LoadStringResource(IDS_RES_2005);
	SavemflDialog.Title = LoadStringResource(IDS_RES_2006);
	SavemflDialog.DefaultExt = TEXT(".mfl");

	SaveLnkDialog.Filter = LoadStringResource(IDS_RES_2007);
	SaveLnkDialog.Title = LoadStringResource(IDS_RES_2008);
	SaveLnkDialog.DefaultExt = TEXT(".ini");

	SaveJpegDialog.Filter = LoadStringResource(IDS_RES_2009);
	SaveJpegDialog.Title = LoadStringResource(IDS_RES_2010);
	SaveJpegDialog.DefaultExt = TEXT(".lnk");

	SaveIniDialog.Filter = LoadStringResource(IDS_RES_2011);
	SaveIniDialog.Title = LoadStringResource(IDS_RES_2012);
	SaveIniDialog.DefaultExt = TEXT(".jpg");

	SavePNGDialog.Filter = LoadStringResource(IDS_RES_2013);
	SavePNGDialog.Title = LoadStringResource(IDS_RES_2014);
	SavePNGDialog.DefaultExt = TEXT(".png");

	SSTimer.Init(hWindow, 1000);
	CursorTimer.Init(hWindow, 1000);

	ColorDialog.SetCustomColor(StockColor);
}

// FullFillColor から DrawColor を得る
Gdiplus::Color CMainForm::GetDrawColor(Gdiplus::Color color)
{
	int R, G, B, M;
	B = color.GetB();
	G = color.GetG();
	R = color.GetR();
	M = R;

	if (G * 3 / 2 > M) M = G * 3 / 2;
	if (B / 2 > M) M = B / 2;
	if (M < 128)
	{
		R += 96; if (R > 255) R = 255;
		G += 96; if (G > 255) G = 255;
		B += 96; if (B > 255) B = 255;
	}
	else
	{
		R -= 96; if (R < 0) R = 0;
		G -= 96; if (G < 0) G = 0;
		B -= 96; if (B < 0) B = 0;
	}

	return (Gdiplus::Color(255, R, G, B));
}



// リソースをメモリに読み込む 
void CMainForm::LoadResource(void)
{
	hParentPopupMenu = LoadMenu(appInstance, MAKEINTRESOURCE(IDR_POPUPMENU));
	hPopupMenu = GetSubMenu(hParentPopupMenu, 0);
	hParentMoveMenu = GetSubMenu(hParentPopupMenu, 1);

	hCursor[0] = LoadCursor(appInstance, TEXT("IDC_NONE"));
	hCursor[1] = LoadCursor(appInstance, TEXT("IDC_HAND_0"));
	hCursor[2] = LoadCursor(appInstance, TEXT("IDC_GRAB_0"));

	hTrayIcon[0] = LoadIcon(appInstance, TEXT("IDI_TRAYICON_0"));
	hTrayIcon[1] = LoadIcon(appInstance, TEXT("IDI_TRAYICON_1"));
	hTrayIcon[2] = LoadIcon(appInstance, TEXT("IDI_TRAYICON_2"));
	hTrayIcon[3] = LoadIcon(appInstance, TEXT("IDI_TRAYICON_3"));
	hTrayIcon[4] = LoadIcon(appInstance, TEXT("IDI_TRAYICON_4"));
	hTrayIcon[5] = LoadIcon(appInstance, TEXT("IDI_TRAYICON_5"));
}

// フォームが作られたときに呼ばれる
void CMainForm::CreateFromMessag(HWND hwnd, LPCREATESTRUCT lp)
{
	CreateDisplayBox(hwnd, lp);
}


void CMainForm::CloseFromMessage(void)
{
	// トレイアイコンを削除
	SetTrayIcon(ESetTrayIconMode_DELETE);

	// フォントオブジェクトがあった場合に削除
	if (hFont != nullptr)DeleteObject(hFont);

	EndGIFAnimeThread();

	if (NotSaveIni == false) SaveIni(IniParamName);
}


// ファイルを表示するリストを作成する
void CMainForm::CreateDisplayBox(HWND hwnd, LPCREATESTRUCT lp)
{
	DisplayBox.Init(hwnd, lp, (LONG)ListBoxProcedure, true);
	DisplayBox.SetPosition(0, 0, Width, Height);
}

// トレイアイコンを登録する
void CMainForm::SetTrayIcon(ESetTrayIconMode Mode)
{
	std::wstring Mes = BalloonTipTitle;

	// 構造体のセット
	NOTIFYICONDATA nid = { 0 };
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.uFlags = (NIF_ICON | NIF_MESSAGE | NIF_TIP);
	nid.uID = ID_TRAYICON;      // アイコン識別子の定数
	nid.hWnd = hWindow;             // ウインドウのハンドル
	nid.hIcon = hTrayIcon[TrayIconColor];            // アイコンのハンドル
	nid.uCallbackMessage = ID_TRAYICON;      // 通知メッセージの定数

	if (Mes.length() > 63)Mes = Mes.substr(0, 63);
	lstrcpy(nid.szTip, Mes.c_str());       // チップヘルプの文字列

	// ここでアイコンの追加
	switch (Mode)
	{
		case ESetTrayIconMode_ADD:
			Shell_NotifyIcon(NIM_ADD, &nid);
			break;
		case ESetTrayIconMode_MODIFY:
			Shell_NotifyIcon(NIM_MODIFY, &nid);
			break;
		case ESetTrayIconMode_DELETE:
			Shell_NotifyIcon(NIM_DELETE, &nid);
			break;
	}

}



// インスタンスモードを設定する
bool CMainForm::SetInstanceMode(std::wstring NewIniParamName)
{
	bool NewMode = (NewIniParamName != TEXT("milligram"));
	IniParamName = NewIniParamName;

	InstanceMode = NewMode;
	SetMenuEnabled(hPopupMenu, ID_OPTION_DELETEINSTANCE, InstanceMode);

	if (NewMode)
	{
		BalloonTipTitle = TEXT("milligram - ") + IniParamName;
	}
	else
	{
		BalloonTipTitle = TEXT("milligram image viewer");
	}
	SetTrayIcon(ESetTrayIconMode_MODIFY);

	return (true);
}



// Ini ファイルを読み込む
bool CMainForm::LoadIni(std::wstring IniName, bool CmdLine)
{
	std::wstring PathName, Temp;
	std::map<std::wstring, std::wstring> Map;

	int i;
	int iColor;

	if (
		(((GetAsyncKeyState(VK_LCONTROL) & 0x8000) & (GetAsyncKeyState(VK_LSHIFT) & 0x8000))
			| ((GetAsyncKeyState(VK_RCONTROL) & 0x8000) & (GetAsyncKeyState(VK_RSHIFT) & 0x8000)))
		)
	{
		MessageBox(hWindow, LoadStringResource(IDS_MES_1002).c_str(), TEXT("Information"), MB_OK | MB_ICONINFORMATION);
		return (true);
	}

	PathName = IniFolderName + TEXT("\\milligram\\") + IniName + TEXT(".ini");

	if (!acfc::FileExists(PathName))
	{
		if (InstanceMode == false) DoInstall();
		SaveIni(IniName);
		return (true);
	}
	else
	{
		IniFileTime = GetCreationTime(PathName);
	}

	if(acfc::LoadMapFromFile(Map, PathName) == false)return(false);

	if (acfc::GetIntegerValue(Map, TEXT("IniFileVersion"), 0, 0, 0) != 2) return (false);

	ShowIndex = acfc::GetIntegerValue(Map, TEXT("ShowIndex"), 0, 0, 0);
	ShowIndexBack = acfc::GetIntegerValue(Map, TEXT("ShowIndexBack"), 0, 0, 0);

	AlwaysTop = acfc::GetBoolValue(Map, TEXT("AlwaysTop"), false);

	HideTaskButton = acfc::GetBoolValue(Map, TEXT("HideTaskButton"), true);

	if (CmdLine || acfc::GetBoolValue(Map, TEXT("ShowingList"), true) == false) ToggleShowList(EShowMode_PICTURE);

	SlideShow = acfc::GetBoolValue(Map, TEXT("SlideShow"), false);
	SSInterval = acfc::GetIntegerValue(Map, TEXT("SSInterval"), SSInterval, 0, 0);

	iColor = acfc::GetIntegerValue(Map, TEXT("FullFillColor"), FullFillColor.ToCOLORREF(), 0, 0);
	if (iColor < 0) iColor = 0;
	if (iColor > 0xFFFFFF) iColor = 0xFFFFFF;
	FullFillColor.SetFromCOLORREF(iColor);
	DrawColor = GetDrawColor(FullFillColor).ToCOLORREF();

	FrameWidth = acfc::GetIntegerValue(Map, TEXT("FrameWidth"), FrameWidth, 0, 0);

	iColor = acfc::GetIntegerValue(Map, TEXT("FrameColor"), 0xFFFFFF, 0, 0);
	if (iColor < 0) iColor = 0;
	if (iColor > 0xFFFFFF) iColor = 0xFFFFFF;
	FrameColor.SetFromCOLORREF(iColor);

	FormAlphaBlendValue = acfc::GetIntegerValue(Map, TEXT("AlphaBlendValue"), FormAlphaBlendValue, 1, 255);

	ApplicationFontSize = acfc::GetIntegerValue(Map, TEXT("FontSize"), ApplicationFontSize, 5, 500);

	FixSizeRatio = acfc::GetBoolValue(Map, TEXT("FixSizeRatio"), false);
	PositionMode = (EPositionMode)acfc::GetIntegerValue(Map, TEXT("PositionMode"), (int)PositionMode, 0, 0);

	RotateValue = acfc::GetIntegerValue(Map, TEXT("RotateValue"), RotateValue, 0, 0);
	FixRotate = acfc::GetBoolValue(Map, TEXT("FixRotate"), false);
	Susie.FixRotate = FixRotate;

	KeepDiagonalLength = acfc::GetIntegerValue(Map, TEXT("DiagonalLength"), KeepDiagonalLength, 0, 0);
	FixDiagonalLength = acfc::GetBoolValue(Map, TEXT("FixDiagonalLength"), false);

	FitToScreen = acfc::GetBoolValue(Map, TEXT("FitToScreen"), FitToScreen);
	UseWholeScreen = acfc::GetBoolValue(Map, TEXT("UseWholeScreen"), UseWholeScreen);
	FullScreen = acfc::GetBoolValue(Map, TEXT("FullScreen"), FullScreen);

	SearchSubFolder = acfc::GetBoolValue(Map, TEXT("SearchSubFolder"), SearchSubFolder);

	LoadLastFile = acfc::GetBoolValue(Map, TEXT("LoadLastFile"), LoadLastFile);

	KeepPreviousPosition = acfc::GetBoolValue(Map, TEXT("KeepPreviousPosition"), KeepPreviousPosition);

	WheelSensitivity = acfc::GetIntegerValue(Map, TEXT("WheelSensitivity"), WheelSensitivity, 1, 60000000);

	Locked = acfc::GetBoolValue(Map, TEXT("Locked"), Locked);

	TrayIconColor = acfc::GetIntegerValue(Map, TEXT("TrayIconColor"), TrayIconColor, 0, 0);

	UseCursorSize = acfc::GetIntegerValue(Map, TEXT("UseCursorSize"), UseCursorSize, 0, 0);
	RWidth = RHeight = WWidth = WHeight = Width = Height = 200 + UseCursorSize * 100;
	MnCursorSize_Click(UseCursorSize);

	ShortCutFileName = acfc::GetStringValue(Map, TEXT("ShortCutFileName"), ShortCutFileName);

	CreateSendToLink = acfc::GetBoolValue(Map, TEXT("CreateSendToLink"), CreateSendToLink);

	JCR[0] = acfc::GetIntegerValue(Map, TEXT("JpegCompressRatio0"), 85, 1, 100);
	JCR[1] = acfc::GetIntegerValue(Map, TEXT("JpegCompressRatio1"), 25, 1, 100);
	JCR[2] = acfc::GetIntegerValue(Map, TEXT("JpegCompressRatio2"), 60, 1, 100);
	JCR[3] = acfc::GetIntegerValue(Map, TEXT("JpegCompressRatio3"), 85, 1, 100);

	FileMaskString = acfc::GetStringValue(Map, TEXT("FileMaskString"), FileMaskString);
	EnableFileMask = acfc::GetBoolValue(Map, TEXT("EnableFileMask"), EnableFileMask);

	AutoLoadFileFolder = acfc::GetBoolValue(Map, TEXT("AutoLoadFileFolder"), AutoLoadFileFolder);

	MaxHistoryNum = acfc::GetIntegerValue(Map, TEXT("MaxHistoryNum"), (int)MaxHistoryNum, 0, 50);

	for (i = 0; i < 16; i++)
	{
		StockColor[i] = acfc::GetIntegerValue(Map, TEXT("StockColor") + std::to_wstring(i), 0, 0, 0);
	}

	//------- 表示位置、サイズに関するデータはなるべく最後の方で読み込む

	CenterX = acfc::GetIntegerValue(Map, TEXT("CenterX"), CenterX, 0, 0);
	CenterY = acfc::GetIntegerValue(Map, TEXT("CenterY"), CenterY, 0, 0);
	WLeft = acfc::GetIntegerValue(Map, TEXT("FormLeft"), WLeft, 0, 0);
	WTop = acfc::GetIntegerValue(Map, TEXT("FormTop"), WTop, 0, 0);
	SizeRatio = acfc::GetDoubleValue(Map, TEXT("SizeRatio"), SizeRatio, 0, 0);

	//---------------------------------------------------------------------

	i = 0;
	while (true)
	{
		Temp = acfc::GetStringValue(Map, TEXT("SpiPath") + std::to_wstring(i), TEXT(""));
		if (Temp == TEXT("")) break;
		SpiPathes.push_back(Temp);
		i++;
	}

	Susie.InternalLoader = acfc::GetStringValue(Map, TEXT("InternalLoader"), Susie.InternalLoader);

	i = 0;
	while (true)
	{
		if (i >= MaxHistoryNum) break;
		Temp = acfc::GetStringValue(Map, TEXT("History") + std::to_wstring(i), TEXT(""));
		if (Temp == TEXT("")) break;
		HistoryList.push_back(Temp);
		i++;
	}

	if ((((GetAsyncKeyState(VK_LCONTROL) & 0x8000) | (GetAsyncKeyState(VK_RCONTROL) & 0x8000)) == 0)
		&& CmdLine == false && LoadLastFile == true)
	{
		if (ShowIndex < 0) ShowIndex = 0;
		LoadFileList(FileList, ArchiveFileList, Map);
		DisplayList = FileList;
		if (ShowIndexBack < 0)
		{
			if (FileList.size() == 0 || acfc::FileExists(FileList[ShowIndex].FileName) == false) ShowIndex = -1;
		}
		else
		{
			i = IndexOfImageInfos(ArchiveFileList, FileList[ShowIndexBack].FileName);
			if (i < 0 || acfc::FileExists(FileList[ShowIndexBack].FileName) == false)
			{
				ShowIndexBack = -1;
			}
		}
	}
	else
	{
		ShowIndex = -1;
	}

	return (true);
}


void CMainForm::SyncMenuChecked(void)
{
	if (AlwaysTop)MnAlwaysTop_Click();
	if (FitToScreen)MnFitToScreen_Click();
	if (UseWholeScreen)MnUseWholeScreen_Click();
	if (FixDiagonalLength)MnFixDiagonalLength_Click(false);
	if (FixSizeRatio)MnFixRatio_Click();
	if (FullScreen)MnFullScreen_Click();
	if (Locked)MnLock_Click();
	
	if (FixRotate)MnRotateFix_Click();
	
	if (HideTaskButton)MnHideTaskButton_Click();
	if (SearchSubFolder)MnSearchSubFolders_Click();
	if (LoadLastFile)MnKeepPreviousFiles_Click();
	if (KeepPreviousPosition)MnKeepPreviousPosition_Click();
	if (EnableFileMask)MnEnableFileMask_Click();
	if (AutoLoadFileFolder)MnAutoLoadFileFolder_Click();

	AbsoluteRotate(0);
	MnTrayIcon_Click(0);
	MnSlideShowInterval_Click(0);
	MnBorder_Click(0);
	MnPositionMode_Click(0);
	MnRotateMode_Click(0);
	MnWheelSensitivity_Click(0);
	MnFontSize_Click(0);
	MnTransparency_Click(0);
	MnCursorSize_Click(0);
}

int  CMainForm::GetMenuIndexByValue(int *ValueList, int Value, int Length)
{
	int i;
	for (i = 0; i < Length; i++)
	{
		if (Value == ValueList[i])return(i);
	}
	return(-1);
}

// Ini ファイルを保存する
bool CMainForm::SaveIni(std::wstring IniName)
{
	std::wstring PathName;
	std::wstring sb;
	int i;

	PathName = IniFolderName + TEXT("\\milligram\\") + IniName + TEXT(".ini");

	sb.append(TEXT("IniFileVersion=2\n"));

	sb.append(TEXT("CenterX=") + std::to_wstring(CenterX) + TEXT("\n"));
	sb.append(TEXT("CenterY=") + std::to_wstring(CenterY) + TEXT("\n"));
	sb.append(TEXT("FormLeft=") + std::to_wstring(WLeft) + TEXT("\n"));
	sb.append(TEXT("FormTop=") + std::to_wstring(WTop) + TEXT("\n"));

	sb.append(TEXT("ShowIndex=") + std::to_wstring(ShowIndex) + TEXT("\n"));
	sb.append(TEXT("ShowIndexBack=") + std::to_wstring(ShowIndexBack) + TEXT("\n"));

	sb.append(TEXT("AlwaysTop=") + acfc::BoolToString(AlwaysTop) + TEXT("\n"));
	sb.append(TEXT("HideTaskButton=") + acfc::BoolToString(HideTaskButton) + TEXT("\n"));
	sb.append(TEXT("ShowingList=") + acfc::BoolToString(ShowingList) + TEXT("\n"));
	sb.append(TEXT("SlideShow=") + acfc::BoolToString(SlideShow) + TEXT("\n"));
	sb.append(TEXT("SSInterval=") + std::to_wstring(SSInterval) + TEXT("\n"));

	sb.append(TEXT("FrameWidth=") + std::to_wstring(FrameWidth) + TEXT("\n"));
	sb.append(TEXT("FrameColor=") + std::to_wstring(FrameColor.ToCOLORREF()) + TEXT("\n"));

	sb.append(TEXT("AlphaBlendValue=") + std::to_wstring(FormAlphaBlendValue) + TEXT("\n"));

	sb.append(TEXT("FontSize=") + std::to_wstring(ApplicationFontSize) + TEXT("\n"));

	sb.append(TEXT("FixSizeRatio=") + acfc::BoolToString(FixSizeRatio) + TEXT("\n"));
	sb.append(TEXT("FitToScreen=") + acfc::BoolToString(FitToScreen) + TEXT("\n"));
	sb.append(TEXT("PositionMode=") + std::to_wstring(PositionMode) + TEXT("\n"));
	sb.append(TEXT("FixRotate=") + acfc::BoolToString(FixRotate) + TEXT("\n"));
	sb.append(TEXT("RotateValue=") + std::to_wstring(RotateValue) + TEXT("\n"));
	sb.append(TEXT("WheelSensitivity=") + std::to_wstring(WheelSensitivity) + TEXT("\n"));
	sb.append(TEXT("FixDiagonalLength=") + acfc::BoolToString(FixDiagonalLength) + TEXT("\n"));
	sb.append(TEXT("DiagonalLength=") + std::to_wstring(KeepDiagonalLength) + TEXT("\n"));
	sb.append(TEXT("UseWholeScreen=") + acfc::BoolToString(UseWholeScreen) + TEXT("\n"));

	sb.append(TEXT("FullScreen=") + acfc::BoolToString(FullScreen) + TEXT("\n"));
	sb.append(TEXT("SizeRatio=") + std::to_wstring(SizeRatio) + TEXT("\n"));
	sb.append(TEXT("Locked=") + acfc::BoolToString(Locked) + TEXT("\n"));

	sb.append(TEXT("SearchSubFolder=") + acfc::BoolToString(SearchSubFolder) + TEXT("\n"));
	sb.append(TEXT("LoadLastFile=") + acfc::BoolToString(LoadLastFile) + TEXT("\n"));
	sb.append(TEXT("KeepPreviousPosition=") + acfc::BoolToString(KeepPreviousPosition) + TEXT("\n"));
	sb.append(TEXT("TrayIconColor=") + std::to_wstring(TrayIconColor) + TEXT("\n"));
	sb.append(TEXT("UseCursorSize=") + std::to_wstring(UseCursorSize) + TEXT("\n"));
	sb.append(TEXT("FullFillColor=") + std::to_wstring(FullFillColor.ToCOLORREF()) + TEXT("\n"));
	sb.append(TEXT("UseCursorSize=") + std::to_wstring(UseCursorSize) + TEXT("\n"));
	sb.append(TEXT("ShortCutFileName=") + ShortCutFileName + TEXT("\n"));
	sb.append(TEXT("CreateSendToLink=") + acfc::BoolToString(CreateSendToLink) + TEXT("\n"));

	sb.append(TEXT("JpegCompressRatio0=") + std::to_wstring(JCR[0]) + TEXT("\n"));
	sb.append(TEXT("JpegCompressRatio1=") + std::to_wstring(JCR[1]) + TEXT("\n"));
	sb.append(TEXT("JpegCompressRatio2=") + std::to_wstring(JCR[2]) + TEXT("\n"));
	sb.append(TEXT("JpegCompressRatio3=") + std::to_wstring(JCR[3]) + TEXT("\n"));

	sb.append(TEXT("FileMaskString=") + FileMaskString + TEXT("\n"));
	sb.append(TEXT("EnableFileMask=") + acfc::BoolToString(EnableFileMask) + TEXT("\n"));
	sb.append(TEXT("AutoLoadFileFolder=") + acfc::BoolToString(AutoLoadFileFolder) + TEXT("\n"));

	for (i = 0; i < (int)SpiPathes.size(); i++)
		sb.append(TEXT("SpiPath") + std::to_wstring(i) + TEXT("=") + SpiPathes[i] + TEXT("\n"));

	sb.append(TEXT("InternalLoader=") + Susie.InternalLoader + TEXT("\n"));

	for (i = 0; i < (int)HistoryList.size(); i++)
		sb.append(TEXT("History") + std::to_wstring(i) + TEXT("=") + HistoryList[i] + TEXT("\n"));

	for (i = 0; i < 16; i++)
		sb.append(TEXT("StockColor") + std::to_wstring(i) + TEXT("=") + std::to_wstring(StockColor[i]) + TEXT("\n"));


	if (LoadLastFile == true)
		sb.append(CreateFileList());

	if (acfc::FolderExists(IniFolderName + TEXT("\\milligram")) == false)
		CreateDirectory((LPCWSTR)(IniFolderName + TEXT("\\milligram")).c_str(), nullptr);

	acfc::SaveTextFile(PathName, sb);

	IniFileTime = GetCreationTime(PathName);

	return (true);
}

// ファイルの作成時刻を取得する
__time64_t CMainForm::GetCreationTime(std::wstring FileName)
{
	struct __stat64 st;
	_wstat64(FileName.c_str(), &st);
	return(st.st_ctime);
}

// ファイルリストを作る
std::wstring CMainForm::CreateFileList(void)
{
	int i, j;
	std::wstring sb;

	for (i = 0; i < (int)FileList.size(); i++)
	{
		sb.append(TEXT("FileList") + std::to_wstring(i) + TEXT("=")
			+ TEXT("Name=") + FileList[i].FileName
			+ TEXT("\tTime=") + std::to_wstring(FileList[i].Timestamp)
			+ TEXT("\tSize=") + std::to_wstring(FileList[i].FileSize)
			+ TEXT("\tRotate=") + std::to_wstring(FileList[i].Rotate) + TEXT("\n"));
	}

	for (j = 0; j < (int)ArchiveFileList.size(); j++)
	{
		sb.append(TEXT("Archive") + std::to_wstring(j) + TEXT("=") + ArchiveFileList[j].FileName + TEXT("\n"));

		std::vector<CImageInfo> tempIIL = ArchiveFileList[j].ImageInfoList;
		for (i = 0; i < (int)tempIIL.size(); i++)
		{
			sb.append(TEXT("ArchiveList") + std::to_wstring(j) + TEXT("-") + std::to_wstring(i) + TEXT("=")
				+ TEXT("Name=") + tempIIL[i].FileName
				+ TEXT("\tTime=") + std::to_wstring(tempIIL[i].Timestamp)
				+ TEXT("\tSize=") + std::to_wstring(tempIIL[i].FileSize)
				+ TEXT("\tRotate=") + std::to_wstring(tempIIL[i].Rotate) + TEXT("\n"));
		}
	}
	return (sb);
}

// ファイルリストを保存する
bool CMainForm::SaveFileList(std::wstring FileName)
{
	std::wstring sb = TEXT("");

	sb = CreateFileList();

	acfc::SaveTextFile(FileName, sb);

	MessageBox(hWindow, LoadStringResource(IDS_MES_1001).c_str(), TEXT("Information"), MB_OK | MB_ICONINFORMATION);

	return (true);
}



bool CMainForm::LoadFileList(std::vector<CImageInfo>& DestSL, std::vector<CImageInfo>& ArcLists, std::map<std::wstring, std::wstring>& Map)
{
	int i, j, n;
	std::wstring Temp, FName, FTime, FSize, FRotate;

	std::map<std::wstring, std::wstring>::iterator itr;

	i = 0;
	while ((itr = Map.find(TEXT("FileList") + std::to_wstring(i))) != Map.end())
	{
		if ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) > 0) break;

		Temp = itr->second;

		FTime = TEXT("0");
		FSize = TEXT("0");
		FRotate = TEXT("-1");

		FName = GetTabString(Temp, TEXT("Name"));
		FTime = GetTabString(Temp, TEXT("Time"));
		FSize = GetTabString(Temp, TEXT("Size"));
		FRotate = GetTabString(Temp, TEXT("Rotate"));

		if (EnableFileMask && acfc::FitsMasks(FName, FileMaskString) == false)
		{
			i++;
			continue;
		}

		CImageInfo NewII;
		NewII.FileName = FName;
		NewII.Timestamp = acfc::GetIntegerValue(FTime);
		NewII.FileSize = acfc::GetIntegerValue(FSize);
		NewII.Rotate = acfc::GetIntegerValue(FRotate);
		DestSL.push_back(NewII);
		i++;
	}

	n = 0;
	while ((itr = Map.find(TEXT("Archive") + std::to_wstring(n))) != Map.end())
	{
		if ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) > 0) break;
		
		Temp = itr->second;

		if (EnableFileMask && acfc::FitsMasks(Temp, FileMaskString) == false)
		{
			i++;
			continue;
		}

		if (IndexOfImageInfos(DestSL, Temp) >= 0)
		{
			CImageInfo NewII;
			NewII.FileName = Temp;
			ArcLists.push_back(NewII);
		}
		n++;
	}

	for (j = 0; j < (int)ArcLists.size(); j++)
	{
		i = 0;
		while ((itr = Map.find(TEXT("ArchiveList") + std::to_wstring(j) + TEXT("-") + std::to_wstring(i))) != Map.end())
		{
			if ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) > 0) break;

			FTime = TEXT("0");
			FSize = TEXT("0");
			FRotate = TEXT("-1");

			FName = GetTabString(Temp, TEXT("Name"));
			FTime = GetTabString(Temp, TEXT("Time"));
			FSize = GetTabString(Temp, TEXT("Size"));
			FRotate = GetTabString(Temp, TEXT("Rotate"));

			if (EnableFileMask && acfc::FitsMasks(FName, FileMaskString) == false)
			{
				i++;
				continue;
			}

			CImageInfo NewII;
			NewII.FileName = FName;
			NewII.Timestamp = acfc::GetIntegerValue(FTime);
			NewII.FileSize = acfc::GetIntegerValue(FSize);
			NewII.Rotate = acfc::GetIntegerValue(FRotate);
			ArcLists[j].ImageInfoList.push_back(NewII);
			i++;
		}
	}
	return (true);
}






bool CMainForm::AddHistoryList_(std::wstring &FileName)
{
	auto i = std::find(HistoryList.begin(), HistoryList.end(), FileName);
	
	if (i != HistoryList.end()) HistoryList.erase(i);

	HistoryList.insert(HistoryList.begin(), FileName);

	while (MaxHistoryNum < (int)HistoryList.size())
	{
		HistoryList.resize(MaxHistoryNum);
	}
	return (true);
}

bool CMainForm::AddHistoryList(std::wstring &FileName)
{
	AddHistoryList_(FileName);
	ConvertHistoryMenu();
	return (true);
}

bool CMainForm::AddHistoryList(std::vector<std::wstring> &FileNames)
{
	int i = 0;
	if (MaxHistoryNum < (int)FileNames.size())
	{
		i = FileNames.size() - MaxHistoryNum;
	}

	while (i < (int)FileNames.size())
	{
		AddHistoryList_(FileNames[i]);
		i++;
	}

	ConvertHistoryMenu();
	return (true);
}


// ヒストリーをメニューに変換する
bool CMainForm::ConvertHistoryMenu(void)
{
	int i;
	if (HistoryList.size() > 0)
	{
		SetMenuEnabled(hPopupMenu, ID_POPUP_HISTORY, true);
		
		if (hHistoryMenu != nullptr)DestroyMenu(hHistoryMenu);
		hHistoryMenu = CreateMenu();

		MENUITEMINFO menuItem = { 0 };
		menuItem.cbSize = sizeof(MENUITEMINFO);
		menuItem.fMask = MIIM_TYPE | MIIM_ID;
		menuItem.fType = MFT_STRING;
		UINT wID = ID_HISTORY_BASE;
		int Index = 0;

		for (i = 0; i < (int)HistoryList.size(); i++)
		{
			menuItem.dwTypeData = BufferString((TCHAR *)HistoryList[i].c_str());
			menuItem.wID = wID;
			InsertMenuItem(hHistoryMenu, Index, TRUE, &menuItem);
			wID++;
			Index++;
		}

		MENUITEMINFO mii = { 0 };
		mii.cbSize = sizeof(MENUITEMINFO);
		mii.fMask = MIIM_SUBMENU;
		GetMenuItemInfo(hPopupMenu, ID_POPUP_HISTORY, FALSE, &mii);
		mii.hSubMenu = hHistoryMenu;
		SetMenuItemInfo(hPopupMenu, ID_POPUP_HISTORY, FALSE, &mii); // これで初めて有効になる
	}
	else
	{
		SetMenuEnabled(hPopupMenu, ID_POPUP_HISTORY, false);
	}

	if (RefreshHistoryMode == 0)
	{
		SaveIni(IniParamName);
	}
	else
	{
		RefreshHistoryMode = 0;
	}
	return (true);
}




bool CMainForm::DoInstall(void)
{
	NoStayOnTop();
	SaveLnkDialog.InitialDirectory = acfc::GetSpecialFolderPath(CSIDL_DESKTOP);
	SaveLnkDialog.FileName = SaveLnkDialog.InitialDirectory + TEXT("\\milligram image viewer.lnk");

	std::wstring mes = LoadStringResource(IDS_MES_1003);

	MessageBox(hWindow, LoadStringBuffer(IDS_MES_1003), TEXT("Information"), MB_OK | MB_ICONINFORMATION);

	if (SaveLnkDialog.ShowDialog(hWindow))
	{
		ShortCutFileName = SaveLnkDialog.FileName;
		acfc::CreateLink(ExeFileName, ShortCutFileName, TEXT(""));
	}
	DoCreateSendToLink(TEXT("milligram"));

	RestoreStayOnTop();
	return (true);
}


bool CMainForm::CreateShortCut(std::wstring InstanceName)
{
	NoStayOnTop();
	SaveLnkDialog.InitialDirectory = acfc::GetSpecialFolderPath(CSIDL_DESKTOP);

	if (InstanceName != TEXT("milligram"))
	{
		SaveLnkDialog.FileName = SaveLnkDialog.InitialDirectory + TEXT("\\milligram image viewer - ") + InstanceName + TEXT(".lnk");

		MessageBox(hWindow, LoadStringResource(IDS_MES_1004).c_str(), TEXT("Information"), MB_OK | MB_ICONINFORMATION);

		if (SaveLnkDialog.ShowDialog(hWindow))
		{
			ShortCutFileName = SaveLnkDialog.FileName;
			acfc::CreateLink(ExeFileName, ShortCutFileName, TEXT("\"-") + InstanceName + TEXT("\""));
			InstanceMode = true;
		}
	}
	else
	{
		SaveLnkDialog.FileName = SaveLnkDialog.InitialDirectory + TEXT("\\milligram image viewer.lnk");

		MessageBox(hWindow, LoadStringResource(IDS_MES_1005).c_str(), TEXT("Information"), MB_OK | MB_ICONINFORMATION);

		if (SaveLnkDialog.ShowDialog(hWindow))
		{
			ShortCutFileName = SaveLnkDialog.FileName;
			acfc::CreateLink(ExeFileName, SaveLnkDialog.FileName, TEXT(""));
			InstanceMode = false;
		}
	}

	RestoreStayOnTop();
	return (true);
}

bool CMainForm::DoCreateSendToLink(std::wstring InstanceName)
{
	int Result;
	Result = MessageBox(hWindow, LoadStringResource(IDS_MES_1006).c_str(), TEXT("Confirmation"), MB_YESNO | MB_ICONQUESTION);

	if (Result == IDYES)
	{
		std::wstring Temp, TargetFolder;
		TargetFolder = acfc::GetSpecialFolderPath(CSIDL_SENDTO);

		if (InstanceName == TEXT("milligram"))
			Temp = TargetFolder + TEXT("\\milligram image viewer.lnk");
		else
			Temp = TargetFolder + TEXT("\\milligram ") + InstanceName + TEXT(".lnk");

		if (acfc::FileExists(Temp) == true)
		{
			Result = MessageBox(hWindow, LoadStringResource(IDS_MES_1007).c_str(), TEXT("Confirmation"), MB_YESNO | MB_ICONQUESTION);
		}

		if (Result == IDYES)
		{
			acfc::CreateLink(ExeFileName, Temp, TEXT(""));
			CreateSendToLink = true;
		}
	}

	if (Result == IDNO) return (false);
	return (true);
}

bool CMainForm::DoUninstall(void)
{
	bool Result = true;
	std::map<std::wstring, std::wstring> IniMap;
	std::map<std::wstring, std::wstring>::iterator itr;
	std::wstring Temp = IniFolderName + TEXT("\\milligram");
	int i;

	std::vector<std::wstring> Files;
	acfc::GetFiles(Files, Temp, TEXT("*.ini"));

	for (i = 0; i < (int)Files.size(); i++)
	{
		acfc::LoadMapFromFile(IniMap, Files[i]);

		itr = IniMap.find(TEXT("ShortCutFileName"));
		if (itr != IniMap.end())
		{
			Temp = itr->second;

			if (acfc::FileExists(Temp))
				DeleteFile(Temp.c_str());
		}

		itr = IniMap.find(TEXT("CreateSendToLink"));
		if (itr != IniMap.end())
		{
			Temp = itr->second;

			if (acfc::LowerCase(Temp) == TEXT("true"))
			{
				Temp = acfc::ChangeFileExt(acfc::GetFileName(Files[i]), TEXT(""));
				Result = Result && DeleteSendToLink(Temp);
			}
		}
	}

	Temp = IniFolderName + TEXT("\\milligram");
	acfc::DeleteFolder(Temp);

	return (Result);
}

bool CMainForm::DeleteSendToLink(std::wstring InstanceName)
{
	std::wstring TargetFolder = acfc::GetSpecialFolderPath(CSIDL_SENDTO);;
	std::wstring Temp;

	if (InstanceName == TEXT("milligram"))
		Temp = (std::wstring)TargetFolder + TEXT("\\milligram image viewer.lnk");
	else
		Temp = (std::wstring)TargetFolder + TEXT("\\milligram ") + InstanceName + TEXT(".lnk");

	if (acfc::FileExists(Temp))
		DeleteFile(Temp.c_str());
	else
		return (false);
	return (true);
}






// メインメッセージ処理
LRESULT CMainForm::ProcessMessages(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool &CallDefault)
{
	switch (message)
	{
		case WM_COMMAND:
			{
				CallDefault = false;
				int wmId = LOWORD(wParam);
				// Parse the menu selections:

				if (wmId >= ID_TRAYICONCOLOR_MIN && wmId <= ID_TRAYICONCOLOR_MAX)
				{
					MnTrayIcon_Click(wmId - ID_TRAYICONCOLOR_MIN);

				}
				else if (wmId >= ID_SLIDESHOWINTERVAL_MIN && wmId <= ID_SLIDESHOWINTERVAL_MAX)
				{
					MnSlideShowInterval_Click(wmId - ID_SLIDESHOWINTERVAL_MIN);
				}
				else if (wmId >= ID_BORDERWIDTH_MIN && wmId <= ID_BORDERWIDTH_MAX)
				{
					MnBorder_Click(wmId - ID_BORDERWIDTH_MIN);
				}
				else if (wmId >= ID_SHOWPOSITION_MIN && wmId <= ID_SHOWPOSITION_MAX)
				{
					MnPositionMode_Click(wmId - ID_SHOWPOSITION_MIN);
				}
				else if (wmId >= ID_ROTATE_MIN && wmId <= ID_ROTATE_MAX)
				{
					if (wmId >= ID_ROTATE_NONE && wmId <= ID_ROTATE_TRUNEDLEFT)
						AbsoluteRotate(wmId - ID_ROTATE_NONE);
					else if (wmId == ID_ROTATE_TURNRIGHT || wmId == ID_ROTATE_TURNLEFT)
						MnRotateOffset_Click(wmId - ID_ROTATE_TURNRIGHT);
					else if (wmId == ID_ROTATE_FIXROTATION)
						MnRotateFix_Click();
					else
						MnRotateMode_Click(wmId - ID_ROTATE_MIN);
				}
				else if (wmId >= ID_WHEELSENSITIVITY_MIN && wmId <= ID_WHEELSENSITIVITY_MAX)
				{
					MnWheelSensitivity_Click(wmId - ID_WHEELSENSITIVITY_MIN);
				}
				else if (wmId >= ID_FONTSIZE_MIN && wmId <= ID_FONTSIZE_MAX)
				{
					MnFontSize_Click(wmId - ID_FONTSIZE_MIN);

				}
				else if (wmId >= ID_TRANSPARENCY_MIN && wmId <= ID_TRANSPARENCY_MAX)
				{
					MnTransparency_Click(wmId - ID_TRANSPARENCY_MIN);
				}
				else if (wmId >= ID_SORTFILELIST_MIN && wmId <= ID_SORTFILELIST_MAX)
				{
					MnSortFileList_Click(wmId - ID_SORTFILELIST_MIN);
				}
				else if (wmId >= ID_CURSORSIZE_MIN && wmId <= ID_CURSORSIZE_MAX)
				{
					MnCursorSize_Click(wmId - ID_CURSORSIZE_MIN);

				}
				else if (wmId >= ID_FILE_MOVE_BASE && wmId <= ID_FILE_MOVE_BASE + (int)MoveData.size())
				{
					MnFileMove_Click(wmId - ID_FILE_MOVE_BASE);
				}
				else if (wmId >= ID_HISTORY_BASE && wmId <= ID_HISTORY_BASE + MaxHistoryNum)
				{
					MnHisotryMenu_Click(wmId - ID_HISTORY_BASE);
				}
				else
				{
					switch (wmId)
					{
					case ID_POPUP_TOGGLEIMAGELIST:
						MnToggleShow_Click();
						break;

					case ID_POPUP_FULLSCREEN:
						MnFullScreen_Click();
						break;

					case ID_POPUP_USEWHOLESCREEN:
						MnUseWholeScreen_Click();
						break;

					case ID_POPUP_FITTOSCREEN:
						MnFitToScreen_Click();
						break;

					case ID_POPUP_SHOWINSCREEN:
						MnInScreen_Click();
						break;

					case ID_POPUP_SHOWPIXELBYPIXEL:
						MnShowPbyP_Click();
						break;

					case ID_POPUP_MOVETOCENTER:
						MnCenter_Click();
						break;

					case ID_POPUP_ALWAYSONTOP:
						MnAlwaysTop_Click();
						break;

					case ID_POPUP_FIXDIAGONALLENGTH:
						MnFixDiagonalLength_Click(true);
						break;

					case ID_POPUP_FIXSIZERATIO:
						MnFixRatio_Click();
						break;

					case ID_POPUP_SETWINDOWSIZE:
						MnSetWindowSize_Click();
						break;

					case ID_POPUP_SLIDESHOW:
						MnSlideShow_Click();
						break;

					case ID_POPUP_REFRESH:
						MnRefresh_Click();
						break;

					case ID_POPUP_LOCK:
						MnLock_Click();
						break;

					case ID_POPUP_SHOWHIDE:
						MnToggleVisible_Click();
						break;

					case ID_POPUP_OPENFILES:
						MnOpenFile_Click();
						break;

					case ID_POPUP_OPENFOLDER:
						MnOpenFolder_Click();
						break;

					case ID_POPUP_OPENEXISTSFOLDERFILES:
						MnOpenFolderExistingShowingFile_Click();
						break;

					case ID_POPUP_CLOSEARCHIVE:
						MnCloseArchive_Click();
						break;

					case ID_POPUP_SHOWFILEINFORMATION:
						MnShowInformation_Click();
						break;

					case ID_POPUP_COPYIMAGETOCLIPBOARD:
						MnCopyImage_Click();
						break;

					case ID_FILE_COPY:
						MnFileCopy_Click();
						break;

					case ID_FILE_CUT:
						MnFileCut_Click();
						break;

					case ID_FILE_PASTE:
						MnFilePaste_Click();
						break;

					case ID_FILE_RENAME:
						MnFileRename_Click();
						break;

					case ID_FILE_COPYFILEPATH:
						MnCopyFilePath_Click();
						break;

					case ID_FILE_MOVE:
						MnMoveFile_Click();
						break;

					case ID_FILE_MOVETORECYCLE:
						MnFileMoveToRecycle_Click(true);
						break;

					case ID_FILE_OPENEXISTSFOLDER:
						MnOpenExistsFolder_Click();
						break;

					case ID_FILE_SAVEJPEGFILE:
						MnJpegSave_Click();
						break;

					case ID_FILE_SAVEJPEGFILESHOWINGSIZE:
						MnJpegSaveShowingSize_Click();
						break;

					case ID_FILE_JPEGSAVESETTING:
						MnJpegSaveSetting_Click();
						break;

					case ID_FILE_SAVEPNG:
						MnSavePNG_Click();
						break;

					case ID_OPTION_SEARCHSUBFOLDERS:
						MnSearchSubFolders_Click();
						break;

					case ID_OPTION_KEEPPREVIOUSFILES:
						MnKeepPreviousFiles_Click();
						break;

					case ID_OPTION_ENABLEFILEMASK:
						MnEnableFileMask_Click();
						break;

					case ID_OPTION_FILEMASKSETTING:
						MnFileMaskSetting_Click();
						break;

					case ID_OPTION_AUTOLOADFILEFOLDER:
						MnAutoLoadFileFolder_Click();
						break;

					case ID_OPTION_KEEPPREVIOUSPOSITION:
						MnKeepPreviousPosition_Click();
						break;

					case ID_OPTION_BACKGROUNDCOLOR:
						MnBackGroundColor_Click();
						break;

					case ID_OPTION_HIDETASKBUTTON:
						MnHideTaskButton_Click();
						break;

					case ID_OPTION_BORDERCOLOR:
						MnBorderColor_Click();
						break;

					case ID_PLUGIN_SETPLUGINFOLDER:
						MnSetPluginFolder_Click();
						break;

					case ID_PLUGIN_OPENPLUGINDIALOG:
						MnOpenPluginDialog_Click();
						break;

					case ID_POPUP_SAVEFILELIST:
						MnSave_Click();
						break;

					case ID_PLUGIN_INTERNALLOADEREXTENSION:
						MnInternalLoader_Click();
						break;

					case ID_OPTION_CREATEINSTANCE:
						MnCreateInstance_Click();
						break;

					case ID_OPTION_DELETEINSTANCE:
						MnDeleteInstance_Click();
						break;

					case ID_OPTION_UNINSTALL:
						MnUninstall_Click();
						break;

					case ID_POPUP_QUIT:
						MnQuit_Click();
						break;

					default:
						CallDefault = true;
						return 0;
					}
				}
			}
			break;

		case WM_CREATE:
			CreateFromMessag(hWnd, (LPCREATESTRUCT)(lParam));
			break;
		
		case WM_CLOSE:
			CloseFromMessage();
			CallDefault = true;
			break;

		case WM_PAINT:
			OnPaint();
			CallDefault = ShowingList;
			break;

		case WM_ERASEBKGND:
			CallDefault = ShowingList;
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		case WM_SHOWWINDOW:
			Visible = (wParam == TRUE);
			CallDefault = true;
			break;

		case WM_SIZE:
			if (wParam == SC_MINIMIZE)
				Visible = false;
			else if (wParam == SC_RESTORE)
				Visible = true;

			CallDefault = true;
			break;

		case WM_MOUSEMOVE:
			OnMouseMove();
			break;

		case WM_LBUTTONDOWN:
			OnMouseDown(EEventButton_LEFT);
			break;

		case WM_RBUTTONDOWN:
			OnMouseDown(EEventButton_RIGHT);
			break;

		case WM_MBUTTONDOWN:
			OnMouseDown(EEventButton_MIDDLE);
			break;

		case WM_LBUTTONUP:
			OnMouseUp(EEventButton_LEFT);
			break;

		case WM_RBUTTONUP:
			OnMouseUp(EEventButton_RIGHT);
			break;

		case WM_MBUTTONUP:
			OnMouseUp(EEventButton_MIDDLE);
			break;

		case WM_LBUTTONDBLCLK:
			MainForm_MouseDoubleClick();
			break;

		case WM_MOUSEWHEEL:
			OnMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam));
			break;

		case WM_MOUSEHOVER:
			MouseEnter();
			break;

		case WM_MOUSELEAVE:
			MouseLeave();
			break;

		case WM_KEYDOWN:
			MainForm_KeyDown(wParam, lParam);
			break;

		case WM_TIMER:
			if (SSTimer.IsThis(wParam))SSTimer_Tick();
			else if (CursorTimer.IsThis(wParam))CursorTimer_Tick();
			break;
		
		case WM_DROPFILES:
			File_DragDrop((HDROP)wParam);
			break;

		case ID_TRAYICON:
			if (wParam == ID_TRAYICON)
			{       // アイコンの識別コード
				switch (lParam)
				{
					case WM_LBUTTONUP:
						TrayIcon_MouseClick(EEventButton_LEFT);
						break;
					case WM_RBUTTONUP:
						TrayIcon_MouseClick(EEventButton_RIGHT);
						break;
					case WM_MBUTTONUP:
						TrayIcon_MouseClick(EEventButton_MIDDLE);
						break;
					default:
						CallDefault = true;
						break;
				}
			}
			break;

		default:
			CallDefault = true;
			return 0;
	}
	return 0;
}

// キー入力チェック
void CMainForm::MainForm_KeyDown(WPARAM wParam, LPARAM lParam)
{
	if (Locked) return;

	int SubKey = 0;

	if (ShowingList == true)return(DisplayBox_KeyDown(wParam, lParam));

	if (((GetAsyncKeyState(VK_LSHIFT) & 0x8000) | (GetAsyncKeyState(VK_RSHIFT) & 0x8000))) SubKey |= 1;
	if (((GetAsyncKeyState(VK_LCONTROL) & 0x8000) | (GetAsyncKeyState(VK_RCONTROL) & 0x8000))) SubKey |= 2;
	if (((GetAsyncKeyState(VK_LMENU) & 0x8000) | (GetAsyncKeyState(VK_RMENU) & 0x8000))) SubKey |= 4;

	switch (wParam)
	{
	case VK_SPACE:
		if (SubKey == 0)
			ShowOffsetImage(1);
		else if (SubKey == 1)
			ShowOffsetImage(-1);
		else if (SubKey == 2)
			MnToggleVisible_Click();
		break;

	case VK_DOWN:
		ShowOffsetImage(1);
		break;
	case VK_UP:
		ShowOffsetImage(-1);
		break;
	case VK_NEXT:
		ShowOffsetImage(10);
		break;
	case VK_PRIOR:
		ShowOffsetImage(-10);
		break;
	case VK_HOME:
		ShowAbsoluteImage(0, 1);
		break;
	case VK_END:
		ShowAbsoluteImage(DisplayList.size() - 1, -1);
		break;

	case VK_RETURN:
		ToggleShowList(EShowMode_LIST);
		break;

	case VK_DELETE:
		MnFileMoveToRecycle_Click(false);
		break;

	case VK_F5:
		ShowAbsoluteImage(ShowIndex, 0);
		break;

	case VK_OEM_PLUS:
	case VK_ADD:
		ZoomImage(SizeRatio * 1.5);
		break;
	case VK_OEM_MINUS:
	case VK_SUBTRACT:
		ZoomImage(SizeRatio / 1.5);
		break;
	case VK_MULTIPLY:
	case VK_OEM_5:
		MnInScreen_Click();
		break;

	case 'O':
		if (SubKey == 2)
			MnOpenExistsFolder_Click();
		else
			MnOpenFile_Click();
		break;

	case 'N':
		if (SubKey == 2)
			MnFileRename_Click();
		break;

	case VK_OEM_2:
		MnShowPbyP_Click();
		break;
	case VK_DIVIDE:
		MnShowPbyP_Click();
		break;
	case 'A':
		MnShowPbyP_Click();
		break;

	case 'S':
		if (SubKey == 0)
			MnInScreen_Click();
		else if (SubKey == 2)
			MnSave_Click();
		else if (SubKey == 1)
			MnJpegSave_Click();
		else if (SubKey == 3)
			MnJpegSaveShowingSize_Click();
		break;

	case 'C':
		if (SubKey == 0)
			MnCenter_Click();
		else if (SubKey == 1)
			MnCopyImage_Click();
		else if (SubKey == 2)
			MnFileCopy_Click();
		else if (SubKey == 3)
			MnCopyFilePath_Click();
		break;

	case 'T':
		if (SubKey == 0)
			MnAlwaysTop_Click();
		else if (SubKey == 2)
			SortFileList(ESortType_NAME);
		break;

	case 'B':
		MnFixRatio_Click();
		break;

	case 'W':
		MnFitToScreen_Click();
		break;
	case 'H':
		MnUseWholeScreen_Click();
		break;
	case 'F':
		MnFullScreen_Click();
		break;

	case 'D':
		MnSlideShow_Click();
		break;
	case 'R':
		if (SubKey == 0)
			ShowAbsoluteImage(ShowIndex, 0);
		else if (SubKey == 2)
			SortFileList(ESortType_RANDOM);
		break;

	case 'X':
		if (SubKey == 2)
			MnFileCut_Click();
		break;

	case VK_BACK:
		MnCloseArchive_Click();
		break;

	case 'V':
		if (SubKey == 2)
			MnFilePaste_Click();
		break;

	case 'I':
		MnShowInformation_Click();
		break;

	case 'P':
		if (SubKey == 2)
			MnOpenExistsFolder_Click();
		else
			MnOpenFolder_Click();
		break;

	case 'M':
		MnMoveFile_Click();
		break;

	case VK_OEM_COMMA:
		OffsetRotate(-1);
		break;

	case VK_OEM_PERIOD:
		OffsetRotate(1);
		break;

	case VK_ESCAPE:
		Close();
		break;
	}
}

// リストボックスメッセージ処理
LRESULT CMainForm::ProcessMessagesListBox(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool &CallDefault)
{
	switch (message)
	{	
		case WM_MOUSEHOVER:
		case WM_MOUSELEAVE:
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
			ProcessMessages(hWnd, message, wParam, lParam, CallDefault);
			break;
		case WM_LBUTTONDBLCLK:
			DisplayBox_DoubleClick();
			break;
		case WM_KEYDOWN:
			DisplayBox_KeyDown(wParam, lParam);
			break;
		default:
			break;
	}

	return CallWindowProc(DisplayBox.OrgProc, hWnd, message, wParam, lParam);
}

// キー入力チェック
void CMainForm::DisplayBox_KeyDown(WPARAM wParam, LPARAM lParam)
{
	if (ShowingList == false)
	{
		MainForm_KeyDown(wParam, lParam);
		return;
	}

	if (Locked) return;

	int SubKey = 0;

	if (((GetAsyncKeyState(VK_LSHIFT) & 0x8000) | (GetAsyncKeyState(VK_RSHIFT) & 0x8000))) SubKey |= 1;
	if (((GetAsyncKeyState(VK_LCONTROL) & 0x8000) | (GetAsyncKeyState(VK_RCONTROL) & 0x8000))) SubKey |= 2;
	if (((GetAsyncKeyState(VK_LMENU) & 0x8000) | (GetAsyncKeyState(VK_RMENU) & 0x8000))) SubKey |= 4;

	switch (wParam)
	{
	case VK_SPACE:
		if (SubKey == 2)
			MnToggleShow_Click();
		break;

	case VK_RETURN:
		ToggleShowList(EShowMode_PICTURE);
		break;

	case VK_DELETE:
		if (SubKey == 0)
			DeleteFileList(0);
		else if (SubKey == 2)
			DeleteFileList(1);
		else if (SubKey == 3)
			DeleteFileList(2);
		break;

	case VK_DOWN:
		if (SubKey == 4)
			MoveSelectedList(1);
		break;

	case VK_UP:
		if (SubKey == 4)
			MoveSelectedList(-1);
		break;

	case 'O':
		if (SubKey == 2)
			MnOpenExistsFolder_Click();
		else
			MnOpenFile_Click();
		break;

	case 'N':
		if (SubKey == 2)
			MnFileRename_Click();
		break;

	case 'A':
		if (SubKey == 2)
			DisplayBox.SelectAll(true);
		break;

	case 'S':
		if (SubKey == 0)
			MnInScreen_Click();
		else if (SubKey == 2)
			MnSave_Click();
		break;

	case 'C':
		if (SubKey == 0)
			MnCenter_Click();
		else if (SubKey == 2)
			MnFileCopy_Click();
		else if (SubKey == 3)
			MnCopyFilePath_Click();
		break;

	case 'T':
		if (SubKey == 0)
			MnAlwaysTop_Click();
		else if (SubKey == 2)
			SortFileList(ESortType_NAME);
		break;

	case 'F':
		MnFullScreen_Click();
		break;

	case 'X':
		if (SubKey == 2)
			MnFileCut_Click();
		break;

	case VK_BACK:
		MnCloseArchive_Click();
		break;

	case 'V':
		if (SubKey == 2)
			MnFilePaste_Click();
		break;

	case 'I':
		if (SubKey == 2)
		{
			DisplayBox.SelectInvert();
		}
		break;

	case 'D':
		if (SubKey == 2)
			DisplayBox.SelectAll(false);
		break;

	case 'P':
		if (SubKey == 2)
			MnOpenExistsFolder_Click();
		else
			MnOpenFolder_Click();
		break;

	case VK_ESCAPE:
		Close();
		break;
	}
}

// 再描画
void CMainForm::OnPaint(void)
{
	if (EnableDraw > 0 || ShowingList == true) return;

	int i;
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hWindow, &ps);

	BeginUpdate();

	if (Susie.Mode != EPluginMode_NONE) // 画像が描画されている場合
	{
		HDC hDC = GetDC(hWindow);
		// 画像表示
		int OldStretchMode = SetStretchBltMode(hDC, STRETCH_HALFTONE);

		Gdiplus::Rect Src, Dest;
		Gdiplus::Rect wRect = { WLeft, WTop, WWidth, WHeight };

		Point offset{ 0, 0 };

		if (FullScreen == false)
		{
			offset = { -RLeft, -RTop };

			if (WLeft >= FrameWidth)
				offset.X += FrameWidth;
			else if (WLeft > 0)
				offset.X += WLeft;

			if (WTop >= FrameWidth)
				offset.Y += FrameWidth;
			else if (WTop > 0)
				offset.Y += WTop;

		}
		else
		{
			offset = { -MLeft + FrameWidth, -MTop + FrameWidth };
		}


		GetTransRect(wRect, Src, Dest);
		Dest.Offset(offset);

		StretchDIBits(hDC,
			Dest.X, Dest.Y, Dest.Width, Dest.Height,
			Src.X, Susie.SrcRHeight - Src.GetBottom(), Src.Width, Src.Height,
			Susie.pBmpData, Susie.pBmpInfo,
			DIB_RGB_COLORS, SRCCOPY);

		Gdiplus::Graphics g(hWindow);

		if (FrameWidth > 0)
		{
			SolidBrush brush(FrameColor);

			Gdiplus::Rect fRect[4];
			fRect[0] = { WLeft - FrameWidth, WTop - FrameWidth, WWidth + FrameWidth * 2, FrameWidth };
			fRect[1] = { WLeft - FrameWidth, WTop, FrameWidth, WHeight };
			fRect[2] = { WLeft + WWidth, WTop, FrameWidth, WHeight };
			fRect[3] = { WLeft - FrameWidth, WTop + WHeight, WWidth + FrameWidth * 2, FrameWidth };

			for (int i = 0; i < 4;i++)
			{
				fRect[i].Offset(offset);
			}
			g.FillRectangles(&brush, fRect, 4);

			DeleteObject(&brush);
		}

		if (FullScreen == true)
		{
			SolidBrush brush(FullFillColor);

			Dest.X -= FrameWidth;
			Dest.Y -= FrameWidth;
			Dest.Width += FrameWidth * 2;
			Dest.Height += FrameWidth * 2;

			Gdiplus::Rect fRect[4];

			fRect[0] = { 0, 0, Width, Dest.Y };
			fRect[1] = { 0, Dest.GetBottom(), Width, Bottom - Dest.GetBottom() };
			fRect[2] = { 0, Dest.Y, Dest.X, Dest.Height };
			fRect[3] = { Dest.GetRight(), Dest.Y, Right, Dest.Height };

			for (i = 0;i < 4;i++)
			{
				if (fRect[i].IntersectsWith(ClientRect))
				{
					fRect[i].Intersect(ClientRect);
				}
				g.FillRectangles(&brush, fRect, 4);
			}
			DeleteObject(&brush);
		}


		SetStretchBltMode(hDC, OldStretchMode);
		ReleaseDC(hWindow, hDC);

		if (GIFRefresh == true)
		{
			if (Susie.GIFAnimateUpDateFrame(EnableDropFrame) == false) // 次のフレームの準備をする
			{
				Susie.GIFLooping = false;
			}
			GIFRefresh = false;

		}
	}
	else  // 画像が描画されていない場合
	{
		Gdiplus::Rect Dest = { RLeft, RTop, RWidth, RHeight };
		Gdiplus::Rect wRect = { WLeft, WTop, WWidth, WHeight };

		Gdiplus::Point offset = {0, 0};

		if (FullScreen == false)
		{
			offset = { -RLeft, -RTop };

			if (WLeft >= FrameWidth)
				offset.X += FrameWidth;
			else if (WLeft > 0)
				offset.X += WLeft;

			if (WTop >= FrameWidth)
				offset.Y += FrameWidth;
			else if (WTop > 0)
				offset.Y += WTop;

		}
		else
		{
			offset = { -MLeft + FrameWidth, -MTop + FrameWidth };
		}

		Dest.Offset(offset);

		Gdiplus::Graphics g(hWindow);

		SolidBrush brush(FullFillColor);

		{
			Pen pen(DrawColor);
			Gdiplus::Rect fRect = { WLeft,  WTop, WWidth - 1, WHeight - 1};
			fRect.Offset(offset);
			g.DrawRectangle(&pen, fRect);
			g.FillRectangle(&brush, fRect.X + 1, fRect.Y + 1, fRect.Width - 1, fRect.Height - 1);
			DeleteObject(&pen);
		}

		if (FrameWidth > 0)
		{
			brush.SetColor(FrameColor);

			Gdiplus::Rect fRect[4];
			fRect[0] = { WLeft - FrameWidth, WTop - FrameWidth, WWidth + FrameWidth * 2, FrameWidth };
			fRect[1] = { WLeft - FrameWidth, WTop, FrameWidth, WHeight };
			fRect[2] = { WLeft + WWidth, WTop, FrameWidth, WHeight };
			fRect[3] = { WLeft - FrameWidth, WTop + WHeight, WWidth + FrameWidth * 2, FrameWidth };

			for(int i = 0;i < 4;i++)
			{
				fRect[i].Offset(offset);
			}
			g.FillRectangles(&brush, fRect, 4);
		}

		if (FullScreen == true)
		{
			brush.SetColor(FullFillColor);

			Dest.X -= FrameWidth;
			Dest.Y -= FrameWidth;
			Dest.Width += FrameWidth * 2;
			Dest.Height += FrameWidth * 2;

			Gdiplus::Rect fRect[4];

			fRect[0] = { 0, 0, Width, Dest.Y };
			fRect[1] = { 0, Dest.GetBottom(), Width, Bottom - Dest.GetBottom() };
			fRect[2] = { 0, Dest.Y, Dest.X, Dest.Height };
			fRect[3] = { Dest.GetRight(), Dest.Y, Right, Dest.Height };

			for(int i = 0;i < 4;i++)
			{
				if (fRect[i].IntersectsWith(ClientRect))
				{
					fRect[i].Intersect(ClientRect);
				}
				g.FillRectangles(&brush, fRect, 4);
			}
		}
		DeleteObject(&brush);
	}

	// 読み込みラインの表示
	//if (LoadState == 1)
	//{
	//    Graphics g = this.CreateGraphics();
	//    SolidBrush brush = new SolidBrush(FullFillColor);

	//    brush.Color = DrawColor;
	//    int barWidth = (int)(ProgressRatio * WWidth);
	//    Gdiplus::Point offset = new Gdiplus::Point(0, 0);
	//    if (FullScreen == false)
	//    {
	//        if (WLeft >= FrameWidth)
	//            offset.X += FrameWidth;
	//        else
	//            offset.X += WLeft;

	//        if (WTop >= FrameWidth)
	//            offset.Y += FrameWidth;
	//        else
	//            offset.Y += WTop;


	//        g.FillRectangle(brush, offset.X, WHeight - 2 + offset.Y, barWidth, 2);
	//    }
	//    else
	//    {
	//        g.FillRectangle(brush, WLeft - MLeft, WTop + WHeight - 2 - MLeft, barWidth, 2);
	//    }

	//    brush.Dispose();
	//    g.Dispose();
	//}

	if (SSIcon >= 0) DrawSSIcon();
	EndUpdate();
}


// 転送矩形を得る
bool CMainForm::GetTransRect(Gdiplus::Rect &wRect, Gdiplus::Rect &Src, Gdiplus::Rect &Dest)
{
	Gdiplus::Rect clip, temp;
	double dl, dt, dr, db;
	double sl, st, sr, sb;
	double rt;

	Src = { 0, 0, Susie.SrcRWidth, Susie.SrcRHeight };
	Dest = wRect;
	rt = (double)(Dest.Width) / (Src.Width);

	if (FullScreen == true)
	{
		clip = ClientRect;
		clip.X = Left;
		clip.Y = Top;
	}
	else
	{
		clip = Desktop;
	}

	//上下左右何ピクセルカットされたかを得る
	temp = Dest;
	temp.Intersect(clip);

	dl = temp.X - Dest.X;
	dr = Dest.GetRight() - temp.GetRight();
	dt = temp.Y - Dest.Y;
	db = Dest.GetBottom() - temp.GetBottom();

	// ソースから何ピクセルカットするかを得る
	sl = floor(dl / rt);
	sr = floor(dr / rt);
	st = floor(dt / rt);
	sb = floor(db / rt);

	// 上下左右に何ピクセル戻すかを計算する
	dl = round(sl * rt);
	dr = round(sr * rt);
	dt = round(st * rt);
	db = round(sb * rt);

	// それぞれソースに反映
	Src.X += (int)sl;
	Src.Width -= (int)sl;
	Src.Y += (int)st;
	Src.Height -= (int)st;
	Src.Width -= (int)sr;
	Src.Height -= (int)sb;

	Dest.X += (int)dl;
	Dest.Width -= (int)dl;
	Dest.Y += (int)dt;
	Dest.Height -= (int)dt;
	Dest.Width -= (int)dr;
	Dest.Height -= (int)db;

	return (true);
}

// スライドショーアイコンを描画する
void CMainForm::DrawSSIcon()
{
	if (WWidth < 20 || WHeight < 20) return;
	Gdiplus::Graphics g(hWindow);
	Gdiplus::Color bgColor(128, FullFillColor.GetR(), FullFillColor.GetG(), FullFillColor.GetB());
	Pen pen(DrawColor, 1);
	SolidBrush brush(DrawColor);
	SolidBrush bgBrush(bgColor);
	Point ofs(RWidth, RHeight);

	Gdiplus::Rect ri, ro, rb;
	Gdiplus::Point p[3];

	switch (UseCursorSize)
	{
	case 0:
		ro = {  -19,  -19, 16, 16 };
		ri = {  -14,  -14, 7, 7 };
		p[0] = {  -14,  -16 };
		p[1] = {  -7,  -11 };
		p[2] = {  -14,  -6 };
		break;
	case 1:
		ro = {  -29,  -29, 24, 24 };
		ri = {  -22,  -22, 11, 11 };
		p[0] = {  -21,  -24 };
		p[1] = {  -11,  -17 };
		p[2] = {  -21,  -10 };
		break;
	case 2:
		ro = {  -39,  -39, 32, 32 };
		ri = {  -30,  -30, 15, 15 };
		p[0] = {  -29,  -33 };
		p[1] = {  -15,  -23 };
		p[2] = {  -29,  -13 };
		break;
	}
	rb = {ro.X - 1, ro.Y - 1, ro.Width + 3, ro.Height + 3};

	if (FullScreen == true)
	{
		ofs = { MWidth, MHeight };
	}

	ro.Offset(ofs);
	ri.Offset(ofs);
	rb.Offset(ofs);
	p[0] = p[0] + ofs;
	p[1] = p[1] + ofs;
	p[2] = p[2] + ofs;

	g.FillRectangle(&bgBrush, rb);
	g.DrawRectangle(&pen, ro);

	if (SlideShow == false)
	{
		g.FillRectangle(&brush, ri);
	}
	else
	{
		g.FillPolygon(&brush, p, 3);
	}
	DeleteObject(&pen);
	DeleteObject(&brush);
	DeleteObject(&bgBrush);
}


// 描画を開始する（再入回避）
void CMainForm::BeginUpdate(void)
{
	EnableDraw++;
}

// 描画を終了する
void CMainForm::EndUpdate(void)
{
	EnableDraw--;
}

// フォームを再描画する
void CMainForm::FormRefresh(void)
{
	if (EnableDraw == 0)SendMessage(hWindow, WM_PAINT, 0, 0);
}


// マウスがフォーム上を移動
void CMainForm::OnMouseMove(void)
{
	StopCount = 0;
	POINT ScCurPos;
	GetCursorPos(&ScCurPos);

	if (Locked)
	{
		PositionType = GetCursorPositionType(ScCurPos);
		return;
	}

	if (Holding == 1)
	{
		double NextWidth = Width, NextHeight = Height, Temp;

		if (PositionType == EMousePositionType_NORMAL)
		{
			RefreshWindowPosition(ScCurPos.x - OfsCurPos.x, ScCurPos.y - OfsCurPos.y);
		}
		else
		{
			if (((GetAsyncKeyState(VK_LCONTROL) & 0x8000) | (GetAsyncKeyState(VK_RCONTROL) & 0x8000)))
			{
				if ((int)PositionType <= (int)EMousePositionType_RB)
				{
					switch (PositionType)
					{
					case EMousePositionType_LT:
						NextWidth = (double)DownWidth + (DownCurPos.x - ScCurPos.x) * 2;
						NextHeight = (double)DownHeight + (DownCurPos.y - ScCurPos.y) * 2;
						break;
					case EMousePositionType_RT:
						NextWidth = (double)DownWidth - (DownCurPos.x - ScCurPos.x) * 2;
						NextHeight = (double)DownHeight + (DownCurPos.y - ScCurPos.y) * 2;
						break;
					case EMousePositionType_LB:
						NextWidth = (double)DownWidth + (DownCurPos.x - ScCurPos.x) * 2;
						NextHeight = (double)DownHeight - (DownCurPos.y - ScCurPos.y) * 2;
						break;
					case EMousePositionType_RB:
						NextWidth = (double)DownWidth - (DownCurPos.x - ScCurPos.x) * 2;
						NextHeight = (double)DownHeight - (DownCurPos.y - ScCurPos.y) * 2;
						break;
					}
					if (NextWidth > NextHeight * Ratio)
						NextHeight = NextWidth / Ratio;
					else
						NextWidth = NextHeight * Ratio;
				}
				else
				{
					switch (PositionType)
					{
					case EMousePositionType_T:
						NextHeight = (double)DownHeight + (DownCurPos.y - ScCurPos.y) * 2;
						NextWidth = NextHeight * Ratio;
						break;
					case EMousePositionType_L:
						NextWidth = (double)DownWidth + (DownCurPos.x - ScCurPos.x) * 2;
						NextHeight = NextWidth / Ratio;
						break;
					case EMousePositionType_R:
						NextWidth = (double)DownWidth - (DownCurPos.x - ScCurPos.x) * 2;
						NextHeight = NextWidth / Ratio;
						break;
					case EMousePositionType_B:
						NextHeight = (double)DownHeight - (DownCurPos.y - ScCurPos.y) * 2;
						NextWidth = NextHeight * Ratio;
						break;
					}
				}

				CheckDiagonalLength(NextWidth, NextHeight);

				WLeft = CenterX + (int)(DownWidth - NextWidth) / 2 - DownWidth / 2;
				WTop = CenterY + (int)(DownHeight - NextHeight) / 2 - DownHeight / 2;
			}
			else
			{
				switch (PositionType)
				{
				case EMousePositionType_LT:
					NextWidth = (double)DownWidth + DownCurPos.x - ScCurPos.x;
					NextHeight = (double)DownHeight + DownCurPos.y - ScCurPos.y;

					if (NextWidth > NextHeight * Ratio)
						NextHeight = NextWidth / Ratio;
					else
						NextWidth = NextHeight * Ratio;

					CheckDiagonalLength(NextWidth, NextHeight);

					WLeft = CenterX + (int)(DownWidth - NextWidth) - DownWidth / 2;
					WTop = CenterY + (int)(DownHeight - NextHeight) - DownHeight / 2;
					break;

				case EMousePositionType_RT:
					NextWidth = (double)DownWidth - DownCurPos.x + ScCurPos.x;
					NextHeight = (double)DownHeight + DownCurPos.y - ScCurPos.y;

					if (NextWidth > NextHeight * Ratio)
						NextHeight = NextWidth / Ratio;
					else
						NextWidth = NextHeight * Ratio;

					CheckDiagonalLength(NextWidth, NextHeight);

					WTop = CenterY + (int)(DownHeight - NextHeight) - DownHeight / 2;
					break;

				case EMousePositionType_LB:
					NextWidth = (double)DownWidth + DownCurPos.x - ScCurPos.x;
					NextHeight = (double)DownHeight - DownCurPos.y + ScCurPos.y;
					if (NextWidth > NextHeight * Ratio)
						NextHeight = NextWidth / Ratio;
					else
						NextWidth = NextHeight * Ratio;

					CheckDiagonalLength(NextWidth, NextHeight);

					WLeft = CenterX + (int)(DownWidth - NextWidth) - DownWidth / 2;
					break;

				case EMousePositionType_RB:
					NextWidth = (double)DownWidth - DownCurPos.x + ScCurPos.x;
					NextHeight = (double)DownHeight - DownCurPos.y + ScCurPos.y;

					if (NextWidth > NextHeight * Ratio)
						NextHeight = NextWidth / Ratio;
					else
						NextWidth = NextHeight * Ratio;

					CheckDiagonalLength(NextWidth, NextHeight);

					break;

				case EMousePositionType_T:
					NextHeight = (double)DownHeight + DownCurPos.y - ScCurPos.y;
					NextWidth = NextHeight * Ratio;

					CheckDiagonalLength(NextWidth, NextHeight);

					Temp = DownWidth - (DownCurPos.x - DownLeft);
					WLeft = (int)(DownLeft + Temp - Temp * NextWidth / DownWidth);
					WTop = ScCurPos.y - OfsCurPos.y;
					WTop = (int)(DownTop + DownHeight - NextHeight);
					break;

				case EMousePositionType_L:
					NextWidth = (double)DownWidth + DownCurPos.x - ScCurPos.x;
					NextHeight = NextWidth / Ratio;

					CheckDiagonalLength(NextWidth, NextHeight);

					Temp = DownHeight - (DownCurPos.y - DownTop);
					WTop = (int)(DownTop + Temp - Temp * NextWidth / DownWidth);
					WLeft = (int)(DownLeft + DownWidth - NextWidth);
					break;

				case EMousePositionType_R:
					NextWidth = (double)DownWidth - DownCurPos.x + ScCurPos.x;
					NextHeight = NextWidth / Ratio;

					CheckDiagonalLength(NextWidth, NextHeight);

					Temp = DownHeight - (DownCurPos.y - DownTop);
					WTop = (int)(DownTop + Temp - Temp * NextWidth / DownWidth);
					break;

				case EMousePositionType_B:
					NextHeight = (double)DownHeight - DownCurPos.y + ScCurPos.y;
					NextWidth = NextHeight * Ratio;

					CheckDiagonalLength(NextWidth, NextHeight);

					Temp = DownWidth - (DownCurPos.x - DownLeft);
					WLeft = (int)(DownLeft + Temp - Temp * NextWidth / DownWidth);
					break;
				}
			}
			WWidth = (int)NextWidth;
			WHeight = (int)NextHeight;
		}

		if (FixDiagonalLength == true)
			KeepDiagonalLength = (int)(sqrt(WWidth * WWidth + WHeight * WHeight));

		SyncWindow();
	}
	else
	{
		PositionType = GetCursorPositionType(ScCurPos);
	}

	PreCurPos = ScCurPos;

}

bool CMainForm::SyncWindow(void)
{
	GetRealWindowRect();

	//if (MustRefresh == false
	//    && Left == RLeft - FrameWidth && Top == RTop - FrameWidth
	//    && Width == RWidth + FrameWidth * 2 && Height == RHeight + FrameWidth * 2) return (false);

	if (FullScreen == false)
	{
		if (ShowingList == false)
		{
			int DefX = 0, DefY = 0;

			if (RLeft - FrameWidth < Desktop.X)
			{
				Left = Desktop.X;
				DefX = Desktop.X - (RLeft - FrameWidth);
				if (DefX > FrameWidth) DefX = FrameWidth;
			}
			else
				Left = RLeft - FrameWidth;

			if (RTop - FrameWidth < Desktop.Y)
			{
				Top = Desktop.Y;
				DefY = Desktop.Y - (RTop - FrameWidth);
				if (DefY > FrameWidth) DefY = FrameWidth;
			}
			else
				Top = RTop - FrameWidth;

			if (RWidth + FrameWidth * 2 - DefX > Desktop.GetRight())
			{
				Width = Desktop.GetRight() - Left;

			}
			else
				Width = RWidth + FrameWidth * 2 - DefX;

			if (RHeight + FrameWidth * 2 - DefY > Desktop.GetBottom())
			{
				Height = Desktop.GetBottom() - Top;
			}
			else
				Height = RHeight + FrameWidth * 2 - DefY;
		}
		else
		{
			Left = WLeft;
			Top = WTop;
			Width = WWidth;
			Height = WHeight;
		}
	}

	SetWindowPos(hWindow, nullptr, Left, Top, Width, Height, (SWP_NOZORDER | SWP_NOOWNERZORDER));
	DisplayBox.SetPosition(0, 0, Width, Height);

	Right = Left + Width;
	Bottom = Top + Height;
	ClientRect = { Left, Top, Width, Height };

	CheckRefreshBuckBuffer();
	FormRefresh();
	if (ReachMinDL == false) SizeRatio = (double)WWidth / Susie.ORotWidth;
	return (true);
}


bool CMainForm::CheckRefreshBuckBuffer(void)
{
	if (Holding > 0) return (false);
	return (Susie.SetFormSize(WWidth, WHeight));
}


void CMainForm::GetRealWindowRect(void)
{
	RLeft = WLeft;
	RWidth = WWidth;
	RTop = WTop;
	RHeight = WHeight;

	int newWidth;
	int newHeight;

	if (RLeft < Desktop.X)
	{
		newWidth = WWidth + (RLeft - Desktop.X);
		RLeft = Desktop.X;
		RWidth = (int)newWidth;
	}

	if (RTop < Desktop.Y)
	{
		newHeight = WHeight + (RTop - Desktop.Y);
		RTop = (int)Desktop.Y;
		RHeight = (int)newHeight;
	}

	if (RLeft + RWidth > Desktop.X + Desktop.Width)
	{
		RWidth = (Desktop.X + Desktop.Width - RLeft);
	}

	if (RTop + RHeight > Desktop.Y + Desktop.Height)
	{
		RHeight = Desktop.Y + Desktop.Height - RTop;
	}
}



bool CMainForm::FixViewOut(void)
{
	if (FitToScreen) MnFitToScreen_Click();
	if (UseWholeScreen) MnUseWholeScreen_Click();
	return (true);
}

/// <summary>
/// ウィンドウの位置を更新する 状況によっては上下左右を拘束する
/// </summary>
/// <param name="X">新しいウィンドウの左端</param>
/// <param name="Y">新しいウィンドウの上端</param>
void CMainForm::RefreshWindowPosition(int X, int Y)
{
	if (FitToScreen == true)
	{
		if ((FitMode & EFitMode_FIT_LEFTANDRIGHT) == 0)
		{
			WLeft = X;
			if (WLeft < MLeft) WLeft = MLeft;
			if (WLeft + WWidth > MRight) WLeft = MRight - WWidth;
		}

		if ((FitMode & EFitMode_FIT_TOPANDBOTTOM) == 0)
		{
			WTop = Y;
			if (WTop < MTop) WTop = MTop;
			if (WTop + WHeight > MBottom) WTop = MBottom - WHeight;
		}
	}
	else if (UseWholeScreen == true)
	{
		if ((FitMode & EFitMode_FIT_LEFTANDRIGHT) == 0)
		{
			WLeft = X;
			if (WLeft > MLeft) WLeft = MLeft;
			if (WLeft + WWidth < MRight) WLeft = MRight - WWidth;
		}
		if ((FitMode & EFitMode_FIT_TOPANDBOTTOM) == 0)
		{
			WTop = Y;
			if (WTop > MTop) WTop = MTop;
			if (WTop + WHeight < MBottom) WTop = MBottom - WHeight;
		}
	}
	else
	{
		WLeft = X;
		WTop = Y;
	}
}


// カーソルがウィンドウのどの位置にあるかによってカーソルの形を変更する
CMainForm::EMousePositionType CMainForm::GetCursorPositionType(POINT &ScCurPos)
{
	if (PreCurTimerPos.x == ScCurPos.x
		&& PreCurTimerPos.y == ScCurPos.y
		&& StopCount > 2) return (EMousePositionType_NONE);

	if (FitMode != 0 // ShowingList ||
		|| ScCurPos.x < WLeft - FrameWidth || ScCurPos.x > WLeft + WWidth + FrameWidth
		|| ScCurPos.y < WTop - FrameWidth || ScCurPos.y > WTop + WHeight + FrameWidth)
	{
		SetCursorMode(EMousePositionType_NONE);
		return (EMousePositionType_NORMAL);
	}

	EMousePositionType Result = EMousePositionType_NORMAL;
	int SD = 16;

	if (Holding < 2)
	{
		if (WWidth < 64 || WHeight < 64)
		{
			if (WWidth > WHeight)
				SD = (int)(WHeight / 4);
			else
				SD = (int)(WWidth / 4);
			if (SD < 4) SD = 4;
		}

		if (ScCurPos.y >= WTop - FrameWidth
			&& ScCurPos.y <= WTop + SD)
		{
			if (ScCurPos.x >= WLeft - FrameWidth
				&& ScCurPos.x <= WLeft + SD) Result = EMousePositionType_LT;
			else
				if (ScCurPos.x >= WLeft + WWidth - SD
					&& ScCurPos.x <= WLeft + WWidth + FrameWidth) Result = EMousePositionType_RT;
				else
					Result = EMousePositionType_T;
		}
		else if (ScCurPos.y >= WTop + WHeight - SD
			&& ScCurPos.y <= WTop + WHeight + FrameWidth)
		{
			if (ScCurPos.x >= WLeft - FrameWidth
				&& ScCurPos.x <= WLeft + SD) Result = EMousePositionType_LB;
			else
				if (ScCurPos.x >= WLeft + WWidth - SD
					&& ScCurPos.x <= WLeft + WWidth + FrameWidth) Result = EMousePositionType_RB;
				else
					Result = EMousePositionType_B;
		}
		else if (ScCurPos.x >= WLeft - FrameWidth
			&& ScCurPos.x <= WLeft + SD) Result = EMousePositionType_L;
		else if (ScCurPos.x >= WLeft + WWidth - SD
			&& ScCurPos.x <= WLeft + WWidth + FrameWidth) Result = EMousePositionType_R;
	}

	SetCursorMode(Result);
	return (Result);
}

bool CMainForm::SetCursorMode(EMousePositionType Result)
{
	if (SSChangeImage) return (true);
	if (Locked)
	{
		if (Result == EMousePositionType_NONE)
			SetCursor(hCursor[0]);
		else
			SetCursor(LoadCursor(0, IDC_ARROW));
	}
	else
	{
		CursorMode = Result;

		switch (Result)
		{
			case EMousePositionType_NONE:
				if (ShowingList == true)
					SetCursor(LoadCursor(0, IDC_ARROW));
				else
					SetCursor(hCursor[0]);
				break;

			case EMousePositionType_NORMAL:
				if (ShowingList == true)
				{
					SetCursor(LoadCursor(0, IDC_ARROW));
				}
				else
				{
					if (Holding != 1)
						SetCursor(hCursor[1]);
					else
						SetCursor(hCursor[2]);
				}
				break;

			case EMousePositionType_RT:
			case EMousePositionType_LB:
				SetCursor(LoadCursor(0, IDC_SIZENESW));
				break;

			case EMousePositionType_LT:
			case EMousePositionType_RB:
				SetCursor(LoadCursor(0, IDC_SIZENWSE));
				break;

			case EMousePositionType_T:
			case EMousePositionType_B:
				SetCursor(LoadCursor(0, IDC_SIZENS));
				break;

			case EMousePositionType_L:
			case EMousePositionType_R:
				SetCursor(LoadCursor(0, IDC_SIZEWE));
				break;
		}
	}

	return (true);
}


// マウスダウン
void CMainForm::OnMouseDown(CMainForm::EEventButton button)
{
	StopCount = 0;
	if (Locked == false)
	{
		POINT p;
		GetCursorPos(&p);
		if (button == EEventButton_LEFT)
		{

			if (!(button == EEventButton_LEFT// && ShowingList == true
				&& p.x >= WLeft && p.x < WLeft + WWidth && p.y >= WTop && p.y < WTop + WHeight))
			{
				PositionType = EMousePositionType_NORMAL;
			}
			SetCenter(WLeft + WWidth / 2, WTop + WHeight / 2);

			OfsCurPos.x = (int)(p.x - WLeft);
			OfsCurPos.y = (int)(p.y - WTop);
			DownCurPos = p;
			PreCurPos = DownCurPos;
			DownLeft = WLeft;
			DownTop = WTop;
			DownWidth = WWidth;
			DownHeight = WHeight;
		}
		Holding = button;
		PositionType = GetCursorPositionType(p);
		SetCapture(hWindow);
	}
}

// マウスアップ
void CMainForm::OnMouseUp(CMainForm::EEventButton button)
{
	POINT p;
	GetCursorPos(&p);
	if (Locked)
	{
		if (button == EEventButton_RIGHT)
		{
			if (RClkCancel == true)
			{
				RClkCancel = false;
			}
			else
			{
				if (((GetAsyncKeyState(VK_LMENU) & 0x8000) | (GetAsyncKeyState(VK_RMENU) & 0x8000)) > 0)
				{
					PopupFileMovePopup(p);
				}
				else
				{
					PopupMenuPopup(p);
				}
			}
		}
	}
	else
	{
		int SubKey = 0;

		if (((GetAsyncKeyState(VK_LSHIFT) & 0x8000) | (GetAsyncKeyState(VK_RSHIFT) & 0x8000))) SubKey |= 1;
		if (((GetAsyncKeyState(VK_LCONTROL) & 0x8000) | (GetAsyncKeyState(VK_RCONTROL) & 0x8000))) SubKey |= 2;
		if (((GetAsyncKeyState(VK_LMENU) & 0x8000) | (GetAsyncKeyState(VK_RMENU) & 0x8000))) SubKey |= 4;
		if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000)) SubKey |= 8;
		if ((GetAsyncKeyState(VK_RBUTTON) & 0x8000)) SubKey |= 16;


		switch (button)
		{
		case EEventButton_LEFT:
			if (Holding > 0)
			{
				DoMouseUp();
				if ((SubKey & 16) > 0)
				{
					MnInScreen_Click();
					RClkCancel = true;
				}
			}
			break;

		case EEventButton_RIGHT:
			if (RClkCancel == true)
			{
				RClkCancel = false;
			}
			else if ((SubKey & 8) > 0)
			{
				DoMouseUp();
				MnInScreen_Click();
			}
			else
			{
				if ((SubKey & 2) > 0)
				{
					PopupFileMovePopup(p);
				}
				else
				{
					PopupMenuPopup(p);
				}
			}
			break;

		case EEventButton_MIDDLE:
			if (ShowingList == false)
			{
				if (SubKey == 24 || SubKey == 3)
				{
					if ((SubKey & 8) > 0) DoMouseUp();
					if ((SubKey & 16) > 0) RClkCancel = true;
					FormAlphaBlendValue = 100;
					CheckAlphaValueMenuCheck();
				}
				else if ((SubKey & 9) > 0)
				{
					if ((SubKey & 8) > 0) DoMouseUp();
					if (RotateValue != 0)
					{
						AbsoluteRotate(0);
					}
				}
				else if ((SubKey & 18) > 0)
				{
					if ((SubKey & 16) > 0) RClkCancel = true;

					if (SizeRatio != 1.0)
						ZoomOrgPosition(1, p.x, p.y);
				}
				else
					Close();
			}
			else
				Close();
			break;
		}
	}
	Holding = EEventButton_NONE;
	if (CheckRefreshBuckBuffer() == true) FormRefresh();
}

void CMainForm::DoMouseUp(void)
{
	SetCenter(WLeft + WWidth / 2, WTop + WHeight / 2);

	Holding = EEventButton_NONE;
	PositionType = EMousePositionType_NORMAL;

	POINT ScCurPos;
	GetCursorPos(&ScCurPos);
	PositionType = GetCursorPositionType(ScCurPos);
	ReleaseCapture();
}


void CMainForm::MouseLeave(void)
{
	Holding = EEventButton_NONE;
	PositionType = EMousePositionType_NONE;
	StopCount = 0;
}

void CMainForm::MouseEnter(void)
{
	StopCount = 0;
}



bool CMainForm::SetCenter(int iX, int iY)
{
	CenterX = iX;
	CenterY = iY;

	if (FullScreen == false && FitMode == 0)
	{
		GetMonitorParameter();
	}
	return (true);
}

bool CMainForm::GetMonitorParameter(void)
{
	POINT p = { CenterX, CenterY };
	HMONITOR hMonitor = MonitorFromPoint(p, MONITOR_DEFAULTTONEAREST);
	MONITORINFO mon = { 0 };
	mon.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(hMonitor, &mon); 
	
	if (FullScreen == false)
	{
		MLeft = mon.rcWork.left + FrameWidth;
		MTop = mon.rcWork.top + FrameWidth;
		MWidth = RECT_W(mon.rcWork) - FrameWidth * 2;
		MHeight = RECT_H(mon.rcWork) - FrameWidth * 2;

		MRight = MLeft + MWidth;
		MBottom = MTop + MHeight;
	}
	else
	{
		MLeft = mon.rcMonitor.left + FrameWidth;
		MTop = mon.rcMonitor.top + FrameWidth;
		MWidth = RECT_W(mon.rcMonitor) - FrameWidth * 2;
		MHeight = RECT_H(mon.rcMonitor) - FrameWidth * 2;

		MRight = MLeft + MWidth;
		MBottom = MTop + MHeight;
	}
	return (true);
}



// マウスホイール
void CMainForm::OnMouseWheel(int delta)
{
	if (Locked || ShowingList || ProgressForm->Visible)return;

	int i = 0;
	int SubKey = 0;

	if ((GetAsyncKeyState(VK_LSHIFT) & 0x8000) || (GetAsyncKeyState(VK_RSHIFT) & 0x8000))SubKey |= 1;
	if ((GetAsyncKeyState(VK_LCONTROL) & 0x8000) || (GetAsyncKeyState(VK_RCONTROL) & 0x8000))SubKey |= 2;
	if ((GetAsyncKeyState(VK_LMENU) & 0x8000) || (GetAsyncKeyState(VK_RMENU) & 0x8000))SubKey |= 4;
	if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000))SubKey |= 8;
	if ((GetAsyncKeyState(VK_RBUTTON) & 0x8000))SubKey |= 16;


	WheelPos += delta;
	if (WheelPos > WheelSensitivity)
	{
		while (WheelPos > WheelSensitivity)
		{
			WheelPos -= WheelSensitivity;
			i--;
		}
	}
	else if (WheelPos < 0)
	{
		while (WheelPos < 0)
		{
			WheelPos += WheelSensitivity;
			i++;
		}
	}

	if (SubKey == 0 && i != 0)
	{
		ShowOffsetImage(i); 
	}
	else if (SubKey == 3 || SubKey == 24 && i != 0)
	{
		if (SubKey & 8)DoMouseUp();
		if (SubKey & 16)RClkCancel = true;
		AlphaValueOffset(i);
	}
	else if (SubKey & 9 && i != 0)
	{
		if (SubKey & 8)DoMouseUp();

		if (i > 0)
			Susie.OffsetRotate(1);
		else
			Susie.OffsetRotate(-1);
		
		RotateValue = Susie.Rotate;
		CheckRotateCheck();
		SetRotateImageSize();
	}
	else if (SubKey & 18)
	{
		POINT ScCurPos;
		GetCursorPos(&ScCurPos);
		if (SubKey & 16)RClkCancel = true;
		ZoomOrgPositionDelta(delta, ScCurPos.x, ScCurPos.y);
	}
}

// デスクトップ矩形を得るコールバック関数
BOOL CALLBACK MainFormMonitorEnumProc(HMONITOR hMon, HDC hdcMon, LPRECT lpMon, LPARAM dwData)
{
	MONITORINFOEX monInfo = { 0 };
	monInfo.cbSize = sizeof(MONITORINFOEX);
	GetMonitorInfo(hMon, &monInfo);

	DEVMODE devMode = { 0 };
	devMode.dmSize = sizeof(DEVMODE);
	// dmFieldsにDM_POSITION、dmDriverExtraにPOINTのサイズを入れるとdmPositionが取得出来る
	devMode.dmDriverExtra = sizeof(POINTL);
	devMode.dmFields = DM_POSITION;
	EnumDisplaySettings(monInfo.szDevice, ENUM_CURRENT_SETTINGS, &devMode);

	// 物理座標系のデスクトップ矩形
	RECT physicalDesktopRect;
	physicalDesktopRect.left = devMode.dmPosition.x;
	physicalDesktopRect.top = devMode.dmPosition.y;
	physicalDesktopRect.right = devMode.dmPosition.x + devMode.dmPelsWidth;
	physicalDesktopRect.bottom = devMode.dmPosition.y + devMode.dmPelsHeight;

	((CMainForm *)dwData)->SetMonitorRect(&physicalDesktopRect);
	return(TRUE);
}

// モードによってスクリーンの状態を得てウィンドウの状態を変更する
bool CMainForm::CheckMonitorIni(void)
{
	GetMonitorParameter();

	if (FullScreen)
	{
		FullScreen = !FullScreen;
		MnFullScreen_Click();
	}

	if (UseWholeScreen)
	{
		UseWholeScreen = !UseWholeScreen;
		MnUseWholeScreen_Click();
	}
	else if (FitToScreen)
	{
		FitToScreen = !FitToScreen;
		MnFitToScreen_Click();
	}

	return (true);
}


// デスクトップ領域を得る
void CMainForm::GetDesktopRect(void)
{
	int MonitorNum = GetSystemMetrics(SM_CMONITORS);
	EnumDisplayMonitors(NULL, NULL, MainFormMonitorEnumProc, (LPARAM)this);
}

// デスクトップ領域を得る関数の補助
void CMainForm::SetMonitorRect(LPRECT lpr)
{
	if (Desktop.X > lpr->left)
		Desktop.X = lpr->left;

	if (Desktop.Y > lpr->top)
		Desktop.Y = lpr->top;

	if (Desktop.GetRight() < lpr->right)
		Desktop.Width = lpr->right - Desktop.X;

	if (Desktop.GetBottom() < lpr->bottom)
		Desktop.Height = lpr->bottom - Desktop.Y;
}

std::wstring CMainForm::GetTabString(std::wstring Src, std::wstring Name)
{
	int i, L, l, P = Src.find(Name + TEXT("="));
	if (P < 0) return (TEXT(""));
	std::wstring Result = TEXT("");
	L = Src.length();
	l = Name.length();
	for (i = P + l; i < L; i++)
	{
		if (Src[i] == TEXT('\t')) break;
	}
	Result = Src.substr(P + l + 1, i - P - l - 1);
	return (Result);
}

int CMainForm::IndexOfImageInfos(std::vector<CImageInfo> &Src, std::wstring FileName)
{
	for (int i = 0; i < (int)Src.size(); i++)
	{
		if (Src[i].FileName == FileName) return (i);
	}
	return (-1);
}


std::wstring CMainForm::LoadStringResource(UINT uID)
{
	std::wstring result;
	TCHAR buf[1024]; // TODO:FindResource が失敗するので決め打ちで

	LoadString(appInstance, uID, buf, 1024);

	result = buf;

	return(result);
}


LPWSTR CMainForm::LoadStringBuffer(UINT uID)
{
	TCHAR *result;
	TCHAR buf[1024];// TODO:FindResource が失敗するので決め打ちで
	LoadString(appInstance, uID, buf, 1024);

	result = BufferString(buf);

	return(result);
}

LPWSTR CMainForm::BufferString(TCHAR *src)
{
	TCHAR *result;
	std::wstring dat = src;
	std::set<std::wstring>::iterator itr;
	itr = StrBuf.find(dat);
	if (itr == StrBuf.end())
	{
		auto itrpair = StrBuf.insert(dat);
		result = (LPWSTR)(*itrpair.first).c_str();
	}
	else
	{
		result = (LPWSTR)(*itr).c_str();
	}
	return(result);
}


std::wstring CMainForm::GetFileVersionString(void)
{
	TCHAR             fileName[MAX_PATH + 1];
	DWORD             size;
	BYTE*             pVersion;
	VS_FIXEDFILEINFO* pFileInfo;
	UINT              queryLen;
	std::wstring      fileVersion;

	GetModuleFileName(NULL, fileName, sizeof(fileName));

	size = ::GetFileVersionInfoSize(fileName, NULL);
	pVersion = new BYTE[size];
	if (GetFileVersionInfo(fileName, NULL, size, pVersion))
	{
		VerQueryValue(pVersion, TEXT("\\"), (void**)&pFileInfo, &queryLen);

		fileVersion = std::to_wstring(HIWORD(pFileInfo->dwFileVersionMS))
			+ TEXT(".") + std::to_wstring(LOWORD(pFileInfo->dwFileVersionMS))
			+ TEXT(".") + std::to_wstring(HIWORD(pFileInfo->dwFileVersionLS))
			+ TEXT(".") + std::to_wstring(LOWORD(pFileInfo->dwFileVersionLS));
	}
	delete[] pVersion;
	return(fileVersion);
}



void CMainForm::SetTaskButtonVisibility(void)
{

	LONG lStyle;

	lStyle = GetWindowLong(hWindow, GWL_EXSTYLE);

	if (HideTaskButton)
	{
		lStyle |= WS_EX_TOOLWINDOW;
	}
	else
	{
		if (lStyle & WS_EX_TOOLWINDOW)
			lStyle ^= WS_EX_TOOLWINDOW;
	}
	lStyle = SetWindowLong(hWindow, GWL_EXSTYLE, lStyle);
	SetWindowPos(hWindow, NULL, 0, 0, 0, 0, (SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER));
}

void CMainForm::NoStayOnTop(void)
{
	Susie.PauseGIFAnimate();

	if (Susie.GIFAnimate == true)
	{
		EnterCriticalSection(&CriticalSection);
		GIFAnimePaused = true;
		LeaveCriticalSection(&CriticalSection);
	}

	SetWindowPos(hWindow, HWND_NOTOPMOST, 0, 0, 0, 0, (SWP_NOMOVE | SWP_NOSIZE));
}


void CMainForm::RestoreStayOnTop(void)
{
	Susie.RestartGIFAnimate();

	if (Susie.GIFAnimate == true)
	{
		EnterCriticalSection(&CriticalSection);
		GIFAnimePaused = false;
		LeaveCriticalSection(&CriticalSection);
	}

	if (AlwaysTop == true)
	{
		SetWindowPos(hWindow, HWND_TOPMOST, 0, 0, 0, 0, (SWP_NOMOVE | SWP_NOSIZE));
	}
	else
	{
		SetWindowPos(hWindow, HWND_NOTOPMOST, 0, 0, 0, 0, (SWP_NOMOVE | SWP_NOSIZE));
	}
}

//--- メニュー編集 ---------------------------------------------------------------

// メニューをチェックのオンオフを行う 0:なし 1:チェック 2:選択
void CMainForm::SetMenuCheckType(HMENU hmenu, UINT ItemID, int Type)
{
	MENUITEMINFO menuItem = { 0 };
	menuItem.cbSize = sizeof(MENUITEMINFO);
	menuItem.fMask = MIIM_STATE | MIIM_CHECKMARKS | MIIM_FTYPE;

	GetMenuItemInfo(hmenu, ItemID, FALSE, &menuItem);


	if (Type == 2)
		menuItem.fType |= MFT_RADIOCHECK;

	if (Type == 1 || Type == 2)
		menuItem.fState = MFS_CHECKED;
	else
		menuItem.fState = MFS_UNCHECKED;

	SetMenuItemInfo(hmenu, ItemID, FALSE, &menuItem);
}

void CMainForm::SetMenuCheck(HMENU hmenu, UINT ItemID, bool Checked)
{
	MENUITEMINFO menuItem = { 0 };
	menuItem.cbSize = sizeof(MENUITEMINFO);
	menuItem.fMask = MIIM_STATE;

	GetMenuItemInfo(hmenu, ItemID, FALSE, &menuItem);

	if (Checked)
		menuItem.fState = MFS_CHECKED;
	else
		menuItem.fState = MFS_UNCHECKED;

	SetMenuItemInfo(hmenu, ItemID, FALSE, &menuItem);
}

// メニューの Enable を設定する
void CMainForm::SetMenuEnabled(HMENU hmenu, UINT ItemID, bool Enabled)
{
	MENUITEMINFO menuItem = { 0 };
	menuItem.cbSize = sizeof(MENUITEMINFO);
	menuItem.fMask = MIIM_STATE;

	GetMenuItemInfo(hmenu, ItemID, FALSE, &menuItem);

	if (Enabled)
		menuItem.fState = IMFS_ENABLED;
	else
		menuItem.fState = IMFS_DISABLED;

	SetMenuItemInfo(hmenu, ItemID, FALSE, &menuItem);
}

// メニューのテキストを設定する
void CMainForm::SetMenuText(HMENU hmenu, UINT ItemID, std::wstring src)
{
	MENUITEMINFO menuItem = { 0 };
	menuItem.cbSize = sizeof(MENUITEMINFO);
	menuItem.fMask = MIIM_STRING;
	menuItem.dwTypeData = BufferString((TCHAR *)src.c_str());

	SetMenuItemInfo(hmenu, ItemID, FALSE, &menuItem);
}


//-------------------------------------------------------------------------------


// ポップアップメニュー表示
void CMainForm::PopupMenuPopup(POINT &p)
{
	TCHAR *text;

	if (HideTaskButton == true)
	{
		if (Visible == false) // IsWindowVisible(hWindow) == false ?? 
		{
			text = LoadStringBuffer(IDS_MES_1047);
		}
		else
		{
			text = LoadStringBuffer(IDS_MES_1046);
		}
	}
	else
	{
		if (Visible == false)
		{
			text = LoadStringBuffer(IDS_MES_1047);
		}
		else
		{
			text = LoadStringBuffer(IDS_MES_1046);
		}
	}

	SetMenuText(hPopupMenu, ID_POPUP_SHOWHIDE, text);
	StopCount = -1;
	TrackPopupMenu(hPopupMenu, TPM_LEFTALIGN | TPM_TOPALIGN, p.x, p.y, 0, hWindow, NULL);
}

void CMainForm::PopupFileMovePopup(POINT &p)
{
	CreateFileMovePPMenu();
	MovePPPosition = p;
	StopCount = -1;
	TrackPopupMenu(hMoveMenu, TPM_LEFTALIGN | TPM_TOPALIGN, p.x, p.y, 0, hWindow, NULL);
}


// ファイル移動メニューを表示する
void CMainForm::CreateFileMovePPMenu(void)
{
	std::wstring Folder, Temp;

	if (hMoveMenu != nullptr)
	{
		DestroyMenu(hMoveMenu);
	}

	if (ShowingList == true)
	{
		if (InArchive == true)
			Temp = OpeningFileName;
		else
			Temp = FileList[ShowIndex].FileName;
	}
	else
		Temp = OpeningFileName;

	Folder = acfc::GetFolderName(Temp);

	if (Folder == L"") return;

	CreateFileMoveMenuFolder(Folder);
}

void CMainForm::CreateFileMoveMenuFolder(std::wstring FolderName)
{
	hMoveMenu = CreateMenu();

	std::vector<std::wstring> temp;

	MENUITEMINFO menuItem = { 0 };
	menuItem.cbSize = sizeof(MENUITEMINFO);
	menuItem.fMask = MIIM_TYPE | MIIM_ID;
	menuItem.fType = MFT_STRING;
	UINT wID = ID_FILE_MOVE_BASE;
	int Index = 0;

	MENUITEMINFO sepItem = { 0 };
	sepItem.cbSize = sizeof(MENUITEMINFO);
	sepItem.fMask = MIIM_TYPE;
	sepItem.fType = MFT_SEPARATOR;

	MoveData.clear();

	if (acfc::GetParentFolder(FolderName) != TEXT(""))
	{
		MoveData.push_back(TEXT("??"));
		menuItem.dwTypeData = BufferString((TCHAR *)TEXT("[..](&0)"));
		menuItem.wID = wID;
		InsertMenuItem(hMoveMenu, Index, TRUE, &menuItem);
		wID++;
		Index++;
	}
	acfc::GetFolders(temp, FolderName);
	for (int i = 0; i < (int)(temp.size()); i++)
	{
		std::wstring sc = TEXT("0");
		sc[0] = (char)(TEXT('0') + i + Index);

		if (sc[0] > TEXT('9')) sc[0] = (TCHAR)(sc[0] - TEXT('9') + TEXT('A'));
		if (sc[0] > TEXT('Z')) sc[0] = (TCHAR)(sc[0] - TEXT('Z') + TEXT('a'));
		if (sc[0] > TEXT('z')) sc[0] = (TCHAR)0;

		if (sc[0] != 0)
		{
			sc =  TEXT("(&") + sc + TEXT(")");
		}
		sc = acfc::GetMiniPathName(temp[i], 1) + sc;
		MoveData.push_back(temp[i]);
		menuItem.dwTypeData = BufferString((TCHAR *)sc.c_str());
		menuItem.wID = wID;
		InsertMenuItem(hMoveMenu, Index, TRUE, &menuItem);
		wID++;
		Index++;
	}

	InsertMenuItem(hMoveMenu, Index, TRUE, &sepItem);
	Index++;

	//-------------------------------------------------------------------------------------------

	std::wstring ThisFolder = acfc::GetFileName(FolderName);
	if (ThisFolder != TEXT(""))
	{
		MoveData.push_back(TEXT("?"));
		menuItem.dwTypeData = LoadStringBuffer(IDS_MES_1036);
		menuItem.wID = wID;
		InsertMenuItem(hMoveMenu, Index, TRUE, &menuItem);
		Index++;
		wID++;
	}
	else
	{
		MoveData.push_back(TEXT("?"));
		menuItem.dwTypeData = LoadStringBuffer(IDS_MES_1037);
		menuItem.wID = wID;
		InsertMenuItem(hMoveMenu, Index, TRUE, &menuItem);
		Index++;
		wID++;
	}

	//-------------------------------------------------------------------------------------------

	InsertMenuItem(hMoveMenu, Index, TRUE, &sepItem);
	Index++;

	//-------------------------------------------------------------------------------------------

	MoveData.push_back(L"*");
	menuItem.dwTypeData = LoadStringBuffer(IDS_MES_1038);
	menuItem.wID = wID;
	InsertMenuItem(hMoveMenu, Index, TRUE, &menuItem);
	wID++;
	Index++;

	MENUITEMINFO mii = { 0 };
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_SUBMENU;
	GetMenuItemInfo(hParentMoveMenu, ID_MOVE_MOVEPARENT, FALSE, &mii);
	mii.hSubMenu = hMoveMenu;
	SetMenuItemInfo(hParentMoveMenu, ID_MOVE_MOVEPARENT, FALSE, &mii); // これで初めて有効になる
}

// ダイアログボックスを表示して現在表示中のファイルが存在するフォルダに新しいフォルダを作る
void CMainForm::CreateFolder(void)
{
	NoStayOnTop();
	std::wstring NewFileName = TEXT("");

	InputForm->SetData(LoadStringResource(IDS_MES_1040), LoadStringResource(IDS_MES_1041), NewFileName);
	if (InputForm->ShowDialog(appInstance, hWindow) == IDOK)
	{
		NewFileName = InputForm->Result;

		int i;
		i = acfc::CheckFileIrregularChar(NewFileName);
		if (i >= 0)
		{
			std::wstring Mes = NewFileName + LoadStringResource(IDS_MES_1022) + NewFileName[i] + LoadStringResource(IDS_MES_1023);
			MessageBox(hWindow, Mes.c_str(), TEXT("Failed"), MB_OK | MB_ICONASTERISK);
			return;
		}

		NewFileName = acfc::GetFolderName(OpeningFileName) + TEXT("\\") + NewFileName;

		if (acfc::FolderExists(NewFileName) == true)
		{
			MessageBox(hWindow, LoadStringResource(IDS_MES_1042).c_str(), TEXT("Failed"), MB_OK | MB_ICONASTERISK);
		}
		else
		{
			CreateDirectory(NewFileName.c_str(), 0);
		}
	}
	RestoreStayOnTop();
}



void CMainForm::AbsoluteRotate(int Value)
{
	if (Susie.Showing == true && Initialized == true)
	{
		Susie.AbsoluteRotate(Value);
		RotateValue = Susie.Rotate;

		CImageInfo tempII = DisplayList[ShowIndex];
		tempII.Rotate = RotateValue;
		DisplayList[ShowIndex] = tempII;

		SetRotateImageSize();
	}

	CheckRotateCheck();
}

void CMainForm::OffsetRotate(int Value)
{
	if (Susie.Showing == false) return;

	Susie.OffsetRotate(Value);
	RotateValue = Susie.Rotate;
	CheckRotateCheck();
	SetRotateImageSize();

	CImageInfo tempII = DisplayList[ShowIndex];
	tempII.Rotate = RotateValue;
	DisplayList[ShowIndex] = tempII;
}


void CMainForm::SetRotateImageSize(void)
{
	FixSizeRatio = true;
	SetNewImageSize();
	FixSizeRatio = false;
}

// 画像をパーセンテージ指定でズームする
bool CMainForm::ZoomImage(double NewPercentage)
{
	if (NewPercentage > MaxSizeRatio) NewPercentage = MaxSizeRatio;
	SizeRatio = NewPercentage;
	FixViewOut();

	double nWidth = Susie.ORotWidth * SizeRatio;
	double nHeight = Susie.ORotHeight * SizeRatio;

	CheckDiagonalLength(nWidth, nHeight);
	SizeRatio = Susie.ORotWidth / nWidth;

	SetWindowSize((int)nWidth, (int)nHeight, false);
	SetDiagonalLength();
	return (true);
}

// 画像をホイールデルタでズームする
bool CMainForm::ZoomOrgPositionDelta(double Delta, int X, int Y)
{
	double NewPercentage = SizeRatio * pow(2, ((double)Delta / WheelSensitivity / 2));
	return (ZoomOrgPosition(NewPercentage, X, Y));
}

// 画像を中心指定でズームする
bool CMainForm::ZoomOrgPosition(double NewPercentage, int X, int Y)
{
	double dx, dy;
	if (NewPercentage > MaxSizeRatio) NewPercentage = MaxSizeRatio;

	dx = (double)(X - WLeft - WWidth / 2) / SizeRatio;
	dy = (double)(Y - WTop - WHeight / 2) / SizeRatio;

	int iWidth = (int)(Susie.ORotWidth * NewPercentage);
	int iHeight = (int)(Susie.ORotHeight * NewPercentage);

	if (CheckMinimalDiagonalLength(iWidth, iHeight))
	{
		NewPercentage = (double)iWidth / Susie.ORotWidth;
	}

	SizeRatio = NewPercentage;

	dx = ((double)X - CenterX) / SizeRatio - dx;
	dy = ((double)Y - CenterY) / SizeRatio - dy;

	CenterX = CenterX + (int)(dx * SizeRatio);
	CenterY = CenterY + (int)(dy * SizeRatio);

	FixViewOut();

	SetWindowSize(iWidth, iHeight, false);
	SetDiagonalLength();
	return (true);
}

// 新しい画像を読み込んだときにウィンドウサイズを決定する
void CMainForm::SetNewImageSize()
{
	double ImageWidth = Susie.ORotWidth;
	double ImageHeight = Susie.ORotHeight;

	Ratio = (double)Susie.ORotWidth / Susie.ORotHeight;

	double MaxWidth = (double)MWidth;// * 9 / 10;
	double MaxHeight = (double)MHeight;// * 9 / 10;

	if (FixSizeRatio || ReloadMode)
	{
		ImageWidth = Susie.ORotWidth * SizeRatio;
		ImageHeight = Susie.ORotHeight * SizeRatio;
	}

	ReachMinDL = CheckDiagonalLength(ImageWidth, ImageHeight);

	if (FitToScreen == true)
	{
		FitMode = EFitMode_NONE;

		if (ImageWidth / MaxWidth > ImageHeight / MaxHeight)
		{
			FitMode = EFitMode_FIT_LEFTANDRIGHT;
			ImageWidth = MaxWidth;
			ImageHeight = MaxWidth / Ratio;
		}
		else
		{
			FitMode = EFitMode_FIT_TOPANDBOTTOM;
			ImageHeight = MaxHeight;
			ImageWidth = MaxHeight * Ratio;
		}

		if (ImageWidth == MaxWidth && ImageHeight == MaxHeight)
			FitMode = (EFitMode)(EFitMode_FIT_TOPANDBOTTOM | EFitMode_FIT_LEFTANDRIGHT);
	}
	else if (FixDiagonalLength == true)
	{
		FitMode = EFitMode_NONE;

		double d = KeepDiagonalLength / (sqrt(ImageWidth * ImageWidth + ImageHeight * ImageHeight));
		ImageWidth = ImageWidth * d;
		ImageHeight = ImageHeight * d;
	}
	else if (UseWholeScreen == true)
	{
		if (ImageWidth / MaxWidth < ImageHeight / MaxHeight)
		{
			FitMode = EFitMode_FIT_LEFTANDRIGHT;
			ImageWidth = MaxWidth;
			ImageHeight = MaxWidth / Ratio;
		}
		else
		{
			FitMode = EFitMode_FIT_TOPANDBOTTOM;
			ImageHeight = MaxHeight;
			ImageWidth = MaxHeight * Ratio;
		}

		if (ImageWidth == MaxWidth && ImageHeight == MaxHeight)
			FitMode = (EFitMode)(EFitMode_FIT_TOPANDBOTTOM | EFitMode_FIT_LEFTANDRIGHT);
	}
	else if (FixSizeRatio == false)
	{
		FitMode = EFitMode_NONE;
		if (MaxWidth < ImageWidth)
		{
			if (MaxHeight < ImageHeight)
			{
				if (ImageWidth / MaxWidth > ImageHeight / MaxHeight)
				{
					ImageWidth = MaxWidth;
					ImageHeight = MaxWidth / Ratio;
				}
				else
				{
					ImageHeight = MaxHeight;
					ImageWidth = MaxHeight * Ratio;
				}
			}
			else
			{
				ImageWidth = MaxWidth;
				ImageHeight = MaxWidth / Ratio;
			}
		}

		if (MaxHeight < ImageHeight)
		{
			ImageHeight = MaxHeight;
			ImageWidth = MaxHeight * Ratio + 0.5;
		}
	}


	SetWindowSize((int)ImageWidth, (int)ImageHeight, true);

	Holding = EEventButton_NONE;
}


// 対角線サイズを計算する
bool CMainForm::SetDiagonalLength(void)
{
	KeepDiagonalLength = (int)(sqrt((double)WWidth * WWidth + (double)WHeight * WHeight));
	return (true);
}

// 対角線サイズをチェックする
bool CMainForm::CheckDiagonalLength(double &dWidth, double &dHeight)
{
	bool Resize = false;
	double TargetDiagonalLength = 0;
	if (MinimalDiagonalLength > ((dWidth * dWidth) + dHeight * dHeight) || dWidth < 0 || dHeight < 0)
	{
		Resize = true;
		TargetDiagonalLength = MinimalDiagonalLength;
	}
	else if (MaximumDiagonalLength < ((dWidth * dWidth) + dHeight * dHeight) || dWidth < 0 || dHeight < 0)
	{
		//                Resize = true;
		//                TargetDiagonalLength = MaximumDiagonalLength;
	}

	if (Resize)
	{
		double x, y, r, d;
		x = (double)Susie.ORotWidth;
		y = (double)Susie.ORotHeight;

		if (x == 0) r = 1;
		else
			r = y / x;

		d = TargetDiagonalLength;
		x = sqrt(d / (r * r + 1));
		y = r * x;

		dWidth = x;
		dHeight = y;
		return (true);
	}
	return (false);
}

// 最小対角線サイズをキープする
bool CMainForm::CheckMinimalDiagonalLength(int &iWidth, int &iHeight)
{
	double dWidth, dHeight;
	dWidth = iWidth;
	dHeight = iHeight;

	CheckDiagonalLength(dWidth, dHeight);

	iWidth = (int)dWidth;
	iHeight = (int)dHeight;

	return (false);
}

void CMainForm::CorrectWindow(void)
{
	int ShowLength = (int)sqrt(MinimalDiagonalLength);

	if (WWidth > Desktop.Width)
		WWidth = Desktop.Width, WHeight = (int)((double)WWidth * Susie.OrgHeight / Susie.OrgWidth);

	if (WHeight > Desktop.Height)
		WHeight = Desktop.Height, WWidth = (int)((double)WHeight * Susie.OrgWidth / Susie.OrgHeight);

	if (WWidth > Desktop.Width)
		WWidth = Desktop.Width, WHeight = (int)((double)WWidth * Susie.OrgHeight / Susie.OrgWidth);

	if (WHeight > Desktop.Height)
		WHeight = Desktop.Height, WWidth = (int)((double)WHeight * Susie.OrgWidth / Susie.OrgHeight);

	if (WWidth < 0)
	{
		if (WHeight > 0)
			WWidth = (int)((double)WHeight * Susie.OrgWidth / Susie.OrgHeight);
		else
			WWidth = Susie.OrgWidth, WHeight = Susie.OrgHeight;

	}

	if (WHeight < 0)
	{
		if (WWidth > 0)
			WHeight = (int)((double)WWidth * Susie.OrgHeight / Susie.OrgWidth);
		else
			WWidth = Susie.OrgWidth, WHeight = Susie.OrgHeight;

	}

	if (Desktop.X + ShowLength > WLeft + WWidth)WLeft = Desktop.X + ShowLength - WWidth;
	if (Desktop.Y + ShowLength > WTop + WHeight)WTop = Desktop.Y + ShowLength - WHeight;
	if (Desktop.X + Desktop.Width - ShowLength < WLeft)WLeft = Desktop.X + Desktop.Width - ShowLength;
	if (Desktop.Y + Desktop.Height - ShowLength < WTop)WTop = Desktop.Y + Desktop.Height - ShowLength;

	CenterX = WLeft + WWidth / 2;
	CenterY = WTop + WTop / 2;

	GetMonitorParameter();

	if (MLeft + ShowLength > WLeft + WWidth)WLeft = MLeft + ShowLength - WWidth;
	if (MTop + ShowLength > WTop + WHeight)WTop = MTop + ShowLength - WHeight;
	if (MRight - ShowLength < WLeft)WLeft = MRight - ShowLength;
	if (MBottom - ShowLength < WTop)WTop = MBottom - ShowLength;

	CenterX = WLeft + WWidth / 2;
	CenterY = WTop + WHeight / 2;

}



void CMainForm::SetWindowSize(int iWidth, int iHeight, bool InMonitor)
{
	WWidth = iWidth;
	WHeight = iHeight;

	if (FitToScreen == true)
	{
		if ((FitMode & EFitMode_FIT_LEFTANDRIGHT) == 0)
		{
			WTop = MTop;
			switch (PositionMode)
			{
			case EPositionMode_FREE:
				WLeft = CenterX - WWidth / 2;
				if (WLeft < MLeft) WLeft = MLeft;
				if (WLeft + WWidth > MRight) WLeft = MRight - WWidth;
				break;
			case EPositionMode_CENTER:
				WLeft = (MLeft + MRight) / 2 - WWidth / 2;
				break;
			case EPositionMode_LEFTTOP:
			case EPositionMode_LEFTBOTTOM:
				WLeft = MLeft;
				break;
			case EPositionMode_RIGHTTOP:
			case EPositionMode_RIGHTBOTTOM:
				WLeft = MRight - WWidth;
				break;
			}
		}

		if ((FitMode & EFitMode_FIT_TOPANDBOTTOM) == 0)
		{
			WLeft = MLeft;
			switch (PositionMode)
			{
			case EPositionMode_FREE:
				WTop = CenterY - WHeight / 2;
				if (WTop < MTop) WTop = MTop;
				if (WTop + WHeight > MBottom) WTop = MBottom - WHeight;
				break;
			case EPositionMode_CENTER:
				WTop = (MTop + MBottom) / 2 - WHeight / 2;
				break;
			case EPositionMode_LEFTTOP:
			case EPositionMode_RIGHTTOP:
				WTop = MTop;
				break;
			case EPositionMode_LEFTBOTTOM:
			case EPositionMode_RIGHTBOTTOM:
				WTop = MBottom - WHeight;
				break;
			}
		}
	}
	else if (UseWholeScreen == true)
	{
		if ((FitMode & EFitMode_FIT_LEFTANDRIGHT) == 0)
		{
			WTop = MTop + FrameWidth;
			switch (PositionMode)
			{
			case EPositionMode_FREE:
				WLeft = CenterX - WWidth / 2;
				if (WLeft > MLeft) WLeft = MLeft;
				if (WLeft + WWidth < MRight) WLeft = MRight - WWidth;
				break;
			case EPositionMode_CENTER:
				WLeft = (MLeft + MRight) / 2 - WWidth / 2;
				if (WLeft > MLeft) WLeft = MLeft;
				if (WLeft + WWidth < MRight) WLeft = MRight - WWidth;
				break;
			case EPositionMode_LEFTTOP:
			case EPositionMode_LEFTBOTTOM:
				WLeft = MLeft;
				break;
			case EPositionMode_RIGHTTOP:
			case EPositionMode_RIGHTBOTTOM:
				WLeft = MBottom - WWidth;
				break;
			}
		}

		if ((FitMode & EFitMode_FIT_TOPANDBOTTOM) == 0)
		{
			WLeft = MLeft;
			switch (PositionMode)
			{
			case EPositionMode_FREE:
				WTop = CenterY - WHeight / 2;
				if (WTop > MTop) WTop = MTop;
				if (WTop + WHeight < MBottom) WTop = MBottom - WHeight;
				break;
			case EPositionMode_CENTER:
				WTop = (MTop + MBottom) / 2 - WHeight / 2;
				break;
			case EPositionMode_LEFTTOP:
			case EPositionMode_RIGHTTOP:
				WTop = MTop;
				break;
			case EPositionMode_LEFTBOTTOM:
			case EPositionMode_RIGHTBOTTOM:
				WTop = MBottom + WHeight;
				break;
			}
		}
	}
	else if (InMonitor && FixSizeRatio == false && ReloadMode == false)
	{
		MoveByPositionMode();
	}
	else
	{
		WLeft = CenterX - WWidth / 2;
		WTop = CenterY - WHeight / 2;
	}

	SyncWindow();
}

bool CMainForm::MoveByPositionMode(void)
{
	if (ReloadMode == true || PositionMode == 0)
	{
		if (WWidth > MWidth)
			CenterX = MWidth / 2 + MLeft;

		if (WHeight > MHeight)
			CenterY = MHeight / 2 + MTop;

		if (CenterX - WWidth / 2 < MLeft)
			WLeft = MLeft;
		else if (CenterX + WWidth / 2 > MLeft + MWidth)
			WLeft = MLeft + MWidth - WWidth;
		else
			WLeft = CenterX - WWidth / 2;

		if (CenterY - WHeight / 2 < MTop)
			WTop = MTop;
		else if (CenterY + WHeight / 2 > MTop + MHeight)
			WTop = MTop + MHeight - WHeight;
		else
			WTop = CenterY - WHeight / 2;
	}
	else
	{
		switch (PositionMode)
		{
		case EPositionMode_CENTER:
			WLeft = MLeft + MWidth / 2 - WWidth / 2;
			WTop = MTop + MHeight / 2 - WHeight / 2;
			SetCenter((int)(WLeft + WWidth / 2), (int)(WTop + WHeight / 2));
			break;

		case EPositionMode_LEFTTOP:
			WLeft = MLeft;
			WTop = MTop;
			SetCenter((int)(WLeft + WWidth / 2), (int)(WTop + WHeight / 2));
			break;

		case EPositionMode_RIGHTTOP:
			WLeft = MLeft + MWidth - WWidth;
			WTop = MTop;
			SetCenter((int)(WLeft + WWidth / 2), (int)(WTop + (WHeight + 1) / 2));
			break;

		case EPositionMode_LEFTBOTTOM:
			WLeft = MLeft;
			WTop = MTop + MHeight - WHeight;
			SetCenter((int)(WLeft + (WWidth + 1) / 2), (int)(WTop + WHeight / 2));
			break;

		case EPositionMode_RIGHTBOTTOM:
			WLeft = MLeft + MWidth - WWidth;
			WTop = MTop + MHeight - WHeight;
			SetCenter((int)(WLeft + (WWidth + 1) / 2), (int)(WTop + (WHeight + 1) / 2));
			break;
		}
	}
	return (true);
}

bool CMainForm::SetPositionMode(int NewPositionMode)
{
	if (PositionMode == (EPositionMode)NewPositionMode) return (true);

	SetMenuCheck(hPopupMenu, mnPositionModes[PositionMode], false);

	PositionMode = (EPositionMode)NewPositionMode;

	SetMenuCheckType(hPopupMenu, mnPositionModes[PositionMode], 2);

	MoveByPositionMode();
	SyncWindow();
	return (true);
}

bool CMainForm::SetDisableFitMode(int aMode)
{
	aMode = 0xff - aMode;
	if ((aMode & 1) > 0)
	{
		if (FixSizeRatio == true) FixSizeRatio = false;
		SetMenuCheck(hPopupMenu, ID_POPUP_FIXSIZERATIO, FixSizeRatio);
	}

	if ((aMode & 2) > 0)
	{
		if (FitToScreen == true) FitToScreen = false;
		SetMenuCheck(hPopupMenu, ID_POPUP_FITTOSCREEN, FitToScreen);
	}

	if ((aMode & 4) > 0)
	{
		if (FixDiagonalLength == true) FixDiagonalLength = false;
		SetMenuCheck(hPopupMenu, ID_POPUP_FIXDIAGONALLENGTH, FixDiagonalLength);
	}

	if ((aMode & 8) > 0)
	{
		if (UseWholeScreen == true) UseWholeScreen = false;
		SetMenuCheck(hPopupMenu, ID_POPUP_USEWHOLESCREEN, UseWholeScreen);
	}
	return (true);
}

bool CMainForm::GoToFullScreen(bool aMode)
{
	GetMonitorParameter();
	if (aMode == true)
	{
		Left = MLeft - FrameWidth;
		Top = MTop - FrameWidth;
		Width = MWidth + FrameWidth * 2;
		Height = MHeight + FrameWidth * 2;
		acfc::SetAbsoluteForegroundWindow(hWindow);
	}
	else
	{
		Left = WLeft - FrameWidth;
		Top = WTop - FrameWidth;
		Width = WWidth + FrameWidth * 2;
		Height = WHeight + FrameWidth * 2;
	}
	if (FitMode != EFitMode_NONE)
		SetNewImageSize();
	else
		SyncWindow();

	return (true);
}

void CMainForm::AlphaValueOffset(int i)
{
	int Index = -1;
	int j;
	for (j = 0; j < ALPHABLENDNUM; j++)
	{
		if (FormAlphaBlendValue >= AlphaBlends[j])
		{
			Index = j;
			break;
		}
	}

	Index += i;
	if (Index < 0) Index = 0;
	if (Index >= ALPHABLENDNUM) Index = ALPHABLENDNUM - 1;
	FormAlphaBlendValue = AlphaBlends[Index];
	CheckAlphaValueMenuCheck();
}

//-----------------------------------------------------------------------------------------------------
//
// 以下基本的にファイル操作
//
//-----------------------------------------------------------------------------------------------------

// ファイルリストにファイル名を設定する
bool CMainForm::SetFileList(void)
{
	int i;
	DisplayBox.Items.clear();
	DisplayBox.SelectedIndex = ShowIndex;
	for (i = 0; i < (int)DisplayList.size(); i++)
	{
		acfc::CListBox::CItem Item(acfc::GetMiniPathName(DisplayList[i].FileName, 1), false);
		DisplayBox.Items.push_back(Item);
	}
	
	if(ShowIndex > 0 && ShowIndex < (int)DisplayList.size())DisplayBox.SetSelected(ShowIndex, true);

	DisplayBox.Set();

	return (true);
}

// 表示中のファイル名を取得する
std::wstring CMainForm::GetImageFileName(void)
{
	std::wstring Result;
	if (ShowingList == true)
	{
		if (DisplayList.size() == 0) return (TEXT(""));
	}
	else
	{
		if (ShowIndex == -1) return (TEXT(""));
	}

	if (InArchive == true)
	{
		Result = FileList[ShowIndexBack].FileName;
	}
	else if (ShowingList == true)
	{
		int Index = DisplayBox.GetSelectedIndex();
		if (Index >= 0)Result = FileList[Index].FileName;
		else
			return(TEXT(""));
	}
	else
	{
		Result = FileList[ShowIndex].FileName;
	}
	return (Result);
}

// 表示中の画像のフォルダを取得する
std::wstring CMainForm::GetImageFileFolder(void)
{
	std::wstring Result, src = GetImageFileName();

	if (src == TEXT(""))
	{
		Result = acfc::GetSpecialFolderPath(CSIDL_DESKTOP);
	}
	else
	{
		Result = acfc::GetFolderName(src);
	}
	return (Result);
}


void CMainForm::SetImageFileNameString(std::wstring src)
{
	OpeningFileName = src;
}

// 現在表示中のファイル名を変更する
bool CMainForm::SetImageFileName(std::wstring src)
{
	CImageInfo tempII;

	if (InArchive == true)
	{
		int i = IndexOfImageInfos(ArchiveFileList, OpeningFileName);

		tempII = ArchiveFileList[ShowIndexBack];
		tempII.FileName = src;
		ArchiveFileList[i] = tempII;

		tempII = FileList[ShowIndexBack];
		tempII.FileName = src;
		FileList[ShowIndexBack] = tempII;

		SetImageFileNameString(src);

		Susie.ChangeArchiveFileName(src);
	}
	else
	{
		if (ShowingList == true)
		{
			DisplayBox.SetText(acfc::GetMiniPathName(src, 1), ShowIndex);
		}

		tempII = FileList[ShowIndex];
		tempII.FileName = src;
		FileList[ShowIndex] = tempII;

		SetImageFileNameString(src);
	}
	return (true);
}

// 表示中のファイル名をダイアログボックスを表示して変更する
bool CMainForm::ChangeImageFileName(std::wstring newName)
{
	if (InArchive == true) return (false);

	DisplayBox.GetSelectedIndex();
	DisplayBox.SetText(acfc::GetMiniPathName(newName, 1), DisplayBox.SelectedIndex);

	CImageInfo tempII;
	tempII = FileList[DisplayBox.SelectedIndex];
	tempII.FileName = newName;
	FileList[DisplayBox.SelectedIndex] = tempII;
	DisplayList[DisplayBox.SelectedIndex] = tempII;

	SetImageFileNameString(newName);

	return (true);
}


// ファイルオープン関係
bool CMainForm::OpenFiles(std::vector<std::wstring> &SrcLists)
{
	bool bTemp = (((GetAsyncKeyState(VK_LCONTROL) & 0x8000) | (GetAsyncKeyState(VK_RCONTROL) & 0x8000)));
	return (OpenFiles(SrcLists, TEXT(""), 0, bTemp));
}


bool CMainForm::OpenFiles(std::vector<std::wstring> &SrcLists, std::wstring SelectedFile, int Offset, bool AddMode)
{
	if (FileLoading > 0) return (false);

	bool AddLists;
	std::vector<CImageInfo> ArcLists;
	std::vector<CImageInfo> TempLists;

	CheckGetLists(TempLists, ArcLists, SrcLists);

	if (TempLists.size() == 0)
	{
		ArchiveFileList.clear();
		return (true);
	}

	CloseArchiveMode();

	if (((GetAsyncKeyState(VK_LCONTROL) & 0x8000)
		| (GetAsyncKeyState(VK_RCONTROL) & 0x8000))
		|| AddMode)
	{
		AddLists = true;
		AddFileList(TempLists, ArcLists, 1);
	}
	else
	{
		AddLists = false;
		AddFileList(TempLists, ArcLists, 0);
	}

	if (FileList.size() == 0) return (false);
	DisplayList = FileList;
	SetFileList();
	if (SelectedFile != TEXT("")) ShowIndex = IndexOfImageInfos(FileList, SelectedFile);

	ShowIndex += Offset;
	while (ShowIndex < 0) ShowIndex += FileList.size();
	while (ShowIndex >= (int)FileList.size()) ShowIndex -= FileList.size();
	ShowFromShowIndex = true;

	if (ShowingList == false || AddLists == false)
	{
		ToggleShowList(EShowMode_FORCEPICTURE);
	}

	return (true);
}

bool CMainForm::CheckGetLists(std::vector<CImageInfo> &DestLists, std::vector<CImageInfo> &ArcLists, std::vector<std::wstring> &DropLists)
{
	int i;
	for (i = 0; i < (int)DropLists.size(); i++)
	{
		if (acfc::GetFileExt(DropLists[i]) == TEXT(".lnk"))
		{
			DropLists[i] = acfc::GetFileFromLink(DropLists[i]);
		}

		if (acfc::FolderExists(DropLists[i]))
		{
			GetImageLists(DropLists[i], DestLists, SearchSubFolder, EnableFileMask, FileMaskString);
		}
		else if (acfc::GetFileExt(DropLists[i]) == TEXT(".mfl"))
		{
			std::map<std::wstring, std::wstring> Map;
			acfc::LoadMapFromFile(Map, DropLists[i]);
			LoadFileList(DestLists, ArcLists, Map);
		}
		else
		{
			if (EnableFileMask && acfc::FitsMasks(DropLists[i], FileMaskString) == false) continue;
			if (acfc::FileExists(DropLists[i]) == false) continue;

			struct _stati64 st;
			_wstat64(DropLists[i].c_str(), &st);

			CImageInfo newII;
			newII.FileName = DropLists[i];
			newII.FileSize = (int)st.st_size;
			newII.Timestamp = st.st_ctime;
			newII.Rotate = -1;
			DestLists.push_back(newII);
		}
	}
	return (true);
}

bool CMainForm::GetImageLists(std::wstring Src, std::vector<CImageInfo> &Dest, bool SubFolder, bool EnableFileMask, std::wstring FileMaskString) // c:\windows\dest\*.dll のような形式で Src は指定する
{
	NoStayOnTop();

	if (EnableFileMask == false)
	{
		FileMaskString = TEXT("*.*");
	}

	Dest.clear();

	ProgressForm->SetData(Src, SubFolder, FileMaskString);

	ProgressForm->ShowDialog(appInstance, hWindow);
	ProgressForm->GetData(Dest);

	RestoreStayOnTop();

	return (true);
}

bool CMainForm::AddFileList(std::vector<CImageInfo> &SrcLists, std::vector<CImageInfo> &ArcLists, int Mode)
{
	CloseArchiveMode();
	switch (Mode)
	{
		// 新規
	case 0:
		ShowIndex = 0;
		FileList.clear();
		ArchiveFileList.clear();
		Susie.Clear(EPluginMode_ALL);

		if (SrcLists.size() > 0)
		{
			FileList = SrcLists;
		}
		if (ArcLists.size() > 0)
		{
			ArchiveFileList = ArcLists;
		}
		break;

		// 追加
	case 1:
		ShowIndex = FileList.size();

		while (SrcLists.size() > 0)
		{
			if (IndexOfImageInfos(FileList, SrcLists[0].FileName) < 0)
				FileList.push_back(SrcLists[0]);

			SrcLists.erase(SrcLists.begin());
		}
		if (ArcLists.size() > 0) std::copy(ArcLists.begin(), ArcLists.end(), std::back_inserter(ArchiveFileList));

		if (ShowIndex >= (int)FileList.size())
			ShowIndex = FileList.size() - 1;
		break;
	}
	return (true);
}

bool CMainForm::DeleteFileList(int DeleteMode)  // 1 ファイルをゴミ箱に移す 2 確認無くゴミ箱に移す
{
	int i;
	DisplayBox.BeginUpdate();
	DisplayBox.Get();
	int SelectedIndex = DisplayBox.SelectedIndex;

	if ((ShowingList == false && ShowIndex == -1) || (ShowingList == true && SelectedIndex < 0)) return (false);

	if (InArchive && DeleteMode > 0)
	{
		MessageBox(hWindow, LoadStringResource(IDS_MES_1008).c_str(), TEXT("Failed"), MB_OK | MB_ICONEXCLAMATION);
		return (false);
	}

	if (DeleteMode > 0)
	{
		bool bResult = true;

		if (ShowingList == true)
		{
			if (DeleteMode == 1)
			{
				int Result;
				std::wstring Mes;

				if (DisplayBox.SelectedCount == 1)
				{
					for (i = 0; i < (int)DisplayBox.Items.size(); i++)
					{
						if (DisplayBox.Items[i].Selected == true) break;
					}
					Mes = acfc::GetMiniPathName(DisplayList[i].FileName, 1) + TEXT("\n");
				}
				else
					Mes = LoadStringResource(IDS_MES_1009);

				Mes += LoadStringResource(IDS_MES_1010);

				Result = MessageBox(hWindow, Mes.c_str(), TEXT("Ask"), MB_YESNO | MB_ICONQUESTION);

				if (Result == IDNO) return (false);
			}

			std::vector<std::wstring> DelList;
			for (i = 0; i < (int)DisplayBox.Items.size(); i++)
			{
				if(DisplayBox.Items[i].Selected)
					DelList.push_back(DisplayList[i].FileName);
			}
			bResult = acfc::DeleteFileToRecycle(DelList, false);
		}
		else
		{
			if (DeleteMode == 1)
			{
				int Result;
				std::wstring Mes;

				Mes = acfc::GetMiniPathName(DisplayList[ShowIndex].FileName, 1) + TEXT("\n");

				Mes += LoadStringResource(IDS_MES_1010);
				Result = MessageBox(hWindow, Mes.c_str(), TEXT("Ask"), MB_YESNO | MB_ICONQUESTION); 
				
				if (Result == IDNO) return (false);

				bResult = acfc::DeleteFileToRecycle(DisplayList[ShowIndex].FileName, false);
			}
		}

		if (bResult == false)
		{
			MessageBox(hWindow, LoadStringResource(IDS_MES_1013).c_str(), TEXT("Failed"), MB_OK | MB_ICONEXCLAMATION);
			return (false);
		}
	}

	if (ShowingList == true)
	{
		if (DisplayBox.SelectedCount > 1)
		{
			for (i = DisplayBox.Items.size() - 1; i >= 0; i--)
			{
				if (DisplayBox.Items[i].Selected)
					DeleteFileInList(i);
			}
		}
		else if (DisplayBox.SelectedIndex >= 0)
		{
			DeleteFileInList(DisplayBox.SelectedIndex);
		}
	}
	else
	{
		if (ShowIndex >= 0 && ShowIndex < (int)DisplayList.size())
			DeleteFileInList(ShowIndex);
	}
	DisplayBox.EndUpdate();
	return (true);
}

bool CMainForm::DeleteFileInList(int i)
{
	if (ShowingList == true)
	{
		DisplayBox.BeginUpdate();
		DisplayBox.Get();

		if (DisplayBox.SelectedCount > 0)
		{
			if (i == ShowIndex)
			{
				ShowIndex = -1;
				Susie.Clear(EPluginMode_PICTURE);
			}

			int PreviousII = DisplayBox.SelectedIndex;
			DisplayList.erase(DisplayList.begin() + i);
			DisplayBox.Delete(i);

			if (DisplayBox.Items.size() == 0)
			{
				if (InArchive == true)
				{
					CloseArchiveMode();
					SetFileList();
					DeleteFileList(0);
				}
			}

			if (i == PreviousII)
			{
				if (DisplayBox.Items.size() == PreviousII)
					DisplayBox.SelectedIndex = PreviousII - 1;
				else
					DisplayBox.SelectedIndex = PreviousII;
				DisplayBox.SetSelectedIndex();
			}
		}

		DisplayBox.EndUpdate();
	}
	else
	{
		DisplayList.erase(DisplayList.begin() + i);
		if (DisplayList.size() == 0)
		{
			if (InArchive == true)
			{
				CloseArchiveMode();
				DeleteFileList(0);
			}
			else
			{
				Susie.Clear(EPluginMode_PICTURE);
				ToggleShowList(EShowMode_LIST);
			}
		}
		ShowOffsetImage(0);
	}
	return (true);
}

bool CMainForm::DeleteArchiveData(std::wstring ArcFileName)
{
	int i = IndexOfImageInfos(FileList, ArcFileName);
	if (i < 0) return (false);

	FileList.erase(FileList.begin() + i);

	i = IndexOfImageInfos(ArchiveFileList, ArcFileName);
	if (i < 0) return (false);

	ArchiveFileList.erase(FileList.begin() + i);
	return (true);
}

bool CMainForm::MoveSelectedList(int Offset)
{
	DisplayBox.BeginUpdate();
	DisplayBox.GetSelectList();
	int i, j, k;

	int SelIndex;

	// まず Offset の値によって場合分け
	if (Offset < 0)
	{
		// 一番上の Selected を取得する
		for (i = 0; i < (int)DisplayBox.Items.size(); i++)
		{
			if (DisplayBox.Items[i].Selected) break;
		}
		SelIndex = i;

		// Selected が連続している部分を飛ばす
		for (i = 0; i < (int)DisplayBox.Items.size(); i++)
		{
			if (DisplayBox.Items[i].Selected == false) break;
		}

		// その次の Selected を取得する
		for (; i < (int)DisplayBox.Items.size(); i++)
		{
			if (DisplayBox.Items[i].Selected) break;
		}
		if (i == (int)DisplayBox.Items.size()) { DisplayBox.EndUpdate(); return (false); }

		for (; i < (int)DisplayBox.Items.size(); i++)
		{
			j = i - 1; k = i + 1;
			while (k < (int)DisplayBox.Items.size() && DisplayBox.Items[k].Selected == true)
				k++;

			k--;

			CImageInfo tempII = DisplayList[j];
			DisplayList.erase(DisplayList.begin() + j);
			DisplayList.insert(DisplayList.begin() + k, tempII);

			acfc::CListBox::CItem tempItem = DisplayBox.Items[j];
			DisplayBox.Delete(j);
			DisplayBox.Insert(tempItem, k);

			i = k;

			while (i < (int)DisplayBox.Items.size() && DisplayBox.Items[i].Selected == false)
				i++;
			i--;
		}
	}
	else
	{
		// 一番下の Selected を取得する
		for (i = (int)DisplayBox.Items.size() - 1; i >= 0; i--)
		{
			if (DisplayBox.Items[i].Selected) break;
		}
		SelIndex = i;

		// Selected が連続している部分を飛ばす
		for (i = (int)DisplayBox.Items.size() - 1; i >= 0; i--)
		{
			if (DisplayBox.Items[i].Selected == false) break;
		}

		// その次の Selected を取得する
		for (; i >= 0; i--)
		{
			if (DisplayBox.Items[i].Selected) break;
		}
		if (i == -1) { DisplayBox.EndUpdate(); return (false); }

		for (; i >= 0; i--)
		{
			j = i + 1; k = i - 1;
			while (k >= 0 && DisplayBox.Items[k].Selected == true)
				k--;

			k++;

			CImageInfo tempII = DisplayList[j];
			DisplayList.erase(DisplayList.begin() + j);
			DisplayList.insert(DisplayList.begin() + k, tempII);

			std::wstring tempText = DisplayBox.Items[j].Text;
			bool tempBool = DisplayBox.Items[j].Selected;
			DisplayBox.Delete(j);
			DisplayBox.Insert(tempText, tempBool, k);

			i = k;

			while (i >= 0 && DisplayBox.Items[i].Selected == false)
				i--;
			i++;
		}
	}

	if (SelIndex + Offset >= 0 && SelIndex + Offset < (int)DisplayBox.Items.size())
	{
		DisplayBox.SelectedIndex = SelIndex + Offset;
		DisplayBox.SetSelected(SelIndex + Offset, true);
	}

	DisplayBox.EndUpdate();
	return (true);
}

bool CMainForm::SortFileList(ESortType Type)
{
	if (DisplayList.size() == 0) return (false);
	DisplayBox.GetSelectList();

	int i, j;
	std::vector<CImageInfo> Temp;
	std::wstring OldFileName = DisplayList[ShowIndex].FileName;

	if (DisplayBox.SelectedCount <= 1)
	{
		std::copy(DisplayList.begin(), DisplayList.end(), std::back_inserter(Temp));
	}
	else
	{
		for (i = 0; i < (int)DisplayList.size(); i++)
		{
			if (DisplayBox.Items[i].Selected)
				Temp.push_back(DisplayList[i]);
		}
	}

	switch (Type)
	{
	case ESortType_NAME:
		std::sort(Temp.begin(), Temp.end(), [](CImageInfo &a, CImageInfo &b) {return(a.FileName < b.FileName); });
		break;
	case ESortType_EXT:
		std::sort(Temp.begin(), Temp.end(), [](CImageInfo &a, CImageInfo &b) {return(acfc::GetFileExt(a.FileName) < acfc::GetFileExt(b.FileName)); });
		break;
	case ESortType_TIMESTAMP:
		std::sort(Temp.begin(), Temp.end(), [](CImageInfo &a, CImageInfo &b) {return(a.Timestamp < b.Timestamp); });
		break;
	case ESortType_FILESIZE:
		std::sort(Temp.begin(), Temp.end(), [](CImageInfo &a, CImageInfo &b) {return(a.FileSize < b.FileSize); });
		break;
	case ESortType_RANDOM:
		{
			std::random_device seed_gen;
			std::mt19937 engine(seed_gen());
			std::shuffle(Temp.begin(), Temp.end(), engine);
		}
		break;
	case ESortType_REVERSE:
		std::reverse(Temp.begin(), Temp.end());
		break;
	}

	if (DisplayBox.SelectedCount <= 1)
	{
		DisplayList = Temp;
	}
	else
	{
		j = 0;
		for (i = 0; i < (int)DisplayList.size(); i++)
		{
			if (DisplayBox.Items[i].Selected)
			{
				DisplayList[i] = Temp[j];
				j++;
				if (j == Temp.size()) break;
			}
		}
	}

	ShowIndex = IndexOfImageInfos(DisplayList, OldFileName);
	SetFileList();
	return (true);
}

CMainForm::EOpenFileResult CMainForm::OpenFile(CImageInfo & imageInfo)
{
	if (EnableFileMask && acfc::FitsMasks(imageInfo.FileName, FileMaskString) == false) return (EOpenFileResult_EXCLUDEBYMASK);

	LoadStart = timeGetTime();
	LoadState = 0;
	ProgressRatio = 0;

	if (InArchive == false)
	{
		if (Susie.SetImageFile(imageInfo) == false)
			return (EOpenFileResult_LOADFAILED);
	}
	else
	{
		if (Susie.SetSubImageFile(imageInfo) == false)
			return (EOpenFileResult_LOADFAILED); ;
	}

	return (EOpenFileResult_SUCCEEDED);
}

bool CMainForm::OpenArchiveMode(CImageInfo &SrcImageInfo, int SubIndex, int Ofs, bool MustShowImage)
{
	InArchive = true;
	SetMenuEnabled(hPopupMenu, ID_POPUP_CLOSEARCHIVE, true);

	SetImageFileNameString(SrcImageInfo.FileName);

	int i = IndexOfImageInfos(ArchiveFileList, SrcImageInfo.FileName);
	int j;
	if (i < 0) // ArchiveFileList にまだ記載がない
	{
		DisplayList.clear();

		i = ArchiveFileList.size();
		ArchiveFileList.push_back(SrcImageInfo);

		// アーカイブファイルからリストを取得
		Susie.GetArchiveFileList(DisplayList);
		for (j = 0; j < (int)DisplayList.size(); j++)
		{
			CImageInfo TempII = DisplayList[j];
			TempII.FileName = DisplayList[j].FileName;
			if (TempII.FileName.length() == 0
				||
				(EnableFileMask && acfc::FitsMasks(DisplayList[j].FileName, FileMaskString) == false)
				)
			{
				DisplayList.erase(DisplayList.begin() + j);
				j--;
			}
		}

		CImageInfo ArcII = ArchiveFileList[i];
		ArcII.FileName = SrcImageInfo.FileName;
		ArcII.ImageInfoList = DisplayList;
		ArchiveFileList[i] = ArcII;
	}
	else
	{
		DisplayList = ArchiveFileList[i].ImageInfoList;
	}

	ShowIndexBack = ShowIndex;

	// 表示するファイルがなければ終了
	if (DisplayList.size() == 0)
	{
		CloseArchiveMode();
		return (false);
	}

	if (SubIndex < 0)
	{
		if (Ofs < 0)
			ShowIndex = DisplayList.size() - 1;
		else
			ShowIndex = 0;
	}
	else
		ShowIndex = SubIndex;

	if (MustShowImage == false)
	{
		SetFileList();
	}
	else if (Susie.SetSubImageFile(DisplayList, ShowIndex, Ofs) == false)
	{
		CloseArchiveMode();
		return (false);
	}
	return (true);
}

bool CMainForm::CloseArchiveMode(void)
{
	if (InArchive == false) return (true);
	SetMenuEnabled(hPopupMenu, ID_POPUP_CLOSEARCHIVE, false);

	if (DisplayList.size() == 0)
		DeleteArchiveData(FileList[ShowIndexBack].FileName);

	Susie.Clear(EPluginMode_ALL);
	DisplayList = FileList;
	ShowIndex = ShowIndexBack;

	if ((int)FileList.size() <= ShowIndex) ShowIndex = FileList.size() - 1;
	if (ShowIndex < 0) ShowIndex = 0;

	ShowIndexBack = -1;
	InArchive = false;

	if ((int)FileList.size() > 0)
	{
		SetImageFileNameString(FileList[ShowIndex].FileName);
	}


	return (true);
}

void CMainForm::ToggleShowList(EShowMode Mode) // Mode 0:反転 1:リスト表示 2:画像表示 3:強制画像表示
{
	bool PreSL = ShowingList;
	int Result = 0;

	switch (Mode)
	{
	case EShowMode_TOGGLE:
		ShowingList = !ShowingList;
		break;
	case EShowMode_LIST:
		ShowingList = true;
		break;
	case EShowMode_PICTURE:
	case EShowMode_FORCEPICTURE:
		ShowingList = false;
		break;
	}

	if (ShowingList == false)
	{
		if (ShowFromShowIndex == false)
		{
			DisplayBox.GetSelectedIndex();
			if (DisplayBox.SelectedIndex >= 0 && DisplayBox.SelectedIndex < (int)DisplayList.size())
				Result = ShowAbsoluteImage(DisplayBox.SelectedIndex, -1, 1, Mode == EShowMode_FORCEPICTURE);
		}
		else
		{
			ShowFromShowIndex = false;
			if (ShowIndex >= 0 && ShowIndex < (int)DisplayList.size())
				Result = ShowAbsoluteImage(ShowIndex, -1, 1, Mode == EShowMode_FORCEPICTURE);
		}
	}

	// 0 正常終了 1 リスト更新の必要有り
	if (Result == 1 && Mode != EShowMode_FORCEPICTURE)
	{
		SetFileList();
		ShowingList = true;
	}

	if (PreSL != ShowingList)
	{
		SyncWindow();
		if (ShowingList == true)
		{
			SetFileList();
			DisplayBox.Show();
			DisplayBox.Focus();
			DisplayBox.SetSelected(ShowIndex, true);
			SetMenuText(hPopupMenu, ID_POPUP_TOGGLEIMAGELIST, LoadStringResource(IDS_MES_1014));

			if (WWidth < MIV_SMALLESTWINDOWSIZE || WHeight < MIV_SMALLESTWINDOWSIZE)
			{
				if (WWidth < MIV_SMALLESTWINDOWSIZE)
				{
					WLeft = WLeft + (WWidth - MIV_SMALLESTWINDOWSIZE) / 2;
					WWidth = MIV_SMALLESTWINDOWSIZE;
				}
				if (WHeight < MIV_SMALLESTWINDOWSIZE)
				{
					WTop = WTop + (WHeight - MIV_SMALLESTWINDOWSIZE) / 2;
					WHeight = MIV_SMALLESTWINDOWSIZE;
				}
				Ratio = (double)WWidth / WHeight;
			}
			InvalidateRect(hWindow, 0, TRUE);
		}
		else
		{
			ShowIndex = DisplayBox.SelectedIndex;
			DisplayBox.Hide();
			SetMenuText(hPopupMenu, ID_POPUP_TOGGLEIMAGELIST, LoadStringResource(IDS_MES_1015));
		}
	}
}



CMainForm::ELoadFileResult CMainForm::LoadFile(int Index, int SubIndex, int Ofs, bool MustShowImage)
{
	EOpenFileResult OpenResult;
	ELoadFileResult FuncResult = ELoadFileResult_OK;

	if (ShowIndex < 0) ShowIndex = 0;
	if (ShowIndex >= (int)DisplayList.size()) ShowIndex = DisplayList.size() - 1;

	Susie.GIFLooping = false;

	CImageInfo tempII = DisplayList[ShowIndex];

	OpenResult = OpenFile(tempII);

	DisplayList[ShowIndex] = tempII;
	LoadState = 0;
	ProgressRatio = 0;

	if (OpenResult == EOpenFileResult_EXCLUDEBYMASK) return (ELoadFileResult_FAILEDORNOMASK);
	if (OpenResult == EOpenFileResult_NOARCHIVEFILE) return (ELoadFileResult_NOARCHIVE);
	if (OpenResult == EOpenFileResult_LOADFAILED) return (ELoadFileResult_FAILEDORNOMASK);

	if ((Susie.Mode & EPluginMode_ARCHIVE) != 0 && InArchive == false)
	{
		if (OpenArchiveMode(tempII, SubIndex, Ofs, MustShowImage) == false)
		{
			if (IndexOfImageInfos(ArchiveFileList, tempII.FileName) != ShowIndex)
			{
				AdjustShowIndex(Ofs);
			}
			return (ELoadFileResult_RETRYSAMEINDEX);
		}
		FileList[ShowIndexBack] = tempII;
		FuncResult = ELoadFileResult_OPENARCHIVE;
	}
	else
	{
		if (InArchive == false)
		{
			tempII = DisplayList[ShowIndex];
			tempII.Timestamp = acfc::GetFileCreationTime(DisplayList[ShowIndex].FileName);
			tempII.FileSize = acfc::GetFileSizeValue(DisplayList[ShowIndex].FileName);
			DisplayList[ShowIndex] = tempII;
		}
		FuncResult = ELoadFileResult_OK;
	}

	EndGIFAnimeThread();

	BeginUpdate();

	SetNewImageSize();
	Susie.SetFormSize(WWidth, WHeight);

	EndUpdate();
	FormRefresh();

	if (Susie.GIFAnimate == true)
		BeginGIFAnimeThread();

	//	Hint = GetShortFileName(DisplayList.Strings[ShowIndex]);
	return (FuncResult);
}

bool CMainForm::AdjustShowIndex(int Ofs)
{
	ShowIndex += Ofs;
	if (DisplayList.size() == 0)
	{
		ShowIndex = -1;
		return (false);
	}
	while (ShowIndex < 0) ShowIndex += DisplayList.size();
	while (ShowIndex >= (int)DisplayList.size()) ShowIndex -= DisplayList.size();
	return (true);
}

bool CMainForm::JumpBorderArcNml(int Ofs)
{
	if (InArchive && (ShowIndex < 0 || ShowIndex >= (int)DisplayList.size()))
	{
		CloseArchiveMode();

		if (DisplayList.size() == 0)
		{
			ShowIndex = -1;
		}
		else
		{
			ShowIndex += Ofs;

			while (ShowIndex < 0) ShowIndex += (int)DisplayList.size();
			while (ShowIndex >= (int)DisplayList.size()) ShowIndex -= DisplayList.size();
		}
	}
	return (true);
}

// 現在表示されている画像から Ofs ずらしたインデックスの画像を表示する
bool CMainForm::ShowOffsetImage(int Ofs)
{
	if (AutoLoadFileFolder == true && ShowingList == false && DisplayList.size() == 1 && InArchive == false && SlideShow == false)
	{
		std::wstring FolderName = acfc::GetFolderName(OpeningFileName);
		if (FolderName == TEXT("")) return (true);

		std::vector<std::wstring> TempSL;
		TempSL.push_back(FolderName);
		OpenFiles(TempSL, OpeningFileName, Ofs, false);
	}
	else if (ShowingList == false && DisplayList.size() > 0)
	{
		ShowIndex += Ofs;

		if (InArchive && (ShowIndex < 0 || ShowIndex >= (int)DisplayList.size()))
			JumpBorderArcNml(Ofs);

		while (ShowIndex < 0)
			ShowIndex += DisplayList.size();

		while (ShowIndex >= (int)DisplayList.size())
			ShowIndex -= DisplayList.size();

		ShowAbsoluteImage(ShowIndex, Ofs);
	}
	return (true);
}

// 指定された値の画像を表示する Ofs は画像表示に失敗したときにどの方向の画像を選ぶか
// -1 なら一つ上の画像、+1 なら一つしたの画像
int CMainForm::ShowAbsoluteImage(int Index, int Ofs)
{
	return (ShowAbsoluteImage(Index, -1, Ofs, true));
}

int CMainForm::ShowAbsoluteImage(int Index, int Ofs, bool MustShowImage)
{
	return (ShowAbsoluteImage(Index, -1, Ofs, MustShowImage));
}

int CMainForm::ShowAbsoluteImage(int Index, int SubIndex, int Ofs, bool MustShowImage)
{
	if (FileLoading > 0) return (0);
	FileLoading++;

	ShowIndex = Index;
	ELoadFileResult Result = ELoadFileResult_OK;

	SSTimer.Enabled(false);

	while (DisplayList.size() > 0)
	{
		Result = LoadFile(ShowIndex, SubIndex, Ofs, MustShowImage);

		if (InArchive == false)
			ShowIndexBack = -1;

		switch (Result)
		{
			// MIV_SI_OK:正常終了
		case ELoadFileResult_OK:
			RotateValue = Susie.Rotate;
			CheckRotateCheck();
			SSTimer.Enabled(SlideShow);
			if (InArchive == false)
			{
				SetImageFileNameString(DisplayList[ShowIndex].FileName);
				//					FileChanged.SetCheckFile(OpeningFileName);
			}
			FileLoading--;
			return (0);
			// MIV_SI_OPENARCHIVE:アーカイブファイルを開いた
		case ELoadFileResult_OPENARCHIVE:
			if (MustShowImage)
			{
				SetNewImageSize();
				//					Hint = GetShortFileName(DisplayList.Strings[ShowIndex]);
			}
			SSTimer.Enabled(SlideShow);
			FileLoading--;
			return (1);

			// MIV_SI_NOARCHIVE:アーカイブファイルがみつからない
		case ELoadFileResult_NOARCHIVE:
			if (InArchive == true) CloseArchiveMode();

			DisplayList.erase(DisplayList.begin() + ShowIndex);
			DisplayBox.Delete(ShowIndex);

			if (Ofs >= 0)
			{
				if (ShowIndex >= (int)DisplayList.size())
				{
					if (InArchive == true)
						JumpBorderArcNml(Ofs);
					else
						ShowIndex = 0;
				}
			}
			else
			{
				ShowIndex--;
				if (ShowIndex < 0)
				{
					if (InArchive == true)
						ShowIndex = DisplayList.size() - 1;
					else
						JumpBorderArcNml(Ofs);
				}
			}
			break;

			// MIV_SI_NOMASKORFILE:マスクに該当しない
		case ELoadFileResult_FAILEDORNOMASK:

			if (InArchive)
				DeleteArchiveData(DisplayList[ShowIndex].FileName);

			DisplayList.erase(DisplayList.begin() + ShowIndex);

			if (Ofs >= 0)
			{
				if (ShowIndex >= (int)DisplayList.size())
				{
					if (InArchive == true)
						JumpBorderArcNml(Ofs);
					else
						ShowIndex = 0;
				}
			}
			else
			{
				ShowIndex--;
				if (ShowIndex < 0)
				{
					if (InArchive == true)
						ShowIndex = DisplayList.size() - 1;
					else
						JumpBorderArcNml(Ofs);
				}
			}
			break;
			// MIV_SI_RETRY:もう一度表示にトライする
		case ELoadFileResult_RETRYSAMEINDEX:
			continue;

		}
	}

	ShowIndex = -1;
	//	Hint = "";
	Susie.Clear(EPluginMode_ALL);
	FormRefresh();

	if (Result == ELoadFileResult_OK)
	{
		RotateValue = Susie.Rotate;
		CheckRotateCheck();
	}

	SSTimer.Enabled(SlideShow);
	OpeningFileName = TEXT("");

	FileLoading--;
	return (2);
}

//-----------------------------------------------------------------------------------------------------
//
// スライドショー
//
//-----------------------------------------------------------------------------------------------------


void CMainForm::IntervalOffset(int i)
{
	int Index = -1;
	int j;
	for (j = 0; j < SLIDESHOWNUM; j++)
	{
		if (SSInterval <= SlideShows[j])
		{
			Index = j;
			break;
		}
	}

	Index += i;
	if (Index < 0) Index = 0;
	if (Index >= SLIDESHOWNUM) Index = SLIDESHOWNUM - 1;
	SSInterval = SlideShows[Index];
	SSTimer.SetInterval(SSInterval);
}



//-----------------------------------------------------------------------------------------------------
//
// GIFAnime
//
//-----------------------------------------------------------------------------------------------------
void CMainForm::GifAnimeTimer(void)
{
	while (Susie.GIFLooping == true)
	{
		// GIF アニメの処理
		if (Susie.GIFAnimate == true && GIFRefresh == false)
		{
			int WaitLeft; // フレームの更新が間に合わなかったときに間引いたほうがいい？
			WaitLeft = Susie.DelayTime;

			while (WaitLeft > MIN_LOOP)
			{
				Sleep(MIN_LOOP);
				WaitLeft -= MIN_LOOP;
				if (Susie.GIFLooping == false)return;
			}
			Sleep(WaitLeft);
			if (Susie.GIFLooping == false)return;

			
			{
				EnterCriticalSection(&CriticalSection);
				bool WaitCloseDialog = GIFAnimePaused;
				LeaveCriticalSection(&CriticalSection);

				while (WaitCloseDialog)
				{
					Sleep(MIN_LOOP);

					EnterCriticalSection(&CriticalSection);
					WaitCloseDialog = GIFAnimePaused;
					LeaveCriticalSection(&CriticalSection);
				}
			}

			GIFRefresh = true;
			FormRefresh();
		}
		if (Susie.GIFLooping == false)return;
	}
}

void CMainForm::BeginGIFAnimeThread(void)
{
	Susie.GIFLooping = true;

	hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadGIFAnime, this, 0, &ThreadID);
	InitializeCriticalSection(&CriticalSection);
}

void CMainForm::EndGIFAnimeThread(void)
{	
	if (hThread == nullptr)return;
	EnterCriticalSection(&CriticalSection);
	Susie.GIFLooping = false;
	LeaveCriticalSection(&CriticalSection);

	BOOL ExitCode = FALSE;
	DWORD Param;
	while (ExitCode == FALSE)
	{
		Sleep(MIN_LOOP);
		ExitCode = GetExitCodeThread(hThread, &Param);
	}

	DeleteCriticalSection(&CriticalSection);
	CloseHandle(hThread);
	hThread = nullptr;

	Susie.GIFLooping = false;
}


//-----------------------------------------------------------------------------------------------------
//
// 以下メニュークリック
//
//-----------------------------------------------------------------------------------------------------

void CMainForm::MnToggleVisible_Click(void)
{
	if (HideTaskButton == true)
	{
		if (Visible == false)
		{
			ShowWindow(hWindow, SW_SHOW);
			Visible = true;
		}
		else
		{
			ShowWindow(hWindow, SW_HIDE);
			Visible = false;
		}
	}
	else
	{
		if (Visible == false)
		{
			ShowWindow(hWindow, SW_SHOWNORMAL);
			Visible = true;
		}
		else
		{
			ShowWindow(hWindow, SW_MINIMIZE);
			Visible = false;
		}
	}
}



void CMainForm::MnFitToScreen_Click(void)
{
	if(Initialized == true)FitToScreen = !FitToScreen;

	SetMenuCheck(hPopupMenu, ID_POPUP_FITTOSCREEN, FitToScreen);

	GetMonitorParameter();

	SetDisableFitMode(2);

	FitMode = EFitMode_NONE;
	if (FitToScreen == true)
	{
		SetNewImageSize();
	}
}

void CMainForm::MnFullScreen_Click(void)
{
	if (Initialized == true)FullScreen = !FullScreen;
	GoToFullScreen(FullScreen);
	SetMenuCheck(hPopupMenu, ID_POPUP_FULLSCREEN, FullScreen);
}


void CMainForm::MnUseWholeScreen_Click(void)
{
	if (Initialized == true)UseWholeScreen = !UseWholeScreen;

	SetMenuCheck(hPopupMenu, ID_POPUP_USEWHOLESCREEN, UseWholeScreen);

	GetMonitorParameter();

	SetDisableFitMode(8);

	FitMode = EFitMode_NONE;
	if (UseWholeScreen == true)
	{
		SetNewImageSize();
	}
}

void CMainForm::MnInScreen_Click(void)
{
	if (UseWholeScreen) MnUseWholeScreen_Click();

	GetMonitorParameter();

	double dTop, dLeft, dWidth, dHeight, pWidth, pHeight;

	dLeft = WLeft;
	dTop = WTop;
	dWidth = pWidth = WWidth;
	dHeight = pHeight = WHeight;

	if (dWidth > MWidth) dWidth = MWidth;
	if (dHeight > MHeight) dHeight = MHeight;
	if ((double)dWidth > (double)dHeight * Ratio)
		dWidth = (double)dHeight * Ratio;
	else
		dHeight = (double)dWidth / Ratio;

	dLeft += (pWidth - dWidth) / 2;
	dTop += (pHeight - dHeight) / 2;

	if (dLeft < MLeft) dLeft = MLeft;
	if (dTop < MTop) dTop = MTop;
	if (dLeft + dWidth > MRight) dLeft = MRight - dWidth;
	if (dTop + dHeight > MBottom) dTop = MBottom - dHeight;

	WLeft = (int)dLeft; WTop = (int)dTop; WWidth = (int)dWidth; WHeight = (int)dHeight;
	SetCenter((int)(dLeft + dWidth / 2), (int)(WTop + dHeight / 2));
	SyncWindow();

	acfc::SetAbsoluteForegroundWindow(hWindow);


}

// ピクセル等倍で表示
void CMainForm::MnShowPbyP_Click(void)
{
	FixViewOut();
	if (SizeRatio == 1.0) return;

	SizeRatio = 1;

	if (WLeft + WWidth / 2 == CenterX && CenterY == WTop + WHeight / 2)
		SetWindowSize(Susie.ORotWidth, Susie.ORotHeight, false);
	else
		SetWindowSize(Susie.ORotWidth, Susie.ORotHeight, true);

	SetDiagonalLength();
}

// 中央に移動
void CMainForm::MnCenter_Click(void)
{
	GetMonitorParameter();

	WLeft = MLeft + MWidth / 2;
	WTop = MTop + MHeight / 2;
	SetCenter(MLeft + MWidth / 2, MTop + MHeight / 2);

	SetWindowSize(WWidth, WHeight, false);
}

void CMainForm::MnPositionMode_Click(int Index)
{
	SetPositionMode(Index);
	CheckPositionModeMenuCheck();
}

void CMainForm::MnRotateMode_Click(int Index)
{
	if (RotateValue != AbsoluteRotates[Index])
	{
		AbsoluteRotate(AbsoluteRotates[Index]);
	}
}

void CMainForm::MnRotateOffset_Click(int Index)
{
	OffsetRotate(OffsetRotates[Index]);
}

void CMainForm::MnRotateFix_Click(void)
{
	if (Initialized == true)FixRotate = !FixRotate;
	SetMenuCheck(hPopupMenu, ID_ROTATE_FIXROTATION, FixRotate);
	Susie.FixRotate = FixRotate;
}

void CMainForm::MnAlwaysTop_Click(void)
{
	if (Initialized == true)AlwaysTop = !AlwaysTop;
	SetMenuCheck(hPopupMenu, ID_POPUP_ALWAYSONTOP, AlwaysTop);
	RestoreStayOnTop();
}

void CMainForm::MnFixDiagonalLength_Click(bool SetNewValue)
{
	if (Initialized == true)FixDiagonalLength = !FixDiagonalLength;
	SetMenuCheck(hPopupMenu, ID_POPUP_FIXDIAGONALLENGTH, FixDiagonalLength);

	SetDisableFitMode(4);

	FitMode = EFitMode_NONE;
	if (FixDiagonalLength == true)
	{
		if (SetNewValue == true && Initialized == true) SetDiagonalLength();
		SetNewImageSize();
	}
}

void CMainForm::MnTransparency_Click(int Index)
{
	if (Initialized == true)FormAlphaBlendValue = AlphaBlends[Index];
	CheckAlphaValueMenuCheck();
}

void CMainForm::MnFixRatio_Click(void)
{
	if (Initialized == true)FixSizeRatio = !FixSizeRatio;
	SetMenuCheck(hPopupMenu, ID_POPUP_FIXSIZERATIO, FixSizeRatio);

	SetDisableFitMode(1);

	FitMode = EFitMode_NONE;
}

void CMainForm::MnSetWindowSize_Click(void)
{
	NoStayOnTop();
	std::vector<std::wstring> temp;

	temp.push_back(LoadStringResource(IDS_MES_1050));
	temp.push_back(LoadStringResource(IDS_MES_1051));
	temp.push_back(LoadStringResource(IDS_MES_1052));

	FormSizeForm->SetData(temp, Susie.ORotWidth, Susie.ORotHeight);
	if (FormSizeForm->ShowDialog(appInstance, hWindow) == IDOK)
	{
		int DestWidth, DestHeight;
		FormSizeForm->GetData(DestWidth, DestHeight);

		FixViewOut();
		SetWindowSize(DestWidth, DestHeight, false);
		SetDiagonalLength();
	}

	RestoreStayOnTop();
}

void CMainForm::MnSlideShow_Click(void)
{
	if (Initialized == true)SlideShow = !SlideShow;
	SetMenuCheck(hPopupMenu, ID_POPUP_SLIDESHOW, SlideShow);
	if (SlideShow)
	{
		SSTimer.SetInterval(SSInterval);
		SSTimer.Enabled(true);
	}
	else
	{
		SSTimer.Enabled(false);
	}
	SSIcon = 0;
	DrawSSIcon();
}

void CMainForm::MnRefresh_Click(void)
{
	ShowAbsoluteImage(ShowIndex, 0);
}

void CMainForm::MnLock_Click(void)
{
	if (Initialized == true)Locked = !Locked;
	SetMenuCheck(hPopupMenu, ID_POPUP_LOCK, Locked);

	POINT p;
	GetCursorPos(&p);
	PositionType = GetCursorPositionType(p);

	FormRefresh();
}

//-----------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------

void CMainForm::MnOpenFile_Click(void)
{
	NoStayOnTop();
	std::wstring FolderName = GetImageFileFolder();
	OpenFileDialog.InitialDirectory = FolderName;
	OpenFileDialog.FileName = TEXT("");

	if (OpenFileDialog.ShowDialog(hWindow))
	{
		std::vector<std::wstring> Files;
		
		OpenFileDialog.GetFiles(Files);
		AddHistoryList(Files);
		OpenFiles(Files);

	}
	RestoreStayOnTop();
}

void CMainForm::MnOpenFolder_Click(void)
{
	NoStayOnTop();
	std::wstring FolderName = GetImageFileFolder();

	FolderSelectDialog.Title = LoadStringResource(IDS_MES_1049);
	FolderSelectDialog.InitialDirectory = FolderName;

	if (FolderSelectDialog.ShowDialog(hWindow))
	{
		FolderName = FolderSelectDialog.FolderName;

		if (FolderName == TEXT("")) return;

		AddHistoryList(FolderName);

		std::vector<std::wstring> TempSL;
		TempSL.push_back(FolderName);

		OpenFiles(TempSL);
	}
	RestoreStayOnTop();
}

void CMainForm::MnOpenFolderExistingShowingFile_Click(void)
{
	if (FileList.size() == 0) return;

	std::wstring FolderName = acfc::GetFolderName(OpeningFileName);
	if (FolderName == TEXT("")) return;

	std::vector<std::wstring> TempSL;
	TempSL.push_back(FolderName);
	OpenFiles(TempSL, OpeningFileName, 0, false);
}

void CMainForm::MnHisotryMenu_Click(int Index)
{
	std::vector<std::wstring> TempSL;
	TempSL.push_back(HistoryList[Index]);
	AddHistoryList(TempSL);
	OpenFiles(TempSL);
}

void CMainForm::MnCloseArchive_Click(void)
{
	if (InArchive == false) return;

	ShowIndex = DisplayList.size();

	if (ShowingList == true)
		JumpBorderArcNml(0);
	else
		JumpBorderArcNml(1);

	while (ShowIndex < 0)
		ShowIndex += DisplayList.size();

	while (ShowIndex >= (int)DisplayList.size())
		ShowIndex -= DisplayList.size();

	if (ShowingList == true)
		SetFileList();
	else
		ShowAbsoluteImage(ShowIndex, 1);
}

//-----------------------------------------------------------------------------------------------------

void CMainForm::MnToggleShow_Click(void)
{
	ShowWindow(hWindow, SW_SHOWNORMAL);
	ToggleShowList(EShowMode_TOGGLE);
}

//-----------------------------------------------------------------------------------------------------

void CMainForm::MnShowInformation_Click(void)
{
	std::wstring Mes = TEXT("");
	NoStayOnTop();

	if (ShowIndex < 0)
	{
		Mes = TEXT("version ") + GetFileVersionString() + TEXT("\ncopyright(C) Downspade software.\nAll rights reserved.");
	}
	else
	{
		double P = (double)WWidth / Susie.OrgWidth * 100.0;
		Mes = acfc::GetMiniPathName(DisplayList[ShowIndex].FileName, 1) + TEXT("\n");

		if (InArchive) Mes += FileList[ShowIndexBack].FileName + TEXT("\n");

		Mes += TEXT("\n") + std::to_wstring(Susie.OrgWidth) + TEXT(" px x ") + std::to_wstring(Susie.OrgHeight) + TEXT(" px\n")
			+ acfc::FormatString(P, 1, false) + TEXT(" %\n\n")
			+ acfc::GetMetricPrefixString(DisplayList[ShowIndex].FileSize, 3, TEXT("")) + TEXT("bytes (") + std::to_wstring(DisplayList[ShowIndex].FileSize) + TEXT(")bytes\n")
			+ TEXT("\nLoader : ") + Susie.PluginName;

		if (Susie.GIFAnimate == true)
		{
			Mes += TEXT("\nFrame Count : ") + std::to_wstring(Susie.FrameCount);
			if (Susie.LoopCount == 0)
				Mes += TEXT("\nLoop : Infinite");
			else
				Mes += TEXT("\nLoop : ") + std::to_wstring(Susie.LoopCount);
			Mes += TEXT("\nPrevious Loop Drop Frame : ") + std::to_wstring(Susie.DropFrame);
		}

	}

	MessageBox(hWindow, Mes.c_str(), TEXT("Information"), MB_OK | MB_ICONINFORMATION);

	RestoreStayOnTop();
}

//-----------------------------------------------------------------------------------------------------

void CMainForm::MnCopyImage_Click(void)
{
	Susie.CopyImageToClipboard();
}

//-----------------------------------------------------------------------------------------------------

void CMainForm::MnFileCopy_Click(void)
{
	// DROPEFFECT_COPY
	NoStayOnTop();
	if (FileList.size() == 0) return;

	if (InArchive == true)
	{
		MessageBox(hWindow, LoadStringResource(IDS_MES_1016).c_str(), TEXT("Information"), MB_OK | MB_ICONINFORMATION);
	}

	//コピーするファイルのパスをstd::wstringCollectionに追加する
	std::vector<std::wstring> files;
	files.push_back(OpeningFileName);
	//クリップボードにコピーする
	acfc::FilesToClipboard(files, DROPEFFECT_COPY);
	RestoreStayOnTop();
}

void CMainForm::MnFileCut_Click(void)
{
	// DROPEFFECT_MOVE
	NoStayOnTop();
	if (FileList.size() == 0) return;

	if (InArchive == true)
	{
		Susie.PauseGIFAnimate();
		MessageBox(hWindow, LoadStringResource(IDS_MES_1017).c_str(), TEXT("Information"), MB_OK | MB_ICONINFORMATION);
		Susie.RestartGIFAnimate();
	}

	//ファイルドロップ形式のDataObjectを作成する
	std::vector<std::wstring> files;
	files.push_back(OpeningFileName);
	//クリップボードにコピーする
	acfc::FilesToClipboard(files, DROPEFFECT_MOVE);
	RestoreStayOnTop();
}

void CMainForm::MnFilePaste_Click(void)
{
	std::vector<std::wstring> pasteList;

	if (acfc::FilesFromClipboard(pasteList) == 0)return;

	AddHistoryList(pasteList);
	OpenFiles(pasteList, pasteList[0], 0, true);
}

//-----------------------------------------------------------------------------------------------------

void CMainForm::MnFileRename_Click(void)
{
	NoStayOnTop();
	std::wstring temp = GetImageFileName();
	std::wstring NewFileName = acfc::GetFileNameWithoutExt(temp);
	std::wstring FileExt = acfc::GetFileExt(temp);


	InputForm->SetData(LoadStringResource(IDS_MES_1018), LoadStringResource(IDS_MES_1019), NewFileName);
	if (InputForm->ShowDialog(appInstance, hWindow) == IDOK)
	{
		NewFileName = InputForm->Result;
		int i;
		i = acfc::CheckFileIrregularChar(NewFileName);
		if (i < 0)
		{
			NewFileName = acfc::GetFolderName(OpeningFileName) + TEXT("\\") + NewFileName + FileExt;

			if (acfc::FileExists(NewFileName) == true)
			{
				MessageBox(hWindow, LoadStringResource(IDS_MES_1020).c_str(), TEXT("Information"), MB_OK | MB_ICONINFORMATION);
			}
			else
			{
				MoveFile(OpeningFileName.c_str(), NewFileName.c_str());
				ChangeImageFileName(NewFileName);

				MessageBox(hWindow, LoadStringResource(IDS_MES_1021).c_str(), TEXT("Information"), MB_OK | MB_ICONINFORMATION);
			}
		}
		else
		{
			std::wstring mes = NewFileName + LoadStringResource(IDS_MES_1022) + NewFileName[i] + LoadStringResource(IDS_MES_1023);
			MessageBox(hWindow, mes.c_str(), TEXT("Failed"), MB_OK | MB_ICONEXCLAMATION);
		}
	}

	RestoreStayOnTop();
}

void CMainForm::MnCopyFilePath_Click(void)
{
	acfc::CopyStringToClipboard(GetImageFileName());
}

void CMainForm::MnMoveFile_Click(void)
{
	POINT p;
	GetCursorPos(&p);
	PopupFileMovePopup(p);
}

void CMainForm::MnFileMoveToRecycle_Click(bool FromMenu)
{
	if (FileList.size() == 0) return;

	if (ShowingList == false)
	{
		DisplayBox.SelectAll(false);
		DisplayBox.SetSelected(ShowIndex, true);
		DisplayBox.SelectedIndex = ShowIndex;
		DisplayBox.SetSelectedIndex();
	}

	int SubKey = 0;

	if (FromMenu == false)
	{
		if (((GetAsyncKeyState(VK_LSHIFT) & 0x8000) | (GetAsyncKeyState(VK_RSHIFT) & 0x8000))) SubKey |= 1;
		if (((GetAsyncKeyState(VK_LCONTROL) & 0x8000) | (GetAsyncKeyState(VK_RCONTROL) & 0x8000))) SubKey |= 2;
		if (((GetAsyncKeyState(VK_LMENU) & 0x8000) | (GetAsyncKeyState(VK_RMENU) & 0x8000))) SubKey |= 4;
	}
	else
	{
		SubKey = 2;
	}

	if (SubKey == 0)
		DeleteFileList(0);
	else if (SubKey == 2)
		DeleteFileList(1);
	else if (SubKey == 3)
		DeleteFileList(2);
}


void CMainForm::MnOpenExistsFolder_Click(void)
{
	if (FileList.size() == 0) return;

	acfc::SelectFileInExplorer(GetImageFileName());
}

//-----------------------------------------------------------------------------------------------------

void CMainForm::MnJpegSave_Click(void)
{
	if (Susie.Showing == false) return;

	NoStayOnTop();
	std::wstring Dest = DisplayList[ShowIndex].FileName;

	SaveJpegDialog.InitialDirectory = acfc::GetFolderName(Dest);

	Dest = acfc::GetNonOverwrapFileName(acfc::ChangeFileExt(Dest, TEXT(".jpg")));
//	Dest = acfc::GetFileName(Dest);

	SaveJpegDialog.FileName = Dest;
	SaveJpegDialog.Title = LoadStringResource(IDS_MES_1043);

	if (SaveJpegDialog.ShowDialog(hWindow) == IDOK)
	{
		Susie.SaveJpeg(SaveJpegDialog.FileName, Susie.OrgWidth, Susie.OrgHeight, JCR[0]);
	}

	RestoreStayOnTop();
}

void CMainForm::MnJpegSaveShowingSize_Click(void)
{
	if (Susie.Showing == false) return;

	NoStayOnTop();
	std::wstring Dest = DisplayList[ShowIndex].FileName;

	SaveJpegDialog.InitialDirectory = acfc::GetFolderName(Dest);

	Dest = acfc::GetNonOverwrapFileName(acfc::ChangeFileExt(Dest, TEXT(".jpg")));
	Dest = acfc::GetFileName(Dest);

	SaveJpegDialog.FileName = Dest;
	SaveJpegDialog.Title = LoadStringResource(IDS_MES_1044);

	if (SaveJpegDialog.ShowDialog(hWindow) == IDOK)
	{
		Susie.SaveJpeg(SaveJpegDialog.FileName, WWidth, WHeight, JCR[0]);
	}
	RestoreStayOnTop();
}

void CMainForm::MnJpegSaveSetting_Click(void)
{
	NoStayOnTop();

	JpegSettingForm->SetData(LoadStringResource(IDS_MES_1011), LoadStringResource(IDS_MES_1012), JCR);
	if (JpegSettingForm->ShowDialog(appInstance, hWindow) == IDOK)
	{
		JpegSettingForm->GetData(JCR);
	}

	RestoreStayOnTop();

}

//-----------------------------------------------------------------------------------------------------

void CMainForm::MnSavePNG_Click(void)
{
	if (Susie.Showing == false) return;

	NoStayOnTop();
	std::wstring Dest = DisplayList[ShowIndex].FileName;

	SavePNGDialog.InitialDirectory = acfc::GetFolderName(Dest);

	Dest = acfc::GetNonOverwrapFileName(acfc::ChangeFileExt(Dest, TEXT(".png")));
	Dest = acfc::GetFileName(Dest);

	SavePNGDialog.FileName = Dest;
	SaveJpegDialog.Title = LoadStringResource(IDS_MES_1048);

	if (SavePNGDialog.ShowDialog(hWindow))
	{
		Susie.SavePNG(SavePNGDialog.FileName, Susie.SrcRWidth, Susie.SrcRHeight);
	}

	RestoreStayOnTop();

}

//-----------------------------------------------------------------------------------------------------

void CMainForm::MnSortFileList_Click(int Index)
{
	SortFileList(SortTypes[Index]);
}

//-----------------------------------------------------------------------------------------------------

void CMainForm::MnSearchSubFolders_Click(void)
{
	if (Initialized == true)SearchSubFolder = !SearchSubFolder;
	SetMenuCheck(hPopupMenu, ID_OPTION_SEARCHSUBFOLDERS, SearchSubFolder);

}

void CMainForm::MnKeepPreviousFiles_Click(void)
{
	if (Initialized == true)LoadLastFile = !LoadLastFile;
	SetMenuCheck(hPopupMenu, ID_OPTION_KEEPPREVIOUSFILES, LoadLastFile);
}

void CMainForm::MnEnableFileMask_Click(void)
{
	if (Initialized == true)EnableFileMask = !EnableFileMask;
	SetMenuCheck(hPopupMenu, ID_OPTION_ENABLEFILEMASK, EnableFileMask);
}

void CMainForm::MnFileMaskSetting_Click(void)
{
	NoStayOnTop();
	InputForm->SetData(LoadStringResource(IDS_MES_1024), LoadStringResource(IDS_MES_1025), FileMaskString);
	if (InputForm->ShowDialog(appInstance, hWindow) == IDOK)
	{
		FileMaskString = InputForm->Result;
	}
	RestoreStayOnTop();
}

void CMainForm::MnAutoLoadFileFolder_Click(void)
{
	if (Initialized == true)AutoLoadFileFolder = !AutoLoadFileFolder;
	SetMenuCheck(hPopupMenu, ID_OPTION_AUTOLOADFILEFOLDER, AutoLoadFileFolder);

}

//-----------------------------------------------------------------------------------------------------

void CMainForm::MnKeepPreviousPosition_Click(void)
{
	if (Initialized == true)KeepPreviousPosition = !KeepPreviousPosition;
	SetMenuCheck(hPopupMenu, ID_OPTION_KEEPPREVIOUSPOSITION, KeepPreviousPosition);

}

void CMainForm::MnSlideShowInterval_Click(int Index)
{
	if (Initialized == true)SSInterval = SlideShows[Index] * 1000;
	CheckIntervalMenuCheck();
	SSTimer.SetInterval(SSInterval);

}

void CMainForm::MnWheelSensitivity_Click(int Index)
{
	if (Initialized == true)WheelSensitivity = WheelSensitivities[Index];
	CheckWheelSenseCheck();
}

void CMainForm::MnBackGroundColor_Click(void)
{
	NoStayOnTop();

	ColorDialog.ColorRef = FullFillColor.ToCOLORREF();
	if (ColorDialog.ShowDialog(hWindow) == IDOK)
	{
		FullFillColor = ColorDialog.ColorRef;
		DrawColor = GetDrawColor(FullFillColor);
		FormRefresh();
	}

	RestoreStayOnTop();
}

void CMainForm::MnHideTaskButton_Click(void)
{
	if (Initialized == true)HideTaskButton = !HideTaskButton;

	SetMenuCheck(hPopupMenu, ID_OPTION_HIDETASKBUTTON, HideTaskButton);

	SetTaskButtonVisibility();

	if (Visible == true)
	{
		ShowWindow(hWindow, SW_HIDE);
		ShowWindow(hWindow, SW_SHOW);
	}
	else
	{
		if (HideTaskButton == true)
		{
			ShowWindow(hWindow, SW_SHOWNORMAL);
		}
		else
		{
			ShowWindow(hWindow, SW_MINIMIZE);
		}
	}
}

void CMainForm::MnBorder_Click(int Index)
{
	if (Initialized == true)FrameWidth = FrameWidths[Index];
	CheckFrameWidthMenuCheck();
	GetMonitorParameter();
	SyncWindow();

}

void CMainForm::MnBorderColor_Click(void)
{
	NoStayOnTop();

	ColorDialog.ColorRef = FrameColor.ToCOLORREF();
	if (ColorDialog.ShowDialog(hWindow) == IDOK)
	{
		FrameColor = ColorDialog.GdipColor;
		FormRefresh();
	}

	RestoreStayOnTop();
}

void CMainForm::MnFontSize_Click(int Index)
{
	if (Initialized == true)ApplicationFontSize = FontSizes[Index];
	CheckFontSizeMenuCheck();

}

void CMainForm::MnTrayIcon_Click(int Index)
{

	int PreTrayIconColor = TrayIconColor;
	if (Initialized == true)TrayIconColor = Index;

	SetMenuCheckType(hPopupMenu, mnTrayIcons[PreTrayIconColor], 0);
	SetTrayIcon(ESetTrayIconMode_MODIFY);
	SetMenuCheckType(hPopupMenu, mnTrayIcons[TrayIconColor], 2);

}

void CMainForm::MnCursorSize_Click(int Index)
{
	if (Initialized == true)UseCursorSize = Index;
	CheckUseCursorSizeCheck();

}

void CMainForm::MnFileMove_Click(int Index)
{
	std::wstring Src;

	Src = GetImageFileName();

	std::wstring Folder = acfc::GetFolderName(Src);
	std::wstring Dest;
	std::wstring MoveFolder = MoveData[Index];

	if(MoveFolder == TEXT("?"))return;

	if (MoveFolder == TEXT("*"))
	{
		NoStayOnTop();
		CreateFolder();
		RestoreStayOnTop();
	}
	else 
	{
		if (MoveFolder == TEXT("??"))
		{
			Folder = acfc::GetParentFolder(Folder) + TEXT("\\");
		}
		else
		{
			Folder = MoveFolder + TEXT("\\");
		}

		Dest = Folder + acfc::GetFileName(Src);

		if (acfc::FileExists(Dest) == true)
		{
			if (
				((GetAsyncKeyState(VK_LCONTROL) & 0x8000)
					| (GetAsyncKeyState(VK_RCONTROL) & 0x8000))
				)
			{
				Dest = acfc::GetNonOverwrapFileName(Dest);
			}
			else
			{
				NoStayOnTop();
				int Result;
				Result = MessageBox(hWindow, LoadStringResource(IDS_MES_1039).c_str(), TEXT("Confirmation"), MB_YESNO | MB_ICONQUESTION);

				if (Result == IDNO) return;
				RestoreStayOnTop();
			}
		}

		//  FileChanged.SetCheckFile("");
		MoveFile(Src.c_str(), Dest.c_str());
		//  FileChanged.SetCheckFile(Dest);

		SetImageFileName(Dest);
	}

	CreateFileMovePPMenu();
	
	POINT p;
	RECT r;
	GetMenuItemRect(hWindow, hMoveMenu, 0, &r);
	p.x = r.left; p.y = r.top;

	PopupFileMovePopup(MovePPPosition);
}



//------------------------------------------------------------------------------------

void CMainForm::MnSetPluginFolder_Click(void)
{
	NoStayOnTop();

	int i;
	std::wstring Temp;

	if (OpenPluginDialog.ShowDialog(hWindow) != IDOK) return;

	Temp = acfc::GetFolderName(OpenPluginDialog.FileName);

	auto itr = std::find(SpiPathes.begin(), SpiPathes.end(), Temp);

	if (itr == SpiPathes.end())
	{
		if (SpiPathes.size() > 0) SpiPathes.erase(SpiPathes.begin());
		if (SpiPathes.size() > 0)
			SpiPathes.insert(SpiPathes.begin(), Temp);
		else
			SpiPathes.push_back(Temp);
	}

	Susie.SetSpiPathes(SpiPathes);     // SPI path set

	Temp = LoadStringResource(IDS_MES_1026);
	for (i = 0; i < (int)SpiPathes.size(); i++)
		Temp = Temp + SpiPathes[0];

	Temp = Temp + LoadStringResource(IDS_MES_1027);

	MessageBox(hWindow, Temp.c_str(), TEXT("Information"), MB_OK | MB_ICONINFORMATION);

	RestoreStayOnTop();
}

void CMainForm::MnOpenPluginDialog_Click(void)
{
	NoStayOnTop();

	if ((Susie.Mode & EPluginMode_SPI) != 0) Susie.OpenSpiSettingDialog(hWindow);

	RestoreStayOnTop();
}

void CMainForm::MnSave_Click(void)
{
	NoStayOnTop();

	if (SavemflDialog.ShowDialog(hWindow) == IDOK)
		SaveFileList(SavemflDialog.FileName);

	RestoreStayOnTop();
}


void CMainForm::MnInternalLoader_Click(void)
{
	NoStayOnTop();
	InputForm->SetData(LoadStringResource(IDS_MES_1053), LoadStringResource(IDS_MES_1054), Susie.InternalLoader);
	if (InputForm->ShowDialog(appInstance, hWindow) == IDOK)
	{
		Susie.InternalLoader = InputForm->Result;
	}
	RestoreStayOnTop();
}

void CMainForm::MnCreateInstance_Click(void)
{
	NoStayOnTop();

	std::wstring NewIniParamName, Temp;

	MessageBox(hWindow, LoadStringResource(IDS_MES_1028).c_str(), TEXT("Information"), MB_OK | MB_ICONINFORMATION);

	Temp = IniFolderName + TEXT("\\milligram");
	SaveIniDialog.InitialDirectory = Temp;

	while (true)
	{
		if (SaveIniDialog.ShowDialog(hWindow) == IDOK)
		{
			if (acfc::GetFolderName(SaveIniDialog.FileName) == Temp)
			{
				SaveIniDialog.FileName = Temp + TEXT("\\") + acfc::GetFileName(SaveIniDialog.FileName);

				NewIniParamName = acfc::GetFileNameWithoutExt(SaveIniDialog.FileName);

				CreateShortCut(NewIniParamName);
				DoCreateSendToLink(NewIniParamName);
				SaveIni(NewIniParamName);

				SetInstanceMode(NewIniParamName);
				break;
			}
			else
			{
				MessageBox(hWindow, LoadStringResource(IDS_MES_1029).c_str(), TEXT("Information"), MB_OK | MB_ICONINFORMATION);
			}
		}
		else
			break;
	}

	RestoreStayOnTop();
}

void CMainForm::MnDeleteInstance_Click(void)
{
	NoStayOnTop();
	int Result;

	Result = MessageBox(hWindow, LoadStringResource(IDS_MES_1033).c_str(), TEXT("Information"), MB_OKCANCEL | MB_ICONINFORMATION);

	if (Result == IDOK)
	{
		DeleteFile((IniFolderName + TEXT("\\milligram\\") + IniParamName + TEXT(".ini")).c_str());

		bool Del = true;
		if (acfc::FileExists(ShortCutFileName))
		{
			try
			{
				DeleteFile(ShortCutFileName.c_str());
			}
			catch(...)
			{
				Del = false;
			}

		}

		if (CreateSendToLink)
			DeleteSendToLink(IniParamName);

		if (Del == true)
		{
			MessageBox(hWindow, LoadStringResource(IDS_MES_1031).c_str(), TEXT("Information"), MB_OK | MB_ICONINFORMATION);
		}
		else
		{
			MessageBox(hWindow, LoadStringResource(IDS_MES_1032).c_str(), TEXT("Information"), MB_OK | MB_ICONINFORMATION);
		}

		NotSaveIni = true;
		Close();
	}

	RestoreStayOnTop();
}

void CMainForm::MnUninstall_Click(void)
{
	NoStayOnTop();
	int Result;

	Result = MessageBox(hWindow, LoadStringResource(IDS_MES_1033).c_str(), TEXT("Information"), MB_OKCANCEL | MB_ICONINFORMATION);
	if (Result == IDOK)
	{
		if (DoUninstall() == true)
		{
			MessageBox(hWindow, LoadStringResource(IDS_MES_1034).c_str(), TEXT("Information"), MB_OK | MB_ICONINFORMATION);
		}
		else
		{
			MessageBox(hWindow, LoadStringResource(IDS_MES_1035).c_str(), TEXT("Information"), MB_OK | MB_ICONINFORMATION);
		}

		NotSaveIni = true;
		Close();
	}

	RestoreStayOnTop();
}


// メニュー Quit クリック
void CMainForm::MnQuit_Click(void)
{
	Close();
}

//-----------------------------------------------------------------------------------------------------


void CMainForm::DisplayBox_DoubleClick(void)
{
	if (Locked) return;
	ToggleShowList(EShowMode_PICTURE);
}

void CMainForm::MainForm_MouseDoubleClick(void)
{
	if (Locked) return;
	ToggleShowList(EShowMode_TOGGLE);
}


void CMainForm::TrayIcon_MouseClick(EEventButton button)
{
	switch (button)
	{
	case EEventButton_LEFT:
		acfc::SetAbsoluteForegroundWindow(hWindow);
		break;
	case EEventButton_RIGHT:
		{
			POINT p;
			GetCursorPos(&p);
			PopupMenuPopup(p);
		}
		break;
	case EEventButton_MIDDLE:
		MnQuit_Click();
		break;
	}
}


void CMainForm::CursorTimer_Tick(void)
{
	POINT ScCurPos;
	GetCursorPos(&ScCurPos);

	if (PreCurTimerPos.x > ScCurPos.x - 3 && PreCurTimerPos.x < ScCurPos.x + 3
		&& PreCurTimerPos.y > ScCurPos.y - 3 && PreCurTimerPos.y < ScCurPos.y + 3)
	{
		if (StopCount >= 0) StopCount++;
	}
	PreCurTimerPos = ScCurPos;

	if (StopCount > 2)
	{
		SetCursorMode(EMousePositionType_NONE);
		StopCount = -1;
	}

	if (SSIcon >= 0)
	{
		if (SSIcon < 2)
		{
			SSIcon++;
			FormRefresh();

		}
		else
		{
			SSIcon = -1;
			FormRefresh();
		}
	}
}

void CMainForm::SSTimer_Tick(void)
{
	SSChangeImage = true;
	ShowOffsetImage(1);
	SSChangeImage = false;
}


void CMainForm::File_DragDrop(HDROP hDrop)
{
	std::vector<std::wstring> DropFiles;
	acfc::GetDropFileName(DropFiles, hDrop);
	AddHistoryList(DropFiles);
	OpenFiles(DropFiles);
}

//-----------------------------------------------------------------------------------------------------
//
// ラジオメニュー変更
//
//-----------------------------------------------------------------------------------------------------

void CMainForm::CheckPositionModeMenuCheck(void)
{
	int i;
	for (i = 0; i < POSITIONMODENUM; i++)
	{
		if (PositionModes[i] == PositionMode)
			SetMenuCheckType(hPopupMenu, mnPositionModes[i], 2);
		else
			SetMenuCheckType(hPopupMenu, mnPositionModes[i], 0);
	}
}

void CMainForm::CheckRotateCheck(void)
{
	int i;
	for (i = 0; i < ABSOLUTEROTATENUM; i++)
	{
		if (AbsoluteRotates[i] == RotateValue)
			SetMenuCheckType(hPopupMenu, mnAbsoluteRotate[i], 2);
		else
			SetMenuCheckType(hPopupMenu, mnAbsoluteRotate[i], 0);
	}
}



void CMainForm::CheckIntervalMenuCheck(void)
{
	int i;
	for (i = 0; i < SLIDESHOWNUM; i++)
	{
		if ((SSInterval / 1000) == SlideShows[i])
			SetMenuCheckType(hPopupMenu, mnSlideShows[i], 2);
		else
			SetMenuCheckType(hPopupMenu, mnSlideShows[i], 0);
	}
}


void CMainForm::CheckFontSizeMenuCheck(void)
{
	int i;
	FontSizeIndex = -1;
	for (i = 0; i < FONTSIZENUM; i++)
	{
		if (ApplicationFontSize == FontSizes[i])
		{
			SetMenuCheckType(hPopupMenu, mnFontSizes[i], 2);
			FontSizeIndex = i;
		}
		else
			SetMenuCheckType(hPopupMenu, mnFontSizes[i], 0);

		// 実際にフォントを変更
		if (hFont != nullptr)DeleteObject(hFont);
		HDC hDC = GetDC(hWindow);
		LOGFONT lfont;
		hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT); // システムのフォントを取得
		GetObject(hFont, sizeof(LOGFONT), &lfont); 
		lfont.lfHeight = MulDiv(ApplicationFontSize, GetDeviceCaps(hDC, LOGPIXELSY), 72); // フォントのサイズを変更
		lfont.lfWidth = 0; // フォントのサイズを変更
		hFont = CreateFontIndirect(&lfont); // フォントの作成
		SendMessage(DisplayBox.handle, WM_SETFONT, (WPARAM)hFont, ShowingList);
		ReleaseDC(hWindow, hDC);
	}
}


void CMainForm::CheckFrameWidthMenuCheck(void)
{
	int i;
	for (i = 0; i < FRAMEWIDTHNUM; i++)
	{
		if (FrameWidth == FrameWidths[i])
			SetMenuCheckType(hPopupMenu, mnFrameWidths[i], 2);
		else
			SetMenuCheckType(hPopupMenu, mnFrameWidths[i], 0);
	}
}

void CMainForm::CheckWheelSenseCheck(void)
{
	int i;
	for (i = 0; i < WHEELSENSENUM; i++)
	{
		if (WheelSensitivity == WheelSensitivities[i])
			SetMenuCheckType(hPopupMenu, mnWheelSense[i], 2);
		else
			SetMenuCheckType(hPopupMenu, mnWheelSense[i], 0);
	}
}

void CMainForm::CheckUseCursorSizeCheck(void)
{
	if (UseCursorSize < 0) UseCursorSize = 0;
	if (UseCursorSize > 2) UseCursorSize = 2;

	std::wstring H = TEXT("IDC_HAND_");
	std::wstring G = TEXT("IDC_GRAB_");
	TCHAR tmp = (TCHAR)(TEXT('0') + UseCursorSize);

	H = H + tmp;
	G = G + tmp;

	hCursor[1] = LoadCursor(appInstance, H.c_str());
	hCursor[2] = LoadCursor(appInstance, G.c_str());

	int i;
	for (i = 0; i < 3; i++)
	{
		if (UseCursorSize == i)
			SetMenuCheckType(hPopupMenu, mnCursorSizes[i], 2);
		else
			SetMenuCheckType(hPopupMenu, mnCursorSizes[i], 0);
	}
}

void CMainForm::CheckAlphaValueMenuCheck(void)
{
	int i;
	for (i = 0; i < ALPHABLENDNUM; i++)
	{
		if (FormAlphaBlendValue == AlphaBlends[i])
			SetMenuCheckType(hPopupMenu, mnAlphaBlends[i], 2);
		else
			SetMenuCheckType(hPopupMenu, mnAlphaBlends[i], 0);
	}
	BYTE Alpha = (BYTE)FormAlphaBlendValue;
	SetLayeredWindowAttributes(hWindow, 0, Alpha, LWA_ALPHA);
}



//----------------------------------------------------------------------------------------------------------
