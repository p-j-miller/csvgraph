//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
/* expression handling code and other generally useful code for Builder C++ */
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
 * IN NO EVENT SHALL THE AUTHOR OR COPYRIGHT HOLDER BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *--------------------------------------------------------------------------*/
/* expression handling code and other generally useful code for Builder C++
   includes rprintf(char *fmt,...) to print to screen with user selectable colours/bold and rcls() to clear screen.
   & symbol table functions  lookup() & install()
   Note: for rprintf to work there must be a tRichEdit control on the main form called Results
   This should be set with:
         Anchors all true (right & bottom are false by default) (so text box resizes with main form)
         ScrollBars = ssBoth
         Readonly = true
         WordWrap = false   (if set to true long lines will be automatically split which confuses my \n code)
    Expressions (in decreasing priority)
        +/-constants, +/-()
        *,/,%
        +,-
        >>,<<
        <,>,>=,<=
        ==,!=
        &
        ^
        |
        &&
        ||

   (C) Peter Miller Sept 2012 [Note NO company copyright for this code]
   Written assuming Borland builder 5.
   Should compile with Builder 6, the latest versions of builder use w_char's for GUI functions and so would need changes.
   expression handler is pure C code.

Version 0.1 26-9-2012  has basic rprintf and working lookup & install
version 0.2 4/10/12 has working rprintf & lookup & install
version 0.3 5/10/12 many more binary operators added & improved error handling
version 0.4 8/10/12 predefined functions added, as well as ~,!
version 0.5 10/10/12 - can now actually execute rpn to evaluate expressions
version 0.6 11/10/12 - constants (like pi) added.
                        hex numbers (0xdddd) can be entered
                        ~constant evaluated at compile time rather than run time.
                        simple rpn optimiser added (for constant expressions converts to just the constant value ).
                        interface to outside world improved now to_rpn(s) is the only function that takes a string
version 0.7 12/10/12 - added ? operator
                        added assign_expr() to allow assignments of variables to expressions
version 1.0 15/10/12 - expr optimisation made much more agressive (incrementally looks for a constant expression as it compiles rpn)
                        bool getfloat(char *s double *d) added that reads a floating point number (only) returns true if valid
                        simple regular expression matches supported using regex_match();
version 1.1 16/10/12 - added lin_reg(), log_reg() and deriv() [deriv is just dummy !]
version 1.2 19/10/12 - added csv parsing code
version 1.3 22/10/12 - added get_line to read a line from the file with no limits on line length.
                       deriv() added at last. Uses a noise reducing derivative whenever possible.
                       fastexp(x) added. 3% max rel error, and very low errors for small x (|x|<0.17).
version 1.4 23/10/12 - more complex string matching expressions added  - sexpr()
version 1.5 24/10/12 - sexpr now supports "" in matched strings
                     - sexpr now supports ~ "regular expression" (already had ! so no need for a !~ )
version 1.6 26/10/12 - various routines changed to take float[] params ratther than double []                     
version 1.7 29/10/12 - log_lin_reg() added
version 1.8 30/10/12 - log_diff_lin_reg() added
version 1.9 31/10/12 - fastlog() added
version 1.10 1/11/12 - much more accurate (but a bit slower) fastlog()
version 1.11 8/11/12 - stdint.h header created. Much more accurate fastexp(), left less accurate version as fasterexp(), added fasterlog().
version 1.12 10/11/12 - added Application->ProcessMessages() called to rprintf to allow windows to update on this call (and added to monitor function of optimiser)
                        optimiser can be run as a thread if required.
version 2.0  20/11/12 - linear regression changed to only take 1 pass over data.
version 2.1  21/11/12 - linear regression changed to only use means on 1 pass. (note r2 value is unchecked).
version 2.2  22/11/12 - linear regression changed so 1 and 2 pass versions both give identical (correct) results - incl r^2
version 2.3  22/11/12 - define onepassreg allows selection of 1 or 2 pass linear regression on all variants.
                        getfloatgt0() and getfloatge0() added.
version 2.4 22/11/12 - generalised (2 param) least squares added.
version 2.5 24/11/12 - fastsqrt() & fastersqrt() added.
version 2.6 26/11/12 - fastsqrt & fastersqrt made faster (and fastersqrt more accurate).
                       fastinv() & fasterinv() added which can be used to remove divisions from code which may result in a speedup if division is slow (especially if multiply is fast).
version 2.7 26/11/12 - fastersqrt() error reduced.
version 2.8 28/11/12 - fastexp() rewritten base on using polynomial fit to error of fasterexp() - slightly faster (but a bit less accurate)
version 2.9 29/11/12 - fastlog() - tried to speed up using a polynomial in same way as fastexp() - failed - orig code was the best !
version 2.10 30/11/12 - fastlog() - sped up using 2 polymonials (one near 1 and the other elsewhere)- avoids the use of fp divides.
version 2.11 1/12/12 - added t90 and t99 to give t-scores  and r2test90 r2test99 to give corresponding values to check r2 against.
                        fastinv() and fasterinv() improved. fastinv now uses an iteration for 1/x which speeds it up.
version 3.0 19-1-13   - copyright notices/license clarified on all code ("Expat" license used)
                        radarcharts added to gnuplot
version 3.1 31-1-13   - validate_num() and cvalidate_num() added for input validation, fpabs converted to inline.
version 3.2  2-2-13   - min(), max() added to expression evaluator  , added matrix.cpp/matrix.h (to allocate/free 2D matrices )
version 3.3 3-2-13    - save_gui() and restore_gui() added  work for editboxes, radiogroups & TCSpinedits
version 3.4 3-5-13    - added nextafterf() function
version 3.5 2-8-13    - added general purpose hash functions
                      - added SetCursorToEnd()
version 4.0 23-8-13   - changed so that does not need access to Form1 in this code (required so could be moved into "common-files")
*/
// #include <vcl.h> /* for AnsiString etc - MUST come at start of file, followed by a #pragma hdrstop  */
// #define USE_SPINEDIT  /* define to support spinedits in load/save code */

#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <Dialogs.hpp>
#define NoForm1 /* to say we do not have form1 available , needed before including expr-code.h */
#include "expr-code.h"
#include <stdlib.h> /* for malloc() and free() */
#include <string.h> /* for strcmp() etc */
#include <stdarg.h> /* for va_start etc */
#include <stdio.h>  /* for printf etc */

#include <ctype.h> /* isspace etc */
#include <math.h>
#include <float.h> /* _isnan() */
#include <values.h> /* MAXFLOAT etc */
#include "stdint.h" /* for uint32_t etc */
#include "interpolate.h"
#ifdef USE_SPINEDIT
#include <CSPIN.h> /* for spinedits */
#endif
#define  onepassreg /* if defined use functions that only make 1 pass over data for regression, otherwise 2 passes are used . Normally 1 pass is faster and gives same results as 2 pass */

#define STATIC_ASSERT(condition) extern int static_assert_##__FILE__##__LINE__[!!(condition)-1]  /* check at compile time and works in a global or function context see https://scaryreasoner.wordpress.com/2009/02/28/checking-sizeof-at-compile-time/ */

/* for numeric expression code */
static void expr0(void); /* deal with ?: operators */
static void expr1(void); /* lowest priority binary operator */
static void expr2(void);
static void expr3(void);
static void expr4(void);
static void expr5(void);
static void expr6(void);
static void expr7(void);
static void expr8(void);
static void expr9(void);
static void expr10(void); /* highest priority binary operator */
static void fact(void); /* recognises constants, variables, parenthesized expressions */
static double execute_rpn_from(int from);  /* execute rpn created by previous call to to_rpn() from specified start to end & return result */
static void opt_const(int from); /* if rpn from onwards evaluates to a constant then replace it just with a constant */

/* for string matching expressions */
static bool sexpr1(void); /* deal with & */
static bool sexpr2(void); /* deal with & */
static bool sexpr3(void); /* deal with (), $n= "..." or $n != "..." */
static bool scmp(int i); /* $n == "..." */
static char **sexpr_in; /* global variables - csv_parsed input & max nos columns */
static int sexpr_n;
static char *sexpr_cp; /* pointer into input expression string */
static bool sflag; /* true if expression is valid, false if not */

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
 Results->Perform(WM_VSCROLL, SB_BOTTOM,0) ;   // this always seems to work (?)
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
            {AnsiString t;
             if(p[j]=='\n')
                t = Results->Lines->Strings[rline]+p.SubString(i,j-i); /* old line + new without \n*/
             else
                t = Results->Lines->Strings[rline]+p.SubString(i,j+1-i); /* old line + all of new (as no \n)*/
             Results->Lines->Delete(rline);  /* delete old */
             colour_text(Results); /* set text to current colours */
             rline=Results->Lines->Add(t);  /* replace with new */
            }
         else
            {colour_text(Results); /* set text to current colours */
             if(p[j]=='\n')
                 rline=Results->Lines->Add(p.SubString(i,j-i));   /* display this line of result (without \n) */
             else
                rline=Results->Lines->Add(p.SubString(i,j+1-i));   /* display this line of result (with whole string as no \n) */
            }
         lastlinehadCR=(p[j]=='\n'); /* note if CR at end of line */
         i=j+1;  /* character after \n */
        }
    }
   Application->ProcessMessages(); /* allow windows to update (but not go idle) */
   return;
}

/* general purpose hash functions based on Fowler/Noll/Vo hash fnv-1a , with optional "mixing" at the end */
void hash_reset(uint32_t *h) // reset hash h to its initial value
{*h=2166136261;
}

static uint32_t hash_internal(char *s,uint32_t  hash) // returns start_hash "plus" string s
{uint32_t newchar;
 while(s!=NULL && *s!= '\0')
        {newchar=*s++; /* get next character */
         hash^=newchar;
         hash*=16777619;   // "magic number" defined for FNV algorithm
        }
 return hash;
}

void hash_add(char *s, uint32_t *h) // adds string s to hash h, updates h
{*h=hash_internal(s,*h);
}

uint32_t hash_str(char *s) // returns hash of string  , always starting from the same initial value for hash
{uint32_t hash;
 hash_reset(&hash);
 hash=hash_internal(s,hash);
 // this following code is proposed in http://home.comcast.net/~bretm/hash/6.html as a way to ensure even a short string impacts all bits of the hash
 // this is not necessary when we are hashing a whole file, but could be useful when hashing for a "symbol table" or similar.
 hash += hash<<13;
 hash ^= hash >>7;
 hash += hash <<3;
 hash ^= hash >>17;
 hash += hash <<5;
 return hash;
}

/* table lookup code based on K&R The C programming language (1st Ed) section 6.6 */

#define HASHSIZE 128   /* number of different hash values - this does not restrict the number of items that can be stored */
                        /* ideally this is a power of 2, if its not the last entry in the hashtab may have less items in it than the others (assuming "uniform" distribution of keys) */
static pnlist hashtab[HASHSIZE]; /* array pointers to nlist structures (which are allocated using malloc() )*/

#if 1
static int hash(char *s)
    {
     return hash_str(s) % HASHSIZE;
    }
#else
/* very simple hash */
static int hash(char *s)
    {int hashval;
     for(hashval=0;*s!= '\0';)
        {hashval+= *s++; /* simplest possible hash - just add up characters - not critical to performance so left simple */
         if(hashval<0) hashval = -hashval; /* trap "overflow" to ensure hashval is always positive [needed for % later] */
        }
     return hashval % HASHSIZE;
    }
#endif

pnlist lookup(char *s) /* lookup s , returns pointer to struct snlist or NULL if not found */
    {pnlist np;
     for(np=hashtab[hash(s)];np!=NULL;np=np->next)
        {if (strcmp(s,np->name)==0)
             return np; /* found it */
        }
     return NULL; /* not found */
    }

char *strsave(char *s)  /* save a copy of string s using malloc , returns NULL if no space */
    {char *p;
     if((p=(char *)malloc(strlen(s)+1)) != NULL)
        {strcpy(p,s); /* got space, copy string */
        }
     return p;
    }

pnlist install(char *name) /* adds name to hashtab if it does not exist, returns pointer to entry created or NULL on error */
    {pnlist np;
     int hashval;
     if((np=lookup(name))==NULL)
        { /* not found , create new entry */
         np=(pnlist) malloc(sizeof(*np));
         if(np==NULL) return NULL;/* out space */
         if((np->name=strsave(name))==NULL)
            return NULL; /* not enough space for "string" name */
         np->value=0.0; /* default value */   
         hashval=hash(np->name);
         np->next=hashtab[hashval]; /* assumes hashtab initialised to NULL */
         hashtab[hashval]=np; /* insert at start of linked list of items from this hashvalue */
        }
     /* else already found */
     return np;
    }

/* expression evaluator code */
/* recursive descent evaluation used */

#define MAXRPN 1000 /* max bytes in rpn generated */
bool flag; /* used in recursive descent parser is true if expression was OK */
unsigned int nos_vars; /* number of variables in expression [0 means expression will evaluate to a constant] */

static char *e; /* expression as a string */
static unsigned char rpn[MAXRPN];
static int rpnptr; /* index into rpn[] next token will be placed */
typedef enum _token {LOR,LAND,OR,XOR,AND,EQ,NEQ,LESS,GT,LE,GE,SHIFTR,SHIFTL,ADD,SUB,MULT,DIV,MOD,MINUS,LNOT,NOT,CONSTANT,VARIABLE,
                     ACOS,ASIN,ATAN,SIN,COS,TAN,LOG,EXP,SQRT,POW,COSH,SINH,TANH,FABS,QN,MAX,MIN} token;
#pragma option -w-pin /* ignore W8-61 Initialisation is only partially bracketed that would be created by struct functions below */
struct functions       /* list of predefined functions/constants - must be in alphabetic order */
        { const char *keyword;
          int nosargs; /* number of arguments, eg pow(x,y) has 2 , -1 means its a constant (and brackets not then needed)*/
          token tk; /* corresponding token */
          double value; /* value if its a constant [by convention set to 0 if its not a constant]*/
        } funtab[]=
        { "abs",1,FABS,0,
          "acos",1,ACOS,0,
          "asin",1,ASIN,0,
          "atan",1,ATAN,0,
          "cos",1,COS,0,
          "cosh",1,COSH,0,
          "exp",1,EXP,0,
          "log",1,LOG,0,
          "max",2,MAX,0,
          "min",2,MIN,0,
          "pi",-1,CONSTANT, M_PI,
          "pow",2,POW,0,
          "sin",1,SIN,0,
          "sinh",1,SINH,0,
          "sqrt",1,SQRT,0,
          "tan",1,TAN,0,
          "tanh",1,TANH,0
         };

