/* interpolate.h
 header file for intrepolate.c

 Written by Peter Miller 16-11-12
 
 This version 30/10/2023 adds double and float versions

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
#ifndef _interpolate_h
 #define _interpolate_h
 #include <stdbool.h> /* as bool used below */
 #ifdef __cplusplus
  extern "C" {
 #endif 
 /* for floats */
ssize_t binarysearch_f(float *a, size_t size_a,float key); /* assumes array a sorted into increasing order */
float interp1D_f(float *xa, float *ya, size_t size, float x, bool clip); /* interpolate y corresponding to x from xa (increasing) and corresponding ya */
typedef struct {
        float *xa;
        float *ya;
        size_t size;
        } interp1_f;

float interp2D_f(interp1_f *p1, float *za,size_t sizearr_p1,float z,float x, bool clip); /* 2D interpolation p1 is an array of interp1 structures that match elemnts of p1[] */
/* for doubles */
ssize_t binarysearch_d(double *a, size_t size_a,double key); /* assumes array a sorted into increasing order */
double interp1D_d(double *xa, double *ya, size_t size, double x, bool clip); /* interpolate y corresponding to x from xa (increasing) and corresponding ya */

typedef struct {
        double *xa;
        double *ya;
        size_t size;
        } interp1_d;

double interp2D_d(interp1_d *p1, double *za,size_t sizearr_p1,double z,double x, bool clip); /* 2D interpolation p1 is an array of interp1 structures that match elemnts of p1[] */

 #ifdef __cplusplus
    }
 #endif
#endif
