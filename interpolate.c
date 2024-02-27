//---------------------------------------------------------------------------

/* interpolate.c

 Binary search & interpolation code

 Written by Peter Miller 16-11-2012

 */
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
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *--------------------------------------------------------------------------*/
#include <stdbool.h>     /* for bool etc */
#include <string.h>     /* for size_t and ssize_t */
#include "interpolate.h"

/* binary search below based on java code in "extra, extra - read all about it: Nearly all binary searches and mergesorts are broken" by Joshua Bloch. */
/* assumes array a is sorted in increasing order. returns -ve number if not found (see comment in code below) */
ssize_t binarysearch_f(float *a, size_t size_a,float key)
{ssize_t low=0;
 ssize_t high=(ssize_t)size_a-1;
 while(low<=high)
		{ssize_t mid=low+((high-low)>>1); /* (low+high)/2 but written so cannot overflow */
         float midVal=a[mid];
         if(midVal<key)
                low=mid+1;
         else if (midVal>key)
                high=mid-1;
         else
                return mid; /* exact match */
        }
 return -(low+1); /* key not found, using this not found is always negative, but you can see where key fits in order by using return value */
}

/* interpolate the value of "y" corresponing to an input "x" given 2 arrays xa (for x in increasing order) and ya (so ya[n] coresponds to xa[n]) */
/* if clip is true truncate y values at y[0] and y[size-1], otherwise extrapolate outside supplied range */
float interp1D_f(float *xa, float *ya, size_t size, float x, bool clip)
{ssize_t bret=binarysearch_f(xa,size,x);
 ssize_t lo,hi;
 if(bret>=0)
        return ya[bret]; /* was an exact match so can directly return the corresponding y value */
 if(bret== -1)
        {/* before 1st value in xa */
         if(clip) return ya[0]; /* if clipped return 1st value */
         lo=0;/* 1st 2 points */
        }
 else if(bret==-(ssize_t)(size+1))
        { /* after last value in xa */
         if(clip) return ya[size-1]; /* if clipped return last value in array */
		 lo=(ssize_t)size-2; /* last 2 points */
        }
 else
        {/* somewhere in the middle of the array */
         lo= -bret-2;
        }
 hi=lo+1;
#if 0
 if(hi>(ssize_t)size-1 || lo<0)
		{rprintf("Error in interp1D_f(xa,ya,%.0f,%g,%s) lo=%.0f hi=%.0f\n",(double)size,x,P_BOOL(clip),(double)lo,(double)hi);
         return 0.0f;
        }
#endif
 return ya[lo]+(ya[hi]-ya[lo])*(x-xa[lo])/(xa[hi]-xa[lo]); /* linear interpolation */
}

/*
typedef struct {
        float *xa;
        float *ya;
        size_t size;
        } interp1_f;
*/
float interp2D_f(interp1_f *p1, float *za,size_t sizearr_p1,float z,float x, bool clip) /* 2D interpolation p1 is an array of interp1 structures that match elemnts of p1[] */
/* arrays p1[] and za[] are in increasing value of z and are same size (ie p1[n] corresponds to z[n]) */
/* returns y corresponding to x,z */
{ssize_t bret=binarysearch_f(za,sizearr_p1,z);
 ssize_t lo,hi;
 float yl,yh;
 if(bret>=0)
        return interp1D_f(p1[bret].xa, p1[bret].ya, p1[bret].size, x,clip); /* was an exact match so can directly return the corresponding y value */
 if(bret== -1)
        {/* before 1st value in xa */
         if(clip) return interp1D_f(p1[0].xa, p1[0].ya, p1[0].size, x,clip); /* if clipped return 1st value */
         lo=0;/* 1st 2 points */
        }
 else if(bret==-((ssize_t)sizearr_p1+1))
        { /* after last value in xa */
         if(clip) return interp1D_f(p1[sizearr_p1-1].xa, p1[sizearr_p1-1].ya, p1[sizearr_p1-1].size, x,clip); /* if clipped return last value in array */
		 lo=(ssize_t)sizearr_p1-2; /* last 2 points */
        }
 else
        {/* somewhere in the middle of the array */
         lo= -bret-2;
        }
 hi=lo+1;
#if 0
 if(hi>(ssize_t)sizearr_p1-1 || lo<0)
        {rprintf("Error in interp2D_f(p1,za,%.0f,%g,%g,%s) lo=%.0f hi=%.0f\n",(double)sizearr_p1,z,x,P_BOOL(clip),(double)lo,(double)hi);
         return 0.0f;
        }
#endif
 yl=interp1D_f(p1[lo].xa, p1[lo].ya, p1[lo].size, x,clip);
 yh=interp1D_f(p1[hi].xa, p1[hi].ya, p1[hi].size, x,clip);
 return yl+(yh-yl)*(z-za[lo])/(za[hi]-za[lo]); /* linear interpolation */
 // return ya[lo]+(ya[hi]-ya[lo])*(x-xa[lo])/(xa[hi]-xa[lo]); /* linear interpolation */
}

