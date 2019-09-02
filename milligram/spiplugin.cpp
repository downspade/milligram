#include "stdafx.h"
#include "spiplugin.h"
#include "acfc.h"

#define IGNORE_FILEMINSIZE 512

namespace milligram
{
	CSpiLoader::CSpiLoader(void)
	{
		InternalLoader = L"*.bmp;*.jpg;*.jpeg;*.png;*.gif;*.webp;";
	}

	CSpiLoader::~CSpiLoader(void)
	{
	}

	void CSpiLoader::Init(HWND hWndNewOwner, FARPROC aProgressCallBack)
	{
		hWindow = hWndNewOwner;
		ProgressCallback = aProgressCallBack;

		Spi = std::vector<CSpiPlugin *>();

		// マルチメディアタイマーの設定
		TIMECAPS TimeCaps;
		timeGetDevCaps(&TimeCaps, sizeof(TIMECAPS));
		ShortestPeriod = TimeCaps.wPeriodMin;

		timeBeginPeriod((UINT)ShortestPeriod);
		Initialized = true;
	}

	void CSpiLoader::Release(void)
	{
		Clear(EPluginMode_ALL);
		Mode = EPluginMode_NONE;

		ClearAllPlugins();

		timeEndPeriod((UINT)ShortestPeriod);
		CheckBackBuffer(0, 0);
	}

	void CSpiLoader::ClearAllPlugins(void)
	{
		if (Spi.size() > 0)
		{
			std::vector<CSpiPlugin *>::iterator itr;
			for (itr = Spi.begin(); itr != Spi.end(); itr++)
			{
				delete *itr;
			}
			Spi.clear();
		}
	}

	// プラグイン一覧を取得し、実際に、プラグインを読み込む
	void CSpiLoader::SetPluginPathes(std::vector<std::wstring> &Pathes)
	{
		CSpiPlugin *Plg;

		if (Pathes.size() == 0)return;

		std::vector<std::wstring> SpiFullLists; // フルパス
		std::vector<std::wstring> SpiNameLists; // ファイル名のみ
		std::vector<std::wstring> SpiGetLists; // 最終的に取得すべきフルパス

		// 古いポインタを削除する
		ClearAllPlugins();

		std::vector<std::wstring> temp;
		// 全部のパスに入っている spi ファイルを取得
		for (int i = 0; i != (int)(Pathes.size()); i++)
		{
#ifdef _WIN64
			acfc::GetFiles(temp, Pathes[i], L"*.sph");
#else
			acfc::GetFiles(temp, Pathes[i], L"*.spi");
#endif
			std::copy(temp.begin(), temp.end(), std::back_inserter(SpiFullLists));
			temp.clear();
		}

		// ファイル名だけで spi ファイルを取得してコピーしておく
		for(int i = 0;i < (int)SpiFullLists.size();i++)
			SpiNameLists.push_back(acfc::GetFileName(SpiFullLists[i]));

		// 同じファイル名の spi ファイルは１回しか読まないようにしておく
		for (int i = 0; i < (int)SpiFullLists.size(); i++)
		{
			if (std::find(SpiGetLists.begin(), SpiGetLists.begin() + i, SpiNameLists[i]) == SpiGetLists.begin() + i)
				SpiGetLists.push_back(SpiFullLists[i]);
		}

		// 一つずつ、LoadLibrary してみる
		for (int i = 0; i < (int)(SpiGetLists.size()); i++)
		{
			Plg = new CSpiPlugin();
			if
				(
					Plg->LoadSpiLL((WCHAR*)SpiGetLists[i].c_str()) == true
					&& (strncmp(Plg->APIVersion, "00IN", 4) == 0 || strncmp(Plg->APIVersion, "00AM", 4) == 0)
					)
				Spi.push_back(Plg);
			else
				delete Plg;
		}

		Showing = false;

	}
	

	// リフレッシュ
	bool CSpiLoader::Refresh(void)
	{
		CImageInfo OldFileInfo = ImageInfo;
		CImageInfo OldSubFileInfo = SubImageInfo;

		Clear(EPluginMode_ALL);
		bool Result;

		if (OldSubFileInfo.FileName == L"")
		{
			Result = SetImageFile(OldFileInfo);
		}
		else
		{
			Result = SetImageFile(OldFileInfo) & SetSubImageFile(OldSubFileInfo);
		}
		return (Result);
	}

	// 読み込むファイルを設定する
	bool CSpiLoader::SetImageFile(CImageInfo &SrcImageInfo)
	{
		DWORD SizeL, SizeH;

		HANDLE hFile;
		HANDLE hMap;

		// ファイルハンドルの取得
		hFile = CreateFileW(SrcImageInfo.FileName.c_str(), GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_WRITE, 0, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, 0);

		bool Result = false;

		if (hFile != NULL && hFile != INVALID_HANDLE_VALUE)
		{

			BYTE *FileData;

			// ファイルはすでに存在していて、この関数は成功します
			SizeL = GetFileSize(hFile, &SizeH);

			if (SizeH == 0 && SizeL < 1024 * 1024 * 1024 && SizeL >= IGNORE_FILEMINSIZE)
			{
				// メモリマップドファイルを作成する
				hMap = CreateFileMapping(hFile, 0, PAGE_READWRITE, 0, 0, 0);
				if (hMap != NULL)
				{
					FileData = (BYTE *)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
					PostSpi = NULL;

					if (CheckFileType(SrcImageInfo, FileData, SizeL) == 1)
					{
						EnterCriticalSection(CriticalSection);
						// 実際にマップドされたメモリから読み込む
						Result = LoadFromFileInMemory(SrcImageInfo, FileData, SizeL, ProgressCallback);
						LeaveCriticalSection(CriticalSection);
					}
					// メモリマップドファイルの終了
					UnmapViewOfFile(FileData);
				}
				CloseHandle(hMap);
			}
			// NOTE:未使用
			if(Result == false)
				Result = LoadFromFileEntity(SrcImageInfo, ProgressCallback);
		}
		CloseHandle(hFile);

		return(Result);
	}

	void CSpiLoader::InitSize(int iWidth, int iHeight)
	{
		OrgWidth = iWidth;
		OrgHeight = iHeight; // もと画像の大きさ
		ORotWidth = iWidth;
		ORotHeight = iHeight; // 転送もと画像のかたち(回転後)

		SrcRWidth = iWidth;
		SrcRHeight = iHeight;   // 転送もとの大きさ(回転後)
	}

	bool CSpiLoader::CheckFileType(CImageInfo &SrcImageInfo, BYTE *FileData, long FileSize)
	{
		bool Result = false;
		LoadingFileType = EWorkFileType_NONE;
		PostAnimate = EAnimationType_NONE;

		if (FileData == nullptr) return (false);

		Result = CheckWebP(SrcImageInfo, FileData, FileSize);
		
		if(Result == false)Result = CheckGIFAnimeEnabled(SrcImageInfo, FileData, FileSize);

		if (Result == false)
		{
			if (acfc::FitsMasks(SrcImageInfo.FileName, InternalLoader))
			{
				if (Result == false) Result = CheckFileTypeGDIP(SrcImageInfo, FileData);
				if (Result == false) Result = CheckFileTypeSPI(SrcImageInfo, FileData);
			}
			else
			{
				if (Result == false) Result = CheckFileTypeSPI(SrcImageInfo, FileData);
				if (Result == false) Result = CheckFileTypeGDIP(SrcImageInfo, FileData);
			}
		}
		return (Result);
	}

	bool CSpiLoader::CheckFileTypeSPI(CImageInfo &SrcImageInfo, BYTE* FileData)
	{
		// -----------------------------------------------------------------------------
		// まず SPI プラグインで読み込めるかどうか調べる
		try 
		{
#ifdef _WIN64 
			PostFileExt = acfc::GetFileExt(SrcImageInfo.FileName);
			int i;
			// SPI プラグインが必要かどうか調べる。
			for (i = 0; i < (int)(Spi.size()); i++)
			{
				if (Spi[i]->IsSupportedW((LPWSTR)PostFileExt.c_str(), FileData) != 0)
				{
					PostSpi = Spi[i];
					PostImageInfo = SrcImageInfo;

					if (strncmp(PostSpi->APIVersion, "00IN", 4) == 0)
						LoadingFileType = EWorkFileType_SPI_PICTURE;
					else if (strncmp(PostSpi->APIVersion, "00AM", 4) == 0)
						LoadingFileType = EWorkFileType_SPI_ARCHIVE;

					return (true);
				}
			}
#else
			PostFileExt = acfc::UnicodeToMultiByte(acfc::GetFileExt(SrcImageInfo.FileName));
			int i;
			// SPI プラグインが必要かどうか調べる。
			for (i = 0; i < (int)(Spi.size()); i++)
			{
				if (Spi[i]->IsSupported((LPSTR)PostFileExt.c_str(), FileData) != 0)
				{
					PostSpi = Spi[i];
					PostImageInfo = SrcImageInfo;

					if (strncmp(PostSpi->APIVersion, "00IN", 4) == 0)
						LoadingFileType = EWorkFileType_SPI_PICTURE;
					else if (strncmp(PostSpi->APIVersion, "00AM", 4) == 0)
						LoadingFileType = EWorkFileType_SPI_ARCHIVE;

					return (true);
				}
			}
#endif
		}
		catch(...)
		{
			return (false);
		}

		// -----------------------------------------------------------------------------
		return (false);
	}

