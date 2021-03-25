
/* header file for matrix.cpp */
/*----------------------------------------------------------------------------
 * Copyright (c) 2012, 2013 Peter Miller
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
 typedef long double **matrix_ld; /* long double 2D matrix */
 typedef double **matrix_d; /* double 2D matrix */
 typedef float **matrix_f;  /* float 2Dmatrix */

 matrix_ld cr_matrix_ld (size_t numrow, size_t numcol); /* create long double matrix with specified rows*colums by allocating a single space on the heap*/
 void fr_matrix_ld (matrix_ld matrix); /* free space for long double matrix */

 matrix_d cr_matrix_d (size_t numrow, size_t numcol); /* create double matrix with specified rows*colums by allocating a single space on the heap*/
 void fr_matrix_d (matrix_d matrix); /* free space for double matrix */

 matrix_f cr_matrix_f (size_t numrow, size_t numcol); /* create float matrix */
 void fr_matrix_f (matrix_f matrix); /* free space for float matrix */

