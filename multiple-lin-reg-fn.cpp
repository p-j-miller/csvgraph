//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
/* =================================================================
   multiple-lin-reg.cpp

  multiple variable linear regression
  Based on an idea in "Programming Classic, Implementing the World's Best Algorithms" chapeter 12.4
  by  Ian Oliver.
   This code in C written by Peter Miller.
   Warning - all arrays are indexed from 1 (not from 0) !!!!

  This version uses long doubles for maximum accuracy.

*/
/*----------------------------------------------------------------------------
 * Copyright (c) 2014,2022 Peter Miller
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
#define NoForm1
#include "expr-code.h" /* for rprintf */
#include <math.h>     /* for fabs etc */
#include <values.h> /* MAXFLOAT etc */
#include <float.h>  /* FLT_EPSILON, DBL_EPSILON etc */
#include <stdlib.h> /* for max() */
#include "UScientificGraph.h"
#include "UScalesWindow.h"
#include "UDataPlotWindow.h"
#include "matrix.h" /* must come before include of multiple-lin-reg.h */
#include "multiple-lin-reg-fn.h"

// internal functions
static void Symmetric_Pivot (matrix_ld X, int N , bool Used[],int Piv );
static void Dispersion_Matrix (float *x_arr,float *y_arr,enum reg_types r, int N ,int SampleSize, matrix_ld S, long double Mean[],void (*filter_callback)(unsigned int i, unsigned int imax));
static void Regression_Stepwise (matrix_ld S,int  N, bool Used[] , long double Fraction);


static void Symmetric_Pivot (matrix_ld X, int N , bool Used[],int Piv )
 // add or remove (if previously added) variable in.
 // uses fully pivoting for best accuracy.
{
 long double *Rpiv= (long double *)calloc(N+1,sizeof(long double)); // Rpiv[N+1]
 long double *Cpiv= (long double *)calloc(N+1,sizeof(long double)); // Cpiv[N+1]

 if (Piv == 0)
    {// initialise array showing none used so far
     for (int i = 1;i<=N;++i)
		Used[i]=false;
    }
 else if (Piv > 0 && Piv <= N)
    {
     Used[Piv] = ! Used[Piv]; // setup current pivot
     // rprintf("Symetric pivot() Piv=%d, X[Piv][Piv]=%g\n",Piv,X[Piv][Piv]);
     Cpiv[Piv] = 1.0 / X[Piv][Piv];   // WARNING might overflow if pivot is zero !!
     Rpiv[Piv] = 1;
     X[Piv][Piv] = 0;
     for (int j= 1; j<= (Piv - 1);++j)
		{// transform the pivot column before the pivot
		 Rpiv[j] = X[Piv][j];
		 if(Used[j] != Used[Piv])
			{
			 Cpiv[j] = -X[Piv][j] * Cpiv[Piv];
			}
		  else
			{ Cpiv[j] = X[Piv][j] * Cpiv[Piv];
			}
		 X[Piv][j]=0;
		}
    for(int i = (Piv + 1 );i<=N;++i)
		{// transform the pivot row after the pivot
		 if(Used[i] == Used [Piv])
			{
			 Rpiv[i] = -X[i][Piv];
			}
          else
			{ Rpiv[i] = X[i][Piv];
			}
		 Cpiv[i] = - X [ i ][Piv] * Cpiv[Piv];
		 X[i][Piv] = 0;
		}
#if 1
    // optimised version
    for (int i = 1;i<=N;++i)	// transform matrix elements
		{long double *Xi=X[i];   // move out of inner loop
		 long double Ci=Cpiv[i];
		 for(int j=1;j<= i;++j)
			{
			 Xi[j] += (Ci * Rpiv[j]);
			}
		}
#else
    // simpler (not optimised version) of above
    for (int i = 1;i<=N;++i)	// transform matrix elements
		{
		 for(int j=1;j<= i;++j)
			{
			 X[i][j] += (Cpiv[i] * Rpiv[j]);
			}
		}
#endif
   }
 free(Rpiv); // free space for arrays that was dynamically allocated at the start
 free(Cpiv);
}



