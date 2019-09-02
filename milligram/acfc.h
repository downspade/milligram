#pragma once

#include <vector>
#include <string>
#include <comdef.h>
#include <gdiplus.h>
#include <filesystem>
#include <iostream>
#include <shobjidl.h>
#include <shlguid.h>
#include <time.h>
#include <mmsystem.h>
#include <ShellAPI.h>
#include <map>
#include <windows.h>
#include <Commdlg.h>
#pragma comment( lib, "Shell32.lib" )

namespace acfc
{
	//-------- �G�p -------------------- 

	// �G���f�B�A����ύX���� 1:���g���G���f�B�A�� 2:�r�b�O�G���f�B�A��
	void Endian(void *Dat, int Size, int Mode);

	// �t�@�C���}�X�N�̍Č��i���񂽂�Ȑ��K�\���j
	bool StrMatch(const TCHAR *text, const TCHAR *pattern);

	// ���l�ɂȂ�悤�ɕ�������C������
	std::wstring CorrectNumericString(std::wstring Src, bool CanMinus, bool CanFloat, std::wstring MinN, std::wstring MaxN);

	// Int ���������T�C�Y�ɕϊ�����(k m ��)
	std::wstring GetMetricPrefixString(int src, int keta, std::wstring space);

	// "," ��؂�̕�����ɂ���
	std::wstring FormatString(int src);

	// ���������_�� keta �Ŏw�肵�������\���̕�����ɂ���
	std::wstring FormatString(double src, int keta, bool Comma);

	// ���̃E�B���h�E���ΓI�ɑO�ɏo��(StayTop ����)
	void SetAbsoluteForegroundWindow(HWND hWnd, bool TopMost);

	// ��������N���b�v�{�[�h�ɃR�s�[
	bool CopyStringToClipboard(std::wstring src);

	//-------- �t�@�C������ -------------------

	// �t�@�C���̑��݊m�F
	bool FileExists(std::wstring Path);

	// �t�H���_�̑��݊m�F
	bool FolderExists(std::wstring Path);

	// �t�H���_�ɑ��݂���t�@�C�����擾����
	bool GetFileNames(std::vector<std::wstring> &file_names, std::wstring folderPath);
	
	// �h���b�v���ꂽ�t�@�C���������b�Z�[�W����擾����
	int GetDropFileName(std::vector<std::wstring> &Dest, HDROP hDrop);

	// �t�@�C���}�X�N��v�`�F�b�N
	bool FitsMasks(std::wstring fileName, std::wstring fileMask);

	// �t�H���_�������ϊ�����
	// ���l�ɂȂ�悤�ɕ�������C������
	std::wstring CorrectNumericString(std::wstring Str);

	// ����t�H���_�ɗL��t�@�C���ꗗ���擾����
	bool GetFiles(std::vector<std::wstring> &file_names, std::wstring folderPath, std::wstring mask, bool ShortCut = false);

	// ����t�H���_�ɗL��t�H���_�ꗗ���擾����
	bool GetFolders(std::vector<std::wstring> &file_names, std::wstring folderPath, bool ShortCut = false);

	// ����t�H���_�ȉ��ɗL��S�Ẵt�@�C���ƃt�H���_�ꗗ���擾���� (�t�H���_�����ǂ�)
	bool GetFolderFiles(std::vector<std::wstring> &file_names, std::vector<std::wstring> &folder_names, std::wstring folderPath);

	// �V���[�g�J�b�g���猳�̃t�@�C���𓾂�
	std::wstring GetFileFromLink(std::wstring Lnk);

	// �V���[�g�J�b�g�t�@�C�������
	HRESULT CreateLink(std::wstring OriginalFile, std::wstring ShortCutFile, std::wstring Params);

	// �t�@�C�����S�~���Ɉړ����� (Explorer �I��)
	bool DeleteFileToRecycle(std::wstring DelFile, bool Confirm);

	// �t�@�C�����S�~���Ɉړ����� (Explorer �I��)
	bool DeleteFileToRecycle(std::vector<std::wstring> &DelFiles, bool Confirm);

	// �t�H���_���폜����
	bool DeleteFolder(std::wstring DelFolder);

	// �t�@�C�����N���b�v�{�[�h�ɃR�s�[����
	void FilesToClipboard(std::vector<std::wstring> &Src, int Flag); // Flag: DROPEFFECT_COPY, DROPEFFECT_MOVE, DROPEFFECT_LINK

