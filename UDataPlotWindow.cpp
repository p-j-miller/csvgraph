//---------------------------------------------------------------------------
// Created by Peter Miller 18/7/2019 as csv file grapher   (csvgraph.exe)
//
//  Loosely based on an original public domain version by  Frank Heinrich, mail@frank-heinrich.de
//
// Originally created using Borland Builder c++ V5
// This version compiled with Embarcadero® C++Builder 10.2
// It should be easy to move to a more recent version of C++Builder, but equally it would not be very hard to revert to earlier versions if necessary.
// This is compiled for (and this is the only version that has been tested) Windows 32 bits.
// It runs on Windows 10 64 bits. In theory it should run on windows versions from Vista onwards and 32bit versions of Windows but this is untested.
//
// Version Date   : changes from previous version
// 1v0 1/1/2021  : 1st version released on github (was 16v6).
// 1v1 4/1/2021  : Bug fix in fft - DC component (zero frequency) could previoulsy become -ve if average value of waveform was -ve.
//               : pdf of manual created so that readers open with bookmarks visible automatically. Manual contents unchanged.
//               : added Menu/Help/Manual to display manual within csv graph (assumes csvgraph.pdf is in the same directory as csvgraph.exe)
//               : if a filename is passed on the command line to csvgraph.exe it is opened at start. This allows csvgraph to be associated with csvfiles.
// 1v2 21/1/2021 : added (many) more types of linear regression eg exponential, log, power, hyperbolic, sqrt.
//               : Also now traps "nan" and "inf" in csv files - previouly files containing these could be read but with give "LOG10" error when trying to display the graph.
// 1v3 27/1/2021 : Geometric Mean regression added as filter option
//     30/1/2021 : fit to y=mx added
//               : y=mx+c min abs error and min abs relative error fits added.
//     2/2/2021  : added y=a*sqrt(x)+b*x+c least squares fit  [ note this is the same as an order 2 poly fit to sqrt(x) but code is more efficient than doing this ]
// 2v0 6/2/2021  : change way x&y values are stored to reduce memory overhead and make it simpler to "filter" data
//               : for 20M data points memory usage reduced from 357.1 MB to 140.1MB and loading/plotting 1 trace reducing from 26.375 to 22.67 secs.
//               : (now uses 8 bytes per data point).
//               : some speed optimisations as well (eg if adding multiple traces at once only process x values once then copy for other traces - assuming they are monotonic in input file)
//               : sort optimised (worse case improved dramatically)
//               : reading times (h:m:s) made more robust - will now read a general number as a float (secs) - but assumes no exponent present
//               : warning - h:m:s date handling code assumes times are in order (all increasing), if they are not then day handling code may get confused...
// 2v1 20/1/2021 : investigation into speedup for myqsort() - avoid % operator, and directly sort 3 elements inline.   Gives 7% speedup on average.
//               : median3() function improved to use 2 or 3 compares (rather than 2 or 4).
//               : myqsort() now sorts up to 4 elements inline and myswap is a macro - this gives 12% speedup over previous best.
//               : option to check depth of recursion in myqsort - is < log2(nos_lines) as expected [ 16 for 16million line file].
//               : myqsort random number generator reset before every sort to give consistent results.
//               : myqsort() now sorts up to 6 elements inline this gives an additional 10% speedup.
//               : myqsort now sorts upto 16 elements inline giving a small speedup
//               : myqsort now sorts upto 32 elements inline gaining another 12% speedup.
//               : added y=(a+bx)/(1+cx)  (least squares fit)
//               : added general least squares multiple variable linear regression (multiple-lin-reg-fn.cpp)
//               :  this allows y=a+b*sqrt(x)+c*x+d*x^1.5 and
//               :   y=(a+b*x+c*x^2)/(1+d*x+e*x^2) to be fitted
//               :  generic poly in sqrt(x) and rational function(poly/poly) fitting allowed with user specified order
//               : better error trapping in basic linear regex (y=mx+c) for 1/x & log's
// 2v2 27/3/2021 : $T1 to Tn allowed in expressions to use values from existing traces on the graph. Traces are numbered from 1. Invalid trace numbers (too big) return 0
//               : user can now set order of linear filter. Implements nth order Butterworth filter (10*order db/decade). Order=0 gives no filtering. Order =1 gives same filtering as previously.
//               : "filters" for integral and derivative added
//               : all filters now report progress as a % (previoulsy min abs error and min rel error did not report progress and they could be quite slow).
//               : skip N lines before csv header option added for cases where csv header is not on the 1st row of the file
//               : Added column numbers to X column and Y column listboxes  to make it easier to select columns when names are not very descriptive (or missing).
// 2v3 3/1/2022  : can optionally use yasort2() for sorting - this has a guarantee on worse case execution time and can use all available processors to speed up sorting
//               : yamedian() used which can calculate median in place without needing to copy the array of y values (but will for speed if memory is available)
//               : added linear regression y=m*x*log2(x)+c
// 2v4 2/2/2022  : Fixed bug - using $Tn in situations where input x values needed to be sorted did not work correctly - fixed. Now also interpolates y value for $Tn if x values are different for new trace and $Tn
// 2v5a 10/2/2022: Fixed bug - if reading multiple columns at once optimisation that used x values from previous pass would fail if some lines were skipped  - now trapped and optimisation not done in this case
// 2v5b 12/2/2022: Improved code for handling errors while reading csv files, used to just ignore errors, but now 1st 2 errors are reported in detail and total count displayed.
//               : Date/time handling also improved, time is now read fully as a double and errors reported. A backwards step of up to 1 sec (eg a leap second) is allowed.
//               : Any backwards step in time will cause data to be sorted, so this is visible to users even if its not directly reported as an error.
// 2v5d 14/2/2022 : files of size >2^31 bytes would give silly % complete values , final graph was OK - fixed.
// 2.5e 16/2/2022 : added "start times from 0" tick box to gui.
// 2v6  20/2/2022 : new median1 (standard median filter) algorithm based on binning (or exact for a relatively small number of data points)
//                : X position of ledgends for the traces moved left to allow longer text to be read
// 2v7  15/3/2022 : prints -3dB frequency for linear filter.
//                : display to user 1 example of every type of error in csv file (via rprintf)
//                : if dates present on some lines then flag lines without a date as a potential error
//                : new (exact) median (recursive median filter) algorithm, which falls back to sampling if the execution time becomes long.
// 2v8  24/3/2022 : strptime() function added for date/time handling
//                : csvsave added % complete and buffering on output to speed up writing to file.
//                : csvsave interpolates if required so x values do not need to be identical on all traces
// 2v9 4/6/2022   : bug when X_Offset used and multiple traces added at once then offset is added once for 1st trace, twice for 2nd etc.
//                : first_time changed from double to long double so accuracy is improved when "start time from 0" is selected
//                : gethms() and gethms_days() also both now return long doubles and are coded to convert numbers to this full resolution.
// 2v10 8/6/22    : readline() changed to limit max line length (avoids a "silly" file with a very long line using all the available memory)
//                : Max length set at ~ 1 million characters so unlikley to be an issue with a real csv file
//                : when outer catch(...) (in csvgraph.cpp) is executed clear flags saying functions are executing so user may be able to continue working.
//  3v0 15/8/22   : 1st 64 bit version built with C++ Builder 10.4
//  3v1 17/8/22   : clean compile for 64 bit version for both debug and release. No obvious bugs found either!
//  3v2 18/8/22   : debugging info added (sizeof int etc). Made to compile for 32 and 64 bits.
//                  when compiled for 64 bits:
//      			_WIN32 defined _WIN64 defined __BORLANDC__ defined __SIZEOF_POINTER__ == 8 Compiled for 64 bit pointers
//					sizeof int=4, long=4, float=4, double=8, size_t=8,uintptr_t=8, intptr_t=8, void *=8
//
//                  when compiled for 32 bits:
//                  _WIN32 defined __BORLANDC__ defined __SIZEOF_POINTER__ == 4 Compiled for 32 bit pointers
//					sizeof int=4, long=4, float=4, double=8, size_t=4,uintptr_t=4, intptr_t=4, void *=4
//                  Minor changes:
//                      When scales menu invoked multiple times, each time values would change slightly
//                      Allowed range of font sizes for Main title and X/Y axis titles expanded a little to 4->19 (was 8->14)
//                      fft now uses multiple processor cores if available.
//                      fft now makes better use of multiple cores + potential overflow when factorising number of points fixed.
//                  Refactored code so code likley to be used elsewhere is now in "..\Commom-files\"
//                  3v2 was released on 14/9/2022 as 1st 64 bit release on github - 3v0 and 3v1 were internal only versions.
//  3v3 22/9/22     count_lines() returned an unsigned int - changed to size_t to allow more than 2^32 lines.
//                  fseek() also did not work with very large files, resulting in floating point divide by zero errors.
//                  Note when you run out of physical ram csvgraph will still continue (using virtual memory), but adding traces gets very slow, as does redrawing unzoomed screen. Zooming is still quick!
//  3v4 1/10/22     Fixed issue with 64 bit version about dpi awareness with multiple monitors.
//  3v5 20/10/22    an expression containing a function like max(1,2) would fail as the , would be treated as the terminator of the expression.
//                  Also ledgend for filtered traces with a time constant don't put (s) in as x axis might be eg hours.
//                  listbox updating for y variables now deals correctly with expressions.
//                  divide by 0 error when polynomial order =0 worked around (could not find actual issue! - but goes away when floating point exceptions turned off - see below).
//                  if csvgraph appears to be busy when user asks for a function like add trace then give them the option to fix this if csvgraph is not busy (eg if previous action caused an exception)
//                  Right mouse click now gives slope of line, and results are presented in a clearer way.
//                  _controlfp() {in csvgraph.cpp) used to turn off floating point exceptions (otherwise can get divide by zero errors etc)
//                  if filename given on command line, 64 bit version did not pick it up - fixed.
// 3v6 6/4/2023     Long column headers now cause a scroll bar to be automatically added to the X & Y listboxes so they can be fully seen
//                  Save x range on screen as CSV added to File menu.
//                  Option (tickbox) added to add basename of filename to legends of traces on the graph,
//                   which is useful if the same column is read from multiple files.
//                  Y axis title automatically added unless user specifies one (based on column header of 1st trace added).
//                  Added option to load X as Value/60 (sec->min), Val/3600 (sec->hrs), val/86400 (sec->days)
//                  Error handling for X values in a user defined date/time format improved, and trailing whitespace now allowed
//  3v7 4/6/2023
//                  Swapped to Builder C++ 11.3 compiler
//                      Fontsize TCSpinEdit caused runtime exceptions (write to invalid address) so changed to SpinEdit control which works.
//                      made Fonts more consistent (Arial used on graph,  Segoe UI used for controls on bar on right)
//                      some font sizes tweaked and controls moved on bar to right to make everything fit
//						Title now centered over graph (and has same width as the graph).
//                      Trace legends now have a "clear" background so are visible even when they overlap traces.
//                      Trace Legends can now be turned off (via tick box)
//                      Set Mantfest/DPI awareness to gdi scaling (was "none").
//
// TO DO:
//
//
// Note if executable is called (eg) csvgraph64.exe then manual needs to be called csvgraph64.pdf
//
//---------------------------------------------------------------------------
/*----------------------------------------------------------------------------
 * Copyright (c) 2019,2020,2021,2022 Peter Miller
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
//---------------------------------------------------------------------------
#pragma option -w-inl /* turn off W8027 Functions containing switch are not expanded inline warning */
#include <vcl.h>
#include <Printers.hpp>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <Clipbrd.hpp>
#pragma hdrstop
#include "UScientificGraph.h"
#include "UScalesWindow.h"
#include "UDataPlotWindow.h"
#include "Unit1.h"
#include "About.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
// #pragma link "CSPIN"
#pragma resource "*.dfm"

#define NoForm1   /* says Form1 is defined in another file */
#include "rprintf.h"
#include "expr-code.h"
#include <sys\stat.h>
#include <stdio.h>
#include <string.h>
#include <alloc.h>
#include <windows.h>
#include <commdlg.h>    // to use GetOpenFileNameA
#include <dos.h>
#include <stdlib.h>
#include <float.h>
#include <values.h>
#include "matrix.h" /* must come before include of multiple-lin-reg.h */
#include "multiple-lin-reg-fn.h"
#include "time_local.h"

#if 1
// I'm sorry for the next 2 lines, but otherwise I get a lot of warnings "zero as null pointer constant"
#undef NULL
#define NULL (nullptr)
#endif

extern TForm1 *Form1;
extern const char * Prog_Name;
#ifdef _WIN64
const char * Prog_Name="CSVgraph (Github) 3v7 (64 bit)";   // needs to be global as used in about box as well.
#else
const char * Prog_Name="CSVgraph (Github) 3v7 (32 bit)";   // needs to be global as used in about box as well.
#endif
#if 1 /* if 1 then use fast_strtof() rather than atof() for floating point conversion. Note in this application this is only slightly faster (1-5%) */
extern "C" float fast_strtof(const char *s,char **endptr); // if endptr != NULL returns 1st character thats not in the number
#define strtod fast_strtof  /* set so we use it in place of strtod() */
#endif
#define WHITE_BACKGROUND /* if defined then use a white background, otherwise use a black background*/
#define UseVCLdialogs /* if defined used VCL dialogs, otherwise use "raw" windows ones */
#define BIG_BUF_SIZE (128*1024) /* Should be a power of 2. 1024*1024=1MB . Testing showed 4K is the min for moderate performance, and after 256k performance gets slightly worse */
  /* times to read 2 columns from a 2BG file are no buf 126secs, 4k 78s, 8k 72s, 16k 71s, 32k 70s, 64k 69s, 128k 68s, 256k 68s, 512k 69s, 1024k 69s, 4096k 71s */
#define CL_BLOCK_SIZE (1024*1024) /* must be a bigish power of 2 , used for count_lines() function to quickly count lines in file 1M seems to be best on my PC */

#define P_UNUSED(x) (void)x; /* a way to avoid warning unused parameter messages from the compiler */


/* next 2 function used to support conversion to unicode vcl see https://blogs.embarcadero.com/migrating-legacy-c-builder-apps-to-c-builder-10-seattle/
*/
#define STR_CONV_BUF_SIZE 2000 // the largest string you may have to convert. depends on your project
#ifdef _WIN64
static wchar_t* __fastcall UnicodeOf(const char* c)
{
	static wchar_t w[STR_CONV_BUF_SIZE];
	memset(w,0,sizeof(w));
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, c, (int)strlen(c), w, STR_CONV_BUF_SIZE);
	return(w);
}
#endif
static char* __fastcall AnsiOf(wchar_t* w)
{
	static char c[STR_CONV_BUF_SIZE];
	memset(c, 0, sizeof(c));
	WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, w, (int)wcslen(w), c, STR_CONV_BUF_SIZE, NULL, NULL);
	return(c);
}

extern  volatile int xchange_running;
extern volatile bool addtraceactive;
extern float yval,xval;

static int line_colour=0;
volatile int xchange_running=-1; // avoid running multiple instances of Edit_XoffsetChange() in parallel, but still do correct number of updates
volatile bool addtraceactive=false; // set to true when add trace active to avoid multiple clicks
static bool zoomed=false; // set when zoomed in to stop autoscaling when new traces added
static bool user_set_trace_colour=false;
static TColor user_trace_colour;
static size_t nos_lines_in_file=0; // number of lines in last file opened
static bool start_time_from_0=false; // if true use 1st time read is as ofset
float yval,xval; // current values being added to graph, xval is set before yval

// dynamic number of columns
static char **col_ptrs=NULL,**hdr_col_ptrs=NULL; // pointers to strings for each field
static char *col_names=NULL; // copy of input line, used to keep column heading strings
static unsigned int MAX_COLS=0;
static AnsiString filename;
static AnsiString save_filename;
const static char *default_x_label="Horizontal axis title";
const static char *default_y_label="Vertical axis title";

void proces_open_filename(char *fn); // open filename - just to peek at header row

// windows getfilename  function

#ifndef UseVCLdialogs
static OPENFILENAME ofn;       // common dialog box structure
static char szFile[260];       // buffer for file name
char *getfilename()    // displays getfilename dialog and returns filename or "" if not entered
{
 // Initialize OPENFILENAME
 memset(&ofn,0, sizeof(ofn));  // zero memory
 ofn.lStructSize = sizeof(ofn);
 ofn.hwndOwner = Application->Handle ;// hwnd even  NULL works.
 ofn.lpstrFile = szFile;
 // Set lpstrFile[0] to '\0' so that GetOpenFileName does not
 // use the contents of szFile to initialize itself.
 ofn.lpstrFile[0] = '\0';
 ofn.nMaxFile = sizeof(szFile);
 ofn.lpstrFilter = "All\0*.*\0Csv\0*.CSV\0";
 ofn.nFilterIndex = 2;
 ofn.lpstrFileTitle = NULL ;
 ofn.nMaxFileTitle = 0;
 ofn.lpstrInitialDir = NULL;
 ofn.lpstrTitle= "Please select the CSV file you wish to read to draw graphs";
 ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

 // Display the Open dialog box.

 if (GetOpenFileName(&ofn)==TRUE)
        {// Got valid filename
         return   ofn.lpstrFile;
        }
 return "";  // no valid filename entered
}
#endif

#ifndef UseVCLdialogs
char *getsaveBMPfilename()    // for saving files : displays getfilename dialog and returns filename or "" if not entered
{
 // Initialize OPENFILENAME
 memset(&ofn,0, sizeof(ofn));  // zero memory
 ofn.lStructSize = sizeof(ofn);
 ofn.hwndOwner = Application->Handle ;// hwnd even  NULL works.
 ofn.lpstrFile = szFile;
 // Set lpstrFile[0] to '\0' so that GetOpenFileName does not
 // use the contents of szFile to initialize itself.
 ofn.lpstrFile[0] = '\0';
 ofn.nMaxFile = sizeof(szFile);
 ofn.lpstrFilter = "Bmp\0*.BMP\0"; // allowed extensions
 ofn.lpstrDefExt="BMP"; // default extension if user does not type one
 ofn.nFilterIndex = 1;
 ofn.lpstrFileTitle = NULL ;
 ofn.nMaxFileTitle = 0;
 ofn.lpstrInitialDir = NULL;
 ofn.lpstrTitle= "Please select the BMP filename you wish to save the graph in";
 ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOREADONLYRETURN ;// warn user if file already exists, do not allow read only files or directories

 // Display the Open dialog box.

 if (GetSaveFileName(&ofn)==TRUE)      // GetSaveFileName
        {// Got valid filename
         return   ofn.lpstrFile;
        }
 return "";  // no valid filename entered
}
#endif

#ifndef UseVCLdialogs
char *getsaveCSVfilename()    // for saving files : displays getfilename dialog and returns filename or "" if not entered
{
 // Initialize OPENFILENAME
 memset(&ofn,0, sizeof(ofn));  // zero memory
 ofn.lStructSize = sizeof(ofn);
 ofn.hwndOwner = Application->Handle ;// hwnd even  NULL works.
 ofn.lpstrFile = szFile;
 // Set lpstrFile[0] to '\0' so that GetOpenFileName does not
 // use the contents of szFile to initialize itself.
 ofn.lpstrFile[0] = '\0';
 ofn.nMaxFile = sizeof(szFile);
 ofn.lpstrFilter = "Csv\0*.CSV\0"; // allowed extensions
 ofn.lpstrDefExt="CSV"; // default extension if user does not type one
 ofn.nFilterIndex = 1;
 ofn.lpstrFileTitle = NULL ;
 ofn.nMaxFileTitle = 0;
 ofn.lpstrInitialDir = NULL;
 ofn.lpstrTitle= "Please select the CSV filename you wish to save the data in";
 ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOREADONLYRETURN ;// warn user if file already exists, do not allow read only files or directories

 // Display the Open dialog box.

 if (GetSaveFileName(&ofn)==TRUE)      // GetSaveFileName
        {// Got valid filename
         return   ofn.lpstrFile;
        }
 return "";  // no valid filename entered
}
#endif

// deal with drag n drop
 void __fastcall TPlotWindow::WmDropFiles(TWMDropFiles& Message)
 {
   char buff[MAX_PATH];
   HDROP hDrop = (HDROP)Message.Drop;
   unsigned int numFiles = DragQueryFile(hDrop, -1, NULL, 0);
#if 1
   if( numFiles==1)
		{// just 1 file given - deal with it
#ifdef _WIN64
		 wchar_t wbuff[sizeof(buff)];
		 DragQueryFile(hDrop, 0, wbuff, sizeof(wbuff));
		 char *tbuff=AnsiOf(wbuff);
		 strncpy(buff,tbuff,sizeof(buff)); // need to tke a copy as AnsiOf uses a static buffer and it may be called again...
#else
		 DragQueryFile(hDrop, 0, buff, sizeof(buff));
#endif

         // process the file in 'buff'
         proces_open_filename(buff); // process file
        }
   else
        { ShowMessage("Error: you can only drag one file at a time onto CSVgraph");
        }
#else
   // accept multiple filenames - csvgraph cannot really deal with this and the code below effectively just loads the last one
   for (int i=0;i < numFiles;i++)
     {
      DragQueryFile(hDrop, i, buff, sizeof(buff));
      // process the file in 'buff'
      proces_open_filename(buff); // process file
     }
#endif

   DragFinish(hDrop);
 }

void __fastcall TPlotWindow::CreateParams(TCreateParams &Params)
{ // change format of window created by operating system
  // see https://community.idera.com/developer-tools/b/blog/posts/multiple-windows-in-delphi
  // and http://bcbjournal.org/bcbcaq/articles/fun3.htm

    TForm::CreateParams(Params);  // do all the standard builderc++ things

    Params.ExStyle |= WS_EX_APPWINDOW; // also add the following flag which should make the minimise "icon" work as normally expected
#if 0
    SetWindowLong(Application->Handle, GWL_EXSTYLE,
        GetWindowLong(Application->Handle,GWL_EXSTYLE) & ~WS_EX_APPWINDOW |  WS_EX_TOOLWINDOW );   // changes flags on application window
#endif        
}

static int initial_ClientWidth,initial_ClientHeight,initial_Panel1_Width,initial_Panel1_Height; // sizes when form created

