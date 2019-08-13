#pragma once

#include <vector>
#include <string>
#include <thread>
#include <comdef.h>
#include <gdiplus.h>
#include <filesystem>
#include <iostream>
#include <shobjidl.h>
#include <shlguid.h>
#include <time.h>
#include <mmsystem.h>
#include <ShellAPI.h>
#include <mutex>

#pragma comment( lib, "Shell32.lib" )
#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus; 

namespace milligram
{
	enum EPluginMode : int
	{
		EPluginMode_NONE = 0,
		EPluginMode_GDIP = 1,
		EPluginMode_SPI = 2,
		EPluginMode_PICTURE = 3,
		EPluginMode_ARCHIVE = 16,
		EPluginMode_ACVINNER = 32,
		EPluginMode_CLEARFILE = 128,
		EPluginMode_ALL = 255,
	};

	enum EWorkFileType : int
	{
		EWorkFileType_NONE = 0,
		EWorkFileType_GDIP_PICTURE = 1,
		EWorkFileType_GDIP_ARCHIVE = 2,
		EWorkFileType_SPI_PICTURE = 3,
		EWorkFileType_SPI_ARCHIVE = 4,
	};


	enum ENoSPIPictureType : int
	{
		ENoSPIPictureType_NONE = 0,
		ENoSPIPictureType_BMP = 1,
		ENoSPIPictureType_JPG = 2,
		ENoSPIPictureType_PNG = 3,
		ENoSPIPictureType_GIF = 4,
		ENoSPIPictureType_TIF = 5,
	};


	enum ESPIResult : int
	{
		ESPIResult_UNKNOWN = -1,
		ESPIResult_IMPORTFILTER = 0,
		ESPIResult_EXPORTFILTER = 1,
		ESPIResult_ARCHIVE = 2,
	};

	struct SPictureInfo
	{
		long Left;     // 展開する位置
		long Top;
		long Width;     // 画像の幅
		long Height;        //	   高さ
		WORD X_Density;     // 画素の水平方向密度
		WORD Y_Density;     // 画素の垂直方向密度
		short Color;            // １画素のbit数
		HLOCAL Info;            // 画像のテキストヘッダ
	};

	struct SArchivedFileInfo
	{
		unsigned char Method[8];   // 圧縮法の種類
		unsigned long Position;    // ファイル上での位置
		unsigned long CompSize;    // 圧縮されたサイズ
		unsigned long FileSize;    // 元のファイルサイズ
		__time32_t Timestamp;          // ファイルの更新日時
		char Path[200];            // 相対パス
		char FileName[200];        // ファイルネーム
		unsigned long CRC;         //CRC
	};


	// 画像の情報と表示方法を保存するための構造体
	struct CImageInfo
	{
	public:
		std::wstring FileName;
		time_t Timestamp;
		unsigned int FileSize;
		int Rotate;
		std::vector<CImageInfo> ImageInfoList;

		CImageInfo(void)
		{
			Clear();
		}
		
		void Clear(void)
		{
			FileName = L"";
			Timestamp = 0;
			FileSize = 0;
			Rotate = -1;
			ImageInfoList.clear();
		};
	};

	class CSpiPlugin
	{
	public:
		int _stdcall CallbackProsedure(int nNum, int nDenom, unsigned int lData);

		int	Err = 0;

		//  ハンドル
		HMODULE SpiHandle = nullptr;

		// 関数ポインタ型定義
		// プラグインの情報を得る
		typedef int (PASCAL *mGetPluginInfo)(int infono, LPSTR buf, int buflen);

		//  展開可能なファイル形式か調べる
		typedef int (PASCAL *mIsSupported)(LPSTR filename, DWORD dw);

		//  画像ファイルに関する情報を得る　
		typedef int (PASCAL *mGetPictureInfo)(LPSTR buf, long len, unsigned int flag
			, struct SPictureInfo *lpInfo);
		//  画像を展開する
		typedef int (PASCAL *mGetPicture)(LPSTR buf, long len, unsigned int flag
			, HANDLE *pHBInfo, HANDLE *pHBm
			, FARPROC lpPrgressCallback, long lData);

