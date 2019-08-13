//---------------------------------------------------------------------------

#ifndef MainH
#define MainH
//---------------------------------------------------------------------------

#include <windows.h>
#include <Classes.hpp>
#include <Controls.hpp>
#include <Dialogs.hpp>
#include <ExtCtrls.hpp>
#include <ImgList.hpp>
#include <Menus.hpp>
#include <StdCtrls.hpp>

#include <Forms.hpp>
#include "acfc_VCLSub.h"
#include <Grids.hpp>
#include <ExtDlgs.hpp>
#include "IdBaseComponent.hpp"
#include "IdComponent.hpp"
#include "IdHTTP.hpp"
#include "IdTCPClient.hpp"
#include "IdTCPConnection.hpp"

//---------------------------------------------------------------------------
class TMainForm : public TForm
{
__published:	// IDE 管理のコンポーネント
	TPopupMenu *PopupMenu;
	TMenuItem *MnClose;
	TMenuItem *N1;
	TMenuItem *MnAlwaysTop;
	TMenuItem *MnShowList;
	TListBox *DisplayBox;
	TMenuItem *MnFixPercentage;
	TMenuItem *N2;
	TMenuItem *N3;
	TMenuItem *MnLock;
	TMenuItem *MnSelectPluginFolder;
	TMenuItem *MnUninstall;
	TMenuItem *MnMoveTo;
	TOpenDialog *OpenPluginDialog;
	TTimer *SSTimer;
	TMenuItem *MnSlideShow;
	TMenuItem *MnSearchSubFolder;
	TMenuItem *MnViewExactrySize;
	TMenuItem *MnLoadLastFile;
	TMenuItem *MnInScreen;
	TTrayIcon *TrayIcon;
	TMenuItem *MnHideTaskButton;
	TMenuItem *O1;
	TMenuItem *MnTrayIconColor;
	TMenuItem *MnTC0;
	TMenuItem *MnTC1;
	TMenuItem *MnTC2;
	TMenuItem *MnTC3;
	TMenuItem *MnTC4;
	TMenuItem *MnTC5;
	TMenuItem *MnSSInterval;
	TMenuItem *MnSS1;
	TMenuItem *MnSS2;
	TMenuItem *MnSS3;
	TMenuItem *MnSS4;
	TMenuItem *MnSS5;
	TMenuItem *MnSS6;
	TMenuItem *N4;
	TMenuItem *MnCreateInstance;
	TMenuItem *MnDeleteInstance;
	TSaveDialog *SaveIniDialog;
	TSaveDialog *SaveLnkDialog;
	TMenuItem *N5;
	TMenuItem *MnFitToScreen;
	TMenuItem *MnFullScreen;
	TMenuItem *MnShowInformationBalloon;
	TMenuItem *MnSaveLists;
	TSaveDialog *SaveFileListsDialog;
	TMenuItem *MnUseWholeScreen;
	TColorDialog *ColorDialog;
	TMenuItem *MnFullScreenColor;
	TMenuItem *MnSortByName;
	TMenuItem *MnReload;
	TMenuItem *N6;
	TMenuItem *MnCloseArchive;
	TMenuItem *MnConstantSize;
	TMenuItem *MnSortByTimeStamp;
	TMenuItem *MnSortReverse;
	TMenuItem *MnSortRandom;
	TMenuItem *N7;
	TMenuItem *MnSnapWindow;
	TMenuItem *MnOpenFile;
	TMenuItem *MnDoubleBufferOff;
	TOpenDialog *OpenFileDialog;
	TTimer *CursorTimer;
	TMenuItem *MnShowCenter;
	TMenuItem *MnShowLT;
	TMenuItem *MnShowRT;
	TMenuItem *MnShowLB;
	TMenuItem *MnShowRB;
	TMenuItem *MnMoveCenter;
	TMenuItem *MnShowFree;
	TTimer *DBTimer;
	TImageList *ImageList;
	TMenuItem *MnRotate;
	TMenuItem *MnRot0;
	TMenuItem *MnRot3;
	TMenuItem *MnRot1;
	TMenuItem *MnRot2;
	TMenuItem *N9;
	TMenuItem *MnCW;
	TMenuItem *MnCCW;
	TMenuItem *MnNoStartAtCursor;
	TMenuItem *N10;
	TMenuItem *MnFixRotate;
	TMenuItem *N11;
	TMenuItem *MnWS0;
	TMenuItem *MnWS1;
	TMenuItem *MnWS2;
	TMenuItem *MnOpenFileFolder;
	TMenuItem *MnSS0;
	TMenuItem *MnOpenFolder;
	TMenuItem *MnSortByFileSize;
	TMenuItem *MnFrameWidth;
	TMenuItem *MnFrameWidth0;
	TMenuItem *MnFrameWidth1;
	TMenuItem *MnFrameWidth2;
	TMenuItem *MnFrameWidth3;
	TMenuItem *MnFrameWidth4;
	TMenuItem *MnFrameWidth5;
	TMenuItem *MnFrameFillColor;
	TMenuItem *MnSS7;
	TMenuItem *MnSS8;
	TMenuItem *MnSS9;
	TMenuItem *MnSSA;
	TMenuItem *MnSSB;
	TMenuItem *MnSSC;
	TMenuItem *MnSSD;
	TMenuItem *MnSSE;
	TMenuItem *N8;
	TMenuItem *MnEnableFileMask;
	TMenuItem *MnSetFileMask;
	TPopupMenu *FilePPMenu;
	TMenuItem *MnSortByFileExt;
	TMenuItem *MnFileMove;
	TMenuItem *MnCopyFilePath;
	TMenuItem *MnExplorerCopy;
	TMenuItem *MnExplorerCut;
	TMenuItem *MnExplorerPaste;
	TMenuItem *MnShow;
	TMenuItem *N12;
	TMenuItem *N13;
	TMenuItem *N14;
	TMenuItem *N15;
	TMenuItem *MnRename;
	TMenuItem *MnSPIOpenDialog;
	TMenuItem *MnCopyImage;
	TMenuItem *L1;
	TMenuItem *MnLFS1;
	TMenuItem *MnLFS4;
	TMenuItem *MnLFS5;
	TMenuItem *MnLFS6;
	TMenuItem *MnLFS7;
	TMenuItem *MnLFS8;
	TMenuItem *MnLFS3;
	TMenuItem *MnLFS2;
	TMenuItem *MnLFS0;
	TMenuItem *MnLFS9;
	TMenuItem *MnJpegSave;
	TMenuItem *MnJpegSizeSave;
	TMenuItem *MnJpegSetting;
	TMenuItem *N16;
	TSaveDialog *SaveJpegDialog;
	TMenuItem *MnSetFormSize;
	TMenuItem *N17;
	TMenuItem *MnABV0;
	TMenuItem *MnABV1;
	TMenuItem *MnABV2;
	TMenuItem *MnABV3;
	TMenuItem *MnReloadFile;
	TPopupMenu *HistoryPPMenu;
	TMenuItem *MnLoadFileFolder;
	TMenuItem *MnAutoLoadFileFolder;
	TMenuItem *X1;
	TMenuItem *MnSpiPrecede;
	TMenuItem *N18;
	TMenuItem *MnPNGSave;
	TSaveDialog *SavePNGDialog;
	void __fastcall FormDestroy(TObject *Sender);
	void __fastcall MnCloseClick(TObject *Sender);
	void __fastcall MnAlwaysTopClick(TObject *Sender);
	void __fastcall MnShowListClick(TObject *Sender);
	void __fastcall FormPaint(TObject *Sender);
	void __fastcall FormMouseLeave(TObject *Sender);
	void __fastcall FormMouseMove(TObject *Sender, TShiftState Shift, int X, int Y);
	void __fastcall FormMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
	void __fastcall FormMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
	void __fastcall FormMouseWheel(TObject *Sender, TShiftState Shift, int WheelDelta,
          TPoint &MousePos, bool &Handled);
	void __fastcall DisplayBoxDblClick(TObject *Sender);
	void __fastcall MnFixPercentageClick(TObject *Sender);
	void __fastcall MnLockClick(TObject *Sender);
	void __fastcall MnSelectPluginFolderClick(TObject *Sender);
	void __fastcall MnUninstallClick(TObject *Sender);
	void __fastcall MnSlideShowClick(TObject *Sender);
	void __fastcall SSTimerTimer(TObject *Sender);
	void __fastcall MnSearchSubFolderClick(TObject *Sender);
	void __fastcall MnViewExactrySizeClick(TObject *Sender);
	void __fastcall FormDblClick(TObject *Sender);
	void __fastcall FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall DisplayBoxKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall MnLoadLastFileClick(TObject *Sender);
	void __fastcall MnInScreenClick(TObject *Sender);
	void __fastcall MnHideTaskButtonClick(TObject *Sender);
	void __fastcall MnTCClick(TObject *Sender);
	void __fastcall MnSSTimeClick(TObject *Sender);
	void __fastcall MnCreateInstanceClick(TObject *Sender);
	void __fastcall MnDeleteInstanceClick(TObject *Sender);
	void __fastcall MnFitToScreenClick(TObject *Sender);
	void __fastcall MnFullScreenClick(TObject *Sender);
	void __fastcall MnShowInformationBalloonClick(TObject *Sender);
	void __fastcall MnSaveListsClick(TObject *Sender);
	void __fastcall MnUseWholeScreenClick(TObject *Sender);
	void __fastcall MnFullScreenColorClick(TObject *Sender);
	void __fastcall MnCloseArchiveClick(TObject *Sender);
	void __fastcall MnSortByNameClick(TObject *Sender);
	void __fastcall MnReloadClick(TObject *Sender);
	void __fastcall MnConstantSizeClick(TObject *Sender);
	void __fastcall MnSortByTimeStampClick(TObject *Sender);
	void __fastcall MnSortReverseClick(TObject *Sender);
	void __fastcall MnSortRandomClick(TObject *Sender);
	void __fastcall MnSnapWindowClick(TObject *Sender);
	void __fastcall MnOpenFileClick(TObject *Sender);
	void __fastcall MnDoubleBufferOffClick(TObject *Sender);
	void __fastcall CursorTimerTimer(TObject *Sender);
	void __fastcall FormMouseEnter(TObject *Sender);
	void __fastcall MnMoveCenterClick(TObject *Sender);
	void __fastcall MnPositionModeClick(TObject *Sender);
	void __fastcall DBTimerTimer(TObject *Sender);
	void __fastcall MnARotClick(TObject *Sender);
	void __fastcall MnAbsRot(TObject *Sender);
	void __fastcall MnNoStartAtCursorClick(TObject *Sender);
	void __fastcall MnFixRotateClick(TObject *Sender);
	void __fastcall MnWheelSenseClick(TObject *Sender);
	void __fastcall TrayIconMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
	void __fastcall MnOpenFileFolderClick(TObject *Sender);
	void __fastcall MnOpenFolderClick(TObject *Sender);
	void __fastcall MnSortByFileSizeClick(TObject *Sender);
	void __fastcall MnFrameFillColorClick(TObject *Sender);
	void __fastcall MnFrameWidthClick(TObject *Sender);
	void __fastcall MnSetFileMaskClick(TObject *Sender);
	void __fastcall MnEnableFileMaskClick(TObject *Sender);
	void __fastcall MnSortByFileExtClick(TObject *Sender);
	void __fastcall MnFileMoveClick(TObject *Sender);
	void __fastcall MnCopyFilePathClick(TObject *Sender);
	void __fastcall MnExplorerCopyClick(TObject *Sender);
	void __fastcall MnExplorerCutClick(TObject *Sender);
	void __fastcall MnExplorerPasteClick(TObject *Sender);
	void __fastcall MnShowClick(TObject *Sender);
	void __fastcall MnRenameClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall MnSPIOpenDialogClick(TObject *Sender);
	void __fastcall MnCopyImageClick(TObject *Sender);
	void __fastcall MnLFS5Click(TObject *Sender);
	void __fastcall MnJpegSettingClick(TObject *Sender);
	void __fastcall MnJpegSaveClick(TObject *Sender);
	void __fastcall MnJpegSizeSaveClick(TObject *Sender);
	void __fastcall MnSetFormSizeClick(TObject *Sender);
	void __fastcall MnAVMClick(TObject *Sender);
	void __fastcall MnReloadFileClick(TObject *Sender);
	void __fastcall MnLoadFileFolderClick(TObject *Sender);
	void __fastcall MnAutoLoadFileFolderClick(TObject *Sender);
	void __fastcall MnSpiPrecedeClick(TObject *Sender);
	void __fastcall MnPNGSaveClick(TObject *Sender);




private:	// ユーザー宣言
public:		// ユーザー宣言
	__fastcall TMainForm(TComponent* Owner);
//	virtual void __fastcall Dispatch(void *Message);