	bool CSpiLoader::CheckWebP(CImageInfo &SrcImageInfo, BYTE* FileData, long FileSize)
	{
		bool Result = WebPGetInfo((uint8_t*)FileData, FileSize, &WebPWidth, &WebPHeight);
		PostNOSPIType = ENoSPIPictureType_WEBP;
		LoadingFileType = EWorkFileType_WEBP;
		return(Result);
	}


	bool CSpiLoader::CheckGIFAnimeEnabled(CImageInfo &SrcImageInfo, BYTE* FileData, long FileSize)
	{
		if (FileSize < 1024)return(false);
		int i, t, AnimFlag = 0;

		// ヘッダのチェック --------------------
		// gif かどうか調べる
		if (strncmp((char *)FileData, "GIF8", 4) != 0)return(false);
		if (*((char *)FileData + 4) != '9' && *((char *)FileData + 4) != '7')return(false);
		if (*((char *)FileData + 5) != 'a')return(false);

		if (FileData[10] & 0x80)// Global Color Table が存在する
		{
			t = (1 << ((FileData[10] & 0x07) + 1)) * 3;
			i = t;
		}
		else
		{
			i = 0;
		}
		i += 13;
		// --------------------

		while (1)
		{
			if (FileData[i] == 0x2C)
			{
				// Image Block
				i += 9; if (i >= FileSize)break;

				if (FileData[i] & 0x80) // Local Color Table が存在する
				{
					t = (1 << ((FileData[i] & 0x07) + 1)) * 3;
					i += t;
				}

				i += 2; if (i >= FileSize)break;

				do
				{
					i += FileData[i] + 1;
					if (i >= FileSize)break;
				} while (FileData[i] != 0);

				if (AnimFlag == 1)AnimFlag = 2;
			}
			else if (FileData[i] == 0x21)
			{
				i++; if (i >= FileSize)break;

				if (FileData[i] == 0xf9)
				{
					// Graphic Control Extension
					i++; if (i >= FileSize)break;

					if (FileData[i] != 0x04)break;

					i += 5; if (i >= FileSize)break;

					if (FileData[i] != 0x00)break;

					if (AnimFlag == 0)AnimFlag = 1;
				}
				else if (FileData[i] == 0xfe || FileData[i] == 0x01 || FileData[i] == 0xff)
				{
					// Comment Extension, Plain Text Extension & Application Extension
					i++; if (i >= FileSize)break;

					do
					{
						i += FileData[i] + 1;
						if (i >= FileSize)break;
					} while (FileData[i] != 0);
				}
			}
			else return(false);

			i++; if (i >= FileSize)break;

			if (FileData[i] == 0x3b)break;
		}

		if (AnimFlag != 2)return(false);

		PostSpi = nullptr;
		PostImageInfo = SrcImageInfo;
		PostNOSPIType = ENoSPIPictureType_GIF;
		LoadingFileType = EWorkFileType_GDIP_PICTURE;
		PostAnimate = EAnimationType_GIF;
		return (true);
	}

	

	bool CSpiLoader::CheckFileTypeGDIP(CImageInfo &SrcImageInfo, BYTE* FileData)
	{
		// 通常のビットマップかどうか調べる
		if (strncmp((char *)FileData, "BM", 2) == 0)
		{
			PostSpi = nullptr;
			PostImageInfo = SrcImageInfo;
			PostNOSPIType = ENoSPIPictureType_BMP;
			LoadingFileType = EWorkFileType_GDIP_PICTURE;
			return (true);
		}

		// jpeg かどうか調べる
		if (FileData[0] == (BYTE)0xFF && FileData[1] == (BYTE)0xD8)
		{
			PostSpi = nullptr;
			PostImageInfo = SrcImageInfo;
			PostNOSPIType = ENoSPIPictureType_JPG;
			LoadingFileType = EWorkFileType_GDIP_PICTURE;
			return (true);
		}

		// PNG かどうか調べる
		if (strncmp((char *)FileData + 1, "PNG", 3) == 0)
		{
			PostSpi = nullptr;
			PostImageInfo = SrcImageInfo;
			PostNOSPIType = ENoSPIPictureType_PNG;
			LoadingFileType = EWorkFileType_GDIP_PICTURE;
			return (true);
		}

		// gif かどうか調べる
		if (strncmp((char *)FileData, "GIF", 3) == 0)
		{
			PostSpi = nullptr;
			PostImageInfo = SrcImageInfo;
			PostNOSPIType = ENoSPIPictureType_GIF;
			LoadingFileType = EWorkFileType_GDIP_PICTURE;
			return (true);
		}

		// tif かどうか調べる
		if (memcmp(FileData, "\x4D\x4D\x00\x2A", 4) == 0 || memcmp(FileData, "\x49\x49\x2a\x00", 4) == 0)
		{
			PostSpi = nullptr;
			PostImageInfo = SrcImageInfo;
			PostNOSPIType = ENoSPIPictureType_TIF;
			LoadingFileType = EWorkFileType_GDIP_PICTURE;
			return (true);
		}
		return(false);
	}

	// NOTE:未使用
	bool CSpiLoader::LoadFromFileEntity(CImageInfo &SrcImageInfo, FARPROC ProgressCallback)
	{
		SPictureInfo PicInfo = {};

		Showing = false;

		if (LoadingFileType == EWorkFileType_SPI_ARCHIVE)
		{
			HANDLE HInfo;
#ifdef _WIN64
			if (PostSpi->GetArchiveInfoW((LPWSTR)SrcImageInfo.FileName.c_str(), 0, 0, (void**)&HInfo) != 0) return (false);
			SArchivedFileInfoW *pInfos = (SArchivedFileInfoW *)LocalLock(HInfo);
#else
			std::string file_n = acfc::UnicodeToMultiByte(SrcImageInfo.FileName);
			if (PostSpi->GetArchiveInfo((LPSTR)file_n.c_str(), 0, 0, (void**)&HInfo) != 0) return (false);
			SArchivedFileInfo *pInfos = (SArchivedFileInfo *)LocalLock(HInfo);
#endif
			Animate = EAnimationType_NONE;

			nowArchivedFileInfo.clear();
			ImageNum = 0;

			while (pInfos[ImageNum].Method[0] != 0)
			{
				nowArchivedFileInfo.push_back(pInfos[ImageNum]);
				ImageNum++;
			}

			LocalUnlock(HInfo);
			LocalFree(HInfo);

			Clear((EPluginMode)(EPluginMode_PICTURE | EPluginMode_ARCHIVE));

			Mode = EPluginMode_ARCHIVE;

			ArchiveSpi = PostSpi;
			ImageInfo = PostImageInfo;

			return (true);
		}
		else if (LoadingFileType == EWorkFileType_SPI_PICTURE)
		{
			Animate = EAnimationType_NONE;

			try
			{
				HANDLE htmpBmpInfo, htmpBmpData;

#ifdef _WIN64
				if (PostSpi->GetPicture((LPCSTR)SrcImageInfo.FileName.c_str(), 0, 1, &htmpBmpInfo, &htmpBmpData, ProgressCallback, 0) != 0) return (false);
#else
				std::string file_n = acfc::UnicodeToMultiByte(SrcImageInfo.FileName);
				if (PostSpi->GetPicture((LPSTR)file_n.c_str(), 0, 0, &htmpBmpInfo, &htmpBmpData, ProgressCallback, 0) != 0) return (false);
#endif

				Clear(EPluginMode_PICTURE);

				HANDLE pBmp = GlobalLock(htmpBmpData); // Win32 API でロック
				BITMAPINFO *pInfo = (BITMAPINFO*)GlobalLock(htmpBmpInfo);

				OrgWidth = pInfo->bmiHeader.biWidth;
				OrgHeight = pInfo->bmiHeader.biHeight;

				BitmapGDIP = new Gdiplus::Bitmap(pInfo, pBmp);
				BitmapGDIP->GetHBITMAP(BGColor, &hBitmap);

				GlobalUnlock(pBmp);
				GlobalUnlock(pInfo);
				GlobalFree(pBmp);
				GlobalFree(pInfo);
			}
			catch (...)
			{
				return(false);
			}
			// NOTE:メモリに一旦読み込む必要があるので見送り
			//if (FixRotate == false)
			//{
			//	Rotate = SrcImageInfo.Rotate;
			//	if (Rotate < 0) Rotate = CheckOrientation(FileData, DataSize);
			//}
			//else
			//{
			//	if (Rotate < 0) Rotate = CheckOrientation(FileData, DataSize);
			//}
			SrcImageInfo.Rotate = Rotate;

			Mode = (EPluginMode)(Mode & (EPluginMode_ALL ^ EPluginMode_PICTURE));
			Mode = (EPluginMode)(Mode | EPluginMode_SPI);

			PluginName = acfc::MultiByteToUnicode(PostSpi->PluginName);
		}
		else return (false);

		BufWidth = ORotWidth = SrcRWidth = OrgWidth;
		BufHeight = ORotHeight = SrcRHeight = OrgHeight;

		if (Rotate % 2 == 0)
		{
			ORotWidth = OrgWidth;
			ORotHeight = OrgHeight;
		}
		else
		{
			ORotWidth = OrgHeight;
			ORotHeight = OrgWidth;
		}

		NowSpi = PostSpi;
		NOSPIType = PostNOSPIType;

		Showing = true;
		MustBackBufferTrans = true;

		CreateRotateBuffer();

		AnimePlaying = (Animate != EAnimationType_NONE);
		return (true);
	}


