/* ya-sort2.c
   =========
  yasort2() sorts an array of numbers and moves the contents of another array of the same size around so the "pairs" are kept together.
  Eg if one array has x co-ords and the 2nd has y co-ords you can sort the x co-ords while keeping the x-y pairs together at the same array index.
   
  It uses a quicksort with the addition of introsort functionality to give both a fast average execution time and o(n*log(n)) worse case execution time.
  See "Introspective sorting and selection algorithms" by D.R.Musser,Software practice and experience, 8:983-993, 1997.
  ya2sort can only sort numbers.
  
  yasort2() takes ~ 71% longer than yasort() which is not surprising as it has to move twice the amount of data around.
  
 It uses yasort2.h to define the type of data that will be sorted ( elem_type_sort2 ).

 If PAR_SORT is defined (see below) then this code uses tasks to use all available processors to speed up sorting
 On a simple test enabling this with a 2 processor system sped up sorting by 1.5*
*/ 
/* 
Copyright (c) 2021,2022 Peter Miller
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
#ifndef __BORLANDC__
 #define NDEBUG /* if defined then asserts become "nothing" */
#endif

// #define inline /* builder C++ v5 does not suport inline */

// #define YA2SORT_TEST_PROGRAM /* if defined compile a test program for this code - make sure that YASORT_MEDIAN_TEST_PROGRAM is NOT defined in ya-sort.c ! */
							 /* For the test program, you also need to ensure the definitions of elem_type_median (in yamedian.h) and elem_type_sort2 (in yasort2.h) are the same (normally double is used for testing) */
#define PAR_SORT /* if defined create a parallel sort using threads  */
// #define DEBUG /* if defined then add a few printf's so you can see whats happening, can be useful if INTROSORT_MULT needs to be tuned, but oherwise not needed */
// you should not need to edit anything below here for normal use of this software (you may need to edit yasort2.h)
// #define USE_PTHREADS /* if defined use pthreads for multitasking - otherwise if running under windows use native windows threads. Pthreads is slightly slower than native Windows threads on Windows */

#include <time.h>
#include <stdint.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>
#if defined(DEBUG) || defined (YA2SORT_TEST_PROGRAM )
 #include <stdio.h>
 #include <time.h>
 #include "hr_timer.h"
#endif
#include <assert.h>
#ifdef PAR_SORT
 #if defined _WIN32 
  #include <process.h> /* for _beginthreadex */
  #include <windows.h> /* for number of processors */
 #elif defined __GNUC__
  #include <unistd.h> /* to get number of processors on OS's other than Windows */
 #else
  #error "Parallel sorting not supported for this complier/OS (undefine PAR_SORT to avoid this error)"
 #endif 
 #ifdef USE_PTHREADS
  #include <pthread.h>
 #endif
#endif

#include "yasort2.h"  /* defines elem_type_sort2 etc */

#define INTROSORT_MULT 2 /* defines when we swap to heapsort 3 means only do so very rarely, 0 means "always" use heapsort. All positive integer values (including 0) will give o(n*log(n)) worse case execution time
							on the 1st step of the test program yasort2(100000001) sorting doubles the following execution speeds were obtained
							with DUAL_PARTITION on yasort2 we get
							INTROSORT_MULT  Time(secs)
							0                       - heapsort already used (slow).
							1				112.687 - heapsort never used, mid-range used more than at 2
							2				112.907 - heapsort never used, mid-range used for some examples
							3				112.925 - heapsort never used, mid range used fo 1 example
						 */

#ifdef YA2SORT_TEST_PROGRAM   
#ifdef DEBUG
 #ifdef __BORLANDC__
  #define dprintf(...) crprintf(__VA_ARGS__) /* convert to crprintf when DEBUG defined */
 #else
  #define dprintf(...) printf(__VA_ARGS__) /* convert to printf when DEBUG defined */
 #endif
#else
#define dprintf(...) /* nothing - only prints when DEBUG defined */
#endif
#endif


#if !defined(PAR_SORT) || defined(YA2SORT_TEST_PROGRAM )
 #define P_UNUSED(x) (void)x /* a way to avoid warning unused parameter messages from the compiler */
#endif

#if defined(YA2SORT_TEST_PROGRAM) || !defined(NDEBUG)
 static bool check_sort( elem_type_sort2 *a, size_t n); // check result of sort is ordered correctly
#endif
static void heapsort2(elem_type_sort2 *a,elem_type_sort2 *b,size_t n); // backup sort automatically used when required.


#if defined __GNUC__

static inline int ilog2(size_t x) { return 63 - __builtin_clzll(x); }

#elif defined _WIN32  && !defined  __BORLANDC__
#include <intrin.h>
static inline int ilog2(size_t x)
{
    unsigned long i = 0;
     _BitScanReverse(&i, x);
    return i;
}

#elif defined _WIN64  && !defined  __BORLANDC__
#include <intrin.h>
static inline int ilog2(size_t x)
{
    unsigned long i = 0;
    _BitScanReverse64(&i, x);
    return i;
}
#else // version in standard C , this is slower than above optimised versions but portable
/* algorithm from https://stackoverflow.com/questions/11376288/fast-computing-of-log2-for-64-bit-integers */
#if (defined __BORLANDC__ && defined _WIN64) || ( defined __SIZEOF_POINTER__ &&  __SIZEOF_POINTER__ == 8)
static inline int  ilog2(size_t x) // unsigned 64 bit version 
{
  #define S(k) if (x >= (UINT64_C(1) << k)) { i += k; x >>= k; }
  int i = 0;
  S(32);
  S(16);
  S(8);
  S(4);
  S(2);
  S(1);
  return i;
  #undef S
}
#elif (defined __BORLANDC__ && defined __WIN32__) || ( defined __SIZEOF_POINTER__ && __SIZEOF_POINTER__ == 4 )
static inline int  ilog2(size_t x) // unsigned 32 bit version 
{
  #define S(k) if (x >= (UINT32_C(1) << k)) { i += k; x >>= k; }
  int i = 0;
  S(16);
  S(8);
  S(4);
  S(2);
  S(1);
  return i;
  #undef S
}
#else
#error "unknown pointer size - expected 4 (32 bit) or 8 (64 bit) "
#endif

#endif


#define cswap(i,j) {elem_type_sort2 _t;_t=(i);i=(j);j=_t;} // example call cswap(r[i], r[minIndex]); WARNING - side effects could be an issue here ! eg cswap(r[i++],r[b])
/* note that an intelligent swap (that only writes back if the value is different) is slightly slower in the test program */
#ifdef YA2SORT_TEST_PROGRAM
inline static void eswap1(size_t i,size_t j,elem_type_sort2 *a) /* swap a[i]  and a[j]  */
	{
	 cswap(a[i],a[j]); // use swap macro
	}
#endif

inline static void eswap2(size_t i,size_t j,elem_type_sort2 *a,elem_type_sort2 *b) /* swap a[i]  and a[j] also swap b[i] and b[j] */
	{  
	 cswap(a[i],a[j]); // use swap macro on array a
	 cswap(b[i],b[j]); // use swap macro on array b
	} 

inline static void eswap2p(elem_type_sort2 *pi,elem_type_sort2 *pj,elem_type_sort2 *a,elem_type_sort2 *b) /* swap pi  and pj also swap b[i] and b[j] */
	{  
	 cswap(*pi,*pj); // use swap macro on array a
	 cswap(*(pi-a+b),*(pj-a+b)); // use swap macro on array b
	} 

#define elem_type_ss elem_type_sort2 /* set type for smallsort correctly */	
#define _yaSORT2 /* tell smallsort we have 2 arrays */
#include "ya-smallsort.h" // contains small_sort() - this needs to be included after cswap is defined
	
