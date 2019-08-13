
//---------------------------------------------------------------------------
//
//	  SUSIE プラグインを C++ Builder で簡単に使用するためのクラスヘッダ
//
//					 class  TSpiload;
//
//					   Programed by CAT.Y
//
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#ifndef SpiPluginH
#define SpiPluginH

#include <vcl.h>
#include <time.h>

#include "acfc_VCLSub.h"

#define MMODE_NOSPI    1
#define MMODE_SPI      2
#define MMODE_ROTATE   4
#define MMODE_PICTURE  7

#define MMODE_ARCHIVE  16
#define MMODE_ACVINNER 32

#define MMODE_CLEARFILE 128

#define MMODE_ALL      255

#define NOSPI_BMP 1
#define NOSPI_JPG 2
#define NOSPI_PNG 3
#define NOSPI_GIF 4

#define SAFE_FREE(p)				 { if(p != NULL) { free (p);       (p) = NULL; } }
#define SAFE_DELETE(p)       { if(p != NULL) { delete (p);     (p) = NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p != NULL) { delete[] (p);   (p) = NULL; } }
#define SAFE_RELEASE(p)      { if(p != NULL) { (p)->Release(); (p) = NULL; } }



//---------------------------------------------------------------------------

// ============================================================================
//			SUSIE plug-in から情報を取得するための構造体
// ============================================================================

struct SPictureInfo
{
	long 	Left, Top;		// 展開する位置
	long 	Width;	   	// 画像の幅
	long 	Height;	  	//	   高さ
	WORD 	X_Density;   	// 画素の水平方向密度
	WORD 	Y_Density;   	// 画素の垂直方向密度
	short 	Color; 			// １画素のbit数
	HLOCAL 	Info;			// 画像のテキストヘッダ
};

struct SArchivedFileInfo
{
	unsigned char Method[8];   // 圧縮法の種類
	unsigned long Position;    // ファイル上での位置
	unsigned long CompSize;    // 圧縮されたサイズ
	unsigned long FileSize;    // 元のファイルサイズ
	time_t Timestamp;          // ファイルの更新日時
	char Path[200];            // 相対パス
	char FileName[200];        // ファイルネーム
	unsigned long CRC;         //CRC
};


// ============================================================================
//			SUSIE plug-in プロシージャ管理テーブル
// ============================================================================
struct	TSpiPlugin
{
	int	Err;

//  ハンドル
	HANDLE			SpiHandle;

// 関数ポインタ群

	// プラグインの情報を得る
	int _export pascal (*GetPluginInfo)(int infono, LPSTR buf, int buflen);

	//  展開可能なファイル形式か調べる
	int _export pascal (*IsSupported)(LPSTR filename, DWORD dw);

	//  画像ファイルに関する情報を得る　
	int _export pascal (*GetPictureInfo)(LPSTR buf, long len, unsigned int flag
										,struct SPictureInfo *lpInfo);
	//  画像を展開する
	int _export pascal (*GetPicture)(LPSTR buf, long len, unsigned int flag
											,HANDLE *pHBInfo, HANDLE *pHBm
											,FARPROC lpPrgressCallback, long lData);

	//  プレビュー・カタログ表示用画像縮小展開ルーティン *
	int _export pascal (*GetPreview)(LPSTR buf, long len, unsigned int flag
										,HANDLE *pHBInfo, HANDLE *pHBm
										,FARPROC lpPrgressCallback, long lData);


	//  アーカイブ内のすべてのファイルの情報を取得する
	int	_export pascal (*GetArchiveInfo)(LPSTR buf, long len
											,unsigned int flag, HLOCAL *lphInf);

	//  アーカイブ内のすべてのファイルの情報を取得する
	int	_export pascal (*GetFileInfo)(LPSTR buf, long len
											,unsigned int flag, HLOCAL *lphInf);

