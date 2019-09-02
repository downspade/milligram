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
	//-------- 雑用 -------------------- 

	// エンディアンを変更する 1:リトルエンディアン 2:ビッグエンディアン
	void Endian(void *Dat, int Size, int Mode);

	// ファイルマスクの再現（かんたんな正規表現）
	bool StrMatch(const TCHAR *text, const TCHAR *pattern);

	// 数値になるように文字列を修正する
	std::wstring CorrectNumericString(std::wstring Src, bool CanMinus, bool CanFloat, std::wstring MinN, std::wstring MaxN);

	// Int をメモリサイズに変換する(k m 等)
	std::wstring GetMetricPrefixString(int src, int keta, std::wstring space);

	// "," 区切りの文字列にする
	std::wstring FormatString(int src);

	// 浮動小数点を keta で指定した桁数表示の文字列にする
	std::wstring FormatString(double src, int keta, bool Comma);

	// そのウィンドウを絶対的に前に出す(StayTop 除く)
	void SetAbsoluteForegroundWindow(HWND hWnd, bool TopMost);

	// 文字列をクリップボードにコピー
	bool CopyStringToClipboard(std::wstring src);

	//-------- ファイル操作 -------------------

	// ファイルの存在確認
	bool FileExists(std::wstring Path);

	// フォルダの存在確認
	bool FolderExists(std::wstring Path);

	// フォルダに存在するファイルを取得する
	bool GetFileNames(std::vector<std::wstring> &file_names, std::wstring folderPath);
	
	// ドロップされたファイル名をメッセージから取得する
	int GetDropFileName(std::vector<std::wstring> &Dest, HDROP hDrop);

	// ファイルマスク一致チェック
	bool FitsMasks(std::wstring fileName, std::wstring fileMask);

	// フォルダ文字列を変換する
	// 数値になるように文字列を修正する
	std::wstring CorrectNumericString(std::wstring Str);

	// あるフォルダに有るファイル一覧を取得する
	bool GetFiles(std::vector<std::wstring> &file_names, std::wstring folderPath, std::wstring mask, bool ShortCut = false);

	// あるフォルダに有るフォルダ一覧を取得する
	bool GetFolders(std::vector<std::wstring> &file_names, std::wstring folderPath, bool ShortCut = false);

	// あるフォルダ以下に有る全てのファイルとフォルダ一覧を取得する (フォルダをたどる)
	bool GetFolderFiles(std::vector<std::wstring> &file_names, std::vector<std::wstring> &folder_names, std::wstring folderPath);

	// ショートカットから元のファイルを得る
	std::wstring GetFileFromLink(std::wstring Lnk);

	// ショートカットファイルを作る
	HRESULT CreateLink(std::wstring OriginalFile, std::wstring ShortCutFile, std::wstring Params);

	// ファイルをゴミ箱に移動する (Explorer 的に)
	bool DeleteFileToRecycle(std::wstring DelFile, bool Confirm);

	// ファイルをゴミ箱に移動する (Explorer 的に)
	bool DeleteFileToRecycle(std::vector<std::wstring> &DelFiles, bool Confirm);

	// フォルダを削除する
	bool DeleteFolder(std::wstring DelFolder);

	// ファイルをクリップボードにコピーする
	void FilesToClipboard(std::vector<std::wstring> &Src, int Flag); // Flag: DROPEFFECT_COPY, DROPEFFECT_MOVE, DROPEFFECT_LINK

	// ファイルをクリップボードから得る
	size_t FilesFromClipboard(std::vector<std::wstring> &Dest);

	// エクスプローラーでファイルを表示して選択
	void SelectFileInExplorer(std::wstring Src);

	// (1) (2) 形式で重複しないファイル名を取得する
	std::wstring GetNonOverwrapFileName(std::wstring Src);

	// ファイルサイズを得る
	size_t GetFileSizeValue(std::wstring src);

	// タイムスタンプを得る
	time_t GetFileCreationTime(std::wstring src);

	// ファイル名に使えない文字を得る -1:なし 0~:そのインデックスの文字
	int CheckFileIrregularChar(std::wstring FileName);

	// フルパスからファイル名の取得
	std::wstring GetFileName(std::wstring src);

	// フルパスからファイル名の拡張子なしで取得
	std::wstring GetFileNameWithoutExt(std::wstring src);
	
	// フルパスから拡張子の取得（. つき）
	std::wstring GetFileExt(std::wstring src);

	// フルパスから拡張子の変更（. 含む）
	std::wstring ChangeFileExt(std::wstring src, std::wstring ext);

	// フルパスからフォルダ名を取得（区切り子なし）
	std::wstring GetFolderName(std::wstring src);

	// 親フォルダの名前を取得する
	std::wstring GetParentFolder(std::wstring src);

	// 短いファイル名を取得する Num = 1 の場合にはひとつ上まで Num = 0 の場合にはそのファイル名
	std::wstring GetMiniPathName(std::wstring src, int Num);

	// 特殊フォルダへのパスを得る
	std::wstring GetSpecialFolderPath(UINT Value);

	//-------- 文字列操作 -------------------

	// ユニコードからマルチバイト文字へ
	std::string UnicodeToMultiByte(const std::wstring& Source, UINT CodePage = CP_ACP, DWORD Flags = 0);

	// マルチバイトからユニコードへ
	std::wstring MultiByteToUnicode(const std::string& Source, UINT CodePage = CP_ACP, DWORD Flags = 0);

	// バイナリファイルを new で確保されたメモリに読み込む
	size_t LoadBinaryFile(std::wstring &FileName, BYTE *&Dest);

	// バイナリファイルを書き込む
	size_t SaveBinaryFile(std::wstring &FileName, BYTE *Src, size_t size);

	// テキストファイルを読み込む
	std::wstring LoadTextFile(std::wstring &FileName);

	// テキストファイルを書き込む
	size_t SaveTextFile(std::wstring &FileName, std::wstring &Data);

	// vector を 区切り記号 Separator で wstring 一つにまとめる
	bool VectorCombineToString(std::wstring &Dest, std::vector<std::wstring> &Src, std::wstring Separator);

	// wstring を vector に区切り記号 Separator で分ける
	bool StringSplitToVector(std::vector<std::wstring> &Dest, std::wstring &Src, std::wstring Separator);

	// コマンドライン引数を解析する Dest[0]:実行ファイル Dest[1]:引数１ 以下続く・・・
	bool ParseCommanLine(std::vector<std::wstring> &Dest, LPCWSTR lpCmdLine);

	// 文字列を得る
	std::wstring GetStringValue(std::map<std::wstring, std::wstring> &Map, std::wstring Src, std::wstring Default);

	// 文字列を数値に変換
	int GetIntegerValue(std::map<std::wstring, std::wstring> &Map, std::wstring Src, int Default, int Min, int Max);
	int GetIntegerValue(std::wstring src);
	int GetIntegerValue(std::wstring src, int Default, int Min, int Max);

	// 文字列を数値に変換
	double GetDoubleValue(std::map<std::wstring, std::wstring> &Map, std::wstring Src, double Default, double Min, double Max);
	double GetDoubleValue(std::wstring src);
	double GetDoubleValue(std::wstring src, double Default, double Min, double Max);

	// 文字列を真偽値に変換
	bool GetBoolValue(std::map<std::wstring, std::wstring> &Map, std::wstring Src, bool Default);
	bool GetBoolValue(std::wstring src);
	bool GetBoolValue(std::wstring src, bool Default);
	std::wstring BoolToString(bool src);

	// 文字列を大文字に変換 (ACII部のみ)
	std::wstring UpperCase(std::wstring src);

	// 文字列を小文字に変換 (ACII部のみ)
	std::wstring LowerCase(std::wstring src);

	// トリミングする
	std::wstring Trim(std::wstring a);

	// ファイルから map::wstring, wstring を作る
	bool LoadMapFromFile(std::map<std::wstring, std::wstring> &Dest, std::wstring &FileName);

	// ファイルを map::wstring, wstring に改行区切りで書き込み
	bool SaveMapToFile(std::map<std::wstring, std::wstring> &Source, std::wstring &FileName);

	// ウィンドウ文字列を得る
	std::wstring GetWindowString(HWND handle);

	//-------- ダイアログクラス -------------------
	// ファイルを開くダイアログ
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

	// ファイルを保存ダイアログ
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

	// カラー選択ダイアログ
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

	// フォルダ選択ダイアログ
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
	// 数値の入ったテキストボックスを実現するためのクラス
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

		HWND handle = nullptr; // ウィンドウハンドル
		WNDPROC OrgProc = nullptr; // 元のオリジナルのイベント処理関数
		WNDPROC EventProc = nullptr; // ユーザー指定のイベント処理関数
		
		HWND EventThrowWindow = nullptr; // イベントを発行するウィンドウ
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

