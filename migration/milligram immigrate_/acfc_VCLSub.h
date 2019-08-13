#ifndef acfcVclSubH
#define acfcVclSubH

#include <vcl.h>
#include <FileCtrl.hpp>
#include <Masks.hpp>
#define NO_WIN32_LEAN_AND_MEAN
#include <windowsx.h>
#include <shlobj.h>
#include <shellapi.h>
#include <Clipbrd.hpp>

#include <jpeg.hpp>
#include <gifimg.hpp>
#include <pngimage.hpp>
#include <Graphics.hpp>

#define ACFC_VCLSUB_CHECK_SUBFOLDER 1
#define ACFC_VCLSUB_DELETEFILES     2
#define ACFC_VCLSUB_ONLY_NEWFILES   4
#define ACFC_VCLSUB_INCLUDEMASK     8



//----------------------------------------------------------------
//
//  �t�@�C������֌W
//
//----------------------------------------------------------------


// �G���f�B�A���̕ύX
void Endian(void *Dat, int Size, int Mode);

// �f�B���N�g�������݂��邩�ǂ����`�F�b�N����
bool DirExists(UnicodeString Dir);

// �t�H���_�̕�����ύX����(\ �� / �ɕύX����)
UnicodeString ChangeFolderSymbol(UnicodeString Str);

// Directory �Ɏg���Ȃ��������폜����
UnicodeString CorrectDirString(UnicodeString Str, bool CanBackSlash);

// �����ȊO�̕������폜����(���̐����̂�)
UnicodeString CorrectNumericString(UnicodeString Str);

// �����ȊO�̕������폜����(�I�v�V�����t��)
UnicodeString CorrectNumericString(UnicodeString Src, bool CanMinus, bool CanFloat, UnicodeString MaxN, UnicodeString MinN);


// �w�肵���t�@�C�����w�肵���}�X�N��ʂ邩�ǂ������ׂ�
bool CheckMasks(UnicodeString FileName, UnicodeString Mask); // true �Ȃ�}�X�N�Ɋ܂܂��

// �t�H���_���폜����
bool DeleteFolder(UnicodeString Dir);

// �t�@�C�����ړ����� Src�t�H���_���� Dest�t�H���_�� Type(*.?) ���ړ�����
bool MoveFiles(UnicodeString Src, UnicodeString Dest, UnicodeString Type, bool OverWrite);

// �t�@�C�����X�g���擾����(Esc�L�[�Ŕ�������)
bool GetFileLists(UnicodeString Src, TStringList *Dest, UnicodeString Mask, int Depth); // c:\windows\dest\*.dll �̂悤�Ȍ`���� Src �͎w�肷��
bool GetFileLists_(UnicodeString Src, TStringList *Dest, UnicodeString Mask, int Depth, TStringList *TempSL, bool &Esc);


bool GetFileListsDialog(UnicodeString Src, TStringList *Dest, UnicodeString Mask, int Depth, TApplication *Application, TLabel *Label); // c:\windows\dest\*.dll �̂悤�Ȍ`���� Src �͎w�肷��
bool GetFileListsDialog_(UnicodeString Src, TStringList *Dest, UnicodeString Mask, int Depth, TStringList *TempSL, bool &Esc, TApplication *Application, TLabel *Label);


// �t�@�C�����R�s�[����(�t�H���_���܂�)
bool CopyFiles(UnicodeString Src, UnicodeString Dest, UnicodeString Type, bool OverWrite);

// �t�@�C�������R�s�[����
bool CopyOnlyFiles(UnicodeString Src, UnicodeString Dest, UnicodeString Type, bool OverWrite);
void CopyFolderFiles(UnicodeString Src, UnicodeString Dest, UnicodeString Type);

// Src �t�H���_���̃t�@�C�����폜���ăt�@�C���������Ȃ�΃t�H���_���폜
void DeleteFolderFiles(UnicodeString Src, UnicodeString Dest, UnicodeString Type);

// Src �t�H���_�ɂ���t�@�C���̂��� Dest �t�H���_�ɂ�����̂� Dest �t�H���_�ɃR�s�[����
void ReplaceFolderFiles(UnicodeString Src, UnicodeString Dest, UnicodeString Type);

// �R��ނ̃^�C���X�^���v�����ׂĎw�肵���l�ɂ���
void SetAllTimeStamp(int Year, int Month, int Day, int Hour, int Minute, int Second, int Milliseconds, UnicodeString DestFileName);

// Src �t�@�C���Ɠ������ԂɃ^�C���X�^���v��ݒ肷��
void SetSameTimeAll(UnicodeString SrcFileName, UnicodeString DestFileName);

