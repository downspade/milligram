//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Progress.h"
#include "Main.h"
#include "SpiPlugin.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TProgressForm *ProgressForm;
//---------------------------------------------------------------------------



__fastcall TProgressForm::TProgressForm(TComponent* Owner)
	: TForm(Owner)
{
	ProgressThread = NULL;
}
//---------------------------------------------------------------------------


void __fastcall TProgressForm::FormDestroy(TObject *Sender)
{
	SAFE_DELETE(ProgressThread);
}
//---------------------------------------------------------------------------


void __fastcall TProgressForm::Reset(void)
{
	Left = MainForm->Left + 10;
	Top = MainForm->Top + 10;
	ProgressLabel->Caption = L"";
	Button->Caption = L"ƒLƒƒƒ“ƒZƒ‹";
	Executed = false;
}

void __fastcall TProgressForm::Execute(void)
{
	SAFE_DELETE(ProgressThread);
	ProgressThread = new CProgressThread(true);
	ProgressThread->Src = Src;
	ProgressThread->Dest = Dest;
	ProgressThread->Mask = Mask;
	ProgressThread->Depth = Depth;
	ProgressThread->Esc = &Esc;
	ProgressThread->Resume();
}


void __fastcall TProgressForm::EndProgress(TMessage msg)
{
	ProgressThread->Terminate();
	Esc = false;
	Close();
}


void __fastcall TProgressForm::FormShow(TObject *Sender)
{
	if(Executed == false)
	{
		Executed = true;
		Execute();
	}
}
//---------------------------------------------------------------------------

void __fastcall TProgressForm::ButtonClick(TObject *Sender)
{
	Esc = true;
}
//---------------------------------------------------------------------------

