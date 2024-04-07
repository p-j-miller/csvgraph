/*  Smoothing Spline
    Takes a parameter lambda - lambda defines the amount of smoothing done, from 0 (max smoothing)=>1 no smoothing
    Note "max smoothing" is actually a least squares straight line fit (but this is not a very efficient way to calculate that)
    It's up to the user to select a suitable value for lambda (0...1)
    Lambda values <= DBL_EPSILON = 2.22 e-16 are treated as zero, but for example Lambda=2.221e-16 can be useful (this gives maximum smoothing without swapping to a straight line)
    smoothing_spline.h has #defines that allow either float * ot double * arrays to be processed.
    x values must be monotonocally increasing.
    
    The code dynamically allocates memory for ~ 10*n doubles (and frees it after use).
    If there is an error the output is the same as the input (ie as if lambda=1)
    
    This version by Peter Miller 28-3-2024
    References:
    C.H. Reinsch "Smothing by spline functions", Numer. Math. 10, 177-183, 1967  
	Carl de Boor, "A Practical Guide to Splines", 1978, in particular chapter XIV	 
    M.F. Hutchinson and F.R. De Hoog, "Smoothing noisy data with spline functions",
     March 1985 Numerische Mathematik 47(1):99-106 DOI:10.1007/BF01389878
     and ACM-Trans. Math. Software, Vol.12, No. 2,Jun., 1986, p. 150.	 
    D.S.G. POLLOCK "SMOOTHING SPLINES", December 1999 DOI:10.1016/B978-012560990-6/50013-0
*/ 	
/*----------------------------------------------------------------------------
 * Copyright (c) 2024 Peter Miller
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
#define SDEBUG /* define for some debugging crprintf() in code, if not defined crprintf() calls do nothing */
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <float.h> /*  DBL_EPSILON , FLT_MAX etc.  */
#ifdef SDEBUG
 #include "rprintf.h"
#else
 #define crprintf(...) /* nothing if not debugging */
#endif
#include "smoothing_spline.h"
/* 2D array simulations for 1D arrays - I assume optimiser is clever enought to do constant integer maths at compile time */
/* note given the Fortan roots to these algorithms "col" defines a 1D array for "row" which is the opposite to how a 2D array would normally be structured in C */
#define C(row, col)  c[ic * (col) + (row)]
#define R(row, col)  r[(n + 2) * (col) + (row)]
#define T(row, col)  t[(n + 2) * (col) + (row)]
#define WK(row, col)  wk[(n + 2) * (col) + (row)]
static void spcof1(s_spline_float *x, double avh, s_spline_float *y, size_t n, double p, double q, double *a, double *c,
                   size_t ic, double *u, double *v)
/*
 * Calculates coefficients of a cubic smoothing spline from
 * parameters calculated by subroutine spfit1.
 * a is yo , so check this is a number (not inf or nan) and if not make it y (no filtering)
 */
{
  double h, qh;
  bool overflow=false;
  /* Calculate a */
  qh = q / ((avh) * (avh));
  for (size_t i = 0; i < n; i++)
    {
      a[i] = y[i] - p * v[i + 1];
      if(!isfinite(a[i]))
      	{overflow=true;
      	}
      u[i + 1] *= qh;
    }
  if(overflow)
	{for (size_t i = 0; i < n; i++)
   		{
      	 a[i] = y[i]; // no filtering
      	}
    }
  /* Calculate c */
  for (size_t i = 1; i < n; i++)
    {
      h = x[i] - x[i - 1];
      C(i - 1, 2) = (u[i + 1] - u[i]) / (3 * h);
      C(i - 1, 0) = (a[i] - a[i - 1]) / h - (h * C(i - 1, 2) + u[i]) * h;
      C(i - 1, 1) = u[i];
    }
}
static void spint1(s_spline_float *x, double *avh, s_spline_float *y, size_t n, double *a, double *c, size_t ic,
                   double *r, double *t)