	// �t�@�C�����N���b�v�{�[�h���瓾��
	size_t FilesFromClipboard(std::vector<std::wstring> &Dest);

	// �G�N�X�v���[���[�Ńt�@�C����\�����đI��
	void SelectFileInExplorer(std::wstring Src);

	// (1) (2) �`���ŏd�����Ȃ��t�@�C�������擾����
	std::wstring GetNonOverwrapFileName(std::wstring Src);

	// �t�@�C���T�C�Y�𓾂�
	size_t GetFileSizeValue(std::wstring src);

	// �^�C���X�^���v�𓾂�
	time_t GetFileCreationTime(std::wstring src);

	// �t�@�C�����Ɏg���Ȃ������𓾂� -1:�Ȃ� 0~:���̃C���f�b�N�X�̕���
	int CheckFileIrregularChar(std::wstring FileName);

	// �t���p�X����t�@�C�����̎擾
	std::wstring GetFileName(std::wstring src);

	// �t���p�X����t�@�C�����̊g���q�Ȃ��Ŏ擾
	std::wstring GetFileNameWithoutExt(std::wstring src);
	
	// �t���p�X����g���q�̎擾�i. ���j
	std::wstring GetFileExt(std::wstring src);

	// �t���p�X����g���q�̕ύX�i. �܂ށj
	std::wstring ChangeFileExt(std::wstring src, std::wstring ext);

	// �t���p�X����t�H���_�����擾�i��؂�q�Ȃ��j
	std::wstring GetFolderName(std::wstring src);

	// �e�t�H���_�̖��O���擾����
	std::wstring GetParentFolder(std::wstring src);

	// �Z���t�@�C�������擾���� Num = 1 �̏ꍇ�ɂ͂ЂƂ�܂� Num = 0 �̏ꍇ�ɂ͂��̃t�@�C����
	std::wstring GetMiniPathName(std::wstring src, int Num);

	// ����t�H���_�ւ̃p�X�𓾂�
	std::wstring GetSpecialFolderPath(UINT Value);

	//-------- �����񑀍� -------------------

	// ���j�R�[�h����}���`�o�C�g������
	std::string UnicodeToMultiByte(const std::wstring& Source, UINT CodePage = CP_ACP, DWORD Flags = 0);

	// �}���`�o�C�g���烆�j�R�[�h��
	std::wstring MultiByteToUnicode(const std::string& Source, UINT CodePage = CP_ACP, DWORD Flags = 0);

	// �o�C�i���t�@�C���� new �Ŋm�ۂ��ꂽ�������ɓǂݍ���
	size_t LoadBinaryFile(std::wstring &FileName, BYTE *&Dest);

	// �o�C�i���t�@�C������������
	size_t SaveBinaryFile(std::wstring &FileName, BYTE *Src, size_t size);

	// �e�L�X�g�t�@�C����ǂݍ���
	std::wstring LoadTextFile(std::wstring &FileName);

	// �e�L�X�g�t�@�C������������
	size_t SaveTextFile(std::wstring &FileName, std::wstring &Data);

	// vector �� ��؂�L�� Separator �� wstring ��ɂ܂Ƃ߂�
	bool VectorCombineToString(std::wstring &Dest, std::vector<std::wstring> &Src, std::wstring Separator);

	// wstring �� vector �ɋ�؂�L�� Separator �ŕ�����
	bool StringSplitToVector(std::vector<std::wstring> &Dest, std::wstring &Src, std::wstring Separator);

	// �R�}���h���C����������͂��� Dest[0]:���s�t�@�C�� Dest[1]:�����P �ȉ������E�E�E
	bool ParseCommanLine(std::vector<std::wstring> &Dest, LPCWSTR lpCmdLine);

	// ������𓾂�
	std::wstring GetStringValue(std::map<std::wstring, std::wstring> &Map, std::wstring Src, std::wstring Default);

	// ������𐔒l�ɕϊ�
	int GetIntegerValue(std::map<std::wstring, std::wstring> &Map, std::wstring Src, int Default, int Min, int Max);
	int GetIntegerValue(std::wstring src);
	int GetIntegerValue(std::wstring src, int Default, int Min, int Max);

