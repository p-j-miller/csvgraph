/* fast_strtof
  
  
    This is a subset (just float fast_strtof() ) of  fast_atof by Peter Miller 23/1/2020
    See below for license information.
    
	 The code should be robust to anything thrown at it (lots of leading of trailing zeros, extremely large numbers of digits, etc ).
	 As well as floating point numbers this also accepts NAN and INF (case does not matter).
 				  
 */   
// #define AFormatSupport /* if defined then support hex floating point numbers (as created by printf with %A */
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
//#define DEBUG
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h> /* for bool */
#include <stdint.h>  /* for int64_t etc */
#include <math.h>    /* for NAN, INFINITY */
//#define NAN (0.0/0.0)
//define INFINITY (1.0/0.0)
/* ieee floating point maths limits:
   double (64 bits) max 				1.797 e + 308
   					min 				2.225 e-308
   					min denormalised	4.94 e-324
   					sig digits			15-17
   	float (32 bit)	max					3.4e38
   					min					1.17e-38
   					min denormalised	1.4e-45
   					sig digits			6-9
Also note that 0x0fff ffff ffff ffff =  1,152,921,504,606,846,975  	, so 18 digits easily fit into a 64 bit unsigned (with 4 bits spare) - which is enough for a double mantissa.
2^64= 18,446,744,073,709,551,616 
0xffff ffff = 	4,294,967,295 so 9 digits fits with a 32 bit unsigned (with 2 bits spare) which is NOT enough for a float mantissa			
 long double
*/   					
float fast_strtof(const char *s,char **endptr); // if endptr != NULL returns 1st character thats not in the number
static const int maxfExponent = 38;	/* Largest possible base 10 for a float exponent. (must match array below) */
static double const dblpowersOf10[] = /* always double */
                {
                    1e0,   1e1,   1e2,   1e3,   1e4,   1e5,   1e6,   1e7,   1e8,    1e9,
                    1e10,  1e11,  1e12,  1e13,  1e14,  1e15,  1e16,  1e17,  1e18,  1e19,
                    1e20,  1e21,  1e22,  1e23,  1e24,  1e25,  1e26,  1e27,  1e28,  1e29,
                    1e30,  1e31,  1e32,  1e33,  1e34,  1e35,  1e36,  1e37,  1e38,  1e39,
                    1e40,  1e41,  1e42,  1e43,  1e44,  1e45,  1e46,  1e47,  1e48,  1e49,
                    1e50,  1e51,  1e52,  1e53,  1e54,  1e55,  1e56,  1e57,  1e58,  1e59,
                    1e60,  1e61,  1e62,  1e63,  1e64,  1e65,  1e66,  1e67,  1e68,  1e69,
                    1e70,  1e71,  1e72,  1e73,  1e74,  1e75,  1e76,  1e77,  1e78,  1e79,
                    1e80,  1e81,  1e82,  1e83,  1e84,  1e85,  1e86,  1e87,  1e88,  1e89,
                    1e90,  1e91,  1e92,  1e93,  1e94,  1e95,  1e96,  1e97,  1e98,  1e99,
                    1e100, 1e101, 1e102, 1e103, 1e104, 1e105, 1e106, 1e107, 1e108, 1e109,
                    1e110, 1e111, 1e112, 1e113, 1e114, 1e115, 1e116, 1e117, 1e118, 1e119,
                    1e120, 1e121, 1e122, 1e123, 1e124, 1e125, 1e126, 1e127, 1e128, 1e129,
                    1e130, 1e131, 1e132, 1e133, 1e134, 1e135, 1e136, 1e137, 1e138, 1e139,
                    1e140, 1e141, 1e142, 1e143, 1e144, 1e145, 1e146, 1e147, 1e148, 1e149,
                    1e150, 1e151, 1e152, 1e153, 1e154, 1e155, 1e156, 1e157, 1e158, 1e159,
                    1e160, 1e161, 1e162, 1e163, 1e164, 1e165, 1e166, 1e167, 1e168, 1e169,
                    1e170, 1e171, 1e172, 1e173, 1e174, 1e175, 1e176, 1e177, 1e178, 1e179,
                    1e180, 1e181, 1e182, 1e183, 1e184, 1e185, 1e186, 1e187, 1e188, 1e189,
                    1e190, 1e191, 1e192, 1e193, 1e194, 1e195, 1e196, 1e197, 1e198, 1e199,
                    1e200, 1e201, 1e202, 1e203, 1e204, 1e205, 1e206, 1e207, 1e208, 1e209,
                    1e210, 1e211, 1e212, 1e213, 1e214, 1e215, 1e216, 1e217, 1e218, 1e219,
                    1e220, 1e221, 1e222, 1e223, 1e224, 1e225, 1e226, 1e227, 1e228, 1e229,
                    1e230, 1e231, 1e232, 1e233, 1e234, 1e235, 1e236, 1e237, 1e238, 1e239,
                    1e240, 1e241, 1e242, 1e243, 1e244, 1e245, 1e246, 1e247, 1e248, 1e249,
                    1e250, 1e251, 1e252, 1e253, 1e254, 1e255, 1e256, 1e257, 1e258, 1e259,
                    1e260, 1e261, 1e262, 1e263, 1e264, 1e265, 1e266, 1e267, 1e268, 1e269,
                    1e270, 1e271, 1e272, 1e273, 1e274, 1e275, 1e276, 1e277, 1e278, 1e279,
                    1e280, 1e281, 1e282, 1e283, 1e284, 1e285, 1e286, 1e287, 1e288, 1e289,
                    1e290, 1e291, 1e292, 1e293, 1e294, 1e295, 1e296, 1e297, 1e298, 1e299,
                    1e300, 1e301, 1e302, 1e303, 1e304, 1e305, 1e306, 1e307, 1e308
                };