	//  アーカイブ内のファイルを取得する
	int	_export pascal (*GetFile)(LPSTR src, long len, LPSTR dest, unsigned int flag
											,FARPROC prgressCallback, long lData);

	// Plug-in設定ダイアログの表示
	int _export pascal (*ConfigurationDlg)(HWND parent, int fnc);

// Constructor
	TSpiPlugin(void){SpiHandle = NULL;};

// Destructor
	~TSpiPlugin(void){if(SpiHandle != NULL)FreeLibrary(SpiHandle);};					// ライブラリ解放

//  DLL Procedure address set
	bool LoadSpiLL(UnicodeString SpiPath)
	{
		bool Result = false;

		SpiHandle = LoadLibrary(SpiPath.c_str());					// Win32 API

		if(SpiHandle != NULL)
		{
			// === Win32 API でアドレス取得 ===
			GetPluginInfo    = (int (__stdcall *)(int infono, LPSTR buf, int buflen))GetProcAddress(SpiHandle, "GetPluginInfo");
			IsSupported	     = (int (__stdcall *)(LPSTR, DWORD))GetProcAddress(SpiHandle, "IsSupported");
			GetPictureInfo   = (int (__stdcall *)(LPSTR, long, unsigned int, struct SPictureInfo *))GetProcAddress(SpiHandle, "GetPictureInfo");
			GetPicture	     = (int (__stdcall *)(LPSTR, long, unsigned int, HANDLE *, HANDLE *, FARPROC, long))GetProcAddress(SpiHandle, "GetPicture");
			GetPreview	     = (int (__stdcall *)(LPSTR, long, unsigned int, HANDLE *, HANDLE *, FARPROC, long))GetProcAddress(SpiHandle, "GetPreview");
			GetArchiveInfo   = (int (__stdcall *)(LPSTR, long,unsigned int, HLOCAL *))GetProcAddress(SpiHandle, "GetArchiveInfo");
			GetFileInfo      = (int (__stdcall *)(LPSTR, long,unsigned int, HLOCAL *))GetProcAddress(SpiHandle, "GetFileInfo");
			GetFile		       = (int (__stdcall *)(LPSTR, long, LPSTR, unsigned int, FARPROC, long))GetProcAddress(SpiHandle, "GetFile");
			ConfigurationDlg = (int (__stdcall *)(HWND, int))GetProcAddress(SpiHandle, "ConfigurationDlg");

			// === 簡易エラーチェック ===
			Result = (IsSupported != NULL) & (GetPicture != NULL);
			if (Result == false)
				Result = GetLastError();
		}
		GetPluginInfo(0, APIVersion, 5);
		GetPluginInfo(1, PluginName, 200);
		return(Result);
	};
	BYTE APIVersion[5];
	BYTE PluginName[200];
};


// ============================================================================
//			SUSIE plug-in 画像取得管理クラス
// ============================================================================
class	TSpiLoader
{
protected:
	bool ErrorFlag;				// エラー時表示フラグ
	UnicodeString FileName;			 // File name
	UnicodeString SubFileName; // Archive 中のFile Name
	UnicodeString PostFileName;	 // 読込中の File name
	AnsiString PostFileExt; // Susie に渡す用の FileExt

	//
	TSpiPlugin **Spi;				// プラグインリスト
	TSpiPlugin *NowSpi;			// 最後に確定したプラグイン
	TSpiPlugin *PostSpi;    // 次に読むこ無事が確定した Spi プラグイン
	TSpiPlugin *ArchiveSpi; // 現在解凍に使っている Spi プラグイン

	// ハンドルデータ
	HANDLE HBmpData, HBmpInfo; // 現在表示中のハンドル
	HANDLE old_HBmpData, old_HBmpInfo; //

	// ポインタデータ
	void *old_pBmpData;
	BITMAPINFO *old_pBmpInfo;

	HANDLE HFileImage, old_HFileImage;
	BYTE *pFileImage, *old_pFileImage;

