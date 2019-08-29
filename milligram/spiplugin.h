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

#ifdef _WIN64
#include "webp\\x64\\decode.h"
#include "webp\\x64\\demux.h"
#pragma comment(lib, "webp\\x64\\libwebp.lib" )
#pragma comment(lib, "webp\\x64\\libwebpdemux.lib" )
#else
#include "webp\\x86\\decode.h"
#include "webp\\x86\\demux.h"
#pragma comment(lib, "webp\\x86\\libwebp.lib" )
#pragma comment(lib, "webp\\x86\\libwebpdemux.lib" )
#endif

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
		EPluginMode_WEBP = 4,
		EPluginMode_PICTURE = 31,
		EPluginMode_ARCHIVE = 32,
		EPluginMode_ACVINNER = 64,
		EPluginMode_CLEARFILE = 128,
		EPluginMode_ALL = 255,
	};

	enum EWorkFileType : int
	{
		EWorkFileType_NONE = 0,
		EWorkFileType_GDIP_PICTURE = 1,
		EWorkFileType_WEBP = 2,
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
		ENoSPIPictureType_WEBP = 6,
	};

	enum EAnimationType : int
	{
		EAnimationType_NONE = 0,
		EAnimationType_GIF = 1,
		EAnimationType_WEBP = 2,
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
#ifdef _WIN64
		char  dummy[2]; // アラインメント
#endif
		HLOCAL Info;            // 画像のテキストヘッダ
	};

	struct SArchivedFileInfo
	{
		unsigned char Method[8];   // 圧縮法の種類
		ULONG_PTR Position;    // ファイル上での位置
		ULONG_PTR CompSize;    // 圧縮されたサイズ
		ULONG_PTR FileSize;    // 元のファイルサイズ
		__time32_t Timestamp;          // ファイルの更新日時
		char Path[200];            // 相対パス
		char FileName[200];        // ファイルネーム
		unsigned long CRC;         //CRC
#ifdef _WIN64
		// 64bit版の構造体サイズは444bytesですが、実際のサイズは
		// アラインメントにより448bytesになります。環境によりdummyが必要です。
		char dummy[4];
#endif
	};

	struct SArchivedFileInfoW
	{
		unsigned char Method[8];   // 圧縮法の種類
		ULONG_PTR Position;    // ファイル上での位置
		ULONG_PTR CompSize;    // 圧縮されたサイズ
		ULONG_PTR FileSize;    // 元のファイルサイズ
		__time32_t Timestamp;          // ファイルの更新日時
		WCHAR Path[200];            // 相対パス
		WCHAR FileName[200];        // ファイルネーム
		unsigned long CRC;         //CRC
#ifdef _WIN64
	   // 64bit版の構造体サイズは844bytesですが、実際のサイズは
	   // アラインメントにより848bytesになります。環境によりdummyが必要です。
		char dummy[4];
#endif
	};


	// 画像の情報と表示方法を保存するための構造体
	struct CImageInfo
	{
	public:
		std::wstring FileName;
		time_t Timestamp;
		size_t FileSize;
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
		typedef int (PASCAL *mGetPluginInfoW)(int infono, LPCWSTR buf, int buflen);

		//  展開可能なファイル形式か調べる
		typedef int (PASCAL *mIsSupported)(LPCSTR filename, void * dw);
		typedef int (PASCAL *mIsSupportedW)(LPCWSTR filename, void * dw);

		//  画像ファイルに関する情報を得る　
		typedef int (PASCAL *mGetPictureInfo)(LPCSTR buf, LONG_PTR len, unsigned int flag
			, struct SPictureInfo *lpInfo);
		typedef int (PASCAL *mGetPictureInfoW)(LPCWSTR buf, LONG_PTR len, unsigned int flag
			, struct SPictureInfo *lpInfo);

		//  画像を展開する
		typedef int (PASCAL *mGetPicture)(LPCSTR buf, LONG_PTR len, unsigned int flag
			, HANDLE *pHBInfo, HANDLE *pHBm
			, FARPROC lpPrgressCallback, LONG_PTR lData);
		typedef int (PASCAL *mGetPictureW)(LPCWSTR buf, LONG_PTR len, unsigned int flag
			, HANDLE *pHBInfo, HANDLE *pHBm
			, FARPROC lpPrgressCallback, LONG_PTR lData);


		//  プレビュー・カタログ表示用画像縮小展開ルーティン *
		typedef int (PASCAL *mGetPreview)(LPCSTR buf, LONG_PTR len, unsigned int flag
			, HANDLE *pHBInfo, HANDLE *pHBm
			, FARPROC lpPrgressCallback, LONG_PTR lData);
		typedef int (PASCAL *mGetPreviewW)(LPCWSTR buf, LONG_PTR len, unsigned int flag
			, HANDLE *pHBInfo, HANDLE *pHBm
			, FARPROC lpPrgressCallback, LONG_PTR lData);


		//  アーカイブ内のすべてのファイルの情報を取得する
		typedef int	(PASCAL *mGetArchiveInfo)(LPCSTR buf, LONG_PTR len
			, unsigned int flag, HLOCAL *lphInf);
		//  アーカイブ内のすべてのファイルの情報を取得する
		typedef int	(PASCAL *mGetArchiveInfoW)(LPCWSTR buf, LONG_PTR len
			, unsigned int flag, HLOCAL *lphInf);


		//  アーカイブ内のすべてのファイルの情報を取得する
		typedef int	(PASCAL *mGetFileInfo)(LPCSTR buf, LONG_PTR len
			, unsigned int flag, HLOCAL *lphInf);
		typedef int	(PASCAL *mGetFileInfoW)(LPCWSTR buf, LONG_PTR len
			, unsigned int flag, HLOCAL *lphInf);

		//  アーカイブ内のファイルを取得する
		typedef int	(PASCAL *mGetFile)(LPCSTR src, LONG_PTR len, LPSTR dest, unsigned int flag
			, FARPROC prgressCallback, LONG_PTR lData);
		typedef int	(PASCAL *mGetFileW)(LPCWSTR src, LONG_PTR len, LPWSTR dest, unsigned int flag
			, FARPROC prgressCallback, LONG_PTR lData);

		// Plug-in設定ダイアログの表示
		typedef int (PASCAL *mConfigurationDlg)(HWND parent, int fnc);

		mGetPluginInfo GetPluginInfo = nullptr;
		mGetPluginInfoW GetPluginInfoW = nullptr;
		mIsSupported IsSupported = nullptr;
		mIsSupportedW IsSupportedW = nullptr;
		mGetPictureInfo GetPictureInfo = nullptr;
		mGetPictureInfoW GetPictureInfoW = nullptr;
		mGetPicture GetPicture = nullptr;
		mGetPictureW GetPictureW = nullptr;
		mGetPreview GetPreview = nullptr;
		mGetPreviewW GetPreviewW = nullptr;
		mGetArchiveInfo GetArchiveInfo = nullptr;
		mGetArchiveInfoW GetArchiveInfoW = nullptr;
		mGetFileInfo GetFileInfo = nullptr;
		mGetFileInfoW GetFileInfoW = nullptr;
		mGetFile GetFile = nullptr;
		mGetFileW GetFileW = nullptr;
		mConfigurationDlg ConfigurationDlg = nullptr;

		// Constructor
		CSpiPlugin(void) { SpiHandle = nullptr; };

		// Destructor
		~CSpiPlugin(void) { if (SpiHandle != nullptr)FreeLibrary(SpiHandle); };					// ライブラリ解放

		//  DLL Procedure address set
		bool LoadSpiLL(WCHAR *PluginPath)
		{
			memset(this, 0, sizeof(CSpiPlugin));
			bool Result = false;

			SpiHandle = LoadLibrary(PluginPath);					// Win32 API

			if (SpiHandle != nullptr)
			{
				// === Win32 API でアドレス取得 ===
				GetPluginInfo = (int(PASCAL *)(int, LPSTR, int))GetProcAddress(SpiHandle, "GetPluginInfo");
				IsSupported = (int(PASCAL *)(LPCSTR, void *))GetProcAddress(SpiHandle, "IsSupported");
				GetPictureInfo = (int(PASCAL *)(LPCSTR, LONG_PTR, unsigned int, struct SPictureInfo *))GetProcAddress(SpiHandle, "GetPictureInfo");
				GetPicture = (int(PASCAL *)(LPCSTR, LONG_PTR, unsigned int, HANDLE *, HANDLE *, FARPROC, LONG_PTR))GetProcAddress(SpiHandle, "GetPicture");
				GetPreview = (int(PASCAL *)(LPCSTR, LONG_PTR, unsigned int, HANDLE *, HANDLE *, FARPROC, LONG_PTR))GetProcAddress(SpiHandle, "GetPreview");
				GetArchiveInfo = (int(PASCAL *)(LPCSTR, LONG_PTR, unsigned int, HLOCAL *))GetProcAddress(SpiHandle, "GetArchiveInfo");
				GetFileInfo = (int(PASCAL *)(LPCSTR, LONG_PTR, unsigned int, HLOCAL *))GetProcAddress(SpiHandle, "GetFileInfo");
				GetFile = (int(PASCAL *)(LPCSTR, LONG_PTR, LPSTR, unsigned int, FARPROC, LONG_PTR))GetProcAddress(SpiHandle, "GetFile");
				ConfigurationDlg = (int(PASCAL *)(HWND, int))GetProcAddress(SpiHandle, "ConfigurationDlg");
#ifdef _WIN64
				GetPluginInfoW = (int(PASCAL *)(int, LPCWSTR, int))GetProcAddress(SpiHandle, "GetPluginInfoW");
				IsSupportedW = (int(PASCAL *)(LPCWSTR, void *))GetProcAddress(SpiHandle, "IsSupportedW");
				GetPictureInfoW = (int(PASCAL *)(LPCWSTR, LONG_PTR, unsigned int, struct SPictureInfo *))GetProcAddress(SpiHandle, "GetPictureInfoW");
				GetPictureW = (int(PASCAL *)(LPCWSTR, LONG_PTR, unsigned int, HANDLE *, HANDLE *, FARPROC, LONG_PTR))GetProcAddress(SpiHandle, "GetPictureW");
				GetPreviewW = (int(PASCAL *)(LPCWSTR, LONG_PTR, unsigned int, HANDLE *, HANDLE *, FARPROC, LONG_PTR))GetProcAddress(SpiHandle, "GetPreviewW");
				GetArchiveInfoW = (int(PASCAL *)(LPCWSTR, LONG_PTR, unsigned int, HLOCAL *))GetProcAddress(SpiHandle, "GetArchiveInfoW");
				GetFileInfoW = (int(PASCAL *)(LPCWSTR, LONG_PTR, unsigned int, HLOCAL *))GetProcAddress(SpiHandle, "GetFileInfoW");
				GetFileW = (int(PASCAL *)(LPCWSTR, LONG_PTR, LPWSTR, unsigned int, FARPROC, LONG_PTR))GetProcAddress(SpiHandle, "GetFileW");
#endif
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
#ifdef _WIN64
		std::wstring PostFileExt = TEXT(""); // Susie に渡す用の FileExt
#else
		std::string PostFileExt = ""; // Susie に渡す用の FileExt
#endif
		//
		std::vector<CSpiPlugin *> Spi;               // プラグインリスト
		CSpiPlugin *NowSpi = nullptr;         // 最後に確定したプラグイン
		CSpiPlugin *PostSpi = nullptr;    // 次に読むこむ事が確定した Spi プラグイン
		CSpiPlugin *ArchiveSpi = nullptr; // 現在解凍に使っている Spi プラグイン

		HBITMAP hBitmap = nullptr; // 読み込まれた hBitmap オリジナル

		BITMAPINFO *pBmpInfo = nullptr; // バックバッファのビットマップインフォ
		HANDLE pBmpData = nullptr; // バックバッファの ビットマップデータ先頭アドレス
		HANDLE hBmpData = nullptr; // バックバッファのハンドル
		BITMAPINFO *pBmpRInfo = nullptr; // 回転バックバッファのビットマップインフォ
		HANDLE pBmpRData = nullptr; // 回転バックバッファの ビットマップデータ先頭アドレス
		HANDLE hBmpRData = nullptr; // 回転バックバッファのハンドル
		bool MustBackBufferTrans = false;


		// ハンドルデータ
		HANDLE HFileImage = nullptr, pre_HFileImage = nullptr; // Susie プラグインが持ってくるハンドル
		BYTE *pFileImage = nullptr, *pre_pFileImage = nullptr; // データへのポインタ
#ifdef _WIN64
		std::vector<SArchivedFileInfoW> nowArchivedFileInfo;
		std::vector<SArchivedFileInfoW> oldArchivedFileInfo;
#else
		std::vector<SArchivedFileInfo> nowArchivedFileInfo;
		std::vector<SArchivedFileInfo> oldArchivedFileInfo;
#endif
		//
		EWorkFileType LoadingFileType = EWorkFileType_NONE;

		// ポインタデータ
		ULONG_PTR GdiplusToken;
		Bitmap *BitmapGDIP = nullptr;
		Bitmap **GIFBitmapGDIP = nullptr;
		HBITMAP *GIFhBitmapGDIP = nullptr;
		Image *ImageGDIP = nullptr;

		HGLOBAL hMemoryGDIP = nullptr;
		IStream *StreamGDIP = nullptr;
		BYTE *pMemoryGDIP = nullptr;

		// アニメーション関連
		EAnimationType Animate = EAnimationType_NONE; // 値変更してはいけない
		EAnimationType PostAnimate = EAnimationType_NONE; // 値変更してはいけない
		int LoopCount = 0;
		int LoopIndex = 0;
		int FrameCount = 0;
		int FrameIndex = 0;
		int *Delay = nullptr; // ディレイデータ
		int DelayTime = 0;
		int DropFrame = 0;
		int DropCount = 0;
		bool AnimePlaying = false; // アニメループ中かどうかを表し MainForm から値を変更してもいい
		bool AnimeProcessing = false; // アニメ処理中かどうか
		bool AllFrame = false; // 全フレームを取得するかどうか
		int MaxAllFrame = 100 * 1000 * 1000; // アニメ全体のデータ量がこれを上回るなら毎フレーム取得する

		// WebP 関係
		int WebPWidth;
		int WebPHeight;

		WebPData WebPBufData = {};
		uint8_t *pWebPBuf = nullptr;
		uint8_t **pWebPAnimeBuf = nullptr;
		HBITMAP *hWebPAnimeBitmap = nullptr;
		WebPAnimDecoder *WebPDecoder = nullptr;
		WebPAnimDecoderOptions WebPDecOptions;
		int PreDelay;

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
		HWND hWindow = nullptr;
		Gdiplus::Color BGColor = Gdiplus::Color(0, 0, 0);
		CRITICAL_SECTION *CriticalSection;

	public:
		int ImageNum = 0;
		int Rotate = -1; // 現在の回転値
		bool FixRotate = false; // 回転を固定するかどうか

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
		void Release(void);

		// すべてのプラグインをクリアする
		void ClearAllPlugins(void);

		// SPi のパスを設定する
		void SetPluginPathes(std::vector<std::wstring> &Paths);

		bool Refresh(void);
		bool SetImageFile(CImageInfo &SrcImageInfo);
		void InitSize(int iWidth, int iHeight);
		bool CheckFileType(CImageInfo &SrcImageInfo, BYTE* FileData, long FileSize);
		bool CheckFileTypeSPI(CImageInfo &SrcImageInfo, BYTE* FileData);
		bool CheckWebP(CImageInfo &SrcImageInfo, BYTE* FileData, long FileSize);

		// GIF アニメファイルかどうか調べる
		bool CheckGIFAnimeEnabled(CImageInfo &SrcImageInfo, BYTE* FileData, long FileSize);
		// GDI+ でファイルタイプを調べる
		bool CheckFileTypeGDIP(CImageInfo &SrcImageInfo, BYTE* FileData);
		// ファイル実体からファイルを読み込む。 Susie Plugin でファイル読み込みしか対応していないものがあるので追加したがバグがあるかもしれない。未使用。
		bool LoadFromFileEntity(CImageInfo & SrcImageInfo, FARPROC ProgressCallback);
		// メモリからファイルを読み込む
		bool LoadFromFileInMemory(CImageInfo &SrcImageInfo, BYTE* FileData, LONG DataSize, FARPROC ProgressCallback);

		// アーカイブファイルの名前を変更する
		bool ChangeArchiveFileName(std::wstring NewFileName);

		// イメージオブジェクトからビットマップオブジェクトを作る
		Gdiplus::Bitmap* CreateBMPFromImage(Gdiplus::Image *Src, HBITMAP *hdest);

		// アーカイブファイル中のインデックスの番号のファイルを読み込む
		int LoadSubIndex(int &SubIndex, int Ofs);
		// バックバッファがその画像の幅高さで使用可能かどうか調べる
		void CheckBackBuffer(int sWidth, int sHeight);
		// フォームのサイズを指定する
		bool SetFormSize(int iWidth, int iHeight);
		bool CreateRotateBuffer(void);
		bool DeleteRotateBuffer(void);
		// バックバッファにデータを転送する
		bool TransBackBuffer(void);

		// 絶対値で画像を回転する
		bool AbsoluteRotate(int Value);
		// 相対値で画像を回転する
		bool OffsetRotate(int Value);

		// アニメーションを一時中断する
		bool PauseAnimate(void);
		// アニメーションを再度再生する
		bool RestartAnimate(void);
		// アニメのフレームを次に進める
		bool AnimateUpDateFrame(bool FrameSkip);
		bool GIFAnimateUpDateFrame(bool FrameSkip);
		bool WebPAnimateUpDateFrame(bool FrameSkip);

	
		// WebP のデータを得る
		void GetWebPData(WebPData *webp_data);
		// WebP のアニメーションデータを得る
		void GetWebPAnimationData(WebPData *webp_data);
		// WebP のExifデータを得る
		void GetWebPExifData(WebPData *webp_data);

		// 回転情報をチェックする
		int CheckOrientation(void);
		// 回転情報をチェックsル
		short CheckOrientation(BYTE* pTemp, unsigned int Size);
		// 回転情報を Exif からひろう
		short CheckOrientationExif(BYTE* pTemp, unsigned int Size);
		// GIF アニメデータをチェックする
		EAnimationType GetGIFAnimeData(void);
		
		// アーカイブファイ中のインデックスの番号のファイルを読み込む
		bool SetShowSubIndex(CImageInfo &SrcImageInfo, int SubIndex, FARPROC ProgressCallback);
		// アーカイブファイルの一覧を得る
		bool GetArchiveFileList(std::vector<CImageInfo>* SubFileList); // ファイルリストを取得

		// アーカイブファイル中の画像を読み込む
		bool SetSubImageFile(CImageInfo &Src);
		// アーカイブファイル中の画像を読み込む
		bool SetSubImageFile(std::vector<CImageInfo>* FileList, int &i, int Dir);

		// データをクリアする
		void Clear(EPluginMode DelMode);

		// バックバッファーを消去する
		void DeleteBackBuffer(void);
		// プラグインダイアログを表示する
		void OpenSpiSettingDialog(HWND hwnd);

		// ビットマップにデータをコピーする
		Gdiplus::Bitmap* DuplicateImage(int newWidth, int newHeight);

		// ビットマップにデータをコピーする
		Gdiplus::Bitmap* DuplicateBGImage(int newWidth, int newHeight);

		// クリップボードに画像をコピーする
		bool CopyImageToClipboard(void);
		// JPEG ファイルで保存する
		bool SaveJpeg(std::wstring svFileName, int svWidth, int svHeight, int svCompLevel, bool TransFormed);
		// PNG ファイルで保存する
		bool SavePNG(std::wstring svFileName, int svWidth, int svHeight, bool TransFormed);


	};


}