// �X�V�������擾����
void GetFileTimeStamp(UnicodeString FileName, FILETIME *fRefreshTime);

// �t�@�C����I�����ăG�N�X�v���[���[�ŕ\��
void SelectFileInExplorer(UnicodeString Src);

// �t�@�C���T�C�Y���擾����
size_t GetFileSize(UnicodeString Target);

// �w�肵���f�B���N�g���̐������f�B���N�g����������폜
UnicodeString DeleteDirString(UnicodeString Dir, int Count);

// ���΃p�X���΃p�X�ɕϊ�
UnicodeString GetAbsoluteFilePath(UnicodeString Root, UnicodeString Relative);

// (1) (2) �`���ŏd�����Ȃ��t�@�C�������擾����
UnicodeString GetNonOverwrapFileName(UnicodeString Src);

// Msg ����h���b�v���ꂽ�t�@�C�������P�擾����
UnicodeString GetDropFileName(TMessage &Msg);
UnicodeString GetDropFileName(TMessage &Msg, bool Original);

// �����N�t�@�C������{�̂̃t�@�C�����擾����
UnicodeString GetFileFromLink(UnicodeString Path);

// ���s�t�@�C���̃����N�t�@�C�����쐬����
HRESULT CreateLink(UnicodeString OriginalFile, UnicodeString ShortCutFile, UnicodeString Params);

// ���ϐ��������W�J���A���̕�������`���ꂽ�l�ɒu�������܂� %PATH% �݂����Ȃ̂��擾����
UnicodeString AsExpandEnvironmentStrings(UnicodeString Src);

// AsExpandEnviromentStrings �̋t
UnicodeString AsPathUnExpandEnvStrings(UnicodeString Src);

// Src �� Dest �̃t�H���_���r���� Dest �t�H���_�����������ꍇ�ɂ͋����쐬����
bool SyncDir(UnicodeString SrcDir, UnicodeString DestDir, UnicodeString MaskStr, int Option);

// (SyncDir ����Ă΂��) Src �� Dest �̃t�H���_���r���āi�I�v�V�����ɂ�邪�j�V�����t�@�C���݂̂��㏑���R�s�[����
bool CheckDirNew(UnicodeString SrcDir, UnicodeString DestDir, UnicodeString MaskStr, int Option, UnicodeString Dir);

// (SyncDir ����Ă΂��) Src �� Dest �̃t�H���_���r���č폜���ꂽ�t�H���_���폜����
bool CheckDirDeleted(UnicodeString SrcDir, UnicodeString DestDir, UnicodeString MaskStr, int Option, UnicodeString Dir);

// �_�C�A���O��\�����ăt�H���_���擾����
UnicodeString GetFolderByDialog(UnicodeString DefaultDir, HWND Handle, UnicodeString Message);

// �t�@�C�����N���b�v�{�[�h�ɃR�s�[�y�[�X�g
void FilesToClipboard(HWND Handle, TStringList *Src, int Flag);
int FilesFromClipboard(TStringList *DestSL);

// �t�@�C�����S�~���Ɉړ�����
bool DeleteFileToRecycle(UnicodeString DelFile, bool Confirm, HWND Handle);
bool DeleteFileToRecycle(TStringList *DelFiles, bool Confirm);

// ini �t�@�C���̃t�@�C������t�H���_���擾����
UnicodeString GetIniFolder(void);
UnicodeString GetIniFileName(UnicodeString IniName);

// �t�@�C�����Ɏg���Ȃ������𓾂� -1:�Ȃ� 0~:���̃C���f�b�N�X�̕���
int CheckFileIrregularChar(UnicodeString FileName);

// �t�@�C��������ȗ������t�@�C�������擾���� C:\Pool\test\gone\done\abc.txt �� done\abc.txt
UnicodeString __fastcall GetShortFileName(UnicodeString Temp, int Count);

// �t�H���_�𐔂����������オ��
UnicodeString __fastcall DirectoryUpward(UnicodeString FullPath, int Count);


//----------------------------------------------------------------
//
//  ���̑�
//
//----------------------------------------------------------------

// Dest ������ SrcTS �����S���ɂ��邩�ǂ����`�F�b�N����
bool CheckStrings(TStringList *SrcTS, UnicodeString Dest);
bool CheckStrings(TStringList *SrcTS, UnicodeString Dest, int &Pos);
UnicodeString CheckStringValue(TStringList *SrcTS, UnicodeString Dest);
UnicodeString CheckStringValue(TStringList *SrcTS, UnicodeString Dest, int &Pos);

