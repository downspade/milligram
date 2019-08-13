#include "acfc_VCLSub.h"
#include "math.h"
#include "sys\stat.h"

//------------------------------------------------------------
//
// ファイルの読み書き
//
//------------------------------------------------------------

void Endian(void *Dat, int Size, int Mode)
{
	BYTE BDat[4];
	int i;
	if(Mode == 1)return;
	if(Mode == 2)
	{
		for(i = 0;i < Size;i++)
			BDat[i] = ((BYTE *)Dat)[Size - i - 1];
		for(i = 0;i < Size;i++)
			((BYTE *)Dat)[i] = BDat[i];
	}
}

bool DirExists(UnicodeString Dir)
{
	if(Dir.Length() < 2)return(false);
	if(Dir.Length() == 2)
	{
		if(Dir[2] != L':')return(false);
		if((Dir[1] >= L'a' && Dir[1] <= L'z') || (Dir[1] >= L'A' && Dir[2] <= L'Z'))
		{
			if(GetDriveType(Dir.c_str()) <= 1)return(false);

			BYTE b;
			if(Dir[1] >= L'a')
				b = Dir[1] - L'a' + 1;
			else
				b = Dir[1] - L'A' + 1;

			if(DiskSize(b) <= 0)return(false);
			return(true);
		}
	}
	else
	{
		return(DirectoryExists(Dir));
	}
	return(true);
}

UnicodeString ChangeFolderSymbol(UnicodeString Str)
{
	UnicodeString RStr = Str;;
	int L = RStr.Length();
	int i;
	for(i = 1;i <= L;i++)
	{
		if(RStr[i] == L'\\')RStr[i] = L'/';
	}
	return(RStr);
}

UnicodeString CorrectDirString(UnicodeString Str, bool CanBackSlash)
{
	UnicodeString RStr = L"";
	int L = Str.Length();
	int i;
	for(i = 1;i <= L;i++)
	{
		if( Str[i] != L'/'
		&&  Str[i] != L':'
		&&  Str[i] != L','
		&&  Str[i] != L';'
		&&  Str[i] != L'*'
		&&  Str[i] != L'?'
		&&  Str[i] != L'"'
		&&  Str[i] != L'<'
		&&  Str[i] != L'>'
		&& (Str[i] != L'\\' || CanBackSlash == true)
		&&  Str[i] != L'|')
		{
			RStr = RStr + Str[i];
		}
	}
	return(RStr);
}

UnicodeString CorrectNumericString(UnicodeString Str)
{
	UnicodeString RStr = L"";
	int L = Str.Length();
	int i;
	for(i = 1;i <= L;i++)
	{
		if(Str[i] >= L'0'
		&& Str[i] <= L'9')
		{
			RStr = RStr + Str[i];
		}
	}
	return(RStr);
}

UnicodeString CorrectNumericString(UnicodeString Src, bool CanMinus, bool CanFloat, UnicodeString MaxN, UnicodeString MinN)
{
	UnicodeString Result = L"";
	Src = Src.Trim();
	bool UseMinus, UseFloat;
	int i, Length, NumberLength;
  __int64 IntegerValue;
  float FloatValue;

	UseMinus = false;
	UseFloat = false;
	NumberLength = 0;
	Length = Src.Length();

	for(i = 1;i <= Length;i++)
	{
		while(1)
		{
			if(CanMinus == true && Src[i] == L'-' && UseMinus == false && Result == L"")
			{
				UseMinus = true;
				break;
			}

			if(CanFloat == true && Src[i] == L'.' && UseFloat == false)
			{
				UseFloat = true;
				Result += L".";
				break;
			}

			if(Src[i] >= L'0' && Src[i] <= L'9' && NumberLength < 9)
			{
				Result += (UnicodeString)Src[i];
				NumberLength++;
				break;
			}
			break;
		}
	}

	if(UseMinus == true && Result.Length() == 0)
	{
		Result = L"-";
		return(Result);
	}

	if(Result.Length() > 0)
	{
		if(CanFloat == true)
		{
			float MaxF, MinF, Num, NewNum;
			Num = StrToFloat(Result);
			if(UseMinus == true)Num = -Num;

			NewNum = Num;
			MaxF = StrToFloat(MaxN);
			MinF = StrToFloat(MinN);

			if(Num < MinF)NewNum = MinF;
			if(Num > MaxF)NewNum = MaxF;

			if(NewNum != Num)
			{
				Num = NewNum;
				Result = FloatToStr(Num);
			}
			else if(UseMinus == true)Result = L"-" + Result;
		}
		else
		{
			__int64 MaxI, MinI, Num, NewNum;
			Num = StrToInt64(Result);
			if(UseMinus == true)Num = -Num;

			NewNum = Num;
			MaxI = StrToInt64(MaxN);
			MinI = StrToInt64(MinN);

			if(Num < MinI)NewNum = MinI;
			if(Num > MaxI)NewNum = MaxI;

			if(NewNum != Num)
			{
				Result = IntToStr(Num);
			}
			else if(UseMinus == true)Result = L"-" + Result;
		}
	}
	return(Result);
}

bool CheckMasks(UnicodeString FileName, UnicodeString Mask) // マスクに該当したら true を返す
{
	if(Mask.IsEmpty())
		return(false);

	UnicodeString Temp = Mask, TempMask;
	Temp.Trim();
	int Index, Length, MaskLength = Temp.Length();

	do
	{
		Index = Temp.Pos(L";");
		TempMask = Temp;
		TempMask.Delete(Index, MaskLength);
		if(MatchesMask(FileName, TempMask))return(true);
		if(Index == 0) break;
		Temp.Delete(1, Index);
		Temp.Trim();
		MaskLength = Temp.Length();
	}
	while(!(Temp.IsEmpty()));

	return(false);
}

bool DeleteFolder(UnicodeString Dir)
{
	TSearchRec sr;
	UnicodeString Target, Path;
	int iAttributes;

	Dir = ExcludeTrailingBackslash(Dir);
	if(DirectoryExists(Dir) == false)return(false);
	Path = Dir + L"\\";

	// まずディレクトリを探す
	Target = Path + L"*.*";
	if(FindFirst(Target, faAnyFile, sr) == 0)
	{
		do
		{
			if((sr.Attr & faDirectory) && sr.Name != L"." && sr.Name != L"..")
			{
				UnicodeString NewDir = Path + sr.Name + L"\\";
				if(DeleteFolder(NewDir) == false)return(false);
				RemoveDir(NewDir);
			}
		}
		while(FindNext(sr) == 0);
		FindClose(sr);
	}

	// 次にファイル
	if(FindFirst(Target, faAnyFile, sr) == 0)
	{
		do
		{
			if((sr.Attr & faDirectory) == 0)
			{
				if(DeleteFile(Path + sr.Name) == false)return(false);
			}
		}
		while(FindNext(sr) == 0);
		FindClose(sr);
	}
	RemoveDir(Dir);
	return(true);
}

/*
bool DeleteFolder(UnicodeString Dir)
{
	TSearchRec sr;
	UnicodeString Target, Dorg;
	int iAttributes;

	Dorg = ExcludeTrailingBackslash(Dir);
	if(DirectoryExists(Dorg) == false)return(false);
	Dir = Dorg + "\\";

	// まずディレクトリを探す
	Target = Dir + "*.*";
	if(FindFirst(Target, faAnyFile, sr) == 0)
	{
		do
		{
			if((sr.Attr & faDirectory) && sr.Name != L"." && sr.Name != L"..")
			{
				UnicodeString NewDir = Dir + sr.Name + "\\";
				if(DeleteFolder(NewDir) == false)return(false);
			}
		}
		while(FindNext(sr) == 0);
		FindClose(sr);
	}

	// 次にファイル
	if(FindFirst(Target, faAnyFile, sr) == 0)
	{
		do
		{
			if((sr.Attr & faDirectory) == 0)
			{
				if(DeleteFile(Dir + sr.Name) == false)return(false);
			}
		}
		while(FindNext(sr) == 0);
		FindClose(sr);
	}

	RemoveDir(Dorg);
	return(true);
}
*/

// ファイルを移動する
bool MoveFiles(UnicodeString Src, UnicodeString Dest, UnicodeString Type, bool OverWrite)
{
	TSearchRec sr;
	UnicodeString Target;
	int iAttributes;

	Src = ExcludeTrailingBackslash(Src);
	Dest = ExcludeTrailingBackslash(Dest);
	if(DirectoryExists(Src) == false)return(false);
	ForceDirectories(Dest);
	Src += L"\\";
	Dest += L"\\";

	// まずディレクトリを探す
	Target = Src + L"*.*";
	if(FindFirst(Target, faAnyFile, sr) == 0)
	{
		do
		{
			if((sr.Attr & faDirectory) && sr.Name != L"." && sr.Name != L"..")
			{
				UnicodeString NewSrc = Src + sr.Name + L"\\";
				UnicodeString NewDest = Dest + sr.Name + L"\\";
				if(MoveFiles(NewSrc, NewDest, L"*.*", OverWrite) == false)return(false);
			}
		}
		while(FindNext(sr) == 0);
		FindClose(sr);
	}

	// 次にファイル
	Target = Src + Type;
	if(FindFirst(Target, faAnyFile, sr) == 0)
	{
		do
		{
			if((sr.Attr & faDirectory) == 0)
			{
				if(FileExists(Dest + sr.Name) == true)
				{
					if(OverWrite == true)
					{
						DeleteFile(Dest + sr.Name);
						if(MoveFileW((Src + sr.Name).c_str(), (Dest + sr.Name).c_str()) == false)return(false);
					}
				}
				else
					if(MoveFileW((Src + sr.Name).c_str(), (Dest + sr.Name).c_str()) == false)return(false);
			}
		}
		while(FindNext(sr) == 0);
		FindClose(sr);
	}

	if(RemoveDir(ExcludeTrailingBackslash(Src)) == false)return(false);
	return(true);
}