	// 画像ファイルを実際に読み込む
	bool CSpiLoader::LoadFromFileInMemory(CImageInfo &SrcImageInfo, BYTE* FileData, LONG DataSize, FARPROC ProgressCallback)
	{
		if (FileData == nullptr) return (false);
		SPictureInfo PicInfo = {};

		Showing = false;

		// アーカイブモードかどうかチェックする
		// アーカイブモードならファイルの一覧を取得する
//		if (PostSpi != nullptr && strncmp(PostSpi->APIVersion, "00AM", 4) == 0 && SubImageInfo.FileName == L"")
		if(LoadingFileType == EWorkFileType_SPI_ARCHIVE)
		{
			HANDLE HInfo;
#ifdef _WIN64
			if (PostSpi->GetArchiveInfoW((LPWSTR)FileData, DataSize, 1, (void**)&HInfo) != 0) return (false);
			SArchivedFileInfoW *pInfos = (SArchivedFileInfoW *)LocalLock(HInfo);
#else
			if (PostSpi->GetArchiveInfo((LPSTR)FileData, DataSize, 1, (void**)&HInfo) != 0) return (false);
			SArchivedFileInfo *pInfos = (SArchivedFileInfo *)LocalLock(HInfo);
#endif
			Animate = EAnimationType_NONE;

			nowArchivedFileInfo.clear();
			ImageNum = 0;

			while (pInfos[ImageNum].Method[0] != 0)
			{
				nowArchivedFileInfo.push_back(pInfos[ImageNum]);
				ImageNum++;
			}

			LocalUnlock(HInfo);
			LocalFree(HInfo);

			Clear((EPluginMode)(EPluginMode_PICTURE | EPluginMode_ARCHIVE));

			Mode = EPluginMode_ARCHIVE;

			ArchiveSpi = PostSpi;
			ImageInfo = PostImageInfo;

			return (true);
		}

		// WebP モードかどうかチェックする
		if (LoadingFileType == EWorkFileType_WEBP)
		{
			Clear(EPluginMode_PICTURE);

			Animate = PostAnimate;

			WebPData webp_data;
			webp_data.bytes = FileData;
			webp_data.size = DataSize;

			GetWebPData(&webp_data);

			OrgWidth = WebPWidth;
			OrgHeight = WebPHeight;

			Mode = (EPluginMode)(Mode & (EPluginMode_ALL ^ EPluginMode_PICTURE));
			Mode = (EPluginMode)(Mode | EPluginMode_WEBP);

			PluginName = L"Internal Loader";

			Rotate = 0;
		}
		// SPI モードでない場合、素でファイルを読み込んでみる
		//if (PostSpi == nullptr)
		else if (LoadingFileType == EWorkFileType_GDIP_PICTURE)
		{
			Clear(EPluginMode_PICTURE);

			Animate = PostAnimate;

			try
			{
				hMemoryGDIP = GlobalAlloc(GMEM_MOVEABLE, DataSize);
				pMemoryGDIP = (BYTE *)GlobalLock(hMemoryGDIP);
				CopyMemory(pMemoryGDIP, FileData, DataSize);
				StreamGDIP = nullptr;
				CreateStreamOnHGlobal(hMemoryGDIP, NULL, &StreamGDIP);

				ImageGDIP = new Gdiplus::Image(StreamGDIP);

				StreamGDIP->Release();
				StreamGDIP = nullptr;
				GlobalUnlock(hMemoryGDIP);
				pMemoryGDIP = nullptr;
			}
			catch(...)
			{
				Animate = EAnimationType_NONE;
				return (false);
			}

			if (ImageGDIP == nullptr)
			{
				Animate = EAnimationType_NONE;
				return(false);
			}

			OrgWidth = ImageGDIP->GetWidth();
			OrgHeight = ImageGDIP->GetHeight();

			BitmapGDIP = CreateBMPFromImage(ImageGDIP, &hBitmap);

			Mode = (EPluginMode)(Mode & (EPluginMode_ALL ^ EPluginMode_PICTURE));
			Mode = (EPluginMode)(Mode | EPluginMode_GDIP);

			if (FixRotate == false)
			{
				Rotate = SrcImageInfo.Rotate;
				if (Rotate < 0) Rotate = CheckOrientation();
			}
			else
			{
				if (Rotate < 0) Rotate = CheckOrientation();
			}
			SrcImageInfo.Rotate = Rotate;

			if (Animate != EAnimationType_NONE)
				Animate = GetGIFAnimeData();

			PluginName = L"Internal Loader";
		}
		else if (LoadingFileType == EWorkFileType_SPI_PICTURE)
		{
			Animate = EAnimationType_NONE;

			try
			{
				HANDLE htmpBmpInfo, htmpBmpData;

				if (PostSpi->GetPicture((LPSTR)FileData, DataSize, 1, &htmpBmpInfo, &htmpBmpData, ProgressCallback, 0) != 0) return (false);

				Clear(EPluginMode_PICTURE);

				HANDLE pBmp = GlobalLock(htmpBmpData); // Win32 API でロック
				BITMAPINFO *pInfo = (BITMAPINFO*)GlobalLock(htmpBmpInfo);

				OrgWidth = pInfo->bmiHeader.biWidth;
				OrgHeight = pInfo->bmiHeader.biHeight;

				BitmapGDIP = new Gdiplus::Bitmap(pInfo, pBmp);
				BitmapGDIP->GetHBITMAP(BGColor, &hBitmap);

				GlobalUnlock(pBmp);
				GlobalUnlock(pInfo);
				GlobalFree(pBmp);
				GlobalFree(pInfo);
			}
			catch (...)
			{
				return(false);
			}

			if (FixRotate == false)
			{
				Rotate = SrcImageInfo.Rotate;
				if (Rotate < 0) Rotate = CheckOrientation(FileData, DataSize);
			}
			else
			{
				if (Rotate < 0) Rotate = CheckOrientation(FileData, DataSize);
			}
			SrcImageInfo.Rotate = Rotate;

			Mode = (EPluginMode)(Mode & (EPluginMode_ALL ^ EPluginMode_PICTURE));
			Mode = (EPluginMode)(Mode | EPluginMode_SPI);

			PluginName = acfc::MultiByteToUnicode(PostSpi->PluginName);
		}
		else return (false);

		BufWidth = ORotWidth = SrcRWidth = OrgWidth;
		BufHeight = ORotHeight = SrcRHeight = OrgHeight;

		if (Rotate % 2 == 0)
		{
			ORotWidth = OrgWidth;
			ORotHeight = OrgHeight;
		}
		else
		{
			ORotWidth = OrgHeight;
			ORotHeight = OrgWidth;
		}

		NowSpi = PostSpi;
		NOSPIType = PostNOSPIType;

		Showing = true;
		MustBackBufferTrans = true;

		CreateRotateBuffer();

		AnimePlaying = (Animate != EAnimationType_NONE);
		return (true);
	}

	Gdiplus::Bitmap* CSpiLoader::CreateBMPFromImage(Gdiplus::Image *Src, HBITMAP *hdest)
	{
		Gdiplus::Bitmap *dest = new Gdiplus::Bitmap(Src->GetWidth(), Src->GetHeight());

		HDC hdc = CreateCompatibleDC(nullptr);
		HBITMAP hdc_hdest = (HBITMAP)SelectObject(hdc, *hdest);

		Graphics g(dest);
		g.DrawImage(Src, 0, 0, Src->GetWidth(), Src->GetHeight());

		SelectObject(hdc, hdc_hdest);

		DeleteObject(hdc);
		DeleteObject(hdc_hdest);

		dest->GetHBITMAP(BGColor, hdest);

		return(dest);
	}

	bool CSpiLoader::ChangeArchiveFileName(std::wstring NewFileName)
	{
		ImageInfo.FileName = NewFileName;
		return (true);
	}

	int CSpiLoader::LoadSubIndex(int &SubIndex, int Ofs)
	{
		if ((Mode & EPluginMode_ARCHIVE) == 0) return (0);
		while (SubIndex < 0) SubIndex += ImageNum;
		while (SubIndex >= ImageNum) SubIndex -= ImageNum;

		int OldSubIndex = SubIndex;
		while (true)
		{
			if (SetShowSubIndex(SubImageInfo, SubIndex, ProgressCallback) == true) return (1);

			SubIndex += Ofs;
			while (SubIndex < 0) SubIndex += ImageNum;
			while (SubIndex >= ImageNum) SubIndex -= ImageNum;
			if (SubIndex == OldSubIndex)
			{
				SubIndex = -1;
				break;
			}
		}
		return (0);
	}

