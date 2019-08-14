#pragma once

#include "windows.h"
#include "spiplugin.h"
#include "resource.h"
#include <set>
#include <map>
#include "acfc.h"

#define MAX_LOADSTRING 100
#define MIV_SMALLESTWINDOWSIZE 200
#define CULTURESTR "en-US"
#define MIN_LOOP 50

using namespace milligram;

class CMainForm
{
	enum EEventButton : int
	{
		EEventButton_NONE = 0,
		EEventButton_LEFT = 1,
		EEventButton_RIGHT = 2,
		EEventButton_MIDDLE = 4,
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
	};

	enum EOpenFileResult : int
	{
		EOpenFileResult_SUCCEEDED = 0,
		EOpenFileResult_LOADFAILED = 1,
		EOpenFileResult_EXCLUDEBYMASK = 2,
		EOpenFileResult_NOARCHIVEFILE = 3,
	};

	enum EFitMode : int
	{
		EFitMode_NONE = 0,
		EFitMode_FIT_TOPANDBOTTOM = 1,
		EFitMode_FIT_LEFTANDRIGHT = 2,
	};

	enum EPositionMode : int // 0:���R 1:���� 2:���� 3:�E�� 4:���� 5:�E��
	{
		EPositionMode_FREE = 0,
		EPositionMode_CENTER = 1,
		EPositionMode_LEFTTOP = 2,
		EPositionMode_RIGHTTOP = 3,
		EPositionMode_LEFTBOTTOM = 4,
		EPositionMode_RIGHTBOTTOM = 5,
	};

	enum EShowMode : int // Mode 0:���] 1:���X�g�\�� 2:�摜�\�� 3:�����摜�\��
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

	HWND hWindow = nullptr; // �t�H�[���̃E�B���h�E�n���h��
	HFONT hFont = nullptr; // �\���t�@�C�����X�g�̃t�H���g�I�u�W�F�N�g
	HMENU hParentPopupMenu = nullptr; // ���ׂẴ��j���[
	HMENU hPopupMenu = nullptr; // �|�b�v�A�b�v���j���[�n���h��
	HMENU hHistoryMenu = nullptr; // �q�X�g�����j���[�n���h��
	HMENU hParentMoveMenu = nullptr; // �q�X�g�����j���[�n���h��
	HMENU hMoveMenu = nullptr; // �t�@�C���ړ����j���[�n���h��
	bool Active = true; // �t�H�[�����A�N�e�B�u���ǂ���
	bool MouseCapturing = false; // �}�E�X���L���v�`�������ǂ���

	acfc::CListBox DisplayBox;

	acfc::COpenFileDialog OpenFileDialog;
	acfc::COpenFileDialog OpenPluginDialog;

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

	CSpiLoader Susie;

	//---------------------------------------------------------

	std::vector<CImageInfo> FileList;
	std::vector<CImageInfo> *DisplayList;
	std::vector<std::wstring> SpiPathes;

	std::wstring IniParamName = TEXT("milligram");
	std::wstring BalloonTipTitle = TEXT("milligram image viewer");
	std::wstring IniFolderName = TEXT("");
	std::wstring ExeFileName = TEXT("");

	std::set<std::wstring> StrBuf; // �����񃊃\�[�X�ۊǏꏊ
	std::set<std::wstring> MoveBuf; // File Move PP ���j���[�̕�����o�b�t�@
	std::vector<std::wstring> MoveData; // FileMove PP �̈ړ��Ή���t���p�X������

	//-----------------------------------------------------------------------------

	POINT OfsCurPos;          // �}�E�X�_�E�������Ƃ��� Left Top �̈ʒu
	POINT DownCurPos;         // �}�E�X�_�E�������Ƃ��̉�ʏ�̈ʒu
	POINT PreCurPos;          // �}�E�X�_�E�����̈�O�̉�ʏ�̈ʒu
	POINT PreCurTimerPos;           // ��O�̃J�[�\���ʒu 
	POINT MovePPPosition; // Move �|�b�v�A�b�v���j���[�̈ʒu
	int UseCursorSize = 0; // �傫�ȃJ�[�\�����g�����ǂ���

	int DownWidth;            // �}�E�X�_�E�������Ƃ��̕�
	int DownHeight;           // �}�E�X�_�E�������Ƃ��̍���
	int DownLeft;               // �}�E�X�_�E�������Ƃ��̕�
	int DownTop;                // �}�E�X�_�E�������Ƃ��̍���

	EEventButton  Holding = EEventButton_NONE;              // �}�E�X�z�[���h���Ă��邩�ǂ��� 0:���ĂȂ� 1:�� 2:�E 3:�^��
	EMousePositionType PositionType = EMousePositionType_NONE;         // �}�E�X�_�E�������Ƃ��̃}�E�X�̈ʒu
	EMousePositionType CursorMode = EMousePositionType_NONE; // ���݂̃J�[�\��
	//-----------------------------------------------------------------------------

	//-----------------------------------------------------------------------------
	double Ratio = 1.0;             // �摜�̔�
	double SizeRatio = 1.0;        // �g�嗦
	EFitMode FitMode = EFitMode_NONE;                            // 1:�㉺���z�� 2:���E���z��
	int WheelPos = 0;             // �z�C�[���l
	int StartnNum = -1;                    // �����ʒu
	//-----------------------------------------------------------------------------

	//-----------------------------------------------------------------------------
	bool NotSaveIni = false;          // Ini �t�@�C�����I�����ɕۑ����邩�ǂ���
	bool InstanceMode = false;        // �C���X�^���X���[�h�œ��쒆���ǂ���
	//-----------------------------------------------------------------------------

	//-----------------------------------------------------------------------------
	int EnableDraw = 0;                    // �ĕ`��̗}��
	//-----------------------------------------------------------------------------

	//-----------------------------------------------------------------------------
	bool InArchive = false;                     // �A�[�J�C�u�t�@�C�����J���Ă��邩�ǂ���
	//-----------------------------------------------------------------------------

	//-----------------------------------------------------------------------------
	bool ReloadMode = false;            // �����N�����ɑ傫���ʒu��ێ����邽�߂Ɏg��
	bool RClkCancel = false;                    // �E�N���b�N���j���[���L�����Z�����邩�ǂ���
	int SSIcon = -1;               // �X���C�h�V���[�A�C�R���̃��[�h
	bool SSChangeImage = false;                     // �}�E�X�̈ړ��𖳎�����
	//-----------------------------------------------------------------------------

	//-----------------------------------------------------------------------------
	int ShowIndex = -1;            // �\�����̉摜�̔ԍ�
	int ShowIndexBack = -1;         // �\�����̉摜�̔ԍ�(�T�u)
	bool AlwaysTop = false;           // ��Ɏ�O�ɕ\��
	bool ShowingList = false;         // �t�@�C�����X�g�\�������ǂ���
	bool PreShowingList = false;         // �O��I�����Ƀt�@�C�����X�g�\�������ǂ���
	bool SlideShow = false;           // �X���C�h�V���[���[�h
	int SSInterval = 5000;           // �X���C�h�V���[�̃C���^�[�o��
	UINT SSId = 1;					// �^�C�}�[�� ID
	bool FixSizeRatio = false;       // �g�嗦�Œ�
	bool FitToScreen = false;         // ��ʂɍ��킹�ĕ\��
	bool UseWholeScreen = false;      // ��ʂ����ׂĎg���ĕ\��
	bool FullScreen = false;          // �t���X�N���[�����ǂ���
	bool Locked = false;              // ���b�N
	bool SearchSubFolder = false;     // �T�u�t�H���_���������邩�ǂ���
	bool LoadLastFile = false;        // �Ō�̉摜������ɓǂݍ��ނ��ǂ���
	bool HideTaskButton = true;      // �^�X�N�{�^���𖢕\���ɂ��邩�ǂ���
	bool FixDiagonalLength = false;              // ���{���ŕ\��
	int KeepDiagonalLength = 0;         // ���\�����̑Ίp���̂Q��
	double MinimalDiagonalLength = 50 * 50 * 2; // �ŏ��̕\���Ίp���̂Q��
	double MaximumDiagonalLength = 10000 * 10000 * 2;
	double MaxSizeRatio = 200;
	bool KeepPreviousPosition = false;           // �J�[�\���ʒu����J�n���Ȃ�
	//-----------------------------------------------------------------------------