	void __fastcall MyIdleHandler(TObject *Sender, bool &Done);

	bool __fastcall GetImageLists(UnicodeString Src, TStringList *Dest, bool SubFolder, bool EnableFileMask, UnicodeString FileMaskString);

	void __fastcall CheckIntervalMenuCheck(void);
	void __fastcall CheckFontSizeMenuCheck(void);
	void __fastcall CheckAlphaValueMenuCheck(void);
	void __fastcall AlphaValueOffset(int i);
	void __fastcall IntervalOffset(int i);
	void __fastcall CheckFrameWidthMenuCheck(void);
	void __fastcall CheckWheelSenseCheck(void);

	void __fastcall SetComponentPointer(void);
	bool __fastcall ClearArchiveFileLists(TStringList *ArcLists);

	bool __fastcall SaveIni(UnicodeString IniName);
	bool __fastcall SaveFileLists(UnicodeString FileName);
	bool __fastcall CreateFileLists(TStringList *TempSL);

	bool __fastcall LoadIni(UnicodeString IniName, bool CmdLine);
	UnicodeString __fastcall GetTabString(UnicodeString Src, UnicodeString Name);
	bool __fastcall LoadFileLists(TStringList *DestSL, TStringList *ArcLists, TStringList *TempSL);
	bool __fastcall CheckMonitorIni(void);

/*
	UnicodeString __fastcall GetIniFolder(void);
	UnicodeString __fastcall GetIniFileName(UnicodeString IniName);
*/
	bool __fastcall DoInstall(void);
	bool __fastcall DoCreateSendToLink(UnicodeString InstanceName);
	bool __fastcall CreateShortCut(UnicodeString InstanceName);