	// バックバッファのサイズを調べて小さければ大きくする
	// sWidth か sHeight に 0 を入れると開放だけ行う
	void CSpiLoader::CheckBackBuffer(int sWidth, int sHeight)
	{
		if (sWidth > 0 && sHeight > 0 && BufMaxWidth >= sWidth && BufMaxHeight >= sHeight) return;

		int cWidth;
		int cHeight;
		if (sWidth > sHeight)
		{
			cWidth = sWidth;
			cHeight = sWidth;
		}
		else
		{
			cWidth = sHeight;
			cHeight = sHeight;
		}

		if (cWidth * cHeight > MaxBBufferPixel)
		{
			cWidth = sWidth;
			cHeight = sHeight;
		}

		if (hBmpData != nullptr)
		{
			delete pBmpInfo;
			DeleteObject(hBmpData);
		}

		if (sWidth == 0 || sHeight == 0) return;


		pBmpInfo = new BITMAPINFO();
		ZeroMemory(&pBmpInfo->bmiHeader, sizeof(BITMAPINFOHEADER));
		ZeroMemory(&pBmpInfo->bmiColors, sizeof(RGBQUAD));
		pBmpInfo->bmiHeader.biSize = (DWORD)sizeof(BITMAPINFOHEADER);

		pBmpInfo->bmiHeader.biPlanes = 1;
		pBmpInfo->bmiHeader.biBitCount = 32;
		pBmpInfo->bmiHeader.biCompression = BI_RGB;
		pBmpInfo->bmiHeader.biWidth = cWidth;
		pBmpInfo->bmiHeader.biHeight = -cHeight;

		hBmpData = CreateDIBSection(nullptr, pBmpInfo, DIB_RGB_COLORS, (void **)&pBmpData, nullptr, 0);

		BufMaxWidth = cWidth;
		BufMaxHeight = cHeight;
	}


	bool CSpiLoader::SetFormSize(int iWidth, int iHeight)
	{
		if (Rotate == 1 || Rotate == 3)
		{
			int c = iWidth;
			iWidth = iHeight; iHeight = c;
		}

		if (OrgWidth > iWidth || OrgHeight > iHeight)
		{
			BufWidth = iWidth;
			BufHeight = iHeight;
			return (TransBackBuffer());
		}
		else if (BufWidth < OrgWidth || BufHeight < OrgHeight)
		{
			BufWidth = OrgWidth;
			BufHeight = OrgHeight;
			return (TransBackBuffer());
		}

		if (MustBackBufferTrans)return(TransBackBuffer());

		return (false);
	}

	bool CSpiLoader::CreateRotateBuffer(void)
	{
		if (pBmpRInfo != nullptr)DeleteRotateBuffer();
		pBmpRInfo = new BITMAPINFO();
		ZeroMemory(&pBmpRInfo->bmiHeader, sizeof(BITMAPINFOHEADER));
		ZeroMemory(&pBmpRInfo->bmiColors, sizeof(RGBQUAD));
		pBmpRInfo->bmiHeader.biSize = (DWORD)sizeof(BITMAPINFOHEADER);

		pBmpRInfo->bmiHeader.biPlanes = 1;
		pBmpRInfo->bmiHeader.biBitCount = 32;
		pBmpRInfo->bmiHeader.biCompression = BI_RGB;
		pBmpRInfo->bmiHeader.biWidth = OrgWidth;
		pBmpRInfo->bmiHeader.biHeight = -OrgHeight;

		hBmpRData = CreateDIBSection(nullptr, pBmpRInfo, DIB_RGB_COLORS, (void **)&pBmpRData, nullptr, 0);
		return(true);
	}

	bool CSpiLoader::DeleteRotateBuffer(void)
	{
		delete pBmpRInfo;
		pBmpRInfo = nullptr;
		DeleteObject(hBmpRData);
		pBmpRData = nullptr;
		hBmpRData = nullptr;
		return(true);
	}


	// バックバッファに画像を転送する
	bool CSpiLoader::TransBackBuffer(void)
	{
		if (Showing == false)return(true);
		POINT points[3];

		if (Rotate == 0 || Rotate == 2 || Rotate == -1)
		{
			SrcRWidth = BufWidth;
			SrcRHeight = BufHeight;
			ORotWidth = OrgWidth;
			ORotHeight = OrgHeight;

			CheckBackBuffer(SrcRWidth, SrcRHeight);
			// 転送先を準備する
			switch (Rotate)
			{
			case 0:
				points[0].x = 0; points[0].y = 0;
				points[1].x = BufWidth; points[1].y = 0;
				points[2].x = 0; points[2].y = BufHeight;
				break;
			case 2:
				points[0].x = BufWidth - 1; points[0].y = BufHeight - 1;
				points[1].x = 0 - 1; points[1].y = BufHeight - 1;
				points[2].x = BufWidth - 1; points[2].y = -1;
				break;
			}

			// 転送元の準備
			HDC sDC = CreateCompatibleDC(nullptr);
			HBITMAP sDC_Bitmap = (HBITMAP)SelectObject(sDC, hBitmap);

			// 転送先の準備
			HDC dDC = CreateCompatibleDC(nullptr);
			HBITMAP dDC_Bitmap = (HBITMAP)SelectObject(dDC, hBmpData);

			// 転送
			int OldStretchMode = SetStretchBltMode(dDC, STRETCH_HALFTONE);
			POINT p;
			SetBrushOrgEx(dDC, 0, 0, &p);

			bool result = PlgBlt(dDC, points,
				sDC,
				0, 0, OrgWidth, OrgHeight,
				nullptr, 0, 0);

			int value;
			if (result == false)
				value = GetLastError();

			SetStretchBltMode(dDC, OldStretchMode);

			SelectObject(sDC, sDC_Bitmap);
			SelectObject(dDC, dDC_Bitmap);

			DeleteObject(sDC_Bitmap);
			DeleteObject(dDC_Bitmap);

			DeleteDC(sDC);
			DeleteDC(dDC);

			MustBackBufferTrans = false;

			if (result == false)return(false);

			return (true);
		}
		else
		{
			SrcRWidth = BufHeight;
			SrcRHeight = BufWidth;
			ORotWidth = OrgHeight;
			ORotHeight = OrgWidth;

			CheckBackBuffer(SrcRWidth, SrcRHeight);

			// 転送元の準備
			HDC sDC = CreateCompatibleDC(nullptr);
			HBITMAP sDC_Bitmap = (HBITMAP)SelectObject(sDC, hBitmap);

			// 転送先の準備
			HDC mDC = CreateCompatibleDC(nullptr);
			HBITMAP mDC_Bitmap = (HBITMAP)SelectObject(mDC, hBmpRData);

			// 転送
			int OldStretchMode = SetStretchBltMode(mDC, STRETCH_HALFTONE);
			POINT p = {};
			SetBrushOrgEx(mDC, 0, 0, &p);

			StretchBlt(mDC, 0, 0, BufWidth, BufHeight,
				       sDC, 0, 0, OrgWidth, OrgHeight,
						SRCCOPY);

			SetStretchBltMode(mDC, OldStretchMode);


			// 転送先を準備する
			switch (Rotate)
			{
			case 1:
				points[0].x = BufHeight; points[0].y = 0;
				points[1].x = BufHeight; points[1].y = BufWidth;
				points[2].x = 0; points[2].y = 0;
				break;
			case 3:
				points[0].x = 0; points[0].y = BufWidth;
				points[1].x = 0; points[1].y = 0;
				points[2].x = BufHeight; points[2].y = BufWidth;
				break;
			}

			// 転送先の準備
			HDC dDC = CreateCompatibleDC(nullptr);
			HBITMAP dDC_Bitmap = (HBITMAP)SelectObject(dDC, hBmpData);

			// 転送
			OldStretchMode = SetStretchBltMode(dDC, STRETCH_HALFTONE);
			p = {};
			SetBrushOrgEx(dDC, 0, 0, &p);

			bool result = PlgBlt(dDC, points,
				mDC,
				0, 0, BufWidth, BufHeight,
				nullptr, 0, 0);

			int value;
			if (result == false)
				value = GetLastError();

			SetStretchBltMode(dDC, OldStretchMode);

			SelectObject(sDC, sDC_Bitmap);
			SelectObject(mDC, mDC_Bitmap);
			SelectObject(dDC, dDC_Bitmap);

			DeleteObject(sDC_Bitmap);
			DeleteObject(mDC_Bitmap);
			DeleteObject(dDC_Bitmap);

			DeleteDC(sDC);
			DeleteDC(mDC);
			DeleteDC(dDC);

			MustBackBufferTrans = false;

			if (result == false)return(false);
		}

		return (true);
	}


	bool CSpiLoader::AbsoluteRotate(int Value)
	{
		if (Showing == false) return (true);
		Rotate = Value;
		return (TransBackBuffer());
	}

