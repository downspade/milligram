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
//  ファイル操作関係
//
//----------------------------------------------------------------


// エンディアンの変更
void Endian(void *Dat, int Size, int Mode);

// ディレクトリが存在するかどうかチェックする
bool DirExists(UnicodeString Dir);

// フォルダの文字を変更する(\ を / に変更する)
UnicodeString ChangeFolderSymbol(UnicodeString Str);

// Directory に使えない文字を削除する
UnicodeString CorrectDirString(UnicodeString Str, bool CanBackSlash);

// 数字以外の文字を削除する(正の整数のみ)
UnicodeString CorrectNumericString(UnicodeString Str);

// 数字以外の文字を削除する(オプション付き)
UnicodeString CorrectNumericString(UnicodeString Src, bool CanMinus, bool CanFloat, UnicodeString MaxN, UnicodeString MinN);


// 指定したファイルが指定したマスクを通るかどうか調べる
bool CheckMasks(UnicodeString FileName, UnicodeString Mask); // true ならマスクに含まれる

// フォルダを削除する
bool DeleteFolder(UnicodeString Dir);

// ファイルを移動する Srcフォルダから Destフォルダに Type(*.?) を移動する
bool MoveFiles(UnicodeString Src, UnicodeString Dest, UnicodeString Type, bool OverWrite);

// ファイルリストを取得する(Escキーで抜けられる)
bool GetFileLists(UnicodeString Src, TStringList *Dest, UnicodeString Mask, int Depth); // c:\windows\dest\*.dll のような形式で Src は指定する
bool GetFileLists_(UnicodeString Src, TStringList *Dest, UnicodeString Mask, int Depth, TStringList *TempSL, bool &Esc);


bool GetFileListsDialog(UnicodeString Src, TStringList *Dest, UnicodeString Mask, int Depth, TApplication *Application, TLabel *Label); // c:\windows\dest\*.dll のような形式で Src は指定する
bool GetFileListsDialog_(UnicodeString Src, TStringList *Dest, UnicodeString Mask, int Depth, TStringList *TempSL, bool &Esc, TApplication *Application, TLabel *Label);


// ファイルをコピーする(フォルダも含む)
bool CopyFiles(UnicodeString Src, UnicodeString Dest, UnicodeString Type, bool OverWrite);

// ファイルだけコピーする
bool CopyOnlyFiles(UnicodeString Src, UnicodeString Dest, UnicodeString Type, bool OverWrite);
void CopyFolderFiles(UnicodeString Src, UnicodeString Dest, UnicodeString Type);

// Src フォルダ内のファイルを削除してファイルが無くなればフォルダを削除
void DeleteFolderFiles(UnicodeString Src, UnicodeString Dest, UnicodeString Type);

// Src フォルダにあるファイルのうち Dest フォルダにあるものを Dest フォルダにコピーする
void ReplaceFolderFiles(UnicodeString Src, UnicodeString Dest, UnicodeString Type);

// ３種類のタイムスタンプをすべて指定した値にする
void SetAllTimeStamp(int Year, int Month, int Day, int Hour, int Minute, int Second, int Milliseconds, UnicodeString DestFileName);

// Src ファイルと同じ時間にタイムスタンプを設定する
void SetSameTimeAll(UnicodeString SrcFileName, UnicodeString DestFileName);

// 更新日時を取得する
void GetFileTimeStamp(UnicodeString FileName, FILETIME *fRefreshTime);

// ファイルを選択してエクスプローラーで表示
void SelectFileInExplorer(UnicodeString Src);

// ファイルサイズを取得する
size_t GetFileSize(UnicodeString Target);

// 指定したディレクトリの数だけディレクトリ文字列を削除
UnicodeString DeleteDirString(UnicodeString Dir, int Count);

// 相対パスを絶対パスに変換
UnicodeString GetAbsoluteFilePath(UnicodeString Root, UnicodeString Relative);

// (1) (2) 形式で重複しないファイル名を取得する
UnicodeString GetNonOverwrapFileName(UnicodeString Src);

// Msg からドロップされたファイル名を１つ取得する
UnicodeString GetDropFileName(TMessage &Msg);
UnicodeString GetDropFileName(TMessage &Msg, bool Original);

// リンクファイルから本体のファイルを取得する
UnicodeString GetFileFromLink(UnicodeString Path);

// 実行ファイルのリンクファイルを作成する
HRESULT CreateLink(UnicodeString OriginalFile, UnicodeString ShortCutFile, UnicodeString Params);

// 環境変数文字列を展開し、その文字列を定義された値に置き換えます %PATH% みたいなのを取得する
UnicodeString AsExpandEnvironmentStrings(UnicodeString Src);

// AsExpandEnviromentStrings の逆
UnicodeString AsPathUnExpandEnvStrings(UnicodeString Src);

// Src と Dest のフォルダを比較する Dest フォルダが無かった場合には強制作成する
bool SyncDir(UnicodeString SrcDir, UnicodeString DestDir, UnicodeString MaskStr, int Option);

// (SyncDir から呼ばれる) Src と Dest のフォルダを比較して（オプションによるが）新しいファイルのみを上書きコピーする
bool CheckDirNew(UnicodeString SrcDir, UnicodeString DestDir, UnicodeString MaskStr, int Option, UnicodeString Dir);

// (SyncDir から呼ばれる) Src と Dest のフォルダを比較して削除されたフォルダを削除する
bool CheckDirDeleted(UnicodeString SrcDir, UnicodeString DestDir, UnicodeString MaskStr, int Option, UnicodeString Dir);