	bool __fastcall DoUninstall(void);
	bool __fastcall DeleteSendToLink(UnicodeString InstanceName);

	void __fastcall LoopProc(TMessage& Msg);
	bool __fastcall OpenFiles(TStringList *SrcLists);
	bool __fastcall OpenFiles(TStringList *SrcLists, UnicodeString SelectedFile, int Offset, bool AddMode);

	bool __fastcall CreateDBImage(void);
	int __fastcall OpenFile(UnicodeString FilePath);
	bool __fastcall OpenArchiveMode(UnicodeString FilePath, int SubIndex, int Ofs, bool MustShowImage);
	bool __fastcall CloseArchiveMode(void);

	void __fastcall ToggleShowList(int Mode);
	bool __fastcall SetFileList(void);
	UnicodeString __fastcall GetShortFileName(UnicodeString Temp);
	void __fastcall SetNewImageSize(void);
	bool __fastcall SetDisableFitMode(int aMode);
	bool __fastcall SetConstantSize(void);

	bool __fastcall CheckGetLists(TStringList *DestLists, TStringList *ArcLists, TStringList *DropLists);
	bool __fastcall AddFileLists(TStringList *SrcLists, TStringList *ArcLists, int Mode);
	bool __fastcall DeleteFileLists(int DeleteMode);
	bool __fastcall DeleteFileInList(int i);
	bool __fastcall DeleteArchiveData(UnicodeString ArcFileName);
	bool __fastcall MoveSelectedList(int Offset);
	bool __fastcall SortByName(void);
	bool __fastcall SortBy(int Type);

