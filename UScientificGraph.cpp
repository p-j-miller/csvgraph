//---------------------------------------------------------------------------
//  TScientificGraph -- an object for scientific plots
// This version by Peter Miller 18/7/2019 as csv file grapher
//
//  Loosely based on an original public domain version by  Frank heinrich, mail@frank-heinrich.de
// Major tidy up 1/9/2019 to remove unused functionality
// 6/2/2021 for 2v0 major change to use float *x_vals,*y_vals rather than SDataPoints in a TLIST.
//
/*----------------------------------------------------------------------------
 * Copyright (c) 2019,2022 Peter Miller
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
 * IN NO EVENT SHALL THE AUTHOR OR COPYRIGHT HOLDER BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *--------------------------------------------------------------------------*/
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include <vcl.h>
#include <math.h>
#include <stdio.h>
#include <values.h>
#include <time.h>
#pragma hdrstop

#include <exception>
#include "UScientificGraph.h"
#include "UScalesWindow.h"
#include "UDataPlotWindow.h"
#include "Unit1.h"
#include "About.h"
#define NoForm1   /* says Form1 is defined in another file */
#include "rprintf.h"
#include "expr-code.h"
#include <cmath>
#include "kiss_fftr.h" // for fft
#include "_kiss_fft_guts.h"
#include "yasort2.h" /* needs to be set so we sort float's ie include "#define elem_type_sort2 float" */
#include "yamedian.h" /* needs to be set to work on floats eg  #define elem_type_median float */
#include "interpolate.h"
#include "smooth_diff.h"
#include "smoothing_spline.h"

// #define USE_double_to_str_exp /* define to use double_to_str_exp() for y axis with max 12 chars, if not defined use gcvt() */

#ifdef USE_double_to_str_exp
 #include "float_to_str.h"
#endif
//---------------------------------------------------------------------------

#pragma package(smart_init)
//#define P_UNUSED(x) (void)x; /* a way to avoid warning unused parameter messages from the compiler */
#define DOUBLE float /* define as double to go back to original, but as points are stored as floats we can use floats in several places */

// I'm sorry for the next 2 lines, but otherwise I get a lot of warnings "zero as null pointer constant"
#undef NULL
#define NULL (nullptr)
#define STR_CONV_BUF_SIZE 2000 // the largest string you may have to convert. depends on your project
static wchar_t* __fastcall Utf8_to_w(const char* c)     // convert utf encoded string to wide chars - new function
{
	static wchar_t w[STR_CONV_BUF_SIZE];
	memset(w,0,sizeof(w));
	MultiByteToWideChar(CP_UTF8, 0, c, -1, w, STR_CONV_BUF_SIZE-1);  // utf-8 (this is a windows function )
	return(w);
}

// Below from  https://stackoverflow.com/questions/1031645/how-to-detect-utf-8-in-plain-c
// modified Peter Miller 29-5-2024
// returns true when not a pure ascii string , but is a valid utf8 string
static bool is_utf8(const char * string);

