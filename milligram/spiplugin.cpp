#include "stdafx.h"
#include "spiplugin.h"
#include "acfc.h"

#define IGNORE_FILEMINSIZE 256

namespace milligram
{
	CSpiLoader::CSpiLoader(void)
	{
		Gdiplus::GdiplusStartupInput gdiplusStartupInput; // = {1, NULL, FALSE, FALSE};
		gdiplusStartupInput.GdiplusVersion = 1;
		gdiplusStartupInput.DebugEventCallback = NULL;
		gdiplusStartupInput.SuppressBackgroundThread = FALSE;
		gdiplusStartupInput.SuppressExternalCodecs = FALSE;

		GdiplusToken = NULL;
		Gdiplus::GdiplusStartup(&GdiplusToken, &gdiplusStartupInput, NULL);

		InternalLoader = L"*.bmp;*.jpg;*.jpeg;*.png;*.gif;";
	}

	CSpiLoader::~CSpiLoader(void)
	{
	}

	void CSpiLoader::Init(HWND hWndNewOwner, FARPROC aProgressCallBack)
	{
		windowHandle = hWndNewOwner;
		ProgressCallback = aProgressCallBack;
		ErrorFlag = true;

		Spi = std::vector<CSpiPlugin *>();

		// マルチメディアタイマーの設定
		TIMECAPS TimeCaps;
		timeGetDevCaps(&TimeCaps, sizeof(TIMECAPS));
		ShortestPeriod = TimeCaps.wPeriodMin;

		timeBeginPeriod((UINT)ShortestPeriod);
	}

