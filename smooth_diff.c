/* smooth_diff.c
   =============

 Savitzky Golay smoothing and differentiation
 
 Includes 2nd derivative (d2y/d2x) which uses the 3 point equation from https://mathformeremortals.wordpress.com/2013/01/12/a-numerical-second-derivative-from-three-points/
 where each point is Savitzky Golay smoothed before use.
 
 Higher order derivatives can be obtained by repeated use of the 1st derivative e.g. the 3rd derivative can be obtained as :
   d3y/dx3 = dy/dx(dy/dx(dy/dx(y(x))))
 
 The "order" parameter defines the order of the curve thats locally fitted to data values up to +/-8 either side of the selected point. This can be 1,2,4.
 An order of 1 gives the lowest noise in the output, while 4 gives more noise but better tracks faster changes in the input.
 With an order of 1 the code effectively least squares fits a straight line to 17 points (+/-8 from the selected index) and uses that to get the mean/derivative.
 If order is 2 a quadratic is fitted, while if order is 4 a quartic is fitted.
 
 These all assume the x values are monotonically increasing (if this is not true strange results or divide by zero errors may occur).
 The x values do NOT need to be evenly spaced.
 Because values either side of the selected index are used, smoothing does not change the shape of the curve (e.g. a maximium will stay at the same x value).
 
 Processing both end of the array (where +/-8 either side do not all exist) are treated as special cases in the code with the aim of giving sensible results.
 
 These routines process 1 point at a time so they need a loop like that below to process a whole array (their normal use case):
 
   for(size_t i=0;i<nos_vals;++i)
	{
	 smooth[i]=Savitzky_Golay_smoothing(y_arr,x_arr,0,nos_vals-1,i,2);// 2nd order smoothing for nos_vals points in array
	}
	
 Written by Peter Miller 14-3-2023
 This version 8-8-2023 Peter Miller
	- tests with unsigned (or size_t which is unsigned) rearranged to just use addition (- could cause underflow which would give a big positive number)
	- code uses a generic polynomial fit for the general case (using orthoganal polys for accuracy),
	- tables of g-s coefficients are still used when possible as that is much faster (and more accurate) that fitting polynomial "on the fly"
  15-8-2023 : dynamic arrays in generic polynomial fit changed to fixed size to avoid lots of alloc/frees
  16-8-2023 : dy_dx() is now the fastest general purpose version, with dy_dx_polyfit() a version that should give the same results but slower.
  19-8-2023 : original function names suffixed with 17 to show 17 points used (+/-8 about index)
			  added 25 option with polyfit (g-s paper gives coeffs for up to 25)
 */

 /* 
Copyright (c) 2023 Peter Miller
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <stdlib.h> /* needed for size_t */
#include <stdbool.h>
#include "smooth_diff.h"
#include "rprintf.h" /* for crprintf() */

#define P_UNUSED(x) (void)x /* a way to avoid warning unused parameter/variables messages from the compiler */

#define MAX_ORDER 10  /* max order for polyreg, if not defined arrays are dynamically allocated (which will be slower but not limited) */
					  /* in practice for smoothing applications code was already limited to 10 */

#define GS_MAX_ORDER 10 /* max order for filtering & derivative functions - should match MAX_ORDER (normally 10) */

bool Polyreg(float *y_vals,float *x_vals,size_t  iCount, unsigned int order, double *coeffs)
	// fit polynomial of specified order regression
	// does least squares fit using orthogonal polynomials to minimise errors
	// Loosely based on algorithm in section 12.3 of Programming Classics by Ian Oliver.
	// This implementation by Peter Miller
	//  - added prescaling for x and y as otherwise numbers can become very big...
	//  - removed need for internal arrays the same size as the x and y value arrays (which dramatically reduces ram needed)
	//  - uses "symbolic execution" to get coefficients for conventional polynomial
	// returns true if works, false if an issue found
	// iCount is number of values in arrays to use
	// coeffs must be size order+1, coeffs[0] is constant term
{
 double x,y;  // need to be double as we scale floats
 long double divisor,previous;
 float minx,maxx,miny,maxy;
 double ym,yc,xm,xc; // a=m*Z+c where a is limited to +/-1 which minimises range of numbers in the calculations making them more robust
 unsigned int i,j;
 bool failed=false; // assume things go OK
#ifdef MAX_ORDER
 long double sx[MAX_ORDER+1],sy[MAX_ORDER+1],v[MAX_ORDER+1];  // fixed size arrays (order is limited below)
#else
 long double *sx=(long double *)calloc(order+1,sizeof(long double));// sum of products involving x - calloc initialises to 0
 long double *sy=(long double *)calloc(order+1,sizeof(long double)); // sum of products involving x & y
 long double *v=(long double *)calloc(order+1,sizeof(long double)); // orthogonal polynomial coefficient
#endif
 long  double pv,qv;
 // set all output coefficients to 0 by default
 for(i=0;i<=order;++i)
	coeffs[i]=0;
#ifdef MAX_ORDER
 if(order>MAX_ORDER) order=MAX_ORDER; // limit max order as arrays are now a fixed size
 for(i=0;i<=order;++i) // need to zero all arrays to match calloc() [ have to do this after order is "clipped"
	{sx[i]=sy[i]=v[i]=0;
	}
#else
 if(v==NULL)  // make sure memory allocations worked
	{
	 if(sy!=NULL) free(sy);
	 if(sx!=NULL) free(sx);
#ifdef DEBUG
	 crprintf("Polynomial fit failed - not enough free RAM\n");
#endif
	 return false;
	}
#endif
#if 0
 if((size_t)order> (iCount>>1))
	{order=(unsigned int)(iCount>>1); // imperical observation is order > 1/2 total number of points then bad things happen numerically, so trap that here
	}
#endif
	minx=maxx=x_vals[0];
	miny=maxy=y_vals[0];
	for(i=0;i<iCount;++i)
		{y=y_vals[i];
		 if(y>maxy) maxy=(float)y;
		 if(y<miny) miny=(float)y;
		 x=x_vals[i];
		 if(x>maxx) maxx=(float)x;
		 if(x<minx) minx=(float)x;
		}
#ifdef DEBUG
	crprintf("Polyfit: %u points, x from %g  to %g y from %g to %g\n",iCount,minx,maxx,miny,maxy);
	// crprintf("sizeof(float)=%u, sizeof(double)=%u, sizeof(long double)=%u\n",sizeof(float), sizeof(double), sizeof(long double));
#endif
	// calculate scaling factors to get all values into range +/-1
	if(maxy==miny)
		{// y is a constant, so just need to make this constant 0 c*1-c=0
		 ym=1;
		 yc= -maxy;
		}
	else
		{
		 ym=2.0/(maxy-miny);      // 2.0/ (maxy-miny) gives +/-1 while 1/(maxy-miny) gives 0..1
		 yc=1.0-ym*maxy;
		}
	if(maxx==minx)
		{coeffs[0]=0.5*maxy+0.5*miny; // set to "middle" y value  (all x values are the same which should never happen !)
		 return true; // if x range is zero then there is nothing to do
		}
	xm=2.0/(maxx-minx);
	xc=1.0-xm*maxx;
	v[0]=0;
	divisor=iCount;
	for(j=0;j<=order;++j)
		{sy[j]=0;
		 sx[j]=0;
		 if(failed) break;// failed means we trapped a divide by zero
		 for(i=0;i<iCount;++i)
			{x=x_vals[i];
			 y=y_vals[i];
			 // scale x,y
			 x=x*xm+xc;
			 y=y*ym+yc;
			 pv=0;
			 qv=1;
			 for(unsigned int j1=1;j1<=j;++j1)   // calculate orthogonal polynomial for x[i]
				{long double t;
				 t=pv;pv=qv;qv=t; // swap(pv,qv)
				 qv=((x-sx[j1-1])*pv)-(v[j1-1]*qv);
				}
			 sy[j]+=y*qv;
			 sx[j]+=x*qv*qv;
			}
		 if(divisor==0)
			{failed=true;
			 break;    // avoid divide by zero
			}
		 sy[j]/=divisor;
		 sx[j]/=divisor;
		 if(j<order)
			{previous=divisor;
			 divisor=0;
			 for(i=0;i<iCount;++i) // calculate orthogonal polynomials for next order
				{x=x_vals[i];
				 // scale x
				 x=x*xm+xc;
				 pv=0;
				 qv=1;
				 for(unsigned int j1=1;j1<=j+1;++j1)   // calculate orthogonal polynomial for x[i]
					{long double t;
					 t=pv;pv=qv;qv=t; // swap(pv,qv)
					 qv=((x-sx[j1-1])*pv)-(v[j1-1]*qv);
					}
				 divisor+=qv*qv;
				}
			 if(previous==0)
				{failed=true;
				 break;    // avoid divide by zero
				}
			 v[j+1]=divisor/previous;
			}
		}

 if(failed)
	{
#ifndef MAX_ORDER
	 free(v);   // free remaining memory
	 free(sy);
	 free(sx);
#endif
#ifdef DEBUG
	 crprintf("Polynomial fit order %u FAILED (try a lower order)\n",order);
#endif
	 return false;
	}
#ifdef DEBUG
 crprintf("Polynomial fit order %u complete:\n",order);
 for(i=0;i<=order;++i)
		crprintf("  sx[%u]=%.12g sy[%u]=%.12g v[%u]=%.12g\n",i,(double)sx[i],i,(double)sy[i],i,(double)v[i]);
#endif

 // Estimate coefficients for a "normal" (Horners rule)polynomial by "symbolic execution" of orthogonal polynomial
 // for pv, qv, sum we need to keep track of the coefficients of powers of x - do that using the following arrays

#ifdef MAX_ORDER
 long double pa[MAX_ORDER+1],qa[MAX_ORDER+1],suma[MAX_ORDER+1],t1a[MAX_ORDER+1],t2a[MAX_ORDER+1]; // fixed sized arrays for speed
 for(i=0;i<=order;++i) // need to zero all arrays to match calloc()
	{pa[i]=qa[i]=suma[i]=t1a[i]=t2a[i]=0;
	}
#else
 bool horner_poly=true; // true if horners rule poly found
 long double *pa=(long double *)calloc(order+1,sizeof(long double));// p,q are long double to match code above. calloc initialises to 0
 long double *qa=(long double *)calloc(order+1,sizeof(long double));
 long double *suma=(long double *)calloc(order+1,sizeof(long double));
 long double *t1a=(long double *)calloc(order+1,sizeof(long double));  // temp arrays
 long double *t2a=(long double *)calloc(order+1,sizeof(long double));

 if(t2a==NULL)
	{// oops out of memory - all memory is freed at the end as we need to continue in this function...
#ifdef DEBUG
	 crprintf("Sorry cannot calculate coefficients of Horners rule polynomial - no RAM\n");
#endif
	 horner_poly=false;
	 // carry on as we need to set new values onto graph from orthogonal poly
	}
 else
#endif
	{// calculate "Horners rule" poly by symbolic execution
	 // easy array (eg qa[]) 0'th coeff is constant term [1] is *x, [2] is *x^2 etc
	 // pv=0 is already done as calloc initislises to zero
	 qa[0]=1; // qv=1.0
	 suma[0]=sy[0];
	 for(j=1;j<=order;++j)   // calc orthogonal poly at x
		{long double t;
		 for(i=0;i<=order;++i)  // swap(pv,qv)
			{t=pa[i];pa[i]=qa[i];qa[i]=t;
			}
		 // t1= (v[j-1]*qv)
		 for(i=0;i<=order;++i)
			{t1a[i]=v[j-1]*qa[i];
			}
		 // t2= ((x-sx[j-1])*pv)   note x=X*xm+xc
		 for(i=0;i<=order;++i)
			{t2a[i]= (xc-sx[j-1])*pa[i]; // xc-sx[j-1]*pv
			}
		 for(i=0;i<order;++i)  // <order as [i+1] below
			{
			 t2a[i+1]+=xm*pa[i]; // x*pv
			}
		 // qa=t2-t1  as qv=((x-sx[j-1])*pv)-(v[j-1]*qv);
		 for(i=0;i<=order;++i)
			{qa[i]=t2a[i]-t1a[i];
			}
		 // sum+=sy[j]*qv
		 for(i=0;i<=order;++i)
			{suma[i]+=sy[j]*qa[i];
			}
		}
#ifdef DEBUG
	 crprintf("Polynomial approximating function is:\nY=");
	 // sum=(sum-yc)/ym      - note ym is scaling factor we calculated at start and we know its not zero.
	 // print in an efficient way to execute eg for quadratic (order 2) print (C2*x+C1)*x+C0   [ This is Horners rule ]
	 for(i=1;i<order;++i)
		rprintf("(");
	 if(order>0)
		{rprintf("%.12g",(double)((suma[order])/ym));
		 for(i=order-1;i!=0;--i)
			rprintf("*X%+.12g)",(double)((suma[i])/ym));//%+ causes the sign to always be printed (+/-)
		 rprintf("*X");
		}
	 rprintf("%+.12g",(double)((suma[0]-yc)/ym));  // C0 (note different scaling)
	 rprintf("\n");
#endif
	 // now set values into coeffs[]
	 if(order>0)
		{coeffs[order]=(double)((suma[order])/ym);
		 for(i=order-1;i!=0;--i)
			coeffs[i]=(double)((suma[i])/ym);
		}
	 coeffs[0]=(double)((suma[0]-yc)/ym);  // C0 (note different scaling)
	}
#ifndef MAX_ORDER
 if(pa!=NULL) free(pa);     // free remaining memory , note "horner_poly" may have failed "out of ram" so only free when allocation was OK.
 if(qa!=NULL) free(qa);
 if(suma!=NULL)free(suma);
 if(t1a!=NULL) free(t1a);
 if(t2a!=NULL) free(t2a);
 if(v!=NULL) free(v);
 if(sy!=NULL) free(sy);
 if(sx!=NULL) free(sx);
 return horner_poly; // all done
#else
 return true; // fixed array sizes so cannot have run out of ram
#endif

}