/*
 * Initializes the arrays c, r and t for one dimensional cubic
 * smoothing spline fitting by subroutine spfit1.  
 * The average of the differences x(i+1) - x(i) is calculated
 * in avh in order to avoid underflow and overflow problems in
 * spfit1.
 * Note x values must be monotonically increasing, n>=3 and ic>= n-1  !
 */
{
  double e, f, g, h;
  /* Get average x spacing in avh = (max X value - min X value) / number of points */
  *avh = (x[n-1]-x[0]) /n; 
  /* Initialize h, f */
  h = (x[1] - x[0]) / (*avh);
  f = (y[1] - y[0]) / h;
  if(!isfinite(f)) 
  	{if(y[1]>=y[0]) f=FLT_MAX; // stop f going to infinity, but keep sign correct, FTL_MAX avoids issues in future calculations using f (DBL_MAX could easily cause another overflow when f is used again)
  	 else f= -FLT_MAX;
  	}
  /* Calculate a, t, r - most divides cannot overflow */
  for (size_t i = 1; i < n - 1; i++)
    {
      g = h;
      h = (x[i + 1] - x[i]) / (*avh);
      e = f;
      f = (y[i + 1] - y[i]) / h;
	  if(!isfinite(f)) 
	  	{if(y[i+1]>=y[i]) f=FLT_MAX; // stop f going to infinity, but keep sign correct
	  	 else f= -FLT_MAX;
	  	}      
      a[i] = f - e;
      T(i + 1, 0) = 2 * (g + h) / 3;
      T(i + 1, 1) = h / 3;
      R(i + 1, 2) = 1.0 / g;
      R(i + 1, 0) = 1.0 / h;
      R(i + 1, 1) = -1.0 / g - 1.0 / h;
    }
  /* Calculate c = r'*r */
  R(n, 1) = 0;
  R(n, 2) = 0;
  R(n + 1, 2) = 0;
  for (size_t i = 1; i < n - 1; i++)
    {
      C(i, 0) = R(i + 1, 0) * R(i + 1, 0) + R(i + 1, 1) * R(i + 1, 1) + R(i + 1, 2) * R(i + 1, 2);
      C(i, 1) = R(i + 1, 0) * R(i + 2, 1) + R(i + 1, 1) * R(i + 2, 2);
      C(i, 2) = R(i + 1, 0) * R(i + 3, 2);
    }
  return ; 
}
static void spfit1(s_spline_float *x, double avh, size_t n, double p, double q, 
                    double *a, double *c, size_t ic, double *r, double *t, double *u, double *v)
