//---------------------------------------------------------------------------
/*----------------------------------------------------------------------------
 * MIT License:
 *
 * Copyright (c) 2020,2022 Peter Miller
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHOR OR COPYRIGHT HOLDER BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *--------------------------------------------------------------------------*/

#include <vcl.h>
#pragma hdrstop
#include <tchar.h>
//---------------------------------------------------------------------------
USEFORM("Unit1.cpp", Form1);
USEFORM("UDataPlotWindow.cpp", PlotWindow);
USEFORM("UScalesWindow.cpp", ScalesWindow);
USEFORM("About.cpp", AboutBox);
//---------------------------------------------------------------------------
USEFORM("Unit1.cpp", Form1)
USEFORM("UScalesWindow.cpp", ScalesWindow)
USEFORM("UDataPlotWindow.cpp", PlotWindow)
USEFORM("About.cpp", AboutBox)
//---------------------------------------------------------------------------
#if 1  /* my version that picks up command line */
extern volatile int xchange_running; // avoid running multiple instances of Edit_XoffsetChange() in parallel, but still do correct number of updates , set to -1 initially
extern volatile bool addtraceactive; // set to true when add trace active to avoid multiple clicks

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
		xchange_running= -1; // avoid running multiple instances of Edit_XoffsetChange() in parallel, but still do correct number of updates, -1 is initial value
		addtraceactive=false; // set to true when add trace active to avoid multiple clicks
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
			xchange_running= -1; // avoid running multiple instances of Edit_XoffsetChange() in parallel, but still do correct number of updates, -1 is initial value
			addtraceactive=false; // set to true when add trace active to avoid multiple clicks
			Application->ShowException(&exception);
		}
	}
	return 0;
}
#endif
//---------------------------------------------------------------------------
