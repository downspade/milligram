#include "acfc_VCLSub.h"
#include "math.h"
#include "sys\stat.h"

//------------------------------------------------------------
//
// �t�@�C���̓ǂݏ���
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

bool CheckMasks(UnicodeString FileName, UnicodeString Mask) // �}�X�N�ɊY�������� true ��Ԃ�
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

	// �܂��f�B���N�g����T��
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

	// ���Ƀt�@�C��
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

	// �܂��f�B���N�g����T��
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

	// ���Ƀt�@�C��
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

// �t�@�C�����ړ�����
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

	// �܂��f�B���N�g����T��
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

	// ���Ƀt�@�C��
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

// �t�@�C�����X�g���擾����(Esc�L�[�Ŕ�������)
bool GetFileLists(UnicodeString Src, TStringList *Dest, UnicodeString Mask, int Depth) // c:\windows\dest\*.dll �̂悤�Ȍ`���� Src �͎w�肷��
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


// �t�@�C�����X�g���擾����(Esc�L�[�Ŕ�������)
bool GetFileListsDialog(UnicodeString Src, TStringList *Dest, UnicodeString Mask, int Depth, TApplication *Application, TLabel *Label) // c:\windows\dest\*.dll �̂悤�Ȍ`���� Src �͎w�肷��
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


// �t�@�C�����R�s�[����
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

// �t�@�C�������R�s�[����i�t�H���_�̓R�s�[���Ȃ��j
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

	// �t�@�C��
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

	// �f�B���N�g����T��
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