#ifdef YA2SORT_TEST_PROGRAM
/* replacement for library rand() function that has known (good) properties
   The default generator in tdm-gcc 10.3.0 has RAND_MAX for 32767 which is not ideal.

Public domain code for JKISS RNG   from   http://www0.cs.ucl.ac.uk/staff/d.jones/GoodPracticeRNG.pdf
claims any of the 2 generators combined still pass all the Dieharder tests (and obviously all 3 combined do).
It also passes the complete BigCrunch test set in testU01.
Its period is ~ 2^127 (1.7e38).
*/
static uint32_t x=123456789,y=987654321,z=43219876,c=6543217;/* Seed variables */
static uint32_t JKISS(void)
{
 uint64_t t;
 x=314527869*x+1234567;
 y^=y<<5;
 y^=y>>7;
 y^=y<<22;
 t =4294584393UL*z+c;
 c= t>>32;
 z= t;
 return x+y+z;
}

static int rand(void)
{return JKISS() & INT_MAX; // convert unsigned to signed
}

static void srand(unsigned int seed)
{ // seed random number generator, 0 gives the same sequence as you get without calling srand() 
 x=123456789+seed;
 y=987654321+seed;
 z=43219876+seed;
 c=6543217+seed;
}

#ifdef RAND_MAX
 #undef RAND_MAX
#endif
#define RAND_MAX INT_MAX /* in case its used */
#ifdef RAND_MAX
 #undef RAND_MAX
#endif
#define RAND_MAX INT_MAX /* in case its used */

static int bi_modal_rand(void) /* returns a random number with a "hole" in the distribution so the mean & (max-min)/2 are significantly different to the median  */
{int r=rand();
 while(r>RAND_MAX/4 && r<RAND_MAX/2) r=rand(); // skip values that fall in the "hole" from RAND_MAX/4 to RAND_MAX/2. "hole" is off centre to make distribution non-symetrical. 
 return r;
}
#endif

#ifndef min /* with gcc this is already defined */
 #define min(a, b)	(a) < (b) ? (a) : (b) /* warning a,b evaluated more than once ! */
#endif
 
#if defined(YA2SORT_TEST_PROGRAM) || !defined(NDEBUG)
static  bool check_sort( elem_type_sort2 *a, size_t n) // check result of sort is ordered correctly
{// returns true if sort is ok
 /* check array actually is sorted correctly in increasing order */
 size_t errs=0; 
 if(n<2) return true;
 {size_t i;
  for(i=0;i<n-1;++i)
	 if(a[i+1]<a[i]) 
		errs++;
  }
 return (errs==0);
}
#endif

/*
* From:
* Fast median search: an ANSI C implementation
* Nicolas Devillard - ndevilla AT free DOT fr
* July 1998
*
* The following routines have been built from knowledge gathered
* around the Web. I am not aware of any copyright problem with
* them, so use it as you want.
* N. Devillard - 1998
* The functions below have been edited by Peter Miller 9/2021 to fit in with the rest of the code here.
*/
#define Zv(a,b) { if ((a)>(b)) cswap((a),(b)); }
 

/*----------------------------------------------------------------------------
Function : opt_med9()
In : pointer to an array of 9 elem_type_sort2 values
Out : an elem_type_sort2
Job : optimized search of the median of 9 elem_type_sort2 values
Notice : in theory, cannot go faster without assumptions on the signal.
Formula from:
XILINX XCELL magazine, vol. 23 by John L. Smith
The input array is modified in the process
The result array is guaranteed to contain the median
value in middle position, but other elements are NOT fully sorted sorted.
---------------------------------------------------------------------------*/
static elem_type_sort2 opt_med9(elem_type_sort2 * p)
{
Zv(p[1], p[2]) ; Zv(p[4], p[5]) ; Zv(p[7], p[8]) ;
Zv(p[0], p[1]) ; Zv(p[3], p[4]) ; Zv(p[6], p[7]) ;
Zv(p[1], p[2]) ; Zv(p[4], p[5]) ; Zv(p[7], p[8]) ;
Zv(p[0], p[3]) ; Zv(p[5], p[8]) ; Zv(p[4], p[7]) ;
Zv(p[3], p[6]) ; Zv(p[1], p[4]) ; Zv(p[2], p[5]) ;
Zv(p[4], p[7]) ; Zv(p[4], p[2]) ; Zv(p[6], p[4]) ;
Zv(p[4], p[2]) ; return(p[4]) ;
}

/*----------------------------------------------------------------------------
Function : opt_med25()
In : pointer to an array of 25 elem_type_sort2 values
Out : an elem_type_sort2
Job : optimized search of the median of 25 elem_type_sort2 values
The input array is modified in the process
The result array is guaranteed to contain the median
value in middle position, but other elements are NOT fully sorted sorted.
Notice : in theory, cannot go faster without assumptions on the signal.
Code taken from Graphic Gems.
---------------------------------------------------------------------------*/
static elem_type_sort2 opt_med25(elem_type_sort2 * p)
{
Zv(p[0], p[1]) ; Zv(p[3], p[4]) ; Zv(p[2], p[4]) ;
Zv(p[2], p[3]) ; Zv(p[6], p[7]) ; Zv(p[5], p[7]) ;
Zv(p[5], p[6]) ; Zv(p[9], p[10]) ; Zv(p[8], p[10]) ;
Zv(p[8], p[9]) ; Zv(p[12], p[13]) ; Zv(p[11], p[13]) ;
Zv(p[11], p[12]) ; Zv(p[15], p[16]) ; Zv(p[14], p[16]) ;
Zv(p[14], p[15]) ; Zv(p[18], p[19]) ; Zv(p[17], p[19]) ;
Zv(p[17], p[18]) ; Zv(p[21], p[22]) ; Zv(p[20], p[22]) ;
Zv(p[20], p[21]) ; Zv(p[23], p[24]) ; Zv(p[2], p[5]) ;
Zv(p[3], p[6]) ; Zv(p[0], p[6]) ; Zv(p[0], p[3]) ;
Zv(p[4], p[7]) ; Zv(p[1], p[7]) ; Zv(p[1], p[4]) ;
Zv(p[11], p[14]) ; Zv(p[8], p[14]) ; Zv(p[8], p[11]) ;
Zv(p[12], p[15]) ; Zv(p[9], p[15]) ; Zv(p[9], p[12]) ;
Zv(p[13], p[16]) ; Zv(p[10], p[16]) ; Zv(p[10], p[13]) ;
Zv(p[20], p[23]) ; Zv(p[17], p[23]) ; Zv(p[17], p[20]) ;
Zv(p[21], p[24]) ; Zv(p[18], p[24]) ; Zv(p[18], p[21]) ;
Zv(p[19], p[22]) ; Zv(p[8], p[17]) ; Zv(p[9], p[18]) ;
Zv(p[0], p[18]) ; Zv(p[0], p[9]) ; Zv(p[10], p[19]) ;
Zv(p[1], p[19]) ; Zv(p[1], p[10]) ; Zv(p[11], p[20]) ;
Zv(p[2], p[20]) ; Zv(p[2], p[11]) ; Zv(p[12], p[21]) ;
Zv(p[3], p[21]) ; Zv(p[3], p[12]) ; Zv(p[13], p[22]) ;
Zv(p[4], p[22]) ; Zv(p[4], p[13]) ; Zv(p[14], p[23]) ;
Zv(p[5], p[23]) ; Zv(p[5], p[14]) ; Zv(p[15], p[24]) ;
Zv(p[6], p[24]) ; Zv(p[6], p[15]) ; Zv(p[7], p[16]) ;
Zv(p[7], p[19]) ; Zv(p[13], p[21]) ; Zv(p[15], p[23]) ;
Zv(p[7], p[13]) ; Zv(p[7], p[15]) ; Zv(p[1], p[9]) ;
Zv(p[3], p[11]) ; Zv(p[5], p[17]) ; Zv(p[11], p[17]) ;
Zv(p[9], p[17]) ; Zv(p[4], p[10]) ; Zv(p[6], p[12]) ;
Zv(p[7], p[14]) ; Zv(p[4], p[6]) ; Zv(p[4], p[7]) ;
Zv(p[12], p[14]) ; Zv(p[10], p[14]) ; Zv(p[6], p[7]) ;
Zv(p[10], p[12]) ; Zv(p[6], p[10]) ; Zv(p[6], p[17]) ;
Zv(p[12], p[17]) ; Zv(p[7], p[17]) ; Zv(p[7], p[10]) ;
Zv(p[12], p[18]) ; Zv(p[7], p[12]) ; Zv(p[10], p[18]) ;
Zv(p[12], p[20]) ; Zv(p[10], p[20]) ; Zv(p[10], p[12]) ;
return (p[12]);
}
/* end of N. Devillard functions */

