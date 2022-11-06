/* count_lines.c - quickly count the number of lines in a file */
/* returns the number of lines in filename. File is open, read , then closed. */
/* the fastest version treats \n as a line terminator whereas windows uses \r\n - in almost all cases this gives the same answer */
/*----------------------------------------------------------------------------
 * Copyright (c) 2022 Peter Miller
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
#include <stdio.h>
#include <stdlib.h>
#include "count_lines.h"
#include "rprintf.h"

// #define DEBUG

/* count the number of lines in a file - quickly...  */
#define CL_BLOCK_SIZE (1024*1024) /* 1M is a reasonable choice (1024*1024)  */
/* in Borland builder 8192 takes 10.141 secs, 1024*1024 takes 8.562 secs (my readline() takes  9.282) */
/* using _rtl_xxx functions takes 8.547 secs with 1M buffer */
/* using windows functions takes 8.437 sec with 1M buffer and is disk i/o limited */
/* 1M is optimal (for my PC) with windows functions 2M is slightly slower (8.532 secs), 512K is 8.672 secs */

#if 1 /* use built in windows functions directly to read file, check for \n's by reading array 64bits at a time */
	/* \n detection from idea in http://graphics.stanford.edu/~seander/bithacks.html#ZeroInWord */
#include "windows.h"
#include <stdint.h> /* for uint64_t, UINT64_C etc */

#ifdef _WIN32 /* was 64 */
#define STR_CONV_BUF_SIZE 2000 // the largest string you may have to convert. depends on your project
static wchar_t* __fastcall UnicodeOf(const char* c)
{
	static wchar_t w[STR_CONV_BUF_SIZE];
	memset(w,0,sizeof(w));
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, c, (int)strlen(c), w, STR_CONV_BUF_SIZE);
	return(w);
}
#endif
/* use built in windows functions directly to read file, check for \n's by reading array 64bits at a time */
	/* \n detection from idea in http://graphics.stanford.edu/~seander/bithacks.html#ZeroInWord */

