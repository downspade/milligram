
//---------------------------------------------------------------------------
//
//	  SUSIE �v���O�C���� C++ Builder �ŊȒP�Ɏg�p���邽�߂̃N���X�w�b�_
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
//			SUSIE plug-in ��������擾���邽�߂̍\����
// ============================================================================

struct SPictureInfo
{
	long 	Left, Top;		// �W�J����ʒu
	long 	Width;	   	// �摜�̕�
	long 	Height;	  	//	   ����
	WORD 	X_Density;   	// ��f�̐����������x
	WORD 	Y_Density;   	// ��f�̐����������x
	short 	Color; 			// �P��f��bit��
	HLOCAL 	Info;			// �摜�̃e�L�X�g�w�b�_
};

struct SArchivedFileInfo
{
	unsigned char Method[8];   // ���k�@�̎��
	unsigned long Position;    // �t�@�C����ł̈ʒu
	unsigned long CompSize;    // ���k���ꂽ�T�C�Y
	unsigned long FileSize;    // ���̃t�@�C���T�C�Y
	time_t Timestamp;          // �t�@�C���̍X�V����
	char Path[200];            // ���΃p�X
	char FileName[200];        // �t�@�C���l�[��
	unsigned long CRC;         //CRC
};


// ============================================================================
//			SUSIE plug-in �v���V�[�W���Ǘ��e�[�u��
// ============================================================================
struct	TSpiPlugin
{
	int	Err;

//  �n���h��
	HANDLE			SpiHandle;

// �֐��|�C���^�Q

	// �v���O�C���̏��𓾂�
	int _export pascal (*GetPluginInfo)(int infono, LPSTR buf, int buflen);

	//  �W�J�\�ȃt�@�C���`�������ׂ�
	int _export pascal (*IsSupported)(LPSTR filename, DWORD dw);

	//  �摜�t�@�C���Ɋւ�����𓾂�@
	int _export pascal (*GetPictureInfo)(LPSTR buf, long len, unsigned int flag
										,struct SPictureInfo *lpInfo);
	//  �摜��W�J����
	int _export pascal (*GetPicture)(LPSTR buf, long len, unsigned int flag
											,HANDLE *pHBInfo, HANDLE *pHBm
											,FARPROC lpPrgressCallback, long lData);

	//  �v���r���[�E�J�^���O�\���p�摜�k���W�J���[�e�B�� *
	int _export pascal (*GetPreview)(LPSTR buf, long len, unsigned int flag
										,HANDLE *pHBInfo, HANDLE *pHBm
										,FARPROC lpPrgressCallback, long lData);


	//  �A�[�J�C�u���̂��ׂẴt�@�C���̏����擾����
	int	_export pascal (*GetArchiveInfo)(LPSTR buf, long len
											,unsigned int flag, HLOCAL *lphInf);

	//  �A�[�J�C�u���̂��ׂẴt�@�C���̏����擾����
	int	_export pascal (*GetFileInfo)(LPSTR buf, long len
											,unsigned int flag, HLOCAL *lphInf);

	//  �A�[�J�C�u���̃t�@�C�����擾����
	int	_export pascal (*GetFile)(LPSTR src, long len, LPSTR dest, unsigned int flag
											,FARPROC prgressCallback, long lData);