/* 
   heapsort() is used within yasort2 as part of its introsort functionality (ie when it struggles to obtain O(n*log(n)) execution time).
   Heapsort has a guaranteed O(n*log(n)) execution time. 
   It is however a LOT slower than yasort2 on the test suite (~ 6* based on the total test runtime)
   Note that in practice heapsort is normally used very little so its slower speed is not an issue.  
   The advantage of using a heapsort here is that it makes the sorting code completely independent of the median code,
   ya_msort (in ya-select.c) could be used in its place and it a lot faster - but this makes no difference to the overall execution time of yasort2().    
*/

// heapsort based on code at http://www.codecodex.com/wiki/Heapsort#C.2FC.2B.2B - which itself is based on the numerical recipees algorithm 
// This version supports elem_type_sort2 (ie will sort any type that can be compared ).
// This sorts array a[] keeping values in b[] in the same relative place

static void heapsort2 (elem_type_sort2 *a,elem_type_sort2 *b, size_t n) 
  	{
     size_t  i = n/2, parent, child;
     elem_type_sort2 t,t2;
     for (;;) 
	   	{ /* Loops until a[] is sorted */
         if (i > 0) 
		  	 { /* First stage - creating the heap , highest value will end up in a[0] */
              i--;          
              t = a[i];    /* Save parent value to t */
              t2= b[i];
          	 } 
		  else 
		   	 {     /* Second stage (when i==0) - Extracting elements in-place */
              n--;           /* Make the new heap smaller */
              if (n == 0) return; /* When the heap is empty, we are done */
              t = a[n];    /* Save last value (it will be overwritten on the line below) */
              a[n] = a[0]; /* Save largest value at the end of arr */
              t2= b[n];    
              b[n] = b[0];              
          	 }
  
         parent = i; /* We will start pushing down t from parent */
         child = parent*2 + 1; /* parent's left child */
  
          /* Sift operation - pushing the value of t down the heap */
         while (child < n) 
		  	 { // Choose the largest child - assume left child and adjust if its the right one
              if (child + 1 < n  &&  a[child + 1] > a[child]) 
			  	 {
                  child++; /*  right child is the largest */
              	 }
              if (a[child] > t) 
			  	 { /* If any child is bigger than the parent */
                  a[parent] = a[child]; /* Move the largest child up */
                  b[parent] = b[child];
                  parent = child; /* Move parent pointer to this child */
                  child = parent*2+1; 
              	 } 
				else 
				 {
                  break; /* t's place is found */
              	 }
          	 }
         a[parent] = t; /* We save t in the heap */
         b[parent] = t2; 
     	}
  	}



// This is a version of quicksort that only partitions into 2 partitions (<= pivot, >= pivot) so inner loop has a simpler structure than one that splits into 3 partitions
// The inner quicksort loop does not degrade into a O(n^2) when there are duplicate values in the array to be sorted (many partitioning schemes do).
// Again it uses introsort techniques to achieve O(n*log2(n)) worse case runtime.
// This version was created by Peter Miller 9/1/2022
static void _yasort2(elem_type_sort2 *x,elem_type_sort2 *y, size_t n,unsigned int nos_p); /* main worker function */
#ifdef PAR_SORT /* helper code for Parallel version */

#define PAR_DIV_N 16 /* divisor on n (current partition size) to check size of partition about to be spawned as a new task is big enough to justify the work of creating a new task */
#define PAR_MIN_N 10000 /* min size of a partition to be spawned as a new task */

struct _params 
	{elem_type_sort2 *xp;
	 elem_type_sort2 *yp;
	 size_t np;
	 unsigned int nos_p_p;
	 volatile int task_fin; /* 0 => task running, 1=> task finished. Volatile as set by seperate task.  This variable makes porting to pthreads simpler*/
	};

#ifdef USE_PTHREADS // void *(*start_routine)(void *)
 static void *  yasortThreadFunc( void * _Arg ) // parallel thread that can sort a partition
#else	
 static unsigned __stdcall yasortThreadFunc( void * _Arg ) // parallel thread that can sort a partition
#endif
	{struct _params* Arg=_Arg;
	 Arg->task_fin=0; // This should be set before starting the thread, waiting till now leaves a short time when the thread may appear to have finished but its actually not started... Its done here "just in case".
 	 _yasort2(Arg->xp,Arg->yp,Arg->np,Arg->nos_p_p); // sort required section 
	 Arg->task_fin=1; // indicate task now finished - note it will actually finish when this function returns so this flag just indicates another task can now "wait" on this task finishing
	 // _endthreadex( 0 ); - _endthreadex is call automatically when we return from this function - see https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/endthread-endthreadex?view=msvc-170 
	 return 0;
	}

#ifndef _WIN32
/* this code is for non-Windows systems , beleived to work under Linux, BSD unix and MAC OS */
static int nos_procs(void) /* return the number of logical processors present and enabled */
{ return sysconf(_SC_NPROCESSORS_ONLN);
}
 #ifdef YASORT_MEDIAN_TEST_PROGRAM /* only needed for the test program */
 static void proc_info(void)
  {
	printf("  %d processors available\n",nos_procs());
	printf("  system - number of cpus  is %d\n", sysconf( _SC_NPROCESSORS_CONF));
    printf("  system - enable number of cpus is %d\n", sysconf(_SC_NPROCESSORS_ONLN));
  }
 #endif
#else
/* we ideally need to know the number of processors - the code below gets this for windows */
/* this is from https://docs.microsoft.com/de-de/windows/win32/api/sysinfoapi/nf-sysinfoapi-getlogicalprocessorinformation with minor changes by Peter Miller */

typedef BOOL (WINAPI *LPFN_GLPI)(
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, 
    PDWORD);


// Helper function to count set bits in the processor mask.
static DWORD CountSetBits(ULONG_PTR bitMask)
{
    DWORD LSHIFT = sizeof(ULONG_PTR)*8 - 1;
    DWORD bitSetCount = 0;
    ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;    
    DWORD i;
    
    for (i = 0; i <= LSHIFT; ++i)
    {
        bitSetCount += ((bitMask & bitTest)?1:0);
        bitTest/=2;
    }

    return bitSetCount;
}

