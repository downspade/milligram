#pragma once

#include "windows.h"
#include "spiplugin.h"
#include "resource.h"
#include <set>
#include <map>
#include "acfc.h"

#define MAX_LOADSTRING 100
#define MIV_SMALLESTLISTWINDOW 200
#define CULTURESTR "en-US"
#define MIN_LOOP 50
#define MIN_WAIT 10
#define DEFAULT_MAXHISTORYNUM 16

using namespace milligram;

class CMainForm
{

	enum EEventButton : int
	{
		EEventButton_NONE = 0,
		EEventButton_LEFT = 1,
		EEventButton_RIGHT = 2,
		EEventButton_MIDDLE = 4,
		EEventButton_XBUTTON1 = 8,
		EEventButton_XBUTTON2 = 16,
	};

	enum ESortType : int
	{
		ESortType_NAME = 0,
		ESortType_EXT = 1,
		ESortType_TIMESTAMP = 2,
		ESortType_FILESIZE = 3,
		ESortType_RANDOM = 4,
		ESortType_REVERSE = 5,
	};


	enum ELoadFileResult : int
	{
		ELoadFileResult_OK = 0,
		ELoadFileResult_OPENARCHIVE = 1,
		ELoadFileResult_FAILEDORNOMASK = 2,
		ELoadFileResult_NOARCHIVE = 3,
		ELoadFileResult_RETRYSAMEINDEX = 4,
		ELoadFileResult_NOFILE = 5,
	};

	enum EOpenFileResult : int
	{
		EOpenFileResult_SUCCEEDED = 0,
		EOpenFileResult_LOADFAILED = 1,
		EOpenFileResult_EXCLUDEBYMASK = 2,
		EOpenFileResult_NOARCHIVEFILE = 3,
	};

	enum EShowAbsoluteImageResult : int
	{
		EShowAbsoluteImageResult_PASSED = 0,
		EShowAbsoluteImageResult_OPENARCHIVE = 1,
		EShowAbsoluteImageResult_NOFILE = 2,
	};

	enum EFitMode : int
	{
		EFitMode_NONE = 0,
		EFitMode_FIT_TOPANDBOTTOM = 1,
		EFitMode_FIT_LEFTANDRIGHT = 2,
	};

	enum EPositionMode : int // 0:自由 1:中央 2:左上 3:右上 4:左下 5:右下
	{
		EPositionMode_FREE = 0,
		EPositionMode_CENTER = 1,
		EPositionMode_LEFTTOP = 2,
		EPositionMode_RIGHTTOP = 3,
		EPositionMode_LEFTBOTTOM = 4,
		EPositionMode_RIGHTBOTTOM = 5,
	};

	enum EShowMode : int // Mode 0:反転 1:リスト表示 2:画像表示 3:強制画像表示
	{
		EShowMode_TOGGLE = 0,
		EShowMode_LIST = 1,
		EShowMode_PICTURE = 2,
		EShowMode_FORCEPICTURE = 3,
	};

	enum EMousePositionType : int
	{
		EMousePositionType_NONE = -1,
		EMousePositionType_NORMAL = 0,
		EMousePositionType_LT = 1,
		EMousePositionType_RT = 2,
		EMousePositionType_LB = 3,
		EMousePositionType_RB = 4,
		EMousePositionType_L = 5,
		EMousePositionType_R = 6,
		EMousePositionType_T = 7,
		EMousePositionType_B = 8,
	};

	enum ESetTrayIconMode : int
	{
		ESetTrayIconMode_ADD = 0,
		ESetTrayIconMode_MODIFY = 1,
		ESetTrayIconMode_DELETE = 2,
	};

public:
	HINSTANCE appInstance = nullptr;
	TCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
	TCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

	std::vector<std::wstring> CmdLine;

	HWND hWindow = nullptr; // フォームのウィンドウハンドル
	HBRUSH hBGBrush; // ウィンドウ背景色ブラシ
	HFONT hFont = nullptr; // 表示ファイルリストのフォントオブジェクト
	HMENU hParentPopupMenu = nullptr; // すべてのメニュー
	HMENU hPopupMenu = nullptr; // ポップアップメニューハンドル
	HMENU hHistoryMenu = nullptr; // ヒストリメニューハンドル
	HMENU hParentMoveMenu = nullptr; // ヒストリメニューハンドル
	HMENU hMoveMenu = nullptr; // ファイル移動メニューハンドル
	int MenuShowIndex = 0; // メニュー表示インデックス 0:なし 1:メインメニュー 2:移動メニュー
	std::wstring MoveSrc = TEXT(""); // ファイルムーブソース
	LANGID LangID = 0; // 言語ID
	bool Active = true; // フォームがアクティブかどうか
	bool MouseCapturing = false; // マウスをキャプチャ中かどうか

