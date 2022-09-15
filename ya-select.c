/*   ya-select.c
	 ===========
	 Fast selection & median functions.
	 
These functions all work on arrays of numbers.

yaMedian() - returns the median of an array of numbers (the array is reordered)
yaselect() - returns the n'th highest number from an array of numbers (the array is reordered)
ya_msort() - sorts an array of numbers	 

The algorith used is based on the paper "Fast Deterministic Selection" by Andrei Alexandrescu, 16th International Symposium on Experimental Algorithms (SEA 2017) 
yaMedian() & yaselect() have guaranteed o(n) execution time by using the principles of introselect - see "Introspective sorting and selection algorithms" by D.R.Musser,Software practice and experience, 8:983-993, 1997.
ya_msort() has a guaranteed O(n*log2(n)) execution time by using a quicksort with the pivot selected as the median using yaMedian().

You select the type of the arrays used by defining elem_type_median in yamedian.h

This implementation by Peter Miller 18/12/2021.

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
// you should not need to edit anything below here for normal use of this software (you may need to edit yamedian.h)
#ifdef DEBUG
 #include <stdio.h>
#endif 
#include <stdint.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h> /* includes memmove() */
#include <assert.h>

#include "yamedian.h"  /* defines elem_type_median etc */

#define INTROMEDIAN_DIV 3 /* divisor  helps define when not enough progress is being made and we should swap to a more robust (but slower) algorithm */
						  /* INTROMEDIAN_DIV must be >= 1, 1 means always use slower algorithm (after LIMIT iterations) */
						  /*  even 1 and 10 for limit means that the slower algorithm is very rarely used so it has almost no impact on execution time with test program */
#define INTROMEDIAN_LIMIT 10 /* the number of cumulative times the limit must be exceed before we swap to guaranteed (but slower) O(n) approach */

#define cswap(i,j) {elem_type_median t;t=(i);i=(j);j=t;} // example call cswap(r[i], r[minIndex]); WARNING - side effects could be an issue here ! eg cswap(r[i++],r[b])

#define tswap(i,j) {size_t t;t=(i);i=(j);j=t;} // example call tswap(a, b ); WARNING - side effects could be an issue here ! eg tswap(i++,b)


#define elem_type_ss elem_type_median /* set type for smallsort correctly */
#include "ya-smallsort.h" // contains small_sort() - this needs to be included after cswap is defined
#if !defined(NDEBUG)
static  bool check_sort( elem_type_median *a, size_t n) // check result of sort is ordered correctly
{// returns true if sort is ok
 /* check array actually is sorted correctly in increasing order */
 size_t errs=0;
 if(n<2) return true;
 for(size_t i=0;i<n-1;++i)
	 if(a[i+1]<a[i])
		errs++;
  return (errs==0);
}
#endif

/* 
 	yaselect()
 	==========
 	
 Based on the algorithm in "Fast Deterministic Selection" by Andrei Alexandrescu, 16th International Symposium on Experimental Algorithms (SEA 2017) 

 The original algorithm always used sampling so in theory its possible its runtime could become very large with some inputs, thats avoided here by (automatically) turning sampling off when this might happen.
 This is done using the principle of introselect - see "Introspective sorting and selection algorithms" by D.R.Musser,Software practice and experience, 8:983-993, 1997.
 Turning off sampling approximatly doubles the run time, so its left on whenever possible.
 Code should always have a runtime O(n).
 It also uses O(log2(n)) RAM [ stack ]. In the test program the max recursion depth was 17 without sampling, 13 with sampling for log2(n)=26.
*/

void select_driver(elem_type_median* r, size_t n, size_t length,int intro_cnt);

