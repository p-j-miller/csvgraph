/* header file for smoothing_spline.c  */
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

#ifndef _zSMOOTHING_SPLINE_H_
 #define _zSMOOTHING_SPLINE_H_
 #define s_spline_float float /* could be float or double */
//#define s_spline_type_double /* only define for double type - can then be checked with a #ifdef */ 
 #ifdef __cplusplus
extern "C" 
 {
 #endif
void SmoothingSpline( s_spline_float *x, s_spline_float *y, s_spline_float *yo, size_t _n, double lambda); // x,y are input data, x,y0 are positions of optimal knots - all arrays of size "n" (0..n-1) . lambda is smoothing 0 (max smoothing)=>1 no smoothing
																					  // values of lambda may need to be (very) small, but values <= DBL_EPSILON = 2.22 e-16 are treated as zero
																					 // if yo is NULL put updated y values back into array y
 #ifdef __cplusplus
 }
 #endif
#endif
