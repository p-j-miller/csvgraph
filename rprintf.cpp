//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
/* rprintf.cpp formatted printing to the screen via a richedit control and other generally useful code for Builder C++ */
/* Note that this version supports utf-8 encoded text as long as the TRichEdit box has a suitable font (Arial works OK). */
/*----------------------------------------------------------------------------
 * Copyright (c) 2012, 2013,2022 Peter Miller
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


#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <Dialogs.hpp>
#define NoForm1 /* to say we do not have form1 available , needed before including expr-code.h */
#include "expr-code.h"
#include <cstdlib> /* for malloc() and free() */
#include <string.h> /* for strcmp() etc */
#include <stdarg.h> /* for va_start etc */
#include <stdio.h>  /* for printf etc */

#include <ctype.h> /* isspace etc */
#include <math.h>
#include <float.h> /* _isnan() */
#include <values.h> /* MAXFLOAT etc */
#include "stdint.h" /* for uint32_t etc */
#include "interpolate.h"
#include "getfloat.h"
#ifdef USE_SPINEDIT
#include <CSPIN.h> /* for spinedits */
#endif



// I'm sorry for the next 2 lines, but otherwise I get a lot of warnings "zero as null pointer constant"
#undef NULL
#define NULL (nullptr)

void _rcls(TRichEdit *Results); /* generic function to clear Results memobox/Richedit box */
void _SetCursorToEnd(TRichEdit *Results); /* generic function to scrolls Results RichEdit viewpoint to the end of the text, will continue to scroll so most recently added text is visible */

/* next function is used to support conversion to unicode vcl see https://blogs.embarcadero.com/migrating-legacy-c-builder-apps-to-c-builder-10-seattle/
*/
#define STR_CONV_BUF_SIZE 2000 // the largest string you may have to convert. depends on your project

static char* __fastcall Utf8Of(wchar_t* w)       /* convert to utf-8 encoding */
{
	static char c[STR_CONV_BUF_SIZE];
	memset(c, 0, sizeof(c));
	WideCharToMultiByte(CP_UTF8, 0, w, -1, c, STR_CONV_BUF_SIZE-1, NULL, NULL);       /* size-1 ensure result is null terminated */
	return(c);
}

static wchar_t* __fastcall Utf8_to_w(const char* c)     // convert utf encoded string to wide chars - new function
{
	static wchar_t w[STR_CONV_BUF_SIZE];
	memset(w,0,sizeof(w));
	MultiByteToWideChar(CP_UTF8, 0, c, /*(int)strlen(c)*/ -1, w, STR_CONV_BUF_SIZE-1);  // utf-8 (this is a windows function )   , -1 for input length means process to (including) null
	return(w);
}

/* below for rprint */
static int rline=0; /* last line printed to - used to allow partial lines to be added */
static bool lastlinehadCR=true; /* true if last line ended in a \n */
static bool text_blue=false;    /* "colour" of future lines */
static bool text_red=false;
static bool text_green=false;
static bool text_bold=false;

/* external functions to alow setting of text "colours", set colours of future lines */
void red_text(void)
{text_red=true;
 text_blue=false;
 text_green=false;
}

void blue_text(void)
{text_blue=true;
 text_red=false;
 text_green=false;
}

void green_text(void)
{text_green=true;
 text_blue=false;
 text_red=false;
}

void black_text(void) /* set text colour to black (default) [Richedit only]*/
{
 text_blue=false;
 text_red=false;
 text_green=false;
}

void bold_text(void)
{
 text_bold=true;
}

void nobold_text(void) /* turn bold text off */
{
 text_bold=false;
}

void normal_text(void) /* resets all attributes to default */
{
 text_blue=false;
 text_red=false;
 text_green=false;
 text_bold=false;
}

/* internal functions to change 1 line of text */
static void red_text1(TRichEdit *Results)
{Results->SelAttributes->Color = clRed;
}
static void blue_text1(TRichEdit *Results)
{Results->SelAttributes->Color = clBlue;
}
static void green_text1(TRichEdit *Results)
{Results->SelAttributes->Color = clGreen;
}
static void bold_text1(TRichEdit *Results)
{TFontStyles s;
 s << fsBold;
 Results->SelAttributes->Style = s;
}

static void normal_text1(TRichEdit *Results) /* resets all attributes to default */
{ Results->SelAttributes = Results->DefAttributes;
}

static void colour_text(TRichEdit *Results) /* set text to correct colour - for 1 line only */
{normal_text1(Results); /* assume text is "normal", change if any flag is set */
 if(text_red) red_text1(Results);
 if(text_blue) blue_text1(Results);
 if(text_green) green_text1(Results);
 if(text_bold) bold_text1(Results);
}

void _rcls(TRichEdit *Results) /* clears Results memobox or Richedit box */
{ rline=0;
  lastlinehadCR=true;
  normal_text(); /* back to default colour scheme */
  colour_text(Results); /* actually set default colour scheme */
  Results->Lines->Clear(); /* actually clear display */
}