/**
expandPartitionRight(elem_type_median* r, size_t hi, size_t rite)
Input assumptions:

(a) hi <= rite
(c) the range r[0 .. hi] contains elements no smaller than r[0]

Output guarantee: same as Hoare partition using r[0] as pivot. Returns the new
position of the pivot.
*/
static inline size_t expandPartitionRight(elem_type_median* r, size_t hi, size_t rite)
{
    size_t pivot = 0;
    elem_type_median r0=r[0];
    assert(pivot <= hi);
    assert(hi <= rite);
    // First loop: spend r[pivot .. hi]
    for (; pivot < hi; --rite)
    {
        if (rite == hi) goto done;
        if (r[rite] >= r0) continue;
        ++pivot;
        assert(r[pivot] >= r0);
        cswap(r[rite], r[pivot]);
    }
    // Second loop: make left and pivot meet
    for (; rite > pivot; --rite)
    {
        if (r[rite] >= r0) continue;
        while (rite > pivot)
        {
            ++pivot;
            if (r0 <r[pivot])
            {
                cswap(r[rite], r[pivot]);
                break;
            }
        }
    }

done:
    cswap(r[0], r[pivot]);
    return pivot;
}

/**
expandPartitionLeft(elem_type_median * r, size_t lo, size_t pivot)
Input assumptions:

(a) lo > 0, lo <= pivot
(b) the range r[lo .. pivot] already contains elements no greater than r[pivot]

Output guarantee: Same as Hoare partition around r[pivot]. Returns the new
position of the pivot.

*/
static inline size_t expandPartitionLeft(elem_type_median * r, size_t lo, size_t pivot)
{
    assert(lo > 0 && lo <= pivot);
    size_t left = 0;
    const size_t oldPivot = pivot;
    elem_type_median op=r[oldPivot];
    for (; lo < pivot; ++left)
    {
        if (left == lo) goto done;
        if (op >= r[left]) continue;
        --pivot;
        assert(op >= r[pivot]);
        cswap(r[left], r[pivot]);
    }
    // Second loop: make left and pivot meet
    for (;; ++left)
    {
        if (left == pivot) break;
        if (op >= r[left]) continue;
        for (;;)
        {
            if (left == pivot) goto done;
            --pivot;
            if (r[pivot] < op)
            {
                cswap(r[left], r[pivot]);
                break;
            }
        }
    }

done:
    cswap(r[oldPivot], r[pivot]);
    return pivot;
}

/**
expandPartition(elem_type_median* r, size_t lo, size_t pivot, size_t hi, size_t length)
Input assumptions:

(a) lo <= pivot, pivot < hi, hi <= length
(b) the range r[lo .. pivot] already contains elements no greater than
r[pivot]
(c) the range r[pivot .. hi] already contains elements no smaller than
r[pivot]

Output guarantee: Same as Hoare partition around r[pivot], returning the new
position of the pivot.
*/
static inline size_t expandPartition(elem_type_median* r, size_t lo, size_t pivot, size_t hi, size_t length)
{
    assert(lo <= pivot && pivot < hi && hi <= length);
    --hi;
    --length;
    size_t left = 0;
    elem_type_median rp=r[pivot];
    for (;; ++left, --length)
    {
        for (;; ++left)
        {
            if (left == lo)
                return pivot +
                    expandPartitionRight(r + pivot, hi - pivot, length - pivot);
            if (r[left] > rp) break;
        }
        for (;; --length)
        {
            if (length == hi)
                return left +
                    expandPartitionLeft(r + left, lo - left, pivot - left);
            if (rp >= r[length]) break;
        }
        cswap(r[left], r[length]);
    }
}


/**
Returns the index of the median of r[a], r[b], and r[c] without writing
anything.
*/
static inline size_t medianIndex(const elem_type_median* r, size_t a, size_t b, size_t c)
{
    if (r[a] > r[c]) tswap(a, c);
    if (r[b] >r[c]) return c;
    if (r[b] < r[a]) return a;
    return b;
}