	acfc::CListBox DisplayBox;

	acfc::COpenFileDialog OpenFileDialog;

	acfc::CSaveFileDialog SavemflDialog;
	acfc::CSaveFileDialog SaveIniDialog;
	acfc::CSaveFileDialog SaveLnkDialog;
	acfc::CSaveFileDialog SaveJpegDialog;
	acfc::CSaveFileDialog SavePNGDialog;

	acfc::CFolderSelectDialog FolderSelectDialog;
	acfc::CSelectColorDialog ColorDialog;
	acfc::CTimer SSTimer;
	acfc::CTimer CursorTimer;

	//-----------------------------------------------------------------------
private:
	bool Initialized = false;

	//---------------------------------------------------------

	CSpiLoader *Susie;

	//---------------------------------------------------------

	std::vector<CImageInfo> FileList;
	std::vector<CImageInfo> *DisplayList;
	std::vector<std::wstring> PluginPathes;

	std::wstring IniParamName = TEXT("milligram");
	std::wstring BalloonTipTitle = TEXT("milligram image viewer");
	std::wstring IniFolderName = TEXT("");
	std::wstring ExeFileName = TEXT("");
	std::wstring TryFileName = TEXT("");// 表示しようとしているファイル名
	std::wstring TryArchiveName = TEXT(""); // 表示しようとしているアーカイブ名

	std::set<std::wstring> StrBuf; // 文字列リソース保管場所
	std::set<std::wstring> MoveBuf; // File Move PP メニューの文字列バッファ
	std::vector<std::wstring> MoveData; // FileMove PP の移動対応先フルパス文字列
	std::wstring LastMovedFolder = TEXT(""); // 最後に移動先に指定されたフォルダ

	//-----------------------------------------------------------------------------

	POINT OfsCurPos;          // マウスダウンしたときの Left Top の位置
	POINT DownCurPos;         // マウスダウンしたときの画面上の位置
	POINT PreCurPos;          // マウスダウン中の一つ前の画面上の位置
	POINT PreCurTimerPos;           // 一つ前のカーソル位置 
	POINT MovePPPosition; // Move ポップアップメニューの位置
	int UseCursorSize = 0; // 大きなカーソルを使うかどうか

	int DownWidth;            // マウスダウンしたときの幅
	int DownHeight;           // マウスダウンしたときの高さ
	int DownLeft;               // マウスダウンしたときの左
	int DownTop;                // マウスダウンしたときの上

	EEventButton Holding = EEventButton_NONE;              // マウスホールドしているかどうか 0:してない 1:左 2:右 3:真ん中
	EMousePositionType PositionType = EMousePositionType_NONE;         // マウスダウンしたときのマウスの位置
	EMousePositionType CursorMode = EMousePositionType_NONE; // 現在のカーソル
	double PosRatioX = 0.0; // カーソルが画像のどの位置にあるか
	double PosRatioY = 0.0;
	//-----------------------------------------------------------------------------

	//-----------------------------------------------------------------------------
	double WbHRatio = 1.0;             // 画像の比
	double SizeRatio = 1.0;        // 拡大率
	EFitMode FitMode = EFitMode_NONE;                            // 1:上下が吸着 2:左右が吸着
	int WheelPos = 0;             // ホイール値
	int StartnNum = -1;                    // 初期位置
	int PreZoomed = false;
	//-----------------------------------------------------------------------------

	//-----------------------------------------------------------------------------
	bool NotSaveIni = false;          // Ini ファイルを終了時に保存するかどうか
	bool InstanceMode = false;        // インスタンスモードで動作中かどうか
	bool UninstallMode = false;		// アンインストールモードとして起動されたかどうか
	//-----------------------------------------------------------------------------

	//-----------------------------------------------------------------------------
	int EnableDraw = 0;                    // 再描画の抑制
	//-----------------------------------------------------------------------------

	//-----------------------------------------------------------------------------
	bool InArchive = false;                     // アーカイブファイルを開いているかどうか
	//-----------------------------------------------------------------------------

