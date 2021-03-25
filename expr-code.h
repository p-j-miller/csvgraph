/* header file for expression handling code

  WARNING: you must #define NoForm1 unless Form1 is defined before including this file !
*/
/*----------------------------------------------------------------------------
 * Copyright (c) 2012, 2013 Peter Miller
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
#include <stdio.h>  /* for FILE * - avoids having to fix order of includes */
#include "stdint.h" /* for uint32_t etc */
#include <stdarg.h> /* for va_start etc */

#ifndef NO_MIN_MAX      /* allows us to not define these if they are defined elsewhere - eg in the STL */
#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))
#endif

#define Allow_dollar_vars_in_expr /* if defined allow variables of $1 .. like awk in xepr */
/* general purpose hash functions based on Fowler/Noll/Vo hash fnv-1a , with optional "mixing" at the end */
void hash_reset(uint32_t *h); // reset hash h to its initial value
void hash_add(char *s, uint32_t *h); // adds string s to hash h, updates h
uint32_t hash_str(char *s); // returns hash of string  , always starting from the same initial value for hash

/* table lookup code based on K&R The C programming language (1st Ed) section 6.6 */

typedef struct snlist
    { /* basic table entry */
      char *name;
      double value;
      struct snlist *next; /* pointer to next entry in chain at this hash value */
    } nlist, *pnlist;

pnlist lookup(char *s); /* lookup s , returns pointer to struct snlist or NULL if not found */
pnlist install(char *name); /* adds name to hashtab if it does not exist, returns pointer to entry created or NULL on error */
#ifdef Allow_dollar_vars_in_expr
double get_dollar_var_value(pnlist p); /* get value of $ variable */
#endif

void red_text(void); /* set text colour to red for new text [Richedit only]*/
void blue_text(void); /* set text colour to blue for new text [Richedit only]*/
void green_text(void); /* set text colour to green for new text [Richedit only]*/
void black_text(void); /* set text colour to black (default) [Richedit only]*/
void bold_text(void);  /* set text bold for new text [Richedit only] - no change on colour */
void nobold_text(void); /* turn bold text off [Richedit only] - no change of colour */
void normal_text(void); /* return text formatting to default [Richedit only] */
                    /* Warning: colour changes only work on whole lines , changes mid line impact the whole line due to the \n handling code */


bool getfloat(char *s, float *d); /*reads a floating point number returns true if valid - allows whitespace as well as a number , d=0 on error*/
bool getfloatgt0(char *s, float *d);/* as above but requires number to be >0 */
bool getfloatge0(char *s, float *d);/* as above but requires number to be >0 */

float gethms(char *s); /* read a time of format hh:mm:ss.s , returns time in seconds */
void reset_days(void);  /* reset static variables for gethms_days() - should be used before using gethms_days() to read times from a file  */
double gethms_days(char *s); /* read time in format hh:mm:ss.s , assumed to be called in sequence and accounts for days when time wraps around. Returns secs */

char * validate_num(char *text, float min, float max, float *d,bool *ok, char *onerror); /* validate input from an edit control etc , returns new value for control [unchanged if OK]*/
TColor cvalidate_num(char *text, float min, float max, float *d,bool *ok); /* validate input from an edit control etc , returns clRED on error*/
double s_to_double(char *s); /* converts s to double if its a number, otherwise returns NAN */

void to_rpn(char *expression); /* convert expression to rpn */
void assign_expr(char *expression); /* deal with assignments , evaluates expression and stores result in variable ; allows multiple assignments on one line */
bool last_expression_ok(); /* returns true if last expression was correct in syntax , false otherwise */
void print_rpn(); /* for debugging - print out rpn "code" created by last call to to_rpn() or optimise_rpn() */
void optimise_rpn(); /* optimise rpn from previous call to to_rpn() */
double execute_rpn(); /* execute  rpn from a previous call to to_rpn() it & return the resultant value , 0 (and flag false) on error */
bool check_function_tab(); /* returns true if function table is valid, false if not */