/**
Tukey's Ninther: compute the median of r[_1], r[_2], r[_3], then the median of
r[_4], r[_5], r[_6], then the median of r[_7], r[_8], r[_9], and then swap the
median of those three medians into r[_5].
*/
static inline void ninther(elem_type_median* r, size_t _1, size_t _2, size_t _3, size_t _4, size_t _5,
    size_t _6, size_t _7, size_t _8, size_t _9)
{
    _2 = medianIndex(r, _1, _2, _3);
    _8 = medianIndex(r, _7, _8, _9);
    if (r[_2] > r[_8]) tswap(_2, _8);
    if (r[_4] > r[_6]) tswap(_4, _6);
    // Here we know that r[_2] and r[_8] are the other two medians and that
    // r[_2] <= r[_8]. We also know that r[_4] <= r[_6]
    if (r[_5] < r[_4])
    {
        // r[_4] is the median of r[_4], r[_5], r[_6]
    }
    else if (r[_5] > r[_6])
    {
        // r[_6] is the median of r[_4], r[_5], r[_6]
        _4 = _6;
    }
    else
    {
        // Here we know r[_5] is the median of r[_4], r[_5], r[_6]
        if (r[_5] < r[_2]) 
			{cswap(r[_5], r[_2]);
			 return;
			}
        if (r[_5] > r[_8]) 
			{cswap(r[_5], r[_8]);
			 return;
			}
        // This is the only path that returns with no swap
        return;
    }
    // Here we know r[_4] is the median of r[_4], r[_5], r[_6]
    if (r[_4] < r[_2]) _4 = _2;
    else if (r[_4] > r[_8]) _4 = _8;
    cswap(r[_5], r[_4]);
}

/**
Median of minima
*/
static inline size_t medianOfMinima(elem_type_median *const r, const size_t n, const size_t length,int intro_cnt)
{
    assert(length >= 2);
    assert(n * 4 <= length);
    assert(n > 0);
    const size_t subset = n * 2,
        computeMinOver = (length - subset) / subset;
    assert(computeMinOver > 0);
    for (size_t i = 0, j = subset; i < subset; ++i)
    {
        const size_t limit = j + computeMinOver;
        size_t minIndex = j;
        while (++j < limit)
            if (r[j] < r[minIndex])
                minIndex = j;
        if (r[minIndex] < r[i])
            cswap(r[i], r[minIndex]);
        assert(j < length || i + 1 == subset);
    }
    select_driver(r, n, subset,intro_cnt);
    return expandPartition(r, 0, n, subset, length);
}

/**
Median of maxima
*/
static inline size_t medianOfMaxima(elem_type_median *const r, const size_t n, const size_t length,int intro_cnt)
{
    assert(length >= 2);
    assert(n * 4 >= length * 3 && n < length);
    const size_t subset = (length - n) * 2,
        subsetStart = length - subset,
        computeMaxOver = subsetStart / subset;
    assert(computeMaxOver > 0);
    for (size_t i = subsetStart, j = i - subset * computeMaxOver; i < length; ++i)
    	{
         const size_t limit = j + computeMaxOver;
         size_t maxIndex = j;
         while (++j < limit)
            if (r[j] > r[maxIndex])
                maxIndex = j;
         if (r[maxIndex] > r[i])
            cswap(r[i], r[maxIndex]);
         assert(j != 0 || i + 1 == length);
    	}
    select_driver(r + subsetStart, length - n, subset,intro_cnt); 
    return expandPartition(r, subsetStart, n, length, length);
}