	//-----------------------------------------------------------------------------
	bool InitialReloading = false;            // 復元起動時に大きさ位置を保持するために使う
	bool IgnoreContextMenu = false;                    // 右クリックメニューをキャンセルするかどうか
	int SSIcon = -1;               // スライドショーアイコンのモード
	bool SSChangeImage = false;                     // マウスの移動を無視する
	//-----------------------------------------------------------------------------

	//-----------------------------------------------------------------------------
	int ShowIndex = -1;            // 表示中の画像の番号
	int ShowArchive = -1;         // 表示中の画像の番号(サブ)
	std::wstring ShowFileName = TEXT("");// 表示中のファイル名
	std::wstring ShowArchiveName = TEXT(""); // 表示中のアーカイブ名
	std::wstring OpeningFileName = TEXT(""); // 表示中のファイル名（実体）アーカイブファイルならアーカイブファイル名
	std::wstring mflFileName = TEXT(""); // 読み込んだ mfl ファイル
	bool AlwaysTop = false;           // 常に手前に表示
	bool ShowingList = false;         // ファイルリスト表示中かどうか
	bool PreShowingList = false;         // 前回終了時にファイルリスト表示中かどうか
	bool SlideShow = false;           // スライドショーモード
	int SSInterval = 5000;           // スライドショーのインターバル
	UINT SSId = 1;					// タイマーの ID
	bool FixSizeRatio = false;       // 拡大率固定
	bool FitToScreen = false;         // 画面に合わせて表示
	bool UseWholeScreen = false;      // 画面をすべて使って表示
	bool FullScreen = false;          // フルスクリーンかどうか
	bool Locked = false;              // ロック
	bool SearchSubFolder = true;     // サブフォルダも検索するかどうか
	bool LoadLastFile = false;        // 最後の画像を次回に読み込むかどうか
	bool HideTaskButton = true;      // タスクボタンを未表示にするかどうか
	bool FixDiagonalLength = false;              // 一定倍率で表示
	int KeepDiagonalLength = 0;         // 一定表示時の対角線の２乗
	double MinimalDiagonalLength = 50 * 50 * 2; // 最小の表示対角線の２乗
//	double MaxLength = 40000;
	double MaxSizeRatio = 200;
	bool KeepPreviousPosition = false;           // カーソル位置から開始しない
	double MinColorONColorArea = DBL_MAX; // 高速伝送モード (COLONCOLOR) で転送する最小の面積
	//-----------------------------------------------------------------------------

	//-----------------------------------------------------------------------------
	int CenterX = 100;              // 画像の中央Ｘ
	int CenterY = 100;              // 画像の中央Ｙ
	int TrayIconColor = 0;        // トレイアイコンの表示番号
	EPositionMode PositionMode = EPositionMode_FREE;                   // 表示位置 0:自由 1:中央 2:左上 3:右上 4:左下 5:右上
	bool FixRotate = false;                     // 回転を固定するかどうか
	int RotateValue = 0;          // 回転角度
	int WheelSensitivity = 120;           // ホイール感度
	int FrameWidth = 0;                     // フレームの幅
	Gdiplus::Color FullFillColor = Color(255, 0, 0, 0);         // フルスクリーン時に塗りつぶす色
	Gdiplus::Color FrameColor = Color(255, 255, 255, 255);    // フレームの塗りつぶし色
	Gdiplus::Color DrawColor = Color(255, 0, 0, 0);    // フレームの塗りつぶし色
	COLORREF StockColor[16] = { 0 };
	//-----------------------------------------------------------------------------

	//-----------------------------------------------------------------------------
	std::wstring ShortCutFileName = TEXT(""); // 作成したショートカットのファイル名
	bool CreateSendToLink = false;    // SendTo にリンクを作成したかどうか
	std::wstring FileMaskString = TEXT("*.*");   // ファイルマスク
	bool EnableFileMask = false;                  // ファイルマスクを有効にするかどうか
	bool AutoLoadFileFolder = true;                // 単独画像表示時に次のファイルを表示しようとするとフォルダを自動で読み込む
	//-----------------------------------------------------------------------------


	int ApplicationFontSize = 7; // ファイル一覧のリストボックスのフォントサイズ
	int FontSizeIndex;
	int FormAlphaBlendValue = 255;