	//-----------------------------------------------------------------------------
	int CenterX = 100;              // �摜�̒����w
	int CenterY = 100;              // �摜�̒����x
	int TrayIconColor = 0;        // �g���C�A�C�R���̕\���ԍ�
	EPositionMode PositionMode = EPositionMode_FREE;                   // �\���ʒu 0:���R 1:���� 2:���� 3:�E�� 4:���� 5:�E��
	bool FixRotate = false;                     // ��]���Œ肷�邩�ǂ���
	int RotateValue = 0;          // ��]�p�x
	int WheelSensitivity = 120;           // �z�C�[�����x
	int FrameWidth = 0;                     // �t���[���̕�
	Gdiplus::Color FullFillColor = Color(255, 0, 0, 0);         // �t���X�N���[�����ɓh��Ԃ��F
	Gdiplus::Color FrameColor = Color(255, 255, 255, 255);    // �t���[���̓h��Ԃ��F
	Gdiplus::Color DrawColor = Color(255, 0, 0, 0);    // �t���[���̓h��Ԃ��F
	COLORREF StockColor[16] = { 0 };
	//-----------------------------------------------------------------------------

	//-----------------------------------------------------------------------------
	std::wstring ShortCutFileName = TEXT(""); // �쐬�����V���[�g�J�b�g�̃t�@�C����
	bool CreateSendToLink = false;    // SendTo �Ƀ����N���쐬�������ǂ���
	std::wstring FileMaskString = TEXT("*.*");   // �t�@�C���}�X�N
	bool EnableFileMask = false;                  // �t�@�C���}�X�N��L���ɂ��邩�ǂ���
	bool AutoLoadFileFolder = false;                // �P�Ɖ摜�\�����Ɏ��̃t�@�C����\�����悤�Ƃ���ƃt�H���_�������œǂݍ���
	//-----------------------------------------------------------------------------


	int ApplicationFontSize = 7; // �t�@�C���ꗗ�̃��X�g�{�b�N�X�̃t�H���g�T�C�Y
	int FontSizeIndex;
	int FormAlphaBlendValue = 255;

	int Left, Top, Right, Bottom; // ���E�B���h�E�̍��A��
	int Width = 200, Height = 200; // ���E�B���h�E�̕��A����
	int WLeft = 0, WTop = 0;     // ���z�E�B���h�E�̍��A��
	int WWidth = 200, WHeight = 200; // ���z�E�B���h�E�̕��A����
	int RLeft = 0, RTop = 0;   // ���ۂ̃E�B���h�E�̍��A��
	int RWidth = 200, RHeight = 200; // ���ۂ̃E�B���h�E�̕��A����
	bool Visible = true;
	Gdiplus::Rect Desktop;
	Gdiplus::Rect ClientRect; // �E�B���h�E��`
	bool ReachMinDL = false; // �ŏ��E�B���h�E�T�C�Y�ɒB�����摜���ǂ���

	int MLeft, MTop;        // �E�B���h�E�����݂��郂�j�^�̍��Ə�
	int MWidth, MHeight;    // �E�B���h�E�����݂��郂�j�^�̕��ƍ���
	int MRight, MBottom;

	int JCR[4] = { 85, 85, 50, 30 };                         // Jpeg �ۑ����̈��k���̐ݒ�

	//-----------------------------------------------------------------------------

	int StopCount = 0;
	int FileLoading = 0;
	time_t LoadStart = 0;
	int LoadState = 0;

	double ProgressRatio = 0;

