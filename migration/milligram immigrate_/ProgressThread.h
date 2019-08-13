//---------------------------------------------------------------------------

#ifndef ProgressThreadH
#define ProgressThreadH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#define WM_PROGRESSEND (WM_APP + 2)

//---------------------------------------------------------------------------
class CProgressThread : public TThread
{
private:
protected:
	void __fastcall Execute();
public:
	__fastcall CProgressThread(bool CreateSuspended);
	bool __fastcall GetFileListsDialog(UnicodeString Src, TStringList *Dest, UnicodeString Mask, int Depth, TStringList *TempSL, bool *Esc);
	void __fastcall UpdateForm(void);

	UnicodeString Label;

	UnicodeString Src;
	TStringList *Dest;
	UnicodeString Mask;
	int Depth;
	bool *Esc;
};
//---------------------------------------------------------------------------
#endif
