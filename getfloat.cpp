//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
/* getfloat.cpp

 c++ as functions are overloaded with different 1st (wchar * or char *) & 2nd (float * or double *) arguments
 
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
 #include "atof.h" /* for fast_strtof() */

 /* next function is used to support conversion to unicode vcl see https://blogs.embarcadero.com/migrating-legacy-c-builder-apps-to-c-builder-10-seattle/
*/
#define STR_CONV_BUF_SIZE 2000 // the largest string you may have to convert. depends on your project

static char* __fastcall Utf8Of(wchar_t* w)       /* convert to utf-8 encoding */
{
	static char c[STR_CONV_BUF_SIZE];
	memset(c, 0, sizeof(c));
	WideCharToMultiByte(CP_UTF8, 0, w, -1, c, STR_CONV_BUF_SIZE-1, nullptr, nullptr);       /* size-1 ensure result is null terminated */
	return(c);
}

 bool getdouble(char *s, double *d)
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
 if(!valid  )
        {*d=0.0;
         return false; /* not valid or too big */
        }
 /* valid */
 *d=r;
 return true;
}

bool getdouble(wchar_t *s, double *d)
{return getdouble(Utf8Of(s),d);
}

 bool getfloat(char *s, float *d)
 /*reads a floating point number returns true if valid - allows whitespace as well as a number , d=0 on error */
{float r=0;
 bool valid=false;
 while(isspace(*s))++s; /* skip whitespace */
 if(isdigit(*s)|| *s=='.' || *s=='-' || *s=='+')
        {/* number must start with decimal point or a digit or a sign (+/-)*/
		 r=fast_strtof(s,&s);       /* read floating point number */
         while(isspace(*s))++s; /* skip whitespace */
         if(*s=='\0') valid=true; /* whitespace number whitespace */
		}
 if(!valid  )
        {*d=0.0;
         return false; /* not valid or too big */
        }
 /* valid */
 *d=(float)r;
 return true;
}

bool getfloatgt0(char *s, float *d)
 /*reads a floating point number thats >0 returns true if valid - allows whitespace as well as a number , d=0 on error */
{float r=0;
 bool valid=false;
 while(isspace(*s))++s; /* skip whitespace */
 if(isdigit(*s)|| *s=='.' || *s=='+')
        {/* number must start with decimal point or a digit or a + sign */
		 r=fast_strtof(s,&s);       /* read floating point number */
         while(isspace(*s))++s; /* skip whitespace */
         if(*s=='\0') valid=true; /* whitespace number whitespace */
        }
 if(!valid || r<=0 )
        {*d=0.0;
         return false; /* not valid or too big, or <=0 */
        }
 /* valid */
 *d=(float)r;
 return true;
}

bool getfloatge0(char *s, float *d)
 /*reads a floating point number thats >=0 returns true if valid - allows whitespace as well as a number , d=0 on error */
{float r=0;
 bool valid=false;
 while(isspace(*s))++s; /* skip whitespace */
 if(isdigit(*s)|| *s=='.' || *s=='+')
        {/* number must start with decimal point or a digit or a + sign */
		 r=fast_strtof(s,&s);       /* read floating point number */
         while(isspace(*s))++s; /* skip whitespace */
         if(*s=='\0') valid=true; /* whitespace number whitespace */
        }
 if(!valid || r<0 )
        {*d=0.0;
         return false; /* not valid or too big, or <0 */
        }
 /* valid */
 *d=(float)r;
 return true;
}

// functions like above that take wchar_t *
bool getfloat(wchar_t *s, float *d)
{return getfloat(Utf8Of(s),d);
}
bool getfloatgt0(wchar_t *s, float *d)
{ return getfloatgt0(Utf8Of(s),d);
}
bool getfloatge0(wchar_t *s, float *d)
{ return getfloatge0(Utf8Of(s),d);
}

// now duplicate all these with double return type (C++ allows the reuse of the same function names)
// =================================================================================================

 bool getfloat(char *s, double *d)
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
 if(!valid  )
        {*d=0.0;
		 return false; /* not valid */
        }
 /* valid */
 *d=r;
 return true;
}

bool getfloatgt0(char *s, double*d)
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
 if(!valid || r<=0 )
        {*d=0.0;
		 return false; /* not valid or <=0 */
        }
 /* valid */
 *d=r;
 return true;
}

bool getfloatge0(char *s, double *d)
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
 if(!valid ||  r<0 )
        {*d=0.0;
         return false; /* not valid or <0 */
        }
 /* valid */
 *d=r;
 return true;
}

// functions like above that take wchar_t *
bool getfloat(wchar_t *s, double*d)
{return getfloat(Utf8Of(s),d);
}
bool getfloatgt0(wchar_t *s, double *d)
{ return getfloatgt0(Utf8Of(s),d);
}
bool getfloatge0(wchar_t *s, double *d)
{ return getfloatge0(Utf8Of(s),d);
}

// now repeat for int's
bool getint(char *s, int *d)
 /*reads a integer number returns true if valid - allows whitespace as well as a number , d=0 on error */
{long r=0;
 bool valid=false;
 while(isspace(*s))++s; /* skip whitespace */
 if(isdigit(*s)|| *s=='+' || *s=='-')
		{/* number must start with a digit or a +/- sign */
		 r=strtol(s,&s,10);       /* read integer (base 10) number */
		 while(isspace(*s))++s; /* skip whitespace */
		 if(*s=='\0') valid=true; /* whitespace number whitespace */
		}
 if(!valid || r>MAXINT || r<MININT )
		{*d=0;
		 return false; /* not valid or too big */
		}
 /* valid */
 *d=(int)r;
 return true;
}

bool getintgt0(char *s, int *d)
 /*reads a integer number thats >0 returns true if valid - allows whitespace as well as a number , d=0 on error */
{long r=0;
 bool valid=false;
 while(isspace(*s))++s; /* skip whitespace */
 if(isdigit(*s)|| *s=='+')
		{/* number must start with a digit or a + sign */
		 r=strtol(s,&s,10);       /* read integer (base 10) number */
		 while(isspace(*s))++s; /* skip whitespace */
		 if(*s=='\0') valid=true; /* whitespace number whitespace */
		}
 if(!valid || r>MAXINT || r<=0 )
		{*d=0;
		 return false; /* not valid or too big, or <=0 */
		}
 /* valid */
 *d=(int)r;
 return true;
}

bool getintge0(char *s, int *d)
 /*reads a integer number thats >=0 returns true if valid - allows whitespace as well as a number , d=0 on error */
{long r=0;
 bool valid=false;
 while(isspace(*s))++s; /* skip whitespace */
 if(isdigit(*s)|| *s=='+')
		{/* number must start with a digit or a + sign */
		 r=strtol(s,&s,10);       /* read integer (base 10) number */
		 while(isspace(*s))++s; /* skip whitespace */
		 if(*s=='\0') valid=true; /* whitespace number whitespace */
		}
 if(!valid || r>MAXINT || r<0 )
		{*d=0;
		 return false; /* not valid or too big, or <0 */
		}
 /* valid */
 *d=(int)r;
 return true;
}

// functions like above that take wchar_t *
bool getint(wchar_t *s, int*d)
{return getint(Utf8Of(s),d);
}
bool getintgt0(wchar_t *s, int *d)
{ return getintgt0(Utf8Of(s),d);
}
bool getintge0(wchar_t *s, int *d)
{ return getintge0(Utf8Of(s),d);
}
