//---------------------------------------------------------------------------

#ifndef ProgressH
#define ProgressH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include "ProgressThread.h"
//---------------------------------------------------------------------------
class TProgressForm : public TForm
{
__published:	// IDE �Ǘ��̃R���|�[�l���g
	TLabel *ProgressLabel;
	TButton *Button;
	void __fastcall FormShow(TObject *Sender);
	void __fastcall FormDestroy(TObject *Sender);
	void __fastcall ButtonClick(TObject *Sender);
private:	// ���[�U�[�錾
public:		// ���[�U�[�錾
	__fastcall TProgressForm(TComponent* Owner);

	void __fastcall EndProgress(TMessage msg);
	void __fastcall Reset(void);
	void __fastcall Execute(void);

	BEGIN_MESSAGE_MAP
	MESSAGE_HANDLER(WM_PROGRESSEND, TMessage, EndProgress)
	END_MESSAGE_MAP(TForm)

	UnicodeString Src;
	TStringList *Dest;
	UnicodeString Mask;
	int Depth;

	CProgressThread *ProgressThread;
	bool Esc;
	bool Executed;

};
//---------------------------------------------------------------------------
extern PACKAGE TProgressForm *ProgressForm;
//---------------------------------------------------------------------------
#endif