#ifdef YA2SORT_TEST_PROGRAM  /* only needed for the test program */
static void proc_info(void)
{
    BOOL done = FALSE;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = NULL;
    DWORD returnLength = 0;
    DWORD logicalProcessorCount = 0;
    DWORD numaNodeCount = 0;
    DWORD processorCoreCount = 0;
    DWORD processorL1CacheCount = 0;
    DWORD processorL2CacheCount = 0;
    DWORD processorL3CacheCount = 0;
    DWORD processorPackageCount = 0;
    DWORD byteOffset = 0;
    PCACHE_DESCRIPTOR Cache;
    while (!done)  /* we need to call GetLogicalProcessorInformation() twice, the 1st time it tells us how big a buffer we need to supply */
    {
        DWORD rc = GetLogicalProcessorInformation(buffer, &returnLength);

        if (FALSE == rc) 
        {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) 
            {
                if (buffer) 
                    free(buffer);

                buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(
                        returnLength);

                if (NULL == buffer) 
                {
                    printf("\nError: Allocation failure\n");
                    return ;
                }
            } 
            else 
            {
                printf("\nError %d\n", (int)GetLastError());
                return ;
            }
        } 
        else
        {
            done = TRUE;
        }
    }

    ptr = buffer;

    while (byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= returnLength) 
    {
        switch (ptr->Relationship) 
        {
        case RelationNumaNode:
            // Non-NUMA systems report a single record of this type.
            numaNodeCount++;
            break;

        case RelationProcessorCore:
            processorCoreCount++;

            // A hyperthreaded core supplies more than one logical processor.
            logicalProcessorCount += CountSetBits(ptr->ProcessorMask);
            break;

        case RelationCache:
            // Cache data is in ptr->Cache, one CACHE_DESCRIPTOR structure for each cache. 
            Cache = &ptr->Cache;
            if (Cache->Level == 1)
            {
                processorL1CacheCount++;
            }
            else if (Cache->Level == 2)
            {
                processorL2CacheCount++;
            }
            else if (Cache->Level == 3)
            {
                processorL3CacheCount++;
            }
            break;

        case RelationProcessorPackage:
            // Logical processors share a physical package.
            processorPackageCount++;
            break;

        default:
            printf("\nError: Unsupported LOGICAL_PROCESSOR_RELATIONSHIP value.\n");
            break;
        }
        byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
        ptr++;
    }

    printf("\n GetLogicalProcessorInformation results:\n");
    printf("  Number of NUMA nodes: %d\n", 
             (int)numaNodeCount);
    printf("  Number of physical processor packages: %d\n", 
             (int)processorPackageCount);
    printf("  Number of processor cores: %d\n", 
             (int)processorCoreCount);
    printf("  Number of logical processors: %d\n", 
             (int)logicalProcessorCount);
    printf("  Number of processor L1/L2/L3 caches: %d/%d/%d\n", 
             (int)processorL1CacheCount,
             (int)processorL2CacheCount,
             (int)processorL3CacheCount);  
    free(buffer);

    return;
}
#endif

static unsigned int nos_procs(void) /* return the number of logical processors present. 
					   This was created by Peter Miller 15/1/2022 based on above code  */
{
    BOOL done = FALSE;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = NULL;
    DWORD returnLength = 0;
    unsigned int logicalProcessorCount = 0;
    unsigned int processorCoreCount = 0;
    DWORD byteOffset = 0; 

    while (!done) /* we need to call GetLogicalProcessorInformation() twice, the 1st time it tells us how big a buffer we need to supply */
    {
        BOOL rc = GetLogicalProcessorInformation(buffer, &returnLength);

        if (rc == FALSE ) 
        {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) 
            {
                if (buffer) 
                    free(buffer);

                buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(
                        returnLength);

                if (NULL == buffer) 
                {
                    // Error: memoery Allocation failure
                    return 0;
                }
            } 
            else 
            {  // other (unexpected) error
                return 0;
            }
        } 
        else
        {
            done = TRUE;
        }
    }

    ptr = buffer;

    while (byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= returnLength) 
    {
        if (ptr->Relationship == RelationProcessorCore) 
        {
            processorCoreCount++;
            // A hyperthreaded core supplies more than one logical processor.
            logicalProcessorCount += CountSetBits(ptr->ProcessorMask);
        }
        byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
        ptr++;
    }

    free(buffer);

    return logicalProcessorCount; // actual number of processor cores is processorCoreCount
}
#endif
#endif