/**
Partitions r[0 .. length] using a pivot of its own choosing. Attempts to pick a
pivot that approximates the median. Returns the position of the pivot.
*/
static inline size_t medianOfNinthers(elem_type_median *const r, const size_t length,int intro_cnt)
{
    assert(length >= 12);
    size_t frac;
	if(intro_cnt>=INTROMEDIAN_LIMIT)
		{ 
         frac =(length)/9; // no sampling [ /9 is smallest possible divisor see calculation of gap below  - this is because the ninther algorithm works on blocks of 9 ]
    	}
    else
	   {	
        /* algorithm is linear for any value of 0< f <=1 but it impacts speed of execution. frac = length*f [multiplication is key for proving o(n) ] */
	    /* steps below are adaquate for 32 bit addresses, with 64 bit addresses its likley another "step" in the calculation of frac will be helpful to minimise the runtime */
		if(length<=1024) frac=(length)/12; // 2^10 f=1/12 
		else if(length<=128*1024) frac=(length)/64; // 128*1024=2^17 => f=1/64 - use power of 2 for efficient divide
		else if(length<=64*1024*1024 ) frac=(length)/1024; // 64*1024*1024 =2^26 f=1/1024 - use power of 2 for efficient divide
		else frac= (length)/(64*1024);// f=1/(64*1024)=1/65536 - use power of 2 for efficient divide
	   }
     
    size_t pivot = frac / 2; // (frac-1)/2 is significantly slower here (eg 5.229 secs vs 4.886 secs) - but note frac is length/z so this is different to a normal pivot calculation 
    const size_t lo = (length) / 2 - pivot, hi = lo + frac; // (length-1)/2 makes odd slightly faster, but even a little slower
    assert(lo >= frac * 4);
    assert(length - hi >= frac * 4);
    assert(lo / 2 >= pivot);
    const size_t gap = (length - 9 * frac) / 4;
    size_t a = lo - 4 * frac - gap, b = hi + gap;
    for (size_t i = lo; i < hi; ++i, a += 3, b += 3)
    	{
         ninther(r, a, i - frac, b, a + 1, i, b + 1, a + 2, i + frac, b + 2);
    	}

    select_driver(r + lo, pivot, frac,intro_cnt);
    return expandPartition(r, lo, lo + pivot, hi, length);
}

/**

driver for medianOfNinthers, medianOfMinima, and medianOfMaxima.
Dispathes to each depending on the relationship between n (the sought order statistics) and length.
n=0 "returns" the min, n=length-1 "returns" the max.
In general it places the nth element of the array r (which has length elements) in the position it would be if the array was actually sorted.
The algoritms also partitions the array r so that all elements to the left of the nth element are <= to those on its right
It does this in linear time o(length) by using the principle of introselect - see "Introspective sorting and selection algorithms" by D.R.Musser,Software practice and experience, 8:983-993, 1997.
Note this function is called recursively, from  medianOfNinthers(), medianOfMaxima() and medianOfMinima() so there is some extra RAM (stack) usage of O(log2(length)) (with a small multiplier) .

The first, second, one before last and last elements are special optimised cases, and for short lengths a sort is used as thats more efficient than continueing with the main algorithm. 
*/

