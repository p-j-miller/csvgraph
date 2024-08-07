//---------------------------------------------------------------------------
/* expression handling code and other generally useful code for Builder C++ */
/*----------------------------------------------------------------------------
 * Copyright (c) 2012, 2013,2022, 2024 Peter Miller
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

Version 0.1 26-9-2012  has basic crprintf and working lookup & install
version 0.2 4/10/12 has working crprintf & lookup & install
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
version 1.12 10/11/12 - added Application->ProcessMessages() called to crprintf to allow windows to update on this call (and added to monitor function of optimiser)
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
version 4.1 7-3-2021  - added void leastsquares_rat3(float *y,float *x,int start, int end, double *a, double *b, double *c)
version 4.2 27/3/2020 - added Allow_dollar_T option to allow variables of form $T1 to appresr in expressions
version 5.0 31/7/2022 - added wchar_t versions of some functions to make porting to builder 10 simpler
version 5.1 11/9/2022 - merged csvgraph and common-files versions as had diverged.
version 6.0 14/9/2022 - split out pure C code - rprintf.cpp and getfloat.cpp made seperate files.
version 7.0 25/2/2024 - NAN added  as constant, comparison (== and !=) made to work as expected (ie NAN=NAN=true)
version 7.1 30/4/2024 - called expr0 (rather than expr1()) after "(" (either in expressions or function calls). means ? operator works in these situations..

*/

#include "expr-code.h"
#include <cstdlib> /* for malloc() and free() */
#include <string.h> /* for strcmp() etc */
#include <stdarg.h> /* for va_start etc */
#include <stdio.h>  /* for printf etc */
#include <time.h>
#include <ctype.h> /* isspace etc */
#include <math.h>
#include <float.h> /* _isnan() */
#include <values.h> /* MAXFLOAT etc */
#include <stdint.h> /* for uint32_t etc */
#include "interpolate.h"


#define  onepassreg /* if defined use functions that only make 1 pass over data for regression, otherwise 2 passes are used . Normally 1 pass is faster and gives same results as 2 pass */
#define STATIC_ASSERT(condition) extern int STATIC_ASSERT_##__FILE__##__LINE__[2*!!(condition)-1]  /* check at compile time and works in a global or function context see https://scaryreasoner.wordpress.com/2009/02/28/checking-sizeof-at-compile-time/ */


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
static double execute_rpn_from(size_t from);  /* execute rpn created by previous call to to_rpn() from specified start to end & return result */
static void opt_const(size_t from); /* if rpn from onwards evaluates to a constant then replace it just with a constant */

/* for string matching expressions */
static bool sexpr1(void); /* deal with & */
static bool sexpr2(void); /* deal with & */
static bool sexpr3(void); /* deal with (), $n= "..." or $n != "..." */
static bool scmp(int i); /* $n == "..." */
static char **sexpr_in; /* global variables - csv_parsed input & max nos columns */
static int sexpr_n;
static char *sexpr_cp; /* pointer into input expression string */
static bool sflag; /* true if expression is valid, false if not */


/* general purpose hash functions based on Fowler/Noll/Vo hash fnv-1a , with optional "mixing" at the end */
void hash_reset(uint32_t *h) // reset hash h to its initial value
{*h=2166136261;
}

static uint32_t hash_internal(const char *s,uint32_t  hash) // returns start_hash "plus" string s
{uint32_t newchar;
 while(s!=NULL && *s!= '\0')
		{newchar=(uint32_t)(*s++); /* get next character */
         hash^=newchar;
         hash*=16777619;   // "magic number" defined for FNV algorithm
        }
 return hash;
}

void hash_add(const char *s, uint32_t *h) // adds string s to hash h, updates h
{*h=hash_internal(s,*h);
}

uint32_t hash_str(const char *s) // returns hash of string  , always starting from the same initial value for hash
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
static int hash(const char *s)
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

pnlist lookup(const char *s) /* lookup s , returns pointer to struct snlist or NULL if not found */
    {pnlist np;
	 for(np=hashtab[hash(s)];np!=NULL;np=np->next)
        {if (strcmp(s,np->name)==0)
             return np; /* found it */
        }
     return NULL; /* not found */
    }

char *strsave(const char *s); /* forward declaration to avoid compiler warning */
char *strsave(const char *s)  /* save a copy of string s using malloc , returns NULL if no space */
    {char *p;
	 if((p=(char *)(malloc(strlen(s)+1))) != NULL)
        {strcpy(p,s); /* got space, copy string */
        }
     return p;
    }

