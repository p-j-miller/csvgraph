/* header file for getfloat.cpp

*/
/*----------------------------------------------------------------------------
 * Copyright (c) 2012, 2013, 2022 Peter Miller
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
#ifndef _getfloat_h
#define _getfloat_h
bool getdouble(char *s, double *d);/*reads a floating point number (double)  returns true if valid - allows whitespace as well as a number , d=0 on error*/
bool getdouble(wchar_t *s, double *d);

bool getfloat(char *s, float *d); /*reads a floating point number returns true if valid - allows whitespace as well as a number , d=0 on error*/
bool getfloatgt0(char *s, float *d);/* as above but requires number to be >0 */
bool getfloatge0(char *s, float *d);/* as above but requires number to be >0 */
// functions like above that take wchar_t *
bool getfloat(wchar_t *s, float *d);
bool getfloatgt0(wchar_t *s, float *d);
bool getfloatge0(wchar_t *s, float *d);
// duplicates of all the above returning doubles, c++ allows the use of the same function name
bool getfloat(char *s, double *d); /*reads a floating point number returns true if valid - allows whitespace as well as a number , d=0 on error*/
bool getfloatgt0(char *s, double *d);/* as above but requires number to be >0 */
bool getfloatge0(char *s, double *d);/* as above but requires number to be >0 */
// functions like above that take wchar_t *
bool getfloat(wchar_t *s, double *d);
bool getfloatgt0(wchar_t *s, double *d);
bool getfloatge0(wchar_t *s, double *d);

// now repeat for int's
bool getint(char *s, int *d);
bool getintgt0(char *s, int *d);
bool getintge0(char *s, int *d);
bool getint(wchar_t *s, int*d);
bool getintgt0(wchar_t *s, int *d);
bool getintge0(wchar_t *s, int *d);
#endif
