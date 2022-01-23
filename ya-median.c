/* ya-median.c
   ===============
   This function returns the median of an array of numbers without changing the array.
   If the order of items in the input array can be changed then yaMedian() (in ya-select.c) is approx 35% faster overall with my test program.
   Type type of data used is defined in yamedian.h (elem_type_median).
   
   Where useful and possible it uses malloc to allocate an array with size up to the size of the input array.
   When this fails it uses a slower approach, but it keeps trying to malloc smaller amounts of space.
   No free space is required to calculate the median (apart from some stack space for local variables), but the more space that can be obtained from malloc the faster the result is obtained.
   
   The algorithm first does a pass of the data to find the min, max and to test for randomness
   1 If the data is not random then a pass of the basic Torben algorithm is done as this can frequently find the median in 1 pass with no extra memory.
   2 If the data is random the if malloc suceeds its used to take a copy of the input array and a fast select algorithm is used.
   3 If the above fails then 2 iterations of the Torben algorithm are executed in an "unrolled" form in one pass of the input data. This may find the median with no extra memory.
   	   This code uses a quadratic fit of the previous results to try and better estimate the median - this results in faster convergence.
   4 If malloc now suceeds a copy of a subset of the array is taken and select used
   5 loop back to step 3
    
 This software (and the underlying algorithm) was created by Peter Miller 2021, but it heavily leaverages work by others:
    Torben Algorithm by Torben Mogensen, the original (public doamin) implementation was by N. Devillard.
	yaselect() based on the paper "Fast Deterministic Selection" by Andrei Alexandrescu, 16th International Symposium on Experimental Algorithms (SEA 2017) 
	Check for randomness is test 66 in "Statistical Tests" by Gpoal Kanji. 
 
 The Torben algorithm uses the mid-range ( (max-min)/2 ) as the "pivot" value.
https://en.wikipedia.org/wiki/Mid-range shows that the mid-range is a reasonable estimate of the mean, for example for a continuous uniform distribution the mid-range is the best estimator 
of the mean, and for normal and laplace distributions it gives an unbiased estimate of the mean with a reasonably low variance.
Calculating the actual mean, in a way that cannot overflow, is too slow to be useful here. 
The mean is known to be within 1 standard deviation of the median - see Lemma 1 of "Fast Computation of the Median by Successive Binning", Ryan J. Tibshirani, October 2008.
The range rule - https://www.thoughtco.com/range-rule-for-standard-deviation-3126231 states that the range=max-min divided by 4 is approximately the standard deviation.
Thus using the mid-range as the pivot will "typically" place the pivot within range/4 of the median which would result in a typical o(n) run-time. 
There are known worse case patterns for this eg:
0,1,2,4,8,16,32,...
ie 0,k^0, k^1,k^2,k^3 where k>=2 
Here pivoting on the mid-range gives a new partition thats only 2 elements smaller, eg (32+0)/2=16 then (8+0)/2=4 then (2+0)/2=1
However, in practice this sequence is limited by the type elem_type_median , eg if this is 32 bit unsigned integers then 2^32 is the largest value and this is only 32 values.
Even using floating point values the exponent range is still very limited (a double only has 11 bits in its exponent so its limited to 2047).
If the sequence is repeated then the reduction every time becomes geometric and the Torben algorithm becomes O(n) run-time.
It is therefore postulated (but not proven) that with the Torben algorithm the worse case run-time is O(n).
When there is enough free memory for a complete copy of the input array then the execution time is guaranteed to be O(n).
 
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

// #define DEBUG /* uncomment to get debug printf's 
// you should not need to edit anything below here for normal use of this software (you may need to edit yamedian.h) 


#ifdef DEBUG
 #include <stdio.h>
#endif
#include <stddef.h> /* for size_t */
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "yamedian.h" /* defines elem_type_median etc */

#define USE_MALLOC   /* if defined (as it normally should be) use malloc whenever possible. It its not defined the correct answer will still be found, but it may take longer to execute */