	bool CSpiLoader::OffsetRotate(int Value)
	{
		if (Showing == false) return (true);

		Rotate += Value;

		while (Rotate < 0) Rotate += 4;
		while (Rotate > 3) Rotate -= 4;

		return (TransBackBuffer());
	}

	bool CSpiLoader::PauseAnimate(void)
	{
		if (Showing == false || Animate == EAnimationType_NONE) return (false);
		PausedTGT = timeGetTime();
		return (true);
	}
	bool CSpiLoader::RestartAnimate(void)
	{
		if (Showing == false || Animate == EAnimationType_NONE) return (false);
		PreTGT += timeGetTime() - PausedTGT;
		return (true);
	}

	bool CSpiLoader::AnimateUpDateFrame(bool FrameSkip)
	{
		if (Animate == EAnimationType_NONE || AnimePlaying == false)return(false);
		AnimeProcessing = true;

		if (LoopCount <= LoopIndex && LoopCount > 0)
		{
			AnimeProcessing = false;
			AnimePlaying = false;
			return(false);
		}

		bool Result = false;
		switch (Animate)
		{
		case EAnimationType_GIF:
			Result = GIFAnimateUpDateFrame(FrameSkip);
			break;
		case EAnimationType_WEBP:
			Result = WebPAnimateUpDateFrame(FrameSkip);
			break;
		}

		AnimeProcessing = false;
		return(Result);
	}

	bool CSpiLoader::GIFAnimateUpDateFrame(bool FrameSkip)
	{
		bool Result = true;

		FrameIndex++;
		if (FrameIndex >= FrameCount)
		{
			LoopIndex++;

			DropFrame = DropCount;
			DropCount = 0;

			if (LoopCount == LoopIndex)return(false);

			FrameIndex = 0;
		}

		NowTGT = timeGetTime();

		int D;
		if (NowTGT - PreTGT < DelayTime)
			D = 0;
		else
			D = (int)(NowTGT - PreTGT) - (int)DelayTime;

		PreTGT = NowTGT;

		if (FrameIndex == 3)
			FrameIndex = FrameIndex;

		DelayTime = Delay[FrameIndex] - D;

		if (FrameSkip == true)
		{
			while (DelayTime < 0)
			{
				FrameIndex++;
				DropCount++;

				if (FrameIndex >= FrameCount)
				{
					DropFrame = DropCount;
					DropCount = 0;
					LoopIndex++;
					if (LoopCount == LoopIndex)
					{
						FrameIndex--;
						break;
					}
					FrameIndex = 0;
				}
				DelayTime += Delay[FrameIndex];
			}
		}
		else
		{
			DropFrame = 0;
			if (DelayTime < 17)DelayTime = 17;
		}

		if (AllFrame)
		{
			hBitmap = GIFhBitmapGDIP[FrameIndex];
		}
		else
		{
			GUID Guid = FrameDimensionTime;
			ImageGDIP->SelectActiveFrame(&Guid, FrameIndex);
			DeleteObject(hBitmap);
			delete BitmapGDIP;
			BitmapGDIP = CreateBMPFromImage(ImageGDIP, &hBitmap);
		}

		TransBackBuffer();
		return (Result);
	}

	bool CSpiLoader::WebPAnimateUpDateFrame(bool FrameSkip)
	{
		bool lockTaken = false;
		bool Result = true;

		if (AllFrame)
		{
			FrameIndex++;
			if (FrameIndex >= FrameCount)
			{
				LoopIndex++;

				DropFrame = DropCount;
				DropCount = 0;

				if (LoopCount == LoopIndex)return(false);

				FrameIndex = 0;
			}

			NowTGT = timeGetTime();

			int D;
			if (NowTGT - PreTGT < DelayTime)
				D = 0;
			else
				D = (int)(NowTGT - PreTGT) - (int)DelayTime;

			PreTGT = NowTGT;

			DelayTime = Delay[FrameIndex] - D;
			if (FrameSkip == true)
			{
				while (DelayTime < 0)
				{
					FrameIndex++;
					DropCount++;

					if (FrameIndex >= FrameCount)
					{
						DropFrame = DropCount;
						DropCount = 0;
						LoopIndex++;
						if (LoopCount == LoopIndex)
						{
							FrameIndex--;
							break;
						}
						FrameIndex = 0;
					}
					DelayTime += Delay[FrameIndex];
				}
			}
			else
			{
				DropFrame = 0;
				if (DelayTime < 17)DelayTime = 17;
			}

			hBitmap = hWebPAnimeBitmap[FrameIndex];

			TransBackBuffer();
		}
		else
		{
			NowTGT = timeGetTime();

			int D; // 遅れ分
			if (NowTGT - PreTGT < DelayTime)
				D = 0;
			else
				D = (int)(NowTGT - PreTGT) - (int)DelayTime;

			PreTGT = NowTGT;

			if (FrameSkip == true)
			{
				int DelayDef;
				uint8_t *buf;
				do 
				{
					if (WebPAnimDecoderHasMoreFrames(WebPDecoder) == false)
					{
						DropFrame = DropCount;
						FrameIndex = 0;
						DropCount = 1;
						WebPAnimDecoderReset(WebPDecoder);
						WebPAnimDecoderGetNext(WebPDecoder, &buf, &Delay[0]);
						DelayDef = Delay[0];
						LoopIndex++;
					}
					else
					{
						FrameIndex++;
						WebPAnimDecoderGetNext(WebPDecoder, &buf, &Delay[FrameIndex]);
						DelayDef = Delay[FrameIndex] - Delay[FrameIndex - 1];
						DropCount++;
					}
					if (DelayDef == 0)DelayDef = 100;
					D = D - DelayDef;
				}
				while(D > 0 && (LoopIndex < LoopCount || LoopCount == 0));
				DropCount--;
				memcpy(pWebPBuf, buf, 4 * WebPWidth * WebPHeight);
				DelayTime = -D;

			}
			else
			{
				DropFrame = 0;
				int DelayDef;
				uint8_t *buf;
				if (WebPAnimDecoderHasMoreFrames(WebPDecoder) == false)
				{
					WebPAnimDecoderReset(WebPDecoder);
					FrameIndex = 0;
					LoopIndex++;
				}
				else
				{
					FrameIndex++;
				}

				WebPAnimDecoderGetNext(WebPDecoder, &buf, &Delay[FrameIndex]);
				memcpy(pWebPBuf, buf, 4 * WebPWidth * WebPHeight);

				if (FrameIndex > 0)
					DelayDef = Delay[FrameIndex] - Delay[FrameIndex - 1];
				else
					DelayDef = Delay[0];
				if (DelayDef == 0)DelayDef = 100;
				DelayTime = DelayDef;
				if (DelayTime < 17)DelayTime = 17;
			}

			TransBackBuffer();
		}
		return (Result);
	}

	void CSpiLoader::GetWebPData(WebPData *webp_data)
	{
		Animate = EAnimationType_NONE;
		WebPDemuxer* demux = WebPDemux(webp_data);

		uint32_t width = WebPDemuxGetI(demux, WEBP_FF_CANVAS_WIDTH);
		uint32_t height = WebPDemuxGetI(demux, WEBP_FF_CANVAS_HEIGHT);
		// ... (Get information about the features present in the WebP file).
		uint32_t flags = WebPDemuxGetI(demux, WEBP_FF_FORMAT_FLAGS);

		if (flags & ANIMATION_FLAG)
		{
			// Animation Exists
			GetWebPAnimationData(webp_data);
		}
		else
		{
			BITMAPINFO *pWebPInfo = new BITMAPINFO();
			ZeroMemory(&pWebPInfo->bmiHeader, sizeof(BITMAPINFOHEADER));
			ZeroMemory(&pWebPInfo->bmiColors, sizeof(RGBQUAD));
			pWebPInfo->bmiHeader.biSize = (DWORD)sizeof(BITMAPINFOHEADER);

			pWebPInfo->bmiHeader.biPlanes = 1;
			pWebPInfo->bmiHeader.biBitCount = 32;
			pWebPInfo->bmiHeader.biCompression = BI_RGB;
			pWebPInfo->bmiHeader.biWidth = WebPWidth;
			pWebPInfo->bmiHeader.biHeight = -WebPHeight;

			hBitmap = CreateDIBSection(nullptr, pWebPInfo, DIB_RGB_COLORS, (void**)&pWebPBuf, nullptr, 0);
			delete pWebPInfo;

			pWebPBuf = WebPDecodeBGRAInto(webp_data->bytes, webp_data->size, pWebPBuf, 4 * WebPWidth * WebPHeight, WebPWidth * 4);
		}

		if (flags & EXIF_FLAG)
		{
			// Exif Exists
			GetWebPExifData(webp_data);
		}


		WebPDemuxDelete(demux);
	}