void select_driver(elem_type_median* r, size_t n, size_t length,int intro_cnt)
{
  // intro_cnt has to be passed as a function argument as this code is called recursively. If must be set to 0 on the first call 
  size_t lastlen=0;
  assert(n < length);
  while(1) /* loop till done */
       {
        // Decide strategy for partitioning
		/* treat looking for min as special case */
	    if (n == 0)
	        {
             size_t imin = 0;// v is min value
             elem_type_median v=r[0];
             // printf("select_driver - min, length=%.0f\n",(double)n);
             for (size_t i=1; i < length; ++i)
                if (r[i] < v ) 
					{imin = i;
					 v=r[i];
					}
             cswap(r[0],r[imin]); 
             return;// all done
	        }
	  
	    /* treat looking for 2nd smallest min as special case */
	    if (n == 1 && length>2)
           {elem_type_median xmin=r[0],xmin1=r[1];// xin/imin = min; xmin1/imin1=2nd lowest
            size_t imin,imin1;
            if(xmin1<xmin)
            	{xmin=r[1]; // swap as x[1] is lowest
            	 xmin1=r[0];
            	 imin=1;
            	 imin1=0;
            	}
             else
             	{imin=0;
             	 imin1=1;
             	}
			for (size_t i=2; i < length; ++i)
				{
                 if (r[i] < xmin1 ) 
					{if(r[i]<xmin)
						{// new smallest
						 xmin1=xmin;// old smallest is now 2nd smallest
						 imin1=imin;
						 xmin=r[i]; // new smallest
						 imin=i;
						}
					 else
					 	{// new 2nd smallest
					 	 xmin1=r[i];
					 	 imin1=i;
					 	}
					}
				}
		    // printf("select_driver - min2, length=%.0f imin=%.0f xmin=%.0f imin1=%.0f xmin1=%.0f\n",(double)n,(double)imin,(double)xmin,(double)imin1,(double)xmin1);	
			/* put min at x[0] and min1 at x[1] - as we have to use swaps watch out for special cases ! */
			if(imin1==0)
				{cswap(r[0],r[1]);
				 if(imin!=1) cswap(r[0],r[imin]);
				}	
			 else
			 	{cswap(r[0],r[imin]);// partition guarantees items to the left are lower, so ensure this 		
                 cswap(r[1],r[imin1]); 
			 	}
        
            return;// all done
           }
	
	    /* treat looking for max as special case */
	    if (n + 1 == length)
	        {
             size_t imax = 0;
             elem_type_median v=r[0]; // value at imax;
             // printf("select_driver - max, length=%.0f\n",(double)n);
             for (size_t i = 1; i < length; ++i)
                if ( r[i] > v) 
					{imax = i;
					 v=r[i];
					}
             cswap(r[imax],r[n]); 
             return;// all done
	        }
	    
	    /* treat looking for 2nd highest as special case */
	    if (n + 2 == length && length>2)
           {elem_type_median xmax=r[0],xmax1=r[1];// xmax/imax = max; xmax1/imax1=2nd highest
            size_t imax,imax1;
            if(xmax1>xmax)
            	{xmax=r[1]; // swap as x[1] is highest
            	 xmax1=r[0];
            	 imax=1;
            	 imax1=0;
            	}
             else
             	{imax=0;
             	 imax1=1;
             	}
            
            for (size_t i=2; i < length; ++i)
            	{
                 if (r[i] > xmax1 ) 
					{if(r[i]>xmax)
						{// new largest
						 xmax1=xmax;// old largest is now 2nd largest
						 imax1=imax;
						 xmax=r[i]; // new largest
						 imax=i;
						}
					 else
					 	{// new 2nd largest
					 	 xmax1=r[i];
					 	 imax1=i;
					 	}
					}
				}
			// printf("select_driver - max2,  length=%.0f imax=%.0f xmax=%.0f imax1=%.0f xmax1=%.0f\n",(double)n,(double)imax,(double)xmax,(double)imax1,(double)xmax1);
			// put max and max1 in their correct place in the array (max @ far end, max 1 just to its left )
			if(imax1==n+1)
				{cswap(r[n+1],r[n]);
				 if(imax!=n) cswap(r[n+1],r[imax]);
				}
			 else
			 	{
				 cswap(r[n+1],r[imax]); // items higher must be to the right		
            	 cswap(r[n],r[imax1]); 
            	}
            return;// all done
           }        
   		if(length<=16) // for small lengths its faster to use sorting . Sorting does a little more work that strictly necessary but thats typically only 1 extra compare/swap which is trivial overall.
			{
			 small_sort(r,length);
			 return ; // all done
			}
   		if(intro_cnt<INTROMEDIAN_LIMIT)
			{if( lastlen!=0 && length >= (lastlen-lastlen/INTROMEDIAN_DIV)  )
				++intro_cnt; // reduction in n is not enough for specified geometric progression
		 	 else if(intro_cnt > -INTROMEDIAN_LIMIT) 
		 		--intro_cnt; // we are within specified geometric progression limits (if() test stops it becoming a very negative number which would then be slow to respond to a "bad patch"		
   		 	 lastlen=length;
   			}
        assert(n < length);
        size_t pivot;
		 /* code that avoids overflows with big n */
        if (n <= length/6)
            pivot = medianOfMinima(r, n, length,intro_cnt);
        else if (n  >= length-length/6)
            pivot = medianOfMaxima(r, n, length,intro_cnt);
        else
        	{    			 
             pivot = medianOfNinthers(r, length,intro_cnt);
			}
        // See how the pivot fares
        if (pivot == n)
        	{
             return;
        	}
        if (pivot > n)
        	{
             length = pivot;
        	}
        else
        	{
             ++pivot;
             r += pivot;
             length -= pivot;
             n -= pivot;
        	}
       }
}