// Dest�����񁁒l �̍s���폜���� Pos �͂��̍s
bool DeleteStringValue(TStringList *SrcTS, UnicodeString Dest, int &Pos);

// ���l���擾
int GetIntegerValue(TStringList *SrcTS, UnicodeString Dest, int Src, int Low, int Hi);
int GetIntegerValue(TStringList *SrcTS, UnicodeString Dest, int Src, int Low, int Hi, int &Pos);
double GetDoubleValue(TStringList *SrcTS, UnicodeString Dest, double Src, double Low, double Hi);
double GetDoubleValue(TStringList *SrcTS, UnicodeString Dest, double Src, double Low, double Hi, int &Pos);

// Bool �l���擾
bool GetBoolValue(TStringList *SrcTS, UnicodeString Dest, bool Src);
bool GetBoolValue(TStringList *SrcTS, UnicodeString Dest, bool Src, int &Pos);

// ��������擾
UnicodeString GetStringValue(TStringList *SrcTS, UnicodeString Dest, UnicodeString Src);
UnicodeString GetStringValue(TStringList *SrcTS, UnicodeString Dest, UnicodeString Src, int &Pos);



// Dest ������ SrcUS �����S���ɂ��邩�ǂ����`�F�b�N����
bool CheckTabStrings(UnicodeString SrcUS, UnicodeString Dest);
UnicodeString CheckTabStringValue(UnicodeString SrcUS, UnicodeString Dest);

// ���l���擾
int GetTabIntegerValue(UnicodeString SrcUS, UnicodeString Dest, int Src, int Low, int Hi);
double GetTabDoubleValue(UnicodeString SrcUS, UnicodeString Dest, double Src, double Low, double Hi);

// Bool �l���擾
bool GetTabBoolValue(UnicodeString SrcUS, UnicodeString Dest, bool Src);

// ��������擾
UnicodeString GetTabStringValue(UnicodeString SrcUS, UnicodeString Dest, UnicodeString Src);




// ���l���������T�C�Y�̕�����ɕύX���� 1024=1Kbyte �̂悤�ɕϊ������
UnicodeString IntToMemorySize(__int64 tSize);

// ���l�������L��̕�����ɕϊ����� 5 = 005 �̂悤�ɕϊ������
UnicodeString IntToDigitStr(int Value, int Digit);

// �O�ʂɃE�B���h�E���ړ�����
void SetAbsoluteForegroundWindow(HWND hWnd);

// �p�f�B���O�T�C�Y���v�Z����
inline int CalPaddingSize(int SrcSize, int SegSize){return((SrcSize + (SrcSize - 1)) / SegSize);};

//----------------------------------------------------------------
//
// CDragDrop �h���b�O�h���b�v�֌W
//
//----------------------------------------------------------------

class CDragDrop
{
public:
	void __fastcall Init(TWinControl *Control);
	void __fastcall Release(TWinControl *Control);
	UnicodeString __fastcall GetDropFileName(TMessage &Msg);
	UnicodeString __fastcall GetDropFileName(TMessage &Msg, bool Original);
	int __fastcall GetDropFileNames(TMessage &Msg, TStringList *TempSL);
	int __fastcall GetDropFileNames(TMessage &Msg, TStringList *TempSL, bool Original);
	UnicodeString __fastcall GetFileFromLink(UnicodeString Path);

	TWndMethod oldWinProc;
};

/*
// Header
	void __fastcall DropProc(TMessage& Msg);
  CDragDrop *Dropper;

// Init
	Dropper = new CDragDrop;
	Dropper->Init(�R���g���[���ւ̃|�C���^);
	�R���g���[���ւ̃|�C���^->WindowProc = DropProc; // �t�H�[���̂ق��ɋL��

// Release
	Dropper->Release(�R���g���[���ւ̃|�C���^);
	delete Dropper;

void __fastcall TMainForm::DropProc(TMessage& Msg)
{
	if(Msg.Msg == WM_DROPFILES)
	{
		UnicodeString FileName = Dropper->GetDropFileName(Msg);
		if(ExtractFileExt(FileName) == ".dmp")
		{
			// ��������
		}
	}
	else
		Dropper->oldWinProc(Msg);
}
*/

//----------------------------------------------------------------
//
// CCriticalSection �r������
//
//----------------------------------------------------------------

class CCriticalSection
{
public:
	CCriticalSection(void);
	~CCriticalSection(void);
	bool Enter(void);
	bool Leave(void);

private:
	CRITICAL_SECTION section;
};

//----------------------------------------------------------------
//
// CBinaryBlock ��������o�C�i���̃f�[�^�ɕϊ�����
//
//----------------------------------------------------------------

