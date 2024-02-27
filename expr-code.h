/* header file for expression handling code

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
#ifndef _expr_code_h
#define _expr_code_h
#include <stdio.h>  /* for FILE * - avoids having to fix order of includes */
#include "stdint.h" /* for uint32_t etc */
#include <stdarg.h> /* for va_start etc */
#include <stdbool.h>
#include "rprintf.h"

#ifdef __cplusplus
  extern "C" {
#endif 
#ifndef NO_MIN_MAX      /* allows us to not define these if they are defined elsewhere - eg in the STL */
#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))
#endif

#define Allow_dollar_vars_in_expr /* if defined allow variables of $1 .. like awk in xepr */
#define Allow_dollar_T /* if defined (as well as above) allow variables like $T1 for "trace 1" */


/* general purpose hash functions based on Fowler/Noll/Vo hash fnv-1a , with optional "mixing" at the end */
void hash_reset(uint32_t *h); // reset hash h to its initial value
void hash_add(const char *s, uint32_t *h); // adds string s to hash h, updates h
uint32_t hash_str(const char *s); // returns hash of string  , always starting from the same initial value for hash

/* table lookup code based on K&R The C programming language (1st Ed) section 6.6 */

typedef struct snlist
    { /* basic table entry */
      char *name;
      double value;
      struct snlist *next; /* pointer to next entry in chain at this hash value */
    } nlist, *pnlist;

pnlist lookup(const char *s); /* lookup s , returns pointer to struct snlist or NULL if not found */
pnlist install(const char *name); /* adds name to hashtab if it does not exist, returns pointer to entry created or NULL on error */
#ifdef Allow_dollar_vars_in_expr
double get_dollar_var_value(pnlist p); /* get value of $ variable */
#endif


long double gethms(char *s); /* read a time of format hh:mm:ss.s , returns time in seconds */
void reset_days(void);  /* reset static variables for gethms_days() - should be used before using gethms_days() to read times from a file  */
long double gethms_days(char *s); /* read time in format hh:mm:ss.s , assumed to be called in sequence and accounts for days when time wraps around. Returns secs */


double s_to_double(char *s); /* converts s to double if its a number, otherwise returns NAN */

void to_rpn(char *expression); /* convert expression to rpn */
void assign_expr(char *expression); /* deal with assignments , evaluates expression and stores result in variable ; allows multiple assignments on one line */
bool last_expression_ok(void); /* returns true if last expression was correct in syntax , false otherwise */
void print_rpn(void); /* for debugging - print out rpn "code" created by last call to to_rpn() or optimise_rpn() */
void optimise_rpn(void); /* optimise rpn from previous call to to_rpn() */
double execute_rpn(void); /* execute  rpn from a previous call to to_rpn() it & return the resultant value , 0 (and flag false) on error */
bool check_function_tab(void); /* returns true if function table is valid, false if not */

bool regex_match(char *regex, char *text); /* regex .^$* as special character (.=any char, ^ start, $ end, *=0+ occurances of previous char + = 1+ occurances */
bool sexpr(char *in[],int maxfields,char *sexpression); /* see if line (split up by parsecsvmatches sexpression matches string expression */
bool last_sexpr_ok(void); /* returns true if last sexpr expression was correct in syntax , false otherwise */

#define P_BOOL(b) ((b)?"true":"false") /* use with a %s param in printf to print a bool out */
#define ELEMENTS_IN_ARR(x) (sizeof(x)/sizeof(x[0])) /* number of elements in array x */

/* generalised least squares - fit y=a*f(x)+b*g(x) - see below for a number of optimised versions for special cases of f(x) and g(x) */
void leastsquares_reg(float *y,float *x,size_t start, size_t  end,double (*f)(float xparam),double (*g)(float xparam), double *a, double *b);
/* generalised least squares - fit y=a*f(x)+b*g(x)+c  */
void leastsquares_reg3(float *y,float *x,size_t  start, size_t  end,double (*f)(float xparam),double (*g)(float xparam), double *a, double *b, double *c);
void leastsquares_rat3(float *y,float *x,size_t  start, size_t  end, double *a, double *b, double *c); /* fits y=(a+bx)/(1+cx) */
/* specialised versions of above - faster and more accurate that using general version above */
void lin_reg_through_a_b(float a, float b,float *y,float *x, size_t  start, size_t  end, double *m, double *c); /* y=mx */
void lin_reg_GMR(float *y,float *x, size_t  start, size_t  end, bool GMR,double *m, double *c, double *r2); // lin reg optional GMR
void fit_min_abs_err_line(float *x, float *y,size_t nos_vals,bool rel_error,double *m_out, double *c_out,double *best_err_out,void (*callback)(size_t cnt,size_t maxcnt)); // fit min rel/abs error line
void lin_reg(float *y,float *x, size_t  start, size_t  end, double *m, double *c, double *r2); /* linear regression */
void log_reg(float *y,float *x, float yoff,float xoff,size_t  start, size_t  end, double *m, double *c, double *r2); /* linear regression on logx / logy*/
/* yoff and xoff are subtracted from *y and *x this is useful as log(z) requires z>0 */
void log_lin_reg(float *y,float *x, float yoff,float xoff,size_t  start, size_t  end, double *m, double *c, double *r2); /* linear regression on logy vs x*/
/* yoff and xoff are subtracted from *y and *x this is useful as log(z) requires z>0 */
void log_diff_lin_reg(float *y,float *x,float xoff,size_t  start, size_t  end, double *m, double *c, double *r2); /* linear regression on log(dy/dx) vs x*/
double deriv(float *y,float *x,size_t  start, size_t  end, size_t  index); /* estimate dy/dx at index - only using data between start and end */



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

float nextafterfp(float f); /* returns the next higher floating point number - should be in  C11 math.h */

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


 #ifdef __cplusplus
    }
 #endif
 
#endif
