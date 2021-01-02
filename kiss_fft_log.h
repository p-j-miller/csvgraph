/*
 *  Copyright (c) 2003-2010, Mark Borgerding. All rights reserved.
 *  This file is part of KISS FFT - https://github.com/mborgerding/kissfft
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 *  See COPYING file for more information.
 */
 /* This file modified by Peter Miller 12/2020
	 #define ERROR -> FFT_ERROR as ERROR was alreday defined elsewhere in csv_graph
     changed from fprintf(stderr,... ) to rprintf() for debugging output so it is visible on windows.
 */

#ifndef kiss_fft_log_h
#define kiss_fft_log_h

#define FFT_ERROR 1
#define FFT_WARNING 2
#define FFT_INFO 3
#define FFT_DEBUG 4

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#if defined(NDEBUG)
# define KISS_FFT_LOG_MSG(severity, ...) ((void)0)
#else
# define KISS_FFT_LOG_MSG(severity, ...) \
	rprintf( "[" #severity "] " __FILE__ ":" TOSTRING(__LINE__) " "); \
	rprintf( __VA_ARGS__); \
	rprintf( "\n")
#endif

#define KISS_FFT_ERROR(...) KISS_FFT_LOG_MSG(ERROR, __VA_ARGS__)
#define KISS_FFT_WARNING(...) KISS_FFT_LOG_MSG(WARNING, __VA_ARGS__)
#define KISS_FFT_INFO(...) KISS_FFT_LOG_MSG(INFO, __VA_ARGS__)
#define KISS_FFT_DEBUG(...) KISS_FFT_LOG_MSG(DEBUG, __VA_ARGS__)



#endif /* kiss_fft_log_h */