// ファイルリストを取得する(Escキーで抜けられる)
bool GetFileLists(UnicodeString Src, TStringList *Dest, UnicodeString Mask, int Depth) // c:\windows\dest\*.dll のような形式で Src は指定する
{
	TSearchRec sr;
	bool Esc = false;

	TStringList *TempSL = new TStringList();

	UnicodeString Path = ExtractFilePath(Src);
	TempSL->Add(ExcludeTrailingPathDelimiter(Path));

	if(Depth != 0)
	{
		Depth--;
		UnicodeString Name = ExtractFileName(Src);

		if(FindFirst(Path + "*", faDirectory, sr) == 0)
		{
			do
			{
				if((sr.Attr & faDirectory) && sr.Name != L"." && sr.Name != L"..")
					GetFileLists_(Path + sr.Name + "\\" + Name, Dest, Mask, Depth, TempSL, Esc);
				TempSL->Add(Path + sr.Name);
			}
			while(FindNext(sr) == 0 && Esc == false);
			FindClose(sr);
		}

		if(Esc == false && FindFirst(Path + "*.lnk", faAnyFile, sr) == 0)
		{
			do
			{
				sr.Name = GetFileFromLink(Path + sr.Name);
				if(DirectoryExists(sr.Name) == true && TempSL->IndexOf(sr.Name) < 0)
				{
					GetFileLists_(sr.Name + "\\" + Name, Dest, Mask, Depth, TempSL, Esc);
					TempSL->Add(sr.Name);
				}
			}
			while(FindNext(sr) == 0 && Esc == false);
			FindClose(sr);
		}
	}

	if(Esc == false && FindFirst(Src, faAnyFile, sr) == 0)
	{
		do
		{
			if(GetAsyncKeyState(VK_ESCAPE) & 0x8000)Esc = true;
			else if(!((sr.Attr & faDirectory) || sr.Name == L"." || sr.Name == L".."))
			{
				int Len = sr.Name.Length();
				if(Len > 5)
					if(sr.Name.SubString(Len - 3, 4) == L".lnk")
					{
						sr.Name = GetFileFromLink(Path + sr.Name);
						if(DirectoryExists(sr.Name) == true)continue;

						if(Mask != L"*" && Mask != L"*.*" && CheckMasks(sr.Name, Mask) == false)continue;
						Dest->AddObject(sr.Name, (TObject *)sr.Time);
						continue;
					}

				if(Mask != L"*" && CheckMasks(sr.Name, Mask) == false)continue;

				Dest->AddObject(Path + sr.Name, (TObject *)sr.Time);
			}
		}
		while(FindNext(sr) == 0 && Esc == false);
		FindClose(sr);
	}

	while(GetAsyncKeyState(VK_ESCAPE) & 0x8000);
	delete TempSL;
	return(true);
}

bool GetFileLists_(UnicodeString Src, TStringList *Dest, UnicodeString Mask, int Depth, TStringList *TempSL, bool &Esc)
{
	TSearchRec sr;

	UnicodeString Path = ExtractFilePath(Src);
	UnicodeString Name = ExtractFileName(Src);

	if(Depth != 0)
	{
		Depth--;
		UnicodeString Name = ExtractFileName(Src);

		if(FindFirst(Path + "*", faDirectory, sr) == 0)
		{
			do
			{
				if((sr.Attr & faDirectory) && sr.Name != L"." && sr.Name != L"..")
					GetFileLists_(Path + sr.Name + "\\" + Name, Dest, Mask, Depth, TempSL, Esc);
				TempSL->Add(Path + sr.Name);
			}
			while(FindNext(sr) == 0 && Esc == false);
			FindClose(sr);
		}

		if(FindFirst(Path + "*.lnk", faAnyFile, sr) == 0)
		{
			do
			{
				sr.Name = GetFileFromLink(Path + sr.Name);
				if(DirectoryExists(sr.Name) == true && TempSL->IndexOf(sr.Name) < 0)
				{
					GetFileLists_(sr.Name + "\\" + Name, Dest, Mask, Depth, TempSL, Esc);
					TempSL->Add(sr.Name);
				}
			}
			while(FindNext(sr) == 0 && Esc == false);
			FindClose(sr);
		}
	}

	if(Esc == false && FindFirst(Src, faAnyFile, sr) == 0)
	{
		do
		{
			if(GetAsyncKeyState(VK_ESCAPE) & 0x8000)Esc = true;
			else if(!((sr.Attr & faDirectory) || sr.Name == L"." || sr.Name == L".."))
			{
				int Len = sr.Name.Length();
				if(Len > 5)
					if(sr.Name.SubString(Len - 3, 4) == L".lnk")
					{
						sr.Name = GetFileFromLink(Path + sr.Name);
						if(DirectoryExists(sr.Name) == true)continue;

						if(Mask != L"*" && Mask != L"*.*" && CheckMasks(sr.Name, Mask) == false)continue;
						Dest->AddObject(sr.Name, (TObject *)sr.Time);
						continue;
					}

				if(Mask != L"*" && CheckMasks(sr.Name, Mask) == false)continue;

				Dest->AddObject(Path + sr.Name, (TObject *)sr.Time);
			}
		}
		while(FindNext(sr) == 0 && Esc == false);
		FindClose(sr);
	}
	return(true);
}


// ファイルリストを取得する(Escキーで抜けられる)
bool GetFileListsDialog(UnicodeString Src, TStringList *Dest, UnicodeString Mask, int Depth, TApplication *Application, TLabel *Label) // c:\windows\dest\*.dll のような形式で Src は指定する
{
	TSearchRec sr;
	bool Esc = false;

	TStringList *TempSL = new TStringList();

	UnicodeString Path = ExtractFilePath(Src);
	TempSL->Add(ExcludeTrailingPathDelimiter(Path));

	Label->Caption = Path;
	Application->ProcessMessages();

	if(Depth != 0)
	{
		Depth--;
		UnicodeString Name = ExtractFileName(Src);

		if(FindFirst(Path + "*", faDirectory, sr) == 0)
		{
			do
			{
				if((sr.Attr & faDirectory) && sr.Name != L"." && sr.Name != L"..")
					GetFileListsDialog_(Path + sr.Name + "\\" + Name, Dest, Mask, Depth, TempSL, Esc, Application, Label);
				TempSL->Add(Path + sr.Name);
			}
			while(FindNext(sr) == 0 && Esc == false);
			FindClose(sr);
		}

		if(Esc == false && FindFirst(Path + "*.lnk", faAnyFile, sr) == 0)
		{
			do
			{
				sr.Name = GetFileFromLink(Path + sr.Name);
				if(DirectoryExists(sr.Name) == true && TempSL->IndexOf(sr.Name) < 0)
				{
					GetFileListsDialog_(sr.Name + "\\" + Name, Dest, Mask, Depth, TempSL, Esc, Application, Label);
					TempSL->Add(sr.Name);
				}
			}
			while(FindNext(sr) == 0 && Esc == false);
			FindClose(sr);
		}
	}

	if(Esc == false && FindFirst(Src, faAnyFile, sr) == 0)
	{
		do
		{
			if(GetAsyncKeyState(VK_ESCAPE) & 0x8000)Esc = true;
			else if(!((sr.Attr & faDirectory) || sr.Name == L"." || sr.Name == L".."))
			{
				int Len = sr.Name.Length();
				if(Len > 5)
					if(sr.Name.SubString(Len - 3, 4) == L".lnk")
					{
						sr.Name = GetFileFromLink(Path + sr.Name);
						if(DirectoryExists(sr.Name) == true)continue;

						if(Mask != L"*" && Mask != L"*.*" && CheckMasks(sr.Name, Mask) == false)continue;
						Dest->AddObject(sr.Name, (TObject *)sr.Time);
						continue;
					}

				if(Mask != L"*" && Mask != L"*.*" && CheckMasks(sr.Name, Mask) == false)continue;

				Dest->AddObject(Path + sr.Name, (TObject *)sr.Time);
			}
		}
		while(FindNext(sr) == 0 && Esc == false);
		FindClose(sr);
	}

	while(GetAsyncKeyState(VK_ESCAPE) & 0x8000);
	delete TempSL;
	return(true);
}

bool GetFileListsDialog_(UnicodeString Src, TStringList *Dest, UnicodeString Mask, int Depth, TStringList *TempSL, bool &Esc, TApplication *Application, TLabel *Label)
{
	TSearchRec sr;

	UnicodeString Path = ExtractFilePath(Src);
	UnicodeString Name = ExtractFileName(Src);

	Label->Caption = Path;
	Application->ProcessMessages();

	if(Depth != 0)
	{
		Depth--;
		UnicodeString Name = ExtractFileName(Src);

		if(FindFirst(Path + "*", faDirectory, sr) == 0)
		{
			do
			{
				if((sr.Attr & faDirectory) && sr.Name != L"." && sr.Name != L"..")
					GetFileListsDialog_(Path + sr.Name + "\\" + Name, Dest, Mask, Depth, TempSL, Esc, Application, Label);
				TempSL->Add(Path + sr.Name);
			}
			while(FindNext(sr) == 0 && Esc == false);
			FindClose(sr);
		}

		if(FindFirst(Path + "*.lnk", faAnyFile, sr) == 0)
		{
			do
			{
				sr.Name = GetFileFromLink(Path + sr.Name);
				if(DirectoryExists(sr.Name) == true && TempSL->IndexOf(sr.Name) < 0)
				{
					GetFileListsDialog_(sr.Name + "\\" + Name, Dest, Mask, Depth, TempSL, Esc, Application, Label);
					TempSL->Add(sr.Name);
				}
			}
			while(FindNext(sr) == 0 && Esc == false);
			FindClose(sr);
		}
	}

	if(Esc == false && FindFirst(Src, faAnyFile, sr) == 0)
	{
		do
		{
			if(GetAsyncKeyState(VK_ESCAPE) & 0x8000)Esc = true;
			else if(!((sr.Attr & faDirectory) || sr.Name == L"." || sr.Name == L".."))
			{
				int Len = sr.Name.Length();
				if(Len > 5)
					if(sr.Name.SubString(Len - 3, 4) == L".lnk")
					{
						sr.Name = GetFileFromLink(Path + sr.Name);
						if(DirectoryExists(sr.Name) == true)continue;

						if(Mask != L"*" && Mask != L"*.*" && CheckMasks(sr.Name, Mask) == false)continue;
						Dest->AddObject(sr.Name, (TObject *)sr.Time);
						continue;
					}

				if(Mask != L"*" && Mask != L"*.*" && CheckMasks(sr.Name, Mask) == false)continue;

				Dest->AddObject(Path + sr.Name, (TObject *)sr.Time);
			}
		}
		while(FindNext(sr) == 0 && Esc == false);
		FindClose(sr);
	}
	return(true);
}