	int Left, Top, Right, Bottom; // 実ウィンドウの左、上
	int Width = 200, Height = 200; // 実ウィンドウの幅、高さ
	int WLeft = 0, WTop = 0;     // 仮想ウィンドウの左、上
	int WWidth = 200, WHeight = 200; // 仮想ウィンドウの幅、高さ
	int RLeft = 0, RTop = 0;   // 実際のウィンドウの左、上
	int RWidth = 200, RHeight = 200; // 実際のウィンドウの幅、高さ
	bool Visible = true;
	Gdiplus::Rect Desktop;
	Gdiplus::Rect ClientRect; // ウィンドウ矩形
	bool ReachMinDL = false; // 最小ウィンドウサイズに達した画像かどうか

	int MLeft, MTop;        // ウィンドウが存在するモニタの左と上
	int MWidth, MHeight;    // ウィンドウが存在するモニタの幅と高さ
	int MRight, MBottom;

	int JCR[4] = { 85, 85, 50, 30 };                         // Jpeg 保存時の圧縮率の設定

	//-----------------------------------------------------------------------------

	int StopCount = 0;
	int FileSearching = 0;
	int ShowingDialog = 0;
	int FileLoading = 0;
	time_t LoadStart = 0;
	int LoadState = 0;

	double ProgressRatio = 0;

	bool ShowFromShowIndex = false; // ini ファイル読み込みで Index が 0 以外から開始される場合に設定

	bool AnimeRefresh = false;
	bool EnableDropFrame = true;
	bool AnimePaused = false;
	bool AnimeTimerPaused = false;
	int ThreadCount = 0;

	std::vector<std::wstring> HistoryList;
	int MaxHistoryNum = DEFAULT_MAXHISTORYNUM;
	int RefreshHistoryMode = 1;

	time_t IniFileTime;

#define TRAYICONNUM 6
#define SLIDESHOWNUM 15
#define FRAMEWIDTHNUM 7
#define POSITIONMODENUM 6
#define OFFSETROTATENUM 2
#define ABSOLUTEROTATENUM 4
#define WHEELSENSENUM 3
#define FONTSIZENUM 10
#define ALPHABLENDNUM 4
#define SORTFILTERLISTNUM 6
#define CURSORSIZENUM 3

	DWORD mnTrayIcons[6] = { ID_TRAYICONCOLOR_GREEN, ID_TRAYICONCOLOR_RED, ID_TRAYICONCOLOR_BLUE, ID_TRAYICONCOLOR_YELLOW, ID_TRAYICONCOLOR_WHITE, ID_TRAYICONCOLOR_BLACK };
	DWORD mnSlideShows[15] = { ID_SLIDESHOWINTERVAL_1S, ID_SLIDESHOWINTERVAL_2S, ID_SLIDESHOWINTERVAL_5S, ID_SLIDESHOWINTERVAL_10S, ID_SLIDESHOWINTERVAL_30S,
													ID_SLIDESHOWINTERVAL_1M, ID_SLIDESHOWINTERVAL_5M, ID_SLIDESHOWINTERVAL_15M, ID_SLIDESHOWINTERVAL_30M,
													ID_SLIDESHOWINTERVAL_1H, ID_SLIDESHOWINTERVAL_2H, ID_SLIDESHOWINTERVAL_3H, ID_SLIDESHOWINTERVAL_6H, ID_SLIDESHOWINTERVAL_12H, ID_SLIDESHOWINTERVAL_24H };
	DWORD mnFrameWidths[7] = { ID_BORDERWIDTH_0PX, ID_BORDERWIDTH_1PX, ID_BORDERWIDTH_5PX, ID_BORDERWIDTH_10PX, ID_BORDERWIDTH_15PX, ID_BORDERWIDTH_20PX, ID_BORDERWIDTH_30PX };
	DWORD mnPositionModes[6] = { ID_SHOWPOSITION_NONE, ID_SHOWPOSITION_CENTER, ID_SHOWPOSITION_LEFTTOP, ID_SHOWPOSITION_RIGHTTOP, ID_SHOWPOSITION_LEFTBOTTON, ID_SHOWPOSITION_RIGHTBOTTOM };
	DWORD mnOffsetRotate[2] = { ID_ROTATE_TURNRIGHT, ID_ROTATE_TURNLEFT };
	DWORD mnAbsoluteRotate[4] = { ID_ROTATE_NONE, ID_ROTATE_TURNEDRIGHT, ID_ROTATE_UPSIDEDOWN, ID_ROTATE_TRUNEDLEFT };
	DWORD mnWheelSense[3] = { ID_WHEELSENSITIVITY_DEFAULT, ID_WHEELSENSITIVITY_MIDDLE, ID_WHEELSENSITIVITY_LOW };
	DWORD mnFontSizes[10] = { ID_FONTSIZE_6PT, ID_FONTSIZE_7PT, ID_FONTSIZE_8PT, ID_FONTSIZE_9PT, ID_FONTSIZE_10PT, ID_FONTSIZE_11PT, ID_FONTSIZE_13PT, ID_FONTSIZE_15PT, ID_FONTSIZE_17PT, ID_FONTSIZE_19PT };
	DWORD mnAlphaBlends[4] = { ID_TRANSPARENCY_100, ID_TRANSPARENCY_75, ID_TRANSPARENCY_50, ID_TRANSPARENCY_25 };
	DWORD mnSortFileList[6] = { ID_SORTFILELIST_BYNAME, ID_SORTFILELIST_BYEXTENSION, ID_SORTFILELIST_BYTIMESTAMP, ID_SORTFILELIST_BYFILESIZE, ID_SORTFILELIST_RANDOMIZE, ID_SORTFILELIST_REVERSE };
	DWORD mnCursorSizes[3] = { ID_CURSORSIZE_NORMAL, ID_CURSORSIZE_LARGE, ID_CURSORSIZE_XLARGE };

