//---------------------------------------------------------------------------

#ifndef FormSizeH
#define FormSizeH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "NumberEdit.h"
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TFormSizeForm : public TForm
{
__published:	// IDE �Ǘ��̃R���|�[�l���g
	TNumberEdit *WidthNE;
	TNumberEdit *HeightNE;
	TButton *OkButton;
	TButton *CancelButton;
	TPanel *Panel;
	TStaticText *WidthST;
	TStaticText *HeightST;
	void __fastcall WidthNEChange(TObject *Sender);
	void __fastcall HeightNEChange(TObject *Sender);
private:	// ���[�U�[�錾
public:		// ���[�U�[�錾
	__fastcall TFormSizeForm(TComponent* Owner);
	int OWidth;
	int OHeight;
	bool Changing;

	void __fastcall SetData(int SrcWidth, int SrcHeight);
	void __fastcall GetData(int &DestWidth, int &DestHeight);
};
//---------------------------------------------------------------------------
extern PACKAGE TFormSizeForm *FormSizeForm;
//---------------------------------------------------------------------------
#endif