// ファイルをコピーする
bool CopyFiles(UnicodeString Src, UnicodeString Dest, UnicodeString Type, bool OverWrite)
{
	TSearchRec sr;
	UnicodeString Target;
	int iAttributes;

	Src = ExcludeTrailingBackslash(Src);
	Dest = ExcludeTrailingBackslash(Dest);
	if(DirectoryExists(Src) == false)return(false);
	ForceDirectories(Dest);
	Src += L"\\";
	Dest += L"\\";

	Target = Src + L"*.*";
	if(FindFirst(Target, faAnyFile, sr) == 0)
	{
		do
		{
			if((sr.Attr & faDirectory) && sr.Name != L"." && sr.Name != L"..")
			{
				UnicodeString NewSrc = Src + sr.Name + L"\\";
				UnicodeString NewDest = Dest + sr.Name + L"\\";
				if(CopyFiles(NewSrc, NewDest, L"*.*", OverWrite) == false)return(false);
			}
		}
		while(FindNext(sr) == 0);
		FindClose(sr);
	}

	Target = Src + Type;
	if(FindFirst(Target, faAnyFile, sr) == 0)
	{
		do
		{
			if((sr.Attr & faDirectory) == 0)
			{
				if(CopyFileW((Src + sr.Name).c_str(), (Dest + sr.Name).c_str(), OverWrite) == 0)return(false);
			}
		}
		while(FindNext(sr) == 0);
		FindClose(sr);
	}
	return(true);
}

// ファイルだけコピーする（フォルダはコピーしない）
bool CopyOnlyFiles(UnicodeString Src, UnicodeString Dest, UnicodeString Type, bool OverWrite)
{
	TSearchRec sr;
	UnicodeString Target;
	int iAttributes;

	Src = ExcludeTrailingBackslash(Src);
	Dest = ExcludeTrailingBackslash(Dest);
	if(DirectoryExists(Src) == false)return(false);
	ForceDirectories(Dest);
	Src += L"\\";
	Dest += L"\\";

	// ファイル
	Target = Src + Type;
	if(FindFirst(Target, faAnyFile, sr) == 0)
	{
		do
		{
			if((sr.Attr & faDirectory) == 0)
			{
				if(CopyFileW((Src + sr.Name).c_str(), (Dest + sr.Name).c_str(), OverWrite) == 0)return(false);
			}
		}
		while(FindNext(sr) == 0);
		FindClose(sr);
	}
	return(true);
}


void CopyFolderFiles(UnicodeString Src, UnicodeString Dest, UnicodeString Type)
{
	TSearchRec sr;
	int iAttributes;
	UnicodeString Path;
	if(DirExists(ExcludeTrailingBackslash(Src)) == false)return;

	// ディレクトリを探す
	Path = Src + L"*.*";
	iAttributes = faAnyFile;
	if(FindFirst(Path, iAttributes, sr) == 0)
	{
		do
		{
			if((sr.Attr & faDirectory) && sr.Name != L"." && sr.Name != L"..")
			{
				ForceDirectories(Dest + sr.Name);
				CopyFolderFiles(Src + sr.Name, L"*.*", Dest + sr.Name);
			}
		}
		while(FindNext(sr) == 0);
		FindClose(sr);
	}

	Path = Src + Type;
	if(FindFirst(Path, iAttributes, sr) == 0)
	{
		do
		{
			if((sr.Attr & faDirectory) == 0)
			{
				CopyFileW((Src + sr.Name).c_str(), (Dest + sr.Name).c_str(), 0);
			}
		}
		while(FindNext(sr) == 0);
		FindClose(sr);
	}
}

// Src フォルダ内のファイルを削除してファイルが無くなればフォルダを削除
void DeleteFolderFiles(UnicodeString Src, UnicodeString Dest, UnicodeString Type)
{
	TSearchRec sr;
	int iAttributes;
	UnicodeString Path;
	if(DirExists(ExcludeTrailingBackslash(Src)) == false)return;

	// ディレクトリを探す
	Path = Src + L"*.*";
	iAttributes = faAnyFile;
	if(FindFirst(Path, iAttributes, sr) == 0)
	{
		do
		{
			if((sr.Attr & faDirectory) && sr.Name != L"." && sr.Name != L"..")
				DeleteFolderFiles(Src + sr.Name, L"*.*", Dest + sr.Name);
		}
		while(FindNext(sr) == 0);
		FindClose(sr);
	}

	Path = Src + Type;
	if(FindFirst(Path, iAttributes, sr) == 0)
	{
		do
		{
			if((sr.Attr & faDirectory) == 0)
				DeleteFile(Dest + sr.Name);
		}
		while(FindNext(sr) == 0);
		FindClose(sr);
	}

	RemoveDir(ExcludeTrailingBackslash(Dest));
}

// Src フォルダにあるファイルのうち Dest フォルダにあるものを Dest フォルダにコピーする
void ReplaceFolderFiles(UnicodeString Src, UnicodeString Dest, UnicodeString Type)
{
	TSearchRec sr;
	int iAttributes;
	UnicodeString Path;
	
	Path = Src + L"*.*";
	if(DirExists(ExcludeTrailingBackslash(Src)) == false)return;

	// ディレクトリを探す
	iAttributes = faAnyFile;
	if(FindFirst(Path, iAttributes, sr) == 0)
	{
		do
		{
			if((sr.Attr & faDirectory) && sr.Name != L"." && sr.Name != L"..")
				ReplaceFolderFiles(Src + sr.Name, L"*.*", Dest + sr.Name);
		}
		while(FindNext(sr) == 0);
		FindClose(sr);
	}

	Path = Src + Type;
	if(FindFirst(Path, iAttributes, sr) == 0)
	{
		do
		{
			if((sr.Attr & faDirectory) == 0)
			{
				if(FileExists(Dest + sr.Name))
					CopyFileW((Src + sr.Name).c_str(), (Dest + sr.Name).c_str(), 0);
			}
		}
		while(FindNext(sr) == 0);
		FindClose(sr);
	}
}

// ３種類のタイムスタンプをすべて指定した値にする
void SetAllTimeStamp(int Year, int Month, int Day, int Hour, int Minute, int Second, int Milliseconds, UnicodeString DestFileName)
{
	SYSTEMTIME  sysTime;
	FILETIME    fTime,
							fLocTime;
	HANDLE      hFile;

	hFile = CreateFileW(DestFileName.c_str(), GENERIC_READ | GENERIC_WRITE,
											0, NULL, OPEN_EXISTING,
											FILE_ATTRIBUTE_NORMAL, NULL);

	if(hFile == INVALID_HANDLE_VALUE)
	{
		return; //ShowMessage( SysErrorMessage( GetLastError() ) );
	}
	else
	{
		sysTime.wYear  = Year;
		sysTime.wMonth  = Month;
		sysTime.wDay    = Day;
		sysTime.wHour          = Hour;
		sysTime.wMinute        = Minute;
		sysTime.wSecond        = Second;
		sysTime.wMilliseconds  = Milliseconds;

		SystemTimeToFileTime( &sysTime, &fLocTime );
		LocalFileTimeToFileTime( &fLocTime, &fTime );
		SetFileTime( hFile, &fTime, &fTime, &fTime );

		CloseHandle( hFile );
	}
}

void SetSameTimeAll(UnicodeString SrcFileName, UnicodeString DestFileName)
{
	FILETIME    fTime1, fTime2, fTime3;
	HANDLE      hSrcFile, hDestFile;

	hSrcFile = CreateFileW(SrcFileName.c_str(), GENERIC_READ | GENERIC_WRITE,
											0, NULL, OPEN_EXISTING,
											FILE_ATTRIBUTE_NORMAL, NULL);

	hDestFile = CreateFileW(DestFileName.c_str(), GENERIC_READ | GENERIC_WRITE,
											0, NULL, OPEN_EXISTING,
											FILE_ATTRIBUTE_NORMAL, NULL);

	if(hSrcFile == INVALID_HANDLE_VALUE)return; //ShowMessage( SysErrorMessage( GetLastError() ) );
	if(hDestFile == INVALID_HANDLE_VALUE)return; //ShowMessage( SysErrorMessage( GetLastError() ) );


	GetFileTime(hSrcFile, &fTime1, &fTime2, &fTime3);
	SetFileTime(hDestFile, &fTime1, &fTime2, &fTime3);

	CloseHandle(hSrcFile);
	CloseHandle(hDestFile);
}

void GetFileTimeStamp(UnicodeString FileName, FILETIME *fRefreshTime)
{
	HANDLE      hSrcFile;
	hSrcFile = CreateFileW(FileName.c_str(), GENERIC_READ | GENERIC_WRITE,
											0, NULL, OPEN_EXISTING,
											FILE_ATTRIBUTE_NORMAL, NULL);

  if(hSrcFile == INVALID_HANDLE_VALUE)return;

	FILETIME fTime1, fTime2;
	GetFileTime(hSrcFile, &fTime1, &fTime2, fRefreshTime);

	CloseHandle(hSrcFile);

	return;
}

// Select a file in explorer
void SelectFileInExplorer(UnicodeString Src)
{
	// This is the command line for explorer which tells it to select the given file
	UnicodeString CommandLine = L"/Select,";
	CommandLine += Src;

	// Prepare shell execution params
	SHELLEXECUTEINFO shExecInfo   = {0};
	shExecInfo.cbSize             = sizeof(shExecInfo);
	shExecInfo.lpFile             = L"Explorer.exe";
	shExecInfo.lpParameters       = CommandLine.c_str();
	shExecInfo.nShow              = SW_SHOWNORMAL;
	shExecInfo.lpVerb             = L"Open"; // Context menu item

	// Just have a look in MSDN to see the relevance of these flags
	shExecInfo.fMask              = SEE_MASK_INVOKEIDLIST | SEE_MASK_FLAG_DDEWAIT | SEE_MASK_FLAG_NO_UI;

	// Select file in explorer
	ShellExecuteExW(&shExecInfo);
}// End SelectFileInExplorer

// ファイルのサイズを取得する
size_t GetFileSize(UnicodeString Target)
{
	struct _stat statbuf;
	_wstat(Target.c_str(), &statbuf);
	return(statbuf.st_size);
}



// 指定したディレクトリの数だけディレクトリ文字列を削除
UnicodeString DeleteDirString(UnicodeString Dir, int Count)
{
	if(Count <= 0)return(Dir);
	Dir = ExcludeTrailingBackslash(Dir);
	int i = Dir.Length();
	if(i == 0)return(L"");
	while(i > 1 && Count > 0)
	{
		if(Dir[i] == L':')
		{
			i++;
			break;
		}
		if(Dir[i] == L'\\' || Dir[i] == L'/')
		{
			Count--;
		}
		i--;
	}
	Dir = Dir.SubString(1, i);
	return(Dir);
}