		//  プレビュー・カタログ表示用画像縮小展開ルーティン *
		typedef int (PASCAL *mGetPreview)(LPSTR buf, long len, unsigned int flag
			, HANDLE *pHBInfo, HANDLE *pHBm
			, FARPROC lpPrgressCallback, long lData);


		//  アーカイブ内のすべてのファイルの情報を取得する
		typedef int	(PASCAL *mGetArchiveInfo)(LPSTR buf, long len
			, unsigned int flag, HLOCAL *lphInf);

		//  アーカイブ内のすべてのファイルの情報を取得する
		typedef int	(PASCAL *mGetFileInfo)(LPSTR buf, long len
			, unsigned int flag, HLOCAL *lphInf);

		//  アーカイブ内のファイルを取得する
		typedef int	(PASCAL *mGetFile)(LPSTR src, long len, LPSTR dest, unsigned int flag
			, FARPROC prgressCallback, long lData);

		// Plug-in設定ダイアログの表示
		typedef int (PASCAL *mConfigurationDlg)(HWND parent, int fnc);

		mGetPluginInfo GetPluginInfo = nullptr;
		mIsSupported IsSupported = nullptr;
		mGetPictureInfo GetPictureInfo = nullptr;
		mGetPicture GetPicture = nullptr;
		mGetPreview GetPreview = nullptr;
		mGetArchiveInfo GetArchiveInfo = nullptr;
		mGetFileInfo GetFileInfo = nullptr;
		mGetFile GetFile = nullptr;
		mConfigurationDlg ConfigurationDlg = nullptr;

		// Constructor
		CSpiPlugin(void) { SpiHandle = nullptr; };

		// Destructor
		~CSpiPlugin(void) { if (SpiHandle != nullptr)FreeLibrary(SpiHandle); };					// ライブラリ解放

		//  DLL Procedure address set
		bool LoadSpiLL(WCHAR *SpiPath)
		{
			memset(this, 0, sizeof(CSpiPlugin));
			bool Result = false;

			SpiHandle = LoadLibrary(SpiPath);					// Win32 API

			if (SpiHandle != nullptr)
			{
				// === Win32 API でアドレス取得 ===
				GetPluginInfo = (int(PASCAL *)(int infono, LPSTR buf, int buflen))GetProcAddress(SpiHandle, "GetPluginInfo");
				IsSupported = (int(PASCAL *)(LPSTR, DWORD))GetProcAddress(SpiHandle, "IsSupported");
				GetPictureInfo = (int(PASCAL *)(LPSTR, long, unsigned int, struct SPictureInfo *))GetProcAddress(SpiHandle, "GetPictureInfo");
				GetPicture = (int(PASCAL *)(LPSTR, long, unsigned int, HANDLE *, HANDLE *, FARPROC, long))GetProcAddress(SpiHandle, "GetPicture");
				GetPreview = (int(PASCAL *)(LPSTR, long, unsigned int, HANDLE *, HANDLE *, FARPROC, long))GetProcAddress(SpiHandle, "GetPreview");
				GetArchiveInfo = (int(PASCAL *)(LPSTR, long, unsigned int, HLOCAL *))GetProcAddress(SpiHandle, "GetArchiveInfo");
				GetFileInfo = (int(PASCAL *)(LPSTR, long, unsigned int, HLOCAL *))GetProcAddress(SpiHandle, "GetFileInfo");
				GetFile = (int(PASCAL *)(LPSTR, long, LPSTR, unsigned int, FARPROC, long))GetProcAddress(SpiHandle, "GetFile");
				ConfigurationDlg = (int(PASCAL *)(HWND, int))GetProcAddress(SpiHandle, "ConfigurationDlg");

				GetPluginInfo(0, APIVersion, 5);
				GetPluginInfo(1, PluginName, 200);
				
				Result = true;
			}
			return(Result);
		};
		CHAR APIVersion[5];
		CHAR PluginName[200];
	};

