/* rprintf.h header file for rprintf.cpp

  WARNING: you must #define NoForm1 unless Form1 is defined before including this file !

  Includes rprintf(char *fmt,...) to print to screen with user selectable colours/bold and rcls() to clear screen.

   Note: for rprintf to work there must be a tRichEdit control on the main form called Results
   This should be set with:
		 Anchors all true (right & bottom are false by default) (so text box resizes with main form)
		 ScrollBars = ssBoth
		 Readonly = true
		 WordWrap = false   (if set to true long lines will be automatically split which confuses my \n code)
*/
/*----------------------------------------------------------------------------
 * Copyright (c) 2012, 2013, 2022 Peter Miller
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
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *--------------------------------------------------------------------------*/
#ifndef _rprintf_h
#define _rprintf_h
#ifdef __cplusplus /* most of this file is only for c++, but it does define crprintf(...) for C */
#include <stdio.h>  /* for FILE * - avoids having to fix order of includes */
#include "stdint.h" /* for uint32_t etc */
#include <stdarg.h> /* for va_start etc */

#if 0
 /* disable try/catch - crude but can help debugging, results in quite a few warnings, but should still compile */
#define TRY_CATCH_DISABLED
#define try /* disabled */
#define catch(...) if(false) /* disabled*/
#endif 

void red_text(void); /* set text colour to red for new text [Richedit only]*/
void blue_text(void); /* set text colour to blue for new text [Richedit only]*/
void green_text(void); /* set text colour to green for new text [Richedit only]*/
void black_text(void); /* set text colour to black (default) [Richedit only]*/
void bold_text(void);  /* set text bold for new text [Richedit only] - no change on colour */
void nobold_text(void); /* turn bold text off [Richedit only] - no change of colour */
void normal_text(void); /* return text formatting to default [Richedit only] */
                    /* Warning: colour changes only work on whole lines , changes mid line impact the whole line due to the \n handling code */

char * validate_num(char *text, float min, float max, float *d,bool *ok, char *onerror); /* validate input from an edit control etc , returns new value for control [unchanged if OK]*/
TColor cvalidate_num(char *text, float min, float max, float *d,bool *ok); /* validate input from an edit control etc , returns clRED on error*/
TColor cvalidate_num(wchar_t *text, float min, float max, float *d,bool *ok); /* validate input from an edit control etc , returns clRED on error*/


void rcls(void); /* clear Results memobox/Richedit box */
void SetCursorToEnd(void); /* scrolls Results RichEdit viewpoint to the end of the text, will continue to scroll so most recently added text is visible */

void rprintf(const char *fmt, ...);    /* like printf but output to Results memobox */
								/* \n's work as expected and colours can be set(see above)  */
extern "C" void crprintf(const char *fmt, ...); /* version callable from C */
void save_gui(FILE *fp,int VersionMajor,int VersionMinor); /* save GUI values in a way that can be restored by restore_gui() */
void restore_gui(FILE *fp,int VersionMajor,int VersionMinor); /* restore GUI values saved by save_gui() */
#ifndef NoForm1
/* Form1 is available - use it here ! */
void _rcls(TRichEdit *Results); /* generic function to clear Results memobox/Richedit box */
void _SetCursorToEnd(TRichEdit *Results); /* generic function to scrolls Results RichEdit viewpoint to the end of the text, will continue to scroll so most recently added text is visible */
void rcls(void) /* clear Results memobox/Richedit box */
        {_rcls(Form1->Results);
        }
void SetCursorToEnd(void) /* scrolls Results RichEdit viewpoint to the end of the text, will continue to scroll so most recently added text is visible */
        {_SetCursorToEnd(Form1->Results);
        }


void _rprintf(TRichEdit *Results,const char *fmt,va_list arglist);    /* generic function to ,like printf but output to Results memobox */
                                /* \n's work as expected and colours can be set(see above)  */
void rprintf(const char *fmt, ...)    /* like printf but output to Results memobox */
                                /* \n's work as expected and colours can be set(see above)  */
        { va_list arglist;
          va_start(arglist, fmt);
          _rprintf(Form1->Results,fmt,arglist);
          va_end(arglist);
        }

extern "C" void crprintf(const char *fmt, ...) /* version callable from C */
        { va_list arglist;
          va_start(arglist, fmt);
          _rprintf(Form1->Results,fmt,arglist);
          va_end(arglist);
        }
		
#if 0
/* following 2 functions need reasonable access to "Form1" so the whole code is below [sorry!] */
void save_gui(FILE *fp,int VersionMajor,int VersionMinor) /* save GUI values in a way that can be restored by restore_gui() */
 /* file must be opened before call and closed after call */
 /* works for editboxes, radiogroups & TCSpinedits */
{
 TEdit * editbox;
 TRadioGroup * rg;
#ifdef USE_SPINEDIT
 TCSpinEdit * se;
#endif
 int nosedit=0,nosradiogroup=0,nosspinedit=0;
 if(fp==NULL) return; // just in case!
 fprintf(fp,"Version,%d,%d\n",VersionMajor,VersionMinor);
 for(int i=0; i < Form1->ComponentCount; i++)
		{if (Form1->Components[i]->ClassNameIs("TEdit") )
				{/* cast to item of correct type */
				 editbox = (TEdit *)Form1->Components[i];
				 fprintf(fp,"TEdit,%s,\"%s\"\n",editbox->Name.c_str(),editbox->Text.c_str());  // print out to file - assumes text does not contain "
				 nosedit++;
			}
		 else if (Form1->Components[i]->ClassNameIs("TRadioGroup"))
				{
				 rg = (TRadioGroup *)Form1->Components[i];
				 fprintf(fp,"TRadioGroup,%s,%d\n",rg->Name.c_str(),rg->ItemIndex);  // print out to file
				 nosradiogroup++;
			}
#ifdef USE_SPINEDIT
		 else if (Form1->Components[i]->ClassNameIs("TCSpinEdit"))
				{
				 se = (TCSpinEdit *)Form1->Components[i];
				 fprintf(fp,"TCSpinEdit,%s,%d\n",se->Name,se->Value);  // print out to file
				 nosspinedit++;
			}
#endif
		}
 rprintf("Save completed OK\nProgram version %d.%d\n%d Editboxes, %d Radiogroups and %d Spinedits saved\n",
		VersionMajor,VersionMinor,nosedit,nosradiogroup,nosspinedit);
}