//---------------------------------------------------------------------------
// #define HIGH_DPI /* if true then application has high dpi set in project properties: application -> enable high dpi */
// #define BASE_WIDTH 1609   /* sizes assumed when windows created with form designer */
#define BASE_PWIDTH 1300  /* base panel1 width */
#define BASE_HEIGHT 980

#if 1       /* version that works with possibility to detach panel 2 */
__fastcall TPlotWindow::TPlotWindow(TComponent* Owner) : TForm(Owner)
{
  Shape1->Visible = false;                   //mouse scaling rect
  Caption = Prog_Name;                            //window caption
  FilterType->ItemIndex=0; // select 1st item (none) otherwise no item is selected.
  // save initial sizes [ used when we resize]
  initial_ClientWidth=ClientWidth;
  initial_ClientHeight=ClientHeight;
   // initial_ClientWidth=1591 initial_ClientHeight=980 initial_Panel1_Width=1300 initial_Panel1_Height=980

  Panel2->Width=Edit_y->Left + Edit_y->Width ; // make sure panel 2 is wide enough to fit widest (far right) item
  Panel2->Left=ClientWidth-Panel2->Width ;

  Panel1->Width  =Panel2->Left-Panel1->Left;    // resize if necessary so graph fits in leaving space for controls on right
  Panel1->Height =initial_ClientHeight;
  iBitmapHeight=Panel1->ClientHeight;        //fit TScientificGraph-Bitmap in
  iBitmapWidth=Panel1->ClientWidth;          //TImage1-object in Panel1
											 //init ScientificGraph with Bitmap-
											 //size
  initial_Panel1_Width=iBitmapWidth;
  initial_Panel1_Height=iBitmapHeight;

  // rprintf("initial_Panel1_Width=iBitmapWidth=%d initial_Panel1_Height=iBitmapHeight=%d\n",initial_Panel1_Width,initial_Panel1_Height);


 #if 0
  // code below attempts to realign right control panel but this does not work
  // TRect control_area(ClientLeft,Panel1->Top,Panel1->Left+ClientWidth,Panel1->Top+Panel1->Height);// Left, Top, Right, Bottom
  TRect control_area=ClientRect;
  TPlotWindow::AlignControls(NULL,control_area) ;
#endif
  pScientificGraph = new TScientificGraph(iBitmapWidth,iBitmapHeight);
#ifdef WHITE_BACKGROUND
  pScientificGraph->ColBackGround = clWhite;                           //set colours, white background
  pScientificGraph->ColGrid = clGray	;
  pScientificGraph->ColAxis = clDkGray	;
  pScientificGraph->ColText = clBlack;
  Edit_title->Color=pScientificGraph->ColBackGround ;    // needs to be the same as the background colour
  Edit_title->Font->Color= pScientificGraph->ColText;
  Label1->Font->Color= pScientificGraph->ColText;
  Label2->Font->Color= pScientificGraph->ColText;
  Color=clBtnFace ;
#else
  pScientificGraph->ColBackGround = clBlack;                           //set colours, Black background
  pScientificGraph->ColGrid = clOlive;
  pScientificGraph->ColAxis = clRed;
  pScientificGraph->ColText = clYellow;
  Edit_title->Color=pScientificGraph->ColBackGround ;    // needs to be the same as the background colour
  Edit_title->Font->Color= pScientificGraph->ColText;
  Label1->Font->Color= pScientificGraph->ColText;
  Label2->Font->Color= pScientificGraph->ColText;
  Color=clBtnFace ;
#endif
#define   dLegendStartX_val 0.3  /* start position for graph ledgends X and Y */
#define   dLegendStartY_val 0.99
#if 1  /* scale positions based on size */
 // 0.1*initial_Panel1_Width/Panel1->ClientWidth;
  pScientificGraph->fLeftBorder = 0.1f*BASE_PWIDTH/initial_Panel1_Width;      //Borders of Plot in Bitmap in %/100  was 0.2
  Edit_title->Width=Panel1->Width-Edit_title->Left-20; // allow to go to (nearly) the far edge  (Title is centered)
  pScientificGraph->fBottomBorder = 0.1f*BASE_HEIGHT/initial_Panel1_Height;   // was 0.25
  pScientificGraph->dLegendStartX=dLegendStartX_val; // Top left positions of trace legends in %/100 in Plot was 0.8 - don't need this to change if user rescaled main window
  pScientificGraph->dLegendStartY=dLegendStartY_val; // was 0.95
#else
  pScientificGraph->fLeftBorder = 0.1;      //Borders of Plot in Bitmap in %/100  was 0.2
  pScientificGraph->fBottomBorder = 0.1;   // was 0.25
  pScientificGraph->dLegendStartX=dLegendStartX_val;     //Legend position in %/100 in Plot was 0.8
  pScientificGraph->dLegendStartY=dLegendStartY_val;    // was 0.95
#endif
  pScientificGraph->bZeroLine=true;        //zero line in plot
  Edit_x->Text=default_x_label;            // set default x label
  Edit_y->Text=default_y_label;            // set default y label
  fnReDraw();                               //redraw plot
}
#else
  // original code does not work with panel 2 present when text is not 100%
__fastcall TPlotWindow::TPlotWindow(TComponent* Owner) : TForm(Owner)
{
  int ofset= (ClientWidth-BASE_WIDTH);  // adjust for change in width
  Shape1->Visible = false;                   //mouse scaling rect
  Caption = Prog_Name;                            //window caption
  // save initial sizes [ used when we resize]
  initial_ClientWidth=ClientWidth;
  initial_ClientHeight=ClientHeight;
   // initial_ClientWidth=1591 initial_ClientHeight=980 initial_Panel1_Width=1300 initial_Panel1_Height=980
  Panel1->Width  =Panel1->ClientWidth+(ofset);    // resize if necessary so graph fits in leaving space for controls on right
  Panel1->Height =initial_ClientHeight;
  iBitmapHeight=Panel1->ClientHeight;        //fit TScientificGraph-Bitmap in
  iBitmapWidth=Panel1->ClientWidth;          //TImage1-object in Panel1
											 //init ScientificGraph with Bitmap-
											 //size
  initial_Panel1_Width=iBitmapWidth;
  initial_Panel1_Height=iBitmapHeight;

  // rprintf("initial_Panel1_Width=iBitmapWidth=%d initial_Panel1_Height=iBitmapHeight=%d\n",initial_Panel1_Width,initial_Panel1_Height);
  if(ofset!=0)
	{
#if 0
	 rprintf("initial_ClientWidth=%d initial_ClientHeight=%d initial_Panel1_Width=%d initial_Panel1_Height=%d, ofset=%d\n",
		initial_ClientWidth,initial_ClientHeight,initial_Panel1_Width,initial_Panel1_Height,ofset);
	 rprintf("ListBoxX: top=%d left=%d height=%d width=%d\n",ListBoxX->Top,ListBoxX->Left , ListBoxX->Height , ListBoxX->Width );
#endif
	 if(ofset<0)
		{// not room for all controls on the right bar, disable ones that are not essential , and move the remainder up
		 int voffset=Label6->Top ; // this must be subtracted from Top of remaining visible items to move them up
		 RadioGroup3->Visible =false;
		 RadioGroup2->Visible =false;
		 CheckBox3->Visible =false;
		 CheckBox1->Visible =false;
		 Label5->Visible =false;
		 Label11->Visible =false;
		 SpinEdit_Fontsize->Visible =false;
		 Label6->Top-=voffset;
		 Edit_y->Top-=voffset;
		 Label7->Top-=voffset;
		 Edit_x->Top-=voffset;
		 Label8->Top-=voffset;
		 StaticText_filename->Top-=voffset;
		 Xcol_type->Top-=voffset;
		 Label3->Top-=voffset;
		 Edit_xcol->Top-=voffset;
		 Label9->Top-=voffset;
		 Edit_Xoffset->Top-=voffset;
		 ListBoxX->Top-=voffset;
		 Label4->Top-=voffset;
		 Edit_ycol->Top-=voffset;
		 ListBoxY->Top-=voffset;
		 FilterType->Top-=voffset;
		 CheckBox_Compress->Top-=voffset;
		 Label10->Top-=voffset;
		 Edit_median_len->Top-=voffset;
		 Button_Filename->Top-=voffset;
		 Button_add_trace->Top-=voffset;
		 Button_clear_all_traces->Top-=voffset;
		 StatusText->Top-=voffset;
		 Label15->Top-=voffset;
		 Label16->Top-=voffset;
		 Edit_skip_lines->Top-=voffset;
		}
	 // move all controls in bar on right
	 RadioGroup3->Left+=ofset;
	 RadioGroup2->Left+=ofset;
	 CheckBox3->Left+=ofset;
	 CheckBox1->Left+=ofset;
	 Label5->Left+=ofset;
	 Label11->Left+=ofset;
	 SpinEdit_Fontsize->Left+=ofset;
	 Label6->Left+=ofset;
	 Edit_y->Left+=ofset;
	 Label7->Left+=ofset;
	 Edit_x->Left+=ofset;
	 Label8->Left+=ofset;
	 StaticText_filename->Left+=ofset;
	 Xcol_type->Left+=ofset;
	 Label3->Left+=ofset;
	 Edit_xcol->Left +=ofset;
	 Label9->Left+=ofset;
	 Edit_Xoffset->Left+=ofset;
	 ListBoxX->Left += ofset;
	 Label4->Left+=ofset;
	 Edit_ycol->Left +=ofset;
	 ListBoxY->Left +=ofset;
	 FilterType->Left +=ofset;
	 CheckBox_Compress->Left +=ofset;
	 Label10->Left+=ofset;
	 Edit_median_len->Left +=ofset;
	 Button_Filename->Left +=ofset;
	 Button_add_trace->Left +=ofset;
	 Button_clear_all_traces->Left +=ofset;
	 StatusText->Left +=ofset;
	 Label5->Left+=ofset;
	 Label6->Left+=ofset;
	 Edit_skip_lines->Left+=ofset;
	 // rprintf("updated ListBoxX: top=%d left=%d height=%d width=%d\n",ListBoxX->Top,ListBoxX->Left , ListBoxX->Height , ListBoxX->Width );
	}

 #if 0
  // code below attempts to realign right control panel but this does not work
  // TRect control_area(ClientLeft,Panel1->Top,Panel1->Left+ClientWidth,Panel1->Top+Panel1->Height);// Left, Top, Right, Bottom
  TRect control_area=ClientRect;
  TPlotWindow::AlignControls(NULL,control_area) ;
#endif
  pScientificGraph = new TScientificGraph(iBitmapWidth,iBitmapHeight);
#ifdef WHITE_BACKGROUND
  pScientificGraph->ColBackGround = clWhite;                           //set colours, white background
  pScientificGraph->ColGrid = clGray	;
  pScientificGraph->ColAxis = clDkGray	;
  pScientificGraph->ColText = clBlack;
  Edit_title->Color=pScientificGraph->ColBackGround ;    // needs to be the same as the background colour
  Edit_title->Font->Color= pScientificGraph->ColText;
  Label1->Font->Color= pScientificGraph->ColText;
  Label2->Font->Color= pScientificGraph->ColText;
  Color=clBtnFace ;
#else
  pScientificGraph->ColBackGround = clBlack;                           //set colours, Black background
  pScientificGraph->ColGrid = clOlive;
  pScientificGraph->ColAxis = clRed;
  pScientificGraph->ColText = clYellow;
  Edit_title->Color=pScientificGraph->ColBackGround ;    // needs to be the same as the background colour
  Edit_title->Font->Color= pScientificGraph->ColText;
  Label1->Font->Color= pScientificGraph->ColText;
  Label2->Font->Color= pScientificGraph->ColText;
  Color=clBtnFace ;
#endif
#if 1  /* scale positions based on size */
 // 0.1*initial_Panel1_Width/Panel1->ClientWidth;
  pScientificGraph->fLeftBorder = 0.1*BASE_PWIDTH/initial_Panel1_Width;      //Borders of Plot in Bitmap in %/100  was 0.2
  pScientificGraph->fBottomBorder = 0.1*BASE_HEIGHT/initial_Panel1_Height;   // was 0.25
  pScientificGraph->dLegendStartX=dLegendStartX_val; // Top left positions of trace legends in %/100 in Plot was 0.8 - don't need this to change if user rescaled main window
  pScientificGraph->dLegendStartY=dLegendStartY_val; // was 0.95
#else
  pScientificGraph->fLeftBorder = 0.1;      //Borders of Plot in Bitmap in %/100  was 0.2
  pScientificGraph->fBottomBorder = 0.1;   // was 0.25
  pScientificGraph->dLegendStartX=dLegendStartX_val;     //Legend position in %/100 in Plot was 0.8
  pScientificGraph->dLegendStartY=dLegendStartY_val;    // was 0.95
#endif
  pScientificGraph->bZeroLine=true;        //zero line in plot
  Edit_x->Text=default_x_label;            // set default x label
  Edit_y->Text=default_y_label;            // set default x label
  fnReDraw();                               //redraw plot
}
#endif
//---------------------------------------------------------------------------
void __fastcall TPlotWindow::FormDestroy(TObject *Sender)
{ P_UNUSED(Sender);
  delete pScientificGraph;                  //free memory
}
//---------------------------------------------------------------------------

void __fastcall TPlotWindow::FormClose(TObject *Sender,
      TCloseAction &Action)
{ P_UNUSED(Sender);
  P_UNUSED(Action);
  DragAcceptFiles(Handle, false);    // stop accepting drag n drop files
  exit(1); // close this screen exits application
  // Action = caMinimize;                //not allowed to close the window

}
//---------------------------------------------------------------------------
void __fastcall TPlotWindow::ResizeExecute(TObject *Sender)
{ P_UNUSED(Sender);
  pScientificGraph->fnResize();                        //resize graph
  Image1->Picture->Assign(pScientificGraph->pBitmap);  //copy bitmap
  zoomed=true;
}
//---------------------------------------------------------------------------
void __fastcall TPlotWindow::ShiftYMinusExecute(TObject *Sender)
{ P_UNUSED(Sender);
  pScientificGraph->fnShiftYMinus();                     //shift
  Image1->Picture->Assign(pScientificGraph->pBitmap);    //copy bitmap
  zoomed=true;
}
//---------------------------------------------------------------------------
void __fastcall TPlotWindow::ShiftYPlusExecute(TObject *Sender)
{ P_UNUSED(Sender);
  pScientificGraph->fnShiftYPlus();                      //see above
  Image1->Picture->Assign(pScientificGraph->pBitmap);
  zoomed=true;
}
//---------------------------------------------------------------------------

void __fastcall TPlotWindow::ShiftXMinusExecute(TObject *Sender)
{ P_UNUSED(Sender);
  pScientificGraph->fnShiftXMinus();                     //see above
  Image1->Picture->Assign(pScientificGraph->pBitmap);
  zoomed=true;
}
//---------------------------------------------------------------------------

void __fastcall TPlotWindow::ShiftXPlusExecute(TObject *Sender)
{ P_UNUSED(Sender);
  pScientificGraph->fnShiftXPlus();                      //see above
  Image1->Picture->Assign(pScientificGraph->pBitmap);
  zoomed=true;
}
//---------------------------------------------------------------------------

void __fastcall TPlotWindow::ZoomOutYExecute(TObject *Sender)
{ P_UNUSED(Sender);
  pScientificGraph->fnZoomOutY();                        //see above
  Image1->Picture->Assign(pScientificGraph->pBitmap);
  zoomed=true;
}
//---------------------------------------------------------------------------

void __fastcall TPlotWindow::ZoomInYExecute(TObject *Sender)
{ P_UNUSED(Sender);
  pScientificGraph->fnZoomInY();                         //see above
  Image1->Picture->Assign(pScientificGraph->pBitmap);
  zoomed=true;
}
//---------------------------------------------------------------------------

void __fastcall TPlotWindow::ZoomOutXExecute(TObject *Sender)
{ P_UNUSED(Sender);
  pScientificGraph->fnZoomOutXFromLeft();                //see above
  Image1->Picture->Assign(pScientificGraph->pBitmap);
  zoomed=true;
}
//---------------------------------------------------------------------------

void __fastcall TPlotWindow::ZoomInXExecute(TObject *Sender)
{ P_UNUSED(Sender);
  pScientificGraph->fnZoomInXFromLeft();                 //see above
  Image1->Picture->Assign(pScientificGraph->pBitmap);
  zoomed=true;
}
//---------------------------------------------------------------------------
void __fastcall TPlotWindow::Scales1Click(TObject *Sender)
{ P_UNUSED(Sender);
  pScalesWindow = new TScalesWindow(this,pScientificGraph);      //Input Window
  pScalesWindow->ShowModal();
  pScientificGraph->fnOptimizeGrids();                           //grids
  pScientificGraph->fnPaint();                                   //repaint
  Image1->Picture->Assign(pScientificGraph->pBitmap);            //assign
  zoomed=true;
}
//------------------------------------------------------------------------------
void __fastcall TPlotWindow::AutoScaleExecute(TObject *Sender)
{ P_UNUSED(Sender);
  pScientificGraph->fnAutoScale();                      //autoscale graph
  pScientificGraph->fnPaint();                          //paint
  Image1->Picture->Assign(pScientificGraph->pBitmap);   //copy bitmap
  zoomed=false;
}
//------------------------------------------------------------------------------
// defines for line drawn on right mouse click ("measure"). Used 3 times and each must be identicel for code to work.
#define RT_CLICK_Width 3         /* width of line in pixels */
#define RT_CLICK_Style psSolid   /* other options don't appear to work [ even with width=1] */
#define RT_CLICK_Mode pmXor      /* has to be XOR as we want to be able to erase line be redrawing it */
#define RT_CLICK_Color clWhite   /* white also seems to be the best choice for visibility */
#define RT_CLICK_A_size 6        /* length of arrow, 0 means no arrow drawn on end of line */
static void Mouse_line(TImage * I1,int a,int b,int d,int e)
{// draw line for right mouse key from x=a,y=b to x=d,y=e
 I1->Canvas->Pen->Width=RT_CLICK_Width;
 I1->Canvas->Pen->Style=RT_CLICK_Style;
 I1->Canvas->Pen->Mode=RT_CLICK_Mode;
 I1->Canvas->Pen->Color=RT_CLICK_Color;
 I1->Canvas->MoveTo(a,b);
 I1->Canvas->LineTo(d,e);
#if RT_CLICK_A_size>0
#if 1
 /* draw arrow on line has to use a slightly different approach for vertical lines, >45 deg lines and < 45 deg "horizontal lines" */
 /* Algorithm below believed to be novel, created by Peter Miller 10/2020 */
 /* this version is better in that it avoids "jmp" at 45 deg */
 double m,c; // line is y=mx+c
 if(a!=d)
        {m=(b-e)/(double)(a-d); // a==d is trapped in if above
         c=b-m*a;
        }
 else
        {m=c=0.0; // these values should not ever be used...
        }
 if(a==d )
        {// "vertical line"  (only exactly vertical line processed here [traps m=infinity])
         if(b<e)
                {int t=b; // swap ends
                 b=e;e=t;
                 t=d; d=a; a=t;
                }
         // now b>=e
         I1->Canvas->MoveTo(a,b);
         I1->Canvas->LineTo(a-RT_CLICK_A_size,b-RT_CLICK_A_size);
         I1->Canvas->MoveTo(a,b);
         I1->Canvas->LineTo(a+RT_CLICK_A_size,b-RT_CLICK_A_size);
         I1->Canvas->MoveTo(d,e);
         I1->Canvas->LineTo(d-RT_CLICK_A_size,e+RT_CLICK_A_size);
         I1->Canvas->MoveTo(d,e);
         I1->Canvas->LineTo(d+RT_CLICK_A_size,e+RT_CLICK_A_size);
        }
 else if(fabs(m)>=1)  // was >= 1.0
         { // vertical > 45 deg (not completely vertical thats done above)  y=m*x+c so x=(y-c)/m
         int miny,maxy,x1,x2;
         if(b<e) {miny=b;maxy=e;x1=a;x2=d;} // line from minx to maxx
         else    {miny=e;maxy=b;x1=d;x2=a;}
         double dy=(RT_CLICK_A_size/m)+0.5;  // arrow shape correction, keeps arrow ~ the same size as it rotates
         I1->Canvas->MoveTo(x1,miny); // top
         I1->Canvas->LineTo((int)((miny+RT_CLICK_A_size-c)/m+0.5+RT_CLICK_A_size),(int)(miny+RT_CLICK_A_size-dy));    // 0.5 for rounding float to int
         I1->Canvas->MoveTo(x1,miny); // top
		 I1->Canvas->LineTo((int)((miny+RT_CLICK_A_size-c)/m-0.5-RT_CLICK_A_size),(int)(miny+RT_CLICK_A_size+dy));
         I1->Canvas->MoveTo(x2,maxy); // bottom
		 I1->Canvas->LineTo((int)((maxy-RT_CLICK_A_size-c)/m+0.5+RT_CLICK_A_size),(int)(maxy-RT_CLICK_A_size-dy));
         I1->Canvas->MoveTo(x2,maxy); // bottom
		 I1->Canvas->LineTo((int)((maxy-RT_CLICK_A_size-c)/m-0.5-RT_CLICK_A_size),(int)(maxy-RT_CLICK_A_size+dy));
        }
 else
        { // "horizontal line (< 45 deg)" y=m*x+c
         int minx,maxx,y1,y2;
         if(a<d) {minx=a;maxx=d;y1=b;y2=e;} // line from minx to maxx
         else    {minx=d;maxx=a;y1=e;y2=b;}
         double dx=RT_CLICK_A_size*m+0.5;   // arrow shape correction, keeps arrow ~ the same size as it rotates
         I1->Canvas->MoveTo(minx,y1); // left end
		 I1->Canvas->LineTo((int)(minx+RT_CLICK_A_size+dx),(int)(m*(minx+RT_CLICK_A_size)+c-RT_CLICK_A_size-0.5));    // 0.5 for rounding float to int
         I1->Canvas->MoveTo(minx,y1); // left end
		 I1->Canvas->LineTo((int)(minx+RT_CLICK_A_size-dx),(int)(m*(minx+RT_CLICK_A_size)+c+RT_CLICK_A_size+0.5));
         I1->Canvas->MoveTo(maxx,y2); // right end
		 I1->Canvas->LineTo((int)(maxx-RT_CLICK_A_size+dx),(int)(m*(maxx-RT_CLICK_A_size)+c-RT_CLICK_A_size-0.5));
         I1->Canvas->MoveTo(maxx,y2); // right end
		 I1->Canvas->LineTo((int)(maxx-RT_CLICK_A_size-dx),(int)(m*(maxx-RT_CLICK_A_size)+c+RT_CLICK_A_size+0.5));
        }
#else    /* use v12.2 algorithm (which is not bad) */
 /* draw arrow on line has to use a slightly different approach for vertical lines, >45 deg lines and < 45 deg "horizontal lines" */
 /* Algorithm below believed to be novel, created by Peter Miller 10/2020 */
 double m,c; // line is y=mx+c
 if(a!=d)
        {m=(b-e)/(double)(a-d); // a==d is trapped in if above
         c=b-m*a;
        }
 else
        {m=c=0.0; // these values should not ever be used...
        }
 if(a==d )
        {// "vertical line"  (only exactly vertical line processed here [traps m=infinity])
         if(b<e)
                {int t=b; // swap ends
                 b=e;e=t;
                 t=d; d=a; a=t;
                }
         // now b>=e
         I1->Canvas->MoveTo(a,b);
         I1->Canvas->LineTo(a-RT_CLICK_A_size,b-RT_CLICK_A_size);
         I1->Canvas->MoveTo(a,b);
         I1->Canvas->LineTo(a+RT_CLICK_A_size,b-RT_CLICK_A_size);
         I1->Canvas->MoveTo(d,e);
         I1->Canvas->LineTo(d-RT_CLICK_A_size,e+RT_CLICK_A_size);
         I1->Canvas->MoveTo(d,e);
         I1->Canvas->LineTo(d+RT_CLICK_A_size,e+RT_CLICK_A_size);
        }
 else if(fabs(m)>=1.0)  // was >= 1.0
         { // vertical > 45 deg (not completely vertical thats done above)  y=m*x+c so x=(y-c)/m
         int miny,maxy,x1,x2;
         if(b<e) {miny=b;maxy=e;x1=a;x2=d;} // line from minx to maxx
         else    {miny=e;maxy=b;x1=d;x2=a;}
         double dx=0.2*(RT_CLICK_A_size/m);  // arrow shape correction, keeps arrow ~ the same size as it rotates
         I1->Canvas->MoveTo(x1,miny); // top
         I1->Canvas->LineTo((miny+RT_CLICK_A_size-c)/m+0.5+RT_CLICK_A_size+fabs(dx),miny+RT_CLICK_A_size-dx-0.5);    // 0.5 for rounding float to int
         I1->Canvas->MoveTo(x1,miny); // top
         I1->Canvas->LineTo((miny+RT_CLICK_A_size-c)/m-0.5-RT_CLICK_A_size-fabs(dx),miny+RT_CLICK_A_size+dx+0.5);
         I1->Canvas->MoveTo(x2,maxy); // bottom
         I1->Canvas->LineTo((maxy-RT_CLICK_A_size-c)/m+0.5+RT_CLICK_A_size+fabs(dx),maxy-RT_CLICK_A_size-dx-0.5);
         I1->Canvas->MoveTo(x2,maxy); // bottom
         I1->Canvas->LineTo((maxy-RT_CLICK_A_size-c)/m-0.5-RT_CLICK_A_size-fabs(dx),maxy-RT_CLICK_A_size+dx+0.5);
        }
 else
        { // "horizontal line (< 45 deg)" y=m*x+c
         int minx,maxx,y1,y2;
         if(a<d) {minx=a;maxx=d;y1=b;y2=e;} // line from minx to maxx
         else    {minx=d;maxx=a;y1=e;y2=b;}
         double dy=0.2*RT_CLICK_A_size*m;   // arrow shape correction, keeps arrow ~ the same size as it rotates
         I1->Canvas->MoveTo(minx,y1); // left end
         I1->Canvas->LineTo(minx+RT_CLICK_A_size+dy+0.5,m*(minx+RT_CLICK_A_size)+c-RT_CLICK_A_size-0.5-fabs(dy));    // 0.5 for rounding float to int
         I1->Canvas->MoveTo(minx,y1); // left end
         I1->Canvas->LineTo(minx+RT_CLICK_A_size-dy-0.5,m*(minx+RT_CLICK_A_size)+c+RT_CLICK_A_size+0.5+fabs(dy));
         I1->Canvas->MoveTo(maxx,y2); // right end
         I1->Canvas->LineTo(maxx-RT_CLICK_A_size+dy+0.5,m*(maxx-RT_CLICK_A_size)+c-RT_CLICK_A_size-0.5-fabs(dy));
         I1->Canvas->MoveTo(maxx,y2); // right end
         I1->Canvas->LineTo(maxx-RT_CLICK_A_size-dy-0.5,m*(maxx-RT_CLICK_A_size)+c+RT_CLICK_A_size+0.5+fabs(dy));
        }
#endif
#endif
}

