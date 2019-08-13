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
__published:	// IDE �Ǘ��̃R���|�[�l���g
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




private:	// ���[�U�[�錾
public:		// ���[�U�[�錾
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

	POINT OfsCurPos;          // �}�E�X�_�E�������Ƃ��� Left Top �̈ʒu
	POINT DownCurPos;         // �}�E�X�_�E�������Ƃ��̉�ʏ�̈ʒu
	POINT PreCurPos;          // �}�E�X�_�E�����̈�O�̉�ʏ�̈ʒu
	POINT PreCurTimerPos;			// ��O�̃J�[�\���ʒu

	int DownWidth;            // �}�E�X�_�E�������Ƃ��̕�
	int DownHeight;           // �}�E�X�_�E�������Ƃ��̍���
	int DownLeft;            	// �}�E�X�_�E�������Ƃ��̕�
	int DownTop;           		// �}�E�X�_�E�������Ƃ��̍���

	int Holding;              // �}�E�X�z�[���h���Ă��邩�ǂ��� 0:���ĂȂ� 1:�� 2:�E 3:�^��
	int PositionType;         // �}�E�X�_�E�������Ƃ��̃}�E�X�̈ʒu
	double Ratio;             // �摜�̔�
	double Percentage;        // �g�嗦
	int FrameWidth;						// �t���[���̕�
	int WheelPos;             // �z�C�[���l
	int *Rotate;							// ��]
	bool NotSaveIni;          // Ini �t�@�C�����I�����ɕۑ����邩�ǂ���
	bool InstanceMode;        // �C���X�^���X���[�h�œ��쒆���ǂ���
	int FitMode;							// 1:�㉺���z�� 2:���E���z��
	bool InArchive;						// �A�[�J�C�u�t�@�C�����J���Ă��邩�ǂ���
	bool PreviousDB;					// ���b�N����O�� DoubleBufferOff ���[�h
	bool EnableDraw;					// �ĕ`��̗}��
	bool ReloadMode;	       	// �����N�����ɑ傫���ʒu��ێ����邽�߂Ɏg��
	bool RClkCancel;					// �E�N���b�N���j���[���L�����Z�����邩�ǂ���
	int FontSizeIndex;						// �t�H���g�T�C�Y�̃��j���[�̃C���f�b�N�X
	TColor FrameColor;		    // �t���[���̐F
	long LoadStart;				   	// �ǂݍ��݊J�n����
	long LoadState;           // ���[�f�B���O�̏��
	int StartnNum;					  // �����ʒu
	int SSIcon;               // �X���C�h�V���[�A�C�R���̃��[�h

	int ShowIndex;            // �\�����̉摜�̔ԍ�
	int ShowIndexBack;         // �\�����̉摜�̔ԍ�(�T�u)
	bool AlwaysTop;           // ��Ɏ�O�ɕ\��
	bool ShowingList;         // �t�@�C�����X�g�\�������ǂ���
	bool SlideShow;           // �X���C�h�V���[���[�h
	int SSInterval;           // �X���C�h�V���[�̃C���^�[�o��
	bool FixPercentage;       // �g�嗦�Œ�
	bool FitToScreen;         // ��ʂɍ��킹�ĕ\��
	bool UseWholeScreen;      // ��ʂ����ׂĎg���ĕ\��
	bool FullScreen;          // �t���X�N���[�����ǂ���
	bool Locked;              // ���b�N
	bool SearchSubFolder;     // �T�u�t�H���_���������邩�ǂ���
	bool LoadLastFile;        // �Ō�̉摜������ɓǂݍ��ނ��ǂ���
	bool HideTaskButton;      // �^�X�N�{�^���𖢕\���ɂ��邩�ǂ���
	int CenterX;              // �摜�̒����w
	int CenterY;              // �摜�̒����x
	bool ShowConstant;				// ���{���ŕ\��
	bool NoStartAtCursor;			// �J�[�\���ʒu����J�n���Ȃ�
	int ConstantSize;         // ���\�����̑Ίp���̂Q��
	int TrayIconColor;        // �g���C�A�C�R���̕\���ԍ�
	int PositionMode;					// �\���ʒu 0:���R 1:���� 2:���� 3:�E�� 4:���� 5:�E��
	bool FixRotate;						// ��]���Œ肷�邩�ǂ���
	int RotateValue;          // ��]�p�x
	int WheelSensitivity;			// �z�C�[�����x
	TColor FullFillColor;		  // �t���X�N���[�����ɓh��Ԃ��F
	TColor FrameFillColor;    // �t���[���̓h��Ԃ��F
	UnicodeString ShortCutFileName; // �쐬�����V���[�g�J�b�g�̃t�@�C����
	bool CreateSendToLink;    // SendTo �Ƀ����N���쐬�������ǂ���

	UnicodeString FileMaskString; 	// �t�@�C���}�X�N
	bool EnableFileMask;			      // �t�@�C���}�X�N��L���ɂ��邩�ǂ���

	bool AutoLoadFileFolder;				// �P�Ɖ摜�\�����Ɏ��̃t�@�C����\�����悤�Ƃ���ƃt�H���_�������œǂݍ���

	bool DoubleBufferOff;           // �_�u���o�b�t�@�[���[�h���g��Ȃ����ǂ���
	bool SSChangeImage;				    	// �}�E�X�̈ړ��𖳎�����

	TMenuItem *TrayIcons[6];
	#define SLIDESHOWNUM 15
	TMenuItem *SlideShows[SLIDESHOWNUM];
	TMenuItem *FrameWidths[6];
	TMenuItem *PositionModes[6];
	TMenuItem *RotateState[4];
	TMenuItem *WheelSense[3];
	#define FONTSIZENUM 10
	TMenuItem *FontSizes[FONTSIZENUM];
	int ApplicationFontSize; // �t�@�C���ꗗ�̃��X�g�{�b�N�X�̃t�H���g�T�C�Y
	#define ALPHABLENDNUM 4
	TMenuItem *AlphaBlends[ALPHABLENDNUM];
	int FormAlphaBlendValue;

	double WLeft, WTop;     // �E�B���h�E�̍��A��
	double WWidth, WHeight; // �E�B���h�E�̕��A����

	int MLeft, MTop;      	// �E�B���h�E�����݂��郂�j�^�̍��Ə�
	int MWidth, MHeight;  	// �E�B���h�E�����݂��郂�j�^�̕��ƍ���

	int JCR[4]; 						// Jpeg �ۑ����̈��k���̐ݒ�

	int StopCount;

	Forms::TMonitor *Monitor;

	int ProgressRatio;

	int VkRButton;
	int VkLButton;

	UnicodeString OpeningFileName;

	bool GIFRefresh;

	long ClockTGT; // GetTickCount�̒l
	long PreTGT;

	long ShortestPeriod; // �v�Z�ł���ŏ��Ԋu

	// �t�@�C�������̏���
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