	std::wstring OpeningFileName = TEXT("");
	bool ShowFromShowIndex = false;

	bool GIFRefresh = false;
	bool EnableDropFrame = true;
	bool GIFAnimePaused = false;

	std::vector<std::wstring> HistoryList;
	int MaxHistoryNum = 16;
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

	// �t�H�[�������

	void Close(void);
		

	// ������

	bool Initialize(HINSTANCE hInstance, int nCmdShow, LPWSTR lpCmdLine); // �t�H�[�������
	bool CreateForm(int nCmdShow); // �t�H�[���̏������Ȃ�

	ATOM MyRegisterClass(void);

	void PrepareDialog(void); // �_�C�A���O����������

	void LoadResource(void); // ���\�[�X���������ɓǂݍ��� 

	Gdiplus::Color GetDrawColor(Gdiplus::Color color);

	void CreateFromMessag(HWND hwnd, LPCREATESTRUCT lp);// �t�H�[�������ꂽ�Ƃ��ɌĂ΂��
	void CreateDisplayBox(HWND hwnd, LPCREATESTRUCT lp); // �t�@�C����\�����郊�X�g���쐬����
	
	void CloseFromMessage(void);

	void SetTrayIcon(ESetTrayIconMode Mode); // �g���C�A�C�R����o�^����

	// Ini �t�@�C���֌W
	 

	bool SetInstanceMode(std::wstring NewIniParamName);
	bool LoadIni(std::wstring IniName, bool CmdLine);

	void SyncMenuChecked(void);
	int GetMenuIndexByValue(int *ValueList, int Value, int Length);

	bool SaveIni(std::wstring IniName);
	__time64_t GetCreationTime(std::wstring FileName);
	std::wstring CreateFileList(void);

	bool SaveFileList(std::wstring FileName); // �t�@�C�����X�g��ۑ�����
	bool LoadFileList(std::vector<CImageInfo>& DestSL, std::map<std::wstring, std::wstring>& Map);
	
	// �q�X�g���[���j���[����

	bool AddHistoryList_(std::wstring &FileName);
	bool AddHistoryList(std::wstring &FileName);
	bool AddHistoryList(std::vector<std::wstring> &FileNames);
	bool ConvertHistoryMenu(void);

	
	// �C���X�g�[���E�A���C���X�g�[���֌W

	bool DoInstall(void);
	bool CreateShortCut(std::wstring InstanceName);
	bool DoCreateSendToLink(std::wstring InstanceName);
	bool DoUninstall(void);
	bool DeleteSendToLink(std::wstring InstanceName);

	// ���b�Z�[�W����

	LRESULT ProcessMessages(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool &CallDefault);
	void MainForm_KeyDown(WPARAM wParam, LPARAM lParam);
	LRESULT ProcessMessagesListBox(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool &CallDefault);
	void DisplayBox_KeyDown(WPARAM wParam, LPARAM lParam);
	LRESULT SpiProgressCallBack(int nNum, int nDenom, long lData);

	// �f�X�N�g�b�v��`�𓾂�

	bool CheckMonitorIni(void); // ���[�h�ɂ���ăX�N���[���̏�Ԃ𓾂ăE�B���h�E�̏�Ԃ�ύX����

	void GetDesktopRect(void); // �f�X�N�g�b�v�̈�𓾂�
	void SetMonitorRect(LPRECT r); // �f�X�N�g�b�v�̈�𓾂�֐��̕⏕


	// �֗��֐�

	std::wstring GetTabString(std::wstring Src, std::wstring Name);
	int IndexOfImageInfos(std::vector<CImageInfo>* Src, std::wstring FileName);

	std::wstring LoadStringResource(UINT uID); // ���\�[�X���當����ǂݍ���
	LPWSTR LoadStringBuffer(UINT uID); // ��������������m�ۂ��ă��\�[�X����ǂݍ���
	LPWSTR BufferString(TCHAR *src); // ��������������m�ۂ���

	// �E�B���h�E�̏�ԕύX
	