void __fastcall TPlotWindow::Image1MouseMove(TObject *Sender,
      TShiftState Shift, int X, int Y)
{ P_UNUSED(Sender);
  double dKoordX,dKoordY;
  AnsiString AString;
  
  if (pScientificGraph->fnPoint2Koord(X,Y,dKoordX,dKoordY))
  {
#if 1 /* 1 for normal use, 0 for debugging */
	AString="(";                                          //position of mouse in "graph" units
	AString+=FloatToStrF(dKoordX,ffGeneral,10,2);         //precision of printing  was 7 , updated to 10 to match max resolution on graph axes
    AString+=";";
    AString+=FloatToStrF(dKoordY,ffGeneral,10,2);
    AString+=")";
#else
    AString="(";                                         //momentary cursor
	AString+=X;
	AString+=";";
    AString+=Y;
    AString+=") L=";
    AString+=pScientificGraph->fnLeftBorder();
    AString+=" R=";
    AString+=pScientificGraph->fnRightBorder();
    AString+=" T=";
    AString+=pScientificGraph->fnTopBorder();
    AString+=" B=";
    AString+=pScientificGraph->fnBottomBorder();

#endif
        // for debug print X,Y
    Label2->Caption=AString;
  }
  else {Label2->Caption="";}

  if (/*Shape1->Visible && */ (Shift.Contains(ssLeft) || Shift.Contains(ssRight)))  //select. rect. visible and a mouse key pressed
  {
    if(Shift.Contains(ssRight) )
        { // Right mouse button shows a line : first delete previous line
         Mouse_line(Image1,iShape1X,iShape1Y,Shape1->Left,Shape1->Top);
        }
    if (X<pScientificGraph->fnLeftBorder())
        {X=pScientificGraph->fnLeftBorder();}
    if (X>pScientificGraph->fnRightBorder())
        {X=pScientificGraph->fnRightBorder();}
    if (Y<pScientificGraph->fnTopBorder())
        {Y=pScientificGraph->fnTopBorder();}
    if (Y>pScientificGraph->fnBottomBorder())
        {Y=pScientificGraph->fnBottomBorder();}
    if(Shift.Contains(ssRight))
        { // Right mouse button shows a line : add new line
         Shape1->Left=X;// remember end of line so we can erase it!
         Shape1->Top=Y;
         Mouse_line(Image1,iShape1X,iShape1Y,X,Y);
        }
    else
     {// left mouse button pressed - draw a rectangle
      if (Y<iShape1Y)                                    //adjust y size
        {
         Shape1->Top=Y;
         Shape1->Height=iShape1Y-Y;
        }
      else
        {
         Shape1->Top=iShape1Y;
         Shape1->Height=Y-iShape1Y;
        }
      if (X<iShape1X)                                    //adjust x size
        {
         Shape1->Left=X;
         Shape1->Width=iShape1X-X;
        }
      else
        {
         Shape1->Left=iShape1X;
         Shape1->Width=X-iShape1X;
        }
     }
  }
  else
  { Shape1->Visible=false;    // make sure shape only shown when a mouse key pressed
  }

}
//---------------------------------------------------------------------------
void __fastcall TPlotWindow::Image1MouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{ P_UNUSED(Sender);
  P_UNUSED(Shift);
  double dX1,dX2,dY1,dY2;

  if (Shape1->Visible && Button==mbLeft && (X!=iShape1X)&&(Y!=iShape1Y)) /* if mouse not moved in X and Y then ignore zoom [would give infinite zoom]*/
   { // left mouse button = "Zoom"
    if (X<pScientificGraph->fnLeftBorder())        //end point has to be in plot
        {X=pScientificGraph->fnLeftBorder();}
    if (X>pScientificGraph->fnRightBorder())
        {X=pScientificGraph->fnRightBorder();}
    if (Y<pScientificGraph->fnTopBorder())
        {Y=pScientificGraph->fnTopBorder();}
    if (Y>pScientificGraph->fnBottomBorder())
        {Y=pScientificGraph->fnBottomBorder();}
    pScientificGraph->fnPoint2Koord(X,Y,dX1,dY1);       //calculate coordinates
    pScientificGraph->fnPoint2Koord(iShape1X,iShape1Y,dX2,dY2);
    pScientificGraph->fnSetScales(dX1,dX2,dY1,dY2);     //set new scales
    TPlotWindow::fnReDraw();
    zoomed=true;
    Shape1->Visible=false;                              //hide shape
   }
  else if(Button==mbRight)
        {// right mouse button => show co-ords and size of selection in messagebox.
         char buf[200];
         double xmin,xmax,ymin,ymax;
         pScientificGraph->fnPoint2Koord(Shape1->Left,Shape1->Top,dX1,dY1);       //calculate coordinates  use end of line already drawn
         pScientificGraph->fnPoint2Koord(iShape1X,iShape1Y,dX2,dY2);
         if(dX1<dX2) {xmin=dX1; xmax=dX2;}
         else {xmin=dX2; xmax=dX1;}
         if(dY1<dY2) {ymin=dY1; ymax=dY2;}
         else {ymin=dY2; ymax=dY1;}
         buf[0]=0;
         if(xmin==xmax && ymin==ymax)
                {// user pressed and released right mouse button on a point without moving mouse
                 snprintf(buf,sizeof(buf),"X = %g\nY = %g",xmin,ymin);
                }
         else if(xmin==xmax)
                { // only y changed
				 snprintf(buf,sizeof(buf),"X = %g\nY from %g to %g (size %g)",xmin,ymin,ymax,ymax-ymin);
                }
          else if(ymin==ymax)
                { // only x changed
				 snprintf(buf,sizeof(buf),"X from %g to %g (size %g)\nY = %g",xmin,xmax,xmax-xmin,ymin);
                }
         else
				{// normal case , both changed   [ means we cannot have a divide by zero error on slope ]
				 snprintf(buf,sizeof(buf),"X from %g to %g (size %g)\nY from %g to %g (size %g)\nSlope dY/dX=%g",xmin,xmax,xmax-xmin,ymin,ymax,ymax-ymin,(ymax-ymin)/(xmax-xmin));
                }
         rprintf("%s\n",buf); // record on text form as well as telling user now via a messagebox
         ShowMessage(buf);
         // Right mouse button showed a line : we need to delete this now
         Mouse_line(Image1,iShape1X,iShape1Y,Shape1->Left,Shape1->Top);
         Shape1->Visible=false;
        }
  else
   {// if mouse not moved
    Shape1->Visible=false;                              //hide shape
   }
}
//---------------------------------------------------------------------------
void __fastcall TPlotWindow::Image1MouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{ // P_UNUSED(Sender);
  // P_UNUSED(Button);
  P_UNUSED(Shift);
  if(Button==mbMiddle)
        {
         TPlotWindow::AutoScaleExecute(Sender);
         zoomed=false;
         Shape1->Visible=false;
         return;
        }
  if(Shape1->Visible==true)
        {Shape1->Visible=false ; // should not happen, but just in case...
        }
  else
  {
     if (X<pScientificGraph->fnLeftBorder())        //point has to be in plot
        {X=pScientificGraph->fnLeftBorder();}
    if (X>pScientificGraph->fnRightBorder())
        {X=pScientificGraph->fnRightBorder();}
    if (Y<pScientificGraph->fnTopBorder())
        {Y=pScientificGraph->fnTopBorder();}
    if (Y>pScientificGraph->fnBottomBorder())
        {Y=pScientificGraph->fnBottomBorder();}
    Shape1->Top=Y;                        //start position for selection rectangle/line
    iShape1Y=Y;                           //save start position
    Shape1->Left=X;
    iShape1X=X;
    Shape1->Height=0;
    Shape1->Width=0;
    if(Button==mbLeft) Shape1->Visible=true;                 //show  box only for left mouse press
    else if(Button==mbRight)
        {// make initial point visible
         Mouse_line(Image1,iShape1X,iShape1Y,Shape1->Left,Shape1->Top);
        }
  }
}
//---------------------------------------------------------------------------
void TPlotWindow::fnReDraw()
{
  pScientificGraph->fnSetGrids(CheckBox1->Checked);             //show grids?

  pScientificGraph->fnCheckScales();                            //range checking
  pScientificGraph->fnOptimizeGrids();                          //opt. grids
                                                                //and ticks
  pScientificGraph->fnScales2Size();                            //set scales as
                                                                //size
  pScientificGraph->fnPaint();                                  //repaint
  Image1->Picture->Assign(pScientificGraph->pBitmap);           //assign
}

//------------------------------------------------------------------------------


void __fastcall TPlotWindow::Button_clear_all_traces1Click(TObject *Sender)
{ P_UNUSED(Sender);
  if(addtraceactive || xchange_running!=-1)
	{// appear to still be busy doing something - ask user if this is true
	 UnicodeString errorText="csvgraph appears to still be busy - continue waiting?";
	 if( MessageDlg(errorText,mtConfirmation,TMsgDlgButtons() <<mbYes <<mbNo ,0) == mrNo)
		{addtraceactive= false;   // if user says "no" then clear flags  (if they say "yes" don't clear flags and we will return below)
		 xchange_running= -1;
		}
	}


  if(addtraceactive) return; // if still processing a previous call of addtrace return now
  if(xchange_running!=-1) return; // still processing a change in x offset
  rprintf("Clear all traces button pressed\n");
  StatusText->Caption="Clearing all traces ...";
  Application->ProcessMessages(); /* allow windows to update (but not go idle) */
  pScientificGraph->fnClearAll();  //clear all graphs
  line_colour=0; // always start with the same line colour
  Edit_x->Text=default_x_label;
  pScientificGraph->XLabel=default_x_label;
  Edit_y->Text=default_y_label;
  pScientificGraph->YLabel1=default_y_label;   // 2 Y axis labels (2 lines allowed)
  pScientificGraph->YLabel2="";
  zoomed=false;
  fnReDraw();
  StatusText->Caption="Cleared all traces";
}
//---------------------------------------------------------------------------


#ifdef Allow_dollar_vars_in_expr
double get_dollar_var_value(pnlist p) /* get value of $ variable */
        {char *v=p->name;
         unsigned int i=0;
		 static int count=10; // set to 10 to disable debuging, 0 for debug
         if(count<10)
				{rprintf("call %d to get_dollar_var_value(),count, name=%s\n",count,v);
                 for(unsigned int j=0;j<MAX_COLS;++j)
						rprintf("Col %-3d : %s\n",j+1,col_ptrs[j]);
                }
         if(*v++!='$')
				return 0; // oops should only be called for $nnn variables
#ifdef Allow_dollar_T
		 bool dollar_T_found=false;
		 if(*v=='t' || *v=='T')
			{dollar_T_found=true;
			 ++v;
			 }
#endif
		 while(isdigit(*v)) i=i*10+(unsigned int)(*v++ -'0'); // get number after $
		 i--; // internally $1 = array[0]
#ifdef Allow_dollar_T
		 if(dollar_T_found)
			{// found $Tn
			 // float fnAddDataPoint_thisy(int iGraphNumber)
			 return Form1->pPlotWindow->pScientificGraph->fnAddDataPoint_thisy((int)i);
			}
#endif
		 // if not $Tn must just be $n
         if(col_ptrs==NULL || i>= MAX_COLS || col_ptrs[i]==NULL || *(col_ptrs[i])==0 ) return 0;
         if(count<10)
                {rprintf("call to get_dollar_var_value(), i=%u, $i=%s (%g)\n",i,(i<MAX_COLS)?col_ptrs[i]:"error",(i<MAX_COLS)?atof(col_ptrs[i]):0.0);
                 count++;
                }
         if(i<MAX_COLS)
                {char *ys=col_ptrs[i];
                 while(isspace(*ys)) ++ys; // skip any leading whitespace
                 if(*ys=='"')
                      {++ys;// skip " if present
                       while(isspace(*ys)) ++ys; // skip any more whitespace
                     }
                 // hopefully just a number left, atof will terminate after the number so trailing whitespace and "'s will be ignored
                 return atof(ys) ;
                }
         else return 0;
        }
#endif

static void filter_callback(size_t i, size_t imax) // callback function from filters  used to display progress
{ static char cstring[32]; // small buffer
  snprintf(cstring,sizeof(cstring),"Filter: %.0f%% complete",100.0*(double)i/(double)imax);
  Form1->pPlotWindow->StatusText->Caption=cstring;
  Application->ProcessMessages(); /* allow windows to update (but not go idle) */
  // rprintf("%s\n",cstring);
}


void TPlotWindow::gen_lin_reg(enum reg_types r,int N,bool write_y,int iGraph)
 // fit fn() with N variables (ie for y=m*x+c N=2)  for trace iGraph
 // if write_y true then write back result of fit to y_values of trace
 {// test code uses general linear least squares fitting
  float *x_arr,*y_arr;
  // test multiple_lin_reg_fn
  size_t iCount=pScientificGraph->fnGetxyarr(&x_arr,&y_arr,iGraph); // allows access to x and y arrays of specified graph, returns nos points
  matrix_ld S;// 2D matrix
  long double *A;   // long double A[N+1]  A = average (mean)
  long double *X; // long double X[N+1];
  bool *U; // bool U[N+1];
  long double T,C;
  S=cr_matrix_ld((size_t)(N+1),(size_t)(N+1));// S[N+1][N+1]
  A=(long double *)calloc((size_t)(N+1),sizeof(long double));
  X=(long double *)calloc((size_t)(N+1),sizeof(long double));
  U=(bool *)calloc((size_t)(N+1),sizeof(bool));
  if(S==NULL || A==NULL || X==NULL || U==NULL)
	{rprintf("Warning - gen_lin_reg() out of RAM\n");
	 if(S!=NULL) fr_matrix_ld(S);
	 if(A!=NULL) free(A);
	 if(X!=NULL) free(X);
	 if(U!=NULL) free(U);
	 return;
	}
  filter_callback(0,3);
  switch(r)
	{case reg_poly: rprintf("Fitting data to polynomial using general least squares linear regression function order=%d:\n",N-1);
					break;
	 case reg_sqrt: rprintf("Fitting data to polynomial in sqrt(x) using general least squares linear regression function order=%d:\n",N-1);
					break;
	 case reg_rat: rprintf("Fitting data to rational function using general least squares linear regression function order=%d:\n",N-1);
					break;
	}
  multi_regression(x_arr,y_arr,r,N,iCount,S,A,U,0.0,filter_callback); // fit specified function
  C=A[1];// constant term in resultant equation
  for(int j = 2;j<= N;++j)
	{
	 if(U[j] == true)
		{
		 // rprintf("  Var[%d] used, multiplier(value-mean)=%g mean=%g\n",j,S[j][1],A[j]);
		 C-= S[j][1]*A[j];
		}
	 else S[j][1]=0.0; // if variable not used set it to zero (avoids needing to check for this case in code below)
	}
  // print equation
  if(r==reg_poly)
	{
	 rprintf("  Y=%g",(double)C);
	 for(int j = 2;j<= N;++j)
		{
		 T=S[j][1];
		 if(j==2)
			rprintf("%+g*X",(double)T);
		 else
			{ if(U[j]) // only print no-zero coeffs
				rprintf("%+g*X^%d",(double)T,j-1);
			}
		}
	 rprintf("\n");
	}
  else if(r==reg_sqrt)
	{
	 rprintf("  Y=%g",(double)C);
	 for(int j = 2;j<= N;++j)
		{
		 T=S[j][1];
		 if(j==2)
			rprintf("%+g*sqrt(X)",(double)T);
		 else if(j==3)
			rprintf("%+g*X",(double)T);
		 else
			{ if(U[j]) // only print no-zero coeffs
				rprintf("%+g*X^%g",(double)T,0.5*(j-1));
			}
		}
	 rprintf("\n");
	}
  else if(r==reg_rat)
	{int j;
	 if(N==1) rprintf("  Y=%g\n",(double)C);
	 else
		{
		 rprintf("  Y=(%g",(double)C);
		 for(j = 2;j<= (N+1)/2;++j)
			{
			 T=S[j][1];
			 if(j==2)
				rprintf("%+g*X",(double)T);
			 else
				{ if(U[j]) // only print no-zero coeffs
					rprintf("%+g*X^%d",(double)T,j-1);
				}
			}
		 T=S[j][1];
		 rprintf(")/(1.0%+g*X",(double)T);
		 for(++j;j<=N;++j)
			{
			 T=S[j][1];
			 if(U[j]) // only print no-zero coeffs
				rprintf("%+g*X^%d",(double)T,j-(N+1)/2);
			}
		 rprintf(")\n");
		}
	}

  long double max_err = 0.0; // max abs error
  for(size_t i=0; i<iCount;++i)
	{long double x=x_arr[i],y=y_arr[i];
	 if(r==reg_rat)
		{
		 /* general case */
		 int j;
		 long double top,bott,P=x; // result = top/bott. P is power of x
		 top=C;
		 for(j = 2;j<= (N+1)/2;++j)
			{
			 T=S[j][1];
			 top+=T*P;
			 P*=x;
			}
		 bott=1.0;P=x;  // bott =1 + b1*x+b2*x^2
		 for(;j<=N;++j)
			{
			 T=S[j][1];
			 bott+=T*P;
			 P*=x;
			}
		 if(bott==0)
			X[1]=0;  // trap divide by zero
		 else
			X[1]=top/bott;
		}
	 else if(r==reg_poly)
		{
		 long double top,P=x; // result = top. P is power of x
		 top=C;
		 for(int j = 2;j<=N;++j)
			{
			 T=S[j][1];
			 top+=T*P;
			 P*=x;
			}
		 X[1]=top;
		}
	 else if(r==reg_sqrt)
		{x=sqrt(x);
		 long double top,P=x; // result = top. P is power of sqrt(x)
		 top=C;
		 for(int j = 2;j<=N;++j)
			{
			 T=S[j][1];
			 top+=T*P;
			 P*=x;
			}
		 X[1]=top;
		}
	 T = X[1] - y; // error
	 if(T<0) T= -T; // T is now abs error
	 if(T>max_err) max_err=T; // max abs error
	 if(write_y) y_arr[i]=(float)X[1]; // set y value to that calculated by the function
	}
  rprintf("  max abs error of general least squares fit is %g\n",(double)max_err);
  filter_callback(3,3);
  fr_matrix_ld(S);
  free(A);free(X);free(U); // free up dynamically allocated arrays   we know none are NULL as this is trapped much earlier
 }