bool regex_match(char *regex, char *text); /* regex .^$* as special character (.=any char, ^ start, $ end, *=0+ occurances of previous char + = 1+ occurances */
bool sexpr(char *in[],int maxfields,char *sexpression); /* see if line (split up by parsecsvmatches sexpression matches string expression */
bool last_sexpr_ok(); /* returns true if last sexpr expression was correct in syntax , false otherwise */

#define P_BOOL(b) ((b)?"true":"false") /* use with a %s param in printf to print a bool out */
#define ELEMENTS_IN_ARR(x) (sizeof(x)/sizeof(x[0])) /* number of elements in array x */

/* generalised least squares - fit y=a*f(x)+b*g(x) - see below for a number of optimised versions for special cases of f(x) and g(x) */
void leastsquares_reg(float *y,float *x,int start, int end,double (*f)(float xparam),double (*g)(float xparam), double *a, double *b);
/* generalised least squares - fit y=a*f(x)+b*g(x)+c  */
void leastsquares_reg3(float *y,float *x,int start, int end,double (*f)(float xparam),double (*g)(float xparam), double *a, double *b, double *c);
void leastsquares_rat3(float *y,float *x,int start, int end, double *a, double *b, double *c); /* fits y=(a+bx)/(1+cx) */
/* specialised versions of above - faster and more accurate that using general version above */
void lin_reg_through_a_b(float a, float b,float *y,float *x, int start, int end, double *m, double *c); /* y=mx */
void lin_reg_GMR(float *y,float *x, int start, int end, bool GMR,double *m, double *c, double *r2); // lin reg optional GMR
void fit_min_abs_err_line(float *x, float *y,unsigned int nos_vals,bool rel_error,double *m_out, double *c_out,double *best_err_out); // fit min rel/abs error line
void lin_reg(float *y,float *x, int start, int end, double *m, double *c, double *r2); /* linear regression */
void log_reg(float *y,float *x, float yoff,float xoff,int start, int end, double *m, double *c, double *r2); /* linear regression on logx / logy*/
/* yoff and xoff are subtracted from *y and *x this is useful as log(z) requires z>0 */
void log_lin_reg(float *y,float *x, float yoff,float xoff,int start, int end, double *m, double *c, double *r2); /* linear regression on logy vs x*/
/* yoff and xoff are subtracted from *y and *x this is useful as log(z) requires z>0 */
void log_diff_lin_reg(float *y,float *x,float xoff,int start, int end, double *m, double *c, double *r2); /* linear regression on log(dy/dx) vs x*/
double deriv(float *y,float *x,int start, int end, int index); /* estimate dy/dx at index - only using data between start and end */



unsigned int csv_count_cols(char *in) ; // csv_count_cols(char *in ) returns number of columns in string "in" using same algorithm as parsecsv()
  /* parse input line into a number of fields (columns) - does NOT change "in"
	 in ends with a \n OR a \0
	 returns number of fields found in line
	 fields are comma seperated.
	 If a field is double quoted ("..." or a "string") commas inside quotes don't act as field seperators
	 "" inside a string is ignored (so the comma in ".."".,." is ignored) 
	 \" inside a string is ignored (so the comma in "..\".,." is ignored) 
  */
unsigned int parsecsv(char *in,char *outfields[],unsigned int maxfields); /* char *outfields[maxfields] needed */
  /* parse input line into a number of fields - CHANGES input !!!
	 in ends with a \n OR a \0
	 returns number of fields found in line
	 if not enough fields present in input the excess entries in outfields are all set to point to the terminating \000
	 if too many fields (>maxfields) are present in the input line then the extra fields are ignored
	 if in == NULL or *in=EOL all outfields are set to a string thats just \000, and returns 0
	 fields are comma seperated.
	 If a field is double quoted ("..." or a "string") commas inside quotes don't act as field seperators
	 "" inside a string is ignored (so the comma in ".."".,." is ignored) 
	 \" inside a string is ignored (so the comma in "..\".,." is ignored) 	 
  */