bool check_function_tab()
{ /* checks above table is in alphabetic order - returns true if it is, false if not */
 int i;
 int n= sizeof(funtab)/sizeof(struct functions); /* nos elements in above table */
 int errs=0;
 for(i=1;i<n;++i)
        {if(strcmp(funtab[i-1].keyword,funtab[i].keyword)>=0)
                errs++;
        }
 // rprintf("check_function() %d functions in table, %d errors found\n",n,errs);
 return errs==0;
}

static int lookup_fun(char *name)
{/* returns index into array, -1 if not found . uses binary search */
 int low,high,mid; // must be signed to work !
 int cond;
 int n=sizeof(funtab)/sizeof(struct functions); /* size of table */
 low=0;
 high=n-1;
 while (low<=high)
        {mid=(low+high)>>1;
         if((cond=strcmp(name,funtab[mid].keyword))<0)
                high=mid-1;
         else if (cond > 0)
                low=mid+1;
         else return mid; /* found */
        }
 return -1; /* not found */
}

static void rpn_init(void) /* initialise ready for rpn creation */
{ rpnptr=0;
 flag=true;
 nos_vars=0;
}

static void addrpn_op(token op) /* add operator to rpn */
{if(rpnptr<MAXRPN)
        {rpn[rpnptr++]=(unsigned char)op;
        }
}

static void addrpn_c(unsigned char c) /* add 1 byte to rpn */
{if(rpnptr<MAXRPN)
        {rpn[rpnptr++]=c;
        }
}

static void addrpn_constant(double v) /* add double constant to rpn */
{union u_tag {
 unsigned char c[sizeof(double)];
 double d;
 } u;
 int i;
 addrpn_op(CONSTANT);
 u.d=v;
 for(i=0;i<sizeof(double);++i) /* write out all 8 bytes of value */
        {
         addrpn_c(u.c[i]);
        }
}

static void addrpn_variable(pnlist v) /* adds variable to rpn */
{union u_tag {
 unsigned char c[sizeof(pnlist)];
 pnlist p;
 } u;
 int i;
 addrpn_op(VARIABLE);
 u.p=v;
 for(i=0;i<sizeof(pnlist);++i) /* write out all 4 bytes of value */
        {
         addrpn_c(u.c[i]);
        }
}

bool last_expression_ok() /* returns true if last expression was correct in syntax , false otherwise */
{ return flag;
}

void to_rpn(char *expression)
{ /* convert expression to rpn, expression just returns a value and contains no assignments*/
 e=expression;
 rpn_init();
 expr0();
 if(*e!='\0') flag=false; /* expression was valid - but did not finish at the end of the string */
 if(rpnptr>=MAXRPN) flag=false; /* expression too complex [rpn too long] */
}

void optimise_rpn()
/* optimise rpn generated by prior call to to_rpn() */
{
#if 1
 /* optimisation is part of parser, this is just the final step... */
 opt_const(0); /* if whole expression evaluates to a constant, then just make expression a single constant */
#else
 /* for now just do simplest thing of optimising expression if its completely a constant */
 if(flag && nos_vars==0)
        {/* whole expression is valid & a constant - evaluate it then just make result a constant */
         double d=execute_rpn();
         if(flag)
                {/* if executed OK */
                 rpn_init(); /* restore rpn */
                 addrpn_constant(d); /* make it just a constant */
                }
         }
#endif
}

void assign_expr(char *expression) /* deal with assignments , evaluates expression and stores result in variable ; allows multiple assignments on one line */
{char *start,lastc; /* start of variable name, and character that terminates name */
 pnlist v;/* variable */
 e=expression;
 while(1)
  {rpn_init(); /* rpn is only for an expression - so need to restart after every ; */
   while(isspace(*e))++e; /* skip whitespace */
   if(isalpha(*e) || *e=='_')
        {/* variable must start with a letter or an underline */
         start=e;
         while(isalnum(*e) || *e=='_') /* variables can include letters, digits, _ */
                ++e;
         lastc= *e; /* termination - 1st char not part of variable name */
         *e='\0'; /* make a variable a null terminated string for now */
         v=install(start); /* lookup in symbol table, will create new entry if required */
         *e=lastc; /* restore input string back to what it was */
         if(v==NULL)
                {/* error - no space ? */
                 flag=false;
                 return;
                }
         /* if we get here have a valid variable "in" v */
        }
   else
        {/* no variable name found */
         flag=false;
         return;
        }
   while(isspace(*e))++e; /* skip whitespace */
   if(*e!='=')
        {flag=false; /* = must come after variable name */
         return;
        }
   ++e; /* skip = */
   expr0(); /* get expression */
   if(!(*e=='\0' || *e==';')) flag=false; /* expression was valid - but did not finish at the end of the string or ; */
   if(rpnptr>=MAXRPN) flag=false; /* expression too complex [rpn too long] */
   if(flag) /* expression valid */
        {double d=execute_rpn(); /* execute expression */
         if(flag)
                {/* if executed OK */
                 v->value=d; /* set variables value to the result of the expression */
                }
         }
   if(!flag) return;/* some error - done */
   if(*e==';')
        {++e;
         continue; /* ; so lets do it all again */
        }
   else return; /* done */
  } /* end of while(1) */
}

static void expr0(void) /* deal with ?: operators */
                        /* note if we really wanted to we could optimise a constant before the ? to only execute one of the 2 other expressions - this is NOT done here */
{int start_rpnptr=rpnptr; /* allows us to check if the result of this is a constant */
 expr1();
 if(!flag) return;
 while(isspace(*e))++e; /* skip whitespace */
 while(*e=='?')
        {++e; /* pass over ? */
         expr0(); /* expression value if true - need to cal expr0 here as 1==2? 3==4?1:0 :1 is valid as is 1==2?2:3 ? 1: 0 )2nd done by while loop here */
         if(!flag) return;
         while(isspace(*e))++e; /* skip whitespace */
         if(*e!=':')
                {/* error : expected */
                 flag=false;
                 return;
                }
          ++e; /* pass over : */
          expr0(); /* expression value if fals */
          if(!flag) return;
          while(isspace(*e))++e; /* skip whitespace */
          addrpn_op(QN);
          opt_const(start_rpnptr); /* see if we can optimise this all as a constant */
        }
}

static void expr1(void) /* lowest priority binary operator  || */
{int start_rpnptr=rpnptr; /* allows us to check if the result of this is a constant */
 expr2();
 if(!flag) return;
 while(isspace(*e))++e; /* skip whitespace */
 while(flag)
        {
         if(*e=='|' && *(e+1)=='|')
                {e+=2;
                 expr2();
                 if(!flag) return;
                 addrpn_op(LOR);
                 opt_const(start_rpnptr); /* see if we can optimise this all as a constant */
                }
         else flag=false;
        }
 flag=true;
}

static void expr2(void) /* binary operator && */
{int start_rpnptr=rpnptr; /* allows us to check if the result of this is a constant */
 expr3();
 if(!flag) return;
 while(isspace(*e))++e; /* skip whitespace */
 while(flag)
        {if(*e=='&' && *(e+1)=='&')
                {e+=2;
                 expr3();
                 if(!flag) return;
                 addrpn_op(LAND);
                 opt_const(start_rpnptr); /* see if we can optimise this all as a constant */
                }
         else flag=false;
        }
 flag=true;
}

static void expr3(void) /* binary operator | */
{int start_rpnptr=rpnptr; /* allows us to check if the result of this is a constant */
 expr4();
 if(!flag) return;
 while(isspace(*e))++e; /* skip whitespace */
 while(flag)
        {if(*e=='|' && *(e+1)!='|') /* | and not || */
                {++e;
                 expr4();
                 if(!flag) return;
                 addrpn_op(OR);
                 opt_const(start_rpnptr); /* see if we can optimise this all as a constant */
                }
         else flag=false;
        }
 flag=true;
}

static void expr4(void) /* binary operator ^ */
{int start_rpnptr=rpnptr; /* allows us to check if the result of this is a constant */
 expr5();
 if(!flag) return;
 while(isspace(*e))++e; /* skip whitespace */
 while(flag)
        {if(*e=='^')
                {++e;
                 expr5();
                 if(!flag) return;
                 addrpn_op(XOR);
                 opt_const(start_rpnptr); /* see if we can optimise this all as a constant */
                }
         else flag=false;
        }
 flag=true;
}

static void expr5(void) /* binary operator & */
{int start_rpnptr=rpnptr; /* allows us to check if the result of this is a constant */
 expr6();
 if(!flag) return;
 while(isspace(*e))++e; /* skip whitespace */
 while(flag)
        {if(*e=='&' && *(e+1)!='&') /* & and not && */
                {++e;
                 expr6();
                 if(!flag) return;
                 addrpn_op(AND);
                 opt_const(start_rpnptr); /* see if we can optimise this all as a constant */
                }
         else flag=false;
        }
 flag=true;
}

static void expr6(void) /* binary operators ==, !=*/
{int start_rpnptr=rpnptr; /* allows us to check if the result of this is a constant */
 expr7();
 if(!flag) return;
 while(isspace(*e))++e; /* skip whitespace */
 while(flag)
        {if(*e=='=' && *(e+1)=='=')
                {e+=2;
                 expr7();
                 if(!flag) return;
                 addrpn_op(EQ);
                 opt_const(start_rpnptr); /* see if we can optimise this all as a constant */
                }
         else if(*e=='!' && *(e+1)=='=')
                {e+=2;
                 expr7();
                 if(!flag) return;
                 addrpn_op(NEQ);
                 opt_const(start_rpnptr); /* see if we can optimise this all as a constant */
                }
         else flag=false;
        }
 flag=true;
}

static void expr7(void) /* binary operators <,>,<=,>= */
{int start_rpnptr=rpnptr; /* allows us to check if the result of this is a constant */
 expr8();
 if(!flag) return;
 while(isspace(*e))++e; /* skip whitespace */
 while(flag)
        {if(*e=='<' && *(e+1)=='=') /* must check 2 character tokens before 1 character ! */
                {e+=2;
                 expr8();
                 if(!flag) return;
                 addrpn_op(LE);
                 opt_const(start_rpnptr); /* see if we can optimise this all as a constant */
                }
         else if(*e=='>' && *(e+1)=='=')
                {e+=2;
                 expr8();
                 if(!flag) return;
                 addrpn_op(GE);
                 opt_const(start_rpnptr); /* see if we can optimise this all as a constant */
                }
        else if(*e=='<')
                {++e;
                 expr8();
                 if(!flag) return;
                 addrpn_op(LESS);
                 opt_const(start_rpnptr); /* see if we can optimise this all as a constant */
                }
         else if(*e=='>')
                {++e;
                 expr8();
                 if(!flag) return;
                 addrpn_op(GT);
                 opt_const(start_rpnptr); /* see if we can optimise this all as a constant */
                }
         else flag=false;
        }
 flag=true;
}

static void expr8(void) /* binary operators >>,<< */
{int start_rpnptr=rpnptr; /* allows us to check if the result of this is a constant */
 expr9();
 if(!flag) return;
 while(isspace(*e))++e; /* skip whitespace */
 while(flag)
        {if(*e=='>' && *(e+1)=='>')
                {e+=2;
                 expr9();
                 if(!flag) return;
                 addrpn_op(SHIFTR);
                 opt_const(start_rpnptr); /* see if we can optimise this all as a constant */
                }
         else if(*e=='<' && *(e+1)=='<')
                {e+=2;
                 expr9();
                 if(!flag) return;
                 addrpn_op(SHIFTL);
                 opt_const(start_rpnptr); /* see if we can optimise this all as a constant */
                }
         else flag=false;
        }
 flag=true;
}

static void expr9(void) /* binary operators +,- */
{int start_rpnptr=rpnptr; /* allows us to check if the result of this is a constant */
 expr10();
 if(!flag) return;
 while(isspace(*e))++e; /* skip whitespace */
 while(flag)
        {if(*e=='+')
                {++e;
                 expr10();
                 if(!flag) return;
                 addrpn_op(ADD);
                 opt_const(start_rpnptr); /* see if we can optimise this all as a constant */
                }
         else if(*e=='-')
                {++e;
                 expr10();
                 if(!flag) return;
                 addrpn_op(SUB);
                 opt_const(start_rpnptr); /* see if we can optimise this all as a constant */
                }
         else flag=false;
        }
 flag=true;
}

static void expr10(void) /* highest  priority binary operators, *,/,% */
{int start_rpnptr=rpnptr; /* allows us to check if the result of this is a constant */
 fact();
 if(!flag) return;
 while(isspace(*e))++e; /* skip whitespace */
 while(flag)
        {if(*e=='*')
                {++e;
                 fact();
                 if(!flag) return;
                 addrpn_op(MULT);
                 opt_const(start_rpnptr); /* see if we can optimise this all as a constant */
                }
         else if(*e=='/')
                {++e;
                 fact();
                 if(!flag) return;
                 addrpn_op(DIV);
                 opt_const(start_rpnptr); /* see if we can optimise this all as a constant */
                }
         else if(*e=='%')
                {++e;
                 fact();
                 if(!flag) return;
                 addrpn_op(MOD);
                 opt_const(start_rpnptr); /* see if we can optimise this all as a constant */
                }
         else flag=false;
        }
 flag=true;
}