	class CSpiLoader
	{
	public:
		bool ErrorFlag = false;             // エラー時表示フラグ
		CImageInfo ImageInfo;          // File name
		CImageInfo SubImageInfo; // Archive 中のFile Name
		CImageInfo PostImageInfo;  // 読込中の File name
		std::string PostFileExt = ""; // Susie に渡す用の FileExt

		//
		std::vector<CSpiPlugin *> Spi;               // プラグインリスト
		CSpiPlugin *NowSpi = nullptr;         // 最後に確定したプラグイン
		CSpiPlugin *PostSpi = nullptr;    // 次に読むこむ事が確定した Spi プラグイン
		CSpiPlugin *ArchiveSpi = nullptr; // 現在解凍に使っている Spi プラグイン

		HBITMAP hBitmap = nullptr; // 読み込まれた hBitmap オリジナル

		BITMAPINFO *pBmpInfo = nullptr; // バックバッファのビットマップインフォ
		HANDLE pBmpData = nullptr; // バックバッファの ビットマップデータ先頭アドレス
		HANDLE hBmpData = nullptr; // バックバッファのハンドル


		// ハンドルデータ
		HANDLE HFileImage = nullptr, pre_HFileImage = nullptr; // Susie プラグインが持ってくるハンドル
		BYTE *pFileImage = nullptr, *pre_pFileImage = nullptr; // データへのポインタ

		std::vector<SArchivedFileInfo> nowArchivedFileInfo;
		std::vector<SArchivedFileInfo> oldArchivedFileInfo;

		//
		EWorkFileType LoadingFileType = EWorkFileType_NONE;

		// ポインタデータ
		ULONG_PTR GdiplusToken;
		Bitmap *BitmapGDIP = nullptr;
		Image *ImageGDIP = nullptr;

		HGLOBAL hMemoryGDIP = nullptr;
		IStream *StreamGDIP = nullptr;
		BYTE *pMemoryGDIP = nullptr;

		// GIF アニメーション関連
		bool GIFAnimate = false;
		bool PostGIFAnimate = false;
		int LoopCount = 0;
		int LoopIndex = 0;
		int FrameCount = 0;
		int FrameIndex = 0;
		int *Delay = nullptr;
		int DelayTime = 0;
		int DropFrame = 0;
		int DropCount = 0;
		bool GIFLooping = false;

		bool Showing = false; // 表示されているかどうか

		int Err = 0;                    // Win32 エラー番号
		int OrgWidth = 200, OrgHeight = 200; // もと画像の大きさ
		int ORotWidth = 200, ORotHeight = 200; // 転送もと画像のかたち(回転後)

		int BufWidth, BufHeight; // 転送もとのフォームの大きさ（回転前）
		int SrcRWidth, SrcRHeight;   // 転送もとの大きさ(回転後)
		int BufMaxWidth, BufMaxHeight; // バックバッファーの画像の大きさ
		EPluginMode Mode = EPluginMode_NONE;

		int MaxBBufferPixel = 4000 * 4000; // バックバッファーの最大ピクセルサイズ 4000 x 4000 は 64M

		FARPROC ProgressCallback = nullptr;
		HWND windowHandle = nullptr;

	public:
		int ImageNum = 0;
		int Rotate = -1; // 現在の回転値
		bool FixRotate = false; // 回転を固定するかどうか
		bool NoRotationRefresh = false;

		ENoSPIPictureType NOSPIType = ENoSPIPictureType_NONE;
		ENoSPIPictureType PostNOSPIType = ENoSPIPictureType_NONE;

		std::wstring InternalLoader = TEXT("");

		HANDLE FormHandle = nullptr;

		std::wstring PluginName = TEXT("");
	private:
		time_t ShortestPeriod; // 計算できる最小間隔
		time_t NowTGT;
		time_t PreTGT;
		time_t PausedTGT;

		//---------------------------------------------------------------------------
	public:
		// コンストラクタ
		CSpiLoader(void);
		// デストラクタ
		~CSpiLoader(void);