elem_type_median ya_median(elem_type_median *m, const size_t n)
{
    elem_type_median  min, max;
    min = max = m[0] ; 
	 /* check for randomness is test 66 in "Statistical Tests" by Gpoal Kanji. */
	 elem_type_median lastt=m[0];
	 size_t nos_plus_signs=0;
     for (elem_type_median *p=m+1; p<m+n; p++) 
		{// first loop, just done once, to find min & max
    	 elem_type_median t=*p;
         if (t<min) min=t;
         if (t>max) max=t; // else is OK as if we have a new min it cannot also be a new max - but I seem to get faster execution speed without the else !
         if(t>lastt) nos_plus_signs++; // count of times sequence increases 
         lastt=t;
    	}
	 /* 1.64*sd is 90% confidence limit */
	 bool is_random=nos_plus_signs>=(n-1)/2-1.64*(n+1)/12 && nos_plus_signs<=(n-1)/2+1.64*(n+1)/12;
#ifdef DEBUG	 
     printf(" check randomness: nos_plus_signs=%llu for random expected %.0f to %.0f so is %s\n",nos_plus_signs,(n-1)/2-1.64*(n+1)/12, (n-1)/2+1.64*(n+1)/12, is_random?"random":"not random"); 
#endif    

     if(is_random)
     	{ /* try to use select as thats faster when the input is random - but we must take a copy of the input array as we must not change that here */
#ifdef USE_MALLOC     	
 		 elem_type_median *c,med; // c=copy of input array 
 		 c=malloc(sizeof(elem_type_median)*n);
 		 if(c!=NULL) // if space for a copy 
 		 	{
#ifdef DEBUG 		 		
			 printf("  Is random so using yaselect()\n");
#endif			 
			 memcpy(c,m,sizeof(elem_type_median)*n);
 			 yaselect(c,(n-1)/2,n);
			 med=c[(n-1)/2];// median
 			 free(c); 
 			 return med;
 			}
#endif			 	
		}  
	 else
	 	{/* else do initial optimised Torben pass - in many cases this will quickly find the median especially as "is_random" is false here */
	 	 size_t     less1, greater1, equal1;
    	 elem_type_median  guess1, maxltguess1, mingtguess1;
	 	 less1 = 0; greater1 = 0; equal1 = 0;		 	 
	 	 if(max==min) 
		 	return max; // only 1 value so no need to search any more
     	 // guess = (min+max)/2;// guess does not have to be a value that actually exists, min and max are always actual values
     	 guess1 = min/2+max/2; // (min+max)/2 could overflow, especially if elem_type_median was an integer type
#ifdef DEBUG         
	 	 printf("  Is not random, so using initial Torben pass: min=%.0f max=%.0f guess1=%.1f\n",min,max,guess1);
#endif		 
     	 maxltguess1 = min ;
     	 mingtguess1 = max ;
     	 /* don't count # equals, calculate it at the end as that might be a little faster */
     	 for (elem_type_median *p=m; p<m+n; p++) 
		 	{elem_type_median t=*p;
    	  	 if (t<guess1) 
		 		{
    	     	 less1++;
    	     	 if (t>maxltguess1) maxltguess1 = t;
    	   		} 
		  	 else if (t!=guess1) // ! < and != means > but != might be faster to calculate than > 
				{
             	 greater1++;
             	 if (t<mingtguess1) mingtguess1 = t;
           		}	 
         	}
     	 equal1=n-less1-greater1;            	
     	 if (less1 <= (n+1)/2 && greater1 <= (n+1)/2) 
	 		{// done just need to work out what to return
    	 	 if (less1 >= (n+1)/2) return maxltguess1;
    	 	 else if (less1+equal1 >= (n+1)/2) return guess1;
    	 	 else return mingtguess1; 
    		}
     	 else if (less1>greater1) max = maxltguess1 ;
     	 else min = mingtguess1;		
		}
  
     int itns=0;
	 elem_type_median  guess1=0,guess2=0,guess3=0;	 
	 size_t     less=1,greater=1; // 1 as we initialise min=max=m[0] and then iterate from m[1]
     size_t     less1=0, greater1, equal1;
     elem_type_median  maxltguess1, mingtguess1;		 
     size_t     less2=0, greater2, equal2;
     elem_type_median  maxltguess2, mingtguess2;    
     size_t     less3=0, greater3, equal3;
     elem_type_median  maxltguess3, mingtguess3; 
     elem_type_median  last_guess1,last_guess2,last_guess3;	 	 
     while (1) 
		{/* do a "binary search" to find median - we actually "unroll" the main loop so two iterations are done in parallel.
		    This could take  0.5*log2(n) iterations of this while loop 
		    Uses the results from the previous iteration fitted to a quadratic to better estimate the median - this speeds up its convergence
		 */	 
		 // greater values are calculated from less, equal outside of loop	 
		 ++itns;// count loops done		 
		 if(max==min) return max; // only 1 value so no need to search any more
         // guess = (min+max)/2;// guess does not have to be a value that actually exists, min and max are always actual values
		 last_guess1=guess1; last_guess2=guess2; last_guess3=guess3;
         guess1 = (min/4)*3+(max/4); 
         guess2 = min/2+max/2; // (min+max)/2 could overflow as could min+(max-min)/2 especially if elem_type_median was a signed integer type
         guess3 = (min/4)+(max/4)*3; 
		// if using integer maths then make estimates more accurate
		if( ((elem_type_median)1)/2 == 0)
			{// if we are using integer maths ; assume optimiser will optimise this out conmpletely if using floating point maths
			 // printf("Int maths ");
			 if( ((uint32_t)min&1) & ((uint32_t)max&1) ) // (uint32_t) is OK here as we are only looking at lsb so size of type does not really matter [ but without cast will not compile if elem_type_median is double]
			 	{guess2++;// eg min=1 max=5 1/2+5/2 =0+2 = 2 but both min & max are odd so final value is 3 
			 	}
			 if(guess1<=min) guess1=min+1;// adjust guess1 & guess 3 as well - we know we have integer maths so +1 is OK
			 if(guess3<=guess2) guess3=guess2+1;			 	
			}		          

	     if(itns>1)
	     	{
	     	 /* estimate median from results of previous pass - quadratic estimation */
	  	 	 /* quadratic is y=a*x^2+b*x+c where y is the guessed median and x is the matching count */
	  	 	 /* this is about 10% faster than linear estimation with the test program */
			 long double a,b,c; // straight line fit - need good precision here as potentially subtracting big numbers that are very close
			 elem_type_median best_guess;
			 less1+=equal1; // count <=last_guess1
			 less2+=equal2;
			 less3+=equal3; // count <=last_guess3
			 if(less1<less2 && less2<less3 && last_guess1<last_guess2 && last_guess2<last_guess3) // if there were results from a previous pass that allow a,b,c to be calculated
			 	{a=((less3-less1)*(last_guess3-last_guess2)-(less3-less2)*(last_guess3-last_guess1)) /
				   ( ((long double)less3*(long double)less3-(long double)less2*(long double)less2)*(less3-less1) - ((long double)less3*(long double)less3-(long double)less1*(long double)less1)*(less3-less2)  );
				 b=(last_guess3-last_guess1-a*((long double)less3*(long double)less3-(long double)less1*(long double)less1))/(less3-less1) ;
				 c=last_guess3-a*(long double)less3*(long double)less3-b*less3;
	
				 less2=(n+1)/2;// make sure calculated as an unsigned integer
				 best_guess=a*(long double)less2*(long double)less2+b*(long double)less2+c; // want guess for (n+1)/2 = median location
	#ifdef DEBUG 			 
				 printf("  less1=%1.f last_guess1=%.1f less3=%.1f last_guess3=%.1f a=%g b=%g c=%g best_guess=%.1f min=%.1f max=%.1f\n",(double)less1,(double)last_guess1,(double)less3,(double)last_guess3,(double)a,(double)b,(double)c,(double)best_guess,(double)min,(double)max); 
	#endif			 
				 if(best_guess>min && best_guess<max)
				 	{if(best_guess<guess2) 
					 	{
	#ifdef DEBUG 				 		
					 	 printf("   guess1(was %.1f) changed to best_guess (%.1f) min=%.1f guess1 (best)=%.1f guess2=%.1f guess3=%.1f max=%.1f\n",(double)guess1,(double)best_guess,(double)min,(double)best_guess,(double)guess2,(double)guess3,(double)max);
	#endif				 	 
					 	 guess1=best_guess;
					 	}
				 	 else if(best_guess>guess2) 
					  	{
	#ifdef DEBUG 				  		
						 printf("  guess3(was %.1f) changed to best_guess (%.1f) min=%.1f guess1=%.1f guess2=%.1f guess3(best)=%.1f max=%.1f\n",(double)guess3,(double)best_guess,(double)min,(double)guess1,(double)guess2,(double)best_guess,(double)max);				  		
	#endif					 
						 guess3=best_guess;
					  	}
				 	}
			 	}   
			}	
 		 less=0; // <=min
		 less1 = 0 ; equal1 = 0;	
		 less2 = 0;  equal2 = 0;	
		 less3 = 0; equal3 = 0;	
         if(guess1<min) guess1=min; // if we are using integer maths the above may not generate the values we expect, so fix that here (this should never be needed for float or double)
         if(guess1>max) guess1=max;        
         if(guess2<guess1) guess2=guess1;// it is OK for guess1==guess2 or gues2==guess3 (or them all to be the same) 
         if(guess2>max) guess2=max;
         if(guess3<guess2) guess3=guess2;
         if(guess3>max) guess3=max;   
		 // here we have min<=guess1<=guess2<=guess3<=max [ we know min!=max as thats trapped above ]      
#ifdef DEBUG         
		 printf("  min=%.0f max=%.0f guess1=%.1f guess2=%.1f guess3=%.1f\n",(double)min,(double)max,(double)guess1,(double)guess2,(double)guess3);
#endif		 
         maxltguess1=maxltguess2=maxltguess3 = min ;
         mingtguess1=mingtguess2=mingtguess3 = max ;
         for (elem_type_median *p=m; p<m+n; p++) 
		 	{elem_type_median t=*p; // *** code below optimised based on knowledge that min<guess1<guess2<guess3<max . We also calculate greater1,2,3 but this is also done outside this loop
		 	 if(t<guess2)
		 	 	{ // guess 2 is the middle on so this approximately splits the array m in 1/2
		 	 	 if(t<=min)
		 	 	 	{less++; // just count values <=min - maxltguess & mingtguess are initialised to min/max so don't need to worry about these here
		 	 	 	 continue;
		 	 	 	}
                 less2++;
                 if (t>maxltguess2) maxltguess2 = t;		 	 	 	
                 less3++;
                 if (t>maxltguess3) maxltguess3 = t;
                 
				 if (t<guess1) 
			 		{
                 	 less1++;
                 	 if (t>maxltguess1) maxltguess1 = t;
                 	 continue;
            		} 
			 	 else if (t!=guess1) // here != means > 
					{
                 	 if (t<mingtguess1) mingtguess1 = t;
                 	 continue;
            		}	 
			 	 else equal1++;
			 	 continue;
			 	}
			 else if (t!=guess2) // t>guess2
				{
				 if(t>=max)
				 	{// no need to count >=max as these are calculated from < and =. maxltguess & mingtguess are initialised to min/max so don't need to worry about these here
				 	 continue;
				 	}
                 if (t<mingtguess2) mingtguess2 = t;
                 if (t<mingtguess1) mingtguess1 = t;   
             	 if (t<guess3) 
			 		{
                 	 less3++;
                 	 if (t>maxltguess3) maxltguess3 = t;
            		} 
			 	 else if (t!=guess3) // here != means > 
					{
                 	 if (t<mingtguess3) mingtguess3 = t;
            		} 
			 	 else equal3++;	
				 continue; // not realy needed but makes it clear					               
            	} 
			 else // t==guess2
			 	{equal2++;
			 	 if(t==guess3) equal3++; // for integers its possible guess2=guess3
				 else 
				 	{less3++;
			 	 	 if (t>maxltguess3) maxltguess3 = t;
			 		}
			 	 if(t==guess1) equal1++; // for integers its possible guess2=guess3	
			 	 else if (t<mingtguess1) mingtguess1 = t;
			 	 continue; // not realy needed but makes it clear
	 			}
        	}
    	 // now adjust values to get required numbers
    	 less1+=less; less2+=less; less3+=less;
    	 greater1=n-less1-equal1; // this should be more efficient than counting greater1,2,3
    	 greater2=n-less2-equal2;
    	 greater3=n-less3-equal3;
      	 
         if (less1 <= (n+1)/2 && greater1 <= (n+1)/2)
		 	{// done 1
#ifdef DEBUG
    		 printf(" Torben(%.0f) [1] took %d iterations\n",(double)n,itns);	
#endif
    		 if(max==min) return max;
    		 if (less1 >= (n+1)/2) return maxltguess1;
    		 else if (less1+equal1 >= (n+1)/2) return guess1;
    		 else return mingtguess1;		 	
			}
         else if (less1>greater1) 
		 	{if(max > maxltguess1) 
			 	{max = maxltguess1; // only narrow search range
			 	 greater=greater1;
			 	}
		 	}
         else 
		 	{if(min < mingtguess1) 
			 	{min = mingtguess1; // only narrow search range
			 	 less=less1;
			 	}
		 	}
         
         if (less2 <= (n+1)/2 && greater2 <= (n+1)/2)
		 	{// done 2
#ifdef DEBUG
    		 printf(" Torben(%.0f) [2] took %d iterations\n",(double)n,itns);	
#endif
    		 if(max==min) return max;
    		 if (less2 >= (n+1)/2) return maxltguess2;
    		 else if (less2+equal2 >= (n+1)/2) return guess2;
    		 else return mingtguess2;		 	
			}
         else if (less2>greater2) 
		 	{if(max > maxltguess2) 
			 	{max = maxltguess2; // only narrow search range
			 	 greater=greater2;
			 	}
		 	}
         else 
		 	{if(min < mingtguess2)
			 	{ min = mingtguess2; // only narrow search range
			 	 less=less2;
			 	}
		 	}
		 
         if (less3 <= (n+1)/2 && greater3 <= (n+1)/2)
		 	{// done 3
#ifdef DEBUG
    		 printf(" Torben(%.0f) [3] took %d iterations\n",(double)n,itns);	
#endif
    		 if(max==min) return max;
    		 if (less3 >= (n+1)/2) return maxltguess3;
    		 else if (less3+equal3 >= (n+1)/2) return guess3;
    		 else return mingtguess3;		 	
			}
         else if (less3>greater3) 
		 	{if(max > maxltguess3) 
			 	{max = maxltguess3; // only narrow search range
			 	 greater=greater3;
			 	}
		 	}
         else 
		 	{if(min < mingtguess3) 
			 	{min = mingtguess3; // only narrow search range
			 	 less=less3;
			 	}
		 	}	
#ifdef DEBUG		 	
		 printf("  at end of iteration %u: less=%u greater=%u so number inbetween=%u\n",itns,(unsigned)less,(unsigned)greater,(unsigned)(n-less-greater));	
#endif	

#ifdef USE_MALLOC  	  
		 if((n-less-greater)>1 )
		 	{ // (n-less-greater) elements left. just collect these elements into an array and use select() on them rather than doing further iterations.	
		 	  // each iteration reduces the number of elements left as min and max get closer to the actual median, so we expect malloc to eventually suceed.
		 	 size_t buf_cnt=0,cnt_lt_min=0;
			 elem_type_median *buf,med; // will allocate via malloc()
			 size_t buf_size=(n-less-greater)+10; // allow a bit more space, just in case
			 buf=malloc(sizeof(elem_type_median)*buf_size); // allocate space needed
			 if(buf==NULL) continue; // not enought free ram - keep using Torben algorithm, on the next iteration the space needed will be less so can try again then.		 

			 for (elem_type_median *p=m; p<m+n; p++) 
			 	{elem_type_median t=*p;
			 	 if(t<min) cnt_lt_min++;
			 	 else if(t<=max && buf_cnt<buf_size)
			 	 	{buf[buf_cnt++]=t;
			 	 	}
			 	}
			 if(buf_cnt>=buf_size)
			 	{
#ifdef DEBUG			 		
				 printf("  *** OOPS buf too small! \n");
#endif				 
				 free(buf);// need to free up space for buffer
			 	 continue; // continue doing Torben, this should be very infrequent (never?) so don't need to worry about the extra time the wasted loop above took
				}
#ifdef DEBUG				
			 printf("  Using yaselect(%u)\n",(unsigned)buf_cnt);	
#endif			 
			 yaselect(buf,(n+1)/2-cnt_lt_min -1,buf_cnt);
			 med=buf[(n+1)/2-cnt_lt_min -1];	
			 free(buf);// need to free up space for buffer
			 return med;		 
			}
#endif			
    	}
 // not reached	 	
}