// 相対パスを絶対パスに変換
// root フォルダから Relative フォルダへの相対を絶対に直す
UnicodeString GetAbsoluteFilePath(UnicodeString Root, UnicodeString Relative)
{
	int i = 1;
	if(Relative.Length() == 0)return(L"");
	if(Relative.Pos(L":") > 0)return(Relative);
	if(Relative.Pos(L"\\\\") > 0)return(Relative);
	if(Relative.Pos(L"//") > 0)return(Relative);

	while(1)
	{
		if(i == Relative.Length())return(Root);
		else if(Relative[i] == L'.')
		{
			int C = 0;
			while(i <= Relative.Length() && Relative[i] == L'.')
			{
				C++;
				i++;
			}
			C--;
			if(C > 0)
			{
				Root = DeleteDirString(Root, C);
			}
		}
		else if(Relative[i] == L'/' || Relative[i] == L'\\')
		{
			while(i <= Relative.Length() &&(Relative[i] == L'/' || Relative[i] == L'\\'))
			{
				i++;
			}
		}
		else break;
	}
	return(IncludeTrailingBackslash(Root) + Relative.SubString(i, Relative.Length() - i + 1));
}

// (1) (2) 形式で重複しないファイル名を取得する
UnicodeString GetNonOverwrapFileName(UnicodeString Src)
{
	UnicodeString Dest = Src, Name, Path, Ext;
	int Count = 1;

	// まず最初のファイル名が (1) 形式かどうかチェック
	Path = ExtractFilePath(Src);
	Ext = ExtractFileExt(Src);
	Name = ChangeFileExt(ExtractFileName(Src), "").Trim();

	if(Name[Name.Length()] == L')')
	{
		int i = Name.Length() - 1;
		while(i > 0)
		{
			if(Name[i] < L'0' || Name[i] > L'9')break;
			i--;
		}

		if(Name[i] == L'(' && i > 1)
		{
			Name = Name.SubString(0, i - 1).Trim();
		}
	}

	Dest = Path + Name + Ext;
	while(FileExists(Dest) == true)
	{
		Dest = Path + Name + L" (" + IntToStr(Count) + L")" + Ext;
		Count++;
	}
	return(Dest);
}


//--------------------------------------------------------
//
// ShellLink が必要
//
//--------------------------------------------------------
#include <shlwapi.h>

UnicodeString GetDropFileName(TMessage &Msg)
{
	return(GetDropFileName(Msg, false));
}

UnicodeString GetDropFileName(TMessage &Msg, bool Original)
{
	UnicodeString Result = L"";
	Msg.Result = 0;
	HANDLE hDrop = (HANDLE)Msg.WParam;
	int nFiles = DragQueryFile(hDrop, -1, NULL, 0);
	wchar_t szBuf[MAX_PATH];

	if(nFiles > 0)
	{
		DragQueryFile(hDrop, 0, szBuf, sizeof(szBuf));
		Result = (UnicodeString)szBuf;
		DragFinish(hDrop);
	}

	if(Original == false)
	{
		int Len = Result.Length();
		if(Len > 5)
			if(Result.SubString(Len - 3, 4) == L".lnk")
				Result = GetFileFromLink(Result);
	}

	return(Result);
}



UnicodeString GetFileFromLink(UnicodeString Path)
{
	UnicodeString ucResult = L"";

	CoInitialize(NULL);

	HRESULT HR;
	IShellLink *psl;
	IPersistFile *ppf;
	wchar_t szGotPath[MAX_PATH];
	wchar_t *Temp;
	WIN32_FIND_DATA wfd;

	HR = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void **)&psl);
	if(SUCCEEDED(HR))
	{
		HR = psl->QueryInterface(IID_IPersistFile, (void **)&ppf);
		if(SUCCEEDED(HR))
		{
			HR = ppf->Load(Path.c_str(), STGM_READ);
			if(SUCCEEDED(HR))
			{
				HR = psl->Resolve(Application->Handle, SLR_ANY_MATCH);
				if(SUCCEEDED(HR))
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

HRESULT CreateLink(UnicodeString OriginalFile, UnicodeString ShortCutFile, UnicodeString Params)
// 第一引数は、ショートカットの元のファイル名
// 第二引数は、ショートカットのファイル名
//（たとえば c:\abc.lnkとか)
{
	HRESULT hres;
	IShellLink* psl = NULL;

	// COMインターフェイスを初期化
	CoInitialize(NULL) ;

	// IShellLink インターフェイスを取得
	hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *)&psl);
	if(SUCCEEDED(hres))
	{
		IPersistFile* ppf;

		// Linkオブジェクトのパスを設定(たとえば、
		// C;\Windows\notepad.exeなど)
		psl->SetPath(OriginalFile.c_str());

		// 引数の設定
		if(Params != "")psl->SetArguments(Params.c_str());

		// IPersistFileインターフェイスを取得し、
		// Linkパスファイル名を保存する。
		// これは、IShellLinkの中の
		// ディスク関連オブジェクトを
		// 取得していると思ってください。
		hres = psl->QueryInterface( IID_IPersistFile, (void**)&ppf);

		if(SUCCEEDED(hres))
		{
			// ディスクに保存する
			hres = ppf->Save(ShortCutFile.c_str(), TRUE);
			ppf->Release();
		}
		psl->Release();
	}
	return hres;
}

UnicodeString AsExpandEnvironmentStrings(UnicodeString Src)
{
	UnicodeString Result = Src;
	wchar_t *Buf;
	DWORD Size;

	Size = ExpandEnvironmentStrings(Src.c_str(), NULL, 0);
	if(Size == 0)return(Result);
	Buf = new wchar_t[Size];
	ExpandEnvironmentStrings(Src.c_str(), Buf, Size);
	Result = Buf;
	delete Buf;
	return(Result);
}

UnicodeString AsPathUnExpandEnvStrings(UnicodeString Src)
{
	UnicodeString Result = Src;
	wchar_t Buf[256];
	PathUnExpandEnvStrings(Src.c_str(), Buf, 256);
	Result = Buf;
	return(Result);
}

bool SyncDir(UnicodeString SrcDir, UnicodeString DestDir, UnicodeString MaskStr, int Option)
{
	SrcDir = IncludeTrailingBackslash(SrcDir);
	DestDir = IncludeTrailingBackslash(DestDir);

	if(DirectoryExists(ExcludeTrailingBackslash(SrcDir)) == false)
		return(false);

	if(DirectoryExists(ExcludeTrailingBackslash(DestDir)) == false)
		ForceDirectories(ExcludeTrailingBackslash(DestDir));

	CheckDirNew(SrcDir, DestDir, MaskStr, Option, L"\\");
	if(Option & ACFC_VCLSUB_DELETEFILES)
		CheckDirDeleted(SrcDir, DestDir, MaskStr, Option, L"\\");

	return(true);
}

bool CheckDirNew(UnicodeString SrcDir, UnicodeString DestDir, UnicodeString MaskStr, int Option, UnicodeString Dir)
{
	int ErCode, Temp;
	TSearchRec SFInfo;
	UnicodeString TempStr;
	UnicodeString DirName;
	UnicodeString CmpStr;
	bool CreateSyncFile;
	UnicodeString CreateFolderName;

	// 最後の \ をとる
	DirName = Dir.SubString(1, Dir.Length() - 1);

	if(FindFirst(SrcDir + Dir + L"*.*", faAnyFile, SFInfo) == 0)
	do
	{
		TempStr = Dir + SFInfo.Name;
		if(SFInfo.Name != "." && SFInfo.Name != L"..")
		{
			if((SFInfo.Attr & faDirectory) != 0)
			{
				if(Option & ACFC_VCLSUB_CHECK_SUBFOLDER)
					CheckDirNew(SrcDir, DestDir, MaskStr, Option, TempStr + L"\\");
			}
			else if(CheckMasks(SFInfo.Name, MaskStr) == ((Option & ACFC_VCLSUB_INCLUDEMASK) != 0))
			{
				CreateSyncFile = false;
				if(FileExists(DestDir + TempStr) == false)CreateSyncFile = true;
				else
				{
					Temp = FileAge(DestDir + TempStr);
					if(
					(Temp < SFInfo.Time)
					|| (Temp > SFInfo.Time && (Option & ACFC_VCLSUB_ONLY_NEWFILES))
					)
					CreateSyncFile = true;
				}

				if(CreateSyncFile == true)
				{
					if(DirectoryExists(DestDir + DirName) == false)
						ForceDirectories(DestDir + DirName);
					CopyFileW((SrcDir + TempStr).c_str(), (DestDir + TempStr).c_str(), false);
				}
			}
		}
	}
	while(FindNext(SFInfo) == 0);
	FindClose(SFInfo);

	return(true);
}

bool CheckDirDeleted(UnicodeString SrcDir, UnicodeString DestDir, UnicodeString MaskStr, int Option, UnicodeString Dir)
{
	int ErCode;
	TSearchRec DFInfo;
	UnicodeString TempStr;

	if(FindFirst(DestDir + Dir + L"*.*", faAnyFile, DFInfo) == 0)
	do
	{
		TempStr = Dir + DFInfo.Name;
		if(DFInfo.Name != L"." && DFInfo.Name != L"..")
		if((DFInfo.Attr & faDirectory) != 0)
		{
			if(Option & ACFC_VCLSUB_CHECK_SUBFOLDER)
				CheckDirDeleted(SrcDir, DestDir, MaskStr, Option, TempStr + "\\");
		}
		else if(CheckMasks(DFInfo.Name, MaskStr) == ((Option & ACFC_VCLSUB_INCLUDEMASK) != 0) == true)
		{
			if(FileExists(SrcDir + TempStr) == false)
			{
				DeleteFile((DestDir + TempStr).c_str());
			}
		}
	}
	while(FindNext(DFInfo) == 0);
	FindClose(DFInfo);

	return(true);
}

int __stdcall acfc_VCLSub_BrowseCallback(HWND hwnd, UINT Msg, LPARAM lParam, LPARAM lpData)
{
	if(Msg == BFFM_INITIALIZED)
	{
		SendMessage(hwnd, BFFM_SETSELECTION, (WPARAM)TRUE, lpData);
	}
	return(0);
}

// ダイアログを表示してフォルダを取得する
UnicodeString GetFolderByDialog(UnicodeString DefaultDir, HWND Handle, UnicodeString Message)
{
	UnicodeString Result;

	BROWSEINFO BI;
	wchar_t DirPath[MAX_PATH];
	wchar_t FName[MAX_PATH];
	LPITEMIDLIST ItemID;

	memset(&BI, 0, sizeof(BROWSEINFO));
	memset(DirPath, 0, MAX_PATH);
	memset(FName, 0, MAX_PATH);

	BI.hwndOwner = Handle;                     // オーナーウィンドウのハンドル
	BI.pidlRoot = NULL;                        // ルートフォルダのIDリスト
	BI.pszDisplayName = FName;                 // 選択フォルダのパス
	BI.lpszTitle = Message.c_str();   // 表示文字
	BI.ulFlags = BIF_RETURNONLYFSDIRS;         // フォルダのみ選択
	BI.lpfn = (BFFCALLBACK)acfc_VCLSub_BrowseCallback;     // コールバック関数
	BI.lParam = (LPARAM)DefaultDir.c_str();    // コールバック関数に渡される値
	BI.iImage = 0;                             // 選択フォルダのイメージリストインデックス

	ItemID = SHBrowseForFolderW(&BI);
	SHGetPathFromIDListW(ItemID, DirPath);
	GlobalFreePtr(ItemID);

	Result = DirPath;
	return(Result);
}

// ファイルをゴミ箱に移動する
bool DeleteFileToRecycle(UnicodeString DelFile, bool Confirm, HWND Handle)
{
	bool Result;
	wchar_t *SrcBuf;
	SHFILEOPSTRUCTW sfs;
	memset(&sfs, 0, sizeof(SHFILEOPSTRUCTW));

	SrcBuf = new wchar_t[DelFile.Length() + 2];
	wcscpy(SrcBuf, DelFile.c_str());
	SrcBuf[DelFile.Length() + 1] = L'\0';

	sfs.fFlags = FOF_NOERRORUI | FOF_SIMPLEPROGRESS | FOF_ALLOWUNDO | FOF_WANTNUKEWARNING;

	if(!Confirm)
		sfs.fFlags |= FOF_NOCONFIRMATION;

	sfs.hwnd  = Handle;
	sfs.wFunc = FO_DELETE;
	sfs.pFrom = SrcBuf;
	sfs.pTo   = NULL;

	Result = (SHFileOperationW(&sfs) == 0);
	delete[] SrcBuf;

	return(Result);
}

// ファイルをクリップボードにコピー（カット、ショートカット作成動作する）
void FilesToClipboard(HWND Handle, TStringList *Src, int Flag) // Flag: DROPEFFECT_COPY, DROPEFFECT_MOVE, DROPEFFECT_LINK
{
	//Unicode で正しく動作しない？
	//CF_HDROPを作成

	DROPFILES dfs;
	ZeroMemory(&dfs, sizeof(DROPFILES));
	dfs.pFiles = sizeof(dfs);
	dfs.fWide = TRUE;

	int Size = 0, i;
	for(i = 0;i < Src->Count;i++)
	{
		Size += Src->Strings[i].Length() + 1;
	}

	Size = sizeof(DROPFILES) + Size * sizeof(wchar_t) + 2;
	HGLOBAL hDrop = GlobalAlloc(GHND, Size);
	BYTE* pDrop = static_cast<BYTE*>(GlobalLock(hDrop));
	memset(pDrop, 0, Size);
	memcpy(pDrop, &dfs, sizeof(DROPFILES));
	pDrop += sizeof(DROPFILES);

	for(i = 0;i < Src->Count;i++)
	{
		wcscpy((wchar_t *)pDrop, Src->Strings[i].c_str());
		pDrop += (Src->Strings[i].Length() + 1) * sizeof(wchar_t);
	}
	GlobalUnlock(hDrop);

	//Preferred DropEffectを作成
	HGLOBAL hDropEffect = GlobalAlloc(GHND, sizeof(DWORD));
	DWORD* pdw = static_cast<DWORD*>(GlobalLock(hDropEffect));
	*pdw = Flag;
	GlobalUnlock(hDropEffect);

	//クリップボードにデーターをセット
	UINT CF_DROPEFFECT = RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT);
	OpenClipboard(Handle);
	EmptyClipboard();
	SetClipboardData(CF_HDROP, hDrop);
	SetClipboardData(CF_DROPEFFECT, hDropEffect);
	CloseClipboard();
}