/* Savitzky Golay smoothing
   17 points orders up to GS_MAX_ORDER (normally 10)
   (that does not a mean 10th order polynomial fit to 17 points is a good idea!, orders to 4 or perhaps 6 are OK)
   Assumes x values are all monotonically increasing or decreasing
   Fits a s_order order polynomial to a small range of the data and evaluates that symbolicaly at "index"
   All orders use up to 17 points (+/-8 either side of point in question)

   This means the 1st order solution gives lower noise, but might supress small spikes
   The 4th order has higher noise but  higer order means finer features are left intact
   At the ends use as many points as possible to at most +/-8 (which means at least 9 points are used)
   As data if fitted by a "custom" least-squares polynomial at every call, the x steps can be any size
   Here order is limited to GS_MAX_ORDER
*/
static double Savitzky_Golay_smoothing_orig17(float *y,float *x,size_t  start, size_t  end, size_t  index, unsigned int s_order); /* give smoothed value at index - only using data between start and end, s_order 1=>1, 2,3 => 2 >3 => 4 */

double Savitzky_Golay_smoothing17(float *y,float *x,size_t  start, size_t  end, size_t  index, unsigned int s_order) /* estimate yat index - only using data between start and end */
{
  if(index<start || index>end || start==end ) return 0; /* index out of range , or only 1 point (start=end) */
   if(s_order>GS_MAX_ORDER) s_order=GS_MAX_ORDER; // ensure order is not too large for array below
   //if(s_order>8) s_order=8; // at start/end may just be using 8 points
   // fit a polynomial of specified order to +/-8 points either side of index (or less if near an end)
   double pcoeff[GS_MAX_ORDER+1];  // 1 more than max allowed diff_order (which is limited to GS_MAX_ORDER by line above)
   for(unsigned int i=0;i<=GS_MAX_ORDER;++i)
		pcoeff[i]=0; // need to initialise array to zero (Polyreg() zeros coefficients used at start based on order
   /* fit polynomial of specified order to first/last few points and differentiate that */
#if 1
   if(index<start+8 && end-start>=16)   // 17 points would be start=0, end=16
	{
	  // near the start, use the first 17 point to fit a polynomial to  (we normally use 17 points, +/-8 values around index)
	  // bool Polyreg(float *y_vals,float *x_vals,size_t  iCount, unsigned int order, double *coeffs);
	  if(Polyreg(y,x,17,s_order,pcoeff))
#else
   if(index<start+8 && end>=index+8)
	{// near the start, use the points before index and 8 points after to fit a polynomial to  (use 8 as we normally use +/-8 values around index)
	  // bool Polyreg(float *y_vals,float *x_vals,size_t  iCount, unsigned int order, double *coeffs);
	  if(Polyreg(y,x,index+9,s_order,pcoeff))
#endif
		{// fit worked OK, calculate value at x[index]
		 double xc=x[index]; // want to calculate derivative at x[index]
		 return horner(pcoeff,xc,s_order);
		}
	}
#if 1
   else if(index+8>end && end>=16)
	{
	  // near the end, use the last 17 points to fit a polynomial to  (we normally use 17 points, +/-8 values around index)
	  // bool Polyreg(float *y_vals,float *x_vals,size_t  iCount, unsigned int order, double *coeffs);
	  if(Polyreg(y+end-16,x+end-16,17,s_order,pcoeff))
#else
   else if(index+8>end && index>=start+8)
	{// near the end, use the points after index and 8 points before to fit a polynomial to
	  // bool Polyreg(float *y_vals,float *x_vals,size_t  iCount, unsigned int order, double *coeffs);
	  if(Polyreg(y+index-8,x+index-8,end-(index-9),s_order,pcoeff))
#endif
		{// fit worked OK, calculate value at x[index]
		 double xc=x[index]; // want to calculate derivative at x[index]
		 return horner(pcoeff,xc,s_order);
		}
	}
   else if(end>=start+16)
	{// else in the middle , use +/-8 from index (so 17 points in total, +16 as 1st point is at arr[0])
	  // bool Polyreg(float *y_vals,float *x_vals,size_t  iCount, unsigned int order, double *coeffs);
	  if(Polyreg(y+index-8,x+index-8,17,s_order,pcoeff))
		{// fit worked OK, calculate value at x[index]
		 double xc=x[index]; // want to calculate derivative at x[index]
		 return horner(pcoeff,xc,s_order);
		}
	}
  // less than 17 points in total, or Polyreg failed  use original Savitzky_Golay_smoothing_orig() code as this deals with a small number of points reasonably sensibily.
  // note that we cannot call  smooth_s_g() as that would likley just call this function recursively!
  return Savitzky_Golay_smoothing_orig17(y,x,start,end,index,s_order);
}

/* same as above but using 25 points (+- 12 about index) */
double Savitzky_Golay_smoothing25(float *y,float *x,size_t  start, size_t  end, size_t  index, unsigned int s_order) /* estimate yat index - only using data between start and end */
{
  if(index<start || index>end || start==end ) return 0; /* index out of range , or only 1 point (start=end) */
   if(s_order>GS_MAX_ORDER) s_order=GS_MAX_ORDER; // ensure order is not too large for array below
   // fit a polynomial of specified order to +/-12 points either side of index (or less if near an end)
   double pcoeff[GS_MAX_ORDER+1];  // 1 more than max allowed diff_order (which is limited to GS_MAX_ORDER by line above)
   for(unsigned int i=0;i<=GS_MAX_ORDER;++i)
		pcoeff[i]=0; // need to initialise array to zero (Polyreg() zeros coefficients used at start based on order
   /* fit polynomial of specified order to first/last few points and differentiate that */
#if 1
   if(index<start+12 && end-start>=24)   // 25 points would be start=0, end=24
	{
	  // near the start, use the first 25 point to fit a polynomial to  (we normally use 25 points, +/-12 values around index)
	  // bool Polyreg(float *y_vals,float *x_vals,size_t  iCount, unsigned int order, double *coeffs);
	  if(Polyreg(y,x,25,s_order,pcoeff))
#else
   if(index<start+12 && end>=index+12)
	{// near the start, use the points before index and 12 points after to fit a polynomial to  (use 12 as we normally use +/-12 values around index)
	  // bool Polyreg(float *y_vals,float *x_vals,size_t  iCount, unsigned int order, double *coeffs);
	  if(Polyreg(y,x,index+13,s_order,pcoeff))
#endif
		{// fit worked OK, calculate value at x[index]
		 double xc=x[index]; // want to calculate derivative at x[index]
		 return horner(pcoeff,xc,s_order);
		}
	}
#if 1
   else if(index+12>end && end>=24)
	{
	  // near the end, use the last 25 points to fit a polynomial to  (we normally use 25 points, +/-12 values around index)
	  // bool Polyreg(float *y_vals,float *x_vals,size_t  iCount, unsigned int order, double *coeffs);
	  if(Polyreg(y+end-24,x+end-24,25,s_order,pcoeff))
#else
   else if(index+12>end && index>=start+12)
	{// near the end, use the points after index and 12 points before to fit a polynomial to
	  // bool Polyreg(float *y_vals,float *x_vals,size_t  iCount, unsigned int order, double *coeffs);
	  if(Polyreg(y+index-12,x+index-12,end-(index-13),s_order,pcoeff))
#endif
		{// fit worked OK, calculate value at x[index]
		 double xc=x[index]; // want to calculate derivative at x[index]
		 return horner(pcoeff,xc,s_order);
		}
	}
   else if(end>=start+24)
	{// else in the middle , use +/-12 from index (so 25 points in total, +24 as 1st point is at arr[0])
	  // bool Polyreg(float *y_vals,float *x_vals,size_t  iCount, unsigned int order, double *coeffs);
	  if(Polyreg(y+index-12,x+index-12,25,s_order,pcoeff))
		{// fit worked OK, calculate value at x[index]
		 double xc=x[index]; // want to calculate derivative at x[index]
		 return horner(pcoeff,xc,s_order);
		}
	}
  // less than 25 points in total, or Polyreg failed  use original Savitzky_Golay_smoothing_orig() code as this deals with a small number of points reasonably sensibily.
  // note that we cannot call  smooth_s_g() as that would likley just call this function recursively!
  return Savitzky_Golay_smoothing_orig17(y,x,start,end,index,s_order);   // call 17 version as we have no choice!
}


/* Savitzky Golay smoothing */
/* Coefficients from original Savitzky & Golay paper (17 points , +/- 8) */
/* this assumes (approximately) constant x increments and by doing so is a lot more efficent than Savitzky_Golay_smoothing()
   for orders 1,2,4 (its also more accurate for those cases).
   Uses Savitzky_Golay_smoothing() for other orders and situations where +/-8 points are not available (eg the ends)
   Note that this code does assume constant x increments, if this is not (nearly) true then you should
   probably use Savitzky_Golay_smoothing() unless speed is more important than accuracy.
*/
double Savitzky_Golay_smoothing_const_xinc17(float *y,float *x,size_t  start, size_t  end, size_t  index, unsigned int s_order) /* give smoothed value at index - only using data between start and end, s_order 1=>1, 2,3 => 2 >3 => 4 */
{ P_UNUSED(x);
  if(index<start || index>end ) return 0; /* index out of range */
  if(start==end) return y[index]; /*  only 1 point (start=end)*/
  if(s_order==1)
	 {// order 1 length 17 - 1st order is a  "moving average" all offsets have equal weight
	  if(index>=start+8 && index+8 <=end)
			{/* use data +/-8 either side of index */
			 return
			 (1.0/17.0) * ((double)y[index]) +
			 (1.0/17.0) * ((double)y[index-1]+y[index+1]) +
			 (1.0/17.0) * ((double)y[index-2]+y[index+2]) +
			 (1.0/17.0) * ((double)y[index-3]+y[index+3]) +
			 (1.0/17.0) * ((double)y[index-4]+y[index+4]) +
			 (1.0/17.0) * ((double)y[index-5]+y[index+5]) +
			 (1.0/17.0) * ((double)y[index-6]+y[index+6]) +
			 (1.0/17.0) * ((double)y[index-7]+y[index+7]) +
			 (1.0/17.0) * ((double)y[index-8]+y[index+8])
			 ;
			}
	 }
  if(s_order==2)
	 {// order 2 length 17
	  if(index>=start+8 && index+8 <=end)
			{/* use data +/-8 either side of index */
			 return
			 (43.0/323.0) * ((double)y[index]) +
			 (42.0/323.0) * ((double)y[index-1]+y[index+1]) +
			 (39.0/323.0) * ((double)y[index-2]+y[index+2]) +
			 (34.0/323.0) * ((double)y[index-3]+y[index+3]) +
			 (27.0/323.0) * ((double)y[index-4]+y[index+4]) +
			 (18.0/323.0) * ((double)y[index-5]+y[index+5]) +
			 (7.0/323.0) * ((double)y[index-6]+y[index+6]) +
			 (-6.0/323.0) * ((double)y[index-7]+y[index+7]) +
			 (-21.0/323.0) * ((double)y[index-8]+y[index+8])
			 ;
			}
	 }
  if(s_order==4)
	 {// order 4 length 17
	  if(index>=start+8 && index+8 <=end)
			{/* use data +/-8 either side of index */
			 return
			 (883.0/4199.0) * ((double)y[index]) +
			 (825.0/4199.0) * ((double)y[index-1]+y[index+1]) +
			 (660.0/4199.0) * ((double)y[index-2]+y[index+2]) +
			 (415.0/4199.0) * ((double)y[index-3]+y[index+3]) +
			 (135.0/4199.0) * ((double)y[index-4]+y[index+4]) +
			 (-117.0/4199.0) * ((double)y[index-5]+y[index+5]) +
			 (-260.0/4199.0) * ((double)y[index-6]+y[index+6]) +
			 (-195.0/4199.0) * ((double)y[index-7]+y[index+7]) +
			 (195.0/4199.0) * ((double)y[index-8]+y[index+8])
			 ;
			}
	 }
  return Savitzky_Golay_smoothing17(y,x,start, end, index,  s_order);
}



/* Savitzky Golay smoothing */
/* Coefficients from original Savitzky & Golay paper */
/* 17 points orders 1,2,4 */
/* general case code, used when least square line fitting solution does not work for some reason
   This code is not callable directly by the user, but its used as a "backup" for the above routines should the more
   general purpose code fail.
*/
static double Savitzky_Golay_smoothing_orig17(float *y,float *x,size_t  start, size_t  end, size_t  index, unsigned int s_order) /* give smoothed value at index - only using data between start and end, s_order 1=>1, 2,3 => 2 >3 => 4 */
{ P_UNUSED(x);
  if(index<start || index>end ) return 0; /* index out of range */
  if(start==end) return y[index]; /*  only 1 point (start=end)*/
  if(index==start)  // at start
	{return((double)y[start]+y[start+1])/(2.0); /* use 1st 2 points */
	}
  if(index==end)
	{   /* at the end */
		return((double)y[end]+y[end-1])/(2.0); /* use last 2 points */
	}
  if(s_order==1)
	 {// order 1 length 17 - 1st order is a  "moving average" all offsets have equal weight
	  if(index>=start+8 && index+8 <=end)
			{/* use data +/-8 either side of index */
			 return
			 (1.0/17.0) * ((double)y[index]) +
			 (1.0/17.0) * ((double)y[index-1]+y[index+1]) +
			 (1.0/17.0) * ((double)y[index-2]+y[index+2]) +
			 (1.0/17.0) * ((double)y[index-3]+y[index+3]) +
			 (1.0/17.0) * ((double)y[index-4]+y[index+4]) +
			 (1.0/17.0) * ((double)y[index-5]+y[index+5]) +
			 (1.0/17.0) * ((double)y[index-6]+y[index+6]) +
			 (1.0/17.0) * ((double)y[index-7]+y[index+7]) +
			 (1.0/17.0) * ((double)y[index-8]+y[index+8])
			 ;
			}
	  else if(index>=start+7 && index+7 <=end)
			{/* use data +/-7 either side of index */
			 return
			 (1.0/15.0) * ((double)y[index]) +
			 (1.0/15.0) * ((double)y[index-1]+y[index+1]) +
			 (1.0/15.0) * ((double)y[index-2]+y[index+2]) +
			 (1.0/15.0) * ((double)y[index-3]+y[index+3]) +
			 (1.0/15.0) * ((double)y[index-4]+y[index+4]) +
			 (1.0/15.0) * ((double)y[index-5]+y[index+5]) +
			 (1.0/15.0) * ((double)y[index-6]+y[index+6]) +
			 (1.0/15.0) * ((double)y[index-7]+y[index+7])
			 ;
			}
	  else if(index>=start+6 && index+6 <=end)
			{/* use data +/-6 either side of index */
			 return
			 (1.0/13.0) * ((double)y[index]) +
			 (1.0/13.0) * ((double)y[index-1]+y[index+1]) +
			 (1.0/13.0) * ((double)y[index-2]+y[index+2]) +
			 (1.0/13.0) * ((double)y[index-3]+y[index+3]) +
			 (1.0/13.0) * ((double)y[index-4]+y[index+4]) +
			 (1.0/13.0) * ((double)y[index-5]+y[index+5]) +
			 (1.0/13.0) * ((double)y[index-6]+y[index+6])
			 ;
			}
	  else if(index>=start+5 && index+5 <=end)
			{/* use data +/-5 either side of index */
			 return
			 (1.0/11.0) * ((double)y[index]) +
			 (1.0/11.0) * ((double)y[index-1]+y[index+1]) +
			 (1.0/11.0) * ((double)y[index-2]+y[index+2]) +
			 (1.0/11.0) * ((double)y[index-3]+y[index+3]) +
			 (1.0/11.0) * ((double)y[index-4]+y[index+4]) +
			 (1.0/11.0) * ((double)y[index-5]+y[index+5])
			 ;
			}
	  else if(index>=start+4 && index+4 <=end)
			{/* use data +/-4 either side of index */
			 return
			 (1.0/9.0) * ((double)y[index]) +
			 (1.0/9.0) * ((double)y[index-1]+y[index+1]) +
			 (1.0/9.0) * ((double)y[index-2]+y[index+2]) +
			 (1.0/9.0) * ((double)y[index-3]+y[index+3]) +
			 (1.0/9.0) * ((double)y[index-4]+y[index+4])
			 ;
			}
	  else if(index>=start+3 && index+3 <=end)
			{/* use data +/-3 either side of index */
			 return
			 (1.0/7.0) * ((double)y[index]) +
			 (1.0/7.0) * ((double)y[index-1]+y[index+1]) +
			 (1.0/7.0) * ((double)y[index-2]+y[index+2]) +
			 (1.0/7.0) * ((double)y[index-3]+y[index+3])
			 ;
			}
	  else if(index>=start+2 && index+2 <=end)
			{/* use data +/-2 either side of index */
			 return
			 (1.0/5.0) * ((double)y[index]) +
			 (1.0/5.0) * ((double)y[index-1]+y[index+1]) +
			 (1.0/5.0) * ((double)y[index-2]+y[index+2])
			 ;
			}
	 }
  if(s_order==2 || s_order==3)
	 {// order 2 length 17
	  if(index>=start+8 && index+8 <=end)
			{/* use data +/-8 either side of index */
			 return
			 (43.0/323.0) * ((double)y[index]) +
			 (42.0/323.0) * ((double)y[index-1]+y[index+1]) +
			 (39.0/323.0) * ((double)y[index-2]+y[index+2]) +
			 (34.0/323.0) * ((double)y[index-3]+y[index+3]) +
			 (27.0/323.0) * ((double)y[index-4]+y[index+4]) +
			 (18.0/323.0) * ((double)y[index-5]+y[index+5]) +
			 (7.0/323.0) * ((double)y[index-6]+y[index+6]) +
			 (-6.0/323.0) * ((double)y[index-7]+y[index+7]) +
			 (-21.0/323.0) * ((double)y[index-8]+y[index+8])
			 ;
			}
	  else if(index>=start+7 && index+7 <=end)
			{/* use data +/-7 either side of index */
			 return
			 (167.0/1105.0) * ((double)y[index]) +
			 (162.0/1105.0) * ((double)y[index-1]+y[index+1]) +
			 (147.0/1105.0) * ((double)y[index-2]+y[index+2]) +
			 (122.0/1105.0) * ((double)y[index-3]+y[index+3]) +
			 (87.0/1105.0) * ((double)y[index-4]+y[index+4]) +
			 (42.0/1105.0) * ((double)y[index-5]+y[index+5]) +
			 (-13.0/1105.0) * ((double)y[index-6]+y[index+6]) +
			 (-78.0/1105.0) * ((double)y[index-7]+y[index+7])
			 ;
			}
	  else if(index>=start+6 && index+6 <=end)
			{/* use data +/-6 either side of index */
			 return
			 (25.0/143.0) * ((double)y[index]) +
			 (24.0/143.0) * ((double)y[index-1]+y[index+1]) +
			 (21.0/143.0) * ((double)y[index-2]+y[index+2]) +
			 (16.0/143.0) * ((double)y[index-3]+y[index+3]) +
			 (9.0/143.0) * ((double)y[index-4]+y[index+4]) +
			 (0.0/143.0) * ((double)y[index-5]+y[index+5]) +
			 (-11.0/143.0) * ((double)y[index-6]+y[index+6])
			 ;
			}
	  else if(index>=start+5 && index+5 <=end)
			{/* use data +/-5 either side of index */
			 return
			 (89.0/429.0) * ((double)y[index]) +
			 (84.0/429.0) * ((double)y[index-1]+y[index+1]) +
			 (69.0/429.0) * ((double)y[index-2]+y[index+2]) +
			 (44.0/429.0) * ((double)y[index-3]+y[index+3]) +
			 (9.0/429.0) * ((double)y[index-4]+y[index+4]) +
			 (-36.0/429.0) * ((double)y[index-5]+y[index+5])
			 ;
			}
	  else if(index>=start+4 && index+4 <=end)
			{/* use data +/-4 either side of index */
			 return
			 (59.0/231.0) * ((double)y[index]) +
			 (54.0/231.0) * ((double)y[index-1]+y[index+1]) +
			 (39.0/231.0) * ((double)y[index-2]+y[index+2]) +
			 (14.0/231.0) * ((double)y[index-3]+y[index+3]) +
			 (-21.0/231.0) * ((double)y[index-4]+y[index+4])
			 ;
			}
	  else if(index>=start+3 && index+3 <=end)
			{/* use data +/-3 either side of index */
			 return
			 (7.0/21.0) * ((double)y[index]) +
			 (6.0/21.0) * ((double)y[index-1]+y[index+1]) +
			 (3.0/21.0) * ((double)y[index-2]+y[index+2]) +
			 (-2.0/21.0) * ((double)y[index-3]+y[index+3])
			 ;
			}
	  else if(index>=start+2 && index+2 <=end)
			{/* use data +/-2 either side of index */
			 return
			 (17.0/35.0) * ((double)y[index]) +
			 (12.0/35.0) * ((double)y[index-1]+y[index+1]) +
			 (-3.0/35.0) * ((double)y[index-2]+y[index+2])
			 ;
			}
	 }
  if(s_order==4)
	 {// order 4 length 17
	  if(index>=start+8 && index+8 <=end)
			{/* use data +/-8 either side of index */
			 return
			 (883.0/4199.0) * ((double)y[index]) +
			 (825.0/4199.0) * ((double)y[index-1]+y[index+1]) +
			 (660.0/4199.0) * ((double)y[index-2]+y[index+2]) +
			 (415.0/4199.0) * ((double)y[index-3]+y[index+3]) +
			 (135.0/4199.0) * ((double)y[index-4]+y[index+4]) +
			 (-117.0/4199.0) * ((double)y[index-5]+y[index+5]) +
			 (-260.0/4199.0) * ((double)y[index-6]+y[index+6]) +
			 (-195.0/4199.0) * ((double)y[index-7]+y[index+7]) +
			 (195.0/4199.0) * ((double)y[index-8]+y[index+8])
			 ;
			}
	  else if(index>=start+7 && index+7 <=end)
			{/* use data +/-7 either side of index */
			 return
			 (11063.0/46189.0) * ((double)y[index]) +
			 (10125.0/46189.0) * ((double)y[index-1]+y[index+1]) +
			 (7500.0/46189.0) * ((double)y[index-2]+y[index+2]) +
			 (3755.0/46189.0) * ((double)y[index-3]+y[index+3]) +
			 (-165.0/46189.0) * ((double)y[index-4]+y[index+4]) +
			 (-2937.0/46189.0) * ((double)y[index-5]+y[index+5]) +
			 (-2860.0/46189.0) * ((double)y[index-6]+y[index+6]) +
			 (2145.0/46189.0) * ((double)y[index-7]+y[index+7])
			 ;
			}
	  else if(index>=start+6 && index+6 <=end)
			{/* use data +/-6 either side of index */
			 return
			 (677.0/2431.0) * ((double)y[index]) +
			 (600.0/2431.0) * ((double)y[index-1]+y[index+1]) +
			 (390.0/2431.0) * ((double)y[index-2]+y[index+2]) +
			 (110.0/2431.0) * ((double)y[index-3]+y[index+3]) +
			 (-160.0/2431.0) * ((double)y[index-4]+y[index+4]) +
			 (-198.0/2431.0) * ((double)y[index-5]+y[index+5]) +
			 (110.0/2431.0) * ((double)y[index-6]+y[index+6])
			 ;
			}
	  else if(index>=start+5 && index+5 <=end)
			{/* use data +/-5 either side of index */
			 return
			 (143.0/429.0) * ((double)y[index]) +
			 (120.0/429.0) * ((double)y[index-1]+y[index+1]) +
			 (60.0/429.0) * ((double)y[index-2]+y[index+2]) +
			 (-10.0/429.0) * ((double)y[index-3]+y[index+3]) +
			 (-45.0/429.0) * ((double)y[index-4]+y[index+4]) +
			 (18.0/429.0) * ((double)y[index-5]+y[index+5])
			 ;
			}
	  else if(index>=start+4 && index+4 <=end)
			{/* use data +/-4 either side of index */
			 return
			 (179.0/429.0) * ((double)y[index]) +
			 (135.0/429.0) * ((double)y[index-1]+y[index+1]) +
			 (30.0/429.0) * ((double)y[index-2]+y[index+2]) +
			 (-55.0/429.0) * ((double)y[index-3]+y[index+3]) +
			 (15.0/429.0) * ((double)y[index-4]+y[index+4])
			 ;
			}
	  else if(index>=start+3 && index+3 <=end)
			{/* use data +/-3 either side of index */
			 return
			 (131.0/231.0) * ((double)y[index]) +
			 (75.0/231.0) * ((double)y[index-1]+y[index+1]) +
			 (-30.0/231.0) * ((double)y[index-2]+y[index+2]) +
			 (5.0/231.0) * ((double)y[index-3]+y[index+3])
			 ;
			}
	  else if(index>=start+2 && index+2 <=end)
			{/* Have to drop to order 2, use data +/-2 either side of index */
			 return
			 (17.0/35.0) * ((double)y[index]) +
			 (12.0/35.0) * ((double)y[index-1]+y[index+1]) +
			 (-3.0/35.0) * ((double)y[index-2]+y[index+2])
			 ;
			}
	 }
  /* use +/- 1 point around index - only used when nothing else can do the job ! */
  return((double)y[index-1]+y[index]+y[index+1])/(3.0); /* use  2 points either side of the one requested as we don't have enough points for any more */
}



static double dy_dx_s_g_orig17(float *y,float *x,size_t  start, size_t  end, size_t  index, unsigned int diff_order); // forward reference

/* returns approx (smoothed) derivative of tabular data.
   Assumes x values are all monotonically increasing or decreasing
   Fits a diff_order order polynomial to a small range of the data and differentiates that symbolicaly at "index"
   All orders use up to 17 points (+/-8 either side of point in question)

   This means the 1st order solution gives lower noise, but might supress small spikes
   The 4th order has higher noise but  higer order means finer features are left intact
   At the ends use as many points as possible to at most +/-8 (which means at least 9 points are used)
   As data if fitted by a "custom" least-squares polynomial at every call, the x steps can be any size
   Here order is limited to GS_MAX_ORDER (normally 10)
 */
double dy_dx_polyfit17(float *y,float *x,size_t  start, size_t  end, size_t  index, unsigned int diff_order) /* estimate dy/dx at index - only using data between start and end, diff order 1 to 10 */
{
   if(index<start || index>end || start==end ) return 0; /* index out of range , or only 1 point (start=end) */
   if(diff_order>GS_MAX_ORDER) diff_order=GS_MAX_ORDER; // ensure order is not too large for array below
   // if(diff_order>8) diff_order=8; // limit to 8 as only +/- 8 points around value are used, and only 8 points are used at start & and
   if(diff_order==0) diff_order=1;   // 0 is invalid for differential
   // fit a polynomial of specified order to +/-8 points either side of index (or less if near an end)
   double pcoeff[GS_MAX_ORDER+1];  // 1 more than max allowed diff_order (which is limited to GS_MAX_ORDER by line above)
   for(unsigned int i=0;i<=GS_MAX_ORDER;++i)
		pcoeff[i]=0; // need to initialise array to zero (Polyreg() zeros coefficients used at start based on order
   /* fit polynomial of specified order to first/last few points and differentiate that */

#if 1
   if(index<start+8 && end-start>=16)   // 17 points would be start=0, end=16
	{
	  // near the start, use the first 17 point to fit a polynomial to  (we normally use 17 points, +/-8 values around index)
	  // bool Polyreg(float *y_vals,float *x_vals,size_t  iCount, unsigned int order, double *coeffs);
	  if(Polyreg(y,x,17,diff_order,pcoeff))
#else
   if(index<start+8 && end>=index+8)
	{
	  // near the start, use the points before index and 8 points after to fit a polynomial to  (use 8 as we normally use +/-8 values around index)
	  // bool Polyreg(float *y_vals,float *x_vals,size_t  iCount, unsigned int order, double *coeffs);
	  if(Polyreg(y,x,index+9,diff_order,pcoeff))
#endif
		{// fit worked OK, calculate deravitive
		 for(size_t i=0;i<diff_order;++i)   // array index goes to diff_order, but < here as use i+1
			{pcoeff[i]=pcoeff[i+1]*(double)(i+1); // d(a*x^n)/dx=n*a*x^(n-1)
			}
		 pcoeff[diff_order]=0.0;// last one is now zero as no x^n left after differentiation
		 double xc=x[index]; // want to calculate derivative at x[index]
		 return horner(pcoeff,xc,diff_order-1);
		}
	}

#if 1
   else if(index+8>end && end>=16)
	{
	  // near the end, use the last 17 points to fit a polynomial to  (we normally use 17 points, +/-8 values around index)
	  // bool Polyreg(float *y_vals,float *x_vals,size_t  iCount, unsigned int order, double *coeffs);
	  if(Polyreg(y+end-16,x+end-16,17,diff_order,pcoeff))
#else
   else if(index+8>end && index>=start+8)
	{
	  // near the end, use the points after index and 8 points before to fit a polynomial to
	  // bool Polyreg(float *y_vals,float *x_vals,size_t  iCount, unsigned int order, double *coeffs);
	  if(Polyreg(y+index-8,x+index-8,end-(index-9),diff_order,pcoeff))
#endif
		{// fit worked OK, calculate deravitive
		 for(size_t i=0;i<diff_order;++i)   // array index goes to diff_order, but < here as use i+1
			{pcoeff[i]=pcoeff[i+1]*(double)(i+1); // d(a*x^n)/dx=n*a*x^(n-1)
			}
		 pcoeff[diff_order]=0.0;// last one is now zero as no x^n left after differentiation
		 double xc=x[index]; // want to calculate derivative at x[index]
		 return horner(pcoeff,xc,diff_order-1);
		}
	}
   else if(end>=start+16)
	{// else in the middle , use +/-8 from index (so 17 points in total)
	  // bool Polyreg(float *y_vals,float *x_vals,size_t  iCount, unsigned int order, double *coeffs);
	  if(Polyreg(y+index-8,x+index-8,17,diff_order,pcoeff))
		{// fit worked OK, calculate deravitive
		 for(size_t i=0;i<diff_order;++i)   // array index goes to diff_order, but < here as use i+1
			{pcoeff[i]=pcoeff[i+1]*(double)(i+1); // d(a*x^n)/dx=n*a*x^(n-1)
			}
		 pcoeff[diff_order]=0.0;// last one is now zero as no x^n left after differentiation
		 double xc=x[index]; // want to calculate derivative at x[index]
		 return horner(pcoeff,xc,diff_order-1);
		}
	}
  // less than 17 points in total, or Polyreg failed  use original dy_dx_s_g_orig() code as this deals with a small number of points reasonably sensibily.
  // note that we cannot call  dy_dx_s_g() as that would likley just call this function recursively!
  return dy_dx_s_g_orig17(y,x,start,end,index,diff_order);
}

/* version that used 25 points (index +/-12) */
double dy_dx_polyfit25(float *y,float *x,size_t  start, size_t  end, size_t  index, unsigned int diff_order) /* estimate dy/dx at index - only using data between start and end, diff order 1 to 10 */
{
   if(index<start || index>end || start==end ) return 0; /* index out of range , or only 1 point (start=end) */
   if(diff_order>GS_MAX_ORDER) diff_order=GS_MAX_ORDER; // ensure order is not too large for array below
   if(diff_order==0) diff_order=1;   // 0 is invalid for differential
   // fit a polynomial of specified order to +/-8 points either side of index (or less if near an end)
   double pcoeff[GS_MAX_ORDER+1];  // 1 more than max allowed diff_order (which is limited to GS_MAX_ORDER by line above)
   for(unsigned int i=0;i<=GS_MAX_ORDER;++i)
		pcoeff[i]=0; // need to initialise array to zero (Polyreg() zeros coefficients used at start based on order
   /* fit polynomial of specified order to first/last few points and differentiate that */
   if(index<start+12 && end>=index+12)
	{// near the start, use the points before index and 12 points after to fit a polynomial to  (use 12 as we normally use +/-12 values around index)
	  // bool Polyreg(float *y_vals,float *x_vals,size_t  iCount, unsigned int order, double *coeffs);
	  if(Polyreg(y,x,index+13,diff_order,pcoeff))
		{// fit worked OK, calculate deravitive
		 for(size_t i=0;i<diff_order;++i)   // array index goes to diff_order, but < here as use i+1
			{pcoeff[i]=pcoeff[i+1]*(double)(i+1); // d(a*x^n)/dx=n*a*x^(n-1)
			}
		 pcoeff[diff_order]=0.0;// last one is now zero as no x^n left after differentiation
		 double xc=x[index]; // want to calculate derivative at x[index]
		 return horner(pcoeff,xc,diff_order-1);
		}
	}
   else if(index+12>end && index>=start+12)
	{// near the end, use the points after index and 8 points before to fit a polynomial to
	  // bool Polyreg(float *y_vals,float *x_vals,size_t  iCount, unsigned int order, double *coeffs);
	  if(Polyreg(y+index-12,x+index-12,end-(index-13),diff_order,pcoeff))
		{// fit worked OK, calculate deravitive
		 for(size_t i=0;i<diff_order;++i)   // array index goes to diff_order, but < here as use i+1
			{pcoeff[i]=pcoeff[i+1]*(double)(i+1); // d(a*x^n)/dx=n*a*x^(n-1)
			}
		 pcoeff[diff_order]=0.0;// last one is now zero as no x^n left after differentiation
		 double xc=x[index]; // want to calculate derivative at x[index]
		 return horner(pcoeff,xc,diff_order-1);
		}
	}
   else if(end>=start+24)
	{// else in the middle , use +/-12 from index (so 25 points in total)
	  // bool Polyreg(float *y_vals,float *x_vals,size_t  iCount, unsigned int order, double *coeffs);
	  if(Polyreg(y+index-12,x+index-12,25,diff_order,pcoeff))
		{// fit worked OK, calculate deravitive
		 for(size_t i=0;i<diff_order;++i)   // array index goes to diff_order, but < here as use i+1
			{pcoeff[i]=pcoeff[i+1]*(double)(i+1); // d(a*x^n)/dx=n*a*x^(n-1)
			}
		 pcoeff[diff_order]=0.0;// last one is now zero as no x^n left after differentiation
		 double xc=x[index]; // want to calculate derivative at x[index]
		 return horner(pcoeff,xc,diff_order-1);
		}
	}
  // less than 25 points in total, or Polyreg failed  use original dy_dx_s_g_orig() code as this deals with a small number of points reasonably sensibily.
  // note that we cannot call  dy_dx_s_g() as that would likley just call this function recursively!
  return dy_dx_s_g_orig17(y,x,start,end,index,diff_order);     // we have to use 17 here as there is no 25 version
}


/* returns approx (smoothed) derivative of tabular data.
   Assumes x values are all monotonically increasing or decreasing
   use coefficients from G&S paper when using all 17 points, otherwise use dy_dx()
   This is (much) more efficient than dy_dx_polyfit() for a a 1st, 2nd or 4th order polynomial,but still gives sensible results for all parameters
   This means the 1st order solution gives lower noise, but might supress small spikes
   The 4th order has higher noise but  higer order means finer features are left intact
   Uses Coefficients from original Savitzky & Golay paper.
   x increments can be any size here as we scale by actual step size(s) [ divisors below ]
 */

double dy_dx17(float *y,float *x,size_t  start, size_t  end, size_t  index, unsigned int diff_order) /* estimate dy/dx at index - only using data between start and end, diff order 1=>1, 2,3 => 2 >3 => 4 */
{
  if(index<start || index>end || start==end) return 0; /* index out of range , or only 1 point (start=end)*/
  /* original approach - just use first 2 points of last 2 points (not used if above enabled) */
  if(index==start)  // at start
	{
	 return dy_dx_polyfit17(y,x,start,end,index,diff_order);   // use dy_dx_polyfit() as this is more accurate here
	}
  if(index==end)
	{/* at the end */
	 return dy_dx_polyfit17(y,x,start,end,index,diff_order);   // use dy_dx_polyfit() as this is more accurate here
	}
  /* need to do above 2 checks before we can do this one */
  if(((double)x[index-1]-x[index+1])==0) return dy_dx_polyfit17(y,x,start,end,index,diff_order); // avoid divide by zero error (common in all code below)
  if(diff_order ==4 )
	 {// order 4 , length 17 - this is better at tracking fast changes, but has lower noise rejection than 2nd order
	  /* for 4th order polynomial fit for sequence lengths 17,15,13,11,9,7 & 5 */
	  /* Coefficients from original Savitzky & Golay paper  Table IV */
	  if(index>=start+8 && index+8 <=end)
			{/* use data +/-8 either side of index  (17 points) */
			 return (358.0*2.0/23256.0)  *  ((double)y[index-1]-y[index+1])/((double)x[index-1]-x[index+1]) +
			 (673.0*4.0/23256.0) * ((double)y[index-2]-y[index+2])/((double)x[index-2]-x[index+2]) +
			 (902.0*6.0/23256.0) * ((double)y[index-3]-y[index+3])/((double)x[index-3]-x[index+3]) +
			 (1002.0*8.0/23256.0) * ((double)y[index-4]-y[index+4])/((double)x[index-4]-x[index+4]) +
			 (930.0*10.0/23256.0) * ((double)y[index-5]-y[index+5])/((double)x[index-5]-x[index+5]) +
			 (643.0*12.0/23256.0) * ((double)y[index-6]-y[index+6])/((double)x[index-6]-x[index+6])+
			 (98.0*14.0/23256.0) * ((double)y[index-7]-y[index+7])/((double)x[index-7]-x[index+7])+
			 (-748.0*16.0/23256.0) * ((double)y[index-8]-y[index+8])/((double)x[index-8]-x[index+8])
			 ;
			}
	 }
  else if(diff_order ==2)
	 {// order 2 length 17 - this gives better noise rejection than order 4 above, but will reduce/remove fast "bumps" in he data
	  /* for 2nd order polynomial fit for sequence lengths 17,15,13,11,9,7, 5 & 3 , lower order polynomial means better noise reduction, but might miss narrow spikes (1 spike/11 bins max) */
	  /* Coefficients from original Savitzky & Golay paper  Table III */
	  if(index>=start+8 && index+8 <=end)
			{/* use data +/-8 either side of index */
			 return (1.0*2.0/408.0)  *  ((double)y[index-1]-y[index+1])/((double)x[index-1]-x[index+1]) +
			 (2.0*4.0/408.0) * ((double)y[index-2]-y[index+2])/((double)x[index-2]-x[index+2]) +
			 (3.0*6.0/408.0) * ((double)y[index-3]-y[index+3])/((double)x[index-3]-x[index+3]) +
			 (4.0*8.0/408.0) * ((double)y[index-4]-y[index+4])/((double)x[index-4]-x[index+4]) +
			 (5.0*10.0/408.0) * ((double)y[index-5]-y[index+5])/((double)x[index-5]-x[index+5]) +
			 (6.0*12.0/408.0) * ((double)y[index-6]-y[index+6])/((double)x[index-6]-x[index+6]) +
			 (7.0*14.0/408.0) * ((double)y[index-7]-y[index+7])/((double)x[index-7]-x[index+7]) +
			 (8.0*16.0/408.0) * ((double)y[index-8]-y[index+8])/((double)x[index-8]-x[index+8])
			 ;
			}

	 }
  else if(diff_order==1)
	 {// order 1 length 17 - 1st order is a  "moving average" all offsets have equal weight
	  if(index>=start+8 && index+8 <=end)
			{/* use data +/-8 either side of index */
			 return (1.0*2.0/72.0)  *  ((double)y[index-1]-y[index+1])/((double)x[index-1]-x[index+1]) +
			 (1.0*4.0/72.0) * ((double)y[index-2]-y[index+2])/((double)x[index-2]-x[index+2]) +
			 (1.0*6.0/72.0) * ((double)y[index-3]-y[index+3])/((double)x[index-3]-x[index+3]) +
			 (1.0*8.0/72.0) * ((double)y[index-4]-y[index+4])/((double)x[index-4]-x[index+4]) +
			 (1.0*10.0/72.0) * ((double)y[index-5]-y[index+5])/((double)x[index-5]-x[index+5]) +
			 (1.0*12.0/72.0) * ((double)y[index-6]-y[index+6])/((double)x[index-6]-x[index+6]) +
			 (1.0*14.0/72.0) * ((double)y[index-7]-y[index+7])/((double)x[index-7]-x[index+7]) +
			 (1.0*16.0/72.0) * ((double)y[index-8]-y[index+8])/((double)x[index-8]-x[index+8])
			 ;
			}

	 }
  // if the above special (but very common) cases don't work, then use dy_dx_polyfit() as this is always reasonably accurate (but slower)
  return dy_dx_polyfit17(y,x,start,end,index,diff_order);
}

/* original code - always use S&G coefficient from paper, so accuracy falls at the ends, this is used as a backup to above code as its quite robust */
/* returns approx (smoothed) derivative of tabular data.
   Assumes x values are all monotonically increasing or decreasing
   Fits a 1st, 2nd or 4th order polynomial to a small range of the data and differentiates that with higher frequencies rolled off to give some filtering for noise
   All orders use up to 17 points (+/-8 either side of point in question)

   This means the 1st order solution gives lower noise, but might supress small spikes
   The 4th order has higher noise but  higer order means finer features are left intact
   At the ends gradually reduces the number of points used so points are used symetrically around selected index (except 1st and last point)
   Uses Coefficients from original Savitzky & Golay paper.
   x increments can be any size here as we scale by actual step size(s) [ divisors below ]
   This code is not callable directly by the user, but its used as a "backup" for the above routines should the more
   general purpose code fail.
 */
static double dy_dx_s_g_orig17(float *y,float *x,size_t  start, size_t  end, size_t  index, unsigned int diff_order) /* estimate dy/dx at index - only using data between start and end, diff order 1=>1, 2,3 => 2 >3 => 4 */
{
  if(index<start || index>end || start==end) return 0; /* index out of range , or only 1 point (start=end)*/
  /* original approach - just use first 2 points of last 2 points (not used if above enabled) */
  if(index==start)  // at start
	{if(((double)x[start]-x[start+1])==0) return 0; // avoid divide by zero error
	 return((double)y[start]-y[start+1])/((double)x[start]-x[start+1]); /* use 1st 2 points */
	}
  if(index==end)
	{/* at the end */
	 if(((double)x[end]-x[end-1])==0) return 0; // avoid divide by zero error
	 return((double)y[end]-y[end-1])/((double)x[end]-x[end-1]); /* use last 2 points */
	}
  if(((double)x[index-1]-x[index+1])==0) return 0; // avoid divide by zero error (common in all code below)
  if(diff_order == 4)
	 {// order 4 , length 17 - this is better at tracking fast changes, but has lower noise rejection than 2nd order
	  /* for 4th order polynomial fit for sequence lengths 17,15,13,11,9,7 & 5 */
	  /* Coefficients from original Savitzky & Golay paper  Table IV */
	  if(index>=start+8 && index+8 <=end)
			{/* use data +/-8 either side of index  (17 points) */
			 return (358.0*2.0/23256.0)  *  ((double)y[index-1]-y[index+1])/((double)x[index-1]-x[index+1]) +
			 (673.0*4.0/23256.0) * ((double)y[index-2]-y[index+2])/((double)x[index-2]-x[index+2]) +
			 (902.0*6.0/23256.0) * ((double)y[index-3]-y[index+3])/((double)x[index-3]-x[index+3]) +
			 (1002.0*8.0/23256.0) * ((double)y[index-4]-y[index+4])/((double)x[index-4]-x[index+4]) +
			 (930.0*10.0/23256.0) * ((double)y[index-5]-y[index+5])/((double)x[index-5]-x[index+5]) +
			 (643.0*12.0/23256.0) * ((double)y[index-6]-y[index+6])/((double)x[index-6]-x[index+6])+
			 (98.0*14.0/23256.0) * ((double)y[index-7]-y[index+7])/((double)x[index-7]-x[index+7])+
			 (-748.0*16.0/23256.0) * ((double)y[index-8]-y[index+8])/((double)x[index-8]-x[index+8])
			 ;
			}
	  else if(index>=start+7 && index+7 <=end)
			{/* use data +/-7 either side of index (15 points) */
			 return (7506.0*2.0/334152.0)  *  ((double)y[index-1]-y[index+1])/((double)x[index-1]-x[index+1]) +
			 (13843.0*4.0/334152.0) * ((double)y[index-2]-y[index+2])/((double)x[index-2]-x[index+2]) +
			 (17842.0*6.0/334152.0) * ((double)y[index-3]-y[index+3])/((double)x[index-3]-x[index+3]) +
			 (18334.0*8.0/334152.0) * ((double)y[index-4]-y[index+4])/((double)x[index-4]-x[index+4]) +
			 (14150.0*10.0/334152.0) * ((double)y[index-5]-y[index+5])/((double)x[index-5]-x[index+5]) +
			 (4121.0*12.0/334152.0) * ((double)y[index-6]-y[index+6])/((double)x[index-6]-x[index+6])+
			 (-12922.0*14.0/334152.0) * ((double)y[index-7]-y[index+7])/((double)x[index-7]-x[index+7])
			 ;
			}
	  else if(index>=start+6 && index+6 <=end)
			{/* use data +/-6 either side of index (13 points) */
			 return (832.0*2.0/24024.0)  *  ((double)y[index-1]-y[index+1])/((double)x[index-1]-x[index+1]) +
			 (1489.0*4.0/24024.0) * ((double)y[index-2]-y[index+2])/((double)x[index-2]-x[index+2]) +
			 (1796.0*6.0/24024.0) * ((double)y[index-3]-y[index+3])/((double)x[index-3]-x[index+3]) +
			 (1578.0*8.0/24024.0) * ((double)y[index-4]-y[index+4])/((double)x[index-4]-x[index+4]) +
			 (660.0*10.0/24024.0) * ((double)y[index-5]-y[index+5])/((double)x[index-5]-x[index+5]) +
			 (-1133.0*12.0/24024.0) * ((double)y[index-6]-y[index+6])/((double)x[index-6]-x[index+6])
			 ;
			}
	  else if(index>=start+5 && index+5 <=end)
			{/* use data +/-5 either side of index (11 points) */
			 return (296.0*2.0/5148.0)  *  ((double)y[index-1]-y[index+1])/((double)x[index-1]-x[index+1]) +
			 (503.0*4.0/5148.0) * ((double)y[index-2]-y[index+2])/((double)x[index-2]-x[index+2]) +
			 (532.0*6.0/5148.0) * ((double)y[index-3]-y[index+3])/((double)x[index-3]-x[index+3]) +
			 (294.0*8.0/5148.0) * ((double)y[index-4]-y[index+4])/((double)x[index-4]-x[index+4]) +
			 (-300.0*10.0/5148.0) * ((double)y[index-5]-y[index+5])/((double)x[index-5]-x[index+5])
			 ;
			}
	  else if(index>=start+4 && index+4 <=end)
			{/* use data +/-4 either side of index ( 9 points) */
             /* note 1st coefficient should be 126 - there is a typo in the S&G paper which shows it as 129 ! */
			 return (126.0*2.0/1188.0)  *  ((double)y[index-1]-y[index+1])/((double)x[index-1]-x[index+1]) +
			 (193.0*4.0/1188.0) * ((double)y[index-2]-y[index+2])/((double)x[index-2]-x[index+2]) +
			 (142.0*6.0/1188.0) * ((double)y[index-3]-y[index+3])/((double)x[index-3]-x[index+3]) +
			 (-86.0*8.0/1188.0) * ((double)y[index-4]-y[index+4])/((double)x[index-4]-x[index+4])
			 ;
			}
	  else if(index>=start+3 && index+3 <=end)
			{/* use data +/-3 either side of index ( 7 points) */
			 return (58.0*2.0/252.0)  *  ((double)y[index-1]-y[index+1])/((double)x[index-1]-x[index+1]) +
			 (67.0*4.0/252.0) * ((double)y[index-2]-y[index+2])/((double)x[index-2]-x[index+2]) +
			 (-22.0*6.0/252.0) * ((double)y[index-3]-y[index+3])/((double)x[index-3]-x[index+3])
			 ;
			}
	  else if(index>=start+2 && index+2 <=end)
			{/* use data +/-2 either side of index (5 points)*/
			 return (8.0*2.0/12.0)  *  ((double)y[index-1]-y[index+1])/((double)x[index-1]-x[index+1]) +
			 (-1.0*4.0/12.0) * ((double)y[index-2]-y[index+2])/((double)x[index-2]-x[index+2])
			 ;
			}
	  /* +/-1 is the same for all cases and so is dealt with below */
	 }
  else if(diff_order == 2)
	 {// order 2 length 17 - this gives better noise rejection than order 4 above, but will reduce/remove fast "bumps" in he data
	  /* for 2nd order polynomial fit for sequence lengths 17,15,13,11,9,7, 5 & 3 , lower order polynomial means better noise reduction, but might miss narrow spikes (1 spike/11 bins max) */
	  /* Coefficients from original Savitzky & Golay paper  Table III */
	  if(index>=start+8 && index+8 <=end)
			{/* use data +/-8 either side of index */
			 return (1.0*2.0/408.0)  *  ((double)y[index-1]-y[index+1])/((double)x[index-1]-x[index+1]) +
			 (2.0*4.0/408.0) * ((double)y[index-2]-y[index+2])/((double)x[index-2]-x[index+2]) +
			 (3.0*6.0/408.0) * ((double)y[index-3]-y[index+3])/((double)x[index-3]-x[index+3]) +
			 (4.0*8.0/408.0) * ((double)y[index-4]-y[index+4])/((double)x[index-4]-x[index+4]) +
			 (5.0*10.0/408.0) * ((double)y[index-5]-y[index+5])/((double)x[index-5]-x[index+5]) +
			 (6.0*12.0/408.0) * ((double)y[index-6]-y[index+6])/((double)x[index-6]-x[index+6]) +
			 (7.0*14.0/408.0) * ((double)y[index-7]-y[index+7])/((double)x[index-7]-x[index+7]) +
			 (8.0*16.0/408.0) * ((double)y[index-8]-y[index+8])/((double)x[index-8]-x[index+8])
			 ;
			}
	  else if(index>=start+7 && index+7 <=end)
			{/* use data +/-7 either side of index */
			 return (1.0*2.0/280.0)  *  ((double)y[index-1]-y[index+1])/((double)x[index-1]-x[index+1]) +
			 (2.0*4.0/280.0) * ((double)y[index-2]-y[index+2])/((double)x[index-2]-x[index+2]) +
			 (3.0*6.0/280.0) * ((double)y[index-3]-y[index+3])/((double)x[index-3]-x[index+3]) +
			 (4.0*8.0/280.0) * ((double)y[index-4]-y[index+4])/((double)x[index-4]-x[index+4]) +
			 (5.0*10.0/280.0) * ((double)y[index-5]-y[index+5])/((double)x[index-5]-x[index+5]) +
			 (6.0*12.0/280.0) * ((double)y[index-6]-y[index+6])/((double)x[index-6]-x[index+6]) +
			 (7.0*14.0/280.0) * ((double)y[index-7]-y[index+7])/((double)x[index-7]-x[index+7])
			 ;
			}
	  else if(index>=start+6 && index+6 <=end)
			{/* use data +/-6 either side of index */
			 return (1.0*2.0/182.0)  *  ((double)y[index-1]-y[index+1])/((double)x[index-1]-x[index+1]) +
			 (2.0*4.0/182.0) * ((double)y[index-2]-y[index+2])/((double)x[index-2]-x[index+2]) +
			 (3.0*6.0/182.0) * ((double)y[index-3]-y[index+3])/((double)x[index-3]-x[index+3]) +
			 (4.0*8.0/182.0) * ((double)y[index-4]-y[index+4])/((double)x[index-4]-x[index+4]) +
			 (5.0*10.0/182.0) * ((double)y[index-5]-y[index+5])/((double)x[index-5]-x[index+5]) +
			 (6.0*12.0/182.0) * ((double)y[index-6]-y[index+6])/((double)x[index-6]-x[index+6])
			 ;
			}
	  else if(index>=start+5 && index+5 <=end)
			{/* use data +/-5 either side of index */
			 return (1.0*2.0/110.0)  *  ((double)y[index-1]-y[index+1])/((double)x[index-1]-x[index+1]) +
			 (2.0*4.0/110.0) * ((double)y[index-2]-y[index+2])/((double)x[index-2]-x[index+2]) +
			 (3.0*6.0/110.0) * ((double)y[index-3]-y[index+3])/((double)x[index-3]-x[index+3]) +
			 (4.0*8.0/110.0) * ((double)y[index-4]-y[index+4])/((double)x[index-4]-x[index+4]) +
			 (5.0*10.0/110.0) * ((double)y[index-5]-y[index+5])/((double)x[index-5]-x[index+5])
			 ;
			}
	  else if(index>=start+4 && index+4 <=end)
			{/* use data +/-4 either side of index */
			 return (1.0*2.0/60.0)  *  ((double)y[index-1]-y[index+1])/((double)x[index-1]-x[index+1]) +
			 (2.0*4.0/60.0) * ((double)y[index-2]-y[index+2])/((double)x[index-2]-x[index+2]) +
			 (3.0*6.0/60.0) * ((double)y[index-3]-y[index+3])/((double)x[index-3]-x[index+3]) +
			 (4.0*8.0/60.0) * ((double)y[index-4]-y[index+4])/((double)x[index-4]-x[index+4])
			 ;
			}
	  else if(index>=start+3 && index+3 <=end)
			{/* use data +/-3 either side of index */
			 return (1.0*2.0/28.0)  *  ((double)y[index-1]-y[index+1])/((double)x[index-1]-x[index+1]) +
			 (2.0*4.0/28.0) * ((double)y[index-2]-y[index+2])/((double)x[index-2]-x[index+2]) +
			 (3.0*6.0/28.0) * ((double)y[index-3]-y[index+3])/((double)x[index-3]-x[index+3])
			 ;
			}
	  else if(index>=start+2 && index+2 <=end)
			{/* use data +/-2 either side of index */
			 return (1.0*2.0/10.0)  *  ((double)y[index-1]-y[index+1])/((double)x[index-1]-x[index+1]) +
			 (2.0*4.0/10.0) * ((double)y[index-2]-y[index+2])/((double)x[index-2]-x[index+2])
			 ;
			}
	   /* +/-1 either side gives the same equation as used below for +/- 1 point */
	 }
  else if(diff_order==1)
	 {// order 1 length 17 - 1st order is a  "moving average" all offsets have equal weight
	  if(index>=start+8 && index+8 <=end)
			{/* use data +/-8 either side of index */
			 return (1.0*2.0/72.0)  *  ((double)y[index-1]-y[index+1])/((double)x[index-1]-x[index+1]) +
			 (1.0*4.0/72.0) * ((double)y[index-2]-y[index+2])/((double)x[index-2]-x[index+2]) +
			 (1.0*6.0/72.0) * ((double)y[index-3]-y[index+3])/((double)x[index-3]-x[index+3]) +
			 (1.0*8.0/72.0) * ((double)y[index-4]-y[index+4])/((double)x[index-4]-x[index+4]) +
			 (1.0*10.0/72.0) * ((double)y[index-5]-y[index+5])/((double)x[index-5]-x[index+5]) +
			 (1.0*12.0/72.0) * ((double)y[index-6]-y[index+6])/((double)x[index-6]-x[index+6]) +
			 (1.0*14.0/72.0) * ((double)y[index-7]-y[index+7])/((double)x[index-7]-x[index+7]) +
			 (1.0*16.0/72.0) * ((double)y[index-8]-y[index+8])/((double)x[index-8]-x[index+8])
			 ;
			}
	  else if(index>=start+7 && index+7 <=end)
			{/* use data +/-7 either side of index */
			 return (1.0*2.0/56.0)  *  ((double)y[index-1]-y[index+1])/((double)x[index-1]-x[index+1]) +
			 (1.0*4.0/56.0) * ((double)y[index-2]-y[index+2])/((double)x[index-2]-x[index+2]) +
			 (1.0*6.0/56.0) * ((double)y[index-3]-y[index+3])/((double)x[index-3]-x[index+3]) +
			 (1.0*8.0/56.0) * ((double)y[index-4]-y[index+4])/((double)x[index-4]-x[index+4]) +
			 (1.0*10.0/56.0) * ((double)y[index-5]-y[index+5])/((double)x[index-5]-x[index+5]) +
			 (1.0*12.0/56.0) * ((double)y[index-6]-y[index+6])/((double)x[index-6]-x[index+6]) +
			 (1.0*14.0/56.0) * ((double)y[index-7]-y[index+7])/((double)x[index-7]-x[index+7])
			 ;
			}
	  else if(index>=start+6 && index+6 <=end)
			{/* use data +/-6 either side of index */
			 return (1.0*2.0/42.0)  *  ((double)y[index-1]-y[index+1])/((double)x[index-1]-x[index+1]) +
			 (1.0*4.0/42.0) * ((double)y[index-2]-y[index+2])/((double)x[index-2]-x[index+2]) +
			 (1.0*6.0/42.0) * ((double)y[index-3]-y[index+3])/((double)x[index-3]-x[index+3]) +
			 (1.0*8.0/42.0) * ((double)y[index-4]-y[index+4])/((double)x[index-4]-x[index+4]) +
			 (1.0*10.0/42.0) * ((double)y[index-5]-y[index+5])/((double)x[index-5]-x[index+5]) +
			 (1.0*12.0/42.0) * ((double)y[index-6]-y[index+6])/((double)x[index-6]-x[index+6])
			 ;
			}
	  else if(index>=start+5 && index+5 <=end)
			{/* use data +/-5 either side of index */
			 return (1.0*2.0/30.0)  *  ((double)y[index-1]-y[index+1])/((double)x[index-1]-x[index+1]) +
			 (1.0*4.0/30.0) * ((double)y[index-2]-y[index+2])/((double)x[index-2]-x[index+2]) +
			 (1.0*6.0/30.0) * ((double)y[index-3]-y[index+3])/((double)x[index-3]-x[index+3]) +
			 (1.0*8.0/30.0) * ((double)y[index-4]-y[index+4])/((double)x[index-4]-x[index+4]) +
			 (1.0*10.0/30.0) * ((double)y[index-5]-y[index+5])/((double)x[index-5]-x[index+5])
			 ;
			}
	  else if(index>=start+4 && index+4 <=end)
			{/* use data +/-4 either side of index */
			 return (1.0*2.0/20.0)  *  ((double)y[index-1]-y[index+1])/((double)x[index-1]-x[index+1]) +
			 (1.0*4.0/20.0) * ((double)y[index-2]-y[index+2])/((double)x[index-2]-x[index+2]) +
			 (1.0*6.0/20.0) * ((double)y[index-3]-y[index+3])/((double)x[index-3]-x[index+3]) +
			 (1.0*8.0/20.0) * ((double)y[index-4]-y[index+4])/((double)x[index-4]-x[index+4])
			 ;
			}
	  else if(index>=start+3 && index+3 <=end)
			{/* use data +/-3 either side of index */
			 return (1.0*2.0/12.0)  *  ((double)y[index-1]-y[index+1])/((double)x[index-1]-x[index+1]) +
			 (1.0*4.0/12.0) * ((double)y[index-2]-y[index+2])/((double)x[index-2]-x[index+2]) +
			 (1.0*6.0/12.0) * ((double)y[index-3]-y[index+3])/((double)x[index-3]-x[index+3])
			 ;
			}
	  else if(index>=start+2 && index+2 <=end)
			{/* use data +/-2 either side of index */
			 return (1.0*2.0/6.0)  *  ((double)y[index-1]-y[index+1])/((double)x[index-1]-x[index+1]) +
			 (1.0*4.0/6.0) * ((double)y[index-2]-y[index+2])/((double)x[index-2]-x[index+2])
			 ;
			}
	   /* +/-1 either side gives the same equation as used below for +/- 1 point as multiplier is 1*2/2 */
	 }

  /* use +/- 1 point around index - only used when nothing else can do the job ! */
  return((double)y[index-1]-y[index+1])/((double)x[index-1]-x[index+1]); /* use  2 points either side of the one requested as we don't have enough points for any more */
}



/* returns approx  2nd derivative of tabular data.
   uses 3 point equation from https://mathformeremortals.wordpress.com/2013/01/12/a-numerical-second-derivative-from-three-points/
   Note we could also get the 2nd deravitive by taking the 1st derivative of the 1st derivative and the 3rd as
   d3y/dx3 = d/dx(d/dx(d/dx(y(x))))  etc, but this would result in multiple repeats of the smoothing action
   This code uses the smoothing function above and takes the 2nd derivative of the result of that
   This makes the routine much simpler than if we used Savitzky Golay coefficients from their paper (and worked out how to apply these to data with potentially non-uniform x increments)
 */
double d2y_d2x_orig17(float *y,float *x,size_t  start, size_t  end, size_t  index, unsigned int diff_order) /* estimate d2y/d2x at index - only using data between start and end, diff order 1=>1, 2,3 => 2 >3 => 4 */
{ double y1,y2,y3,x1,x2,x3; // 3 points for simple cases
  if(index<start || index>end || start==end || start+1==end ) return 0; /* index out of range , or only 1 or 2 points point (start=end)*/
  if(index==start)
	{// at start, use 1st 3 points
	 // Savitzky_Golay_smoothing(float *y,float *x,size_t  start, size_t  end, size_t  index, int s_order)
	 x1=x[index];
	 x2=x[index+1];
	 x3=x[index+2];
	 y1=Savitzky_Golay_smoothing17(y,x,start, end,index, diff_order);
	 y2=Savitzky_Golay_smoothing17(y,x,start, end,index+1, diff_order);
	 y3=Savitzky_Golay_smoothing17(y,x,start, end,index+2, diff_order);
	}
  else if(index==end)
	{// at end, use last 3 points
	 x1=x[index-2];
	 x2=x[index-1];
	 x3=x[index];
	 y1=Savitzky_Golay_smoothing17(y,x,start, end,index-2, diff_order);
	 y2=Savitzky_Golay_smoothing17(y,x,start, end,index-1, diff_order);
	 y3=Savitzky_Golay_smoothing17(y,x,start, end,index, diff_order);
	}
  else
	{
	 // general case for 3 points in the middle
	 x1=x[index-1];
	 x2=x[index];
	 x3=x[index+1];
	 y1=Savitzky_Golay_smoothing17(y,x,start, end,index-1, diff_order);
	 y2=Savitzky_Golay_smoothing17(y,x,start, end,index, diff_order);
	 y3=Savitzky_Golay_smoothing17(y,x,start, end,index+1, diff_order);
	}
  // trap potential divide by zero issues  (should not ever happen if x values are monotonically increasing)
  if((x2-x1)==0 || (x3-x2)==0 || (x3-x1)==0)
	return 0;
  return (2.0*y1)/((x2-x1)*(x3-x1))-(2.0*y2)/((x3-x2)*(x2-x1))+(2.0*y3)/((x3-x2)*(x3-x1));
}

/* returns approx (smoothed) 2nd derivative of tabular data.
   Assumes x values are all monotonically increasing or decreasing
   Fits a diff_order order polynomial to a small range of the data and differentiates that symbolicaly at "index"
   All orders use up to 17 points (+/-8 either side of point in question)

   This means the 1st order solution gives lower noise, but might supress small spikes
   The 4th order has higher noise but  higer order means finer features are left intact
   At the ends use as many points as possible to at most +/-8 (which means at least 9 points are used)
   As data if fitted by a "custom" least-squares polynomial at every call, the x steps can be any size
   Here order is limited to GS_MAX_ORDER (normally 10)
 */
double d2y_d2x17(float *y,float *x,size_t  start, size_t  end, size_t  index, unsigned int diff_order) /* estimate dy/dx at index - only using data between start and end, diff order 1=>1, 2,3 => 2 >3 => 4 */
{
   if(index<start || index>end || start==end ) return 0; /* index out of range , or only 1 point (start=end) */
   if(diff_order>GS_MAX_ORDER) diff_order=GS_MAX_ORDER; // ensure order is not too large for array below
   if(diff_order<2) return d2y_d2x_orig17(y,x,start,end,index,diff_order);// trap it here to make sure we don't go outside an array bounds
   // fit a polynomial of specified order to +/-8 points either side of index (or less if near an end)
   double pcoeff[GS_MAX_ORDER+1];  // 1 more than max allowed diff_order (which is limited to GS_MAX_ORDER by line above)
   for(unsigned int i=0;i<=GS_MAX_ORDER;++i)
		pcoeff[i]=0; // need to initialise array to zero (Polyreg() zeros coefficients used at start based on order
   /* fit polynomial of specified order to first/last few points and differentiate that */
#if 1
   if(index<start+8 && end-start>=16)   // 17 points would be start=0, end=16
	{
	  // near the start, use the first 17 point to fit a polynomial to  (we normally use 17 points, +/-8 values around index)
	  // bool Polyreg(float *y_vals,float *x_vals,size_t  iCount, unsigned int order, double *coeffs);
	  if(Polyreg(y,x,17,diff_order,pcoeff))
#else
   if(index<start+8 && end>=index+8)
	{// near the start, use the points before index and 8 points after to fit a polynomial to  (use 8 as we normally use +/-8 values around index)
	  // bool Polyreg(float *y_vals,float *x_vals,size_t  iCount, unsigned int order, double *coeffs);
	  if(Polyreg(y,x,index+9,diff_order,pcoeff))
#endif
		{// fit worked OK, calculate deravitive
		 for(size_t i=0;i<diff_order;++i)   // array index goes to diff_order, but < here as use i+1
			{pcoeff[i]=pcoeff[i+1]*(double)(i+1); // d(a*x^n)/dx=n*a*x^(n-1)
			}
		 pcoeff[diff_order]=0.0;// last one is now zero as no x^n left after differentiation
		 // now calculate 2nd derivative
		 for(size_t i=0;i<diff_order;++i)   // array index goes to diff_order, but < here as use i+1
			{pcoeff[i]=pcoeff[i+1]*(double)(i+1); // d(a*x^n)/dx=n*a*x^(n-1)
			}
		 pcoeff[diff_order]=0.0;// last one is now zero as no x^n left after differentiation
		 double xc=x[index]; // want to calculate 2nd derivative at x[index]
		 return horner(pcoeff,xc,diff_order-2);
		}
	}
 #if 1
   else if(index+8>end && end>=16)
	{
	  // near the end, use the last 17 points to fit a polynomial to  (we normally use 17 points, +/-8 values around index)
	  // bool Polyreg(float *y_vals,float *x_vals,size_t  iCount, unsigned int order, double *coeffs);
	  if(Polyreg(y+end-16,x+end-16,17,diff_order,pcoeff))
#else
   else if(index+8>end && index>=start+8)
	{// near the end, use the points after index and 8 points before to fit a polynomial to
	  // bool Polyreg(float *y_vals,float *x_vals,size_t  iCount, unsigned int order, double *coeffs);
	  if(Polyreg(y+index-8,x+index-8,end-(index-9),diff_order,pcoeff))
#endif
		{// fit worked OK, calculate deravitive
		 for(size_t i=0;i<diff_order;++i)   // array index goes to diff_order, but < here as use i+1
			{pcoeff[i]=pcoeff[i+1]*(double)(i+1); // d(a*x^n)/dx=n*a*x^(n-1)
			}
		 pcoeff[diff_order]=0.0;// last one is now zero as no x^n left after differentiation
		 // now calculate 2nd derivative
		 for(size_t i=0;i<diff_order;++i)   // array index goes to diff_order, but < here as use i+1
			{pcoeff[i]=pcoeff[i+1]*(double)(i+1); // d(a*x^n)/dx=n*a*x^(n-1)
			}
		 pcoeff[diff_order]=0.0;// last one is now zero as no x^n left after differentiation
		 double xc=x[index]; // want to calculate 2nd derivative at x[index]
		 return horner(pcoeff,xc,diff_order-2);
		}
	}
   else if(end>=start+16)
	{// else in the middle , use +/-8 from index (so 17 points in total)
	  // bool Polyreg(float *y_vals,float *x_vals,size_t  iCount, unsigned int order, double *coeffs);
	  if(Polyreg(y+index-8,x+index-8,17,diff_order,pcoeff))
		{// fit worked OK, calculate deravitive
		 for(size_t i=0;i<diff_order;++i)   // array index goes to diff_order, but < here as use i+1
			{pcoeff[i]=pcoeff[i+1]*(double)(i+1); // d(a*x^n)/dx=n*a*x^(n-1)
			}
		 pcoeff[diff_order]=0.0;// last one is now zero as no x^n left after differentiation
		 // now calculate 2nd derivative
		 for(size_t i=0;i<diff_order;++i)   // array index goes to diff_order, but < here as use i+1
			{pcoeff[i]=pcoeff[i+1]*(double)(i+1); // d(a*x^n)/dx=n*a*x^(n-1)
			}
		 pcoeff[diff_order]=0.0;// last one is now zero as no x^n left after differentiation
		 double xc=x[index]; // want to calculate 2nd derivative at x[index]
		 return horner(pcoeff,xc,diff_order-2);
		}
	}
  // less than 17 points in total, or Polyreg failed  use original d2y_d2x_orig() code as this deals with a small number of points reasonably sensibily.
  return d2y_d2x_orig17(y,x,start,end,index,diff_order);
}

/* as above but using 25 points (index +/- 12) */
double d2y_d2x25(float *y,float *x,size_t  start, size_t  end, size_t  index, unsigned int diff_order) /* estimate dy/dx at index - only using data between start and end, diff order 1 to 10 */
{
   if(index<start || index>end || start==end ) return 0; /* index out of range , or only 1 point (start=end) */
   if(diff_order>GS_MAX_ORDER) diff_order=GS_MAX_ORDER; // ensure order is not too large for array below
   if(diff_order<2) return d2y_d2x_orig17(y,x,start,end,index,diff_order);// trap it here to make sure we don't go outside an array bounds
   // fit a polynomial of specified order to +/-12 points either side of index (or less if near an end)
   double pcoeff[GS_MAX_ORDER+1];  // 1 more than max allowed diff_order (which is limited to GS_MAX_ORDER by line above)
   for(unsigned int i=0;i<=GS_MAX_ORDER;++i)
		pcoeff[i]=0; // need to initialise array to zero (Polyreg() zeros coefficients used at start based on order
   /* fit polynomial of specified order to first/last few points and differentiate that */
#if 1
   if(index<start+12 && end-start>=24)   // 25 points would be start=0, end=24
	{
	  // near the start, use the first 25 point to fit a polynomial to  (we normally use 25 points, +/-12 values around index)
	  // bool Polyreg(float *y_vals,float *x_vals,size_t  iCount, unsigned int order, double *coeffs);
	  if(Polyreg(y,x,25,diff_order,pcoeff))
#else
   if(index<start+12 && end>=index+12)
	{// near the start, use the points before index and 12 points after to fit a polynomial to  (use 12 as we normally use +/-12 values around index)
	  // bool Polyreg(float *y_vals,float *x_vals,size_t  iCount, unsigned int order, double *coeffs);
	  if(Polyreg(y,x,index+13,diff_order,pcoeff))
#endif
		{// fit worked OK, calculate deravitive
		 for(size_t i=0;i<diff_order;++i)   // array index goes to diff_order, but < here as use i+1
			{pcoeff[i]=pcoeff[i+1]*(double)(i+1); // d(a*x^n)/dx=n*a*x^(n-1)
			}
		 pcoeff[diff_order]=0.0;// last one is now zero as no x^n left after differentiation
		 // now calculate 2nd derivative
		 for(size_t i=0;i<diff_order;++i)   // array index goes to diff_order, but < here as use i+1
			{pcoeff[i]=pcoeff[i+1]*(double)(i+1); // d(a*x^n)/dx=n*a*x^(n-1)
			}
		 pcoeff[diff_order]=0.0;// last one is now zero as no x^n left after differentiation
		 double xc=x[index]; // want to calculate 2nd derivative at x[index]
		 return horner(pcoeff,xc,diff_order-2);
		}
	}
#if 1
   else if(index+12>end && end>=24)
	{
	  // near the end, use the last 25 points to fit a polynomial to  (we normally use 25 points, +/-12 values around index)
	  // bool Polyreg(float *y_vals,float *x_vals,size_t  iCount, unsigned int order, double *coeffs);
	  if(Polyreg(y+end-24,x+end-24,25,diff_order,pcoeff))
#else
   else if(index+12>end && index>=start+12)
	{// near the end, use the points after index and 12 points before to fit a polynomial to
	  // bool Polyreg(float *y_vals,float *x_vals,size_t  iCount, unsigned int order, double *coeffs);
	  if(Polyreg(y+index-12,x+index-12,end-(index-13),diff_order,pcoeff))
#endif
		{// fit worked OK, calculate deravitive
		 for(size_t i=0;i<diff_order;++i)   // array index goes to diff_order, but < here as use i+1
			{pcoeff[i]=pcoeff[i+1]*(double)(i+1); // d(a*x^n)/dx=n*a*x^(n-1)
			}
		 pcoeff[diff_order]=0.0;// last one is now zero as no x^n left after differentiation
		 // now calculate 2nd derivative
		 for(size_t i=0;i<diff_order;++i)   // array index goes to diff_order, but < here as use i+1
			{pcoeff[i]=pcoeff[i+1]*(double)(i+1); // d(a*x^n)/dx=n*a*x^(n-1)
			}
		 pcoeff[diff_order]=0.0;// last one is now zero as no x^n left after differentiation
		 double xc=x[index]; // want to calculate 2nd derivative at x[index]
		 return horner(pcoeff,xc,diff_order-2);
		}
	}
   else if(end>=start+24)
	{// else in the middle , use +/-12 from index (so 25 points in total)
	  // bool Polyreg(float *y_vals,float *x_vals,size_t  iCount, unsigned int order, double *coeffs);
	  if(Polyreg(y+index-12,x+index-12,25,diff_order,pcoeff))
		{// fit worked OK, calculate deravitive
		 for(size_t i=0;i<diff_order;++i)   // array index goes to diff_order, but < here as use i+1
			{pcoeff[i]=pcoeff[i+1]*(double)(i+1); // d(a*x^n)/dx=n*a*x^(n-1)
			}
		 pcoeff[diff_order]=0.0;// last one is now zero as no x^n left after differentiation
		 // now calculate 2nd derivative
		 for(size_t i=0;i<diff_order;++i)   // array index goes to diff_order, but < here as use i+1
			{pcoeff[i]=pcoeff[i+1]*(double)(i+1); // d(a*x^n)/dx=n*a*x^(n-1)
			}
		 pcoeff[diff_order]=0.0;// last one is now zero as no x^n left after differentiation
		 double xc=x[index]; // want to calculate 2nd derivative at x[index]
		 return horner(pcoeff,xc,diff_order-2);
		}
	}
  // less than 25 points in total, or Polyreg failed  use original d2y_d2x_orig() code as this deals with a small number of points reasonably sensibily.
  return d2y_d2x_orig17(y,x,start,end,index,diff_order); /* have to use 17 as no 25 version  */
}