static void _yasort2(elem_type_sort2 *x,elem_type_sort2 *y, size_t n,unsigned int nos_p ) /* main worker function, nos_p is nos processors available */
{
 bool need_max=true,need_min=true;
 elem_type_sort2  min, max;
 int itn=0;
 if(n<=1) return; // avoid trying to take log2 of 0 - anyway an array of length 1 is already sorted  
 const int max_itn=INTROSORT_MULT*ilog2(n); // max_itn defines point we swap to mid-range pivot, then at 2*max_int we swap to heapsort. if INTROSORT_MULT=0 then "always" use heapsort, 3 means "almost never" use heapsort
 min = max = x[0] ;
#ifdef PAR_SORT
 struct _params params;
 #ifdef USE_PTHREADS
 pthread_t *th=NULL;// set to &thread_id when running
 pthread_t thread_id;
 #else
 HANDLE th=NULL; // handle for worker thread (Windows threads)
 #endif
#else
 P_UNUSED(nos_p); // this param is not used unless PAR_SORT is defined
#endif
 while(1) // replace tail recursion with a loop
 	{
	 if (n <= 1) goto sortend; // need a common end point as may be using threads in which case we need to wait for them to complete
	 /* Use a simple (fast) sort for small n [ this is normally an optimal sort (so use n<=32) ] Using small_sort2() is ~ 6% faster than using the insertion sort below for n<=32 */
	 if(n<=32)
	   		{   			
		 	 small_sort2( x,y, n);	
		 	 assert(check_sort( x, n) ); // check result of sort is ordered correctly
		 	 goto sortend; // need a common end point as may be using threads in which case we need to wait for them to complete
	    	}    	
	 // next try an insertion sort, abort this if its taking too much effort
	 // this efficiently sorts arrays that are almost perfectly sorted already.
	 // using this gives an average 33% speedup using the test program when MAX_INS_MOVES=2
	 // There is also  a potentially useful side effect of this, if we have to move to using quicksort then 2 values will have been moved which could help break up bad patterns of data.		 
	 #define MAX_INS_MOVES 2 /* max allowed number of moves - for the test program 2 is the optimum value */
	 size_t nos_ins_moves=0;
	 for (elem_type_sort2 *p=x,*p2=y; p<x+n-1; ++p,++p2) 
			{
			 elem_type_sort2 t = p[1],t2=p2[1];
			 elem_type_sort2  *j = p,*j2=p2;
			 if(*j>t)
			 	{// out of order 
			 	 if(++nos_ins_moves>MAX_INS_MOVES) goto do_qsort; // too many moves  - swap to qsort	
			 	 do
			 		{j--;j2--;
			 		} while(j>=x && *j>t);
				 memmove(j+2,j+1,(size_t)(p-j)*sizeof(elem_type_sort2));// move a portion of array x right by 1 to make space for t
			 	 j[1]=t;
			 	 memmove(j2+2,j2+1,(size_t)(p2-j2)*sizeof(elem_type_sort2));// move a portion of array y right by 1 to make space for t2
			 	 j2[1]=t2;			  		
			 	}
	        }	   		
	 assert(check_sort( x, n) ); // check result of sort is ordered correctly
	 goto sortend; // need a common end point as may be using threads in which case we need to wait for them to complete
	  
	do_qsort:  ; // ; as label can only be attached to a statement and the line below is a declaration ... 

   // if we have made too many iterations of this while loop then we need to swap to heapsort 
   if(++itn>2*max_itn)
  		{	 
#ifdef DEBUG 	  
	  	 printf("yasort2: using heapsort(%.0f)\n",(double)n);
#endif	  
	  	 heapsort2(x,y,n);
	  	 assert(check_sort( x, n) ); // check result of sort is ordered correctly
  	  	 goto sortend; // need a common end point as may be using threads in which case we need to wait for them to complete
  		}
	/* start of quicksort */
	/* The partitioning algorithm used here partitions into 2 groups, <= pivot and >= pivot 
	   The code can also create a partition = pivot in some cases but this partition does not include all values = pivot
	*/
	/* select pivot into v - if a large number of elements use median of 25 elements otherwise use median of 9 elements.
	   if we have made > max_itn loops then we use the mid-range for the pivot.
	   If we get here we know array has >32 elements in it 
	*/
	elem_type_sort2 v; // pivot value
	if(itn>max_itn)
		 { /* using medians to select the pivot has failed, swap to using the mid-range ( (max-min)/2 ) as the pivot value.
		 	  This is the pivot value used by the Torben algorithm - see ya-median.c for more details.
 			  Because this pivot selection method looks at all the values in x[] it always makes some progress and is not sensitive to the "pattern" of the data in x[]
 			  You could also consider this as a radix exchange sort (especially if sorting integers) - see "The art of concurrency, A thread monkey's guide to writing parallel applications", Clay Breshears, 2009, page 182-3 
			  A radix exchange sort has a linear time [ O(n) ] asymptotic complexity. 			  
		   */
#ifdef DEBUG 	  
	  	   printf("yasort2: using mid-range as pivot (%.0f)\n",(double)n);
#endif	 		   
		   if(need_max && need_min)
		    	{
                 min = max = x[0] ;
			     for (elem_type_sort2 *p=x+1; p<x+n; p++) 
                    {// find min & max
			    	 elem_type_sort2 t=*p;
			         if (t<min) min=t;
			         if (t>max) max=t; 
			    	}
			    }
			else if(need_max)
		    	{max = x[0] ; 
				 for (elem_type_sort2 *p=x+1; p<x+n; p++) 
                    {//  find max
			    	 elem_type_sort2 t=*p;
			         if (t>max) max=t; 
			    	}
			    }	
			else if(need_min)/* need_min */
                {min =  x[0] ;
			     for (elem_type_sort2 *p=x+1; p<x+n; p++) 
					{//  find min
			    	 elem_type_sort2 t=*p;
			         if (t<min) min=t;
			    	}
			    }	    
		 	if(max==min) 
		 	 	{
			 	 // only 1 value - so nothing left to sort. This should never happen as the insertion sort would have trapped this and already returned...
			 	 goto sortend; // need a common end point as may be using threads in which case we need to wait for them to complete
			 	}    	
		 	else
		 		{
				 v= min/2+max/2; // reasonable estimate of median, so use as pivot.		
				}
   		 }	
	else if(n>10000 )
		{// large number of elements then take median of 25 values
	 	 // use 1st 5 middle 5 and last 5 and two sets of 5 inbetween to try and be "cache friendly"
	 	 elem_type_sort2 a[25];
	 	 const size_t b=(n-1)/2; // middle item
	 	 size_t c=b/2;// 1/4 point
	 	 // copy values into array a in blocks of 5
	 	 memcpy(a,x,5*sizeof(elem_type_sort2)); // to,from,size :start: x[0],x[1],x[2],x[3],x[4]
	 	 memcpy(a+5,x+c-2,5*sizeof(elem_type_sort2));// 1/4 : x[c-2],x[c-1],x[c],x[c+1],x[c+2]
		 memcpy(a+10,x+b-2,5*sizeof(elem_type_sort2)); // middle: x[b-2],x[b-1],x[b],x[b+1],x[b+2] 
		 c+=b;// 3/4 point
		 memcpy(a+15,x+c-2,5*sizeof(elem_type_sort2)); // 3/4: x[c-2],x[c-1],x[c],x[c+1],x[c+2]
		 memcpy(a+20,x+n-5,5*sizeof(elem_type_sort2)); // end: x[n-5],x[n-4],x[n-3],x[n-2],x[n-1]
		 v=opt_med25(a); 	
		}
	 else	
  		 /* Median of 9 items selected uniformly across dataset  */
		{
	 	 elem_type_sort2 a[9];
	 	 const size_t b=(n-1)/2; // middle item
	 	 const size_t c=(n-1)/9; 
	 	 a[0]=x[0];
	 	 a[1]=x[2*c];
	 	 a[2]=x[3*c];
	 	 a[3]=x[4*c];
	 	 a[4]=x[b];
	 	 a[5]=x[6*c];
	 	 a[6]=x[7*c];
	 	 a[7]=x[8*c];
	 	 a[8]=x[n-1];
	 	 v=opt_med9(a);
	 	}
    /* partitioning scheme with pivot as a value (v) */
	 // v is pivot (for this implementation this does not have to be a value thats actually in the array, but it should be in the range of values in the array !
     /* this version uses pointers rather than array indices - this might be simpler for a compiler to optimise ? and it also avoids issues with unsigned values going to "-1" */
	 /* with TDM-gcc 10.3.0 this is about 2% faster on the test program than a version using array indices */
	 elem_type_sort2 *pi,*pj;// 2 pointers to replace indices i,j
	 pi=x-1;// one before start as loop below starts with pi++
	 pj=x+n; // one after end as loop below starts pj--
	 for (;;) 
	 	{
		 do pi++; while (pj >= pi && *pi < v); // was i < n
		 do pj--; while (pj >= pi && *pj > v);
		 if (pj < pi) break; 
		 //eswap2(pi-x,pj-x, x, y); // still uses indices
		 eswap2p(pi,pj, x, y); 
		}	
	 pj=pi;// now look for multiple values equal to v adjacent to pivot position (pi)
	 if(*pi==v)
	 	{
	 	 while(pi<x+n-1 && pi[1]==v) ++pi;
	 	 while(pj>x && pj[-1]==v) --pj;
	 	}	
#ifdef PAR_SORT	 
	  /* if using parallel tasks check here to see if task spawned from this function has finished, if so we can spawn another one to keep it busy
	     We allow spawned tasks and recursive calls to spawn more tasks if we have enough processors (thats the (nos_p)/2 passed as a paramater)
		 This approach should scale reasonably well without needing any complex interactions betweens tasks (as they are all working on seperate portions of the arrays x & y)
		 With 1 processor everything is in main function.
		 With 2 processors we use both
		 With 4 processors we use 3 [ as (4)/2=2, then 2/2=1 , so we use main processor and spawn 2 threads]
		 With 8 (logical) processors we use 7 [ (8)/2=4, (4)/2=2, then 2/2= 1 so we use main processor + 2 threads + 4 threads = 7 in total ]
		 In terms of run time - using a processor with 8 logical cores (4 physical cores) available:
		 1 core (PAR_SORT not defined) - test program sorting largest size took 52.662 secs
		 4 cores																21.767 secs = 2.4* speedup
		 8 cores 						 										19.350 secs = 2.7* speedup (there were only 4 physical cores which may partly explain the reduced improvement)
	  */ 	
	  if(th!=NULL && n>2*PAR_MIN_N )
	  	{// if thread active and partition big enough that we might be able to use a parallel task (2* as we will at least halve the size of the partition for the parallel task) 
 #ifdef USE_PTHREADS
 	  	 if( params.task_fin==1 )
	  		{// if thread has finished 
	  		 pthread_join(thread_id,NULL);
			 th=NULL; // set to NULL so we can reuse it
			 // dprintf("Thread finished within function processing size=%llu nos_p=%d\n",n,nos_p); 
			}
 #else	  	
	  	 if( params.task_fin==1 && WaitForSingleObject( th, 0 )!=WAIT_TIMEOUT)
	  		{// if thread has finished - using params.task_fin==1 might be more efficient than always calling WaitForSingleObject() - but anyway it makes porting to pthreads simpler
    		 CloseHandle( th );// Destroy the thread object.
			 th=NULL; // set to NULL so we can reuse it
			 // dprintf("Thread finished within function processing size=%llu nos_p=%d\n",n,nos_p); 
			}
 #endif	
		}
#endif     
	 if((size_t)(pj-x)<n-(size_t)(pi-x)) // j<n-i
	 	{
#ifdef PAR_SORT
		if( th==NULL && nos_p>1 && (size_t)(pj-x) > n/PAR_DIV_N && (size_t)(pj-x) > PAR_MIN_N)
			{// use a worker thread last 2 tests check the overhead of thread creation is worth it.
			 params.xp=x;
			 params.yp=y;
			 params.np=(size_t)(pj-x);
			 params.nos_p_p=(nos_p)/2; // if we still have spare processors allow more threads to be started
			 params.task_fin=0; // task has not yet finished
 #ifdef USE_PTHREADS
 			 if(pthread_create(&thread_id,NULL,yasortThreadFunc,&params)==0)
 			 	{th=&thread_id; // success
 			 	}
 			 else
 			 	{th=NULL; // failed to run task
 				}
 #else		/* use native Windows threads */	 
			 th=(HANDLE)(uintptr_t)_beginthreadex(NULL,0,yasortThreadFunc,&params,0,NULL);
 #endif			 			 
			 if(th==NULL) _yasort2(x,y, (size_t)(pj-x),0); // if starting thread fails then do in this process.  nos_p=0 so don't run any tasks from here
			}
		else
			{_yasort2(x,y, (size_t)(pj-x),(nos_p)/2); // recurse for smalest partition so stack depth is bounded at O(log2(n)). Allow more threads from subroutine if we still have some processors spare
			 assert(check_sort(x,(size_t)(pj-x)));// check sort worked correctly
			}
#else	 		
		 _yasort2(x,y, pj-x,0); // recurse for smalest partition so stack depth is bounded at O(log2(n))
		 assert(check_sort(x,pj-x));// check sort worked correctly
#endif		 

	 	 y+=pi-x;
	 	 n-=(size_t)(pi-x);
	 	 x=pi;
	 	}
	 else
	 	{
#ifdef PAR_SORT
		if( th==NULL && nos_p>1 && (n-(size_t)(pi-x)) > n/PAR_DIV_N && (n-(size_t)(pi-x)) > PAR_MIN_N)
			{// use a worker thread
			 params.xp=pi;
			 params.yp=y+(pi-x);
			 params.np=n-(size_t)(pi-x);
			 params.nos_p_p=(nos_p)/2;			 
			 params.task_fin=0; // task has not yet finished
 #ifdef USE_PTHREADS
 			 if(pthread_create(&thread_id,NULL,yasortThreadFunc,&params)==0)
 			 	{th=&thread_id; // success
 			 	}
 			 else
 			 	{th=NULL; // failed to run task
 				}
 #else		/* use native Windows threads */	 
			 th=(HANDLE)(uintptr_t)_beginthreadex(NULL,0,yasortThreadFunc,&params,0,NULL);
 #endif	
			 if(th==NULL) _yasort2(pi,y+(pi-x), n-(size_t)(pi-x),0); // if starting thread fails then do in this process.  nos_p=0 so don't run any tasks from here
			}
		else
			{ _yasort2(pi,y+(pi-x), n-(size_t)(pi-x),(nos_p)/2); // recurse for smalest partition so stack depth is bounded at O(log2(n))
	 	 	 assert(check_sort(pi,(size_t)(n-(size_t)(pi-x)))); // check sort worked correctly
			}
#else			
		 _yasort2(pi,y+(pi-x), n-(size_t)(pi-x),0);
	 	 assert(check_sort(pi,n-(pi-x))); // check sort worked correctly
#endif	 	 
	 	 n=(size_t)(pj-x);	
		}		 		 
	} // end while(1)
 sortend: ; // need a common end point as may be using threads in which case we need to wait for them to complete	
 #ifdef PAR_SORT
 if(th!=NULL)
 	{// if a thread used and its still running, need to wait for it to finish
  #ifdef USE_PTHREADS
	 pthread_join(thread_id,NULL);
  #else	 /* using native Windows threads */ 	 
 	 WaitForSingleObject( th, INFINITE );
     // Destroy the thread object.
     CloseHandle( th );
  #endif
	}  	
 #endif
 return; 
}