	void SetTaskButtonVisibility(void);
	void NoStayOnTop(void);
	void RestoreStayOnTop(void);

	std::wstring GetFileVersionString(void); // �t�@�C���̃o�[�W������������擾����

	// �`��֌W

	void OnPaint(void); // �`��C�x���g
	bool GetTransRect(Gdiplus::Rect &wRect, Gdiplus::Rect &Src, Gdiplus::Rect &Dest); // �]����`�𓾂�
	void DrawSSIcon(void); // �X���C�h�V���[�A�C�R����`�悷��
	void BeginUpdate(void); // �`����J�n����i�ē�����j
	void EndUpdate(void); // �`����I������
	void FormRefresh(void); // �t�H�[�����ĕ`�悷��


	// �t�H�[���T�C�Y�ύX�Ȃǃ}�E�X�C�x���g

	void OnMouseMove(void); // �}�E�X���t�H�[������ړ�
	void OnMouseDown(CMainForm::EEventButton button); // �}�E�X�_�E��
	void OnMouseUp(CMainForm::EEventButton button); // �}�E�X�A�b�v
	void DoMouseUp(void);
	void MouseLeave(void); // �}�E�X���E�B���h�E�O�Ɉړ�
	void MouseEnter(void); // �}�E�X���E�B���h�E���Ɉړ�
	bool SetCenter(int iX, int iY);
	bool GetMonitorParameter(void);


	// �t�H�[���̃T�C�Y�ړ����E�B���h�E�T�C�Y�ƈʒu�֌W

	bool SyncWindow(void); // ���ۂ̃t�H�[���̈ʒu������ݒ肷��
	bool CheckRefreshBuckBuffer(void); // �t�H�[���̃T�C�Y���ς�����Ƃ��Ƀo�b�N�o�b�t�@�̑傫����ύX����
	void GetRealWindowRect(void);
	bool FixViewOut(void); // �T�C�Y�Œ胂�[�h����o��
	void RefreshWindowPosition(int X, int Y); // ���z�E�B���h�E�̈ʒu���X�V����


	// �J�[�\����ݒ肷��

	EMousePositionType GetCursorPositionType(POINT &ScCurPos);
	bool SetCursorMode(EMousePositionType Result);


	// �z�C�[���C�x���g

	void OnMouseWheel(int delta); // �}�E�X�z�C�[��


	// ���j���[�ҏW

	void SetMenuCheckType(HMENU hmenu, UINT ItemID, int Type);
	void SetMenuCheck(HMENU hmenu, UINT ItemID, bool Checked);
	void SetMenuEnabled(HMENU hmenu, UINT ItemID, bool Enabled);
	void SetMenuText(HMENU hmenu, UINT ItemID, std::wstring src);


	// �|�b�v�A�b�v���j���[�\��

	void PopupMenuPopup(POINT &p);
	void PopupFileMovePopup(POINT &p);
	void CreateFileMovePPMenu(void);
	void CreateFileMoveMenuFolder(std::wstring FolderName);
	void CreateFolder(void);


	// �A���t�@�̃��j���[�̃`�F�b�N�̃I���I�t���R���g���[������

	void CheckAlphaValueMenuCheck(void);


	// ��Βl�ŉ摜����]����

	void AbsoluteRotate(int Value);
	void OffsetRotate(int Value);
	void SetRotateImageSize(void);


	// �摜���Y�[������

	bool ZoomImage(double NewPercentage);
	bool ZoomOrgPositionDelta(double Delta, int X, int Y);
	bool ZoomOrgPosition(double NewPercentage, int X, int Y);
	

	// �V�����摜��ǂݍ��񂾂Ƃ��ɃE�B���h�E�T�C�Y�����肷��

	void SetNewImageSize(void); 


	// �Ίp���T�C�Y����ʂ̑傫���̐������`�F�b�N����

	bool SetDiagonalLength(void); // �Ίp���T�C�Y���v�Z����
	bool CheckDiagonalLength(double &dWidth, double &dHeight); // �Ίp���̒����̍ŏ��l��ۂ�
	bool CheckMinimalDiagonalLength(int &iWidth, int &iHeight); // �ŏ��Ίp���T�C�Y���L�[�v����
	void CorrectWindow(void); // ��ʂ̒��ɏ����ł�����悤�ɕ\����ύX����


	// �E�B���h�E�̈ʒu�Ƒ傫�������߂�

	void SetWindowSize(int iWidth, int iHeight, bool InMonitor);
	bool MoveByPositionMode(void);
	bool SetPositionMode(int NewPositionMode);
	bool SetDisableFitMode(int aMode); // �t�B�b�g���[�h����
	bool GoToFullScreen(bool aMode); // �t���X�N���[�����[�h
	void AlphaValueOffset(int i); // �A���t�@�̒l�𑊑Βl�Ŏw��
	

	bool SetFileList(void);	// �t�@�C�����X�g��ݒ肷��
	std::wstring GetImageFileName(void); // �\�����̃t�@�C�������擾����
	std::wstring GetImageFileFolder(void);
	void SetImageFileNameString(std::wstring src);
	bool SetImageFileName(std::wstring src);
	bool ChangeImageFileName(std::wstring newName);

	
	// �t�@�C�����J��
	
	bool OpenFiles(std::vector<std::wstring> &SrcLists);
	bool OpenFiles(std::vector<std::wstring> &SrcLists, std::wstring SelectedFile, int Offset, bool AddMode);
	bool CheckGetLists(std::vector<CImageInfo>& DestLists, std::vector<std::wstring>& DropLists);
	bool GetImageLists(std::wstring Src, std::vector<CImageInfo>& Dest, bool SubFolder, bool EnableFileMask, std::wstring FileMaskString);
	bool AddFileList(std::vector<CImageInfo>& SrcLists, int Mode);
	bool DeleteFileList(int DeleteMode);
	
	// �t�@�C�����X�g����
	
	bool DeleteFileInList(int i);
	bool MoveSelectedList(int Offset);
	bool SortFileList(ESortType Type);

	EOpenFileResult OpenFile(CImageInfo & imageInfo);
	bool OpenArchiveMode(CImageInfo & SrcImageInfo, int SubIndex, int Ofs, bool MustShowImage);
	bool CloseArchiveMode(void);

	void ToggleShowList(EShowMode Mode);

	ELoadFileResult LoadFile(int Index, int SubIndex, int Ofs, bool MustShowImage);

	
	// �\���ʒu����

	bool AdjustShowIndex(int Ofs);
	bool JumpBorderArcNml(int Ofs);
	bool ShowOffsetImage(int Ofs);
	int ShowAbsoluteImage(int Index, int Ofs);
	int ShowAbsoluteImage(int Index, int Ofs, bool MustShowImage);
	int ShowAbsoluteImage(int Index, int SubIndex, int Ofs, bool MustShowImage);

	// �X���C�h�V���[
	
	void IntervalOffset(int i);

	
	// GIF �A�j��
	
	void GifAnimeTimer(void);
	void BeginGIFAnimeThread(void);
	void EndGIFAnimeThread(void);


	// ���j���[����

	void MnToggleVisible_Click(void);

	void MnFitToScreen_Click(void); // ��ʂɍ��킹�ĕ\��
	void MnFullScreen_Click(void); // �t���X�N���[�����[�h
	void MnUseWholeScreen_Click(void); // ���ׂẲ�ʂ��g���ĕ\��
	void MnInScreen_Click(void); // ��ʂ̒��ɕ\������
	void MnShowPbyP_Click(void); // �s�N�Z�����{�ŕ\��
	void MnCenter_Click(void); // �����Ɉړ�

	void MnPositionMode_Click(int Index); // �\������ʒu��ݒ�
	void MnRotateMode_Click(int Index);
	void MnRotateOffset_Click(int Index); // �摜����]
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
	void MnSlideShow_Click(void);
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
	void MnQuit_Click(void); // �I������

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