	void CSpiLoader::GetWebPAnimationData(WebPData *webp_data)
	{
		WebPAnimDecoderOptionsInit(&WebPDecOptions);
		WebPDecOptions.color_mode = MODE_BGRA;
		WebPDecOptions.use_threads = true;
		// Tune 'WebPDecOptions' as needed.
		WebPDecoder = WebPAnimDecoderNew(webp_data, &WebPDecOptions);
		if (WebPDecoder == nullptr)return;
		WebPAnimInfo anim_info;
		WebPAnimDecoderGetInfo(WebPDecoder, &anim_info);
		
		FrameCount = anim_info.frame_count;
		LoopCount = anim_info.loop_count;

		if (WebPWidth * WebPHeight * 32 * FrameCount < MaxAllFrame)
			AllFrame = true;
		else
			AllFrame = false;

		if (FrameCount == 0)return;

		LoopIndex = 0;
		FrameIndex = 0;
		DropFrame = 0;
		DropCount = 0;

		if (AllFrame == true)
		{
			BITMAPINFO *pWebPInfo = new BITMAPINFO();
			ZeroMemory(&pWebPInfo->bmiHeader, sizeof(BITMAPINFOHEADER));
			ZeroMemory(&pWebPInfo->bmiColors, sizeof(RGBQUAD));
			pWebPInfo->bmiHeader.biSize = (DWORD)sizeof(BITMAPINFOHEADER);

			pWebPInfo->bmiHeader.biPlanes = 1;
			pWebPInfo->bmiHeader.biBitCount = 32;
			pWebPInfo->bmiHeader.biCompression = BI_RGB;
			pWebPInfo->bmiHeader.biWidth = WebPWidth;
			pWebPInfo->bmiHeader.biHeight = -WebPHeight;

			Delay = new int[FrameCount];
			pWebPAnimeBuf = new uint8_t *[FrameCount];
			hWebPAnimeBitmap = new HBITMAP[FrameCount];
			for (int i = 0; i < FrameCount; i++)
			{
				hWebPAnimeBitmap[i] = CreateDIBSection(nullptr, pWebPInfo, DIB_RGB_COLORS, (void**)&pWebPAnimeBuf[i], nullptr, 0);
			}
			delete pWebPInfo;

			int i = 0;
			while (WebPAnimDecoderHasMoreFrames(WebPDecoder) && i < FrameCount)
			{
				uint8_t *buf;
				WebPAnimDecoderHasMoreFrames(WebPDecoder);
				WebPAnimDecoderGetNext(WebPDecoder, &buf, &Delay[i]);
				memcpy(pWebPAnimeBuf[i], buf, 4 * WebPWidth * WebPHeight);
				i++;
			}

			for (i = FrameCount - 1; i > 0; i--)
			{
				Delay[i] -= Delay[i - 1];
				if (Delay[i] == 0)Delay[i] = 100;
			}

			DelayTime = Delay[0];
		}
		else
		{
			WebPBufData.size = webp_data->size;
			WebPBufData.bytes = new BYTE[WebPBufData.size];
			memcpy((void *)WebPBufData.bytes, webp_data->bytes, WebPBufData.size);
			WebPAnimDecoderDelete(WebPDecoder);
			WebPDecoder = WebPAnimDecoderNew(&WebPBufData, &WebPDecOptions);
			WebPAnimDecoderGetInfo(WebPDecoder, &anim_info);

			BITMAPINFO *pWebPInfo = new BITMAPINFO();
			ZeroMemory(&pWebPInfo->bmiHeader, sizeof(BITMAPINFOHEADER));
			ZeroMemory(&pWebPInfo->bmiColors, sizeof(RGBQUAD));
			pWebPInfo->bmiHeader.biSize = (DWORD)sizeof(BITMAPINFOHEADER);

			pWebPInfo->bmiHeader.biPlanes = 1;
			pWebPInfo->bmiHeader.biBitCount = 32;
			pWebPInfo->bmiHeader.biCompression = BI_RGB;
			pWebPInfo->bmiHeader.biWidth = WebPWidth;
			pWebPInfo->bmiHeader.biHeight = -WebPHeight;

			Delay = new int[FrameCount];

			hBitmap = CreateDIBSection(nullptr, pWebPInfo, DIB_RGB_COLORS, (void**)&pWebPBuf, nullptr, 0);
			delete pWebPInfo;

			uint8_t *buf;
			WebPAnimDecoderHasMoreFrames(WebPDecoder);
			WebPAnimDecoderGetNext(WebPDecoder, &buf, &Delay[0]);
			memcpy(pWebPBuf, buf, 4 * WebPWidth * WebPHeight);

			if (Delay[0] == 0)Delay[0] = 100;
			DelayTime = Delay[0];
		}
		PreTGT = NowTGT = timeGetTime();
		Animate = EAnimationType_WEBP;
	}

	void CSpiLoader::GetWebPExifData(WebPData * webp_data)
	{
		//NOTE:未実装
	}
		
	int CSpiLoader::CheckOrientation(void)
	{
		//            if (BitmapGDIP.RawFormat.Equals(ImageFormat.Jpeg))

		try
		{
			UINT size;
			Gdiplus::PropertyItem *buf;
			size = ImageGDIP->GetPropertyItemSize(PropertyTagOrientation);
			if (size == 0)return(0);
			
			buf = (PropertyItem *)malloc(size);
			ImageGDIP->GetPropertyItem(PropertyTagOrientation, size, buf);
			BYTE value = ((BYTE *)(buf->value))[0];
			free(buf);

			switch (value)
			{
			default:
				return(0);
			case 6:
				return (1);
			case 3:
				return (2);
			case 8:
				return (3);
			}
		}
		catch(...)
		{
			return (0);
		}
		//            return (0);
	}

	short CSpiLoader::CheckOrientation(BYTE* pTemp, unsigned int Size)
	{
		short Value = CheckOrientationExif(pTemp, Size);
		short Table[] = { 0, 0, 0, 2, 0, 0, 1, 0, 3 };
		if (Value < 0 || Value > 8) return (0);
		return (Table[Value]);
	}


	short CSpiLoader::CheckOrientationExif(BYTE* pTemp, unsigned int Size)
	{
		unsigned int Int, p;
		unsigned short SInt;
		unsigned short USInt;
		short Result = 0;

		int TMode = 0;
		unsigned int Ofs = 0;
		unsigned int i;

		while (1)
		{
			if (Size < 256) return (0);

			if (pTemp[0] != 0xff || pTemp[1] != 0xd8) return (0);
			if (pTemp[2] != 0xff) return (0);

			p = (unsigned int)(pTemp[4] * 256 + pTemp[5] + 4);

			if (Size <= p) return (0);

			if (pTemp[p] != 0xff) return (0);

			if (pTemp[2] == 0xff && pTemp[3] == 0xe1)
			{
				if (memcmp(pTemp + 6, "Exif\0\0", 6) == 0)
					Ofs = 12;
				else
					return(0);
			}
			else
				return (0);

			//---------------------------------------------

			for (i = Ofs; i < Size; i++)
			{
				//		SInt = *(short *)(void *)(pTemp + i + 2);
				//		if(pTemp[i] == 0x49 && pTemp[i + 1] == 0x49 && SInt == 0x2a){TMode = 1; break;}
				//		if(pTemp[i] == 0x4d && pTemp[i + 1] == 0x4d && SInt == 0x2a00){TMode = 2; break;}
				if (pTemp[i] == 0x49 && pTemp[i + 1] == 0x49) { TMode = 1; break; }
				if (pTemp[i] == 0x4d && pTemp[i + 1] == 0x4d) { TMode = 2; break; }
			}
			if (TMode == 0) break;



			//---------------------------------------------

			Int = *(int *)(void *)(pTemp + i + 4);
			Ofs = i;

			acfc::Endian(&Int, 4, TMode);
			Ofs += Int;
			if (Size < Ofs)return(0);

			//-------------------------------------------------------------------------------------
			// Orientation を探す
			//-------------------------------------------------------------------------------------
			SInt = *(short *)(void *)(pTemp + Ofs);
			Ofs += 2;
			acfc::Endian(&SInt, 2, TMode);
			i = SInt;

			while (i > 0)
			{
				if (Size <= Ofs + 8)return(0);
				USInt = *(short *)(void *)(pTemp + Ofs);
				acfc::Endian(&USInt, 2, TMode);

				switch (USInt)
				{
				case 0x0112:
					Result = *(short *)(void *)(pTemp + Ofs + 8);
					acfc::Endian(&Result, 2, TMode);
					return(Result);
				default:
					break;
				}

				Ofs += 12;
				i--;
			}
			break;
		}
		return(Result);
	}

