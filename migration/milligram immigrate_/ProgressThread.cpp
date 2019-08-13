//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "ProgressThread.h"
#include "Progress.h"
#include "acfc_VCLSub.h"
#include "SpiPlugin.h"

#pragma package(smart_init)
//---------------------------------------------------------------------------

// 注意：異なるスレッドが所有する VCL のメソッド/関数/プロパティを別の
// レッド所有のオブジェクトに対しては Synchronize を使用できます。
//
//      Synchronize(&UpdateCaption);
//
// 例えば UpdateCaption を以下のように定義し
//
//      void __fastcall CProgressThread::UpdateCaption()
//      {
//        Form1->Caption = "スレッドから書き換えました";
//      }
//---------------------------------------------------------------------------

__fastcall CProgressThread::CProgressThread(bool CreateSuspended)
	: TThread(CreateSuspended)
{
}
//---------------------------------------------------------------------------
void __fastcall CProgressThread::Execute()
{
	//---- Place thread code here ----
	TStringList *TempSL = new TStringList();
  GetFileListsDialog(Src, Dest, Mask, Depth, TempSL, Esc);
	TempSL->Add(Src);
	delete TempSL;
	SendMessage(ProgressForm->Handle, WM_PROGRESSEND, 0, 0);
}
//---------------------------------------------------------------------------


bool __fastcall CProgressThread::GetFileListsDialog(UnicodeString Src, TStringList *Dest, UnicodeString Mask, int Depth, TStringList *TempSL, bool *Esc)
{
	TSearchRec sr;

	UnicodeString Path = ExtractFilePath(Src);
	UnicodeString Name = ExtractFileName(Src);

	Label = Path;
	Synchronize(&UpdateForm);

	if(Depth != 0 && *Esc == false)
	{
		Depth--;
		UnicodeString Name = ExtractFileName(Src);

		if(FindFirst(Path + "*", faDirectory, sr) == 0)
		{
			do
			{
				if((sr.Attr & faDirectory) && sr.Name != L"." && sr.Name != L"..")
					GetFileListsDialog(Path + sr.Name + "\\" + Name, Dest, Mask, Depth, TempSL, Esc);
				TempSL->Add(Path + sr.Name);
			}
			while(FindNext(sr) == 0 && *Esc == false);
			FindClose(sr);
		}

		if(FindFirst(Path + "*.lnk", faAnyFile, sr) == 0)
		{
			do
			{
				sr.Name = GetFileFromLink(Path + sr.Name);
				if(DirectoryExists(sr.Name) == true && TempSL->IndexOf(sr.Name) < 0)
				{
					GetFileListsDialog(sr.Name + "\\" + Name, Dest, Mask, Depth, TempSL, Esc);
					TempSL->Add(sr.Name);
				}
			}
			while(FindNext(sr) == 0 && *Esc == false);
			FindClose(sr);
		}
	}

	if(*Esc == false && FindFirst(Src, faAnyFile, sr) == 0)
	{
		do
		{
			if(!((sr.Attr & faDirectory) || sr.Name == L"." || sr.Name == L".."))
			{
				int Len = sr.Name.Length();
				if(Len > 5)
					if(sr.Name.SubString(Len - 3, 4) == L".lnk")
					{
						sr.Name = GetFileFromLink(Path + sr.Name);
						if(DirectoryExists(sr.Name) == true)continue;

						if(Mask != L"*" && Mask != L"*.*" && CheckMasks(sr.Name, Mask) == false)continue;
						SImageInfo *NewII = new SImageInfo;
						NewII->Timestamp = sr.Time;
						NewII->FileSize = sr.Size;
						NewII->Rotate = -1;
						Dest->AddObject(sr.Name, (TObject *)NewII);
						continue;
					}

				if(Mask != L"*" && Mask != L"*.*" && CheckMasks(sr.Name, Mask) == false)continue;

				SImageInfo *NewII = new SImageInfo;
				NewII->Timestamp = sr.Time;
				NewII->FileSize = sr.Size;
				NewII->Rotate = -1;
				Dest->AddObject(Path + sr.Name, (TObject *)NewII);
			}
		}
		while(FindNext(sr) == 0 && *Esc == false);
		FindClose(sr);
	}
	return(true);
}


void __fastcall CProgressThread::UpdateForm(void)
{
	ProgressForm->ProgressLabel->Caption = Label;
}