	int TrayIcons[6] = { 0, 1, 2, 3, 4, 5 };
	int SlideShows[15] = { 1, 2, 5, 10, 30, 60, 5 * 60, 15 * 60, 30 * 60, 60 * 60, 2 * 60 * 60, 3 * 60 * 60, 6 * 60 * 60, 12 * 60 * 60, 24 * 60 * 60 };
	int FrameWidths[7] = { 0, 1, 5, 10, 15, 20, 30 };
	int PositionModes[6] = { EPositionMode_FREE, EPositionMode_CENTER, EPositionMode_LEFTTOP, EPositionMode_RIGHTTOP, EPositionMode_LEFTBOTTOM, EPositionMode_RIGHTBOTTOM };
	int AbsoluteRotates[5] = { 0, 1, 2, 3, 4 };
	int WheelSensitivities[3] = { 120, 720, 1440 };
	int FontSizes[10] = { 6, 7, 8, 9, 10, 11, 13, 15, 17, 19 };
	int AlphaBlends[4] = { 255, 192, 128, 64 };
	int UseCursorSizes[3] = { 0, 1, 2 };

	int OffsetRotates[2] = { -1, 1 };

	ESortType SortTypes[6] = { ESortType_NAME, ESortType_EXT, ESortType_TIMESTAMP, ESortType_FILESIZE, ESortType_RANDOM, ESortType_REVERSE };

	HICON hTrayIcon[6];

	unsigned int ThreadID;
	CRITICAL_SECTION CriticalSection;
	HANDLE hThread = nullptr;
	bool TerminateGIFAnime = false;

	//---------------------------------------------------------

	//---------------------------------------------------------
	HCURSOR hCursor[3] = { 0 };
	//---------------------------------------------------------

	//---------------------------------------------------------
public:
	CMainForm(void);
	~CMainForm(void);

	// フォームを閉じる

	void Close(void);
		

	// 初期化

	bool Initialize(HINSTANCE hInstance, int nCmdShow, LPWSTR lpCmdLine); // フォームを作る
	void CheckExistsFileListCorrect(void);
	void ShowIndexRangeInFileList(void);
	void ShowArchiveRangeInFileList(void);
	void ShowArchiveRangeInArchiveList(void);
	bool CreateForm(int nCmdShow); // フォームの初期化など

	ATOM MyRegisterClass(void);

	void PrepareDialog(void); // ダイアログを準備する
	void GetLanguageInfo(void); // 言語情報を取得する
	void LoadResource(void); // リソースをメモリに読み込む 

	Gdiplus::Color GetDrawColor(Gdiplus::Color color);

	void CreateFromMessage(HWND hwnd, LPCREATESTRUCT lp);// フォームが作られたときに呼ばれる
	void CreateDisplayBox(HWND hwnd, LPCREATESTRUCT lp); // ファイルを表示するリストを作成する
	
	void CloseFromMessage(void);

	void SetTrayIcon(ESetTrayIconMode Mode); // トレイアイコンを登録する

	// Ini ファイル関係
	 

	bool SetInstanceMode(std::wstring NewIniParamName);
	bool LoadIni(std::wstring IniName, bool CmdLine);