/*
 * Fits a cubic smoothing spline to data 
 * for a given value of the smoothing parameter
 * rho using an algorithm based on that of C.H. Reinsch (1967),
 * Numer. Math. 10, 177-183.
 *
 * The arrays a, c, r and t are assumed to have been initialized
 * by the subroutine spint1.  
 * overflow and underflow problems are avoided by using p and q (where p+q==1 and p,q both 0..1) 
 * and by scaling the differences x(i+1) - x(i) by avh.
 * There are a few traps for overflows - just in case!
 *
 */
{
  double f, g, h;
  /* Rational cholesky decomposition of p*c + q*t */
  f = g = h = 0;
  R(0, 0) = 0;
  R(1, 0) = 0;
  for (size_t i = 2; i < n; i++)
    { double Ri;// just in case we end by with a divide by zero
      R(i - 2, 2) = g * R(i - 2, 0);
      R(i - 1, 1) = f * R(i - 1, 0);
      Ri = 1 / (p * C(i - 1, 0) + q * T(i, 0) - f * R(i - 1, 1) - g * R(i - 2, 2));
      if(isfinite(Ri))
      	R(i,0)=Ri;
      else
      	R(i,0)=0.0; // this makes sense as divide by zero means there are multiple solutions so we can just pick 1
      f = p * C(i - 1, 1) + q * T(i, 1) - h * R(i - 1, 1);
      g = h;
      h = p * C(i - 1, 2);
    }
  /* Solve for u */
  u[0] = 0;
  u[1] = 0;
  for (size_t i = 2; i < n; i++) u[i] = a[i - 1] - R(i - 1, 1) * u[i - 1] - R(i - 2, 2) * u[i - 2];
  u[n] = 0;
  u[n + 1] = 0;
  for (size_t i = n - 1; i > 1; i--) u[i] = R(i, 0) * u[i] - R(i, 1) * u[i + 1] - R(i, 2) * u[i + 2];
  /* Calculate residual vector v */
  h = 0;
  for (size_t i = 1; i < n; i++)
    {
      g = h;
      h = (u[i + 1] - u[i]) / ((x[i] - x[i - 1]) / (avh));
      if(!isfinite(h)) h=g; // above line could overflow if x[i] very close to x[i-1], avh is large and u[i+1]-u[i] is large, setting to g means v[i]=0 so yo=y in spcof1 which makes sense
      v[i] =  (h - g);
    }
  v[n] =  (-h);
  /* Calculate upper three bands of inverse matrix */
  R(n, 0) = 0;
  R(n, 1) = 0;
  R(n + 1, 0) = 0;
  for (size_t i = n - 1; i > 1; i--)
    {
      g = R(i, 1);
      h = R(i, 2);
      R(i, 1) = -g * R(i + 1, 0) - h * R(i + 1, 1);
      R(i, 2) = -g * R(i + 1, 1) - h * R(i + 2, 0);
      R(i, 0) -= (g * R(i, 1) + h * R(i, 2));
    }  
}
/* Linear regression for straight line this version just makes 1 pass over data, uses "updating algorithm" for higher accuracy */
static void lin_reg(s_spline_float *x,s_spline_float*y, size_t start, size_t  end, double *m, double *c, double *r2) /* linear regression */
{long double meanx=0,meany=0; /* initial values set to mean that N=0 or N=1 do not need to be treated as special cases below */
 long double meanx2=0,meanxy=0,meany2=0; /* mean x^2 , mean x*y and mean y^2 */
 long double xi,yi;
 size_t  i,N=0; /* N is count of items */
 for(i=start;i<=end;++i) /* only use 1 pass here - to calculate means directly */
        {++N;
         xi = x[i];
         yi = y[i];
         meanx+= (xi-meanx)/(long double) N; /* calculate means as mi+1=mi+(xi+1 - mi)/i+1 , this should give accurate results and avoids the possibility of the "sum" overflowing*/
         meany+= (yi-meany)/(long double) N;
         meanx2+= (xi*xi-meanx2)/(long double) N;
         meanxy+= (xi*yi-meanxy)/(long double) N;
         meany2+= (yi*yi-meany2)/(long double) N;
        }
 if(meanx*meanx==meanx2)
        {/* y is independent of x [if this is not trapped we get a divide by zero error trying to set m to infinity] or 0 or 1 point given */
         *m=0.0 ;
		 *c=(double)meany;
         *r2=0.0;
        }
 else   {/* have a valid line */
         double rt,rb,rm;
		 rm=(double)((meanx*meany-meanxy)/(meanx*meanx-meanx2));
         if(!isfinite(rm)) rm=0; // trap overflow or other "sillies"
         *m=rm;
		 *c=(double)(meany-rm*meanx); /* y=mx+c so c=y-mx */
		 rt=(double)(meanxy-meanx*meany);
		 rb=(double)(meany2-meany*meany);
         // crprintf("rm=%g rt=%g rb=%g rt/rb=%g \n",rm,rt,rb,rt/rb);
         if(rb!=0)     /* trap divide by zero */
            *r2= rm * (rt/rb) ;
          else
            *r2=1.0;/* should be in range 0-1 */
         }
#ifdef SDEBUG 
 crprintf("lin_reg() m=%g c=%g r2=%g\n",*m,*c,*r2);
#endif         
}
static void lambda0(s_spline_float *x, s_spline_float *y, s_spline_float *yo, size_t n) // deal with lambda=0 -> result is best least squares straight line
 	{double m,c,r2;
 	 lin_reg(x,y,0,n-1,&m,&c,&r2); // fit straight line
 	 if(yo==NULL)
 		{
	 	 for(size_t j=0;j<n;++j)
			y[j]=(s_spline_float)(m*x[j]+c); // save output y values, overwriting original y value
 		}	
 	  else
 		{
	 	 for(size_t j=0;j<n;++j)
			yo[j]=(s_spline_float)(m*x[j]+c); // save output into yo
		}
 	 return;
 	}
 	
