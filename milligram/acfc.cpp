#include "stdafx.h"
#include "acfc.h"

namespace acfc
{
	void Endian(void *Dat, int Size, int Mode)
	{
		BYTE BDat[4];
		int i;
		if (Mode == 1)return;
		if (Mode == 2)
		{
			for (i = 0; i < Size; i++)
				BDat[i] = ((BYTE *)Dat)[Size - i - 1];
			for (i = 0; i < Size; i++)
				((BYTE *)Dat)[i] = BDat[i];
		}
	}

	bool StrMatch(const TCHAR *text, const TCHAR *pattern)
	{
		switch (*pattern)
		{
		case '\0':
			return (!*text);
		case '*':
			return (StrMatch(text, pattern + 1) || *text && StrMatch(text + 1, pattern));
		case '?':
			return (*text && StrMatch(text + 1, pattern + 1));
		default:
			return ((*text == *pattern) && StrMatch(text + 1, pattern + 1));
		}
	}

	std::wstring CorrectNumericString(std::wstring src)
	{
		std::wstring rStr = L"";
		size_t i, L = src.length();
		for (i = 0; i < L; i++)
		{
			if (src[i] >= L'0'
				&& src[i] <= L'9')
			{
				rStr = rStr + src[i];
			}
		}
		return(rStr);
	}

	std::wstring CorrectNumericString(std::wstring src, bool CanMinus, bool CanFloat, std::wstring MinN, std::wstring MaxN)
	{
		std::wstring Result = L"";
		bool UseMinus, UseFloat;
		size_t i;
		size_t Length, NumberLength;

		UseMinus = false;
		UseFloat = false;
		NumberLength = 0;
		Length = src.length();

		for (i = 0; i < Length; i++)
		{
			while (1)
			{
				if (CanMinus == true && src[i] == L'-' && UseMinus == false && Result == L"")
				{
					UseMinus = true;
					break;
				}

				if (CanFloat == true && src[i] == L'.' && UseFloat == false)
				{
					UseFloat = true;
					Result += L".";
					break;
				}

				if (src[i] >= L'0' && src[i] <= L'9' && NumberLength < 9)
				{
					Result += src[i];
					NumberLength++;
					break;
				}
				break;
			}
		}

		if (UseMinus == true && Result.length() == 0)
		{
			Result = L"-";
			return(Result);
		}

		if (Result.length() > 0)
		{
			if (CanFloat == true)
			{
				double MaxF, MinF, Num, NewNum;
				Num = wcstod(Result.c_str(), nullptr);
				if (UseMinus == true)Num = -Num;

				NewNum = Num;
				MaxF = wcstod(MaxN.c_str(), nullptr);
				MinF = wcstod(MinN.c_str(), nullptr);

				if (Num < MinF)NewNum = MinF;
				if (Num > MaxF)NewNum = MaxF;

				if (NewNum != Num)
				{
					Num = NewNum;
					Result = std::to_wstring(Num);
				}
				else if (UseMinus == true)Result = L"-" + Result;
			}
			else
			{
				__int64 MaxI, MinI, Num, NewNum;
				Num = wcstol(Result.c_str(), nullptr, 10);
				if (UseMinus == true)Num = -Num;

				NewNum = Num;
				MaxI = wcstol(MaxN.c_str(), nullptr, 10);
				MinI = wcstol(MinN.c_str(), nullptr, 10);

				if (Num < MinI)NewNum = MinI;
				if (Num > MaxI)NewNum = MaxI;

				if (NewNum != Num)
				{
					Result = std::to_wstring(Num);
				}
				else if (UseMinus == true)Result = L"-" + Result;
			}
		}
		return(Result);
	}

	static int ShishaGonyu(int src, int keta)
	{
		int s = src, k = keta, p;
		if (keta < 0) return (src);
		while (k > 1)
		{
			s /= 10;
			k--;
		}
		p = s % 10;
		if (p >= 5) s += 10;
		s /= 10;
		k = 0;
		while (k < keta)
		{
			s *= 10;
			k++;
		}
		return (s);
	}




	std::wstring GetMetricPrefixString(int src, int keta, std::wstring space)
	{
		std::wstring kstr[] = { L"", L"K", L"M", L"G", L"T", L"P", L"E", L"Z", L"Y" };
		std::wstring res = L"";

		int v = src;
		int k = 0;
		int ss = keta;
		int j;
		while (v > 0)
		{
			v = v / 10;
			k++;
			ss--;
		}

		src = ShishaGonyu(src, k - keta);

		j = k % 3;
		k = (k - 1) / 3;


		res = std::to_wstring(src);
		res = res.substr(0, keta) + space + kstr[k];
		if (j != 0) res = res.insert(j, L".");
		return (res);
	}

	std::wstring FormatString(int src)
	{
		std::wstring result = TEXT("");
		int k = 0;
		if (src == 0)
		{
			result = TEXT("0");
			return(result);
		}
		
		while (src > 0)
		{
			result = std::to_wstring(TEXT('0') + (src % 10)) + result;
			src /= 10;
			k++;
			if ((k % 3) == 0 && src >= 10)
			{
				result = TEXT(",") + result;
			}
		}
		return(result);
	}

	std::wstring FormatString(double src, int keta, bool Comma)
	{
		std::wstring result = TEXT("");
		std::wstring ustr = TEXT("");
		std::wstring dstr = TEXT("");
		size_t u, d;
		size_t k = 0;
		if (src == 0.0)
		{
			result = TEXT("0");
			return(result);
		}

		u = (int)src;
		src = src - (double)u;
		while (keta > 0)
		{
			src *= 10;
			keta--;
		}
		if ((((int)(src * 10)) % 10) > 4)
			src += 1;

		d = (int)src;

		if (u == 0)
		{
			ustr = TEXT("0");
		}
		else
		{
			while (u > 0)
			{
				ustr = std::to_wstring(u % 10) + ustr;
				u /= 10;
				k++;
				if (Comma && (k % 3) == 0 && u >= 10)
				{
					ustr = TEXT(",") + ustr;
				}
			}
		}
		
		k = 0;
		if (d == 0)
		{
			result = ustr;
		}
		else
		{
			while (d > 0)
			{
				dstr = std::to_wstring(TEXT('0') + (d % 10)) + dstr;
				d /= 10;
				k++;
				if (Comma && (k % 3) == 0 && d >= 10)
				{
					result = TEXT(",") + result;
				}
			}

			k = dstr.length() - 1;
			while (dstr[k] == TEXT('0'))
			{
				k--;
			}
			result = ustr + TEXT(".") + dstr.substr(k, dstr.length() - k);
		}

		return(result);
	}

	void SetAbsoluteForegroundWindow(HWND hWnd, bool TopMost)
	{
		int nTargetID, nForegroundID;
		DWORD sp_time;

		// フォアグラウンドウィンドウを作成したスレッドのIDを取得
		nForegroundID = GetWindowThreadProcessId(GetForegroundWindow(), NULL);
		// 目的のウィンドウを作成したスレッドのIDを取得
		nTargetID = GetWindowThreadProcessId(hWnd, NULL);

		// スレッドのインプット状態を結び付ける
		AttachThreadInput(nTargetID, nForegroundID, TRUE);  // TRUE で結び付け

		// 現在の設定を sp_time に保存
		SystemParametersInfo(SPI_GETFOREGROUNDLOCKTIMEOUT, 0, &sp_time, 0);
		// ウィンドウの切り替え時間を 0ms にする
		SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, NULL, 0);

		// ウィンドウをフォアグラウンドに持ってくる
		SetForegroundWindow(hWnd);

		SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, (SWP_NOMOVE | SWP_NOSIZE));

		if (TopMost == false)
			SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, (SWP_NOMOVE | SWP_NOSIZE));

		// 設定を元に戻す
		SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, &sp_time, 0);

		// スレッドのインプット状態を切り離す
		AttachThreadInput(nTargetID, nForegroundID, FALSE);  // FALSE で切り離し
	}

	bool CopyStringToClipboard(std::wstring src)
	{
		HGLOBAL hg;
		PTSTR	strMem;

		if (!OpenClipboard(0)) return false;
		EmptyClipboard();

		hg = GlobalAlloc(GHND | GMEM_SHARE, sizeof(TCHAR) * (src.length() + 1));
		strMem = (PTSTR)GlobalLock(hg);
		lstrcpy(strMem, src.c_str());
		GlobalUnlock(hg);

		SetClipboardData(CF_UNICODETEXT, hg);

		CloseClipboard();
		return(true);
	}

	
	bool FileExists(std::wstring Path)
	{
		return(PathFileExists(Path.c_str()) && !PathIsDirectory(Path.c_str()));
	}

	bool FolderExists(std::wstring Path)
	{
		return(PathFileExists(Path.c_str()) && PathIsDirectory(Path.c_str()));
	}


	bool GetFileNames(std::vector<std::wstring> &file_names, std::wstring folderPath)
	{
#ifdef _WIN64
		using namespace std::experimental::filesystem;
#else
		using namespace std::filesystem;
#endif
		directory_iterator itr(folderPath), end;
		std::error_code err;

		for (; itr != end && !err; itr.increment(err)) {
			const directory_entry entry = *itr;

			file_names.push_back(entry.path().wstring());
		}

		/* エラー処理 */
		if (err) {
			return false;
		}
		return true;
	}

	int GetDropFileName(std::vector<std::wstring> &Dest, HDROP hDrop)
	{
#ifdef _WIN64
		using namespace std::experimental::filesystem;
#else
		using namespace std::filesystem;
#endif
		std::wstring fileName = TEXT("");
		int nFiles = DragQueryFile(hDrop, -1, NULL, 0);
		TCHAR szBuf[MAX_PATH];

		if (nFiles > 0)
		{
			for (int i = 0; i < nFiles; i++)
			{
				DragQueryFile(hDrop, i, szBuf, sizeof(szBuf) / sizeof(TCHAR));
				fileName = szBuf;
				size_t Len = fileName.length();
				if (Len > 5)
				{
					if (fileName.substr(Len - 3, 4) == TEXT(".lnk"))
					{
						path lnk(fileName);
						if (is_symlink(lnk))
						{
							fileName = read_symlink(lnk);
						}
					}
				}
				Dest.push_back(fileName);
			}
			DragFinish(hDrop);
		}


		return((int)Dest.size());
	}

	bool FitsMasks(std::wstring fileName, std::wstring fileMask)
	{
		std::vector<std::wstring> Temp;
		acfc::StringSplitToVector(Temp, fileMask, TEXT(";"));
		for (auto itr = Temp.begin(); itr != Temp.end(); itr++)
		{
			std::wstring msk = Trim(*itr);
			if(StrMatch(fileName.c_str(), msk.c_str()) == true)return(true);
		}

		return (false);
	}



	bool GetFiles(std::vector<std::wstring> &file_names, std::wstring folderPath, std::wstring mask, bool ShortCut)
	{
		HANDLE hFind;
		WIN32_FIND_DATA win32fd;
		std::wstring search_name = folderPath + L"\\" + mask;
		std::wstring foundFile = TEXT("");
		std::wstring foundFileD = TEXT("");

		hFind = FindFirstFile(search_name.c_str(), &win32fd);

		if (hFind == INVALID_HANDLE_VALUE)
		{
			return(false);
		}

		/* 指定のディレクトリ以下のファイル名をファイルがなくなるまで取得する */
		do
		{
			if ((win32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
			{
				/* ファイルが見つかったらVector配列に保存する */
				if (ShortCut)
				{
					foundFile = folderPath + TEXT("\\") + win32fd.cFileName;
					if (GetFileExt(foundFile) == TEXT(".lnk"))
					{
						foundFileD = GetFileFromLink(foundFile);
						file_names.push_back(foundFileD);
					}
				}
				else
				{
					file_names.push_back(folderPath + TEXT("\\") + win32fd.cFileName);
				}
			}
		} while (FindNextFile(hFind, &win32fd));

		FindClose(hFind);

		return(true);
	}

	bool GetFolders(std::vector<std::wstring> &file_names, std::wstring folderPath, bool ShortCut)
	{
		HANDLE hFind;
		WIN32_FIND_DATA win32fd;
		std::wstring search_name = folderPath + L"\\*";
		std::wstring foundFile = TEXT("");
		std::wstring foundFileD = TEXT("");

		hFind = FindFirstFile(search_name.c_str(), &win32fd);

		if (hFind == INVALID_HANDLE_VALUE)
		{
			return(false);
		}

		/* 指定のディレクトリ以下のファイル名をファイルがなくなるまで取得する */
		do
		{
			if (win32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				foundFile = win32fd.cFileName;
				if(foundFile != TEXT(".") && foundFile != TEXT(".."))
				{
					// ディレクトリは保存
					file_names.push_back(folderPath + TEXT("\\") + win32fd.cFileName);
				}
			}
			else if (ShortCut)
			{
				foundFile = folderPath + TEXT("\\") + win32fd.cFileName;
				if (GetFileExt(foundFile) == TEXT(".lnk") && FolderExists(foundFileD = GetFileFromLink(foundFile)) == true)
				{
					file_names.push_back(foundFileD);
				}

			}

		} while (FindNextFile(hFind, &win32fd));

		FindClose(hFind);

		return(true);
	}

	bool GetFolderFiles(std::vector<std::wstring>& file_names, std::vector<std::wstring>& folder_names, std::wstring folderPath)
	{
		std::vector<std::wstring>folders;
		GetFolders(folders, folderPath);
		GetFiles(file_names, folderPath, TEXT("*.*"));

		for (auto i = folders.begin(); i != folders.end(); i++)
		{
			GetFolderFiles(file_names, folder_names, *i);
		}

		std::copy(folders.begin(), folders.end(), std::back_inserter(folder_names));
		return (true);
	}

	std::wstring GetFileFromLink(std::wstring Lnk)
	{
		std::wstring ucResult = TEXT("");

		CoInitialize(NULL);

		HRESULT HR;
		IShellLink *psl;
		IPersistFile *ppf;
		TCHAR szGotPath[MAX_PATH];
		WIN32_FIND_DATA wfd;

		HR = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void **)&psl);
		if (SUCCEEDED(HR))
		{
			HR = psl->QueryInterface(IID_IPersistFile, (void **)&ppf);
			if (SUCCEEDED(HR))
			{
				HR = ppf->Load(Lnk.c_str(), STGM_READ);
				if (SUCCEEDED(HR))
				{
					HR = psl->Resolve(nullptr, SLR_ANY_MATCH);
					if (SUCCEEDED(HR))
					{
						HR = psl->GetPath(szGotPath, MAX_PATH, (WIN32_FIND_DATA *)&wfd, SLGP_UNCPRIORITY);
						ucResult = szGotPath;
					}
				}
				ppf->Release();
			}
			psl->Release();
		}
		CoUninitialize();
		return(ucResult);
	}

	HRESULT CreateLink(std::wstring OriginalFile, std::wstring ShortCutFile, std::wstring Params)
		// 第一引数は、ショートカットの元のファイル名
		// 第二引数は、ショートカットのファイル名
		//（たとえば c:\abc.lnkとか)
	{
		HRESULT hres;
		IShellLink* psl = NULL;

		// COMインターフェイスを初期化
		CoInitialize(NULL);

		// IShellLink インターフェイスを取得
		hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *)&psl);
		if (SUCCEEDED(hres))
		{
			IPersistFile* ppf;

			// Linkオブジェクトのパスを設定(たとえば、
			// C;\Windows\notepad.exeなど)
			psl->SetPath(OriginalFile.c_str());

			// 引数の設定
			if (Params != TEXT(""))psl->SetArguments(Params.c_str());

			// IPersistFileインターフェイスを取得し、
			// Linkパスファイル名を保存する。
			// これは、IShellLinkの中の
			// ディスク関連オブジェクトを
			// 取得していると思ってください。
			hres = psl->QueryInterface(IID_IPersistFile, (void**)&ppf);

			if (SUCCEEDED(hres))
			{
				// ディスクに保存する
				hres = ppf->Save(ShortCutFile.c_str(), TRUE);
				ppf->Release();
			}
			psl->Release();
		}
		return hres;
	}


	// ファイルをゴミ箱に移動する
	bool DeleteFileToRecycle(std::wstring DelFile, bool Confirm)
	{
		bool Result;
		wchar_t *SrcBuf;
		SHFILEOPSTRUCTW sfs;
		memset(&sfs, 0, sizeof(SHFILEOPSTRUCTW));

		SrcBuf = new wchar_t[DelFile.length() + 2];
		StrCpy(SrcBuf, DelFile.c_str());
		SrcBuf[DelFile.length() + 1] = L'\0';

		sfs.fFlags = FOF_NOERRORUI | FOF_SIMPLEPROGRESS | FOF_ALLOWUNDO | FOF_WANTNUKEWARNING;

		if (!Confirm)
			sfs.fFlags |= FOF_NOCONFIRMATION;

		sfs.hwnd = nullptr;
		sfs.wFunc = FO_DELETE;
		sfs.pFrom = SrcBuf;
		sfs.pTo = NULL;

		Result = (SHFileOperationW(&sfs) == 0);
		delete[] SrcBuf;

		return(Result);
	}

	// ファイルをゴミ箱に移動する
	bool DeleteFileToRecycle(std::vector<std::wstring>& DelFiles, bool Confirm)
	{
		bool Result;
		wchar_t *SrcBuf;
		SHFILEOPSTRUCTW sfs;
		memset(&sfs, 0, sizeof(SHFILEOPSTRUCTW));
		size_t L = 0, p = 0;

		for (auto i = DelFiles.begin(); i != DelFiles.end(); i++)
			L = L + i->length() + 1;
		L++;

		SrcBuf = new wchar_t[L];

		for (auto i = DelFiles.begin(); i != DelFiles.end(); i++)
		{
			StrCpy(SrcBuf + p, i->c_str());
			p += i->length() + 1;
		}
		SrcBuf[p] = TEXT('\0');

		sfs.fFlags = FOF_NOERRORUI | FOF_SIMPLEPROGRESS | FOF_ALLOWUNDO | FOF_WANTNUKEWARNING;

		if (!Confirm)
			sfs.fFlags |= FOF_NOCONFIRMATION;

		sfs.hwnd = nullptr;
		sfs.wFunc = FO_DELETE;
		sfs.pFrom = SrcBuf;
		sfs.pTo = NULL;

		Result = (SHFileOperationW(&sfs) == 0);
		delete[] SrcBuf;

		return(Result);
	}

	// あるフォルダを削除する
	bool DeleteFolder(std::wstring DelFolder)
	{
		std::vector<std::wstring> FileList;
		std::vector<std::wstring> FolderList;
		GetFolderFiles(FileList, FolderList, DelFolder);

		for (auto i = FileList.begin(); i != FileList.end(); i++)
		{
			DeleteFile(i->c_str());
		}

		for (auto i = FolderList.begin(); i != FolderList.end(); i++)
		{
			RemoveDirectory(i->c_str());
		}
		RemoveDirectory(DelFolder.c_str());
		return (false);
	}

	// ファイルをクリップボードにコピー（カット、ショートカット作成動作する）
	void FilesToClipboard(std::vector<std::wstring> &Src, int Flag) // Flag: DROPEFFECT_COPY, DROPEFFECT_MOVE, DROPEFFECT_LINK
	{
		//Unicode で正しく動作しない？
		//CF_HDROPを作成

		DROPFILES dfs;
		ZeroMemory(&dfs, sizeof(DROPFILES));
		dfs.pFiles = sizeof(dfs);
		dfs.fWide = TRUE;

		size_t Size = 0, i;
		for (i = 0; i < (int)(Src.size()); i++)
		{
			Size += Src[i].length() + 1;
		}

		Size = sizeof(DROPFILES) + Size * sizeof(wchar_t) + 2;
		HGLOBAL hDrop = GlobalAlloc(GHND, Size);
		BYTE* pDrop = static_cast<BYTE*>(GlobalLock(hDrop));
		memset(pDrop, 0, Size);
		memcpy(pDrop, &dfs, sizeof(DROPFILES));
		pDrop += sizeof(DROPFILES);

		for (i = 0; i < (int)(Src.size()); i++)
		{
			StrCpy((wchar_t *)pDrop, Src[i].c_str());
			pDrop += (Src[i].length() + 1) * sizeof(wchar_t);
		}
		GlobalUnlock(hDrop);

		//Preferred DropEffectを作成
		HGLOBAL hDropEffect = GlobalAlloc(GHND, sizeof(DWORD));
		DWORD* pdw = static_cast<DWORD*>(GlobalLock(hDropEffect));
		*pdw = Flag;
		GlobalUnlock(hDropEffect);

		//クリップボードにデーターをセット
		UINT CF_DROPEFFECT = RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT);
		OpenClipboard(nullptr);
		EmptyClipboard();
		SetClipboardData(CF_HDROP, hDrop);
		SetClipboardData(CF_DROPEFFECT, hDropEffect);
		CloseClipboard();
	}

	// ファイルをクリップボードから得る
	size_t FilesFromClipboard(std::vector<std::wstring> &Dest)
	{
		if (IsClipboardFormatAvailable(CF_HDROP))
		{
			OpenClipboard(nullptr);

			HDROP hData;
			hData = (HDROP)::GetClipboardData(CF_HDROP);

			UINT nFiles = ::DragQueryFile((HDROP)hData, (UINT)-1, nullptr, 0);

			for (UINT iFile = 0; iFile < nFiles; ++iFile)
			{
				TCHAR szPath[MAX_PATH];
				DragQueryFile((HDROP)hData, iFile, szPath, sizeof(szPath));
				Dest.push_back(szPath);
			}
			CloseClipboard();
		}
		return(Dest.size());
	}

	void SelectFileInExplorer(std::wstring Src)
	{
		// This is the command line for explorer which tells it to select the given file
		std::wstring CommandLine = TEXT("/Select,");
		CommandLine += Src;

		// Prepare shell execution params
		SHELLEXECUTEINFO shExecInfo = {};
		shExecInfo.cbSize = sizeof(shExecInfo);
		shExecInfo.lpFile = TEXT("Explorer.exe");
		shExecInfo.lpParameters = CommandLine.c_str();
		shExecInfo.nShow = SW_SHOWNORMAL;
		shExecInfo.lpVerb = TEXT("Open"); // Context menu item

		// Just have a look in MSDN to see the relevance of these flags
		shExecInfo.fMask = SEE_MASK_INVOKEIDLIST | SEE_MASK_FLAG_DDEWAIT | SEE_MASK_FLAG_NO_UI;

		// Select file in explorer
		ShellExecuteExW(&shExecInfo);
	}
	std::wstring GetNonOverwrapFileName(std::wstring Src)
	{
		std::wstring Dest = Src, Name, Path, Ext;
		size_t Count = 1;

		// まず最初のファイル名が (1) 形式かどうかチェック
		Path = GetFolderName(Src);
		Ext = GetFileExt(Src);
		Name = GetFileNameWithoutExt(Src);

		if (Name[Name.length() - 1] == TEXT(')'))
		{
			size_t i = Name.length() - 2;
			while (i >= 0)
			{
				if (Name[i] < TEXT('0') || Name[i] > TEXT('9'))break;
				i--;
			}

			if (Name[i] == TEXT('(') && i > 0)
			{
				Name = Trim(Name.substr(0, i - 1));
			}
		}

		Dest = Path + TEXT("\\") + Name + Ext;
		while (FileExists(Dest) == true)
		{
			Dest = Path + TEXT("\\") + Name + TEXT(" (") + std::to_wstring(Count) + TEXT(")") + Ext;
			Count++;
		}
		return(Dest);
	}

	size_t GetFileSizeValue(std::wstring src)
	{
		HANDLE hFile;
		DWORD size_low, size_high;

		hFile = CreateFile(src.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)return 0;

		size_low = GetFileSize(hFile, &size_high);

		CloseHandle(hFile);

		return (size_low + ((__int64)size_high << 32));
	}

	time_t GetFileCreationTime(std::wstring src)
	{
		HANDLE hFile;
		FILETIME ftFileTime;
		FILETIME ftLocalTime;

		hFile = CreateFile(src.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)return 0;

		GetFileTime(hFile, &ftFileTime, NULL, NULL);
		FileTimeToLocalFileTime(&ftFileTime, &ftLocalTime);

		CloseHandle(hFile);

		return (ftLocalTime.dwLowDateTime + ((__int64)ftLocalTime.dwHighDateTime << 32));
	}
	// End SelectFileInExplorer

	// ファイル名に使えない文字を得る -1:なし 0~:そのインデックスの文字
	int CheckFileIrregularChar(std::wstring FileName)
	{
		size_t i;
		for (i = 0; i < (int)(FileName.length()); i++)
		{
			if (FileName[i] == TEXT('/')
				|| FileName[i] == TEXT('\\')
				|| FileName[i] == TEXT(':')
				|| FileName[i] == TEXT('*')
				|| FileName[i] == TEXT('?')
				|| FileName[i] == TEXT('\'')
				|| FileName[i] == TEXT('<')
				|| FileName[i] == TEXT('>')
				|| FileName[i] == TEXT('|')
				)
			{
				return((int)i);
			}
		}
		return(-1);
	}

	std::wstring GetFileName(std::wstring src)
	{
		size_t p;

		p = src.rfind(TEXT('\\'));
		if (p != std::wstring::npos) {
			return (src.substr(p + 1, src.size() - p - 1));
		}

		p = src.rfind(TEXT('/'));
		if (p != std::wstring::npos) {
			return (src.substr(p + 1, src.size() - p - 1));
		}

		return(src);
	}

	std::wstring GetFileNameWithoutExt(std::wstring src)
	{
		return (ChangeFileExt(GetFileName(src), TEXT("")));
	}

	std::wstring GetFileExt(std::wstring src)
	{
		size_t p;

		p = src.rfind(L'.');
		if (p != std::wstring::npos)
		{
			return (src.substr(p, src.size() - p));
		}
		return(src);
	}

	std::wstring ChangeFileExt(std::wstring src, std::wstring ext)
	{
		size_t p;

		p = src.rfind(L'.');
		if (p != std::wstring::npos)
		{
			return (src.substr(0, p) + ext);
		}
		return(src + ext);
	}

	std::wstring GetFolderName(std::wstring src)
	{
		size_t p;

		p = src.rfind(L'\\');
		if (p != std::wstring::npos)
		{
			return (src.substr(0, p));
		}

		p = src.rfind(L'/');
		if (p != std::wstring::npos)
		{
			return (src.substr(0, p));
		}

		return(src);
	}

	std::wstring GetParentFolder(std::wstring src)
	{
		size_t p;
		p = src.find(TEXT("\\\\"));
		if (p == 0) // ネットワークドライブ
		{
			size_t q = src.rfind(TEXT('\\'));
			if (q != std::wstring::npos)
			{
				size_t r = src.find(TEXT('\\'), p + 2);
				if (r != q)
				{
					return(src.substr(0, q));
				}
			}
		}
		else
		{
			size_t q = src.rfind(TEXT('\\'));
			if (q != std::wstring::npos)
			{
				if (q >= src.length() - 1)return(TEXT(""));

				return(src.substr(0, q));
			}
		}

		return(TEXT(""));
	}

	std::wstring GetMiniPathName(std::wstring src, int Num)
	{
		size_t l = src.length(), p;
		size_t q = l - 1;
		while (Num >= 0)
		{
			p = src.rfind(TEXT('\\'), q);
			if (p == std::wstring::npos)break;
			q = p - 1;
			if (q < 0)
			{
				q = 0;
				break;
			}
			Num--;
		}
		return(src.substr(p + 1, l - p - 1));
	}

	std::wstring GetSpecialFolderPath(UINT Value)
	{
		TCHAR waFolderPath[MAX_PATH];
		SHGetSpecialFolderPath(0, waFolderPath, Value, 0);
		std::wstring result = waFolderPath;

		return (result);
	}


	std::string UnicodeToMultiByte(const std::wstring& Source, UINT CodePage, DWORD Flags)
	{
		if (int Len = WideCharToMultiByte(CodePage, Flags, Source.c_str(), static_cast<int>(Source.size()), NULL, 0, NULL, NULL))
		{
			std::vector<char> Dest(Len);
			if (Len = WideCharToMultiByte(CodePage, Flags, Source.c_str(), static_cast<int>(Source.size()), &Dest[0], static_cast<int>(Dest.size()), NULL, NULL))
			{
				return (std::string(Dest.begin(), Dest.begin() + Len));
			}
		}
		return ("");
	}

	std::wstring MultiByteToUnicode(const std::string& Source, UINT CodePage, DWORD Flags)
	{
		if (int Len = MultiByteToWideChar(CodePage, Flags, Source.c_str(), static_cast<int>(Source.size()), NULL, 0))
		{
			std::vector<TCHAR> Dest(Len);
			if (Len = MultiByteToWideChar(CodePage, 0, Source.c_str(), static_cast<int>(Source.size()), &Dest[0], static_cast<int>(Dest.size())))
			{
				return (std::wstring(Dest.begin(), Dest.begin() + Len));
			}
		}
		return (L"");
	}

	size_t LoadBinaryFile(std::wstring &FileName, BYTE *&Dest)
	{
		HANDLE hFile;
		hFile = CreateFile(FileName.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

		DWORD hsize;
		DWORD size = GetFileSize(hFile, &hsize);
		size_t fsize = size + (hsize << 16);
		DWORD rsize;
		
		if (hFile == nullptr)return(0);
		Dest = new BYTE[fsize];

		ReadFile(hFile, Dest, (DWORD)fsize, &rsize, nullptr);

		CloseHandle(hFile);
		return(rsize);
	}

	size_t SaveBinaryFile(std::wstring &FileName, BYTE *Src, size_t size)
	{
		HANDLE hFile;
		hFile = CreateFile(FileName.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

		if (hFile == nullptr)return(0);
		DWORD rsize;

		WriteFile(hFile, Src, (DWORD)size, &rsize, 0);

		CloseHandle(hFile);
		return(rsize);
	}

	std::wstring LoadTextFile(std::wstring &FileName)
	{
		BYTE *Data;
		size_t size;
		size = LoadBinaryFile(FileName, Data);
		if (size == 0)return(TEXT(""));

		TCHAR *wbuf = new TCHAR[size / 2 + 1];
		wbuf = (TCHAR *)Data;

		if(*wbuf == 0xFFFE || *wbuf == 0xFEFF)wbuf++; // BOM を捨てる

		TCHAR *rbuf = new TCHAR[size / (sizeof(TCHAR) / sizeof(BYTE)) + 1];
		size /= 2;
		int i, p = 0;
		for (i = 0; i < (int)(size - 1); i++) // 改行マークの処理
		{
			if (wbuf[i] != 0x0D)
			{
				rbuf[p] = wbuf[i];
				p++;
			}
		}
		rbuf[p] = TEXT('\0');

		std::wstring res = rbuf;

		delete[] Data;
		delete[] rbuf;

		return(res);
	}

	size_t SaveTextFile(std::wstring &FileName, std::wstring &Data)
	{
		size_t L = Data.length();
		size_t size = L + 1;
		TCHAR *Dest;
		size_t i, p = 0;
		for (i = 0; i < L; i++)
		{
			if (Data[i] == TEXT('\n'))size++;
		}

		Dest = new TCHAR[size];
		Dest[p] = (TCHAR)0xFEFF;
		p++;
		for (i = 0; i < L; i++)
		{
			if (Data[i] == TEXT('\n'))
			{
				Dest[p] = 0x0D; p++;
				Dest[p] = 0x0A; p++;
			}
			else
			{
				Dest[p] = Data[i];
				p++;
			}
		}

		BYTE *tmp = (BYTE *)Dest;
		size *= 2;

		return(SaveBinaryFile(FileName, tmp, size));
	}

	bool VectorCombineToString(std::wstring &Dest, std::vector<std::wstring> &Src, std::wstring Separator)
	{
		Dest = L"";
		int i;
		for (i = 0; i < (int)(Src.size()); i++)
		{
			Dest = Dest + Src[i] + Separator;
		}
		return(true);
	}

	bool StringSplitToVector(std::vector<std::wstring> &Dest, std::wstring &Src, std::wstring Separator)
	{
		size_t s = 0, p = 0, l = Src.length();
		while (s < l)
		{
			p = Src.find(Separator, s);
			if (p == std::wstring::npos)
			{
				Dest.push_back(Src.substr(s, l - s));
				break;
			}
			Dest.push_back(Src.substr(s, p - s));
			s = p + 1;
		}
		return(true);
	}

	bool ParseCommanLine(std::vector<std::wstring> &Dest, LPCWSTR lpCmdLine)
	{
		int Num;
		LPWSTR *Cmd;
		Cmd = CommandLineToArgvW(lpCmdLine, &Num);
		for(int i = 0;i < Num;i++)Dest.push_back(Cmd[i]);
		LocalFree(Cmd);
		return(true);
	}

	std::wstring GetStringValue(std::map<std::wstring, std::wstring> &Map, std::wstring Src, std::wstring Default)
	{
		std::map<std::wstring, std::wstring>::iterator itr = Map.find(Src);
		if (itr == Map.end())return (Default);

		return(itr->second);
	}

	// 文字列を数値に変換
	int GetIntegerValue(std::map<std::wstring, std::wstring> &Map, std::wstring Src, int Default, int Min, int Max)
	{
		std::map<std::wstring, std::wstring>::iterator itr = Map.find(Src);
		if (itr == Map.end())return (Default);

		std::wstring src = itr->second;
		return(GetIntegerValue(src, Default, Min, Max));
	}

	// 文字列を数値に変換
	int GetIntegerValue(std::wstring src)
	{
		return(GetIntegerValue(src, 0, 0, 0));
	}

	// 文字列を数値に変換
	int GetIntegerValue(std::wstring src, int Default, int Min, int Max)
	{
		bool Minus = false;

		size_t i, l = src.length();
		__int64 c = 0;

		for (i = 0; i < l; i++)
		{
			if (src[i] == TEXT('-'))
			{
				Minus = true;
				continue;
			}
			if (src[i] < TEXT('0') || src[i] > TEXT('9'))continue;
			c = c * 10 + src[i] - TEXT('0');
		}

		if (Minus) c = -c;

		if (Max != Min && c > Max)
		{
			c = (__int64)Max;
		}

		if (Max != Min && c < Min)
		{
			c = (__int64)Min;
		}

		return((int)c);
	}

	// 文字列を数値に変換
	double GetDoubleValue(std::map<std::wstring, std::wstring> &Map, std::wstring Src, double Default, double Min, double Max)
	{
		std::map<std::wstring, std::wstring>::iterator itr = Map.find(Src);
		if (itr == Map.end())return (Default);

		std::wstring src = itr->second;
		return(GetDoubleValue(src, Default, Min, Max));
	}

	// 文字列を数値に変換
	double GetDoubleValue(std::wstring src)
	{
		return(GetDoubleValue(src, 0.0, 0.0, 0.0));
	}

	// 文字列を数値に変換
	double GetDoubleValue(std::wstring src, double Default, double Min, double Max)
	{
		bool Minus = false;
		bool Double = false;


		int i, c = 0, l = (int)(src.length());
		double d = 0.0, dp = 10.0;

		for (i = 0; i < l; i++)
		{
			if (src[i] == TEXT('-'))
			{
				Minus = true;
				continue;
			}
			if (src[i] == TEXT('.'))
			{
				Double = true;
			}
			if (src[i] < TEXT('0') || src[i] > TEXT('9'))continue;
			if (Double == false)
			{
				c = c * 10 + src[i] - TEXT('0');
			}
			else
			{
				d = d * 10 + src[i] - TEXT('0');
				dp *= 10;
			}
		}

		double r;
		if (Double)
		{
			r = c + d / dp;
		}
		else
		{
			r = c;
		}

		if (Minus) r = -r;

		if (Max != Min && r > Max)
		{
			r = Max;
		}

		if (Max != Min && r < Min)
		{
			r = Min;
		}

		return(r);
	}

	// 文字列を真偽値に変換
	bool GetBoolValue(std::map<std::wstring, std::wstring> &Map, std::wstring Src, bool Default)
	{
		std::map<std::wstring, std::wstring>::iterator itr = Map.find(Src);
		if (itr == Map.end())return (Default);
		std::wstring src = itr->second;
		return(GetBoolValue(src, Default));
	}

	bool GetBoolValue(std::wstring src)
	{
		return(GetBoolValue(src, false));
	}

	// 文字列を真偽値に変換
	bool GetBoolValue(std::wstring src, bool Default)
	{
		std::wstring s = LowerCase(Trim(src));

		if (s == TEXT("true"))return(true);
		if (s == TEXT("false"))return(false);
		return(Default);
	}

	std::wstring BoolToString(bool src)
	{
		if (src)return(TEXT("true"));
		return (TEXT("false"));
	}

	// 大文字に変換(ASCII 部のみ)
	std::wstring UpperCase(std::wstring a)
	{
		for (int i = 0; i < (int)(a.length()); i++)
		{
			if (a[i] >= TEXT('a') && a[i] <= TEXT('z'))a[i] = a[i] - TEXT('a') + TEXT('A');
		}
		return (a);
	}

	// 小文字に変換(ASCII 部のみ)
	std::wstring LowerCase(std::wstring a)
	{
		for (int i = 0; i < (int)(a.length()); i++)
		{
			if (a[i] >= TEXT('A') && a[i] <= TEXT('Z'))a[i] = a[i] - TEXT('A') + TEXT('a');
		}
		return (a);
	}

	// トリミングする
	std::wstring Trim(std::wstring a)
	{
		size_t s, e;
		std::wstring res;

		for (s = 0; s < (int)(a.length()); s++)
		{
			if (a[s] >= (TCHAR)32 && a[s] != TEXT(' ') || a[s] != TEXT('　'))break;
		}

		for (e = a.length() - 1; e >= 0; e--)
		{
			if (a[e] >= (TCHAR)32 && a[e] != TEXT(' ') || a[e] != TEXT('　'))break;
		}

		res = a.substr(s, e - s + 1);
		return(res);

	}

	bool LoadMapFromFile(std::map<std::wstring, std::wstring> &Dest, std::wstring &FileName)
	{
		std::wstring file;
		file = LoadTextFile(FileName);
		if (file.length() == 0)return(false);

		std::vector<std::wstring> para;
		
		StringSplitToVector(para, file, TEXT("\n"));

		std::vector<std::wstring> pair;
		for (int i = 0; i < (int)(para.size()); i++)
		{
			size_t p = para[i].find(TEXT('='));
			if (p == std::wstring::npos)continue;
			std::wstring key = para[i].substr(0, p);
			std::wstring val = para[i].substr(p + 1, para[i].length() - p - 1);
			Dest[key] = val;
		}
		return(true);
	}

	static bool SaveMapToFile(std::map<std::wstring, std::wstring> &Source, std::wstring &FileName)
	{
		std::wstring tmp = TEXT("");
		std::map<std::wstring, std::wstring>::iterator itr;
		for (itr = Source.begin(); itr != Source.end(); itr++)
		{
			tmp = tmp + itr->first + TEXT("=") + itr->second + TEXT("\n");
		}
		size_t s = SaveTextFile(FileName, tmp);

		return(s != 0);
	}

	std::wstring GetWindowString(HWND handle)
	{
		std::wstring result;
		int l = GetWindowTextLength(handle) + 1;
		TCHAR *buf = new TCHAR[l];
		GetWindowText(handle, buf, l);;
		result = buf;
		return (result);
	}

	//Rectangle GetDesktopRectangle()
	//{
	//	Rectangle r = new Rectangle(0, 0, 0, 0);
	//	Screen[] screens = Screen.AllScreens;
	//	foreach(var a in screens)
	//	{
	//		if (r.Y > a.Bounds.Y) r.Y = a.Bounds.Y;
	//		if (r.X + r.Width < a.Bounds.X + a.Bounds.Width) r.Width = (a.Bounds.X + a.Bounds.Width) - r.X;
	//		if (r.X > a.Bounds.X) r.X = a.Bounds.X;
	//		if (r.Y + r.Height < a.Bounds.Y + a.Bounds.Height) r.Height = (a.Bounds.Y + a.Bounds.Height) - r.Y;
	//	}
	//	return (r);
	//}
	//
	//Rectangle GetMonitorParameter()
	//{
	//	Rectangle r = new Rectangle(0, 0, 0, 0);
	//	Screen[] screens = Screen.AllScreens;
	//	foreach(var a in screens)
	//	{
	//		if (r.Y > a.Bounds.Y) r.Y = a.Bounds.Y;
	//		if (r.X + r.Width < a.Bounds.X + a.Bounds.Width) r.Width = (a.Bounds.X + a.Bounds.Width) - r.X;
	//		if (r.X > a.Bounds.X) r.X = a.Bounds.X;
	//		if (r.Y + r.Height < a.Bounds.Y + a.Bounds.Height) r.Height = (a.Bounds.Y + a.Bounds.Height) - r.Y;
	//	}
	//	return (r);
	//}


	//-----------------------------------------
	//
	// OpenFileDialog Class
	//
	//-----------------------------------------

	COpenFileDialog::COpenFileDialog(void)
	{
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = MAX_PATH * 256;
	}

	bool COpenFileDialog::ShowDialog(HWND hwnd)
	{
		ofn.hwndOwner = hwnd;
		ofn.lpstrInitialDir = (TCHAR*)InitialDirectory.c_str();
		ofn.lpstrTitle = (LPWSTR)Title.c_str();

		memset(szFile, 0, sizeof(TCHAR) * MAX_PATH * 256);

		if (MultiSelect) ofn.Flags |= OFN_ALLOWMULTISELECT;

		TCHAR *ftr = new TCHAR[Filter.length() + 2];

		int i = 0;
		while (i < (int)Filter.length())
		{
			if (Filter[i] != TEXT('|'))
				ftr[i] = Filter[i];
			else
				ftr[i] = 0;
			i++;
		}
		ftr[i] = 0; i++;
		ftr[i] = 0;
		ofn.lpstrFilter = ftr;

		BOOL result = GetOpenFileName(&ofn);
		delete ftr;

		if (result == false)return(false);

		GetFiles(FileNames);
		if (FileNames.size() > 0)
			FileName = FileNames[0];
		else
			FileName = TEXT("");


		return(result);
	}

	bool COpenFileDialog::GetFiles(std::vector<std::wstring>& Dest)
	{
		size_t i = 0;
		size_t  j = 0;
		std::wstring filename;
		std::wstring path;
		
		while (szFile[i + 1] != 0)
		{
			if (j == 0)
			{
				path = szFile + i;
				j++;
			}
			else
			{
				filename = path + TEXT("\\") + (szFile + i);
				Dest.push_back(filename);
				j++;
			}
			i += wcslen(szFile) + 1;
		}

		if (j == 1)
		{
			Dest.push_back(path);
		}

		return(j != 0);

	}
	

	//-----------------------------------------
	//
	// SaveFileDialog Class
	//
	//-----------------------------------------

	CSaveFileDialog::CSaveFileDialog(void)
	{
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = MAX_PATH;
	}

	bool CSaveFileDialog::ShowDialog(HWND hwnd)
	{
		ofn.hwndOwner = hwnd;
		ofn.lpstrInitialDir = (LPWSTR)InitialDirectory.c_str();
		ofn.lpstrDefExt = (LPWSTR)DefaultExt.c_str();
		ofn.lpstrTitle = (LPWSTR)Title.c_str();

		if (Overwrite) ofn.Flags |= OFN_OVERWRITEPROMPT;

		TCHAR *ftr = new TCHAR[Filter.length() + 2];

		int i = 0;
		while (i < (int)Filter.length())
		{
			if (Filter[i] != TEXT('|'))
				ftr[i] = Filter[i];
			else
				ftr[i] = 0;
			i++;
		}
		ftr[i] = 0; i++;
		ftr[i] = 0;
		ofn.lpstrFilter = ftr;

		memset(szFile, 0, MAX_PATH * sizeof(TCHAR));
		StrCpy(szFile, FileName.c_str());

		BOOL result = GetSaveFileName(&ofn);

		delete ftr;

		if (result == false)return(false);

		FileName = szFile;

		return(result);
	}

	bool CSaveFileDialog::GetFile(std::wstring& Dest)
	{
		Dest = szFile;
		return(true);
	}

	//-----------------------------------------
	//
	// SelectColorDialog Class
	//
	//-----------------------------------------
	
	CSelectColorDialog::CSelectColorDialog(void)
	{
		cs.lStructSize = sizeof(CHOOSECOLOR);
		cs.lpCustColors = CustColors;
		cs.Flags = CC_RGBINIT | CC_FULLOPEN;
		cs.lCustData = NULL;
		cs.lpfnHook = NULL;
		cs.lpTemplateName = NULL;
	}

	bool CSelectColorDialog::ShowDialog(HWND hwnd)
	{
		cs.hwndOwner = hwnd;
		cs.rgbResult = ColorRef;
		//カラーダイアログ表示
		BOOL Result = ChooseColor(&cs);
		if (Result)
		{
			ColorRef = cs.rgbResult;
			GdipColor.SetFromCOLORREF(ColorRef);
		}
		return(Result);
	}

	void CSelectColorDialog::SetColorGdip(Gdiplus::Color Src)
	{
		cs.rgbResult = Src.ToCOLORREF();
		ColorRef = Src.ToCOLORREF();
		GdipColor = Src;
	}

	void CSelectColorDialog::SetColorRef(COLORREF Src)
	{
		cs.rgbResult = Src;
		ColorRef = Src;
		GdipColor.SetFromCOLORREF(Src);
	}

	Gdiplus::Color CSelectColorDialog::GetColorGdip(void)
	{
		return (GdipColor);
	}

	COLORREF CSelectColorDialog::GetColorRef(void)
	{
		return (ColorRef);
	}


	void CSelectColorDialog::SetCustomColor(COLORREF * pColorRef)
	{
		cs.lpCustColors = pColorRef;
	}

	//-----------------------------------------
	//
	// OpenFolderDialog Class
	//
	//-----------------------------------------

	CFolderSelectDialog::CFolderSelectDialog(void)
	{

	}

	bool CFolderSelectDialog::ShowDialog(HWND hwnd)
	{
		IFileDialog *pDialog = NULL;
		HRESULT hr;
		IShellItem *psiFolder;

		// インスタンス生成
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDialog));
		if (FAILED(hr))return(false);
		
		SHCreateItemFromParsingName(InitialDirectory.c_str(), NULL, IID_PPV_ARGS(&psiFolder));

		// 設定の初期化
		pDialog->SetFolder(psiFolder);
		pDialog->GetOptions(&Options);
		pDialog->SetOptions(Options | FOS_PICKFOLDERS);
		pDialog->SetTitle(Title.c_str());

		// フォルダ選択ダイアログを表示
		hr = pDialog->Show(hwnd);

		// 結果取得
		if (SUCCEEDED(hr))
		{
			IShellItem *pItem = NULL;
			PWSTR pPath = NULL;

			hr = pDialog->GetResult(&pItem);
			if (SUCCEEDED(hr))
			{
				hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pPath);
				if (SUCCEEDED(hr))
				{
					// pPathを用いて所定の処理をする
					FolderName = pPath;
					// pPathのメモリを開放する
					CoTaskMemFree(pPath);
				}
			}
		}
		// ダイアログインスタンスの開放
		pDialog->Release();
		return(true);
	}


	//-----------------------------------------
	//
	// NumberEdit Class
	//
	//-----------------------------------------

	CNumberStr::CNumberStr(void)
	{
		IMin = 0;
		IMax = 0;
		IValue = 0;

		DMin = 0;
		DMax = 0;
		DValue = 0;

		MinusEnabled = false;
		DoubleMode = false;
		MinEnabled = false;
		MaxEnabled = false;
	}

	CNumberStr::~CNumberStr(void)
	{
	}

	void CNumberStr::SetIntegerMode(int SrcValue, bool aMinusEnabled, bool aMinEnabled, bool aMaxEnabled, int aIMin, int aIMax)
	{
		MinusEnabled = aMinusEnabled;
		MaxEnabled = aMaxEnabled;
		MinEnabled = aMinEnabled;
		IMax = aIMax;
		IMin = aIMin;
		DoubleMode = false;
		SetIntegerValue(SrcValue);
	}

	void CNumberStr::SetDoubleMode(double SrcValue, bool aMinusEnabled, bool aMinEnabled, bool aMaxEnabled, double aDMin, double aDMax)
	{
		MinusEnabled = aMinusEnabled;
		MaxEnabled = aMaxEnabled;
		MinEnabled = aMinEnabled;
		DMax = aDMax;
		DMin = aDMin;
		DoubleMode = true;
		SetDoubleValue(SrcValue);
	}


	std::wstring CNumberStr::CheckString(std::wstring src)
	{
		size_t pos = 0;
		return(CheckString(src, pos));
	}


	std::wstring CNumberStr::CheckString(std::wstring src, size_t &pos)
	{
		size_t i = 0;
		bool minus = false;
		bool dot = false;
		double dvalue = 0;
		double dratio = 1;
		int ivalue = 0;
		bool LastDot = false;
		if (src.length() == 0 || src == TEXT("-")) return (src);


		size_t orgLen = src.length();

		for (i = 0; i < src.length(); i++)
		{
			if (DoubleMode)
			{
				if (minus == false && src[i] == TEXT('-') && MinusEnabled)
				{
					minus = true;
				}
				else if (src[i] >= TEXT('0') && src[i] <= TEXT('9'))
				{
					if (dot == false)
					{
						dvalue = dvalue * 10 + src[i] - TEXT('0');
					}
					else
					{
						dratio /= 10;
						dvalue = dvalue + (src[i] - TEXT('0')) * dratio;
					}
				}
				else if (src[i] == '.')
				{
					if (i == src.length() - 1)
					{
						if (dot == false || (src.length() > 1 && src[src.length() - 2] == TEXT('.')))LastDot = true;
					}

					dot = true;
				}
			}
			else
			{
				if (minus == false && src[i] == TEXT('-') && MinusEnabled)
				{
					minus = true;
				}
				else if (src[i] >= TEXT('0') && src[i] <= TEXT('9'))
				{
					ivalue = ivalue * 10 + src[i] - TEXT('0');
				}
			}
		}

		if (DoubleMode)
		{
			if (minus == true) dvalue = -dvalue;
			dvalue = SetDoubleValue(dvalue);
			src = std::to_wstring(dvalue);
			if (LastDot) src += TEXT(".");
		}
		else
		{
			if (minus == true) ivalue = -ivalue;
			ivalue = SetIntegerValue(ivalue);
			src = std::to_wstring(ivalue);
		}
		pos = pos + src.length() - orgLen;
		if (pos < 0)pos = 0;
		if (pos > src.length())pos = src.length();

		SValue = src;

		return (src);
	}

	int CNumberStr::SetIntegerValue(int src)
	{
		if (MinEnabled && IMin > src) src = IMin;
		if (MaxEnabled && IMax < src) src = IMax;
		IValue = src;
		return(IValue);
	}

	double CNumberStr::SetDoubleValue(double src)
	{
		if (MinEnabled && DMin > src) src = DMin;
		if (MaxEnabled && DMax < src) src = DMax;
		DValue = src;
		return(DValue);
	}

	int CNumberStr::GetIntegerValue(void)
	{
		return(IValue);
	}

	double CNumberStr::GetDoubleValue(void)
	{
		return(DValue);
	}

	//-----------------------------------------
	//
	// BaseWindow Class
	//
	//-----------------------------------------
	CBaseWindow::CBaseWindow(void)
	{
	}

	CBaseWindow::~CBaseWindow(void)
	{
		if (handle != nullptr)
		{
			DestroyWindow(handle); // 多分不要
		}
	}

	void CBaseWindow::Show(void)
	{
		ShowWindow(handle, TRUE);
		Visible = true;
	}

	void CBaseWindow::Hide(void)
	{
		ShowWindow(handle, FALSE);
		Visible = false;
	}

	void CBaseWindow::Focus(void)
	{
		SetFocus(handle);
	}

	void CBaseWindow::SetPositionAndSize(int ax, int ay, int aWidth, int aHeight)
	{
		SetWindowPos(handle, nullptr, ax, ay, aWidth, aHeight, SWP_NOZORDER);
		x = ax;
		y = ay;
		Width = aWidth;
		Height = aHeight;
	}

	void CBaseWindow::SetWindowPosition(int ax, int ay)
	{
		SetWindowPos(handle, nullptr, ax, ay, 0, 0, SWP_NOZORDER | SWP_NOMOVE);
		x = ax;
		y = ay;
	}

	void CBaseWindow::SetWindowSize(int aWidth, int aHeight)
	{
		SetWindowPos(handle, nullptr, 0, 0, aWidth, aHeight, SWP_NOZORDER | SWP_NOSIZE);
		Width = aWidth;
		Height = aHeight;
	}

	bool CBaseWindow::SetHWND(HWND hWnd)
	{
		handle = hWnd;
		GetWindowStatus();
		return(true);
	}

	void CBaseWindow::GetWindowStatus(void)
	{
		RECT r;
		GetWindowRect(handle, &r);
		Width = r.right - r.left;
		Height = r.bottom - r.top;
		x = r.left;
		y = r.top;
		HWNDset = true;
		Visible = IsWindowVisible(handle);

	}

	//-----------------------------------------
	//
	// ListBox Class
	//
	//-----------------------------------------

	LRESULT CALLBACK CListBoxProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		CListBox *ListBox;
		ListBox = (CListBox *)GetProp(hWnd, TEXT("THIS"));
		LRESULT result = ListBox->ProcessMessages(hWnd, message, wParam, lParam);
		return(result);
	}

	CListBox::CListBox(void)
	{
	}

	CListBox::~CListBox(void)
	{
	}

	void CListBox::Init(HWND hwnd, LPCREATESTRUCT lp, LONG_PTR EventProcedure, bool CallProcedure)
	{
		handle = CreateWindow(
			TEXT("LISTBOX"), NULL,
			WS_CHILD | WS_VSCROLL | LBS_EXTENDEDSEL,
			0, 0, 100, 100, hwnd, (HMENU)1,
			lp->hInstance, NULL
		);
		if(CallProcedure == true)
			OrgProc = (WNDPROC)SetWindowLongPtr(handle, GWLP_WNDPROC, EventProcedure);
		Get();
	}

	bool CListBox::SetHWND(HWND hWnd)
	{
		if (CBaseWindow::SetHWND(hWnd) == false)return(false);
		OrgProc = (WNDPROC)GetWindowLongPtr(handle, GWLP_WNDPROC);
		SetWindowLongPtr(handle, GWLP_WNDPROC, (LONG_PTR)CListBoxProc);
		SetProp(handle, TEXT("THIS"), (HANDLE)this);
		return(true);
	}


	LRESULT CListBox::ProcessMessages(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		LRESULT result = 0;
		switch (message)
		{
		case WM_KEYDOWN:
		case WM_LBUTTONUP:
			GetSelectedIndex();
			break;
		}

		if (EventProc != nullptr)
			CallWindowProc(EventProc, hWnd, message, wParam, lParam);
		result = CallWindowProc(OrgProc, hWnd, message, wParam, lParam);
		return (result);
	}

	void CListBox::SetText(std::wstring Src, int Index)
	{
		LRESULT num = SendMessage(handle, LB_GETCOUNT, 0, 0);
		if (Index < 0 || Index >= num)return;

		Items[Index].Text = Src;
		SendMessage(handle, WM_SETREDRAW, FALSE, 0); // 再描画停止
		bool sel = (SendMessage(handle, LB_GETSEL, Index, 0) != 0);
		LRESULT idx = SendMessage(handle, LB_GETCURSEL, 0, 0);

		SendMessage(handle, LB_DELETESTRING, Index, (LPARAM)Items[Index].Text.c_str());
		SendMessage(handle, LB_INSERTSTRING, Index, (LPARAM)Items[Index].Text.c_str());

		if (sel)
			SendMessage(handle, LB_SETSEL, Index, TRUE);

		if (idx == Index)
			SendMessage(handle, LB_SETCURSEL, Index, 0);
		
		SendMessage(handle, WM_SETREDRAW, TRUE, 0); // 再描画停止

	}

	void CListBox::Insert(std::wstring Src, int Index)
	{
		LRESULT num = SendMessage(handle, LB_GETCOUNT, 0, 0);
		if (Index < 0)return;

		CItem Item(Src, false);
	
		auto itr = Items.insert(Items.begin() + Index, Item);
		
		SendMessage(handle, LB_INSERTSTRING, Index, (LPARAM)itr->Text.c_str());
		SetSelected(Index, false);
	}

	void CListBox::Insert(std::wstring Src, bool Sel, int Index)
	{
		LRESULT num = SendMessage(handle, LB_GETCOUNT, 0, 0);
		if (Index < 0)return;

		CItem Item(Src, Sel);

		auto itr = Items.insert(Items.begin() + Index, Item);
		if (Sel)SelectedCount++;

		SendMessage(handle, LB_INSERTSTRING, Index, (LPARAM)itr->Text.c_str());
		SetSelected(Index, Sel);
	}

	void CListBox::Insert(CItem Src, int Index)
	{
		LRESULT num = SendMessage(handle, LB_GETCOUNT, 0, 0);
		if (Index < 0)return;

		auto itr = Items.insert(Items.begin() + Index, Src);
		if (Src.Selected)SelectedCount++;

		SendMessage(handle, LB_INSERTSTRING, Index, (LPARAM)itr->Text.c_str());
		SetSelected(Index, Src.Selected);
	}

	void CListBox::Delete(int Index)
	{
		LRESULT num = SendMessage(handle, LB_GETCOUNT, 0, 0);
		if (Index < 0 || Index >= num)return;

		if (Items[Index].Selected)SelectedCount--;
		Items.erase(Items.begin() + Index);

		SendMessage(handle, LB_DELETESTRING, Index, 0);
	}

	void CListBox::Clear(void)
	{
		Items.clear();
		SendMessage(handle, LB_RESETCONTENT, 0, 0);
	}

	bool CListBox::IsSelected(int i)
	{
		LRESULT num = SendMessage(handle, LB_GETCOUNT, 0, 0);
		if (i < 0 || i >= num)return(false);
		Items[i].Selected = SendMessage(handle, LB_GETSEL, i, 0);
		return(Items[i].Selected);
	}

	void CListBox::SetSelected(int i, bool sel)
	{
		LRESULT num = SendMessage(handle, LB_GETCOUNT, 0, 0);
		if (i < 0 || i >= num)return;
		if(sel)
			SendMessage(handle, LB_SETSEL, TRUE, i);
		else
			SendMessage(handle, LB_SETSEL, FALSE, i);
		Items[i].Selected = sel;
	}

	void CListBox::SelectAll(bool sel)
	{
		SendMessage(handle, WM_SETREDRAW, FALSE, 0); // 再描画停止
		for (size_t i = 0; i < Items.size(); i++)
		{
			Items[i].Selected = true;
			SendMessage(handle, LB_SETSEL, sel, i);
		}
		SendMessage(handle, WM_SETREDRAW, TRUE, 0); // 再描画停止
	}

	void CListBox::SelectInvert(void)
	{
		SendMessage(handle, WM_SETREDRAW, FALSE, 0); // 再描画停止
		for (size_t i = 0; i < Items.size(); i++)
		{
			Items[i].Selected = !Items[i].Selected;
			SendMessage(handle, LB_SETSEL, Items[i].Selected, i);
		}
		SendMessage(handle, WM_SETREDRAW, TRUE, 0); // 再描画停止
	}

	void CListBox::Get(void)
	{
		GetText();
		GetSelectedIndex();
		GetSelectList();
	}

	void CListBox::GetText(void)
	{
		TCHAR buf[MAX_PATH];
		LRESULT num = SendMessage(handle, LB_GETCOUNT, 0, 0);
		Items.clear();
		std::wstring temp;
		for (int i = 0; i < num; i++)
		{
			SendMessage(handle, LB_GETTEXT, i, (LPARAM)buf);
			CItem Item(buf, false);
			Items.push_back(Item);
		}
	}

	int CListBox::GetSelectedIndex(void)
	{
		SelectedIndex = (int)SendMessage(handle, LB_GETCURSEL, 0, 0);
		return(SelectedIndex);
	}

	void CListBox::GetSelectList(void)
	{
		LRESULT num = SendMessage(handle, LB_GETCOUNT, 0, 0);
		if (Items.size() != num)GetText();

		SelectedCount = 0;
		std::wstring temp;
		for (int i = 0; i < num; i++)
		{
			if (SendMessage(handle, LB_GETSEL, i, 0) != 0)
			{
				Items[i].Selected = true;
				SelectedCount++;
			}
			else
				Items[i].Selected = false;
		}
	}

	void CListBox::Set(void)
	{
		SetText();
		SetSelectedIndex();
		SetSelectList();
	}

	void CListBox::SetText(void)
	{
		SendMessage(handle, WM_SETREDRAW, FALSE, 0); // 再描画停止

		SendMessage(handle, LB_RESETCONTENT, 0, 0); // 項目をクリアする
		SendMessage(handle, LB_INITSTORAGE, Items.size(), 0); // ストレージを用意する

		for (auto itr = Items.begin(); itr != Items.end(); itr++)
		{
			SendMessage(handle, LB_ADDSTRING, 0, (LPARAM)itr->Text.c_str());
		}

		SendMessage(handle, WM_SETREDRAW, TRUE, 0);
	}

	void CListBox::SetSelectedIndex(void)
	{
		SelectedIndex = (int)SendMessage(handle, LB_SETCURSEL, SelectedIndex, 0);
	}

	void CListBox::SetSelectList(void)
	{
		SendMessage(handle, WM_SETREDRAW, FALSE, 0); // 再描画停止
		SelectedCount = 0;
		for (int i = 0; i != Items.size(); i++)
		{
			if (Items[i].Selected == true)
			{
				SendMessage(handle, LB_SETSEL, TRUE, i);
				SelectedCount++;
			}
			else
				SendMessage(handle, LB_SETSEL, FALSE, i);
		}
		SendMessage(handle, WM_SETREDRAW, TRUE, 0);
	}

	void CListBox::BeginUpdate(void)
	{
		SendMessage(handle, WM_SETREDRAW, FALSE, 0);
	}

	void CListBox::EndUpdate(void)
	{
		SendMessage(handle, WM_SETREDRAW, TRUE, 0);
	}

	//-----------------------------------------
	//
	// CSlider Class
	//
	//-----------------------------------------

	LRESULT CALLBACK CSliderProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		CSlider *Slider;
		Slider = (CSlider *)GetProp(hWnd, TEXT("THIS"));
		LRESULT result = Slider->ProcessMessages(hWnd, message, wParam, lParam);
		return(result);
	}


	CSlider::CSlider(void)
	{
	}

	CSlider::~CSlider(void)
	{
		RemoveProp(handle, TEXT("THIS"));
	}

	bool CSlider::SetHWND(HWND hWnd)
	{
		if (CBaseWindow::SetHWND(hWnd) == false)return(false);
		OrgProc = (WNDPROC)GetWindowLongPtr(handle, GWLP_WNDPROC);
		SetWindowLongPtr(handle, GWLP_WNDPROC, (LONG_PTR)CSliderProc);
		SetProp(handle, TEXT("THIS"), (HANDLE)this);
		return(true);
	}

	void CSlider::Init(int aMin, int aMax, int aStep, int aValue)
	{
		Min = aMin;
		Max = aMax;
		Step = aStep;
		Value = aValue;

		SendMessage(handle, WM_SETREDRAW, FALSE, 0);
		SendMessage(handle, TBM_SETTICFREQ, Step, 0);
		SendMessage(handle, TBM_SETPAGESIZE, Step, 0);
		SendMessage(handle, TBM_SETRANGE, FALSE, (LPARAM)MAKELPARAM(Min, Max));
		SendMessage(handle, WM_SETREDRAW, TRUE, 0);

		SendMessage(handle, TBM_SETPOS, TRUE, Value);
	}

	void CSlider::SetSliderValue(int src) // 初期化は 値直接代入でいいが、コンポーネントが初期化されたらこちらを使って位置を同期させる
	{
		Value = src;
		if (Value > Max)Value = Max;
		if (Value < Min)Value = Min;
		SendMessage(handle, TBM_SETPOS, TRUE, Value);
	}

	LRESULT CSlider::ProcessMessages(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		LRESULT result = 0;
		static bool MouseDown = false;
		switch (message)
		{
		case WM_MOUSEMOVE:
			if (MouseDown)
				Value = (int)SendMessage(handle, TBM_GETPOS, 0, 0);
			break;
		case WM_LBUTTONUP:
		case WM_KEYUP:
			Value = (int)SendMessage(handle, TBM_GETPOS, 0, 0);
			MouseDown = false;
			break;
		case WM_LBUTTONDOWN:
			MouseDown = true;
			break;
		}
		if (EventProc != nullptr)
			CallWindowProc(EventProc, hWnd, message, wParam, lParam);
		result = CallWindowProc(OrgProc, hWnd, message, wParam, lParam);
		return (result);
	}

	//-----------------------------------------
	//
	// CTimer Class
	//
	//-----------------------------------------
	CTimer::CTimer(void)
	{
		Index = TimerIndex;
		TimerIndex++;
	}

	CTimer::~CTimer(void)
	{
		if (Started)KillTimer(nullptr, Index);
	}

	void CTimer::Enabled(bool aEnable)
	{
		if (aEnable == true)
		{
			if (Started)KillTimer(handle, Index);
			SetTimer(handle, Index, Interval, nullptr);
			Started = true;
		}
		else
		{
			KillTimer(handle, Index);
			Started = false;
		}
	}

	bool CTimer::IsThis(WPARAM i)
	{
		return (i == Index);
	}

	void CTimer::SetInterval(int aInterval)
	{
		Interval = aInterval;
		if (Started == true)
		{
			KillTimer(handle, Index);
			SetTimer(handle, Index, Interval, nullptr);
		}
	}

	void CTimer::Init(HWND hWnd, int aInterval)
	{
		handle = hWnd;
		Interval = aInterval;
	}

	//-----------------------------------------
	//
	// CEditBox Class
	//
	//-----------------------------------------

	LRESULT CALLBACK CEdiBoxProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		CEditBox *EditBox;
		EditBox = (CEditBox *)GetProp(hWnd, TEXT("THIS"));
		LRESULT result = EditBox->ProcessMessages(hWnd, message, wParam, lParam);
		return(result);
	}


	CEditBox::CEditBox(void)
	{
	}

	CEditBox::~CEditBox(void)
	{
	}

	bool CEditBox::SetHWND(HWND hWnd)
	{
		if (CBaseWindow::SetHWND(hWnd) == false)return(false);
		OrgProc = (WNDPROC)GetWindowLongPtr(handle, GWLP_WNDPROC);
		SetWindowLongPtr(handle, GWLP_WNDPROC, (LONG_PTR)CEdiBoxProc);
		SetProp(handle, TEXT("THIS"), (HANDLE)this);
		return(true);
	}

	LRESULT CEditBox::ProcessMessages(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		LRESULT result = 0;
		if (EventProc != nullptr)
			CallWindowProc(EventProc, hWnd, message, wParam, lParam);
		result = CallWindowProc(OrgProc, hWnd, message, wParam, lParam);
		return (result);
	}

	std::wstring CEditBox::GetEditString(void)
	{
		int l = GetWindowTextLength(handle) + 1;
		TCHAR *buf = new TCHAR[l];
		GetWindowText(handle, buf, l);;
		std::wstring str = buf;
		return (str);
	}

}