	void SyncMenuChecked(void);
	int GetMenuIndexByValue(int *ValueList, int Value, int Length);

	bool SaveIni(std::wstring IniName);
	__time64_t GetCreationTime(std::wstring FileName);

	bool SaveFileList(std::wstring FileName); // ファイルリストを保存する
	bool LoadFileList(std::wstring FileName, std::vector<CImageInfo> &DestLists, int &NewIndex, int &NewSubIndex, std::wstring &sFileName, std::wstring &sArchiveName);
	bool CreateFileList(std::wstring &sb);
	bool ReadFileList(std::vector<CImageInfo>& DestSL, std::map<std::wstring, std::wstring>& Map);
	
	// ヒストリーメニュー操作

	bool AddHistoryList_(std::wstring &FileName);
	bool AddHistoryList(std::wstring &FileName);
	bool AddHistoryList(std::vector<std::wstring> &FileNames);
	bool CreateHistoryMenu(void);
	bool ChangeHistoryName(std::wstring Src, std::wstring Dest);

	
	// インストール・アンインストール関係

	bool DoInstall(void);
	bool CreateShortCut(std::wstring InstanceName);
	bool DoCreateSendToLink(std::wstring InstanceName);
	bool DoUninstall(void);
	bool DeleteSendToLink(std::wstring InstanceName);

	// メッセージ処理

	LRESULT ProcessMessages(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool &CallDefault);
	bool MainForm_KeyDown(WPARAM wParam, LPARAM lParam);
	LRESULT ProcessMessagesListBox(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool &CallDefault);
	bool DisplayBox_KeyDown(WPARAM wParam, LPARAM lParam);
	bool ProcessAppCommand(LPARAM cmd, LPARAM uDevicem, LPARAM dwKeys);
	LRESULT SpiProgressCallBack(int nNum, int nDenom, long lData);

	// デスクトップ矩形を得る

	bool CheckMonitorIni(void); // モードによってスクリーンの状態を得てウィンドウの状態を変更する
	void GetDesktopRect(void); // デスクトップ領域を得る
	void SetMonitorRect(LPRECT r); // デスクトップ領域を得る関数の補助


	// 便利関数

	std::wstring GetTabString(std::wstring Src, std::wstring Name);
	int IndexOfImageInfos(std::vector<CImageInfo>* Src, std::wstring FileName);

	std::wstring LoadStringResource(UINT uID); // リソースから文字列読み込み
	LPWSTR LoadStringBuffer(UINT uID); // メモリ文字列を確保してリソースから読み込み
	LPWSTR StringBuffer(TCHAR *src); // メモリ文字列を確保する

	// ウィンドウの状態変更
	
	void SetTaskButtonVisibility(void);
	void NoStayOnTop(void);
	void RestoreStayOnTop(void);
	void PauseAnimeThread(void);
	void RestartAnimeThread(void);

	void ResetTransMode(void); // 高速伝送モードの設定を初期化する
	void CalclurateMaxSizeRatio(void);

	std::wstring GetFileVersionString(void); // ファイルのバージョン文字列を取得する

	// 描画関係

	void DoPaint(bool FullRefresh); // 描画イベント
	void DoFillBGColor(void);
	bool GetTransRect(Gdiplus::Rect &wRect, Gdiplus::Rect &Src, Gdiplus::Rect &Dest); // 転送矩形を得る
	void DrawSSIcon(void); // スライドショーアイコンを描画する
	void ClearSSIcon(void); // スライドショーアイコンを消去する
	void BeginUpdate(void); // 描画を開始する（再入回避）
	void EndUpdate(void); // 描画を終了する
	void FormRefresh(void); // フォームを再描画する


	// フォームサイズ変更などマウスイベント

	void OnMouseMove(void); // マウスがフォーム上を移動
	void OnMouseDown(EEventButton button); // マウスダウン
	void OnMouseUp(EEventButton button); // マウスアップ
	void DoMouseUp(void);
	void MouseLeave(void); // マウスがウィンドウ外に移動
	void MouseEnter(void); // マウスがウィンドウ内に移動
	bool SetCenter(int iX, int iY);
	bool GetMonitorParameter(void);


	// フォームのサイズ移動等ウィンドウサイズと位置関係