void yasort2(elem_type_sort2 *x,elem_type_sort2 *y, size_t n) /* user function  */
{
#ifdef PAR_SORT	
 unsigned int nos_p=nos_procs() ;// total number of (logical) processors available
 _yasort2(x,y, n,nos_p); /* call main worker function */
#else
 _yasort2(x,y, n,1); /* call main worker function , with only 1 processor (as NOS_PAR not defined ) */
#endif
}


#ifdef YA2SORT_TEST_PROGRAM /* test program required, times execution - while doing a set of benchmark tests for functionality */

#include "hr_timer.h"
#include <math.h>
#include "yamedian.h" // we also test median code, so need header for that as well

static size_t dataLen=100000001; /* should normally be 100000001 (needs this for median of 3 killer pattern) and elem_type_sort2 should be double for most tests */
#define NOS_TESTS 3 /* number of runs of median() to time [normally 3 - we use median of this many runs ]*/
#define TIMES_IGNORE 0 /* ignore this many longest runtimes [ normally 0 - median time which rejects outliers so this has little use now ] */

/* use the #if chain below to select what you want to check/benchmark. */

#if 1 /* test ya2sort  */

// void qsort2(elem_type_sort2 *a,elem_type_sort2 *b, int n)
#define MEDIAN_NAME "yasort2-2 partition " // name of algorithm we are testing
#define median(a,b,s) (yasort2(a,b,s),a[(s-1)/2])  // function call to test 
#define IS_SORT /* method sorts data */

#else /* test heapsort2 */
#define MEDIAN_NAME "heapsort2" // name of algorithm we are testing
#define median(a,b,s) (heapsort2(a,b,s),a[(s-1)/2])  // function call to test 
#define IS_SORT /* method sorts data */

#endif

/* insertion sort for small arrays of doubles (used in test program below) */
static inline void dbl_ins_sort( double *x, const size_t n)
	{
	 if(n<2) return;// if length 1 then already sorted  
     for (double *p=x; p<x+n-1; ++p) 
		{
		 double t = p[1];
		 double *j = p;
		 if(*j>t)
		 	{// out of order 
		 	 do
		 		{j--;
		 		} while(j>=x && *j>t);
		 	 memmove(j+2,j+1,(p-j)*sizeof(double));// move a portion of array x right by 1 to make space for t 		 
		 	 j[1]=t;		
		 	}
        }							
	 return ; // all done
	}

#define STR(x)   #x
#define XSTR(x)  STR(x) /* generate a string that the macro expanded value of x */