static float const fltpowersOf10[] = /* always float */
                {
                    1e0f,   1e1f,   1e2f,   1e3f,   1e4f,   1e5f,   1e6f,   1e7f,   1e8f,    1e9f,
                    1e10f,  1e11f,  1e12f,  1e13f,  1e14f,  1e15f,  1e16f,  1e17f,  1e18f,  1e19f,
                    1e20f,  1e21f,  1e22f,  1e23f,  1e24f,  1e25f,  1e26f,  1e27f,  1e28f,  1e29f,
                    1e30f,  1e31f,  1e32f,  1e33f,  1e34f,  1e35f,  1e36f,  1e37f,  1e38f
                };
               

static uint32_t u32powersOf10[]=
				{
					UINT32_C(1), 	// 10^ 0
					UINT32_C(10), 	// 10^1
					UINT32_C(100), 	// 10^2
					UINT32_C(1000),	// 10^3
					UINT32_C(10000),// 10^4
					UINT32_C(100000),// 5
					UINT32_C(1000000),// 6
					UINT32_C(10000000),// 7
					UINT32_C(100000000),// 8
					UINT32_C(1000000000),// 9   [ largest possible 10^10 gives compiler error (overflow) ]
				};
/*
 *----------------------------------------------------------------------
 *
 * float fast_strtof(const char *s,char **endptr)
 *
 *	This procedure converts a floating-point number from an ASCII
 *	decimal representation to internal single-precision format.
 *  Also accepts "NaN", "Inf" and "Infinity" (any mix of case) which return NAN and INFINITY
 * Results:
 *	The return value is the floating-point equivalent of string.
 *	*endptr is set to the first character after the valid number 
 *
 * If endptr == NULL it is ignored.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

 /* version of fast_strtof() using unit32 */
 /* ======================================*/
 /* I could not get this to work to full accuracy by just using a uint32 mantissa, so the code below uses a uint32 for as long as possible then swaps to a uint64 */
 /* this reduces the number of uint64 multiplies and additions and so is faster that just using a uint64 all the time */
 static const int maxfdigits=18; // see above this is the largest possible in a uint64
 