	bool SyncWindow(void); // 実際のフォームの位置高さを設定する
	bool CheckRefreshBackBuffer(void); // フォームのサイズが変わったときにバックバッファの大きさを変更する
	void GetRealWindowRect(void);
	bool FixViewOut(void); // サイズ固定モードから出る
	void RefreshWindowPosition(int X, int Y); // 仮想ウィンドウの位置を更新する


	// カーソルを設定する

	EMousePositionType GetCursorPositionType(POINT &ScCurPos);
	bool SetCursorMode(EMousePositionType Result);


	// ホイールイベント

	void OnMouseWheel(int delta); // マウスホイール


	// メニュー編集

	void SetMenuCheckType(HMENU hmenu, UINT ItemID, int Type);
	void SetMenuCheck(HMENU hmenu, UINT ItemID, bool Checked);
	void SetMenuEnabled(HMENU hmenu, UINT ItemID, bool Enabled);
	void SetMenuText(HMENU hmenu, UINT ItemID, std::wstring src);


	// コンテキストメニューボタンを押した

	void ShowContextMenu(void);

	// ポップアップメニュー表示

	void PopupMenuPopup(POINT &p);
	void PopupFileMovePopup(POINT &p);
	void CreateFileMovePPMenu(void);
	void CreateFileMoveMenuFolder(std::wstring FolderName);
	std::wstring CreateFolder(void);


	// アルファのメニューのチェックのオンオフをコントロールする

	void CheckAlphaValueMenuCheck(void);


	// 画像を回転する

	void AbsoluteRotate(int Value);
	void OffsetRotate(int Value);
	void RotateImage(void);


	// 画像をズームする

	bool ZoomImage(double NewPercentage);
	bool ZoomOrgPositionDelta(double Delta, int X, int Y);
	bool ZoomOrgPosition(double NewPercentage, int X, int Y);
	

	// 新しい画像を読み込んだときにウィンドウサイズを決定する

	void SetNewImageSize(void); 


	// 対角線サイズ等画面の大きさの制限をチェックする

	bool SetDiagonalLength(void); // 対角線サイズを計算する
	bool CheckDiagonalLength(double &dWidth, double &dHeight); // 対角線の長さの最小値を保つ
	bool CheckMinimalDiagonalLength(int &iWidth, int &iHeight); // 最小対角線サイズをキープする
	bool CorrectWindow(void); // 画面の中に少しでも入るように表示を変更する


	// ウィンドウの位置と大きさを決める

	void SetWindowSize(int iWidth, int iHeight, bool InMonitor);
	bool MoveByPositionMode(void);
	bool SetPositionMode(int NewPositionMode);
	bool SetDisableFitMode(int aMode); // フィットモード解除
	bool GoToFullScreen(bool aMode); // フルスクリーンモード
	void AlphaValueOffset(int i); // アルファの値を相対値で指定
	

	bool SetFileList(void);	// ファイルリストを設定する
	std::wstring GetImageFileFolder(void);
	void SetImageFileName(void);
	bool ChangeImageFileName(std::wstring src);
	bool RenameImageFile(std::wstring newName);

	
	// ファイルを開く
	
	bool OpenFiles(std::vector<std::wstring> &SrcList);
	bool OpenFiles(std::vector<std::wstring> &SrcList, std::wstring SelectedFile, int Offset, bool AddMode);
	bool CheckGetList(std::vector<CImageInfo>& DestList, std::vector<std::wstring>& DropList, int Dir, int & NewIdx, int & NewSubIdx, std::wstring & LoadedmflFile);
	bool GetImageList(std::wstring Src, std::vector<CImageInfo>& Dest, bool SubFolder, bool EnableFileMask, std::wstring FileMaskString);
	bool AddFileList(std::vector<CImageInfo>& SrcList, int Mode);
	bool DeleteFileList(int DeleteMode);
	
	// ファイルリスト操作
	
	bool DeleteFileInList(int i);
	bool MoveSelectedList(int Offset);
	bool SortFileList(ESortType Type);

	EOpenFileResult OpenFile(CImageInfo & imageInfo);
	bool OpenArchiveMode(CImageInfo & SrcImageInfo, int SubIndex, int Dir, bool MustShowImage);
	bool CloseArchiveMode(void);

	void ToggleShowList(EShowMode Mode);
	void ToggleShowList(EShowMode Mode, int Dir);

	ELoadFileResult LoadFile(int Index, int SubIndex, int Dir, bool MustShowImage);

	
	// 表示位置調整