	bool __fastcall ShowOffsetImage(int Ofs);
	int __fastcall ShowAbsoluteImage(int Index, int Ofs);
	int __fastcall ShowAbsoluteImage(int Index, int Ofs, bool MustShowImage);
	int __fastcall ShowAbsoluteImage(int Index, int SubIndex, int Ofs, bool MustShowImage);
	int __fastcall LoadFile(int Index, int SubIndex, int Ofs, bool MustShowImage);
	bool __fastcall AdjustShowIndex(int Ofs);
	bool __fastcall JumpBorderArcNml(int Ofs);

	void __fastcall RefreshWindowPosition(int X, int Y);
	bool __fastcall SyncWindow(bool FeedBack);
	void __fastcall BeginUpdate(void);
	void __fastcall EndUpdate(void);
	bool __fastcall GetMonitorParameter(void);
	int __fastcall GetCursorPositionType(POINT &ScCurPos);
	bool __fastcall SetCursorMode(int Result);
	void __fastcall DoMouseUp(void);

	void __fastcall SetWindowSize(int iWidth, int iHeight, bool InScreen);
	bool __fastcall MoveByPositionMode(void);
	bool __fastcall ZoomImage(double NewPercentage);
	bool __fastcall ZoomOrgPositionDelta(double Delta, int X, int Y);
	bool __fastcall ZoomOrgPosition(double Delta, int X, int Y);
	void __fastcall SetWindowPosition(void);
	bool __fastcall SetCenter(int iX, int iY);
	bool __fastcall SetPositionMode(int NewPositionMode);
	bool __fastcall GoToFullScreen(bool Mode);
	bool __fastcall FixViewOut(void);

