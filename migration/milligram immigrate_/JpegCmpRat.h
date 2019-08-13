//---------------------------------------------------------------------------

#ifndef JpegCmpRatH
#define JpegCmpRatH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "NumberEdit.h"
#include <Buttons.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TJpegCmpRatForm : public TForm
{
__published:	// IDE 管理のコンポーネント
	TNumberEdit *JCR0NE;
	TButton *OkButton;
	TButton *CancelButton;
	TTrackBar *JCR0TB;
	TButton *JCR1B;
	TButton *JCR2B;
	TButton *JCR3B;
	TPanel *Panel;
	void __fastcall JCR0TBChange(TObject *Sender);
	void __fastcall JCR0NEChange(TObject *Sender);
	void __fastcall OkButtonClick(TObject *Sender);
	void __fastcall CancelButtonClick(TObject *Sender);
	void __fastcall JCRBClick(TObject *Sender);
private:	// ユーザー宣言
public:		// ユーザー宣言
	__fastcall TJpegCmpRatForm(TComponent* Owner);
	bool Changing;

	void __fastcall SetData(int *JCR);
	void __fastcall GetData(int *JCR);
};
//---------------------------------------------------------------------------
extern PACKAGE TJpegCmpRatForm *JpegCmpRatForm;
//---------------------------------------------------------------------------
#endif
