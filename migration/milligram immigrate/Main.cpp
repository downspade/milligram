//---------------------------------------------------------------------------


#include <Windows.h>
#include <vcl.h>

#include <math.h>
#include "SpiPlugin.h"
#pragma hdrstop
#include "Main.h"
#include <Registry.hpp>
#include <Clipbrd.hpp>
#include "Progress.h"
#include "acfc_SubFunction.h"
#include "JpegCmpRat.h"
#include "FormSize.h"
#include "Exif.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "IdBaseComponent"
#pragma link "IdComponent"
#pragma link "IdHTTP"
#pragma link "IdTCPClient"
#pragma link "IdTCPConnection"
#pragma resource "*.dfm"
#pragma resource "*.res"

TMainForm *MainForm;
TSpiLoader*	Susie;
TStringList *SpiPathes;

//---------------------------------------------------------------------------
int PASCAL ProgressCallback(int nNum,int nDenom,long lData);

bool __fastcall GetButtonSwap(void)
{
	TRegistry *Reg = new TRegistry();
	UnicodeString ASKey;
	bool Result = true;

	if(Reg->OpenKey(L"Control Panel\\Mouse", false) == false)
	{
		Result = false;
	}
	else
	{
		try
		{
			ASKey = Reg->ReadString(L"SwapMouseButtons");
			Reg->CloseKey();
			if(ASKey != L"1")Result = false;
		}
		catch(...)
		{
			Result = false;
		}
	}
	delete Reg;
	return(Result);
}


//---------------------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* Owner)
	: TForm(Owner)
{
	Screen->Cursors[1] = LoadCursor(HInstance, L"Hand");
	Screen->Cursors[2] = LoadCursor(HInstance, L"Grab");
	Screen->Cursors[3] = LoadCursor(HInstance, L"None");

	// 初期パラメータの設定
	ShowIndex = -1;
	ShowIndexBack = -1;
	AlwaysTop = false;
	SlideShow = false;
	ShowingList = true;
	SSInterval = 5000;
	FixPercentage = false;
	FitToScreen = false;
	UseWholeScreen = false;
	FullScreen = false;
	Locked = false;
	SearchSubFolder = false;
	LoadLastFile = false;
	NoStartAtCursor = false;
	CenterX = 100;
	CenterY = 100;
	TrayIconColor = 0;
	PositionMode = 0;
	FixRotate = false;
	RotateValue = 0;
	WheelSensitivity = 120;
	FullFillColor = clBlack;
	FrameColor = GetFrameColor();
	ShortCutFileName = L"";
	CreateSendToLink = false;
	SSIcon = -1;
	FileMaskString = "*.*;";
	EnableFileMask = false;
	AutoLoadFileFolder = true;
	DoubleBufferOff = false;
	SSChangeImage = false;

	Holding = 0;
	WWidth = 200;
	WHeight = 200;
	WLeft = 0;
	WTop = 0;
	PositionType = 0;
	Ratio = 1;
	Percentage = 1;

	FrameWidth = 0;
	FrameFillColor = clWhite;

	FormAlphaBlendValue = 255;

	ApplicationFontSize = 7;

	WheelPos = WheelSensitivity / 2;
	SlideShow = false;
	NotSaveIni = false;
	LoadLastFile = false;
	HideTaskButton = true;
	InstanceMode = false;
	EnableDraw = true;
	StopCount = 0;
	RClkCancel = false;
	Monitor = NULL;

	GIFRefresh = false;

	HistoryList = new TStringList();
	MaxHistoryNum = 16;
	HistoryMenu = NULL;
	RefreshHistoryMode = 1;

	JCR[0] = 85;
	JCR[1] = 25;
	JCR[2] = 60;
	JCR[3] = 85;

	// マウスのボタンの読み込み
	if(GetButtonSwap() == false)
	{
		VkLButton = VK_LBUTTON;
		VkRButton = VK_RBUTTON;
	}
	else
	{
		VkLButton = VK_RBUTTON;
		VkRButton = VK_LBUTTON;
	}

	// Ini ファイルを読み込む＆プラグイン読み込みパスも読み込む
	Susie = new TSpiLoader(ProgressCallback);
	Susie->Width = 200;
	Susie->Height = 200;

	FileLists = new TStringList();
	ArchiveFileLists = new TStringList();
	SpiPathes = new TStringList();

	DisplayLists = FileLists;

	IniParamName = L"milligram";

	Dropper = NULL;
}