int FilesFromClipboard(TStringList *DestSL)
{
	if(Clipboard()->HasFormat(CF_HDROP) == 0)return(0);

	int Count = 0, i;

	HDROP hDrop = reinterpret_cast<HDROP>(Clipboard()->GetAsHandle(CF_HDROP));
	int nFiles = DragQueryFile(hDrop, -1, NULL, 0);
	wchar_t szBuf[MAX_PATH];

	if(nFiles > 0)
	{
		for(i = 0;i < nFiles;i++)
		{
			DragQueryFileW(hDrop, i, szBuf, sizeof(szBuf));
			DestSL->Add((UnicodeString)szBuf);
		}
		DragFinish(hDrop);
		Count = nFiles;
	}

	for(i = 0;i < nFiles;i++)
	{
		int Len = DestSL->Strings[i].Length();
		if(Len > 5)
			if(DestSL->Strings[i].SubString(Len - 3, 4) == L".lnk")
				DestSL->Strings[i] = GetFileFromLink(DestSL->Strings[i]);
	}
	return(Count);
}

bool DeleteFileToRecycle(TStringList *DelFiles, bool Confirm)
{
	bool Result;
	wchar_t *SrcBuf, p;
	SHFILEOPSTRUCTW sfs;
	if(DelFiles->Count == 0)return(true);
	memset(&sfs, 0, sizeof(SHFILEOPSTRUCTW));

	int i, L = 0;
	for(i = 0;i < DelFiles->Count;i++)
		L += DelFiles->Strings[i].Length() + 1;
	L++;

	p = 0;
	SrcBuf = new wchar_t[L];
	for(i = 0;i < DelFiles->Count;i++)
	{
		wcscpy(SrcBuf + p, DelFiles->Strings[i].c_str());
		p += DelFiles->Strings[i].Length() + 1;
	}
	SrcBuf[p] = L'\0';

	sfs.fFlags = FOF_NOERRORUI | FOF_SIMPLEPROGRESS | FOF_ALLOWUNDO | FOF_WANTNUKEWARNING;

	if(!Confirm)
		sfs.fFlags |= FOF_NOCONFIRMATION;

	if(DelFiles->Count > 1)
		sfs.fFlags |= FOF_MULTIDESTFILES;

	sfs.hwnd  = NULL;
	sfs.wFunc = FO_DELETE;
	sfs.pFrom = SrcBuf;

	Result = (SHFileOperationW(&sfs) == 0);
	delete[] SrcBuf;

	return(Result);
}



UnicodeString GetIniFolder(void)
{
	UnicodeString Result;
	wchar_t UserFolder[MAX_PATH];
	SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA , NULL, 0, UserFolder);

	Result = (UnicodeString)UserFolder;
	return(Result);
}

UnicodeString GetIniFileName(UnicodeString IniName)
{
	UnicodeString Result = IncludeTrailingPathDelimiter(GetIniFolder()) + IniName;
	return(Result);
}

// ファイル名に使えない文字を得る -1:なし 0~:そのインデックスの文字
int CheckFileIrregularChar(UnicodeString FileName)
{
	int i;
	for(i = 1;i <= FileName.Length();i++)
	{
		if(FileName[i] == L'/'
		|| FileName[i] == L'\\'
		|| FileName[i] == L':'
		|| FileName[i] == L'*'
		|| FileName[i] == L'?'
		|| FileName[i] == L'\''
		|| FileName[i] == L'<'
		|| FileName[i] == L'>'
		|| FileName[i] == L'|'
		)
		{
			return(i);
		}
	}
	return(-1);
}


UnicodeString __fastcall GetShortFileName(UnicodeString Temp, int Count)
{
	if(Count == -1)return(Temp);
	int i, c = Count + 1;
	i = Temp.Length();

	while(i > 1)
	{
		if(Temp[i] == L'\\')
		{
			c--;
			if(c == 0)break;
		}
		i--;
	}

	if(i > 1)i++;
	Temp = Temp.SubString(i, Temp.Length() - i + 1);
	return(Temp);
}


//----------------------------------------------------------------
//
//  その他
//
//----------------------------------------------------------------

bool CheckStrings(TStringList *SrcTS, UnicodeString Dest)
{
	int i = 0;
	return(CheckStrings(SrcTS, Dest, i));
}

bool CheckStrings(TStringList *SrcTS, UnicodeString Dest, int &Pos)
{
	int i;
	for(i = Pos;i < SrcTS->Count;i++)
	{
		if(SrcTS->Strings[i].Pos(Dest) == 1)
		{
			UnicodeString Temp = SrcTS->Strings[i].SubString(Dest.Length() + 1, SrcTS->Strings[i].Length() - Dest.Length());
			Temp = Temp.Trim();
			if(Temp[1] == L'=')
			{
				Pos = i + 1;
				return(true);
			}
		}
	}
	return(false);
}

bool DeleteStringValue(TStringList *SrcTS, UnicodeString Dest, int &Pos)
{
	int i;
	Pos = -1;
	for(i = 0;i < SrcTS->Count;i++)
	{
		if(SrcTS->Strings[i].Pos(Dest) == 1)
		{
			SrcTS->Delete(i);
			Pos = i;
			i--;
		}
	}
	if(Pos >= 0)return(true);
	return(false);
}


UnicodeString CheckStringValue(TStringList *SrcTS, UnicodeString Dest)
{
	int i = 0;
	return(CheckStringValue(SrcTS, Dest, i));
}

UnicodeString CheckStringValue(TStringList *SrcTS, UnicodeString Dest, int &Pos)
{
	int i;
	for(i = Pos;i < SrcTS->Count;i++)
	{
		if(SrcTS->Strings[i].Pos(Dest) == 1)
		{
			UnicodeString Temp = SrcTS->Strings[i].SubString(Dest.Length() + 1, SrcTS->Strings[i].Length() + 1 - Dest.Length());
			Temp = Temp.Trim();
			if(Temp[1] == L'=')
			{
				Temp = Temp.SubString(2, Temp.Length() - 1);
				Temp = Temp.Trim();
				Pos = i + 1;
				return(Temp);
			}
		}
	}
	return(L"");
}

// 数値を取得
int GetIntegerValue(TStringList *SrcTS, UnicodeString Dest, int Src, int Low, int Hi)
{
	int i = 0;
	return(GetIntegerValue(SrcTS, Dest, Src, Low, Hi, i));
}

