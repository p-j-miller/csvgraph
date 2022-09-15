//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
/* getfloat.cpp

 c++ as functions are overloaded with different 1st arguments
 
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
 #include "getfloat.h"
 #include <values.h> /* MAXFLOAT etc */

 /* next function is used to support conversion to unicode vcl see https://blogs.embarcadero.com/migrating-legacy-c-builder-apps-to-c-builder-10-seattle/
*/
#define STR_CONV_BUF_SIZE 2000 // the largest string you may have to convert. depends on your project

static char* __fastcall AnsiOf(wchar_t* w)
{
	static char c[STR_CONV_BUF_SIZE];
	memset(c, 0, sizeof(c));
	WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, w, (int)wcslen(w), c, STR_CONV_BUF_SIZE, (nullptr),(nullptr));
	return(c);
}

 bool getfloat(char *s, float *d)
 /*reads a floating point number returns true if valid - allows whitespace as well as a number , d=0 on error */
{double r=0;
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
 *d=(float)r;
 return true;
}

bool getfloatgt0(char *s, float *d)
 /*reads a floating point number thats >0 returns true if valid - allows whitespace as well as a number , d=0 on error */
{double r=0;
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
 *d=(float)r;
 return true;
}

bool getfloatge0(char *s, float *d)
 /*reads a floating point number thats >=0 returns true if valid - allows whitespace as well as a number , d=0 on error */
{double r=0;
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
 *d=(float)r;
 return true;
}

// functions like above that take wchar_t *
bool getfloat(wchar_t *s, float *d)
{return getfloat(AnsiOf(s),d);
}
bool getfloatgt0(wchar_t *s, float *d)
{ return getfloatgt0(AnsiOf(s),d);
}
bool getfloatge0(wchar_t *s, float *d)
{ return getfloatge0(AnsiOf(s),d);
}