	bool __fastcall SetInstanceMode(UnicodeString NewIniParamName);

	void __fastcall CheckRotateCheck(void);
	bool __fastcall SetRotateImageSize(void);
	TColor __fastcall GetFrameColor(void);
	void __fastcall DrawSSIcon(void);

	UnicodeString OriginName;
	TStringList *FolderLists;
  bool __fastcall CreateFilePPMenu(void);
	void __fastcall CreateFileMenuFolder(UnicodeString FolderName);
	void __fastcall FileMenuClick(TObject *Sender);
	TMenuItem *__fastcall GetNewMenu(UnicodeString MenuName, UnicodeString FullPath, int Count);
  void __fastcall CreateFolder(void);

	UnicodeString __fastcall GetOpeningFileName(void);
	bool __fastcall SetOpeningFileName(UnicodeString Dest);
	UnicodeString __fastcall GetImageFileName(void);
	UnicodeString __fastcall GetImageFileFolder(void);

	void __fastcall RestoreStayOnTop(void);

	CDragDrop *Dropper;
	TStringList *FileLists;
	TStringList *ArchiveFileLists;
	TStringList *DisplayLists;

	UnicodeString IniParamName;

	POINT OfsCurPos;          // マウスダウンしたときの Left Top の位置
	POINT DownCurPos;         // マウスダウンしたときの画面上の位置
	POINT PreCurPos;          // マウスダウン中の一つ前の画面上の位置
	POINT PreCurTimerPos;			// 一つ前のカーソル位置

	int DownWidth;            // マウスダウンしたときの幅
	int DownHeight;           // マウスダウンしたときの高さ
	int DownLeft;            	// マウスダウンしたときの幅
	int DownTop;           		// マウスダウンしたときの高さ

	int Holding;              // マウスホールドしているかどうか 0:してない 1:左 2:右 3:真ん中
	int PositionType;         // マウスダウンしたときのマウスの位置
	double Ratio;             // 画像の比
	double Percentage;        // 拡大率
	int FrameWidth;						// フレームの幅
	int WheelPos;             // ホイール値
	int *Rotate;							// 回転
	bool NotSaveIni;          // Ini ファイルを終了時に保存するかどうか
	bool InstanceMode;        // インスタンスモードで動作中かどうか
	int FitMode;							// 1:上下が吸着 2:左右が吸着
	bool InArchive;						// アーカイブファイルを開いているかどうか
	bool PreviousDB;					// ロックする前の DoubleBufferOff モード
	bool EnableDraw;					// 再描画の抑制
	bool ReloadMode;	       	// 復元起動時に大きさ位置を保持するために使う
	bool RClkCancel;					// 右クリックメニューをキャンセルするかどうか
	int FontSizeIndex;						// フォントサイズのメニューのインデックス
	TColor FrameColor;		    // フレームの色
	long LoadStart;				   	// 読み込み開始時刻
	long LoadState;           // ローディングの状態
	int StartnNum;					  // 初期位置
	int SSIcon;               // スライドショーアイコンのモード