void restore_gui(FILE *fp,int VersionMajor,int VersionMinor) /* restore GUI values saved by save_gui() */
 /* file must be opened before call and closed after call */
 /* works for editboxes, radiogroups & TCSpinedits */
{
 TEdit * editbox;
 TRadioGroup * rg;
#ifdef USE_SPINEDIT
 TCSpinEdit * se;
#endif
 int nosedit=0,nosradiogroup=0,nosspinedit=0;    /* numbers read from input */
 int noseditgui=0,nosradiogroupgui=0,nosspineditgui=0;    /* numbers actually in gui */
 /* bool parsecsv(char *in,char *outfields[],unsigned int maxfields) /* char *outfields[maxfields] needed */
 const unsigned int maxfields=3;/* type,name,value */
 char *outfields[maxfields];
 char *line;
 if(fp==NULL) return; // just in case!
 for(int i=0; i < Form1->ComponentCount; i++)
		{if (Form1->Components[i]->ClassNameIs("TEdit") )
				{// 1st just count number of each type in gui
				 noseditgui++;
			}
		 else if (Form1->Components[i]->ClassNameIs("TRadioGroup"))
				{
				 nosradiogroupgui++;
			}
#ifdef USE_SPINEDIT
		 else if (Form1->Components[i]->ClassNameIs("TCSpinEdit"))
				{
				 nosspineditgui++;
			}
#endif
		}
 while(line=readline(fp),line!=NULL)
		{/* got a valid line - parse CSV */
		 if(!parsecsv(line,outfields,maxfields))
		   continue;
		 // got a valid line, process it
		 if(strcmp(outfields[0],"Version")==0)   // should be 1st line in file
				{if(atoi(outfields[1])==VersionMajor && atoi(outfields[2])== VersionMinor)
						{/* file saved by same version as we are reading */
						}
				 else
						{rprintf("warning: File read created by version %d.%d, this is version %d.%d\n",
								 atoi(outfields[1]), atoi(outfields[2]), VersionMajor, VersionMinor);
						}
				}
		 else if(strcmp(outfields[0],"TEdit")==0)
				{ /* search for it in gui */
				 char *cp=NULL;
				 for(int i=0; i < Form1->ComponentCount; i++)
						{if (Form1->Components[i]->ClassNameIs("TEdit") )
								{/* cast to item of correct type */
								 editbox = (TEdit *)Form1->Components[i];
								 if(editbox->Name==outfields[1]) /* found it! */
										{ *outfields[2]=0;// delete leading "
										  for(cp=outfields[2]+1;*cp;++cp);// find end of string
										  cp--; // char before end should be a "
										  *cp=0;// delete final "
										  cp=outfields[2]+1; /* after initial " */
										  editbox->Text=cp;
										  ++nosedit;
										  break;
										}
								}
						}
				 if(cp==NULL) /* not found */
					  rprintf("Warning %s %s with value %s not found in gui\n",outfields[0],outfields[1],outfields[2]);
				}
		 else if(strcmp(outfields[0],"TRadioGroup")==0)
				{ /* search for it in gui */
				 char *cp=NULL;
				 for(int i=0; i < Form1->ComponentCount; i++)
						{if (Form1->Components[i]->ClassNameIs("TRadioGroup") )
								{/* cast to item of correct type */
								 rg = (TRadioGroup *)Form1->Components[i];
								 if(rg->Name==outfields[1]) /* found it! */
										{
										  cp=outfields[2];
										  rg->ItemIndex=atoi(cp);
										  ++nosradiogroup;
										  break;
										}
								}
						}
				 if(cp==NULL) /* not found */
					  rprintf("Warning %s %s with value %s not found in gui\n",outfields[0],outfields[1],outfields[2]);
				}
#ifdef USE_SPINEDIT
		 else if(strcmp(outfields[0],"TCSpinEdit")==0)
				{ /* search for it in gui */
				 char *cp=NULL;
				 for(int i=0; i < Form1->ComponentCount; i++)
						{if (Form1->Components[i]->ClassNameIs("TCSpinEdit") )
								{/* cast to item of correct type */
								 se = (TCSpinEdit *)Form1->Components[i];
								 if(se->Name==outfields[1]) /* found it! */
										{
										  cp=outfields[2];
										  se->Value=atoi(cp);
										  ++nosspinedit;
										  break;
										}
								}
						}
				 if(cp==NULL) /* not found */
					  rprintf("Warning %s %s with value %s not found in gui\n",outfields[0],outfields[1],outfields[2]);
				}
#endif
		 else   {
				 rprintf("Error: Unexpected line in loaded file: %s %s with value %s\n",outfields[0],outfields[1],outfields[2]);
				}
		}
 rprintf("Load completed\n%d out of %d Editboxes, %d out of %d Radiogroups and %d out of %d Spinedits loaded\n",
		nosedit,noseditgui,nosradiogroup,nosradiogroupgui,nosspinedit,nosspineditgui);
}
#endif

#endif
#else /* declarations for C */
void crprintf(const char *fmt, ...); /* version callable from C */
#endif
#endif