		// 初期化
		void Init(HWND hWndNewOwner, FARPROC aProgressCallBack);

		// すべてのプラグインをクリアする
		void ClearAllPlugins(void);

		// SPi のパスを設定する
		void SetSpiPathes(std::vector<std::wstring> &Paths);

		bool Refresh(void);
		bool SetImageFile(CImageInfo &SrcImageInfo);
		void InitSize(int iWidth, int iHeight);
		bool CheckFileType(CImageInfo &SrcImageInfo, BYTE* FileData, long FileSize);
		bool CheckFileTypeSPI(CImageInfo &SrcImageInfo, BYTE* FileData);

		// GIF アニメファイルかどうか調べる
		bool CheckGIFAnimeEnabled(CImageInfo &SrcImageInfo, BYTE* FileData, long FileSize);
		// GDI+ でファイルタイプを調べる
		bool CheckFileTypeGDIP(CImageInfo &SrcImageInfo, BYTE* FileData);
		// メモリからファイルを読み込む
		bool LoadFromFileInMemory(CImageInfo &SrcImageInfo, BYTE* FileData, UINT DataSize, FARPROC ProgressCallback);

		// アーカイブファイルの名前を変更する
		bool ChangeArchiveFileName(std::wstring NewFileName);

		// イメージオブジェクトからビットマップオブジェクトを作る
		Gdiplus::Bitmap* CreateBMPFromImage(Gdiplus::Image *Src);

		// アーカイブファイル中のインデックスの番号のファイルを読み込む
		int LoadSubIndex(int &SubIndex, int Ofs);
		// バックバッファがその画像の幅高さで使用可能かどうか調べる
		void CheckBackBuffer(int sWidth, int sHeight);
		// フォームのサイズを指定する
		bool SetFormSize(int iWidth, int iHeight);
		// バックバッファにデータを転送する
		bool TransBackBuffer(void);

		// 絶対値で画像を回転する
		bool AbsoluteRotate(int Value);
		// 相対値で画像を回転する
		bool OffsetRotate(int Value);

		// GIF アニメーションを一時中断する
		bool PauseGIFAnimate(void);
		// GIF アニメーションを再度再生する
		bool RestartGIFAnimate(void);
		// GIF アニメのフレームを次に進める
		bool GIFAnimateUpDateFrame(bool FrameSkip);
		
		// 回転情報をチェックする
		int CheckOrientation(void);
		// 回転情報をチェックsル
		short CheckOrientation(BYTE* pTemp, unsigned int Size);
		// 回転情報を Exif からひろう
		short CheckOrientationExif(BYTE* pTemp, unsigned int Size);
		// GIF アニメデータをチェックする
		bool GetGIFAnimeData(void);
		
		// アーカイブファイ中のインデックスの番号のファイルを読み込む
		bool SetShowSubIndex(CImageInfo &SrcImageInfo, int SubIndex, FARPROC ProgressCallback);
		// アーカイブファイルの一覧を得る
		bool GetArchiveFileList(std::vector<CImageInfo> &SubFileList); // ファイルリストを取得

		// アーカイブファイル中の画像を読み込む
		bool SetSubImageFile(CImageInfo &Src);
		// アーカイブファイル中の画像を読み込む
		bool SetSubImageFile(std::vector<CImageInfo> &FileList, int &i, int Ofs);

		// データをクリアする
		void Clear(EPluginMode DelMode);

		// バックバッファーを消去する
		void DeleteBackBuffer(void);
		// プラグインダイアログを表示する
		void OpenSpiSettingDialog(HWND hwnd);

		// ビットマップにデータをコピーする
		Gdiplus::Bitmap* DuplicateImage(int newWidth, int newHeight);
		// クリップボードに画像をコピーする
		bool CopyImageToClipboard(void);
		// JPEG ファイルで保存する
		bool SaveJpeg(std::wstring svFileName, int svWidth, int svHeight, int svCompLevel);
		// PNG ファイルで保存する
		bool SavePNG(std::wstring svFileName, int svWidth, int svHeight);


};


}