pnlist install(const char *name) /* adds name to hashtab if it does not exist, returns pointer to entry created or NULL on error */
    {pnlist np;
     int hashval;
     if((np=lookup(name))==NULL)
        { /* not found , create new entry */
		 np=(pnlist)( malloc(sizeof(*np)));
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
static bool flag; /* used in recursive descent parser is true if expression was OK */
static unsigned int nos_vars; /* number of variables in expression [0 means expression will evaluate to a constant] */

static char *e; /* expression as a string */
static unsigned char rpn[MAXRPN];
static size_t rpnptr; /* index into rpn[] next token will be placed */
typedef enum _token {LOR,LAND,OR,XOR,AND,EQ,NEQ,LESS,GT,LE,GE,SHIFTR,SHIFTL,ADD,SUB,MULT,DIV,MOD,MINUS,LNOT,NOT,CONSTANT,VARIABLE,
                     ACOS,ASIN,ATAN,SIN,COS,TAN,LOG,EXP,SQRT,POW,COSH,SINH,TANH,FABS,QN,MAX,MIN} token;
#pragma option -w-pin /* ignore W8-61 Initialisation is only partially bracketed that would be created by struct functions below */
static struct functions       /* list of predefined functions/constants - must be in alphabetic order */
        { const char *keyword;
          int nosargs; /* number of arguments, eg pow(x,y) has 2 , -1 means its a constant (and brackets not then needed)*/
          token tk; /* corresponding token */
          double value; /* value if its a constant [by convention set to 0 if its not a constant]*/
        } funtab[]=
        { {"abs",1,FABS,0},
		  {"acos",1,ACOS,0},
		  {"asin",1,ASIN,0},
		  {"atan",1,ATAN,0},
		  {"cos",1,COS,0},
		  {"cosh",1,COSH,0},
		  {"exp",1,EXP,0},
		  {"log",1,LOG,0},
		  {"max",2,MAX,0},
		  {"min",2,MIN,0},
		  {"nan",-1,CONSTANT, __builtin_nan("0") },  // NAN is not considered by the compiler as a constant
		  {"pi",-1,CONSTANT, M_PI},
		  {"pow",2,POW,0},
		  {"sin",1,SIN,0},
		  {"sinh",1,SINH,0},
		  {"sqrt",1,SQRT,0},
		  {"tan",1,TAN,0},
		  {"tanh",1,TANH,0}
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
 // crprintf("check_function() %d functions in table, %d errors found\n",n,errs);
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
 size_t i;
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
 size_t i;
 addrpn_op(VARIABLE);
 u.p=v;
 for(i=0;i<sizeof(pnlist);++i) /* write out all 4/8 bytes of pointer */
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
{size_t start_rpnptr=rpnptr; /* allows us to check if the result of this is a constant */
 expr1();
 if(!flag) return;
 while(isspace(*e))++e; /* skip whitespace */
 while(*e=='?')
        {++e; /* pass over ? */
		 expr0(); /* expression value if true - need to call expr0 here as 1==2? 3==4?1:0 :1 is valid as is 1==2?2:3 ? 1: 0 )2nd done by while loop here */
         if(!flag) return;
         while(isspace(*e))++e; /* skip whitespace */
         if(*e!=':')
                {/* error : expected */
                 flag=false;
                 return;
                }
          ++e; /* pass over : */
          expr0(); /* expression value if false */
          if(!flag) return;
          while(isspace(*e))++e; /* skip whitespace */
          addrpn_op(QN);
          opt_const(start_rpnptr); /* see if we can optimise this all as a constant */
        }
}

static void expr1(void) /* lowest priority binary operator  || */
{size_t start_rpnptr=rpnptr; /* allows us to check if the result of this is a constant */
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
{size_t start_rpnptr=rpnptr; /* allows us to check if the result of this is a constant */
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
{size_t start_rpnptr=rpnptr; /* allows us to check if the result of this is a constant */
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
{size_t start_rpnptr=rpnptr; /* allows us to check if the result of this is a constant */
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
{size_t start_rpnptr=rpnptr; /* allows us to check if the result of this is a constant */
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
{size_t start_rpnptr=rpnptr; /* allows us to check if the result of this is a constant */
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
{size_t start_rpnptr=rpnptr; /* allows us to check if the result of this is a constant */
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
{size_t start_rpnptr=rpnptr; /* allows us to check if the result of this is a constant */
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
{size_t start_rpnptr=rpnptr; /* allows us to check if the result of this is a constant */
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
{size_t start_rpnptr=rpnptr; /* allows us to check if the result of this is a constant */
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
 size_t start_rpnptr=rpnptr; /* allows us to check if the result of this is a constant */
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
#ifdef Allow_dollar_T
				 /* allow $T1 etc to refer to trace 1 */
				 if(*e=='t' || *e=='T') ++e;   // just need to skip t here, code below checks that a number follows
#endif
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
         // crprintf("before lookup_fun(%s)\n",start);
         i=lookup_fun(start);
         // crprintf("lookup_fun(%s) returns %d\n",start,i);
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
                        {expr0(); /* get an expression */
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
         /*crprintf("constant %g\n",d);*/
         addrpn_constant(d);
        }
 else if(*e=='(')
        { /* bracketed expression */
         ++e;
         expr0();
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
{size_t i,j;
 union u_tagc {
 unsigned char c[sizeof(double)];
 double d;
 } uconst;
 union u_tagv {
 unsigned char c[sizeof(pnlist)];
 pnlist p;
 } uvar;
 if(flag)
		crprintf("Flag=true.  rpn=");
 else
        crprintf("Flag=false. rpn=");
 for(i=0;i<rpnptr;++i)
        {switch((token)(rpn[i]))
               {
                case LOR: crprintf("<LOR>"); break;
                case LAND: crprintf("<LAND>"); break;
                case OR: crprintf("<OR>"); break;
                case XOR: crprintf("<XOR>"); break;
                case AND: crprintf("<AND>"); break;
                case EQ: crprintf("<EQ>"); break;
                case NEQ: crprintf("<NEQ>"); break;
                case LESS: crprintf("<LESS>"); break;
                case GT: crprintf("<GT>"); break;
                case LE: crprintf("<LE>"); break;
                case GE: crprintf("<GE>"); break;
                case SHIFTR: crprintf("<SHIFTR>"); break;
                case SHIFTL: crprintf("<SHIFTL>"); break;
                case ADD: crprintf("<ADD>"); break;
                case SUB: crprintf("<SUB>"); break;
                case MULT: crprintf("<MULT>"); break;
                case DIV: crprintf("<DIV>"); break;
                case MOD: crprintf("<MOD>"); break;
                case MINUS: crprintf("<MINUS>"); break;
                case LNOT:  crprintf("<LNOT>"); break;
                case NOT:   crprintf("<NOT>"); break;
                /* functions */
                case ACOS:   crprintf("<ACOS>"); break;
                case ASIN:   crprintf("<ASIN>"); break;
                case ATAN:   crprintf("<ATAN>"); break;
                case SIN:   crprintf("<SIN>"); break;
                case COS:   crprintf("<COS>"); break;
                case TAN:   crprintf("<TAN>"); break;
                case LOG:   crprintf("<LOG>"); break;
                case EXP:   crprintf("<EXP>"); break;
                case SQRT:   crprintf("<SQRT>"); break;
                case POW:   crprintf("<POW>"); break;
                case COSH:   crprintf("<COSH>"); break;
                case SINH:   crprintf("<SINH>"); break;
                case TANH:   crprintf("<TANH>"); break;
                case FABS:   crprintf("<ABS>"); break;
                case MAX:    crprintf("<MAX>"); break;
                case MIN:    crprintf("<MIN>"); break;

                case QN: crprintf("<QN>"); break;

                case CONSTANT:  crprintf("<CONSTANT=");
								for(j=0;j<sizeof(double);++j)
                                        {
                                         crprintf("%02x",rpn[++i]);
                                         uconst.c[j]=rpn[i];
                                        }
                                crprintf(":%g>",uconst.d);
                         break;
                case VARIABLE:  crprintf("<VARIABLE=");
                                for(j=0;j<sizeof(pnlist);++j)
                                        {
                                         crprintf("%02x",rpn[++i]);
                                         uvar.c[j]=rpn[i];
                                        }
                                crprintf(":%s:%g>",uvar.p->name,uvar.p->value);
                        break;
				// default:  crprintf("<??%02x>",rpn[i]); break;
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

void opt_const(size_t from) /* if rpn from specified location to end is a constant then just replace it with a constant */
{bool t_flag=flag; /* save a copy of flag and restore it at the end */
 double d=execute_rpn_from(from);
 if(flag & rpn_constant)
		{rpnptr=from; /* can delete entire expression and replace with a constant */
         addrpn_constant(d);
        }
 flag=t_flag;
}

static double execute_rpn_from(size_t from)  /* execute rpn created by previous call to to_rpn() from specified start to end & return result */
{size_t i,j;
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
				case LOR: stack[sp-2]=(stack[sp-2]!=0) || (stack[sp-1]!=0); sp-=1; break;
				case LAND: stack[sp-2]=(stack[sp-2]!=0) && (stack[sp-1]!=0); sp-=1; break;
                        /* bitwise operators work on uisigned ints (32 bits) which can be exactly represented as doubles */
                case OR: stack[sp-2]= (unsigned int)(stack[sp-2]) | (unsigned int)(stack[sp-1]); sp-=1; break;
                case XOR: stack[sp-2]= (unsigned int)(stack[sp-2]) ^ (unsigned int)(stack[sp-1]); sp-=1; break;
				case AND: stack[sp-2]= (unsigned int)(stack[sp-2]) & (unsigned int)(stack[sp-1]); sp-=1; break;
				case EQ:  if(isnan(stack[sp-1]) || isnan(stack[sp-2]) )
							{// deal with NAN's as special case as we want NAN=NAN to give true
							 stack[sp-2]=(isnan(stack[sp-1]) && isnan(stack[sp-2]) ) ;
							}
						  else
							{// not NAN's - can use standard == operator
							 stack[sp-2]=stack[sp-2] == stack[sp-1];
							}
						  sp-=1; break;
				case NEQ: if(isnan(stack[sp-1]) || isnan(stack[sp-2]) )
							{// deal with NAN's as special case as we want NAN!=NAN to give false
							 stack[sp-2]=!(isnan(stack[sp-1]) && isnan(stack[sp-2]) ) ;
							}
						  else
							{// not NAN's - can use standard != operator
							 stack[sp-2]=stack[sp-2] != stack[sp-1];
							}
						  sp-=1; break;
                case LESS: stack[sp-2]=stack[sp-2] < stack[sp-1]; sp-=1; break;
                case GT: stack[sp-2]=stack[sp-2] > stack[sp-1]; sp-=1; break;
                case LE: stack[sp-2]=stack[sp-2] <= stack[sp-1]; sp-=1; break;
                case GE: stack[sp-2]=stack[sp-2] >= stack[sp-1]; sp-=1; break;
                case SHIFTR: stack[sp-2]= (unsigned int)(stack[sp-2]) >> (unsigned int)(stack[sp-1]); sp-=1; break;
                case SHIFTL: stack[sp-2]= (unsigned int)(stack[sp-2]) << (unsigned int)(stack[sp-1]); sp-=1; break;
                case ADD: stack[sp-2]+=stack[sp-1]; sp-=1; break;
                case SUB: stack[sp-2]-=stack[sp-1]; sp-=1; break;
                case MULT: stack[sp-2]*=stack[sp-1]; sp-=1; break;
				case DIV: if(isnan(stack[sp-1]) || isnan(stack[sp-2]) ) stack[sp-2]=NAN;
						   else if(stack[sp-1]!=0) stack[sp-2]/=stack[sp-1];
                           else if(stack[sp-2]!=0) stack[sp-2]=MAXFLOAT; // x/0 = big  - cannot use MAXDOUBLE as use floats for values elsewhere
                           else stack[sp-2]=0; // 0/0 = 0 here (should perhaps be NAN)
                          sp-=1; break;
                case MOD: stack[sp-2]=fmod(stack[sp-2],stack[sp-1]); sp-=1; break; // mod(?,0) is defined to be 0 by fmod so no special cases need to be trapped
                case MINUS: stack[sp-1] = -stack[sp-1]; break;
				case LNOT:  stack[sp-1] = (stack[sp-1]!=0)?0.0:1.0 ; break;
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
				case QN: /* ?: */ stack[sp-3]= (stack[sp-3]!=0) ? stack[sp-2] : stack[sp-1]; sp-=2; break;

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
										stack[sp++]=get_dollar_var_value(uvar.p); /* get variable of $ variable */
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



/* functions to deal with time input */
/* this version uses uint64 and long double to offer reasonable speed, high resolution with no possibility of an overflow */
long double gethms(char *s)
{/* read a time of the format hh:mm:ss.s , returns it as a long double value in seconds */
 /* if just a number is found this will be treated as seconds (which can include a decimal point and digits after the dp)
	 This is not a general purpose numeric input routine, it does not accept a sign, nor an exponent,
	  but it does accept mumbers of the type nnn.nnn (overall its limited by long double to a total of 18 sf)
	if aa:bb is found this will be treated as aa mins and bb secs  (which can include a decimal point and digits after the dp)
	if aa:bb:cc is found this will be treated as aa hours, bb mins and cc secs (which can include a decimal point and digits after the dp)
	returns 0 if does not start with a number, otherwise converts as much as possible based on the above format
	This means in particular that trailing whitespace and "'s are ignored
	23:59:59 => 86399 secs. A long double allows lots of digits after the dp, or a time purely given in seconds.
 */
 uint64_t sec1=0;  // sec1 is current set of digits, sec is previous total  uint64 allows 18sf for sec1 which matches long doubles.
 long double sec=0;
 int power10=0; // exponent
 if(!isdigit(*s)) return 0; /* must start with a number */
 sec1=(uint64_t)(*s++ -'0'); // ascii->decimal for 1st digit
 while(isdigit(*s))
	{if(sec1==0)
		 sec1=(uint64_t)(*s++ -'0'); // ascii->decimal for 1st digit
	 else
		{if( (sec1&UINT64_C(0xf000000000000000)) == 0) // 0x1000000000000000=1,152,921,504,606,846,976 so 18sf min
			sec1=sec1*10+(uint64_t)(*s++ -'0'); // ascii->decimal in general
		 else
			{s++;
			 power10++; // too many digits for uint64 - keep track of decimal point
			}
		}
	if(*s==':' && isdigit(s[1]))
		{ // previous must have been minutes (or hours)  so multiply by 60 to get secs [ or mins]
		 if(power10!=0 )
			{sec=(sec+(long double)sec1*pow10l(power10))*60.0;
			 power10=0;
			}
		 else
			{sec=(sec+(long double)sec1)*60.0;
			}
		 sec1=0; // ready to get next set of digits
		 ++s; // skip :
		}
	}
 // add in extra seconds from sec1
 if(power10!=0)
	{sec+= (long double)sec1*pow10l(power10);
	}
 else
	{sec+=(long double)sec1;
	}
 if(*s=='.')
	{ // seconds contains dp , so we now need to keep track of digits after dp and watch out for uint64 overflowing
	 ++s; // skip dp
	 sec1=0;   // ready to get digits after dp
	 power10=0;
	 while(isdigit(*s) && (sec1&UINT64_C(0xf000000000000000)) == 0 )
		{sec1=sec1*10+(uint64_t)(*s++ -'0');
		 power10++; // keep track of decimal point position
		}
	 if(isdigit(*s) && *s>='5') sec1++; // round if more digits present
	 return sec+(long double)sec1/pow10l(power10);     // if digits after dp
	}
 return sec;  // if seconds is an integer
}

static unsigned int days=0;
static long double last_time_secs=0;
static bool skip=false; // skip 1st number in a big step
void reset_days(void)  /* reset static variables for gethms_days() - should be used before using gethms_days() to read times from a file  */
{days=0;
 last_time_secs=0;
 skip=true; // believe 1st value
}

long double gethms_days(char *s) /* read time in format hh:mm:ss.s , assumed to be called in sequence and accounts for days when time wraps around. Returns secs, or -ve number on error */
	/* this returns a long double as we could have a lot of days and we would quickly run out of resolution with a float */
{long double t;
 if(!isdigit(*s)) return -1; /* should start with a number, return -1 to flag this is an error  */
 t=gethms(s);
 if(t<last_time_secs)
		{if(last_time_secs-t <= 1.0)
			skip=false; // allow up to 1 second backwards, data will need to be sorted to get correct order but is possibly OK (eg it could be due to a leap second?).
		 else if((last_time_secs - t) > 64800.0 )   // 64800=18.0*60.0*60.0
				{
				 days++; /* if time appears to have  gone > 18 hours backwards  assume this is because we have passed into a new day */
				 skip=false;  // assume time is valid
				}
		 else
				{if(!skip)
						{skip=true;
						 // crprintf("gethms_days(%s): last_time_secs=%.9g t=%.9g\n",s,last_time_secs,t);
						 t= -2;/* time has gone backwards for no reason - return -2 to indicate an error */
						}
				 else skip=false; // repeated , accept value  (could be due to a gap in the log [power cut?] that crossed midnight)
				}
		 // crprintf("gethms_day(%s) days=%u last_time=%.1f t=%.1f returns %.1f\n",s,days,last_time_secs,tc,t+24.0*60.0*60.0*(double)days);
		}
 else skip=false; // value appears to be OK
 if(t>=0)
		{ // -ve value of t indicate an error
		 last_time_secs=t;
		 if(days!=0)
			return t+86400.0*(long double)days; /* 86400=24.0*60.0*60.0 , 24 hours a day, 3600 secs in a hour */
		}
 return t;
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
void leastsquares_reg(float *y,float *x,size_t  start, size_t  end,double (*f)(float xparam),double (*g)(float xparam), double *a, double *b)
{double u=0,v=0,w=0,z=0,t=0; /* see my blue notebook for explanation dated 9-12-83 */
 size_t i;
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

/* generalised least squares - fit y=a*f(x)+b*g(x)+c - see below for a number of optimised versions for special cases of f(x) and g(x)
   The basic approach is described in http://www.geometrictools.com/Documentation/LeastSquaresFitting.pdf
   in section 3 Planar Fitting of 3D Points of Form( y; f(x; y))
   The solution of the matrix equation was done symbolically using  Maxima to get:
	   a=ratsimp(invert(matrix([s1,s2,s3],[s4,s5,s6],[s7,s8,s9])).matrix([s10],[s11],[s12])); which gives:
	   matrix([((s10*s5-s11*s2)*s9+(s11*s3-s10*s6)*s8+s12*s2*s6-s12*s3*s5)/((s1*s5-s2*s4)*s9+(s3*s4-s1*s6)*s8+(s2*s6-s3*s5)*s7)],
	           [-((s10*s4-s1*s11)*s9+(s11*s3-s10*s6)*s7+s1*s12*s6-s12*s3*s4)/((s1*s5-s2*s4)*s9+(s3*s4-s1*s6)*s8+(s2*s6-s3*s5)*s7)],
			   [((s10*s4-s1*s11)*s8+(s11*s2-s10*s5)*s7+s1*s12*s5-s12*s2*s4)/((s1*s5-s2*s4)*s9+(s3*s4-s1*s6)*s8+(s2*s6-s3*s5)*s7)])
        Then as a check
		a=ratsimp(matrix([s1,s2,s3],[s4,s5,s6],[s7,s8,s9]).ratsimp(invert(matrix([s1,s2,s3],[s4,s5,s6],[s7,s8,s9])).matrix([s10],[s11],[s12]))); gives
		matrix([s10],[s11],[s12]) which is what you would expect.
    s1=sum Xi^2
    s2=sum Xi*Yi
    s3=sum Xi
    s4=sum XiYi
    s5=sum Yi^2
    s6=sum Yi
    s7=sum Xi
    s8=sum Yi
    s9=sum 1
    s10=sum XiZi
    s11=sum YiZi
    s12=sum Zi
    And equation to be fitted is z=A*x+B*y+C but here we allow functions so
      y=A*f(x)+B*g(x)+C
*/
void leastsquares_reg3(float *y,float *x,size_t  start, size_t  end,double (*f)(float xparam),double (*g)(float xparam), double *a, double *b, double *c)
{long double s1=0,s2=0,s3=0,s4,s5=0,s6=0,s7,s8,s9=0,s10=0,s11=0,s12=0; /* see above, use long doubles for accuracy, but as x[] and y[] are float arrays it probably makes no difference */
 long double dom;
 size_t i;
 for(i=start;i<=end;++i) /* 1st calculate sums */
        {
         s1+=((*f)(x[i])) * ((*f)(x[i]));       /* *f is Xi, *g is Yi */
         s2+=((*f)(x[i])) * ((*g)(x[i]));
         s3+= ((*f)(x[i]));
         s5+=((*g)(x[i])) * ((*g)(x[i]));
         s6+= ((*g)(x[i]));
         s9+=1;
         s10+= y[i] * ((*f)(x[i]));
         s11+= y[i] * ((*g)(x[i]));
		 s12+= y[i];
        }
  s4=s2; /* some duplicates set here (I tried avoiding these in maxima but the equations were basically unchanged)*/
  s7=s3;
  s8=s6;
  /* calculate demoninator as its the same in all cases */
  dom=((s1*s5-s2*s4)*s9+(s3*s4-s1*s6)*s8+(s2*s6-s3*s5)*s7);    /* equation from Maxima - see comments at head of this function */
  if(dom==0)
		{/* cannot divide by zero so just set a default result */
         *a=*b=*c=0;
         return;
        }
  *a=(double)(((s10*s5-s11*s2)*s9+(s11*s3-s10*s6)*s8+s12*s2*s6-s12*s3*s5)/dom);  /* equations from Maxima - see comments at head of this function */
  *b=(double)(  -((s10*s4-s1*s11)*s9+(s11*s3-s10*s6)*s7+s1*s12*s6-s12*s3*s4)/dom);
  *c= (double)(((s10*s4-s1*s11)*s8+(s11*s2-s10*s5)*s7+s1*s12*s5-s12*s2*s4)/dom);
  return;
}

/* variant of above that fits that rational function y=(a+bx)/(1+cx)
   Note this cannot be done with leastsquares_reg3 as g() uses y as well as x.
   y=a+b*x-c*x*y   so f(x)=x and g(x,y)=x*y
*/
void leastsquares_rat3(float *y,float *x,size_t  start, size_t  end, double *a, double *b, double *c)
{long double s1=0,s2=0,s3=0,s4,s5=0,s6=0,s7,s8,s9=0,s10=0,s11=0,s12=0; /* see above, use long doubles for accuracy, but as x[] and y[] are float arrays it probably makes no difference */
 long double dom;
 size_t  i;
 for(i=start;i<=end;++i) /* 1st calculate sums */
		{long double f=x[i],yi=y[i];
		 long double g=f*yi;
		 s1+=f*f;
		 s2+=f*g;
		 s3+=f;
		 s5+=g*g;
		 s6+=g;
		 s9+=1;
		 s10+= yi*f;
		 s11+= yi*g;
		 s12+= yi;
		}
  s4=s2; /* some duplicates set here (I tried avoiding these in maxima but the equations were basically unchanged)*/
  s7=s3;
  s8=s6;
  /* calculate demoninator as its the same in all cases */
  dom=((s1*s5-s2*s4)*s9+(s3*s4-s1*s6)*s8+(s2*s6-s3*s5)*s7);    /* equation from Maxima - see comments at head of this function */
  if(dom==0)
		{/* cannot divide by zero so just set a default result */
		 *a=*b=*c=0;
		 return;
		}
  // compared to leastsquares_reg3 above a=c, b=a, c=-b
  *b=(double)(((s10*s5-s11*s2)*s9+(s11*s3-s10*s6)*s8+s12*s2*s6-s12*s3*s5)/dom);  /* equations from Maxima - see comments at head of this function */
  *c= (double)( ((s10*s4-s1*s11)*s9+(s11*s3-s10*s6)*s7+s1*s12*s6-s12*s3*s4)/dom);
  *a= (double)(((s10*s4-s1*s11)*s8+(s11*s2-s10*s5)*s7+s1*s12*s5-s12*s2*s4)/dom);
  return;
}

/* linear regression line forced to pass through point x=a,y=b ; does not calculate r^2 as not obvious how to use this as point a,b impacts the fit*/
void lin_reg_through_a_b(float a, float b,float *y,float *x, size_t  start, size_t  end, double *m, double *c)
/* this version just makes 1 pass over data, uses "updating algorithm" for higher accuracy [works well with floats - but doubles used here for best accuracy]*/
/* underlying equation for the best straight line through the origin=sum(XiYi)/sum(Xi^2) from Yang Feng (Columbia Univ) Simultaneous Inferences, pp 18/20. */
{
 double meanx2=0,meanxy=0; /* mean x^2 , mean x*y */
 double xi,yi;
 size_t  i,N=0; /* N is count of items */
 for(i=start;i<=end;++i) /* only use 1 pass here - to calculate means directly */
		{++N;
		 xi = x[i]-a; // ofset x,y as we want line to pass through point (a,b)
		 yi = y[i]-b;
		 meanx2+= (xi*xi-meanx2)/(double) N;
		 meanxy+= (xi*yi-meanxy)/(double) N;
		}
 if(meanx2==0)
		{/* y is independent of x [if this is not trapped we get a divide by zero error trying to set m to infinity] or 0 or 1 point given */
		 *m=0.0 ;
		 *c=b; /* make sure line passes through point (a,b) */
		}
 else   {/* have a valid line */
		 double rm;
		 rm=(meanxy)/(meanx2);
		 *m=rm;
		 *c=b-rm*a; /* force to pass though point a,b */
		 }
}

/* Least squares fit to a straight line y=m*x+c
   Written by Peter Miler 2012
   Uses "updating formula" so gives accurate results with 1 pass through data
   Option to use Geometric mean regression (GMR) also called Triangular regression see equation 18 in
   	"Least Squares Methods for Treating Problems with Uncertainty in x and y", Joel Tellinghuisen,
   	Anal. Chem. 2020, 92, 10863-10871.
*/
void lin_reg_GMR(float *y,float *x, size_t start, size_t end, bool GMR,double *m, double *c, double *r2) /* linear regression */
{double meanx=0,meany=0; /* initial values set to mean that N=0 or N=1 do not need to be treated as special cases below */
 double meanx2=0,meanxy=0,meany2=0; /* mean x^2 , mean x*y and mean y^2 */
 double xi,yi;
 size_t i,N=0; /* N is count of items */
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
         if(GMR)
			{/* Geometric Mean regression  */
			 double gm;
			 gm=sqrt((meany*meany-meany2)/(meanx*meanx-meanx2));
		 	 if(meanxy<0) gm= -gm;
		 	 *m=gm;
		 	}
		 else
		 	{// normal least squares
         	 *m=rm;
         	}
         *c=meany-*m*meanx; /* y=mx+c so c=y-mx */
         rt=(meanxy-meanx*meany);
         rb=(meany2-meany*meany);
         // crprintf("rm=%g rt=%g rb=%g rt/rb=%g \n",rm,rt,rb,rt/rb);
         if(rb!=0)     /* trap divide by zero */
            *r2= rm * (rt/rb) ;
          else
            *r2=1.0;/* should be in range 0-1 */
         }
}

static inline double calc_err(bool rel_error, double yi,double c) // returns relative or abs error depending on state of rel_error
{if(rel_error)
	{// relative error
	 if(yi!=0)
	 	return fabs((yi-c)/yi);
	 else return fabs(c); // if yi==0 returns abs error (avoids divide by zero), 0 if no error
	}
 else
 	{// abs error
 	 return fabs(yi-c);
	}
}

// This function tries to fit a min absolute error or min abs relative error line y=m*x+c
// as a secondary objective where values of min abs error are equal aims to minimise sum abs error (or abs rel error) - this is so there is one unique solution
// This is a non-linear optimisation so initially does a number of line fits and uses these to find a likely range of m,c and
// then searches these ranges for the minimum error.
// Code designed so its normally fast, but it has a timeout just in case which should still give a reasonable result.
// This algorithm was created by Peter Miller   May 2020.
// if callback is not NULL its called regularly to provide an update on progress
void fit_min_abs_err_line(float *x, float *y,size_t nos_vals,bool rel_error,double *m_out, double *c_out,double *best_err_out,void (*callback)(size_t cnt,size_t maxcnt))
{bool first=true;
 double m,c,bestm=0,bestc=0,err,besterr=0,er,sumerr,bestsumerr=0;
 double minm=0, maxm=0;
 double minc=0,maxc=0;
 double r2;// for least squares line fit
#define NOS_PTS_TO_USE_SEGS 100 /* if more than this many points then we split data into "segments" to speed analysis */
#define NOS_SEGS 10 /* number of segments to split data into */
 double seg_max_x[NOS_SEGS],seg_max_y[NOS_SEGS]; // co-ords of max in each segment
 double seg_min_x[NOS_SEGS],seg_min_y[NOS_SEGS]; // co-ords of min in each segment
 double sx[NOS_SEGS*2],sy[NOS_SEGS*2]; // array of points containing min/max of all segments
 size_t seg_size=nos_vals/NOS_SEGS;
 size_t seg_end=seg_size; // end of next segment
 size_t seg_nos=0;
 clock_t start_t=clock();
 if(nos_vals<=2)
	{// Need at least 2 points to fit a straight line
 	 *m_out=0;
 	 *c_out=0;
 	 *best_err_out=0;
 	 return;
	}
if(callback!=NULL) callback(0,100);   // 0%
if(nos_vals<NOS_PTS_TO_USE_SEGS)
	{
 	 // we know that the min sum abs error line passes through at least two datapoints (re wikipedia article/thesis) [note here we select line with min abs error, NOT min sum abs error ]
	 // as there are only a few points we can test all possible pairs - this is very fast to do.
	for(size_t i1=0;i1<nos_vals-1;++i1)
		for(size_t i2=i1+1;i2<nos_vals;++i2)
			{if(x[i1]!=x[i2])
				{// find line from i1 to i2 as y=m*x+c
				 m=(y[i1]-y[i2])/(x[i1]-x[i2]);
				 c=y[i1]-m*x[i1];
				 // now update min/max m/c
				 if(first)
				 	{minm=maxm=m;
				 	 minc=maxc=c;
				 	}
				 else
				 	{if(m>maxm) maxm=m;
					 if(m<minm) minm=m;
					 if(c>maxc) maxc=c;
					 if(c<minc) minc=c;
					}
				 // now find abs error of this line
				 err=0;
				 sumerr=0;
				 for(size_t i=0;i<nos_vals;++i)
					{ er=calc_err(rel_error,y[i],(m*x[i]+c));
					  sumerr+=er;
					  if(er>err)
						{err=er; // max abs error of this line
	 				  	 if(!first && (err>besterr|| (err==besterr && sumerr>=bestsumerr))) break; // shortcut - error is already worse so no point continueing to look for a larger error
	 				  	}
	 				  if(!first && (err==besterr && sumerr>=bestsumerr)) break; // error already higher than best, go straight to next one
	 				 }
				 if(first || err<besterr || (err==besterr && sumerr<bestsumerr))
				 	{first=false;
					 besterr=err; // best result so far
					 bestsumerr=sumerr;
					 bestc=c;
					 bestm=m;
					}
				}
			}
	}
 else
 	{// lots of points find split range into a number of segments and for each segment find min/max, then use min/max of all segments as points to consider
	 // this focuses on finding the range of m,c to explore - its not very good for directly finding the best line
	// first find min/max y values (and corresponding x values) in each segment
	for(size_t i=0;i<nos_vals;++i)
		{if(first)
			{seg_max_x[seg_nos]=x[i];
			 seg_max_y[seg_nos]=y[i];
			 seg_min_x[seg_nos]=x[i];
			 seg_min_y[seg_nos]=y[i];
			 first=false;
			}
		if(y[i]>seg_max_y[seg_nos])
			{// new max
			 seg_max_x[seg_nos]=x[i];
			 seg_max_y[seg_nos]=y[i];
			}
		if(y[i]<seg_min_y[seg_nos])
			{// new min
			 seg_min_x[seg_nos]=x[i];
			 seg_min_y[seg_nos]=y[i];
			}
		if(i>=seg_end)
			{seg_end+=seg_size;
			 if(seg_nos<NOS_SEGS-1)
			 	{seg_nos++; // ensure array indices stay in range
			 	 first=true;
			 	}
			}
		}
	// copy min max values into sx/sy arrays for easier processing below
	for(size_t i=0;i<NOS_SEGS;++i)
		{sx[i]=seg_max_x[i];
		 sy[i]=seg_max_y[i];
		 sx[i+NOS_SEGS]=seg_min_x[i];
		 sy[i+NOS_SEGS]=seg_min_y[i];
		}
	// now search these
	first=true;
	for(size_t i1=0;i1<NOS_SEGS*2-1;i1++)
		{for(size_t i2=i1+1;i2<NOS_SEGS*2;i2++)
			{if(sx[i1]!=sx[i2])
				{// find line from i1 to i2 as y=m*x+c
				 m=(sy[i1]-sy[i2])/(sx[i1]-sx[i2]);
				 c=sy[i1]-m*sx[i1];
				 // now update min/max m/c
				 if(first)
				 	{minm=maxm=m;
				 	 minc=maxc=c;
				 	}
				 else
				 	{if(m>maxm) maxm=m;
					 if(m<minm) minm=m;
					 if(c>maxc) maxc=c;
					 if(c<minc) minc=c;
					}
				 // now find abs error of this line
				 err=0;
				 sumerr=0;
				 // 1st check min/max array as that may let us stop if error is large
				 for(size_t i=0;i<NOS_SEGS*2;i++)
					{ er=calc_err(rel_error,sy[i],(m*sx[i]+c));
					  sumerr+=er;
					  if(er>err)
						{err=er; // max abs error of this line
	 				  	 if(!first && (err>besterr|| (err==besterr && sumerr>=bestsumerr))) break; // shortcut - error is already worse so no point continueing to look for a larger error
	 				  	}
	 				  if(!first && (err==besterr && sumerr>=bestsumerr)) break; // error already higher than best, go straight to next one  // error already higher than best, go straight to next one
	 				 }
				 // if we get here we need to check all the points	(no need to reset err as points checked above are all on line so will be checked again)
		 		 if(first || err<besterr || (err==besterr && sumerr<bestsumerr))
		 			{// check every point to find max abs error
					 sumerr=0;
					 for(size_t i=0;i<nos_vals;++i)
						{ er=calc_err(rel_error,y[i],(m*x[i]+c));
						 sumerr+=er;
						 if(er>err)
							{err=er; // max abs error of this line
	 				  	 	 if(!first && (err>besterr || (err==besterr && sumerr>=bestsumerr))) break; // shortcut - error is already worse so no point continueing to look for a larger error
	 				  		}
	 				  	 if(!first && (err==besterr && sumerr>=bestsumerr)) break; // error already higher than best, go straight to next one  // error already higher than best, go straight to next one
	 				 	}
	 				}
				 if(first || err<besterr || (err==besterr && sumerr<bestsumerr))
				 	{first=false;
					 besterr=err; // best result so far
					 bestsumerr=sumerr;
					 bestc=c;
					 bestm=m;
					}
				}
			}
		}
	}
 if(callback!=NULL) callback(10,100);   // 10%
 lin_reg_GMR(y,x,0,nos_vals-1,false,&m,&c,&r2);
 // got fit
 // now find max abs error of this line
 err=0;
 sumerr=0;
 for(size_t i=0;i<nos_vals;++i)
	{ er=calc_err(rel_error,y[i],(m*x[i]+c));
	  sumerr+=er;
	  if(er>err)
		{err=er; // max abs error of this line
 	  	}
	 }

 if(err<besterr || (err==besterr && sumerr<bestsumerr))
  	{// got a lower error than previously
  	 besterr=err;
	 bestsumerr=sumerr;
  	 bestm=m;
  	 bestc=c;
	}
 lin_reg_GMR(y,x,0,nos_vals-1,true,&m,&c,&r2);
 // got GMR fit
 // now find max abs error of this line
 err=0;
 sumerr=0;
 for(size_t i=0;i<nos_vals;++i)
	{ er=calc_err(rel_error,y[i],(m*x[i]+c));
	  sumerr+=er;
	  if(er>err)
		{err=er; // max abs error of this line
 	  	}
	 }
 if(err<besterr || (err==besterr && sumerr<bestsumerr))
  	{// got a lower error than previously
  	 besterr=err;
	 bestsumerr=sumerr;
  	 bestm=m;
  	 bestc=c;
 	}
 /* now vary line a little to see if we can find a line with a smaller min abs deviation */
 if(besterr==0)
	{// all done (cannot do better than zero error)
	 *m_out=bestm;
	 *c_out=bestc;
	 *best_err_out=besterr;
	 return;
	}
 // use min/max m/c range found by fitting straight lines to all pairs of points, use 2000 steps between these for initial exploration
 if(callback!=NULL) callback(20,100);   // 20%
 double tm,tc;
 tm=maxm-minm; // range of m from fitting straight lines by pairs of points
 maxm=bestm+tm;
 minm=bestm-tm;
 tc=maxc-minc; // range of c from fitting straight lines by pairs of points
 maxc=bestc+tc;
 minc=bestc-tc;
 for(size_t im=0;im<=2000;++im)     // use ints for loop counters as mstep may be very small and give us resolution issues
	{m=minm+tm*(double)im/1000.0;  // from minm to maxm step tm/1000
	 for(size_t ic=0;ic<=2000;++ic)
		{c=minc+tc*(double)ic/1000.0;  // from minc to maxc step tc/1000
		 err=0;
		 if(nos_vals>=NOS_PTS_TO_USE_SEGS)
			{sumerr=0;
			 // we are using segments, 1st check min/max array as that may let us stop if error is large
			 for(size_t i=0;i<NOS_SEGS*2;i++)
				{ er=calc_err(rel_error,sy[i],(m*sx[i]+c));
				  sumerr+=er;
				  if(er>err)
					{err=er; // max abs error of this line
					 if(err>besterr|| (err==besterr && sumerr>=bestsumerr)) break; // shortcut - error is already worse so no point continueing to look for a larger error
					}
				  if(err==besterr && sumerr>=bestsumerr) break; // error already higher than best, go straight to next one
				 }
			}
		 if(err<besterr || (err==besterr && sumerr<bestsumerr))
			{// check every point to find max abs error
			 sumerr=0;
			 for(size_t i=0;i<nos_vals;++i)
				{er=calc_err(rel_error,y[i],(m*x[i]+c));
				 sumerr+=er;
				 if(er>err)
					{err=er; // max abs error of this line
					 if(err>besterr|| (err==besterr && sumerr>=bestsumerr)) break; // error already higher than best, go straight to next one
					}
				 if(err==besterr && sumerr>=bestsumerr) break; // error already higher than best, go straight to next one
				}
			}
		 if(err<besterr|| (err==besterr && sumerr<bestsumerr))
			{// got a lower error than previously
			 besterr=err;
			 bestsumerr=sumerr;
			 bestm=m;
			 bestc=c;
			}
		}
	}
 // having found area of best - zoom in , look +/-10 steps from previous best, dividing step by 2 each iteration
 size_t last_improvement_j=0;
 tm/=100.0;// tm/1000 was steps used above *10 to give new step
 tc/=100.0;
 for(size_t j=0;j<70;++j)// nos of zooms to do, normally we will run out of resolution and so break out of loop before this naturally terminates
  {
   minm=bestm-tm;
   maxm=bestm+tm;
   minc=bestc-tc;
   maxc=bestc+tc;
   volatile double mz=maxm+tm/20.0,cz=maxc+tc/20.0; // try and avoid optimiser being too clever in if statement below
   if(mz==maxm && cz==maxc) break; // out of resolution  on both loops, if one loop out of resolution we can still keep going refining the other one
   // now do general search
   crprintf("itn %-2d: search m from %g to %g step %g and c from %g to %g step %g\n",j,minm,maxm,tm/20.0,minc,maxc,tc/20.0);
   if(callback!=NULL) callback((unsigned int)(j+30),100);   // from 30% to 100%
   for(int im=-1;im<=40;++im)     // use ints for loop counters as mstep may be very small and give us resolution issues
	{if(im<0) m=bestm; // make sure we already test bestm exactly
	 else m=minm+tm*(double)im/20.0;  // from minm to maxm step tm/20
	 for(int ic=-1;ic<=40;++ic)
		{if(ic<0) c=bestc; // make sure we test bestc exactly
		 else c=minc+tc*(double)ic/20.0;  // from minc to maxc step tc/20
		 err=0;
		 if(nos_vals>=NOS_PTS_TO_USE_SEGS)
			{sumerr=0;
			 // we are using segments, 1st check min/max array as that may let us stop if error is large
			 for(size_t i=0;i<NOS_SEGS*2;i++)
				{ er=calc_err(rel_error,sy[i],(m*sx[i]+c));
				  sumerr+=er;
				  if(er>err)
					{err=er; // max abs error of this line
					 if(err>besterr|| (err==besterr && sumerr>=bestsumerr)) break; // shortcut - error is already worse so no point continueing to look for a larger error
					}
				  if(err==besterr && sumerr>=bestsumerr) break; // error already higher than best, go straight to next one
				 }
			}
		 if( err<besterr || (err==besterr && sumerr<bestsumerr))
			{// check every point to find max abs error
			 sumerr=0;
			 for(size_t i=0;i<nos_vals;++i)
				{ er=calc_err(rel_error,y[i],(m*x[i]+c));
				 sumerr+=er;
				 if(er>err)
					{err=er; // max abs error of this line
					 if(err>besterr|| (err==besterr && sumerr>=bestsumerr)) break; // error already higher than best, go straight to next one
					}
			     if(err==besterr && sumerr>=bestsumerr) break; // error already higher than best, go straight to next one
				}
			}
		 if(err<besterr || (err==besterr && sumerr<bestsumerr))
			{// got a lower error than previously
			 last_improvement_j=j;
			 besterr=err;
			 bestsumerr=sumerr;
			 bestm=m;
			 bestc=c;
			}
		}
	 }

   if(((j-last_improvement_j)>10 && (clock()-start_t)/(double)CLOCKS_PER_SEC>30.0) || (clock()- start_t)/(double)CLOCKS_PER_SEC>100.0)
		{crprintf(" Min abs error fit: Giving up with %d iterations without improvement at %g secs total\n",j-last_improvement_j,(clock()- start_t)/(double)CLOCKS_PER_SEC);
		 break; // give up if we are taking a very long time - > 30 secs and no improvement or > 100 secs
		}
   tc/=2.0; // have steps for next iteration
   tm/=2.0;
  }

 *m_out=bestm;
 *c_out=bestc;
 *best_err_out=besterr;
}


#ifdef  onepassreg
/* this version just makes 1 pass over data, uses "updating algorithm" for higher accuracy [works well with floats - but doubles used here for best accuracy]*/
void lin_reg(float *y,float *x, size_t start, size_t  end, double *m, double *c, double *r2) /* linear regression */
{double meanx=0,meany=0; /* initial values set to mean that N=0 or N=1 do not need to be treated as special cases below */
 double meanx2=0,meanxy=0,meany2=0; /* mean x^2 , mean x*y and mean y^2 */
 double xi,yi;
 size_t  i,N=0; /* N is count of items */
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
         // crprintf("rm=%g rt=%g rb=%g rt/rb=%g \n",rm,rt,rb,rt/rb);
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
void log_reg(float *y,float *x, float yoff,float xoff,size_t  start, size_t  end, double *m, double *c, double *r2) /* linear regression on logx / logy*/
/* yoff and xoff are subtracted from *y and *x this is useful as log(z) requires z>0 */
/* even though inputs are only floats, we use doubles internally to keep high accuracy */
/* this version uses 1 pass over data and should be as accurate as 2 pass solution */
{double meanx=0,meany=0; /* initial values set to mean that N=0 or N=1 do not need to be treated as special cases below */
 double meanx2=0,meanxy=0,meany2=0; /* mean x^2 , mean x*y and mean y^2 */
 double xi,yi;
 size_t  i,N=0; /* N is count of items */
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
         // crprintf("rm=%g rt=%g rb=%g rt/rb=%g \n",rm,rt,rb,rt/rb);
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
  // crprintf("log_reg() N=%d ssxx=%g\n",N,ssxx);
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
void log_lin_reg(float *y,float *x, float yoff,float xoff,size_t start, size_t  end, double *m, double *c, double *r2) /* linear regression on logy vs x*/
/* yoff and xoff are subtracted from *y and *x this is useful as log(z) requires z>0 */
/* even though inputs are only floats, we use doubles internally to keep high accuracy */
/* this version makes just 1 pass over data while still giving high accuracy */
{double meanx=0,meany=0; /* initial values set to mean that N=0 or N=1 do not need to be treated as special cases below */
 double meanx2=0,meanxy=0,meany2=0; /* mean x^2 , mean x*y and mean y^2 */
 double xi,yi;
 size_t  i,N=0; /* N is count of items */
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
         // crprintf("rm=%g rt=%g rb=%g rt/rb=%g \n",rm,rt,rb,rt/rb);
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
  // crprintf("log_lin_reg()x[start]=%g y[start]=%g x[end]=%g y[end]=%g N=%d ssxx=%g ssxy=%g meanx=%g meany=%g\n",x[start],y[start],x[end],y[end],N,ssxx,ssxy,meanx,meany);
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
double deriv(float *y,float *x,size_t  start, size_t  end, size_t  index) /* estimate dy/dx at index - only using data between start and end */
{ const int points_lin_reg=100; /* number of points we will do linear regression over - ideally symetrically about point */
  if(index<start || index>end || start==end) return 0; /* index out of range , or only 1 point (start=end)*/
  if(end-start > 2*points_lin_reg)
        {/* we have lots of data - we can use linear regression */
		 size_t  ds,de;
		 if(index<start+points_lin_reg/2)
				{/* want deriv near start, ensure we use a different e for every value of index to hopefully get a "smooth" deriv */
				 ds=start;
				 de=index+points_lin_reg/2;
				}
		 else if(index>end-points_lin_reg/2)
				{/* want deriv near end, ensure we use a different e for every value of index to hopefully get a "smooth" deriv  */
				 ds=index-points_lin_reg/2;
				 de=end;
				}
		 else
				{/* can do lin reg symetrically about index */
				 ds=index-points_lin_reg/2;
				 de=index+points_lin_reg/2;
				}
		 double m,c,r2;
		 /* void lin_reg(float *y,float *x, int start, int end, double *m, double *c, double *r2) - linear regression */
		 lin_reg(y,x,ds,de,&m,&c,&r2);
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
void log_diff_lin_reg(float *y,float *x,float xoff,size_t  start, size_t  end, double *m, double *c, double *r2)
 /* linear regression on log(dy/dx) vs x*/
 /* xoff is subtracted from *x */
 /* y is differentiated then loged before doing linear regression. differentiation effectively removes any constants so no need for yoff */
 /* even though inputs are only floats, we use doubles internally to keep high accuracy */
 /* uses double deriv(float *y,float *x,int start, int end, int index)  estimate dy/dx at index - only using data between start and end */
 /* this version makes 1 pass over data - which means we call deriv() once for every point */
{double meanx=0,meany=0; /* initial values set to mean that N=0 or N=1 do not need to be treated as special cases below */
 double meanx2=0,meanxy=0,meany2=0; /* mean x^2 , mean x*y and mean y^2 */
 double xi,yi;
 size_t  i,N=0; /* N is count of items */
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
         // crprintf("rm=%g rt=%g rb=%g rt/rb=%g \n",rm,rt,rb,rt/rb);
         if(rb!=0)     /* trap divide by zero */
            *r2= rm * (rt/rb) ;
          else
            *r2=1.0;/* should be in range 0-1 */
         }
}
#else
void log_diff_lin_reg(float *y,float *x,float xoff,size_t  start, size_t  end, double *m, double *c, double *r2)
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
  // crprintf("log_diff_lin_reg()x[start]=%g y[start]=%g x[end]=%g y[end]=%g N=%d ssxx=%g ssxy=%g meanx=%g meany=%g\n",x[start],y[start],x[end],y[end],N,ssxx,ssxy,meanx,meany);
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
 static char *null_str=(char *)("");
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
	{static char *null_str=(char *)("");
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


static const int FIRST_SIZE=256; /* initial size of line buffer start at eg 256, size is doubled if its too small */
#define MAX_LIN_SIZE 1048576 /* 1,048,576= 2^20 max allowed line size [ also limited by free RAM], avoids a silly line using all the available RAM */
static char *buf=NULL; /* input buffer */
static int  buf_size=0; /* input buffer size, 0 means not yet allocated, must be int as read via fgets() which uses int */
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
 int new_size;
#ifdef  DEBUG_readline
 crprintf("readline() called: buf_size=%d\n",buf_size);
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
         crprintf(" after call to fgets(), buf_size=%d\n",buf_size);
         crprintf(" cp[n-2]=0x%x cp[n-1]=0x%x\n",(int)(cp[n-2]),(int)(cp[n-1]));
         crprintf(" string=%s\n",buf);
#endif
         if(cpr==NULL)
                return NULL; /* if we hit EOF then return NULL */
         if(cp[n-1]==0x7f || (cp[n-2]=='\n' && cp[n-1]==0)) return buf; // normal return
         new_size=buf_size <<1; /* double size of buffer */
		 if(new_size > MAX_LIN_SIZE || (new_buf=(char *)realloc(buf,(size_t)new_size))==NULL)
				{/* line too long or realloc failed "eat" rest of line and return truncated line to caller */
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

float nextafterfp(float f) /* returns the next higher floating point number - should be in  C11 math.h */
{  union _f_andi32
		{float uf;
		 int32_t i; // must be signed for >>31 "trick" to work
		} f_i;
	f_i.uf=f;
	if((uint32_t)f_i.i== 0x80000000) f_i.i=0; // trap -0 and convert it to +0
	f_i.i += (f_i.i >> 31) | 1;   // this will add 1 for positive f and -1 for negative f
        // if(_isnan(f_i.uf)) return MINFLOAT; // just in case above does not trap all special cases
        // crprintf("nextafterf(%g) returns %g\n",f,f_i.uf);
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
		 i = (int32_t) x;
         f = x - (float)(i-1);
        }
   else
        { /* x>= 0 */
         if(x<=0.048705f) return 1.0f+x;/* exp(x)=1+x+x^2/2+x^3/6+...  , below has relative error 0.00237 so 0.048705 is sqrt(0.00237) */
         if(x>87.0f) return MAXFLOAT;  /* trap over and under flow */
         x *= 1.44269504f; /* 1/ln(2) as below give 2^x and we want e^x */
		 i = (int32_t)x;
         f = x - (float)i;   /* f=fractional part of x (i is integer part of x) */
        }
#if 1
 f_i.ui= (int32_t)(8388610.0f * (x+(3.43149624e-01f*f-3.48095891e-01f)*f+127.002342f));  /* all in 1 equation, otherwise identical to below */
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
 i = (int32_t)( x * 12102203.16156f );   /* 12102203.16156f = (2^23)/ln(2) - note this will not overflow a 32 bit int with x=87 */
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
        {y=x-1.0f;  /* set to optimium for order  4 main poly , using order 5 correction [which has same # fp ops as "main" calculation of order 4 */
         return (((-0.25f*y+0.3333333333f)*y-0.5f)*y+1.0f)*y; /* higher order approxm for use near x=1, exact at x=1 */
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
 f_i.i |= (uint32_t)(sign) & 0x80000000;/* put sign back on - as iteration below needs this (as x is signed) */
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
 f_i.i |= (uint32_t)(sign) & 0x80000000;/* put sign back on */
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
  // crprintf("sexpr() sflag=%s\n", P_BOOL(sflag));
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
#define MAX_REGEX 1000  /* sorry for fixed size, but don't really want to be allocating variable length strings here as may fragement memory */
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
                 // crprintf("sexpr3: ~ found regex=<%s> $n=<%s>\n",regex,sexpr_in[n-1]);
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
 // crprintf("scmp(%d) ip=<%s> sexpr_cp=<%s>\n",i,ip,sexpr_cp);
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
{ float t90val[]={      6.314f,2.920f,2.353f,2.132f,2.015f,
						1.943f,1.895f,1.860f,1.833f,1.812f,
						1.796f,1.782f,1.771f,1.761f,1.753f,
						1.746f,1.740f,1.734f,1.729f,1.725f,
						1.721f,1.717f,1.714f,1.711f,1.708f,
						1.706f,1.703f,1.701f,1.699f,1.697f,
						1.684f,1.671f,1.658f,1.645f
						};
   float t90dt[]={      1.0f,2.0f,3.0f,4.0f,5.0f,
						6.0f,7.0f,8.0f,9.0f,10.0f,
						11.0f,12.0f,13.0f,14.0f,15.0f,
						16.0f,17.0f,18.0f,19.0f,20.0f,
						21.0f,22.0f,23.0f,24.0f,25.0f,
						26.0f,27.0f,28.0f,29.0f,30.0f,
                        40.0f,60.0f,120.0f,MAXFLOAT
						};
STATIC_ASSERT(ELEMENTS_IN_ARR(t90dt) == ELEMENTS_IN_ARR(t90val));

 // float interp1D_f(float *xa, float *ya, int size, float x, bool clip)
 return interp1D_f(t90dt,t90val,ELEMENTS_IN_ARR(t90dt),dt,true);
}

float t99(int dt) /* returns t score for checking both upper and lower limits at 99% certainty - from pp154 Texas Instruments TI-51-III owners manual */
                  /* note if we are only checkking an upper or a lower limit this is t1sided99_5 the 99.5% limit */
{ float t99val[]={      63.657f,9.925f,5.841f,4.604f,4.032f,
						3.707f,3.499f,3.355f,3.250f,3.169f,
						3.106f,3.055f,3.012f,2.977f,2.947f,
						2.921f,2.898f,2.878f,2.861f,2.845f,
						2.831f,2.819f,2.807f,2.797f,2.787f,
						2.779f,2.771f,2.763f,2.756f,2.750f,
                        2.704f,2.660f,2.617f,2.576f
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

 // float interp1D_f(float *xa, float *ya, int size, float x, bool clip)
 return interp1D_f(t99dt,t99val,ELEMENTS_IN_ARR(t99dt),dt,true);
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