	EAnimationType CSpiLoader::GetGIFAnimeData(void)
	{
		UINT count;
		UINT TotalBuffer;
		PropertyItem* pItem;
		GUID *pDimensionIDs;
		WCHAR strGuid[39];
		UINT m_FrameCount;

		// フレームディメンションの数を取得
		//frames[animation_frame_index][how_many_animation];
		count = ImageGDIP->GetFrameDimensionsCount();

		// フレームディメンションのリストを取得 GUID を取得する
		pDimensionIDs = new GUID[count];
		ImageGDIP->GetFrameDimensionsList(pDimensionIDs, count);

		// GIF アニメの場合には frame_index = 0 にアニメーションが入っている
		StringFromGUID2(pDimensionIDs[0], strGuid, 39);
		m_FrameCount = ImageGDIP->GetFrameCount(&pDimensionIDs[0]);

		FrameCount = m_FrameCount;
		delete[] pDimensionIDs;

		// ループ回数を得る
		TotalBuffer = ImageGDIP->GetPropertyItemSize(PropertyTagLoopCount);
		pItem = (PropertyItem*)malloc(TotalBuffer);
		ImageGDIP->GetPropertyItem(PropertyTagLoopCount, TotalBuffer, pItem);

		LoopCount = (int)*((short int *)pItem->value);
		free(pItem);

		Delay = new int[FrameCount];

		// ディレイデータを得る
		TotalBuffer = ImageGDIP->GetPropertyItemSize(PropertyTagFrameDelay);
		pItem = (PropertyItem*)malloc(TotalBuffer);
		ImageGDIP->GetPropertyItem(PropertyTagFrameDelay, TotalBuffer, pItem);
		for (int i = 0; i < FrameCount; i++)
		{
			Delay[i] = (int)((unsigned int *)pItem[0].value)[i];
			Delay[i] *= 10;
			if (Delay[i] == 0) Delay[i] = 100;
		}

		free(pItem);
		
		LoopIndex = 0;
		FrameIndex = 0;
		DropFrame = 0;
		DropCount = 0;
		DelayTime = Delay[0];

		GUID Guid = FrameDimensionTime;
		PreTGT = NowTGT = timeGetTime();

		if (OrgWidth * OrgHeight * 32 * FrameCount < MaxAllFrame)
			AllFrame = true;
		else
			AllFrame = false;

		if (AllFrame == true)
		{
			DeleteObject(hBitmap); // 0 フレーム目と重複するので削除
			GIFBitmapGDIP = new Bitmap *[FrameCount];
			GIFhBitmapGDIP = new HBITMAP[FrameCount];
			for (int i = 0; i < FrameCount; i++)
			{
				ImageGDIP->SelectActiveFrame(&Guid, i);
				GIFBitmapGDIP[i] = CreateBMPFromImage(ImageGDIP, &GIFhBitmapGDIP[i]);
			}
			hBitmap = GIFhBitmapGDIP[0];
		}
		else
		{
			GUID Guid = FrameDimensionTime;
			ImageGDIP->SelectActiveFrame(&Guid, 0);
		}

		return (EAnimationType_GIF);
	}

	bool CSpiLoader::SetShowSubIndex(CImageInfo &SrcImageInfo, int SubIndex, FARPROC ProgressCallback)
	{
		if (nowArchivedFileInfo[SubIndex].FileSize > 0)
		{
			std::string SFileName = acfc::UnicodeToMultiByte(ImageInfo.FileName);

			if (ArchiveSpi->GetFile((LPSTR)SFileName.c_str(), (int)nowArchivedFileInfo[SubIndex].Position, (LPSTR)&HFileImage, 0x0100,
				nullptr, 0
			) != 0) return (false);

			pFileImage = (BYTE *)LocalLock(HFileImage);

#ifdef _WIN64
			SArchivedFileInfoW tempAFI = nowArchivedFileInfo[SubIndex];
			std::wstring tPath = tempAFI.Path;
			std::wstring tFileName = tempAFI.FileName;
#else
			SArchivedFileInfo tempAFI = nowArchivedFileInfo[SubIndex];
			std::wstring tPath = acfc::MultiByteToUnicode(tempAFI.Path);
			std::wstring tFileName = acfc::MultiByteToUnicode(tempAFI.FileName);
#endif


			std::wstring AFileName = tPath + tFileName;
			SrcImageInfo.FileName = AFileName;
			SrcImageInfo.FileSize = tempAFI.FileSize;
			SrcImageInfo.Timestamp = tempAFI.Timestamp;

			if (CheckFileType(SrcImageInfo, pFileImage, (LONG)nowArchivedFileInfo[SubIndex].FileSize))
			{
				if (LoadFromFileInMemory(SrcImageInfo, pFileImage, (LONG)nowArchivedFileInfo[SubIndex].FileSize, ProgressCallback) == true)
				{
					Clear(EPluginMode_ACVINNER);
					Mode = (EPluginMode)(Mode | EPluginMode_ACVINNER);
					pre_HFileImage = HFileImage;
					pre_pFileImage = pFileImage;
					return (true);
				}
			}
			LocalUnlock(HFileImage);
			LocalFree(HFileImage);
			HFileImage = nullptr;
		}

		return (false);
	}


	bool CSpiLoader::GetArchiveFileList(std::vector<CImageInfo>* SubFileList) // ファイルリストを取得
	{
		int i;
		SubFileList->clear();
		for (i = 0; i < (int)(nowArchivedFileInfo.size()); i++)
		{
#ifdef _WIN64
			std::wstring tPath = nowArchivedFileInfo[i].Path;
			std::wstring tFileName = nowArchivedFileInfo[i].FileName;
#else
			std::wstring tPath = acfc::MultiByteToUnicode(nowArchivedFileInfo[i].Path);
			std::wstring tFileName = acfc::MultiByteToUnicode(nowArchivedFileInfo[i].FileName);
#endif

			CImageInfo NewII;

			NewII.Timestamp = nowArchivedFileInfo[i].Timestamp;
			NewII.FileSize = nowArchivedFileInfo[i].FileSize;
			NewII.Rotate = -1;
			NewII.FileName = tPath + tFileName;
			SubFileList->push_back(NewII);
		}
		return (true);
	}

	bool CSpiLoader::SetSubImageFile(CImageInfo &Src)
	{
		int i;

		for (i = 0; i < ImageNum; i++)
		{
#ifdef _WIN64
			SArchivedFileInfoW tempAFI = nowArchivedFileInfo[i];
			std::wstring tPath = tempAFI.Path;
			std::wstring tFileName = tempAFI.FileName;
#else
			SArchivedFileInfo tempAFI = nowArchivedFileInfo[i];
			std::wstring tPath = acfc::MultiByteToUnicode(tempAFI.Path);
			std::wstring tFileName = acfc::MultiByteToUnicode(tempAFI.FileName);
#endif


			if (Src.FileName == tPath + tFileName) break;
		}
		if (i == ImageNum) return (false);
		bool Result = SetShowSubIndex(Src, i, ProgressCallback);
		SubImageInfo = Src;
		return (Result);
	}

	bool CSpiLoader::SetSubImageFile(std::vector<CImageInfo>* FileList, int &i, int Dir)
	{
		while (FileList->size() > 0)
		{
			if (SetSubImageFile((*FileList)[i]) == true)
			{
				return (true);
			}

			std::vector<CImageInfo>::iterator itr = FileList->begin();
			itr += i;
			FileList->erase(itr);

			if (Dir < 0) i += Dir;
			if (i < 0) break;
			if (i >= (int)(FileList->size())) break;
		}
		return (false);
	}

	// 画像データをクリアする
	void CSpiLoader::Clear(EPluginMode DelMode)
	{
		switch ((Mode & DelMode & EPluginMode_PICTURE))
		{
		case EPluginMode_GDIP:
			if (hBitmap != nullptr)
			{
				DeleteObject(hBitmap);
				hBitmap = nullptr;
			}

			if (BitmapGDIP != nullptr)
			{
				delete BitmapGDIP;
				BitmapGDIP = nullptr;
			}

			if (ImageGDIP != nullptr)
			{
				delete ImageGDIP;
				ImageGDIP = nullptr;
			}

			if (hMemoryGDIP != nullptr)
			{
				GlobalFree(hMemoryGDIP);
				hMemoryGDIP = nullptr;
			}
			
			if (StreamGDIP != nullptr)
			{
				StreamGDIP->Release();
				StreamGDIP = nullptr;
			}

			if (Animate != EAnimationType_NONE)
			{
				if (Delay != nullptr)
				{
					delete[] Delay;
					Delay = nullptr;
				}

				if (GIFBitmapGDIP != nullptr)
				{
					for (int i = 0; i < FrameCount; i++)
					{
						delete GIFBitmapGDIP[i];
						DeleteObject(GIFhBitmapGDIP[i]);
					}
					delete[] GIFBitmapGDIP;
					delete[] GIFhBitmapGDIP;
					GIFBitmapGDIP = nullptr;
					GIFhBitmapGDIP = nullptr;
				}
				Animate = EAnimationType_NONE;
			}
			break;

		case EPluginMode_SPI:
			if (BitmapGDIP != nullptr)
			{
				delete BitmapGDIP;
				BitmapGDIP = nullptr;
			}

			if (hBitmap != nullptr)
			{
				DeleteObject(hBitmap);
				hBitmap = nullptr;
			}
			break;

		case EPluginMode_WEBP:
			if (WebPDecoder != nullptr)
			{
				WebPAnimDecoderDelete(WebPDecoder);
				WebPDecoder = nullptr;

				if (WebPBufData.size > 0)
				{
					delete[]WebPBufData.bytes;
					WebPBufData.bytes = 0;
					WebPBufData.size = 0;
				}

				if (hWebPAnimeBitmap != nullptr)
				{
					if (Delay != nullptr)
					{
						delete[] Delay;
						Delay = nullptr;
					}
					for (int i = 0; i < FrameCount; i++)
					{
						DeleteObject(hWebPAnimeBitmap[i]);
					}
					delete[] hWebPAnimeBitmap; hWebPAnimeBitmap = nullptr;
					delete[] pWebPAnimeBuf; pWebPAnimeBuf = nullptr;
					
					hBitmap = nullptr; // hWebPAimeBitmap[i] のいずれかが入っているので削除する必要はない
				}
				Animate = EAnimationType_NONE;
			}
			if (hBitmap != nullptr)
			{
				DeleteObject(hBitmap);
				hBitmap = nullptr;
			}
			break;
		}


		if ((DelMode & EPluginMode_PICTURE) != 0)
		{
			OrgWidth = 0;
			OrgHeight = 0;
			DeleteRotateBuffer();
		}

		if ((Mode & DelMode & EPluginMode_ARCHIVE) != 0)
		{
			if (oldArchivedFileInfo.size() != 0)
			{
				oldArchivedFileInfo.clear();
			}
			ImageInfo.Clear();
			SubImageInfo.Clear();
		}

		if ((Mode & DelMode & EPluginMode_ACVINNER) != 0)
		{
			if(pre_HFileImage != nullptr)
			{
				LocalUnlock(pre_HFileImage);
				LocalFree(pre_HFileImage);
				pre_HFileImage = nullptr;
				pre_pFileImage = nullptr;
			}

			SubImageInfo.Clear();
		}

		if ((DelMode & EPluginMode_CLEARFILE) != 0)
		{
			ImageInfo.Clear();
			SubImageInfo.Clear();
		}

		Mode = (EPluginMode)(Mode & (EPluginMode_ALL ^ DelMode));
	}