	// ������𐔒l�ɕϊ�
	double GetDoubleValue(std::map<std::wstring, std::wstring> &Map, std::wstring Src, double Default, double Min, double Max);
	double GetDoubleValue(std::wstring src);
	double GetDoubleValue(std::wstring src, double Default, double Min, double Max);

	// �������^�U�l�ɕϊ�
	bool GetBoolValue(std::map<std::wstring, std::wstring> &Map, std::wstring Src, bool Default);
	bool GetBoolValue(std::wstring src);
	bool GetBoolValue(std::wstring src, bool Default);
	std::wstring BoolToString(bool src);

	// �������啶���ɕϊ� (ACII���̂�)
	std::wstring UpperCase(std::wstring src);

	// ��������������ɕϊ� (ACII���̂�)
	std::wstring LowerCase(std::wstring src);

	// �g���~���O����
	std::wstring Trim(std::wstring a);

	// �t�@�C������ map::wstring, wstring �����
	bool LoadMapFromFile(std::map<std::wstring, std::wstring> &Dest, std::wstring &FileName);

	// �t�@�C���� map::wstring, wstring �ɉ��s��؂�ŏ�������
	bool SaveMapToFile(std::map<std::wstring, std::wstring> &Source, std::wstring &FileName);

	// �E�B���h�E������𓾂�
	std::wstring GetWindowString(HWND handle);

	//-------- �_�C�A���O�N���X -------------------
	// �t�@�C�����J���_�C�A���O
	class COpenFileDialog
	{
	public:
		TCHAR szFile[MAX_PATH * 256] = {};
		OPENFILENAME ofn = {};
		std::wstring InitialDirectory = TEXT("");
		std::wstring Filter = TEXT("");
		std::wstring Title = TEXT("");
		std::vector<std::wstring>FileNames;
		std::wstring FileName = TEXT("");
		bool MultiSelect = false;

		COpenFileDialog(void);
		bool ShowDialog(HWND hwnd);
		bool GetFiles(std::vector<std::wstring>& Dest);
	};

	// �t�@�C����ۑ��_�C�A���O
	class CSaveFileDialog
	{
	public:
		TCHAR szFile[MAX_PATH] = {};
		OPENFILENAME ofn = {};
		std::wstring InitialDirectory = TEXT("");
		std::wstring Filter = TEXT("");
		std::wstring DefaultExt = TEXT("");
		std::wstring Title = TEXT("");
		std::wstring FileName = TEXT("");
		bool Overwrite = false;

		CSaveFileDialog(void);
		bool ShowDialog(HWND hwnd);
		bool GetFile(std::wstring& Dest);
	};

	// �J���[�I���_�C�A���O
	class CSelectColorDialog
	{
	private:
		CHOOSECOLOR cs = {};
		COLORREF ColorRef = 0;
		COLORREF CustColors[16] = {};
		Gdiplus::Color GdipColor;
	public:
		CSelectColorDialog(void);
		bool ShowDialog(HWND hwnd);
		void SetColorGdip(Gdiplus::Color Src);
		void SetColorRef(COLORREF Src);
		Gdiplus::Color GetColorGdip(void);
		COLORREF GetColorRef(void);
		void SetCustomColor(COLORREF *ColorRef);
	};

	// �t�H���_�I���_�C�A���O
	class CFolderSelectDialog
	{
	public:
		std::wstring InitialDirectory = TEXT("");
		std::wstring Title = TEXT("");
		std::wstring FolderName = TEXT("");

		CFolderSelectDialog(void);
		bool ShowDialog(HWND hwnd);
		DWORD Options;
	};

	//-----------------------------------------------------
	// ���l�̓������e�L�X�g�{�b�N�X���������邽�߂̃N���X
	class CNumberStr
	{
	private:
		std::wstring SValue;

		int IMin = 0;
		int IMax = 0;
		int IValue = 0;
		
		double DMin = 0.0;
		double DMax = 0.0;
		double DValue = 0.0;

		bool MinusEnabled = false;
		bool DoubleMode = false;
		bool MinEnabled = false;
		bool MaxEnabled = false;

	public:
		CNumberStr(void);
		~CNumberStr(void);

		void SetIntegerMode(int SrcValue, bool aMinusEnabled, bool aMinEnabled, bool aMaxEnabled, int aIMin, int aIMax);
		void SetDoubleMode(double SrcValue, bool aMinusEnabled, bool aMinEnabled, bool aMaxEnabled, double aDMin, double aDMax);