static void fact(void) /* recognises constants, variables, (predefined) functions, parenthesized expressions */
{bool isneg=false;
 bool islognot=false; /* ! */
 bool isnot=false;    /* ~ */
 int start_rpnptr=rpnptr; /* allows us to check if the result of this is a constant */
 while(isspace(*e))++e; /* skip whitespace */
 /* note the C standard appears to only allow one monadic operator (-,+,!,~) so thats whats implemented here */
 if(*e=='-')
        {/* monadic minus */
         ++e;
         while(isspace(*e))++e; /* skip whitespace */
         isneg=true;
        }
  else if(*e=='+')
        {/* monadic plus [allowed in C89 and seems reasonable as "+1" is a sensible expression */
         ++e;
         while(isspace(*e))++e; /* skip whitespace */
        }
 else if(*e=='!')
        {/* monadic logical NOT  */
         ++e;
         while(isspace(*e))++e; /* skip whitespace */
         islognot=true;
        }
 else if(*e=='~')
        {/* monadic bitwise NOT */
         ++e;
         while(isspace(*e))++e; /* skip whitespace */
         isnot=true;
        }
#ifdef Allow_dollar_vars_in_expr
 /* new code allows variables named $1.. $999... like awk - these can only be read from (they are assumed to be automatically set eg from a column of a csv file) */
 if(isalpha(*e) || *e=='_' || *e=='$')
        {/* variable/function/constant (eg pi) must start with a letter or an underline or $ */
         char *start=e,lastc;
         pnlist v;
         int i;
         if(*e=='$')
                {++e; /* skip $  */
                 if(!isdigit(*e) || *e=='0')
                        {
                         flag=false; /* $ must be followed by an integer 1 or more [0 is not allowed] */
                         return;
                        }
                 while(isdigit(*e))
                        ++e; /* can be followed by any number of digits  */
                }
         else
                {/* "standard variable" (does not start with $ */
                 while(isalnum(*e) || *e=='_') /* variables can include letters, digits, _ */
                        ++e;
                }
#else
 /* original code that does not allow $xxx variable names */
 if(isalpha(*e) || *e=='_')
        {/* variable/function/constant (eg pi) must start with a letter or an underline */
         char *start=e,lastc;
         pnlist v;
         int i;
         while(isalnum(*e) || *e=='_') /* variables can include letters, digits, _ */
                ++e;
#endif
         lastc= *e; /* termination - 1st char not part of variable name */
         *e='\0'; /* make a variable a null terminated string for now */
         // rprintf("before lookup_fun(%s)\n",start);
         i=lookup_fun(start);
         // rprintf("lookup_fun(%s) returns %d\n",start,i);
         if(i>=0 && funtab[i].nosargs == -1)
                { /* constant found (like "pi" ) */
                 double d=funtab[i].value;
                 *e=lastc; /* restore input string back to what it was */
                 addrpn_constant(d);
                }
         else if (i>=0)
                {/* valid function name found */
                 int nosargs=funtab[i].nosargs;
                 *e=lastc; /* restore input string back to what it was */
                 while(isspace(*e))++e; /* skip whitespace */
                 if(*e!='(')
                        {flag=false; /* function must be followed by (  */
                         return;
                        }
                 ++e; /* skip over ( , then get a list of comma seperated arguments */
                 while(nosargs-- >0)
                        {expr1(); /* get an expression */
                         if(!flag) return; /* error */
                         if(nosargs>0)
                                {/* more arguments to come, need a comma now */
                                  while(isspace(*e))++e; /* skip whitespace */
                                  if(*e!=',')
                                        {flag=false;
                                         return;
                                        }
                                   ++e; /* skip over , */
                                  }
                         }
                 while(isspace(*e))++e; /* skip whitespace */
                 if(*e!=')')
                        {flag=false; /* no trailing ) in function call */
                         return;
                        }
                  ++e; /* skip over ) */
                  addrpn_op(funtab[i].tk); /* RPN to actually execute function */
                }
         else
                {/* try to see it its a variable */
#ifdef Allow_dollar_vars_in_expr
                 if(*start=='$')
                   v=install(start); /* lookup variable in symbol table - adding it if required as $nnn "variables" are predefined */
                 else
#endif
                   v=lookup(start);   /* lookup in symbol table */
                 *e=lastc; /* restore input string back to what it was */
                 if(v==NULL)
                        {/* error - variable not defined */
                         flag=false;
                         return;
                        }
                 else
                        {
                         addrpn_variable(v);
                         ++nos_vars; /* keep count of number of variables in expression */
                        }
                }
        }
 else if(isdigit(*e)|| *e=='.')
        { /* constant must start with a number or a decimal point */
          /* hex numbers starting 0x or 0X are supported */
         double d;
         if(*e=='0' && (*(e+1)=='x' || *(e+1)=='X'))
                d=strtol(e,&e,0);     /* 0x=> hex number - note bitwise operators assume 32bit unsigned ints but this allows entry of larger numbers [which may not be exactly represented as a double] */
          else  d=strtod(e,&e);       /* floating point number */
         /*rprintf("constant %g\n",d);*/
         addrpn_constant(d);
        }
 else if(*e=='(')
        { /* bracketed expression */
         ++e;
         expr1();
         if(!flag) return;
         while(isspace(*e))++e; /* skip whitespace */
         if(*e==')')
                {++e; /* flag already true because of if(!flag) above so no need to set it again here */
                }
         else flag=false; /* error missing ")" */
        }
 else
        {flag=false; /* error */
        }
 if(flag)
  {/* if no errors so far, then deal with monadic operators here , note if these apply to a constant they will be optimised out by call to opt_const() below */
   if(isneg) addrpn_op(MINUS);
   if(islognot) addrpn_op(LNOT);
   if(isnot) addrpn_op(NOT);
  }
 opt_const(start_rpnptr); /* see if we can optimise this all away to a single constant */
}

void print_rpn()  /* print rpn for debugging */
{int i,j;
 union u_tagc {
 unsigned char c[sizeof(double)];
 double d;
 } uconst;
 union u_tagv {
 unsigned char c[sizeof(pnlist)];
 pnlist p;
 } uvar;
 if(flag)
        rprintf("Flag=true.  rpn=");
 else
        rprintf("Flag=false. rpn=");
 for(i=0;i<rpnptr;++i)
        {switch((token)(rpn[i]))
               {
                case LOR: rprintf("<LOR>"); break;
                case LAND: rprintf("<LAND>"); break;
                case OR: rprintf("<OR>"); break;
                case XOR: rprintf("<XOR>"); break;
                case AND: rprintf("<AND>"); break;
                case EQ: rprintf("<EQ>"); break;
                case NEQ: rprintf("<NEQ>"); break;
                case LESS: rprintf("<LESS>"); break;
                case GT: rprintf("<GT>"); break;
                case LE: rprintf("<LE>"); break;
                case GE: rprintf("<GE>"); break;
                case SHIFTR: rprintf("<SHIFTR>"); break;
                case SHIFTL: rprintf("<SHIFTL>"); break;
                case ADD: rprintf("<ADD>"); break;
                case SUB: rprintf("<SUB>"); break;
                case MULT: rprintf("<MULT>"); break;
                case DIV: rprintf("<DIV>"); break;
                case MOD: rprintf("<MOD>"); break;
                case MINUS: rprintf("<MINUS>"); break;
                case LNOT:  rprintf("<LNOT>"); break;
                case NOT:   rprintf("<NOT>"); break;
                /* functions */
                case ACOS:   rprintf("<ACOS>"); break;
                case ASIN:   rprintf("<ASIN>"); break;
                case ATAN:   rprintf("<ATAN>"); break;
                case SIN:   rprintf("<SIN>"); break;
                case COS:   rprintf("<COS>"); break;
                case TAN:   rprintf("<TAN>"); break;
                case LOG:   rprintf("<LOG>"); break;
                case EXP:   rprintf("<EXP>"); break;
                case SQRT:   rprintf("<SQRT>"); break;
                case POW:   rprintf("<POW>"); break;
                case COSH:   rprintf("<COSH>"); break;
                case SINH:   rprintf("<SINH>"); break;
                case TANH:   rprintf("<TANH>"); break;
                case FABS:   rprintf("<ABS>"); break;
                case MAX:    rprintf("<MAX>"); break;
                case MIN:    rprintf("<MIN>"); break;

                case QN: rprintf("<QN>"); break;

                case CONSTANT:  rprintf("<CONSTANT=");
                                for(j=0;j<sizeof(double);++j)
                                        {
                                         rprintf("%02x",rpn[++i]);
                                         uconst.c[j]=rpn[i];
                                        }
                                rprintf(":%g>",uconst.d);
                         break;
                case VARIABLE:  rprintf("<VARIABLE=");
                                for(j=0;j<sizeof(pnlist);++j)
                                        {
                                         rprintf("%02x",rpn[++i]);
                                         uvar.c[j]=rpn[i];
                                        }
                                rprintf(":%s:%g>",uvar.p->name,uvar.p->value);
                        break;
                default:  rprintf("<??%02x>",rpn[i]); break;
               }
        }
}

#define MAXSTACK 30 /* maxsize of stack */
static double stack[MAXSTACK];
static unsigned int sp;
static bool rpn_constant; /* used to show if rpn expression is a constant */

double execute_rpn()  /* execute rpn created by previous call to to_rpn() & return result */
{return execute_rpn_from(0);
}

void opt_const(int from) /* if rpn from specified location to end is a constant then just replace it with a constant */
{bool t_flag=flag; /* save a copy of flag and restore it at the end */
 double d=execute_rpn_from(from);
 if(flag & rpn_constant)
        {rpnptr=from; /* can delete entire expression and replace with a constant */
         addrpn_constant(d);
        }
 flag=t_flag;
}

static double execute_rpn_from(int from)  /* execute rpn created by previous call to to_rpn() from specified start to end & return result */
{int i,j;
 union u_tagc {
 unsigned char c[sizeof(double)];
 double d;
 } uconst;
 union u_tagv {
 unsigned char c[sizeof(pnlist)];
 pnlist p;
 } uvar;

 if(!flag)
        {/* expression error - return 0 */
         return 0.0;
        }
 rpn_constant=true; /* assume rpn is a constant, set to false if any variables found in rpn */
 sp=0;
 /* valid expression - execute it */
 for(i=from;i<rpnptr;++i)
        {switch((token)(rpn[i]))
               {/* note 1 - 2 => 1 2 - so we need [sp-2]=[sp-2] OP [sp-1]  */
                case LOR: stack[sp-2]=stack[sp-2] || stack[sp-1]; sp-=1; break;
                case LAND: stack[sp-2]=stack[sp-2] && stack[sp-1]; sp-=1; break;
                        /* bitwise operators work on uisigned ints (32 bits) which can be exactly represented as doubles */
                case OR: stack[sp-2]= (unsigned int)(stack[sp-2]) | (unsigned int)(stack[sp-1]); sp-=1; break;
                case XOR: stack[sp-2]= (unsigned int)(stack[sp-2]) ^ (unsigned int)(stack[sp-1]); sp-=1; break;
                case AND: stack[sp-2]= (unsigned int)(stack[sp-2]) & (unsigned int)(stack[sp-1]); sp-=1; break;
                case EQ: stack[sp-2]=stack[sp-2] == stack[sp-1]; sp-=1; break;
                case NEQ: stack[sp-2]=stack[sp-2] != stack[sp-1]; sp-=1; break;
                case LESS: stack[sp-2]=stack[sp-2] < stack[sp-1]; sp-=1; break;
                case GT: stack[sp-2]=stack[sp-2] > stack[sp-1]; sp-=1; break;
                case LE: stack[sp-2]=stack[sp-2] <= stack[sp-1]; sp-=1; break;
                case GE: stack[sp-2]=stack[sp-2] >= stack[sp-1]; sp-=1; break;
                case SHIFTR: stack[sp-2]= (unsigned int)(stack[sp-2]) >> (unsigned int)(stack[sp-1]); sp-=1; break;
                case SHIFTL: stack[sp-2]= (unsigned int)(stack[sp-2]) << (unsigned int)(stack[sp-1]); sp-=1; break;
                case ADD: stack[sp-2]+=stack[sp-1]; sp-=1; break;
                case SUB: stack[sp-2]-=stack[sp-1]; sp-=1; break;
                case MULT: stack[sp-2]*=stack[sp-1]; sp-=1; break;
                case DIV: if(stack[sp-1]!=0) stack[sp-2]/=stack[sp-1];
                           else if(stack[sp-2]!=0) stack[sp-2]=MAXFLOAT; // x/0 = big  - cannot use MAXDOUBLE as use floats for values elsewhere
                           else stack[sp-2]=0; // 0/0 = 0 here (should perhaps be NAN)
                          sp-=1; break;
                case MOD: stack[sp-2]=fmod(stack[sp-2],stack[sp-1]); sp-=1; break; // mod(?,0) is defined to be 0 by fmod so no special cases need to be trapped
                case MINUS: stack[sp-1] = -stack[sp-1]; break;
                case LNOT:  stack[sp-1] = (stack[sp-1])?0.0:1.0 ; break;
				case NOT:   stack[sp-1]= ~(unsigned int)(stack[sp-1]); break;
                /* functions */
                case ACOS:   stack[sp-1] = acos(stack[sp-1]); break;
                case ASIN:   stack[sp-1] = asin(stack[sp-1]); break;
                case ATAN:   stack[sp-1] = atan(stack[sp-1]); break;
                case SIN:   stack[sp-1] = sin(stack[sp-1]); break;
                case COS:   stack[sp-1] = cos(stack[sp-1]); break;
                case TAN:   stack[sp-1] = tan(stack[sp-1]); break;
                case LOG:   stack[sp-1] = stack[sp-1]>0?log(stack[sp-1]):0.0; break; // trap -ve arg to log
                case EXP:   stack[sp-1] = exp(stack[sp-1]); break;
                case SQRT:   stack[sp-1] = stack[sp-1]>0?sqrt(stack[sp-1]):0.0; break; // trap -ve arg to sqrt
                case POW:   stack[sp-2] = pow(stack[sp-2],stack[sp-1]); sp-=1; break;
                case COSH:   stack[sp-1] = cosh(stack[sp-1]); break;
                case SINH:   stack[sp-1] = sinh(stack[sp-1]); break;
                case TANH:   stack[sp-1] = tanh(stack[sp-1]); break;
                case FABS:   stack[sp-1] = fabs(stack[sp-1]); break;
                case MAX:   stack[sp-2] = max(stack[sp-2],stack[sp-1]); sp-=1; break;
                case MIN:   stack[sp-2] = min(stack[sp-2],stack[sp-1]); sp-=1; break;
                /* "special" */
                case QN: /* ?: */ stack[sp-3]= stack[sp-3] ? stack[sp-2] : stack[sp-1]; sp-=2; break;

                case CONSTANT:  if(sp>=MAXSTACK-1)
                                        {flag=false; /* stack overflow */
                                         return 0.0;
                                        }
                                for(j=0;j<sizeof(double);++j)
                                        uconst.c[j]=rpn[++i];
                                stack[sp++]=uconst.d;
                         break;
                case VARIABLE: if(sp>=MAXSTACK-1)
                                        {flag=false; /* stack overflow */
                                         return 0.0;
                                        }
                                for(j=0;j<sizeof(pnlist);++j)
                                        uvar.c[j]=rpn[++i];
#ifdef Allow_dollar_vars_in_expr
                                if(uvar.p->name[0]=='$')
                                        stack[sp++]=get_dollar_var_value(uvar.p); /* get varibel of $ variable */
                                else
#endif
                                        stack[sp++]=uvar.p->value; /* value of "normal variable" */
                                rpn_constant=false; /* note rpn contains at least one variable so is not a constant */
                        break;
                /* no need for default as we covered all cases - if fact compiler gives warning if we add one */
               }
        }
  if(sp!=1)
        {/* stack should just have 1 item on it at the end - the value of the expression */
         flag=false;
         return 0.0;
        }
  return stack[sp-1]; /* result is on the top of the stack */
 }


bool getfloat(char *s, float *d)
 /*reads a floating point number returns true if valid - allows whitespace as well as a number , d=0 on error */
{double r;
 bool valid=false;
 while(isspace(*s))++s; /* skip whitespace */
 if(isdigit(*s)|| *s=='.' || *s=='-' || *s=='+')
        {/* number must start with decimal point or a digit or a sign (+/-)*/
         r=strtod(s,&s);       /* read floating point number */
         while(isspace(*s))++s; /* skip whitespace */
         if(*s=='\0') valid=true; /* whitespace number whitespace */
        }
 if(!valid || r>MAXFLOAT || r< -MAXFLOAT )
        {*d=0.0;
         return false; /* not valid or too big */
        }
 /* valid */
 *d=r;
 return true;
}

bool getfloatgt0(char *s, float *d)
 /*reads a floating point number thats >0 returns true if valid - allows whitespace as well as a number , d=0 on error */
{double r;
 bool valid=false;
 while(isspace(*s))++s; /* skip whitespace */
 if(isdigit(*s)|| *s=='.' || *s=='+')
        {/* number must start with decimal point or a digit or a + sign */
         r=strtod(s,&s);       /* read floating point number */
         while(isspace(*s))++s; /* skip whitespace */
         if(*s=='\0') valid=true; /* whitespace number whitespace */
        }
 if(!valid || r>MAXFLOAT || r<=0 )
        {*d=0.0;
         return false; /* not valid or too big, or <=0 */
        }
 /* valid */
 *d=r;
 return true;
}

bool getfloatge0(char *s, float *d)
 /*reads a floating point number thats >=0 returns true if valid - allows whitespace as well as a number , d=0 on error */
{double r;
 bool valid=false;
 while(isspace(*s))++s; /* skip whitespace */
 if(isdigit(*s)|| *s=='.' || *s=='+')
        {/* number must start with decimal point or a digit or a + sign */
         r=strtod(s,&s);       /* read floating point number */
         while(isspace(*s))++s; /* skip whitespace */
         if(*s=='\0') valid=true; /* whitespace number whitespace */
        }
 if(!valid || r>MAXFLOAT || r<0 )
        {*d=0.0;
         return false; /* not valid or too big, or <0 */
        }
 /* valid */
 *d=r;
 return true;
}

/* functions to deal with time input */
#if 1
/* this is a faster version that tries to avoid floating point maths, this reduced load time of 2 columns from 67 secs to 51 secs */
double gethms(char *s)
{/* read a time of the format hh:mm:ss.s , returns it as a double value in seconds */
 /* if just a number is found this will be treated as seconds (which can include a decimal point and digits after the dp)
	if aa:bb is found this will be treated as aa mins and bb secs  (which can include a decimal point and digits after the dp)
	if aa:bb:cc is found this will be treated as aa hours, bb mins and cc secs (which can include a decimal point and digits after the dp)
	returns 0 if does not start with a number, otherwise converts as much as possible based on the above format
	This means in particular that trailing whitespace and "'s are ignored
 */
 uint32_t sec=0,sec1=0;  // sec1 is current set of digits, sec is previous total
 uint32_t pow10=1;
 if(!isdigit(*s)) return 0; /* must start with a number */
 while(isdigit(*s))
	{sec1=sec1*10+(uint32_t)(*s-'0'); // ascii->decimal
	++s;
	if(*s==':' && isdigit(s[1]))
		{sec=(sec+sec1)*60; // previous must have been minutes (or hours)  so multiply by 60 to get secs [ or mins]
		 sec1=0; // ready to get next set of digits
		 ++s; // skip :
		}
	}
 if(*s=='.')
	{ // seconds contains dp , so we now need to keep track of dp and watch out for uint32 overflowing - we have at most 59 secs in sec1 so we have plenty of resolution
	 ++s; // skip dp
	 while(isdigit(*s) && (sec1&0xf0000000) == 0 && (pow10&0xf0000000) == 0  )
		{sec1=sec1*10+(uint32_t)(*s-'0');
		 pow10*=10; // keep track of decimal point position
		 ++s;
		}
	 if(isdigit(*s) && *s>='5') sec1++; // round if more digits present
	 return (double)sec+(double)sec1/(double)pow10;
	}
 return (double)(sec+sec1);  // if seconds is an integer
}

static unsigned int days=0;
static double last_time_secs=0;
static bool skip=false; // skip 1st number in a big step
void reset_days(void)  /* reset static variables for gethms_days() - should be used before using gethms_days() to read times from a file  */
{days=0;
 last_time_secs=0;
 skip=true; // believe 1st value
}

double gethms_days(char *s) /* read time in format hh:mm:ss.s , assumed to be called in sequence and accounts for days when time wraps around. Returns secs, or -ve number on error */
{double t;
 if(!isdigit(*s)) return -1; /* should start with a number, return -1 to flag this is an error  */
 t=gethms(s);
 if(t<last_time_secs)
		{
		 if((last_time_secs - t) > 18.0*60.0*60.0 )
				{
				 days++; /* if time appears to have  gone > 18 hours backwards  assume this is because we have passed into a new day */
				 skip=false;  // assume time is valid
				}
		 else
				{if(!skip)
						{skip=true;
						 t= -2;/* time has gone backwards for no reason - return -2 to indicate an error */
						}
				 else skip=false; // repeated , accept value  (could be due to a gap in the log [power cut?] that crossed midnight)
				}
		 // rprintf("gethms_day(%s) days=%u last_time=%.1f t=%.1f returns %.1f\n",s,days,last_time_secs,tc,t+24.0*60.0*60.0*(double)days);
		}
 else skip=false; // value appears to be OK
 if(t>=0)
		{ // -ve value of t indicate an error
		 last_time_secs=t;
		 if(days!=0)
				t+=24.0*60.0*60.0*(double)days;   /* 24 hours a day, 3600 secs in a hour */
		}
 return t;
 }
#else
 /* original (slower) code */
double gethms(char *s)
{/* read a time of the format hh:mm:ss.s , returns it as a double value in seconds */
 /* if just a number is found this will be treated as seconds (which can include a decimal point and digits after the dp)
	if aa:bb is found this will be treated as aa mins and bb secs  (which can include a decimal point and digits after the dp)
	if aa:bb:cc is found this will be treated as aa hours, bb mins and cc secs (which can include a decimal point and digits after the dp)
	returns 0 if does not start with a number, otherwise converts as much as possible based on the above format
	This means in particular that trailing whitespace and "'s are ignored
 */
 double secs,d;
 if(!isdigit(*s)) return 0; /* must start with a number */
 secs=strtod(s,&s); /* this could be secs or mins(:secs) or hours(:min:secs) */
 if(*s==':' && isdigit(s[1]))
        {++s; /* skip : , we have 1 or 2:'s*/
         d=strtod(s,&s); /* read mins */
		 secs=secs*60+d; /* convert to secs assuming min:secs */
        }
 if(*s==':' && isdigit(s[1]))
		{++s; /* skip : (2 :'s means we have h:m:s) */
         d=strtod(s,&s); /* read secs */
         secs=secs*60+d; /* convert to secs */
		}
 return secs;
}

static unsigned int days=0;
static double last_time_secs=0;
static bool skip=false; // skip 1st number in a big step
void reset_days(void)  /* reset static variables for gethms_days() - should be used before using gethms_days() to read times from a file  */
{days=0;
 last_time_secs=0;
 skip=true; // believe 1st value
}

double gethms_days(char *s) /* read time in format hh:mm:ss.s , assumed to be called in sequence and accounts for days when time wraps around. Returns secs, or -ve number on error */
{double t;
 if(!isdigit(*s)) return -1; /* should start with a number, return -1 to flag this is an error  */
 t=gethms(s);
 if(t<last_time_secs)
		{
		 if((last_time_secs - t) > 18.0*60.0*60.0 )
				{
				 days++; /* if time appears to have  gone > 18 hours backwards  assume this is because we have passed into a new day */
				 skip=false;  // assume time is valid
				}
		 else
				{if(!skip)
						{skip=true;
						 t= -2;/* time has gone backwards for no reason - return -2 to indicate an error */
						}
				 else skip=false; // repeated , accept value  (could be due to a gap in the log [power cut?] that crossed midnight)
				}
		 // rprintf("gethms_day(%s) days=%u last_time=%.1f t=%.1f returns %.1f\n",s,days,last_time_secs,tc,t+24.0*60.0*60.0*(double)days);
		}
 else skip=false; // value appears to be OK
 if(t>0)
		{ // -ve value of t indicate an error
		 last_time_secs=t;
		 if(days!=0)
				t+=24.0*60.0*60.0*(double)days;   /* 24 hours a day, 3600 secs in a hour */
		}
 return t;
 }
#endif
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

double s_to_double(char *s) /* converts s to double if its a number, otherwise returns NAN */
{double v;
 char *endc;
 v=strtod(s,&endc);// strtod() will read NAN as a "valid" number, correctly returning NAN 
 while(isspace(*endc))++endc; // allow trailing whitespace in column
 if(*endc!=0) 
	{// if not a number set to NAN 
 	 v=strtod("NAN",NULL);// c++ builder 5 does not appear to define NAN as a constant 
	}
 return v;	
}

/* simple regular expression match based on code from http://www.cs.princeton.edu/courses/archive/spr09/cos333/beautiful.html
   "A regular Expression Matcher" by Rob Pike & Brian Kernighan

  This code implements the following simple regular expression matches
        c matches any literal character c  (c cannot be .,^(at start of string),$(at end of string),*.+)
        . matches any single character
        ^ matches the start of the input string (must be 1st character in regex or treated as a normal character
        $ matches the end of the input string (must be the last character in the regex or treated as a normal character
        * matches zero or more occurances of the previous character (shortest match is found )
        + matches 1 or more occurances of the previous character (shortest match is found )

  Note these routines are recursive - maxdepth = length of regex so this should not be excessive.
  Also note that Borland 5 has pcreposix.h (PCRE) which supports more complex regular expressions (Perl style).
*/

static bool matchhere(char *regex, char *text); /* match regex here in text*/
static bool matchstar(int c, char *regex, char *text); /* seach for c*regex at start of text */

bool regex_match(char *regex, char *text)
{ if(regex[0] == '^')
        return matchhere(regex+1,text); /* must match at start */
  do {                                  /* can match anywhere so work along string till match */
        if(matchhere(regex,text))
                return true;
     } while (*text++ != '\0');
  return false;
}

static bool matchhere(char *regex, char *text) /* match regex here in text*/
{if(regex[0] == '\0')
        return true;
 if(regex[1] == '*')
        return matchstar(regex[0],regex+2,text);
 if(regex[1] == '+')
        {if(*text!='\0' && (regex[0]=='.' || regex[0]== *text)) /* 1st char must match, then same as * */
                return matchstar(regex[0],regex+2,text+1);
         else return false;
        }
 if(regex[0] == '$' && regex[1] == '\0')
        return *text=='\0'; /* $ matches end of string */
 if(*text!='\0' && (regex[0]=='.' || regex[0]== *text))
        return matchhere(regex+1,text+1); /* matches this character, try next */
 return false;
}

static bool matchstar(int c, char *regex, char *text) /* search for c*regex at start of text */
{ do {
        if(matchhere(regex,text))
                return true;
      } while(*text != '\0' && (*text++ == c || c == '.'));
  return false;
}

/* generalised least squares - fit y=a*f(x)+b*g(x) - see below for a number of optimised versions for special cases of f(x) and g(x) */
void leastsquares_reg(float *y,float *x,int start, int end,double (*f)(float xparam),double (*g)(float xparam), double *a, double *b)
{double u=0,v=0,w=0,z=0,t=0; /* see my blue notebook for explanation dated 9-12-83 */
 int i;
 for(i=start;i<=end;++i) /* 1st calculate sums */
        {
         u+=((*f)(x[i])) * ((*f)(x[i]));
         v+=((*f)(x[i])) * ((*g)(x[i]));
         w+=((*g)(x[i])) * ((*g)(x[i]));
         z+= y[i] * ((*f)(x[i]));
         t+= y[i] * ((*g)(x[i]));
        }
 /* calclate b 1st as its used in calculation of a */
 if((v*v-u*w)==0.0 )
        {/* would cause a divide by zero error so just set a default result */
         *b=0.0;
        }
 else   {
         *b=(v*z-u*t)/(v*v-u*w);
         }
 if(u ==0.0 )
        {/* would cause a divide by zero error so just set a default result */
         *a=0.0 ;
        }
 else   {
         *a=(z-(*b)*v)/u;
         }

}


#ifdef  onepassreg
/* this version just makes 1 pass over data, uses "updating algorithm" for higher accuracy [works well with floats - but doubles used here for best accuracy]*/
void lin_reg(float *y,float *x, int start, int end, double *m, double *c, double *r2) /* linear regression */
{double meanx=0,meany=0; /* initial values set to mean that N=0 or N=1 do not need to be treated as special cases below */
 double meanx2=0,meanxy=0,meany2=0; /* mean x^2 , mean x*y and mean y^2 */
 double xi,yi;
 int i,N=0; /* N is count of items */
 for(i=start;i<=end;++i) /* only use 1 pass here - to calculate means directly */
        {++N;
         xi = x[i];
         yi = y[i];
         meanx+= (xi-meanx)/(double) N; /* calculate means as mi+1=mi+(xi+1 - mi)/i+1 , this should give accurate results and avoids the possibility of the "sum" overflowing*/
         meany+= (yi-meany)/(double) N;
         meanx2+= (xi*xi-meanx2)/(double) N;
         meanxy+= (xi*yi-meanxy)/(double) N;
         meany2+= (yi*yi-meany2)/(double) N;
        }
 if(meanx*meanx==meanx2)
        {/* y is independent of x [if this is not trapped we get a divide by zero error trying to set m to infinity] or 0 or 1 point given */
         *m=0.0 ;
         *c=meany;
         *r2=0.0;
        }
 else   {/* have a valid line */
         double rt,rb,rm;
         rm=(meanx*meany-meanxy)/(meanx*meanx-meanx2);
         *m=rm;
         *c=meany-rm*meanx; /* y=mx+c so c=y-mx */
         rt=(meanxy-meanx*meany);
         rb=(meany2-meany*meany);
         // rprintf("rm=%g rt=%g rb=%g rt/rb=%g \n",rm,rt,rb,rt/rb);
         if(rb!=0)     /* trap divide by zero */
            *r2= rm * (rt/rb) ;
          else
            *r2=1.0;/* should be in range 0-1 */
         }
}
#else
/* this version makes 2 passes over data, coded to minimise errors - in practice above 1 pass version gives identical results faster */
void lin_reg(float *y,float *x, int start, int end, double *m, double *c, double *r2) /* linear regression */
{double meanx,meany;
 double xi,yi;
 double sx=0,sy=0; /* sums */
 double ssxy=0,ssxx=0,ssyy=0; /* sum of (x-meanx)*(y-meany) etc */
 double sxy=0,ssy=0,ssx=0; /* sum of x*y etc */
 double xm,ym; /* x-meanx etc */
 int i,N=0; /* N is count of items */
 for(i=start;i<=end;++i) /* 1st calculate means */
        {++N;
         sx+=x[i];
         sy+=y[i];
        }
 if(N>=1) /* avoid /0 */
        {
         meanx=sx/(double) N; /* calculate means as sum/N */
         meany=sy/(double) N;
        }
 /* now calculate others on a 2nd pass */
 for(i=start;i<=end;++i)
        {xi=x[i]; /* just get values from array once for each i */
         yi=y[i];
         sxy+= xi*yi;
         ssx+= xi*xi;
         ssy+= yi*yi;
         xm=xi-meanx;
         ym=yi-meany;
         ssxy+=xm*ym;
         ssxx+=xm*xm;
         ssyy+=ym*ym;
        }
  if(N<2)
        {/* need 2 points to define a straight line, if not given return something "safe" */
         *m=0.0;
         if(N==1)
                *c=y[start]; /* one point given - make line pass through it */
         else
                *c=0.0;
         *r2=0.0;
        }
 else if(ssxx==0.0)
        {/* y is independent of x [if this is not trapped we get a divide by zero error trying to set m to infinity */
         *m=0.0 ;
         *c=meany;
         *r2=0.0;
        }
 else   {/* have a valid line */
         double rm;
         rm=ssxy/ssxx;
         *m=rm;
         *c=meany-rm*meanx; /* y=mx+c so c=y-mx */
        if(ssyy!=0)
           *r2=ssxy*ssxy/(ssxx*ssyy);
        else
           *r2=1.0;
         }
}

#endif

#ifdef  onepassreg
void log_reg(float *y,float *x, float yoff,float xoff,int start, int end, double *m, double *c, double *r2) /* linear regression on logx / logy*/
/* yoff and xoff are subtracted from *y and *x this is useful as log(z) requires z>0 */
/* even though inputs are only floats, we use doubles internally to keep high accuracy */
/* this version uses 1 pass over data and should be as accurate as 2 pass solution */
{double meanx=0,meany=0; /* initial values set to mean that N=0 or N=1 do not need to be treated as special cases below */
 double meanx2=0,meanxy=0,meany2=0; /* mean x^2 , mean x*y and mean y^2 */
 double xi,yi;
 int i,N=0; /* N is count of items */
 for(i=start;i<=end;++i) /* only use 1 pass here - to calculate means directly */
        {xi=x[i]-xoff;
         yi=y[i]-yoff;
         if(xi<=0 ||yi<=0) continue; /* skip erronous input data */
         xi=log(xi);
         yi=log(yi);
         ++N;
         meanx+= (xi-meanx)/(double) N; /* calculate means as mi+1=mi+(xi+1 - mi)/i+1 , this should give accurate results and avoids the possibility of the "sum" overflowing*/
         meany+= (yi-meany)/(double) N;
         meanx2+= (xi*xi-meanx2)/(double) N;
         meanxy+= (xi*yi-meanxy)/(double) N;
         meany2+= (yi*yi-meany2)/(double) N;
        }
 if(meanx*meanx==meanx2)
        {/* y is independent of x [if this is not trapped we get a divide by zero error trying to set m to infinity] or 0 or 1 point given */
         *m=0.0 ;
         *c=meany;
         *r2=0.0;
        }
 else   {/* have a valid line */
         double rt,rb,rm;
         rm=(meanx*meany-meanxy)/(meanx*meanx-meanx2);
         *m=rm;
         *c=meany-rm*meanx; /* y=mx+c so c=y-mx */
         rt=(meanxy-meanx*meany);
         rb=(meany2-meany*meany);
         // rprintf("rm=%g rt=%g rb=%g rt/rb=%g \n",rm,rt,rb,rt/rb);
         if(rb!=0)     /* trap divide by zero */
            *r2= rm * (rt/rb) ;
          else
            *r2=1.0;/* should be in range 0-1 */
         }
}
#else
void log_reg(float *y,float *x, float yoff,float xoff,int start, int end, double *m, double *c, double *r2) /* linear regression on logx / logy*/
/* yoff and xoff are subtracted from *y and *x this is useful as log(z) requires z>0 */
/* even though inputs are only floats, we use doubles internally to keep high accuracy */
/* this version uses 2 passes over data */
{double meanx=0,meany=0;
 double xi,yi;
 double sx=0,sy=0; /* sums */
 double ssxy=0,ssxx=0,ssyy=0; /* sum of (x-meanx)*(y-meany) etc */
 double sxy=0,ssy=0,ssx=0; /* sum of x*y etc */
 double xm,ym; /* x-meanx etc */
 int i,N=0; /* N is count of items */
 for(i=start;i<=end;++i) /* 1st calculate means */
        {xi=x[i]-xoff;
         yi=y[i]-yoff;
         if(xi>0 && yi>0)
                {
                 ++N;
                 sx+=log(xi);
                 sy+=log(yi);
                }
        }
 if(N>=1) /* avoid /0 */
        {
         meanx=sx/(double) N; /* calculate means as sum/N */
         meany=sy/(double) N;
        }

 /* now calculate others on a 2nd pass */
 for(i=start;i<=end;++i)
        {xi=x[i]-xoff;
         yi=y[i]-yoff;
         if(xi<=0 ||yi<=0) continue; /* skip erronous input data */
         xi=log(xi);
         yi=log(yi);
         sxy+= xi*yi;
         ssx+= xi*xi;
         ssy+= yi*yi;
         xm=xi-meanx;
         ym=yi-meany;
         ssxy+=xm*ym;
         ssxx+=xm*xm;
         ssyy+=ym*ym;
        }
  // rprintf("log_reg() N=%d ssxx=%g\n",N,ssxx);
  if(N<2)
        {/* need 2 points to define a straight line, if not given return something "safe" */
         *m=0.0;
         if(N==1)
                *c=y[start]; /* one point given - make line pass through it */
         else
                *c=0.0;
         *r2=0.0;
        }
 else if(ssxx==0.0)
        {/* y is independent of x [if this is not trapped we get a divide by zero error trying to set m to infinity */
         *m=0.0 ;
         *c=meany;
         *r2=0.0;
        }
 else   {/* have a valid line */
         double rt,rb;
         rt=ssxy/ssxx;
         *m=rt;
         *c=meany-rt*meanx; /* y=mx+c so c=y-mx */
          rt=((double)(N)*sxy)-(sx*sy);
          rb= (((double)(N)*ssx)-(sx*sx))*(((double)(N)*ssy)-(sy*sy));
          if(rb!=0)
            *r2=rt*rt/rb ;
          else
            *r2=1.0;/* should be in range 0-1 */
         }
}
#endif

#ifdef  onepassreg
void log_lin_reg(float *y,float *x, float yoff,float xoff,int start, int end, double *m, double *c, double *r2) /* linear regression on logy vs x*/
/* yoff and xoff are subtracted from *y and *x this is useful as log(z) requires z>0 */
/* even though inputs are only floats, we use doubles internally to keep high accuracy */
/* this version makes just 1 pass over data while still giving high accuracy */
{double meanx=0,meany=0; /* initial values set to mean that N=0 or N=1 do not need to be treated as special cases below */
 double meanx2=0,meanxy=0,meany2=0; /* mean x^2 , mean x*y and mean y^2 */
 double xi,yi;
 int i,N=0; /* N is count of items */
 for(i=start;i<=end;++i) /* only use 1 pass here - to calculate means directly */
        {
         xi=x[i]-xoff;
         yi=y[i]-yoff;
         if(yi<=0) continue;
         yi=log(yi);
         ++N;
         meanx+= (xi-meanx)/(double) N; /* calculate means as mi+1=mi+(xi+1 - mi)/i+1 , this should give accurate results and avoids the possibility of the "sum" overflowing*/
         meany+= (yi-meany)/(double) N;
         meanx2+= (xi*xi-meanx2)/(double) N;
         meanxy+= (xi*yi-meanxy)/(double) N;
         meany2+= (yi*yi-meany2)/(double) N;
        }
 if(meanx*meanx==meanx2)
        {/* y is independent of x [if this is not trapped we get a divide by zero error trying to set m to infinity] or 0 or 1 point given */
         *m=0.0 ;
         *c=meany;
         *r2=0.0;
        }
 else   {/* have a valid line */
         double rt,rb,rm;
         rm=(meanx*meany-meanxy)/(meanx*meanx-meanx2);
         *m=rm;
         *c=meany-rm*meanx; /* y=mx+c so c=y-mx */
         rt=(meanxy-meanx*meany);
         rb=(meany2-meany*meany);
         // rprintf("rm=%g rt=%g rb=%g rt/rb=%g \n",rm,rt,rb,rt/rb);
         if(rb!=0)     /* trap divide by zero */
            *r2= rm * (rt/rb) ;
          else
            *r2=1.0;/* should be in range 0-1 */
         }
}
#else
void log_lin_reg(float *y,float *x, float yoff,float xoff,int start, int end, double *m, double *c, double *r2) /* linear regression on logy vs x*/
/* yoff and xoff are subtracted from *y and *x this is useful as log(z) requires z>0 */
/* even though inputs are only floats, we use doubles internally to keep high accuracy */
/* this version makes 2 passes over data */
{double meanx=0,meany=0;
 double xi,yi;
 double sx=0,sy=0; /* sums */
 double ssxy=0,ssxx=0,ssyy=0; /* sum of (x-meanx)*(y-meany) etc */
 double sxy=0,ssy=0,ssx=0; /* sum of x*y etc */
 double xm,ym; /* x-meanx etc */
 int i,N=0; /* N is count of items */
 for(i=start;i<=end;++i) /* 1st calculate means */
        {xi=x[i]-xoff;
         yi=y[i]-yoff;
         if(yi>0)
                {
                 ++N;
                 sx+=xi;
                 sy+=log(yi);
                }
        }
 if(N>=1) /* avoid /0 */
        {
         meanx=sx/(double) N; /* calculate means as sum/N */
         meany=sy/(double) N;
        }

 /* now calculate others on a 2nd pass */
 for(i=start;i<=end;++i)
        {xi=x[i]-xoff;
         yi=y[i]-yoff;
         if(yi<=0) continue; /* skip erronous input data */
         yi=log(yi);
         sxy+= xi*yi;
         ssx+= xi*xi;
         ssy+= yi*yi;
         xm=xi-meanx;
         ym=yi-meany;
         ssxy+=xm*ym;
         ssxx+=xm*xm;
         ssyy+=ym*ym;
        }
  // rprintf("log_lin_reg()x[start]=%g y[start]=%g x[end]=%g y[end]=%g N=%d ssxx=%g ssxy=%g meanx=%g meany=%g\n",x[start],y[start],x[end],y[end],N,ssxx,ssxy,meanx,meany);
  if(N<2)
        {/* need 2 points to define a straight line, if not given return something "safe" */
         *m=0.0;
         if(N==1)
                *c=y[start]; /* one point given - make line pass through it */
         else
                *c=0.0;
         *r2=0.0;
        }
 else if(ssxx==0.0)
        {/* y is independent of x [if this is not trapped we get a divide by zero error trying to set m to infinity */
         *m=0.0 ;
         *c=meany;
         *r2=0.0;
        }
 else   {/* have a valid line */
         double rt,rb;
         rt=ssxy/ssxx;
         *m=rt;
         *c=meany-rt*meanx; /* y=mx+c so c=y-mx */
          rt=((double)(N)*sxy)-(sx*sy);
          rb= (((double)(N)*ssx)-(sx*sx))*(((double)(N)*ssy)-(sy*sy));
          if(rb!=0)
            *r2=rt*rt/rb ;
          else
            *r2=1.0;/* should be in range 0-1 */
         }
}
#endif

 /* take approx (smoothed) derivative of tabular data.
    If a lot of data is supplied (> 2*points_lin_reg - normally 200) then we use linear regression over points_lin_reg points to estimate slope
    if a "reasonable amount" of data is supplied (at least 7 points)  :
    at the start a 5th order forward method is used
    in the middle a 7th order (index+/-3) method is used
    at the end is 5th order historical data only method is used
    if only a small amount of data is given then simpler (and less accurate) methods are used
 */
double deriv(float *y,float *x,int start, int end, int index) /* estimate dy/dx at index - only using data between start and end */
{ const int points_lin_reg=100; /* number of points we will do linear regression over - ideally symetrically about point */
  if(index<start || index>end || start==end) return 0; /* index out of range , or only 1 point (start=end)*/
  if(end-start > 2*points_lin_reg)
        {/* we have lots of data - we can use linear regression */
         int s,e;
         if(index<start+points_lin_reg/2)
                {/* want deriv near start, ensure we use a different e for every value of index to hopefully get a "smooth" deriv */
				 s=start;
                 e=index+points_lin_reg/2;
                }
         else if(index>end-points_lin_reg/2)
                {/* want deriv near end, ensure we use a different e for every value of index to hopefully get a "smooth" deriv  */
                 s=index-points_lin_reg/2;
                 e=end;
                }
         else
                {/* can do lin reg symetrically about index */
                 s=index-points_lin_reg/2;
                 e=index+points_lin_reg/2;
                }
         double m,c,r2;
         /* void lin_reg(float *y,float *x, int start, int end, double *m, double *c, double *r2) - linear regression */
         lin_reg(y,x,s,e,&m,&c,&r2);
         return m;
        }
  if(index>=start+3 && index <=end-3)
        {/* use data +/-3 either side of index */
         /* can use a noise reducing derivative, 7th order at least halves noise  */
         /* see http://www.holoborodko.com/pavel/numerical-methods/numerical-derivative/smooth-low-noise-differentiators/ - I used n=2, M=7 here */
         return 0.3125 /* 5/32 *2 */ *  (y[index-1]-y[index+1])/(x[index-1]-x[index+1]) +
         0.5 /* 4/32 *4 */ * (y[index-2]-y[index+2])/(x[index-2]-x[index+2]) +
         0.1875 /* 1/32 *6*/ * (y[index-3]-y[index+3])/(x[index-3]-x[index+3]) ;
        }
  if(end-index>=5)
        {/* use 4 items of data after index derived by PMi based on the 4 items before case */
         /* use a noise reducing derivative, 5th order to give noise reduction of at least 0.65X  based on historical values only
            see http://www.holoborodko.com/pavel/numerical-methods/numerical-derivative/smooth-low-noise-differentiators/ */
         return 0.5 /* 1/8 *4 */ *  (y[index]-y[index+4])/(x[index]-x[index+4]) +
         0.5 /* 2/8 *2 */ * (y[index+1]-y[index+3])/(x[index+1]-x[index+3]);
        }
  if(index==start)
    {return(y[start]-y[start+1])/(x[start]-x[start+1]); /* use 1st 2 points */
    }
  if(index-start>=5)
        {/* use 4 items of data before index */
         /* use a noise reducing derivative, 5th order to give noise reduction of at least 0.65X  based on historical values only
            see http://www.holoborodko.com/pavel/numerical-methods/numerical-derivative/smooth-low-noise-differentiators/ */
         return 0.5 /* 1/8 *4 */ *  (y[index]-y[index-4])/(x[index]-x[index-4]) +
         0.5 /* 2/8 *2 */ * (y[index-1]-y[index-3])/(x[index-1]-x[index-3]);
        }
  if(index==end)
    {   /* normally we would use 4 items before index (above), this deals with the case where there is not this much data availabel */
        return(y[end]-y[end-1])/(x[end]-x[end-1]); /* use last 2 points */
    }
  /* use +/- 1 point around index - only used when nothing else can do the job ! */
  return(y[index-1]-y[index+1])/(x[index-1]-x[index+1]); /* use  2 points either side of the one requested as we don't have enough points for 7th order */
}

#ifdef  onepassreg
void log_diff_lin_reg(float *y,float *x,float xoff,int start, int end, double *m, double *c, double *r2)
 /* linear regression on log(dy/dx) vs x*/
 /* xoff is subtracted from *x */
 /* y is differentiated then loged before doing linear regression. differentiation effectively removes any constants so no need for yoff */
 /* even though inputs are only floats, we use doubles internally to keep high accuracy */
 /* uses double deriv(float *y,float *x,int start, int end, int index)  estimate dy/dx at index - only using data between start and end */
 /* this version makes 1 pass over data - which means we call deriv() once for every point */
{double meanx=0,meany=0; /* initial values set to mean that N=0 or N=1 do not need to be treated as special cases below */
 double meanx2=0,meanxy=0,meany2=0; /* mean x^2 , mean x*y and mean y^2 */
 double xi,yi;
 int i,N=0; /* N is count of items */
 for(i=start;i<=end;++i) /* only use 1 pass here - to calculate means directly */
        {xi=x[i]-xoff;
         yi=fabs(deriv(y,x,start,end,i));
         if(yi<=0) continue; /* skip erronous input data */
         yi=log(yi);
         ++N;
         meanx+= (xi-meanx)/(double) N; /* calculate means as mi+1=mi+(xi+1 - mi)/i+1 , this should give accurate results and avoids the possibility of the "sum" overflowing*/
         meany+= (yi-meany)/(double) N;
         meanx2+= (xi*xi-meanx2)/(double) N;
         meanxy+= (xi*yi-meanxy)/(double) N;
         meany2+= (yi*yi-meany2)/(double) N;
        }
 if(meanx*meanx==meanx2)
        {/* y is independent of x [if this is not trapped we get a divide by zero error trying to set m to infinity] or 0 or 1 point given */
         *m=0.0 ;
         *c=meany;
         *r2=0.0;
        }
 else   {/* have a valid line */
         double rt,rb,rm;
         rm=(meanx*meany-meanxy)/(meanx*meanx-meanx2);
         *m=rm;
         *c=meany-rm*meanx; /* y=mx+c so c=y-mx */
         rt=(meanxy-meanx*meany);
         rb=(meany2-meany*meany);
         // rprintf("rm=%g rt=%g rb=%g rt/rb=%g \n",rm,rt,rb,rt/rb);
         if(rb!=0)     /* trap divide by zero */
            *r2= rm * (rt/rb) ;
          else
            *r2=1.0;/* should be in range 0-1 */
         }
}
#else
void log_diff_lin_reg(float *y,float *x,float xoff,int start, int end, double *m, double *c, double *r2)
 /* linear regression on log(dy/dx) vs x*/
 /* xoff is subtracted from *x */
 /* y is differentiated then loged before doing linear regression. differentiation effectively removes any constants so no need for yoff */
 /* even though inputs are only floats, we use doubles internally to keep high accuracy */
 /* uses double deriv(float *y,float *x,int start, int end, int index)  estimate dy/dx at index - only using data between start and end */
 /* this version makes 2 passes over data - which is quite "expensive" as it means we call deriv() twice for every point */
{double meanx=0,meany=0;
 double xi,yi;
 double sx=0,sy=0; /* sums */
 double ssxy=0,ssxx=0,ssyy=0; /* sum of (x-meanx)*(y-meany) etc */
 double sxy=0,ssy=0,ssx=0; /* sum of x*y etc */
 double xm,ym; /* x-meanx etc */
 int i,N=0; /* N is count of items */
 for(i=start;i<=end;++i) /* 1st calculate means */
        {xi=x[i]-xoff;
         yi=fabs(deriv(y,x,start,end,i));
         if(yi>0)
                {
                 ++N;
                 sx+=xi;
                 sy+=log(yi);
                }
        }
 if(N>=1) /* avoid /0 */
        {
         meanx=sx/(double) N; /* calculate means as sum/N */
         meany=sy/(double) N;
        }

 /* now calculate others on a 2nd pass */
 for(i=start;i<=end;++i)
        {xi=x[i]-xoff;
         yi=fabs(deriv(y,x,start,end,i));
         if(yi<=0) continue; /* skip erronous input data */
         yi=log(yi);
         sxy+= xi*yi;
         ssx+= xi*xi;
         ssy+= yi*yi;
         xm=xi-meanx;
         ym=yi-meany;
         ssxy+=xm*ym;
         ssxx+=xm*xm;
         ssyy+=ym*ym;
        }
  // rprintf("log_diff_lin_reg()x[start]=%g y[start]=%g x[end]=%g y[end]=%g N=%d ssxx=%g ssxy=%g meanx=%g meany=%g\n",x[start],y[start],x[end],y[end],N,ssxx,ssxy,meanx,meany);
  if(N<2)
        {/* need 2 points to define a straight line, if not given return something "safe" */
         *m=0.0;
         if(N==1)
                *c=y[start]; /* one point given - make line pass through it */
         else
                *c=0.0;
         *r2=0.0;
        }
 else if(ssxx==0.0)
        {/* y is independent of x [if this is not trapped we get a divide by zero error trying to set m to infinity */
         *m=0.0 ;
         *c=meany;
         *r2=0.0;
        }
 else   {/* have a valid line */
         double rt,rb;
         rt=ssxy/ssxx;
         *m=rt;
         *c=meany-rt*meanx; /* y=mx+c so c=y-mx */
          rt=((double)(N)*sxy)-(sx*sy);
          rb= (((double)(N)*ssx)-(sx*sx))*(((double)(N)*ssy)-(sy*sy));
          if(rb!=0)
            *r2=rt*rt/rb ;
          else
            *r2=1.0;/* should be in range 0-1 */
         }
}
#endif


// csv_count_cols(char *in ) returns number of columns in string "in" using same algorithm as parsecsv()
// does NOT change "in"
unsigned int csv_count_cols(char *in) 
{  /* parse input line into a number of fields (columns)
	 in ends with a \n OR a \0  or \r
	 returns number of fields found in line
	 fields are comma seperated.
	 If a field is double quoted ("..." or a "string") commas inside quotes don't act as field seperators
	 "" inside a string is ignored (so the comma in ".."".,." is ignored) 
	 \" inside a string is ignored (so the comma in "..\".,." is ignored) 
  */
 unsigned int nos_fields;
 char *p;
 p=in; /* start of 1st field */
 if(in==NULL || *p==0 || *p=='\n' || *p=='\r') // trap special cases, return 0 cols
	{
	  return 0;
	}
 nos_fields=1; // if we have got here at least 1 field is present
 while(*p)
		{
		 // if not already at the end of the string then scan for next comma (keeping track of "...")	
		 while(1)
				{/* scan a field */
				 if( *p=='\n' || *p=='\r')
				 	p++;// skip \n (expect 0 as next character so will be trapped on following line
				 if( *p==0)
						{/* end of input string */
						 break;
						}
				 if(*p==',')
						{p++; /* flag end of a field and move onto next char */
						 nos_fields++;
						 break;
						 }
				 if(*p=='"')
						{++p; /* found a quoted string - within this "" means a single quote character */
						 while(*p)
								{if(*p=='"')
										{p++;
										 break; /* end of quoted string, go back to looking for a comma */
												/* note if this was a "" in the middle of a string the next " will restart this loop so this is NOT a special case*/
										}
								 if(*p=='\\' && p[1]=='"')
								 		{p+=2; // \" does not terminate string
										}
								 else ++p; /* if any other character (including a comma) then skip it (still inside quoted string ) */
								}
						}
				  else ++p; /* any other character is part of the field */
				 }
		}
 return nos_fields;
}

unsigned int parsecsv(char *in,char *outfields[],unsigned int maxfields) /* char *outfields[maxfields] needed */
{  /* parse input line into a number of fields - CHANGES input !!!
	 in ends with a \n OR a \0  or a \r
	 returns number of fields found in line
	 if not enough fields present in input the excess entries in outfields are all set to point to the terminating \000
	 if too many fields (>maxfields) are present in the input line then the extra fields are ignored
	 if in == NULL or *in=EOL all outfields are set to a string thats just \000, and returns 0
	 fields are comma seperated.
	 If a field is double quoted ("..." or a "string") commas inside quotes don't act as field seperators
	 "" inside a string is ignored (so the comma in ".."".,." is ignored) 
	 \" inside a string is ignored (so the comma in "..\".,." is ignored) 
  */
 unsigned int i, nos_fields=0;
 char *p,*startf;
 static char *null_str=(char *)"";
 if(outfields==NULL) return 0; // outfields may have been malloced, so NULL means no space...
 startf=p=in; /* start of 1st field */
 if(in==NULL || *p==0 || *p=='\n' || *p=='\r') // trap special case, return something sensible
	{
	  for(i=0;i<maxfields;++i)
		  outfields[i]=null_str;
	  return nos_fields;
	}
 nos_fields=1; // if we have got here at least 1 field is present
 for(i=0;i<maxfields;++i)
		{// special case if we are already at the end of the string
		 if(*p==0)
		 	{for(;i<maxfields;++i)
				outfields[i]=null_str; // quickly set rest of fields to null string (this ~ doubles execution speed!)
		 	 break;	
		 	}
		 // if not already at the end of the string then scan for next comma (keeping track of "...")	
		 while(1)
				{/* scan a field */
				 if(*p=='\n' || *p=='\r') *p=0;/* end of string - avoids us needing to strip \n out of strings */
				 if(*p==0)
						{/* end of input string do not change p here so all furter fields will point to same (zero length) string (but see optimisation above)*/
						 outfields[i]=startf;
						 startf=p;
						 break;
						}
				 if(*p==',')
						{*p++=0; /* flag end of a field and move onto next char */
						 outfields[i]=startf;
						 startf=p; /* start of next field */
						 if(nos_fields<maxfields) nos_fields++;
						 break;
						 }
				 if(*p=='"')
						{++p; /* found a quoted string - within this "" means a single quote character */
						 while(*p)
								{if(*p=='"')
										{p++;
										 break; /* end of quoted string, go back to looking for a comma */
												/* note if this was a "" in the middle of a string the next " will restart this loop so this is NOT a special case*/
										}
								 if(*p=='\\' && p[1]=='"')
								 		{p+=2; // \" does not terminate string
										}
								 else ++p; /* if any other character (including a comma) then skip it (still inside quoted string ) */
								}
						}
				  else ++p; /* any other character is part of the field */
				 }
		}
 return nos_fields;
}


unsigned int parsewhitesp(char *in,char *outfields[],unsigned int maxfields) /* char *outfields[maxfields] needed */
{  /* parse input line into a number of fields - CHANGES input !!!
	 field seperator is "whitespace"  , multiple "whitespace" chars are treated as a single seperator
	 leading whitespace on any field is skipped (so whitespace at the start of a line does not cause the field to be incremented )
	 in ends with a \n OR a \0
	 if not enough fields present in input the excess are all set to point to the terminating \000
	 if too many fields (>maxfields) are present in the input line then the extra fields are ignored
	 trailing whitespace at the end of the line is also ignored
	 if in == NULL or *on=EOL all outfields are set to a string thats just \000, and returns 0, otherwise returns nos fields in input
  */
 unsigned int i,nos_fields=0;
 char *p,*startf;
 p=in; /* start of 1st field */
 if(in!=NULL)
	 while(isspace(*p)) ++p; // skip whitespace at the start of the line
 if(in==NULL || *p=='\0' || *p=='\n' ) // trap special case, return something sensible
	{static char *null_str=(char *)"";
	 for(i=0;i<maxfields;++i)
		  outfields[i]=null_str;
	  return nos_fields;
	}
 nos_fields=1; // 0 is a special case above, so if we have got here at least 1 field is present
 startf=p;
 for(i=0;i<maxfields;++i)
		{while(1)
				{/* scan a field */
				 if(*p=='\n') *p='\0';/* end of string - avoids us needing to strip \n out of strings */
				 if(*p=='\0')
						{/* end of input string do not change p here so all furter fields will point to same (zero length) string */
						 outfields[i]=startf;
						 startf=p;
						 break;
						}
				 if(isspace(*p))
						{*p++='\0'; /* flag end of a field and move onto next char */
						 while(isspace(*p)) ++p; // multiple spaces are treated as one
						 outfields[i]=startf;
						 startf=p; /* start of next field */
						 if(nos_fields<maxfields && *p!='\0') nos_fields++; // don't want trailing space to add an extra field
						 break;
						 }
				 if(*p=='"')
						{++p; /* found a quoted string - within this "" means a single quote character */
						 while(*p)
								{if(*p=='"')
										{p++;
										 break; /* end of quoted string, go back to looking for whitespace */
												/* note if this was a "" in the middle of a string the next " will restart this loop so this is NOT a special case*/
										}
								 else ++p; /* if any other character (including whitespace) then skip it (still inside quoted string ) */
								}
						}
				  else ++p; /* any other character is part of the field */
				 }
		}
 return nos_fields;
}


static const unsigned int FIRST_SIZE=256; /* initial size of line buffer start at eg 256, size is doubled if its too small */
static char *buf=NULL; /* input buffer */
static unsigned int buf_size=0; /* input buffer size, 0 means not yet allocated */
// #define DEBUG_readline
#if 1
// new version using fgets() which may be faster as reads from file in big chunks - actualy makes very little difference
char *readline (FILE *fp)
/* read next line from input and return a pointer to it. Returns NULL on EOF or error . Deletes \n from end of line */
/* the same buffer is reused for the next input */
{int n;
 char *cp,*cpr;
 int c;
 char *new_buf;
 unsigned int new_size;
#ifdef  DEBUG_readline
 rprintf("readline() called: buf_size=%d\n",buf_size);
#endif
 if(buf_size==0)
        {if((buf=(char *)malloc(FIRST_SIZE))==NULL )
                return NULL; /* oops no space at all */
         buf_size=FIRST_SIZE;
        }
 cp=buf;
 n=buf_size;
 while(1)
        {cp[n-1]=0x7f; // flag used to detect line too long
         cp[n-2]=0x7f;
         cpr=fgets(cp,n,fp);
#ifdef  DEBUG_readline
         rprintf(" after call to fgets(), buf_size=%d\n",buf_size);
         rprintf(" cp[n-2]=0x%x cp[n-1]=0x%x\n",(int)(cp[n-2]),(int)(cp[n-1]));
         rprintf(" string=%s\n",buf);
#endif
         if(cpr==NULL)
                return NULL; /* if we hit EOF then return NULL */
         if(cp[n-1]==0x7f || (cp[n-2]=='\n' && cp[n-1]==0)) return buf; // normal return
         new_size=buf_size <<1; /* double size of buffer */
         if((new_buf=(char *)realloc(buf,new_size))==NULL)
                {/* realloc failed "eat" rest of line and return truncated line to caller */
                 while((c=getc(fp)) != EOF && c!= '\n');
                 *cp='\0';
                 return buf;
                }
           else
                { /* realloc went OK */
                 cp=new_buf+(buf_size-1); /* reposition pointer into new buffer */
                 n=buf_size+1;
                 buf=new_buf;
                 buf_size=new_size;
                }
        }
/* NOT REACHED */
}
#else
 // original version only using getc()
char *readline (FILE *fp)
/* read next line from input and return a pointer to it. Returns NULL on EOF or error . Deletes \n from end of line */
/* the same buffer is reused for the next input */
{int n;
 char *cp;
 int c=0;
 char *new_buf;
 unsigned int new_size;
 if(buf_size==0)
        {if((buf=(char *)malloc(FIRST_SIZE))==NULL )
                return NULL; /* oops no space at all */
         buf_size=FIRST_SIZE;
        }
 cp=buf;
 n=buf_size;
 while(1)
        {while(--n && (c=getc(fp)) != EOF) /* while fits into existing buffer */
                {if(c=='\r') {++n;continue;} // just skip \r (CR) = should only be present if input file is in binary mode
                 if(c=='\n')
                        {*cp='\0'; /* end of string */
                         return buf;
                        }
                  *cp++=c;
                 }
         if(c==EOF)
                return cp==buf ? NULL : buf; /* if we hit EOF then return last line if any */
         new_size=buf_size <<1; /* double size of buffer */
         if((new_buf=(char *)realloc(buf,new_size))==NULL)
                {/* realloc failed "eat" rest of line and return truncated line to caller */
                 while((c=getc(fp)) != EOF && c!= '\n');
                 *cp='\0';
                 return buf;
                }
           else
                { /* realloc went OK */
                 cp=new_buf+(buf_size-1); /* reposition pointer into new buffer */
                 n=buf_size+1;
                 buf=new_buf;
                 buf_size=new_size;
                }
        }
/* NOT REACHED */
}
#endif

float nextafterf(float f) /* returns the next higher floating point number - should be in  C11 math.h */
{  union _f_andi32
		{float uf;
		 int32_t i; // must be signed for >>31 "trick" to work
		} f_i;
	f_i.uf=f;
	if(f_i.i== 0x80000000) f_i.i=0; // trap -0 and convert it to +0
	f_i.i += (f_i.i >> 31) | 1;   // this will add 1 for positive f and -1 for negative f
        // if(_isnan(f_i.uf)) return MINFLOAT; // just in case above does not trap all special cases
        // rprintf("nextafterf(%g) returns %g\n",f,f_i.uf);
	return f_i.uf;
}
#if 0  /* from https://gcc.gnu.org/legacy-ml/gcc-patches/2004-08/msg00109.html , see also https://stackoverflow.com/questions/5897874/nextafterf-on-visual-c/29908831   */
 /* This is a portable implementation of nextafterf that is intended to be
    independent of the floating point format or its in memory representation.
    This implementation skips denormalized values, for example returning
    FLT_MIN as the next value after zero, as many target's frexpf, scalbnf
    and ldexpf functions don't work as expected with denormalized values.  */
 float
 nextafterf(float x, float y)
 {
   int origexp, newexp;

   if (isnan(x) || isnan(y))
     return x+y;
   if (x == y)
     return x;

   if (x == 0.0f)
     return y > 0.0f ? FLT_MIN : -FLT_MIN;

   frexpf(x, &origexp);
   if (x >= 0.0)
     {
       if (y > x)
 	{
 	  if (x < FLT_MIN)
 	    return FLT_MIN;
 	  return x + scalbnf(FLT_EPSILON, origexp-1);
 	}
       else if (x > FLT_MIN)
 	{
 	  float temp = x - scalbnf(FLT_EPSILON, origexp-1);
	  frexpf(temp, &newexp);
 	  if (newexp == origexp)
 	    return temp;
	  return x - scalbnf(FLT_EPSILON, origexp-2);
 	}
       else
 	return 0.0f;
     }
   else
     {
       if (y < x)
 	{
 	  if (x > -FLT_MIN)
 	    return -FLT_MIN;
 	  return x - scalbnf(FLT_EPSILON, origexp-1);
 	}
       else if (x < -FLT_MIN)
 	{
	  float temp = x + scalbnf(FLT_EPSILON, origexp-1);
 	  frexpf(temp, &newexp);
	  if (newexp == origexp)
 	    return temp;
 	  return x + scalbnf(FLT_EPSILON, origexp-2);
 	}
      else
 	return 0.0f;
     }
 }
#endif

#if 1
float fastexp(float x) /* fast approximation to exp(x) max_rel_err = 0.00237 , very accurate for x near 0 */
                        /* this version obtained by fitting a polynomial to the error from fasterexp(x) */
                        /* errors are very small for small |x| */
                        /* its almost exactly the same speed as version below on a typical optimisation ! */
{
 int32_t i ; /* integer part of x */
 float f;    /* fractional part of x */
 union _f_andi32
		{float uf;
		 int32_t ui;
		} f_i;
 if(x<0.0f)
        {
         if(x>=-0.048705f) return 1.0f+x;/* exp(x)=1+x+x^2/2+x^3/6+...  , below has relative error 0.00237 so 0.048705 is sqrt(0.00237) */
         if(x<-87.0f) return 0.0f; /* underflow */
         x *= 1.44269504f; /* 1/ln(2) as below give 2^x and we want e^x */
         i = x;
         f = x - (float)(i-1);
        }
   else
        { /* x>= 0 */
         if(x<=0.048705f) return 1.0f+x;/* exp(x)=1+x+x^2/2+x^3/6+...  , below has relative error 0.00237 so 0.048705 is sqrt(0.00237) */
         if(x>87.0f) return MAXFLOAT;  /* trap over and under flow */
         x *= 1.44269504f; /* 1/ln(2) as below give 2^x and we want e^x */
         i = x;
         f = x - (float)i;   /* f=fractional part of x (i is integer part of x) */
        }
#if 1
 f_i.ui= 8388610.0f * (x+(3.43149624e-01f*f-3.48095891e-01f)*f+127.002342f);  /* all in 1 equation, otherwise identical to below */
#else
 float y;    /* correction from polynomial */
 y=(3.43149624e-01f*f-3.48095891e-01f)*f+2.37234927e-03f; /* polynomial correction for errors, avoids needing fp divides which can be slow */
 f_i.ui= 8388610.0f * (x+y); /* 8388610 = 2^23 */
 f_i.ui += 0x3f800000 ;
#endif
 return f_i.uf;
}
#else
float fastexp(float x) /* fast approximation to exp(x) max_rel_err = 3.10978e-05 , very accurate for x near 0 */
{
 int32_t i ; /* integer part of x */
 float f;    /* fractional part of x */
 union _f_andi32
        {float uf;
         int32_t ui;
        } f_i;
 if(x<0.0f)
        {if(x>=-0.0077f) return 1.0f+x;/* exp(x)=1+x+x^2/2+x^3/6+...  , below has relative error 3e-5 so 0.0077 is sqrt(3e-5) */
         if(x<-87.0f) return 0.0f; /* underflow */
         x *= 1.44269504f; /* 1/ln(2) as below give 2^x and we want e^x */
         i = x;
         f = x - (float)(i-1);
        }
   else
        { /* x>= 0 */
         if(x<=0.0077f) return 1.0f+x;/* exp(x)=1+x+x^2/2+x^3/6+...  , below has relative error 3e-5 so 0.0077 is sqrt(3e-5) */
         if(x>87.0f) return MAXFLOAT;  /* trap over and under flow */
         x *= 1.44269504f; /* 1/ln(2) as below give 2^x and we want e^x */
         i = x;
         f = x - (float)i;   /* f=fractional part of x (i is integer part of x) */
        }
 f_i.ui= 8388610.0f * (x+121.2740575f + 27.7280233f/(4.84252568f - f)-1.49012907f*f); /* 8388610 = 2^23 */
 return f_i.uf;
}
#endif

float fasterexp(float x) /* fast approximation to exp(x) max_rel_err = 0.030280, very accurate for x near 0 (<0.0161 rel and 0.0152 abs error for |x|<0.17 and zero error at x=0) */
        /* 1 fp + in best case , 1 fp * in worse case */
{
 int32_t i ;
 union _f_andi32
        {float uf;
         int32_t ui;
        } f_i;
 if((x>=0.0f && x<=0.17f)||(x<0.0f && x>=-0.17f)) return 1.0f+x;/* exp(x)=1+x+x^2/2+x^3/6+...  , below has relative error 0.03 so 0.17 is sqrt(0.03) */
 if(x>87.0f) return MAXFLOAT;
 if(x<-87.0f) return 0.0f;
 i = ( x * 12102203.16156f );   /* 12102203.16156f = (2^23)/ln(2) - note this will not overflow a 32 bit int with x=87 */
 f_i.ui = i + 0x3f800000 - 361007;  /* 361007 = (0.08607133/2)*(2^23) */
 return f_i.uf;
}

#if 1
float fastlog (float x)   /* approximation to ln(x) applying correction polynomial */
						  /* rel error near 1 is 0.00114578 at x=1.45801, rel error far from 1 is 0.000180066 , abs error 0.00050354     */
						  /* uses 8 fp ops (4*,4+) near x=1 and 8 fp ops 4*,4+ elsewhere */
{ union _f_andi32
        {float f;
         int32_t i;
		} f_i;
 float y,f;
 f_i.f=x;
 if(f_i.i<0) return 0.0f; /* really an error, 0 already gives a large negative number */
 if(x>= 0.72f && x<= 1.28f)   /* if near 1 use a series expansion thats accurate for x near 1 */
        {float y=x-1.0f;  /* set to optimium for order  4 main poly , using order 5 correction [which has same # fp ops as "main" calculation of order 4 */
         return (((-0.25*y+0.3333333333f)*y-0.5f)*y+1.0f)*y; /* higher order approxm for use near x=1, exact at x=1 */
        }
 y=f_i.i;
 y *= 8.262958295e-8f; /* 2^-23 *ln(2) so value ~= ln(x)+128*ln(2) */
 f_i.i=(f_i.i & 0x7fffff) | 0x3f000000;  /* "mantissa" of x */
 f=f_i.f;
#if 1
 return y + ((8.61987926e-01f*f-2.88153022f)*f+2.81358874f)*f-88.823357893f; /* order 4 poly */
#else
 float p;
 p=((8.61987926e-01f*f-2.88153022f)*f+2.81358874f)*f-7.93665993e-01f; /* order 4 poly */
 return y- 88.0296919f + p; /* approx to minimise error of ln approximation */
#endif
}
#else
float fastlog (float x)   /* fast approximation to loge(x) maxrelative error of 0.00330059 & max abs error of 0.000107527 */
                          /* uses 7 fp ops in total (including 1 divide, 2 mults , 4+) */
{ union _f_andi32
        {float f;
         int32_t i;
        } f_i;
 float y;
 f_i.f=x;
 if(f_i.i<0) return 0.0f; /* really an error, 0 already gives a large negative number */
 y=f_i.i;
 y *= 8.262958295e-8f; /* 2^-23 *ln(2) so value ~= ln(x)+128*ln(2) */
 f_i.i=(f_i.i & 0x7fffff) | 0x3f000000;  /* "mantissa" of x */
 return y-86.10656548f - 1.03835548f*f_i.f-1.196288856f/(0.3520887068f+f_i.f); /* approx to minimise error of ln approximation */
}
#endif

float fasterlog (float x)   /* faster approximation to loge(x) maxrelative error of 0.0732524 & max abs error of 0.0397146 very accurate near |x|=1*/
        /* 1 fp - in best case , 1 fp * and 1 fp - in worse case */
{ union _f_andi32
        {float f;
         int32_t i;
        } f_i;
 float y;
 f_i.f=x;
 if(f_i.i<0) return 0.0f; /* really an error, 0 already gives a large negative number */
 if(x>0.85f && x<1.15f) return x-1.0f; /* more accurate solution near x=1, exact at x=1 (where log(1)=0 so this improves relative error significantly */
 y=f_i.i;
 y *= 8.262958295e-8f; /* 2^-23 *ln(2) so value ~= ln(x)+128*ln(2) */
 return y-87.989977f; /* simple (fast) approx to minimise error of ln approximation */
}

float fastsqrt(float x) /* fast sqrt maxrelative error of 0.00175123 */
                        /* error can be dramatically improved by uncommenting one line of code below if required */
{float y0;   /* approx 1/sqrt(x) - doing it this way avoids divisions in this code */
 float x0=x*0.5f;
 union _f_andi32
        {float f;
         int32_t i;
        } f_i;
 int32_t ei ;
 f_i.f=x;
 ei=f_i.i;
 if(ei<=0) return 0.0f; /* really an error except for 0 which should give 0 */
 ei=0x5f375a86 - (ei>>1); /* approx to 1/sqrt(x) */
		/* const       init rel error   after 1 itn     after 2 itns       */
		/* 0x5f3759df   0.0343741       0.00175206      4.63023e-06        */
        /* 0x5f37642f   0.0342129       0.00177582      4.76837e-06        */
        /* 0x5f375a86   0.0343644       0.00175123      4.63388e-06 */
 f_i.i=ei;
 y0=f_i.f; /* y0 ~= 1/sqrt(x) */
 // y0*=(1.5f-x0*y0*y0);    /* 2 iterations of newton-rephson iteration [comment out this for just 1 itn ] */
 y0*=(1.5f-x0*y0*y0)*x;  /* (/sqrt(x) )*x = sqrt(x) */
 return y0;
}

float fastersqrt(float x) /* even faster sqrt but worse error - maxrelative error of 0.0351635 at x=4.29687   */
                          /* this version directly estimates sqrt(x) so needs no FP operations  */
{
 union _f_andi32
        {float f;
         int32_t i;
        } f_i;
 int32_t ei ;
 f_i.f=x;
 ei=f_i.i;
 if(ei<=0) return 0.0f; /* really an error except for 0 which should give 0 */
 ei= (ei>>1) + 0x1FBB4000;/* ei=(ei>>1) +(1<<29) - (1<<22) - 0x4c000; 1<<22 = 1<<23 /2 removes last bit of exponent, 1<<29 adds 64 to exponent to restore excess 128 after >>1 */
 f_i.i=ei;
 return f_i.f; /* ~sqrt(x) */
}

#if 1
float fastinv(float x) /* fast 1/x maxrelative error of 0.00477122 , useful to avoid slow divisions */
                        /* error can be dramatically improved by uncommenting one line of code below if required */
                        /* uses fact that 1/x=1/sqrt(x)*1/sqrt(x) to get initial estimate for 1/x then 1 itn newtons itn for 1/x to refine */
                        /* 3 FP* and 1 FP- */
{float y0;   /* approx 1/(x) - doing it this way avoids divisions in this code */
 union _f_andi32
        {float f;
         int32_t i;
        } f_i;
 int32_t ei,sign ;
 f_i.f=x;
 sign=f_i.i;
 f_i.i &=  0x7fffffff; /* get rid of sign */
 ei=f_i.i; /* |x| */
 if(ei==0) return MAXFLOAT; /* 1/0=inf */
 ei=0x5f3759df - (ei>>1); /* approx to 1/sqrt(x) */
        /* const        after 1 itn rel error in 1/x approx     */
		/* 0x5f3759df   0.00477122 <-- best                     */
        /* 0x5f37642f   0.00484363                              */
        /* 0x5f375a86   0.00477576                              */
 f_i.i=ei;
 f_i.f*=f_i.f; /* f_i.f~=1/sqrt(x) so f_i.f^2 ~= 1/x */
 f_i.i |= sign & 0x80000000;/* put sign back on - as iteration below needs this (as x is signed) */
 y0=f_i.f;
 // y0*=(2.0f-x*y0);    /* 2 iterations of newton-raphson iteration [comment out this for just 1 itn ] - give max rel error of 2.28274e-05 */
 y0*=(2.0f-x*y0);  /* (better approx to 1/x */
 return y0;
}
#else
float fastinv(float x) /* fast 1/x maxrelative error of 0.00349936 , useful to avoid slow divisions */
                        /* error can be dramatically improved by uncommenting one line of code below if required */
                        /* uses fact that 1/x=1/sqrt(x)*1/sqrt(x) */
                        /* 5 FP* and 1 FP- */
{float y0;   /* approx 1/sqrt(x) - doing it this way avoids divisions in this code */
 float x0; /* 0.5*|x| */
 union _f_andi32
        {float f;
         int32_t i;
        } f_i;
 int32_t ei,sign ;
 f_i.f=x;
 sign=f_i.i;
 f_i.i &=  0x7fffffff; /* get rid of sign */
 x0=f_i.f*0.5f;
 ei=f_i.i; /* |x| */
 if(ei==0) return MAXFLOAT; /* 1/0=inf */
 ei=0x5f375a86 - (ei>>1); /* approx to 1/sqrt(x) */
 f_i.i=ei;
 y0=f_i.f; /* y0 ~= 1/sqrt(x) */
 // y0*=(1.5f-x0*y0*y0);    /* 2 iterations of newton-raphson iteration [comment out this for just 1 itn ] */
 y0*=(1.5f-x0*y0*y0);  /* (better approx to 1/sqrt(x) */
 f_i.f=y0*y0;  /* 1/x=1/sqrt(x)*1/sqrt(x) */
 f_i.i |= sign & 0x80000000;/* put sign back on */
 return f_i.f;
}
#endif

float fasterinv(float x) /* even faster 1/x but worse error - maxrelative error of 0.0690738   .Useful to avoid slow divisions */
                         /* only uses 1 floating point operation - a multiply ! */
                         /* this version is based on an algorithm in "Fast Inverse Square Root" by Chris Lomont */
{
 union _f_andi32
        {float f;
         int32_t i;
        } f_i;
 int32_t ei,sign ;
 f_i.f=x;
 sign=f_i.i;
 ei=sign&0x7fffffff;
 if(ei==0) return MAXFLOAT; /* 1/(+/-0)=inf */
 ei=0x5f3759df - (ei>>1); /* approx to 1/sqrt(|x|) */
        /* max rel errors when used to get 1/x:  */
        /*  0x5f3759df gives 0.0690738 at x=2.577   <= this is the best */
        /*  0x5f37642f gives 0.0695962 at x=2.577   */
        /*  0x5f375a86 gives 0.0691068 at x=2.577  */
 f_i.i=ei;
 f_i.f=f_i.f*f_i.f ; /* 1/x=1/sqrt(x)*1/sqrt(x) */
 f_i.i |= sign & 0x80000000;/* put sign back on */
 return  f_i.f;
}

bool sexpr(char *in[],int maxfields,char *sexpression)
  /* see if line (split up by parsecsv matches sexpression matches string expression */
  /* returns false if expression invalid or is just whitespace*/
  /* expressions are !( ...) $n=="..." $n !="...."
     && or &
     || or |
  */
{ bool r;
  sexpr_n= maxfields;
  sexpr_in=in;
  sexpr_cp=sexpression;
  sflag=true;
  while(isspace(*sexpr_cp)) ++sexpr_cp; /* skip whitespace */
  if(*sexpr_cp=='\0') return false; /* special case - null expression always returns false */
  r=sexpr1();
  while(isspace(*sexpr_cp)) ++sexpr_cp; /* skip whitespace */
  if(sflag && *sexpr_cp=='\0') return r;
  // rprintf("sexpr() sflag=%s\n", P_BOOL(sflag));
  sflag=false; /* syntax error */
  return false;
}

bool sexpr1(void)
{/* deal with | expressions */
 bool v;
 while(isspace(*sexpr_cp)) ++sexpr_cp; /* skip whitespace */
 v=sexpr2();
 if(!sflag) return false;
 while(sflag)
        {while(isspace(*sexpr_cp)) ++sexpr_cp; /* skip whitespace */
         if(*sexpr_cp=='|')
                {++sexpr_cp;
                 if(*sexpr_cp=='|')  ++sexpr_cp;/* also allow || for compatiblity with expr() */
                 v=sexpr2() || v;
                 if(!sflag) return false;
                }
          else sflag=false;
         }
 sflag=true;
 return v;
}

bool sexpr2(void)
{/* deal with & expressions */
 bool v;
 while(isspace(*sexpr_cp)) ++sexpr_cp; /* skip whitespace */
 v=sexpr3();
 if(!sflag) return false;
 while(sflag)
        {while(isspace(*sexpr_cp)) ++sexpr_cp; /* skip whitespace */
         if(*sexpr_cp=='&')
                {++sexpr_cp;
                 if(*sexpr_cp=='&')  ++sexpr_cp; /* also allow && for compatiblity with expr() */
                 v=sexpr3() && v;
                 if(!sflag) return false;
                }
          else sflag=false;
         }
 sflag=true;
 return v;
}

bool sexpr3(void)
{ /* deal with ! (..) or $n == "..." or $n!= "..." or ~ "regex" */
 bool v, lnot=false;
 while(isspace(*sexpr_cp)) ++sexpr_cp; /* skip whitespace */
 if(*sexpr_cp=='!')
        {lnot=true;
         sexpr_cp++; /* skip ! */
         while(isspace(*sexpr_cp)) ++sexpr_cp; /* skip whitespace */
        }
 if(*sexpr_cp=='(')
        {
         sexpr_cp++; /* skip ( */
         while(isspace(*sexpr_cp)) ++sexpr_cp; /* skip whitespace */
         v=sexpr1();
         if(!sflag) return false;
         while(isspace(*sexpr_cp)) ++sexpr_cp; /* skip whitespace */
         if(*sexpr_cp!=')')
                {sflag=false;
                 return false;
                }
          ++sexpr_cp;
         }
 else if(*sexpr_cp=='$')
        { /* $n */
         int n=0;
         char op;
         ++sexpr_cp; /* skip $ */
         if(!isdigit(*sexpr_cp))
                {sflag=false; /* $ must be followed by a number [with no spaces ] */
                 return false;
                }
         while(isdigit(*sexpr_cp))
                {n=n*10+*sexpr_cp-'0';
                 ++sexpr_cp;
                }
         if(n==0 || n>sexpr_n)
                {/* n out of range */
                 sflag=false;
                 return false;
                }
         while(isspace(*sexpr_cp)) ++sexpr_cp; /* skip whitespace */
         if(*sexpr_cp=='~')
                { /* regex match need to remove ".." from regex inside "..." "" becomes a single " */
                 const int MAX_REGEX=1000;  /* sorry for fixed size, but don't really want to be allocating variable length strings here as may fragement memory */
                 static char regex[MAX_REGEX]; /* static is OK as this bit of the function is not recursive */
                 int i=0;
                 sexpr_cp++; /* skip ~ */
                 while(isspace(*sexpr_cp)) ++sexpr_cp; /* skip whitespace */
                 if(*sexpr_cp !='"')
                        {sflag=false; /* no leading " for regex */
                         return false;
                        }
                 ++sexpr_cp; /* skip leading " - then copy rest of regex into a buffer */        
                 while(i<MAX_REGEX-1 && (*sexpr_cp != '"' || sexpr_cp[1]=='"') && *sexpr_cp!= '\0')
                        {if(*sexpr_cp == '"') ++sexpr_cp; /* "" becomes a single " in regex */
                         regex[i]= *sexpr_cp++;
                         ++i;
                        }
                 regex[i]='\0'; /* null terminate regex */
                 // rprintf("sexpr3: ~ found regex=<%s> $n=<%s>\n",regex,sexpr_in[n-1]);
                 if(*sexpr_cp!='"')
                        {sflag=false; /* regex too long or no trailing " */
                         return false;
                        }
                 sexpr_cp++ ; /* skip trailing " */
                 v=regex_match(regex,sexpr_in[n-1]);
                }
         else if((*sexpr_cp=='!' && sexpr_cp[1]=='=') || (*sexpr_cp=='=' && sexpr_cp[1]=='=') )
                {/* got an operator (!= or == ) */
                 op=*sexpr_cp++;
                 sexpr_cp++; /* != needs to skip 2 chars */
                 while(isspace(*sexpr_cp)) ++sexpr_cp; /* skip whitespace */
                 if(*sexpr_cp !='"')
                        {sflag=false; /* error need a quoted string */
                         return false;
                        }
                  v=scmp(n-1); /* see if expression is true */
                  if(*sexpr_cp!='"')
                        {sflag=false; /* error string not terminated */
                         return false;
                        }
                  ++sexpr_cp; /* skip trailing " */
                  if(op=='!') v= !v; /* operator was != */
                 }
         else
                {sflag=false; /* did not get a valid operator (!= or = )*/
                 return false;
                }
        }
 else
        {/* invalid */
         sflag=false;
         return false;
        }
 if(lnot) v=!v; /* expression started with a ! */
 return v;
}

bool scmp(int i)
{/* match a string on entry *cp=" and on exit it should also be on a " */
 /* allows "" within a match string = " so """" would match a single " */
 bool same=true;
 char *ip=sexpr_in[i]; /* $n */
 // rprintf("scmp(%d) ip=<%s> sexpr_cp=<%s>\n",i,ip,sexpr_cp);
 ++sexpr_cp; /* skip leading " */
 while (same && *sexpr_cp != '\0' && (*sexpr_cp !='"' || sexpr_cp[1] =='"') && *ip != '\0')
        {if(*sexpr_cp == '"') ++ sexpr_cp; /* "" is 2 characters so skip the 1st here */
         same=*sexpr_cp++ == *ip++ ;
        }
 if(!same) /* "eat up" rest of match string so it parses correctly */
        while (*sexpr_cp != '\0' && (*sexpr_cp !='"' || sexpr_cp[1] =='"') )
        {if(*sexpr_cp == '"') ++ sexpr_cp; /* "" is 2 characters so skip the 1st here */
         sexpr_cp++ ;
        }
 return same && *ip=='\0' && *sexpr_cp=='"'; /* both strings need to completely match to return true */
 }

 bool last_sexpr_ok() /* returns true if last sexpr expression was correct in syntax , false otherwise */
{ return sflag;
}

float t90(int dt) /* returns t score for checking both upper and lower limits at 90% certainty - from pp154 Texas Instruments TI-51-III owners manual */
                  /* note if we are only checkking an upper or a lower limit this is t1sided95 the 95% limit */
{ float t90val[]={      6.314,2.920,2.353,2.132,2.015,
                        1.943,1.895,1.860,1.833,1.812,
                        1.796,1.782,1.771,1.761,1.753,
                        1.746,1.740,1.734,1.729,1.725,
                        1.721,1.717,1.714,1.711,1.708,
                        1.706,1.703,1.701,1.699,1.697,
                        1.684,1.671,1.658,1.645
                        };
   float t90dt[]={      1,2,3,4,5,
                        6,7,8,9,10,
                        11,12,13,14,15,
                        16,17,18,19,20,
                        21,22,23,24,25,
                        26,27,28,29,30,
                        40,60,120,MAXFLOAT
						};
STATIC_ASSERT(ELEMENTS_IN_ARR(t90dt) == ELEMENTS_IN_ARR(t90val));

 // float interp1D(float *xa, float *ya, int size, float x, bool clip)
 return interp1D(t90dt,t90val,ELEMENTS_IN_ARR(t90dt),dt,true);
}

float t99(int dt) /* returns t score for checking both upper and lower limits at 99% certainty - from pp154 Texas Instruments TI-51-III owners manual */
                  /* note if we are only checkking an upper or a lower limit this is t1sided99_5 the 99.5% limit */
{ float t99val[]={      63.657,9.925,5.841,4.604,4.032,
                        3.707,3.499,3.355,3.250,3.169,
                        3.106,3.055,3.012,2.977,2.947,
                        2.921,2.898,2.878,2.861,2.845,
                        2.831,2.819,2.807,2.797,2.787,
                        2.779,2.771,2.763,2.756,2.750,
                        2.704,2.660,2.617,2.576
                        };
  float t99dt[]={       1,2,3,4,5,
                        6,7,8,9,10,
                        11,12,13,14,15,
						16,17,18,19,20,
                        21,22,23,24,25,
                        26,27,28,29,30,
                        40,60,120,MAXFLOAT
						};
STATIC_ASSERT(ELEMENTS_IN_ARR(t99dt) == ELEMENTS_IN_ARR(t99val));

 // float interp1D(float *xa, float *ya, int size, float x, bool clip)
 return interp1D(t99dt,t99val,ELEMENTS_IN_ARR(t99dt),dt,true);
}

float r2test90(int nossamples) /* gives r2test value for 90% confidence  - from pp77 Texas Instruments TI-51-III owners manual */
{float t=t90(nossamples-2);
 t*=t;
 return t/(t+(float)(nossamples-2));
}

float r2test99(int nossamples) /* gives r2test value for 99% confidence  - from pp77 Texas Instruments TI-51-III owners manual */
{float t=t99(nossamples-2);
 t*=t;
 return t/(t+(float)(nossamples-2));
}


