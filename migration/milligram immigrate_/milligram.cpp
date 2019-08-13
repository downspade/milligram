//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
#include <tchar.h>
//---------------------------------------------------------------------------
USEFORM("Progress.cpp", ProgressForm);
USEFORM("FormSize.cpp", FormSizeForm);
USEFORM("JpegCmpRat.cpp", JpegCmpRatForm);
USEFORM("Main.cpp", MainForm);
//---------------------------------------------------------------------------
WINAPI _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int)
{
	try
	{
		Application->Initialize();
		Application->MainFormOnTaskBar = true;
		Application->Title = "milligram";
		Application->CreateForm(__classid(TMainForm), &MainForm);
		Application->CreateForm(__classid(TJpegCmpRatForm), &JpegCmpRatForm);
		Application->CreateForm(__classid(TProgressForm), &ProgressForm);
		Application->CreateForm(__classid(TFormSizeForm), &FormSizeForm);
		Application->Run();
	}
	catch (Exception &exception)
	{
		Application->ShowException(&exception);
	}
	catch (...)
	{
		try
		{
			throw Exception("");
		}
		catch (Exception &exception)
		{
			Application->ShowException(&exception);
		}
	}
	return 0;
}
//---------------------------------------------------------------------------