	void CSpiLoader::Release(void)
	{
		Clear(EPluginMode_ALL);
		Mode = EPluginMode_NONE;

		ClearAllPlugins();

		timeEndPeriod((UINT)ShortestPeriod);
		CheckBackBuffer(0, 0);

		Gdiplus::GdiplusShutdown(GdiplusToken);
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
	void CSpiLoader::SetSpiPathes(std::vector<std::wstring> &Pathes)
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

			acfc::GetFiles(temp, Pathes[i], L"*.spi");
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

		bool Result = 0;

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
						// 実際にマップドされたメモリから読み込む
						Result = LoadFromFileInMemory(SrcImageInfo, FileData, SizeL, ProgressCallback);
					}
					// メモリマップドファイルの終了
					UnmapViewOfFile(FileData);
				}
				CloseHandle(hMap);
			}
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
		if (FileData == nullptr) return (false);

		Result = CheckGIFAnimeEnabled(SrcImageInfo, FileData, FileSize);

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
			PostFileExt = acfc::UnicodeToMultiByte(acfc::GetFileExt(SrcImageInfo.FileName));
			int i;
			// SPI プラグインが必要かどうか調べる。
			for (i = 0; i < (int)(Spi.size()); i++)
			{
				if (Spi[i]->IsSupported((LPSTR)PostFileExt.c_str(), (DWORD)FileData) != 0)
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
		}
		catch(...)
		{
			return (false);
		}

		// -----------------------------------------------------------------------------
		return (false);
	}

	bool CSpiLoader::CheckGIFAnimeEnabled(CImageInfo &SrcImageInfo, BYTE* FileData, long FileSize)
	{
		PostGIFAnimate = false;
		if (FileSize < 18)return(true);
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
		PostGIFAnimate = true;
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
		if ((int)((BYTE *)FileData[0]) == 0xFF && (int)((BYTE *)FileData[1]) == 0xD8)
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

	// 画像ファイルを実際に読み込む
	bool CSpiLoader::LoadFromFileInMemory(CImageInfo &SrcImageInfo, BYTE* FileData, UINT DataSize, FARPROC ProgressCallback)
	{
		if (FileData == nullptr) return (false);
		SPictureInfo PicInfo = { 0 };

		Showing = false;

		// アーカイブモードかどうかチェックする
		// アーカイブモードならファイルの一覧を取得する
		if (PostSpi != nullptr && strncmp(PostSpi->APIVersion, "00AM", 4) == 0 && SubImageInfo.FileName == L"")
		{
			HANDLE HInfo;
			if (PostSpi->GetArchiveInfo((LPSTR)FileData, (int)DataSize, 1, (void**)&HInfo) != 0) return (false);

			SArchivedFileInfo *pInfos = (SArchivedFileInfo *)LocalLock(HInfo);
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

		// SPI モードでない場合、素でファイルを読み込んでみる
		if (PostSpi == nullptr)
		{
			Clear(EPluginMode_PICTURE);

			GIFAnimate = PostGIFAnimate;

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
			}
			catch(...)
			{
				GIFAnimate = false;
				return (false);
			}

			OrgWidth = ImageGDIP->GetWidth();
			OrgHeight = ImageGDIP->GetHeight();

			BitmapGDIP = CreateBMPFromImage(ImageGDIP);
			BitmapGDIP->GetHBITMAP(0, &hBitmap);

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

			if (GIFAnimate == true)
				GIFAnimate = GetGIFAnimeData();

			PluginName = L"Internal Loader";
		}
		else if (strncmp(PostSpi->APIVersion, "00IN", 4) == 0) // TODO:Susie Plugin 動作確認
		{
			if (FixRotate == false)
			{
				Rotate = SrcImageInfo.Rotate;
				if (Rotate < 0) Rotate = CheckOrientation(FileData, (int)DataSize);
			}
			else
			{
				if (Rotate < 0) Rotate = CheckOrientation(FileData, (int)DataSize);
			}
			SrcImageInfo.Rotate = Rotate;

			try
			{
				//if (PostSpi->GetPictureInfo((LPSTR)FileData, (int)DataSize, 1, &PicInfo) == 0) return (false); 必ず失敗する関数があるらしい
				//if (PicInfo.Info != 0)GlobalFree(PicInfo.Info);         // TEXT info 解放
				HANDLE HBmpInfo, HBmpData;

				if (PostSpi->GetPicture((LPSTR)FileData, (int)DataSize, 1, &HBmpInfo, &HBmpData, ProgressCallback, 0) != 0) return (false);

				Clear(EPluginMode_PICTURE);

				HANDLE pBmp = GlobalLock(HBmpData); // Win32 API でロック
				BITMAPINFO *pInfo = (BITMAPINFO*)GlobalLock(HBmpInfo);

				OrgWidth = pInfo->bmiHeader.biWidth;
				OrgHeight = pInfo->bmiHeader.biHeight;

				BitmapGDIP = new Gdiplus::Bitmap(pInfo, pBmp);
				BitmapGDIP->GetHBITMAP(0, &hBitmap);

				GlobalUnlock(pBmp);
				GlobalUnlock(pInfo);
				GlobalFree(pBmp);
				GlobalFree(pInfo);
			}
			catch (...)
			{
				return(false);
			}

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
		TransBackBuffer();
		return (true);
	}

	Gdiplus::Bitmap* CSpiLoader::CreateBMPFromImage(Gdiplus::Image *Src)
	{
		Gdiplus::Bitmap *dest = new Gdiplus::Bitmap(Src->GetWidth(), Src->GetHeight());

		HBITMAP hdest;
		dest->GetHBITMAP(0, &hdest);

		HDC hdc = CreateCompatibleDC(nullptr);
		HBITMAP hdc_hdest = (HBITMAP)SelectObject(hdc, hdest);

		Graphics g(dest);
		g.DrawImage(Src, 0, 0, Src->GetWidth(), Src->GetHeight());
		g.ReleaseHDC(hdc);

		SelectObject(hdc, hdc_hdest);
		ReleaseDC(nullptr, hdc);

		DeleteDC(hdc);
		DeleteObject(hdest);

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
		pBmpInfo->bmiHeader.biSize = (DWORD)sizeof(BITMAPINFOHEADER);

		pBmpInfo->bmiHeader.biPlanes = 1;
		pBmpInfo->bmiHeader.biBitCount = 32;
		pBmpInfo->bmiHeader.biCompression = BI_RGB;
		pBmpInfo->bmiHeader.biWidth = cWidth;
		pBmpInfo->bmiHeader.biHeight = cHeight;

		hBmpData = CreateDIBSection(nullptr, pBmpInfo, DIB_RGB_COLORS, (void **)&pBmpData, nullptr, 0);

		BufMaxWidth = cWidth;
		BufMaxHeight = cHeight;
	}


	bool CSpiLoader::SetFormSize(int iWidth, int iHeight)
	{
		switch (Rotate)
		{
		case 1:
		case 3:
			int c = iHeight;
			iHeight = iWidth;
			iWidth = c;
			break;

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

		return (false);
	}

	// バックバッファに画像を転送する
	bool CSpiLoader::TransBackBuffer(void)
	{
		if (Showing == false)return(true);
		POINT points[3];

		switch (Rotate)
		{
		case 0:
		case 2:
		default:
			SrcRWidth = BufWidth;
			SrcRHeight = BufHeight;
			ORotWidth = OrgWidth;
			ORotHeight = OrgHeight;
			break;
		case 1:
		case 3:
			SrcRWidth = BufHeight;
			SrcRHeight = BufWidth;
			ORotWidth = OrgHeight;
			ORotHeight = OrgWidth;
			break;
		}

		CheckBackBuffer(SrcRWidth, SrcRHeight);
		// 転送先を準備する
		switch (Rotate)
		{
		case 0:
			points[0].x = 0; points[0].y = BufMaxHeight - BufHeight;
			points[1].x = BufWidth; points[1].y = BufMaxHeight - BufHeight;
			points[2].x = 0; points[2].y = BufMaxHeight;
			break;
		case 1:
			points[0].x = BufHeight; points[0].y = BufMaxHeight - BufWidth;
			points[1].x = BufHeight; points[1].y = BufMaxHeight;
			points[2].x = 0; points[2].y = 0 + BufMaxHeight - BufWidth;
			break;
		case 2:
			points[0].x = BufWidth - 1; points[0].y = BufMaxHeight - 1;
			points[1].x = 0 - 1; points[1].y = BufMaxHeight - 1;
			points[2].x = BufWidth - 1; points[2].y = BufMaxHeight - BufHeight - 1;
			break;
		case 3:
			points[0].x = 0; points[0].y = BufMaxHeight;
			points[1].x = 0; points[1].y = BufMaxHeight - BufWidth;
			points[2].x = BufHeight; points[2].y = BufMaxHeight;
			break;
		}


		// 転送元の準備
		HDC sDC = CreateCompatibleDC(nullptr);
		HBITMAP sDC_Bitmap = (HBITMAP)SelectObject(sDC, hBitmap);

		// 転送先の準備
		HDC dDC = CreateCompatibleDC(sDC);
		HBITMAP dDC_Bitmap = (HBITMAP)SelectObject(dDC, hBmpData);

		// 転送
		int OldStretchMode = SetStretchBltMode(dDC, STRETCH_HALFTONE);
		POINT p;
		SetBrushOrgEx(dDC, 0, 0, &p);

		bool result = PlgBlt(dDC, points,
			sDC,
			0, 0, OrgWidth, OrgHeight,
			nullptr, 0, 0);

		SetStretchBltMode(dDC, OldStretchMode);

		int value;
		if (result == false)
		{
			value = GetLastError();
			SelectObject(sDC, sDC_Bitmap);
			SelectObject(dDC, dDC_Bitmap);
			DeleteDC(sDC);
			DeleteDC(dDC);
			return (false);
		}

		SetStretchBltMode(dDC, OldStretchMode);

		SelectObject(sDC, sDC_Bitmap);
		SelectObject(dDC, dDC_Bitmap);
		DeleteDC(sDC);
		DeleteDC(dDC);

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

	bool CSpiLoader::PauseGIFAnimate(void)
	{
		if (Showing == false || GIFAnimate == false) return (false);
		PausedTGT = timeGetTime();
		return (true);
	}
	bool CSpiLoader::RestartGIFAnimate(void)
	{
		if (Showing == false || GIFAnimate == false) return (false);
		PreTGT += timeGetTime() - PausedTGT;
		return (true);
	}

	bool CSpiLoader::GIFAnimateUpDateFrame(bool FrameSkip)
	{
		bool lockTaken = false;
		bool Result = true;
		try
		{
			while (true)
			{
				if (LoopCount <= LoopIndex && LoopCount > 0)
				{
					Result = false;
					break;
				}

				FrameIndex++;
				if (FrameIndex >= FrameCount)
				{
					LoopIndex++;

					DropFrame = DropCount;
					DropCount = 0;

					if (LoopCount == LoopIndex)
					{
						Result = false;
						break;
					}

					FrameIndex = 0;
				}

				if (GIFAnimate == false)
				{
					Result = false;
					break;
				}
				DeleteObject(hBitmap);

				NowTGT = timeGetTime();

				int D;
				if (NowTGT - PreTGT < DelayTime)
				{
					D = 0;
				}
				else
				{
					D = (int)(NowTGT - PreTGT) - (int)DelayTime;
				}
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
					if (DelayTime < 0)DelayTime = 17;
				}
				GetGIFAnimeData(FrameIndex);

				TransBackBuffer();
				break;
			}
		}
		catch (...)
		{
		}

		return (Result);
	}

	void CSpiLoader::GetGIFAnimeData(int aFrameIndex)
	{
		if (GIFAllFrame)
		{
			DeleteObject(hBitmap);
			GIFBitmapGDIP[FrameIndex]->GetHBITMAP(0, &hBitmap);
		}
		else
		{
			GUID Guid = FrameDimensionTime;
			ImageGDIP->SelectActiveFrame(&Guid, FrameIndex);
			DeleteObject(hBitmap);
			delete BitmapGDIP;
			BitmapGDIP = CreateBMPFromImage(ImageGDIP);

			BitmapGDIP->GetHBITMAP(0, &hBitmap);
		}
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


		if (Size < 256) return (0);

		if (pTemp[0] != 0xff || pTemp[1] != 0xd8) return (0);
		if (pTemp[2] != 0xff) return (0);

		p = (unsigned int)(pTemp[4] * 256 + pTemp[5] + 4);

		if (Size < p) return (0);

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
		if (TMode == 0) goto setorientationlefttop_break;



	//---------------------------------------------

		Int = *(int *)(void *)(pTemp + i + 4);
		Ofs = i;

		acfc::Endian(&Int, 4, TMode);
		Ofs += Int;
		if(Size < Ofs)return(0);

	//-------------------------------------------------------------------------------------
	// Orientation を探す
	//-------------------------------------------------------------------------------------
		SInt = *(short int *)(void *)(pTemp + Ofs);
		Ofs += 2;
		acfc::Endian(&SInt, 2, TMode);
		i = SInt;

		while(i > 0)
		{
			if(Size < Ofs + 8)return(0);
			USInt = *(short int *)(void *)(pTemp + Ofs);
			acfc::Endian(&USInt, 2, TMode);

			switch(USInt)
			{
				case 0x0112:
					Result = *(short int *)(void *)(pTemp + Ofs + 8);
					acfc::Endian(&Result, 2, TMode);
					return(Result);
				default:
					break;
			}

			Ofs += 12;
			i--;
		}

	setorientationlefttop_break:
		return(Result);
	}

	bool CSpiLoader::GetGIFAnimeData(void)
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

		LoopCount = (int)((short int *)pItem[0].value)[0];
		free(pItem);

		Delay = new int[FrameCount];

		// ディレイデータを得る
		TotalBuffer = ImageGDIP->GetPropertyItemSize(PropertyTagFrameDelay); // TODO:GIF アニメの読み込みの確認
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

		if (OrgWidth * OrgHeight * 32 * FrameCount < GIFMaxAllFrame)
			GIFAllFrame = true;
		else
			GIFAllFrame = false;

		if (GIFAllFrame == true)
		{
			GIFBitmapGDIP = new Bitmap *[FrameCount];
			for (int i = 0; i < FrameCount; i++)
			{
				ImageGDIP->SelectActiveFrame(&Guid, i);
				GIFBitmapGDIP[i] = CreateBMPFromImage(ImageGDIP);
			}
		}
		else
		{
			GUID Guid = FrameDimensionTime;
			ImageGDIP->SelectActiveFrame(&Guid, 0);
		}


		return (true);
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

			SArchivedFileInfo tempAFI = nowArchivedFileInfo[SubIndex];

			std::wstring tPath = acfc::MultiByteToUnicode(tempAFI.Path);
			std::wstring tFileName = acfc::MultiByteToUnicode(tempAFI.FileName);

			std::wstring AFileName = tPath + tFileName;
			SrcImageInfo.FileName = AFileName;
			SrcImageInfo.FileSize = tempAFI.FileSize;
			SrcImageInfo.Timestamp = tempAFI.Timestamp;

			if (CheckFileType(SrcImageInfo, pFileImage, nowArchivedFileInfo[SubIndex].FileSize))
			{
				if (LoadFromFileInMemory(SrcImageInfo, pFileImage, nowArchivedFileInfo[SubIndex].FileSize, ProgressCallback) == true)
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
			std::wstring tPath = acfc::MultiByteToUnicode(nowArchivedFileInfo[i].Path);
			std::wstring tFileName = acfc::MultiByteToUnicode(nowArchivedFileInfo[i].FileName);

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
			SArchivedFileInfo tempAFI = nowArchivedFileInfo[i];

			std::wstring tPath = acfc::MultiByteToUnicode(tempAFI.Path);
			std::wstring tFileName = acfc::MultiByteToUnicode(tempAFI.FileName);

			if (Src.FileName == tPath + tFileName) break;
		}
		if (i == ImageNum) return (false);
		bool Result = SetShowSubIndex(Src, i, ProgressCallback);
		SubImageInfo = Src;
		return (Result);
	}

	bool CSpiLoader::SetSubImageFile(std::vector<CImageInfo>* FileList, int &i, int Ofs)
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

			if (Ofs < 0) i += Ofs;
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
//				GlobalUnlock(hMemoryGDIP);
				GlobalFree(hMemoryGDIP);
				hMemoryGDIP = nullptr;
				pMemoryGDIP = nullptr;
			}
			
			if (StreamGDIP != nullptr)
			{
				StreamGDIP->Release();
				StreamGDIP = nullptr;
			}

			if (GIFAnimate)
			{
				delete[] Delay;
				Delay = nullptr;
				if (GIFBitmapGDIP != nullptr)
				{
					for (int i = 0; i < FrameCount; i++)
					{
						delete GIFBitmapGDIP[i];
					}
					delete[] GIFBitmapGDIP;
					GIFBitmapGDIP = nullptr;
				}
				GIFAnimate = false;
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
			}
			break;
		}

		if ((DelMode & EPluginMode_PICTURE) != 0)
		{
			OrgWidth = 0;
			OrgHeight = 0;
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
		
		// 転送先の準備

		HDC dDC = CreateCompatibleDC(nullptr);
		HBITMAP hBmp = CreateCompatibleBitmap(dDC, newWidth, newHeight);
		HDC sDC = CreateCompatibleDC(dDC);
		HBITMAP sDC_hBitmap = (HBITMAP)SelectObject(sDC, hBitmap);

		int OldStretchMode = SetStretchBltMode(dDC, STRETCH_HALFTONE);
		bool res = StretchBlt(dDC,
			0, 0, newWidth, newHeight,
			sDC,
			0, 0, OrgWidth, OrgHeight,
			SRCCOPY);

		SetStretchBltMode(dDC, OldStretchMode);

		DeleteDC(dDC);
		SelectObject(sDC, sDC_hBitmap);
		DeleteDC(sDC);

		Gdiplus::Bitmap *bmp = new Bitmap(hBitmap, 0);

		DeleteObject(hBmp);


/*		// 転送先の準備
		HDC dDC = GetDC(windowHandle);
		HBITMAP hBmp = CreateCompatibleBitmap(dDC, newWidth, newHeight);
		HDC sDC = CreateCompatibleDC(dDC);
		HBITMAP sDC_hBitmap = (HBITMAP)SelectObject(sDC, hBitmap);

		int OldStretchMode = SetStretchBltMode(dDC, STRETCH_HALFTONE);
		bool res = StretchBlt(dDC,
			0, 0, newWidth, newHeight,
			sDC,
			0, 0, OrgWidth, OrgHeight,
			SRCCOPY);

		SetStretchBltMode(dDC, OldStretchMode);
		
		ReleaseDC(windowHandle, dDC);
		SelectObject(sDC, sDC_hBitmap);
		DeleteDC(sDC);

		Gdiplus::Bitmap *bmp = new Bitmap(hBitmap, 0);

		DeleteObject(hBmp);*/

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

		//Convert DIB to DDB
		HDC hdc = GetDC(NULL);
		HBITMAP hBmp = CreateDIBitmap(hdc, &ds.dsBmih, CBM_INIT,
			ds.dsBm.bmBits, (BITMAPINFO*)&ds.dsBmih, DIB_RGB_COLORS);
		ReleaseDC(NULL, hdc);
		DeleteDC(hdc);

		OpenClipboard(windowHandle);
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

	bool CSpiLoader::SaveJpeg(std::wstring svFileName, int svWidth, int svHeight, int svCompLevel)
	{
		Bitmap *bmp = DuplicateImage(svWidth, svHeight);
		CLSID clsid;
		GetEncoderClsid(L"image/jpeg", &clsid);

		ULONG qualityValue = svCompLevel;
		Gdiplus::EncoderParameters params = { 0 };
		params.Count = 1;
		params.Parameter[0].Guid = Gdiplus::EncoderQuality;
		params.Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong;
		params.Parameter[0].NumberOfValues = 1;
		params.Parameter[0].Value = &qualityValue;

		bmp->Save(svFileName.c_str(), &clsid, &params);

		delete bmp;
		return(true);
	}

	bool CSpiLoader::SavePNG(std::wstring svFileName, int svWidth, int svHeight)
	{
		Bitmap *bmp = DuplicateImage(SrcRWidth, SrcRHeight);
		CLSID clsid;
		GetEncoderClsid(L"image/png", &clsid);
		bmp->Save(svFileName.c_str(), &clsid);
		delete bmp;
		return (true);
	}
}