	int SpiNum;

	SArchivedFileInfo *ArchivedFileInfo, *old_ArchivedFileInfo;

	bool Refresh(void);

	int SetShowSubIndex(int SubIndex, FARPROC ProgressCallback);
	bool LoadFromFileInMemory(BYTE *FileData, int DataSize, FARPROC ProgressCallback);

	bool CreateDBuffer(void);
	bool DeleteDBuffer(void);
	bool DoubleBuffered;

public:
	void *pBmpData; // 読み込んだ画像のオリジナルデータ
	BITMAPINFO *pBmpInfo; // 同上

private:
	void *pDBuf;
	BITMAPINFO DBufInfo;
	HDC HDBufDC;

	void *pRBuf;
	BITMAPINFO RBufInfo;
	HBITMAP HRBuf;

	HBITMAP HDBuf;
	__int64 DBMaxSize, DBFailSize;

	bool CreateRotateBuffer(void);
	bool DoOffsetRotate(int Rotate);

	TUCBitmap *BMPImage, *old_BMPImage;
	TUCJPEGImage *JPEGImage;
	TUCPngImage *PNGImage;
	TUCGIFImage *GIFImage, *old_GIFImage;
	TGIFRenderer *GIFRenderer;

	bool Showing;

public:
	bool GIFAnimate;
	UnicodeString PluginName;

	int	Err;					// Win32 エラー番号
	int Width;
	int Height;
	int Mode;
	int DWidth, DHeight;
	int OWidth, OHeight;

	BITMAPINFO *pBitmapInfo;
	void *pBitmapData;
	FARPROC ProgressCallback;

	TSpiLoader(void *aProgressCallback);								// Constructor
	~TSpiLoader(void);								// Destructor

	void SetSpiPathes(TStringList *Pathes);	// SPI のパスを設定
	bool SetImageFile(UnicodeString Src);		// file name set & load
	int CheckFileType(UnicodeString FileName, BYTE *FileData, DWORD FileSize); // ファイルの種類を調べる
	int CheckFileTypeSPI(UnicodeString SrcFileName, BYTE *FileData); // SPI で読み込めるかどうかチェック
	int CheckFileTypeVNR(UnicodeString SrcFileName, BYTE *FileData); // 素で読み込めるかどうかチェック
	int CheckGifAnime(UnicodeString SrcFileName, BYTE *FileData, DWORD FileSize); // Gif アニメかどうかチェック

	void AnimateGIF(void);
	bool ChangeArchiveFileName(UnicodeString NewFileName);

	void Clear(int DelMode);					// 廃棄可能なﾃﾞｰﾀを廃棄

	int ImageNum;
	int *Rotate;
	int RotateValue;
	bool FixRotate;

	bool NoRotationRefresh;

	int LoadSubIndex(int &SubIndex, int Ofs);

	bool AbsoluteRotate(int Value);
	bool OffsetRotate(int Value);

	bool GetArchiveFileLists(TStringList *SubFileLists); // ファイルリストを取得

	int SetSubImageFile(UnicodeString Src);
	int SetSubImageFile(TStringList *FileLists, int &i, int Ofs);
	int CreateDBImage(int SWidth, int SHeight);
	bool SetDoubleBufferMode(bool aMode);

	bool CreateReSizeBuffer(void);
	bool DeleteReSizeBuffer(void);
	bool StartReSizeMode(void);
	bool EndReSizeMode(void);

	short int GetOrientation(BYTE *pTemp, int Size);
	short int GetOrientationExif(BYTE *pTemp, int Size);

	void OpenSpiSettingDialog(HWND hwnd);

	bool CopyImageToClipboard(void);

	int NOSPIType, PostNOSPIType;

	bool SpiPrecede;
};

//---------------------------------------------------------------------------

struct SImageInfo
{
	time_t Timestamp;
	size_t FileSize;
	int Rotate;
};

#endif
