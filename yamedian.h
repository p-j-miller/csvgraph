/* yamedian.h */
#ifndef __YAMEDIAN_H
 #define __YAMEDIAN_H
 #ifdef elem_type_median 
  #error "attempt to redefine elem_type_median"  
 #endif
 
 /* the (single) #define below is the ONLY line you should edit in this header file! */
 #define elem_type_median float /* type of array to process - should be an standard type for which compares are defined eg int,unsigned, float, double etc */

 #ifdef __cplusplus
  extern "C" {
 #endif  
 void yaselect(elem_type_median* r, size_t n, size_t length); // select or nth_element . Places the n th element of a sequence in the position that it would be if the array was sorted. Changes array r. 
 elem_type_median yaMedian(elem_type_median *a, size_t s); // return median of array a using above code. As a side effect this changes array a. This is faster than ya_median().
 elem_type_median ya_median(elem_type_median *m, const size_t n);// calculates median without changing array m. Uses free memory (via malloc) if available to speed process up - but will work with no free memory. This is slower than yaMedian().
 void ya_msort(elem_type_median *a,size_t n); // sort based on yaMedian. Guaranteed O(n*log(n)) execution speed but ~ 50% slower than yasort() on test program 
 #ifdef __cplusplus
    }
 #endif 
#endif