class CBinaryBlock
{
// CBinaryBlock �N���X�B��������o�C�i���̃f�[�^�ɕϊ�����
public:
	CBinaryBlock(void);
  ~CBinaryBlock(void);
  bool StrToBin(UnicodeString SrcData);
	bool StrToBin(UnicodeString SrcData, int &TempSize);
	void *BinaryData;

private:
	size_t GetDataSize(UnicodeString Data);
	void SetData(UnicodeString Data);
};


//----------------------------------------------------------------
//
//  ���W�X�g���A�N�Z�X�N���X
//
//----------------------------------------------------------------

class CReg
{
private:
	HKEY FRootKey;
	HKEY FHKey;
	bool FOpened;

public:
	CReg();
	virtual ~CReg();

	bool OpenKey(const UnicodeString KeyName, bool IsCreate);
	bool CreateKey(const UnicodeString KeyName);
	void CloseKey(void);
	bool DeleteKey(const UnicodeString Key);
private:
	DWORD RegDeleteKeyNT(HKEY hStartKey, LPTSTR pKeyName);

public:
	void WriteBool(const UnicodeString Name, const bool Flag);
	void WriteInteger(const UnicodeString Name, const int Value);
	void WriteString(const UnicodeString Name, const UnicodeString Str);
	void WriteData(const UnicodeString Name, const void *Data, int Size);

	bool ReadBool(const UnicodeString Name, bool Def);
	int ReadInteger(const UnicodeString Name, int Def);
	UnicodeString ReadString(const UnicodeString Name, const UnicodeString Def);
	bool ReadData(const UnicodeString Name, void *Buff, int size, const void *Def);

	bool DeleteValue(const UnicodeString Name);

	__property HKEY RootKey = {read=FRootKey, write=FRootKey};
};

//----------------------------------------------------------------
//
//  Unicode �Ή��摜�N���X
//
//----------------------------------------------------------------

class TUCFileStream : public THandleStream
{
public:
	virtual __fastcall TUCFileStream(UnicodeString FileName, DWORD AccessMode, DWORD ShareMode);
	virtual __fastcall ~TUCFileStream(void);
};

class TUCBitmap : public Graphics::TBitmap
{
public:
	void __fastcall LoadFromFile(const UnicodeString FileName);
	void __fastcall SaveToFile(const UnicodeString FileName);
};

class TUCJPEGImage : public TJPEGImage
{
public:
	void __fastcall LoadFromFile(const UnicodeString FileName);
	void __fastcall SaveToFile(const UnicodeString FileName);

	virtual void __fastcall SetPalette(void *){};
	virtual void __fastcall LoadFromClipboardFormat(unsigned short, unsigned int, void *){};
	virtual void __fastcall SaveToClipboardFormat(unsigned short&, unsigned int&, void *&){};
};


class TUCPngImage : public TPngImage
{
public:
	void __fastcall LoadFromFile(const UnicodeString FileName);
	void __fastcall SaveToFile(const UnicodeString FileName);

	virtual void __fastcall SetPalette(void *){};
	virtual void __fastcall LoadFromClipboardFormat(unsigned short, unsigned int, void *){};
	virtual void __fastcall SaveToClipboardFormat(unsigned short&, unsigned int&, void *&){};
};

class TUCGIFImage : public TGIFImage
{
public:
	void __fastcall LoadFromFile(const UnicodeString FileName);
	void __fastcall SaveToFile(const UnicodeString FileName);

	virtual void __fastcall SetPalette(void *){};
	virtual void __fastcall LoadFromClipboardFormat(unsigned short, unsigned int, void *){};
	virtual void __fastcall SaveToClipboardFormat(unsigned short&, unsigned int&, void *&){};
};

//----------------------------------------------------------------
//
//  �t�@�C���̍X�V�`�F�b�N�N���X
//
//----------------------------------------------------------------
/*

��CreateEvent �֐������s���邽�߂Ɏg�p�s�\

class CCheckFileChanged : public TThread
{
private:
protected:
	void __fastcall Execute();
public:
	UnicodeString FileName;
	HANDLE hNotify[2];
	int FileAgeValue;
	TForm *Form;
	UINT MessageValue;
	bool Changed;
	__fastcall CCheckFileChanged(bool CreateSuspended);
	__fastcall ~CCheckFileChanged(void);

	bool __fastcall SetMessageParameter(TForm *aForm, UnicodeString aMessageString);
	bool __fastcall Restart(void);
	bool __fastcall SetCheckFile(UnicodeString NewFileName);
	bool __fastcall StopChecking(void);
};

*/

#endif