static void Dispersion_Matrix (float *x_arr,float *y_arr,enum reg_types r, int N ,int SampleSize, matrix_ld S, long double Mean[],void (*filter_callback)(unsigned int i, unsigned int imax))
// create matrix defining equations to be solved
// uses recursive formulations for calculations to minimise errors and reduce risk of overflow.
{long double Deviate;
 long double *Z=(long double *)calloc(N+1,sizeof(long double));
 if(Z==NULL) return; // no RAM
 for(int i = 1; i<=N;++i) // Initialize arrays
    {
	 Mean[i] = 0;
	 Z[i]=0;
	 for(int j = 1 ;j<=i;++j)
		{
		 S[i][j]=0;
		}
	}
 for(int k = 0 ;k< SampleSize;++k)
	{int k1=k+1;
	 if(filter_callback!=NULL && (k & 0x3ffff)==0 )
		(*filter_callback)(k,SampleSize+1); // update on progress
	 // calculate Z[i]'s for required function [ these are hardcoded for speed ]
	 if(r==reg_poly)
		{long double x=x_arr[k];
		 long double r=x;
		 for(int i = 1; i<= N;++i)
			{
			 // if i=1 returns y
			 // if i=2 returns x
			 // if i=3 returns x^2 etc
			 if(i==1) Z[1]= y_arr[k];
			 else
				{Z[i]=r;
				 r*=x;
				}
			}
		}
	 else if(r==reg_sqrt)
		{long double x=sqrt((long double)x_arr[k]);
		 long double r=x;
		 for(int i = 1; i<= N;++i)
			{
			 // if i=1 returns y
			 // if i=2 returns sqrt(x)
			 // if i=3 returns x etc
			 if(i==1) Z[1]= y_arr[k];
			 else
				{Z[i]=r;
				 r*=x;
				}
			}
		}
	 else // r==reg_rat
		{long double x=x_arr[k];
		 long double r=x;
		 long double y=y_arr[k];
		 int i;
		 for(i = 1; i<=(N+1)/2;++i)
			{
			 // if i=1 returns y
			 // if i=2 returns x
			 // if i=3 returns x^2 etc
			 if(i==1) Z[1]= y;
			 else
				{Z[i]=r;
				 r*=x;
				}
			}
		 r=-y*x; // "bottom line"
		 for(; i<=N;++i)
			{
			 // -y*x, -y*x^2, etc
			 Z[i]=r;
			 r*=x;
			}
		}
	 for(int i = 1; i<= N;++i) // Accumulate sums of squares and products
		{
		 for(int j=1;j<=i;++j)
			{
			 Deviate = (Z[i] - Mean[i]) * (Z[j] - Mean[j]);
			 S[i][j] += Deviate - (Deviate / k1);
			}
		}

	 for(int i = 1; i<= N;++i) //  Accumulate means
		{
		 Deviate = Z[i] - Mean[i];
		 Mean[i] += (Deviate / k1);
		}
	}
 free(Z);
}


static void Regression_Stepwise (matrix_ld S,int  N, bool Used[] , long double Fraction)
// Fraction is relative accuracy required (terms that don't impact by result this much are not added)
// if fraction >= 0 terms that produce less change than last term added are also dropped
// if fraction <0 then all terms are added that reduce error, once added terms are not dropped
// note fraction <0 does NOT produce the lowest error, normally fraction=0 will give the lowest error
{long double Change, Limit, MaxChange, MinChange;
 int Insert, Remove;
 Limit = Fraction * S [ 1][ 1];
 do {
	 MaxChange = 0;
	 Insert = -1;
	 for(int i = 2;i<= N;++i) // Find variable that produces largest change in S [ 1 , 1]
		{
		 if(Used[i] == false && S[i][ i] > 0 )
			{
			 Change = (S[i][ 1] * S[i][ 1]) / S[i][ i];
			 if(Change > MaxChange)
				{
				 MaxChange = Change;
				 Insert = i;
				}
			}
		}
	 Symmetric_Pivot (S, N , Used, Insert); // Insert variable into equation
	 if(Fraction>=0.0 && Insert != -1 )
		do {    // only allow variable removal if fraction >= 0 and a variable added above
			MinChange = MaxChange; // find any variables that would make less difference than variable just added and remove them
			Remove = -1;
			for(int i = 2 ; i<= N; ++i)
				{//  Find a variable that produces change in S [ 1 , 1] less than variable added above
				 if(Used[i] == true ) // if used[i]=true, s[i][i] must be > 0 as thats a condition of it being inserted above
					{
					 Change = (S[i][ 1] * S[i][ 1]) / S[i][ i];
					 if(Change < MinChange && i != Insert)
						{
						 MinChange = Change;
						 Remove = i;
						}
					}
				}
			Symmetric_Pivot (S, N , Used, Remove); // Remove variable from equation
		   } while ( Remove != -1 );
	} while (S [ 1 ][ 1] > Limit &&  Insert != -1 );
} // end Regression-Stepwise



void Regression_Predict (matrix_ld S,int N , long double Mean[], bool Used[],long double X[])
// calculate value at point defined by X, returns calculated value in X[1]
{long double x; // should give a little more accurate results
 x = Mean[1];
 for(int j = 2;j<= N;++j)
	{
	 if(Used[j] == true)
		{
		 x += (S[j][ 1] * (X[j] - Mean[j]));
		}
	}
 X[1]=x;
} // end Regression-Predict

void multi_regression(float *x_arr,float *y_arr,enum reg_types r, int N ,int SampleSize, matrix_ld S, long double Mean[], bool Used[],long double Fraction,void (*filter_callback)(unsigned int i, unsigned int imax))
{// do full regression
 // float *x_arr,float *y_arr,double (*fn)(float x,float y,int c) - input: x values, y values and a function to calculate other params
 //    if c=1 fn should return y, for polynomial if c=2 return x, c=3 return x^2 etc.
 // N is number of variables to be fitted
 // SampleSize is size of x_arr &y_arr (both are indexed from 0 to SampleSize-1 )
 // S is long double[N+1][N+1]          - output
 // Mean is long double[N+1]            - output
 // used is bool[N+1]              - output
 // Fraction is 0..1 with 0 giving the most accurate fit (used to drop variables that only make a small change to accuracy of fit) - input
 Dispersion_Matrix (x_arr,y_arr,r, N , SampleSize, S, Mean,filter_callback); // setup matrices readuy to be solved
 Symmetric_Pivot(S, N , Used, 0); // needed 1st to setup arrays
 Regression_Stepwise (S, N, Used , Fraction); // solve
}