int GetIntegerValue(TStringList *SrcTS, UnicodeString Dest, int Src, int Low, int Hi, int &Pos)
{
	UnicodeString Temp;
	int Result = Src, Len, By = 1, i;

	Temp = CheckStringValue(SrcTS, Dest, Pos);
	if(Temp == L"")return(Src);
	Len = Temp.Length();

	Result = 0;
	i = 1;
	if(Temp[i] == L'-')
		By = -1, i++;

	while(i <= Len)
	{
		if(Temp[i] >= L'0' && Temp[i] <= L'9')
		{
			Result = Result * 10 + Temp[i] - L'0';
			i++;
		}
		else
			break;
	}
	Result *= By;

	if(Low != Hi)
	{
		if(Result < Low)Result = Low;
		if(Result > Hi)Result = Hi;
	}

	return(Result);
}

// 数値を取得
double GetDoubleValue(TStringList *SrcTS, UnicodeString Dest, double Src, double Low, double Hi)
{
	int i = 0;
	return(GetDoubleValue(SrcTS, Dest, Src, Low, Hi, i));
}

double GetDoubleValue(TStringList *SrcTS, UnicodeString Dest, double Src, double Low, double Hi, int &Pos)
{
	UnicodeString Temp;
	double Result = Src;

	Temp = CheckStringValue(SrcTS, Dest);
	if(Temp == L"")return(Src);
	Result = 0;

	Result = StrToFloat(Temp);

	if(Low != Hi)
	{
		if(Result > Low)Result = Low;
		if(Result > Hi)Result = Hi;
	}

	return(Result);
}
// Bool 値を取得
bool GetBoolValue(TStringList *SrcTS, UnicodeString Dest, bool Src)
{
	int i = 0;
	return(GetBoolValue(SrcTS, Dest, Src, i));
}

bool GetBoolValue(TStringList *SrcTS, UnicodeString Dest, bool Src, int &Pos)
{
	UnicodeString Temp;
	bool Result = Src;

	Temp = CheckStringValue(SrcTS, Dest);
	if(Temp == L"" || Temp.Length() < 4)return(Src);

	if(Temp.SubString(1, 4).UpperCase() == L"TRUE")
		Result = true;
	else
		Result = false;

	return(Result);
}

// 文字列を取得
UnicodeString GetStringValue(TStringList *SrcTS, UnicodeString Dest, UnicodeString Src)
{
	int i = 0;
	return(GetStringValue(SrcTS, Dest, Src, i));
}

UnicodeString GetStringValue(TStringList *SrcTS, UnicodeString Dest, UnicodeString Src, int &Pos)
{
	UnicodeString Temp;

	Temp = CheckStringValue(SrcTS, Dest, Pos);
	if(Temp == L"")return(Src);
	return(Temp);
}


// Dest 文字が SrcTS 文字郡内にあるかどうかチェックする
bool CheckTabStrings(UnicodeString SrcUS, UnicodeString Dest)
{
	Dest = Dest + L"=";
	if(SrcUS.SubString(1, Dest.Length()) == Dest)return(true);
	Dest = L"\t" + Dest;
	if(SrcUS.Pos(Dest) > 0)return(true);
	return(false);
}

UnicodeString CheckTabStringValue(UnicodeString SrcUS, UnicodeString Dest)
{
	Dest = Dest + L"=";
	UnicodeString Result;
	int P = 0, i, l = Dest.Length(), L = SrcUS.Length();
	if(SrcUS.SubString(1, l) != Dest)
	{
		P = SrcUS.Pos(Dest);
		if(P == 0)return("");
	}
	else
		P = 1;

	for(i = P + l;i <= L;i++)
	{
		if(SrcUS[i] == L'\t')break;
	}
	Result = SrcUS.SubString(P + l, i - P - l).Trim();
	return(Result);
}


// 数値を取得
int GetTabIntegerValue(UnicodeString SrcUS, UnicodeString Dest, int Src, int Low, int Hi)
{
	UnicodeString Temp;
	int Result = Src, Len, By = 1, i;

	Temp = CheckTabStringValue(SrcUS, Dest);
	if(Temp == L"")return(Src);
	Len = Temp.Length();

	Result = 0;
	i = 1;
	if(Temp[i] == L'-')
		By = -1, i++;

	while(i <= Len)
	{
		if(Temp[i] >= L'0' && Temp[i] <= L'9')
		{
			Result = Result * 10 + Temp[i] - L'0';
			i++;
		}
		else
    	break;
	}
	Result *= By;

	if(Low != Hi)
	{
		if(Result < Low)Result = Low;
		if(Result > Hi)Result = Hi;
  }

	return(Result);
}

// 数値を取得
double GetTabDoubleValue(UnicodeString SrcUS, UnicodeString Dest, double Src, double Low, double Hi)
{
	UnicodeString Temp;
	double Result = Src;

	Temp = CheckTabStringValue(SrcUS, Dest);
	if(Temp == L"")return(Src);
	Result = 0;

  Result = StrToFloat(Temp);

	if(Low != Hi)
	{
		if(Result > Low)Result = Low;
		if(Result > Hi)Result = Hi;
	}

	return(Result);
}
// Bool 値を取得
bool GetTabBoolValue(UnicodeString SrcUS, UnicodeString Dest, bool Src)
{
	UnicodeString Temp;
	bool Result = Src;

	Temp = CheckTabStringValue(SrcUS, Dest);
	if(Temp == L"" || Temp.Length() < 4)return(Src);

	if(Temp.SubString(1, 4).UpperCase() == L"TRUE")
		Result = true;
	else
		Result = false;

	return(Result);
}

// 文字列を取得
UnicodeString GetTabStringValue(UnicodeString SrcUS, UnicodeString Dest, UnicodeString Src)
{
	UnicodeString Temp;

	Temp = CheckTabStringValue(SrcUS, Dest);
	if(Temp == L"")return(Src);
	return(Temp);
}





/*
bool DeleteFolder(UnicodeString Dir)
{
	TSearchRec sr;
	UnicodeString Target;
	int iAttributes;

	Dir = ExcludeTrailingBackslash(Dir);
	if(DirectoryExists(Dir) == false)return(false);
	Dir += "\\";

	// まずディレクトリを探す
	Target = Dir + "*.*";
	if(FindFirst(Target, faAnyFile, sr) == 0)
	{
		do
		{
			if((sr.Attr & faDirectory) && sr.Name != "." && sr.Name != "..")
			{
				UnicodeString NewDir = Dir + sr.Name + "\\";
				if(DeleteFolder(NewDir) == false)return(false);
			}
		}
		while(FindNext(sr) == 0);
		FindClose(sr);
	}

	// 次にファイル
	if(FindFirst(Target, faAnyFile, sr) == 0)
	{
		do
		{
			if((sr.Attr & faDirectory) == 0)
			{
				if(DeleteFile(Dir + sr.Name) == false)return(false);
			}
		}
		while(FindNext(sr) == 0);
		FindClose(sr);
	}
	return(true);
}

*/

 UnicodeString IntToMemorySize(__int64 tSize)
{
	UnicodeString RStr;
	UnicodeString SizeName[5] = {L" Byte", L" KB", L" MB", L" GB", L" TB"};
	__int64 Keta[5] = {1,
										 1024,
										 1024 * 1024,
										 1024 * 1024 * 1024,
										 1024 * 1024 * 1024 * 1024,
										 };
	__int64 T = tSize, M;
	int i = 0;
	while(T >= 1024 && i < 4)
	{
		T /= 1024;
		i++;
	}

	RStr = IntToStr(T);
	if(i < 10 && i > 0)
	{
		M = tSize - T * Keta[i];
		T = M / Keta[i - 1] % 102;
		M = M / Keta[i - 1] / 102;
		if(T > 50)M++;
		RStr += L"." + IntToStr(M);
	}
	RStr += SizeName[i];
	return(RStr);
}

UnicodeString IntToDigitStr(int Value, int Digit)
{
	wchar_t Dest[20];
	Dest[Digit] = L'\0';
	while(Value > 0 && Digit > 0)
	{
		Digit--;
		Dest[Digit] = L'0' + (Value % 10);
		Value /= 10;
	}
	return((UnicodeString)Dest);
}

void SetAbsoluteForegroundWindow(HWND hWnd)
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

	// 設定を元に戻す
	SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, &sp_time, 0);

	// スレッドのインプット状態を切り離す
	AttachThreadInput(nTargetID, nForegroundID, FALSE);  // FALSE で切り離し
}








//----------------------------------------------------------------
//
// CDragDrop ドラッグドロップ関係
//
//----------------------------------------------------------------

void __fastcall CDragDrop::Init(TWinControl *Control)
{
	// ポインタを保存しておく
	oldWinProc = Control->WindowProc;
	// ドラッグドロップの受け入れ
	DragAcceptFiles(Control->Handle, true);
}
//---------------------------------------------------------------------------

void __fastcall CDragDrop::Release(TWinControl *Control)
{
	Control->WindowProc = oldWinProc;
	DragAcceptFiles(Control->Handle, false);
}
//---------------------------------------------------------------------------

UnicodeString __fastcall CDragDrop::GetDropFileName(TMessage &Msg)
{
	return(GetDropFileName(Msg, false));
}

UnicodeString __fastcall CDragDrop::GetDropFileName(TMessage &Msg, bool Original)
{
	UnicodeString Result = L"";
	Msg.Result = 0;
	HANDLE hDrop = (HANDLE)Msg.WParam;
	int nFiles = DragQueryFile(hDrop, -1, NULL, 0);
	wchar_t szBuf[MAX_PATH];

	if(nFiles > 0)
	{
		DragQueryFileW(hDrop, 0, szBuf, MAX_PATH);
		Result = (UnicodeString)szBuf;
		DragFinish(hDrop);
	}

	if(Original == false)
	{
		int Len = Result.Length();
		if(Len > 5)
			if(Result.SubString(Len - 3, 4) == L".lnk")
				Result = GetFileFromLink(Result);
  }

	return(Result);
}
//---------------------------------------------------------------------------

int __fastcall CDragDrop::GetDropFileNames(TMessage &Msg, TStringList *TempSL)
{
	GetDropFileNames(Msg, TempSL, false);
	return(0);
}

int __fastcall CDragDrop::GetDropFileNames(TMessage &Msg, TStringList *TempSL, bool Original)
{
	int Count = 0, i;

	Msg.Result = 0;
	HANDLE hDrop = (HANDLE)Msg.WParam;
	int nFiles = DragQueryFile(hDrop, -1, NULL, 0);
	wchar_t szBuf[MAX_PATH];

	if(nFiles > 0)
	{
		for(i = 0;i < nFiles;i++)
		{
			DragQueryFileW(hDrop, i, szBuf, sizeof(szBuf));
			TempSL->Add((UnicodeString)szBuf);
		}
		DragFinish(hDrop);
		Count = nFiles;
	}

	if(Original == false)
	{
		for(i = 0;i < nFiles;i++)
		{
			int Len = TempSL->Strings[i].Length();
			if(Len > 5)
				if(TempSL->Strings[i].SubString(Len - 3, 4) == L".lnk")
					TempSL->Strings[i] = GetFileFromLink(TempSL->Strings[i]);
		}
	}
	return(Count);
}
//---------------------------------------------------------------------------