int main(int argc, char *argv[])
{
 elem_type_sort2 *data,*data2; // actual array of items that needs to be sorted - space allocated by malloc below 
 size_t i;
 elem_type_sort2 m,m_s=0;// m is median from my function, m_s is median from sort
 double start_t,end_t; // for hr_timer
 double durations[NOS_TESTS];
 double av_duration=0;
 double total_time=0;
 double max_time=0;
 int nos_lengths=0;// number of different lengths of data[] we have tested 
 int errs=0;
 uint64_t sum_before=0,sum_after; // sum of all values to be sorted (before sort, after sort)
 uint32_t xor_before=0,xor_after; // ditto but xor of all values
 FILE *logout=NULL;
 P_UNUSED(argc) ;/* a way to avoid warning unused parameter messages from the compiler */
 P_UNUSED(argv) ;/* a way to avoid warning unused parameter messages from the compiler */
 init_HR_Timer();
#if __SIZEOF_POINTER__ == 8 /* 64 bit pointers */
 printf("Compiled for 64 bit pointers : ");
#elif __SIZEOF_POINTER__ == 4 /* 32 bit pointers */
 printf("Compiled for 32 bit pointers : ");
#else
 printf("Compiled for unknown pointer size ! : "); 
#endif 
#if defined(PAR_SORT) 
 proc_info(); // info about the processor thats running this
 printf("%d logical processor(s) available\n",nos_procs());
 #ifdef USE_PTHREADS
  printf("Using pthreads\n");
 #else
  printf("Using native Windows threads\n");
 #endif 
#endif 
 logout=fopen("logout.csv","w");
 if(logout==NULL) printf("Warning: cannot open logfile\n");
 else fprintf(logout,"Size,average time(s),max time(s)\n");
 printf("elem_type_sort2=%s,sizeof(size_t)=%d log2(sizeof(size_t))=%d log2(%d)=%d\n",XSTR(elem_type_sort2),(int)sizeof(size_t),ilog2(sizeof(size_t)),(int)dataLen,ilog2(dataLen)); 
 dprintf("RAND_MAX=%u (0x%x)\n",RAND_MAX,RAND_MAX);
 printf("Timing %s(%d)\n",MEDIAN_NAME,(int)dataLen);
 int testtype=0;// normally 0 to do all tests
 size_t k;
 data=(elem_type_sort2 *)malloc(dataLen * sizeof(elem_type_sort2));/// allocate memory for data to be searched for medians
 if(data==NULL)
 	{fprintf(stderr,"Error - not enough RAM to allocate data array of size %u\n",(unsigned)dataLen);
 	 exit(1);
 	}
 data2=(elem_type_sort2 *)malloc(dataLen * sizeof(elem_type_sort2));/// allocate memory for data to be searched for medians
 if(data==NULL)
 	{fprintf(stderr,"Error - not enough RAM to allocate data array2 of size %u\n",(unsigned)dataLen);
 	 exit(1);
 	} 	
 while(1)
  {++testtype;// 1...
   for(int test_nos=0;test_nos<NOS_TESTS;++test_nos) 
 	{
 	 switch(testtype) // 1...
	  {case 1:	
 	 		for(i=0; i<dataLen;++i) data[i]=i; // just increasing
 	 		if(test_nos==0) printf("Test pattern is ramp 0...%d\n",(int)dataLen-1);
			break;		 	 
	   case 2:
 	 		for(i=0; i<dataLen;++i) data[i]=i+1; // just increasing 1,2,... but zero at end
 	 		data[i-1]=0; // 0 at end 
 	 		if(test_nos==0) printf("Test pattern is ramp 1...%d,0\n",(int)dataLen-1);
 	 		break;
	   case 3:
 	 		for(i=0; i<dataLen;++i) data[i]=i&0xff; // sawtooth	
 	 		if(test_nos==0) printf("Test pattern is sawtooth 0..255\n");
 	 		break;
 	   case 4:
 			k=dataLen/2;
 			for(i=0;i<k;++i)
				{
	 	 		 data[i]=((i)&1)==0?i+1:i+k;// k=size/2; arr=1,k+1,3,k+3,5,.... 
				} 
 			for(;i<dataLen;++i)
				{
	 	 		 data[i]=(i+1-k)*2;// k=size/2; arr=2,4,6....2k-2,2k 
				} 	 
 			if(test_nos==0) printf("Test pattern is median of 3 killer:\n"); // *** needs size 10000001			 
 			break;
 		case 5: 
 			for(i=0;i<dataLen;++i)
				{
	 			data[i]=5;
				}
			if(test_nos==0)  printf("Array is all 5:\n");
			break;
 		case 6: 
 			for(i=0;i<dataLen;++i)
				{
	 			data[i]=5;
				}
			data[dataLen/5]=0;// 2 different  values spread in array
 			data[dataLen/20]=10; 		
			if(test_nos==0)  printf("Array is all 5 apart from 2 elements which are 0,10:\n");
			break;	
 		case 7: 
 			for(i=0;i<dataLen;++i)
				{
	 			data[i]=(i&0xff) ^ 0x5a;//to give lots repeating values in a "random" order
				}		
			if(test_nos==0)  printf("Array has lots of repeating values in a random order:\n");
			break;	
 		case 8: 
 			for(i=0;i<dataLen;++i)
				{
	 			data[i]=(i>>6);// >>6 = /64, array sizes upto 32 are special cases so wanted to be more than that
				}		
			if(test_nos==0)  printf("Array is slowly increasing with lots of repeating values:\n");
			break;
 		case 9: 
 			for(i=0;i<dataLen;++i)
				{
	 			data[i]=dataLen-(i>>6);// >>6 = /64, array sizes upto 32 are special cases so wanted to be more than that
				}		
			if(test_nos==0)  printf("Array is slowly decreasing with lots of repeating values:\n");
			break;
		case 10: 
			srand(testtype); // we always want the same sequence as we only want to check the median via sort once
 			for(i=0;i<dataLen;++i)
				{
	 			data[i]=rand();
				}		
			if(test_nos==0)  printf("Array is full of random values:\n");
			break;	
		case 11: 
			srand(testtype); // we always want the same sequence as we only want to check the median via sort once
 			for(i=0;i<dataLen;++i)
				{
	 			data[i]=rand()&1;// 0 and 1 only
				}		
			if(test_nos==0)  printf("Array is full of random binary (0/1) values:\n");
			break;	
		case 12: 
			srand(testtype); // we always want the same sequence as we only want to check the median via sort once
 			for(i=0;i<dataLen;++i)
				{
	 			data[i]=bi_modal_rand();
				}		
			if(test_nos==0)  printf("Array is full of bi-modally distributed random values:\n");
			break;				
	    case 13:	
 			for(i=0;i<dataLen;++i) data[i]=dataLen-1-i; // just decreasing
 	 		if(test_nos==0) printf("Test pattern is ramp %d...0\n",(int)dataLen-1);
 	 		break;
	    case 14:
 	 		for(i=0;i<dataLen;++i) data[i]=dataLen-i; // just decreasing but zero at start
 	 		data[0]=0; // 0 at start 
 	 		if(test_nos==0) printf("Test pattern is ramp 0,%d...1\n",(int)dataLen-1);
 	 		break;	
		case 15: 
			srand(testtype); // we always want the same sequence as we only want to check the median via sort once
 			for(i=0;i<dataLen;++i)
				{
	 			data[i]=rand()/4 +rand()/4+rand()/4+rand()/4;// approximate a normal distribution by adding four uniform distribution (only true if elem_type_sort2 has 32 bits or more or is floating point ) 
				}		
			if(test_nos==0)  printf("Array is full of normally distributed values:\n");
			break;	
		case 16: 
			// exponential ramp - repeating like a sawtooth. 0,1,2,4,8,16,32,... then back to 0,1,2,...
			data[0]=0;
 			for(i=1;i<dataLen;++i)
				{
				 if(data[i-1]==0 ) data[i]=1;
				 else data[i]=data[i-1]*2; // *2, assume overflows to 0 if elem_type_sort2 is an "integer" type.
				 if(!isnormal((double)data[i])) data[i]=0;// if elem_type_sort2 is float or double then trap overflow and make 0 in the same was as integers behave.
				}		
			if(test_nos==0)  printf("Array is exponentially ramping sawtooth:\n");
			break;		
		case 17: 
			// shuffle of simple sequence 0,..n-1
 	 		for(i=0; i<dataLen;++i) data[i]=i; // just increasing
 	 		if(test_nos==0) 
			  	{printf("Test pattern is shuffle of ramp 0...%d\n",(int)dataLen-1);			
			  	 srand(testtype); // initialise random generator - only do this once so we get different shuffles every time, as this is a shuffle all the means should be the same
			  	}
			// Fisher Yates shuffle see eg Programming classics page 244 algorithm 8.2.1 Random~Shuffle
 			for(i=0;i<dataLen-1;++i)
				{
				 eswap1(i, i+rand()%(dataLen-i),data); // swap element with random later element - the use of % will not quite give a uniform distribution for large dataLen but thats not really an issue here
				}		
			for(i=0;i<20;++i) {dprintf(" data[%d]=%.0f",(int)i,(double)data[i]);} dprintf("\n"); // print start of shuffled sequence
			break;										  																 					 
 		default:
 			if(logout!=NULL) fprintf(logout,"%.0f,%.3f,%.3f\n",(double)dataLen,total_time/(testtype-1),max_time); 
 			printf("Sum of times for all sorts is %.3f secs, average sort time was %.3f secs, max sort time was %.3f secs\n",total_time,total_time/(testtype-1),max_time); 
 			testtype=1;// get ready to repeat with a different length for data[]
 			test_nos= -1;// -1 as incremented by for loop before next iteration
 			if((dataLen&1)==1) --dataLen; // change odd to even
 			else dataLen=(dataLen>>1)|1; // divide by 2 and make even
 			total_time=0;
 			max_time=0;
 			nos_lengths++;
 			if(nos_lengths==10) 
			 	{if(errs==0)
			 		printf("\n  no errors found during tests\n");
			 	 else
				 	printf("\n  a total of %d error(s) found during tests\n",errs);
				 if(logout!=NULL) 
				 	{printf("Time vs size stored in logout.csv\n");	
				 	 fclose(logout);	
				 	}
				 exit(errs);// all done, normal exit, 0 means good <>0 means error(s)
				}
 			printf("**** Size now is %d\n",(int)dataLen);
 			continue;
 	   }
	 if(test_nos==0)
	 	{ // 1st time for each testtype we calculate before sum and xor, no need to repeat this as it will not change for the same testtype	   
	 	 sum_before=sum_after=0;
	 	 xor_before=xor_after=0;
		 for(i=0; i<dataLen;++i)  
	 		{sum_before+=(uint32_t)data[i];
	 	 	 xor_before ^= (uint32_t)data[i];
	 	 	 data2[i]=-data[i]; // nice simple pattern to check
	 		}		
	 	}
	 else
	 	{ 	   
	 	 sum_after=0;
	 	 xor_after=0;	
	 	 // always need to restore data2[]
		 for(i=0; i<dataLen;++i)  
	 		{
	 	 	 data2[i]=-data[i]; // nice simple pattern to check
	 		}				  	
	 	}	 	
 	 // printf("S");  
 	 start_t=read_HR_Timer(); // start of timed section
 	 m=median(data,data2,dataLen); // warning median changes order of elements of array iarr!
 	 end_t=read_HR_Timer(); // end of timed section
 	 // printf("E");
 	 durations[test_nos]=end_t-start_t; // duration for this test
	 dprintf("Check-full - sort:\n"); 	 
 	 // check we still have the same relationship between the 2 arrays, and the sum and xor are unchanged which means there is a high probability all the data items are unchanged. 
 	 // xor (longitudinal parity) will bit up an odd number of bit errors in every bit position, while the sum ("checksum") will detect most other errors
	 for(i=0; i<dataLen;++i)  
	 	{sum_after+=(uint32_t)data[i];
	 	 xor_after ^= (uint32_t)data[i];
	 	 if(data2[i] != -data[i])	 
	 		{printf("  data[%.0f]=%.0f data2[%.0f]=%.0f expected %.0f\n",(double)i,(double)data[i],(double)i,(double)data2[i],(double)(-data[i])); 		 
			 ++errs;
			}
		}
	 if(sum_after!=sum_before)
	 	{printf(" Error: Checksum changed by sort before=%llX after=%llX\n",sum_before,sum_after);
		 ++errs;
		}
	 if(xor_after!=xor_before)
	 	{printf(" Error: Parity changed by sort before=%X after=%X\n",xor_before,xor_after);
		 ++errs;
		}		

   	 if(!check_sort(data,dataLen)) 
   		{printf("  data is not sorted after median calculation\n");  
   	 	 ++errs;
   		} 
   	 if(testtype==1 || testtype==2 || testtype==13 || testtype==14 || testtype==17)
   	 	{ // all of these should give a sorted result that is easy to check exactly */
   	 	 for(i=0; i<dataLen;++i) 
			 	if(data[i]!=i) // just increasing
		 			{printf("  data[%.0f]=%.0f expected %.0f\n",(double)i,(double)data[i],(double)i); 		 
		 	 		 ++errs;
		 			}
		}
 	 if(test_nos==0)
 	 	{m_s=ya_median(data,dataLen); // get median using a method that does not change array (and does not involve sorting).
   	     printf ("%s(iarr,%d)=%.0f (via ya_median is %.0f)",MEDIAN_NAME,(int)dataLen,(double)m,(double)m_s);
   		 if(m==m_s) printf(" : OK\n"); 
 	  	 else 
	   		{printf(" : Median incorrect!\n");
	     	 ++errs;
			}					
 	 	}
	 else
	 	{// no need to repeat ya_median() as we already know correct median (m_s)
   		 if(m!=m_s)  
	   		{printf ("%s(iarr,%d)=%.0f (via ya_median is %.0f)",MEDIAN_NAME,(int)dataLen,(double)m,(double)m_s);
			 printf(" test_nos=%d : Median incorrect!\n",test_nos);
	     	 ++errs;
			}	 		
		}
 	}
   for(i=0;i<NOS_TESTS;++i)
   	 printf("%c%.3f ms",i==0?' ':',',1000.0*durations[i]);
   dbl_ins_sort(durations,NOS_TESTS); // sort times (array doubles) 
   av_duration=0;
   for(i=0;i<NOS_TESTS-TIMES_IGNORE;++i)
 		av_duration+=durations[i];
   av_duration/=(double)(NOS_TESTS-TIMES_IGNORE);	
 
   printf(" Sorting via %s(%d) took %.3f msecs [range was %.3f to %.3f ms, average %.3f ms] \n",MEDIAN_NAME,(int)dataLen,1000.0*durations[(NOS_TESTS-TIMES_IGNORE-1)/2]/*median*/,1000.0*durations[0],1000.0*durations[NOS_TESTS-TIMES_IGNORE-1],1000.0*av_duration);
   total_time+=durations[(NOS_TESTS-TIMES_IGNORE-1)/2];// used to be durations[0] which added min times, but median is more reasonable as some test cases have a single min time thats much smaller than the other times.
   if(durations[(NOS_TESTS-TIMES_IGNORE-1)/2] > max_time) max_time=durations[(NOS_TESTS-TIMES_IGNORE-1)/2];
  } // end while(1)
 exit(1); // should not get here 
}
#endif
