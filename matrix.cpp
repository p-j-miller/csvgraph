//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
/* Simple matrix code */
/* support float, double and long double 2Dmatrices.
   access elements using syntax matrix[row][column]
   note array indices are 0..max-1 as normal in C
*/

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
#include <stdlib.h>
#include "matrix.h"

matrix_ld cr_matrix_ld (size_t numrow, size_t numcol)
 /* create long double matrix with specified rows*colums by allocating a single space on the heap*/
 /* at start of space we set pointers to the rows of the matrix, then we have the actual matrix element values */
 { matrix_ld matrix;
   size_t sizecol=numcol*sizeof(long double);
   matrix=(matrix_ld)calloc(numrow,sizeof(long double *)+sizecol);
   if(matrix!=NULL)
        {size_t i;
		 matrix[0]=(long double *)&(matrix[numrow]); /* 1st starts after the pointers */
         for(i=1;i<numrow;++i)
				{matrix[i]=(long double *) &(matrix[i-1][numcol]);
                }
        }
   return matrix;
 }

void fr_matrix_ld (matrix_ld matrix) /* free space for long double matrix */
 { if(matrix!=NULL) free(matrix);
 }


matrix_d cr_matrix_d (size_t numrow, size_t numcol)
 /* create double matrix with specified rows*colums by allocating a single space on the heap*/
 /* at start of space we set pointers to the rows of the matrix, then we have the actual matrix element values */
 { matrix_d matrix;
   size_t sizecol=numcol*sizeof(double);
   matrix=(matrix_d)calloc(numrow,sizeof(double *)+sizecol);
   if(matrix!=NULL)
        {size_t i;
         matrix[0]=(double *)&(matrix[numrow]); /* 1st starts after the pointers */
         for(i=1;i<numrow;++i)
                {matrix[i]=(double *) &(matrix[i-1][numcol]);
                }
        }
   return matrix;
 }

void fr_matrix_d (matrix_d matrix) /* free space for double matrix */
 { if(matrix!=NULL) free(matrix);
 }

matrix_f cr_matrix_f (size_t numrow, size_t numcol)
 /* create float matrix with specified rows*colums by allocating a single space on the heap*/
 /* at start of space we set pointers to the rows of the matrix, then we have the actual matrix element values */
 { matrix_f matrix;
   size_t sizecol=numcol*sizeof(float);
   matrix=(matrix_f)calloc(numrow,sizeof(float *)+sizecol);
   if(matrix!=NULL)
        {size_t i;
         matrix[0]=(float *)&(matrix[numrow]); /* 1st starts after the pointers */
         for(i=1;i<numrow;++i)
                {matrix[i]=(float *) &(matrix[i-1][numcol]);
                }
        }
   return matrix;
 }

void fr_matrix_f (matrix_f matrix) /* free space for float matrix */
 { if(matrix!=NULL) free(matrix);
 }