#if 1
static void deriv_trace(int iGraph)
{ // take derivative of specified trace . No filtering (that can be applied using csvgraph if required then accessed with $Tn
  // derivative is based on previous point and next point (that means the derivative is centered around the point)
  // in general the results from this version "look better" that the alternative below
  float *x_arr,*y_arr;
  double d,ld;
  size_t iCount=Form1->pPlotWindow->pScientificGraph->fnGetxyarr(&x_arr,&y_arr,iGraph); // allows access to x and y arrays of specified graph, returns nos points
  size_t i,j,k;
  if(iCount<2) return;
  // 1st point - can only look forward to next point
  if(x_arr[1]==x_arr[0]) ld=0;// avoid divide by zero when x value does not change
  else ld=(y_arr[1]-y_arr[0])/(x_arr[1]-x_arr[0]); // slope
  for(i=1; i<iCount-1;++i)  // for all points bar the first & last , use y values either side to calculate slope
	{j=i+1;
	 k=i-1;
	 if(x_arr[j]==x_arr[k]) d=0;// avoid divide by zero when x value does not change
	 else d=(y_arr[j]-y_arr[k])/(x_arr[j]-x_arr[k]); // slope
	 y_arr[k]=(float)ld; // can only write back values 1 step behind as orig value for [i] is needed on next iteration
	 ld=d;
	}
  // last point can only look back to the point before before
  k=i-1;
  if(x_arr[i]==x_arr[k]) d=0;// avoid divide by zero when x value does not change
  else d=(y_arr[i]-y_arr[k])/(x_arr[i]-x_arr[k]); // slope
  y_arr[k]=(float)ld; // previous point
  y_arr[i]=(float)d; // last point
}
#else
void deriv_trace(int iGraph)
{ // take derivative of specified trace . No filtering (that can be applied using csvgraph if required then accessed with $Tn
  // derivative is based on current point and next point (so slope appears by eye to be ofset by ~ 1/2 a sample).
  float *x_arr,*y_arr;
  double d=0;
  unsigned int iCount=Form1->pPlotWindow->pScientificGraph->fnGetxyarr(&x_arr,&y_arr,iGraph); // allows access to x and y arrays of specified graph, returns nos points
  int i,j;
  if(iCount<2) return;
  for(i=0; i<iCount-1;++i)  // for all points bar the last
	{j=i+1;
	 if(x_arr[j]==x_arr[i]) d=0;// avoid divide by zero when x value does not change
	 else d=(y_arr[j]-y_arr[i])/(x_arr[j]-x_arr[i]); // slope
	 y_arr[i]=d;
	}
  y_arr[i]=d; // last point has same slope as the point before
}
#endif
static void integral_trace(int iGraph)
{ // take integral of specified trace .  Uses trapezoidal rule for integration
  float *x_arr,*y_arr,y_i;
  double integral=0;
  size_t iCount=Form1->pPlotWindow->pScientificGraph->fnGetxyarr(&x_arr,&y_arr,iGraph); // allows access to x and y arrays of specified graph, returns nos points
  size_t i,j;
  if(iCount<2) return;
  y_i=y_arr[0]; // need this before its overwritten
  y_arr[0]=0;         // 1st point has integral zero
  for(i=0; i<iCount-1;++i)  // for all the rest of the points add in extra area between adjacent points
	{j=i+1;
	 integral+=0.5*(y_i+y_arr[j])*(x_arr[j]-x_arr[i]); // trapezoidal rule for integration
	 y_i=y_arr[j];// this y[j] will be y[i] in the next iteration  (its about to be overwritten).
	 y_arr[j]=(float)integral;
	}
}