long double test_fn(float x,float y, int c, int N)
{// calculate  value of c'th component at x,y
 // if c=1 returns y
 // if c=2 returns x
 // if c=3 returns x^2 etc
 long double r;
 if(c==1) return y;
 for (r=x;c>2;c--) r*=x; // for polynomials, if c=2 returns x, if c=3 returns x^2 etc
 return r;
}
// TEST PROGRAM
void test_multiregression(int mode)
{ // fits polynomial to exp(-x) [this is not the best way to fit a polynomial!]
  // if mode = 0 then calls individual routines, with an additional call to  Symmetric_Pivot(S, N , U, N) to check a variable can be deleted. Fraction=0
  // if mode = 1 then calls multi_regression(...) as a user normally would. Fraction =0
  // if mode == 2 then calls multi_regression with fraction = -1 (so all terms used that reduce error)
  // if mode == 3 then calls multi_regression with fraction = 0.001
 const int M = 30, N = 20;  // N is nos variables in fitting function, M is nunber of points to fits
 matrix_ld S;// 2D matrix
 long double A[N+1];   // A = average (mean)
 long double X[N+1];
 float x_arr[M],y_arr[M] ; // X and Y values
 bool U[N+1];
 long double T,C;
 S=cr_matrix_ld(N+1,N+1);// S[N+1][N+1]

 for(int i = 0 ;i< M;++i) // initialise data vectors to x & y
	{
	 x_arr[i] = i * 10.0 /M;   // x
	 y_arr[i]=exp(-x_arr[i]);  // y
	}
 if(mode==0)  // test removing variables works
   {rprintf("multi_regression, mode==0: testing removing variables works, fraction=0\n");
	Dispersion_Matrix (x_arr,y_arr,reg_poly, N , M, S, A,NULL); // fit equation to data points
	Symmetric_Pivot(S, N , U, 0); // needed 1st to setup arrays
	Symmetric_Pivot(S, N , U, N);
	Regression_Stepwise (S, N, U , 0.0);
   }
 else if(mode==1) // normal use, fraction = 0
   {rprintf("multi_regression, mode==1: fraction=0\n");
	// multi_regression(matrix_d Data, int N ,int SampleSize, matrix_d S, double Mean[], bool Used[],double Fraction)
	multi_regression(x_arr,y_arr,reg_poly,N,M,S,A,U,0.0,NULL);
   }
 else if(mode==2) // normal use, fraction = -1 so all terms kept
   {rprintf("multi_regression, mode==2: fraction= -1\n");
	// multi_regression(matrix_d Data, int N ,int SampleSize, matrix_d S, double Mean[], bool Used[],double Fraction)
	multi_regression(x_arr,y_arr,reg_poly,N,M,S,A,U,-1.0,NULL);
   }
 else if(mode==3) // normal use, fraction = 0.001
   {rprintf("multi_regression, mode==3: fraction=0.001\n");
	// multi_regression(matrix_d Data, int N ,int SampleSize, matrix_d S, double Mean[], bool Used[],double Fraction)
	multi_regression(x_arr,y_arr,reg_poly,N,M,S,A,U,0.001,NULL);
   }
 C=A[1];
 for(int j = 2;j<= N;++j)
	{
	 if(U[j] == true)
		{
		 rprintf("Var[%d] used, multiplier(value-mean)=%g mean=%g\n",j,(double)S[j][1],(double)A[j]);
		 C-= S[j][1]*A[j];
		}
	}
 rprintf("Constant C=%g\n\n",(double)C);
 // print equation
 rprintf("Y=%g",(double)C);
 for(int j = 2;j<= N;++j)
	{T=0;// if not used coeff is 0
	 if(U[j] == true) T=S[j][1];
	 if(j==2)
		rprintf("%+g*X",(double)T);
	 else
		rprintf("%+g*X^%d",(double)T,j-1);
	}
 rprintf("\n");
 T = 0.0; // calculate residual sum of squares to see how good fit was
 rprintf("Point: Exact     Approx\n");
 for(int i=0; i<M;++i)
	{
	 for(int j = 2; j<=N;++j)
		{X[j]=test_fn(x_arr[i],y_arr[i],j,N);
		}
	 Regression_Predict (S, N, A, U , X);
	 T = T +((X[1] - y_arr[i]) * (X[1] - y_arr[i]));
	 rprintf("%4d: %8g  %8g\n",i,y_arr[i],(double)X[1]);
	}
 rprintf("residual sum of squares = %g (should be near 0)\n\n",(double)T);
 fr_matrix_ld(S);
}