		std::wstring CheckString(std::wstring src);
		std::wstring CheckString(std::wstring src, size_t &pos);

		int SetIntegerValue(int src);
		double SetDoubleValue(double src);
		int GetIntegerValue(void);
		double GetDoubleValue(void);

	};

	//-------------------------------------------------------
	// Window
	class CBaseWindow
	{
	public:
		CBaseWindow(void);
		~CBaseWindow(void);
		void Show(void);
		void Hide(void);
		void Focus(void);
		void SetPositionAndSize(int ax, int ay, int aWidth, int aHeight);
		void SetWindowSize(int aWidth, int aHeight);
		void SetWindowPosition(int ax, int ay);
		bool SetHWND(HWND hWnd);
		void GetWindowStatus(void);
		int x = 0, y = 0;
		int Width = 100, Height = 100;
		bool Visible = false;

		HWND handle = nullptr; // �E�B���h�E�n���h��
		WNDPROC OrgProc = nullptr; // ���̃I���W�i���̃C�x���g�����֐�
		WNDPROC EventProc = nullptr; // ���[�U�[�w��̃C�x���g�����֐�
		
		HWND EventThrowWindow = nullptr; // �C�x���g�𔭍s����E�B���h�E
	private:
		bool HWNDset = false;
	};
	
	//-------------------------------------------------------
	// ListBox
	class CListBox : public CBaseWindow
	{
	public:
		class CItem
		{
		public:
			std::wstring Text;
			bool Selected;
			CItem(std::wstring Src, bool bTemp)
			{
				Text = Src;
				Selected = bTemp;
			}
		};

		CListBox(void);
		~CListBox(void);

		void Init(HWND hwnd, LPCREATESTRUCT lp, LONG_PTR EventProcedure, bool CallProcedure);
		bool SetHWND(HWND hWnd);
		LRESULT ProcessMessages(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		void SetText(std::wstring Src, int Index);
		void Insert(std::wstring Src, int Index);
		void Insert(std::wstring Src, bool Selected, int Index);
		void Insert(CItem Src, int Index);
		void Delete(int Index);
		void Clear(void);
		bool IsSelected(int i);
		void SetSelected(int i, bool sel);
		void SelectAll(bool sel);
		void SelectInvert(void);

		std::vector<CItem> Items;
		int SelectedIndex;
		int SelectedCount;

		void Get(void);
		void GetText(void);
		int GetSelectedIndex(void);
		void GetSelectList(void);

		void Set(void);
		void SetText(void);
		void SetSelectedIndex(void);
		void SetSelectList(void);

		void BeginUpdate(void);
		void EndUpdate(void);
	};

	//-------------------------------------------------------
	// EditBox
	class CEditBox : public CBaseWindow
	{
	public:
		CEditBox(void);
		~CEditBox(void);
		bool SetHWND(HWND hWnd);
		LRESULT ProcessMessages(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		std::wstring GetEditString(void);
	};

	//-------------------------------------------------------
	// NumberEditBox
	class CNumberEdit : public CBaseWindow, public CNumberStr
	{
	public:
		CNumberEdit(void);
		~CNumberEdit(void);
		bool SetHWND(HWND hWnd);
		bool SuspendCorrect = true;
		LRESULT ProcessMessages(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		void CorrectNumericString(void);
		std::wstring GetEditString(void);

		void SetIntegerValue(int Src);
		void SetDoubleValue(double Src);
	};


	//-------------------------------------------------------
	// Slider
	class CSlider : public CBaseWindow
	{
	public:
		CSlider(void);
		~CSlider(void);
		bool SetHWND(HWND hWnd);
		void Init(int aMin, int aMax, int aStep, int aValue);
		void SetSliderValue(int src);
		LRESULT ProcessMessages(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

		int Max = 0;
		int Min = 100;
		int Step = 10;
		int Value = 50;
	};

	//-------------------------------------------------------
	// Timer
	static WPARAM TimerIndex = 1;
	class CTimer
	{
	public:
		CTimer(void);
		~CTimer(void);
		void Enabled(bool aEnable);
		bool IsThis(WPARAM i);
		void SetInterval(int aInterval);
		void Init(HWND hWnd, int aInterval);
		HWND handle = nullptr;
	private:
		WPARAM Index;
		bool Started;
		int Interval;
	};
}

