#pragma hdrstop

#include "acfc_VCLSub.cpp"

#include "SpiPlugin.h"
#include "stdio.h"
#include "sys\stat.h"

//---------------------------------------------------------------------------

TSpiLoader::TSpiLoader(void *aProgressCallback)
{
	memset(this, 0, sizeof(TSpiLoader));
	ProgressCallback = (FARPROC)aProgressCallback;
	ErrorFlag = true;
	SpiPrecede = false;
}

TSpiLoader::~TSpiLoader(void)
{
	Clear(MMODE_ALL);
	DeleteDBuffer();

	if(Spi != NULL)
	{
		int i;
		for(i = 0;i < SpiNum;i++)
		{
			delete Spi[i];
		}
		delete[] Spi;
	}
	Mode = 0;
}


// プラグイン一覧を取得し、実際に、プラグインを読み込む
void	TSpiLoader::SetSpiPathes(TStringList *Pathes)
{
	int i;
	UnicodeString Temp;
	TSpiPlugin *Plg;
	TList *SpiPLists;

	TStringList *SpiFullLists; // フルパス
	TStringList *SpiNameLists; // ファイル名のみ
	TStringList *SpiGetLists; // 最終的に取得すべきフルパス

	// 古いポインタを削除する
	if(Spi != NULL && SpiNum > 0)
	{
		for(i = 0;i < SpiNum;i++)
			delete Spi[i];

		delete[] Spi;
	}

	//
	SpiFullLists = new TStringList();
	SpiNameLists = new TStringList();
	SpiGetLists = new TStringList();
	SpiPLists = new TList();

	// 全部のパスに入っている spi ファイルを取得
	for(i = 0;i < Pathes->Count;i++)
	{
		Temp = IncludeTrailingPathDelimiter(Pathes->Strings[i]) + "*.spi";
		GetFileLists(Temp, SpiFullLists, L"*", -1);
	}

	// ファイル名だけで spi ファイルを取得してコピーしておく
	for(i = 0;i < SpiFullLists->Count;i++)
		SpiNameLists->Add(ExtractFileName(SpiFullLists->Strings[i]));

	// 同じファイル名の spi ファイルは１回しか読まないようにしておく
	for(i = 0;i < SpiFullLists->Count;i++)
	{
		if(SpiGetLists->IndexOf(SpiNameLists->Strings[i]) < 0)
		{
			SpiGetLists->Add(SpiFullLists->Strings[i]);
		}
	}

	// 一つずつ、LoadLibrary してみる
	for(i = 0;i < SpiGetLists->Count;i++)
	{
		Plg = new TSpiPlugin;
		if
		(
			Plg->LoadSpiLL(SpiGetLists->Strings[i]) == true
		&&		(strncmp(Plg->APIVersion, "00IN", 4) == 0
				|| strncmp(Plg->APIVersion, "00IM", 4) == 0
				|| strncmp(Plg->APIVersion, "00AM", 4) == 0)
		 )
			SpiPLists->Add(Plg);
		else
			delete Plg;
	}

	if(SpiPLists->Count > 0)
	{
		Spi = new TSpiPlugin*[SpiPLists->Count];
		for(i = 0;i < SpiPLists->Count;i++)
			Spi[i] = (TSpiPlugin *)SpiPLists->Items[i];
		SpiNum = SpiPLists->Count;
	}

	// 開放して終了
	delete SpiFullLists;
	delete SpiNameLists;
	delete SpiGetLists;
	delete SpiPLists;

	Showing = false;
}

// リフレッシュ
bool TSpiLoader::Refresh(void)
{
	UnicodeString OldFileName = FileName;
	UnicodeString OldSubFilename = SubFileName;

	Clear(MMODE_ALL);
	bool Result;

	if(OldSubFilename == L"")
	{
		Result = SetImageFile(OldFileName);
	}
	else
	{
		Result = SetImageFile(OldFileName) & SetSubImageFile(OldSubFilename);
	}
	return(Result);
}