void SmoothingSpline( s_spline_float *x, s_spline_float *y, s_spline_float *yo, size_t n, double lambda) // x,y are input data, x,y0 are positions of optimal knots - all arrays of size "n" (0..n-1) . lambda is smoothing 0 (max smoothing)=>1 no smoothing
																					 // if yo is NULL put updated y values back into array y
{
 double avh, p, q;
 bool OK=true;
 if(x==NULL || y==NULL ) return;// both x and y must be provided (yo can be null in which case the output is put back into y)
 for (size_t i = 1; i < n; i++)
    {
      if (x[i] <= x[i-1])
		{ OK=false; // not monotonically increasing
		  crprintf("SmoothingSpline(): x values not monotonic x[%zu]=%.9g <= x[%zu]=%.9g\n",i,(double)(x[i]),i-1,(double)(x[i-1]));
          break;
        }
    }
 if(!OK || lambda>=1.0 || n<3)
	{// lambda=1 gves no smoothing so nothing to do, >1 is not defined, so assume user wants nothing done.
	 // n<3 also means nothing to do
 	 // might still have to copy y to yo 
 	 // also do this if x values are not monotonically increasing
#ifdef SDEBUG
	 if(!OK) 
		{//red_text();
		 crprintf("SmoothingSpline() line %d: x values not monotonic\n",__LINE__);
		 //normal_text();
		}
#endif 	 
 	 if(yo!=NULL)
 		{
	 	 for(size_t j=0;j<n;++j)
	 	 	yo[j]=y[j]; // save output into yo (same as input in this case
		}
	 return; 
 	}
  // all error checking of inputs is now complete ... We also know lambda <1
#if 1
 // non-linear scaling of lambda - which factors in n - hopefully avoids the need for very small values of lambda with large n
 // 1=>1 , but smaller values of lambda get reduced by a factor n
 if(lambda<=0)
 	lambda=0;
 else
 	lambda=lambda/(1.0+(1.0-lambda)*n);
#endif
 if(lambda<=DBL_EPSILON) /* <= 0 is OK when x,y are doubles, at least for the test program - but this is needed when the are floats */
 	{// 0 means max smoothing requested => straight line fit. <0 is undefined so treat as if 0
 	 lambda0(x, y, yo,n);// do fit and store in y or yo as required
 	 return;
 	}
 // if we get here we will have to do a spline fit - so do dynamic memory allocations
 double *_yo=(double *)calloc(n,sizeof(double)); // output values
 size_t ic=n+1;// +1 just in case!
 double *c=(double *)calloc((ic)*3,sizeof(double)); // array for calculated spline coeffs
 double *wk=(double *)calloc(7*(n + 2),sizeof(double)); // array for working values
 if( _yo!=NULL && c!=NULL && wk!=NULL)
 	{// all the memory allocations suceeded, we can carry on
	 /* Initialize */
     spint1(x, &avh, y, n, _yo, c, ic, wk, &WK(0, 3));
 	 if(lambda>1.0) lambda=1.0;// ensure 0<=lambda<=1
 	 if(lambda<0) lambda=0;
	 q=lambda;
	 // ensure p+q == 1
	 if(q<DBL_EPSILON)
	 	{q=0;
	 	 p=1;
	 	}
	 else
	 	p=1.0-q;
      /* Calculate spline coefficients */
     spfit1(x, avh, n,  p, q, _yo, c, ic, wk, &WK(0, 3), &WK(0, 5), &WK(0, 6));
     spcof1(x, avh, y, n, p, q, _yo, c, ic, &WK(0, 5), &WK(0, 6));
 	 
	 if(yo==NULL)
	 	{for(size_t i=0;i<n;++i)
			y[i]=(s_spline_float)_yo[i]; // output to y
	 	}
	 else
	  	{for(size_t i=0;i<n;++i)
			yo[i]=(s_spline_float)_yo[i]; // output to yo
	 	}
	 	
	}
  else
  	{// memory allocation(s) failed, if necessary just copy y to yo (unchanged) 	
  	 crprintf("SmoothingSpline: not enough memory\n"); 
	 if(yo!=NULL)
	  	{for(size_t i=0;i<n;++i)
	 		yo[i]=y[i]; // if output to yo  required
	 	}
	}  
 // free up memory - but only if allocations were sucessfull originally	
 if(_yo!=NULL) free(_yo);
 if(c!=NULL) free(c);
 if(wk!=NULL) free(wk);
}		

