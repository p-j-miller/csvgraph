/* smooth_diff.h
   =============
 header file for smooth_diff.c

 Written by Peter Miller 14-3-2023

*/
/*----------------------------------------------------------------------------
 * Copyright (c) 2023Peter Miller
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
#ifndef _smooth_diff_h
 #define _smooth_diff_h
 #ifdef __cplusplus
  extern "C" {
 #endif 
double Savitzky_Golay_smoothing17(float *y,float *x,size_t  start, size_t  end, size_t  index, unsigned int s_order); /* give smoothed value at index - only using data between start and end */
double Savitzky_Golay_smoothing25(float *y,float *x,size_t  start, size_t  end, size_t  index, unsigned int s_order); /* give smoothed value at index - only using data between start and end */
double Savitzky_Golay_smoothing_const_xinc17(float *y,float *x,size_t  start, size_t  end, size_t  index, unsigned int s_order); /* optimsed version of above for constant xinc and orders 1,2,4 (works for any order 1->10) */
/* My approach to fit calculate a derivative, do a least squares polynomial fit and symbolicaly differentiate polynomial and evaluate it at "index" */
double dy_dx_polyfit17(float *y,float *x,size_t  start, size_t  end, size_t  index, unsigned int diff_order); /* estimate dy/dx at index - only using data between start and end, diff order 1=>10 */
double dy_dx_polyfit25(float *y,float *x,size_t  start, size_t  end, size_t  index, unsigned int diff_order); /* estimate dy/dx at index - only using data between start and end, diff order 1=>10 */
/* derivative based on Savitzky & Golay paper */
double dy_dx17(float *y,float *x,size_t  start, size_t  end, size_t  index, unsigned int diff_order); /* estimate dy/dx at index - only using data between start and end, diff order 1=>10, 1,2,4 are very efficient */
double d2y_d2x17(float *y,float *x,size_t  start, size_t  end, size_t  index, unsigned int diff_order); /* estimate d2y/d2x at index - only using data between start and end, diff order 1=>10 */
double d2y_d2x25(float *y,float *x,size_t  start, size_t  end, size_t  index, unsigned int diff_order); /* estimate d2y/d2x at index - only using data between start and end, diff order 1=>10 */
double d2y_d2x_orig17(float *y,float *x,size_t  start, size_t  end, size_t  index, unsigned int diff_order); /* estimate d2y/d2x at index - only using data between start and end, diff order 1=>10 */
bool Polyreg(float *y_vals,float *x_vals,size_t  iCount, unsigned int order, double *coeffs); /* this should probabbly be in another file... */
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
	
/* evaluate polynomial at specified point */
/* eg for order 4 coeff[0]+coeff[1]*x+coeff[2]*x*x+pcoeff[3]*x*x*x+pcoeff[4]*x*x*x*x */
static inline double horner(double *coeff,double x, unsigned int order)
{long double r=coeff[order];    // double is probably OK, but polynomial fits can be poorly conditioned so do this just in case.
 for(unsigned int i=order;i>0;--i)
	{r=r*x+coeff[i-1];
	}
 return (double)r;
}	
 #ifdef __cplusplus
    }
 #endif
#endif