static bool is_utf8(const char * string)
{bool not_ascii=false;
	if(!string)
		return 0;
	const unsigned char * bytes = (const unsigned char *)string;   // in case char is signed
	while(*bytes)
	{
		if( (// ASCII - only allow values that may reasonably be in a utf8 string
				bytes[0] == 0x09 ||
				bytes[0] == 0x0A ||
				bytes[0] == 0x0D ||
				(0x20 <= bytes[0] && bytes[0] <= 0x7E)
			)
		) {
			bytes ++;
			continue;
		  }

		if( (// non-overlong 2-byte
				(0xC2 <= bytes[0] && bytes[0] <= 0xDF) &&
				(0x80 <= bytes[1] && bytes[1] <= 0xBF)
			)
		) { not_ascii=true;
			bytes += 2;
			continue;
		  }

		if( (// excluding overlongs
				bytes[0] == 0xE0 &&
				(0xA0 <= bytes[1] && bytes[1] <= 0xBF) &&
				(0x80 <= bytes[2] && bytes[2] <= 0xBF)
			) ||
			(// straight 3-byte
				((0xE1 <= bytes[0] && bytes[0] <= 0xEC) ||
					bytes[0] == 0xEE ||
					bytes[0] == 0xEF) &&
				(0x80 <= bytes[1] && bytes[1] <= 0xBF) &&
                (0x80 <= bytes[2] && bytes[2] <= 0xBF)
			) ||
			(// excluding surrogates
                bytes[0] == 0xED &&
                (0x80 <= bytes[1] && bytes[1] <= 0x9F) &&
				(0x80 <= bytes[2] && bytes[2] <= 0xBF)
			)
		) { not_ascii=true;
            bytes += 3;
			continue;
		  }

        if( (// planes 1-3
				bytes[0] == 0xF0 &&
                (0x90 <= bytes[1] && bytes[1] <= 0xBF) &&
				(0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
                (0x80 <= bytes[3] && bytes[3] <= 0xBF)
			) ||
            (// planes 4-15
                (0xF1 <= bytes[0] && bytes[0] <= 0xF3) &&
				(0x80 <= bytes[1] && bytes[1] <= 0xBF) &&
				(0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
                (0x80 <= bytes[3] && bytes[3] <= 0xBF)
            ) ||
			(// plane 16
				bytes[0] == 0xF4 &&
				(0x80 <= bytes[1] && bytes[1] <= 0x8F) &&
                (0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
				(0x80 <= bytes[3] && bytes[3] <= 0xBF)
            )
		) { not_ascii=true;
            bytes += 4;
			continue;
		  }

		return false;   // not a valid utf-8 string
	}

	return not_ascii;  // is valid utf8 string, but only return true if not ascii
}


extern TForm1 *Form1;

extern double   actual_dXMin,actual_dXMax,actual_dYMin,actual_dYMax;
extern int zoom_fun_level;

double   actual_dXMin=0,actual_dXMax=100,actual_dYMin=-1,actual_dYMax=1;// initialised to same values as below
int zoom_fun_level=0;  // used to keep track of level of recursion in zoom function (allows partial display refresh for speed in multiple zooms)

TScientificGraph::TScientificGraph(int iBitmapWidthK, int iBitmapHeightK)
{
  iBitmapWidth = iBitmapWidthK;                      //bitmap
  iBitmapHeight = iBitmapHeightK;
  iNumberOfGraphs = 0;                               //no graphs

  pHistory = new TList();                            //Graphs
  pBitmap = new Graphics::TBitmap;                   //bitmap
  pBitmap->Width = iBitmapWidth;
  pBitmap->Height = iBitmapHeight;

  ColBackGround = clBlack;                           //default settings
  ColGrid = clOlive;
  ColAxis = clRed;
  ColText = clYellow;
  iTextSize = 10;  // size of most text
  aTextSize = 12; // size of axis titles
  PSAxis = psSolid;
  PSGrid = psDot;
  iTickLength=10;
  iPenWidthAxis=1;
  iPenWidthGrid=1;
  iTextOffset=5;

  sSizeX.dMin=0;
  sSizeX.dMax=100;
  sSizeY.dMin=-1;
  sSizeY.dMax=1;

  iGridsPerX=10;
  iGridsPerY=10;
  bGrids=true;
  bZeroLine=true;

  fLeftBorder = 0.13f;
  fRightBorder = 0.025f;
  fTopBorder = 0.04f;
  fBottomBorder = 0.11f;

  dInX=0.8;
  dOutX=10.0/8.0;
  dInY=0.8;
  dOutY=10.0/8.0;
  dShiftFactor=0.1;

  XLabel="Horizontal axis title";
  YLabel1="Vertical";
  YLabel2="axis title";
             /* note these defaults are changed in UDataPlotWindow.cpp */
  dLegendStartX=0.5; // was 0.5
  dLegendStartY=0.95; // was 0.95
  dCaptionStartX=0.5;  // was 0.5
  dCaptionStartY=0.5;  // was 0.5

  iSkipLineLevel=2; // adjacent points

  fnResize();                                        //put size to scales
};

//------------------------------------------------------------------------------
TScientificGraph::~TScientificGraph()
{  fnClearAll() ; // clear (delete) all lines + the data they hold
  pHistory->Clear();
  delete pHistory;
  delete pBitmap;
  pBitmap=NULL;
};

//-----------------------------------------------------------------------------
void TScientificGraph::resize_bitmap(int iBitmapWidthK, int iBitmapHeightK)
{ // resize bitmap
  iBitmapWidth = iBitmapWidthK;                      //bitmap
  iBitmapHeight = iBitmapHeightK;
  if(pBitmap!=NULL)
        {
         pBitmap->Width = iBitmapWidth;
         pBitmap->Height = iBitmapHeight;
        }
}

//-----------------------------------------------------------------------------

bool TScientificGraph::fnKoord2Point(TPoint *pPoint, double dXValueF,
     double dYValueF)
{
  double dXKoord, dYKoord;
                                            //calculate bitmap coordinates
  dXKoord = (dXValueF-sScaleX.dMin)
       /(sScaleX.dMax-sScaleX.dMin);
  dXKoord *= iBitmapWidth*(1-fRightBorder-fLeftBorder);
  dXKoord += iBitmapWidth*fLeftBorder;

  dYKoord = (dYValueF-sScaleY.dMin)
       /(sScaleY.dMax-sScaleY.dMin);
  dYKoord *= iBitmapHeight*(1-fTopBorder-fBottomBorder);
  dYKoord = iBitmapHeight-iBitmapHeight*fBottomBorder-dYKoord;

  if(dXKoord>0)                            //round to integer
  {dXKoord += 0.5;} else {dXKoord-=0.5;}
  if(dYKoord>0)
  {dYKoord += 0.5;} else {dYKoord-=0.5;}
#if 1
  // pPoint values are ints (32 bits) max values are MaxInt 2147483647
  if (dXKoord>MaxInt) {dXKoord=MaxInt;}     //no integer overflow
  if (dXKoord<-MaxInt) {dXKoord=-MaxInt;}    // was -32768
  if (dYKoord>MaxInt) {dYKoord=MaxInt;}
  if (dYKoord<-MaxInt) {dYKoord=-MaxInt;}    // was -32768
#else
  if (dXKoord>32767) {dXKoord=32767;}     //no integer overflow
  if (dXKoord<-32767) {dXKoord=-32767;}    // was -32768
  if (dYKoord>32767) {dYKoord=32767;}
  if (dYKoord<-32767) {dYKoord=-32767;}    // was -32768
#endif
  pPoint->x=(long)dXKoord;                      //to integer
  pPoint->y=(long)dYKoord;


  if ((dXKoord<iBitmapWidth*fLeftBorder) ||           //point in plot?
     (dXKoord>iBitmapWidth*(1-fRightBorder)) ||
     (dYKoord<iBitmapHeight*fTopBorder) ||
     (dYKoord>iBitmapHeight*(1-fBottomBorder)))
  {  return false;}                                   //no.

  return true;                                        //yes.
};

//------------------------------------------------------------------------------
static double xs,ys; // start of next line (end of previous line)
	 /* Dan Cohen & Ivan Sunderland clipping algorithm - see  Principles of interactive computer graphics 2nd Ed, pp 65-67   */
	 /* this version is functionally the same as the original with various bugs resolved and efficiency improvements */

#define LEFT 1 /* bits - must all be different powers of 2 */
#define RIGHT 2
#define BOTTOM 4
#define TOP 8

void TScientificGraph::graph_line(double xe,double ye,double xmin,double xmax,double ymin,double ymax)
{// draw line from xs,ys to xe,ye clipped by min/max . Afterwards set xs,ys to xe,ye.
 int outcode1=0, outcode2=0;
 int c=0;
 bool line_visible; 	//decides if line is to be drawn
 double x1=xs,y1=ys,x2=xe,y2=ye;
 xs=xe; // save line end as start of next line , means we can just return if nothing to draw
 ys=ye;
 if(x1 < xmin) outcode1 =LEFT;   // for 1st one we can just assign as we know previous value was 0
 if(x2 < xmin) outcode2 =LEFT;   // for 1st one we can just assign as we know previous value was 0
 if(x1 > xmax) outcode1 |=RIGHT; // need to OR in the remainder of the values
 if(x2 > xmax) outcode2 |=RIGHT;
 if(outcode1&outcode2) return; // return if x values out of range , no need to check anything else
 if(y1 > ymax) outcode1 |=TOP;
 if(y1 < ymin) outcode1 |=BOTTOM;
 if(y2 > ymax) outcode2 |=TOP;
 if(y2 < ymin) outcode2 |=BOTTOM;
 if(outcode1&outcode2) return; // return if y values out of range
 	 //AND of both codes != 0 => Line is outside. Reject line

 // if some of all line is visible find which bit
 line_visible = true;   // if we get here assume line will be visible
 while(outcode1 | outcode2)   // every iteration removes 1 bit from outcode1 or outcode2 as (outcode1 & outcode2)==0 at the start we can do at most 4 iterations of this loop
     {
      // line part in and part out
      double m,x,y;
      try
         {
           if(x1!=x2)
                m=(y2-y1)/(x2-x1); // slope of line
           else
                {// vertical line (at x=x1) process as a special case here
                 if(y1>y2) {y=y1;y1=y2;y2=y;} // swap so y2>y1
                 if(y1>ymax || y2<ymin || x1<xmin || x1>xmax)
                        {// not visible
                         line_visible = false;
                         break;
                        }
                  if(y2>ymax) y2=ymax;
                  if(y1<ymin) y1=ymin;
                  break;
                 }
	   int temp;
	   //Decide if point1 is inside, if not, calculate intersection
	   if(outcode1==0)
                {
	 	 temp = outcode2;
                 x=x2;
                 y=y2;
                }
	   else
                {
	 	 temp = outcode1;
                 x=x1;
                 y=y1;
                }

	   if(temp & TOP)  //Line clips top edge , need to check this is still true before we adjust x,y as previous adjustments may already have fixed this
                {c= TOP;
                 if(y>ymax)
                        {
                         if(fabs(m)<1e-250)
                                {// y=mx+c if m=0 then y=c if y>ymax then cannot fix this so line is not visible
                                  line_visible = false;
                                  break;
                                }
                         else
	 	                x = x1+ (ymax-y1)/m;
	 	         y = ymax;
                        }
	 	}
	   else if(temp & BOTTOM)
                { 	//Line clips bottom edge
                 c=BOTTOM;
                 if(y<ymin)
                        {
                         if(fabs(m)<1e-250)
                                {// y=mx+c if m=0 then y=c if y<ymin then cannot fix this so line is not visible
                                  line_visible = false;
                                  break;
                                }
                         else
	 	                x = x1+ (ymin-y1)/m;
	 	         y = ymin;
                        }
                }
	   else if(temp & LEFT)
                { 	//Line clips left edge
                 c=LEFT;
                 if(x<xmin)
                        {
	 	         x = xmin;
	 	         y = y1+ m*(xmin-x1);
                        }
                }
	   else if(temp & RIGHT)
                { 	//Line clips right edge
                 c=RIGHT;
                 if(x>xmax)
                        {
	 	         x = xmax;
	 	         y = y1+ m*(xmax-x1);
                        }
	        }
	   //Check which point we had selected earlier as temp, and replace its co-ordinates , and delete this limit from its list
	   if(outcode1 != 0)
                {
	 	 x1 = x;
	 	 y1 = y;
	 	 outcode1 &= ~c;
                }
	    else  {
	 	 x2 = x;
	 	 y2 = y;
	 	 outcode2 &= ~c;
	        }
           }
      catch (...)
           {// rprintf("CLIP:catch!\n");
            line_visible = false;
             break; // just in case there is a maths error not trapped in the code above as a special case (all should be trapped)
           }

     }// end while(...)
 if(line_visible)
        {// visible portion of line is from x1,y1 to x2,y2
         TPoint Point1; // avoid dynamic memory allocation overhead if we used new and delete
         TPoint *pPoint1=&Point1;
         TPoint Point2;
         TPoint *pPoint2=&Point2;
         fnKoord2Point(pPoint1,x1,y1);
         fnKoord2Point(pPoint2,x2,y2);
         if((Point1.x!=Point2.x || Point1.y!=Point2.y))
                {// if line start and end are different then draw line
                 pBitmap->Canvas->PenPos=Point1;
                 pBitmap->Canvas->LineTo(Point2.x,Point2.y);
                }
        }
 return;
}

//------------------------------------------------------------------------------

void TScientificGraph::fnPaint()

{
  double fMinGrid, fMaxGrid; // double to allow effectively unlimited zooming
  size_t iCount;
  int i,j;
  double dADoub;
  double dX, dY;    // even in inner loops these need to be doubles [ due to my extra clipping code]
  char szAString[31],lasttick[31];
  AnsiString AAnsiString;

  TRect LayoutRect;
  TSize ASize;
  SGraph *pAGraph;
  TPoint Point; // avoid dynamic memory allocation overhead if we used new and delete
  TPoint *pPoint=&Point;
  TPoint Point2; // avoid dynamic memory allocation overhead if we used new and delete
  TPoint *pPoint2=&Point2;
  // rprintf("fnPaint()\n");
  fnCheckScales();                                                //check scales
   // double xd=sScaleX.dMax-sScaleX.dMin; // total span
  //Background
  LayoutRect.Left = 0;
  LayoutRect.Top = 0;
  LayoutRect.Right = pBitmap->Width;
  LayoutRect.Bottom = pBitmap->Height;

  pBitmap->Canvas->Brush->Color = ColBackGround;
  pBitmap->Canvas->FillRect(LayoutRect);
  pBitmap->Canvas->Font->Name="Arial";

  //Gridlines, Ticks, Ticklabels
  double gridsize=dGridSizeX; // inital grid size
  //x-axis
  fMinGrid = ceil(sScaleX.dMin/gridsize);     //min grid
  fMaxGrid = floor(sScaleX.dMax/gridsize);    //max grid
  const char *tickformat="%.8g"; // default tick format
  lasttick[0]=0; // zero length string to start so 1st label always printed
  iCount=0; // nos ticks values skipped
  // rprintf("xaxis fMinGrid=%g fMaxGrid=%g iCount=%d\n",fMinGrid, fMaxGrid,iCount);
  //paint grids,ticks
  // check if all labels will be the same in .8g format
  snprintf(lasttick,sizeof(lasttick),"%.8g",fMinGrid*gridsize);
  snprintf(szAString,sizeof(szAString),"%.8g",fMaxGrid*gridsize);
  if(strncmp(szAString,lasttick,sizeof(szAString))==0 || fMinGrid>=fMaxGrid )
        {// same display fewer points to higher accuracy
         gridsize=5.0;
         fMinGrid = ceil(sScaleX.dMin/gridsize);     //min grid
         fMaxGrid = floor(sScaleX.dMax/gridsize);    //max grid
		 tickformat="%.10g"; // high resolution tick format   [was .14g but this is rather excessive when values are only floats  ]
        }
  lasttick[0]=0; // zero length string to start so 1st label always printed
  // rprintf("xaxis fMinGrid=%g fMaxGrid=%g (max-min=%g) gridsize=%g fmt=%s\n",fMinGrid, fMaxGrid,fMaxGrid-fMinGrid,gridsize,tickformat);
  // see how many points will be skipped
  i=0;
	/* # pragma's below work for gcc and clang compilers , issue is that format argument to snprintf (tickformat) is a variable */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
  if(fMinGrid<fMaxGrid)
   for (double fi = fMinGrid; fi<=fMaxGrid && i< 100; fi=fi+1.0,++i)    //  && i< 100 stops looping forever if we run out of resolution
   {
    dADoub = fi * gridsize;                             //x-value grid
    snprintf(szAString,sizeof(szAString),tickformat,dADoub);  //tick labels .8g is the largest that will fit with full grid
    if(strncmp(szAString,lasttick,sizeof(szAString))!=0 || (fi+1.0>fMaxGrid && iCount<2))
        {// new tick label different to previous one , or last one and only printed 1 previously
         snprintf(lasttick,sizeof(szAString),"%s",szAString); // save this one as last printed , cannot use strncpy as that does not guarantee a null terminated string
        }
     else ++iCount; // point skipped
   }
  // now actually draw axes for real
  lasttick[0]=0; // zero length string to start so 1st label always printed
  if(fMinGrid<fMaxGrid && iCount<=1 && i<15) // if at most one point skipped and 14 plotted
  {i=0;
   iCount=0;
   for (double fi = fMinGrid; fi<=fMaxGrid &&  i< 100 && iCount<15 ; fi=fi+1.0,++i)    //  && i< 100 stops looping forever if we run out of resolution
   {
    dADoub = fi * gridsize;                             //x-value grid
    if (bGrids) fnPaintGridX(dADoub);                    //paint grids
    fnPaintTickX(dADoub,1);                              //paint ticks
    snprintf(szAString,sizeof(szAString),tickformat,dADoub);  //tick labels .8g is the largest that will fit with full grid
    if(strncmp(szAString,lasttick,sizeof(szAString))!=0 || (fi+1.0>fMaxGrid && iCount<2))
        {// new tick label different to previous one , or last one and only printed 1 previously
         ++iCount;
         AAnsiString=AnsiString(szAString);
         fnKoord2Point(pPoint,dADoub,sScaleY.dMin);
         ASize = pBitmap->Canvas->TextExtent(AAnsiString);
         pBitmap->Canvas->Font->Color=ColText;
         pBitmap->Canvas->Font->Size=iTextSize;
		 pBitmap->Canvas->TextOut(pPoint->x-ASize.cx/2,pPoint->y+iTextOffset,AAnsiString);
         snprintf(lasttick,sizeof(szAString),"%s",szAString); // save this one as last printed , cannot use strncpy as that does not guarantee a null terminated string
        }
   }// end for
  }
  else
#if 1  /* double step  so we can increase resolution of numbers */
  {i=0;
   iCount=0;
   tickformat="%.10g";
   for (double fi = fMinGrid; fi<=fMaxGrid &&  i< 100 && iCount<11 ; fi=fi+2.0,++i)    //  && i< 100 stops looping forever if we run out of resolution
   {
    dADoub = fi * gridsize;                             //x-value grid  2* what it was above
    if (bGrids) fnPaintGridX(dADoub);                    //paint grids
    fnPaintTickX(dADoub,1);                              //paint ticks
	snprintf(szAString,sizeof(szAString),tickformat,dADoub);  //tick labels .8g is the largest that will fit with full grid
    if(strncmp(szAString,lasttick,sizeof(szAString))!=0 || (fi+2.0>fMaxGrid && iCount<2))
        {// new tick label different to previous one , or last one and only printed 1 previously
         ++iCount;
         AAnsiString=AnsiString(szAString);
         fnKoord2Point(pPoint,dADoub,sScaleY.dMin);
         ASize = pBitmap->Canvas->TextExtent(AAnsiString);
         pBitmap->Canvas->Font->Color=ColText;
         pBitmap->Canvas->Font->Size=iTextSize;
		 pBitmap->Canvas->TextOut(pPoint->x-ASize.cx/2,pPoint->y+iTextOffset,AAnsiString);
         snprintf(lasttick,sizeof(szAString),"%s",szAString); // save this one as last printed , cannot use strncpy as that does not guarantee a null terminated string
		}
   }// end for
  }
#else /* print 5 ticks values evenly spaced along x axis*/
   {

    for(double z=0;z<=1.000000001;z+=0.25)     //location of tick 0,0.25,0.5,0.75,1, check at <=1.000000001 to allow for 1 with small rounding erros
        {
         dADoub = sScaleX.dMin+z*(sScaleX.dMax-sScaleX.dMin);

         if (bGrids) fnPaintGridX(dADoub);                    //paint grids
         fnPaintTickX(dADoub,1);                              //paint ticks
         snprintf(szAString,sizeof(szAString),"%.10g",dADoub);  //tick labels can add more resolution here as plenty of space
         AAnsiString=AnsiString(szAString);
         fnKoord2Point(pPoint,dADoub,sScaleY.dMin);
         ASize = pBitmap->Canvas->TextExtent(AAnsiString);
         pBitmap->Canvas->Font->Color=ColText;
         pBitmap->Canvas->Font->Size=iTextSize;
         pBitmap->Canvas->TextOutA(pPoint->x-2*ASize.cx/3,pPoint->y+iTextOffset,AAnsiString);     // was -ASize/2
       }
    }
#endif
#pragma GCC diagnostic pop /* GCC diagnostic ignored "-Wformat-nonliteral" */
   //y-axis
  fMinGrid = ceil(sScaleY.dMin/dGridSizeY);     //min grid
  fMaxGrid = floor(sScaleY.dMax/dGridSizeY);    //max grid
  lasttick[0]=0; // zero length string to start so 1st label always printed
  iCount=0; // nos ticks values printed
  // rprintf("yaxis fMinGrid=%g fMaxGrid=%g iCount=%d\n",fMinGrid, fMaxGrid,iCount);
  //paint grids
  i=0;
  for (double fi = fMinGrid; fi<=fMaxGrid && i< 100 ; fi=fi+1.0,++i)    //  && i< 100 stops looping forever if we run out of resolution
  {
    dADoub = fi * dGridSizeY;                             //y-value grid


    if (bGrids) fnPaintGridY(dADoub);                    //paint grids
    fnPaintTickY(dADoub,1);                              //paint ticks
#ifdef USE_double_to_str_exp
   /* void double_to_str_exp(double x, int sf,enum fpout_type out_type, int len, char *s) // sf = significant figures - max 19 */
   {int sf=12; // have space for at most 12 chars
    if(dADoub<0) sf--; // negative sign takes 1
    do{// try until it fits in <= 12 chars
       double_to_str_exp(dADoub,sf--,round_nearest | fmt_g,sizeof(szAString),szAString);
      }
    while(strlen(szAString)>12 && sf>1);   // && sf>1 traps infinite loop (just in case!)
   }
#else
	// gcvt(dADoub,10, szAString); // max 12 characters so 10 digits plus sign, dp
   {int sf=12; // have space for at most 12 chars
	if(dADoub<0) sf--; // negative sign takes 1
	do{// try until it fits in <= 12 chars
	   gcvt(dADoub,sf--,szAString);
	  }
	while(strlen(szAString)>12 && sf>1);   // && sf>1 traps infinite loop (just in case!)
   }
#endif
    if(strncmp(szAString,lasttick,sizeof(szAString))!=0 || (fi+1.0>fMaxGrid && iCount<2))
        { // new tick label different to previous one , or last one and only printed 1 previously
         ++iCount;
         AAnsiString=AnsiString(szAString);
         fnKoord2Point(pPoint,sScaleX.dMin,dADoub);
         ASize = pBitmap->Canvas->TextExtent(AAnsiString);
         pBitmap->Canvas->Font->Color=ColText;
         pBitmap->Canvas->Font->Size=iTextSize;
		 pBitmap->Canvas->TextOut(pPoint->x-ASize.cx-iTextOffset,pPoint->y-ASize.cy/2,AAnsiString);
         snprintf(lasttick,sizeof(szAString),"%s",szAString); // save this one as last printed , cannot use strncpy as that does not guarantee a null terminated string
        }
  }
  // rprintf("yaxis2 fMinGrid=%g fMaxGrid=%g iCount=%d\n",fMinGrid, fMaxGrid,iCount);
  if(iCount==0)
   {  // zoomed too much so no values printed above - just print 1 value here (mid)
    dADoub = 0.5* (sScaleY.dMin+sScaleY.dMax);            //y-value grid - just mid point
    if(fMinGrid==fMaxGrid)
        {  // add tick/grid line as code above will not have printed one
         if (bGrids) fnPaintGridY(dADoub);                    //paint grids
         fnPaintTickY(dADoub,1);                              //paint ticks
        }
#ifdef USE_double_to_str_exp
   /* void double_to_str_exp(double x, int sf,enum fpout_type out_type, int len, char *s) // sf = significant figures - max 19 */
   {int sf=12; // have space for at most 12 chars
	if(dADoub<0) sf--; // negative sign takes 1
	do{// try until it fits in <= 12 chars
	   double_to_str_exp(dADoub,sf--,round_nearest | fmt_g,sizeof(szAString),szAString);
	  }
	while(strlen(szAString)>12 && sf>1);   // && sf>1 traps infinite loop (just in case!)
   }
#else
   {int sf=12; // have space for at most 12 chars
	if(dADoub<0) sf--; // negative sign takes 1
	do{// try until it fits in <= 12 chars
	   gcvt(dADoub,sf--,szAString);
	  }
	while(strlen(szAString)>12 && sf>1);   // && sf>1 traps infinite loop (just in case!)
   }
#endif
	AAnsiString=AnsiString(szAString);
	fnKoord2Point(pPoint,sScaleX.dMin,dADoub);
	ASize = pBitmap->Canvas->TextExtent(AAnsiString);
	pBitmap->Canvas->Font->Color=ColText;
	pBitmap->Canvas->Font->Size=iTextSize;
	pBitmap->Canvas->TextOut(pPoint->x-ASize.cx-iTextOffset,pPoint->y-ASize.cy/2,AAnsiString);
   }
  //Zeroline
  if (!bGrids & bZeroLine)                             //zeroline not necess. if
													   //grids are enabled
	if (fnInScaleY(0))                                 //zeroline in plot?
	{
	  fnKoord2Point(pPoint,sScaleX.dMin,0);            //paint line in gridstyle
	  pBitmap->Canvas->PenPos=*pPoint;
	  fnKoord2Point(pPoint,sScaleX.dMax,0);
	  pBitmap->Canvas->Pen->Color = ColGrid;
	  pBitmap->Canvas->Pen->Width = iPenWidthGrid;
	  pBitmap->Canvas->Pen->Style = PSGrid;
	  pBitmap->Canvas->LineTo(pPoint->x,pPoint->y);
	}
#if 0 /* set to 1 to print legend 1st, before traces (means it may not be visible) */
  //Legend
															   //calc position
  dX=dLegendStartX*(sScaleX.dMax
	 -sScaleX.dMin)+sScaleX.dMin
     ;
  dY=dLegendStartY*(sScaleY.dMax
     -sScaleY.dMin)+sScaleY.dMin
	 ;
  fnKoord2Point(pPoint,dX,dY);                                //calc. coordinat.
  pBitmap->Canvas->Font->Size=iTextSize;
  for (j=0; j<iNumberOfGraphs; j++)                           //all graphs
  {
    pAGraph = ((SGraph*) pHistory->Items[j]);
	if (pAGraph->Caption!="")
    {
	  *pPoint2=*pPoint;
	  if (((pAGraph->ucStyle) & 1) == 1)                      //paint data point
	  {                                                       //for legend
		i=pBitmap->Canvas->TextHeight(pAGraph->Caption);
		i/=2;
		pPoint->y+=i;
		pPoint->x+=pBitmap->Canvas->TextWidth("22");
		LayoutRect.Left=(pPoint->x)-(pAGraph->iSizeDataPoint/2);
		LayoutRect.Right=(pPoint->x)+(pAGraph->iSizeDataPoint/2);
		LayoutRect.Top=(pPoint->y)-(pAGraph->iSizeDataPoint/2);
		LayoutRect.Bottom=(pPoint->y)+(pAGraph->iSizeDataPoint/2);
		pBitmap->Canvas->Pen->Width=1;
		pBitmap->Canvas->Pen->Color=pAGraph->ColDataPoint;
		pBitmap->Canvas->Pen->Style=psSolid;
		pBitmap->Canvas->Brush->Color=pAGraph->ColDataPoint;
		fnPaintDataPoint(LayoutRect,pAGraph->ucPointStyle);
		if (((pAGraph->ucStyle) & 4) == 4)
		{
		  *pPoint=*pPoint2;
		}
		else
		{
		  pPoint->y-=i;
		  pPoint->x+=pBitmap->Canvas->TextWidth("333");
		  pBitmap->Canvas->Font->Color=pAGraph->ColDataPoint;
		}
	  }
	  if (((pAGraph->ucStyle) & 4) == 4)                     //paint short line
	  {                                                      //for legend
		i=pBitmap->Canvas->TextHeight(pAGraph->Caption);
		i/=2;
		pPoint->y+=i;
		pBitmap->Canvas->PenPos=*pPoint;
		pPoint->x+=pBitmap->Canvas->TextWidth("4444");
		pBitmap->Canvas->Pen->Width=pAGraph->iWidthLine;;
		pBitmap->Canvas->Pen->Color=pAGraph->ColLine;
		pBitmap->Canvas->Pen->Style=pAGraph->LineStyle;
		pBitmap->Canvas->LineTo(pPoint->x,pPoint->y);
		pPoint->y-=i;
		pPoint->x+=pBitmap->Canvas->TextWidth("1");
		pBitmap->Canvas->Font->Color=pAGraph->ColLine;
	  }
	  pBitmap->Canvas->Font->Size=iTextSize;                //paint caption
	  pBitmap->Canvas->TextOut(pPoint->x,pPoint->y,Utf8_to_w(pAGraph->Caption));
	  *pPoint=*pPoint2;
	  if ((pAGraph->iSizeDataPoint>pBitmap->Canvas->TextHeight("0"))&&
		 (((pAGraph->ucStyle) & 1) == 1))
		pPoint->y+=pAGraph->iSizeDataPoint+5;
	  else pPoint->y+=pBitmap->Canvas->TextHeight("0");
	}
  }
#endif
  //Clip Rect
  HRGN MyRgn;

  fnKoord2Point(pPoint2,sScaleX.dMax,sScaleY.dMax);
  fnKoord2Point(pPoint,sScaleX.dMin,sScaleY.dMin);
  // rprintf("Clipping region from X=%.20g Y=%.20g to X=%.20g Y=%.20g\n",(double)(pPoint2->x),(double)(pPoint2->y),(double)(pPoint->x),(double)(pPoint->y));
  double x_width_pixels= (double)(pPoint2->x)-(double)(pPoint->x);
  if(x_width_pixels<0) x_width_pixels= -x_width_pixels;
#if 1
  MyRgn = ::CreateRectRgn(pPoint2->x,pPoint2->y,pPoint->x,pPoint->y);
#else
  MyRgn = ::CreateRectRgn(pPoint->x,pPoint2->y,pPoint2->x,pPoint->y);
#endif
  ::SelectClipRgn(pBitmap->Canvas->Handle,MyRgn);

  //Data points and Error Bars, lines
  for (j = 0; j < iNumberOfGraphs; j++)
  {
	pAGraph = ((SGraph*) pHistory->Items[j]);
    if (((pAGraph->ucStyle) & 1) == 1)             //style: data point
    { unsigned int istep;
         /* new, more intelligent way to display points
           if we have to skip points, show min/max by an vertical "error bar"
           so that range is obvious
           If you zoom in enough just points are shown
         */
      DOUBLE ymax,ymin,x_ymax,x_ymin; // used to capture features in skipped data
      DOUBLE lastx,lasty;
      double xd=sScaleX.dMax-sScaleX.dMin; // total span
      double xi=xd/x_width_pixels; // use all the pixels available
      xd=sScaleX.dMin-xi; // -xi as add xi before its used
	  iCount=pAGraph->nos_vals ;
#if 1
      // do binary search to find start of area thats visible on the screen
	  ssize_t starti;    // index just before start
      {
	   ssize_t low=0;
	   ssize_t high=(ssize_t)iCount-1;
       bool found=false;
       double key=sScaleX.dMin;  // needs to be double as otherwise compare midval<key can generate an overflow if dMin -? dMax is a very large range
	   ssize_t mid=0;
       while(low<=high && !found)
        {mid=low+((high-low)>>1); /* (low+high)/2 but written so cannot overflow */
		 double midVal=pAGraph->x_vals[mid];
		 if(midVal<key)
				low=mid+1;
		 else if (midVal>key)
				high=mid-1;
		 else
				found=true; // mid is exact match
		}
	   if(found) starti=mid;
	   else starti=low-1;   // not found want 1 before
	   if(starti<0) starti=0; // ensure not before start (don't worry if its past the end as for loop below deals with that case)
	  }
	  for (size_t ii=(size_t)starti; ii<iCount; ii++)  // was i+=step
#else
	  for (size_t ii=0; ii<iCount; ii++)  // was i+=step
#endif
	  {
	   dX = pAGraph->x_vals[ii];
	   if(!fnInScaleX(dX))
				{if(dX> sScaleX.dMax) break; // past end so all done for this trace
                 else continue; // PMi optimisation - skip values before xmin
                                // this is important when zooming in as otherwise code below will see the whole file and will "compress" the graph incorrectly
                }
	   ymax=ymin=pAGraph->y_vals[ii];// dY
	   x_ymax=x_ymin=(float)dX;
	   if(zoom_fun_level)
		  {Application->ProcessMessages(); /* allow windows to update (but not go idle) - potentially causes recursion ! */
		   if(zoom_fun_level>1)
				goto fnpaint_end;// > 1 means we have recursion , abort present update as we need to do another with different scaling
		  }
		// we know scaling so we can calculate how many points we need to skip
	   xd+=xi; // this works better when "skip equal y values is set" as x values are not then evenly spaced and this way points selected are evenly spaced
	   dX = pAGraph->x_vals[ii]; // dX,dY is 1st point examined, lastx,lasty is last point in this "segment"
	   dY = pAGraph->y_vals[ii];
	   for(istep=1;ii+istep<iCount && pAGraph->x_vals[ii+istep]<xd ;++istep)
		{lastx = pAGraph->x_vals[ii+istep];
		 lasty = pAGraph->y_vals[ii+istep];
         if(lasty>ymax) {ymax=lasty;x_ymax=lastx;}
         if(lasty<ymin) {ymin=lasty;x_ymin=lastx;}
        }
	   ii+=istep-1;
       bool show_main_pt=false;
       if (fnKoord2Point(pPoint,dX,dY))
        { show_main_pt=true;
          LayoutRect.Left=(pPoint->x)-(pAGraph->iSizeDataPoint/2);
          LayoutRect.Right=(pPoint->x)+(pAGraph->iSizeDataPoint/2);
          LayoutRect.Top=(pPoint->y)-(pAGraph->iSizeDataPoint/2);
          LayoutRect.Bottom=(pPoint->y)+(pAGraph->iSizeDataPoint/2);

          pBitmap->Canvas->Pen->Width=1;
          pBitmap->Canvas->Pen->Color=pAGraph->ColDataPoint;
          pBitmap->Canvas->Pen->Style=psSolid;
          pBitmap->Canvas->Brush->Color=pAGraph->ColDataPoint;
          fnPaintDataPoint(LayoutRect,pAGraph->ucPointStyle);
        }
       if (ymin!=ymax)     // draw points at min and max (so we have 3 vertical points)  [assuming they are far enough away to show ]
          {
            if( fnKoord2Point(pPoint2,x_ymin,ymin) && (!show_main_pt ||
                 (abs(pPoint->x-pPoint2->x)+ abs(pPoint->y-pPoint2->y) >iSkipLineLevel))
                ) 
                { // if point is far enough away to be worthwhile drawing
                 LayoutRect.Left=(pPoint2->x)-(pAGraph->iSizeDataPoint/2);
                 LayoutRect.Right=(pPoint2->x)+(pAGraph->iSizeDataPoint/2);
                 LayoutRect.Top=(pPoint2->y)-(pAGraph->iSizeDataPoint/2);
                 LayoutRect.Bottom=(pPoint2->y)+(pAGraph->iSizeDataPoint/2);
                 pBitmap->Canvas->Pen->Width=1;
                 pBitmap->Canvas->Pen->Color=pAGraph->ColDataPoint;
                 pBitmap->Canvas->Pen->Style=psSolid;
                 pBitmap->Canvas->Brush->Color=pAGraph->ColDataPoint;
                 fnPaintDataPoint(LayoutRect,pAGraph->ucPointStyle);
                }
            if(  fnKoord2Point(pPoint2,x_ymax,ymax) && (!show_main_pt ||
                 (abs(pPoint->x-pPoint2->x)+ abs(pPoint->y-pPoint2->y) >iSkipLineLevel))
                )
                { // if point is far enough away to be worthwhile drawing
                 LayoutRect.Left=(pPoint2->x)-(pAGraph->iSizeDataPoint/2);
                 LayoutRect.Right=(pPoint2->x)+(pAGraph->iSizeDataPoint/2);
                 LayoutRect.Top=(pPoint2->y)-(pAGraph->iSizeDataPoint/2);
                 LayoutRect.Bottom=(pPoint2->y)+(pAGraph->iSizeDataPoint/2);
                 pBitmap->Canvas->Pen->Width=1;
                 pBitmap->Canvas->Pen->Color=pAGraph->ColDataPoint;
                 pBitmap->Canvas->Pen->Style=psSolid;
                 pBitmap->Canvas->Brush->Color=pAGraph->ColDataPoint;
                 fnPaintDataPoint(LayoutRect,pAGraph->ucPointStyle);
                }
        }   // end if error "bar"
      }   // end for(ii)

    }  // end if (((pAGraph->ucStyle) & 1) == 1) (if style: data point)
	if (((pAGraph->ucStyle)&4)==4)                        //style: line
    { unsigned int istep;
      bool first=True;    // used to trap start and end of region we wish to view (when zoomed)
      bool last=False;
      DOUBLE ymax,ymin;
      double x_ymax,x_ymin; // used to capture features in skipped data (need to be double due to clipping)
      DOUBLE lastx,lasty;
	  iCount=pAGraph->nos_vals;
      pBitmap->Canvas->Pen->Width=pAGraph->iWidthLine;
      pBitmap->Canvas->Pen->Color=pAGraph->ColLine;
      pBitmap->Canvas->Pen->Style=pAGraph->LineStyle;
      //iCount=pAList->Count;
      if (iCount!=0)
	  {
		dX = pAGraph->x_vals[0];
		dY = pAGraph->y_vals[0];
        fnKoord2Point(pPoint,dX,dY);
        pBitmap->Canvas->PenPos=*pPoint;
        *pPoint2=*pPoint;

      }
      double xd=sScaleX.dMax-sScaleX.dMin; // total span
      double xi=xd/x_width_pixels; // use all the pixels available
      xd=sScaleX.dMin-xi; // -xi as add xi before its used
#if 1
      // do binary search to find start of area thats visible on the screen
	  ssize_t starti;    // index just before start
      {
	   ssize_t low=0;
	   ssize_t high=(ssize_t)iCount-1;
       bool found=false;
       double key=sScaleX.dMin;   // without this being a double we get floating point overflow on midVal<key below when dMin->dMax is very large
	   ssize_t mid=0;
       while(low<=high && !found)
		{mid=low+((high-low)>>1); /* (low+high)/2 but written so cannot overflow */
		 double midVal=pAGraph->x_vals[mid];
         if(midVal<key)
                low=mid+1;
         else if (midVal>key)
                high=mid-1;
         else
                found=true; // mid is exact match
        }
	   if(found) starti=mid;
       else starti=low-1;   // not found want 1 before
       if(starti<0) starti=0; // ensure not before start (don't worry if its past the end as for loop below deals with that case)
      }
	  for (size_t ii=(size_t)starti; ii<iCount; ii++)  // start processing just where we need to.
#else
	  for (size_t ii=0; ii<iCount; ii++)  // linear search from start
#endif      
      {
	   dX = pAGraph->x_vals[ii];
	   if(first && dX >sScaleX.dMin)
        { // 1st point after min x value - want to process this
        }
       else if(last==false && dX>sScaleX.dMax)
        { // 1st point after max x value - also want to process this
         last=true;
        }
       else if(!fnInScaleX(dX))
                {if(dX> sScaleX.dMax) break; // past end so all done for this trace
                 else continue; // PMi optimisation - skip values before xmin
                                // this is important when zooming in as otherwise code below will see the whole file and will "compress" the graph incorrectly
                }
       if(first)
        {// first point to be displayed - need to define start of the 1st line
         if(ii>0)
				{xs= pAGraph->x_vals[ii-1];
				 ys =pAGraph->y_vals[ii-1];
                }
		 else
				{xs= pAGraph->x_vals[0];
				 ys = pAGraph->y_vals[0] ;
                }
        }

	   dX = pAGraph->x_vals[ii];
	   dY = pAGraph->y_vals[ii] ;
	   ymax=ymin=(float)dY;
	   x_ymin=x_ymax=(float)dX;
       if(zoom_fun_level)
          {Application->ProcessMessages(); /* allow windows to update (but not go idle) - potentially causes recursion ! */
           if(zoom_fun_level>1)
                goto fnpaint_end;// > 1 means we have recursion , abort present update as we need to do another with different scaling
          }
        // we know scaling so we can calculate how many points we need to skip
       xd+=xi; // this works better when "skip equal y values is set" as x values are not then evenly spaced and this way points selected are evenly spaced
	   lastx=(float)dX ; // dX,dY is 1st point examined, lastx,lasty is last point in this "segment"
	   lasty=(float)dY ;
	   for(istep=1;ii+istep<iCount && pAGraph->x_vals[ii+istep]<xd ;++istep)
		{lastx = pAGraph->x_vals[ii+istep];
		 lasty = pAGraph->y_vals[ii+istep];
		 if(lasty>ymax) {ymax=lasty;x_ymax=lastx;}
         if(lasty<ymin) {ymin=lasty;x_ymin=lastx;}
        }
       ii+=istep-1;
        {
         if(x_ymin>x_ymax)
                {double tx,ty; // swap around to make sure we stay in order of increasing x
                 tx=x_ymin;ty=ymin;
                 x_ymin=x_ymax; ymin=ymax;
                 x_ymax=tx; ymax=(float)ty;
                }
         // graph_line(double xe,double ye,double xmin,double xmax,double ymin,double ymax)
         graph_line(x_ymin,ymin,sScaleX.dMin,sScaleX.dMax,sScaleY.dMin,sScaleY.dMax); // draw line (clipped)
         if(ymin!=ymax)
                {// print min and max if they are different (only happens when points skipped)
                 graph_line(x_ymax,ymax,sScaleX.dMin,sScaleX.dMax,sScaleY.dMin,sScaleY.dMax); // draw line (clipped)
                }
         if(lasty!=ymax)
                {// end point of this region (if different)
                 graph_line(lastx,lasty,sScaleX.dMin,sScaleX.dMax,sScaleY.dMin,sScaleY.dMax); // draw line (clipped)
                }
        }
       first=False;
      }
    }
  }
fnpaint_end:  // tidy up then return if we get here via a goto.
  //delete ClipRect
  ::SelectClipRgn(pBitmap->Canvas->Handle,NULL);
  ::DeleteObject(MyRgn);
  if(zoom_fun_level>1)
       return; // finish now as need to restart over with new scaling

#if 1 /* set to 1 to print trace legends last (means they should be visible) if using "LEGEND_CLEAR_BACKGROUND" code */
  //Legend , if required draw them
 if(Form1!=NULL && Form1->pPlotWindow!=NULL && Form1->pPlotWindow->CheckBox_legend->State==cbChecked)
 {//calc position
  dX=dLegendStartX*(sScaleX.dMax
	 -sScaleX.dMin)+sScaleX.dMin
	 ;
  dY=dLegendStartY*(sScaleY.dMax
	 -sScaleY.dMin)+sScaleY.dMin
	 ;
  fnKoord2Point(pPoint,dX,dY);                                //calc. coordinat.
  pBitmap->Canvas->Font->Size=iTextSize;
  for (j=0; j<iNumberOfGraphs; j++)                           //all graphs
  {
	pAGraph = ((SGraph*) pHistory->Items[j]);
	if (pAGraph->Caption!="")
	{
	  *pPoint2=*pPoint;
#if 1 /* LEGEND_CLEAR_BACKGROUND */
	  {
		i=pBitmap->Canvas->TextHeight(Utf8_to_w(pAGraph->Caption.c_str()));
		i/=2;
		pPoint->y+=i;
		//pPoint->x+=pBitmap->Canvas->TextWidth("22");
		LayoutRect.Left=(pPoint->x)-(pAGraph->iSizeDataPoint/2);
		LayoutRect.Right=(pPoint->x)+(pAGraph->iSizeDataPoint/2)+pBitmap->Canvas->TextWidth("22");
		//LayoutRect.Top=(pPoint->y)-(pAGraph->iSizeDataPoint/2);
		//LayoutRect.Bottom=(pPoint->y)+(pAGraph->iSizeDataPoint/2);
		LayoutRect.Top=(pPoint->y)-(i);
		LayoutRect.Bottom=(pPoint->y)+(i);
		LayoutRect.Right+=pBitmap->Canvas->TextWidth("4444");
		LayoutRect.Right+=pBitmap->Canvas->TextWidth(Utf8_to_w(pAGraph->Caption.c_str()));
		pBitmap->Canvas->Brush->Color = ColBackGround;
		pBitmap->Canvas->FillRect(LayoutRect);
		*pPoint=*pPoint2;// restore back ready for code below
	  }
#endif
	  if (((pAGraph->ucStyle) & 1) == 1)                      //paint data point
	  {                                                       //for legend
		i=pBitmap->Canvas->TextHeight(Utf8_to_w(pAGraph->Caption.c_str()));
		i/=2;
		pPoint->y+=i;
		pPoint->x+=pBitmap->Canvas->TextWidth("22");
		LayoutRect.Left=(pPoint->x)-(pAGraph->iSizeDataPoint/2);
		LayoutRect.Right=(pPoint->x)+(pAGraph->iSizeDataPoint/2);
		LayoutRect.Top=(pPoint->y)-(pAGraph->iSizeDataPoint/2);
		LayoutRect.Bottom=(pPoint->y)+(pAGraph->iSizeDataPoint/2);

		pBitmap->Canvas->Pen->Width=1;
		pBitmap->Canvas->Pen->Color=pAGraph->ColDataPoint;
		pBitmap->Canvas->Pen->Style=psSolid;
		pBitmap->Canvas->Brush->Color=pAGraph->ColDataPoint;
		fnPaintDataPoint(LayoutRect,pAGraph->ucPointStyle);
		if (((pAGraph->ucStyle) & 4) == 4)
		{
		  *pPoint=*pPoint2;
		}
		else
		{
		  pPoint->y-=i;
		  pPoint->x+=pBitmap->Canvas->TextWidth("333");
		  pBitmap->Canvas->Font->Color=pAGraph->ColDataPoint;
		}
	  }
	  if (((pAGraph->ucStyle) & 4) == 4)                     //paint short line
	  {                                                      //for legend
		i=pBitmap->Canvas->TextHeight(Utf8_to_w(pAGraph->Caption.c_str()));
		i/=2;
		pPoint->y+=i;
		pBitmap->Canvas->PenPos=*pPoint;
		pPoint->x+=pBitmap->Canvas->TextWidth("4444");
		pBitmap->Canvas->Pen->Width=pAGraph->iWidthLine;;
		pBitmap->Canvas->Pen->Color=pAGraph->ColLine;
		pBitmap->Canvas->Pen->Style=pAGraph->LineStyle;
		pBitmap->Canvas->LineTo(pPoint->x,pPoint->y);
		pPoint->y-=i;
		pPoint->x+=pBitmap->Canvas->TextWidth("1");
		pBitmap->Canvas->Font->Color=pAGraph->ColLine;
	  }
	  pBitmap->Canvas->Font->Size=iTextSize;                //paint caption
	  pBitmap->Canvas->TextOut(pPoint->x,pPoint->y,Utf8_to_w(pAGraph->Caption.c_str()));
	  *pPoint=*pPoint2;
	  if ((pAGraph->iSizeDataPoint>pBitmap->Canvas->TextHeight("0"))&&
		 (((pAGraph->ucStyle) & 1) == 1))
		pPoint->y+=pAGraph->iSizeDataPoint+5;
	  else pPoint->y+=pBitmap->Canvas->TextHeight("0");
	}
  }
 }
#endif

  //axis caption

  int off2=pBitmap->Canvas->TextWidth("-0.000000000000");  // needs to be done with Font Size = iTextSize
  pBitmap->Canvas->Font->Size=aTextSize; // now swap to x/y axis ledgends font size
  fnKoord2Point(pPoint,sScaleX.dMin,
               (sScaleY.dMax
               -sScaleY.dMin)
               *dCaptionStartY+sScaleY.dMin
               );
  if(pBitmap->Canvas->TextExtent(YLabel1).cx>
			pBitmap->Canvas->TextExtent(YLabel2).cx)
  {
	ASize=pBitmap->Canvas->TextExtent(YLabel1);
  }
  else
  {
	ASize=pBitmap->Canvas->TextExtent(YLabel2);
  }
  pBitmap->Canvas->Font->Color=ColText;
  pBitmap->Canvas->Font->Size=aTextSize;

  int off1=off2+pBitmap->Canvas->TextHeight("0"); // needs to be done with Font Size = aTextSize , as text is vertical here need to add Height of 2nd line
  // PMi rotate text for y axis so it fits better into available space
#define ROT_TXT
#ifdef ROT_TXT
#if 1   /* new (much simpler) way to rotate via VCL */
  pBitmap->Canvas->Font->Orientation=900; // 90 deg rotation
#if 0
  // now print axis label - as space is not a big now issue just combine 2 labels into 1
  pBitmap->Canvas->TextOutA(pPoint->x-iTextOffset*2
	   -pBitmap->Canvas->TextWidth("-0.00000000000000"),pPoint->y,YLabel1+" "+YLabel2);
#else
  // now print axis labels
  int off_len_LY1=pBitmap->Canvas->TextWidth(YLabel1);
  int off_len_LY2=pBitmap->Canvas->TextWidth(YLabel2);
  if(YLabel2=="")
		{ // only 1 label to print, put it nearest to the axis , // +off_len_LY?/2 centres Ylabel
		 pBitmap->Canvas->TextOut(pPoint->x-iTextOffset*2
				-off2,pPoint->y+off_len_LY1/2,YLabel1);
		}
  else
		{// 2 ledgends to print , have to space them out
		 pBitmap->Canvas->TextOut(pPoint->x-iTextOffset*2
				-off1,pPoint->y+off_len_LY1/2,YLabel1);
		 pBitmap->Canvas->TextOut(pPoint->x-iTextOffset*2
				-off2,pPoint->y+off_len_LY2/2,YLabel2);
		}
#endif
  // restore original values back
  pBitmap->Canvas->Font->Orientation=0;
#else /* orig code */
  // the following code is based on http://www.bcbjournal.org/articles/vol2/9801/Rotated_fonts.htm?PHPSESSID=caec6429be51c2338b088838c96fe7ff
  static LOGFONT lf;   // DOES need to be static
  GetObject(pBitmap->Canvas->Font->Handle,sizeof(LOGFONT), &lf);
  // Change escapement, orientation, output precision
  lf.lfEscapement = 900; // rotation in deg*10
  lf.lfOrientation = 900;
  lf.lfOutPrecision = OUT_TT_ONLY_PRECIS;
  // Create new font; assign to Canvas Font's Handle.
  pBitmap->Canvas->Font->Handle = CreateFontIndirect(&lf);
  // The following only works on NT!
  SetGraphicsMode(pBitmap->Canvas->Handle, GM_ADVANCED);
  // Set the brush style to clear.
  pBitmap->Canvas->Brush->Style = bsClear;
#if 0
  // now print axis label - as space is not a big now issue just combine 2 labels into 1
  pBitmap->Canvas->TextOutA(pPoint->x-iTextOffset*2
	   -pBitmap->Canvas->TextWidth("-0.00000000000000"),pPoint->y,YLabel1+" "+YLabel2);
#else
  // now print axis labels
  int off_len_LY1=pBitmap->Canvas->TextWidth(Utf8_to_w(YLabel1.c_str()));
  int off_len_LY2=pBitmap->Canvas->TextWidth(Utf8_to_w(YLabel2.c_str()));
  if(YLabel2=="")
		{ // only 1 label to print, put it nearest to the axis , // +off_len_LY?/2 centres Ylabel
		 pBitmap->Canvas->TextOut(pPoint->x-iTextOffset*2
				-off2,pPoint->y+off_len_LY1/2,Utf8_to_w(YLabel1.c_str()));
		}
  else
		{// 2 ledgends to print , have to space them out
		 pBitmap->Canvas->TextOut(pPoint->x-iTextOffset*2
				-off1,pPoint->y+off_len_LY1/2,Utf8_to_w(YLabel1.c_str()));
		 pBitmap->Canvas->TextOut(pPoint->x-iTextOffset*2
				-off2,pPoint->y+off_len_LY2/2,Utf8_to_w(YLabel2.c_str()));
		}
#endif
  // restore original values back
  lf.lfEscapement = 0; // rotation in deg*10
  lf.lfOrientation = 0;
  lf.lfOutPrecision = OUT_DEFAULT_PRECIS;  // restore to original value
  // Create new font with zero rotation ; assign to Canvas Font's Handle.
  pBitmap->Canvas->Font->Handle = CreateFontIndirect(&lf);
#endif
#else
  // original code
  pBitmap->Canvas->TextOutA(pPoint->x-ASize.cx-iTextOffset*2
	   -pBitmap->Canvas->TextWidth("-0.0000000"),pPoint->y,Utf8_to_w(YLabel1.c_str()));
  pBitmap->Canvas->TextOutA(pPoint->x-ASize.cx-iTextOffset*2
	   -pBitmap->Canvas->TextWidth("-0.0000000"),pPoint->y+ASize.cy,Utf8_to_w(YLabel2.c_str()));
#endif
  fnKoord2Point(pPoint,(sScaleX.dMax
				-sScaleX.dMin)*dCaptionStartX
				+sScaleX.dMin,
				sScaleY.dMin);
  ASize=pBitmap->Canvas->TextExtent(XLabel);
  pBitmap->Canvas->Font->Color=ColText;
  int off_len_LX=pBitmap->Canvas->TextWidth(XLabel);
#if 1  /* better position of x axis label over a wider ramge of font sizes */
  // rprintf("Text height=%d\n",pBitmap->Canvas->TextHeight(XLabel));
  pBitmap->Canvas->TextOut(pPoint->x-off_len_LX/2,pPoint->y+20+iTextOffset,XLabel);   // -off_len_LX/2 centres Xlabel
#else
  pBitmap->Canvas->TextOut(pPoint->x-off_len_LX/2,pPoint->y+ASize.cy+iTextOffset,XLabel);   // -off_len_LX/2 centres Xlabel
#endif
  //borders
  // pVertices = new TPoint[5];
  TPoint pVertices[5]; // avoid dynamic memory allocation overhead if we used new and delete

  pBitmap->Canvas->Pen->Color = ColAxis;
  pBitmap->Canvas->Pen->Width = iPenWidthAxis;
  pBitmap->Canvas->Pen->Style = PSAxis;

  fnKoord2Point(pPoint,sScaleX.dMin,sScaleY.dMin);
  pVertices[0] = *pPoint;
  pVertices[4] = *pPoint;
  fnKoord2Point(pPoint,sScaleX.dMin,sScaleY.dMax);
  pVertices[1] = *pPoint;
  fnKoord2Point(pPoint,sScaleX.dMax,sScaleY.dMax);
  pVertices[2] = *pPoint;
  fnKoord2Point(pPoint,sScaleX.dMax,sScaleY.dMin);
  pVertices[3] = *pPoint;
  pBitmap->Canvas->Polyline(pVertices,4);

}


//------------------------------------------------------------------------------

bool TScientificGraph::fnAddDataPoint(float dXValueF, float dYValueF,int iGraphNumberF)    // returns true is added OK, false if not
{ if(iGraphNumberF<0 || iGraphNumberF >=iNumberOfGraphs) return false; // invalid graph number
  SGraph *pAGraph = ((SGraph*) pHistory->Items[iGraphNumberF]);
  size_t i=pAGraph->nos_vals; // current size
  if(i >=pAGraph->size_vals_arrays ) return false; // array full [ could try and extend arrays here, but should not be needed ]
  pAGraph->x_vals[i]= dXValueF;
  pAGraph->y_vals[i]= dYValueF;
  pAGraph->nos_vals=i+1; // one more data point stored
  // rprintf("addpoint X=%g Y=%g graphnos=%d point#=%d\n",dXValueF,dYValueF,iGraphNumberF,i);
  return true; // data point added OK
};

float TScientificGraph::fnAddDataPoint_nextx(int iGraphNumberF)    // returns next x value for this graph assuming its the same as the previous graph
{
  if(iGraphNumberF<1 || iGraphNumberF >=iNumberOfGraphs) return 0; // invalid graph number
  SGraph *pAGraph = ((SGraph*) pHistory->Items[iGraphNumberF]);
  SGraph *pAGraph_1 = ((SGraph*) pHistory->Items[iGraphNumberF-1]); // previous trace added
  size_t i=pAGraph->nos_vals; // current size
  if(i >=pAGraph_1->nos_vals )
	{rprintf("Warning:fnAddDataPoint_nextx(%d): pAGraph->nos_vals=%.0f pAGraph_1->nos_vals=%.0f\n",iGraphNumberF,(double)i,(double)(pAGraph_1->nos_vals));
	 return 0; // past end of previous x array
	}
  return pAGraph_1->x_vals[i];     // value from previous trace
};

#if 1  /* use interpolation to find matching y value to current x value even if current x value is not actually in the array */
float TScientificGraph::fnAddDataPoint_thisy(int iGraphNumber)    // returns next y value of iGraphNumber (locn from current graph number)  used to do $T1
{  extern float xval; // x value of current point
  if(iGraphNumber<0 || iGraphNumber >=iNumberOfGraphs-1) return 0; // invalid graph number (-1 as cannot refer to current trace
  SGraph *pAGraph = ((SGraph*) pHistory->Items[iGraphNumber]); // previous trace added
  // float interp1D(float *xa, float *ya, int size, float x, bool clip)
  return interp1D_f(pAGraph->x_vals,pAGraph->y_vals,pAGraph->nos_vals ,xval,false);
};
#else /* original code - does not work if x values need to be sorted afterwards */
float TScientificGraph::fnAddDataPoint_thisy(int iGraphNumber)    // returns next y value of iGraphNumber (locn from current graph number)  used to do $T1
{
  if(iGraphNumber<0 || iGraphNumber >=iNumberOfGraphs-1) return 0; // invalid graph number (-1 as cannot refer to current trace
  SGraph *pAGraph_add = ((SGraph*) pHistory->Items[iNumberOfGraphs-1]);
  SGraph *pAGraph = ((SGraph*) pHistory->Items[iGraphNumber]); // previous trace added
  int i=pAGraph_add->nos_vals; // current size
  if(i >=pAGraph->nos_vals ) return 0; // past end of previous x array
  return pAGraph->y_vals[i];     // value from previous trace
};
#endif
bool TScientificGraph::fnChangeXoffset(double dX) // change all X values by adding dX to the most recently added graph if at least 2 graphs defined
{
  if( iNumberOfGraphs<2) return false;   // must be at least 2 graphs to do this
  int j=iNumberOfGraphs-1;
  SGraph *pAGraph = ((SGraph*) pHistory->Items[j]);
  size_t iCount=pAGraph->nos_vals; // current size;
  for (size_t i=0; i<iCount; i++)  // for all items in list add dX to x value
      {
	   pAGraph->x_vals[i]+=dX;
	  }
  return true;    
};


#if 1
float median3(float y0,float y1, float y2);

float median3(float y0,float y1, float y2) // returns median of 3 values  with 2 or 3 comparisons base on Sort3 in Programming Classics page 162
{float t;
 if (y0 > y1)
		{
		 // swap y0 & y1   (both y0 and y1 are used below so actually do need to swap them)
		 t = y0;
		 y0 = y1;
		 y1 = t;
		}

 if (y1 > y2)
		{
		 // swap y1 & y2  (don't need y2 so don't need to actually swap values)
		 // t = y1;
		 y1 = y2;
		 // y2 = t;

		 if (y0 > y1)
			{
			 // swap y0 & y1 (don't need y0 so don't need to actually swap values)
			 y1=y0;
			 // t = y0;
			 // y0 = y1;
			 // y1 = t;
			}
		}
 // y1 always contains the median value
 return y1;
}
#else
float median3(float y0,float y1, float y2) // returns median of 3 values
{if(y1<=y0 && y0<=y2) return y0; // y0 is middle value (median)
 if(y0<=y1 && y1<=y2) return y1; // y1 is middle value
 return y2; // y2 must be middle value
}
#endif


void TScientificGraph::fnMedian_filt(unsigned int median_ahead, int iGraphNumberF ) // apply median filter to graph in place
{// mt=median(mint+,maxt+,mt-1)  - min, max look ahead 2+ . This algorithm provides a fast (if median_ahead is smallish) but good approximation to a true median
 if(median_ahead>1)
		{double m,ymin,ymax;
		 SGraph *pAGraph = ((SGraph*) pHistory->Items[iGraphNumberF]);
		 size_t iCount=pAGraph->nos_vals;
		 m=pAGraph->y_vals[0]; // initial value
		 for (size_t i=0; i<iCount; i++)  // for all items in list
				{
                 if(i+1<iCount)
                        {// find min/max up to median_ahead
						 ymax=ymin=pAGraph->y_vals[i+1];
                         for (unsigned int j=2;j<=median_ahead && i+j<iCount; j++)
								{double t= pAGraph->y_vals[i+j];
                                 if(t>ymax) ymax=t;
                                 else if(t<ymin) ymin=t;
                                }
						 if(m>ymax) m=ymax; // median cal
                         else if(m<ymin) m=ymin; // else m is median
                        }
				 pAGraph->y_vals[i]=(float)m; // put back filtered value
                }
        }
}




#if 1 /* new version of the median 1 filter for 2v6 - this uses "binning" to give a defined accuracy for the median */
/* This implements an standard median filter.
   It will use an exact algorithm if the run-time will not be too large, otherwise it uses an approximation based on a histogram (binning) approach.
   The binning approcah is designed to look indentical to the exact approach when viewed at the default scaling (not zoomed in).
   The implementation is by Peter Miller 20-2-2022
   See the paper "MEDIAN FILTERS THEORY AND APPLICATIONS" by Milan STORK for the definition of a standard median filter
   and the pro's and con's compared to a recursive median filter (implemented as fnMedian_filt_time() below.
*/

 #define NOS_BINS 4096 /* number of bins to use, ideally a (power of 2) -1 to make cache friendly 4095 seems to be a good choice */
 #define MED1MAX_EXACT 10000 /* max number of data points for which we will use exact algorithm (which is slower) */
void TScientificGraph::fnMedian_filt_time1(double median_ahead_t, int iGraphNumberF, void (*callback)(size_t cnt,size_t maxcnt)) // apply median filter to graph in place  , lookahead defined in time
{
 // callback() is called periodically to let caller know progress. This is done based on time (once/sec).
 time_t lastT;
 size_t i,j,k;
 SGraph *pAGraph = ((SGraph*) pHistory->Items[iGraphNumberF]);
 size_t maxi=pAGraph->nos_vals ;
 float miny,maxy,medy;
 float firstx,tmax;
 float *yp=pAGraph->y_vals;
 float *xp=pAGraph->x_vals;// we know this is already sorted into ascending order
 if(median_ahead_t<=0 || maxi<3) // need at least 3 points for initial median and need a positive value for the look ahead time
	return;
 if( (xp[maxi-1]-xp[0])<=median_ahead_t )
	{// just take (exact) median and set all values to this
	 medy=ya_median(pAGraph->y_vals,maxi);
	 rprintf("Median1: Exact median=%g: all y values set to this\n",medy);
	 for(i=0;i<maxi;++i)
		yp[i]=medy;
	 return;
	}
 lastT=clock(); // used to keep callbacks at uniform time intervals
 if(maxi<=MED1MAX_EXACT)
	{ // if only a small number of points calculate medians exactly (this approach is slower than the "binning" approach below).
	 // space for new y values (need old values after new ones are calculated)
	 rprintf("Median1 (standard median filter): using exact algorithm - lookahead = %g seconds\n",median_ahead_t);
	 float *newy=(float *)malloc(maxi*sizeof(float));
	 if(newy==NULL)
		{rprintf("Median1: Not enough ram to calculate median1 (0)\n");
		 return;
		}
	 size_t lasti=0;
	 // now process the values till x gets to the end
	 tmax=xp[maxi-1];
	 size_t firsti=0;
	 for(i=0;i<maxi && xp[i]<=tmax;++i)
		{
		 if(callback!=NULL && (i & 0x3ff)==0 && (clock()-lastT)>= CLOCKS_PER_SEC)
			{lastT=clock();   // update on progress every second (approximately - use i & 0xff to keep average overhead of time() check very low
			 (*callback)(i,maxi); // give user an update on progress
			}
		 firstx=(float)(xp[i]-(median_ahead_t));
		 // subtract values for bin corresponding to those x values that are now too old
		 while(firsti<maxi && xp[firsti]<firstx)
			{
			 firsti++;
			}
		 // now add in new y value(s)
		 while(lasti<maxi && xp[lasti]<=xp[i]+(median_ahead_t))
			{
			 ++lasti;// value time+med_ahead_t
			}
		 medy=ya_median(yp+firsti,lasti-firsti); // calculate required median
		 newy[i]=medy;
		}
	 free(yp);
	 pAGraph->y_vals=newy;// put in new y values
	 return; // all done
	}
 // else approximate median using binning algorithm
 rprintf("Median1 (standard median filter): using approximate \"binning\" algorithm - lookahead = %g seconds\n",median_ahead_t);
 // first find miny/maxy
 miny=maxy=yp[0];
 for(i=1;i<maxi;++i)
	{float y=yp[i];
	 if(y<miny) miny=y;
	 if(y>maxy) maxy=y;
	}
 // rprintf("Median1: miny=%g maxy=%g\n",miny,maxy);
 if(miny==maxy) return; // all values the same, they are all equal to the median so we are done

 // space for new y values (need old values after new ones are calculated)
 float *newy=(float *)malloc(maxi*sizeof(float));
 if(newy==NULL)
	{rprintf("Median1: Not enough ram to calculate median1 (1)\n");
	 return;
	}

 // zero all bins
 size_t *bincounts=(size_t *)calloc(NOS_BINS+1,sizeof(size_t));// allocates memory and sets to all zero
 if(newy==NULL)
	{rprintf("Median1: Not enough ram to calculate median1 (2)\n");
	 free(newy);
	 return;
	}

 const double scalefactor = (double)NOS_BINS/((double)maxy-(double)miny);  // we know maxy!=miny as this was trapped above
 // rprintf("Median1: Width of a bin is %g\n",1.0/scalefactor);
 size_t nos_vals=0;
 size_t lasti=0;

 // now process the values till x gets to the end
 tmax=xp[maxi-1];
 // rprintf("Median1: tmax=%g\n",tmax);
 size_t firsti=0;
 for(i=0;i<maxi && xp[i]<=tmax;++i)
	{
	 if(callback!=NULL && (i & 0xffff)==0 && (clock()-lastT)>= CLOCKS_PER_SEC)
			{lastT=clock();   // update on progress every second (approximately - use i & 0xff to keep average overhead of time() check very low
			 (*callback)(i,maxi); // give user an update on progress
			}
	 firstx=(float)(xp[i]-(median_ahead_t));
	 // subtract values for bin corresponding to those x values that are now too old
	 while(firsti<maxi && xp[firsti]<firstx)
		{size_t bin=(size_t)(((double)yp[firsti]-(double)miny) * scalefactor);
		 if(bincounts[bin]>0 && nos_vals>0 )
			{bincounts[bin]--;
			 nos_vals--; // keep track of number of active values in bins
			}
		 firsti++;
		}
	 //rprintf("firstx=%g firsti=%u about to add in \n",firstx,(unsigned)firsti);
	 // now add in new y value(s)
	 while(lasti<maxi && xp[lasti]<=xp[i]+(median_ahead_t))
		{
		 bincounts[(size_t)(((double)yp[lasti]-(double)miny) * scalefactor)]++;// new bin value
		 nos_vals++; // account for the added value into bins
		 ++lasti;// value time+med_ahead_t
		}
	 j = (nos_vals+1)/2;
	 k=0;
#if 0  /* for debug */
	 if(i<5)
		{rprintf("i=%u: nos_vals=%u j=%u\nBincounts[]=",(unsigned)i,(unsigned)nos_vals,(unsigned)j);
		 for(size_t m=0;m<10;++m)
			rprintf(" [%u]=%u",(unsigned)m,(unsigned)bincounts[m]);
		 rprintf("\n");
		}
#endif
	 while(k<NOS_BINS && j>bincounts[k])
		{
		 j-=bincounts[k]; // j is unsigned, but we already know j>bincounts[k] so this must give a value >0
		 k++;  // k can goto NOS_BINS at which point the loop terminates, this is still a valid index of bincounts[]
		}
	 // rprintf("i=%u: (nos_vals+1)/2=%u j=%u k=%u bincounts[%u]=%u\n",(unsigned)i,(unsigned)((nos_vals+1)/2),(unsigned)j,(unsigned)k,(unsigned)k,(unsigned)bincounts[k]);
	 // approx median of values till  median_ahead_t
	 if(j==bincounts[k])
		{medy=(float)((k+0.5f)/scalefactor+miny);  // 1/2 way point when we are at the end to give a smoother transition
		}
	 else
		{
		 medy=(float)((k)/scalefactor+miny);
		}
	 newy[i]=medy;
	}
 free(yp);
 pAGraph->y_vals=newy;// put in new y values
 free(bincounts);
 return; // all done
}

#else /* the version below was used for the median1 filter till 2v5 */
 // This code was written from scratch 28/12/2019 Peter Miller
 // code looks ahead from the end of the previous lookahead so that portion of the code examines each x value once.
 // knowing starting and ending value of range (from above) we can take equally spaced samples and then another set of samples with a "prime" period to the 1st
 // This uses a linear filter as well as the median - using the linear filter (which looks ahead median_ahead_t) directly unless clipped to stay in the range of the median
 // This also shows a few affects by only sampling on the look ahead - but its probably the best compromise for "median1" as for moderate look aheads it keeps the median in the centre of the "noise band" in regions where y is constant or slowly changing
 //  and for long lookaheads initialisation to the average at the start works well.
 void TScientificGraph::fnMedian_filt_time1(double median_ahead_t, int iGraphNumberF, void (*callback)(size_t cnt,size_t maxcnt)) // apply median filter to graph in place  , lookahead defined in time
{
 // callback() is called periodically to let caller know progress   . This is done based on time (once/sec).
 time_t lastT;
 unsigned int i,j,last_endi=0,endi,jinc ;
 float this_endT,maxy,miny,nearesty,y,m;
 float f,dt; // linear filtered value (range limited based on median limits)
 float k,lastdt=0,x,lastx=0;
 SGraph *pAGraph = ((SGraph*) pHistory->Items[iGraphNumberF]);
 unsigned int maxi=pAGraph->nos_vals ;
 lastT=clock(); // used to keep callbacks at uniform time intervals
 if(median_ahead_t>0 && maxi>=3) // need at least 3 points for initial median and need a positive value for the look ahead time
		{f=m=median3(pAGraph->y_vals[0],pAGraph->y_vals[1],pAGraph->y_vals[2]);
		 //rprintf("initial median of 1st 3 points (%g,%g,%g) selected as %g\n",((SDataPoint*)pAList->Items[0])->dYValue,((SDataPoint*)pAList->Items[1])->dYValue,((SDataPoint*)pAList->Items[2])->dYValue,m);
		 for(i=0;i<maxi;++i)
				{// process all points one by one
				 if(callback!=NULL && (i & 0xffff)==0 && (clock()-lastT)>= CLOCKS_PER_SEC)
						{lastT=clock();   // update on progress every second (approximately - use i & 0xff to keep average overhead of time() check very low
						 (*callback)(i,maxi);
						}
				 this_endT=pAGraph->x_vals[i]+median_ahead_t; // time for the end of the look ahead
				 if(last_endi!=0) lastx=pAGraph->x_vals[last_endi-1];
				 for(endi=last_endi;endi<maxi &&pAGraph->x_vals[endi] < this_endT;++endi) // search forward to find i that matches end of look ahead time
						{ // update linear filter while we do this so it "runs" median_ahead_t in advance of current location so approximately gives average of +/-median_ahead_t around current time
						 if(i!=0) // if i==0 don't do anything here as we will calculate the actual median below and use this to initialise f
								{x= pAGraph->x_vals[endi];
								 dt= x - lastx;
								 lastx=x;
                                 if(dt>0)
										{// above if avoids possible divide by zero below
                                         if(dt!=lastdt) // only calculate k when dt changes [ saves ~ 1 sec ]
												{k=1.0-exp(-(dt)/(2.0*median_ahead_t)); // *2.0 as we want to look forward and back by median_ahead_t
                                                 lastdt=dt;
												}
										 f+=k*(pAGraph->y_vals[endi]-f);
										}
                                }

                        }
				 if(i==0)
						{
						 f=ya_median(pAGraph->y_vals,endi); // calculate median in place (don't change arr).
                        }
				 last_endi=endi; // ensure we don't check values we have already processed
                 if(endi>=maxi)
						{// lookahead goes beyond end of data , in this case we assume the limits are unbounded and leave current median value alone
						  pAGraph->y_vals[i]=f; // put back current filtered value
						  continue;
                        }
				 // now we need to process from i to endi, if this is a lot of points limit to max 63 [range 63 jinc=1 (63 points), range 64 jinc=2 (32 points)  ]
                 jinc=1+(endi-i)/64;  // 64 is a power of 2 for efficiency, but also visibally seems to give sensible results, while being faster than the original code
				 // now search for min,max,nearest y
				 maxy=miny=nearesty= pAGraph->y_vals[i];  // this is 1st value so loop below start on 2nd value

                 for(j=i+jinc;j<maxi && j<endi;j+=jinc)
						{y= pAGraph->y_vals[j];
                         if(y>maxy) maxy=y;
						 if(y<miny) miny=y;
                         if(fabs(y-f)<fabs(nearesty-f)) nearesty=y; // nearest to f
						}
                 if(jinc>1) //  if it was 1 we have already sampled every point, otherwise take a 2nd pass
						{
                         // now make another pass taking more samples (but less numerically than loop above) that are not (very) harmonicaly related to those taken above
						 jinc++;
                         while(jinc&(jinc-1)) jinc&=jinc-1; // decrease jinc by deleting lower order bits until its a power of 2
						 jinc=(jinc<<1)-1; // a power of 2 -1 is frequently prime, at least its unlikley to have common factors with the original jinc used above
                         // for example if jinc was 2 originally on this 2nd pass it will become 3, if it was 3,4,5 or 6  it will become 7
						 for(j=i+jinc;j<maxi && j<endi;j+=jinc)
								{y= pAGraph->y_vals[j];
								 if(y>maxy) maxy=y;
                                 if(y<miny) miny=y;
								 if(fabs(y-m)<fabs(nearesty-m)) nearesty=y;
                                }
						}
                 // approximate median must be between miny and maxy
				 if(f<miny)
                        {f=m=miny;
						}
                 else
					   {if(f>maxy)
                            {f=m=maxy;
							}
                        else
							{m=f; //  use filtered value directly
                            }
					   }
				 pAGraph->y_vals[i]=m; // put back filtered value
				}
         }
 }
#endif

#if 1 /* new recursive median filter for 2v7, if 0 use earlier (approximate) version */
/* This implements an exact recursive median filter, but if the execution time starts to get large it swaps to an approximate sampling scheme
   The implementation is by Peter Miller 22-3-2022
   See the paper "MEDIAN FILTERS THEORY AND APPLICATIONS" by Milan STORK for the definition of a recursive median filter
   and the pro's and con's compared to a standard median filter (implemented as fnMedian_filt_time1() above
*/
void TScientificGraph::fnMedian_filt_time(double median_ahead_t, int iGraphNumberF, void (*callback)(size_t cnt,size_t maxcnt)) // apply median filter to graph in place  , lookahead defined in time
{
 // callback() is called periodically to let caller know progress   . This is done based on time (once/sec).
 time_t lastT;
 size_t i,j,k,lasti=0;
 SGraph *pAGraph = ((SGraph*) pHistory->Items[iGraphNumberF]);
 size_t maxi=pAGraph->nos_vals ;
 unsigned int time_taken_secs=0;
 float miny,maxy,medy;
 size_t miny_pos=0,maxy_pos=0;// positions of min/max
 // float firstx,tmax;
 float *yp=pAGraph->y_vals;
 float *xp=pAGraph->x_vals;// we know this is already sorted into ascending order
 if(median_ahead_t<=0 || maxi<3) // need at least 3 points for initial median and need a positive value for the look ahead time
	return;
 if( (xp[maxi-1]-xp[0])<=median_ahead_t )
	{// just take (exact) median and set all values to this
	 medy=ya_median(pAGraph->y_vals,maxi);
	 rprintf("Median: Exact median=%g: all y values set to this\n",medy);
	 for(i=0;i<maxi;++i)
		yp[i]=medy;
	 return;
	}
 lastT=clock(); // used to keep callbacks at uniform time intervals
 miny=maxy=medy=yp[0];   // initial value
 // y[n]=medy=median(medy,maxy[n->n+lookahead],miny[n->n+lookahead]
 rprintf("Median: using exact algorithm for recursive median filter - lookahead=%g seconds\n",median_ahead_t);
 // now process the whole array, each time looking ahead median_ahead_t
 bool exact_median_cals=true;
 for(i=0;i<maxi ;++i)
		{
		 if(callback!=NULL && (i & 0xff)==0 && (clock()-lastT)>= CLOCKS_PER_SEC)
			{
			 (*callback)(i,maxi); // give user an update on progress
			  time_taken_secs+=(unsigned int)((clock()-lastT)/CLOCKS_PER_SEC);// this could be well over 1 sec if the loops are very slow
			  if(exact_median_cals && time_taken_secs>=3 )
				{exact_median_cals=false;
				 rprintf("Median: exact calculations are taking a long time so swapping to an approximate (sampling) algorithm\n");
				}
			 lastT=clock();   // update on progress every second (approximately - use i & 0xff to keep average overhead of time() check very low
			}
		 /* look ahead finding local min/max */
		 float lmaxy,lminy;
		 size_t lmaxy_pos,lminy_pos,start_lasti;
		 if(lasti>=maxi) lasti=maxi-1;// make sure we don't go past the end of the array
		 lmaxy=lminy=yp[lasti];
		 lmaxy_pos=lminy_pos=lasti;
		 start_lasti=lasti;
		 if(lasti+1<maxi && xp[lasti+1]<=xp[i]+(median_ahead_t) )
			{
			 for(lasti++;lasti<maxi && xp[lasti]<=xp[i]+(median_ahead_t);++lasti)
				{// look ahead from end of previous lookahead, updating min/max if we can
				 float t=yp[lasti];
				 if(t>=lmaxy)
					{lmaxy=t; // update max and min. Use >= as we want to update _pos on equal
					 lmaxy_pos=lasti;
					}
				 if(t<=lminy)
					{lminy=t;
					 lminy_pos=lasti;
					}
				}
			}
		 if(lmaxy>=maxy) // we found a new max during lookahead
			{maxy=lmaxy;
			 maxy_pos=lmaxy_pos;
			}
		 if(lminy<=miny) // we found a new min during lookahead
			{miny=lminy;
			 miny_pos=lminy_pos;
			}
		 if(maxy_pos<i || miny_pos<i)
			{// need to recalculate min/max as previous values are too old
			 maxy=miny=yp[i];
			 maxy_pos=miny_pos=i;
			 k=1; // k is increment. If 1 all values are used (exact calculation) if k>1 data is sampled and we have an approximate algorithm
			 if(time_taken_secs<=2)
				k=1;// for 1st (almost) 3 seconds exact calculation - look at all values
			 else if(time_taken_secs<=3)
				k=2; // 3-4 secs increase increment so we sample elements rather than examine them all (calculation is now approximate)
			 else if(time_taken_secs<20)
				k=(start_lasti-i)/((20-time_taken_secs)*5); // take a progressively smaller sample the longer it takes
			 else k=(start_lasti-i)/5; // trap very long execution times, k=0 might also happen due to divides above
			 if(k!=0)
			  {// if k=0 use yp[i] (1st value) only - thats already set above,otherwise loop to get min/max with k defining sample size
			   for(j=i+1; j<start_lasti && j<maxi ;j+=k )// only loop to start_lasti as we have already got results beyond there
				{float t=yp[j];
				 if(t>=maxy)
					{maxy=t; // update max and min
					 maxy_pos=j;
					}
				 if(t<=miny)
					{miny=t;
					 miny_pos=j;
					}
				}
			  }
			 if(lmaxy>=maxy) // we found a new max during lookahead
				{maxy=lmaxy;
				 maxy_pos=lmaxy_pos;
				}
			 if(lminy<=miny) // we found a new min during lookahead
				{miny=lminy;
				 miny_pos=lminy_pos;
				}
			}
		 /* yp[i]=medy=median3(medy,maxy,miny), but we know maxy>miny so we can optimise calculation:*/
		 if(medy>maxy) medy=maxy;
		 else if(medy<miny) medy=miny;
		 yp[i]=medy;
		}
  return; // all done

 }
#else  /* approximate a recursive median filter */
// This code was written from scratch 28/12/2019 Peter Miller
 // code looks ahead from the end of the previous lookahead so that portion of the code examines each x value once.
 // knowing starting and ending value of range (from above) we can take equally spaced samples and then another set of samples with a "prime" period to the 1st
 //
 //
 void TScientificGraph::fnMedian_filt_time(double median_ahead_t, int iGraphNumberF, void (*callback)(size_t cnt,size_t maxcnt)) // apply median filter to graph in place  , lookahead defined in time
{
 // callback() is called periodically to let caller know progress   . This is done based on time (once/sec).
 time_t lastT;
 unsigned int i,j,last_endi=0,endi,jinc ;
 float this_endT,maxy,miny,y,m;
 SGraph *pAGraph = ((SGraph*) pHistory->Items[iGraphNumberF]);
 unsigned int maxi=pAGraph->nos_vals ;
 lastT=clock(); // used to keep callbacks at uniform time intervals
 if(median_ahead_t>0 && maxi>=3) // need at least 3 points for initial median and need a positive value for the look ahead time
		{m=median3(pAGraph->y_vals[0],pAGraph->y_vals[1],pAGraph->y_vals[2]);
         //rprintf("initial median of 1st 3 points (%g,%g,%g) selected as %g\n",((SDataPoint*)pAList->Items[0])->dYValue,((SDataPoint*)pAList->Items[1])->dYValue,((SDataPoint*)pAList->Items[2])->dYValue,m);
         for(i=0;i<maxi;++i)
                {// process all points one by one
                 if(callback!=NULL && (i & 0xffff)==0 && (clock()-lastT)>= CLOCKS_PER_SEC)
                        {lastT=clock();   // update on progress every second (approximately - use i & 0xff to keep average overhead of time() check very low
                         (*callback)(i,maxi);
                        }
				 this_endT=pAGraph->x_vals[i]+median_ahead_t; // time for the end of the look ahead
				 for(endi=last_endi;endi<maxi && pAGraph->x_vals[endi] < this_endT;++endi); // search forward to find i that matches end of look ahead time
				 if(i==0)
						{
						 m=ya_median(pAGraph->y_vals,endi); // calculate median in place (don't change arr).
						}
				 last_endi=endi; // ensure we don't check values we have already processed
				 if(endi>=maxi)
						{// lookahead go beyond end of data , in this case we assume the limits are unbounded and leave current median value alone
						 pAGraph->y_vals[i]=m; // put back current median value
						  continue;
						}
				 // now we need to process from i to endi, if this is a lot of points limit to max 63 [range 63 jinc=1 (63 points), range 64 jinc=2 (32 points)  ]
				 jinc=1+(endi-i)/64;  // 64 is a power of 2 for efficiency, but also visibally seems to give sensible results, while being faster than the original code

				 // now search for min,max
				 maxy=miny= pAGraph->y_vals[i]; // this is 1st value so loop below start on 2nd value
				 for(j=i+jinc;j<maxi && j<endi;j+=jinc)
						{y= pAGraph->y_vals[j];
						 if(y>maxy) maxy=y;
						 if(y<miny) miny=y;
						}
				 if(jinc>1) //  if it was 1 we have already sampled every point, otherwise take a 2nd pass to add extra samples
						{
						 // now make another pass taking more samples (but less numerically than loop above) that are not (very) harmonicaly related to those taken above
						 jinc++;
						 while(jinc&(jinc-1)) jinc&=jinc-1; // decrease jinc by deleting lower order bits until its a power of 2
						 jinc=(jinc<<1)-1; // a power of 2 -1 is frequently prime, at least its unlikley to have common factors with the original jinc used above
						 // for example if jinc was 2 originally on the 2nd pass it will become 3, if it was 3,4,5 or 6  it will become 7
						 for(j=i+jinc;j<maxi && j<endi;j+=jinc)
								{y= pAGraph->y_vals[j];
								 if(y>maxy) maxy=y;
								 if(y<miny) miny=y;
								}
						}
				 // approximate median m must be between miny and maxy
				 if(m<miny) m=miny;
				 if(m>maxy) m=maxy;
				 pAGraph->y_vals[i]=m; // put back filtered value
				}
		 }
 }
#endif

void TScientificGraph::fnLinear_filt_time(double tc, int iGraphNumberF, void (*callback)(size_t cnt,size_t maxcnt)) // apply linear filter to graph in place
{// f(t)+=k*(y(t)+f(t-1))
 // k=1-exp(-dt/tc) where dt is time between samples
 // callback() is called periodically to let caller know progress
 if(tc>0)
		{double m;
		 double lastx,x,y,k;
		 SGraph *pAGraph = ((SGraph*) pHistory->Items[iGraphNumberF]);
		 size_t iCount=pAGraph->nos_vals ;
		 if(iCount<2) return; // not enough data in graph to process
		 m=pAGraph->y_vals[0]; // initial value
		 lastx=pAGraph->x_vals[0];
		 for (unsigned int i=0; i<iCount; i++)  // for all items in list
                {
				 y=pAGraph->y_vals[i];
				 x=pAGraph->x_vals[i];
                 if(x>lastx)
                        {// above if avoids possible maths error below
                         k=1.0-exp(-(x-lastx)/tc);
                         m+=k*(y-m);
                        }
                 lastx=x;
				 if(callback!=NULL && (i & 0x3fffff)==0)
						(*callback)(i,iCount); // update on progress
				 pAGraph->y_vals[i]=(float)m; // put back filtered value
				}
        }
}

size_t TScientificGraph::fnGetxyarr(float **x_arr,float **y_arr,int iGraphNumberF)
 // allow access to x and y arrays of specified graph, returns nos points
 {SGraph *pAGraph = ((SGraph*) pHistory->Items[iGraphNumberF]);
  size_t iCount=pAGraph->nos_vals ;
  *x_arr=pAGraph->x_vals;
  *y_arr=pAGraph->y_vals;
  return iCount;
 }

void deriv_trace(int iGraph); // in UDataPlotWindow.cpp
void TScientificGraph::deriv_filter(unsigned int diff_order,int iGraphNumberF)
{ // take derivative of specified trace .
  //  uses 17 point Savitzky Golay algorithm
  // needs to create a new array for results as uses points either side of index to calculate derivative
  float *x_arr,*y_arr;
  SGraph *pAGraph = ((SGraph*) pHistory->Items[iGraphNumberF]);
  size_t iCount=pAGraph->nos_vals ;
  float *newy=(float *)malloc(iCount*sizeof(float));
  if(newy==NULL)
		{rprintf("deriv_filter: Not enough ram to calculate filtered derivative, using unfiltered derivative\n");
		 deriv_trace(iGraphNumberF) ;
		 return;
		}
  x_arr=pAGraph->x_vals;
  y_arr=pAGraph->y_vals;
  for(size_t i=0;i<iCount;++i)
	{
	 newy[i]=(float)dy_dx17(y_arr, x_arr,0,iCount-1, i,diff_order );  // calculate derivative at point "i"
	}
  free(y_arr); // delete original y values
  pAGraph->y_vals=newy;// put in new y values
  return; // all done
}

void TScientificGraph::deriv2_filter(unsigned int diff_order,int iGraphNumberF)
{ // take 2nd derivative of specified trace .
  //  uses 25/17 point Savitzky Golay algorithm
  // needs to create a new array for results as uses points either side of index to calculate derivative
  float *x_arr,*y_arr;
  SGraph *pAGraph = ((SGraph*) pHistory->Items[iGraphNumberF]);
  size_t iCount=pAGraph->nos_vals ;
  float *newy=(float *)malloc(iCount*sizeof(float));
  if(newy==NULL)
		{rprintf("deriv2_filter: Not enough ram to calculate filtered 2nd derivative - no filter applied\n");
		 return;
		}
  x_arr=pAGraph->x_vals;
  y_arr=pAGraph->y_vals;
  for(size_t i=0;i<iCount;++i)
	{
#if 1
	 newy[i]=(float)d2y_d2x25(y_arr, x_arr,0,iCount-1, i,diff_order );  // calculate 2nd derivative at point "i"
#else
	 newy[i]=(float)d2y_d2x17(y_arr, x_arr,0,iCount-1, i,diff_order );  // calculate 2nd derivative at point "i"
#endif
	}
  free(y_arr); // delete original y values
  pAGraph->y_vals=newy;// put in new y values
  return; // all done
}

void TScientificGraph::Savitzky_Golay_smoothing(unsigned int s_order,int iGraphNumberF) // Savitzky Golay smoothing
{ // Savitzky Golay smoothing of specified trace fitting a polynomial of specified order
  //  uses 25/17 point Savitzky Golay algorithm
  // needs to create a new array for results as uses points either side of index to calculate filtered value
  float *x_arr,*y_arr;
  SGraph *pAGraph = ((SGraph*) pHistory->Items[iGraphNumberF]);
  size_t iCount=pAGraph->nos_vals ;
  float *newy=(float *)malloc(iCount*sizeof(float));
  if(newy==NULL)
		{rprintf("Savitzky Golay smoothing: Not enough ram to calculate filtered result\n");
		 return;
		}
  x_arr=pAGraph->x_vals;
  y_arr=pAGraph->y_vals;
  for(size_t i=0;i<iCount;++i)
	{
#if 1
     newy[i]=(float)Savitzky_Golay_smoothing25(y_arr, x_arr,0,iCount-1, i,s_order );  // calculate smoothed value at point "i"
#else
	 newy[i]=(float)Savitzky_Golay_smoothing17(y_arr, x_arr,0,iCount-1, i,s_order );  // calculate smoothed value at point "i"
#endif
	}
  free(y_arr); // delete original y values
  pAGraph->y_vals=newy;// put in new y values
  return; // all done
}

void TScientificGraph::Spline_smoothing(double tc,int iGraphNumberF) // Smoothing spline smoothing "tc" is a number 0..1
{ // Smoothing spline smoothing of specified trace
  SGraph *pAGraph = ((SGraph*) pHistory->Items[iGraphNumberF]);
  // void SmoothingSpline( s_spline_float *x, s_spline_float *y, s_spline_float *yo, size_t _n, double lambda);
  SmoothingSpline(pAGraph->x_vals, pAGraph->y_vals,NULL, pAGraph->nos_vals,tc);  // does all the hard work!
  return; // all done
}

void TScientificGraph::fnLinreg_origin( int iGraphNumberF, void (*callback)(size_t cnt,size_t maxcnt))
{ // straight line passing through origin    y=m*x
  // underlying equation for the best straight line through the origin=sum(XiYi)/sum(Xi^2) from Yang Feng (Columbia Univ) Simultaneous Inferences, pp 18/20.
 SGraph *pAGraph = ((SGraph*) pHistory->Items[iGraphNumberF]);
 size_t iCount=pAGraph->nos_vals ;
 double meanx2=0,meanxy=0; /* mean x^2 , mean x*y */
 double xi,yi;
 double m;
 double maxe=0; // max abs error of fit
 double e;
 size_t i,N=0; /* N is count of items */
 if(iCount<2) return; // not enough data in graph to process
 for(i=0;i<iCount;++i) /* only use 1 pass here - to calculate means directly */
		{++N;
		 xi=pAGraph->x_vals[i];
		 yi=pAGraph->y_vals[i];
		 meanx2+= (xi*xi-meanx2)/(double) N;
		 meanxy+= (xi*yi-meanxy)/(double) N;
		 if(callback!=NULL && (i & 0x3fffff)==0)
			(*callback)(i>>1,iCount); // update on progress  (this is 1st pass so go 0-50%)
		}
 if(meanx2==0)
		{/* y is independent of x [if this is not trapped we get a divide by zero error trying to set m to infinity] or 0 or 1 point given */
		 m=0.0 ;
		}
 else   {/* have a valid line */
		 m=(meanxy)/(meanx2);
		 }
 rprintf("Best Least squares straight line that passes through the origin is Y=%g*X\n",m);
 // now put new y values back, calculated as y=m*x+c
 for(i=0;i<iCount;++i)
	{yi=pAGraph->y_vals[i];
	 xi=pAGraph->x_vals[i];
	 try{ // code below has tests for common issues, but use try to catch anything else
		 pAGraph->y_vals[i]=(float)(m*xi);
		 e=fabs(yi-pAGraph->y_vals[i]);
		 if(e>maxe) maxe=e;
		}
	 catch(...)
		{ pAGraph->y_vals[i]=0; // if something goes wrong put 0 in as a placeholder.
		}

	 if(callback!=NULL && (i & 0x3fffff)==0)
		(*callback)((i>>1) + (iCount>>1),iCount); // update on progress , this is 2nd pass through data so goes 50% - 100%
	}
 rprintf("  Max abs error of above curve is %g\n",maxe);
}

void TScientificGraph::fnLinreg_abs(bool rel, int iGraphNumberF, void (*callback)(size_t cnt,size_t maxcnt))
{  // fit y=mx+c with either min abs error or min abs rel error
 SGraph *pAGraph = ((SGraph*) pHistory->Items[iGraphNumberF]);
 size_t iCount=pAGraph->nos_vals ;
 if(iCount<2) return; // not enough data in graph to process
 size_t i;
 double m,c,best_err;
 double yi,xi,e,maxe=0;
 float *x_arr=pAGraph->x_vals ; // x values
 float *y_arr=pAGraph->y_vals; // y values

 // void fit_min_abs_err_line(float *x, float *y,unsigned int nos_vals,bool rel_error,double *m_out, double *c_out,double *best_err_out)
 fit_min_abs_err_line(x_arr, y_arr,iCount,rel,&m,&c,&best_err,callback);  // do all the hard work ...

 // now put new y values back, calculated as y=m*x+c
 for(i=0;i<iCount;++i)
	{yi=y_arr[i];
	 xi=x_arr[i];
	 try{ // code below has tests for common issues, but use try to catch anything else
		 y_arr[i]=(float)(m*xi+c);
		 e=fabs(yi-y_arr[i]);
		 if(e>maxe) maxe=e;
		}
	 catch(...)
		{ y_arr[i]=0; // if something goes wrong put 0 in as a placeholder.
		}
	}
 if(rel)
	rprintf("Best min abs relative error straight line is Y=%g*X%+g (error=%g%%)\n",m,c,100.0*best_err);
  else
	rprintf("Best min abs error straight line is Y=%g*X%+g (error=%g)\n",m,c,best_err);
 rprintf("  Max abs error of above curve is %g\n",maxe);
}

static double fun_x(float xparam)
{return xparam;     // return X
}

static double fun_sqrt(float xparam)
{if(xparam<=0) return 0;
 return sqrt((double)xparam);   // return sqrt(X) if x>=0 else 0
}

void TScientificGraph::fnLinreg_3(int iGraphNumberF, void (*callback)(size_t cnt,size_t maxcnt))
{ // fit y=a*x+b*sqrt(x)+c
 SGraph *pAGraph = ((SGraph*) pHistory->Items[iGraphNumberF]);
 size_t iCount=pAGraph->nos_vals ;
 if(iCount<2) return; // not enough data in graph to process
 unsigned int i;
 double yi,xi,e,maxe=0;
 float *x_arr=pAGraph->x_vals ; // x values
 float *y_arr=pAGraph->y_vals; // y values
 double a,b,c;     // coefficients of equation
 // void leastsquares_reg3(float *y,float *x,int start, int end,double (*f)(float xparam),double (*g)(float xparam), double *a, double *b, double *c)
 leastsquares_reg3(y_arr, x_arr,0,iCount-1,fun_x,fun_sqrt,&a,&b,&c);  // do all the hard work ...
 // put new values back  y=a*x+b*sqrt(x)+c
 for(i=0;i<iCount;++i)
	{yi=y_arr[i];
	 xi=x_arr[i];
	 try{ // code below has tests for common issues, but use try to catch anything else
		 y_arr[i]=(float)(a*fun_x((float)xi)+b*fun_sqrt((float)xi)+c);
		 e=fabs(yi-y_arr[i]);
		 if(e>maxe) maxe=e;
		}
	 catch(...)
		{ y_arr[i]=0; // if something goes wrong put 0 in as a placeholder.
		}
	 if(callback!=NULL && (i & 0x3fffff)==0)
		(*callback)((i),iCount); // update on progress
	}
 rprintf("Best fit found is Y=%g*X%+g*sqrt(X)%+g\n",a,b,c);    // %+g always prints sign [+-]
 rprintf("  Max abs error of above curve is %g\n",maxe);
}


void TScientificGraph::fnrat_3(int iGraphNumberF, void (*callback)(size_t cnt,size_t maxcnt))
{ // fits y=(a+bx)/(1+cx)
 SGraph *pAGraph = ((SGraph*) pHistory->Items[iGraphNumberF]);
 size_t iCount=pAGraph->nos_vals ;
 if(iCount<2) return; // not enough data in graph to process
 unsigned int i;
 double yi,xi,e,maxe=0;
 float *x_arr=pAGraph->x_vals ; // x values
 float *y_arr=pAGraph->y_vals; // y values
 double a,b,c;     // coefficients of equation
 // void leastsquares_rat3(float *y,float *x,int start, int end, double *a, double *b, double *c); /* fits y=(a+bx)/(1+cx) */
 leastsquares_rat3(y_arr, x_arr,0,iCount-1,&a,&b,&c);  // do all the hard work ...
 // put new values back  y=(a+bx)/(1+cx)
 for(i=0;i<iCount;++i)
	{yi=y_arr[i];
	 xi=x_arr[i];
	 try{ // code below has tests for common issues, but use try to catch anything else
		 y_arr[i]=(float)((a+b*xi)/(1.0+c*xi));
		 e=fabs(yi-y_arr[i]);
		 if(e>maxe) maxe=e;
		}
	 catch(...)
		{ y_arr[i]=0; // if something goes wrong put 0 in as a placeholder.
		}
	 if(callback!=NULL && (i & 0x3fffff)==0)
		(*callback)((i),iCount); // update on progress
	}
 rprintf("Best fit found is Y=(%g%+g*X)/(1.0%+g*X)\n",a,b,c);    // %+g always prints sign [+-]
 rprintf("  Max abs error of above curve is %g\n",maxe);
}

void TScientificGraph::fnLinreg(enum LinregType type, int iGraphNumberF, void (*callback)(size_t cnt,size_t maxcnt))
 // apply 1st order least squares linear regression (y=mx+c) to graph in place
 // can also do GMR for a straight line when type = LinLin_GMR
 // enum LinregType  {LinLin,LogLin,LinLog,LogLog,RecipLin,LinRecip,RecipRecip,SqrtLin}; defines preprocessing of variables before linear regression 1st is X 2nd is Y
 // results checked using csvfun3.csv. R^2 values (and coefficients) also checked against Excel for the fits excel can do.
{// to save copying data this is done inline
 SGraph *pAGraph = ((SGraph*) pHistory->Items[iGraphNumberF]);
 size_t iCount=pAGraph->nos_vals ;
 double meanx=0,meany=0; /* initial values set to mean that N=0 or N=1 do not need to be treated as special cases below */
 double meanx2=0,meanxy=0,meany2=0; /* mean x^2 , mean x*y and mean y^2 */
 double xi,yi;
 double m,c,r2; // results of least squares fit
 double maxe=0; // max abs error of fit
 double e;
 size_t i,N=0; /* N is count of items */
 if(iCount<2) return; // not enough data in graph to process
 for(i=0;i<iCount;++i) /* only use 1 pass here - to calculate means directly */
		{
		 xi=pAGraph->x_vals[i];
		 yi=pAGraph->y_vals[i];
		 // apply "preprocessing"
		 if(type== LogLin || type== LogLog)
			{if(xi<=0) continue;// log requires value >0
			 xi=log(xi);
			}
		 if(type== LinLog || type==LogLog)
			{if(yi<=0) continue;// log requires value >0
			 yi=log(yi);
			}
		 if(type== RecipLin || type==RecipRecip)
			{if(xi==0 ) continue; // avoid divide by zero
			 xi=1.0/xi;
			}
		 if(type== LinRecip || type==RecipRecip)
			{if( yi==0) continue; // avoid divide by zero
			 yi=1.0/yi;
			}
		 if(type== SqrtLin)
			{if(xi<0 ) continue; // avoid sqrt of a negative number
			 xi=sqrt(xi);
			}
		 if(type== Nlog2nLin)
			{if(xi<=0 ) continue; // avoid ln of a negative number
			 // M_LOG2E 1.44269504088896340736 is defined in math.h // 1/ln(2) as multiplication below is probably faster than division
			 xi=xi*log(xi)*M_LOG2E;    // xi*log2(xi) = xi*ln(xi)/ln(2)
			}
		 ++N;
         meanx+= (xi-meanx)/(double) N; /* calculate means as mi+1=mi+(xi+1 - mi)/i+1 , this should give accurate results and avoids the possibility of the "sum" overflowing*/
         meany+= (yi-meany)/(double) N;
         meanx2+= (xi*xi-meanx2)/(double) N;
         meanxy+= (xi*yi-meanxy)/(double) N;
		 meany2+= (yi*yi-meany2)/(double) N;
		 if(callback!=NULL && (i & 0x3fffff)==0)
			(*callback)(i>>1,iCount); // update on progress  (this is 1st pass so go 0-50%)
		}
 if(meanx*meanx==meanx2)
        {/* y is independent of x [if this is not trapped we get a divide by zero error trying to set m to infinity] or 0 or 1 point given */
		 m=0.0 ;
		 c=meany;
		 r2=0.0;
		}
 else   {/* have a valid line */
		 double rt,rb,rm;
		 rm=(meanx*meany-meanxy)/(meanx*meanx-meanx2);
		 if(type!=LinLin_GMR)
			{m=rm; // normal least squares regression
			}
		 else
			{
			  /* Geometric mean regression (GMR) also called Triangular regression see equation 18 in
				"Least Squares Methods for Treating Problems with Uncertainty in x and y", Joel Tellinghuisen,
				Anal. Chem. 2020, 92, 10863-10871.
			  */
			 m=sqrt((meany*meany-meany2)/(meanx*meanx-meanx2));
			 if(meanxy<0) m= -m;
			}
		 c=meany-m*meanx; /* y=mx+c so c=y-mx */
		 rt=(meanxy-meanx*meany);
		 rb=(meany2-meany*meany);
		 // rprintf("rm=%g rt=%g rb=%g rt/rb=%g \n",rm,rt,rb,rt/rb);
		 if(rb!=0)     /* trap divide by zero */
			r2= rm * (rt/rb) ;
		  else
			r2=1.0;/* should be in range 0-1 */
		 }
 switch(type)
	{ // enum LinregType  {LinLin,LogLin,LinLog,LogLog,RecipLin,LinRecip,RecipRecip,SqrtLin};
	  // %+g always prints sign which looks better than +%g which prints +-1.23 for negative numbers.
	  // Use %g rather than %.12g as accuracy of coefficients not so critical here compared to higher order polynomials
	 case LinLin:
		rprintf("Best Least squares straight line is Y=%g*X%+g which has an R^2 of %g\n",m,c,r2);
		break;
	 case LinLin_GMR:
		rprintf("Best GMR straight line is Y=%g*X%+g which has an R^2 of %g\n",m,c,r2);
		break;
	 case LogLin:
		// log(x) : y=m*log(x)+c
		rprintf("Best Least squares curve is Y=%g*log(X)%+g which has an R^2 of %g\n",m,c,r2);
		break;
	case LinLog:
		// Exponential: log(y) : y=a*b^x ; log(y)=log(a)+(log b)*x   ; OR y=a*exp(b*x)  =>log(y)=log(a)+b*x
		c=exp(c); //  transform depends on equation used
		rprintf("Best Least squares curve is Y=%g*%g^(X) OR y=%g*exp(%g*X) which has an R^2 of %g\n",c,exp(m),c,m,r2);
		break;
	case LogLog:
		// Power: Log(x) log(y) : y=a*x^b ; log(y)=log(a)+b*log(x)
		c=exp(c);
		rprintf("Best Least squares curve is Y=%g*X^%g which has an R^2 of %g\n",c,m,r2);
		break;
	 case RecipLin:
		// 1/x : y=m/x+c
		rprintf("Best Least squares curve is Y=%g/X%+g which has an R^2 of %g\n",m,c,r2);
		break;
	case LinRecip:
		// 1/y : 1/y=m*x+c ;y=1/(m*x+c)
		rprintf("Best Least squares curve is Y=1/(%g*X%+g) which has an R^2 of %g\n",m,c,r2);
		break;
	case RecipRecip:
		// Hyperbolic: 1/x,1/y : 1/y=m/x+c ;y=x/(m+c*x)
		rprintf("Best Least squares curve is Y=X/(%g%+g*X) which has an R^2 of %g\n",m,c,r2);
		break;
	 case SqrtLin:
		// sqrt(x): y=m*sqrt(x)+c
		rprintf("Best Least squares curve is Y=%g*sqrt(X)%+g which has an R^2 of %g\n",m,c,r2);
		break;
	 case Nlog2nLin:
		// n*log2(n): y=m*x*log2(x)+c
		rprintf("Best Least squares curve is Y=%g*X*log2(X)%+g which has an R^2 of %g\n",m,c,r2); // log base 2
		rprintf("Best Least squares curve is Y=%g*X*log(X)%+g\n",m*M_LOG2E,c); // log base e
		break;
	}
 // now put new y values back, calculated as y=m*x+c
 for(i=0;i<iCount;++i)
	{yi=pAGraph->y_vals[i];
	 xi=pAGraph->x_vals[i];
	 try{ // code below has tests for common issues, but use try to catch anything else
	  switch(type)
		{ // enum LinregType  {LinLin,LogLin,LinLog,LogLog,RecipLin,LinRecip,RecipRecip,SqrtLin};
		 case LinLin:    // fall through...
		 case LinLin_GMR:
			pAGraph->y_vals[i]=(float)(m*xi+c);
			break;
		 case LogLin:
			// log(x) : y=m*log(x)+c       log(minfloat)=-103.28 so use -104 for negative/zero
			pAGraph->y_vals[i]=(float)(m*(xi>0?log(xi):-104.0f)+c);
			break;
		 case LinLog:
			// Exponential: log(y) : y=a*b^x ; log(y)=log(a)+(log b)*x   ; OR y=a*exp(b*x)  =>log(y)=log(a)+b*x
			pAGraph->y_vals[i]=(float)(c*exp(m*xi));// this might overflow which is not trapped here
			break;
		 case LogLog:
			// Power: Log(x) log(y) : y=a*x^b ; log(y)=log(a)+b*log(x)
			if(xi>0) pAGraph->y_vals[i]=(float)(c*pow(xi,m));
			else pAGraph->y_vals[i]=0;
			break;
		 case RecipLin:
			// 1/x : y=m/x+c
			pAGraph->y_vals[i]=(float)(m*(xi==0? 0.0 : 1.0f/xi)+c);
			break;
		 case LinRecip:
			// 1/y : 1/y=m*x+c ;y=1/(m*x+c)
			if(m*xi+c==0) pAGraph->y_vals[i]=0.0;
			else pAGraph->y_vals[i]=(float)(1.0/(m*xi+c));
			break;
		 case RecipRecip:
			// Hyperbolic: 1/x,1/y : 1/y=m/x+c ;y=x/(m+c*x)
			if(m+c*xi==0) pAGraph->y_vals[i]=0.0;
			else pAGraph->y_vals[i]=(float)(xi/(m+c*xi));
			break;
		 case SqrtLin:
			// sqrt(x): y=m*sqrt(x)+c
			pAGraph->y_vals[i]=(float)(m*(xi>=0?sqrt(xi):0.0f)+c);
			break;
		 case Nlog2nLin:
			// n*log2(n): y=m*x*log2(x)+c
			pAGraph->y_vals[i]=(float)(m*(xi>0?xi*log(xi)*M_LOG2E:0.0f)+c);
			break;
		}
	   e=fabs(yi-pAGraph->y_vals[i]);
	   if(e>maxe) maxe=e;
	   }
	 catch(...)
		{ pAGraph->y_vals[i]=0; // if something goes wrong put 0 in as a placeholder.
		}

	 if(callback!=NULL && (i & 0x3fffff)==0)
		(*callback)((i>>1) + (iCount>>1),iCount); // update on progress , this is 2nd pass through data so goes 50% - 100%
	}
 rprintf("  Max abs error of above curve is %g\n",maxe);
}


bool TScientificGraph::fnPolyreg(unsigned int order,int iGraphNumberF, void (*callback)(size_t cnt,size_t maxcnt))
	// fit polynomial of specified order regression to graph in place
	// does least squares fit using orthogonal polynomials to minimise errors
	// Loosely based on algorithm in section 12.3 of Programming Classics by Ian Oliver.
	// This implementation by Peter Miller
	//  - added prescaling for x and y as otherwise numbers can become very big...
	//  - removed need for arrays the same size as the x and y values (which dramatically reduces ram needed)
	//  - uses "symbolic execution" to get coefficients for conventional polynomial
	// returns true if works, false if an issue found
{
 SGraph *pAGraph = ((SGraph*) pHistory->Items[iGraphNumberF]);
 size_t iCount=pAGraph->nos_vals ;
 double x,y;  // need to be double as we scale floats
 long double divisor,previous;
 float minx,maxx,miny,maxy;
 double ym,yc,xm,xc; // a=m*Z+c where a is limited to +/-1 which minimises range of numbers in the calculations making them more robust
 unsigned int i,j;
 bool failed=false; // assume things go OK
 long double *sx=(long double *)calloc(order+1,sizeof(long double));// sum of products involving x - calloc initialises to 0
 long double *sy=(long double *)calloc(order+1,sizeof(long double)); // sum of products involving x & y
 long double *v=(long double *)calloc(order+1,sizeof(long double)); // orthogonal polynomial coefficient
 long  double pv,qv;
 if(v==NULL)
	{
	 if(sy!=NULL) free(sy);
	 if(sx!=NULL) free(sx);
	 rprintf("Polynomial fit failed - not enough free RAM\n");
	 return false;
	}
 if((size_t)order> (iCount>>1))
	{order=(unsigned int)(iCount>>1); // imperical observation is order > 1/2 total number of points then bad things happen numerically, so trap that here
	}
 try{ /* the code below may fail if given a high enough order so trap that here */
	minx=pAGraph->x_vals[0];   // xvalues are sorted into order
	maxx=pAGraph->x_vals[iCount-1];
	miny=maxy=pAGraph->y_vals[0];
	for(i=0;i<iCount;++i)
		{y=pAGraph->y_vals[i];
		 if(y>maxy) maxy=(float)y;
		 if(y<miny) miny=(float)y;
		}
	rprintf("Polyfit: %u points, x from %g  to %g y from %g to %g\n",iCount,minx,maxx,miny,maxy);
	// rprintf("sizeof(float)=%u, sizeof(double)=%u, sizeof(long double)=%u\n",sizeof(float), sizeof(double), sizeof(long double));
	// calculate scaling factors to get all values into range +/-1
	if(maxy==miny)
		{// y is a constant, so just need to make this constant 0 c*1-c=0
		 ym=1;
		 yc= -maxy;
		}
	else
		{
		 ym=2.0/(maxy-miny);      // 2.0/ (maxy-miny) gives +/-1 while 1/(maxy-miny) gives 0..1
		 yc=1.0-ym*maxy;
		}
	if(maxx==minx)
		return true; // if x range is zero then there is nothing to do
	xm=2.0/(maxx-minx);
	xc=1.0-xm*maxx;
	v[0]=0;
	divisor=iCount;
	for(j=0;j<=order;++j)
		{sy[j]=0;
		 sx[j]=0;
		 if(callback!=NULL)
			(*callback)(j,order+1); // update on progress
		 for(i=0;i<iCount;++i)
			{x=pAGraph->x_vals[i];
			 y=pAGraph->y_vals[i];
			 // scale x,y
			 x=x*xm+xc;
			 y=y*ym+yc;
			 pv=0;
			 qv=1;
			 for(unsigned int j1=1;j1<=j;++j1)   // calculate orthogonal polynomial for x[i]
				{long double t;
				 t=pv;pv=qv;qv=t; // swap(pv,qv)
				 qv=((x-sx[j1-1])*pv)-(v[j1-1]*qv);
				}
			 sy[j]+=y*qv;
			 sx[j]+=x*qv*qv;
			}
		 sy[j]/=divisor;
		 sx[j]/=divisor;
		 if(j<order)
			{previous=divisor;
			 divisor=0;
			 for(i=0;i<iCount;++i) // calculate orthogonal polynomials for next order
				{x=pAGraph->x_vals[i];
				 // scale x
				 x=x*xm+xc;
				 pv=0;
				 qv=1;
				 for(unsigned int j1=1;j1<=j+1;++j1)   // calculate orthogonal polynomial for x[i]
					{long double t;
					 t=pv;pv=qv;qv=t; // swap(pv,qv)
					 qv=((x-sx[j1-1])*pv)-(v[j1-1]*qv);
					}
				 divisor+=qv*qv;
				}
			 v[j+1]=divisor/previous;
			}
		}
	}
 catch(...)
	{failed=true;
	}
 if(failed)
	{
	 free(v);   // free remaining memory
	 free(sy);
	 free(sx);
	 rprintf("Polynomial fit order %u FAILED (try a lower order)\n",order);
	 return false;
	}
 rprintf("Polynomial fit order %u complete:\n",order);
 for(i=0;i<=order;++i)
		rprintf("  sx[%u]=%.12g sy[%u]=%.12g v[%u]=%.12g\n",i,(double)sx[i],i,(double)sy[i],i,(double)v[i]);


 // Estimate coefficients for a "normal" (Horners rule)polynomial by "symbolic execution" of orthogonal polynomial
 // for pv, qv, sum we need to keep track of the coefficients of powers of x - do that using the following arrays
 bool horner_poly=true; // true if horners rule poly found
 long double *pa=(long double *)calloc(order+1,sizeof(long double));// p,q are long double to match code above. calloc initialises to 0
 long double *qa=(long double *)calloc(order+1,sizeof(long double));
 long double *suma=(long double *)calloc(order+1,sizeof(long double));
 long double *t1a=(long double *)calloc(order+1,sizeof(long double));  // temp arrays
 long double *t2a=(long double *)calloc(order+1,sizeof(long double));
 if(t2a==NULL)
	{// oops out of memory - all memory is freed at the end as we need to continue in this function...
	 rprintf("Sorry cannot calculate coefficients of Horners rule polynomial - no RAM\n");
	 horner_poly=false;
	 // carry on as we need to set new values onto graph from orthogonal poly
	}
 else
	{// calculate "Horners rule" poly by symbolic execution
	 // easy array (eg qa[]) 0'th coeff is constant term [1] is *x, [2] is *x^2 etc
	 // pv=0 is already done as calloc initislises to zero
	 qa[0]=1; // qv=1.0
	 suma[0]=sy[0];
	 for(j=1;j<=order;++j)   // calc orthogonal poly at x
		{long double t;
		 for(i=0;i<=order;++i)  // swap(pv,qv)
			{t=pa[i];pa[i]=qa[i];qa[i]=t;
			}
		 // t1= (v[j-1]*qv)
		 for(i=0;i<=order;++i)
			{t1a[i]=v[j-1]*qa[i];
			}
		 // t2= ((x-sx[j-1])*pv)   note x=X*xm+xc
		 for(i=0;i<=order;++i)
			{t2a[i]= (xc-sx[j-1])*pa[i]; // xc-sx[j-1]*pv
			}
		 for(i=0;i<order;++i)  // <order as [i+1] below
			{
			 t2a[i+1]+=xm*pa[i]; // x*pv
			}
		 // qa=t2-t1  as qv=((x-sx[j-1])*pv)-(v[j-1]*qv);
		 for(i=0;i<=order;++i)
			{qa[i]=t2a[i]-t1a[i];
			}
		 // sum+=sy[j]*qv
		 for(i=0;i<=order;++i)
			{suma[i]+=sy[j]*qa[i];
			}
		}
	 rprintf("Polynomial approximating function is:\nY=");
	 // sum=(sum-yc)/ym
	 // print in an efficient way to execute eg for quadratic (order 2) print (C2*x+C1)*x+C0   [ This is Horners rule ]
	 for(i=1;i<order;++i)
		rprintf("(");
	 if(order>0)
		{rprintf("%.12g",(double)((suma[order])/ym));
		 for(i=order-1;i!=0;--i)
			rprintf("*X%+.12g)",(double)((suma[i])/ym));//%+ causes the sign to always be printed (+/-)
		 rprintf("*X");
		}
	 rprintf("%+.12g",(double)((suma[0]-yc)/ym));  // C0 (note different scaling)
	 rprintf("\n");
	}

  // now use calculated orthogonal polynomial to put calculated y values back
 long double sum,yp;
 long double err;   	 // p version is with conventional poly, without p orthogonal poly version
 double maxe=0,maxep=0;  // max abs error found between poly approximation and original data points
 long double meane2=0,meane2p=0;  // mean (error^2)
 for(i=0;i<iCount;++i)
	{x= pAGraph->x_vals[i];
	 y= pAGraph->y_vals[i];
	 if(horner_poly)
		{
		 // evaluate conventional poly by Horners rule(gives y*ym so need to divide by ym at the end)
		 // note that for high orders Horners rule can give very large errors, but for small orders (typically <=10) its accuracy is very good.
		 if(order>0)
			{yp=suma[order];
			 for(j=order-1;j>0;--j)
				{yp=yp*x+suma[j]; // ideally this would use fmal() which was introduced in c++11 but it does not appear to be supported in builder 10.2
				}
			 yp=yp*x+(suma[0]-yc); // final constant term
			}
		 else
			{ // order 0 - just a constant term
			 yp=(suma[0]-yc); // final constant term
			}
		 err=yp/ym-y; // error when using conventional poly
		 if(fabs(err) > maxep)
			maxep=fabs((double)err); // calculate max abs error
		 meane2p+=(err*err-meane2p)/(long double)(i+1);  // incremental update for mean(error^2)
		}
	 // now do for orthogonal poly  . This method should be accurate for all orders of polynomial.
	 x=x*xm+xc;    // scale x
	 pv=0;
	 qv=1.0;
	 sum=sy[0];
	 for(j=1;j<=order;++j)   // calc orthogonal poly at x
		{long double t;
		 t=pv;pv=qv;qv=t; // swap(pv,qv)
		 qv=((x-sx[j-1])*pv)-(v[j-1]*qv);
		 sum+=sy[j]*qv;
		}

	  sum=(sum-yc)/ym; // inverse scale y so sum is now new value of y calculated from the polynomial

	  err=sum-y;// error when using orthogonal poly
	  if(fabs(err) > maxe)
		maxe=fabs((double)err); // calculate max abs error
	  meane2+=(err*err-meane2)/(long double)(i+1);  // incremental update for mean(error^2)
	  pAGraph->y_vals[i]=(float)sum; // put calculated value (orthogonal poly as that should be the most accurate) back
	}
 rprintf("  with orthogonal poly   : max abs error is %.12g, rms error is %.12g\n",(double)maxe,(double)sqrt(meane2));
 if(horner_poly)
	rprintf("  with conventional poly: max abs error is %.12g, rms error is %.12g\n",(double)maxep,(double)sqrt(meane2p));
 if(pa!=NULL) free(pa);     // free remaining memory , note "horner_poly" may have failed "out of ram" so only free when allocation was OK.
 if(qa!=NULL) free(qa);
 if(suma!=NULL)free(suma);
 if(t1a!=NULL) free(t1a);
 if(t2a!=NULL) free(t2a);
 if(v!=NULL) free(v);
 if(sy!=NULL) free(sy);
 if(sx!=NULL) free(sx);
 return true; // all done OK.
}

bool TScientificGraph::fnFFT(bool dBV_result,bool Hanning,int iGraphNumberF, void (*callback)(size_t cnt,size_t maxcnt)) // apply FFT to data. returns true if OK, false if failed.
{// real fft on data - assumes time steps are equal and in secs
 // actual FFT is done by KISS FFT
 // average value is subtracted from y values before fft & replaced afterwards to help dynamic range.
 // returns power spectrum (correctly scaled).
 // if dBV is true returns result in dBV (ie 20*log10(magnitude))
 // if Hanning is true use a Hanning (Hann) window - this is the recommended general purpose window at https://download.ni.com/evaluation/pxi/Understanding%20FFTs%20and%20Windowing.pdf
 //    which says "In general, the Hanning window is satisfactory in 95 percent of cases. It has good frequency resolution and reduced spectral leakage. "
 //  Note we still need to create a copy for rin as its size can be larger than y_vals[]
 SGraph *pAGraph = ((SGraph*) pHistory->Items[iGraphNumberF]);
 size_t iCount=pAGraph->nos_vals ;
 double x,y;
 float lastx,xmin,xmax,xinc_min,xinc_max;
 double xinc,xinc_av,y_av,freq,freq_step;
 double y2_av;// av( y^2) for rms
 size_t i,nfft;
 nfft=kiss_fftr_next_fast_size_real(iCount);// get sensible (ie fast) (larger) size for fft - this will be even as required by fftr()
 // (double)clock()/(double)CLOCKS_PER_SEC;
 kiss_fftr_cfg  kiss_fftr_state;
 kiss_fft_scalar *rin;
 kiss_fft_cpx *sout;
 if(iCount<=2) return false; // need more than 2 points to be able to do an fft
 rin=(kiss_fft_scalar *)calloc(nfft+2,sizeof(kiss_fft_scalar)); // kiss_fft_scalar is float by default
 sout=(kiss_fft_cpx *)calloc(nfft,sizeof(kiss_fft_cpx));     // output is complex
 if(sout==NULL)
	{if(rin!=NULL) free(rin);
	 rprintf("Sorry- not enough ram for FFT\n");
	 return false;
	}
 if(callback!=NULL)
	(*callback)(0,4); // update on progress  - crude but fft is quick
 kiss_fftr_state = kiss_fftr_alloc(nfft,0,NULL,NULL);
 if(kiss_fftr_state==NULL)
	{free(rin);
	 free(sout);
	 rprintf("Sorry- not enough ram for FFT\n");
	 return false;
	}
  // get "stats" from input data
  lastx=xmin= pAGraph->x_vals[0]; // x values are sorted
  xmax=pAGraph->x_vals[iCount-1];
  xinc=pAGraph->x_vals[1] -xmin;
  xinc_min=xinc_max=(float)xinc;
  xinc_av=xinc;
  y_av=pAGraph->y_vals[0];
  y2_av=y_av*y_av;
  for (i=1;i<iCount;++i)
	{
	 x=pAGraph->x_vals[i];
	 y=pAGraph->y_vals[i];
	 xinc=x-lastx;
	 if(xinc>xinc_max) xinc_max=(float)xinc;
	 if(xinc<xinc_min) xinc_min=(float)xinc;
	 xinc_av+=(xinc-xinc_av)/(double)(i);
	 y_av+=(y-y_av)/(double)(i+1); // i+1 is correct as one less xinc value
	 y2_av+=(y*y-y2_av)/(double)(i+1);
	 lastx=(float)x;
	}
  freq_step=(1.0/xinc_av)/nfft; //  1/xinc_av = max freq so divide by nfft to get step
  rprintf("fft(nfft=%u,iCount=%u): xsteps from %g to %g average %g secs so frequency step after fft=%g Hz.\n y average=%g, rms=%g\n",nfft,iCount,xinc_min,xinc_max,xinc_av,freq_step,y_av,(double)sqrt(y2_av));
  if(callback!=NULL)
	(*callback)(1,4); // update on progress  - crude but fft is quick
  if(xinc_min < 0.9* xinc_av || xinc_max > 1.1*xinc_av)
	ShowMessage("Warning: x increment varies a lot - assuming average value for fft but frequencies will only be approximate");
  // y_av=0; /* uncomment to see the impact of removing the DC component - for test data in csvfun2.csv it makes little difference.
  // setup input array for fft
  for (i=0;i<iCount;++i)
	{if(Hanning)
		{// need to apply Hann(ing) window - see Numerical Recipees or wikipedia for more details.
		 double window=0.5*(1.0-cos(6.283185307179586476925286766559*(double)i/(double)nfft));// 6.28... = 2*PI
		 rin[i] = (float)(window*(pAGraph->y_vals[i] - y_av));
		}
	 else
		{
		 rin[i] =(float)( pAGraph->y_vals[i] - y_av);
		}
	}
 // rest of rin array needs to be filled with zero - this has already been done by calloc()
 if(callback!=NULL)
	(*callback)(2,4); // update on progress  - crude but fft is quick
 {time_t start_t=clock();
  kiss_fftr(kiss_fftr_state,rin,sout); // actually do fft
  rprintf(" fft completed in %g secs\n",(clock()-start_t)/(double)CLOCKS_PER_SEC);
 }
 if(callback!=NULL)
	(*callback)(3,4); // update on progress  - crude but fft is quick
 rprintf(" results from kiss_fftr: (%g,%g), (%g,%g), (%g,%g) ...\n "
			, sout[0].r , sout[0].i
			, sout[1].r , sout[1].i
			, sout[2].r , sout[2].i);

 // now need to put result back - there are less values to put back (due to Nyquist limit) - but we increased the fft size to get an efficient fft so also check against iCount just in case
 freq=0;
 for (i=0;i<(nfft/2)+1 && i<iCount;++i,freq+=freq_step)
	{x=sout[i].r;
	 y=sout[i].i;
#if 1
	// this should be a more accurate way to get the magnitude of the complex number (x+iy)
	// in practice as sout[i].r,.i are both floats no difference is seen as alternative method uses doubles.
	double a,b;// a=max, b=min
	x=fabs(x);
	y=fabs(y);
	if(x>=y)
		{a=x;
		 b=y;
		}
	else
		{a=y;
		 b=x;
		}
	if(a!=0.0)
		{b/=a;
		 y=a*sqrt(1.0+b*b);
		}
	// if a == 0 then y is already 0 as this can only happen if x & y are both 0
#else
	 y=sqrt(x*x+y*y);
#endif
	 y/=nfft;// scaling factor from fft
     if(i==0) y+=fabs(y_av); // put average value back in again. Needs to be fabs() as we have magnitude of conmplex numbers which is >=0.
	 // for the line below dc and max freq are only in 1 bin, but all other frequencies the resultant power is split between 2 bins.
	 if(i!=0 && i!= nfft/2) y*=1.4142135623730950488016887242097; /* mult by sqrt(2) to account for energy in the -ve frequency range */
	 if(dBV_result)
		{// want result in db(V)
		 if(y<=0) y=20.0*-45.0; // 1.4e-45 is the min (denormalised) value for a float
		 else y=20*log10(y);// convert result to dBV
		}
	 pAGraph->y_vals[i]=(float)y;  // |result|
	 pAGraph->x_vals[i]=(float)freq; // freq in Hz, starting at DC
	}
 free(rin);    // free up dynamic memory used
 free(sout);
 free(kiss_fftr_state);
 kiss_fft_cleanup(); // final cleanup for fft functions
 if((nfft/2)+1<iCount)
	{
	 pAGraph->nos_vals=(nfft/2)+1; // shrink array to number of values put back (this does NOT actually change size of arrays).
	 pAGraph->x_vals=(float *)realloc(pAGraph->x_vals,sizeof(float)*pAGraph->nos_vals);  // resize arrays
	 pAGraph->y_vals=(float *)realloc(pAGraph->y_vals,sizeof(float)*pAGraph->nos_vals);
	 pAGraph->size_vals_arrays =pAGraph->nos_vals; // new size of arrays
	}

 return true; // good return
}

void TScientificGraph::compress_y(int iGraphNumberF) // compress by deleting points with equal y values except for 1st and last in a row
{// needs to be done on sorted x values
 // makes just 1 pass over the array of points, with 2 pointers i (to the item being tested) and j (j<=i) where items will be moved to (current end of compressed list)
 // at end items >=j need to be deleted (that is done at the end of this function)
 double lasty,lastx,x,y;
 SGraph *pAGraph = ((SGraph*) pHistory->Items[iGraphNumberF]);
 size_t iCount=pAGraph->nos_vals ;
 size_t i,j;
 bool skipy=false; // set to true while we are skipping equal y values
 if(iCount<2) return; // not enough data in graph to process
 y=lasty=pAGraph->y_vals[0];
 x=lastx=pAGraph->x_vals[0];
 for (i=j=1; i<iCount; i++)  // for all items in list except 1st, i is where we read from, j is where we write to
		{
		 y=pAGraph->y_vals[i];
		 x=pAGraph->x_vals[i];
         if(y==lasty)
                {// in block of repeats
                 skipy=true;
                 lasty=y;   // keep track of last point
                 lastx=x;
                 continue; // keep going till we find the end of the block
                }
         if(skipy)
                {// we have skipped some values, put the last one in
				 pAGraph->y_vals[j]=(float)lasty;
				 pAGraph->x_vals[j]=(float)lastx;
				 ++j;
                 skipy=false;
                }
		 pAGraph->y_vals[j]=(float)y;// y value is different, copy point over
		 pAGraph->x_vals[j]=(float)x;
		 ++j;
         lasty=y;
         lastx=x;
		}
 if(skipy)
	{// need to add in final point  if we were still in a constant run when the end was reached
	 pAGraph->y_vals[j]=(float)y;// y value is different, copy point over
	 pAGraph->x_vals[j]=(float)x;
	 ++j;
	}
 // now delete values not used    [ have used array elements from 0 to j-1 ]
 rprintf("compress: %u point(s) removed from trace (previous size=%u new size=%u)\n",iCount-j,iCount,j);
 pAGraph->nos_vals=j;// resize array that holds points  (frees up memory space in that as well)
 pAGraph->x_vals=(float *)realloc(pAGraph->x_vals,sizeof(float)*j);  // resize arrays
 pAGraph->y_vals=(float *)realloc(pAGraph->y_vals,sizeof(float)*j);
 pAGraph->size_vals_arrays =j;// new size of arrays
}


void TScientificGraph::sortx( int iGraphNumberF) // sort ordered on x values  (makes x values increasing)
{
 SGraph *pAGraph = ((SGraph*) pHistory->Items[iGraphNumberF]);
 size_t iCount=pAGraph->nos_vals ;
 time_t start_t=clock();
  /* sort using yasort2() */

 float *xa=pAGraph->x_vals;
 float *ya=pAGraph->y_vals;
 yasort2(xa,ya,iCount);
 rprintf(" sort (yasort2()) completed in %g secs\n",(clock()-start_t)/(double)CLOCKS_PER_SEC);

 /* check array actually is sorted correctly, also fixes repeated values (so code is always required) */
 int errs=0;

 for(size_t i=0;i<iCount-1;++i)
	 { // sort will bring identical x values together, increase slightly to make different (this may make later values > so fix those to..)
	  if(xa[i]>=xa[i+1])
		{//rprintf("  sort: values equal at x[%zu]=%.9g x[%zu]=%.9g\n",i,(double)pAGraph->x_vals[i],i+1,(double)pAGraph->x_vals[i+1]);
		 xa[i+1]=nextafterf(xa[i],FLT_MAX); // increase slightly to make different
		 //rprintf("   adjusted to x[%zu]=%.9g\n",i+1,pAGraph->x_vals[i+1]);
		 ++errs;// number of corrections
		}
	 }
 if(errs>0)
	{ // recheck after corrections above
	  rprintf("  sort: warning %d x values out of order after 1st sort due to duplicate values, checking corrections\n",errs);
	  errs=0;  // ready to try again
	  for(size_t i=0;i<iCount-1;++i)
		 { // check result is monotonically increasing (no duplicates)
		  if(xa[i]>=xa[i+1])
			{rprintf("  sort: x values not increasing at x[%zu]=%.9g x[%zu]=%.9g\n",i,(double)(xa[i]),i+1,(double)(xa[i+1]));
			 errs++;
			}
		 }
	  if(errs==0) rprintf("  sort: x values are now monotonically increasing\n");
	}
 if(errs>0)    // this should never happen as code above should have fixed up any equal values while keeping the x values sorted
	{rprintf("  sort: warning %d x values out of order after 1st sort due to duplicate values - repeating sort\n",errs);
	 yasort2(xa,ya,iCount);
	 rprintf(" sort (yasort2()) completed total time for both sorts was %g secs\n",(clock()-start_t)/(double)CLOCKS_PER_SEC);

	 errs=0;  // ready to try again

	 for(size_t i=0;i<iCount-1;++i)
		 { // check result is monotonically increasing (no duplicates)
		  if(xa[i]>=xa[i+1])
			{//rprintf("  sort: x values not increasing at x[%zu]=%.9g x[%zu]=%.9g\n",i,(double)(xa[i]),i+1,(double)(xa[i+1]));
			 errs++;
			}
		 }
	}
 if(errs>0)
	{rprintf(" sort: error %d x values out of order\n",errs);
	 ShowMessage("Error: sorting x values failed!\n");
	}
}

//------------------------------------------------------------------------------

void TScientificGraph::fnSetColDataPoint(TColor Color, int iGraphNumberF)
     {((SGraph*)pHistory->Items[iGraphNumberF])->ColDataPoint=Color;}

void TScientificGraph::fnSetColErrorBar(TColor Color, int iGraphNumberF)
     {((SGraph*)pHistory->Items[iGraphNumberF])->ColErrorBar=Color;}

void TScientificGraph::fnSetColLine(TColor Color, int iGraphNumberF)
     {((SGraph*)pHistory->Items[iGraphNumberF])->ColLine=Color;}

void TScientificGraph::fnSetSizeDataPoint(int iSize, int iGraphNumberF)
{((SGraph*)pHistory->Items[iGraphNumberF])->iSizeDataPoint=iSize;}

void TScientificGraph::fnSetErrorBarWidth(int iWidth, int iGraphNumberF)
{((SGraph*)pHistory->Items[iGraphNumberF])->iWidthErrorBar=iWidth;}

void TScientificGraph::fnSetLineWidth(int iWidth, int iGraphNumberF)
{((SGraph*)pHistory->Items[iGraphNumberF])->iWidthLine=iWidth;}

void TScientificGraph::fnSetStyle(unsigned short ucStyleF, int iGraphNumberF)
{((SGraph*)pHistory->Items[iGraphNumberF])->ucStyle=(unsigned char)ucStyleF;}

void TScientificGraph::fnSetPointStyle(unsigned short ucStyleF,
                                                              int iGraphNumberF)
{((SGraph*)pHistory->Items[iGraphNumberF])->ucPointStyle=(unsigned char)ucStyleF;}

void TScientificGraph::fnSetLineStyle(TPenStyle Style, int iGraphNumberF)
{((SGraph*)pHistory->Items[iGraphNumberF])->LineStyle=Style;}


//------------------------------------------------------------------------------

void TScientificGraph::fnResize()
{
  sScaleX=sSizeX;                            //size to scale
  sScaleY=sSizeY;

  fnOptimizeGrids();                         //optimize grids

  fnPaint();                                 //paint
};
//------------------------------------------------------------------------------
void TScientificGraph::fnShiftXPlus()
{
  double dInterval, dDiff;

  dInterval = sScaleX.dMax
              -sScaleX.dMin;         //shift plot right
  dDiff = dInterval*dShiftFactor;


  sScaleX.dMin=sScaleX.dMin+dDiff;
  sScaleX.dMax=sScaleX.dMax+dDiff;

  fnPaint();

};

void TScientificGraph::fnShiftXMinus()       //shift plot left
{
  double dInterval, dDiff;

  dInterval = sScaleX.dMax
              -sScaleX.dMin;         //shift plot right
  dDiff = dInterval*dShiftFactor;

  sScaleX.dMin=sScaleX.dMin-dDiff;
  sScaleX.dMax=sScaleX.dMax-dDiff;

  fnPaint();

};

void TScientificGraph::fnShiftYPlus()        //shift plot up
{
  double dInterval, dDiff;

  dInterval = sScaleY.dMax
              -sScaleY.dMin;
  dDiff=dInterval*dShiftFactor;

  sScaleY.dMin=dDiff+sScaleY.dMin;
  sScaleY.dMax=dDiff+sScaleY.dMax;

  fnPaint();

};

void TScientificGraph::fnShiftYMinus()       //shift plot down
{
  double dInterval, dDiff;

  dInterval = sScaleY.dMax
              -sScaleY.dMin;
  dDiff=dInterval*dShiftFactor;

  sScaleY.dMin=sScaleY.dMin-dDiff;
  sScaleY.dMax=sScaleY.dMax-dDiff;

  fnPaint();

};

void TScientificGraph::fnZoomInX()            //zoom into x, no fixed border
{
  double dInterval, dDiff;

  dInterval = sScaleX.dMax
              -sScaleX.dMin;
  dDiff=(1-dInX)*dInterval/2;
  sScaleX.dMin=sScaleX.dMin+dDiff;
  sScaleX.dMax=sScaleX.dMax-dDiff;
  if(sScaleX.dMax-nextafterfp((float)sScaleX.dMin)<=0)
        sScaleX.dMax= nextafterfp((float)sScaleX.dMin); // limit amount of zoom
  fnOptimizeGrids();
  fnPaint();
};

void TScientificGraph::fnZoomInXFromLeft()   //zoom into x, left border fixed
{
  double dInterval, dDiff;

  dInterval = sScaleX.dMax
              -sScaleX.dMin;
  dDiff=(1-dInX)*dInterval;
  sScaleX.dMax=sScaleX.dMax-dDiff;
  if(sScaleX.dMax-nextafterfp((float)sScaleX.dMin)<=0)
		sScaleX.dMax= nextafterfp((float)sScaleX.dMin); // limit amount of zoom
  fnOptimizeGrids();
  fnPaint();
};

void TScientificGraph::fnZoomOutX()          //zoom out x, no fixed border
{
  double dInterval, dDiff;

  dInterval = sScaleX.dMax
              -sScaleX.dMin;
  dDiff=(1-dOutX)*dInterval/2;
  sScaleX.dMin=sScaleX.dMin+dDiff;
  sScaleX.dMax=sScaleX.dMax-dDiff;
  fnOptimizeGrids();
  fnPaint();

};

void TScientificGraph::fnZoomOutXFromLeft() //zoom out x, left border fixed
{
  double dInterval, dDiff;

  dInterval = sScaleX.dMax
              -sScaleX.dMin;
  dDiff=(1-dOutX)*dInterval;
  sScaleX.dMax=sScaleX.dMax-dDiff;
  fnOptimizeGrids();
  fnPaint();
};

void TScientificGraph::fnZoomInY()           //zoom in y, no border fixed
{
  double dInterval, dDiff;

  dInterval = sScaleY.dMax
              -sScaleY.dMin;
  dDiff=(1-dInY)*dInterval/2;
  sScaleY.dMin=sScaleY.dMin+dDiff;
  sScaleY.dMax=sScaleY.dMax-dDiff;
  if(sScaleY.dMax-nextafterfp((float)sScaleY.dMin)<=0)
		sScaleY.dMax= nextafterfp((float)sScaleY.dMin); // limit amount of zoom
  fnOptimizeGrids();
  fnPaint();
};

void TScientificGraph::fnZoomIn()           // zoom in both x and y , more efficient than calling 2 routines to do Y then X
{
  double dInterval, dDiff;
  // Y first
  dInterval = sScaleY.dMax
              -sScaleY.dMin;
  dDiff=(1-dInY)*dInterval/2;
  sScaleY.dMin=sScaleY.dMin+dDiff;
  sScaleY.dMax=sScaleY.dMax-dDiff;
  if(sScaleY.dMax-nextafterfp((float)sScaleY.dMin)<=0)
		sScaleY.dMax= nextafterfp((float)sScaleY.dMin); // limit amount of zoom
  // now X
  dInterval = sScaleX.dMax
              -sScaleX.dMin;
  dDiff=(1-dInX)*dInterval/2;
  sScaleX.dMin=sScaleX.dMin+dDiff;
  sScaleX.dMax=sScaleX.dMax-dDiff;
  if(sScaleX.dMax-nextafterfp((float)sScaleX.dMin)<=0)
		sScaleX.dMax= nextafterfp((float)sScaleX.dMin); // limit amount of zoom

  fnOptimizeGrids();
  if(zoom_fun_level==0)
        {// no recursion (yet)
         zoom_fun_level=1;   // tell fnPaint() depth of recursion
         while(zoom_fun_level)
                {fnPaint();
                 zoom_fun_level--;
                }
        }
   else zoom_fun_level++;// flag zoom level changed
}

void TScientificGraph::fnZoomOut()           // zoom out both x and y , more efficient than calling 2 routines to do Y then X
{ double dInterval, dDiff;
  // Y 1st
  dInterval = sScaleY.dMax
              -sScaleY.dMin;
  dDiff=(1-dOutY)*dInterval/2;
  sScaleY.dMin=sScaleY.dMin+dDiff;
  sScaleY.dMax=sScaleY.dMax-dDiff;
  // then X
  dInterval = sScaleX.dMax
              -sScaleX.dMin;
  dDiff=(1-dOutX)*dInterval/2;
  sScaleX.dMin=sScaleX.dMin+dDiff;
  sScaleX.dMax=sScaleX.dMax-dDiff;
  fnOptimizeGrids();
  if(zoom_fun_level==0)
        {// no recursion (yet)
         zoom_fun_level=1;   // tell fnPaint() depth of recursion
         while(zoom_fun_level)
                {fnPaint();
                 zoom_fun_level--;
                }
        }
   else zoom_fun_level++;// flag zoom level changed
}

void TScientificGraph::fnZoomInYFromBottom() //zoom in y, bottom border fixed
{
  double dInterval, dDiff;

  dInterval = sScaleY.dMax
              -sScaleY.dMin;
  dDiff=(1-dInY)*dInterval;
  sScaleY.dMax=sScaleY.dMax-dDiff;
  if(sScaleY.dMax-nextafterfp((float)sScaleY.dMin)<=0)
		sScaleY.dMax= nextafterfp((float)sScaleY.dMin); // limit amount of zoom
  fnOptimizeGrids();
  fnPaint();
};

void TScientificGraph::fnZoomOutY()          //zoom out y, no border fixed
{
  double dInterval, dDiff;

  dInterval = sScaleY.dMax
              -sScaleY.dMin;
  dDiff=(1-dOutY)*dInterval/2;
  sScaleY.dMin=sScaleY.dMin+dDiff;
  sScaleY.dMax=sScaleY.dMax-dDiff;
  fnOptimizeGrids();
  fnPaint();
};

void TScientificGraph::fnZoomOutYFromBottom() //zoom out y, bottom border fixed
{
  double dInterval, dDiff;

  dInterval = sScaleY.dMax
              -sScaleY.dMin;
  dDiff=(1-dOutY)*dInterval;
  sScaleY.dMax=sScaleY.dMax-dDiff;
  fnOptimizeGrids();
  fnPaint();
};

//------------------------------------------------------------------------------

void TScientificGraph::fnOptimizeGrids()
{
  double dGrid;

  dGrid=(sScaleX.dMax-sScaleX.dMin)
         /iGridsPerX;
  dGridSizeX=fnMakeANiceNumber(dGrid);                //new gridsize

                                                               //same for y axis
  dGrid=(sScaleY.dMax-sScaleY.dMin)
         /iGridsPerY;
  dGridSizeY=fnMakeANiceNumber(dGrid);

}

//------------------------------------------------------------------------------

void TScientificGraph::fnScales2Size()
{
  sSizeX = sScaleX;                                //set scales as size
  sSizeY = sScaleY;
};

//-----------------------------------------------------------------------------

void TScientificGraph::fnClearAll()                //clear all graphs
{
  int i;
  for (i=iNumberOfGraphs-1; i>=0; i--)
  {
   fnDeleteGraph(i);// delete graphs 1 by 1, this changes  iNumberOfGraphs which is why for loop above is "strange"
  }
  pHistory->Clear();
  iNumberOfGraphs=0;
};

//------------------------------------------------------------------------------
void TScientificGraph::fnDeleteGraph(int iGraphNumberF)     //deletes graph
{
  if ((iGraphNumberF<iNumberOfGraphs)&&(iGraphNumberF>=0))
  { SGraph *pAGraph = ((SGraph*) pHistory->Items[iGraphNumberF]);
	if(pAGraph->x_vals !=NULL) free(pAGraph->x_vals);    // delete all data points
	if(pAGraph->y_vals !=NULL) free(pAGraph->y_vals);
	delete (SGraph*) pHistory->Items[iGraphNumberF]; //  delete SGraph structure (see fnAddgraph() below)
    pHistory->Delete(iGraphNumberF); // remove item from list
    pHistory->Capacity=pHistory->Count; // resize list
    iNumberOfGraphs--;
  }
}
//------------------------------------------------------------------------------

int TScientificGraph::fnAddGraph(size_t max_points)              //insert new graph with nos_points datapoints (at most), return -1 on error
{
  SGraph *pGraph;

  pGraph = new SGraph;                          //allocate
  pGraph->ColDataPoint = clGreen;               //default settings
  pGraph->ColErrorBar = clGreen;
  pGraph->ColLine = clGreen;
  pGraph->iSizeDataPoint=7;
  pGraph->iWidthErrorBar=1;
  pGraph->iWidthLine=2;
  pGraph->ucStyle=3;
  pGraph->ucPointStyle=0;
  pGraph->LineStyle=psSolid;
  pGraph->Caption="";
  pGraph->iTextSize=10;
  // now create space for data points
  pGraph->nos_vals=0; // currently no data points
  pGraph->size_vals_arrays=max_points;
  pGraph->x_vals=(float *)calloc(max_points,sizeof(float));
  pGraph->y_vals=(float *)calloc(max_points,sizeof(float));
  if(pGraph->y_vals==NULL && pGraph->x_vals!=NULL)
	{ // out of space, but x_vals allocated ok
	 free(pGraph->x_vals);
	 pGraph->x_vals=NULL;
	}
  if(pGraph->x_vals==NULL)
	{pGraph->size_vals_arrays=0; // no space allocated
	 rprintf(" Warning: not enought ram for %.0f datapoints\n",(double)max_points);
	 return -1; // error
	}
  pHistory->Add(pGraph);                       //add to history

  iNumberOfGraphs++;
  return iNumberOfGraphs-1;                    //returns item number
};

//------------------------------------------------------------------------------
void TScientificGraph::fnTextOut(double dx, double dy, AnsiString Text,
      TFontStyle Style, int iSize, TColor Color)
{
  int ix,iy;

  ix=(int)((double)iBitmapWidth*dx);        //calculates coordinates from
  iy=(int)((double)iBitmapHeight*dy);        //percentage values dx,dy

  pBitmap->Canvas->Font->Size=iSize;        //settings
  pBitmap->Canvas->Font->Color=Color;
  pBitmap->Canvas->Font->Style=TFontStyles()<<Style;
  pBitmap->Canvas->TextOut(ix,iy,Text);     //text out
  pBitmap->Canvas->Font->Size=8;
  pBitmap->Canvas->Font->Style=TFontStyles();
}
//------------------------------------------------------------------------------
void TScientificGraph::fnTextOut(double dx, double dy, AnsiString Text,
      int iSize, TColor Color)
{
  int ix,iy;

  ix=(int)((double)iBitmapWidth*dx);        //see above
  iy=(int)((double)iBitmapHeight*dy);

  pBitmap->Canvas->Font->Size=iSize;
  pBitmap->Canvas->Font->Color=Color;
  pBitmap->Canvas->TextOut(ix,iy,Text);
  pBitmap->Canvas->Font->Size=8;
}
//------------------------------------------------------------------------------
void TScientificGraph::fnSetCaption(AnsiString Caption, int iGraphNumberF)
{if(iGraphNumberF<0 || iGraphNumberF >=iNumberOfGraphs) return; // invalid graph number
 ((SGraph*)pHistory->Items[iGraphNumberF])->Caption=Caption;}
//------------------------------------------------------------------------------
bool TScientificGraph::fnInScaleY(double d)
{ return (d<sScaleY.dMax && d>sScaleY.dMin);}
//------------------------------------------------------------------------------
bool TScientificGraph::fnInScaleX(double d)
{ return (d<sScaleX.dMax && d>sScaleX.dMin);}
//------------------------------------------------------------------------------
void TScientificGraph::fnAutoScale()
{
  int i;
  size_t j;
  float dXMin, dXMax,dYMin,dYMax;
  int max_graph=-1,min_graph=-1;  // graph for min/max
  float X_for_minY=0,X_for_maxY=0;  // location of min/max
  SGraph *aGraph=NULL;

  if (iNumberOfGraphs>0)
  {
    // there might not be an Items[0] if the length is zero
    dXMax=dYMax=-MAXFLOAT;
	dXMin=dYMin=MAXFLOAT;
    X_for_minY=X_for_maxY=dXMin;

  }
  else
  {
    dXMin=dYMin=0;
    dXMax=dYMax=1;
  }
  for (i=0; i<iNumberOfGraphs; i++)              //search in all graphs for
  {                                              //extrema
	aGraph=(SGraph*) pHistory->Items[i];
	if(aGraph->nos_vals==0) continue; // if no values for this graph skip further processing
 /* know x values are sorted so can move finding xmin/max outside of the loop for speed */
	if (aGraph->x_vals[0]<dXMin)
		{dXMin=aGraph->x_vals[0];
        }
	if (aGraph->x_vals[aGraph->nos_vals-1]>dXMax)
		{dXMax=aGraph->x_vals[aGraph->nos_vals-1];
        }
	// now loop over all elemnts to find y min/max
	for (j=0; j<aGraph->nos_vals; j++)
	{ float y=aGraph->y_vals[j];
	  if (y<dYMin)
        {dYMin=y;
         min_graph=i;
		 X_for_minY= aGraph->x_vals[j];
		}
      if (y>dYMax)
        {dYMax=y;
		 max_graph=i;
		 X_for_maxY= aGraph->x_vals[j];
        }
	}
  }
  if(max_graph>=0)
        {aGraph=(SGraph*) pHistory->Items[max_graph];
		 rprintf("Maximum value = %g found on trace %d (%s) at X=%g\n",dYMax,max_graph+1,aGraph->Caption.c_str(),X_for_maxY);
        }
  else  dXMax=dYMax=1;
  if(min_graph>=0)
		{if(min_graph!=max_graph)
			aGraph=(SGraph*) pHistory->Items[min_graph];
         rprintf("Minimum value = %g found on trace %d (%s) at X=%g\n",dYMin,min_graph+1,aGraph->Caption.c_str(),X_for_minY);
        }
  else dXMin=dYMin=0;
  float dy=dYMax-dYMin;     //space to axis
  dy*=0.1f;
  float dx= (dXMax-dXMin)*0.002f;  // expand range a little so points at both ends are visible also reduces impact of rounding errors when calculating good scaling
  fnSetScales(dXMin-dx,dXMax+dx,dYMin-dy,dYMax+dy);      //actualize scales
  // save actual min/max values
  actual_dXMin=dXMin-dx;
  actual_dXMax=dXMax+dx;
  actual_dYMin=dYMin-dy;
  actual_dYMax=dYMax+dy;

  fnOptimizeGrids();                                    //optimize grids

}
//------------------------------------------------------------------------------
size_t TScientificGraph::fnGetNumberOfDataPoints(int iGraphNumberF)
{if(iGraphNumberF<0 || iGraphNumberF >=iNumberOfGraphs) return 0; // invalid graph number
 return ((SGraph*) pHistory->Items[iGraphNumberF])->nos_vals ;
}
//------------------------------------------------------------------------------
double TScientificGraph::fnGetDataPointYValue(size_t iChannelF,
												   int iGraphNumberF)
{ SGraph *aGraph;
  if ((pHistory->Count-1)>=iGraphNumberF && iGraphNumberF>=0)
  { aGraph=(SGraph*) pHistory->Items[iGraphNumberF];
	if(aGraph->nos_vals-1 >=iChannelF)
		{return aGraph->x_vals[iChannelF];
		}
  }
  return 0; // default value on error
}
//------------------------------------------------------------------------------
bool TScientificGraph::fnPoint2Koord(int iPointX, int iPointY, double &dKoordX,
                                     double &dKoordY)
{
  double dRatioX, dRatioY;

  dRatioX=(((double)iPointX)/((double)iBitmapWidth)-fLeftBorder) //relative position
           /(1-fRightBorder-fLeftBorder);                        //in plot
  dRatioY=(((double)iPointY)/((double)iBitmapHeight)-fTopBorder)
           /(1-fBottomBorder-fTopBorder);
                                                             //calc. coordinates
  dKoordX=sScaleX.dMin+dRatioX
          *(sScaleX.dMax
          -sScaleX.dMin);
  dKoordY=sScaleY.dMax-dRatioY
          *(sScaleY.dMax
          -sScaleY.dMin);

                                                             //true if point in
                                                             //plot
  if ((dRatioX<0) || (dRatioX>1) || (dRatioY<0) || (dRatioY>1))
  {return false;} else {return true;}
}
//------------------------------------------------------------------------------
void TScientificGraph::fnSetScales(double dXMin, double dXMax, double dYMin,
                                   double dYMax)
{
  double temp;

  if (dXMin>dXMax) {temp=dXMin; dXMin=dXMax; dXMax=temp;} //correct relations
  if (dYMin>dYMax) {temp=dYMin; dYMin=dYMax; dYMax=temp;}

  if (dXMin==dXMax) {dXMin=dXMin-0.1; dXMax=dXMax+0.1;}   //no zero ranges
  if (dYMin==dYMax) {dYMin=dYMin-0.1; dYMax=dYMax+0.1;}

#if 0    /* expand a little so if we are very close to a power of 10 a tick will appear where we would expect one */
  temp= (dXMax-dXMin)*0.001;
  sScaleX.dMin=dXMin-temp;              //set scales slightly wider than limits
  sScaleX.dMax=dXMax+temp;              // so that something stopping near a power of 10 has a better set of ticks
  temp= (dYMax-dYMin)*0.001;
  sScaleY.dMin=dYMin-temp;
  sScaleY.dMax=dYMax+temp;
#else
  sScaleX.dMin=dXMin;                                     //set scales
  sScaleX.dMax=dXMax;
  sScaleY.dMin=dYMin;
  sScaleY.dMax=dYMax;
#endif
}

//------------------------------------------------------------------------------
double TScientificGraph::fnMakeANiceNumber(double d)
{
  double dExponent;
  double dBase;
  const double MINd=MINFLOAT/100.0;   // avoid maths errors that would happen if we tried to take to log of 0. As we use floats to store data from csv file MINFLOAT/100 should be OK
  if(d<MINd) d=MINd; // avoid maths errors that would happen if we tried to take to log of 0. As we use floats to store data from csv file MINFLOAT shold be OK
  dExponent=floor(log10(d));                      //make d a x.0Ey or
  dBase=d/pow(10,dExponent);                      // x.5Ey or x.25Ey or x.75Ey
  if (dBase<=1.0){dBase=1.0;}                     // number for gridsize   1,2,5 
  else if (dBase<=2.0){dBase=2.0;}                 
  // else if (dBase<=2.5){dBase=2.5;}
  else if (dBase<=5) {dBase=5;}
  // else if (dBase<=7.5) {dBase=7.5;}
  else {dBase=10;}
  return (dBase*pow(10,dExponent));                //return nice number

}
//------------------------------------------------------------------------------
void TScientificGraph::fnPaintGridX(double dADoub)
{
  TPoint Point; // avoid dynamic memory allocation overhead if we used new and delete
  TPoint *pPoint=&Point;


  pBitmap->Canvas->Pen->Color = ColGrid;                       //style settings
  pBitmap->Canvas->Pen->Width = iPenWidthGrid;
  pBitmap->Canvas->Pen->Style = PSGrid;
  fnKoord2Point(pPoint,dADoub,sScaleY.dMin);     //paint
  pBitmap->Canvas->MoveTo(pPoint->x,pPoint->y);
  fnKoord2Point(pPoint,dADoub,sScaleY.dMax);
  pBitmap->Canvas->LineTo(pPoint->x,pPoint->y);
}

void TScientificGraph::fnPaintGridY(double dADoub)
{
  TPoint Point; // avoid dynamic memory allocation overhead if we used new and delete
  TPoint *pPoint=&Point;

  pBitmap->Canvas->Pen->Color = ColGrid;              //settings
  pBitmap->Canvas->Pen->Width = iPenWidthGrid;
  pBitmap->Canvas->Pen->Style = PSGrid;
  fnKoord2Point(pPoint,sScaleX.dMin,dADoub);
  pBitmap->Canvas->MoveTo(pPoint->x,pPoint->y);
  fnKoord2Point(pPoint,sScaleX.dMax,dADoub);
  pBitmap->Canvas->LineTo(pPoint->x,pPoint->y);
}

void TScientificGraph::fnPaintTickX(double dADoub, double dScaling)
{
  TPoint Point; // avoid dynamic memory allocation overhead if we used new and delete
  TPoint *pPoint=&Point;

  pBitmap->Canvas->Pen->Color = ColAxis;               //settings for ticks
  pBitmap->Canvas->Pen->Width = iPenWidthAxis;
  pBitmap->Canvas->Pen->Style = PSAxis;

  fnKoord2Point(pPoint,dADoub,sScaleY.dMin);     //ticks
  pBitmap->Canvas->MoveTo(pPoint->x,pPoint->y);
  pBitmap->Canvas->LineTo(pPoint->x,(int)(pPoint->y-iTickLength*dScaling));
  fnKoord2Point(pPoint,dADoub,sScaleY.dMax);
  pBitmap->Canvas->MoveTo(pPoint->x,pPoint->y);
  pBitmap->Canvas->LineTo(pPoint->x,(int)(pPoint->y+iTickLength*dScaling));
}

void TScientificGraph::fnPaintTickY(double dADoub, double dScaling)
{
  TPoint Point; // avoid dynamic memory allocation overhead if we used new and delete
  TPoint *pPoint=&Point;

  pBitmap->Canvas->Pen->Color = ColAxis;              //tick settings
  pBitmap->Canvas->Pen->Width = iPenWidthAxis;
  pBitmap->Canvas->Pen->Style = PSAxis;

  fnKoord2Point(pPoint,sScaleX.dMin,dADoub);       //ticks
  pBitmap->Canvas->MoveTo(pPoint->x,pPoint->y);
  pBitmap->Canvas->LineTo((int)(pPoint->x+iTickLength*dScaling),pPoint->y);
  fnKoord2Point(pPoint,sScaleX.dMax,dADoub);
  pBitmap->Canvas->MoveTo(pPoint->x,pPoint->y);
  pBitmap->Canvas->LineTo((int)(pPoint->x-iTickLength*dScaling),pPoint->y);
}
//------------------------------------------------------------------------------
void TScientificGraph::fnPaintDataPoint(TRect Rect, unsigned char ucStyle)
{
  TPoint points[4];

  if ((ucStyle&4)==4) pBitmap->Canvas->Brush->Style=bsSolid;
  else pBitmap->Canvas->Brush->Style=bsClear;
  switch (ucStyle&3)
  {
    case 0:
    {
      pBitmap->Canvas->Ellipse(Rect);
      break;
    }
    case 1:
    {
      pBitmap->Canvas->Rectangle(Rect);
      break;
    }
    case 2:
    {
      points[0] = Point((Rect.Left+Rect.Right)/2,Rect.Top);
      points[1] = Point(Rect.Right,Rect.Bottom);
      points[2] = Point(Rect.Left,Rect.Bottom);
      pBitmap->Canvas->Polygon(points,2);
      break;
    }
    case 3:
    {
      points[0] = Point((Rect.Left+Rect.Right)/2,Rect.Bottom);
      points[1] = Point(Rect.Right,Rect.Top);
      points[2] = Point(Rect.Left,Rect.Top);
      pBitmap->Canvas->Polygon(points,2);
      break;
    }
  }
  pBitmap->Canvas->Brush->Style=bsClear;
}
//------------------------------------------------------------------------------
void TScientificGraph::fnCheckScales()
{  // check scales are sensible, and if not fix them. Scales are doubles, but we store x,y points as floats so can limit range based on floats
  if(sScaleX.dMax> MAXFLOAT) sScaleX.dMax=MAXFLOAT;
  if(sScaleX.dMin< -MAXFLOAT) sScaleX.dMin=-MAXFLOAT;
#if 1
  if(sScaleX.dMax-nextafterfp((float)sScaleX.dMin)<=0)
		{sScaleX.dMax= nextafterfp((float)sScaleX.dMin); // limit amount of zoom
         sScaleX.dMin=sScaleX.dMin-(sScaleX.dMax-sScaleX.dMin);
        }
#else
  if(sScaleX.dMax-nextafterfp(sScaleX.dMin)<=0)
        sScaleX.dMax= nextafterfp(sScaleX.dMin); // limit amount of zoom
#endif
  if(sScaleX.dMax < sScaleX.dMin)
        {double t=sScaleX.dMax ; // wrong order - swap so max> min
         sScaleX.dMax =sScaleX.dMin;
         sScaleX.dMin =t;
        }

  if(sScaleY.dMax> MAXFLOAT) sScaleY.dMax=MAXFLOAT;
  if(sScaleY.dMin< -MAXFLOAT) sScaleY.dMin=-MAXFLOAT;
#if 1
  if(sScaleY.dMax-nextafterfp((float)sScaleY.dMin)<=0)
        {sScaleY.dMax= nextafterfp((float)sScaleY.dMin); // limit amount of zoom
         sScaleY.dMin=sScaleY.dMin-(sScaleY.dMax-sScaleY.dMin);
        }
#else
  if(sScaleY.dMax-nextafterfp((float)sScaleY.dMin)<=0)
		sScaleY.dMax= nextafterfp((float)sScaleY.dMin); // limit amount of zoom
#endif
  if(sScaleY.dMax < sScaleY.dMin)
        {double t=sScaleY.dMax ; // wrong order - swap so max> min
         sScaleY.dMax =sScaleY.dMin;
         sScaleY.dMin =t;
        }
}



bool TScientificGraph::SaveCSV(char *filename,char *x_axis_name, double xmin, double xmax)
{          // save data into specified csv filename - only save xvalues in defined range
 size_t j=0;
 SGraph *aGraph,*xGraph;
 FILE *fp;
 char cstr[256]; // buffer for sprintf strings
 time_t start_t=clock();
 time_t lastT=start_t;
 cstr[sizeof(cstr)-1]=0; // make sure null terminated
 if(iNumberOfGraphs==0)
		{snprintf(cstr,sizeof(cstr)-1,"Error on CSV save: Nothing to save!");    // sizeof -1 as last character set to null above
		 ShowMessage(cstr);
         return false;
        }
 // check if all graphs have the same number of points in them , if not warn user and extrapolate missing data
 for (int i=0; i<iNumberOfGraphs; i++)              //for all graphs
  {
	aGraph=(SGraph*) pHistory->Items[i];
	if(i==0)
		{j=aGraph->nos_vals ; // nos items in trace 0
		}
	else
		{if(j!= aGraph->nos_vals)
				{snprintf(cstr,sizeof(cstr)-1,"Warning on CSV save: traces on graph have different numbers of points (trace %.0f has %.0f while trace 1 has %.0f) - y values will be interpolated to match 1st trace x values.",(double)(i+1),(double)aGraph->nos_vals,(double)j);    // sizeof -1 as last character set to null above
				 ShowMessage(cstr);
				 // return false;      This is no longer a fault as we can interpolate
				 break; // only show warning message once
				}
		}
   }
 //  open file for writing
 //fp=fopen(filename,"wt");
 fp=_wfopen(Utf8_to_w(filename),L"wt");
 if(fp==NULL)
		{snprintf(cstr,sizeof(cstr)-1,"Error on CSV save: cannot create file %s",filename);    // sizeof -1 as last character set to null above
		 ShowMessage(Utf8_to_w(cstr));
		 return false;
		}
 setvbuf(fp,NULL,_IOFBF,128*1024); // set a reasonably large output buffer, and full buffering
 #if 1 /* utf-8 handling */
 int BOM_needed=IDNO; // default to not needing utf-8 BOM
 // look to see if any headers need utf-8  (rather than just 7 bit ascii chars)
 if(is_utf8(x_axis_name)) BOM_needed=IDYES;
 for (int i=0; i<iNumberOfGraphs; i++)              //for all graphs
	{aGraph=(SGraph*) pHistory->Items[i];
	 if(is_utf8(aGraph->Caption.c_str())) BOM_needed=IDYES;
	}
 if( BOM_needed==IDYES)
	{BOM_needed=Application->MessageBox(L"Create UTF8-BOM csv file [needed for Excel]?\n\"No\" will result in a standard ANSI/UTF-8 file)", L"CSV File type", MB_YESNO);
	}
 // rprintf("UTF8-BOM request returned %0x: YES=%x NO=%x\n",BOM_needed,IDYES,IDNO);
 if(BOM_needed==IDYES)
	{fprintf(fp,"\xEF\xBB\xBF"); // print "magic" uft-8 BOM at start of file so other software can recognise the filetype [ csvgraph will understand this when loading the file ]
	 rprintf("UTF8-BOM written to file \"%s\"\n",filename);
	}
 #endif
 // now write header line for csv file with names for each column
 fprintf(fp,"\"%s\"",x_axis_name);
 for (int i=0; i<iNumberOfGraphs; i++)              //for all graphs
	{aGraph=(SGraph*) pHistory->Items[i];
	 if( aGraph->Caption.c_str()[0]=='"')
		{// caption already has "...", so don't add another set
		 fprintf(fp,",%s",aGraph->Caption.c_str());
		}
	   else
		{// no quotes at present, add them
		 fprintf(fp,",\"%s\"",aGraph->Caption.c_str());
		}
	}
 fprintf(fp,"\n"); // end of header line
 // now print out main csv file
 xGraph=(SGraph*) pHistory->Items[0];
 for (j=0; j<xGraph->nos_vals; j++)
	{float xj;
	if((j&0x0ffff)==0 && (clock()-lastT)>= CLOCKS_PER_SEC)
		{// display progress  every second
		 lastT=clock();
		 snprintf(cstr,sizeof(cstr),"csv save: %.0f%% complete",100.0*(double)j/(double)(xGraph->nos_vals));
		 Form1->pPlotWindow->StatusText->Caption=cstr;
		 Application->ProcessMessages(); /* allow windows to update (but not go idle) */
		}
	  xj= xGraph->x_vals[j];
	  if(xj<xmin || xj>xmax) continue; // outside of range to save
	  fprintf(fp,"%.9g",xj);    // printf x value first. %.9g (9sf) gives max resolution for a float
	  for (int i=0; i<iNumberOfGraphs; i++)  // now print y values for all traces
		{aGraph=(SGraph*) pHistory->Items[i];
		 if(i==0)  fprintf(fp,",%.9g",aGraph->y_vals[j]); // trace 0: can always just print 1st y value as that trace provides x values
		 else if(j<aGraph->nos_vals && aGraph->x_vals[j]==xj)
			fprintf(fp,",%.9g",aGraph->y_vals[j]);// if x value matches trace 0 then just print matching y value (this is faster than always interpolating)
		 else
			{// need to interpolate to get correct y value
			 // float interp1D(float *xa, float *ya, int size, float x, bool clip);
			 float yj=interp1D_f(aGraph->x_vals,aGraph->y_vals,aGraph->nos_vals,xj,true);
			 fprintf(fp,",%.9g",yj); // interpolated value
			}
		}
	  fprintf(fp,"\n");
	}
 fclose(fp);
 snprintf(cstr,sizeof(cstr),"csv save: finished in %.1f secs",(clock()-start_t)/(double)CLOCKS_PER_SEC);
 Form1->pPlotWindow->StatusText->Caption=cstr;
 rprintf("File \"%s\" - %s\n",filename,cstr);
 Application->ProcessMessages(); /* allow windows to update (but not go idle) */
 return true; // good exit
}