unsigned int parsewhitesp(char *in,char *outfields[],unsigned int maxfields); /* char *outfields[maxfields] needed */
  /* parse input line into a number of fields - CHANGES input !!!
	 field seperator is "whitespace"  , multiple "whitespace" chars are treated as a single seperator
	 leading whitespace on any field is skipped (so whitespace at the start of a line does not cause the field to be incremented )
	 in ends with a \n OR a \0
	 if not enough fields present in input the excess are all set to point to the terminating \000
	 if too many fields (>maxfields) are present in the input line then the extra fields are ignored
	 trailing whitespace 9at the end of the line is also ignored
	 if in == NULL or *on=EOL all outfields are set to a string thats just \000, and returns 0, otherwise returns nos fields in input
  */

char *readline (FILE *fp); /* read next line from input and return a pointer to it. Returns NULL on EOF or error . Deletes \n from end of line */

float nextafterf(float f); /* returns the next higher floating point number - should be in  C11 math.h */

float fastexp(float x);    /* fast approximation to exp(x) max_rel_err = 3.10978e-05 , very accurate for x near 0 [for |x|<0.0077 has same accuracy and execution speed as below]*/
float fasterexp(float x);  /* faster, but less accurate approximation to exp(x) max_rel_err = 0.030280, very accurate for x near 0 [for |x|<0.0077 has same accuracy and execution speed as above] */
float fastlog (float x);   /* fast approximation to loge(x) maxrelative error of 0.00330059 & max abs error of 0.000107527 */
float fasterlog (float x); /* faster, but less accurate approximation to loge(x) maxrelative error of 0.0732524 & max abs error of 0.0397146 very accurate near |x|=1*/
float fastsqrt(float x);   /* fast sqrt maxrelative error of 0.00175123 */
float fastersqrt(float x); /* faster, but less accurate sqrt maxrelative error of 0.0351635  */
float fastinv(float x);    /* fast 1/x maxrelative error of 0.00477122 , useful to avoid slow divisions */
float fasterinv(float x);  /* even faster 1/x but worse error - maxrelative error of 0.0690738 . Useful to avoid slow divisions */

#if 1
static inline float fpabs(float f) /* whole thing needs to be in header if its to be inline */
{if(f<0.0f) return -f;
 return f;
}
#else
 /* this version may be faster - just removes the sign bit from the ieee format float */
 /* Warning: causes a bug in borland builder 5 if used  before #pragma hdrstop  */
static inline float fpabs(float f)
{union _f_andi32
        {float uf;
         int32_t ui;
        } f_i;
 f_i.uf=f;
 f_i.ui &= 0x7fffffff;
 return f_i.uf;
}
#endif

float t90(int dt); /* returns t score for checking both upper and lower limits at 90% certainty - from pp154 Texas Instruments TI-51-III owners manual */
float t99(int dt); /* returns t score for checking both upper and lower limits at 99% certainty - from pp154 Texas Instruments TI-51-III owners manual */
float r2test90(int nossamples); /* gives r2test value for 90% confidence  - from pp77 Texas Instruments TI-51-III owners manual */
float r2test99(int nossamples); /* gives r2test value for 99% confidence  - from pp77 Texas Instruments TI-51-III owners manual */

#ifdef NoForm1 /* if form1 is not available in this file then these routines are "external" */
void rcls(void); /* clear Results memobox/Richedit box */
void SetCursorToEnd(void); /* scrolls Results RichEdit viewpoint to the end of the text, will continue to scroll so most recently added text is visible */

void rprintf(const char *fmt, ...);    /* like printf but output to Results memobox */
                                /* \n's work as expected and colours can be set(see above)  */
void save_gui(FILE *fp,int VersionMajor,int VersionMinor); /* save GUI values in a way that can be restored by restore_gui() */
void restore_gui(FILE *fp,int VersionMajor,int VersionMinor); /* restore GUI values saved by save_gui() */
#else
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