size_t count_lines(char *cfilename)
{   HANDLE hFile;
	size_t lines=0;
	char *buf=(char *)malloc(CL_BLOCK_SIZE); // buffer for reads - malloc guarantees sensible alignment for buf;
	char *cp;
	DWORD nBytesRead = 0;
	BOOL bResult   = FALSE;
#ifdef DEBUG
	crprintf("count_lines: Filename=%s BLOCK_SIZE=%u\n",cfilename,CL_BLOCK_SIZE);
#endif
	if(buf==NULL)
		{crprintf(" count_lines() - not enough RAM - sorry!\n");
		 return 0; // No RAM
		}
	/*
	HANDLE CreateFileA(
	LPCSTR                lpFileName,
	DWORD                 dwDesiredAccess,
	DWORD                 dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD                 dwCreationDisposition,
	DWORD                 dwFlagsAndAttributes,
	HANDLE                hTemplateFile
	);
	*/
#ifdef _WIN32 /* was 64 */
	hFile = CreateFile(UnicodeOf(cfilename),               // file to open
#else
	hFile = CreateFile(cfilename,               // file to open
#endif
					   GENERIC_READ,          // open for reading
					   FILE_SHARE_READ,       // share for reading
					   NULL,                  // default security
					   OPEN_EXISTING,         // existing file only
					   FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, // normal file  , will be read sequentially
					   NULL);                 // no attr. template

	if (hFile == INVALID_HANDLE_VALUE)
		{free(buf);
		 crprintf(" count_lines(%s) - cannot open file\n");
		 return 0; // cannot open file
		}
	while ( (bResult = ReadFile(hFile, buf, CL_BLOCK_SIZE, &nBytesRead, NULL)))    /* bResult is false only on error */
		{
		 // Check for eof.
		 if (bResult &&  nBytesRead == 0)
			{
			 // at the end of the file
			 break;
			}
		 uint64_t v64;
		 uint64_t *pv64;
	/* # pragma's below work for gcc and clang compilers. This code is done for efficiency, buf is obtained from heap so has suitable alignment */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
		 for(pv64=(uint64_t *)buf; nBytesRead>=8;nBytesRead-=8)
			{// read buffer 8 bytes at a time looking for \n = 0a*/
#pragma GCC diagnostic pop
			 v64=*pv64++; // read next 8 characters from buffer
			 v64^= UINT64_C(0x0a0a0a0a0a0a0a0a); // convert \n's to zero's as test below is for zero bytes
			 if ((v64 - UINT64_C(0x0101010101010101)) & (~v64) & UINT64_C(0x8080808080808080))
				{// at least 1 byte is a \n, count them, there is 1 bit set in data for every \n character in the 8 bytes
                 v64=(v64 - UINT64_C(0x0101010101010101)) & (~v64) & UINT64_C(0x8080808080808080);
				 ++lines; // at least 1 \n found
				 while((v64=(v64&(v64-1)))) ++lines; // (x&(x-1)) removes a single bit set from data, so this counts the remaining \n's
				}
			}
		 cp=(char *)pv64;// might be some bytes left to process, do them here 1 character at a time
		 while(nBytesRead--)
			if(*cp++=='\n') ++lines;
		}
	CloseHandle(hFile);
	free(buf);
#ifdef DEBUG
	crprintf("  %.0f lines found\n",(double)lines);
#endif
	return lines;
}

#elif 1 /* use built in windows functions directly */
#include "Windows.h"
size_t count_lines(char *filename)
{   HANDLE hFile;
	size_t lines=0;
	char *buf=malloc(BLOCK_SIZE); // buffer for reads - malloc guarantees sensible alignment for buf;
	char *cp;
	DWORD nBytesRead = 0;
	BOOL bResult   = FALSE;
	//crprintf("count_lines: Filename=%s BLOCK_SIZE=%u\n",filename,BLOCK_SIZE);
	/*
	HANDLE CreateFileA(
	LPCSTR                lpFileName,
	DWORD                 dwDesiredAccess,
	DWORD                 dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD                 dwCreationDisposition,
	DWORD                 dwFlagsAndAttributes,
	HANDLE                hTemplateFile
	);
	*/
	hFile = CreateFile(filename,               // file to open
                       GENERIC_READ,          // open for reading
                       FILE_SHARE_READ,       // share for reading
                       NULL,                  // default security
					   OPEN_EXISTING,         // existing file only
					   FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, // normal file  , will be read sequentially
                       NULL);                 // no attr. template

    if (hFile == INVALID_HANDLE_VALUE)
		{free(buf);
		 return 0; // cannot open file
		}
	while ( (bResult = ReadFile(hFile, buf, BLOCK_SIZE, &nBytesRead, NULL)))    /* bResult is false only on error */
		{
		 // Check for eof.
		 if (bResult &&  nBytesRead == 0)
			{
			 // at the end of the file
			 break;
			}
		 cp=buf;
		 while(nBytesRead--)
			if(*cp++=='\n') ++lines;
		}
	CloseHandle(hFile);
	free(buf);
	// crprintf("  %u lines found\n",lines);
	return lines;
}
#elif 1     /* use borland _rtl_xxx functions */
#include <fcntl.h> /* O_RDONLY|O_BINARY  */
#include <io.h> /* read(),  open(), close() */
size_t count_lines(char *filename)
{
	int fd;
	int n;
	size_t lines=0;
	char *buf=malloc(BLOCK_SIZE); // buffer for reads - malloc guarantees sensible alignment for buf;
	char *cp;
	if(buf==NULL) return 0; // not enough free ram
	//crprintf("count_lines: Filename=%s BLOCK_SIZE=%u\n",filename,BLOCK_SIZE);
	fd=_rtl_open(filename,O_RDONLY|O_BINARY );
	if(fd==-1)
		{// crprintf("  Error: cannot open file [%s]\n",filename);
		 free(buf);
		 return 0;
		}
	while((n=_rtl_read(fd,buf,BLOCK_SIZE))>0)
		{cp=buf;
		 while(n--)
			if(*cp++=='\n') ++lines;
		}
	_rtl_close(fd);
	// crprintf("  %u lines found\n",lines);
	free(buf);
	return lines;
}
#else  /* use generic C low level i/o functions */
#include <fcntl.h> /* O_RDONLY|O_BINARY  */
#include <io.h> /* read(),  open(), close() */
size_t count_lines(char *filename)
{
	int fd;
	int n;
	size_t lines=0;
	static char buf[BLOCK_SIZE];
	char *cp;
	//crprintf("count_lines: Filename=%s BLOCK_SIZE=%u\n",filename,BLOCK_SIZE);
	fd=open(filename,O_RDONLY|O_BINARY ,0);
	if(fd==-1)
		{// crprintf("  Error: cannot open file [%s]\n",filename);
		 return 0;
		}
	while((n=read(fd,buf,BLOCK_SIZE))>0)
		{cp=buf;
		 while(n--)
			if(*cp++=='\n') ++lines;
		}
	close(fd);
	// crprintf("  %u lines found\n",lines);
	return lines;
}
#endif