	void CSpiLoader::DeleteBackBuffer(void)
	{
		if (hBmpData != nullptr)
		{
			DeleteObject(hBmpData);
			hBmpData = nullptr;
			BufMaxWidth = 0;
			BufMaxHeight = 0;
		}
	}

	void CSpiLoader::OpenSpiSettingDialog(HWND hwnd)
	{
		if (NowSpi->ConfigurationDlg != nullptr)
			NowSpi->ConfigurationDlg(hwnd, 1);
		else
			return;
	}

	Gdiplus::Bitmap* CSpiLoader::DuplicateImage(int newWidth, int newHeight)
	{
		if ((Mode & EPluginMode_ALL) == 0) return (nullptr);

		if (Showing == false)return(false);

		DIBSECTION ds;
		GetObject(hBitmap, sizeof(DIBSECTION), &ds);

		//make sure compression is BI_RGB
		ds.dsBmih.biCompression = BI_RGB;

		// NOTE:WebP フォーマットは逆に入っている フォーマットを追加する場合には増やす必要がある
		if (Mode & EPluginMode_WEBP)ds.dsBmih.biHeight = -ds.dsBmih.biHeight;

		//Convert DIB to DDB
		HDC hdc = GetDC(nullptr);
		HBITMAP hBmp = CreateDIBitmap(hdc, &ds.dsBmih, CBM_INIT,
			ds.dsBm.bmBits, (BITMAPINFO*)&ds.dsBmih, DIB_RGB_COLORS);
		ReleaseDC(nullptr, hdc);
		DeleteDC(hdc);

		Gdiplus::Bitmap *bmp = new Bitmap(hBmp, 0);

		DeleteObject(hBmp);

		return (bmp);
	}


	Gdiplus::Bitmap* CSpiLoader::DuplicateBGImage(int newWidth, int newHeight)
	{
		if ((Mode & EPluginMode_ALL) == 0) return (nullptr);
		
		if (Showing == false)return(false);

		// 転送先の準備 
		BITMAPINFO *pbi = new BITMAPINFO();
		BYTE *pB;
		HBITMAP hbd;
		ZeroMemory(&pbi->bmiHeader, sizeof(BITMAPINFOHEADER));
		ZeroMemory(&pbi->bmiColors, sizeof(RGBQUAD));
		pbi->bmiHeader.biSize = (DWORD)sizeof(BITMAPINFOHEADER);

		pbi->bmiHeader.biPlanes = 1;
		pbi->bmiHeader.biBitCount = 32;
		pbi->bmiHeader.biCompression = BI_RGB;
		pbi->bmiHeader.biWidth = newWidth;
		pbi->bmiHeader.biHeight = -newHeight;

		hbd = CreateDIBSection(nullptr, pbi, DIB_RGB_COLORS, (void **)&pB, nullptr, 0);

		// 転送元の準備
		HDC sDC = CreateCompatibleDC(nullptr);
		HBITMAP sDC_Bitmap = (HBITMAP)SelectObject(sDC, hBmpData);

		// 転送先の準備
		HDC dDC = CreateCompatibleDC(sDC);
		HBITMAP dDC_Bitmap = (HBITMAP)SelectObject(dDC, hbd);

		// 転送
		int OldStretchMode = SetStretchBltMode(dDC, STRETCH_HALFTONE);
		POINT p = {};
		SetBrushOrgEx(dDC, 0, 0, &p);

		StretchBlt(dDC, 0, 0, newWidth, newHeight,
			sDC, 0, BufMaxHeight - SrcRHeight, SrcRWidth, SrcRHeight,
			SRCCOPY);

		SetStretchBltMode(dDC, OldStretchMode);

		Gdiplus::Bitmap *bmp = new Bitmap(hbd, 0);

		SelectObject(sDC, sDC_Bitmap);
		SelectObject(dDC, dDC_Bitmap);
		DeleteDC(sDC);
		DeleteDC(dDC);

		delete pbi;
		DeleteObject(hbd);

		return (bmp);
	}

	// 画像をクリップボードにコピー
	bool CSpiLoader::CopyImageToClipboard(void)
	{
		if (Showing == false)return(false);

		DIBSECTION ds;
		GetObject(hBitmap, sizeof(DIBSECTION), &ds);

		//make sure compression is BI_RGB
		ds.dsBmih.biCompression = BI_RGB;

		// NOTE:WebP フォーマットは逆に入っている フォーマットを追加する場合には増やす必要がある
		if (Mode & EPluginMode_WEBP)ds.dsBmih.biHeight = -ds.dsBmih.biHeight;

		//Convert DIB to DDB
		HDC hdc = GetDC(nullptr);
		HBITMAP hBmp = CreateDIBitmap(hdc, &ds.dsBmih, CBM_INIT,
			ds.dsBm.bmBits, (BITMAPINFO*)&ds.dsBmih, DIB_RGB_COLORS);
		ReleaseDC(nullptr, hdc);
		DeleteDC(hdc);

		OpenClipboard(hWindow);
		EmptyClipboard();
		SetClipboardData(CF_BITMAP, hBmp);
		CloseClipboard();

		return (true);
	}

	int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
	{
		using namespace Gdiplus;
		UINT  num = 0;          // number of image encoders
		UINT  size = 0;         // size of the image encoder array in bytes

		ImageCodecInfo* pImageCodecInfo = NULL;

		GetImageEncodersSize(&num, &size);
		if (size == 0)
			return -1;  // Failure

		pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
		if (pImageCodecInfo == NULL)
			return -1;  // Failure

		GetImageEncoders(num, size, pImageCodecInfo);

		for (UINT j = 0; j < num; ++j)
		{
			if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
			{
				*pClsid = pImageCodecInfo[j].Clsid;
				free(pImageCodecInfo);
				return j;  // Success
			}
		}

		free(pImageCodecInfo);
		return 0;
	}

	bool CSpiLoader::SaveJpeg(std::wstring svFileName, int svWidth, int svHeight, int svCompLevel, bool TransFormed)
	{
		Bitmap *bmp;
		if(TransFormed == false)
			bmp = DuplicateImage(svWidth, svHeight);
		else
			bmp = DuplicateBGImage(svWidth, svHeight);

		CLSID clsid;
		GetEncoderClsid(L"image/jpeg", &clsid);

		ULONG qualityValue = svCompLevel;
		Gdiplus::EncoderParameters params = {};
		params.Count = 1;
		params.Parameter[0].Guid = Gdiplus::EncoderQuality;
		params.Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong;
		params.Parameter[0].NumberOfValues = 1;
		params.Parameter[0].Value = &qualityValue;

		bmp->Save(svFileName.c_str(), &clsid, &params);

		delete bmp;
		return(true);
	}

	bool CSpiLoader::SavePNG(std::wstring svFileName, int svWidth, int svHeight, bool TransFormed)
	{
		Bitmap *bmp;
		if (TransFormed == false)
			bmp = DuplicateImage(svWidth, svHeight);
		else
			bmp = DuplicateBGImage(svWidth, svHeight);

		CLSID clsid;
		GetEncoderClsid(L"image/png", &clsid);
		bmp->Save(svFileName.c_str(), &clsid);
		delete bmp;
		return (true);
	}
}