// 読み込むファイルを設定する
bool TSpiLoader::SetImageFile(UnicodeString Src)
{
	DWORD SizeL, SizeH;

	HANDLE hFile;
	HANDLE hMap;

	// ファイルハンドルの取得
	hFile = CreateFileW(Src.c_str(), GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_WRITE, 0, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, 0);

	bool Result = 0;

	if(hFile != NULL && hFile != INVALID_HANDLE_VALUE)
	{

		BYTE *FileData;

		// ファイルはすでに存在していて、この関数は成功します
		SizeL = GetFileSize(hFile, &SizeH);

		if(SizeH == 0 && SizeL < 1024 * 1024 * 1024)
		{
			// メモリマップドファイルを作成する
			hMap = CreateFileMapping(hFile, 0, PAGE_READWRITE, 0, 0, 0);
			if(hMap != NULL)
			{
				FileData = (BYTE *)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
				PostSpi = NULL;

				if(CheckFileType(Src, FileData, SizeL) == 1)
				{
					// 実際にマップドされたメモリから読み込む
					Result = LoadFromFileInMemory(FileData, SizeL, ProgressCallback);
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

int TSpiLoader::CheckFileType(UnicodeString SrcFileName, BYTE *FileData, DWORD FileSize)
{
	int Result = 0;

	Result = CheckGifAnime(SrcFileName, FileData, FileSize);
	if(SpiPrecede == false)
	{
		if(Result == 0)Result = CheckFileTypeVNR(SrcFileName, FileData);
		if(Result == 0)Result = CheckFileTypeSPI(SrcFileName, FileData);
	}
	else
	{
		if(Result == 0)Result = CheckFileTypeSPI(SrcFileName, FileData);
		if(Result == 0)Result = CheckFileTypeVNR(SrcFileName, FileData);
	}
	return(Result);
}

int TSpiLoader::CheckFileTypeSPI(UnicodeString SrcFileName, BYTE *FileData)
{
// -----------------------------------------------------------------------------
	// まず SPI プラグインで読み込めるかどうか調べる
	try
	{
		PostFileExt = ExtractFileExt(SrcFileName);
		int i;
		// SPI プラグインが必要かどうか調べる。
		for(i = 0;i < SpiNum;i++)
		{
			if((*Spi[i]->IsSupported)(PostFileExt.c_str(), (DWORD)FileData) != 0)
			{
				PostSpi = Spi[i];
				PostFileName = SrcFileName;
				return(1);
			}
		}
	}
	catch(...)
	{
		return(0);
	}

// -----------------------------------------------------------------------------
	return(0);
}

int TSpiLoader::CheckGifAnime(UnicodeString SrcFileName, BYTE *FileData, DWORD FileSize)
{
	if(FileSize < 18)return(1);
	int i, t, AnimFlag = 0;

	// ヘッダのチェック --------------------
	// gif かどうか調べる
	if(strncmp((char *)FileData, "GIF8", 4) != 0)return(0);
	if(*((char *)FileData + 4) != '9' && *((char *)FileData + 4) != '7')return(0);
	if(*((char *)FileData + 5) != 'a')return(0);

	if(FileData[10] & 0x80)// Global Color Table が存在する
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

	while(1)
	{
		if(FileData[i] == 0x2C)
		{
			// Image Block
			i+= 9; if(i >= FileSize)break;

			if(FileData[i] & 0x80) // Local Color Table が存在する
			{
				t = (1 << ((FileData[i] & 0x07) + 1)) * 3;
				i += t;
			}

			i += 2; if(i >= FileSize)break;

			do
			{
				i += FileData[i] + 1;
				if(i >= FileSize)break;
			}
			while(FileData[i] != 0);

			if(AnimFlag == 1)AnimFlag = 2;
		}
		else if(FileData[i] == 0x21)
		{
			i++; if(i >= FileSize)break;

			if(FileData[i] == 0xf9)
			{
				// Graphic Control Extension
				i++; if(i >= FileSize)break;

				if(FileData[i] != 0x04)break;

				i += 5; if(i >= FileSize)break;

				if(FileData[i] != 0x00)break;

				if(AnimFlag == 0)AnimFlag = 1;
			}
			else if(FileData[i] == 0xfe || FileData[i] == 0x01 || FileData[i] == 0xff)
			{
				// Comment Extension, Plain Text Extension & Application Extension
				i++; if(i >= FileSize)break;

				do
				{
					i += FileData[i] + 1;
					if(i >= FileSize)break;
				}
				while(FileData[i] != 0);
			}
		}
		else return(0);

		i++; if(i >= FileSize)break;

		if(FileData[i] == 0x3b)break;
	}

	if(AnimFlag != 2)return(0);

	PostSpi = NULL;
	PostFileName = SrcFileName;
	PostNOSPIType = NOSPI_GIF;
	return(1);
}

int TSpiLoader::CheckFileTypeVNR(UnicodeString SrcFileName, BYTE *FileData)
{
	// 通常のビットマップかどうか調べる
	if(strncmp((char *)FileData, "BM", 2) == 0)
	{
		PostSpi = NULL;
		PostFileName = SrcFileName;
		PostNOSPIType = NOSPI_BMP;
		return(1);
	}

	// jpeg かどうか調べる
	if((int)((BYTE *)FileData[0]) == 0xFF && (int)((BYTE *)FileData[1]) == 0xD8)
	{
		PostSpi = NULL;
		PostFileName = SrcFileName;
		PostNOSPIType = NOSPI_JPG;
		return(1);
	}

	// PNG かどうか調べる
	if(strncmp((char *)FileData + 1, "PNG", 3) == 0)
	{
		PostSpi = NULL;
		PostFileName = SrcFileName;
		PostNOSPIType = NOSPI_PNG;
		return(1);
	}

	// gif かどうか調べる
	if(strncmp((char *)FileData, "GIF", 3) == 0)
	{
		PostSpi = NULL;
		PostFileName = SrcFileName;
		PostNOSPIType = NOSPI_GIF;
		return(1);
	}
	return(0);
}

// 画像ファイルを実際に読み込む
bool TSpiLoader::LoadFromFileInMemory(BYTE *FileData, int DataSize, FARPROC ProgressCallback)
{
	SPictureInfo PInfo;

	// アーカイブモードかどうかチェックする
	// アーカイブモードならファイルの一覧を取得する
	if(PostSpi != NULL && strncmp(PostSpi->APIVersion, "00AM", 4) == 0 && SubFileName == L"")
	{
		int i;
		HANDLE HInfo;
		if((*PostSpi->GetArchiveInfo)(FileData, DataSize, 1, (void **)&HInfo)!= 0)return(false);

		SArchivedFileInfo *pInfo = (SArchivedFileInfo *)(::LocalLock(HInfo));
		ImageNum = 0;

		while(pInfo[ImageNum].Method[0] != '\0')
			ImageNum++;

		if(ImageNum > 0)
		{
			ArchivedFileInfo = new SArchivedFileInfo[ImageNum];
			for(i = 0;i < ImageNum;i++)
				ArchivedFileInfo[i] = pInfo[i];
		}

		::LocalUnlock(HInfo);
		::LocalFree(HInfo);

		Clear(MMODE_PICTURE | MMODE_ARCHIVE);

		old_ArchivedFileInfo = ArchivedFileInfo;

		Mode = MMODE_ARCHIVE;

		ArchiveSpi = PostSpi;
		FileName = PostFileName;

		return(true);
	}

	// SPI モードでない場合、素でファイルを読み込んでみる
	if(PostSpi == NULL)
	{
		TMemoryStream *Stream = new TMemoryStream();
		Stream->Position = 0;
		Stream->Write(FileData, DataSize);
		Stream->Position = 0;

		switch(PostNOSPIType)
		{
			case NOSPI_BMP:
				{
					try
					{
						// ビットマップ画像の場合には普通に読み込む
						BMPImage = new TUCBitmap();
						BMPImage->LoadFromStream(Stream);

						unsigned int InfoSize, ImageSize;
						::GetDIBSizes(BMPImage->Handle, InfoSize, ImageSize);
						pBmpInfo = (TBitmapInfo *)new Byte[sizeof(BITMAPINFO)];
						pBmpData = new Byte[ImageSize];
						GetDIB(BMPImage->Handle, BMPImage->Palette, pBmpInfo, pBmpData);

						OWidth = Width = BMPImage->Width;
						OHeight = Height = BMPImage->Height;
						SAFE_DELETE(BMPImage);
						delete Stream;
					}
					catch(...)
					{
						SAFE_DELETE(BMPImage);
						delete Stream;
						return(false);
					}
				}
				break;
			case NOSPI_JPG:
				{
					try
					{
						JPEGImage = new TUCJPEGImage();
						JPEGImage->LoadFromStream(Stream);

						BMPImage = new TUCBitmap();
						BMPImage->Width = JPEGImage->Width;
						BMPImage->Height = JPEGImage->Height;
						BMPImage->Canvas->Draw(0, 0, JPEGImage);

						unsigned int InfoSize, ImageSize;
						::GetDIBSizes(BMPImage->Handle, InfoSize, ImageSize);
						pBmpInfo = (TBitmapInfo *)new Byte[sizeof(BITMAPINFO)];
						pBmpData = new Byte[ImageSize];
						GetDIB(BMPImage->Handle, BMPImage->Palette, pBmpInfo, pBmpData);

						OWidth = Width = BMPImage->Width;
						OHeight = Height = BMPImage->Height;

						SAFE_DELETE(BMPImage);
						SAFE_DELETE(JPEGImage);

						delete Stream;

						if(FixRotate == true)*Rotate = RotateValue;
						if(*Rotate < 0)*Rotate = GetOrientation(FileData, DataSize);
					}
					catch(...)
					{
						SAFE_DELETE(JPEGImage);
						SAFE_DELETE(BMPImage);
						delete Stream;
						return(false);
					}
				}
				break;
			case NOSPI_PNG:
				{
					try
					{
						PNGImage = new TUCPngImage();
						PNGImage->LoadFromStream(Stream);

						BMPImage = new TUCBitmap();
						BMPImage->Width = PNGImage->Width;
						BMPImage->Height = PNGImage->Height;
						BMPImage->Canvas->Draw(0, 0, PNGImage);

						unsigned int InfoSize, ImageSize;
						::GetDIBSizes(BMPImage->Handle, InfoSize, ImageSize);
						pBmpInfo = (TBitmapInfo *)new Byte[sizeof(BITMAPINFO)];
						pBmpData = new Byte[ImageSize];
						GetDIB(BMPImage->Handle, BMPImage->Palette, pBmpInfo, pBmpData);

						OWidth = Width = BMPImage->Width;
						OHeight = Height = BMPImage->Height;

						SAFE_DELETE(BMPImage);
						SAFE_DELETE(PNGImage);
						delete Stream;
					}
					catch(...)
					{
						SAFE_DELETE(PNGImage);
						SAFE_DELETE(BMPImage);
						delete Stream;
						return(false);
					}
				}
				break;
			case NOSPI_GIF:
				{
					try
					{
						GIFImage = new TUCGIFImage();
						GIFImage->LoadFromStream(Stream);

						BMPImage = new TUCBitmap();
						BMPImage->Width = GIFImage->Width;
						BMPImage->Height = GIFImage->Height;
						BMPImage->Canvas->Draw(0, 0, GIFImage);

						unsigned int InfoSize, ImageSize;
						::GetDIBSizes(BMPImage->Handle, InfoSize, ImageSize);
						pBmpInfo = (TBitmapInfo *)new Byte[sizeof(BITMAPINFO)];
						pBmpData = new Byte[ImageSize];
						GetDIB(BMPImage->Handle, BMPImage->Palette, pBmpInfo, pBmpData);

						OWidth = Width = BMPImage->Width;
						OHeight = Height = BMPImage->Height;

						if(GIFImage->Images->Count > 1)
						{
							GIFImage->Animate = true;
							GIFImage->ResumeDraw();

							// GIFアニメでダブルバッファの場合、前回のダブルバッファを削除
							if(DoubleBuffered == true)
							{
								DeleteDBuffer();
							}
						}
						else
						{
							GIFImage->Animate = false;
							SAFE_DELETE(BMPImage);
							SAFE_DELETE(GIFImage);
						}
						delete Stream;
					}
					catch(...)
					{
						SAFE_DELETE(GIFImage);
						SAFE_DELETE(BMPImage);
						delete Stream;
						return(false);
					}
				}
				break;
		}

		Clear(MMODE_PICTURE);

		FileName = PostFileName;
		PluginName = "Internal Loader";

		Mode &= (MMODE_ALL - MMODE_PICTURE);
		Mode |= MMODE_NOSPI;
	}
	else if(strncmp(PostSpi->APIVersion, "00IN", 4) == 0)
	{
		if(FixRotate == true)*Rotate = RotateValue;
		if(*Rotate < 0)*Rotate = GetOrientation(FileData, DataSize);

		if((*PostSpi->GetPictureInfo)(FileData, DataSize, 1, &PInfo) != 0)return(false);
		GlobalFree(PInfo.Info);			// TEXT info 解放

		if((*PostSpi->GetPicture)(FileData, DataSize, 1, &HBmpInfo, &HBmpData, ProgressCallback, 0) != 0)return(false);

		Clear(MMODE_PICTURE);

		pBmpData = GlobalLock(HBmpData); // Win32 API でロック
		pBmpInfo = (LPBITMAPINFO)GlobalLock(HBmpInfo);

		OWidth = Width = pBmpInfo->bmiHeader.biWidth;
		OHeight = Height = pBmpInfo->bmiHeader.biHeight;

		old_pBmpData = pBmpData;
		old_pBmpInfo = pBmpInfo;
		old_HBmpData = HBmpData;
		old_HBmpInfo = HBmpInfo;

		Mode &= (MMODE_ALL - MMODE_PICTURE);
		Mode |= MMODE_SPI;

		PluginName = (UnicodeString)((AnsiString)((char *)PostSpi->PluginName));
	}
	else return(false);

	NowSpi = PostSpi;
	NOSPIType = PostNOSPIType;

	old_pBmpData = pBmpData;
	old_pBmpInfo = pBmpInfo;
	old_HBmpData = HBmpData;
	old_HBmpInfo = HBmpInfo;

	Showing = true;

	if(*Rotate < 0)
	{
		*Rotate = 0;
	}
	else if(*Rotate != 0)
	{
		CreateRotateBuffer();
	}

	if(NowSpi == NULL && NOSPIType == NOSPI_GIF && GIFImage != NULL && GIFImage->Animate == true)
	{
		old_BMPImage = BMPImage;
		old_GIFImage = GIFImage;
		GIFAnimate = true;
	}
	else
	{
		GIFAnimate = false;
	}

	// ダブルバッファを作る
	if(DoubleBuffered == true && GIFAnimate == false)
	{
		DeleteDBuffer();
		CreateDBuffer();
	}
	else
	{
		DWidth = Width;
		DHeight = Height;
		pBitmapInfo = pBmpInfo;
		pBitmapData = pBmpData;
	}

	NoRotationRefresh = GIFAnimate;

	return(true);
}

void TSpiLoader::AnimateGIF(void)
{
	if(GIFAnimate == false)return;

	BMPImage->Canvas->Draw(0, 0, GIFImage);

	SAFE_DELETE_ARRAY(pBmpInfo);
	SAFE_DELETE_ARRAY(pBmpData);
	unsigned int InfoSize, ImageSize;
	::GetDIBSizes(BMPImage->Handle, InfoSize, ImageSize);
	pBmpInfo = (TBitmapInfo *)new Byte[sizeof(BITMAPINFO)];
	pBmpData = new Byte[ImageSize];
	GetDIB(BMPImage->Handle, BMPImage->Palette, pBmpInfo, pBmpData);

	old_pBmpData = pBmpData;
	old_pBmpInfo = pBmpInfo;

	pBitmapInfo = pBmpInfo;
	pBitmapData = pBmpData;
}

bool TSpiLoader::ChangeArchiveFileName(UnicodeString NewName)
{
	FileName = NewName;
	return(true);
}

bool TSpiLoader::CreateRotateBuffer(void)
{
	if(Showing == false)return(true);

	void *pNewBuf;
	BITMAPINFO *NewInfo = new BITMAPINFO;
	HBITMAP HNewBuf;

	*NewInfo = *pBmpInfo;
	NewInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	NewInfo->bmiHeader.biWidth = Width;
	NewInfo->bmiHeader.biHeight = Height;
	NewInfo->bmiHeader.biPlanes = 1;
	NewInfo->bmiHeader.biBitCount = 32;
	NewInfo->bmiHeader.biCompression = BI_RGB;
	HNewBuf = CreateDIBSection(NULL, NewInfo, DIB_RGB_COLORS, (void **)(&pNewBuf), NULL, 0);

	// テンポラリにコピー
	HDC TempDC = CreateCompatibleDC(NULL);
	SelectObject(TempDC, HNewBuf);
	SetDIBitsToDevice(TempDC, 0, 0, Width, Height, 0, 0, 0, Height, pBmpData, pBmpInfo, DIB_RGB_COLORS);
	DeleteDC(TempDC);

	Clear(MMODE_PICTURE);

	old_HBmpData = HBmpData = HNewBuf;
	old_pBmpData = pBmpData = pNewBuf;
	old_HBmpInfo = HBmpInfo = NULL;
	old_pBmpInfo = pBmpInfo = NewInfo;

	Mode |= MMODE_ROTATE;

	DoOffsetRotate(*Rotate);
	if(GIFAnimate == true)
	{
		SAFE_DELETE(old_BMPImage);
		SAFE_DELETE(old_GIFImage);
		GIFAnimate = false;
	}
	return(true);
}

bool TSpiLoader::DoOffsetRotate(int RotateValue)
{
	if(RotateValue == 0 || Showing == false)return(true);

	void *pNewBuf;
	BITMAPINFO *NewInfo = new BITMAPINFO;
	HBITMAP HNewBuf;

	int p, q, x, y, Size = Width * Height - 1, YOfs, BNum, Mask;
	int *Src, *Dest;

	switch(RotateValue)
	{
		case 1:
		case 3:
			*NewInfo = *pBmpInfo;
			NewInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			NewInfo->bmiHeader.biWidth = Height;
			NewInfo->bmiHeader.biHeight = Width;
			NewInfo->bmiHeader.biPlanes = 1;
			NewInfo->bmiHeader.biBitCount = 32;
			NewInfo->bmiHeader.biCompression = BI_RGB;
			HNewBuf = CreateDIBSection(NULL, NewInfo, DIB_RGB_COLORS, (void **)(&pNewBuf), NULL, 0);

			Dest = (int *)pNewBuf;

			if(RotateValue == 1)
			{
				for(y = 0;y < Height;y++)
				{
					Src = (int *)pBmpData + y * Width;
					for(x = 0;x < Width;x++)
					{
						Dest[y + (Width - x - 1) * Height] = *Src;
						Src++;
					}
				}
			}
			else
			{
				for(y = 0;y < Height;y++)
				{
					Src = (int *)pBmpData + y * Width;
					for(x = 0;x < Width;x++)
					{
						Dest[(Height - y - 1) + x * Height] = *Src;
						Src++;
					}
				}
			}
			Width = NewInfo->bmiHeader.biWidth;
			Height = NewInfo->bmiHeader.biHeight;
			break;

		case 2:
			*NewInfo = *pBmpInfo;
			NewInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			NewInfo->bmiHeader.biWidth = Width;
			NewInfo->bmiHeader.biHeight = Height;
			NewInfo->bmiHeader.biPlanes = 1;
			NewInfo->bmiHeader.biBitCount = 32;
			NewInfo->bmiHeader.biCompression = BI_RGB;
			HNewBuf = CreateDIBSection(NULL, NewInfo, DIB_RGB_COLORS, (void **)(&pNewBuf), NULL, 0);

			Src = (int *)pBmpData;
			Dest = (int *)pNewBuf;
			for(p = 0;p < Size;p++)
				Dest[Size - p] = Src[p];

			break;
	}

	Clear(MMODE_PICTURE);

	old_HBmpData = HBmpData = HNewBuf;
	old_pBmpData = pBmpData = pNewBuf;
	old_HBmpInfo = HBmpInfo = NULL;
	old_pBmpInfo = pBmpInfo = NewInfo;

	Mode |= MMODE_ROTATE;
	return(true);
}

bool TSpiLoader::AbsoluteRotate(int Value)
{
	if(Showing == false || Rotate == NULL)return(true);

	int Offset = Value - *Rotate;

	return(OffsetRotate(Offset));
}

bool TSpiLoader::OffsetRotate(int Value)
{
	if(Showing == false || Rotate == NULL)return(true);

	int Offset = Value;

	while(Offset < 0)Offset += 4;
	while(Offset > 3)Offset -= 4;

	*Rotate = *Rotate + Value;
	while(*Rotate < 0)*Rotate += 4;
	while(*Rotate > 3)*Rotate -= 4;


	if((Mode & MMODE_ROTATE) != 0 && NoRotationRefresh == true && *Rotate == 0)
	{
		return(Refresh());
	}

	if((Mode & MMODE_ROTATE) == 0)
	{
		CreateRotateBuffer();
	}
	else
	{
		DoOffsetRotate(Offset);
	}

	if(DoubleBuffered == true && GIFAnimate == false)
	{
		DeleteDBuffer();
		CreateDBuffer();
	}
	else
	{
		DWidth = Width;
		DHeight = Height;
		pBitmapInfo = pBmpInfo;
		pBitmapData = pBmpData;
	}

	return(true);
}


int TSpiLoader::LoadSubIndex(int &SubIndex, int Ofs)
{
	if((Mode & MMODE_ARCHIVE) == 0)return(0);
	while(SubIndex < 0)SubIndex += ImageNum;
	while(SubIndex >= ImageNum)SubIndex -= ImageNum;

	int OldSubIndex = SubIndex;
	while(1)
	{
		if(SetShowSubIndex(SubIndex, ProgressCallback) == 1)return(1);

		SubIndex += Ofs;
		while(SubIndex < 0)SubIndex += ImageNum;
		while(SubIndex >= ImageNum)SubIndex -= ImageNum;
		if(SubIndex == OldSubIndex)
		{
			SubIndex = -1;
			break;
		}
	}
	return(0);
}


bool TSpiLoader::CreateDBuffer(void)
{
	if(DoubleBuffered == false || Mode == 0)return(false);

	DBufInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	DBufInfo.bmiHeader.biWidth = Width;
	DBufInfo.bmiHeader.biHeight = Height;
	DBufInfo.bmiHeader.biPlanes = 1;
	DBufInfo.bmiHeader.biBitCount = 32;
	DBufInfo.bmiHeader.biCompression = BI_RGB;

	HDBuf = CreateDIBSection(NULL, &DBufInfo, DIB_RGB_COLORS, (void **)(&pDBuf), NULL, 0);
	if(HDBuf == NULL)
	{
		DWidth = 0, DHeight = 0;
		return(false);
	}

	HDBufDC = CreateCompatibleDC(NULL);
	SelectObject(HDBufDC, HDBuf);
	DWidth = 0, DHeight = 0;
	CreateDBImage(Width, Height);

//	CreateReSizeBuffer();
	return(true);
}

int TSpiLoader::CreateDBImage(int SWidth, int SHeight)
{
	if(Mode == 0 || DoubleBuffered == false)return(false);
	if(SWidth > Width || SHeight > Height)
	{
		SWidth = Width;
		SHeight = Height;
	}
	if(SWidth == DWidth || SHeight == DHeight)return(1);

	int OldStretchMode = SetStretchBltMode(HDBufDC, STRETCH_HALFTONE);

	int Result = 0;

	if(SWidth * SHeight < DBFailSize || DBFailSize == 0)
	{
		Result = StretchDIBits(HDBufDC, 0, Height - SHeight, SWidth, SHeight,
												 0, 0, Width,  Height, pBmpData, pBmpInfo, DIB_RGB_COLORS, SRCCOPY);
	}

	if(Result != Height)
	{
		DBFailSize = (__int64)SWidth * SHeight;
		SetDIBitsToDevice(HDBufDC, 0, 0, Width, Height, 0, 0, 0, Height, pBmpData, pBmpInfo, DIB_RGB_COLORS);
		SWidth = Width, SHeight = Height;
	}

	SetStretchBltMode(HDBufDC, OldStretchMode);
	DWidth = SWidth;
	DHeight = SHeight;

	pBitmapInfo = &DBufInfo;
	pBitmapData = pDBuf;
	return(1);
}

bool TSpiLoader::SetDoubleBufferMode(bool aMode)
{
	DoubleBuffered = aMode;
	if(DoubleBuffered == true && GIFAnimate == false)
		CreateDBuffer();
	else
		DeleteDBuffer();
	return(true);
}

bool TSpiLoader::DeleteDBuffer(void)
{
	if(HDBuf == NULL)return(False);

	DeleteDC(HDBufDC);
	DeleteObject(HDBuf);

	HDBufDC = NULL;
	HDBuf = NULL;
	pDBuf = NULL;

	DWidth = Width;
	DHeight = Height;
	pBitmapInfo = pBmpInfo;
	pBitmapData = pBmpData;

//	DeleteReSizeBuffer();
	return(true);
}

bool TSpiLoader::CreateReSizeBuffer(void)
{
	int ScWidth = GetSystemMetrics(SM_CXSCREEN);
	int ScHeight = GetSystemMetrics(SM_CYSCREEN);

	if(Width > ScWidth || Height > ScHeight)
	{
		ScWidth /= 2;
		ScHeight /= 2;
		if((double)Width / ScWidth < (double)Height / ScHeight)
		{
			RBufInfo = DBufInfo;
			RBufInfo.bmiHeader.biWidth = ScWidth;
			RBufInfo.bmiHeader.biHeight = (double)Height * ScWidth / Width;
		}
		else
		{
			RBufInfo = DBufInfo;
			RBufInfo.bmiHeader.biWidth = (double)Width * ScHeight / Height;
			RBufInfo.bmiHeader.biHeight = Height;
		}
		HRBuf = CreateDIBSection(NULL, &RBufInfo, DIB_RGB_COLORS, (void **)(&pRBuf), NULL, 0);

		HDC TempDC = CreateCompatibleDC(NULL);
		SelectObject(TempDC, HRBuf);
		int OldStretchMode = SetStretchBltMode(TempDC, STRETCH_HALFTONE);

		StretchDIBits(TempDC,
									0, Height - RBufInfo.bmiHeader.biHeight, RBufInfo.bmiHeader.biWidth, RBufInfo.bmiHeader.biHeight,
									0, 0, Width,  Height, pBmpData, pBmpInfo, DIB_RGB_COLORS, SRCCOPY);

		SetStretchBltMode(TempDC, OldStretchMode);
		DeleteDC(TempDC);
	}
	else
	{
		HRBuf = NULL;
	}
	return(true);
}

bool TSpiLoader::DeleteReSizeBuffer(void)
{
	if(HRBuf)
	{
		DeleteObject(HRBuf);
		HRBuf = NULL;
	}
	return(true);
}

bool TSpiLoader::StartReSizeMode(void)
{
	if(HRBuf)
	{
		if(DWidth < RBufInfo.bmiHeader.biWidth * 2)
		{
			DWidth = RBufInfo.bmiHeader.biWidth;
			DHeight = RBufInfo.bmiHeader.biHeight;
			pBitmapInfo = &RBufInfo;
			pBitmapData = pRBuf;
		}
	}
	return(true);
}

bool TSpiLoader::EndReSizeMode(void)
{
	if(HRBuf)
	{
		if(HDBuf)
		{
			pBitmapInfo = &DBufInfo;
			pBitmapData = pDBuf;
		}
		else
		{
			pBitmapInfo = pBmpInfo;
			pBitmapData = pBmpData;
		}
	}
	return(true);
}

int TSpiLoader::SetShowSubIndex(int SubIndex, FARPROC ProgressCallback)
{
	if(ArchivedFileInfo[SubIndex].FileSize > 0)
	{
		if((*ArchiveSpi->GetFile)(((AnsiString)FileName).c_str(),
														ArchivedFileInfo[SubIndex].Position,
														(char *)&HFileImage,
														0x0100,
														NULL, NULL
														) != 0)return(-1);

		pFileImage = (BYTE *)(::LocalLock(HFileImage));

		if(CheckFileType(ArchivedFileInfo[SubIndex].FileName, pFileImage, ArchivedFileInfo[SubIndex].FileSize) == true)
		{
			if(LoadFromFileInMemory(pFileImage, ArchivedFileInfo[SubIndex].FileSize, ProgressCallback) == true)
			{
				Clear(MMODE_ACVINNER);
				Mode |= MMODE_ACVINNER;
				old_HFileImage = HFileImage;
				old_pFileImage = pFileImage;
				return(1);
			}
		}
		LocalUnlock(HFileImage);
		LocalFree(HFileImage);
	}

	return(0);
}


bool TSpiLoader::GetArchiveFileLists(TStringList *SubFileLists) // ファイルリストを取得
{
	int i;
	SubFileLists->Clear();
	char cTemp[400];
	for(i = 0;i < ImageNum;i++)
	{
//		if(strlen(ArchivedFileInfo[i].FileName) > 0)
		{
			strcpy(cTemp, ArchivedFileInfo[i].Path);
			strcat(cTemp, ArchivedFileInfo[i].FileName);
			SImageInfo *NewII = new SImageInfo;
			NewII->Timestamp = ArchivedFileInfo[i].Timestamp;
			NewII->FileSize = ArchivedFileInfo[i].FileSize;
			NewII->Rotate = 0;
			SubFileLists->AddObject((UnicodeString)cTemp, (TObject *)NewII);
		}
	}
	return(true);
}

int TSpiLoader::SetSubImageFile(UnicodeString Src)
{
	int i;
	SubFileName = L"";
	AnsiString Temp = (AnsiString)Src;
	char cTemp[400];
	for(i = 0;i < ImageNum;i++)
	{
		strcpy(cTemp, ArchivedFileInfo[i].Path);
		strcat(cTemp, ArchivedFileInfo[i].FileName);
		if(strcmp(Temp.c_str(), cTemp) == 0)break;
	}
	if(i == ImageNum)return(0);
	SubFileName = Src;
	return(SetShowSubIndex(i, ProgressCallback));
}

int TSpiLoader::SetSubImageFile(TStringList *FileLists, int &i, int Ofs)
{
	do
	{
		Rotate = &((SImageInfo *)FileLists->Objects[i])->Rotate;
		if(SetSubImageFile(FileLists->Strings[i]) == true)return(1);

		delete (SImageInfo *)FileLists->Objects[i];
		FileLists->Delete(i);

		if(Ofs < 0)i += Ofs;
		if(i < 0)break;
		if(i >= FileLists->Count)break;
	}
	while(FileLists->Count >= 0);
	return(0);
}

// 画像データをクリアする
void	TSpiLoader::Clear(int DelMode)
{
	switch(Mode & DelMode & MMODE_PICTURE)
	{
		case MMODE_NOSPI:
			SAFE_DELETE(old_BMPImage);
			SAFE_DELETE(old_GIFImage);

			SAFE_DELETE_ARRAY(old_pBmpInfo);
			SAFE_DELETE_ARRAY(old_pBmpData);
			break;
		case MMODE_SPI:
			if(old_HBmpData != NULL)
			{
				GlobalUnlock(old_HBmpData);
				GlobalFree(old_HBmpData);
				old_HBmpData = NULL;
				old_pBmpData = NULL;
			}

			if(old_HBmpInfo != NULL)
			{
				GlobalUnlock(old_HBmpInfo);
				GlobalFree(old_HBmpInfo);
				old_HBmpInfo = NULL;
				old_pBmpInfo = NULL;
			}
			break;
	}

	if(Mode & DelMode & MMODE_ROTATE)
	{
		if(old_HBmpData != NULL)
		{
			DeleteObject(old_HBmpData);
			old_HBmpData = NULL;
			old_pBmpData = NULL;
			delete old_pBmpInfo;
			old_pBmpInfo = NULL;
		}
	}

	if(Mode & DelMode & MMODE_ARCHIVE)
	{
		if(old_ArchivedFileInfo != NULL)
		{
			delete[] old_ArchivedFileInfo;
			old_ArchivedFileInfo = NULL;
		}
		FileName = L"";
		SubFileName = L"";
	}

	if(Mode & DelMode & MMODE_ACVINNER)
	{
		if(old_HFileImage != NULL)
		{
			LocalUnlock(old_HFileImage);
			LocalFree(old_pFileImage);
			old_HFileImage = NULL;
			old_pFileImage = NULL;
		}
		SubFileName = L"";
	}

	if(DelMode & MMODE_CLEARFILE)
	{
		FileName = L"";
		SubFileName = L"";
	}

	Mode &= (MMODE_ALL - DelMode);
}

short int TSpiLoader::GetOrientation(BYTE *pTemp, int Size)
{
	short int Value = GetOrientationExif(pTemp, Size);
	short int Table[] = {-1, 0, -1, 2, 0, -1, 1, -1, 3};
	if(Value < 0 || Value > 8)return(-1);
	return(Table[Value]);
}


short int TSpiLoader::GetOrientationExif(BYTE *pTemp, int Size)
{
	int Int, p;
	short int SInt;
	unsigned short int USInt;
	short int Result = 0;

	int i, PFile, TMode = 0;
	unsigned long Ofs = 0;


	if(Size < 256)return(0);

	if(pTemp[0] != 0xff || pTemp[1] != 0xd8)return(0);
	if(pTemp[2] != 0xff)return(0);

	p = pTemp[4] * 256 + pTemp[5] + 4;

	if(Size < p)return(0);

	if(pTemp[p] != 0xff)return(0);

	if(pTemp[2] == 0xff && pTemp[3] == 0xe1)
	{
		if(strncmp(pTemp + 6, "Exif\0\0", 6) == 0)
			Ofs = 12;
		else
			return(0);
	}
	else
  	return(0);

//---------------------------------------------

	for(i = Ofs;i < Size;i++)
	{
//		SInt = *(short int *)(void *)(pTemp + i + 2);
//		if(pTemp[i] == 0x49 && pTemp[i + 1] == 0x49 && SInt == 0x2a){TMode = 1; break;}
//		if(pTemp[i] == 0x4d && pTemp[i + 1] == 0x4d && SInt == 0x2a00){TMode = 2; break;}
		if(pTemp[i] == 0x49 && pTemp[i + 1] == 0x49){TMode = 1; break;}
		if(pTemp[i] == 0x4d && pTemp[i + 1] == 0x4d){TMode = 2; break;}
	}
	if(TMode == 0)goto setorientationlefttop_break;


//---------------------------------------------

	Int = *(int *)(void *)(pTemp + i + 4);
	Ofs = i;

	Endian(&Int, 4, TMode);
	Ofs += Int;
	if(Size < Ofs)return(0);

//-------------------------------------------------------------------------------------
// Orientation を探す
//-------------------------------------------------------------------------------------
	SInt = *(short int *)(void *)(pTemp + Ofs);
	Ofs += 2;
	Endian(&SInt, 2, TMode);
	i = SInt;

	while(i > 0)
	{
		if(Size < Ofs + 8)return(0);
		USInt = *(short int *)(void *)(pTemp + Ofs);
		Endian(&USInt, 2, TMode);

		switch(USInt)
		{
			case 0x0112:
				Result = *(short int *)(void *)(pTemp + Ofs + 8);
				Endian(&Result, 2, TMode);
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

void TSpiLoader::OpenSpiSettingDialog(HWND hwnd)
{
	if(NowSpi->ConfigurationDlg != NULL)
		(NowSpi->ConfigurationDlg)(hwnd, 1);
	else
  	return;
}

bool TSpiLoader::CopyImageToClipboard(void)
{
	if(pBmpInfo == NULL || pBmpData == NULL)return(false);
	Graphics::TBitmap *Bitmap;
	TClipboard* cb = Clipboard();
	unsigned short MyFormat;
	unsigned int AData;
	HPALETTE APalette;

	Bitmap = new Graphics::TBitmap();
	Bitmap->Width = Width;
	Bitmap->Height = Height;

	// 画像表示
	int OldStretchMode = SetStretchBltMode(Bitmap->Canvas->Handle, STRETCH_HALFTONE);
	try
	{
		StretchDIBits(Bitmap->Canvas->Handle,
								0, 0, Width, Height,
								0, 0, Width, Height,
								pBmpData, pBmpInfo,
								DIB_RGB_COLORS, SRCCOPY);

		Bitmap->SaveToClipboardFormat(MyFormat, AData, APalette);
		cb->SetAsHandle(MyFormat, AData);
	}
	__finally
	{
		SetStretchBltMode(Bitmap->Canvas->Handle, OldStretchMode);
		delete Bitmap;
	}
	return(true);
}