/* double versions */
/* =============== */
/* binary search below based on java code in "extra, extra - read all about it: Nearly all binary searches and mergesorts are broken" by Joshua Bloch. */
/* assumes array a is sorted in increasing order. returns -ve number if not found (see comment in code below) */
ssize_t binarysearch_d(double *a, size_t size_a,double key)
{ssize_t low=0;
 ssize_t high=(ssize_t)size_a-1;
 while(low<=high)
		{ssize_t mid=low+((high-low)>>1); /* (low+high)/2 but written so cannot overflow */
         double midVal=a[mid];
         if(midVal<key)
                low=mid+1;
         else if (midVal>key)
                high=mid-1;
         else
                return mid; /* exact match */
        }
 return -(low+1); /* key not found, using this not found is always negative, but you can see where key fits in order by using return value */
}

/* interpolate the value of "y" corresponing to an input "x" given 2 arrays xa (for x in increasing order) and ya (so ya[n] coresponds to xa[n]) */
/* if clip is true truncate y values at y[0] and y[size-1], otherwise extrapolate outside supplied range */
double interp1D_d(double *xa, double *ya, size_t size, double x, bool clip)
{ssize_t bret=binarysearch_d(xa,size,x);
 ssize_t lo,hi;
 if(bret>=0)
        return ya[bret]; /* was an exact match so can directly return the corresponding y value */
 if(bret== -1)
        {/* before 1st value in xa */
         if(clip) return ya[0]; /* if clipped return 1st value */
         lo=0;/* 1st 2 points */
        }
 else if(bret==-(ssize_t)(size+1))
        { /* after last value in xa */
         if(clip) return ya[size-1]; /* if clipped return last value in array */
		 lo=(ssize_t)size-2; /* last 2 points */
        }
 else
        {/* somewhere in the middle of the array */
         lo= -bret-2;
        }
 hi=lo+1;
#if 0
 if(hi>(ssize_t)size-1 || lo<0)
		{rprintf("Error in interp1D_d(xa,ya,%.0f,%g,%s) lo=%.0f hi=%.0f\n",(double)size,x,P_BOOL(clip),(double)lo,(double)hi);
         return 0.0f;
        }
#endif
 return ya[lo]+(ya[hi]-ya[lo])*(x-xa[lo])/(xa[hi]-xa[lo]); /* linear interpolation */
}

/*
typedef struct {
        double *xa;
        double *ya;
        size_t size;
        } interp1_d;
*/
double interp2D_d(interp1_d *p1, double *za,size_t sizearr_p1,double z,double x, bool clip) /* 2D interpolation p1 is an array of interp1 structures that match elemnts of p1[] */
/* arrays p1[] and za[] are in increasing value of z and are same size (ie p1[n] corresponds to z[n]) */
/* returns y corresponding to x,z */
{ssize_t bret=binarysearch_d(za,sizearr_p1,z);
 ssize_t lo,hi;
 double yl,yh;
 if(bret>=0)
        return interp1D_d(p1[bret].xa, p1[bret].ya, p1[bret].size, x,clip); /* was an exact match so can directly return the corresponding y value */
 if(bret== -1)
        {/* before 1st value in xa */
         if(clip) return interp1D_d(p1[0].xa, p1[0].ya, p1[0].size, x,clip); /* if clipped return 1st value */
         lo=0;/* 1st 2 points */
        }
 else if(bret==-((ssize_t)sizearr_p1+1))
        { /* after last value in xa */
         if(clip) return interp1D_d(p1[sizearr_p1-1].xa, p1[sizearr_p1-1].ya, p1[sizearr_p1-1].size, x,clip); /* if clipped return last value in array */
		 lo=(ssize_t)sizearr_p1-2; /* last 2 points */
        }
 else
        {/* somewhere in the middle of the array */
         lo= -bret-2;
        }
 hi=lo+1;
#if 0
 if(hi>(ssize_t)sizearr_p1-1 || lo<0)
        {rprintf("Error in interp2D_d(p1,za,%.0f,%g,%g,%s) lo=%.0f hi=%.0f\n",(double)sizearr_p1,z,x,P_BOOL(clip),(double)lo,(double)hi);
         return 0.0f;
        }
#endif
 yl=interp1D_d(p1[lo].xa, p1[lo].ya, p1[lo].size, x,clip);
 yh=interp1D_d(p1[hi].xa, p1[hi].ya, p1[hi].size, x,clip);
 return yl+(yh-yl)*(z-za[lo])/(za[hi]-za[lo]); /* linear interpolation */
 // return ya[lo]+(ya[hi]-ya[lo])*(x-xa[lo])/(xa[hi]-xa[lo]); /* linear interpolation */
}