// ダイアログを表示してフォルダを取得する
UnicodeString GetFolderByDialog(UnicodeString DefaultDir, HWND Handle, UnicodeString Message);

// ファイルをクリップボードにコピーペースト
void FilesToClipboard(HWND Handle, TStringList *Src, int Flag);
int FilesFromClipboard(TStringList *DestSL);

// ファイルをゴミ箱に移動する
bool DeleteFileToRecycle(UnicodeString DelFile, bool Confirm, HWND Handle);
bool DeleteFileToRecycle(TStringList *DelFiles, bool Confirm);

// ini ファイルのファイル名やフォルダを取得する
UnicodeString GetIniFolder(void);
UnicodeString GetIniFileName(UnicodeString IniName);

// ファイル名に使えない文字を得る -1:なし 0~:そのインデックスの文字
int CheckFileIrregularChar(UnicodeString FileName);

// ファイル名から省略したファイル名を取得する C:\Pool\test\gone\done\abc.txt → done\abc.txt
UnicodeString __fastcall GetShortFileName(UnicodeString Temp, int Count);

// フォルダを数えた数だけ上がる
UnicodeString __fastcall DirectoryUpward(UnicodeString FullPath, int Count);


//----------------------------------------------------------------
//
//  その他
//
//----------------------------------------------------------------

// Dest 文字が SrcTS 文字郡内にあるかどうかチェックする
bool CheckStrings(TStringList *SrcTS, UnicodeString Dest);
bool CheckStrings(TStringList *SrcTS, UnicodeString Dest, int &Pos);
UnicodeString CheckStringValue(TStringList *SrcTS, UnicodeString Dest);
UnicodeString CheckStringValue(TStringList *SrcTS, UnicodeString Dest, int &Pos);

// Dest文字列＝値 の行を削除する Pos はその行
bool DeleteStringValue(TStringList *SrcTS, UnicodeString Dest, int &Pos);

// 数値を取得
int GetIntegerValue(TStringList *SrcTS, UnicodeString Dest, int Src, int Low, int Hi);
int GetIntegerValue(TStringList *SrcTS, UnicodeString Dest, int Src, int Low, int Hi, int &Pos);
double GetDoubleValue(TStringList *SrcTS, UnicodeString Dest, double Src, double Low, double Hi);
double GetDoubleValue(TStringList *SrcTS, UnicodeString Dest, double Src, double Low, double Hi, int &Pos);

// Bool 値を取得
bool GetBoolValue(TStringList *SrcTS, UnicodeString Dest, bool Src);
bool GetBoolValue(TStringList *SrcTS, UnicodeString Dest, bool Src, int &Pos);

// 文字列を取得
UnicodeString GetStringValue(TStringList *SrcTS, UnicodeString Dest, UnicodeString Src);
UnicodeString GetStringValue(TStringList *SrcTS, UnicodeString Dest, UnicodeString Src, int &Pos);



// Dest 文字が SrcUS 文字郡内にあるかどうかチェックする
bool CheckTabStrings(UnicodeString SrcUS, UnicodeString Dest);
UnicodeString CheckTabStringValue(UnicodeString SrcUS, UnicodeString Dest);

// 数値を取得
int GetTabIntegerValue(UnicodeString SrcUS, UnicodeString Dest, int Src, int Low, int Hi);
double GetTabDoubleValue(UnicodeString SrcUS, UnicodeString Dest, double Src, double Low, double Hi);

// Bool 値を取得
bool GetTabBoolValue(UnicodeString SrcUS, UnicodeString Dest, bool Src);

// 文字列を取得
UnicodeString GetTabStringValue(UnicodeString SrcUS, UnicodeString Dest, UnicodeString Src);




// 数値をメモリサイズの文字列に変更する 1024=1Kbyte のように変換される
UnicodeString IntToMemorySize(__int64 tSize);

// 数値を桁数有りの文字列に変換する 5 = 005 のように変換される
UnicodeString IntToDigitStr(int Value, int Digit);

// 前面にウィンドウを移動する
void SetAbsoluteForegroundWindow(HWND hWnd);

// パディングサイズを計算する
inline int CalPaddingSize(int SrcSize, int SegSize){return((SrcSize + (SrcSize - 1)) / SegSize);};

//----------------------------------------------------------------
//
// CDragDrop ドラッグドロップ関係
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
	Dropper->Init(コントロールへのポインタ);
	コントロールへのポインタ->WindowProc = DropProc; // フォームのほうに記入

// Release
	Dropper->Release(コントロールへのポインタ);
	delete Dropper;

void __fastcall TMainForm::DropProc(TMessage& Msg)
{
	if(Msg.Msg == WM_DROPFILES)
	{
		UnicodeString FileName = Dropper->GetDropFileName(Msg);
		if(ExtractFileExt(FileName) == ".dmp")
		{
			// 処理する
		}
	}
	else
		Dropper->oldWinProc(Msg);
}
*/

//----------------------------------------------------------------
//
// CCriticalSection 排他処理
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
// CBinaryBlock 文字列をバイナリのデータに変換する
//
//----------------------------------------------------------------

class CBinaryBlock
{
// CBinaryBlock クラス。文字列をバイナリのデータに変換する
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
//  レジストリアクセスクラス
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
//  Unicode 対応画像クラス
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
//  ファイルの更新チェッククラス
//
//----------------------------------------------------------------
/*

※CreateEvent 関数が失敗するために使用不可能

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
