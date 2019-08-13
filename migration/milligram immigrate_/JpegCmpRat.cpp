//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "JpegCmpRat.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "NumberEdit"
#pragma resource "*.dfm"
TJpegCmpRatForm *JpegCmpRatForm;
//---------------------------------------------------------------------------
__fastcall TJpegCmpRatForm::TJpegCmpRatForm(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TJpegCmpRatForm::JCR0TBChange(TObject *Sender)
{
	Changing = true;
	JCR0NE->Text = IntToStr(JCR0TB->Position);
	Changing = false;
}
//---------------------------------------------------------------------------
void __fastcall TJpegCmpRatForm::JCR0NEChange(TObject *Sender)
{
	JCR0TB->Position = JCR0NE->GetIntegerValue();
}
//---------------------------------------------------------------------------

void __fastcall TJpegCmpRatForm::OkButtonClick(TObject *Sender)
{
	Tag = JCR0TB->Position;
}
//---------------------------------------------------------------------------

void __fastcall TJpegCmpRatForm::CancelButtonClick(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------

void __fastcall TJpegCmpRatForm::SetData(int *JCR)
{
	JCR0TB->Position = JCR[0];
	JCR1B->Caption = IntToStr(JCR[1]); JCR1B->Tag = JCR[1];
	JCR2B->Caption = IntToStr(JCR[2]); JCR2B->Tag = JCR[2];
	JCR3B->Caption = IntToStr(JCR[3]); JCR3B->Tag = JCR[3];
}

void __fastcall TJpegCmpRatForm::GetData(int *JCR)
{
	JCR[0] = JCR0TB->Position;
	JCR[1] = JCR1B->Tag;
	JCR[2] = JCR2B->Tag;
	JCR[3] = JCR3B->Tag;
}

void __fastcall TJpegCmpRatForm::JCRBClick(TObject *Sender)
{
	if(Sender == NULL)return;

	TButton *TempB;
	TempB = dynamic_cast<TButton *>(Sender);

	if(
		 (GetAsyncKeyState(VK_LCONTROL) & 0x8000) || (GetAsyncKeyState(VK_RCONTROL) & 0x8000)
	)
	{
		TempB->Tag = JCR0TB->Position;
		TempB->Caption = IntToStr(TempB->Tag);
	}
	else
	{
		JCR0TB->Position = TempB->Tag;
	}
}
//---------------------------------------------------------------------------

