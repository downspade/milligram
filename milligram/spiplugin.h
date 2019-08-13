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
		long Left;     // �W�J����ʒu
		long Top;
		long Width;     // �摜�̕�
		long Height;        //	   ����
		WORD X_Density;     // ��f�̐����������x
		WORD Y_Density;     // ��f�̐����������x
		short Color;            // �P��f��bit��
		HLOCAL Info;            // �摜�̃e�L�X�g�w�b�_
	};

	struct SArchivedFileInfo
	{
		unsigned char Method[8];   // ���k�@�̎��
		unsigned long Position;    // �t�@�C����ł̈ʒu
		unsigned long CompSize;    // ���k���ꂽ�T�C�Y
		unsigned long FileSize;    // ���̃t�@�C���T�C�Y
		__time32_t Timestamp;          // �t�@�C���̍X�V����
		char Path[200];            // ���΃p�X
		char FileName[200];        // �t�@�C���l�[��
		unsigned long CRC;         //CRC
	};


	// �摜�̏��ƕ\�����@��ۑ����邽�߂̍\����
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

		//  �n���h��
		HMODULE SpiHandle = nullptr;

		// �֐��|�C���^�^��`
		// �v���O�C���̏��𓾂�
		typedef int (PASCAL *mGetPluginInfo)(int infono, LPSTR buf, int buflen);

		//  �W�J�\�ȃt�@�C���`�������ׂ�
		typedef int (PASCAL *mIsSupported)(LPSTR filename, DWORD dw);

		//  �摜�t�@�C���Ɋւ�����𓾂�@
		typedef int (PASCAL *mGetPictureInfo)(LPSTR buf, long len, unsigned int flag
			, struct SPictureInfo *lpInfo);
		//  �摜��W�J����
		typedef int (PASCAL *mGetPicture)(LPSTR buf, long len, unsigned int flag
			, HANDLE *pHBInfo, HANDLE *pHBm
			, FARPROC lpPrgressCallback, long lData);

		//  �v���r���[�E�J�^���O�\���p�摜�k���W�J���[�e�B�� *
		typedef int (PASCAL *mGetPreview)(LPSTR buf, long len, unsigned int flag
			, HANDLE *pHBInfo, HANDLE *pHBm
			, FARPROC lpPrgressCallback, long lData);


		//  �A�[�J�C�u���̂��ׂẴt�@�C���̏����擾����
		typedef int	(PASCAL *mGetArchiveInfo)(LPSTR buf, long len
			, unsigned int flag, HLOCAL *lphInf);

		//  �A�[�J�C�u���̂��ׂẴt�@�C���̏����擾����
		typedef int	(PASCAL *mGetFileInfo)(LPSTR buf, long len
			, unsigned int flag, HLOCAL *lphInf);

		//  �A�[�J�C�u���̃t�@�C�����擾����
		typedef int	(PASCAL *mGetFile)(LPSTR src, long len, LPSTR dest, unsigned int flag
			, FARPROC prgressCallback, long lData);

		// Plug-in�ݒ�_�C�A���O�̕\��
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
		~CSpiPlugin(void) { if (SpiHandle != nullptr)FreeLibrary(SpiHandle); };					// ���C�u�������

		//  DLL Procedure address set
		bool LoadSpiLL(WCHAR *SpiPath)
		{
			memset(this, 0, sizeof(CSpiPlugin));
			bool Result = false;

			SpiHandle = LoadLibrary(SpiPath);					// Win32 API

			if (SpiHandle != nullptr)
			{
				// === Win32 API �ŃA�h���X�擾 ===
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
		bool ErrorFlag = false;             // �G���[���\���t���O
		CImageInfo ImageInfo;          // File name
		CImageInfo SubImageInfo; // Archive ����File Name
		CImageInfo PostImageInfo;  // �Ǎ����� File name
		std::string PostFileExt = ""; // Susie �ɓn���p�� FileExt

		//
		std::vector<CSpiPlugin *> Spi;               // �v���O�C�����X�g
		CSpiPlugin *NowSpi = nullptr;         // �Ō�Ɋm�肵���v���O�C��
		CSpiPlugin *PostSpi = nullptr;    // ���ɓǂނ��ގ����m�肵�� Spi �v���O�C��
		CSpiPlugin *ArchiveSpi = nullptr; // ���݉𓀂Ɏg���Ă��� Spi �v���O�C��

		HBITMAP hBitmap = nullptr; // �ǂݍ��܂ꂽ hBitmap �I���W�i��

		BITMAPINFO *pBmpInfo = nullptr; // �o�b�N�o�b�t�@�̃r�b�g�}�b�v�C���t�H
		HANDLE pBmpData = nullptr; // �o�b�N�o�b�t�@�� �r�b�g�}�b�v�f�[�^�擪�A�h���X
		HANDLE hBmpData = nullptr; // �o�b�N�o�b�t�@�̃n���h��


		// �n���h���f�[�^
		HANDLE HFileImage = nullptr, pre_HFileImage = nullptr; // Susie �v���O�C���������Ă���n���h��
		BYTE *pFileImage = nullptr, *pre_pFileImage = nullptr; // �f�[�^�ւ̃|�C���^

		std::vector<SArchivedFileInfo> nowArchivedFileInfo;
		std::vector<SArchivedFileInfo> oldArchivedFileInfo;

		//
		EWorkFileType LoadingFileType = EWorkFileType_NONE;

		// �|�C���^�f�[�^
		ULONG_PTR GdiplusToken;
		Bitmap *BitmapGDIP = nullptr;
		Image *ImageGDIP = nullptr;

		HGLOBAL hMemoryGDIP = nullptr;
		IStream *StreamGDIP = nullptr;
		BYTE *pMemoryGDIP = nullptr;

		// GIF �A�j���[�V�����֘A
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

		bool Showing = false; // �\������Ă��邩�ǂ���

		int Err = 0;                    // Win32 �G���[�ԍ�
		int OrgWidth = 200, OrgHeight = 200; // ���Ɖ摜�̑傫��
		int ORotWidth = 200, ORotHeight = 200; // �]�����Ɖ摜�̂�����(��]��)

		int BufWidth, BufHeight; // �]�����Ƃ̃t�H�[���̑傫���i��]�O�j
		int SrcRWidth, SrcRHeight;   // �]�����Ƃ̑傫��(��]��)
		int BufMaxWidth, BufMaxHeight; // �o�b�N�o�b�t�@�[�̉摜�̑傫��
		EPluginMode Mode = EPluginMode_NONE;

		int MaxBBufferPixel = 4000 * 4000; // �o�b�N�o�b�t�@�[�̍ő�s�N�Z���T�C�Y 4000 x 4000 �� 64M

		FARPROC ProgressCallback = nullptr;
		HWND windowHandle = nullptr;

	public:
		int ImageNum = 0;
		int Rotate = -1; // ���݂̉�]�l
		bool FixRotate = false; // ��]���Œ肷�邩�ǂ���
		bool NoRotationRefresh = false;

		ENoSPIPictureType NOSPIType = ENoSPIPictureType_NONE;
		ENoSPIPictureType PostNOSPIType = ENoSPIPictureType_NONE;

		std::wstring InternalLoader = TEXT("");

		HANDLE FormHandle = nullptr;

		std::wstring PluginName = TEXT("");
	private:
		time_t ShortestPeriod; // �v�Z�ł���ŏ��Ԋu
		time_t NowTGT;
		time_t PreTGT;
		time_t PausedTGT;

		//---------------------------------------------------------------------------
	public:
		// �R���X�g���N�^
		CSpiLoader(void);
		// �f�X�g���N�^
		~CSpiLoader(void);

		// ������
		void Init(HWND hWndNewOwner, FARPROC aProgressCallBack);

		// ���ׂẴv���O�C�����N���A����
		void ClearAllPlugins(void);

		// SPi �̃p�X��ݒ肷��
		void SetSpiPathes(std::vector<std::wstring> &Paths);

		bool Refresh(void);
		bool SetImageFile(CImageInfo &SrcImageInfo);
		void InitSize(int iWidth, int iHeight);
		bool CheckFileType(CImageInfo &SrcImageInfo, BYTE* FileData, long FileSize);
		bool CheckFileTypeSPI(CImageInfo &SrcImageInfo, BYTE* FileData);

		// GIF �A�j���t�@�C�����ǂ������ׂ�
		bool CheckGIFAnimeEnabled(CImageInfo &SrcImageInfo, BYTE* FileData, long FileSize);
		// GDI+ �Ńt�@�C���^�C�v�𒲂ׂ�
		bool CheckFileTypeGDIP(CImageInfo &SrcImageInfo, BYTE* FileData);
		// ����������t�@�C����ǂݍ���
		bool LoadFromFileInMemory(CImageInfo &SrcImageInfo, BYTE* FileData, UINT DataSize, FARPROC ProgressCallback);

		// �A�[�J�C�u�t�@�C���̖��O��ύX����
		bool ChangeArchiveFileName(std::wstring NewFileName);

		// �C���[�W�I�u�W�F�N�g����r�b�g�}�b�v�I�u�W�F�N�g�����
		Gdiplus::Bitmap* CreateBMPFromImage(Gdiplus::Image *Src);

		// �A�[�J�C�u�t�@�C�����̃C���f�b�N�X�̔ԍ��̃t�@�C����ǂݍ���
		int LoadSubIndex(int &SubIndex, int Ofs);
		// �o�b�N�o�b�t�@�����̉摜�̕������Ŏg�p�\���ǂ������ׂ�
		void CheckBackBuffer(int sWidth, int sHeight);
		// �t�H�[���̃T�C�Y���w�肷��
		bool SetFormSize(int iWidth, int iHeight);
		// �o�b�N�o�b�t�@�Ƀf�[�^��]������
		bool TransBackBuffer(void);

		// ��Βl�ŉ摜����]����
		bool AbsoluteRotate(int Value);
		// ���Βl�ŉ摜����]����
		bool OffsetRotate(int Value);

		// GIF �A�j���[�V�������ꎞ���f����
		bool PauseGIFAnimate(void);
		// GIF �A�j���[�V�������ēx�Đ�����
		bool RestartGIFAnimate(void);
		// GIF �A�j���̃t���[�������ɐi�߂�
		bool GIFAnimateUpDateFrame(bool FrameSkip);
		
		// ��]�����`�F�b�N����
		int CheckOrientation(void);
		// ��]�����`�F�b�Ns��
		short CheckOrientation(BYTE* pTemp, unsigned int Size);
		// ��]���� Exif ����Ђ낤
		short CheckOrientationExif(BYTE* pTemp, unsigned int Size);
		// GIF �A�j���f�[�^���`�F�b�N����
		bool GetGIFAnimeData(void);
		
		// �A�[�J�C�u�t�@�C���̃C���f�b�N�X�̔ԍ��̃t�@�C����ǂݍ���
		bool SetShowSubIndex(CImageInfo &SrcImageInfo, int SubIndex, FARPROC ProgressCallback);
		// �A�[�J�C�u�t�@�C���̈ꗗ�𓾂�
		bool GetArchiveFileList(std::vector<CImageInfo> &SubFileList); // �t�@�C�����X�g���擾

		// �A�[�J�C�u�t�@�C�����̉摜��ǂݍ���
		bool SetSubImageFile(CImageInfo &Src);
		// �A�[�J�C�u�t�@�C�����̉摜��ǂݍ���
		bool SetSubImageFile(std::vector<CImageInfo> &FileList, int &i, int Ofs);

		// �f�[�^���N���A����
		void Clear(EPluginMode DelMode);

		// �o�b�N�o�b�t�@�[����������
		void DeleteBackBuffer(void);
		// �v���O�C���_�C�A���O��\������
		void OpenSpiSettingDialog(HWND hwnd);

		// �r�b�g�}�b�v�Ƀf�[�^���R�s�[����
		Gdiplus::Bitmap* DuplicateImage(int newWidth, int newHeight);
		// �N���b�v�{�[�h�ɉ摜���R�s�[����
		bool CopyImageToClipboard(void);
		// JPEG �t�@�C���ŕۑ�����
		bool SaveJpeg(std::wstring svFileName, int svWidth, int svHeight, int svCompLevel);
		// PNG �t�@�C���ŕۑ�����
		bool SavePNG(std::wstring svFileName, int svWidth, int svHeight);


};


}


