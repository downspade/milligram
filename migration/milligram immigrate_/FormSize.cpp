//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FormSize.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "NumberEdit"
#pragma resource "*.dfm"
TFormSizeForm *FormSizeForm;
//---------------------------------------------------------------------------
__fastcall TFormSizeForm::TFormSizeForm(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------

void __fastcall TFormSizeForm::SetData(int SrcWidth, int SrcHeight)
{
	OWidth = SrcWidth;
	OHeight = SrcHeight;
	Changing = true;
	WidthNE->Text = IntToStr(OWidth);
	HeightNE->Text = IntToStr(OHeight);
	Changing = false;
}

void __fastcall TFormSizeForm::GetData(int &DestWidth, int &DestHeight)
{
	DestWidth = WidthNE->GetIntegerValue();
	DestHeight = HeightNE->GetIntegerValue();
}

void __fastcall TFormSizeForm::WidthNEChange(TObject *Sender)
{
	if(Changing == false)
	{
		Changing = true;
		HeightNE->Text = IntToStr(WidthNE->GetIntegerValue() * OHeight / OWidth);
		Changing = false;
	}
}
//---------------------------------------------------------------------------
void __fastcall TFormSizeForm::HeightNEChange(TObject *Sender)
{
	if(Changing == false)
	{
		Changing = true;
		WidthNE->Text = IntToStr(HeightNE->GetIntegerValue() * OWidth / OHeight);
		Changing = false;
	}

}
//---------------------------------------------------------------------------