elem_type_median yaMedian(elem_type_median *a, size_t s) // return median of array a using above code 
{
 select_driver(a,(s-1)/2,s,0);// 0 for normal use ; INTROMEDIAN_LIMIT to always use non-sampling method
 return a[(s-1)/2];
}

void yaselect(elem_type_median* r, size_t n, size_t length) // select or nth_element . Places the n th element of a sequence in the position that it would be if the array was sorted. Changes array r. 
{
 select_driver(r, n, length,0); // select or nth_element . Places the n th element of a sequence in the position that it would be if the array was sorted. Changes array r. intro_cnt should be 0 on call.
}

/*   
   This sort uses yaMedian() to create a simple and reasonably efficient quicksort with guaranteed  O(n*log(n)) execution time as the pivot value is always the median.
   Code by Peter Miller.
   This is ~ 50% slower than yasort on average
   Note yaMedian uses introselect functionality so its worse case is abut 2* its average speed. 
*/
void ya_msort(elem_type_median *a,size_t n)
{
 elem_type_median m;
 size_t p;
 while(1)
 	{
	 if(n<2) return;
	 if(n<=32)
	 	{small_sort(a,n); // this is faster than continueing the main algorithm all the way down to very small partitions
	 	 return;
	 	}
	 // next try an insertion sort, abort this if its taking too much effort
	 // this efficiently sorts arrays that are almost perfectly sorted already.
	 // using this gives an average 33% speedup using the test program when MAX_INS_MOVES=2
	 // There is also  a potentially useful side effect of this, if we have to move to using quicksort then 2 values will have been moved which could help break up bad patterns of data.
#define MAX_INS_MOVES 2 /* max allowed number of moves - for the test program 2 is the optimum value */
   	 size_t nos_ins_moves=0;	 
	 for (elem_type_median  *pm =  &(a[1]); pm<&(a[n]); ++pm)
		{
		 elem_type_median  *q = pm;
		 elem_type_median *q_1 = pm-1;
		 // Compare first so we can avoid 2 moves for an element already positioned correctly.
         if (*q< *q_1) 
		 	{if(++nos_ins_moves>MAX_INS_MOVES) goto do_qsort; // too many moves - swap to qsort
           	 elem_type_median t = *q;
           	 do 
			 	{ q--;
				}
         	 	while (q != a && t < *--q_1); 
			 memmove(q+1,q,(size_t)(pm-q)*sizeof(elem_type_median));// move a portion of array x right by 1 to make space for t
           	 *q = t; // insert t in its correct place in array x
        	}     	
        }	
	 assert(check_sort( a, n) ); // check result of sort is ordered correctly
   	 return ; // all done
do_qsort: 	 	
	 m=yaMedian(a,n);// get median, as a "side effect" also puts values <= the median to the left of the array and values >= the meadian on the right. 
	 p=(n-1)/2; // position of median
#ifdef DEBUG 
	 // check that elements in a before median are <= median
	 for(size_t i=0;i<p;++i)
	 	if(a[i]>m) printf("msort(a,%.0f) a[%.0f]=%.0f expected <= median(%.0f)\n",(double)n,(double)i,(double)a[i],(double)m);
	 // check that elements in a after median are >= median
	 for(size_t i=p;i<n;++i)
	 	if(a[i]<m) printf("msort(a,%.0f) a[%.0f]=%.0f expected >= median(%.0f)\n",(double)n,(double)i,(double)a[i],(double)m); 	
#endif
	 while(p>1 && a[p-1]==m) p--; // scan down until we find a value thats != median (values that are = to median are already sorted)
	 ya_msort(a,p); // recursion - but depth max log2(n) as we split input at true median 
	 p=(n-1)/2;// back to known position of median
	 while(p<n-2 && a[p+1]==m) p++;// scan up until we find a value thats != median (values that are = to median are already sorted)
	 a+=p;// ready for next iteration; removes tail recursion - ya_msort(a+p,n-p)
	 n-=p; 
	}	
}