void __fastcall TPlotWindow::Button_add_trace1Click(TObject *Sender)
{  // add graph
   // here we want to display a csv file
  P_UNUSED(Sender);
  int iGraph;
  bool yexpr; // set to true if ycol contains an expression rather than just a number
  unsigned short ucMask;
  AnsiString FString; // Name of filter selected , "" if no filter selected
  static char cstring[64]; // small buffer  to use with snprintf
  TColor Col;
  FILE *fin;
  int lines_in_file;  // line number within file being read
  char *csv_line=NULL;// line of csv file
  char *st;
  int xcol,ycol;
  int64_t filesize;    // gives filesize which is then used to show progress as a %
  bool xmonotonic=true; // true is trace is monotonic in x
  bool firstxvalue; // true on 1st x value
  bool gotyvalue; // set to true when a valid number found for y value
  bool skipy; // set to true if we skip a string of identical y values
  bool compress;
  bool showsortmessage=true; // only show "sort message" once per press of the "add traces" button
  bool showerrmessage=true; // ditto for message to warn users of errors in format of csv file
  float previousxvalue=0,previousyvalue=0;
  double median_ahead_t; // >0 for median filtering to be used
  int poly_order=2;
  long double first_time=0; // used when reading times in for x axis, store times relative to 1st time
  int nos_traces_added=0;
  bool allvalidxvals=true;
  double x_offset;
  clock_t start_t,end_t;
  int nos_errs=0; // count of errors found when reading values
#define MAX_ERRS 2 /* max errors that will be displayyed in full */
  /* defined below allow 1 example of each error type to be displayed to the user */
#define ERR_TYPE1 0
#define ERR_TYPE2 1
#define ERR_TYPE3 2
#define ERR_TYPE4 3
#define ERR_TYPE5 4
#define ERR_TYPE6 5
#define ERR_TYPE7 6
#define MAX_ERR_TYPES 7
  bool found_error_type[MAX_ERR_TYPES];
try{
  for(int i=0;i<MAX_ERR_TYPES;++i) found_error_type[i]=false;// set to true when an example of this type of error is found
  if(addtraceactive || xchange_running!=-1)
	{// appear to still be busy doing something - ask user if this is true
	 UnicodeString errorText="csvgraph appears to still be busy - continue waiting?";
	 if( MessageDlg(errorText,mtConfirmation,TMsgDlgButtons() <<mbYes <<mbNo ,0) == mrNo)
		{addtraceactive= false;   // if user says "no" then clear flags  (if they say "yes" don't clear flags and we will return below)
		 xchange_running= -1;
		}
	}
  start_t=clock(); // want total execution time
  if(xchange_running!=-1) return; // still processing a change in x offset
  if(addtraceactive) return; // if still processing a previous call of addtrace
  addtraceactive=true;
  if(filename=="")
		{ShowMessage("You must press \"Set Filename\" button to set the filename 1st");
         StatusText->Caption="No filename set";
         addtraceactive=false;// finished
         return;
         }
   compress=CheckBox_Compress->Checked;
   median_ahead_t=atof(AnsiOf(Edit_median_len->Text.c_str()));
   // poly_order=_wtoi(Polyorder->Text.c_str());  // polynomial order for poly fit
   poly_order=(int)atof(AnsiOf(Polyorder->Text.c_str()));  // polynomial order for poly fit
   extern int my_initialisation;
   // rprintf("my_initialisation=%d (should be 12345)\n",my_initialisation);    // used to check _tMinMain() ran OK
#if 1
   if(poly_order<1)
		{ShowMessage("Warning: polynomial order set to 0, but its minimum value is 1 - I will set it to 1");
		 poly_order=1;// min of 1   [ without this get divide by zero error when compiled for w64 ! {before _controlfp() added to csvgraph.cpp} ]
		 Polyorder->Text="1"; // actuall set it in the gui
		}
#endif
   //rprintf("Poly_order=%d\n",poly_order);
   if(FilterType->ItemIndex>0)
		FString=FilterType->Items->Strings[FilterType->ItemIndex]; // get name of filter user has specified directly from Listbox "FilterType"
   else FString="" ; // "" means no filtering
   bool is_filter=strstr(FString.c_str(),"Filter")!= NULL;  // true if "filter" appears in the text
   bool is_fft=strstr(FString.c_str(),"FFT")!= NULL;  // true if "FFT" appears in the text
   bool is_order=strstr(FString.c_str(),"order:")!= NULL;// true if "order:" appears in string (ie order must be set by user)
   if(is_filter &&  median_ahead_t<=0)
		{ShowMessage("Request to filter ignored as filter time constant has not been set");
		 FString="";
		}
   else if(is_order)
		{// general poly or rational function fit , order = 0 is OK  and is unsigned so cannot go negative
		 if(compress)  ShowMessage("Warning: both compress and fit requested so fitting will be done on compressed data");
		 // change "order:" to "order %u "
		 int so=FString.Pos("order:");
		 int LF=FString.Length() ;
		 AnsiString FS1=FString;
		 FString.SetLength(so-1); // get rid of order:  and anything after it
		 snprintf(cstring,sizeof(cstring),"order %u ",poly_order);  // replace "order: with new text
		 FString=FString+cstring+FS1.SubString(so+6,LF-(so-1+6));      // add on origonal text that was after "order:" (length 6)
		}
   else if(compress && FilterType->ItemIndex !=0)
		{ShowMessage("Warning: both compress and filter requested so filtering will be done on compressed data");
		}


   // use combination thats fastest  (binary and big buffer) - which ~ halves time
#ifdef BIG_BUF_SIZE
   static char *read_buf=NULL ;   // static so we only malloc it once
   if(read_buf==NULL)
		{read_buf=(char *)malloc(BIG_BUF_SIZE) ;
		 if( read_buf==NULL)
				{
				 StatusText->Caption="No free RAM!";
				 ShowMessage("No free RAM! cannot malloc space for read_buf");
				 // continue anyway - just cannot use big read buffer
				}
		}
#endif
   fin=fopen(filename.c_str(),"rb");
   if(fin==NULL)
		{ShowMessage("Error: cannot open file"+filename);
		 addtraceactive=false;// finished
		 return;
		}
  _fseeki64(fin, 0, SEEK_END);  // seek to end of file
  filesize=_ftelli64(fin); // get size of file
  _fseeki64(fin,0,SEEK_SET); // back to start of file
#ifdef BIG_BUF_SIZE
   if( read_buf!=NULL)
		setvbuf(fin,read_buf,_IOFBF,BIG_BUF_SIZE); // buffer input if we have free RAM
#endif

  // rprintf("filename selected is %s\n",filename.c_str());
  {int skip_initial_lines=_wtoi(Form1->pPlotWindow->Edit_skip_lines->Text.c_str());
   for(int l=0;l<=skip_initial_lines;++l)    // need to read 1 line if skip=0, 2 lines for skip=1, etc
	 csv_line=readline(fin);
  }
  if(csv_line==NULL)
        {ShowMessage("Error: cannot read headers from file "+filename);
         fclose(fin);
         filename="";
         StatusText->Caption="No filename set";
         addtraceactive=false;// finished
         return;
        }

  StatusText->Caption="Reading File";
  /* need to scan headers again , as need to have valid pointers in col_ptrs in case ycol is an expression */
  parsecsv(csv_line,col_ptrs, MAX_COLS);
  //xcol must be an unsigned integer number or $number  (unless x source is "linenumber")
  char *s=strdup(AnsiOf(Edit_xcol->Text.c_str())),*xs,*ns; // must take a copy of string as if we just use pointer supplied below strange things happen !
  if(s==NULL)
        {
         ShowMessage("Error (No RAM): invalid xcol ["+Edit_xcol->Text+"] (valid range 1.."+AnsiString(MAX_COLS)+")");
         fclose(fin);
         StatusText->Caption="Invalid xcol";
         addtraceactive=false;// finished
         return;
        }
  xs=s; // save copy so we can free s at the end
  if(Xcol_type->ItemIndex==0)
    { // use line number  so X column value on gui is ignored (and so can be invalid)
     xcol=1;// sensible default so tests passed without needing lots of special cases
    }
  else
    {// need a valid value for X column
     while(isspace(*xs)) ++xs; // skip optional leading whitespace
     if(*xs=='$') ++xs; // allow for an optional $
     ns=xs; // start of actual number
     while(isdigit(*xs)) ++xs; // number  - no sign
     while(isspace(*xs)) ++xs; // optional trailing whitespace
     if(*xs!=0)    // should be at the end of the string if its just an unsigned integer number
        {// not a number
         ShowMessage("Error: invalid xcol ["+Edit_xcol->Text+"] (valid range 1.."+AnsiString(MAX_COLS)+")");
         fclose(fin);
         StatusText->Caption="Invalid xcol";
         free(s);
         addtraceactive=false;// finished
         return;
        }
     xcol=atoi(ns);// convert number (with $ removed)
    }
  free(s);
  if(xcol<1 || (unsigned int)xcol> MAX_COLS)
        {
         // rprintf("Error: invalid xcol (range 1..%d)\n",MAX_COLS);
         ShowMessage("Error: invalid xcol (range 1.."+AnsiString(MAX_COLS)+")");
         fclose(fin);
         StatusText->Caption="Invalid xcol";
         addtraceactive=false;// finished
         return;
		}
  // get x offset from gui to a local variable as a double.
  x_offset=atof(AnsiOf(Edit_Xoffset->Text.c_str()));

  // ycol can either by an unsigned integer number or an expression  OR a comma seperated list of numbers
  char *ys;
  char *start_s;
  char *se=NULL;// expression (if there is one)
  bool isnumber;
  char *date_time_fmt=NULL;
  date_time_fmt=strdup(AnsiOf(Date_time_fmt->Text.c_str()));  // only do this once
  if(Xcol_type->ItemIndex==6) rprintf("Date/time format is: %s\n",date_time_fmt);
  s=strdup(AnsiOf(Edit_ycol->Text.c_str()));
  if(s==NULL||date_time_fmt==NULL)
        {
         ShowMessage("Error (No RAM): invalid ycol ["+Edit_ycol->Text+"] (valid range 1.."+AnsiString(MAX_COLS)+")");
         fclose(fin);
         StatusText->Caption="Invalid ycol";
		 addtraceactive=false;// finished
		 return;
        }
  ys=s; // save copy so we can free at the end
#if 1
  // if x axis label is the default label then set it based on the data just about to be read in
  if( pScientificGraph->XLabel==default_x_label)
		{ // update label from data read
		 switch(Xcol_type->ItemIndex)
				{case 0:
						if(is_fft)
							{pScientificGraph->XLabel="Frequency"; // x=linenumber in file
							 Edit_x->Text="Frequency";
							}
						else
							{pScientificGraph->XLabel="Line number"; // x=linenumber in file
							 Edit_x->Text="Line number";
							}
                        break;
				 case 1: // time h:m:s.s  - converted into time (secs)
				 case 6: // date & time - also converted into seconds
						if(is_fft)
							{
							 pScientificGraph->XLabel="Frequency (Hz)";
							 Edit_x->Text="Frequency (Hz)";
							}
						else
							{
							 pScientificGraph->XLabel="Time (secs)";
							 Edit_x->Text="Time (secs)";
							}
						break;
				 case 3: // val/60 assume this gives minutes
						if(is_fft)
							{  // frequency, but not Hz!
							 pScientificGraph->XLabel="Frequency";
							 Edit_x->Text="Frequency";
							}
						else
							{
							 pScientificGraph->XLabel="Time (mins)";
							 Edit_x->Text="Time (mins)";
							}
						break;
				 case 4: // val/3600 assume this gives hours
						if(is_fft)
							{  // frequency, but not Hz!
							 pScientificGraph->XLabel="Frequency";
							 Edit_x->Text="Frequency";
							}
						else
							{
							 pScientificGraph->XLabel="Time (hours)";
							 Edit_x->Text="Time (hours)";
							}
						break;
				 case 5: // val/86400 assume this gives days
						if(is_fft)
							{  // frequency, but not Hz!
							 pScientificGraph->XLabel="Frequency";
							 Edit_x->Text="Frequency";
							}
						else
							{
							 pScientificGraph->XLabel="Time (days)";
							 Edit_x->Text="Time (days)";
							}
						break;
				 default: // should only be "2" - value in specified column
						if(is_fft)
							{pScientificGraph->XLabel="Frequency"; // x=linenumber in file
							 Edit_x->Text="Frequency";
							}
						else
							{
							 char *xhrd=strdup(hdr_col_ptrs[xcol-1]);
							 char *orig_xhdr= xhrd; // save copy of original as need to free it
							 if(xhrd!=NULL)
								{
								 if(xhrd[0]=='"')
									{// need to remove double quotes around field
									 xhrd++;  // skip leading "
									 if(xhrd[0] && xhrd[strlen(xhrd)-1]=='"')  xhrd[strlen(xhrd)-1]=0; // remove final "  if its present
									}
								 pScientificGraph->XLabel=xhrd;
								 Edit_x->Text=xhrd;
								 free(orig_xhdr);
								}
							 else
								{// copy failed - use orig label
								 pScientificGraph->XLabel=hdr_col_ptrs[xcol-1];
								 Edit_x->Text=hdr_col_ptrs[xcol-1];
								}
							}
						break;
				}
        }
#endif

repeatcomma: // sorry for this !!!, come back here to add next trace if we find a comma in list of traces to add.
  ++nos_traces_added; // number of traces added
  // xmonotonic, true means trace is monotonic in x. This is set true at the start as the same x values are used for every trace added by this function, so for 2nd trace onwards is known to be valid
  firstxvalue=true; // true on 1st x value
  skipy=false; // have not skipped any values yet
  gotyvalue=false; // have not got a valid y value yet

  while(isspace(*ys)) ++ys; // skip optional leading whitespace
  start_s=ys; // start of this item [skipped initial whitespace]
  if(*ys=='$') ++ys; // allow for an optional $
  ns=ys; // start of actual number
  isnumber=false;
  yexpr=false; // assume this item is not an expression
  while(isdigit(*ys))
        {isnumber=true; // we have a valid number
         ++ys; // number  - no sign
        }
  while(isspace(*ys)) ++ys; // optional trailing whitespace
  if(isnumber && (*ys==',' || *ys==0) )
        {// Its a simple number (ie specifying a column directly).  multiple items are comma seperated .
         ycol=atoi(ns);   // number (leading $ removed), conversion will be stopped on comma   or end of string
        }
  else // not a simple number - see if its an expression
	{if(*start_s!=0)
		{// not a number or $nnn so could be an expression  , if so need to set yexpr=true
		 // the expression is terminated by a comma (,), but a command can be used in the expression eg as in max(1,2) so need to keep track of brackets as well as commas...
		  int nos_bracket=0;
		  se=strdup(start_s); // take a copy of the potential expression
		  char *sp=se;
		  while(!(nos_bracket==0 && *sp==',' ) && *sp)
			{if(*sp=='(') nos_bracket++;
			 else if(*sp==')' && nos_bracket>0) nos_bracket--;
			 ++sp; // look for a comma outside of a bracket
			}
		  if(*sp==',')
			 {*sp=0; // end of expression
			 }
		  ys=start_s+(sp-se); // also need to move ys forward to next item
		  rprintf("Found an expression for y:%s (remaining string is \"%s\")\n",se,ys);
		  to_rpn(se); // compile expression to rpn
		  if(last_expression_ok())
				{ yexpr=true; // flag expression as valid
				  ycol=1; // so existing code does not raise an error
				  optimise_rpn(); // optimise rpn just generated
#if 0
				  print_rpn(); // print out rpn for debugging
				  rprintf("\n");
#endif
				  // use se later in title for trace so cannot free yet
				}
		  else
				{
				 // not a number , or a valid expression
				 ShowMessage("Error: ycol ["+Edit_ycol->Text+"] contains an invalid expression["+se+"]");
                 fclose(fin);
                 StatusText->Caption="Invalid expression for ycol";
                 free(s);
				 free(se);
				 if(date_time_fmt!=NULL) {free(date_time_fmt); date_time_fmt=NULL;}
                 addtraceactive=false;// finished
                 return;
                }
        }
     else
        { // actually nothing left ...
         ycol=atoi(ns);   // number (leading $ removed)
        }
    }
  if(*ys!=',') {free(s);s=NULL;}
  if(ycol<1 || (unsigned int)ycol> MAX_COLS)
        {
         // rprintf("Error: invalid ycol (range 1..%d)\n",MAX_COLS);
		 ShowMessage("Error: invalid ycol (range 1.."+AnsiString(MAX_COLS)+")");
		 fclose(fin);
		 StatusText->Caption="Invalid ycol";
		 if(date_time_fmt!=NULL) {free(date_time_fmt); date_time_fmt=NULL;}
		 addtraceactive=false;// finished
		 return;
        }
  // rprintf("xcol=%d ycol=%d\n",xcol,ycol);
  line_colour=(line_colour+1);    // next colour

  switch (line_colour & 0x07)      //line color    (only 8 different colours that are easy to see
  {
#ifdef WHITE_BACKGROUND
        /* white background */
#if 1
        // Use Blue,Green and grey 1st  as per Millbrook request.   Then use colours that are easy to distingish
    case 0: {Col=(TColor)0xFFFF00; break;}      // cyan (green+blue)
	case 1: if(line_colour==1) Col=(TColor)0xA65A08;      // "Millbrook" Blue  note order is BGR not RGB
                else  Col=(TColor)0xFF0000;     // pure blue
             break;
    case 2: if(line_colour==2) Col=(TColor)0x29BD6E;     // "Millbrook" Green
                 else  Col=(TColor)0x00FF00;     // pure green
             break;
    case 3: if(line_colour==3) Col=(TColor)0x595A5A;      // "Millbrook" Grey
                 else  Col=(TColor)0x000000;     // pure black
             break;
    case 4: {Col=(TColor)0xFF00FF; break;}      // purple       note order is BGR
    case 5: {Col=(TColor)0x0000FF; break;}      // red   note order is BGR
    case 6: {Col=(TColor)0x00E0FF; break;}      // yellow (not pure yellow as that does not show up well on a white background) note order is BGR
    case 7: {Col=(TColor)0x007FFF; break;}      // orange    note order is BGR
    default : {Col=clYellow ; break;}   // bright yellow should not be selected - does not show up well on a white background
#else
        // Use Blue and Green 1st  as per Millbrook request.
    case 0: {Col=clLtGray; break;}
    case 1: {Col=(TColor)0xA65A08; break;}   // "Millbrook" Blue  note order is BGR not RGB
    case 2: {Col=(TColor)0x29BD6E; break;}   // "Millbrook" Green
    case 3: {Col=(TColor)0x595A5A; break;}   // "Millbrook" Grey
    case 4: {Col=clMaroon; break;}
    case 5: {Col=clRed; break;}      // bright red
    case 6: {Col=clLime; break;}     // bright green
    case 7: {Col=clAqua; break;}     // light blue
    default : {Col=clYellow ; break;}   // bright yellow should not be selected - does not show up well on a white background
#endif
#else
        /* black background */
    case 0: {Col=clPurple ; break;}
    case 1: {Col=clWhite;  break;}   // 1st colour used
    case 2: {Col=clYellow; break;}   // bright yellow
    case 3: {Col=clRed; break;}      // bright red
    case 4: {Col=clAqua; break;}     // light blue
    case 5: {Col=clLime; break;}     // bright green
    case 6: {Col=clSilver; break;}    // clGreen= dark green
    case 7: {Col=clMaroon; break;}
    default : {Col=clBlue; break;}   // should not be selected - does not show up well on a black background
#endif
  }
  if(user_set_trace_colour)
	{// colour is set by user for this trace
	 user_set_trace_colour=false;
	 Col=user_trace_colour;
	}


  iGraph=pScientificGraph->fnAddGraph(nos_lines_in_file);  //iGraph==graph index  , 0 for 1st, 1 for 2nd...  -1 => error
  if(iGraph<0)
		{ShowMessage("Error: Not enough RAM");
		 fclose(fin);
		 StatusText->Caption="Not enough RAM";
		 if(date_time_fmt!=NULL) {free(date_time_fmt); date_time_fmt=NULL;}
		 addtraceactive=false;// finished
		 return; // error (not enough memory)
		}
  //rprintf("iGraph=%d\n",iGraph);
  pScientificGraph->fnSetColDataPoint(Col,iGraph);    //set colors
  pScientificGraph->fnSetColErrorBar(Col,iGraph);
  pScientificGraph->fnSetColLine(Col,iGraph);

  pScientificGraph->fnSetSizeDataPoint(7,iGraph);         //sizes
  pScientificGraph->fnSetErrorBarWidth(1,iGraph);
  pScientificGraph->fnSetLineWidth(1,iGraph);  // was 2, set to 1 to allow use of various line styles below.
  switch((line_colour-1)/8)     // we only have 8 distinct colours, then goto line styles to make lines unique - these only work with a line width of 1
  {case 0:  pScientificGraph->fnSetLineStyle(psSolid,iGraph); break;             //solid line
   case 1:  pScientificGraph->fnSetLineStyle(psDot,iGraph); break;               //A line made up of a series of dots
   case 2:  pScientificGraph->fnSetLineStyle(psDash,iGraph); break;              //A line made up of a series of dashes.
   case 3:  pScientificGraph->fnSetLineStyle(psDashDot ,iGraph); break;          //A line made up of alternating dashes and dots.
   default: pScientificGraph->fnSetLineStyle(psDashDotDot,iGraph); break;        //A line made up of a series of dash-dot-dot combinations.
  }

  switch (RadioGroup3->ItemIndex)                         //graph style
  {
    case 0:
    {
      pScientificGraph->fnSetStyle(1,iGraph);             //scatter
      break;
    }
    case 1:
    {
      pScientificGraph->fnSetStyle(4,iGraph);             //line
      break;
    }
    case 2:
    {
      pScientificGraph->fnSetStyle(5,iGraph);             //scatter + line
      break;
    }
    case 3:
    {
      pScientificGraph->fnSetStyle(3,iGraph);             //scatter + errorbar
      break;
    }
  }

  if (CheckBox3->Checked) {ucMask=4;}                     //filled symbol for
  else {ucMask=0;}                                        //data points

  switch (RadioGroup2->ItemIndex)                         //data point style
  {
    case 0:
    {
      pScientificGraph->fnSetPointStyle((unsigned short)(0|ucMask),iGraph);        //circle
      break;
    }
    case 1:
    {
      pScientificGraph->fnSetPointStyle((unsigned short)(1|ucMask),iGraph);        //square
      break;
    }
    case 2:
    {
      pScientificGraph->fnSetPointStyle((unsigned short)(2|ucMask),iGraph);        //triangle up
      break;
	}
	case 3:
	{
	  pScientificGraph->fnSetPointStyle((unsigned short)(3|ucMask),iGraph);        //triangle down
	  break;
	}
  }

  char cap_str[256]; // caption for trace   (used for error messages later as well as graph caption her)
  // add ledgend for trace - add (XXX filter=%g) if a filter is in use
  AnsiString basename;
  basename=filename.SubString(filename.LastDelimiter("\\:")+1,128)+" : ";// in case CheckBox_legend_add_filename is ticked
  if(yexpr)
		{ rprintf("Adding trace of %s (expression)\n vs %s (col %d)\n",se,hdr_col_ptrs[xcol-1],xcol);
		  if(FString=="")
			  snprintf(cap_str,sizeof(cap_str),"%s",se);
		  else
			{
			 if(is_filter) snprintf(cap_str,sizeof(cap_str),"%s (%s, t/c=%g)",se,FString.c_str(), median_ahead_t);
			 else          snprintf(cap_str,sizeof(cap_str),"%s (%s)",se,FString.c_str());
			}
		  if(CheckBox_legend_add_filename->State==cbChecked)
			{// add basename of filename to legend
			 pScientificGraph->fnSetCaption(basename+cap_str,iGraph); //graph caption
			}
		  else
			pScientificGraph->fnSetCaption(cap_str,iGraph); //graph caption
		  free(se); // can free se now as have printed titles
		}
  else
		{ rprintf("Adding trace of %s (col %d)\n vs %s (col %d)\n",hdr_col_ptrs[ycol-1],ycol,hdr_col_ptrs[xcol-1],xcol);
		  if(FString=="")
			  snprintf(cap_str,sizeof(cap_str),"%s ",hdr_col_ptrs[ycol-1]);
		  else
			{
			 if(is_filter) snprintf(cap_str,sizeof(cap_str),"%s (%s, t/c=%g)",hdr_col_ptrs[ycol-1],FString.c_str(), median_ahead_t);
			 else          snprintf(cap_str,sizeof(cap_str),"%s (%s)",hdr_col_ptrs[ycol-1],FString.c_str());
			}
		  if(CheckBox_legend_add_filename->State==cbChecked)
			{// add basename of filename to legend
			 pScientificGraph->fnSetCaption(basename+cap_str,iGraph); //graph caption
			}
		  else
			pScientificGraph->fnSetCaption(cap_str,iGraph); //graph caption
		}
#if 1
  // automatically add y axis label
  // note there is only 1 y axis, so assume user does something sensible when displaying multiple graphs
  if(Edit_y->Text==default_y_label && iGraph==0)
	{// if y axis title has not been set, and this is the 1st trace added, set it automatically here
	 Edit_y->Text=hdr_col_ptrs[ycol-1];
	}
#endif

  unsigned int max_col=max((unsigned int)xcol,(unsigned int)ycol);

  max_col=min(max_col,MAX_COLS); // only process the required channels on csv read (saves a little time).

  if(yexpr) max_col=MAX_COLS;  // if there is an expression for y we need to read all the columns
#if 0
  // some checks of functions that read times
  char *t_string;
  t_string="123.4";
  rprintf("Checking time functions\n");
  rprintf("gethms(%s) returns %g should be 123.4\n",t_string,gethms(t_string));
  rprintf("gethms_days(%s) returns %g should be 123.4\n",t_string,gethms_days(t_string));
  t_string="12:04.5";
  rprintf("gethms(%s) returns %g should be 724.5\n",t_string,gethms(t_string));
  rprintf("gethms_days(%s) returns %g should be 724.5\n",t_string,gethms_days(t_string));
  t_string="12:23.4";
  rprintf("gethms(%s) returns %g should be 743.4\n",t_string,gethms(t_string));
  rprintf("gethms_days(%s) returns %g should be 743.4\n",t_string,gethms_days(t_string));
  t_string="10:02:03.4";
  rprintf("gethms(%s) returns %g should be 36123.4\n",t_string,gethms(t_string));
  rprintf("gethms_days(%s) returns %g should be 36123.4\n",t_string,gethms_days(t_string));
  t_string="23:03:04.5";
  rprintf("gethms(%s) returns %g should be 82984.5\n",t_string,gethms(t_string));
  rprintf("gethms_days(%s) returns %g  should be 82984.5\n",t_string,gethms_days(t_string));
  t_string="23:59:59.9";
  rprintf("gethms(%s) returns %g should be 86399.9\n",t_string,gethms(t_string));
  rprintf("gethms_days(%s) returns %g should be 86399.9\n",t_string,gethms_days(t_string));
  t_string="00:00:00.0";
  rprintf("gethms(%s) returns %g should be 0\n",t_string,gethms(t_string));
  rprintf("gethms_days(%s) returns %g should be 86400\n",t_string,gethms_days(t_string));
  t_string="00:00:00.1";
  rprintf("gethms(%s) returns %g should be 0.1\n",t_string,gethms(t_string));
  rprintf("gethms_days(%s) returns %g should be 86400.1\n",t_string,gethms_days(t_string));
  t_string="00:00:00.2";
  rprintf("gethms(%s) returns %g should be 0.2\n",t_string,gethms(t_string));
  rprintf("gethms_days(%s) returns %g should be 86400.2\n",t_string,gethms_days(t_string));
  t_string="4294967296"; // 2^32
  rprintf("gethms(%s) returns %g should be 4.29497e+09\n",t_string,gethms(t_string));
  t_string="4294967296000"; // 2^32 *1000
  rprintf("gethms(%s) returns %g should be 4.29497e+12\n",t_string,gethms(t_string));
  t_string="4294967296000.123"; // 2^32  *1000 +.123
  rprintf("gethms(%s) returns %g should be 4.29497e+12\n",t_string,gethms(t_string));
  t_string="0.0000000000001"; // 1e-13
  rprintf("gethms(%s) returns %g should be 1e-13\n",t_string,gethms(t_string));
#endif
#if 0
   // check multiple_lin_reg_fn
   test_multiregression(0);
   test_multiregression(1);
   test_multiregression(2);
   test_multiregression(3);
#endif
 // this is the normal working code

  reset_days(); // in case we are reading in times
  bool file_has_dates=false;
  bool got_date;
  lines_in_file=0;
  nos_errs=0;  // count of errors for this "column" as thats how its reported to user
  clock_t begin_t,end_t2s;
  begin_t=clock();
  while(1)
		{
		 csv_line=readline(fin);
		 if(csv_line==NULL)
				{break;// whole file read
				}

		 parsecsv(csv_line,col_ptrs, max_col);   // was MAX_COLS, its slightly faster using max_col
		 lines_in_file++;
		 if((lines_in_file & 0x7fff)==1)     // ==1 means we start with 0% read . Was 0x3ff   , 0x7fff gives one every 40ms in a typical case
				{// let user see something happening , printing to the screen is slow so only do this occasionally
				 end_t2s=clock();
				 if(lines_in_file==1 || end_t2s-begin_t > 2*CLK_TCK)
						{// 1st line or more than 2 secs difference
						 if(lines_in_file!=1)
								begin_t+=2*CLK_TCK; // move forward 2 secs  (unless 1st line in file)
#if 1
						 if(yexpr)
								{snprintf(cstring,sizeof(cstring),"%.0f %% read",100.0*(double)_ftelli64(fin)/(double)filesize);
								}
						  else
								{snprintf(cstring,sizeof(cstring),"%.0f %% read of column %d",100.0*(double)_ftelli64(fin)/(double)filesize,ycol);
								}
#else
						 snprintf(cstring,sizeof(cstring),"%d lines read ",lines_in_file);
#endif
						 StatusText->Caption=cstring;
                        }
                 Application->ProcessMessages(); /* allow windows to update (but not go idle), do this regularly [not just every 2 secs] */
				}

		 // get x value
		 if(allvalidxvals && nos_traces_added>1 && xmonotonic && !compress )  // This optimisation disabled if lines with invalid xvals found (and skipped)
			{xval=(float)((double)(pScientificGraph->fnAddDataPoint_nextx(iGraph))-x_offset); // same value as previous graph loaded  as this is faster than decoding it again , x_offset will be added later but is already in previous trace so must subtract here
			 if(!firstxvalue && xval<previousxvalue)
				{if(++nos_errs<=MAX_ERRS)
					rprintf("Error: adding x value from previous trace and values not monotonic at line %d xval=%g\n",lines_in_file+1,xval);
				 xval=previousxvalue; // try and fix issue
				 allvalidxvals=false; // stop problem repeating by turning off this optimisation
				}
			}
		 else
			{long double ti;   // ti is the time just read in - this needs as much resolution as possible - first_time is also a long double
			 switch(Xcol_type->ItemIndex)
				{case 0: xval=lines_in_file; // x=linenumber in file
						break;
				 case 1: // time h:m:s.s (with optional date of form 05-Jul-19 or 2020-03-31 or similar terminated in whitespace)
						st=col_ptrs[xcol-1];
						while(isspace(*st)) ++st; // skip any leading whitespace
						if(*st=='"')
								{++st;// skip " if present
								 while(isspace(*st)) ++st; // skip any more whitespace
								}
						 {char *dptr; // look ahead to see if there is a date we need to skip
										// date is either 05-Jul-19 or 2020-03-31 or 12/12/20 or similar terminated in whitespace
						  got_date=false;
						  for(dptr=st;*dptr;++dptr)
								{if(*dptr=='-' || *dptr=='/')
										{got_date=true;
										 break;
										}
								 if(*dptr==':')
										{// found a time, not a date
										 break;
										}
								}
						   if(got_date)
								{for(;*dptr;++dptr)
										if(isspace(*dptr)) break;// carry on from where we were, look for whitespace which marks end of date
								 while(isspace(*dptr)) ++dptr;  // skip whitespace so should be at start of time
								 if(isdigit(*dptr)) st=dptr; // if we ended up on a digit then assume this is the start of the time
								 file_has_dates=true;
								}
						  }
						// convert time into seconds , gethms_days() ignores trailing whitespace and "'s
						ti=gethms_days(st);  // note we called reset_days above so we always start correctly at 0 days
						if(ti<0)
							{allvalidxvals=false;
							 if((++nos_errs<=MAX_ERRS || !found_error_type[ERR_TYPE1]) && ti== -1)
								{found_error_type[ERR_TYPE1]=true; // note we have printed an example of this type of error
								 rprintf("Warning: x value on line %d has an invalid date/time (time does not start with a number): %s\n",lines_in_file+1,col_ptrs[xcol-1]);
								}
							 if((nos_errs<=MAX_ERRS || !found_error_type[ERR_TYPE2]) && ti!= -1)
								{found_error_type[ERR_TYPE2]=true;  // note we have printed an example of this type of error
								 rprintf("Warning: x value on line %d has an invalid date/time (time goes backwards!): %s\n",lines_in_file+1,col_ptrs[xcol-1]);
								}
							 continue;   // gethms_days() returns a -ve value when it finds an error, so ignore this line when that happens
							}
						 if(!got_date && file_has_dates)
							{ // this has to be after checks on time as we want a header line to be picked up as ERR_TYPE1 not type7
							 if((++nos_errs<=MAX_ERRS || !found_error_type[ERR_TYPE7]) )
								{found_error_type[ERR_TYPE7]=true; // note we have printed an example of this type of error
								 rprintf("Warning: x value on line %d has no date but previous lines do have dates: %s\n",lines_in_file+1,col_ptrs[xcol-1]);
								}
							}
						if(firstxvalue) first_time=ti;  // remember 1st value, and potentially use as ofset for the rest of the values
						if(start_time_from_0)
							{
							 /* line below assumes times are in increasing order which is NOT guaranteed ! */
							 /* However, first_time is a double, as is ti so this potentially offers higher resolution as the difference is stored as a float */
							 xval=(float)(ti-first_time);  // ofset time by time of 1st value (so we maximise resolution in the float xval)
							}
						  else
							{
							 xval=(float)ti; // can loose resolution for big times, but we are limited by storing xvalues as floats.
							}
						break;
				 case 6: // date/time with user specified format  - stored in char * date_time_fmt
						{ // there is no need to skip initial whitespace or deal with quotes here as can be defined in date_time_fmt
						 //char * ya_strptime(const char *s, const char *format, struct tm *tm)
						 char *dptr;
						 struct tm my_tm;
						 memset(&my_tm, 0, sizeof(struct tm));// zero all members of tm
						 strp_tz.initialised=0; // mark as not initialised, so will be zeroed on call to ya_strptime();
						 st=col_ptrs[xcol-1];
						 dptr=ya_strptime(st,date_time_fmt,&my_tm);
						 if(dptr==NULL)
							{// something was wrong with date/time or format
							 allvalidxvals=false;
							 if(++nos_errs<=MAX_ERRS || !found_error_type[ERR_TYPE1] )
								{found_error_type[ERR_TYPE1]=true; // note we have printed an example of this type of error
								 rprintf("Warning: x value on line %d has an invalid date/time (strptime(\"%s\") returned NULL): %s\n",lines_in_file+1,date_time_fmt,col_ptrs[xcol-1]);
								}
							 continue;      // ignore line
							}
						 while(isspace(*dptr)) ++dptr; // if remainder is whitespace then thats OK
						 if(*dptr!=0)
							{ // ya_strptime() did not process all of the string  again issue could be date/time or format
							 allvalidxvals=false;
							 if(++nos_errs<=MAX_ERRS || !found_error_type[ERR_TYPE2] )
								{found_error_type[ERR_TYPE2]=true;  // note we have printed an example of this type of error
								 rprintf("Warning: x value on line %d has an invalid date/time (format \"%s\" did not match whole string: %s [\"%s\" left])\n",lines_in_file+1,date_time_fmt,col_ptrs[xcol-1],dptr);
								}
							 continue;      // ignore line
							}
						 if(!check_tm(&my_tm))
							{ // invalid date/time found
							 allvalidxvals=false;
							 if(++nos_errs<=MAX_ERRS || !found_error_type[ERR_TYPE3] )
								{found_error_type[ERR_TYPE3]=true;  // note we have printed an example of this type of error
								 rprintf("Warning: x value on line %d has an invalid date/time (check_tm() failed): %s\n",lines_in_file+1,col_ptrs[xcol-1]);
								}
							 continue;   // ignore line
							}
						 ti=ya_mktime(&my_tm); /* fully functional version of mktime() that returns secs and takes (and changes if necessary) timeptr */
						 ti+=strp_tz.f_secs;// add in any fractional seconds (ti is a long double to maximise resolution here) */

						if(firstxvalue)
							{first_time=ti;  // remember 1st value, and potentially use as offset for the rest of the values
							}
						if(start_time_from_0)
							{
							 /* line below assumes times are in increasing order which is NOT guaranteed ! */
							 /* However, first_time is a double, as is ti so this potentially offers higher resolution as the difference is stored as a float */
							 xval=(float)(ti-first_time);  // offset time by time of 1st value [both are long doubles] (so we maximise resolution in the float xval)
							}
						  else
							{
							 xval=(float)ti; // can loose resolution for big times, but we are limited by storing xvalues as floats.
							}
#if 0
						if(lines_in_file<5)
							{// useful for debugging
							 rprintf("date/time on line %d returns %g secs (format \"%s\" line: %s, ti=%g)\n",lines_in_file+1,xval,date_time_fmt,col_ptrs[xcol-1],ti);
							}
#endif
						}
						break;
				 default: // could be 2,3,4 or 5 value in specified column, potentially divided by a constant
						st=col_ptrs[xcol-1];
						while(isspace(*st)) ++st; // skip any leading whitespace
						if(*st=='"')
								{++st;// skip " if present
								 while(isspace(*st)) ++st; // skip any more whitespace
								}
						char *end;
						xval= strtod(st,&end);   // get value for this column
						// rprintf("Xval: xcol=%d string=%s =%g\n",xcol,st,xval);
						if(st==end)
							{allvalidxvals=false;
							 if(++nos_errs<=MAX_ERRS || !found_error_type[ERR_TYPE3])
								{found_error_type[ERR_TYPE3]=true;
								 rprintf("Warning: x value on line %d has an invalid number: %s\n",lines_in_file+1,col_ptrs[xcol-1]);
								}
							 continue;    // no valid number found
							}
						// if(xval!=yval) rprintf("**** xval (col %d) %s=>%g\n",xcol,st,xval);
						if(Xcol_type->ItemIndex==3)
							xval/=60.0; // sec->min
						else if(Xcol_type->ItemIndex==4)
							xval/=3600.0; // sec->hrs
						else if(Xcol_type->ItemIndex==5)
							xval/=86400.0; // sec->days
						break;
				}
			}
		 if(yexpr)
				{try
						{
						 yval=(float)execute_rpn(); // expression - excute it
						}
				 catch (...)   // assume the issue is an error in the expression
						{yval=0;
						}
				}
         else
                {st=col_ptrs[ycol-1];
                 while(isspace(*st)) ++st; // skip any leading whitespace
                 if(*st=='"')
                        {++st;// skip " if present
                         while(isspace(*st)) ++st; // skip any more whitespace
                        }
				 // now hope we have a number left - strtod() will terminate at the end of the number so any trailing whitespace or " will be ignored
                 char *end;
				 yval= strtod(st,&end);   // just a column number - get value for this column
				 if(st==end)
					{allvalidxvals=false; // line skipped
					 if(++nos_errs<=MAX_ERRS || !found_error_type[ERR_TYPE4])
							{found_error_type[ERR_TYPE4]=true;
							 rprintf("Warning: y value on line %d has an invalid number: %s\n",lines_in_file+1,col_ptrs[ycol-1]);
							}
					 continue;    // no valid number found
					}
				 // rprintf("yval (col %d) %s=>%g\n",ycol,st,yval);
				}
		 gotyvalue=true;// if we get here we have a valid y value
		 if(!_finite(xval) || !_finite(yval))
			{allvalidxvals=false; // line skipped
			 if(!_finite(xval) && (++nos_errs<=MAX_ERRS || !found_error_type[ERR_TYPE5]))
				{found_error_type[ERR_TYPE5]=true;
				 rprintf("Warning: x value on line %d has an invalid number: %s\n",lines_in_file+1,col_ptrs[xcol-1]);
				}
			 if( !_finite(yval) && (++nos_errs<=MAX_ERRS || !found_error_type[ERR_TYPE6]))
				{found_error_type[ERR_TYPE6]=true;
				 rprintf("Warning: y value on line %d has an invalid number: %s\n",lines_in_file+1,col_ptrs[ycol-1]);
				}
			 continue; // need 2 valid numbers [eg ignore "inf" ]
			}
		 if(firstxvalue)
				{firstxvalue=false;
				 previousxvalue=xval;
				 previousyvalue=yval;
				}
		   else
				{if(xval<previousxvalue)
						{ // x value is NOT monotonically increasing
						 xmonotonic=false;
						}
				 else   {// x value is monotonically increasing

#if 1   /* this way of compressing only works if x values are monotonically increasing, we only know this correctly on the 2nd trace added onwards when we can trust xmonotonic */
		/* There is a general solution for compression in function compress_y() but that requires reading all data into an array and then compressing it (and then freeing extra memory) so peak RAM is lower by doing it here if we can */
						 if(nos_traces_added>1 && xmonotonic && compress && yval==previousyvalue)        // don't bother to store lots of identical values if "compress" is enabled
								{skipy=true;
								 previousxvalue=xval;
								 continue;
								}
						 if(skipy) // if have skipped some values put last point in so we see a correct straight line
								{skipy=false;
								 pScientificGraph->fnAddDataPoint((float)(previousxvalue+x_offset),previousyvalue,iGraph); // ignore out of ram as that will be trapped below
								}
#endif
						 previousxvalue=xval;
						 previousyvalue=yval;
						}
				}
		 // rprintf("before addDatapoint: xval=%g x_offset=%g yval=%g iGraph=%d\n",xval,x_offset,yval,iGraph);
		 if(!pScientificGraph->fnAddDataPoint((float)(xval+x_offset),yval,iGraph))
				{// out of RAM
				 ShowMessage("Error: not enough memory to load all specified columns");
				 fclose(fin);
				 StatusText->Caption="Error: not enough memory to load all specified columns";
				 if(s) free(s);
                 if(date_time_fmt!=NULL) {free(date_time_fmt); date_time_fmt=NULL;}
				 addtraceactive=false;// finished
				 pScientificGraph->fnDeleteGraph(iGraph); // delete partial column
				 if(iGraph==0 || !zoomed)
						{ // if 1st graph or not already zoomed then autoscale, otherwise leave this to the user.
						 StatusText->Caption="Autoscaling";
						 Application->ProcessMessages(); /* allow windows to update (but not go idle) */
						 pScientificGraph->fnAutoScale();
						}
				 StatusText->Caption="Drawing graph";
				 Application->ProcessMessages(); /* allow windows to update (but not go idle) */
				 fnReDraw();
				 StatusText->Caption="Error: not enough memory to load all specified columns";
				 return;
				}
		}
  if(nos_errs==0)
	rprintf("%d lines read from csv file (no errors found)\n",lines_in_file,nos_errs);
  else if(nos_errs==1)
	rprintf("%d lines read from csv file (1 line skipped dues to errors)\n",lines_in_file,nos_errs);
  else
	rprintf("%d lines read from csv file (%d lines skipped dues to errors - at least one example of each error type is shown above)\n",lines_in_file,nos_errs);
  if(*ys!=',') fclose(fin);
  if(!gotyvalue)
        {// no valid y values found
		 strncat(cap_str,": Warning: no valid numbers found for y values of column",sizeof(cap_str)-1-strlen(cap_str));
         ShowMessage(cap_str);
        }
  else if(firstxvalue)
        {// no valid x values found
		 strncat(cap_str,": Warning: no valid numbers found for x values of column",sizeof(cap_str)-1-strlen(cap_str));
		 ShowMessage(cap_str);
		}
  else if(nos_errs>0 && showerrmessage)
		{// errors found in file just read in , tell user (just once)
		 char temp_str[100];
		 showerrmessage=false;
		 snprintf(temp_str,sizeof(temp_str),": Warning: %d errors found while reading file",nos_errs);
		 strncat(cap_str,temp_str,sizeof(cap_str)-1-strlen(cap_str));
		 ShowMessage(cap_str);
		}
  if(!xmonotonic)
		{StatusText->Caption="X Values need sorting";
		 Application->ProcessMessages(); /* allow windows to update (but not go idle) */

		 if(showsortmessage)
				{ShowMessage("Warning: X values just read in are not always increasing - about to sort values.");
				 showsortmessage=false; // only show it once for a given press of "add Traces" button as x axis data is the same for all added traces
				}
		 StatusText->Caption="Sorting...";
		 Application->ProcessMessages(); /* allow windows to update (but not go idle) */
		 pScientificGraph->sortx(iGraph); // sort on x values
		 StatusText->Caption="X values sorted";
		 Application->ProcessMessages(); /* allow windows to update (but not go idle) */
		}
#if 1    /* general purpose compression - works well but does require all data for the trace is read into ram , then excess is returned so peak ram is high */
   if(compress && !(nos_traces_added>1 && xmonotonic) ) // && ! bit traps cases that can be done when reading the trace data in (above)
		{StatusText->Caption="Compressing...";
         Application->ProcessMessages(); /* allow windows to update (but not go idle) */
         pScientificGraph->compress_y(iGraph); // compress by deleting points with equal y values except for 1st and last in a row
         StatusText->Caption="Compression completed";
         Application->ProcessMessages(); /* allow windows to update (but not go idle) */
        }
#endif
  // now implement filter on data just read in if user requires this
  switch(FilterType->ItemIndex)
        {case 0: // no filtering , do nothing
                break;
         case 1:
                // median filter defined in terms of time (this is a bit slower than doing based on samples, but makes more sense when the sampling rate varies

                if(median_ahead_t>0.0)
                        {
						 StatusText->Caption=FString;
                         pScientificGraph->fnMedian_filt_time(median_ahead_t,iGraph,filter_callback);
                        }
                break;
         case 2:
                // median1 filter defined in terms of time (this is a bit slower than doing based on samples, but makes more sense when the sampling rate varies

                if(median_ahead_t>0.0)
						{
						 StatusText->Caption=FString;
						 pScientificGraph->fnMedian_filt_time1(median_ahead_t,iGraph,filter_callback);
                        }
                break;
        case 3:
				// linear filter  with user specified time constant and order. No filtering is done if order=0
				if(median_ahead_t>0.0 && poly_order>0)
						{
						 StatusText->Caption=FString;
						 for(int i=0;i<poly_order;++i)
							pScientificGraph->fnLinear_filt_time(median_ahead_t,iGraph,filter_callback);
						 rprintf("Linear filter order %d and time constant %g seconds has -3dB frequency of %g Hz\n",
						 	poly_order,median_ahead_t,sqrt(pow(2.0,1.0/(double)poly_order)-1.0)/(2.0*3.14159265358979*median_ahead_t));
                        }
				break;
		case 4:

				// lin regression y=mx
				StatusText->Caption=FString;
				pScientificGraph->fnLinreg_origin(iGraph,filter_callback);
				break;
		case 5:
#if 0
				{
				 // test code uses general linear least squares fitting , false arguments means y values are not changed
				 gen_lin_reg(reg_poly,1,false,iGraph); // y=a
				 gen_lin_reg(reg_poly,2,false,iGraph); // y=a+b*x
				 gen_lin_reg(reg_poly,3,false,iGraph); // y=a+b*x+c*x^2
				 gen_lin_reg(reg_poly,4,false,iGraph); // y=a+b*x+c*x^2+d*x^3
				 gen_lin_reg(reg_poly,5,false,iGraph); // y=a+b*x+c*x^2+d*x^3+e*x^4
				 gen_lin_reg(reg_poly,6,false,iGraph); // y=a+b*x+c*x^2+d*x^3+e*x^4+f*x^5
				 gen_lin_reg(reg_poly,7,false,iGraph); // y=a+b*x+c*x^2+d*x^3+e*x^4+f*x^5+g*x^6

				 gen_lin_reg(reg_sqrt,1,false,iGraph); // y=a
				 gen_lin_reg(reg_sqrt,2,false,iGraph); // y=a+b*sqrt(x)
				 gen_lin_reg(reg_sqrt,3,false,iGraph); // y=a+b*sqrt(x)+c*x
				 gen_lin_reg(reg_sqrt,4,false,iGraph); // y=a+b*sqrt(x)+c*x+d*x^1.5
				 gen_lin_reg(reg_sqrt,5,false,iGraph); // y=a+b*sqrt(x)+c*x+d*x^1.5+e*x^2
				 gen_lin_reg(reg_sqrt,6,false,iGraph); // y=a+b*sqrt(x)+c*x+d*x^1.5+e*x^2+f*x^2.5
				 gen_lin_reg(reg_sqrt,7,false,iGraph); // y=a+b*sqrt(x)+c*x+d*x^1.5+e*x^2+f*x^2.5+g*x^3

				 gen_lin_reg(reg_rat,1,false,iGraph);  // N=1=> y=(a0)/(1)
				 gen_lin_reg(reg_rat,2,false,iGraph);  // N=2=> y=(a0)/(1+b1*x)
				 gen_lin_reg(reg_rat,3,false,iGraph);  // N=3=> y=(a0+a1*x)/(1+b1*x)
				 gen_lin_reg(reg_rat,4,false,iGraph);  // N=4=> y=(a0+a1*x)/(1+b1*x+b2*x^2)
				 gen_lin_reg(reg_rat,5,false,iGraph);  // N=5=> y=(a0+a1*x+a2*x^2)/(1+b1*x+b2*x^2)
				 gen_lin_reg(reg_rat,6,false,iGraph);  // N=6=> y=(a0+a1*x+a2*x^2)/(1+b1*x+b2*x^2+b3*x^3)
				 gen_lin_reg(reg_rat,7,false,iGraph);  // N=7=> y=(a0+a1*x+a2*x^2+a2*x^3)/(1+b1*x+b2*x^2+b3*x^3)
				}
#endif
				// lin regression y=mx+c
				StatusText->Caption=FString;
				pScientificGraph->fnLinreg(LinLin,iGraph,filter_callback);
				break;
		case 6:
				// lin regression y=mx+c  via GMR
				StatusText->Caption=FString;
				pScientificGraph->fnLinreg(LinLin_GMR,iGraph,filter_callback);
				break;
		case 7:
				// minimal max abs error: y=mx+c
				StatusText->Caption=FString;
				pScientificGraph->fnLinreg_abs(false,iGraph,filter_callback);
				break;
		case 8:
				// minimal max abs relative error: y=mx+c
				StatusText->Caption=FString;
				pScientificGraph->fnLinreg_abs(true,iGraph,filter_callback);
				break;
		case 9:
				// log regression
				StatusText->Caption=FString;
				pScientificGraph->fnLinreg(LogLin,iGraph,filter_callback);
				break;
		case 10:
				// exponentail regression
				StatusText->Caption=FString;
				pScientificGraph->fnLinreg(LinLog,iGraph,filter_callback);
				break;
		case 11:
				// powerregression
				StatusText->Caption=FString;
				pScientificGraph->fnLinreg(LogLog,iGraph,filter_callback);
				break;
		case 12:
				// recip regression
				StatusText->Caption=FString;
				pScientificGraph->fnLinreg(RecipLin,iGraph,filter_callback);
				break;
		case 13:
				// lin-recip regression
				StatusText->Caption=FString;
				pScientificGraph->fnLinreg(LinRecip,iGraph,filter_callback);
				break;
		case 14:
				// hyperbolic regression
				StatusText->Caption=FString;
				pScientificGraph->fnLinreg(RecipRecip,iGraph,filter_callback);
				break;
		case 15:
				// sqrt regression y=m*sqrt(x)+c
				StatusText->Caption=FString;
				pScientificGraph->fnLinreg(SqrtLin,iGraph,filter_callback);
				break;
		case 16:
				// nlog2(n) regression y=m*x*log2(x)+c
				StatusText->Caption=FString;
				pScientificGraph->fnLinreg(Nlog2nLin,iGraph,filter_callback);
				break;
		case 17:
				// y=a*x+b*sqrt(x)+c  (least squares fit)
				StatusText->Caption=FString;
				pScientificGraph->fnLinreg_3(iGraph,filter_callback);
				break;

		case 18:
				// y=a+b*sqrt(x)+c*x+d*x^1.5
				StatusText->Caption=FString;
				gen_lin_reg(reg_sqrt,4,true,iGraph);
				break;
		case 19:
				// y=(a+bx)/(1+cx)  (least squares fit)
				StatusText->Caption=FString;
				pScientificGraph->fnrat_3(iGraph,filter_callback);
				break;
		case 20:
				// N=5=> y=(a0+a1*x+a2*x^2)/(1+b1*x+b2*x^2)
				StatusText->Caption=FString;
				gen_lin_reg(reg_rat,5,true,iGraph);
				break;
		case 21: //  general purpose polynomial fit   (least squares using orthogonal polynomials)
				StatusText->Caption=FString;
				if(!pScientificGraph->fnPolyreg((unsigned int)poly_order,iGraph,filter_callback))
						{StatusText->Caption="Polynomial fit failed";
						 ShowMessage("Warning: Polynomial fit failed - adding original trace to graph");
						}
				break;
		case 22:
				// general purpose polynomial fit in sqrt(x) with user defined order
				StatusText->Caption=FString;
				gen_lin_reg(reg_sqrt,poly_order+1,true,iGraph);
				break;
		case 23:
				// rational fit (poly1/poly2)  with user defined order
				StatusText->Caption=FString;
				gen_lin_reg(reg_rat,poly_order+1,true,iGraph);
				break;
		case 24:
				// derivative
				StatusText->Caption=FString;
				deriv_trace(iGraph);
				break;
		case 25:
				// integral
				StatusText->Caption=FString;
				integral_trace(iGraph);
				break;
		case 26: // bool TScientificGraph::fnFFT(bool dBV_result,bool hanning,int iGraphNumberF, void (*callback)(unsigned int cnt,unsigned int maxcnt))
				// fft return ||
				StatusText->Caption=FString;
				if(!pScientificGraph->fnFFT(false,false,iGraph,filter_callback))
						{StatusText->Caption="FFT failed";
						 ShowMessage("Warning: FFT failed - adding original trace to graph");
						}
				break;
		case 27: // bool TScientificGraph::fnFFT(bool dBV_result,bool hanning,int iGraphNumberF, void (*callback)(unsigned int cnt,unsigned int maxcnt))
				// fft return dBV
				StatusText->Caption=FString;
				if(!pScientificGraph->fnFFT(true,false,iGraph,filter_callback))
						{StatusText->Caption="FFT failed";
						 ShowMessage("Warning: FFT failed - adding original trace to graph");
						}
				break;
		case 28: // bool TScientificGraph::fnFFT(bool dBV_result,bool hanning,int iGraphNumberF, void (*callback)(unsigned int cnt,unsigned int maxcnt))
				// fft with Hanning window, return ||
				StatusText->Caption=FString;
				if(!pScientificGraph->fnFFT(false,true,iGraph,filter_callback))
						{StatusText->Caption="FFT failed";
						 ShowMessage("Warning: FFT failed - adding original trace to graph");
						}
				break;
		case 29: // bool TScientificGraph::fnFFT(bool dBV_result,bool hanning,int iGraphNumberF, void (*callback)(unsigned int cnt,unsigned int maxcnt))
				// fft with Hanning window return dBV
				StatusText->Caption=FString;
				if(!pScientificGraph->fnFFT(true,true,iGraph,filter_callback))
						{StatusText->Caption="FFT failed";
						 ShowMessage("Warning: FFT failed - adding original trace to graph");
						}
				break;
		}
  if(*ys!=',')
	{// if this is the final trace then rescale & actually plot, otherwise skip this step to save time
	 if(iGraph==0 || !zoomed)
		{ // if 1st graph or not already zoomed then autoscale, otherwise leave this to the user.
		 StatusText->Caption="Autoscaling";
		 Application->ProcessMessages(); /* allow windows to update (but not go idle) */
		 pScientificGraph->fnAutoScale();
		}
	 StatusText->Caption="Drawing graph";
	 Application->ProcessMessages(); /* allow windows to update (but not go idle) */
	 fnReDraw();
	}
  end_t=clock();

  if(*ys==',')
		{// multiple items are comma seperated
		 ++ys; // skip ,
		 rewind(fin); // back to start of file
		 char *text_first_line=NULL;
		 {int skip_initial_lines=_wtoi(Form1->pPlotWindow->Edit_skip_lines->Text.c_str());
		  for(int l=0;l<=skip_initial_lines;++l)    // need to read 1 line if skip=0, 2 lines for skip=1, etc
				text_first_line=readline(fin);
		 }
		 if(text_first_line==NULL)  // check header line to check file can be read again
                {
				 ShowMessage("Error: cannot rewind input file to read next column");
				 fclose(fin);
                 StatusText->Caption="Error reading multiple columns";
				 if(s) free(s);
				 if(date_time_fmt!=NULL) {free(date_time_fmt); date_time_fmt=NULL;}
                 addtraceactive=false;// finished
                 return;
                }
		 StatusText->Caption="Reading next column...";
         rprintf("\n");
         Application->ProcessMessages(); /* allow windows to update (but not go idle) */
         goto repeatcomma; // go and process next 
		}
   // free memory potentially used
   if(s) free(s);
   if(date_time_fmt!=NULL) {free(date_time_fmt); date_time_fmt=NULL;}
// #define DEBUG_RAM_USED /* when defined use rprintf to give "user" more info */
#if 0 /* for 64 bit compiles this always gives 2.5MB  */
   TMemoryMap  mmap;
   GetMemoryMap(mmap);
   unsigned int ram_used=0;    // in 64kbytes chunks
   double dram_used;
   for(int i=0;i<65536;++i)
	ram_used+=(mmap[i]==csAllocated)||(mmap[i]==csReserved);       // in use by this process or reserved for it
#ifdef DEBUG_RAM_USED
   rprintf("%d 64k chunks in use = %g MB\n",ram_used,((double)ram_used*64.0)/1024.0);
#endif
   dram_used=ram_used*64.0/1024.0;  // convert to MB
   if(nos_traces_added>1)
		{
		 snprintf(cstring,sizeof(cstring),"Added %d traces in %.0f secs. %.1f MB ram used",nos_traces_added,(end_t-start_t)/CLK_TCK,dram_used);

		}
  else if(yexpr)
		{
		 snprintf(cstring,sizeof(cstring),"Added trace in %.1f secs. %.1f MB ram used",(end_t-start_t)/CLK_TCK,dram_used);
		}
	else
		{
		 snprintf(cstring,sizeof(cstring),"Added column %d in %1f secs. %.1f MB ram used",ycol,(end_t-start_t)/CLK_TCK,dram_used);
		}
#elif 1 /* this returns 125MB when task manager gives 136.8MB - but as it only returns RAM used this is sensible */
  TMemoryManagerState memstatus;
  // TSmallBlockTypeState bs;
  double ram_used=0;
  double dram_used;
  GetMemoryManagerState(memstatus);
  //ram_used=(memstatus.TotalAllocatedMediumBlockSize)/(1024*1024)+memstatus.TotalAllocatedLargeBlockSize/(1024*1024) ; // in MB
  dram_used=(double)memstatus.TotalAllocatedMediumBlockSize+(double)memstatus.TotalAllocatedLargeBlockSize ;
  for(int i=0;i< 46;++i)    // was 55 now 46 as higher values seem to give invalid results in win64
	{ ram_used+=memstatus.SmallBlockTypeStates[i].ReservedAddressSpace;
#ifdef DEBUG_RAM_USED
	  rprintf("SmallBlockTypeStates[%u]: InternalBlockSize=%.0f UseableBlockSize=%0.f AllocatedBlockCount=%.0f ReservedAddressSpace=%.0f\n",i,
		 (double)memstatus.SmallBlockTypeStates[i].InternalBlockSize, (double)memstatus.SmallBlockTypeStates[i].UseableBlockSize, (double)memstatus.SmallBlockTypeStates[i].AllocatedBlockCount ,(double)memstatus.SmallBlockTypeStates[i].ReservedAddressSpace);
#endif
	}
#ifdef DEBUG_RAM_USED
  rprintf("memstatus.TotalAllocatedMediumBlockSize=%.0f  memstatus.TotalAllocatedLargeBlockSize=%.0f dram_used=%.0f ram_used=%.0f\n",
	(double)memstatus.TotalAllocatedMediumBlockSize, (double)memstatus.TotalAllocatedLargeBlockSize, dram_used,ram_used);
  rprintf(" AllocatedMediumBlockCount=%.0f AllocatedLargeBlockCount=%.0f\n",(double)memstatus.AllocatedMediumBlockCount, (double)memstatus.AllocatedLargeBlockCount);
#endif
  dram_used=(ram_used+dram_used)/(1024.0*1024.0); // convert to MB
  if(nos_traces_added>1)
		{
		 snprintf(cstring,sizeof(cstring),"Added %d traces in %.0f secs. %.0fMB ram used",nos_traces_added,(end_t-start_t)/CLK_TCK,dram_used);

		}
  else if(yexpr)
		{
		 snprintf(cstring,sizeof(cstring),"Added trace in %.1f secs. %.0fMB ram used",(end_t-start_t)/CLK_TCK,dram_used);
		}
	else
		{
		 snprintf(cstring,sizeof(cstring),"Added column %d in %1f secs. %.0fMB ram used",ycol,(end_t-start_t)/CLK_TCK,dram_used);
		}

#else     /* THeapStatus is deprecated - this returns 357MB when task manager gives 358.3 MB */
  THeapStatus heapstatus=GetHeapStatus();
  if(nos_traces_added>1)
		{
		 snprintf(cstring,sizeof(cstring),"Added %d traces in %.0f secs. %uMB ram used",nos_traces_added,(end_t-start_t)/CLK_TCK,heapstatus.TotalAddrSpace/(1024*1024));

		}
  else if(yexpr)
		{
		 snprintf(cstring,sizeof(cstring),"Added trace in %.1f secs. %uMB ram used",(end_t-start_t)/CLK_TCK,heapstatus.TotalAddrSpace/(1024*1024));
		}
	else
		{
		 snprintf(cstring,sizeof(cstring),"Added column %d in %1f secs. %uMB ram used",ycol,(end_t-start_t)/CLK_TCK,heapstatus.TotalAddrSpace/(1024*1024));
		}
#endif
  rprintf("%s\n\n",cstring);
  StatusText->Caption=cstring;
  Application->ProcessMessages(); /* allow windows to update (but not go idle) */
  addtraceactive=false;// finished
 } // end of try
catch (Exception &exception)
	{
		xchange_running= -1; // avoid running multiple instances of Edit_XoffsetChange() in parallel, but still do correct number of updates, -1 is initial value
		addtraceactive=false; // set to true when add trace active to avoid multiple clicks
		rprintf("Exception3:\n");
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
			rprintf("Exception4:\n");
#ifndef TRY_CATCH_DISABLED
			Application->ShowException(&exception);
#endif
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TPlotWindow::ReDrawExecute(TObject *Sender)
{ P_UNUSED(Sender);
  fnReDraw();
}
//---------------------------------------------------------------------------
static void set_ListboxXY(void) // highlight default columns in ListBoxX/Y (if valid columns are specified)
{
  int i;
  int nos_cols_in_file=Form1->pPlotWindow->ListBoxY->Items->Count ; // number of columns
  i=_wtoi(Form1->pPlotWindow->Edit_xcol->Text.c_str())-1;
  if(i>=0 && i<nos_cols_in_file)
	Form1->pPlotWindow->ListBoxX->ItemIndex=i;// highlight default column (only 1 for x )
  // y is more complex as can be more than 1 column, comma seperated
  // first clear all existing selections
  for(i=0;i<nos_cols_in_file;++i)
	   Form1->pPlotWindow->ListBoxY->Selected[i]=false;

  char *s=AnsiOf(Form1->pPlotWindow->Edit_ycol->Text.c_str());
  // first check that string contains just whitespace, integers and comma's  (as we cannot do highlighting for anything more complex)
  while(*s)
	{while(isspace(*s)) ++s;// skip leading whitespace
	 while(isdigit(*s)) ++s;// skip an integer
	 while(isspace(*s)) ++s;// skip trailing whitespace
	 if(*s==',') ++s;// look to see if there is a comma, if there is skip it and go around loop looking for more
	 else if(*s) return;// else if not end of the string we have found something more complex, do not do highlighting
	}
  // now parse string in Edit_ycol which is a list of comma seperated numbers to extract column numbers
  s=AnsiOf(Form1->pPlotWindow->Edit_ycol->Text.c_str());
  while(*s)
	{if(isspace(*s)) ++s;// skip leading whitespace
	 if(isdigit(*s))
		{i=atoi(s)-1; // get number of column
		 if(i>=0 && i<nos_cols_in_file)
			Form1->pPlotWindow->ListBoxY->Selected[i]=true; // if number in range then show it as selected
		}
	 while(*s && *s!=',') ++s;// look to see if there is a comma
	 if(*s==',') ++s;// if there is a comma, then skip it and look for another number
	}
}
//---------------------------------------------------------------------------
/* use built in windows functions directly to read file, check for \n's by reading array 64bits at a time */
	/* \n detection from idea in http://graphics.stanford.edu/~seander/bithacks.html#ZeroInWord */

static size_t count_lines(char *cfilename, double filesize)
{   HANDLE hFile;
	size_t lines=0;
	char *buf=(char *)malloc(CL_BLOCK_SIZE); // buffer for reads - malloc guarantees sensible alignment for buf;
	char *cp;
	DWORD nBytesRead = 0;
	BOOL bResult   = FALSE;
	double total_bytes_read=0;
	clock_t begin_t;
	begin_t=clock();
	// rprintf("count_lines: Filename=%s filesize=%.0f KB\n",cfilename,filesize/1024.0);
	if(buf==NULL)
		{rprintf(" count_lines() - not enough RAM - sorry!\n");
		 return 0; // No RAM
		}
	/*
	HANDLE CreateFileA(
	LPCSTR                lpFileName,
	DWORD                 dwDesiredAccess,
	DWORD                 dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD                 dwCreationDisposition,
	DWORD                 dwFlagsAndAttributes,
	HANDLE                hTemplateFile
	);
	*/
#ifdef _WIN64
	hFile = CreateFile(UnicodeOf(cfilename),               // file to open
#else
	hFile = CreateFile(cfilename,               // file to open
#endif
					   GENERIC_READ,          // open for reading
					   FILE_SHARE_READ,       // share for reading
					   NULL,                  // default security
					   OPEN_EXISTING,         // existing file only
					   FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, // normal file  , will be read sequentially
					   NULL);                 // no attr. template

	if (hFile == INVALID_HANDLE_VALUE)
		{free(buf);
		 return 0; // cannot open file
		}
	while ( (bResult = ReadFile(hFile, buf, CL_BLOCK_SIZE, &nBytesRead, NULL)))    /* bResult is false only on error */
		{
		 // Check for eof.
		 if (bResult &&  nBytesRead == 0)
			{
			 // at the end of the file
			 break;
			}
		 total_bytes_read+=nBytesRead; // keep track of bytes read to date
		 uint64_t v64;
		 uint64_t *pv64;
	/* # pragma's below work for gcc and clang compilers. This code is done for efficiency, buf is obtained from heap so has suitable alignment */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
		 for(pv64=(uint64_t *)buf; nBytesRead>=8;nBytesRead-=8)
			{// read buffer 8 bytes at a time looking for \n = 0a*/
#pragma GCC diagnostic pop
			 v64=*pv64++; // read next 8 characters from buffer
			 v64^= UINT64_C(0x0a0a0a0a0a0a0a0a); // convert \n's to zero's as test below is for zero bytes
			 if ((v64 - UINT64_C(0x0101010101010101)) & (~v64) & UINT64_C(0x8080808080808080))
				{// at least 1 byte is a \n, count them, there is 1 bit set in data for every \n character in the 8 bytes
                 v64=(v64 - UINT64_C(0x0101010101010101)) & (~v64) & UINT64_C(0x8080808080808080);
				 ++lines; // at least 1 \n found
				 while((v64=(v64&(v64-1)))) ++lines; // (x&(x-1)) removes a single bit set from data, so this counts the remaining \n's
				}
			}
		 cp=(char *)pv64;// might be some bytes left to process, do them here 1 character at a time
		 while(nBytesRead--)
			if(*cp++=='\n') ++lines;
		 if( clock()-begin_t > 2*CLK_TCK)
			{// more than 2 secs difference , give user something to see
             char str_buf[128];
			 begin_t+=2*CLK_TCK; // move forward 2 secs
			 snprintf(str_buf,sizeof(str_buf),"%.0f %% : %.0f lines so far",100.0*(double)total_bytes_read/filesize,(double)lines);
			 Form1->pPlotWindow->StatusText->Caption=str_buf;
			 Application->ProcessMessages(); /* allow windows to update (but not go idle), do this regularly  */
			}
		}
	CloseHandle(hFile);
	free(buf);
	//rprintf("  %.0f lines found\n",(double)lines);
	return lines;
}

//---------------------------------------------------------------------------
void proces_open_filename(char *fn) // open filename - just to peek at header row  : now also counts the number of lines in the file
{
// Form1->pPlotWindow->StatusText->Caption=cstring;
  FILE *fin;
  char *csv_line=NULL;// line of csv file
  unsigned int nos_cols_in_file;
  unsigned int j;
  AnsiString basename;
  char str_buf[1000];  // temp buffer
  int64_t filesize;    // gives filesize which is then used to show progress as a %
  if(addtraceactive || xchange_running!=-1)
	{// appear to still be busy doing something - ask user if this is true
	 UnicodeString errorText="csvgraph appears to still be busy - continue waiting?";
	 if( MessageDlg(errorText,mtConfirmation,TMsgDlgButtons() <<mbYes <<mbNo ,0) == mrNo)
		{addtraceactive= false;   // if user says "no" then clear flags  (if they say "yes" don't clear flags and we will return below)
		 xchange_running= -1;
		}
	}
  if(addtraceactive) return; // currently adding traces so cannot change filename
  nos_lines_in_file=0; // we need to count the number of lines in the file
  if(fn==NULL || strcmp(fn,"")==0)
		{filename="";
		 Form1->pPlotWindow->StatusText->Caption="No filename set";
		 Form1->pPlotWindow->StaticText_filename->Text="Not set";
         return; // if no filename then done here
		}
  filename=fn;
  basename=filename.SubString(filename.LastDelimiter("\\:")+1,128); // window will add scroll bars automaticaly if name is too long
  fin=fopen(filename.c_str(),"rb");
  if(fin==NULL)
        {ShowMessage("Error: cannot open file "+filename);
         Form1->pPlotWindow->StatusText->Caption="No filename set";
         Form1->pPlotWindow->StaticText_filename->Text="Not set";
		 rprintf("Cannot open filename <%s>\n",filename.c_str());
         filename="";
         return;
		}

  _fseeki64(fin, 0, SEEK_END);  // seek to end of file
  filesize=_ftelli64(fin); // get size of file
  _fseeki64(fin,0,SEEK_SET); // back to start of file

  rcls();
  // rprintf("filesize=%llu =(double) %.0f\n",filesize,(double)filesize);
#if 0
  /* add some debugging info */
#ifdef _WIN32
  rprintf("_WIN32 defined ");
#endif
#ifdef _WIN64
  rprintf("_WIN64 defined ");
#endif
#ifdef  __BORLANDC__
  rprintf("__BORLANDC__ defined ");
#endif
#ifdef _OPENMP
  rprintf("_OPENMP defined ");
#endif
#if __SIZEOF_POINTER__ == 8 /* 64 bit pointers */
  rprintf("__SIZEOF_POINTER__ == 8 Compiled for 64 bit pointers\n");
#elif __SIZEOF_POINTER__ == 4 /* 32 bit pointers */
  rprintf("__SIZEOF_POINTER__ == 4 Compiled for 32 bit pointers\n");
#else
  rprintf("Compiled for unknown pointer size !\n");
#endif
  rprintf("sizeof int=%d,long=%d,float=%d,double=%d,size_t=%d,ssize_t=%d,uintptr_t=%d,intptr_t=%d,void *=%d\n",
	sizeof(int),sizeof(long),sizeof(float),sizeof(double),sizeof(size_t),sizeof(ssize_t),sizeof(uintptr_t),sizeof(intptr_t),sizeof(void *));

#endif
  rprintf("filename selected is %s\n",filename.c_str());
  {int skip_initial_lines=_wtoi(Form1->pPlotWindow->Edit_skip_lines->Text.c_str());
   for(int l=0;l<=skip_initial_lines;++l)    // need to read 1 line if skip=0, 2 lines for skip=1, etc
	 csv_line=readline(fin);
  }
  if(csv_line==NULL)
        {ShowMessage("Error: cannot read headers from file "+filename);
         fclose(fin);
         filename="";
         Form1->pPlotWindow->StatusText->Caption="No filename set";
         Form1->pPlotWindow->StaticText_filename->Text="Not set";
         return;
        }
  if(col_names!=NULL) free(col_names);
  col_names=strdup(csv_line); // copy input line as we want to save column header strings
  if(col_names==NULL)
        {ShowMessage("Error no RAM): cannot read headers from file "+filename);
         fclose(fin);
         filename="";
         Form1->pPlotWindow->StatusText->Caption="No filename set";
         Form1->pPlotWindow->StaticText_filename->Text="Not set";
         return;
        }
  j=csv_count_cols(col_names); // count how many columns are present in header row
  if(j==0 || (j==1 && strlen(col_names)>300) )   // no columns means empty line, 1 column means no commas so if line is long its suspect
        {ShowMessage("Error: file does not appear to be a csv file\n");
         fclose(fin);
         filename="";
         Form1->pPlotWindow->StatusText->Caption="No filename set";
         Form1->pPlotWindow->StaticText_filename->Text="Not set";
         return;
        }


    // read 2nd line check # columns the same as in the 1st line
  csv_line=readline(fin);
  if(csv_line==NULL)
        {ShowMessage("Error: cannot read 2nd line from file "+filename);
         fclose(fin);
         filename="";
         Form1->pPlotWindow->StatusText->Caption="No filename set";
         Form1->pPlotWindow->StaticText_filename->Text="Not set";
         return;
        }
  if(j!=csv_count_cols(csv_line))
        {// warn user
         ShowMessage("Warning: number of columns in 2nd line of csv file is different to number of columns in 1st line!");
         // allow user to carry on - its only a problem if they try and load a column from file
        }
  j+=2; // + 2 so we can detect lines that have too many columns in them
  if(j>MAX_COLS)
        {if(col_ptrs!=NULL) free(col_ptrs) ; // if space previously allocated for too small a number of columns then free it
         col_ptrs=(char **)malloc(( j )*sizeof(char *))  ; // allocate space for required columns
         if(hdr_col_ptrs!=NULL) free(hdr_col_ptrs) ; // if space previously allocated for too small a number of columns then free it
         hdr_col_ptrs=(char **)malloc(( j )*sizeof(char *))  ; // allocate space for required columns
        }
  if(col_ptrs==NULL || hdr_col_ptrs==NULL)
        {ShowMessage("Error - malloc for col-ptrs failed - out of RAM\n");
         fclose(fin);
         Form1->pPlotWindow->StatusText->Caption="No more RAM!";
         return;
        }
  if(j==1)
        {// might be a csv file, but only 1 column is unusual so check more carefully for a valid number
         char *s=csv_line; // start of 2nd line
         int c=*s++; // get 1st char in line
         while( isspace(c) )  c=*s++; // skip leading whitespace  if present
         if(c=='"') c=*s++; // skip "  if present
         while( isspace(c) )  c=*s++; // skip whitespace if present
         if(c=='+' || c=='+') c=*s++; // skip sign if present
         if(c=='.') c=*s; // skip leading decimal point  if present [no need to change s as this is its last use ]
         if(!isdigit(c))
                {// we have not found a valid number - warn user
                  ShowMessage("Warning: file does not appear to be a csv file; no number found on 2nd line of file");
                  // allow user to carry on - its only a problem if they try and load a column from file
                }
        }
  MAX_COLS=j;
  nos_cols_in_file=parsecsv(col_names,hdr_col_ptrs, MAX_COLS);// 1st line = headers
  parsecsv(csv_line,col_ptrs,MAX_COLS); // 2nd line (real data)
   {// update list boxes with names of columns found, use { so we can declare local variables
	// add in horizontal scroll bar only if its required
	int max_str_len_pixels=0,str_len_pixels;
	TLabel* label = new TLabel(Form1); // dynamic TLabel, just used so we can get the length of strings
	label->AutoSize = true;
	label->Font=Form1->pPlotWindow->ListBoxX->Font;
	//label->Font->Name = font; // your font
	// label->Font->Size = size; // your font size
	Form1->pPlotWindow->ListBoxX->Items->Clear();
	Form1->pPlotWindow->ListBoxY->Items->Clear();
	for(j=0;j<nos_cols_in_file;++j)
		{ // example column name from REPS is /'Data'/'Date/Time', dlete leading data and single quotes
		 if(strncmp(hdr_col_ptrs[j],"/'Data'/'",strlen("/'Data'/'"))==0)
				{hdr_col_ptrs[j]+=strlen("/'Data'/'");  // get rid of leading portion

				 for(char *s=hdr_col_ptrs[j];*s;++s)
						{if(s[1]==0 && *s=='\'') *s=0;   // get rid of final '
						}
				}
		 if(hdr_col_ptrs[j][0]=='"') // if column name is in double quotes then delete them
				{hdr_col_ptrs[j]++;  // get rid of leading "

				 for(char *s=hdr_col_ptrs[j];*s;++s)
						{if(s[1]==0 && *s=='"') *s=0;   // get rid of final "
						}
				}
		 rprintf("Col %-3d : %s  =%s,...\n",j+1,hdr_col_ptrs[j],col_ptrs[j]);// print col number, name of column from header line and value from 2nd line
#if 1
		 /* add text as column number:text from header . This helps if column headers are not very descriptive [or missing] */



		 snprintf(str_buf,sizeof(str_buf)-1,"%d: %s",j+1,hdr_col_ptrs[j]);
		 label->Caption =str_buf;
		 str_len_pixels=label->Width+5; // gets width of string in pixels . The extra "+5" gives a little space at the end so the whole character can be seen
		 if(str_len_pixels>max_str_len_pixels) max_str_len_pixels=str_len_pixels;
		 Form1->pPlotWindow->ListBoxX->Items->Add(str_buf);
		 Form1->pPlotWindow->ListBoxY->Items->Add(str_buf);
#else
		 label->Caption =hdr_col_ptrs[j];
		 str_len_pixels=label->Width;
		 if(str_len_pixels>max_str_len_pixels) max_str_len_pixels=str_len_pixels;
		 Form1->pPlotWindow->ListBoxX->Items->Add(hdr_col_ptrs[j]);
		 Form1->pPlotWindow->ListBoxY->Items->Add(hdr_col_ptrs[j]);
#endif
		}
	if(max_str_len_pixels>Form1->pPlotWindow->ListBoxX->Width)
		 Form1->pPlotWindow->ListBoxX->ScrollWidth=max_str_len_pixels;  // allow scrolling
	else
		Form1->pPlotWindow->ListBoxX->ScrollWidth=0;     // no scroll bar needed as text fits
	if(max_str_len_pixels>Form1->pPlotWindow->ListBoxY->Width)
		 Form1->pPlotWindow->ListBoxY->ScrollWidth=max_str_len_pixels;
	else
		Form1->pPlotWindow->ListBoxY->ScrollWidth=0;
	delete label;
   }
  fclose(fin);// only want 1st line here (just display headers so user can select ones to graph
  set_ListboxXY(); // highlight items in ListBoxX & Y that have been selected in Edit_xcol & Edit_ycol
  Application->ProcessMessages(); /* allow windows to update (but not go idle), do this regularly  */
  // now do fast count of lines in file
  addtraceactive=true;  // stop user adding traces until we have counted the number of lines in the file
  Form1->pPlotWindow->StatusText->Caption="Counting lines in file..." ;
  clock_t begin_t=clock();
  nos_lines_in_file=count_lines(filename.c_str(),filesize);  // this will keep user updated on its progress by updating status line message every 2 secs
  addtraceactive=false;
  rprintf(" File has %.0f lines (file read in %g secs)\n",(double)nos_lines_in_file,(double)(clock()-begin_t)/CLK_TCK);
  snprintf(str_buf,sizeof(str_buf),"Ready : %.0f lines found in file",(double)nos_lines_in_file);
  Form1->pPlotWindow->StatusText->Caption=str_buf;
  Form1->pPlotWindow->StaticText_filename->Text=basename;
}


void __fastcall TPlotWindow::Button_Filename1Click(TObject *Sender)
{  P_UNUSED(Sender);
  // pressed to set filename, as well as setting filename displays column headers
  if(addtraceactive || xchange_running!=-1)
	{// appear to still be busy doing something - ask user if this is true
	 UnicodeString errorText="csvgraph appears to still be busy - continue waiting?";
	 if( MessageDlg(errorText,mtConfirmation,TMsgDlgButtons() <<mbYes <<mbNo ,0) == mrNo)
		{addtraceactive= false;   // if user says "no" then clear flags  (if they say "yes" don't clear flags and we will return below)
		 xchange_running= -1;
		}
	}
  if(addtraceactive) return; // if still processing a previous call of addtrace return
  if(xchange_running!=-1) return; // still processing a change in x offset
  rcls();
  if(!check_function_tab())
        {ShowMessage("Internal error: check_function_tab() returned false");
         return;
        }
  rprintf("Set filename button pressed\n");

#ifndef UseVCLdialogs
  // use windows new getfilename dialog rather than old vcl one
  filename=getfilename(); // use new windows function to get filename , returns "" or a valid filename
#else
  // use old vcl getfilename dialog
  // ForceCurrentDirectory=True;// make current directory default for file
  if(FileOpenDialogCSV->Execute())
        {
          filename = FileOpenDialogCSV->FileName;
        }
  else
        {filename="";
        }
#endif
 proces_open_filename(filename.c_str());
}

//---------------------------------------------------------------------------

void __fastcall TPlotWindow::Edit_yChange(TObject *Sender)
{ P_UNUSED(Sender);
#if 0
 // only use Ylabel1 - Ylabel2 could be made user setable if required
 pScientificGraph->YLabel1=Edit_y->Text;
 pScientificGraph->YLabel2="";
#else
 /* let user split label into 2 parts by embeddeding \n in the text. Part goes into Ylabel1 and the rest onto YLabel2 */
 if(Edit_y->Text.Pos("\\n")>0)
        { // label is long - split into 2 lines on the 1st space
          int i=Edit_y->Text.Pos("\\n") ;
          int l= Edit_y->Text.Length();
          pScientificGraph->YLabel1=Edit_y->Text.SubString(1,i-1);
          pScientificGraph->YLabel2=Edit_y->Text.SubString(i+2,l); // +2 to skip \n
         }
 else
        { // label relatively short
         pScientificGraph->YLabel1=Edit_y->Text;
         pScientificGraph->YLabel2="";
        }
#endif
 fnReDraw();
}
//---------------------------------------------------------------------------

void __fastcall TPlotWindow::Edit_xChange(TObject *Sender)
{ P_UNUSED(Sender);
 pScientificGraph->XLabel=Edit_x->Text;
 fnReDraw();
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------







void __fastcall TPlotWindow::Edit_XoffsetChange(TObject *Sender)
{ P_UNUSED(Sender);
  static double last_Xoffset=0;
  if(addtraceactive || xchange_running!=-1)
	{// appear to still be busy doing something - ask user if this is true
	 UnicodeString errorText="csvgraph appears to still be busy - continue waiting?";
	 if( MessageDlg(errorText,mtConfirmation,TMsgDlgButtons() <<mbYes <<mbNo ,0) == mrNo)
		{addtraceactive= false;   // if user says "no" then clear flags  (if they say "yes" don't clear flags and we will return below)
		 xchange_running= -1;
		}
	}
  if(addtraceactive) return; // if still processing a previous call of addtrace return now
  if(++xchange_running!=0) return; // normally xchange_running would start at -1 so ++ makes it 0 , if code is already running just return here

  do{
      double new_Xoffset=atof(AnsiOf(Edit_Xoffset->Text.c_str())); // now get X offset
      bool changed;
      changed=pScientificGraph->fnChangeXoffset( new_Xoffset-last_Xoffset); // change ofset if possible
      if(changed)
        {
#if 0      /* its normally better not to rescale as user may have zoomed in on a feature to see the impact of changing the offset */
         StatusText->Caption="Autoscaling";
         pScientificGraph->fnAutoScale();
#endif
         StatusText->Caption="Drawing graph";
         fnReDraw();
         rprintf("Xofset changed by %g from %g to %g\n",new_Xoffset-last_Xoffset,last_Xoffset,new_Xoffset);
         StatusText->Caption="X offset updated";
        }
      last_Xoffset=new_Xoffset;
    } while(--xchange_running >=0) ; // normally  xchange_running will be 0 so -- makes it -1 and loop will exit. If another update request occurred while processing this one we repeat the while loop again
}
//---------------------------------------------------------------------------



void __fastcall TPlotWindow::About1Click(TObject *Sender)
{ P_UNUSED(Sender);
AboutBox->ShowModal();
}
//---------------------------------------------------------------------------



void __fastcall TPlotWindow::FormCreate(TObject *Sender)
{ P_UNUSED(Sender);
  DragAcceptFiles(Handle, true);
}
//---------------------------------------------------------------------------

void __fastcall TPlotWindow::Edit_titleChange(TObject *Sender)
{ P_UNUSED(Sender);
  Edit_title->Font->Size= pScientificGraph-> aTextSize;   // set correct size of text (same as X and Y axis titles)
}
//---------------------------------------------------------------------------

void __fastcall TPlotWindow::Exit1Click(TObject *Sender)
{ P_UNUSED(Sender);
  DragAcceptFiles(Handle, false);    // stop accepting drag n drop files
  exit(1); // close this screen exits application      
}
//---------------------------------------------------------------------------

void __fastcall TPlotWindow::ButtonSave1Click(TObject *Sender)
{ P_UNUSED(Sender);
 // saveas button pressed
#ifndef UseVCLdialogs
 save_filename=getsaveBMPfilename();// use new file selector
 if(save_filename!="")
        {
         Image1->Picture->SaveToFile(save_filename); //save image to file .bmp works
        }
#else
#if 0
 // this gives the old style windows dialog box, but at least the file extensions can be set automatically
 SavePictureDialog1->DefaultExt = GraphicExtension(__classid(Graphics::TBitmap));   // is only .bmp (at least in builder 5)
 SavePictureDialog1->Filter = GraphicFilter(__classid(Graphics::TBitmap));          // is only .bmp (at least in builder 5)

 if(SavePictureDialog1->Execute()) // get a filename     was SavePictureDialog1->Execute()
		{save_filename=SavePictureDialog1->FileName;
		 Image1->Picture->SaveToFile(save_filename); //save image to file .bmp works
		}
#else
 // new vista dialog, file extensions have to get set correctly in form editor
 // FileSaveDialog1->DefaultExtension = GraphicExtension(__classid(Graphics::TBitmap));   // is only .bmp (at least in builder 5)
 // FileSaveDialog1->FileTypes = GraphicFileMask(__classid(Graphics::TBitmap));          // is only .bmp (at least in builder 5)

 if(FileSaveDialog1->Execute()) // get a filename     was SavePictureDialog1->Execute()
		{save_filename=FileSaveDialog1->FileName;
		 Image1->Picture->SaveToFile(save_filename); //save image to file .bmp works
		}
#endif
#endif
 }
//---------------------------------------------------------------------------
#if 0   /* example from helptext allows save and saveas */
void __fastcallTForm1::Save1Click(TObject *Sender)

{ P_UNUSED(Sender);
  if (!CurrentFile.IsEmpty())
    Image->Picture->SaveToFile(CurrentFile);   // save if already named 
else SaveAs1Click(Sender);                     // otherwise get a name
}

void __fastcallTForm1::Saveas1Click(TObject *Sender)

{ P_UNUSED(Sender);
  if (SaveDialog1->Execute())              // get a file name
  {
    CurrentFile = SaveDialog1->FileName;  // save user-specified name
    Save1Click(Sender);                   // then save normally
  }
}
#endif
void __fastcall TPlotWindow::Button_PlotToClipboard1Click(TObject *Sender)
{ P_UNUSED(Sender);
  // rprintf("Copy to clipboard\n");
  Clipboard()->Assign(Image1->Picture); /* copy plot to clipboard */
}
//---------------------------------------------------------------------------


void __fastcall TPlotWindow::SaveDataAs1Click(TObject *Sender)
{ P_UNUSED(Sender);
 // save as CSV file
#ifndef UseVCLdialogs
 save_filename=getsaveCSVfilename();// use windows file selector
 if(save_filename!="")
		{
		 if(pScientificGraph->SaveCSV(save_filename.c_str(),AnsiOf(Edit_x->Text.c_str()),-MAXDOUBLE,MAXDOUBLE))   // use x axis title as name of 1st column in csv file
				{ShowMessage("CSV save completed OK");
				}
		}
#else
  // use VCL "vista" fileselector
  if(FileSaveDialogCSV->Execute()) // get a filename     was SavePictureDialog1->Execute()
		{save_filename=FileSaveDialogCSV->FileName;
		 if(pScientificGraph->SaveCSV(save_filename.c_str(),AnsiOf(Edit_x->Text.c_str()),-MAXDOUBLE,MAXDOUBLE))   // use x axis title as name of 1st column in csv file
				{ShowMessage("CSV save completed OK");
				}
		}
#endif
}
//---------------------------------------------------------------------------

void __fastcall TPlotWindow::ListBoxYClick(TObject *Sender)
{P_UNUSED(Sender);
 // user has selected some item(s) for Y columns
 bool first=true;
 AnsiString new_text="";  // we cannot gradually update Edit_ycol->Text as that has an "OnChange" event which will mess up the updates
 //rprintf("Y column selection made: items selected=%d\n",ListBoxY->SelCount );
 for (int i = 0; i < ListBoxY->Items->Count; i++)
        {
         if (ListBoxY->Selected[i])
                {// item has been selected by user
                 if(first)
                        {// 1st item just add number
                         //rprintf("1st Item %d selected\n",i+1);
						 new_text=i+1;
                         first=false;
                        }
                  else
                        {// If not the 1st item need to add ,number
                         //rprintf("Item %d also selected\n",i+1);
						 new_text=new_text+",";
						 new_text=new_text+(i+1);
                        }
                }
		}
 Edit_ycol->Text=new_text;
}
//---------------------------------------------------------------------------


void __fastcall TPlotWindow::ListBoxXClick(TObject *Sender)
{  P_UNUSED(Sender);
  // define column for X axis
  int Index = ListBoxX->ItemIndex;
  if(Index>=0)     // if something selected
       Edit_xcol->Text=Index+1;
}
//---------------------------------------------------------------------------

void __fastcall TPlotWindow::FormMouseWheelDown(TObject *Sender,
      TShiftState Shift, TPoint &MousePos, bool &Handled)
{      // mouse wheel moved Down, zoom in
  TPoint mp; // mouse in local co-ords
  double  dKoordX,dKoordY;
  P_UNUSED(Sender);
  P_UNUSED(Shift);
  mp=ScreenToClient(MousePos);
  if (pScientificGraph->fnPoint2Koord(mp.x,mp.y,dKoordX,dKoordY))
  // if(mp.x<(Panel1->Left+Panel1->Width))
        {  // only zoom if cursor inside actual graph area
         pScientificGraph-> fnZoomIn();
         Image1->Picture->Assign(pScientificGraph->pBitmap);
         zoomed=true;
         Handled=True;// tell system we have already dealt with scroll
        }
  // rprintf("Mouse wheel scrolled Down\n");

}
//---------------------------------------------------------------------------

void __fastcall TPlotWindow::FormMouseWheelUp(TObject *Sender,
      TShiftState Shift, TPoint &MousePos, bool &Handled)
{      // mouse wheel moved Up, zoom out
  TPoint mp; // mouse in local co-ords
  double  dKoordX,dKoordY;
  P_UNUSED(Sender);
  P_UNUSED(Shift);
  mp=ScreenToClient(MousePos);
  if (pScientificGraph->fnPoint2Koord(mp.x,mp.y,dKoordX,dKoordY))
  //if(mp.x<(Panel1->Left+Panel1->Width))
        {  // only zoom if cursor inside actual graph area
         pScientificGraph->fnZoomOut();
         Image1->Picture->Assign(pScientificGraph->pBitmap);
         zoomed=true;
         Handled=True;// tell system we have already dealt with scroll
        }
  // rprintf("Mouse wheel scrolled Up\n");
}
//---------------------------------------------------------------------------

extern int zoom_fun_level;  // used to keep track of level of recursion in zoom function (allows partial display refresh for speed in multiple zooms)

#if 1
// new version for use with undockable panel 2
 void __fastcall TPlotWindow::FormResize(TObject *Sender)
{
 /* resize of whole window, most movement of contents is automatic, here we fix the things that are not automatic */
 P_UNUSED(Sender);
 //int new_width, new_height ;
 if(pScientificGraph == NULL || pScientificGraph->pBitmap==NULL) return; // exiting - ignore the resize
 if(Panel2->Floating)
	{  // if panel 2 is floating panel 1 takes up all the space
        // rprintf("resizing panel 1 as panel 2 undocked\n");
		Panel1->Width =ClientWidth;
		Panel1->Height = ClientHeight<initial_Panel1_Height? initial_Panel1_Height : ClientHeight;
	}
  else
	{// use basically the same code as at startup - give panel 2 all the space it needs and panel 1 has the rest
	 Panel2->Width=Edit_y->Left + Edit_y->Width ; // make sure panel 2 is wide enough to fit widest (far right) item
	 Panel2->Left=ClientWidth-Panel2->Width ;
	 Panel2->Height = ClientHeight<initial_Panel1_Height? initial_Panel1_Height : ClientHeight;
	 Panel1->Width  =Panel2->Left-Panel1->Left;    // resize if necessary so graph fits in leaving space for controls on right
	 Panel1->Height = ClientHeight<initial_Panel1_Height? initial_Panel1_Height : ClientHeight;
	}

  Edit_title->Width=Panel1->Width-Edit_title->Left-20; // allow to go to (nearly) the far edge  (Title is centered)

 /* scale positions based on size */

  pScientificGraph->fLeftBorder = 0.1f*BASE_PWIDTH/Panel1->ClientWidth;      //Borders of Plot in Bitmap in %/100  was 0.2
  pScientificGraph->fBottomBorder = 0.1f*BASE_HEIGHT/Panel1->ClientHeight;   // was 0.25

  /* actually resize bitmap */
  Image1->Height=Panel1->ClientHeight;        //fit TScientificGraph-Bitmap in
  Image1->Width=Panel1->ClientWidth;          //TImage1-object in Panel1
  iBitmapHeight=Panel1->ClientHeight;
  iBitmapWidth=Panel1->ClientWidth;
  pScientificGraph->resize_bitmap(iBitmapWidth, iBitmapHeight);   // change actual size of bitmap
  if(zoom_fun_level==0)
	{// no recursion (yet)
	 zoom_fun_level=1;   // tell fnPaint() depth of recursion
	 while(zoom_fun_level)
		{if(zoom_fun_level==1)
			{pScientificGraph->fnPaint();                    //repaint (note this might be slow, so if another resize event occurs we abort fnPaint())
			 Image1->Picture->Assign(pScientificGraph->pBitmap);           //assign
			}
		  else
			{Image1->Picture->Assign(pScientificGraph->pBitmap);           //assign
			 Application->ProcessMessages(); /* allow windows to update (but not go idle) - potentially causes recursion ! */
			}
		 zoom_fun_level--;
		}
	 }
  else zoom_fun_level++;// flag zoom level changed
}
#else
 // old version that worked with undocked displays
void __fastcall TPlotWindow::FormResize(TObject *Sender)
{
 /* resize of whole window, most movement of contents is automatic, here we fix the things that are not automatic */
 P_UNUSED(Sender);
 int new_width, new_height ;
 // initial_ClientWidth=1591 initial_ClientHeight=980 initial_Panel1_Width=1300 initial_Panel1_Height=980
 new_width =initial_Panel1_Width+(ClientWidth-initial_ClientWidth);
 new_height =initial_Panel1_Height+(ClientHeight-initial_ClientHeight);
 if( new_width<100) return; // min allowed width of graph
 if(new_height<initial_Panel1_Height) new_height = initial_Panel1_Height; // scroll bar will appear if this happens so no point in reducing graph size as right "panel" will not shrink
 Panel1->Width  =new_width;
 Panel1->Height =new_height;
#if 1  /* scale positions based on size */
 if(pScientificGraph != NULL && pScientificGraph->pBitmap!=NULL)
	{
	 pScientificGraph->fLeftBorder = 0.1*BASE_PWIDTH/Panel1->ClientWidth;      //Borders of Plot in Bitmap in %/100  was 0.2
	 pScientificGraph->fBottomBorder = 0.1*BASE_HEIGHT/Panel1->ClientHeight;   // was 0.25
	 // pScientificGraph->dLegendStartX=0.6*BASE_PWIDTH/Panel1->ClientWidth;     //Legend position in %/100 in Plot was 0.8
	 // pScientificGraph->dLegendStartY=0.99*BASE_HEIGHT/Panel1->ClientHeight;    // was 0.95
	}
#endif
#if 1  /* actually resize bitmap */
  Image1->Height=Panel1->ClientHeight;        //fit TScientificGraph-Bitmap in
  Image1->Width=Panel1->ClientWidth;          //TImage1-object in Panel1
  iBitmapHeight=Panel1->ClientHeight;        //fit TScientificGraph-Bitmap in
  iBitmapWidth=Panel1->ClientWidth;          //TImage1-object in Panel1
											 //init ScientificGraph with Bitmap-
											 //size    #
  if(pScientificGraph != NULL && pScientificGraph->pBitmap!=NULL)
		{// make sure pBitmap is valid before we use it
		 pScientificGraph->resize_bitmap(iBitmapWidth, iBitmapHeight);   // change actual size of bitmap
		 if(zoom_fun_level==0)
				{// no recursion (yet)
				 zoom_fun_level=1;   // tell fnPaint() depth of recursion
				 while(zoom_fun_level)
						{if(zoom_fun_level==1)
								{pScientificGraph->fnPaint();                    //repaint (note this might be slow, so if another resize event occurs we abort fnPaint())
								 Image1->Picture->Assign(pScientificGraph->pBitmap);           //assign
								}
						 else
								{Image1->Picture->Assign(pScientificGraph->pBitmap);           //assign
								 Application->ProcessMessages(); /* allow windows to update (but not go idle) - potentially causes recursion ! */
								}
						 zoom_fun_level--;
						}
				}
		 else zoom_fun_level++;// flag zoom level changed
		}
#else  /* just rescale fixed size bitmap */
 Image1->Height =978+(new_height-980);
 Image1->Width =1298+(new_width-1591);
#endif
 // Image1->Picture->Assign(pScientificGraph->pBitmap);
}
#endif
//---------------------------------------------------------------------------

static bool firstpanel2undock=true;

void __fastcall TPlotWindow::Panel2EndDock(TObject *Sender, TObject *Target, int X,
		  int Y)
{P_UNUSED(Target);
 P_UNUSED(X);
 P_UNUSED(Y);
 // undock of panel2 (the main control panel)  - Note the Undock() event does not appear to be called so do it this way instead

#if 0
 if(Panel2->Floating)
	{SetFocus();
	}
#endif
 if(firstpanel2undock)
	{ if(Panel2->Floating)
		{firstpanel2undock=false;
		 // rprintf("Undock!\n");
		 // initial_Panel1_Width+=Panel2->Width ; // panel 1  (the graph) can now take all the space on the form
		 Panel3->Visible =true; // enable menu item so can make visible again if user clicks "X".
		 Panel3->Enabled =true;
		 visible1->Visible =true; // enable menu item so can make visible again if user clicks "X".
		 visible1->Enabled =true;
		 Panel2->AutoSize =false; // needs to be false to see scroll bars
		 Panel2->AutoScroll =true; // enable scroll bars
		 FormResize(Sender); // do the rest of the work for a form resize
		 Application->ProcessMessages();
#if 0    /* horiz scroll bar does not work very well..   */
		 rprintf("Panel2->VertScrollBar->Size=%d\n",(int)(Panel2->VertScrollBar->Size));
		 rprintf("Panel2: Width=%d Left=%d Top=%d Height=%d\n", Panel2->Width,Panel2->Left,Panel2->Top,Panel2->Height);
		 //Panel2->Width=Panel2->Width+20;
		 Panel2->HorzScrollBar->Range=Panel2->Width;   // +20 to allow for vertical scroll bar
         Panel2->HorzScrollBar->Visible=true;
		 Application->ProcessMessages();
#endif
		}
	}
}
//---------------------------------------------------------------------------



void __fastcall TPlotWindow::Panel2Resize(TObject *Sender)
{P_UNUSED(Sender);
#if 0
 if(Panel2->Floating  )
	{Panel2->AutoSize =false;
	 Panel2->AutoScroll =true; // enable scroll bars
	}
#endif
}
//---------------------------------------------------------------------------



void __fastcall TPlotWindow::visible1Click(TObject *Sender)
{P_UNUSED(Sender);
 // menu item panel/visible clicked    , make panel 2 visible again
 if(Panel2->Floating && !Panel2->Visible )
    Panel2->Visible =true;
}
//---------------------------------------------------------------------------

void __fastcall TPlotWindow::BitBtn_set_colourClick(TObject *Sender)
{P_UNUSED(Sender);
  /* Set trace colour button pressed */
  if(ColourDialog1->Execute())
	{
	 user_trace_colour=ColourDialog1->Color;      // invoke colour picker dialog
	 user_set_trace_colour=true;
	}
}
//---------------------------------------------------------------------------


void __fastcall TPlotWindow::Edit_xcolChange(TObject *Sender)
{P_UNUSED(Sender);
    set_ListboxXY(); // highlight items in ListBoxX & Y that have been selected in Edit_xcol & Edit_ycol
}
//---------------------------------------------------------------------------

void __fastcall TPlotWindow::Edit_ycolChange(TObject *Sender)
{P_UNUSED(Sender);
  set_ListboxXY(); // highlight items in ListBoxX & Y that have been selected in Edit_xcol & Edit_ycol
}
//---------------------------------------------------------------------------



void __fastcall TPlotWindow::Action1Execute(TObject *Sender)
{   // Help manual
  P_UNUSED(Sender);
#if 1
  AnsiString argv0=ParamStr(0);  // needed for builder 10.4 */
  char *progname=(char *)malloc(strlen(argv0.c_str())+1); /* +1 for null - will be shorter than this as remove a bit from argv[0] */
  strcpy(progname,argv0.c_str());
#else
  char *progname=strdup(_argv[0]);  // argv[0] is the filename & path to the csvgraph executable  , assume csvgraph.pdf is in the same directory
#endif
  size_t L=strlen(progname);
  progname[L-3]='p';// change extension from .exe to .pdf as thats the manual
  progname[L-2]='d';
  progname[L-1]='f';
  rprintf("Manual is at %s\n",progname);
#ifdef _WIN64
  ShellExecute(NULL,NULL, UnicodeOf(progname), NULL, NULL, SW_SHOW); // assume .pdf extension is associated with a suitable reader application
#else
  ShellExecute(NULL,NULL,progname, NULL, NULL, SW_SHOW); // assume .pdf extension is associated with a suitable reader application
#endif
  free(progname);
}
//---------------------------------------------------------------------------




void __fastcall TPlotWindow::Time_from0Click(TObject *Sender)
{ P_UNUSED(Sender);
  start_time_from_0=Time_from0->State==cbChecked;
}
//---------------------------------------------------------------------------

void __fastcall TPlotWindow::Save2Click(TObject *Sender)
{  // save x range on screen as CSV
 P_UNUSED(Sender);
 double xmin,xmax;
 xmin=pScientificGraph->fnGetScaleXMin();
 xmax=pScientificGraph->fnGetScaleXMax();
 rprintf("Save X range on screen (%g to %g) as CSV\n",xmin,xmax);
#ifndef UseVCLdialogs
 save_filename=getsaveCSVfilename();// use windows file selector
 if(save_filename!="")
		{
		 if(pScientificGraph->SaveCSV(save_filename.c_str(),AnsiOf(Edit_x->Text.c_str()),xmin,xmax))   // use x axis title as name of 1st column in csv file
				{ShowMessage("CSV save of X range on screen completed OK");
				}
		}
#else
  // use VCL "vista" fileselector
  if(FileSaveDialogCSV->Execute()) // get a filename     was SavePictureDialog1->Execute()
		{save_filename=FileSaveDialogCSV->FileName;
		 if(pScientificGraph->SaveCSV(save_filename.c_str(),AnsiOf(Edit_x->Text.c_str()),xmin,xmax))   // use x axis title as name of 1st column in csv file
				{ShowMessage("CSV save of X range on screen completed OK");
				}
		}
#endif
}
//---------------------------------------------------------------------------

void __fastcall TPlotWindow::SpinEdit_FontsizeChange(TObject *Sender)
{
 P_UNUSED(Sender);
 pScientificGraph->aTextSize=SpinEdit_Fontsize->Value;   // set size of text for axis titles and main title
 Edit_title->Font->Size=SpinEdit_Fontsize->Value;
 fnReDraw(); // redraw screen with new font size
}
//---------------------------------------------------------------------------

void __fastcall TPlotWindow::SpinEdit_FontsizeKeyPress(TObject *Sender, System::WideChar &Key)

{ P_UNUSED(Sender); // stops user typing a number into the font size (this would not be clipped to be between MinValue & MaxValue
   Key=0;  // make system ignore key
}
//---------------------------------------------------------------------------

void __fastcall TPlotWindow::FormGetSiteInfo(TObject *Sender, TControl *DockClient,
		  TRect &InfluenceRect, TPoint &MousePos, bool &CanDock)
{
 P_UNUSED(Sender);
 P_UNUSED(DockClient);
 P_UNUSED(InfluenceRect);
 P_UNUSED(MousePos);
 CanDock=false;// no docking allowed
}
//---------------------------------------------------------------------------