UnicodeString __fastcall CDragDrop::GetFileFromLink(UnicodeString Path)
{
	UnicodeString asResult = L"";

	CoInitialize(NULL);

	HRESULT HR;
	IShellLinkW *psl;
	IPersistFile *ppf;
	wchar_t szGotPath[MAX_PATH];
	WIN32_FIND_DATAW wfd;

	HR = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void **)&psl);
	if(SUCCEEDED(HR))
	{
		HR = psl->QueryInterface(IID_IPersistFile, (void **)&ppf);
		if(SUCCEEDED(HR))
		{
			HR = ppf->Load(Path.c_str(), STGM_READ);
			if(SUCCEEDED(HR))
			{
				HR = psl->Resolve(Application->Handle, SLR_ANY_MATCH);
				if(SUCCEEDED(HR))
				{
					HR = psl->GetPath(szGotPath, MAX_PATH, (WIN32_FIND_DATAW *)&wfd, SLGP_UNCPRIORITY);
					asResult = szGotPath;
				}
			}
			ppf->Release();
		}
		psl->Release();
	}
	CoUninitialize();
	return(asResult);
}
//---------------------------------------------------------------------------






//----------------------------------------------------------------
//
// CCriticalSection 排他処理
//
//----------------------------------------------------------------

CCriticalSection::CCriticalSection(void)
{
	InitializeCriticalSection(&section);
}

CCriticalSection::~CCriticalSection(void)
{
	DeleteCriticalSection(&section);
}

bool CCriticalSection::Enter(void)
{
	EnterCriticalSection(&section);
	return(true);
}

bool CCriticalSection::Leave(void)
{
	LeaveCriticalSection(&section);
	return(true);
}

//----------------------------------------------------------------
//
// CBinaryBlock 文字列をバイナリのデータに変換する
//
//----------------------------------------------------------------


CBinaryBlock::CBinaryBlock(void)
{
	BinaryData = NULL;
}

CBinaryBlock::~CBinaryBlock(void)
{
	if(BinaryData != NULL)
	{
		delete[] BinaryData;
	}
	BinaryData = NULL;
}

bool CBinaryBlock::StrToBin(UnicodeString SrcData)
{
	int Dummy;
	StrToBin(SrcData, Dummy);
	return(true);
}

bool CBinaryBlock::StrToBin(UnicodeString SrcData, int &TempSize)
{
	size_t Size;
	UnicodeString TempString;

	if(BinaryData != NULL)
	{
		delete[] BinaryData;
	}

	Size = GetDataSize(SrcData);
	BinaryData = new wchar_t[Size];
	SetData(SrcData);
	TempSize = Size;
	return(true);
}

size_t CBinaryBlock::GetDataSize(UnicodeString Data)
{
	int i = 1, j, Length;
	size_t DataSize = 0;
	Length = Data.Length();

	while(1)
	{
		if(i > Length)break;

		switch(Data[i])
		{
			case L'l':
			case L'L':
				{
					DataSize += sizeof(long);
					while(Data[i] != L',')i++;
				}
				break;
			case L's':
			case L'S':
				{
					DataSize += sizeof(short int);
					while(Data[i] != L',')i++;
				}
				break;
			case L'c':
			case L'C':
				{
					DataSize += sizeof(wchar_t);
					while(Data[i] != L',')i++;
				}
				break;
			case L'f':
			case L'F':
				{
					DataSize += sizeof(float);
					while(Data[i] != L',')i++;
				}
				break;
			case L'm':
			case L'M':
				{
					while(Data[i] != L'\'')i++;i++;j = i;
					while(Data[i] != L'\'')i++;i++;
					j = i - j;
					DataSize += j;
					while(Data[i] != L',')i++;
				}
				break;
			case L'i':
			case L'I':
				{
					while(Data[i] != L'\'')i++;i++;j = i;
					while(Data[i] != L'\'')i++;i++;
					j = i - j;
					j = ((j + 1) / 2) * 2;
					DataSize += j + sizeof(short int);
					while(Data[i] != L',')i++;
				}
				break;
			case L'e':
			case L'E':
				return(DataSize);

			default:
				break;
		}
		i++;
	}
	return(DataSize);
}

void CBinaryBlock::SetData(UnicodeString Data)
{
	void *p = BinaryData;

	int i = 1, j, k, l, NumData, Length;
	float NumDataF;
	Length = Data.Length();
	UnicodeString TempString;

	while(1)
	{
		if(i > Length)break;
		j = i;

		switch(Data[i])
		{
			case L'l':
			case L'L':
				{
					while(Data[i] != L',')i++;
					TempString = Data.SubString(j + 1, i - j - 1);
					TempString = TempString.Trim();
					NumData = TempString.ToInt();
					*(long *)p = (long)NumData;
					((long *)p)++;
				}
				break;
			case L's':
			case L'S':
				{
					while(Data[i] != L',')i++;
					TempString = Data.SubString(j + 1, i - j - 1);
					TempString = TempString.Trim();
					NumData = TempString.ToInt();
					*(short int *)p = (short int)NumData;
					((short int *)p)++;
				}
				break;
			case L'c':
			case L'C':
				{
					while(Data[i] != L',')i++;
					TempString = Data.SubString(j + 1, i - j - 1);
					TempString = TempString.Trim();
					NumData = TempString.ToInt();
					*(wchar_t *)p = (wchar_t)NumData;
					((wchar_t *)p)++;
				}
				break;
			case L'f':
			case L'F':
				{
					while(Data[i] != L',')i++;
					TempString = Data.SubString(j + 1, i - j - 1);
					TempString = TempString.Trim();
					NumDataF = StrToFloat(TempString);
					*(float *)p = (float)NumDataF;
					((float *)p)++;
				}
				break;
			case L'm':
			case L'M':
				{
					while(Data[i] != L'\'')i++;i++;j = i;
					while(Data[i] != L'\'')i++;i++;

					for(;j < i - 1;j++)
					{
						*(wchar_t *)p = Data[j];
						((wchar_t *)p)++;
					}
					*(wchar_t *)p = 0;
					((wchar_t *)p)++;

					while(Data[i] != L',')i++;
				}
				break;
			case L'i':
			case L'I':
				{
					while(Data[i] != L'\'')i++;i++;j = k = i;
					while(Data[i] != L'\'')i++;i++;

					k = i - j;
					l = (k + 1) / 2;

					*(short int *)p = (short int)l;
					((short int *)p)++;

					for(;j < i - 1;j++)
					{
						*(wchar_t *)p = Data[j];
						((wchar_t *)p)++;
					}
					*(wchar_t *)p = 0;
					((wchar_t *)p)++;
					if(k % 2 > 0)
					{
						*(wchar_t *)p = 0;
						((wchar_t *)p)++;
					}
					while(Data[i] != L',')i++;
				}
				break;
			case L'e':
			case L'E':
				return;

			default:
				break;
		}
		i++;
	}
	return;
}


//---------------------------------------------------------------------------
//  レジストリアクセスクラス
//---------------------------------------------------------------------------
//  コンストラクタ
CReg::CReg()
{
	FRootKey = HKEY_CURRENT_USER;
	FOpened = false;
}

//-----------------------------------------------------------------------------
//  デストラクタ
CReg::~CReg()
{
	CloseKey();
}

//-----------------------------------------------------------------------------
//  キーの作成
bool CReg::CreateKey(const UnicodeString KeyName)
{
	DWORD rc;
	int rc2;

	CloseKey();
	rc2 = RegCreateKeyExW(FRootKey,
												KeyName.c_str(),
												0,
												0,
												REG_OPTION_NON_VOLATILE,
												KEY_ALL_ACCESS,
												0,
												&FHKey,
												&rc);

	FOpened = (rc2 == ERROR_SUCCESS);
	return(FOpened);
}

//-----------------------------------------------------------------------------
//  キーのオープン( IsCreate==trueなら開けない場合に作成します)
bool CReg::OpenKey(const UnicodeString KeyName, bool IsCreate)
{
	bool Result;

	CloseKey();
	Result = (RegOpenKeyExW(FRootKey, KeyName.c_str(), 0, KEY_ALL_ACCESS, &FHKey) == ERROR_SUCCESS);
	FOpened = Result;

	if(!Result && IsCreate)
		Result = CreateKey(KeyName);

	return(Result);
}


//-----------------------------------------------------------------------------
//  キーを閉じる
void CReg::CloseKey()
{
	if(FOpened)
		RegCloseKey(FHKey);
	FOpened = false;
}

//-----------------------------------------------------------------------------
//  キーを削除する
bool CReg::DeleteKey(const UnicodeString Key)
{
	DWORD dwRtn;
	if(OpenKey("", false) == false)return(false);
	dwRtn = RegDeleteKeyW(FHKey, Key.c_str());
	CloseKey();
	return(dwRtn == ERROR_SUCCESS);
}

#define MAX_KEY_LENGTH 256
DWORD CReg::RegDeleteKeyNT(HKEY hStartKey, LPTSTR pKeyName)
{
	DWORD   dwRtn, dwLength;
	WCHAR   szStr[MAX_KEY_LENGTH]; // (256) this should be dynamic.
	HKEY    hKey;

	// Do not allow NULL or empty key name
	if (pKeyName && lstrlen(pKeyName))
	{
		if((dwRtn = RegOpenKeyEx(hStartKey, pKeyName,
				0, KEY_ENUMERATE_SUB_KEYS | DELETE, &hKey )) == ERROR_SUCCESS)
		{
			while(dwRtn == ERROR_SUCCESS)
			{
				dwLength = MAX_KEY_LENGTH;
				dwRtn = RegEnumKeyEx(
														 hKey,
														 0,       // always index zero
														 szStr,
														 &dwLength,
														 NULL,
														 NULL,
														 NULL,
														 NULL
														 );

				if(dwRtn == ERROR_NO_MORE_ITEMS)
				{
					dwRtn = RegDeleteKey(hStartKey, pKeyName);
					break;
				}
				else if(dwRtn == ERROR_SUCCESS)
					dwRtn = RegDeleteKeyNT(hKey, szStr);
			}
			RegCloseKey(hKey);
			// Do not save return code because error
			// has already occurred
		}
	}
	else
		dwRtn = ERROR_BADKEY;

	return(dwRtn);
}
#undef MAX_KEY_LENGTH

