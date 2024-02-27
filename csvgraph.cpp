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
USEFORM("About.cpp", AboutBox);
USEFORM("UScalesWindow.cpp", ScalesWindow);
USEFORM("Unit1.cpp", Form1);
USEFORM("UDataPlotWindow.cpp", PlotWindow);
//---------------------------------------------------------------------------
USEFORM("Unit1.cpp", Form1)
USEFORM("UScalesWindow.cpp", ScalesWindow)
USEFORM("UDataPlotWindow.cpp", PlotWindow)
USEFORM("About.cpp", AboutBox)
//---------------------------------------------------------------------------
#define NoForm1
#include "rprintf.h"
#include "expr-code.h"
#include <float.h>

#if 1  /* my version that picks up command line */
extern volatile int xchange_running; // avoid running multiple instances of Edit_XoffsetChange() in parallel, but still do correct number of updates , set to -1 initially
extern volatile bool addtraceactive; // set to true when add trace active to avoid multiple clicks

 /* next function used to support conversion to unicode vcl see https://blogs.embarcadero.com/migrating-legacy-c-builder-apps-to-c-builder-10-seattle/
*/
#define STR_CONV_BUF_SIZE 2000 // the largest string you may have to convert. depends on your project

static char* __fastcall AnsiOf(wchar_t* w)
{
	static char c[STR_CONV_BUF_SIZE];
	memset(c, 0, sizeof(c));
	WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, w, (int)wcslen(w), c, STR_CONV_BUF_SIZE, nullptr, nullptr);
	return(c);
}

void proces_open_filename(char *fn); // open filename - just to peek at header row
int WINAPI _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int)
{LPWSTR *szArglist;
 int nArgs;
 _controlfp( MCW_EM,MCW_EM );  // disable (bits set) all floating point exceptions see https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/control87-controlfp-control87-2?view=msvc-170
 // command line handling  based on the example at https://learn.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-commandlinetoargvw
 // It is therefore windows specific, but should be portable to other compilers
 szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
 try
		{
		 Application->Initialize();
		 Application->MainFormOnTaskBar = true;
		 Application->Title = "CSVgraph";
		 // Application->CreateForm(__classid(TAboutBox), &AboutBox);
		 Application->CreateForm(__classid(TForm1), &Form1);
		Application->CreateForm(__classid(TScalesWindow), &ScalesWindow);
		Application->CreateForm(__classid(TAboutBox), &AboutBox);
		if(szArglist != nullptr && nArgs==2)  // one argument, assume its a filename and load this file
				  {
				   proces_open_filename( AnsiOf(szArglist[1])); // open filename supplied on command line - just to peek at header row
				  }
		 Application->Run();
		}
 catch (Exception &exception)
		{
		 xchange_running= -1; // avoid running multiple instances of Edit_XoffsetChange() in parallel, but still do correct number of updates, -1 is initial value
		 addtraceactive=false; // set to true when add trace active to avoid multiple clicks
		 rprintf("Exception1:\n");
#ifndef TRY_CATCH_DISABLED
		 Application->ShowException(&exception);
#endif
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
			 rprintf("Exception2:\n");
#ifndef TRY_CATCH_DISABLED
			 Application->ShowException(&exception);
#endif
			}
		}

 return 0;
}
#endif
//---------------------------------------------------------------------------