//---------------------------------------------------------------------------
void __fastcall TMainForm::FormShow(TObject *Sender)
{
	// 1回だけ呼ばれる
	static bool FirstShow = true;
	if(FirstShow == false)return;
	FirstShow = false;

	TStringList *DropLists = new TStringList();

	int i;
	bool CmdLine = false;
	UnicodeString Temp;

	// 引数のチェック
	if(ParamCount() > 0)
	{
		int Len;
		for(i = 1;i <= ParamCount();i++)
		{
			if(ParamStr(i)[1] == L'-')
			{
				SetInstanceMode(ParamStr(i).SubString(2, ParamStr(1).Length() - 1));
			}
			else
			{
				Temp = ParamStr(i);
				Len = Temp.Length();
				if(Len > 5)
					if(Temp.SubString(Len - 3, 4) == L".lnk")
						Temp = GetFileFromLink(Temp);

				DropLists->Add(Temp);
				CmdLine = true;
			}
		}
	}

	// ------------- ini ファイルの読み込み --------------
	LoadIni(IniParamName, CmdLine);

	// ファイルヒストリの処理
	if(HistoryMenu == NULL)HistoryMenu = new TMenuItem*[MaxHistoryNum];
	CreateHistoryMenu();
	ConvertHistoryMenu();

	int OLeft = WLeft, OTop = WTop;

	Susie->SetDoubleBufferMode(!DoubleBufferOff && !Locked);

	SetComponentPointer();

	TrayIcon->Visible = true;
	TrayIcon->IconIndex = TrayIconColor;

	if(InstanceMode == true)
		MnDeleteInstance->Enabled = true;

	if(SpiPathes->Count == 0)
		SpiPathes->Add(ExtractFileDir(Application->ExeName));

	Susie->SetSpiPathes(SpiPathes);		// SPI path set

	if(CmdLine)
	{
    AddHistoryList(DropLists);

		TStringList *GetLists = new TStringList();
		TStringList *ArcLists = new TStringList();

		CheckGetLists(GetLists, ArcLists, DropLists);

		AddFileLists(GetLists, ArcLists, 0);
		delete GetLists;
		delete ArcLists;

		ShowIndexBack = -1;
	}

	if(FileLists->Count > 0)
	{
		if(ShowIndex >= 0)
		{
			if(PositionMode == 0 && NoStartAtCursor == false && CmdLine)
			{
				POINT ScCurPos;
				GetCursorPos(&ScCurPos);
				CenterX = ScCurPos.x;
				CenterY = ScCurPos.y;
			}
			CheckMonitorIni();

			ReloadMode = true & !CmdLine;

/*
			if(ArchiveFileLists->IndexOf(ShowFileName) < 0)
			{
				if(FileLists->Count < ShowIndex || FileLists->Strings[ShowIndex] != ShowFileName)
					ShowIndex = FileLists->IndexOf(ShowFileName);
				ShowIndexBack = -1;
			}
			else
			{
				if(FileLists->Count < ShowIndexBack || FileLists->Strings[ShowIndexBack] != ShowFileBack)
					ShowIndexBack = FileLists->IndexOf(ShowFileBack);

				if(ShowIndexBack < 0)
				{
					ShowIndex = 0;
				}
			}
*/
			if(ShowIndexBack >= 0)
			{
				if(ShowIndexBack >= FileLists->Count || FileExists(FileLists->Strings[ShowIndexBack]) == false)
				{
					ShowIndexBack = -1;
				}
			}

//--------------------------------------------------
//			if(ShowIndexBack < 0 || CmdLine ||
//			(ShowIndexBack < FileLists->Count && ArchiveFileLists->IndexOf(FileLists->Strings[ShowIndexBack]) < 0))
//--------------------------------------------------

			if(ShowIndexBack < 0 || CmdLine)
			{
				ShowAbsoluteImage(ShowIndex, 1);
			}
			else
			{
				ShowAbsoluteImage(ShowIndexBack, ShowIndex, 1, true);
			}

			if(ReloadMode)
			{
				WLeft = OLeft;
				WTop = OTop;
				SyncWindow(false);
			}
			ReloadMode = false;
		}
		if(ShowingList)SetFileList();
	}
	else
	{
		ShowIndex = -1, ShowIndexBack = -1;
	}

	if(ShowIndex < 0)
	{
		CheckMonitorIni();

		if(FixPercentage == true)
		{
			WWidth *= Percentage;
			WHeight *= Percentage;
		}
		else
			Percentage = 1;

		if(CenterX - WWidth / 2 < MLeft)
			WLeft = MLeft;
		else if(CenterX + WWidth / 2 > MLeft + MWidth)
			WLeft = MLeft + MWidth - WWidth;
		else
			WLeft = CenterX - WWidth / 2;

		if(CenterY - WHeight / 2 < MTop)
			WTop = MTop;
		else if(CenterY + WHeight / 2 > MTop + MHeight)
			WTop = MTop + MHeight - WHeight;
		else
			WTop = CenterY - WHeight / 2;

		SetCenter(WLeft + WWidth / 2, WTop + WHeight / 2);
		SyncWindow(false);
	}

	if(HideTaskButton == true)
		SetWindowLong(Handle, GWL_EXSTYLE, WS_EX_TOOLWINDOW);

//	Show();

	// ドロップイベントの設定
	Dropper = new CDragDrop;
	Dropper->Init(this);
	WindowProc = LoopProc; // フォームのほうに記入
	Application->OnIdle = MyIdleHandler;

	FolderLists = new TStringList();

	delete DropLists;
	SetAbsoluteForegroundWindow(Handle);

	// マルチメディアタイマーの設定
	{
		TIMECAPS TimeCaps;
		ZeroMemory(&TimeCaps, sizeof(TIMECAPS));
		timeGetDevCaps(&TimeCaps, sizeof(TIMECAPS));
		ShortestPeriod = TimeCaps.wPeriodMin;
	}

	timeBeginPeriod(ShortestPeriod);

	ClockTGT = PreTGT = timeGetTime();

	CheckAlphaValueMenuCheck();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void __fastcall TMainForm::FormDestroy(TObject *Sender)
{
	int i;
	if(NotSaveIni == false)SaveIni(IniParamName);
	Dropper->Release(MainForm);
	ClearArchiveFileLists(ArchiveFileLists);

	delete SpiPathes;
	delete ArchiveFileLists;

	for(i = 0;i < FileLists->Count;i++)
		delete (SImageInfo *)FileLists->Objects[i];

	delete FolderLists;
	delete FileLists;

	delete Dropper;
	delete Susie;

	delete HistoryList;
	for(i = 0;i < MaxHistoryNum;i++)
	{
		delete HistoryMenu[i];
	}
	delete[] HistoryMenu;

	timeEndPeriod(ShortestPeriod);
}
//---------------------------------------------------------------------------

bool __fastcall TMainForm::GetImageLists(UnicodeString Src, TStringList *Dest, bool SubFolder, bool EnableFileMask, UnicodeString FileMaskString) // c:\windows\dest\*.dll のような形式で Src は指定する
{
	if(EnableFileMask == false)
	{
		FileMaskString = "*.*";
	}
	int Depth = 0;
	if(SubFolder == true)Depth = -1;

	Dest->Clear();

	ProgressForm->Reset();

	ProgressForm->Src = Src;
	ProgressForm->Dest = Dest;
	ProgressForm->Depth = Depth;
	ProgressForm->Mask = FileMaskString;

	FormStyle = fsNormal;

	ProgressForm->ShowModal();

	RestoreStayOnTop();

	return(true);
}

//---------------------------------------------------------------------------

bool __fastcall TMainForm::ClearArchiveFileLists(TStringList *ArcLists)
{
	int i, j;
	TStringList *TempSL;
	for(i = 0;i < ArcLists->Count;i++)
	{
		TempSL = dynamic_cast<TStringList *>(ArcLists->Objects[i]);
		for(j = 0;j < TempSL->Count;j++)
    	delete (SImageInfo *)TempSL->Objects[j];
		delete TempSL;
	}
	ArcLists->Clear();
	return(true);
}

bool __fastcall TMainForm::SaveIni(UnicodeString IniName)
{
	UnicodeString Temp, Path;
	int i;

	Path = GetIniFileName(L"\\milligram\\" + IniName + ".ini");

	TStringList* IniFile = new TStringList;

	IniFile->Add(L"CenterX=" + IntToStr(CenterX));
	IniFile->Add(L"CenterY=" + IntToStr(CenterY));
	IniFile->Add(L"FormLeft=" + IntToStr((int)WLeft));
	IniFile->Add(L"FormTop=" + IntToStr((int)WTop));

	IniFile->Add(L"ShowIndex=" + IntToStr(ShowIndex));
	IniFile->Add(L"ShowIndexBack=" + IntToStr(ShowIndexBack));

	IniFile->Add(L"AlwaysTop=" + BoolToStr(AlwaysTop, true));
	IniFile->Add(L"HideTaskButton=" + BoolToStr(HideTaskButton, true));
	IniFile->Add(L"ShowingList=" + BoolToStr(ShowingList, true));
	IniFile->Add(L"SlideShow=" + BoolToStr(SlideShow, true));
	IniFile->Add(L"SSInterval=" + IntToStr(SSInterval));

	IniFile->Add(L"FrameWidth=" + IntToStr(FrameWidth));
	IniFile->Add(L"FrameColor=" + IntToStr((int)FrameFillColor));

	IniFile->Add(L"AlphaBlendValue=" + IntToStr((int)FormAlphaBlendValue));

	IniFile->Add(L"FontSize=" + IntToStr(ApplicationFontSize));

	IniFile->Add(L"FixPercentage=" + BoolToStr(FixPercentage, true));
	IniFile->Add(L"FitToScreen=" + BoolToStr(FitToScreen, true));
	IniFile->Add(L"PositionMode=" + IntToStr(PositionMode));
	IniFile->Add(L"FixRotate=" + BoolToStr(FixRotate, true));
	IniFile->Add(L"RotateValue=" + IntToStr(RotateValue));
	IniFile->Add(L"WheelSensitivity=" + IntToStr(WheelSensitivity));
	IniFile->Add(L"ShowConstant=" + BoolToStr(ShowConstant, true));
	IniFile->Add(L"ConstantSize=" + IntToStr(ConstantSize));
	IniFile->Add(L"UseWholeScreen=" + BoolToStr(UseWholeScreen, true));

	IniFile->Add(L"FullScreen=" + BoolToStr(FullScreen, true));
	IniFile->Add(L"ViewPercentage=" + FloatToStr(Percentage));
	IniFile->Add(L"Locked=" + BoolToStr(Locked, true));

	IniFile->Add(L"SearchSubFolder=" + BoolToStr(SearchSubFolder, true));
	IniFile->Add(L"LoadLastFile=" + BoolToStr(LoadLastFile, true));
	IniFile->Add(L"ScreenSnap=" + BoolToStr(ScreenSnap, true));
	IniFile->Add(L"NoStartAtCursor=" + BoolToStr(NoStartAtCursor, true));
	IniFile->Add(L"DoubleBufferOff=" + BoolToStr(DoubleBufferOff, true));
	IniFile->Add(L"TrayIconColor=" + IntToStr(TrayIconColor));
	IniFile->Add(L"FullFillColor=" + IntToStr((int)FullFillColor));
	IniFile->Add(L"ShortCutFileName=" + ShortCutFileName);
	IniFile->Add(L"CreateSendToLink=" + BoolToStr(CreateSendToLink, true));

	IniFile->Add(L"JpegCompressRatio0=" + IntToStr(JCR[0]));
	IniFile->Add(L"JpegCompressRatio1=" + IntToStr(JCR[1]));
	IniFile->Add(L"JpegCompressRatio2=" + IntToStr(JCR[2]));
	IniFile->Add(L"JpegCompressRatio3=" + IntToStr(JCR[3]));

	IniFile->Add(L"FileMaskString=" + FileMaskString);
	IniFile->Add(L"EnableFileMask=" + BoolToStr(EnableFileMask, true));
	IniFile->Add(L"AutoLoadFileFolder=" + BoolToStr(AutoLoadFileFolder, true));

	for(i = 0;i < SpiPathes->Count;i++)
		IniFile->Add(L"SpiPath" + IntToStr(i) + L"=" + SpiPathes->Strings[i]);

	IniFile->Add(L"SpiPrecede=" + BoolToStr(Susie->SpiPrecede, true));

	for(i = 0;i < HistoryList->Count;i++)
		IniFile->Add(L"History" + IntToStr(i) + L"=" + HistoryList->Strings[i]);

	if(LoadLastFile == true)
		CreateFileLists(IniFile);

	if(DirectoryExists(ExtractFileDir(Path)) == false)
		ForceDirectories(ExtractFileDir(Path));

	SaveStringList(IniFile, Path);
	GetFileTimeStamp(Path, &IniFileTime);

	delete IniFile;
	return(true);
}



bool __fastcall TMainForm::SaveFileLists(UnicodeString FileName)
{
	TStringList *TempSL = new TStringList();
	CreateFileLists(TempSL);

	SaveStringList(TempSL, FileName);

	MessageBeep(MB_ICONASTERISK);
	ShowMessage(LoadStr(1001));

	delete TempSL;
	return(true);
}

bool __fastcall TMainForm::CreateFileLists(TStringList *TempSL)
{
	int i, j;
	TStringList *AcSL;
	SImageInfo *Temp;

	for(i = 0;i < FileLists->Count;i++)
	{
		Temp = (SImageInfo *)FileLists->Objects[i];
		TempSL->Add(L"FileList" + IntToStr(i) + L"=" + "\tName=" + FileLists->Strings[i] + "\tTime=" + IntToStr((int)Temp->Timestamp) + "\tSize=" + IntToStr((int)Temp->FileSize) + "\tRotate=" + IntToStr((int)Temp->Rotate));
	}

	for(j = 0;j < ArchiveFileLists->Count;j++)
	{
		AcSL = (TStringList *)ArchiveFileLists->Objects[j];
		TempSL->Add(L"Archive" + IntToStr(j) + L"=" + ArchiveFileLists->Strings[j]);
		for(i = 0;i < AcSL->Count;i++)
		{
			Temp = (SImageInfo *)AcSL->Objects[i];
			TempSL->Add(L"ArchiveList" + IntToStr(j) + "-" + IntToStr(i) + L"=" + "\tName=" + AcSL->Strings[i] + "\tTime=" + IntToStr((int)Temp->Timestamp) + "\tSize=" + IntToStr((int)Temp->FileSize) + "\tRotate=" + IntToStr((int)Temp->Rotate));
		}
	}
	return(true);
}

bool __fastcall TMainForm::LoadIni(UnicodeString IniName, bool CmdLine)
{
	UnicodeString Path, Temp;

	int i;

	if(
		 ((GetAsyncKeyState(VK_LCONTROL) & 0x8000) && (GetAsyncKeyState(VK_LSHIFT) & 0x8000))
	|| ((GetAsyncKeyState(VK_RCONTROL) & 0x8000) && (GetAsyncKeyState(VK_RSHIFT) & 0x8000))
	)
	{
		MessageBeep(MB_ICONEXCLAMATION);
		ShowMessage(LoadStr(1002));
		return(true);
	}

	Path = GetIniFileName(L"\\milligram\\" + IniName + ".ini");

	if(!FileExists(Path))
	{
		if(InstanceMode == false)DoInstall();
		SaveIni(IniName);
		return(true);
	}
	else
	{
		GetFileTimeStamp(Path, &IniFileTime);
	}

	TStringList *IniFile = new TStringList;
	LoadStringList(IniFile, Path);

	ShowIndex = GetIntegerValue(IniFile, L"ShowIndex", 0, 0, 0);
	ShowIndexBack = GetIntegerValue(IniFile, L"ShowIndexBack", 0, 0, 0);

	if(GetBoolValue(IniFile, L"AlwaysTop", false) == true)MnAlwaysTopClick(NULL);

	HideTaskButton = GetBoolValue(IniFile, L"HideTaskButton", true);
	MnHideTaskButton->Checked = HideTaskButton;

	if(CmdLine || GetBoolValue(IniFile, L"ShowingList", true) == false)ToggleShowList(2);

	if(GetBoolValue(IniFile, L"SlideShow", false) == true)MnSlideShowClick(NULL);
	SSInterval = GetIntegerValue(IniFile, L"SSInterval", SSInterval, 0, 0);

	FrameWidth = GetIntegerValue(IniFile, L"FrameWidth", FrameWidth, 0, 0);
	FrameFillColor = GetIntegerValue(IniFile, L"FrameColor", FrameFillColor, 0, 0);

	FormAlphaBlendValue = GetIntegerValue(IniFile, L"AlphaBlendValue", ApplicationFontSize, 1, 255);

	ApplicationFontSize = GetIntegerValue(IniFile, L"FontSize", ApplicationFontSize, 5, 500);

	if(GetBoolValue(IniFile, L"FixPercentage", false) == true)MnFixPercentageClick(NULL);
	PositionMode = GetIntegerValue(IniFile, L"PositionMode", PositionMode, 0, 0);

	RotateValue = GetIntegerValue(IniFile, L"RotateValue", RotateValue, 0, 0);
	if(GetBoolValue(IniFile, L"FixRotate", false) == true)MnFixRotateClick(NULL);
	Susie->FixRotate = FixRotate;

	ConstantSize = GetIntegerValue(IniFile, L"ConstantSize", ConstantSize, 0, 0);
	if(GetBoolValue(IniFile, L"ShowConstant", false) == true)MnConstantSizeClick(NULL);

	FitToScreen = GetBoolValue(IniFile, L"FitToScreen", FitToScreen);
	UseWholeScreen = GetBoolValue(IniFile, L"UseWholeScreen", UseWholeScreen);
	FullScreen = GetBoolValue(IniFile, L"FullScreen", FullScreen);

	SearchSubFolder = GetBoolValue(IniFile, L"SearchSubFolder", SearchSubFolder);
	MnSearchSubFolder->Checked = SearchSubFolder;

	LoadLastFile = GetBoolValue(IniFile, L"LoadLastFile", LoadLastFile);
	MnLoadLastFile->Checked = LoadLastFile;

	ScreenSnap = GetBoolValue(IniFile, L"ScreenSnap", ScreenSnap);
	MnSnapWindow->Checked = ScreenSnap;

	NoStartAtCursor = GetBoolValue(IniFile, L"NoStartAtCursor", NoStartAtCursor);
	MnNoStartAtCursor->Checked = NoStartAtCursor;

	WheelSensitivity = GetIntegerValue(IniFile, L"WheelSensitivity", WheelSensitivity, 1, 60000000);

	DoubleBufferOff = GetBoolValue(IniFile, L"DoubleBufferOff", DoubleBufferOff);
	MnDoubleBufferOff->Checked = DoubleBufferOff;

	Locked = GetBoolValue(IniFile, L"Locked", Locked);
	MnLock->Checked = Locked;

	TrayIconColor = GetIntegerValue(IniFile, L"TrayIconColor", TrayIconColor, 0, 0);

	FullFillColor = GetIntegerValue(IniFile, L"FullFillColor", (int)FullFillColor, 0, 0);
	FrameColor = GetFrameColor();

	ShortCutFileName = GetStringValue(IniFile, L"ShortCutFileName", ShortCutFileName);

	CreateSendToLink = GetBoolValue(IniFile, L"CreateSendToLink", CreateSendToLink);

	JCR[0] = GetIntegerValue(IniFile, L"JpegCompressRatio0", 85, 1, 100);
	JCR[1] = GetIntegerValue(IniFile, L"JpegCompressRatio1", 25, 1, 100);
	JCR[2] = GetIntegerValue(IniFile, L"JpegCompressRatio2", 60, 1, 100);
	JCR[3] = GetIntegerValue(IniFile, L"JpegCompressRatio3", 85, 1, 100);

	FileMaskString = GetStringValue(IniFile, L"FileMaskString", FileMaskString);
	EnableFileMask = GetBoolValue(IniFile, L"EnableFileMask", EnableFileMask);
	MnEnableFileMask->Checked = EnableFileMask;

	AutoLoadFileFolder = GetBoolValue(IniFile, L"AutoLoadFileFolder", AutoLoadFileFolder);
	MnAutoLoadFileFolder->Checked = AutoLoadFileFolder;

	MaxHistoryNum = GetIntegerValue(IniFile, L"MaxHistoryNum", (int)MaxHistoryNum, 0, 50);

	HistoryMenu = new TMenuItem*[MaxHistoryNum];

	//------- 表示位置、サイズに関するデータはなるべく最後の方で読み込む

	CenterX = GetIntegerValue(IniFile, L"CenterX", CenterX, 0, 0);
	CenterY = GetIntegerValue(IniFile, L"CenterY", CenterY, 0, 0);
	WLeft = GetIntegerValue(IniFile, L"FormLeft", WLeft, 0, 0);
	WTop = GetIntegerValue(IniFile, L"FormTop", WTop, 0, 0);
	Percentage = GetDoubleValue(IniFile, L"ViewPercentage", Percentage, 0, 0);

	//---------------------------------------------------------------------

	i = 0;
	while(CheckStrings(IniFile, L"SpiPath" + IntToStr(i)) == true)
	{
		Temp = IniFile->Values[L"SpiPath" + IntToStr(i)];
		SpiPathes->Add(Temp);
		i++;
	}

	Susie->SpiPrecede = GetBoolValue(IniFile, L"SpiPrecede", Susie->SpiPrecede);
	MnSpiPrecede->Checked = Susie->SpiPrecede;

	i = 0;
	while(i < MaxHistoryNum && CheckStrings(IniFile, L"History" + IntToStr(i)) == true)
	{
		Temp = IniFile->Values[L"History" + IntToStr(i)];
		HistoryList->Add(Temp);
		i++;
	}

	if(((GetAsyncKeyState(VK_LCONTROL) & 0x8000) == 0 && (GetAsyncKeyState(VK_RCONTROL) & 0x8000) == 0)
	&& CmdLine == false && LoadLastFile == true)
	{
		LoadFileLists(FileLists, ArchiveFileLists, IniFile);
	}
	else
	{
		ShowIndex = -1;
	}

	while(GetAsyncKeyState(VK_ESCAPE) & 0x8000);

	delete IniFile;
	return(true);
}

UnicodeString __fastcall TMainForm::GetTabString(UnicodeString Src, UnicodeString Name)
{
	int i, L, l, P = Src.Pos(Name + L"=");
	if(P == 0)return(L"");
	UnicodeString Result = L"";
	L = Src.Length();
	l = Name.Length() + 1;
	for(i = P + l;i <= L;i++)
	{
		if(Src[i] == L'\t')break;
	}
	Result = Src.SubString(P + l, i - P - l);
	return(Result);
}

bool __fastcall TMainForm::LoadFileLists(TStringList *DestSL, TStringList *ArcLists, TStringList *TempSL)
{
	int i, j, n, p, q, Pos = 0;
	UnicodeString Temp, FName, FTime, FSize, FRotate;
	TStringList *NewSL;

	i = 0;
	while(CheckStrings(TempSL, L"FileList" + IntToStr(i), Pos) == true)
	{
		if(GetAsyncKeyState(VK_ESCAPE) & 0x8000)break;

		FTime = "0";
		FSize = "0";
		FRotate = "-1";

		while(1)
		{
			Temp = TempSL->Values[L"FileList" + IntToStr(i)];
			FName = GetTabString(Temp, L"Name");
			FTime = GetTabString(Temp, L"Time");
			FSize = GetTabString(Temp, L"Size");
			FRotate = GetTabString(Temp, L"Rotate");
			break;
		}

		if(EnableFileMask && CheckMasks(FName, FileMaskString) == false)
		{
			i++;
			continue;
		}

		SImageInfo *NewII = new SImageInfo;
		NewII->Timestamp = (time_t)StrToInt(FTime);
		NewII->FileSize = (size_t)StrToInt(FSize);
		NewII->Rotate = StrToInt(FRotate);
		DestSL->AddObject(FName, (TObject *)NewII);
		i++;
	}

	n = 0;
	while(CheckStrings(TempSL, L"Archive" + IntToStr(n)) == true)
	{
		Temp = TempSL->Values[L"Archive" + IntToStr(n)];

		if(EnableFileMask && CheckMasks(Temp, FileMaskString) == false)
		{
			i++;
			continue;
		}

		if(DestSL->IndexOf(Temp) >= 0)
		{
			NewSL = new TStringList();
			ArcLists->AddObject(Temp, NewSL);
		}
		n++;
	}

	for(j = 0;j < ArcLists->Count;j++)
	{
		NewSL = (TStringList *)ArcLists->Objects[j];

		i = 0;
		while(CheckStrings(TempSL, L"ArchiveList" + IntToStr(j) + "-" + IntToStr(i), Pos))
		{
			if(GetAsyncKeyState(VK_ESCAPE) & 0x8000)break;

			FTime = "0";
			FSize = "0";
			FRotate = "-1";

			while(1)
			{
				Temp = TempSL->Values[L"ArchiveList" + IntToStr(j) + "-" + IntToStr(i)];
				FName = GetTabString(Temp, L"Name");
				FTime = GetTabString(Temp, L"Time");
				FSize = GetTabString(Temp, L"Size");
				FRotate = GetTabString(Temp, L"Rotate");
				break;
			}

			if(EnableFileMask && CheckMasks(FName, FileMaskString) == false)
			{
				i++;
				continue;
			}

			SImageInfo *NewII = new SImageInfo;
			NewII->Timestamp = (time_t)StrToInt(FTime);
			NewII->FileSize = (size_t)StrToInt(FSize);
			NewII->Rotate = StrToInt(FRotate);
			NewSL->AddObject(FName, (TObject *)NewII);
			i++;
		}
	}
	return(true);
}

bool __fastcall TMainForm::CheckMonitorIni(void)
{
	if(Monitor != NULL)return(false);

	Monitor = Screen->MonitorFromPoint(Point(CenterX, CenterY), mdNearest);
	GetMonitorParameter();

	if(FullScreen)
	{
  	FullScreen = !FullScreen;
		MnFullScreenClick(NULL);
	}

	if(UseWholeScreen)
	{
    UseWholeScreen = !UseWholeScreen;
		MnUseWholeScreenClick(NULL);
	}
	else if(FitToScreen)
	{
  	FitToScreen = !FitToScreen;
		MnFitToScreenClick(NULL);
	}

	return(true);
}


void __fastcall TMainForm::SetComponentPointer(void)
{
	TrayIcons[0] = MnTC0;
	TrayIcons[1] = MnTC1;
	TrayIcons[2] = MnTC2;
	TrayIcons[3] = MnTC3;
	TrayIcons[4] = MnTC4;
	TrayIcons[5] = MnTC5;

	TrayIcons[TrayIconColor]->Checked = true;

	SlideShows[0] = MnSS0;
	SlideShows[1] = MnSS1;
	SlideShows[2] = MnSS2;
	SlideShows[3] = MnSS3;
	SlideShows[4] = MnSS4;
	SlideShows[5] = MnSS5;
	SlideShows[6] = MnSS6;
	SlideShows[7] = MnSS7;
	SlideShows[8] = MnSS8;
	SlideShows[9] = MnSS9;
	SlideShows[10] = MnSSA;
	SlideShows[11] = MnSSB;
	SlideShows[12] = MnSSC;
	SlideShows[13] = MnSSD;
	SlideShows[14] = MnSSE;

	FrameWidths[0] = MnFrameWidth0;
	FrameWidths[1] = MnFrameWidth1;
	FrameWidths[2] = MnFrameWidth2;
	FrameWidths[3] = MnFrameWidth3;
	FrameWidths[4] = MnFrameWidth4;
	FrameWidths[5] = MnFrameWidth5;

	PositionModes[0] = MnShowFree;
	PositionModes[1] = MnShowCenter;
	PositionModes[2] = MnShowLT;
	PositionModes[3] = MnShowRT;
	PositionModes[4] = MnShowLB;
	PositionModes[5] = MnShowRB;

	PositionModes[PositionMode]->Checked = true;

	RotateState[0] = MnRot0;
	RotateState[1] = MnRot1;
	RotateState[2] = MnRot2;
	RotateState[3] = MnRot3;

	WheelSense[0] = MnWS0;
	WheelSense[1] = MnWS1;
	WheelSense[2] = MnWS2;

	FontSizes[0] = MnLFS0;
	FontSizes[1] = MnLFS1;
	FontSizes[2] = MnLFS2;
	FontSizes[3] = MnLFS3;
	FontSizes[4] = MnLFS4;
	FontSizes[5] = MnLFS5;
	FontSizes[6] = MnLFS6;
	FontSizes[7] = MnLFS7;
	FontSizes[8] = MnLFS8;
	FontSizes[9] = MnLFS9;

	AlphaBlends[0] = MnABV0;
	AlphaBlends[1] = MnABV1;
	AlphaBlends[2] = MnABV2;
	AlphaBlends[3] = MnABV3;

	CheckIntervalMenuCheck();
	SSTimer->Interval = SSInterval;
	CheckFrameWidthMenuCheck();
	CheckFontSizeMenuCheck();
	CheckWheelSenseCheck();
}

void __fastcall TMainForm::CheckIntervalMenuCheck(void)
{
	int i;
	for(i = 0;i < SLIDESHOWNUM;i++)
	{
		if((SSInterval / 1000) == SlideShows[i]->Tag)
			SlideShows[i]->Checked = true;
		else
			SlideShows[i]->Checked = false;
	}
}


void __fastcall TMainForm::CheckFontSizeMenuCheck(void)
{
	int i;
	FontSizeIndex = -1;
	for(i = 0;i < FONTSIZENUM;i++)
	{
		if(ApplicationFontSize == FontSizes[i]->Tag)
			FontSizes[i]->Checked = true;
		else
			FontSizes[i]->Checked = false;

		DisplayBox->Font->Size = ApplicationFontSize;
	}
}

void __fastcall TMainForm::CheckAlphaValueMenuCheck(void)
{
	int i;
	FontSizeIndex = -1;
	for(i = 0;i < ALPHABLENDNUM;i++)
	{
		if(FormAlphaBlendValue == AlphaBlends[i]->Tag)
			AlphaBlends[i]->Checked = true;
		else
			AlphaBlends[i]->Checked = false;

		DisplayBox->Font->Size = ApplicationFontSize;
	}

	if(FormAlphaBlendValue == 255)
	{
//		AlphaBlend = false;
		AlphaBlendValue = FormAlphaBlendValue;
	}
	else
	{
//		AlphaBlend = true;
		AlphaBlendValue = FormAlphaBlendValue;
	}
}

void __fastcall TMainForm::AlphaValueOffset(int i)
{
	int Index = -1;
	int j;
	for(j = 0;j < ALPHABLENDNUM;j++)
	{
		if(FormAlphaBlendValue >= AlphaBlends[j]->Tag)
		{
			Index = j;
			break;
		}
	}

	Index += i;
	if(Index < 0)Index = 0;
	if(Index >= ALPHABLENDNUM)Index = ALPHABLENDNUM - 1;
	FormAlphaBlendValue = AlphaBlends[Index]->Tag;
	CheckAlphaValueMenuCheck();
}

void __fastcall TMainForm::IntervalOffset(int i)
{
	int Index = -1;
	int j;
	for(j = 0;j < SLIDESHOWNUM;j++)
	{
		if(SSInterval <= SlideShows[j]->Tag)
		{
			Index = j;
			break;
		}
	}

	Index += i;
	if(Index < 0)Index = 0;
	if(Index >= SLIDESHOWNUM)Index = SLIDESHOWNUM - 1;
	SSInterval = SlideShows[Index]->Tag;
	SSTimer->Interval = SSInterval;
}

void __fastcall TMainForm::CheckFrameWidthMenuCheck(void)
{
	int i;
	for(i = 0;i < 6;i++)
	{
		if(FrameWidth == FrameWidths[i]->Tag)
			FrameWidths[i]->Checked = true;
		else
			FrameWidths[i]->Checked = false;
	}
}

void __fastcall TMainForm::CheckWheelSenseCheck(void)
{
	int i;
	for(i = 0;i < 3;i++)
	{
		if(WheelSensitivity == WheelSense[i]->Tag)
			WheelSense[i]->Checked = true;
	}
}


bool __fastcall TMainForm::DoInstall(void)
{
	wchar_t TargetFolder[MAX_PATH];
	UnicodeString Temp;
	int Result;

	SHGetSpecialFolderPath(NULL, TargetFolder, CSIDL_DESKTOPDIRECTORY, FALSE);
	SaveLnkDialog->InitialDir = (UnicodeString)TargetFolder;

	MessageBeep(MB_ICONASTERISK);
	ShowMessage(LoadStr(1003));
	SaveLnkDialog->FileName = SaveLnkDialog->InitialDir + L"\\milligram image viewer.lnk";

	if(SaveLnkDialog->Execute() == true)
	{
		ShortCutFileName = SaveLnkDialog->FileName;
		CreateLink(Application->ExeName, SaveLnkDialog->FileName, L"");
	}

	DoCreateSendToLink(L"milligram");
	return(true);
}

bool __fastcall TMainForm::CreateShortCut(UnicodeString InstanceName)
{
	wchar_t DesktopFolder[MAX_PATH];
	SHGetSpecialFolderPath(NULL, DesktopFolder, CSIDL_DESKTOPDIRECTORY, FALSE);
	SaveLnkDialog->InitialDir = (UnicodeString)DesktopFolder;

	FormStyle = fsNormal;

	if(InstanceName != L"milligram")
	{
		MessageBeep(MB_ICONASTERISK);
		ShowMessage(LoadStr(1004));
		SaveLnkDialog->FileName = SaveLnkDialog->InitialDir + L"\\milligram " + InstanceName + ".lnk";

		if(SaveLnkDialog->Execute() == false)return(false);

		CreateLink(Application->ExeName, SaveLnkDialog->FileName, L"\"-" + InstanceName + L"\"");
		InstanceMode = true;
	}
	else
	{
		MessageBeep(MB_ICONASTERISK);
		ShowMessage(LoadStr(1005));
		SaveLnkDialog->FileName = SaveLnkDialog->InitialDir + L"\\milligram image viewer.lnk";

		if(SaveLnkDialog->Execute() == false)return(false);

		CreateLink(Application->ExeName, SaveLnkDialog->FileName, L"");
		InstanceMode = false;
	}
	ShortCutFileName = SaveLnkDialog->FileName;

	return(true);
}


bool __fastcall TMainForm::DoCreateSendToLink(UnicodeString InstanceName)
{
	wchar_t TargetFolder[MAX_PATH];
	UnicodeString Temp;
	int Result;

	MessageBeep(MB_ICONQUESTION);
	Result = MessageDlg(LoadStr(1006), mtConfirmation, TMsgDlgButtons() << mbYes << mbNo, NULL);
	if(Result == mrYes)
	{
		SHGetSpecialFolderPath(NULL, TargetFolder, CSIDL_SENDTO, FALSE);

		if(InstanceName == "milligram")
			Temp = (UnicodeString)TargetFolder + L"\\milligram image viewer.lnk";
		else
			Temp = (UnicodeString)TargetFolder + L"\\milligram " + InstanceName + L".lnk";

		if(FileExists(Temp) == true)
		{
			MessageBeep(MB_ICONQUESTION);
			Result = MessageDlg(LoadStr(1007), mtConfirmation, TMsgDlgButtons() << mbYes << mbNo, NULL);
		}

		if(Result == mrYes)
		{
			CreateLink(Application->ExeName, Temp, L"");
			CreateSendToLink = true;
		}
	}

	if(Result == mrNo)return(false);
	return(true);
}

bool __fastcall TMainForm::DoUninstall(void)
{
	wchar_t TargetFolder[MAX_PATH];
	bool Result = true;
	TStringList *FileSL = new TStringList;
	TStringList *IniSL = new TStringList;
	UnicodeString Temp = GetIniFolder() + L"\\milligram\\*.ini";
	int i;

	GetFileLists(Temp, FileSL, L"*", -1);

	for(i = 0;i < FileSL->Count;i++)
	{
		LoadStringList(IniSL, FileSL->Strings[i]);

		if(CheckStrings(IniSL, L"ShortCutFileName") == true)
		{
			Temp = IniSL->Values[L"ShortCutFileName"];

			if(FileExists(Temp))
				Result = Result && DeleteFile(Temp);
		}

		if(CheckStrings(IniSL, L"CreateSendToLink") == true)
		{
			Temp = IniSL->Values[L"CreateSendToLink"];

			if(Temp.LowerCase() == L"true")
			{
				Temp = ChangeFileExt(ExtractFileName(FileSL->Strings[i]), L"");
				Result = Result && DeleteSendToLink(Temp);
			}
		}
	}

	delete FileSL;
	delete IniSL;

	Temp = GetIniFolder() + L"\\milligram";
	DeleteFolder(Temp);

	return(Result);
}

bool __fastcall TMainForm::DeleteSendToLink(UnicodeString InstanceName)
{
	wchar_t TargetFolder[MAX_PATH];
	UnicodeString Temp;

	SHGetSpecialFolderPath(NULL, TargetFolder, CSIDL_SENDTO, FALSE);

	if(InstanceName == "milligram")
		Temp = (UnicodeString)TargetFolder + L"\\milligram image viewer.lnk";
	else
		Temp = (UnicodeString)TargetFolder + L"\\milligram " + InstanceName + L".lnk";

	if(FileExists(Temp))
		DeleteFile(Temp);
	else
  	return(false);
	return(true);
}


bool __fastcall TMainForm::OpenFiles(TStringList *SrcLists)
{
	return(OpenFiles(SrcLists, L"", 0, (GetAsyncKeyState(VK_LCONTROL) & 0x8000) || (GetAsyncKeyState(VK_RCONTROL) & 0x8000)));
}


bool __fastcall TMainForm::OpenFiles(TStringList *SrcLists, UnicodeString SelectedFile, int Offset, bool AddMode)
{
	bool AddLists;
	TStringList *ArcLists = new TStringList();
	TStringList *TempLists = new TStringList();

	CheckGetLists(TempLists, ArcLists, SrcLists);

	if(TempLists->Count == 0)
	{
		ClearArchiveFileLists(ArcLists);
		delete ArcLists;
		delete TempLists;
		return(true);
	}

	CloseArchiveMode();

	if((GetAsyncKeyState(VK_LCONTROL) & 0x8000)
	|| (GetAsyncKeyState(VK_RCONTROL) & 0x8000)
	|| AddMode)
	{
		AddLists = true;
		AddFileLists(TempLists, ArcLists, 1);
	}
	else
	{
		AddLists = false;
		AddFileLists(TempLists, ArcLists, 0);
	}

	SetFileList();
	if(SelectedFile != L"")ShowIndex = FileLists->IndexOf(SelectedFile);

	ShowIndex += Offset;
	while(ShowIndex < 0)ShowIndex += FileLists->Count;
	while(ShowIndex >= FileLists->Count)ShowIndex -= FileLists->Count;
	DisplayBox->ItemIndex = ShowIndex;

	if(ShowingList == false || AddLists == false)
	{
		ToggleShowList(3);
	}

	delete ArcLists;
	delete TempLists;
	return(true);
}


void __fastcall TMainForm::LoopProc(TMessage& Msg)
{
	if(Locked == true)
	{
		if(
			(Msg.Msg == WM_APPCOMMAND
		&& Msg.LParamHi == APPCOMMAND_BROWSER_FORWARD
		&& (GetAsyncKeyState(VkRButton) & 0x8000))
		||
			(Msg.Msg == WM_XBUTTONUP
		&& Msg.WParamHi == XBUTTON2
		&& (GetAsyncKeyState(VkRButton) & 0x8000))
			)
		{
			RClkCancel = true;
			MnLockClick(NULL);
		}
		else
			Dropper->oldWinProc(Msg);
	}
	else
	switch(Msg.Msg)
	{
		case WM_DROPFILES:
			{
				bool AddLists;
				TStringList *DropLists = new TStringList();

				Dropper->GetDropFileNames(Msg, DropLists);
				AddHistoryList(DropLists);
				OpenFiles(DropLists);

				delete DropLists;
				SetAbsoluteForegroundWindow(Handle);
			}
			break;

		case WM_APPCOMMAND:
			switch(Msg.LParamHi)
			{
				case APPCOMMAND_MEDIA_PREVIOUSTRACK:
					ShowOffsetImage(-1);
					break;
				case APPCOMMAND_MEDIA_NEXTTRACK:
					ShowOffsetImage(1);
					break;

				case APPCOMMAND_BROWSER_BACKWARD:
					if(GetAsyncKeyState(VkRButton) & 0x8000)
					{
						RClkCancel = true;
						MnCloseArchiveClick(NULL);
					}
					else
						ShowOffsetImage(-1);

					break;

				case APPCOMMAND_BROWSER_FORWARD:
					if(GetAsyncKeyState(VkRButton) & 0x8000)
					{
						RClkCancel = true;
						MnLockClick(NULL);
					}
					else
						ShowOffsetImage(1);
					break;

				case APPCOMMAND_MEDIA_CHANNEL_UP:
					ZoomImage(Percentage / 1.5);
					break;
				case APPCOMMAND_MEDIA_CHANNEL_DOWN:
					ZoomImage(Percentage * 1.5);
					break;
				case APPCOMMAND_MEDIA_STOP:
					IntervalOffset(0);
					SlideShow = false;
					MnSlideShow->Checked = false;
					SSTimer->Enabled = false;
					break;
				case APPCOMMAND_MEDIA_PLAY_PAUSE:
          MnSlideShowClick(NULL);
        	break;
				case APPCOMMAND_MEDIA_PLAY:
					IntervalOffset(0);
					SlideShow = true;
					MnSlideShow->Checked = true;
					SSTimer->Enabled = true;
					break;
				case APPCOMMAND_MEDIA_REWIND:
					IntervalOffset(-1);
					break;
				case APPCOMMAND_MEDIA_FAST_FORWARD:
					IntervalOffset(1);
					break;
			}
			break;

		case WM_XBUTTONUP:
			switch(Msg.WParamHi)
			{
				case XBUTTON1:
					if(GetAsyncKeyState(VkRButton) & 0x8000)
					{
						RClkCancel = true;
						MnCloseArchiveClick(NULL);
					}
					else
						ShowOffsetImage(-1);
					break;

				case XBUTTON2:
					if(GetAsyncKeyState(VkRButton) & 0x8000)
					{
						RClkCancel = true;
						MnLockClick(NULL);
					}
					else
						ShowOffsetImage(1);
					break;
			}
			break;

		case WM_SYSCOMMAND:
			switch(Msg.WParam)
			{
				case SC_MINIMIZE:
					if(WindowState == fsNormal)
					{
						MnShowClick(NULL);
					}
					break;

				default:
					Dropper->oldWinProc(Msg);
					break;
			}
			break;

		default:
			Dropper->oldWinProc(Msg);
			break;
	}

/*
	ClockTGT = timeGetTime();
	if(Susie->GIFAnimate == true)
	{
		if(ClockTGT - PreTGT > Susie->GIFInterval)
		{
			GIFRefresh = true;
			Paint();
		}
	}
	*/
}


void __fastcall TMainForm::MyIdleHandler(TObject *Sender, bool &Done)
{
	// GIF アニメの処理
	ClockTGT = timeGetTime();
	if(Susie->GIFAnimate == true && RotateValue == 0)
	{
		if(ClockTGT - PreTGT > 10)
		{
			GIFRefresh = true;
			Paint();
		}
	}

	// 終了
	Done = true;
}

bool __fastcall TMainForm::CheckGetLists(TStringList *DestLists, TStringList *ArcLists, TStringList *DropLists)
{
	int i;
	for(i = 0;i < DropLists->Count;i++)
	{
		if(DirectoryExists(DropLists->Strings[i]))
		{
			GetImageLists(IncludeTrailingPathDelimiter(DropLists->Strings[i]) + "\*.*", DestLists, SearchSubFolder, EnableFileMask, FileMaskString);
		}
		else if(ExtractFileExt(DropLists->Strings[i]) == ".mfl")
		{
			TStringList *TempSL = new TStringList();
			LoadStringList(TempSL, DropLists->Strings[i]);
			LoadFileLists(DestLists, ArcLists, TempSL);
			delete TempSL;
		}
		else
		{
			if(EnableFileMask && CheckMasks(DropLists->Strings[i], FileMaskString) == false)continue;

			int FAge = FileAge(DropLists->Strings[i]);

			SImageInfo *NewII = new SImageInfo;
			NewII->Timestamp = FAge;
			NewII->Rotate = -1;
			DestLists->AddObject(DropLists->Strings[i], (TObject *)NewII);
		}
	}
	return(true);
}

bool __fastcall TMainForm::AddFileLists(TStringList *SrcLists, TStringList *ArcLists, int Mode)
{
	int i, j;
	switch(Mode)
	{
		// 新規
		case 0:
			ShowIndex = 0;
			ClearArchiveFileLists(ArchiveFileLists);

			for(j = 0;j < FileLists->Count;j++)
				delete (SImageInfo *)FileLists->Objects[j];
			FileLists->Clear();
			Susie->Clear(MMODE_ALL);

			while(SrcLists->Count > 0)
			{
				if(FileLists->IndexOf(SrcLists->Strings[0]) < 0)
					FileLists->AddObject(SrcLists->Strings[0], SrcLists->Objects[0]);
				else
					delete (SImageInfo *)SrcLists->Objects[0];

				SrcLists->Delete(0);
			}
			if(ArcLists != NULL)ArchiveFileLists->AddStrings(ArcLists);
			break;

		// 追加
		case 1:
			CloseArchiveMode();
			ShowIndex = FileLists->Count;

			while(SrcLists->Count > 0)
			{
				if(FileLists->IndexOf(SrcLists->Strings[0]) < 0)
					FileLists->AddObject(SrcLists->Strings[0], SrcLists->Objects[0]);
				else
					delete (SImageInfo *)SrcLists->Objects[0];

				SrcLists->Delete(0);
			}
			if(ArcLists != NULL)ArchiveFileLists->AddStrings(ArcLists);

			if(ShowIndex >= FileLists->Count)
				ShowIndex = FileLists->Count - 1;
			break;
	}
	return(true);
}

bool __fastcall TMainForm::DeleteFileLists(int DeleteMode)  // 1 ファイルをゴミ箱に移す 2 確認無くゴミ箱に移す
{
	int i;
	if((ShowingList == false && ShowIndex == -1) || (ShowingList == true && DisplayBox->ItemIndex < 0))return(false);

	if(InArchive && DeleteMode > 0)
	{
		MessageBeep(MB_ICONASTERISK);
		ShowMessage(LoadStr(1008));
		return(false);
	}

	if(DeleteMode)
	{
		bool bResult;

		if(ShowingList == true)
		{
			if(DeleteMode == 1)
			{
				int Result;
				UnicodeString Mes;

				if(DisplayBox->SelCount == 1)
				{
					for(i = 0;i < DisplayBox->Count;i++)
					{
						if(DisplayBox->Selected[i] == true)break;
					}
					Mes = GetShortFileName(DisplayLists->Strings[i]) + L"\n";
				}
				else
					Mes = LoadStr(1009);

				Mes += LoadStr(1010);
				Result = MessageDlg(Mes, mtConfirmation, TMsgDlgButtons() << mbYes << mbCancel, NULL);

				if(Result == mrCancel)return(false);
			}

			TStringList *DelList = new TStringList();
			for(i = 0;i < DisplayBox->Count;i++)
			{
				if(DisplayBox->Selected[i] == true)
				{
					DelList->Add(DisplayLists->Strings[i]);
				}
			}
			bResult = DeleteFileToRecycle(DelList, false);
			delete DelList;
		}
		else
		{
			if(DeleteMode == 1)
			{
				int Result;
				UnicodeString Mes;

				if(DisplayBox->SelCount == 1)
					Mes = GetShortFileName(DisplayLists->Strings[ShowIndex]) + L"\n";
				else
					Mes = LoadStr(1011);

				Mes += LoadStr(1012);
				Result = MessageDlg(Mes, mtConfirmation, TMsgDlgButtons() << mbYes << mbCancel, NULL);

				if(Result == mrCancel)return(false);

				bResult = DeleteFileToRecycle(DisplayLists->Strings[ShowIndex], false, Handle);
			}
		}

		if(bResult == false)
		{
			MessageBeep(MB_ICONASTERISK);
			ShowMessage(LoadStr(1013));
			return(false);
		}
	}

	if(ShowingList == true)
	{
		if(DisplayBox->SelCount > 1)
		{
			for(i = DisplayBox->Count - 1;i >= 0;i--)
			{
				if(DisplayBox->Selected[i] == true)
				{
					DeleteFileInList(i);
					if(DisplayBox->ItemIndex == i)
					{
						DisplayBox->ItemIndex--;
						if(DisplayBox->ItemIndex >= DisplayBox->Count)DisplayBox->ItemIndex = DisplayBox->Count - 1;
					}
				}
			}
		}
		else if(DisplayBox->ItemIndex >= 0)
		{
			DeleteFileInList(DisplayBox->ItemIndex);
		}

		if(DisplayBox->Count > 0)
		{
			if(DisplayBox->ItemIndex < 0)DisplayBox->ItemIndex = 0;
			DisplayBox->Selected[DisplayBox->ItemIndex] = true;
		}

	}
	else
	{
		if(ShowIndex >= 0 && ShowIndex < DisplayLists->Count)
			DeleteFileInList(ShowIndex);
	}
	return(true);
}

bool __fastcall TMainForm::DeleteFileInList(int i)
{
	if(ShowingList)
	{
		if(DisplayBox->Count > 0)
		{
			if(i == ShowIndex)
			{
				ShowIndex = -1;
				Susie->Clear(MMODE_PICTURE);
//				Hint = L"";
			}

			int PreviousII = DisplayBox->ItemIndex;
			delete (SImageInfo *)DisplayLists->Objects[i];
			DisplayLists->Delete(i);
			DisplayBox->Items->Delete(i);

			if(DisplayBox->Count == 0)
			{
				if(InArchive == true)
				{
					CloseArchiveMode();
					SetFileList();
					DeleteFileLists(0);
				}
			}

			if(i == PreviousII)
			{
				if(DisplayBox->Count == PreviousII)
					DisplayBox->ItemIndex = PreviousII - 1;
				else
					DisplayBox->ItemIndex = PreviousII;
			}
		}
	}
	else
	{
		delete (SImageInfo *)DisplayLists->Objects[i];
		DisplayLists->Delete(i);
		if(DisplayLists->Count == 0)
		{
			if(InArchive == true)
			{
				CloseArchiveMode();
				DeleteFileLists(0);
			}
			else
				ToggleShowList(1);
		}
		ShowOffsetImage(0);
	}
	return(true);
}

bool __fastcall TMainForm::DeleteArchiveData(UnicodeString ArcFileName)
{
	int i = FileLists->IndexOf(ArcFileName);
	if(i < 0)return(false);

	delete (SImageInfo *)FileLists->Objects[i];
	FileLists->Delete(i);

	i = ArchiveFileLists->IndexOf(ArcFileName);
	if(i < 0)return(false);

	ArchiveFileLists->Delete(i);

	return(true);
}

bool __fastcall TMainForm::MoveSelectedList(int Offset)
{
	int i, j, k;
	// まず Offset の値によって場合分け
	if(Offset < 0)
	{
		// 一番上の Selected を取得する
		for(i = 0;i < DisplayBox->Count;i++)
		{
			if(DisplayBox->Selected[i])break;
		}
		if(i == DisplayBox->Count || i == 0)return(false);

		for(;i < DisplayBox->Count;i++)
		{
			j = i - 1; k = i + 1;
			while(k < DisplayBox->Count && DisplayBox->Selected[k] == true)
				k++;

			k--;
			DisplayLists->Move(j, k);
			DisplayBox->Items->Move(j, k);
			i = k;

			while(i < DisplayBox->Count && DisplayBox->Selected[i] == false)
				i++;
			i--;
		}
		DisplayBox->ItemIndex += Offset;
	}
	else
	{
		// 一番下の Selected を取得する
		for(i = DisplayBox->Count - 1;i >= 0;i--)
		{
			if(DisplayBox->Selected[i])break;
		}
		if(i == DisplayBox->Count - 1 || i == -1)return(false);

		for(;i >= 0;i--)
		{
			j = i + 1; k = i - 1;
			while(k >= 0 && DisplayBox->Selected[k] == true)
				k--;

			k++;
			DisplayLists->Move(j, k);
			DisplayBox->Items->Move(j, k);
			i = k;

			while(i >= 0 && DisplayBox->Selected[i] == false)
				i--;
			i++;
		}
		DisplayBox->ItemIndex += Offset;
	}
	return(true);
}

bool __fastcall TMainForm::SortByName(void)
{
	int i, j;
	TStringList *TempList = new TStringList();
	UnicodeString OldFileName = DisplayLists->Strings[ShowIndex];

	if(DisplayBox->SelCount <= 1)
		TempList->AddStrings(DisplayLists);
	else
	{
		for(i = 0;i < DisplayLists->Count;i++)
		{
			if(DisplayBox->Selected[i])
				TempList->AddObject(DisplayLists->Strings[i], DisplayLists->Objects[i]);
		}
	}

	TempList->Sort();

	if(DisplayBox->SelCount <= 1)
	{
		DisplayLists->Clear();
		DisplayLists->AddStrings(TempList);
	}
	else
	{
		j = 0;
		for(i = 0;i < DisplayLists->Count;i++)
		{
			if(DisplayBox->Selected[i])
			{
				DisplayLists->Strings[i] = TempList->Strings[j];
				DisplayLists->Objects[i] = TempList->Objects[j];
				j++;
				if(j == TempList->Count)break;
			}
		}
	}

	delete TempList;
	ShowIndex = DisplayLists->IndexOf(OldFileName);
	SetFileList();
	return(true);
}

struct SSortInfo
{
	UnicodeString Name;
	UnicodeString Ext;
	SImageInfo *ImageInfo;
};

int __fastcall CompareExt(void *Item1, void *Item2)
{
	SSortInfo *TempSI1, *TempSI2;
	TempSI1 = (SSortInfo *)Item1;
	TempSI2 = (SSortInfo *)Item2;
	return(wcscmp(TempSI1->Ext.c_str(), TempSI2->Ext.c_str()));
}

int __fastcall CompareTime(void *Item1, void *Item2)
{
	SSortInfo *TempSI1, *TempSI2;
	TempSI1 = (SSortInfo *)Item1;
	TempSI2 = (SSortInfo *)Item2;
	return(TempSI1->ImageInfo->Timestamp - TempSI2->ImageInfo->Timestamp);
}

int __fastcall CompareSize(void *Item1, void *Item2)
{
	SSortInfo *TempSI1, *TempSI2;
	TempSI1 = (SSortInfo *)Item1;
	TempSI2 = (SSortInfo *)Item2;
	return(TempSI1->ImageInfo->FileSize - TempSI2->ImageInfo->FileSize);
}

bool __fastcall TMainForm::SortBy(int Type)
{
	int i, j;
	TList *TempList = new TList();
	SSortInfo *TempSI;

	UnicodeString OldFileName = DisplayLists->Strings[ShowIndex];

	if(DisplayBox->SelCount <= 1)
	{
		if(Type == 0)
		{
			for(i = 0;i < DisplayLists->Count;i++)
			{
				TempSI = new SSortInfo;
				TempSI->ImageInfo = (SImageInfo *)DisplayLists->Objects[i];
				TempSI->Name = DisplayLists->Strings[i];
				TempSI->Ext = ExtractFileExt(TempSI->Name);
				TempList->Add(TempSI);
			}
		}
		else
		{
			for(i = 0;i < DisplayLists->Count;i++)
			{
				TempSI = new SSortInfo;
				TempSI->ImageInfo = (SImageInfo *)DisplayLists->Objects[i];
				TempSI->Name = DisplayLists->Strings[i];
				TempList->Add(TempSI);
			}
		}
	}
	else
	{
		if(Type == 0)
		{
			for(i = 0;i < DisplayLists->Count;i++)
			{
				if(DisplayBox->Selected[i])
				{
					TempSI = new SSortInfo;
					TempSI->ImageInfo = (SImageInfo *)DisplayLists->Objects[i];
					TempSI->Name = DisplayLists->Strings[i];
					TempSI->Ext = ExtractFileExt(TempSI->Name);
					TempList->Add(TempSI);
				}
			}
		}
		else
		{
			for(i = 0;i < DisplayLists->Count;i++)
			{
				if(DisplayBox->Selected[i])
				{
					TempSI = new SSortInfo;
					TempSI->ImageInfo = (SImageInfo *)DisplayLists->Objects[i];
					TempSI->Name = DisplayLists->Strings[i];
					TempList->Add(TempSI);
				}
			}
		}
	}

	switch(Type)
	{
		case 0:
			TempList->Sort(CompareExt);
			break;
		case 1:
			TempList->Sort(CompareTime);
			break;
		case 2:
			TempList->Sort(CompareSize);
			break;
	}

	if(DisplayBox->SelCount <= 1)
	{
		DisplayLists->Clear();
		for(i = 0;i < TempList->Count;i++)
		{
			TempSI = (SSortInfo *)TempList->Items[i];
			DisplayLists->AddObject(TempSI->Name, (TObject *)TempSI->ImageInfo);
			delete TempSI;
		}
	}
	else
	{
		j = 0;
		for(i = 0;i < DisplayLists->Count;i++)
		{
			if(DisplayBox->Selected[i])
			{
				TempSI = (SSortInfo *)TempList->Items[i];
				DisplayLists->Strings[i] = TempSI->Name;
				DisplayLists->Objects[i] = (TObject *)TempSI->ImageInfo;
				delete TempSI;
				j++;
				if(j == TempList->Count)break;
			}
		}
	}

	delete TempList;
	ShowIndex = DisplayLists->IndexOf(OldFileName);
	SetFileList();
	return(true);
}



bool __fastcall TMainForm::CreateDBImage(void)
{
	DBTimer->Enabled = true;
	return(true);
}

int PASCAL ProgressCallback(int nNum, int nDenom, long lData)
{
	long Now = timeGetTime();
	if(MainForm->SlideShow == false)
	{
		switch(MainForm->LoadState)
		{
			case 0:
				if(nNum == 0)break;
				if(nNum > 0 && nNum < nDenom)
				{
					if((Now - MainForm->LoadStart) * nDenom / nNum > 500)
					{
						MainForm->StartnNum = nNum;
						MainForm->LoadState = 1;
					}
					else
						MainForm->LoadState = 2;
				}
				break;

			case 1:
				{
					double P = (double)(nNum - MainForm->StartnNum) / (nDenom - MainForm->StartnNum);
					MainForm->Canvas->Pen->Style = psSolid;

					MainForm->Canvas->Pen->Color = MainForm->FrameColor;
					MainForm->Canvas->PenPos = TPoint(0, MainForm->Height - 1);
					MainForm->Canvas->LineTo(P * MainForm->Width, MainForm->Height - 1);

					MainForm->Canvas->Pen->Color = MainForm->FullFillColor;
					MainForm->Canvas->LineTo(MainForm->Width, MainForm->Height - 1);
				}
				break;

			case 2:
				break;
		}
	}

	if(GetAsyncKeyState(VK_ESCAPE) & 0x8000)
	{
		while(GetAsyncKeyState(VK_ESCAPE) & 0x8000);
		return(1);
	}

	return(0);
}

int __fastcall TMainForm::OpenFile(UnicodeString FilePath)
{
	int Result = 0;

//  if(EnableFileMask && CheckMasks(FilePath, FileMaskString) == false)return(false);

	LoadStart = timeGetTime();
	LoadState = 0;

	if(InArchive == false)
	{
		Result = Susie->SetImageFile(FilePath);
		if(Susie->GIFAnimate == true)
		{
			PreTGT = ClockTGT;
		}
	}
	else
		Result = Susie->SetSubImageFile(FilePath);

	Caption = ExtractFileName(FilePath);

	return(Result);
}

bool __fastcall TMainForm::OpenArchiveMode(UnicodeString Src, int SubIndex, int Ofs, bool MustShowImage)
{
	InArchive = true;
	MnCloseArchive->Enabled = true;

	OpeningFileName = Src;
//	FileChanged->SetCheckFile(OpeningFileName);

	int i = ArchiveFileLists->IndexOf(Src), j;
	if(i < 0)
	{
		DisplayLists = new TStringList();
		ArchiveFileLists->AddObject(Src, (TObject *)DisplayLists);
		Susie->GetArchiveFileLists(DisplayLists);
		for(j = 0;j < DisplayLists->Count;j++)
		{
			DisplayLists->Strings[j] = DisplayLists->Strings[j].Trim();
			if(DisplayLists->Strings[j].Length() == 0
			||
				(EnableFileMask && CheckMasks(DisplayLists->Strings[j], FileMaskString) == false)
			)
			{
				delete (SImageInfo *)DisplayLists->Objects[j];
				DisplayLists->Delete(j);
				j--;
			}
		}
	}
	else
	{
		DisplayLists = (TStringList *)ArchiveFileLists->Objects[i];
	}

	if(DisplayLists->Count == 0)CloseArchiveMode();

	ShowIndexBack = ShowIndex;
	if(SubIndex < 0)
	{
		if(Ofs < 0)
			ShowIndex = DisplayLists->Count - 1;
		else
			ShowIndex = 0;
	}
	else
		ShowIndex = SubIndex;

	if(MustShowImage == false)
	{
		SetFileList();
	}
	else if(Susie->SetSubImageFile(DisplayLists, ShowIndex, Ofs) == false)
	{
		CloseArchiveMode();
		return(false);
	}
	return(true);
}

bool __fastcall TMainForm::CloseArchiveMode(void)
{
	if(InArchive == false)return(true);
	MnCloseArchive->Enabled = false;

	if(DisplayLists->Count == 0)
		DeleteArchiveData(FileLists->Strings[ShowIndexBack]);

	Susie->Clear(MMODE_ALL);
	DisplayLists = FileLists;
	ShowIndex = ShowIndexBack;

	if(ShowIndex < 0)ShowIndex = 0;
	if(FileLists->Count <= ShowIndex)ShowIndex = FileLists->Count - 1;

	ShowIndexBack = -1;
	InArchive = false;

	OpeningFileName = FileLists->Strings[ShowIndex];
//	FileChanged->SetCheckFile(OpeningFileName);

	return(true);
}

void __fastcall TMainForm::ToggleShowList(int Mode) // Mode 0:反転 1:表示 2:非表示 3:強制非表示
{
	bool PreSL = ShowingList;
	int Result;

	switch(Mode)
	{
		case 0:
			ShowingList = !ShowingList;
			break;
		case 1:
			ShowingList = true;
			break;
		case 2:
		case 3:
			ShowingList = false;
			break;
	}

	if(ShowingList == false
	&& DisplayLists->Count > 0
	&& DisplayBox->ItemIndex >= 0
	&& DisplayBox->ItemIndex < DisplayBox->Count)
	{
		Result = ShowAbsoluteImage(DisplayBox->ItemIndex, -1, 1, Mode == 3);
	}

	// 0 正常終了 1 リスト更新の必要有り
	if(Result == 1 && Mode != 3)
	{
		SetFileList();
		ShowingList = true;
	}

	#define MIV_SMALLESTWINDOWSIZE 200
	if(PreSL != ShowingList)
	{
		if(ShowingList == true)
		{
			SetFileList();
			DisplayBox->Visible = true;
			DisplayBox->TopIndex = DisplayBox->ItemIndex - (DisplayBox->ClientHeight / DisplayBox->ItemHeight) / 2;
			DisplayBox->SetFocus();
			MnShowList->Caption = LoadStr(1014);

			if(WWidth < MIV_SMALLESTWINDOWSIZE || WHeight < MIV_SMALLESTWINDOWSIZE)
			{
				if(WWidth < MIV_SMALLESTWINDOWSIZE)
				{
					WLeft = WLeft + (WWidth - MIV_SMALLESTWINDOWSIZE) / 2;
					WWidth = MIV_SMALLESTWINDOWSIZE;
				}
				if(WHeight < MIV_SMALLESTWINDOWSIZE)
				{
					WTop = WTop + (WHeight - MIV_SMALLESTWINDOWSIZE) / 2;
					WHeight = MIV_SMALLESTWINDOWSIZE;
				}
				Ratio = WWidth / WHeight;
			}
		}
		else
		{
			DisplayBox->Visible = false;
			MnShowList->Caption = LoadStr(1015);
		}
	}
}

bool __fastcall TMainForm::SetFileList(void)
{
	int i;
	DisplayBox->Clear();
	for(i = 0;i < DisplayLists->Count;i++)
	{
		DisplayBox->Items->Add(GetShortFileName(DisplayLists->Strings[i]));
	}
	if(ShowIndex >= 0 && DisplayBox->Count > ShowIndex)
	{
		DisplayBox->ItemIndex = ShowIndex;
		DisplayBox->Selected[ShowIndex] = true;
	}
	else
		DisplayBox->ItemIndex = -1;

	DisplayBox->Hint = L"";
	DisplayBox->ShowHint = false;
	return(true);
}


UnicodeString __fastcall TMainForm::GetShortFileName(UnicodeString Temp)
{
	int L;
	L = Temp.Length();

	while(L > 1)
	{
		if(Temp[L] == L'\\')
			break;
		L--;
	}

	if(L > 2)
	{
		L--;
		while(L > 1)
		{
			if(Temp[L] == L'\\')
				break;
			L--;
		}
	}

	if(L > 1)L++;

	Temp = Temp.SubString(L, Temp.Length() - L + 1);
	return(Temp);
}


void __fastcall TMainForm::SetNewImageSize(void)
{
	double ImageWidth = Susie->Width;
	double ImageHeight = Susie->Height;
	Ratio = (double)Susie->Width / Susie->Height;

	double MaxWidth = (double)MWidth;// * 9 / 10;
	double MaxHeight = (double)MHeight;// * 9 / 10;

	if(FixPercentage || ReloadMode)
	{
		ImageWidth = Susie->Width * Percentage;
		ImageHeight = Susie->Height * Percentage;
	}

	if(FitToScreen == true)
	{
		FitMode = 0;

		if(ImageWidth / MaxWidth > ImageHeight / MaxHeight)
			FitMode = 2,
			ImageWidth = MaxWidth,
			ImageHeight = MaxWidth / Ratio;
		else
			FitMode = 1,
			ImageHeight = MaxHeight,
			ImageWidth = MaxHeight * Ratio;

		if(ImageWidth == MaxWidth && ImageHeight == MaxHeight)
			FitMode = 3;
	}
	else if(ShowConstant == true)
	{
		FitMode = 0;

		double d = ConstantSize / (sqrt(ImageWidth * ImageWidth + ImageHeight * ImageHeight));
		ImageWidth = ImageWidth * d;
		ImageHeight = ImageHeight * d;
	}
	else if(UseWholeScreen == true)
	{
		if(ImageWidth / MaxWidth < ImageHeight / MaxHeight)
			FitMode = 2,
			ImageWidth = MaxWidth,
			ImageHeight = MaxWidth / Ratio;
		else
			FitMode = 1,
			ImageHeight = MaxHeight,
			ImageWidth = MaxHeight * Ratio;

		if(ImageWidth == MaxWidth && ImageHeight == MaxHeight)
			FitMode = 3;
	}
	else if(FixPercentage == false)
	{
		FitMode = 0;
		if(MaxWidth < ImageWidth)
		{
			if(MaxHeight < ImageHeight)
			{
				if(ImageWidth / MaxWidth > ImageHeight / MaxHeight)
					ImageWidth = MaxWidth,
					ImageHeight = MaxWidth / Ratio;
				else
					ImageHeight = MaxHeight,
					ImageWidth = MaxHeight * Ratio;
			}
			else
			{
				ImageWidth = MaxWidth;
				ImageHeight = MaxWidth / Ratio;
			}
		}

		if(MaxHeight < ImageHeight)
		{
			ImageHeight = MaxHeight;
			ImageWidth = MaxHeight * Ratio + 0.5;
		}
	}

	SetWindowSize(ImageWidth, ImageHeight, true);
	Susie->CreateDBImage(WWidth, WHeight);
	Paint();
	Holding = 0;
}

void __fastcall TMainForm::SetWindowSize(int iWidth, int iHeight, bool InMonitor)
{
	WWidth = iWidth;
	WHeight = iHeight;

	if(FitToScreen == true)
	{
		if((FitMode & 2) == 0)
		{
			WTop = Monitor->Top + FrameWidth;
			switch(PositionMode)
			{
				case 0:
					WLeft = CenterX - WWidth / 2 + FrameWidth;
					if(WLeft < Monitor->Left + FrameWidth)WLeft = Monitor->Left + FrameWidth;
					if(WLeft + WWidth > Monitor->Left + Monitor->Width - FrameWidth)WLeft = Monitor->Left + Monitor->Width - WWidth - FrameWidth;
					break;
				case 1:
					WLeft = Monitor->Left + Monitor->Width / 2 - WWidth / 2 + FrameWidth;
					break;
				case 2:
				case 4:
					WLeft = Monitor->Left + FrameWidth;
					break;
				case 3:
				case 5:
					WLeft = Monitor->Left + Monitor->Width - WWidth + FrameWidth;
					break;
			}
		}

		if((FitMode & 1) == 0)
		{
			WLeft = Monitor->Left + FrameWidth;
			switch(PositionMode)
			{
				case 0:
					WTop = CenterY - WHeight / 2 + FrameWidth;
					if(WTop < Monitor->Top + FrameWidth)WTop = Monitor->Top + FrameWidth;
					if(WTop + WHeight > Monitor->Top + Monitor->Height - FrameWidth)WTop = Monitor->Top + Monitor->Height - WHeight - FrameWidth;
					break;
				case 1:
					WTop = Monitor->Top + Monitor->Height / 2 - WHeight / 2 + FrameWidth;
					break;
				case 2:
				case 3:
					WTop = Monitor->Top + FrameWidth;
					break;
				case 4:
				case 5:
					WTop = Monitor->Top + Monitor->Height - WHeight + FrameWidth;
					break;
			}
		}
	}
	else if(UseWholeScreen == true)
	{
		if((FitMode & 2) == 0)
		{
			WTop = Monitor->Top + FrameWidth;
			switch(PositionMode)
			{
				case 0:
					WLeft = CenterX - WWidth / 2 + FrameWidth;
					if(WLeft > Monitor->Left + FrameWidth)WLeft = Monitor->Left + FrameWidth;
					if(WLeft + WWidth < Monitor->Left + Monitor->Width - FrameWidth)WLeft = Monitor->Left + Monitor->Width - WWidth - FrameWidth;
					break;
				case 1:
					WLeft = Monitor->Left + Monitor->Width / 2 - WWidth / 2 + FrameWidth;
					if(WLeft > Monitor->Left + FrameWidth)WLeft = Monitor->Left + FrameWidth;
					if(WLeft + WWidth < Monitor->Left + Monitor->Width - FrameWidth)WLeft = Monitor->Left + Monitor->Width - WWidth - FrameWidth;
					break;
				case 2:
				case 4:
					WLeft = Monitor->Left + FrameWidth;
					break;
				case 3:
				case 5:
					WLeft = Monitor->Left + Monitor->Width - WWidth + FrameWidth;
					break;
			}
		}

		if((FitMode & 1) == 0)
		{
			WLeft = Monitor->Left + FrameWidth;
			switch(PositionMode)
			{
				case 0:
					WTop = CenterY - WHeight / 2 + FrameWidth;
					if(WTop > Monitor->Top + FrameWidth)WTop = Monitor->Top + FrameWidth;
					if(WTop + WHeight < Monitor->Top + Monitor->Height - FrameWidth)WTop = Monitor->Top + Monitor->Height - WHeight - FrameWidth;
					break;
				case 1:
					WTop = Monitor->Top + Monitor->Height / 2 - WHeight / 2 + FrameWidth;
					break;
				case 2:
				case 3:
					WTop = Monitor->Top + FrameWidth;
					break;
				case 4:
				case 5:
					WTop = Monitor->Top + Monitor->Height - WHeight + FrameWidth;
					break;
			}
		}
	}
	else if(InMonitor && FixPercentage == false && ReloadMode == false)
	{
		MoveByPositionMode();
	}
	else
	{
		WLeft = CenterX - WWidth / 2;
		WTop = CenterY - WHeight / 2;
	}

	ScreenSnap = false;
	SyncWindow(false);
	ScreenSnap = MnSnapWindow->Checked;
}

bool __fastcall TMainForm::MoveByPositionMode(void)
{
	if(ReloadMode == true || PositionMode == 0)
	{
		if(WWidth > MWidth)
			CenterX = MWidth / 2 + MLeft;

		if(WHeight > MHeight)
			CenterY = MHeight / 2 + MTop;

		if(CenterX - WWidth / 2 < MLeft)
			WLeft = MLeft;
		else if(CenterX + WWidth / 2 > MLeft + MWidth)
			WLeft = MLeft + MWidth - WWidth;
		else
			WLeft = CenterX - WWidth / 2;

		if(CenterY - WHeight / 2 < MTop)
			WTop = MTop;
		else if(CenterY + WHeight / 2 > MTop + MHeight)
			WTop = MTop + MHeight - WHeight;
		else
			WTop = CenterY - WHeight / 2;
	}
	else
	{
		switch(PositionMode)
		{
			case 1:
				WLeft = MLeft + MWidth / 2 - WWidth / 2;
				WTop = MTop + MHeight / 2 - WHeight / 2;
				SetCenter(WLeft + WWidth / 2, WTop + WHeight / 2);
				break;

			case 2:
				WLeft = MLeft;
				WTop = MTop;
				SetCenter(WLeft + WWidth / 2, WTop + WHeight / 2);
				break;

			case 3:
				WLeft = MLeft + MWidth - WWidth;
				WTop = MTop;
				SetCenter(WLeft + WWidth / 2, WTop + (WHeight + 1) / 2);
				break;

			case 4:
				WLeft = MLeft;
				WTop = MTop + MHeight - WHeight;
				SetCenter(WLeft + (WWidth + 1) / 2, WTop + WHeight / 2);
				break;

			case 5:
				WLeft = MLeft + MWidth - WWidth;
				WTop = MTop + MHeight - WHeight;
				SetCenter(WLeft + (WWidth + 1) / 2, WTop + (WHeight + 1) / 2);
				break;
		}
	}
	return(true);
}

bool __fastcall TMainForm::ZoomImage(double NewPercentage)
{
	Percentage = NewPercentage;
	FixViewOut();
	SetWindowSize(Susie->Width * Percentage, Susie->Height * Percentage, false);
	CreateDBImage();
	SetConstantSize();
	return(true);
}

bool __fastcall TMainForm::ZoomOrgPositionDelta(double Delta, int X, int Y)
{
	double NewPercentage = Percentage * pow(2, ((double)Delta / WheelSensitivity / 2));
	return(ZoomOrgPosition(NewPercentage, X, Y));
}

bool __fastcall TMainForm::ZoomOrgPosition(double NewPercentage, int X, int Y)
{
	double dx, dy;

	dx = (double)(X - WLeft - WWidth / 2) / Percentage;
	dy = (double)(Y - WTop - WHeight / 2) / Percentage;

	Percentage = NewPercentage;

	dx = (double)(X - CenterX) / Percentage - dx;
	dy = (double)(Y - CenterY) / Percentage - dy;

	CenterX += dx * Percentage;
	CenterY += dy * Percentage;

	FixViewOut();
	SetWindowSize(Susie->Width * Percentage, Susie->Height * Percentage, false);
	CreateDBImage();
	SetConstantSize();
	return(true);
}

bool __fastcall TMainForm::SetPositionMode(int NewPositionMode)
{
	if(PositionMode == NewPositionMode)return(true);

	PositionMode = NewPositionMode;
	PositionModes[PositionMode]->Checked = true;

	MoveByPositionMode();
	SyncWindow(false);
	return(true);
}


bool __fastcall TMainForm::SetDisableFitMode(int aMode)
{
	aMode = 0xff - aMode;
	if(aMode & 1)
	{
		if(FixPercentage == true)FixPercentage = false;
		MnFixPercentage->Checked = FixPercentage;
	}

	if(aMode & 2)
	{
		if(FitToScreen == true)FitToScreen = false;
		MnFitToScreen->Checked = FitToScreen;
	}

	if(aMode & 4)
	{
		if(ShowConstant == true)ShowConstant = false;
		MnConstantSize->Checked = ShowConstant;
	}

	if(aMode & 8)
	{
		if(UseWholeScreen == true)UseWholeScreen = false;
		MnUseWholeScreen->Checked = UseWholeScreen;
	}
	return(true);
}

bool __fastcall TMainForm::SetConstantSize(void)
{
	ConstantSize = sqrt((double)WWidth * WWidth + (double)WHeight * WHeight);
	return(true);
}

bool __fastcall TMainForm::ShowOffsetImage(int Ofs)
{
	if(AutoLoadFileFolder == true && ShowingList == false && DisplayLists->Count == 1 && InArchive == false)
	{
		UnicodeString FolderName = ExtractFileDir(OpeningFileName);
		if(FolderName == L"")return(true);

		TStringList *TempSL = new TStringList();;
		TempSL->Add(FolderName);
		OpenFiles(TempSL, OpeningFileName, Ofs, false);
		delete TempSL;
	}
	else if(ShowingList == false && DisplayLists->Count > 0)
	{
		ShowIndex += Ofs;

		if(InArchive && (ShowIndex < 0 || ShowIndex >= DisplayLists->Count))
	    JumpBorderArcNml(Ofs);

		while(ShowIndex < 0)
			ShowIndex += DisplayLists->Count;

		while(ShowIndex >= DisplayLists->Count)
			ShowIndex -= DisplayLists->Count;

		ShowAbsoluteImage(ShowIndex, Ofs);
	}
	return(true);
}

int __fastcall TMainForm::ShowAbsoluteImage(int Index, int Ofs)
{
	return(ShowAbsoluteImage(Index, -1, Ofs, true));
}

int __fastcall TMainForm::ShowAbsoluteImage(int Index, int Ofs, bool MustShowImage)
{
	return(ShowAbsoluteImage(Index, -1, Ofs, MustShowImage));
}

#define MIV_SI_OK 0
#define MIV_SI_OPENARCHIVE 1
#define MIV_SI_NOMASKORFILE 2
#define MIV_SI_NOARCHIVE 3
#define MIV_SI_RETRY 4

int __fastcall TMainForm::ShowAbsoluteImage(int Index, int SubIndex, int Ofs, bool MustShowImage)
{
	ShowIndex = Index;
	int Result;

	SSTimer->Enabled = false;

	while(DisplayLists->Count > 0)
	{
		Result = LoadFile(Index, SubIndex, Ofs, MustShowImage);

		if(InArchive == false)
			ShowIndexBack = -1;

		switch(Result)
		{
// MIV_SI_OK:正常終了
			case MIV_SI_OK:
				CheckRotateCheck();
				SSTimer->Enabled = SlideShow;
				if(InArchive == false)
				{
					OpeningFileName = DisplayLists->Strings[ShowIndex];
//					FileChanged->SetCheckFile(OpeningFileName);
				}
				return(0);
// MIV_SI_OPENARCHIVE:アーカイブファイルを開いた
			case MIV_SI_OPENARCHIVE:
				if(MustShowImage)
				{
					SetNewImageSize();
//					Hint = GetShortFileName(DisplayLists->Strings[ShowIndex]);
				}
				SSTimer->Enabled = SlideShow;
				return(1);

// MIV_SI_NOARCHIVE:アーカイブファイルがみつからない
			case MIV_SI_NOARCHIVE:
				if(InArchive == true)CloseArchiveMode();

// MIV_SI_NOMASKORFILE:マスクに該当しない
			case MIV_SI_NOMASKORFILE:
				delete (SImageInfo *)DisplayLists->Objects[ShowIndex];

				DisplayLists->Delete(ShowIndex);

				if(Ofs >= 0)
				{
					if(ShowIndex >= DisplayLists->Count)
					{
						if(InArchive == true)
							JumpBorderArcNml(Ofs);
						else
							ShowIndex = 0;
					}
				}
				else
				{
					ShowIndex--;
					if(ShowIndex < 0)
					{
						if(InArchive == true)
							ShowIndex = DisplayLists->Count - 1;
						else
							JumpBorderArcNml(Ofs);
					}
				}
				break;
// MIV_SI_RETRY:もう一度表示にトライする
			case MIV_SI_RETRY:
				continue;

		}
	}

	ShowIndex = -1;
//	Hint = L"";
	Susie->Clear(MMODE_ALL);
	Paint();

	if(Result == MIV_SI_OK)CheckRotateCheck();

	SSTimer->Enabled = SlideShow;
	OpeningFileName = L"";
	return(2);
}

int __fastcall TMainForm::LoadFile(int Index, int SubIndex, int Ofs, bool MustShowImage)
{
	int Result;
	if(ShowIndex < 0)ShowIndex = 0;
	if(ShowIndex >= DisplayLists->Count)ShowIndex = DisplayLists->Count - 1;

	Susie->Rotate = &((SImageInfo *)DisplayLists->Objects[ShowIndex])->Rotate;

	Result = OpenFile(DisplayLists->Strings[ShowIndex]);

	if(Result == 0)return(MIV_SI_NOMASKORFILE);
	if(Result == -1)return(MIV_SI_NOARCHIVE);

	if((Susie->Mode & MMODE_ARCHIVE) != 0 && InArchive == false)
	{
		if(OpenArchiveMode(DisplayLists->Strings[ShowIndex], SubIndex, Ofs, MustShowImage) == false)
		{
			if(AdjustShowIndex(Ofs) == false)return(MIV_SI_RETRY);
			return(MIV_SI_NOMASKORFILE);
		}
		return(MIV_SI_OPENARCHIVE);
	}
	else
	{
		if(InArchive == false)
		{
			TSearchRec sr;
			if(FindFirst(DisplayLists->Strings[ShowIndex], faAnyFile, sr))
			{
				SImageInfo *TempII = (SImageInfo *)DisplayLists->Objects[ShowIndex];
				TempII->Timestamp = sr.Time;
				TempII->FileSize = sr.Size;
				FindClose(sr);
			}
		}
	}

	SetNewImageSize();
//	Hint = GetShortFileName(DisplayLists->Strings[ShowIndex]);
	return(MIV_SI_OK);
}

bool __fastcall TMainForm::AdjustShowIndex(int Ofs)
{
	ShowIndex += Ofs;
	if(DisplayLists->Count == 0)
	{
		ShowIndex = -1;
		return(false);
	}
	while(ShowIndex < 0)ShowIndex += DisplayLists->Count;
	while(ShowIndex >= DisplayLists->Count)ShowIndex -= DisplayLists->Count;
	return(true);
}

bool __fastcall TMainForm::JumpBorderArcNml(int Ofs)
{
	if(InArchive && (ShowIndex < 0 || ShowIndex >= DisplayLists->Count))
	{
		CloseArchiveMode();

		if(DisplayLists->Count == 0)
		{
			ShowIndex = -1;
		}
		else
		{
			ShowIndex += Ofs;

			while(ShowIndex < 0)ShowIndex += DisplayLists->Count;
			while(ShowIndex >= DisplayLists->Count)ShowIndex -= DisplayLists->Count;
		}
	}
	return(true);
}

void __fastcall TMainForm::RefreshWindowPosition(int X, int Y)
{
	TRect MRect;

	MRect = Monitor->WorkareaRect;
	MRect.Left = MRect.Left + FrameWidth;
	MRect.Right = MRect.Right - FrameWidth;
	MRect.Top = MRect.Top + FrameWidth;
	MRect.Bottom = MRect.Bottom - FrameWidth;

	if(FitToScreen == true)
	{
		if((FitMode & 2) == 0)
		{
			WLeft = X;
			if(WLeft < MRect.Left)WLeft = MRect.Left;
			if(WLeft + WWidth > MRect.Right)WLeft = MRect.Right - WWidth;
		}

		if((FitMode & 1) == 0)
		{
			WTop = Y;
			if(WTop < MRect.Top)WTop = MRect.Top;
			if(WTop + WHeight > MRect.Bottom)WTop = MRect.Bottom - WHeight;
		}
	}
	else if(UseWholeScreen == true)
	{
		if((FitMode & 2) == 0)
		{
			WLeft = X;
			if(WLeft > MRect.Left)WLeft = MRect.Left;
			if(WLeft + WWidth < MRect.Right)WLeft = MRect.Right - WWidth;
		}
		if((FitMode & 1) == 0)
		{
			WTop = Y;
			if(WTop > MRect.Top)WTop = MRect.Top;
			if(WTop + WHeight < MRect.Bottom)WTop = MRect.Bottom - WHeight;
		}
	}
	else
	{
		WLeft = X;
		WTop = Y;
	}
}

bool __fastcall TMainForm::SyncWindow(bool FeedBack)
{
	if(Left == WLeft - FrameWidth && Top == WTop - FrameWidth
	&& Width == WWidth + FrameWidth * 2 && Height == WHeight + FrameWidth * 2)return(false);

	if(WWidth < 1)WWidth = 1;
	if(WHeight < 1)WHeight = 1;

	if(FullScreen == false)
	{
		BeginUpdate();

		Left = WLeft - FrameWidth;
		Top = WTop - FrameWidth;
		Width = WWidth + FrameWidth * 2;
		Height = WHeight + FrameWidth * 2;

		if(FeedBack == true)
		{
			WLeft = Left + FrameWidth;
			WTop = Top + FrameWidth;
			WWidth = Width - FrameWidth * 2;
			WHeight = Height - FrameWidth * 2;
		}
		EndUpdate();

		Paint();
	}
	else
	{
		Paint();
	}

	Percentage = (double)WWidth / Susie->Width;
	return(true);
}

//RECT RedrawRect;
void __fastcall TMainForm::BeginUpdate(void)
{
/*
  	SendMessage(Handle, WM_SETREDRAW, 0, 0);
  	RedrawRect.left = Left;
  	RedrawRect.top = Top;
		RedrawRect.right = Left + Width;
  	RedrawRect.bottom = Top + Height;

*/	EnableDraw = false;
}

void __fastcall TMainForm::EndUpdate(void)
{
	EnableDraw = true;
//	SendMessage(Handle, WM_SETREDRAW, 1, 0);

//	HDC DDC = GetDC(NULL);
//	Invalidate();
//	Application->ProcessMessages();
//	InvalidateRect(GetDesktopWindow(), &RedrawRect, FALSE);
//	::InvalidateRect(, &RedrawRect, TRUE);
//	ValidateRect(NULL, );

//	Paint();
//	UpdateWindow(Handle);
//RedrawWindow(GetDesktopWindow(), NULL, NULL, RDW_INVALIDATE | RDW_INTERNALPAINT | RDW_UPDATENOW | RDW_ERASE);
//	DeleteDC(DDC);

}


bool __fastcall TMainForm::GetMonitorParameter(void)
{
	int MaxWidth, MaxHeight;
	if(FullScreen == false)
	{
		MLeft = Monitor->WorkareaRect.Left + FrameWidth;
		MTop = Monitor->WorkareaRect.Top + FrameWidth;
		MWidth = Monitor->WorkareaRect.Width() - FrameWidth * 2;// * 9 / 10;
		MHeight = Monitor->WorkareaRect.Height() - FrameWidth * 2;// * 9 / 10;
	}
	else
	{
		MLeft = Monitor->Left + FrameWidth;
		MTop = Monitor->Top + FrameWidth;
		MWidth = Monitor->Width - FrameWidth * 2; // WorkAreaWidth// * 9 / 10;
		MHeight = Monitor->Height - FrameWidth * 2; // WorkAreaHeight;// * 9 / 10;
	}
	return(true);
}

int __fastcall TMainForm::GetCursorPositionType(POINT &ScCurPos)
{
	if(PreCurTimerPos.x == ScCurPos.x
  && PreCurTimerPos.y == ScCurPos.y
	&& Screen->Cursor == (TCursor)3)return(0);

	if(FitMode != 0 // ShowingList ||
	|| ScCurPos.x < WLeft - FrameWidth || ScCurPos.x > WLeft + WWidth + FrameWidth
	|| ScCurPos.y < WTop - FrameWidth  || ScCurPos.y > WTop + WHeight + FrameWidth)
	{
		SetCursorMode(0);
		return(0);
	}

	int Result = 0, SD = 16;

	if(Holding < 2)
	{
		if(WWidth < 64 || WHeight < 64)
		{
			if(WWidth > WHeight)
				SD = WHeight / 4;
			else
				SD = WWidth / 4;
			if(SD < 4)SD = 4;
		}

		if(ScCurPos.y >= WTop - FrameWidth
		&& ScCurPos.y <= WTop + SD)
		{
				if(ScCurPos.x >= WLeft - FrameWidth
				&& ScCurPos.x <= WLeft + SD)Result = 1;
			else
				if(ScCurPos.x >= WLeft + WWidth - SD
				&& ScCurPos.x <= WLeft + WWidth + FrameWidth)Result = 2;
			else
				Result = 5;
		}
		else if(ScCurPos.y >= WTop + WHeight - SD
				 && ScCurPos.y <= WTop + WHeight + FrameWidth)
		{
				if(ScCurPos.x >= WLeft - FrameWidth
				&& ScCurPos.x <= WLeft + SD)Result = 3;
			else
				if(ScCurPos.x >= WLeft + WWidth - SD
				&& ScCurPos.x <= WLeft + WWidth + FrameWidth)Result = 4;
			else
				Result = 8;
		}
		else if(ScCurPos.x >= WLeft - FrameWidth
				 && ScCurPos.x <= WLeft + SD)Result = 6;
		else if(ScCurPos.x >= WLeft + WWidth - SD
				 && ScCurPos.x <= WLeft + WWidth + FrameWidth)Result = 7;
	}

	SetCursorMode(Result);
	return(Result);
}

bool __fastcall TMainForm::SetCursorMode(int Result)
{
	if(SSChangeImage)return(true);
	if(Locked)
	{
		if(Result == -1)
			Screen->Cursor = (TCursor)3;
		else
			Screen->Cursor = crDefault;
	}
	else
	{
		switch(Result)
		{
			case -1:
				if(ShowingList)
					Screen->Cursor = crDefault;
				else
					Screen->Cursor = (TCursor)3;
				break;
			case 0:
				if(ShowingList)
					Screen->Cursor = crDefault;
				else if(Holding == 0)
					Screen->Cursor = (TCursor)1;
				else
					Screen->Cursor = (TCursor)2;
				break;
			case 2:
			case 3:
				Screen->Cursor = crSizeNESW;
				break;
			case 1:
			case 4:
				Screen->Cursor = crSizeNWSE;
				break;
			case 5:
			case 8:
				Screen->Cursor = crSizeNS;
				break;
			case 6:
			case 7:
				Screen->Cursor = crSizeWE;
				break;
		}
	}
	return(true);
}

void __fastcall TMainForm::DoMouseUp(void)
{
	SetCenter(WLeft + WWidth / 2, WTop + WHeight / 2);

	Holding = 0;
	PositionType = 0;

	POINT ScCurPos;
	GetCursorPos(&ScCurPos);
	PositionType = GetCursorPositionType(ScCurPos);
}

bool __fastcall TMainForm::SetCenter(int iX, int iY)
{
	CenterX = iX;
	CenterY = iY;

	if(FullScreen == false && FitMode == 0)
	{
		Monitor = Screen->MonitorFromPoint(Point(iX, iY), mdNearest);
		GetMonitorParameter();
	}
	return(true);
}

bool __fastcall TMainForm::GoToFullScreen(bool aMode)
{
	Monitor = Screen->MonitorFromPoint(Point(CenterX, CenterY), mdNearest);
	GetMonitorParameter();
	if(aMode == true)
	{
		Left = Monitor->Left;
		Top = Monitor->Top;
		Width = Monitor->Width;
		Height = Monitor->Height;
		GetMonitorParameter();
		SetAbsoluteForegroundWindow(Handle);
	}
	else
	{
		Left = WLeft - FrameWidth;
		Top = WTop - FrameWidth;
		Width = WWidth + FrameWidth * 2;
		Height = WHeight + FrameWidth * 2;
		GetMonitorParameter();
	}
	if(FitMode)
		SetNewImageSize();
	return(true);
}

bool __fastcall TMainForm::FixViewOut(void)
{
	if(FitToScreen)MnFitToScreenClick(NULL);
	if(UseWholeScreen)MnUseWholeScreenClick(NULL);
	return(true);
}


bool __fastcall TMainForm::SetInstanceMode(UnicodeString NewIniParamName)
{
	bool NewMode = (NewIniParamName != L"milligram");
	IniParamName = NewIniParamName;

	InstanceMode = NewMode;
	MnDeleteInstance->Enabled = InstanceMode;

	if(NewMode)
	{
		TrayIcon->BalloonTitle = L"milligram - " + IniParamName;
		TrayIcon->Hint = L"milligram - " + IniParamName;
	}
	else
	{
		TrayIcon->BalloonTitle = L"milligram image viewer";
		TrayIcon->Hint = L"milligram image viewer";
	}

	return(true);
}

void __fastcall TMainForm::FormPaint(TObject *Sender)
{
	if(EnableDraw == false)return;

	if(Susie->Mode)
	{
		if(GIFRefresh == true)
		{
			GIFRefresh = false;
			PreTGT = ClockTGT;
			Susie->AnimateGIF();
		}

		// 画像表示
		int OldStretchMode = SetStretchBltMode(Canvas->Handle, HALFTONE);
		if(FullScreen == false)
		{
			StretchDIBits(Canvas->Handle,
										FrameWidth, FrameWidth, WWidth, WHeight,
										0, 0, Susie->DWidth, Susie->DHeight,
										Susie->pBitmapData, Susie->pBitmapInfo,
										DIB_RGB_COLORS, SRCCOPY);

			if(FrameWidth > 0)
			{
				Canvas->Brush->Color = FrameFillColor;
				Canvas->FillRect(TRect(0, 0, Width, FrameWidth)); //上
				Canvas->FillRect(TRect(0, FrameWidth + WHeight, Width, Height)); // 下
				Canvas->FillRect(TRect(0, FrameWidth, FrameWidth, FrameWidth + WHeight)); // 左
				Canvas->FillRect(TRect(FrameWidth + WWidth, FrameWidth, Width, FrameWidth + WHeight)); // 右
			}
		}
		else
		{
			int TLeft = WLeft - Monitor->Left, TTop = WTop - Monitor->Top, TWidth = WWidth, THeight = WHeight;
			StretchDIBits(Canvas->Handle,
										TLeft, TTop, TWidth, THeight,
										0, 0, Susie->DWidth, Susie->DHeight,
										Susie->pBitmapData, Susie->pBitmapInfo,
										DIB_RGB_COLORS, SRCCOPY);

			if(FrameWidth > 0)
			{
				TLeft -= FrameWidth, TTop -= FrameWidth, TWidth += FrameWidth * 2, THeight += FrameWidth * 2;
				Canvas->Brush->Color = FrameFillColor;
				if(TTop + FrameWidth > 0)Canvas->FillRect(TRect(TLeft, TTop, TLeft + TWidth, TTop + FrameWidth)); //上
				if(TTop + THeight - FrameWidth < Height)Canvas->FillRect(TRect(TLeft, TTop + THeight - FrameWidth, TLeft + TWidth, TTop + THeight)); // 下
				if(TLeft + FrameWidth > 0)Canvas->FillRect(TRect(TLeft, TTop + FrameWidth, TLeft + FrameWidth, TTop + THeight - FrameWidth)); // 左
				if(TLeft + TWidth - FrameWidth < Width)Canvas->FillRect(TRect(TLeft + TWidth - FrameWidth, TTop + FrameWidth, TLeft + TWidth, TTop + THeight - FrameWidth)); // 右
			}

			Canvas->Brush->Color = FullFillColor;
			if(TTop > 0)Canvas->FillRect(TRect(0, 0, Width, TTop)); //上
			if(TTop + THeight < Height)Canvas->FillRect(TRect(0, TTop + THeight, Width, Height)); // 下
			if(TLeft > 0)Canvas->FillRect(TRect(0, TTop, TLeft, TTop + THeight)); // 左
			if(TLeft + TWidth < Width)Canvas->FillRect(TRect(TLeft + TWidth, TTop, Width, TTop + THeight)); // 右
		}

		SetStretchBltMode(Canvas->Handle, OldStretchMode);
	}
	else
	{
		if(FrameWidth == 0)
		{
			Canvas->Pen->Style = psSolid;
			Canvas->Pen->Color = FrameColor;
			Canvas->Brush->Color = FullFillColor;

			if(FullScreen == false)
			{
				Canvas->Rectangle(0, 0, WWidth, WHeight);
			}
			else
			{
				int TLeft = WLeft - Monitor->Left, TTop = WTop - Monitor->Top;
				Canvas->Rectangle(TLeft, TTop, TLeft + WWidth, TTop + WHeight);
				if(TTop > 0)Canvas->FillRect(TRect(0, 0, Width, TTop)); //上
				if(TTop + WHeight < Height)Canvas->FillRect(TRect(0, TTop + WHeight, Width, Height)); // 下
				if(TLeft > 0)Canvas->FillRect(TRect(0, TTop, TLeft, TTop + WHeight)); // 左
				if(TLeft + WWidth < Width)Canvas->FillRect(TRect(TLeft + WWidth, TTop, Width, TTop + WHeight)); // 右
			}
		}
		else
		{
			if(FullScreen == false)
			{
				Canvas->Brush->Color = FullFillColor;
				Canvas->FillRect(TRect(FrameWidth, FrameWidth, Width - FrameWidth, Height - FrameWidth)); //中

				Canvas->Brush->Color = FrameFillColor;
				Canvas->FillRect(TRect(0, 0, Width, FrameWidth)); //上
				Canvas->FillRect(TRect(0, FrameWidth + WHeight, Width, Height)); // 下
				Canvas->FillRect(TRect(0, FrameWidth, FrameWidth, FrameWidth + WHeight)); // 左
				Canvas->FillRect(TRect(FrameWidth + WWidth, FrameWidth, Width, FrameWidth + WHeight)); // 右
			}
			else
			{
				int TLeft = WLeft - Monitor->Left, TTop = WTop - Monitor->Top, TWidth = WWidth, THeight = WHeight;

				Canvas->Brush->Color = FullFillColor;
				Canvas->FillRect(TRect(TLeft, TTop, TLeft + TWidth, TTop + THeight)); //中

				TLeft -= FrameWidth, TTop -= FrameWidth, TWidth += FrameWidth * 2, THeight += FrameWidth * 2;
				Canvas->Brush->Color = FrameFillColor;
				if(TTop + FrameWidth > 0)Canvas->FillRect(TRect(TLeft, TTop, TLeft + TWidth, TTop + FrameWidth)); //上
				if(TTop + THeight - FrameWidth < Height)Canvas->FillRect(TRect(TLeft, TTop + THeight - FrameWidth, TLeft + TWidth, TTop + THeight)); // 下
				if(TLeft + FrameWidth > 0)Canvas->FillRect(TRect(TLeft, TTop + FrameWidth, TLeft + FrameWidth, TTop + THeight - FrameWidth)); // 左
				if(TLeft + TWidth - FrameWidth < Width)Canvas->FillRect(TRect(TLeft + TWidth - FrameWidth, TTop + FrameWidth, TLeft + TWidth, TTop + THeight - FrameWidth)); // 右

				Canvas->Brush->Color = FullFillColor;
				if(TTop > 0)Canvas->FillRect(TRect(0, 0, Width, TTop)); //上
				if(TTop + THeight < Height)Canvas->FillRect(TRect(0, TTop + THeight, Width, Height)); // 下
				if(TLeft > 0)Canvas->FillRect(TRect(0, TTop, TLeft, TTop + THeight)); // 左
				if(TLeft + TWidth < Width)Canvas->FillRect(TRect(TLeft + TWidth, TTop, Width, TTop + THeight)); // 右
			}
    }
	}
	if(SSIcon >= 0)DrawSSIcon();
}

TColor __fastcall TMainForm::GetFrameColor(void)
{
	int R, G, B, M;
	B  = ((int)FullFillColor & 0xFF0000) >> 16;
	G  = ((int)FullFillColor & 0x00FF00) >>  8;
	R  = ((int)FullFillColor & 0x0000FF)      ;
	M = R; if(G * 3 / 2 > M)M = G * 3 / 2; if(B / 2 > M)M = B / 2;
	if(M < 128)
	{
		R += 96; if(R > 255)R = 255;
		G += 96; if(G > 255)G = 255;
		B += 96; if(B > 255)B = 255;
	}
	else
	{
		R -= 96; if(R < 0)R = 0;
		G -= 96; if(G < 0)G = 0;
		B -= 96; if(B < 0)B = 0;
	}

	return((TColor)((B << 16) | (G << 8) | R));
}


//---------------------------------------------------------------------------


void __fastcall TMainForm::FormDblClick(TObject *Sender)
{
	if(Locked)return;
	ToggleShowList(1);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)

{
	// 18 37 戻る
	// 18 39 進む
	if(ProgressForm->Showing)return;

	static int mbMode = false;
	if(ShowingList)
	{
		DisplayBoxKeyDown(Sender, Key, Shift);
		return;
	}

	if(Locked)return;

	int SubKey = 0;

	if((GetAsyncKeyState(VK_LSHIFT) & 0x8000) || (GetAsyncKeyState(VK_RSHIFT) & 0x8000))SubKey |= 1;
	if((GetAsyncKeyState(VK_LCONTROL) & 0x8000) || (GetAsyncKeyState(VK_RCONTROL) & 0x8000))SubKey |= 2;
	if((GetAsyncKeyState(VK_LMENU) & 0x8000) || (GetAsyncKeyState(VK_RMENU) & 0x8000))SubKey |= 4;

	if(mbMode)
	{
		switch(Key)
		{
			case 37:
				ShowOffsetImage(-1);
				break;

			case 39:
				ShowOffsetImage(1);
				break;
		}
		mbMode = false;
		return;
	}

	if(Key == 18)
	{
		mbMode = true;
		return;
	}

	switch(Key)
	{
		case VK_SPACE:
			if(SubKey == 0)
				ShowOffsetImage(1);
			else if(SubKey == 1)
				ShowOffsetImage(-1);
			else if(SubKey == 2)
				MnShowClick(NULL);
			break;

		case VK_DOWN:
			ShowOffsetImage(1);
			break;
		case VK_BACK:
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
			ShowAbsoluteImage(DisplayLists->Count - 1, -1);
			break;

		case VK_RETURN:
			ToggleShowList(1);
			break;

		case VK_DELETE:
			if(SubKey == 0)
				DeleteFileLists(0);
			else if(SubKey == 2)
				DeleteFileLists(1);
			else if(SubKey == 3)
				DeleteFileLists(2);
			break;

		case VK_F5:
			ShowAbsoluteImage(ShowIndex, 0);
			break;

		case 187:
		case VK_ADD:
			ZoomImage(Percentage * 1.5);
			break;
		case 189:
		case VK_SUBTRACT:
			ZoomImage(Percentage / 1.5);
			break;
		case VK_MULTIPLY:
		case 220:
			MnInScreenClick(NULL);
			break;

		case L'O':
			MnOpenFileClick(NULL);
			break;

		case L'N':
			if(SubKey == 2)
				MnRenameClick(NULL);
			break;

		case 191:
		case VK_DIVIDE:
		case L'A':
			MnViewExactrySizeClick(NULL);
			break;

		case L'S':
			if(SubKey == 0)
				MnInScreenClick(NULL);
			else if(SubKey == 2)
				MnSaveListsClick(NULL);
			else if(SubKey == 1)
				MnJpegSaveClick(NULL);
			else if(SubKey == 3)
				MnJpegSizeSaveClick(NULL);
			break;

		case L'C':
			if(SubKey == 0)
				MnMoveCenterClick(NULL);
			else if(SubKey == 1)
				MnCopyImageClick(NULL);
			else if(SubKey == 2)
				MnExplorerCopyClick(NULL);
			else if(SubKey == 3)
				MnCopyFilePathClick(NULL);
			break;

		case L'T':
			if(SubKey == 0)
				MnAlwaysTopClick(NULL);
			else if(SubKey == 2)
				MnSortByNameClick(NULL);
			break;

		case L'B':
			MnFixPercentageClick(NULL);
			break;

		case L'W':
			MnFitToScreenClick(NULL);
			break;
		case L'K':
			MnConstantSizeClick(Sender);
			break;
		case L'H':
			MnUseWholeScreenClick(NULL);
			break;
		case L'F':
			MnFullScreenClick(NULL);
			break;

		case L'D':
			MnSlideShowClick(NULL);
			break;
		case L'R':
			if(SubKey == 0)
				MnReloadClick(NULL);
			else if(SubKey == 2)
				MnSortRandomClick(NULL);
			break;

		case L'X':
			if(SubKey == 0)
				MnCloseArchiveClick(NULL);
			else if(SubKey == 2)
				MnExplorerCutClick(NULL);
			break;

		case L'V':
			if(SubKey == 2)
				MnExplorerPasteClick(NULL);
			break;

		case L'I':
			MnShowInformationBalloonClick(NULL);
			break;

		case L'P':
			if(SubKey == 2)
				MnOpenFileFolderClick(NULL);
			else
				MnOpenFolderClick(NULL);
			break;

		case L'G':
			if(SubKey == 2)
				MnSortByTimeStampClick(NULL);
			break;

		case L'E':
			if(SubKey == 2)
				MnSortByFileExtClick(NULL);
			break;

		case L'Y':
			if(SubKey == 2)
				MnSortByFileSizeClick(NULL);
			break;

		case L'U':
			if(SubKey == 2)
				MnSortReverseClick(NULL);
			break;

		case 'M':
			MnFileMoveClick(NULL);
			break;

		case VK_OEM_COMMA:
			Susie->OffsetRotate(-1);
			CheckRotateCheck();
			SetRotateImageSize();
			break;

		case VK_OEM_PERIOD:
			Susie->OffsetRotate(1);
			CheckRotateCheck();
			SetRotateImageSize();
			break;

		case VK_ESCAPE:
			Close();
			break;
	}
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::FormMouseEnter(TObject *Sender)
{
	StopCount = 0;
	CursorTimer->Enabled = true;
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::FormMouseLeave(TObject *Sender)
{
	Screen->Cursor = crDefault;
	Holding = 0;
	PositionType = 0;
	StopCount = 0;
	CursorTimer->Enabled = false;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::FormMouseMove(TObject *Sender, TShiftState Shift,
					int X, int Y)
{

	StopCount = 0;
	POINT ScCurPos, ClCurPos;
	GetCursorPos(&ScCurPos);

	if(Locked)
	{
		PositionType = GetCursorPositionType(ScCurPos);
		return;
	}

	if(Holding == 1 || Holding == 3)
	{
		double NextWidth, NextHeight, Temp;

		if(PositionType == 0)
		{
			RefreshWindowPosition(ScCurPos.x - OfsCurPos.x, ScCurPos.y - OfsCurPos.y);
		}
		else
		{
			if((GetAsyncKeyState(VK_LCONTROL) & 0x8000) || (GetAsyncKeyState(VK_RCONTROL) & 0x8000))
			{
				if(PositionType <= 4)
				{
					switch(PositionType)
					{
						case 1:
							NextWidth  = (double)DownWidth + (DownCurPos.x - ScCurPos.x) * 2;
							NextHeight = (double)DownHeight + (DownCurPos.y - ScCurPos.y) * 2;
							break;
						case 2:
							NextWidth  = (double)DownWidth - (DownCurPos.x - ScCurPos.x) * 2;
							NextHeight = (double)DownHeight + (DownCurPos.y - ScCurPos.y) * 2;
							break;
						case 3:
							NextWidth  = (double)DownWidth + (DownCurPos.x - ScCurPos.x) * 2;
							NextHeight = (double)DownHeight - (DownCurPos.y - ScCurPos.y) * 2;
							break;
						case 4:
							NextWidth  = (double)DownWidth - (DownCurPos.x - ScCurPos.x) * 2;
							NextHeight = (double)DownHeight - (DownCurPos.y - ScCurPos.y) * 2;
							break;
					}
					if(NextWidth > NextHeight * Ratio)
						NextHeight = NextWidth / Ratio;
					else
						NextWidth = NextHeight * Ratio;
				}
				else
				{
					switch(PositionType)
					{
						case 5:
							NextHeight = (double)DownHeight + (DownCurPos.y - ScCurPos.y) * 2;
							NextWidth = NextHeight * Ratio;
							break;
						case 6:
							NextWidth  = (double)DownWidth + (DownCurPos.x - ScCurPos.x) * 2;
							NextHeight = NextWidth / Ratio;
							break;
						case 7:
							NextWidth  = (double)DownWidth - (DownCurPos.x - ScCurPos.x) * 2;
							NextHeight = NextWidth / Ratio;
							break;
						case 8:
							NextHeight = (double)DownHeight - (DownCurPos.y - ScCurPos.y) * 2;
							NextWidth = NextHeight * Ratio;
							break;
						}
				}

				WLeft = CenterX + (int)(DownWidth - NextWidth) / 2 - DownWidth / 2;
				WTop = CenterY + (int)(DownHeight - NextHeight) / 2 - DownHeight / 2;
			}
			else
			{
				switch(PositionType)
				{
					case 1:
						NextWidth  = (double)DownWidth + DownCurPos.x - ScCurPos.x;
						NextHeight = (double)DownHeight + DownCurPos.y - ScCurPos.y;

						if(NextWidth > NextHeight * Ratio)
							NextHeight = NextWidth / Ratio;
						else
							NextWidth = NextHeight * Ratio;

						WLeft = CenterX + (int)(DownWidth - NextWidth) - DownWidth / 2;
						WTop = CenterY + (int)(DownHeight - NextHeight) - DownHeight / 2;
						break;

					case 2:
						NextWidth  = (double)DownWidth - DownCurPos.x + ScCurPos.x;
						NextHeight = (double)DownHeight + DownCurPos.y - ScCurPos.y;

						if(NextWidth > NextHeight * Ratio)
							NextHeight = NextWidth / Ratio;
						else
							NextWidth = NextHeight * Ratio;

						WTop = CenterY + (int)(DownHeight - NextHeight) - DownHeight / 2;
						break;

					case 3:
						NextWidth  = (double)DownWidth + DownCurPos.x - ScCurPos.x;
						NextHeight = (double)DownHeight - DownCurPos.y + ScCurPos.y;
						if(NextWidth > NextHeight * Ratio)
							NextHeight = NextWidth / Ratio;
						else
							NextWidth = NextHeight * Ratio;

						WLeft = CenterX + (int)(DownWidth - NextWidth) - DownWidth / 2;
						break;

					case 4:
						NextWidth  = (double)DownWidth - DownCurPos.x + ScCurPos.x;
						NextHeight = (double)DownHeight - DownCurPos.y + ScCurPos.y;

						if(NextWidth > NextHeight * Ratio)
							NextHeight = NextWidth / Ratio;
						else
							NextWidth = NextHeight * Ratio;

						break;

					case 5:
						NextHeight = (double)DownHeight + DownCurPos.y - ScCurPos.y;
						NextWidth = NextHeight * Ratio;

						Temp = DownWidth - (DownCurPos.x - DownLeft);
						WLeft = DownLeft + Temp - Temp * NextWidth / DownWidth;
						WTop = ScCurPos.y - OfsCurPos.y;
						break;

					case 6:
						NextWidth = (double)DownWidth + DownCurPos.x - ScCurPos.x;
						NextHeight = NextWidth / Ratio;

						Temp = DownHeight - (DownCurPos.y - DownTop);
						WTop = DownTop + Temp - Temp * NextWidth / DownWidth;
						WLeft = ScCurPos.x - OfsCurPos.x;
						break;

					case 7:
						NextWidth = (double)DownWidth - DownCurPos.x + ScCurPos.x;
						NextHeight = NextWidth / Ratio;

						Temp = DownHeight - (DownCurPos.y - DownTop);
						WTop = DownTop + Temp - Temp * NextWidth / DownWidth;
						break;

					case 8:
						NextHeight = (double)DownHeight - DownCurPos.y + ScCurPos.y;
						NextWidth = NextHeight * Ratio;

						Temp = DownWidth - (DownCurPos.x - DownLeft);
						WLeft = DownLeft + Temp - Temp * NextWidth / DownWidth;
						break;
				}
			}
			WWidth = (int)NextWidth;
			WHeight = (int)NextHeight;
		}

		if(ShowConstant == true)
			ConstantSize = sqrt(WWidth * WWidth + WHeight * WHeight);
	}
	else
	{
		PositionType = GetCursorPositionType(ScCurPos);
	}
	PreCurPos = ScCurPos;
	SyncWindow(true);
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::FormMouseDown(TObject *Sender, TMouseButton Button,
					TShiftState Shift, int X, int Y)
{
	StopCount = 0;
	if(Locked == false && Button == mbLeft)
	{
		POINT ScCurPos;
		GetCursorPos(&ScCurPos);

		if(!(Button == mbLeft// && ShowingList == true
		&& ScCurPos.x >= WLeft && ScCurPos.x < WLeft + WWidth && ScCurPos.y >= WTop && ScCurPos.y < WTop + WHeight))
		{
			PositionType = 0;
		}
		SetCenter(WLeft + WWidth / 2, WTop + WHeight / 2);

		Holding = (int)Button + 1;
		OfsCurPos.x = ScCurPos.x - WLeft;
		OfsCurPos.y = ScCurPos.y - WTop;
		DownCurPos = ScCurPos;
		PreCurPos = DownCurPos;
		DownLeft = WLeft;
		DownTop = WTop;
		DownWidth = WWidth;
		DownHeight = WHeight;
		PositionType = GetCursorPositionType(ScCurPos);
	}
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::FormMouseUp(TObject *Sender, TMouseButton Button,
					TShiftState Shift, int X, int Y)
{
	if(Locked)
	{
		if(Button == mbRight)
		{
			if(RClkCancel == true)
			{
				RClkCancel = false;
			}
			else
			{
				POINT ScCurPos;
				GetCursorPos(&ScCurPos);

				if(
				 (GetAsyncKeyState(VK_LMENU) & 0x8000)
				|| (GetAsyncKeyState(VK_RMENU) & 0x8000)
				)
				{
					if(CreateFilePPMenu() == true)
					{
						OriginName = ExtractFileName(FileLists->Strings[ShowIndex]);
						FilePPMenu->Popup(ScCurPos.x, ScCurPos.y);
					}
				}
				else
				{
					PopupMenuPopup(ScCurPos.x, ScCurPos.y);
				}
			}
		}
	}
	else
	{
		int SubKey = 0;

		if((GetAsyncKeyState(VK_LSHIFT) & 0x8000) || (GetAsyncKeyState(VK_RSHIFT) & 0x8000))SubKey |= 1;
		if((GetAsyncKeyState(VK_LCONTROL) & 0x8000) || (GetAsyncKeyState(VK_RCONTROL) & 0x8000))SubKey |= 2;
		if((GetAsyncKeyState(VK_LMENU) & 0x8000) || (GetAsyncKeyState(VK_RMENU) & 0x8000))SubKey |= 4;
		if((GetAsyncKeyState(VkLButton) & 0x8000))SubKey |= 8;
		if((GetAsyncKeyState(VkRButton) & 0x8000))SubKey |= 16;

		switch(Button)
		{
			case mbLeft:
				if(Holding)
				{
					DoMouseUp();
					CreateDBImage();
					if(SubKey & 16)
					{
						MnInScreenClick(NULL);
						RClkCancel = true;
					}
				}
				break;

			case mbRight:
				if(RClkCancel == true)
				{
					RClkCancel = false;
				}
				else if(SubKey & 8)
				{
					DoMouseUp();
					MnInScreenClick(NULL);
				}
				else
				{
					POINT ScCurPos;
					GetCursorPos(&ScCurPos);

					if(SubKey & 2)
					{
						if(CreateFilePPMenu() == true)
						{
							OriginName = ExtractFileName(OpeningFileName);
							FilePPMenu->Popup(ScCurPos.x, ScCurPos.y);
						}
					}
					else
					{
						PopupMenuPopup(ScCurPos.x, ScCurPos.y);
					}
				}
				break;

			case mbMiddle:
				if(ShowingList == false)
				{
					if(SubKey == 24 || SubKey == 3)
					{
						if(SubKey & 8)DoMouseUp();
						if(SubKey & 16)RClkCancel = true;
						FormAlphaBlendValue = 255;
						CheckAlphaValueMenuCheck();
					}
					else if(SubKey & 9)
					{
						if(SubKey & 8)DoMouseUp();
						if(RotateValue != 0)
						{
							Susie->AbsoluteRotate(0);
							CheckRotateCheck();
							SetRotateImageSize();
						}
					}
					else if(SubKey & 18)
					{
						POINT ScCurPos;
						GetCursorPos(&ScCurPos);
						if(SubKey & 16)RClkCancel = true;

						if(Percentage != 1.0)
							ZoomOrgPosition(1, ScCurPos.x, ScCurPos.y);
					}
					else
						Close();
				}
				else
					Close();
				break;
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::FormMouseWheel(TObject *Sender, TShiftState Shift,
					int WheelDelta, TPoint &MousePos, bool &Handled)
{

	if(Locked || ShowingList || ProgressForm->Showing)return;

	int i = 0;
	int SubKey = 0;

	if((GetAsyncKeyState(VK_LSHIFT) & 0x8000) || (GetAsyncKeyState(VK_RSHIFT) & 0x8000))SubKey |= 1;
	if((GetAsyncKeyState(VK_LCONTROL) & 0x8000) || (GetAsyncKeyState(VK_RCONTROL) & 0x8000))SubKey |= 2;
	if((GetAsyncKeyState(VK_LMENU) & 0x8000) || (GetAsyncKeyState(VK_RMENU) & 0x8000))SubKey |= 4;
	if((GetAsyncKeyState(VkLButton) & 0x8000))SubKey |= 8;
	if((GetAsyncKeyState(VkRButton) & 0x8000))SubKey |= 16;


	WheelPos += WheelDelta;
	if(WheelPos > WheelSensitivity)
	{
		while(WheelPos > WheelSensitivity)
		{
			WheelPos -= WheelSensitivity;
			i--;
		}
	}
	else if(WheelPos < 0)
	{
		while(WheelPos < 0)
		{
			WheelPos += WheelSensitivity;
			i++;
		}
	}

	if(SubKey == 0 && i != 0)
	{
		ShowOffsetImage(i);
	}
	else if(SubKey == 3 || SubKey == 24 && i != 0)
	{
		if(SubKey & 8)DoMouseUp();
		if(SubKey & 16)RClkCancel = true;
		AlphaValueOffset(i);
	}
	else if(SubKey & 9 && i != 0)
	{
		if(SubKey & 8)DoMouseUp();

		if(i > 0)
			Susie->OffsetRotate(1);
		else
			Susie->OffsetRotate(-1);

		CheckRotateCheck();
		SetRotateImageSize();
	}
	else if(SubKey & 18)
	{
		POINT ScCurPos;
		GetCursorPos(&ScCurPos);
		if(SubKey & 16)RClkCancel = true;
		ZoomOrgPositionDelta(WheelDelta, ScCurPos.x, ScCurPos.y);
	}
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::DisplayBoxKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)
{
	if(ShowingList == false)
	{
		FormKeyDown(Sender, Key, Shift);
		return;
	}

	if(Locked)return;

	int SubKey = 0;

	if((GetAsyncKeyState(VK_LSHIFT) & 0x8000) || (GetAsyncKeyState(VK_RSHIFT) & 0x8000))
		SubKey |= 1;

	if((GetAsyncKeyState(VK_LCONTROL) & 0x8000) || (GetAsyncKeyState(VK_RCONTROL) & 0x8000))
		SubKey |= 2;

	if((GetAsyncKeyState(VK_LMENU) & 0x8000) || (GetAsyncKeyState(VK_RMENU) & 0x8000))
		SubKey |= 4;

	switch(Key)
	{
		case VK_SPACE:
			if(SubKey == 2)
				MnShowClick(NULL);
			break;

		case VK_RETURN:
			ToggleShowList(2);
			break;

		case VK_DELETE:
			if(SubKey == 0)
				DeleteFileLists(0);
			else if(SubKey == 2)
				DeleteFileLists(1);
			else if(SubKey == 3)
				DeleteFileLists(2);
			break;

		case VK_DOWN:
			if(SubKey == 4)
				MoveSelectedList(1);
			break;

		case VK_UP:
			if(SubKey == 4)
				MoveSelectedList(-1);
			break;

		case L'O':
			MnOpenFileClick(NULL);
			break;

		case L'N':
			if(SubKey == 2)
				MnRenameClick(NULL);
			break;

		case L'A':
			if(SubKey == 2)
				DisplayBox->SelectAll();
			break;

		case L'S':
			if(SubKey == 0)
				MnInScreenClick(NULL);
			else if(SubKey == 2)
				MnSaveListsClick(NULL);
			break;

		case L'C':
			if(SubKey == 0)
				MnMoveCenterClick(NULL);
			else if(SubKey == 2)
				MnExplorerCopyClick(NULL);
			else if(SubKey == 3)
				MnCopyFilePathClick(NULL);
			break;

		case L'T':
			if(SubKey == 0)
				MnAlwaysTopClick(NULL);
			else if(SubKey == 2)
				MnSortByNameClick(NULL);
			break;

		case L'F':
			MnFullScreenClick(NULL);
			break;

		case L'X':
			if(SubKey == 0)
				MnCloseArchiveClick(NULL);
			else if(SubKey == 2)
				MnExplorerCutClick(NULL);
			break;

		case L'V':
			if(SubKey == 2)
				MnExplorerPasteClick(NULL);
			break;

		case L'I':
			if(SubKey == 2)
			{
				int i;
				for(i = 0;i < DisplayBox->Count;i++)
					DisplayBox->Selected[i] = !(DisplayBox->Selected[i]);
			}
			break;

		case L'P':
			MnOpenFolderClick(NULL);
			break;

		case L'G':
			if(SubKey == 2)
				MnSortByTimeStampClick(NULL);
			break;

		case L'E':
			if(SubKey == 2)
				MnSortByFileExtClick(NULL);
			break;

		case L'Y':
			if(SubKey == 2)
				MnSortByFileSizeClick(NULL);
			break;

		case L'U':
			if(SubKey == 2)
				MnSortReverseClick(NULL);
			break;

		case L'R':
			if(SubKey == 2)
				MnSortRandomClick(NULL);
			break;

		case VK_ESCAPE:
			Close();
			break;
	}
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::MnOpenFileClick(TObject *Sender)
{
	UnicodeString FolderName = GetImageFileFolder();
	OpenFileDialog->InitialDir = FolderName;
	if(OpenFileDialog->Execute() == false)return;
	OpenFiles(dynamic_cast<TStringList *>(OpenFileDialog->Files));

	AddHistoryList(dynamic_cast<TStringList *>(OpenFileDialog->Files));
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MnExplorerCopyClick(TObject *Sender)
{
// DROPEFFECT_COPY
	if(FileLists->Count == 0)return;

	if(InArchive == true)
	{
		MessageBeep(MB_ICONASTERISK);
		ShowMessage(LoadStr(1016));
	}

	TStringList *TempSL = new TStringList();

	TempSL->Add(OpeningFileName);
	FilesToClipboard(Handle, TempSL, DROPEFFECT_COPY);

	delete TempSL;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MnExplorerCutClick(TObject *Sender)
{
// DROPEFFECT_MOVE
	if(FileLists->Count == 0)return;

	if(InArchive == true)
	{
		MessageBeep(MB_ICONASTERISK);
		ShowMessage(LoadStr(1017));
	}

	TStringList *TempSL = new TStringList();

	TempSL->Add(OpeningFileName);
	FilesToClipboard(Handle, TempSL, DROPEFFECT_MOVE);

	delete TempSL;
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::MnExplorerPasteClick(TObject *Sender)
{
	TStringList *DropLists = new TStringList();

	FilesFromClipboard(DropLists);
	if(DropLists->Count > 0)OpenFiles(DropLists, DropLists->Strings[0], 0, true);

	delete DropLists;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MnRenameClick(TObject *Sender)
{
	UnicodeString NewFileName = ExtractFileName(GetOpeningFileName());
	UnicodeString FileExt = ExtractFileExt(NewFileName);
	NewFileName = ChangeFileExt(NewFileName, L"");

	FormStyle = fsNormal;

	if(InputQuery(LoadStr(1018), LoadStr(1019), NewFileName) == true)
	{
		int i;
		i = CheckFileIrregularChar(NewFileName);
		if(i < 0)
		{
			NewFileName = ExtractFilePath(OpeningFileName) + NewFileName + FileExt;

			if(FileExists(NewFileName) == true)
			{
				MessageBeep(MB_ICONASTERISK);
				ShowMessage(LoadStr(1020));
			}
			else
			{
				RenameFile(OpeningFileName, NewFileName);
				SetOpeningFileName(NewFileName);
				MessageBeep(MB_ICONASTERISK);
				ShowMessage(LoadStr(1021));
			}
		}
		else
		{
			MessageBeep(MB_ICONASTERISK);
			ShowMessage(NewFileName + LoadStr(1022) + NewFileName[i] + LoadStr(1023));
		}
	}

	RestoreStayOnTop();
}

//---------------------------------------------------------------------------
void __fastcall TMainForm::MnCopyFilePathClick(TObject *Sender)
{
	Clipboard()->SetTextBuf(GetImageFileName().c_str());
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MnFileMoveClick(TObject *Sender)
{
	if(CreateFilePPMenu() == true)
	{
		POINT ScCurPos;
		GetCursorPos(&ScCurPos);
		OriginName = ExtractFileName(OpeningFileName);
		FilePPMenu->Popup(ScCurPos.x, ScCurPos.y);
	}
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MnOpenFileFolderClick(TObject *Sender)
{
	if(FileLists->Count == 0)return;
	SelectFileInExplorer(OpeningFileName);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MnOpenFolderClick(TObject *Sender)
{
	UnicodeString FolderName = GetImageFileFolder();
	FolderName = GetFolderByDialog(FolderName, Handle, LoadStr(1049));
	if(FolderName == L"")return;

	AddHistoryList(FolderName);

	TStringList *TempSL = new TStringList();;
	TempSL->Add(FolderName);
	OpenFiles(TempSL);
	delete TempSL;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MnViewExactrySizeClick(TObject *Sender)
{
	FixViewOut();
	if(Percentage == 1.0)return;

	Percentage = 1;

	if(WLeft + WWidth / 2 == CenterX && CenterY == WTop + WHeight / 2)
		SetWindowSize(Susie->Width, Susie->Height, false);
	else
		SetWindowSize(Susie->Width, Susie->Height, true);

	CreateDBImage();
	SetConstantSize();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MnInScreenClick(TObject *Sender)
{
	if(UseWholeScreen)MnUseWholeScreenClick(NULL);

	double dTop, dLeft, dWidth, dHeight, pWidth, pHeight;

	dLeft = WLeft;
	dTop = WTop;
	dWidth = pWidth = WWidth;
	dHeight = pHeight = WHeight;

	if(dWidth > MWidth)dWidth = MWidth;
	if(dHeight > MHeight)dHeight = MHeight;
	if((double)dWidth > (double)dHeight * Ratio)
		dWidth = (double)dHeight * Ratio;
	else
		dHeight = (double)dWidth / Ratio;

	dLeft += (pWidth - dWidth) / 2;
	dTop += (pHeight - dHeight) / 2;

	if(dLeft < MLeft)dLeft = MLeft;
	if(dTop < MTop)dTop = MTop;
	if(dLeft + dWidth > MLeft + MWidth)dLeft = MWidth - dWidth + MLeft;
	if(dTop + dHeight > MTop + MHeight)dTop = MHeight - dHeight + MTop;

  WLeft = dLeft, WTop = dTop, WWidth = dWidth, WHeight = dHeight;
	SetCenter(dLeft + dWidth / 2, WTop + dHeight / 2);
	SyncWindow(false);
	CreateDBImage();
	SetAbsoluteForegroundWindow(Handle);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MnMoveCenterClick(TObject *Sender)
{
	WLeft = MLeft + MWidth / 2;
	WTop = MTop + MHeight / 2;
	SetCenter(MLeft + MWidth / 2, MTop + MHeight / 2);
	SetWindowSize(WWidth, WHeight, false);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MnPositionModeClick(TObject *Sender)
{
	SetPositionMode((dynamic_cast<TComponent *>(Sender))->Tag);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MnAlwaysTopClick(TObject *Sender)
{
	AlwaysTop = !AlwaysTop;
	if(AlwaysTop == true)
	{
		FormStyle = fsStayOnTop;
		MnAlwaysTop->Checked = true;
	}
	else
	{
		FormStyle = fsNormal;
		MnAlwaysTop->Checked = false;
	}
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MnFixPercentageClick(TObject *Sender)
{
	FixPercentage = !FixPercentage;
	MnFixPercentage->Checked = FixPercentage;

  SetDisableFitMode(1);

	FitMode = 0;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MnFitToScreenClick(TObject *Sender)
{
	FitToScreen = !FitToScreen;
	MnFitToScreen->Checked = FitToScreen;

	SetDisableFitMode(2);

	FitMode = 0;
	if(FitToScreen == true)
	{
		SetNewImageSize();
	}
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MnConstantSizeClick(TObject *Sender)
{
	ShowConstant = !ShowConstant;
	MnConstantSize->Checked = ShowConstant;

	SetDisableFitMode(4);

	FitMode = 0;
	if(ShowConstant == true)
	{
		if(Sender != NULL)SetConstantSize();
		SetNewImageSize();
	}
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::MnUseWholeScreenClick(TObject *Sender)
{
	UseWholeScreen = !UseWholeScreen;
	MnUseWholeScreen->Checked = UseWholeScreen;

	SetDisableFitMode(8);

	FitMode = 0;
	if(UseWholeScreen == true)
	{
		SetNewImageSize();
  }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MnFullScreenClick(TObject *Sender)
{
	FullScreen = !FullScreen;
	GoToFullScreen(FullScreen);
	Paint();
	MnFullScreen->Checked = FullScreen;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MnSlideShowClick(TObject *Sender)
{
	SlideShow = !SlideShow;
	MnSlideShow->Checked = SlideShow;
	SSTimer->Enabled = SlideShow;
	SSIcon = 0;
	DrawSSIcon();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MnReloadClick(TObject *Sender)
{
	ShowAbsoluteImage(ShowIndex, 0);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MnLockClick(TObject *Sender)
{
	Locked = !Locked;
	MnLock->Checked = Locked;

	Susie->SetDoubleBufferMode(!DoubleBufferOff && !Locked);
	CreateDBImage();

	POINT ScCurPos;
	GetCursorPos(&ScCurPos);
	PositionType = GetCursorPositionType(ScCurPos);

	Paint();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MnCloseArchiveClick(TObject *Sender)
{
  if(InArchive == false)return;

	ShowIndex = DisplayLists->Count;

	if(ShowingList)
		JumpBorderArcNml(0);
	else
		JumpBorderArcNml(1);

	while(ShowIndex < 0)
		ShowIndex += DisplayLists->Count;

	while(ShowIndex >= DisplayLists->Count)
		ShowIndex -= DisplayLists->Count;

	if(ShowingList)
		SetFileList();
	else
		ShowAbsoluteImage(ShowIndex, 1);
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::MnShowInformationBalloonClick(TObject *Sender)
{
	UnicodeString Mes = L"";
	if(ShowIndex < 0)
	{

		Mes = L"version 1.99\ncopyright(C) Downspade software.\nAll rights reserved.";
	}
	else
	{
		int P = Percentage * 100;
		Mes = GetShortFileName(DisplayLists->Strings[ShowIndex]) + L"\n";

		if(InArchive)
			Mes += FileLists->Strings[ShowIndexBack] + L"\n\n";

		Mes += "\n" + IntToStr(Susie->OWidth) + L" px x " + IntToStr(Susie->OHeight) + L" px\n" + IntToStr(P) + L"%\nLoader : " + Susie->PluginName;;
	}

	ShowMessage(Mes);
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::MnSaveListsClick(TObject *Sender)
{
	if(SaveFileListsDialog->Execute() == false)return;

	SaveFileLists(SaveFileListsDialog->FileName);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MnShowListClick(TObject *Sender)
{
	Show();
	ToggleShowList(0);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MnSortByNameClick(TObject *Sender)
{
	if(DisplayLists->Count == 0)return;
	SortByName();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MnSortByFileExtClick(TObject *Sender)
{
	if(DisplayLists->Count == 0)return;
	SortBy(0);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MnSortByTimeStampClick(TObject *Sender)
{
	if(DisplayLists->Count == 0)return;
	SortBy(1);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MnSortByFileSizeClick(TObject *Sender)
{
	if(DisplayLists->Count == 0)return;
	SortBy(2);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::MnSortReverseClick(TObject *Sender)
{
	if(DisplayLists->Count == 0)return;
	UnicodeString OldFileName = DisplayLists->Strings[ShowIndex];

	TStringList *TempSL = new TStringList();
	TempSL->AddStrings(DisplayLists);
	DisplayLists->Clear();
	int i, j;
	for(i = 0;i < TempSL->Count;i++)
	{
		j = TempSL->Count - i - 1;
		DisplayLists->AddObject(TempSL->Strings[j], TempSL->Objects[j]);
	}
	delete TempSL;

	ShowIndex = DisplayLists->IndexOf(OldFileName);
	SetFileList();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MnSortRandomClick(TObject *Sender)
{
	if(DisplayLists->Count == 0)return;
	UnicodeString OldFileName = DisplayLists->Strings[ShowIndex];

	Randomize();
	TStringList *TempSL = new TStringList();
	TempSL->AddStrings(DisplayLists);
	DisplayLists->Clear();
	int i;
	while(TempSL->Count > 0)
	{
		i = Random(TempSL->Count);
		DisplayLists->AddObject(TempSL->Strings[i], TempSL->Objects[i]);
    TempSL->Delete(i);
	}
	delete TempSL;

	ShowIndex = DisplayLists->IndexOf(OldFileName);
	SetFileList();
}
//---------------------------------------------------------------------------



void __fastcall TMainForm::MnCloseClick(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------




void __fastcall TMainForm::MnSearchSubFolderClick(TObject *Sender)
{
	SearchSubFolder = !SearchSubFolder;
	MnSearchSubFolder->Checked = SearchSubFolder;
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::MnLoadLastFileClick(TObject *Sender)
{
	LoadLastFile = !LoadLastFile;
	MnLoadLastFile->Checked = LoadLastFile;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MnEnableFileMaskClick(TObject *Sender)
{
	EnableFileMask = !EnableFileMask;
	MnEnableFileMask->Checked = EnableFileMask;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MnSetFileMaskClick(TObject *Sender)
{
	FileMaskString = InputBox(LoadStr(1024), LoadStr(1025), FileMaskString);
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::MnFullScreenColorClick(TObject *Sender)
{
	ColorDialog->Color = FullFillColor;
	if(ColorDialog->Execute() == false)return;
	FullFillColor = ColorDialog->Color;
	FrameColor = GetFrameColor();
	Paint();
}
//---------------------------------------------------------------------------



void __fastcall TMainForm::MnHideTaskButtonClick(TObject *Sender)
{
	HideTaskButton = !HideTaskButton;

	MnHideTaskButton->Checked = HideTaskButton;

	if(HideTaskButton == true)
	{
		SetWindowLong(Handle, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_ACCEPTFILES);
	}
	else
	{
		SetWindowLong(Handle, GWL_EXSTYLE, WS_EX_ACCEPTFILES);
	}

	if(Showing == true)
	{
		Hide();
		Show();
	}
	else
	{
		if(HideTaskButton == true)
		{
			WindowState = wsNormal;
			Hide();
		}
		else
		{
			WindowState = wsMinimized;
			Show();
		}
	}

}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MnSnapWindowClick(TObject *Sender)
{
	ScreenSnap = !ScreenSnap;
	MnSnapWindow->Checked = ScreenSnap;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MnNoStartAtCursorClick(TObject *Sender)
{
	NoStartAtCursor = !NoStartAtCursor;
	MnNoStartAtCursor->Checked = NoStartAtCursor;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MnDoubleBufferOffClick(TObject *Sender)
{
	DoubleBufferOff = !DoubleBufferOff;
	MnDoubleBufferOff->Checked = DoubleBufferOff;

	Susie->SetDoubleBufferMode(!DoubleBufferOff && !Locked);
	CreateDBImage();
	Paint();
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::MnFrameFillColorClick(TObject *Sender)
{
	ColorDialog->Color = FrameFillColor;
	if(ColorDialog->Execute() == false)return;
	FrameFillColor = ColorDialog->Color;
	Paint();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MnSelectPluginFolderClick(TObject *Sender)
{
	int i;
	AnsiString Temp;
	if(OpenPluginDialog->Execute())
	{
		Temp = ExtractFileDir(OpenPluginDialog->FileName);
		if(SpiPathes->Count > 0)SpiPathes->Delete(0);
		if(SpiPathes->Count > 0)
			SpiPathes->Insert(0, Temp);
		else
			SpiPathes->Add(Temp);

		Susie->SetSpiPathes(SpiPathes);		// SPI path set
	}

	Temp = LoadStr(1026);
	for(i = 0;i < SpiPathes->Count;i++)
		Temp = Temp + SpiPathes->Strings[0];

	Temp = Temp + LoadStr(1027);
	MessageBeep(MB_ICONASTERISK);
	ShowMessage(Temp);
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::MnCreateInstanceClick(TObject *Sender)
{
	UnicodeString NewIniParamName, Temp;

	MessageBeep(MB_ICONASTERISK);
	ShowMessage(LoadStr(1028));
	Temp = GetIniFolder() + L"\\milligram";
	SaveIniDialog->InitialDir = Temp;

	while(1)
	{
		if(SaveIniDialog->Execute() == false)return;
		if(ExtractFileDir(SaveIniDialog->FileName) == Temp)break;
		MessageBeep(MB_ICONASTERISK);
		ShowMessage(LoadStr(1029));
		SaveIniDialog->FileName = Temp + L"\\" + (ExtractFileName(SaveIniDialog->FileName));
	}

	NewIniParamName = ChangeFileExt(ExtractFileName(SaveIniDialog->FileName), L"");

	CreateShortCut(NewIniParamName);
	DoCreateSendToLink(NewIniParamName);
	SaveIni(NewIniParamName);

	SetInstanceMode(NewIniParamName);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MnDeleteInstanceClick(TObject *Sender)
{
	MessageBeep(MB_ICONQUESTION);
	int Result = MessageDlg(LoadStr(1030), mtConfirmation, TMsgDlgButtons() << mbOK << mbCancel, NULL);
	if(Result == mrOk)
	{
		DeleteFile(GetIniFileName(L"\\milligram\\" + IniParamName + ".ini"));

		bool Del = false;
		if(FileExists(ShortCutFileName))
			Del = DeleteFile(ShortCutFileName);

		if(CreateSendToLink)
			DeleteSendToLink(IniParamName);

		if(Del == true)
		{
			MessageBeep(MB_ICONASTERISK);
			ShowMessage(LoadStr(1031));
		}
		else
		{
			MessageBeep(MB_ICONASTERISK);
			ShowMessage(LoadStr(1032));
		}

		NotSaveIni = true;
		Close();
	}
}
//---------------------------------------------------------------------------



void __fastcall TMainForm::MnUninstallClick(TObject *Sender)
{
	int Result;
	MessageBeep(MB_ICONQUESTION);
	Result = MessageDlg(LoadStr(1033), mtConfirmation, TMsgDlgButtons() << mbOK << mbCancel, NULL);
	if(Result == mrOk)
	{
		if(DoUninstall() == true)
		{
			MessageBeep(MB_ICONASTERISK);
			ShowMessage(LoadStr(1034));
		}
		else
		{
			MessageBeep(MB_ICONASTERISK);
			ShowMessage(LoadStr(1035));
		}

		NotSaveIni = true;
		Close();
	}
	//
}
//---------------------------------------------------------------------------



void __fastcall TMainForm::MnTCClick(TObject *Sender)
{
	TrayIcons[TrayIconColor]->Checked = false;
	TrayIconColor = (dynamic_cast<TComponent *>(Sender))->Tag;
	TrayIcon->IconIndex = TrayIconColor;
	TrayIcons[TrayIconColor]->Checked = true;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MnSSTimeClick(TObject *Sender)
{
	SSInterval = (dynamic_cast<TComponent *>(Sender))->Tag * 1000;
	CheckIntervalMenuCheck();
	SSTimer->Interval = SSInterval;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MnFrameWidthClick(TObject *Sender)
{
	FrameWidth = (dynamic_cast<TComponent *>(Sender))->Tag;
	CheckFrameWidthMenuCheck();
	GetMonitorParameter();
	SetNewImageSize();
	SyncWindow(false);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MnWheelSenseClick(TObject *Sender)
{
	WheelSensitivity = (dynamic_cast<TComponent *>(Sender))->Tag;
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::DisplayBoxDblClick(TObject *Sender)
{
	if(Locked)return;
	ToggleShowList(2);
}
//---------------------------------------------------------------------------





void __fastcall TMainForm::SSTimerTimer(TObject *Sender)
{
	SSChangeImage = true;
	ShowOffsetImage(1);
	SSChangeImage = false;
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::CursorTimerTimer(TObject *Sender)
{
	POINT ScCurPos;
	GetCursorPos(&ScCurPos);

	if(PreCurTimerPos.x > ScCurPos.x - 3 && PreCurTimerPos.x < ScCurPos.x < 3
	&& PreCurTimerPos.y > ScCurPos.y - 3 && PreCurTimerPos.x < ScCurPos.x < 3)
	{
  	if(StopCount >= 0)StopCount++;
	}
	PreCurTimerPos = ScCurPos;

	if(StopCount > 2)
	{
		SetCursorMode(-1);
		StopCount = -1;
	}

	if(SSIcon >= 0)
	{
		if(SSIcon < 2)
		{
			DrawSSIcon();
			SSIcon++;
		}
		else
		{
			SSIcon = -1;
			Paint();
		}
	}
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::DrawSSIcon(void)
{
	if(WWidth < 20 || WHeight < 20)return;
	Canvas->Pen->Style = psSolid;
	Canvas->Pen->Color = FrameColor;
	Canvas->Brush->Color = FullFillColor;
	Canvas->Rectangle(Width - 19, Height - 19, Width - 3, Height - 3);

	Canvas->Brush->Color = FrameColor;

	if(SlideShow == false)
	{
		Canvas->Rectangle(Width - 15, Height - 15, Width - 7, Height - 7);
	}
	else
	{
		TPoint PolPoint[3];
		PolPoint[0].x = Width - 15;
		PolPoint[0].y = Height - 15;
		PolPoint[1].x = Width - 8;
		PolPoint[1].y = Height - 11;
		PolPoint[2].x = Width - 15;
		PolPoint[2].y = Height - 8;
		Canvas->Polygon(PolPoint, 2);
	}
}


void __fastcall TMainForm::DBTimerTimer(TObject *Sender)
{
	Susie->CreateDBImage(WWidth, WHeight);
//	Susie->EndReSizeMode();
	Paint();
	DBTimer->Enabled = false;
}
//---------------------------------------------------------------------------



void __fastcall TMainForm::MnARotClick(TObject *Sender)
{
	Susie->OffsetRotate((dynamic_cast<TComponent *>(Sender))->Tag);
	CheckRotateCheck();
	SetRotateImageSize();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MnFixRotateClick(TObject *Sender)
{
	FixRotate = !FixRotate;
	MnFixRotate->Checked = FixRotate;
	Susie->FixRotate = FixRotate;
	Susie->RotateValue = RotateValue;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MnAbsRot(TObject *Sender)
{
	if(Susie->Rotate == NULL)return;
	if(RotateValue != (dynamic_cast<TComponent *>(Sender))->Tag)
	{
		Susie->AbsoluteRotate((dynamic_cast<TComponent *>(Sender))->Tag);
		CheckRotateCheck();
		SetRotateImageSize();
	}
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::CheckRotateCheck(void)
{
	if(Susie == NULL || Susie->Rotate == NULL)return;
	if(Susie->Mode)
		RotateState[*Susie->Rotate]->Checked = true;
	else
		RotateState[0]->Checked = true;
	RotateValue = *Susie->Rotate;
	Susie->RotateValue = RotateValue;
}

bool __fastcall TMainForm::SetRotateImageSize(void)
{
	FixPercentage = true;
	SetNewImageSize();
	FixPercentage = false;
	return(true);
}

bool __fastcall TMainForm::CreateFilePPMenu(void)
{
	UnicodeString Folder, Temp;

	if(FilePPMenu->Items->Count > 0)
	{
		FilePPMenu->Items->Clear();
	}

	if(ShowingList == true)
	{
		if(InArchive == true)
			Temp = OpeningFileName;
		else
			Temp = FileLists->Strings[DisplayBox->ItemIndex];
	}
	else
		Temp = OpeningFileName;

	Folder = ExtractFileDir(Temp);

	if(Folder == "")
	{
		return(false);
	}

	CreateFileMenuFolder(Folder);
	return(true);
}

void __fastcall TMainForm::CreateFileMenuFolder(UnicodeString FolderName)
{
	TSearchRec sr;
	int Count = 0;
	FolderLists->Clear();
	TMenuItem *NewItem;

	if(FindFirst(FolderName + "\\*", faDirectory, sr) == 0)
	{
		do
		{
			if(sr.Attr & faDirectory)
			{
				if(sr.Name == L".")
				{
				}
				else if(sr.Name == L"..")
				{
					FilePPMenu->Items->Insert(0, GetNewMenu(L"..", FolderName, 0));
				}
				else
				{
					FolderLists->Add(sr.Name);
					FilePPMenu->Items->Insert(FilePPMenu->Items->Count, GetNewMenu(sr.Name, FolderName, Count));//最後のメニューに追加
					Count++;
				}
			}
		}
		while(FindNext(sr) == 0);
		FindClose(sr);

		NewItem = new TMenuItem(this);
		NewItem->Tag = 0; NewItem->Caption =  + L"-";

		FilePPMenu->Items->Insert(FilePPMenu->Items->Count, NewItem);//最後のメニューに追加
	}

	//-------------------------------------------------------------------------------------------

	NewItem = new TMenuItem(this);
	NewItem->Tag = -2;
	UnicodeString ThisFolder = ExtractFileName(FolderName);
	if(ThisFolder != L"")
	{
		NewItem->Caption = ThisFolder + LoadStr(1036);
	}
	else
	{
		NewItem->Caption = FolderName + LoadStr(1037);
	}
	NewItem->OnClick = FileMenuClick;

	FilePPMenu->Items->Insert(FilePPMenu->Items->Count, NewItem);

	//-------------------------------------------------------------------------------------------

	NewItem = new TMenuItem(this);
	NewItem->Tag = 0; NewItem->Caption =  + L"-";

	FilePPMenu->Items->Insert(FilePPMenu->Items->Count, NewItem);//最後のメニューに追加

	//-------------------------------------------------------------------------------------------

	NewItem = new TMenuItem(this);
	NewItem->Tag = -3;

	NewItem->Caption = LoadStr(1038);
	NewItem->OnClick = FileMenuClick;

	FilePPMenu->Items->Insert(FilePPMenu->Items->Count, NewItem);

}

TMenuItem *__fastcall TMainForm::GetNewMenu(UnicodeString MenuName, UnicodeString FullPath, int Count)
{
  TMenuItem *NewItem = new TMenuItem(this);

	NewItem->Tag = Count;

	if(MenuName == L"..")
	{
		NewItem->Tag = -1;

		int KeyboardType = GetKeyboardType(0);

		if(KeyboardType == 7)
		{
			NewItem->Caption =  + L"[..]" + ExtractFileDir(GetShortFileName(FullPath)) + L"(&\\)";
		}
		else
		{
			NewItem->Caption =  + L"[..]" + ExtractFileDir(GetShortFileName(FullPath)) + L"(&`)";
		}
	}
	else if(Count < 9)
	{
		NewItem->Caption = MenuName + L"(&" + (wchar_t)((int)L'1' + Count) + L")";
	}
	else if(Count == 9)
	{
		NewItem->Caption = MenuName + L"(&" + (wchar_t)((int)L'0') + L")";
	}
	else if(Count < 35)
	{
		NewItem->Caption = MenuName + L"(&" + (wchar_t)((int)L'A' + Count - 10) + L")";
	}
	else
	{
		NewItem->Caption = MenuName;
	}

	NewItem->OnClick = FileMenuClick;

	return(NewItem);
}

void __fastcall TMainForm::FileMenuClick(TObject *Sender)
{
/*
	static int Count = 0;
	Count++;
	if(Count == 2)
	{
		Count = 3;
	}
*/
	int Index = ((TComponent *)Sender)->Tag;
	UnicodeString Src;

	Src = GetOpeningFileName();

	UnicodeString Folder = ExtractFileDir(Src);
	UnicodeString Dest;

	if(Folder[Folder.Length()] == L'\\')
	{
		Folder = Folder.SubString(1, Folder.Length() - 1);
	}

	if(Index == -3)
	{
		if(AlwaysTop == true)
			FormStyle = fsNormal;

		CreateFolder();

		if(AlwaysTop == true)
			FormStyle = fsStayOnTop;
	}
	else if(Index == -2)return;
	else
	{
		if(Index == -1)
		{
			Folder = ExtractFilePath(Folder);
		}
		else
		{
			Folder = Folder + L"\\" + FolderLists->Strings[Index] + L"\\";
		}

		Dest = Folder + OriginName;

		if(FileExists(Dest) == true)
		{
			if(
			 (GetAsyncKeyState(VK_LCONTROL) & 0x8000)
			|| (GetAsyncKeyState(VK_RCONTROL) & 0x8000)
			)
			{
				Dest = GetNonOverwrapFileName(Dest);
			}
			else
			{
				int Result;
				MessageBeep(MB_ICONQUESTION);
				Result = MessageDlg(LoadStr(1039), mtConfirmation, TMsgDlgButtons() << mbYes << mbNo, NULL);

				if(Result == mrNo)return;
			}
		}

	//  FileChanged->SetCheckFile(L"");
		MoveFileEx(Src.c_str(), Dest.c_str(), MOVEFILE_REPLACE_EXISTING);
	//  FileChanged->SetCheckFile(Dest);

		SetOpeningFileName(Dest);
	}

	CreateFilePPMenu();

	FilePPMenu->Popup(FilePPMenu->PopupPoint.x, FilePPMenu->PopupPoint.y);
}

void __fastcall TMainForm::CreateFolder(void)
{
	UnicodeString NewFileName = L"";

	if(InputQuery(LoadStr(1040), LoadStr(1041), NewFileName) == false)return;

	int i;
	i = CheckFileIrregularChar(NewFileName);
	if(i >= 0)
	{
		MessageBeep(MB_ICONASTERISK);
		ShowMessage(NewFileName + LoadStr(1022) + NewFileName[i] + LoadStr(1023));
		return;
	}

	NewFileName = ExtractFilePath(OpeningFileName) + NewFileName;

	if(DirectoryExists(NewFileName) == true)
	{
		MessageBeep(MB_ICONASTERISK);
		ShowMessage(LoadStr(1042));
	}
	else
	{
		MkDir(NewFileName);
	}
}


UnicodeString __fastcall TMainForm::GetOpeningFileName(void)
{
	if(ShowingList == true)
	{
		if(InArchive == true)
			return(OpeningFileName);
		else
			return(FileLists->Strings[DisplayBox->ItemIndex]);
	}

	return(OpeningFileName);
}

bool __fastcall TMainForm::SetOpeningFileName(UnicodeString Dest)
{
	if(ShowingList == true)
	{
		if(InArchive == true)
		{
			int i = ArchiveFileLists->IndexOf(OpeningFileName);
			ArchiveFileLists->Strings[i] = Dest;
			FileLists->Strings[ShowIndexBack] = Dest;
			OpeningFileName = Dest;
			Susie->ChangeArchiveFileName(Dest);
		}
		else
		{
			DisplayBox->Items->Strings[DisplayBox->ItemIndex] = GetShortFileName(Dest);
			FileLists->Strings[DisplayBox->ItemIndex] = Dest;
		}
	}
	else
	{
		if(InArchive == true)
		{
			int i = ArchiveFileLists->IndexOf(OpeningFileName);
			ArchiveFileLists->Strings[i] = Dest;
			FileLists->Strings[ShowIndexBack] = Dest;
			OpeningFileName = Dest;
			Susie->ChangeArchiveFileName(Dest);
		}
		else
		{
			FileLists->Strings[ShowIndex] = Dest;
			OpeningFileName = Dest;
		}
	}
	return(true);
}

UnicodeString __fastcall TMainForm::GetImageFileName(void)
{
	UnicodeString Result;
	if(ShowingList)
	{
		if(DisplayBox->ItemIndex == -1)return(L"");
	}
	else
	{
		if(ShowIndex == -1)return(L"");
	}

	if(InArchive == true)
	{
		Result = FileLists->Strings[ShowIndexBack];
	}
	else if(ShowingList == true)
	{
		Result = FileLists->Strings[DisplayBox->ItemIndex];
	}
	else
	{
		Result = FileLists->Strings[ShowIndex];
	}
	return(Result);
}

UnicodeString __fastcall TMainForm::GetImageFileFolder(void)
{
	UnicodeString Result;
	Result = ExtractFileDir(GetImageFileName());
	return(Result);
}


void __fastcall TMainForm::TrayIconMouseUp(TObject *Sender, TMouseButton Button,
					TShiftState Shift, int X, int Y)
{
	switch(Button)
	{
		case mbLeft:
			if(WindowState == fsNormal)
			{
				SetAbsoluteForegroundWindow(Handle);
				SetFocus();
			}
			else
			{
				MnShowClick(NULL);
			}
			break;

		case mbRight:
			PopupMenuPopup(X, Y);
			break;

		case mbMiddle:
			MnShowClick(NULL);
			break;
	}
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::MnShowClick(TObject *Sender)
{
	if(HideTaskButton == true)
	{
		if(Showing == true)
		{
			Hide();
		}
		else
		{
			Show();
		}
	}
	else
	{
		if(WindowState == wsNormal)
		{
			WindowState = wsMinimized;
		}
		else
		{
			WindowState = wsNormal;
		}
	}
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::MnSPIOpenDialogClick(TObject *Sender)
{
	if(Susie->Mode & MMODE_SPI)Susie->OpenSpiSettingDialog(MainForm->Handle);
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::MnCopyImageClick(TObject *Sender)
{
	if(EnableDraw == false)return;
	Susie->CopyImageToClipboard();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::RestoreStayOnTop(void)
{
	if(AlwaysTop == true)
	{
		FormStyle = fsStayOnTop;
		MnAlwaysTop->Checked = true;
	}
	else
	{
		FormStyle = fsNormal;
		MnAlwaysTop->Checked = false;
	}
}

void __fastcall TMainForm::MnLFS5Click(TObject *Sender)
{
	ApplicationFontSize = (dynamic_cast<TComponent *>(Sender))->Tag;
	CheckFontSizeMenuCheck();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MnJpegSettingClick(TObject *Sender)
{
	FormStyle = fsNormal;

	JpegCmpRatForm->SetData(JCR);
	if(JpegCmpRatForm->ShowModal() == mrOk)
	{
		JpegCmpRatForm->GetData(JCR);
	}

	RestoreStayOnTop();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MnJpegSaveClick(TObject *Sender)
{
	UnicodeString Dest = DisplayLists->Strings[ShowIndex];

	SaveJpegDialog->InitialDir = ExtractFileDir(Dest);

	Dest = GetNonOverwrapFileName(ChangeFileExt(Dest, ".jpg"));
	Dest = ExtractFileName(Dest);

	SaveJpegDialog->FileName = Dest;
	SaveJpegDialog->Title = LoadStr(1043);
	if(SaveJpegDialog->Execute() == true)
	{
		Graphics::TBitmap *Bmp = new Graphics::TBitmap();
		Bmp->Width = Susie->Width;
		Bmp->Height = Susie->Height;
		Bmp->PixelFormat = pf24bit;

		int OldStretchMode = SetStretchBltMode(Bmp->Canvas->Handle, HALFTONE);
		StretchDIBits(Bmp->Canvas->Handle,
									0, 0, Susie->Width, Susie->Height,
									0, 0, Susie->Width, Susie->Height,
									Susie->pBmpData, Susie->pBmpInfo,
									DIB_RGB_COLORS, SRCCOPY);

		TJPEGImage * Jpeg = new TJPEGImage();

		Jpeg->Performance = jpBestQuality;
		Jpeg->PixelFormat = jf24Bit;
		Jpeg->Grayscale = false;
		Jpeg->Assign(Bmp);

		Jpeg->CompressionQuality = (char)JCR[0];
		Jpeg->Compress();
		Jpeg->SaveToFile(SaveJpegDialog->FileName);

		SetStretchBltMode(Bmp->Canvas->Handle, OldStretchMode);
		delete Bmp;
		delete Jpeg;
	}
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MnJpegSizeSaveClick(TObject *Sender)
{
	UnicodeString Dest = DisplayLists->Strings[ShowIndex];

	SaveJpegDialog->InitialDir = ExtractFileDir(Dest);

	Dest = GetNonOverwrapFileName(ChangeFileExt(Dest, ".jpg"));
	Dest = ExtractFileName(Dest);

	SaveJpegDialog->FileName = Dest;
	SaveJpegDialog->Title = LoadStr(1044);
	if(SaveJpegDialog->Execute() == true)
	{
		Graphics::TBitmap *Bmp = new Graphics::TBitmap();
		Bmp->Width = WWidth;
		Bmp->Height = WHeight;
		Bmp->PixelFormat = pf24bit;

		int OldStretchMode = SetStretchBltMode(Bmp->Canvas->Handle, HALFTONE);
		StretchDIBits(Bmp->Canvas->Handle,
									0, 0, WWidth, WHeight,
									0, 0, Susie->DWidth, Susie->DHeight,
									Susie->pBitmapData, Susie->pBitmapInfo,
									DIB_RGB_COLORS, SRCCOPY);

		TJPEGImage * Jpeg = new TJPEGImage();

		Jpeg->Performance = jpBestQuality;
		Jpeg->PixelFormat = jf24Bit;
		Jpeg->Grayscale = false;
		Jpeg->Assign(Bmp);

		Jpeg->CompressionQuality = (char)JCR[0];
		Jpeg->Compress();
		Jpeg->SaveToFile(SaveJpegDialog->FileName);

		SetStretchBltMode(Bmp->Canvas->Handle, OldStretchMode);
		delete Bmp;
		delete Jpeg;
	}
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MnSetFormSizeClick(TObject *Sender)
{
	FormStyle = fsNormal;

	FormSizeForm->SetData(Susie->Width, Susie->Height);

	if(FormSizeForm->ShowModal() == mrOk)
	{
		int DestWidth, DestHeight;
		FormSizeForm->GetData(DestWidth, DestHeight);

		FixViewOut();
		SetWindowSize(DestWidth, DestHeight, false);
		CreateDBImage();
		SetConstantSize();
	}

	RestoreStayOnTop();
}
//---------------------------------------------------------------------------



void __fastcall TMainForm::MnAVMClick(TObject *Sender)
{
	FormAlphaBlendValue = (dynamic_cast<TComponent *>(Sender))->Tag;
	CheckAlphaValueMenuCheck();
}
//---------------------------------------------------------------------------


bool __fastcall TMainForm::CreateHistoryMenu(void)
{
	int i;
	for(i = 0;i < MaxHistoryNum;i++)
	{
		HistoryMenu[i] = new TMenuItem(this);
		HistoryMenu[i]->Caption = (L"none");
		HistoryMenu[i]->Visible = false;
		HistoryMenu[i]->OnClick = HistoryMnClick;
		HistoryPPMenu->Items->Add(HistoryMenu[i]);//最後のメニューに追加
	}

	return(true);
}


bool __fastcall TMainForm::ConvertHistoryMenu(void)
{
	int i;
	if(HistoryList->Count > 0)
	{
		MnReloadFile->Enabled = true;
		for(i = 0;i < HistoryList->Count;i++)
		{
			HistoryMenu[i]->Caption = GetShortFileName(HistoryList->Strings[i]);
			HistoryMenu[i]->Visible = true;
		}
		for(;i < MaxHistoryNum;i++)
		{
			HistoryMenu[i]->Visible = false;
		}
	}
	else
	{
		MnReloadFile->Enabled = false;
	}

	if(RefreshHistoryMode == 0)
	{
		SaveIni(IniParamName);
	}
	else
	{
		RefreshHistoryMode = 0;
	}
	return(true);
}


void __fastcall TMainForm::HistoryMnClick(TObject *Sender)
{
	int i, p = -1;
	for(i = 0;i < MaxHistoryNum;i++)
	{
		if(HistoryMenu[i] == Sender)
			p = i;
	}

	if((GetAsyncKeyState(VK_LSHIFT) & 0x8000) || (GetAsyncKeyState(VK_RSHIFT) & 0x8000))
	{
		HistoryList->Delete(p);
		ConvertHistoryMenu();
    if(HistoryList->Count > 0)HistoryListPopUp();
		return;
	}

	if(FileExists(HistoryList->Strings[p]) == false)
	{
		if(DirectoryExists(IncludeTrailingBackslash(HistoryList->Strings[p])) == false)
		{
			UnicodeString Temp = HistoryList->Strings[p];
			HistoryList->Delete(p);
			ConvertHistoryMenu();
			MessageBeep(MB_ICONHAND);
			MessageDlg(Temp + LoadStr(1045), mtWarning, TMsgDlgButtons() << mbOK, 0);
			return;
		}
	}

	TStringList *TempLists = new TStringList();
	TempLists->Add(HistoryList->Strings[p]);
	OpenFiles(TempLists);
	delete TempLists;

	HistoryList->Move(p, 0);
	ConvertHistoryMenu();
}

bool __fastcall TMainForm::AddHistoryList_(UnicodeString FileName)
{
	int i;
	i = HistoryList->IndexOf(FileName);
	if(i >= 0)HistoryList->Delete(i);

	HistoryList->Insert(0, FileName);

	while(HistoryList->Count > MaxHistoryNum)
	{
		HistoryList->Delete(HistoryList->Count - 1);
	}
	return(true);
}

bool __fastcall TMainForm::AddHistoryList(UnicodeString FileName)
{
	AddHistoryList(FileName);
	ConvertHistoryMenu();
	return(true);
}


bool __fastcall TMainForm::AddHistoryList(TStringList *FileNames)
{
	int i = 0;
	if(FileNames->Count > MaxHistoryNum)
	{
		i = FileNames->Count - MaxHistoryNum;
	}

	while(i < FileNames->Count)
	{
		AddHistoryList_(FileNames->Strings[i]);
		i++;
	}

	ConvertHistoryMenu();
	return(true);
}


void __fastcall TMainForm::MnReloadFileClick(TObject *Sender)
{
	POINT ScCurPos;
	GetCursorPos(&ScCurPos);
	HistoryPopUpX = ScCurPos.x;
	HistoryPopUpY = ScCurPos.y;
	if(HistoryList->Count > 0)HistoryListPopUp();
}


void __fastcall TMainForm::HistoryListPopUp(void)
{
	FILETIME TempFileStamp;

	UnicodeString Path, Temp;
	Path = GetIniFileName(L"\\milligram\\" + IniParamName + ".ini");
	if(FileExists(Path) == false)return;

	GetFileTimeStamp(Path, &TempFileStamp);

	if(TempFileStamp.dwLowDateTime != IniFileTime.dwLowDateTime || TempFileStamp.dwHighDateTime != IniFileTime.dwHighDateTime)
	{
		TStringList *IniFile = new TStringList();
		LoadStringList(IniFile, Path);

		int i = 0;
		HistoryList->Clear();
		while(i < MaxHistoryNum && CheckStrings(IniFile, L"History" + IntToStr(i)) == true)
		{
			Temp = IniFile->Values[L"History" + IntToStr(i)];
			HistoryList->Add(Temp);
			i++;
		}
		delete IniFile;
		RefreshHistoryMode = 1;
		ConvertHistoryMenu();

		IniFileTime.dwLowDateTime = TempFileStamp.dwLowDateTime;
		IniFileTime.dwHighDateTime = TempFileStamp.dwHighDateTime;
	}

	HistoryPPMenu->Popup(HistoryPopUpX, HistoryPopUpY);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MnLoadFileFolderClick(TObject *Sender)
{
	if(FileLists->Count == 0)return;

	UnicodeString FolderName = ExtractFileDir(OpeningFileName);
	if(FolderName == L"")return;

	TStringList *TempSL = new TStringList();;
	TempSL->Add(FolderName);
	OpenFiles(TempSL, OpeningFileName, 0, false);
	delete TempSL;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MnAutoLoadFileFolderClick(TObject *Sender)
{
	AutoLoadFileFolder = !AutoLoadFileFolder;
	MnAutoLoadFileFolder->Checked = AutoLoadFileFolder;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::PopupMenuPopup(int X, int Y)
{
	if(HideTaskButton == true)
	{
		if(Showing == true)
		{
			MnShow->Caption = LoadStr(1046);
		}
		else
		{
			MnShow->Caption = LoadStr(1047);
		}
	}
	else
	{
		if(WindowState == wsNormal)
		{
			MnShow->Caption = LoadStr(1046);
		}
		else
		{
			MnShow->Caption = LoadStr(1047);
		}
	}

	PopupMenu->Popup(X, Y);
}



void __fastcall TMainForm::MnSpiPrecedeClick(TObject *Sender)
{
	Susie->SpiPrecede = !Susie->SpiPrecede;
	MnSpiPrecede->Checked = Susie->SpiPrecede;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MnPNGSaveClick(TObject *Sender)
{
	UnicodeString Dest = DisplayLists->Strings[ShowIndex];

	SavePNGDialog->InitialDir = ExtractFileDir(Dest);

	Dest = GetNonOverwrapFileName(ChangeFileExt(Dest, ".png"));
	Dest = ExtractFileName(Dest);

	SavePNGDialog->FileName = Dest;
	SavePNGDialog->Title = LoadStr(1048);
	if(SavePNGDialog->Execute() == true)
	{
		Graphics::TBitmap *Bmp = new Graphics::TBitmap();
		Bmp->Width = Susie->Width;
		Bmp->Height = Susie->Height;
		Bmp->PixelFormat = pf24bit;

		int OldStretchMode = SetStretchBltMode(Bmp->Canvas->Handle, HALFTONE);
		StretchDIBits(Bmp->Canvas->Handle,
									0, 0, Susie->Width, Susie->Height,
									0, 0, Susie->Width, Susie->Height,
									Susie->pBmpData, Susie->pBmpInfo,
									DIB_RGB_COLORS, SRCCOPY);

		TPNGObject * PNG = new TPNGObject();

		PNG->Assign(Bmp);

		PNG->CompressionLevel = 5;
		PNG->SaveToFile(SavePNGDialog->FileName);

		SetStretchBltMode(Bmp->Canvas->Handle, OldStretchMode);
		delete Bmp;
		delete PNG;
	}
}
//---------------------------------------------------------------------------