//-----------------------------------------------------------------------------
//  boolの読み書き
void CReg::WriteBool(const UnicodeString Name, bool Flag)
{
	RegSetValueExW(FHKey,Name.c_str(),0,REG_DWORD,(BYTE*)&Flag,sizeof(DWORD));
}

bool CReg::ReadBool(const UnicodeString Name, bool Def)
{
	bool Result;
	DWORD Size,Mode;

	Size = sizeof(DWORD);
	Mode = REG_DWORD;

	if(RegQueryValueExW(FHKey,Name.c_str(), 0, &Mode, (BYTE*)&Result, &Size) != ERROR_SUCCESS)
		Result = Def;

	return(Result);
}

//-----------------------------------------------------------------------------
//  Integerの読み書き
void CReg::WriteInteger(const UnicodeString Name, int Value)
{
	RegSetValueExW(FHKey,Name.c_str(),0,REG_DWORD,(const BYTE*)&Value,sizeof(DWORD));
}

int CReg::ReadInteger(const UnicodeString Name,int Def)
{
	DWORD Result = 0;
	DWORD Size,Mode;

	Size = sizeof(DWORD);
		Mode = REG_DWORD;

	if(RegQueryValueExW(FHKey, Name.c_str(), 0, &Mode, (BYTE*)&Result, &Size) != ERROR_SUCCESS)
			Result = Def;

	return((int)Result);
}

//-----------------------------------------------------------------------------
//  Integerの読み書き
void CReg::WriteString(const UnicodeString Name, const UnicodeString Str)
{
	DWORD Size;

	Size = Str.Length() + 1;
	RegSetValueExW(FHKey,Name.c_str(), 0, REG_SZ, (BYTE*)Str.c_str(), Size);
}

UnicodeString CReg::ReadString(const UnicodeString Name, const UnicodeString Def)
{
	UnicodeString Result;

	DWORD Mode,Size;
	BYTE tmp[255];

	Mode = REG_SZ;
	Size = sizeof(tmp);

	if(RegQueryValueExW(FHKey,Name.c_str(), 0, &Mode, tmp, &Size) != ERROR_SUCCESS)
	{
		Result = Def;
	}
	else
	{
		Result = (wchar_t *)tmp;
	}

	return(Result);
}

//-----------------------------------------------------------------------------
//  Binaryの読み書き
void CReg::WriteData(const UnicodeString Name, const void *Data, int Size)
{
	RegSetValueExW(FHKey, Name.c_str(), 0, REG_BINARY, (const BYTE*)Data, Size);
}

bool CReg::ReadData(const UnicodeString Name, void *Buff, int Size, const void *Def)
{
	bool Result;
	DWORD Mode;

	Mode = REG_BINARY;

	if(RegQueryValueExW(FHKey, Name.c_str(), 0, &Mode, (BYTE*)Buff, (DWORD*)&Size) != ERROR_SUCCESS)
	{
		memcpy(Buff,Def,Size);
		Result = false;
	}
	else
		Result = true;

	return(Result);
}

//-----------------------------------------------------------------------------
//  データの削除
bool CReg::DeleteValue(const UnicodeString Name)
{
	return(RegDeleteValueW(FHKey, Name.c_str()) == ERROR_SUCCESS);
}


//----------------------------------------------------------------
//
//  Unicode 対応画像クラス
//
//----------------------------------------------------------------

__fastcall TUCFileStream::TUCFileStream(UnicodeString FileName, DWORD AccessMode = GENERIC_READ, DWORD ShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE)
: THandleStream(
	(int)CreateFileW(
		FileName.c_str(),   	 //ファイル名
		AccessMode,            //アクセスモード
		ShareMode,             //共有モード
		NULL,                   //セキュリティ
		(AccessMode & GENERIC_WRITE) != NULL ? OPEN_ALWAYS : OPEN_EXISTING,          //作成方法
		FILE_ATTRIBUTE_NORMAL, //ファイル属性
		0                      //テンプレート
	)
){}

__fastcall TUCFileStream::~TUCFileStream(void)
{
}

void __fastcall TUCBitmap::LoadFromFile(const UnicodeString FileName)
{
	TStream *Stream = new TUCFileStream(FileName);
	LoadFromStream(Stream);
	delete Stream;
}

void __fastcall TUCBitmap::SaveToFile(const UnicodeString FileName)
{
	TStream *Stream = new TUCFileStream(FileName);
	SaveToStream(Stream);
	delete Stream;
}

void __fastcall TUCJPEGImage::LoadFromFile(const UnicodeString FileName)
{
	TStream *Stream = new TUCFileStream(FileName);
	LoadFromStream(Stream);
	delete Stream;
}

void __fastcall TUCJPEGImage::SaveToFile(const UnicodeString FileName)
{
	TStream *Stream = new TUCFileStream(FileName);
	SaveToStream(Stream);
	delete Stream;
}


void __fastcall TUCPngImage::LoadFromFile(const UnicodeString FileName)
{
	TStream *Stream = new TUCFileStream(FileName);
	LoadFromStream(Stream);
	delete Stream;
}

void __fastcall TUCPngImage::SaveToFile(const UnicodeString FileName)
{
	TStream *Stream = new TUCFileStream(FileName);
	SaveToStream(Stream);
	delete Stream;
}

void __fastcall TUCGIFImage::LoadFromFile(const UnicodeString FileName)
{
	TStream *Stream = new TUCFileStream(FileName);
	LoadFromStream(Stream);
	delete Stream;
}

void __fastcall TUCGIFImage::SaveToFile(const UnicodeString FileName)
{
	TStream *Stream = new TUCFileStream(FileName);
	SaveToStream(Stream);
	delete Stream;
}


//---------------------------------------------------------------------------
//
// 注意：異なるスレッドが所有する VCL のメソッド/関数/プロパティを
//       別のスレッドの中から扱う場合，排他処理の問題が発生します。
//
//       メインスレッドの所有するオブジェクトに対しては Synchronize
//       メソッドを使う事ができます。他のオブジェクトを参照するため
//       のメソッドをスレッドクラスに追加し，Synchronize メソッドの
//       引数として渡します。
//
//       たとえば UpdateCaption を以下のように定義し、
//
//          void __fastcall Unit1::UpdateCaption()
//          {
//            Form1->Caption = "スレッドから書き換えました";
//          }
//
//       Execute メソッドの中で Synchronize メソッドに渡すことでメイ
//       ンスレッドが所有する Form1 の Caption プロパティを安全に変
//       更できます。
//
//          Synchronize(UpdateCaption);
//
//---------------------------------------------------------------------------

/*
__fastcall CCheckFileChanged::CCheckFileChanged(bool CreateSuspended)
	: TThread(CreateSuspended)
{
	FileName = L"";
	hNotify[0] = NULL;
	hNotify[1] = NULL;
	FileAgeValue = 0;
	Form = NULL;
	MessageValue = 0;
	Changed = false;
	Priority = tpLowest;

	// 指定フォルダ（サブフォルダも含む）内のフォルダの変更を監視
	//-----------------------------------------------------------------------------------------------------
	hNotify[1] = CreateEventW(NULL, TRUE, FALSE, L"ACFCVCLSUB_CCHECKFILECHANGED"); // 調べるファイルが変更された
	//
	// ↑これが必ず失敗する。C++Builder のバグのようだ
	//
	//-----------------------------------------------------------------------------------------------------

	DWORD Err= GetLastError();
	DWORD SSS=Err;
}

__fastcall CCheckFileChanged::~CCheckFileChanged(void)
{
	FindCloseChangeNotification(hNotify[0]);//監視の終了
	Terminate();
}

//---------------------------------------------------------------------------

void __fastcall CCheckFileChanged::Execute()
{
	// スレッドとして実行したいコードをここに記述
	UnicodeString DirName = ExtractFileDrive(FileName);

	while(1)
	{
		if(hNotify[0] == NULL)
		{
			DWORD Result = WaitForSingleObject(hNotify[1], INFINITE);
			if(Terminated == true)return;

			// 新しいファイルを調べる
			DirName = ExtractFileDrive(FileName);
			hNotify[0] = FindFirstChangeNotificationW(DirName.c_str(), FALSE, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE);
			ResetEvent(hNotify[1]);
		}
		else
		{
			DWORD Result = WaitForMultipleObjects(2, hNotify, FALSE, INFINITE);
			if(Terminated == true)return;

			Result -= WAIT_OBJECT_0;

			if(FileExists(FileName) == false || FileAge(FileName) != FileAgeValue || Result == 1)
			{
				if(FileName != L"" && Result == 0)
				{
					if(MessageValue != 0)SendMessage(Form->Handle, MessageValue, 0, 0);
					Changed = true;
				}

				Suspend();

				FindCloseChangeNotification(hNotify);//監視の終了

				// 新しいファイルを調べる
				DirName = ExtractFileDrive(FileName);
				hNotify[0] = FindFirstChangeNotificationW(DirName.c_str(), FALSE, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE);
				ResetEvent(hNotify[0]);
				ResetEvent(hNotify[1]);
			}
			else
				FindNextChangeNotification(hNotify);//再度監視を行う
		}
	}
}
//---------------------------------------------------------------------------

bool __fastcall CCheckFileChanged::SetMessageParameter(TForm *aForm, UnicodeString aMessageString)
{
	MessageValue = RegisterWindowMessage(aMessageString.c_str());
	Form = aForm;
	if(MessageValue == 0)return(false);
	return(true);
}

bool __fastcall CCheckFileChanged::Restart(void)
{
	if(Changed == true)
	{
		FindNextChangeNotification(hNotify);//再度監視を行う
		Changed = false;
		Resume();
	}
	return(true);
}

bool __fastcall CCheckFileChanged::StopChecking(void)
{
	FileAgeValue = 0;
	FileName = L"";
	Changed = false;
  Suspend();
	return(true);
}

bool __fastcall CCheckFileChanged::SetCheckFile(UnicodeString NewFileName)
{
	FileAgeValue = 0;
	FileName = NewFileName;

	if(FileExists(FileName) == true)
	{
		FileAgeValue = FileAge(FileName);

		if(Changed == false && hNotify[0] != NULL)
		{
			SetEvent(hNotify[1]);
		}

		Resume();
	}
	else
		Suspend();

	Changed = false;
	return(true);
}

*/
