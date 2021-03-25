//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
#include <tchar.h>
//---------------------------------------------------------------------------
USEFORM("UDataPlotWindow.cpp", PlotWindow);
USEFORM("Unit1.cpp", Form1);
USEFORM("UScalesWindow.cpp", ScalesWindow);
USEFORM("About.cpp", AboutBox);
//---------------------------------------------------------------------------
#if 1  /* my version that picks up command line */
void proces_open_filename(char *fn); // open filename - just to peek at header row
int WINAPI _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int)
{
  try
		{
				 Application->Initialize();
				 Application->MainFormOnTaskBar = true;
				 Application->Title = "CSVgraph";
				 // Application->CreateForm(__classid(TAboutBox), &AboutBox);
		Application->CreateForm(__classid(TForm1), &Form1);
		Application->CreateForm(__classid(TScalesWindow), &ScalesWindow);
		Application->CreateForm(__classid(TAboutBox), &AboutBox);
		if(_argc==2)  // one argument, assume its a filename and load this file
				  {
				   proces_open_filename( _argv[1]); // open filename supplied on command line - just to peek at header row
				  }
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
#endif
//---------------------------------------------------------------------------