float fast_strtof(const char *s,char **endptr) // if endptr != NULL returns 1st character thats not in the number
 {
  double dr; // may need to use double precision to get an accurate float - we try hard below to avoid this either by uisng just a uint32, or just by using float's
  bool sign=false,expsign=false,got_number=false;
  uint_fast32_t r32=0; // mantissa, uint32 can hold 9 digits 
  uint_fast64_t r64=0; // if we get too many digits in mantissa then we swap to using this
  bool usingr64=false; // set to true when we use r64
  int_fast16_t exp=0,rexp=0;
  int_fast16_t nos_mant_digits=0;
  const char *se=s; // string end - candidate for endptr
#ifdef DEBUG
  fprintf(stderr,"strtof(%s):\n",s);
#endif    
  while(isspace(*s)) ++s; // skip initial whitespace	
  // deal with leading sign
  if(*s=='+') ++s;
  else if(*s=='-')
  	{sign=true;
  	 ++s;
    }
  // NAN is a special case - NAN is  signed in the input but always returns NAN
    /* # pragma's below work for gcc and clang compilers , issue is that arguments and return value of function are standardised so cannot avoid this */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
  if((*s=='n' || *s=='N') && (s[1]=='a' || s[1]=='A') && (s[2]=='n' || s[2]=='N'))
  	{if(endptr!=NULL) *endptr=(char *)s+3;// 3 for NAN
  	 return NAN;
  	}    
  // INF or Infinity is a special case - and is signed
  if((*s=='i' || *s=='I') && (s[1]=='n' || s[1]=='N') && (s[2]=='f' || s[2]=='F'))
  	{s+=3;// INF
  	 if((*s=='i' || *s=='I') && (s[1]=='n' || s[1]=='N') && (s[2]=='i' || s[2]=='I') && (s[3]=='t' || s[3]=='T') && (s[4]=='y' || s[4]=='Y') )
  	  	s+=5; // "Infinity" is 5 more chars (inity) than "inf"
  	 if(endptr!=NULL) *endptr=(char *)s;
  	 if(sign) return -INFINITY;
  	 return INFINITY;
	}
#ifdef AFormatSupport 
	/* support hex floating point numbers of the format 0xh.hhhhp+/-d as generated by printf %a */
  if(*s=='0' && (s[1]=='x' || s[1] =='X'))
  	{ // got hex number
  	 float h;
  	 uint_fast64_t r=0;// always use a 64 bit mantissa as we only do shitfs and adds here on mantissa so these should be fast enough on 64 bits.
  	 s+=2; // skip 0x
  	 // no need to skip leading zero's as we can just check is ms 4 bits of r are not zero
	 while(isxdigit(*s))
		{got_number=true; // have a valid number
	  	 if((r & UINT64_C(0xf000000000000000))==0)
	  		{ if(*s<='9')
			  	r=r*16+(*s-'0'); // 0..9
			  else
			  	r=r*16+(tolower(*s)-'a'+10); // a-f or A-F
			}
		else if(exp<2048)
		      exp+=4; // cannot actually capture digits beyond 16 but keep track of decimal point, trap  ensures we don't overflow exp when given a number with a silly number of digits (that would overflow a double)
		 ++s;
		}
  	 // now look for optional decimal point (and fractional bit of mantissa)
  	 if(*s=='.')
  		{ // got decimal point, skip and then look for fractional bit
  	 	 ++s;
		 while(isxdigit(*s))
			{got_number=true; // have a valid number
	  	 	 if((r & UINT64_C(0xf000000000000000))==0)
	  			{ if(*s<='9')
			  		r=r*16+(*s-'0'); // 0..9
			  	 else
			  		r=r*16+(tolower(*s)-'a'+10); // a-f or A-F
			  	 exp-=4;	
				}
			 // if we have too many digits after dp just ignore them
		 	 ++s;
		 	}
		}			  
  	 // got all of mantissa - see if its a valid number, if not we are done
  	 if(!got_number)
 		{if(endptr!=NULL) *endptr=(char *)se;
#ifdef DEBUG
 		 fprintf(stderr," strtof returns 0 (invalid hex number)\n"); 
#endif  	
 	 	 return 0;
 		}	
  	 se=s; // update to reflect end of a valid mantissa
  	 // now see if we have an  exponent
  	 if(*s=='p' || *s=='P')
  		{// have exponent, optional sign is 1st
  	 	 ++s ; // skip 'p'
  	 	 if(*s=='+') ++s;
  	 	 else if(*s=='-') 
  	 		{expsign=true;
  	 	 	 ++s;
  	 		}
  	 	while(isdigit(*s))
	   		{if(rexp<=2048)
		   		rexp=rexp*10+(*s - '0');  // if statement clips at a value that will result in +/-inf but will not overflow int
		 	 ++s;  
		 	 se=s; // update to reflect end of a valid exponent (p[+-]digit+)
			}
		}
 	 if(endptr!=NULL) *endptr=(char *)se; // we now know the end of the number - so save it now (means we can have multiple returns going forward without having to worry about this)	
 	 if(expsign) rexp=-rexp;	
 	 rexp+=exp; // add in correct to exponent from mantissa processing				
	 h=ldexpf((float)r,rexp); // combine mantissa and exponent 
	 if(sign) h=-h;
#ifdef DEBUG
 	 fprintf(stderr," strtof (0x) returns %.18g [0x%.16A] (rexp=%d, exp=%d)\n",h,h,rexp,exp); 
#endif  	 
	 return h; // all done 	
	}
#endif	     
  // skip leading zeros
  while(*s=='0')
  	{got_number=true; // have a number (0)
	 ++s;
	}
  // now read rest of the mantissa	
  while(isdigit(*s))
  	{ got_number=true; // have a valid number
	  if(!usingr64 && r32<=429496728  )
		{ r32=r32*10+(uint_fast32_t)(*s-'0');// uint32 can hold upto 4,294,967,295 so we can directly store this extra digit
		  nos_mant_digits++;	
		}
	  else if(nos_mant_digits < maxfdigits)	
	    {if(!usingr64) 
			{r64=r32;// too many digits for a uint32, swap to a uint64
			 usingr64=true;
			}
		 r64=r64*10+(uint_fast64_t)(*s-'0');
		 nos_mant_digits++;	
		}		
	  else if(exp<2*maxfExponent)
		      exp++; // cannot actually capture digits as more than 9 but keep track of decimal point, trap at 2*maxExponent ensures we don't overflow exp when given a number with a silly number of digits (that would overflow a double)		
	  ++s;
	}
  // now look for optional decimal point (and fractional bit of mantissa)
  if(*s=='.')
  	{ // got decimal point, skip and then look for fractional bit
  	 ++s;
  	 if(r32==0)
  	 	{// number is zero at present, so deal with leading zeros in fractional bit of mantissa
  	 	 while(*s=='0')
  	 	 	{got_number=true;
  	 	 	 ++s;
  	 	 	 if(exp > -2*maxfExponent)
  	 	 	 	exp--; // test avoids issues with silly number of leading zeros
  	 	    }
  	 	}
  	 // now process the rest of the fractional bit of the mantissa
	 while(isdigit(*s))
	 	{got_number=true;
	 	 if(!usingr64 && r32<=429496728 )
				{ r32=r32*10+(uint_fast32_t)(*s-'0'); // uint32 can hold upto 4,294,967,295 so we can directly store this extra digit
		  		  nos_mant_digits++;	
		  		  exp--;
				}
	  	 else if(nos_mant_digits < maxfdigits)	
	    	{if(!usingr64) 
				{r64=r32;// too many digits for a uint32, swap to a uint64
				 usingr64=true;
				}
			 r64=r64*10+(uint_fast64_t)(*s-'0');
		 	 nos_mant_digits++;	
		 	 exp--;
			}		  			
		 else
	  			{ 
				   // cannot actually capture digits as more than 18, so just ignore them as they after after decimal point
				}
		++s;
		}
 	}
  // got all of mantissa - see if its a valid number, if not we are done
  if(!got_number)
	{if(endptr!=NULL) *endptr=(char *)se;
#ifdef DEBUG
 	fprintf(stderr," strtof returns 0 (invalid number)\n"); 
#endif  	
 	 return 0;
 	}	
  if(!usingr64) r64=r32;// in this case put mantissa into both r64 and r32 so code below can use either as appropiate
  se=s; // update to reflect end of a valid mantissa
  // now see if we have an  exponent
  if(*s=='e' || *s=='E')
  	{// have exponent, optional sign is 1st
  	 ++s ; // skip 'e'
  	 if(*s=='+') ++s;
  	 else if(*s=='-') 
  	 	{expsign=true;
  	 	 ++s;
  	 	}
  	 while(isdigit(*s))
	   	{if(rexp<=2*maxfExponent)
		   rexp=rexp*10+(int_fast16_t)(*s - '0');  // if statement clips at a value that will result in +/-inf but will not overflow int
		 ++s;  
		 se=s; // update to reflect end of a valid exponent (e[+-]digit+)
		}
	}
 if(endptr!=NULL) *endptr=(char *)se; // we now know the end of the number - so save it now (means we can have multiple returns going forward without having to worry about this)	
 if(expsign) rexp=(-rexp);
 rexp+=exp; // add in correct to exponent from mantissa processing
#if 1 /* if 0 removes the optimisations which just results in slower code - there is no loss of accuracy with these optimisations */
 if(!usingr64 && rexp>0 && rexp+nos_mant_digits<=9)
 	{// optimisation: can do all calculations using uint32 which is exact and fast
 	 r32*=u32powersOf10[rexp];
 	 if(sign) return -((float)r32); // negative exponent means we divide by powers of 10
 	 else return (float)r32;
 	}
 else if(!usingr64 && rexp<0 && rexp >= -7 && nos_mant_digits<=7 )
 	{// in this region we can use float rather than double as 10^6 is exact as a float (another speed optimisation, but one than thats common and therefore worthwhile)
 	 // mantissa in a float is 23 bits+ the hidden bit so 24 bits, 2^24-1 = 1.67e7 so for integers we are exact for 7 sig digits.
 	 if(sign) return -((float)r32/fltpowersOf10[-rexp]); // negative exponent means we divide by powers of 10
 	 else return (float)r32/fltpowersOf10[-rexp];
	}
#endif
 // calculate dr=(float)r*pow(10,rexp), but by using a lookup table of powers of 10 for speed and accuracy, and using doubles to ensure accuracy.
 if(rexp>0)
 	{if(rexp>maxfExponent)
		{// we have defininaly overflowed
		 if(sign) return -INFINITY;
 		 return INFINITY;
 		}
 	 if(!usingr64) 	dr=(double)r32*dblpowersOf10[rexp]; // as mantissa >= 1 this may overflow, but thats OK.
 	 else dr=(double)r64*dblpowersOf10[rexp]; // as mantissa >= 1 this may overflow, but thats OK.
	}
 else if(rexp<0)
 	{// need to take care here as mantissa is > 1 so even dividing by 10^maxExponent may not be enough, here we all division by upto 10^2*maxExponet is is by far enough
	 rexp=( -rexp);
	 exp=rexp;
	 if(rexp>maxfExponent)
	 	{
		 rexp=maxfExponent;
		 exp-=maxfExponent; // any excess which we will also need to divide by (if its > 0)
 		}
 	  else exp=0;	
 	  if(!usingr64) dr=(double)r32/dblpowersOf10[rexp]; // negative exponent means we divide by powers of 10
 	  else dr=(double)r64/dblpowersOf10[rexp]; // negative exponent means we divide by powers of 10
 	  if(exp>0)
 	  	{dr/=dblpowersOf10[exp]; // divide by some more, we should only be dividing by max 10^18 as we only have 18 sig figs in mantissa (plus a few more if we consider creation of denormalised numbers)
 	    }
	}	
 else
 	{// special case, rexp==0
	 // do not need to use double here, so we use float for speed.
	 if(!usingr64)
	 	{ if(sign) return -((float)r32); // r is unsigned so cannot do -r !
 	 	  else return (float) r32;
	 	}
	 else
	 	{	
 	 	 if(sign) return -((float)r64); // r is unsigned so cannot do -r !
 	 	 else return (float) r64;
 	 	}
	}
 if(sign) dr= -dr;
#ifdef DEBUG
 // while this is the normal return there are several earlier return possibilities, which this will not print for (sorry).
 fprintf(stderr," strtof returns %.18g (rexp=%d, exp=%d)\n",(double)dr,rexp,exp); 
#endif 
 return (float)dr;
}
#pragma GCC diagnostic pop