	int ShowIndex;            // 表示中の画像の番号
	int ShowIndexBack;         // 表示中の画像の番号(サブ)
	bool AlwaysTop;           // 常に手前に表示
	bool ShowingList;         // ファイルリスト表示中かどうか
	bool SlideShow;           // スライドショーモード
	int SSInterval;           // スライドショーのインターバル
	bool FixPercentage;       // 拡大率固定
	bool FitToScreen;         // 画面に合わせて表示
	bool UseWholeScreen;      // 画面をすべて使って表示
	bool FullScreen;          // フルスクリーンかどうか
	bool Locked;              // ロック
	bool SearchSubFolder;     // サブフォルダも検索するかどうか
	bool LoadLastFile;        // 最後の画像を次回に読み込むかどうか
	bool HideTaskButton;      // タスクボタンを未表示にするかどうか
	int CenterX;              // 画像の中央Ｘ
	int CenterY;              // 画像の中央Ｙ
	bool ShowConstant;				// 一定倍率で表示
	bool NoStartAtCursor;			// カーソル位置から開始しない
	int ConstantSize;         // 一定表示時の対角線の２乗
	int TrayIconColor;        // トレイアイコンの表示番号
	int PositionMode;					// 表示位置 0:自由 1:中央 2:左上 3:右上 4:左下 5:右上
	bool FixRotate;						// 回転を固定するかどうか
	int RotateValue;          // 回転角度
	int WheelSensitivity;			// ホイール感度
	TColor FullFillColor;		  // フルスクリーン時に塗りつぶす色
	TColor FrameFillColor;    // フレームの塗りつぶし色
	UnicodeString ShortCutFileName; // 作成したショートカットのファイル名
	bool CreateSendToLink;    // SendTo にリンクを作成したかどうか

	UnicodeString FileMaskString; 	// ファイルマスク
	bool EnableFileMask;			      // ファイルマスクを有効にするかどうか

	bool AutoLoadFileFolder;				// 単独画像表示時に次のファイルを表示しようとするとフォルダを自動で読み込む

	bool DoubleBufferOff;           // ダブルバッファーモードを使わないかどうか
	bool SSChangeImage;				    	// マウスの移動を無視する

	TMenuItem *TrayIcons[6];
	#define SLIDESHOWNUM 15
	TMenuItem *SlideShows[SLIDESHOWNUM];
	TMenuItem *FrameWidths[6];
	TMenuItem *PositionModes[6];
	TMenuItem *RotateState[4];
	TMenuItem *WheelSense[3];
	#define FONTSIZENUM 10
	TMenuItem *FontSizes[FONTSIZENUM];
	int ApplicationFontSize; // ファイル一覧のリストボックスのフォントサイズ
	#define ALPHABLENDNUM 4
	TMenuItem *AlphaBlends[ALPHABLENDNUM];
	int FormAlphaBlendValue;

	double WLeft, WTop;     // ウィンドウの左、上
	double WWidth, WHeight; // ウィンドウの幅、高さ

	int MLeft, MTop;      	// ウィンドウが存在するモニタの左と上
	int MWidth, MHeight;  	// ウィンドウが存在するモニタの幅と高さ

	int JCR[4]; 						// Jpeg 保存時の圧縮率の設定

	int StopCount;

	Forms::TMonitor *Monitor;

	int ProgressRatio;

	int VkRButton;
	int VkLButton;

	UnicodeString OpeningFileName;

	bool GIFRefresh;

	long ClockTGT; // GetTickCountの値
	long PreTGT;

	long ShortestPeriod; // 計算できる最小間隔

	// ファイル履歴の処理
	bool __fastcall CreateHistoryMenu(void);
	bool __fastcall ConvertHistoryMenu(void);
	void __fastcall HistoryMnClick(TObject *Sender);

	bool __fastcall AddHistoryList_(UnicodeString FileName);
	bool __fastcall AddHistoryList(UnicodeString FileName);
	bool __fastcall AddHistoryList(TStringList *FileNames);

	bool __fastcall SendUpdateHistoryMessage(void);
	bool __fastcall RefreshHistoryFormMessage(void);

	void __fastcall HistoryListPopUp(void);
	void __fastcall PopupMenuPopup(int X, int Y);

	TStringList *HistoryList;
	int MaxHistoryNum;
	TMenuItem **HistoryMenu;
	int RefreshHistoryMode;
	FILETIME IniFileTime;

	int HistoryPopUpX, HistoryPopUpY;
};
//---------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;
//---------------------------------------------------------------------------
#endif
