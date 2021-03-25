/* =================================================================
   multiple-lin-reg.h
  Note you must include "matrix.h" before using this code
  multiple variable linear regression
  Based on an idea in "Programming Classic, Implementing the World's Best Algorithms" chapeter 12.4
  by  Ian Oliver.
*/
/*----------------------------------------------------------------------------
 * Copyright (c) 2014 Peter Miller
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

void Regression_Predict (matrix_ld S,int N , long double Mean[], bool Used[],long double X[]);
void multi_regression(float *x_arr,float *y_arr,enum reg_types r, int N ,int SampleSize, matrix_ld S, long double Mean[], bool Used[],long double Fraction,void (*filter_callback)(unsigned int i, unsigned int imax)) ;
 // do full regression
 // float *x_arr,float *y_arr,enum reg_types r - input: x values, y values and a function to calculate other params
 // N is number of variables to be fitted
 // SampleSize is size of x_arr &y_arr (both are indexed from 0 to SampleSize-1 )
 // S is long double[N+1][N+1]          - output
 // Mean is long double[N+1]            - output
 // used is bool[N+1]              - output
 // Fraction is 0..1 with 0 giving the most accurate fit (used to drop variables that only make a small change to accuracy of fit) - input
void test_multiregression(int mode); // test program   , mode is 0,1,2,3