// Src �t�H���_���̃t�@�C�����폜���ăt�@�C���������Ȃ�΃t�H���_���폜
void DeleteFolderFiles(UnicodeString Src, UnicodeString Dest, UnicodeString Type)
{
	TSearchRec sr;
	int iAttributes;
	UnicodeString Path;
	if(DirExists(ExcludeTrailingBackslash(Src)) == false)return;

	// �f�B���N�g����T��
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

// Src �t�H���_�ɂ���t�@�C���̂��� Dest �t�H���_�ɂ�����̂� Dest �t�H���_�ɃR�s�[����
void ReplaceFolderFiles(UnicodeString Src, UnicodeString Dest, UnicodeString Type)
{
	TSearchRec sr;
	int iAttributes;
	UnicodeString Path;
	
	Path = Src + L"*.*";
	if(DirExists(ExcludeTrailingBackslash(Src)) == false)return;

	// �f�B���N�g����T��
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

// �R��ނ̃^�C���X�^���v�����ׂĎw�肵���l�ɂ���
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

// �t�@�C���̃T�C�Y���擾����
size_t GetFileSize(UnicodeString Target)
{
	struct _stat statbuf;
	_wstat(Target.c_str(), &statbuf);
	return(statbuf.st_size);
}



// �w�肵���f�B���N�g���̐������f�B���N�g����������폜
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


// ���΃p�X���΃p�X�ɕϊ�
// root �t�H���_���� Relative �t�H���_�ւ̑��΂��΂ɒ���
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

// (1) (2) �`���ŏd�����Ȃ��t�@�C�������擾����
UnicodeString GetNonOverwrapFileName(UnicodeString Src)
{
	UnicodeString Dest = Src, Name, Path, Ext;
	int Count = 1;

	// �܂��ŏ��̃t�@�C������ (1) �`�����ǂ����`�F�b�N
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
// ShellLink ���K�v
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
// �������́A�V���[�g�J�b�g�̌��̃t�@�C����
// �������́A�V���[�g�J�b�g�̃t�@�C����
//�i���Ƃ��� c:\abc.lnk�Ƃ�)
{
	HRESULT hres;
	IShellLink* psl = NULL;

	// COM�C���^�[�t�F�C�X��������
	CoInitialize(NULL) ;

	// IShellLink �C���^�[�t�F�C�X���擾
	hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *)&psl);
	if(SUCCEEDED(hres))
	{
		IPersistFile* ppf;

		// Link�I�u�W�F�N�g�̃p�X��ݒ�(���Ƃ��΁A
		// C;\Windows\notepad.exe�Ȃ�)
		psl->SetPath(OriginalFile.c_str());

		// �����̐ݒ�
		if(Params != "")psl->SetArguments(Params.c_str());

		// IPersistFile�C���^�[�t�F�C�X���擾���A
		// Link�p�X�t�@�C������ۑ�����B
		// ����́AIShellLink�̒���
		// �f�B�X�N�֘A�I�u�W�F�N�g��
		// �擾���Ă���Ǝv���Ă��������B
		hres = psl->QueryInterface( IID_IPersistFile, (void**)&ppf);

		if(SUCCEEDED(hres))
		{
			// �f�B�X�N�ɕۑ�����
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

	// �Ō�� \ ���Ƃ�
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

// �_�C�A���O��\�����ăt�H���_���擾����
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

	BI.hwndOwner = Handle;                     // �I�[�i�[�E�B���h�E�̃n���h��
	BI.pidlRoot = NULL;                        // ���[�g�t�H���_��ID���X�g
	BI.pszDisplayName = FName;                 // �I���t�H���_�̃p�X
	BI.lpszTitle = Message.c_str();   // �\������
	BI.ulFlags = BIF_RETURNONLYFSDIRS;         // �t�H���_�̂ݑI��
	BI.lpfn = (BFFCALLBACK)acfc_VCLSub_BrowseCallback;     // �R�[���o�b�N�֐�
	BI.lParam = (LPARAM)DefaultDir.c_str();    // �R�[���o�b�N�֐��ɓn�����l
	BI.iImage = 0;                             // �I���t�H���_�̃C���[�W���X�g�C���f�b�N�X

	ItemID = SHBrowseForFolderW(&BI);
	SHGetPathFromIDListW(ItemID, DirPath);
	GlobalFreePtr(ItemID);

	Result = DirPath;
	return(Result);
}

// �t�@�C�����S�~���Ɉړ�����
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

// �t�@�C�����N���b�v�{�[�h�ɃR�s�[�i�J�b�g�A�V���[�g�J�b�g�쐬���삷��j
void FilesToClipboard(HWND Handle, TStringList *Src, int Flag) // Flag: DROPEFFECT_COPY, DROPEFFECT_MOVE, DROPEFFECT_LINK
{
	//Unicode �Ő��������삵�Ȃ��H
	//CF_HDROP���쐬

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

	//Preferred DropEffect���쐬
	HGLOBAL hDropEffect = GlobalAlloc(GHND, sizeof(DWORD));
	DWORD* pdw = static_cast<DWORD*>(GlobalLock(hDropEffect));
	*pdw = Flag;
	GlobalUnlock(hDropEffect);

	//�N���b�v�{�[�h�Ƀf�[�^�[���Z�b�g
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

// �t�@�C�����Ɏg���Ȃ������𓾂� -1:�Ȃ� 0~:���̃C���f�b�N�X�̕���
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
//  ���̑�
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

// ���l���擾
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

// ���l���擾
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
// Bool �l���擾
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

// ��������擾
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


// Dest ������ SrcTS �����S���ɂ��邩�ǂ����`�F�b�N����
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


// ���l���擾
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

// ���l���擾
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
// Bool �l���擾
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

// ��������擾
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

	// �܂��f�B���N�g����T��
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

	// ���Ƀt�@�C��
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

	// �t�H�A�O���E���h�E�B���h�E���쐬�����X���b�h��ID���擾
	nForegroundID = GetWindowThreadProcessId(GetForegroundWindow(), NULL);
	// �ړI�̃E�B���h�E���쐬�����X���b�h��ID���擾
	nTargetID = GetWindowThreadProcessId(hWnd, NULL);

	// �X���b�h�̃C���v�b�g��Ԃ����ѕt����
	AttachThreadInput(nTargetID, nForegroundID, TRUE);  // TRUE �Ō��ѕt��

	// ���݂̐ݒ�� sp_time �ɕۑ�
	SystemParametersInfo(SPI_GETFOREGROUNDLOCKTIMEOUT, 0, &sp_time, 0);
	// �E�B���h�E�̐؂�ւ����Ԃ� 0ms �ɂ���
	SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, NULL, 0);

	// �E�B���h�E���t�H�A�O���E���h�Ɏ����Ă���
	SetForegroundWindow(hWnd);

	// �ݒ�����ɖ߂�
	SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, &sp_time, 0);

	// �X���b�h�̃C���v�b�g��Ԃ�؂藣��
	AttachThreadInput(nTargetID, nForegroundID, FALSE);  // FALSE �Ő؂藣��
}








//----------------------------------------------------------------
//
// CDragDrop �h���b�O�h���b�v�֌W
//
//----------------------------------------------------------------

void __fastcall CDragDrop::Init(TWinControl *Control)
{
	// �|�C���^��ۑ����Ă���
	oldWinProc = Control->WindowProc;
	// �h���b�O�h���b�v�̎󂯓���
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
// CCriticalSection �r������
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
// CBinaryBlock ��������o�C�i���̃f�[�^�ɕϊ�����
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
//  ���W�X�g���A�N�Z�X�N���X
//---------------------------------------------------------------------------
//  �R���X�g���N�^
CReg::CReg()
{
	FRootKey = HKEY_CURRENT_USER;
	FOpened = false;
}

//-----------------------------------------------------------------------------
//  �f�X�g���N�^
CReg::~CReg()
{
	CloseKey();
}

//-----------------------------------------------------------------------------
//  �L�[�̍쐬
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
//  �L�[�̃I�[�v��( IsCreate==true�Ȃ�J���Ȃ��ꍇ�ɍ쐬���܂�)
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
//  �L�[�����
void CReg::CloseKey()
{
	if(FOpened)
		RegCloseKey(FHKey);
	FOpened = false;
}

//-----------------------------------------------------------------------------
//  �L�[���폜����
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
//  bool�̓ǂݏ���
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
//  Integer�̓ǂݏ���
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
//  Integer�̓ǂݏ���
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
//  Binary�̓ǂݏ���
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
//  �f�[�^�̍폜
bool CReg::DeleteValue(const UnicodeString Name)
{
	return(RegDeleteValueW(FHKey, Name.c_str()) == ERROR_SUCCESS);
}


//----------------------------------------------------------------
//
//  Unicode �Ή��摜�N���X
//
//----------------------------------------------------------------

__fastcall TUCFileStream::TUCFileStream(UnicodeString FileName, DWORD AccessMode = GENERIC_READ, DWORD ShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE)
: THandleStream(
	(int)CreateFileW(
		FileName.c_str(),   	 //�t�@�C����
		AccessMode,            //�A�N�Z�X���[�h
		ShareMode,             //���L���[�h
		NULL,                   //�Z�L�����e�B
		(AccessMode & GENERIC_WRITE) != NULL ? OPEN_ALWAYS : OPEN_EXISTING,          //�쐬���@
		FILE_ATTRIBUTE_NORMAL, //�t�@�C������
		0                      //�e���v���[�g
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
// ���ӁF�قȂ�X���b�h�����L���� VCL �̃��\�b�h/�֐�/�v���p�e�B��
//       �ʂ̃X���b�h�̒����爵���ꍇ�C�r�������̖�肪�������܂��B
//
//       ���C���X���b�h�̏��L����I�u�W�F�N�g�ɑ΂��Ă� Synchronize
//       ���\�b�h���g�������ł��܂��B���̃I�u�W�F�N�g���Q�Ƃ��邽��
//       �̃��\�b�h���X���b�h�N���X�ɒǉ����CSynchronize ���\�b�h��
//       �����Ƃ��ēn���܂��B
//
//       ���Ƃ��� UpdateCaption ���ȉ��̂悤�ɒ�`���A
//
//          void __fastcall Unit1::UpdateCaption()
//          {
//            Form1->Caption = "�X���b�h���珑�������܂���";
//          }
//
//       Execute ���\�b�h�̒��� Synchronize ���\�b�h�ɓn�����ƂŃ��C
//       ���X���b�h�����L���� Form1 �� Caption �v���p�e�B�����S�ɕ�
//       �X�ł��܂��B
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

	// �w��t�H���_�i�T�u�t�H���_���܂ށj���̃t�H���_�̕ύX���Ď�
	//-----------------------------------------------------------------------------------------------------
	hNotify[1] = CreateEventW(NULL, TRUE, FALSE, L"ACFCVCLSUB_CCHECKFILECHANGED"); // ���ׂ�t�@�C�����ύX���ꂽ
	//
	// �����ꂪ�K�����s����BC++Builder �̃o�O�̂悤��
	//
	//-----------------------------------------------------------------------------------------------------

	DWORD Err= GetLastError();
	DWORD SSS=Err;
}

__fastcall CCheckFileChanged::~CCheckFileChanged(void)
{
	FindCloseChangeNotification(hNotify[0]);//�Ď��̏I��
	Terminate();
}

//---------------------------------------------------------------------------

void __fastcall CCheckFileChanged::Execute()
{
	// �X���b�h�Ƃ��Ď��s�������R�[�h�������ɋL�q
	UnicodeString DirName = ExtractFileDrive(FileName);

	while(1)
	{
		if(hNotify[0] == NULL)
		{
			DWORD Result = WaitForSingleObject(hNotify[1], INFINITE);
			if(Terminated == true)return;

			// �V�����t�@�C���𒲂ׂ�
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

				FindCloseChangeNotification(hNotify);//�Ď��̏I��

				// �V�����t�@�C���𒲂ׂ�
				DirName = ExtractFileDrive(FileName);
				hNotify[0] = FindFirstChangeNotificationW(DirName.c_str(), FALSE, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE);
				ResetEvent(hNotify[0]);
				ResetEvent(hNotify[1]);
			}
			else
				FindNextChangeNotification(hNotify);//�ēx�Ď����s��
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
		FindNextChangeNotification(hNotify);//�ēx�Ď����s��
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