void _SetCursorToEnd(TRichEdit *Results) /* scrolls Results RichEdit viewpoint to the end of the text */
                        /* once cursor is there it stays there and windows will automatically scroll so lastest text is visible */
{
#if 1
 Results->Perform(WM_VSCROLL, SB_BOTTOM,static_cast<NativeInt>(0)) ;   // this always seems to work (?)
#else
 // another more complex solution...  Note this will still cause exceptions if done at start !

 try
   {Results->SelStart=Results->GetTextLen();
    Results->SetFocus();   //  if this is done before any text is added to control in  an oncreate event then this error is tripped (and function does not work!)
                                   // the solution is to add some text 1st!
    Results->Perform(EM_SCROLLCARET,0,0);
   }
 catch( const Exception &E) { };  // don't do anything other than catch error
 // catch (...) {} ;// don't do anything other than catch error

#endif

}

void _rprintf(TRichEdit *Results,const char *fmt, va_list arglist) ;

 /* This version fully supports utf-8 encoded strings */
void _rprintf(TRichEdit *Results,const char *fmt, va_list arglist)    /* like printf but output to Results memobox \n's work as you would expect */
								/* this should be the only way to write to memobox (apart from rcls() to clear )*/
								/* this version uses p.vprintf and AnsiStrings only so no limits to buffer size */
								/* note because it takes a va_list argument it cannot be called from "user code" directly - it needs a "wrapper" like rprintf in the header file */
{
   int i=1,j;   // i=start of current line, j is index looking for newline characters
   AnsiString p;    // for result of printf
   p.vprintf( fmt, arglist);  /* puts result into string p */
   // newline (and colour) handling
   for(j=1;j<=p.Length();++j) /* indexing of strings starts from 1 ! */
	{if(p[j]=='\n' || j==p.Length() )
		{// newline, or reached end of string (so print what we have)
		 if(!lastlinehadCR)
			{WideString t;
			 if(p[j]=='\n')
				t = Results->Lines->Strings[rline]+Utf8_to_w((p.SubString(i,j-i)).c_str()); /* old line + new without \n*/
			 else
				t = Results->Lines->Strings[rline]+Utf8_to_w((p.SubString(i,j+1-i)).c_str()); /* old line + all of new (as no \n)*/
			 Results->Lines->Delete(rline);  /* delete old */
			 colour_text(Results); /* set text to current colours */
			 rline=Results->Lines->Add(t);  /* replace with new */
			}
		 else
			{colour_text(Results); /* set text to current colours */
			 if(p[j]=='\n')
				 rline=Results->Lines->Add(Utf8_to_w((p.SubString(i,j-i)).c_str()));   /* display this line of result (without \n) */
			 else
				rline=Results->Lines->Add(Utf8_to_w((p.SubString(i,j+1-i)).c_str()));   /* display this line of result (with whole string as no \n) */
			}
		 lastlinehadCR=(p[j]=='\n'); /* note if CR at end of line */
		 i=j+1;  /* character after \n */
		}
	}
   Application->ProcessMessages(); /* allow windows to update (but not go idle) */
   return;
}


/* functions to validate numeric input */
char * validate_num(char *text, float min, float max, float *d,bool *ok, char *onerror)
  /* validate input from an edit control etc , returns new value for control [unchanged if OK]*/
  /* this seems a hard way to do it, but AnsiStrings cannot be passed as parameters to functions ! */
 /* returns *text, with OK true is input is a valid number, and value in *d
    otherwise returns onerror, with *d to min and returns OK=false
    use:
    AnsiString s="12.34";
    float val;
    book OK;
    s=validate_num(s.c_str(),0,100,&val,&OK,"error");
     Note if used with a TEdit control OnChange Event user may need to select whole input and 1 character needs to be valid
    */
{float val;
 if(getfloat(text,&val) && val>=min && val<=max)
        {/* OK */
         *d=val;
         *ok=true;
         return text;
        }
 /* bad */
 *ok=false;
 *d=min;
 return onerror;
}
TColor cvalidate_num(char *text, float min, float max, float *d,bool *ok)
  /* validate input from an edit control etc , returnd clRED on error*/
  /* *d and *ok can be NULL */
 /* returns *text, with OK true is input is a valid number, and value in *d
    otherwise returns onerror, with *d to min and returns OK=false
    use:
    TEdit e1;
    float val;
    book OK;
    e1->Font->Color=cvalidate_num(e1->Text.c_str(),0,100,&val,&OK);
    */
{float val;
 if(getfloat(text,&val) && val>=min && val<=max)
        {/* OK */
         if(d!=NULL) *d=val;
         if(ok!=NULL) *ok=true;
         return clWindowText;
        }
 /* bad */
 if(ok!=NULL) *ok=false;
 if(d!=NULL) *d=min;
 return clRed;
}

TColor cvalidate_num(wchar_t *text, float min, float max, float *d,bool *ok) // like above for wchar_t *
{ return cvalidate_num(Utf8Of(text),min, max, d,ok);
}