	// Plug-in�ݒ�_�C�A���O�̕\��
	int _export pascal (*ConfigurationDlg)(HWND parent, int fnc);

// Constructor
	TSpiPlugin(void){SpiHandle = NULL;};

// Destructor
	~TSpiPlugin(void){if(SpiHandle != NULL)FreeLibrary(SpiHandle);};					// ���C�u�������

//  DLL Procedure address set
	bool LoadSpiLL(UnicodeString SpiPath)
	{
		bool Result = false;

		SpiHandle = LoadLibrary(SpiPath.c_str());					// Win32 API

		if(SpiHandle != NULL)
		{
			// === Win32 API �ŃA�h���X�擾 ===
			GetPluginInfo    = (int (__stdcall *)(int infono, LPSTR buf, int buflen))GetProcAddress(SpiHandle, "GetPluginInfo");
			IsSupported	     = (int (__stdcall *)(LPSTR, DWORD))GetProcAddress(SpiHandle, "IsSupported");
			GetPictureInfo   = (int (__stdcall *)(LPSTR, long, unsigned int, struct SPictureInfo *))GetProcAddress(SpiHandle, "GetPictureInfo");
			GetPicture	     = (int (__stdcall *)(LPSTR, long, unsigned int, HANDLE *, HANDLE *, FARPROC, long))GetProcAddress(SpiHandle, "GetPicture");
			GetPreview	     = (int (__stdcall *)(LPSTR, long, unsigned int, HANDLE *, HANDLE *, FARPROC, long))GetProcAddress(SpiHandle, "GetPreview");
			GetArchiveInfo   = (int (__stdcall *)(LPSTR, long,unsigned int, HLOCAL *))GetProcAddress(SpiHandle, "GetArchiveInfo");
			GetFileInfo      = (int (__stdcall *)(LPSTR, long,unsigned int, HLOCAL *))GetProcAddress(SpiHandle, "GetFileInfo");
			GetFile		       = (int (__stdcall *)(LPSTR, long, LPSTR, unsigned int, FARPROC, long))GetProcAddress(SpiHandle, "GetFile");
			ConfigurationDlg = (int (__stdcall *)(HWND, int))GetProcAddress(SpiHandle, "ConfigurationDlg");

			// === �ȈՃG���[�`�F�b�N ===
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
//			SUSIE plug-in �摜�擾�Ǘ��N���X
// ============================================================================
class	TSpiLoader
{
protected:
	bool ErrorFlag;				// �G���[���\���t���O
	UnicodeString FileName;			 // File name
	UnicodeString SubFileName; // Archive ����File Name
	UnicodeString PostFileName;	 // �Ǎ����� File name
	AnsiString PostFileExt; // Susie �ɓn���p�� FileExt

	//
	TSpiPlugin **Spi;				// �v���O�C�����X�g
	TSpiPlugin *NowSpi;			// �Ō�Ɋm�肵���v���O�C��
	TSpiPlugin *PostSpi;    // ���ɓǂނ��������m�肵�� Spi �v���O�C��
	TSpiPlugin *ArchiveSpi; // ���݉𓀂Ɏg���Ă��� Spi �v���O�C��

	// �n���h���f�[�^
	HANDLE HBmpData, HBmpInfo; // ���ݕ\�����̃n���h��
	HANDLE old_HBmpData, old_HBmpInfo; //

	// �|�C���^�f�[�^
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
	void *pBmpData; // �ǂݍ��񂾉摜�̃I���W�i���f�[�^
	BITMAPINFO *pBmpInfo; // ����

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

	int	Err;					// Win32 �G���[�ԍ�
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

	void SetSpiPathes(TStringList *Pathes);	// SPI �̃p�X��ݒ�
	bool SetImageFile(UnicodeString Src);		// file name set & load
	int CheckFileType(UnicodeString FileName, BYTE *FileData, DWORD FileSize); // �t�@�C���̎�ނ𒲂ׂ�
	int CheckFileTypeSPI(UnicodeString SrcFileName, BYTE *FileData); // SPI �œǂݍ��߂邩�ǂ����`�F�b�N
	int CheckFileTypeVNR(UnicodeString SrcFileName, BYTE *FileData); // �f�œǂݍ��߂邩�ǂ����`�F�b�N
	int CheckGifAnime(UnicodeString SrcFileName, BYTE *FileData, DWORD FileSize); // Gif �A�j�����ǂ����`�F�b�N

	void AnimateGIF(void);
	bool ChangeArchiveFileName(UnicodeString NewFileName);

	void Clear(int DelMode);					// �p���\���ް���p��

	int ImageNum;
	int *Rotate;
	int RotateValue;
	bool FixRotate;

	bool NoRotationRefresh;

	int LoadSubIndex(int &SubIndex, int Ofs);

	bool AbsoluteRotate(int Value);
	bool OffsetRotate(int Value);

	bool GetArchiveFileLists(TStringList *SubFileLists); // �t�@�C�����X�g���擾

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
