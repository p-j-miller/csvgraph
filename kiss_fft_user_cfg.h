/* kiss_fft_user_cfg.h
 * ===================
 * This is a header file to allow a user to configure kiss_ftt
 *  
 * Created by Peter Miller 3/9/2022
 * 
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 *  See COPYING file for more information.
 */
 //#define USE_SIMD /* to use SIMD extensions - this needs other code changes to work transparently */
 // #define FIXED_POINT 32 /* define = 32 or 16 to use 32 or 16 bit fixed point maths */
 // #define kiss_fft_scalar float /* if not FIXED_POINT or SIMD then define floating point type required - default is "float" if neither FIXED_POINT or kiss_fft_scalar is defined */
 #define USE_WTHREADS /* if defined use windows threads to give a speedup on a multiprocessor system */