	bool AdjustShowIndex(int Dir);
	bool JumpBorderArcNml(int Dir);
	bool ShowOffsetImage(int Offset);
	EShowAbsoluteImageResult ShowAbsoluteImage(int Index, int Dir);
	EShowAbsoluteImageResult ShowAbsoluteImage(int Index, int Dir, bool MustShowImage);
	EShowAbsoluteImageResult ShowAbsoluteImage(int Index, int SubIndex, int Dir, bool MustShowImage);

	// スライドショー
	
	void IntervalOffset(int i);

	
	// GIF アニメ
	
	void AnimeThread(void);
	void SusieClear(EPluginMode Mode);
	void BeginAnimeThread(void);
	void EndAnimeThread(void);


	// メニュー操作

	void MnToggleVisible_Click(void);

	void MnFitToScreen_Click(void); // 画面に合わせて表示
	void MnFullScreen_Click(void); // フルスクリーンモード
	void MnUseWholeScreen_Click(void); // すべての画面を使って表示
	void MnInScreen_Click(void); // 画面の中に表示する
	void MnShowPbyP_Click(void); // ピクセル等倍で表示
	void MnCenter_Click(void); // 中央に移動

	void MnPositionMode_Click(int Index); // 表示する位置を設定
	void MnRotateMode_Click(int Index);
	void MnRotateOffset_Click(int Index); // 画像を回転
	void MnTransparency_Click(int Index);
	void MnHisotryMenu_Click(int Index);
	void MnSortFileList_Click(int Index);
	void MnSlideShowInterval_Click(int Index);
	void MnWheelSensitivity_Click(int Index);

	void MnRotateFix_Click(void);
	void MnAlwaysTop_Click(void);
	void MnFixDiagonalLength_Click(bool SetNewValue);
	void MnFixRatio_Click(void);
	void MnSetWindowSize_Click(void);
	void MnSlideShow_Click(int Mode = 2);
	void MnRefresh_Click(void);
	void MnLock_Click(void);
	void MnOpenFile_Click(void);
	void MnOpenFolder_Click(void);
	void MnLoadFolderExistingShowingFile_Click(void);
	void MnCloseArchive_Click(void);
	void MnToggleShow_Click(void);
	void MnShowInformation_Click(void);
	void MnCopyImage_Click(void);
	void MnFileCopy_Click(void);
	void MnFileCut_Click(void);
	void MnFilePaste_Click(void);
	void MnFileRename_Click(void);
	void MnCopyFilePath_Click(void);
	void MnMoveFile_Click(void);
	void MnFileMoveToRecycle_Click(bool FromMenu);
	void MnOpenExistsFolder_Click(void);
	void MnJpegSave_Click(void);
	void MnJpegSaveShowingSize_Click(void);
	void MnJpegSaveSetting_Click(void);
	void MnSavePNG_Click(void);
	void MnSearchSubFolders_Click(void);
	void MnKeepPreviousFiles_Click(void);
	void MnEnableFileMask_Click(void);
	void MnFileMaskSetting_Click(void);
	void MnAutoLoadFileFolder_Click(void);
	void MnKeepPreviousPosition_Click(void);
	void MnBackGroundColor_Click(void);
	void MnHideTaskButton_Click(void);
	void MnBorder_Click(int Index);
	void MnBorderColor_Click(void);
	void MnFontSize_Click(int Index);
	void MnTrayIcon_Click(int Index);
	void MnCursorSize_Click(int Index);
	void MnFileMove_Click(int Index);
	void MnSetPluginFolder_Click(void);
	void MnOpenPluginDialog_Click(void);
	void MnSave_Click(void);
	void MnInternalLoader_Click(void);
	void MnCreateInstance_Click(void);
	void MnDeleteInstance_Click(void);
	void MnUninstall_Click(void);
	void MnQuit_Click(void); // 終了する

	void DisplayBox_DoubleClick(void);
	void MainForm_MouseDoubleClick(void);
	void TrayIcon_MouseClick(EEventButton button);
	void CursorTimer_Tick(void);
	void SSTimer_Tick(void);
	void File_DragDrop(HDROP hDrop);

	void CheckPositionModeMenuCheck(void);
	void CheckRotateCheck(void);
	void CheckIntervalMenuCheck(void);
	void CheckFontSizeMenuCheck(void);
	void CheckFrameWidthMenuCheck(void);
	void CheckWheelSenseCheck(void);
	void CheckUseCursorSizeCheck(void);


};

extern CMainForm *MainForm;