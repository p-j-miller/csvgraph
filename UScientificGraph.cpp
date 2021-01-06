//---------------------------------------------------------------------------
//  TScientificGraph -- an object for scientific plots
// This version by Peter Miller 18/7/2019 as csv file grapher
//
//  Loosely based on an original public domain version by  Frank heinrich, mail@frank-heinrich.de
// Major tidy up 1/9/2019 to remove unused functionality
/*----------------------------------------------------------------------------
 * Copyright (c) 2019 Peter Miller
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
#define NoForm1   /* says Form1 is defined in another file */
#include "expr-code.h"
#include <cmath>
#include "kiss_fftr.h" // for fft
#include "_kiss_fft_guts.h"

// #define USE_double_to_str_exp /* define to use double_to_str_exp() for y axis with max 12 chars, if not defined use gcvt() */

#ifdef USE_double_to_str_exp
 #include "float_to_str.h"
#endif
//---------------------------------------------------------------------------

#pragma package(smart_init)
#define P_UNUSED(x) (void)x; /* a way to avoid warning unused parameter messages from the compiler */
#define DOUBLE float /* define as double to go back to original, but as points are stored as floats we can use floats in several places */

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

  fLeftBorder = 0.13;
  fRightBorder = 0.025;
  fTopBorder = 0.04;
  fBottomBorder = 0.11;

  dInX=0.8;
  dOutX=10.0/8.0;
  dInY=0.8;
  dOutY=10.0/8.0;
  dShiftFactor=0.1;

  XLabel="Horizontal axis title";
  YLabel1="Vertical";
  YLabel2="axis title";

  dLegendStartX=0.5; // was 0.88
  dLegendStartY=0.95; // was 0.95
  dCaptionStartX=0.5;  // was 0.6
  dCaptionStartY=0.5;  // was 0.9

  iSkipLineLevel=2; // adjacent points

  fnResize();                                        //put size to scales
};

//------------------------------------------------------------------------------
TScientificGraph::~TScientificGraph()
{
  int i;
  for (i=0; i<iNumberOfGraphs; i++)                        //free graphs
  {
    ((SGraph*) pHistory->Items[i])->pDataList->Clear();
    delete ((SGraph*) pHistory->Items[i])->pDataList;
  }
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
  pPoint->x=dXKoord;                      //to integer
  pPoint->y=dYKoord;


  if ((dXKoord<iBitmapWidth*fLeftBorder) ||           //point in plot?
     (dXKoord>iBitmapWidth*(1-fRightBorder)) ||
     (dYKoord<iBitmapHeight*fTopBorder) ||
     (dYKoord>iBitmapHeight*(1-fBottomBorder)))
  {  return false;}                                   //no.

  return true;                                        //yes.
};

//------------------------------------------------------------------------------
double xs,ys; // start of next line (end of previous line)
	 /* Dan Cohen & Ivan Sunderland clipping algorithm - see  Principles of interactive computer graphics 2nd Ed, pp 65-67   */
	 /* this version is functionally the same as the original with various bugs resolved and efficiency improvements */

#define LEFT 1 /* bits - must all be different powers of 2 */
#define RIGHT 2
#define BOTTOM 4
#define TOP 8

void TScientificGraph::graph_line(double xe,double ye,double xmin,double xmax,double ymin,double ymax)
{// draw line from xs,ys to xe,ye clipped by min/max . Afterwards set xs,ys to xe,ye.
 int outcode1=0, outcode2=0;
 int c;
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
  unsigned int iCount;
  int i,j;
  double dADoub;
  double dX, dY;    // even in inner loops these need to be doubles [ due to my extra clipping code]
  char szAString[31],lasttick[31];
  AnsiString AAnsiString;

  TRect LayoutRect;
  TSize ASize;
  TList *pAList;
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
         pBitmap->Canvas->TextOutA(pPoint->x-ASize.cx/2,pPoint->y+iTextOffset,AAnsiString);
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
         pBitmap->Canvas->TextOutA(pPoint->x-ASize.cx/2,pPoint->y+iTextOffset,AAnsiString);
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
         pBitmap->Canvas->TextOutA(pPoint->x-ASize.cx-iTextOffset,pPoint->y-ASize.cy/2,AAnsiString);
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
    pBitmap->Canvas->TextOutA(pPoint->x-ASize.cx-iTextOffset,pPoint->y-ASize.cy/2,AAnsiString);
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
      pBitmap->Canvas->TextOutA(pPoint->x,pPoint->y,pAGraph->Caption);
      *pPoint=*pPoint2;
      if ((pAGraph->iSizeDataPoint>pBitmap->Canvas->TextHeight("0"))&&
         (((pAGraph->ucStyle) & 1) == 1))
        pPoint->y+=pAGraph->iSizeDataPoint+5;
      else pPoint->y+=pBitmap->Canvas->TextHeight("0");
    }
  }

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
    pAList = pAGraph->pDataList;
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
      iCount=pAList->Count;
#if 1
      // do binary search to find start of area thats visible on the screen
      int starti;    // index just before start
      {
       int low=0;
       int high=iCount-1;
       bool found=false;
       double key=sScaleX.dMin;  // needs to be double as otherwise compare midval<key can generate an overflow if dMin -? dMax is a very large range
       int mid;
       while(low<=high && !found)
        {mid=low+((high-low)>>1); /* (low+high)/2 but written so cannot overflow */
         double midVal=((SDataPoint*) pAList->Items[mid])->dXValue;
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
      for (unsigned int i=starti; i<iCount; i++)  // was i+=step
#else
      for (unsigned int i=0; i<iCount; i++)  // was i+=step
#endif
      {
       dX = ((SDataPoint*) pAList->Items[i])->dXValue;
       if(!fnInScaleX(dX))
                {if(dX> sScaleX.dMax) break; // past end so all done for this trace
                 else continue; // PMi optimisation - skip values before xmin
                                // this is important when zooming in as otherwise code below will see the whole file and will "compress" the graph incorrectly
                }
       ymax=ymin=((SDataPoint*) pAList->Items[i])->dYValue; // dY
       x_ymax=x_ymin=dX;
       if(zoom_fun_level)
          {Application->ProcessMessages(); /* allow windows to update (but not go idle) - potentially causes recursion ! */
           if(zoom_fun_level>1)
                goto fnpaint_end;// > 1 means we have recursion , abort present update as we need to do another with different scaling
          }
        // we know scaling so we can calculate how many points we need to skip
       xd+=xi; // this works better when "skip equal y values is set" as x values are not then evenly spaced and this way points selected are evenly spaced
       dX = ((SDataPoint*) pAList->Items[i])->dXValue; // dX,dY is 1st point examined, lastx,lasty is last point in this "segment"
       dY = ((SDataPoint*) pAList->Items[i])->dYValue;
       for(istep=1;i+istep<iCount && (((SDataPoint*) pAList->Items[i+istep])->dXValue)<xd ;++istep)
        {lastx = ((SDataPoint*) pAList->Items[i+istep])->dXValue;
         lasty = ((SDataPoint*) pAList->Items[i+istep])->dYValue;
         if(lasty>ymax) {ymax=lasty;x_ymax=lastx;}
         if(lasty<ymin) {ymin=lasty;x_ymin=lastx;}
        }
       i+=istep-1;
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
      }   // end for(i)

    }  // end if (((pAGraph->ucStyle) & 1) == 1) (if style: data point)
    if (((pAGraph->ucStyle)&4)==4)                        //style: line
    { unsigned int istep;
      bool first=True;    // used to trap start and end of region we wish to view (when zoomed)
      bool last=False;
      DOUBLE ymax,ymin;
      double x_ymax,x_ymin; // used to capture features in skipped data (need to be double due to clipping)
      DOUBLE lastx,lasty;
      iCount=pAList->Count;
      pBitmap->Canvas->Pen->Width=pAGraph->iWidthLine;
      pBitmap->Canvas->Pen->Color=pAGraph->ColLine;
      pBitmap->Canvas->Pen->Style=pAGraph->LineStyle;
      //iCount=pAList->Count;
      if (iCount!=0)
      {
        dX = ((SDataPoint*) pAList->Items[0])->dXValue;
        dY = ((SDataPoint*) pAList->Items[0])->dYValue;
        fnKoord2Point(pPoint,dX,dY);
        pBitmap->Canvas->PenPos=*pPoint;
        *pPoint2=*pPoint;

      }
      double xd=sScaleX.dMax-sScaleX.dMin; // total span
      double xi=xd/x_width_pixels; // use all the pixels available
      xd=sScaleX.dMin-xi; // -xi as add xi before its used
#if 1
      // do binary search to find start of area thats visible on the screen
      int starti;    // index just before start
      {
       int low=0;
       int high=iCount-1;
       bool found=false;
       double key=sScaleX.dMin;   // without this being a double we get floating point overflow on midVal<key below when dMin->dMax is very large
       int mid;
       while(low<=high && !found)
        {mid=low+((high-low)>>1); /* (low+high)/2 but written so cannot overflow */
         double midVal=((SDataPoint*) pAList->Items[mid])->dXValue;
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
      for (unsigned int i=starti; i<iCount; i++)  // start processing just where we need to.
#else
      for (unsigned int i=0; i<iCount; i++)  // linear search from start
#endif      
      {
       dX = ((SDataPoint*) pAList->Items[i])->dXValue;
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
         if(i>0)
                {xs= ((SDataPoint*) pAList->Items[i-1])->dXValue;
                 ys = ((SDataPoint*) pAList->Items[i-1])->dYValue;
                }
         else
                {xs= ((SDataPoint*) pAList->Items[0])->dXValue;
                 ys = ((SDataPoint*) pAList->Items[0])->dYValue;
                }
        }

       dX = ((SDataPoint*) pAList->Items[i])->dXValue;
       dY = ((SDataPoint*) pAList->Items[i])->dYValue;
       ymax=ymin=dY;
       x_ymin=x_ymax=dX;
       if(zoom_fun_level)
          {Application->ProcessMessages(); /* allow windows to update (but not go idle) - potentially causes recursion ! */
           if(zoom_fun_level>1)
                goto fnpaint_end;// > 1 means we have recursion , abort present update as we need to do another with different scaling
          }
        // we know scaling so we can calculate how many points we need to skip
       xd+=xi; // this works better when "skip equal y values is set" as x values are not then evenly spaced and this way points selected are evenly spaced
       lastx=dX ; // dX,dY is 1st point examined, lastx,lasty is last point in this "segment"
       lasty=dY ;
       for(istep=1;i+istep<iCount && (((SDataPoint*) pAList->Items[i+istep])->dXValue)<xd ;++istep)
        {lastx = ((SDataPoint*) pAList->Items[i+istep])->dXValue;
         lasty = ((SDataPoint*) pAList->Items[i+istep])->dYValue;
         if(lasty>ymax) {ymax=lasty;x_ymax=lastx;}
         if(lasty<ymin) {ymin=lasty;x_ymin=lastx;}
        }
       i+=istep-1;
        {
         if(x_ymin>x_ymax)
                {double tx,ty; // swap around to make sure we stay in order of increasing x
                 tx=x_ymin;ty=ymin;
                 x_ymin=x_ymax; ymin=ymax;
                 x_ymax=tx; ymax=ty;
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
  int off_len_LY1=pBitmap->Canvas->TextWidth(YLabel1);
  int off_len_LY2=pBitmap->Canvas->TextWidth(YLabel2);
  if(YLabel2=="")
        { // only 1 label to print, put it nearest to the axis , // +off_len_LY?/2 centres Ylabel
         pBitmap->Canvas->TextOutA(pPoint->x-iTextOffset*2
                -off2,pPoint->y+off_len_LY1/2,YLabel1);
        }
  else
        {// 2 ledgends to print , have to space them out
         pBitmap->Canvas->TextOutA(pPoint->x-iTextOffset*2
                -off1,pPoint->y+off_len_LY1/2,YLabel1);
         pBitmap->Canvas->TextOutA(pPoint->x-iTextOffset*2
                -off2,pPoint->y+off_len_LY2/2,YLabel2);
        }
#endif
  // restore original values back
  lf.lfEscapement = 0; // rotation in deg*10
  lf.lfOrientation = 0;
  lf.lfOutPrecision = OUT_DEFAULT_PRECIS;  // restore to original value
  // Create new font with zero rotation ; assign to Canvas Font's Handle.
  pBitmap->Canvas->Font->Handle = CreateFontIndirect(&lf);
#else
  // original code
  pBitmap->Canvas->TextOutA(pPoint->x-ASize.cx-iTextOffset*2
       -pBitmap->Canvas->TextWidth("-0.0000000"),pPoint->y,YLabel1);
  pBitmap->Canvas->TextOutA(pPoint->x-ASize.cx-iTextOffset*2
       -pBitmap->Canvas->TextWidth("-0.0000000"),pPoint->y+ASize.cy,YLabel2);
#endif
  fnKoord2Point(pPoint,(sScaleX.dMax
                -sScaleX.dMin)*dCaptionStartX
                +sScaleX.dMin,
                sScaleY.dMin);
  ASize=pBitmap->Canvas->TextExtent(XLabel);
  pBitmap->Canvas->Font->Color=ColText;
  int off_len_LX=pBitmap->Canvas->TextWidth(XLabel);
  pBitmap->Canvas->TextOutA(pPoint->x-off_len_LX/2,pPoint->y+ASize.cy+iTextOffset,XLabel);   // -off_len_LX/2 centres Xlabel

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

};


//------------------------------------------------------------------------------

bool TScientificGraph::fnAddDataPoint(double dXValueF, double dYValueF,int iGraphNumberF)    // returns true is added OK, false if not
{
  TList *pAList;
  SDataPoint *pDataPoint;
  try {
        pDataPoint = new SDataPoint;                  //allocate point
        pDataPoint->dXValue=dXValueF;                 //x
        pDataPoint->dYValue=dYValueF;                 //y
		pAList = ((SGraph*) pHistory->Items[iGraphNumberF])->pDataList;  //address of data list
		pAList->Add(pDataPoint);                      //insert point in list   (pAList is a list of pointers to SDataPoint objects)
#if 0      /* with builder 10.2 this makes no difference to execution time but increases space used by traces a little */
		if(pAList->Count == pAList->Capacity )
            pAList->Capacity +=4000; // add space for extra 4000 items if list was full, would automatically expand but this may be more efficient
#endif
#if 0
        pAList->Expand(); // make more space in array (not strictly needed but faster as adds space in larger chunks)  [ actually its faster NOT doing this!]
#endif        
        return true;
       }
   catch (...) {  // assume the issue is out of memory
         return false;
        }
};

bool TScientificGraph::fnChangeXoffset(double dX) // change all X values by adding dX to the most recently added graph if at least 2 graphs defined
{
  if( iNumberOfGraphs<2) return false;   // must be at least 2 graphs to do this
  int j=iNumberOfGraphs-1;
  SGraph *pAGraph = ((SGraph*) pHistory->Items[j]);
  TList *pAList = pAGraph->pDataList;
  unsigned int iCount=pAList->Count;
  for (unsigned int i=0; i<iCount; i++)  // for all items in list add dX to x value
      {
       (((SDataPoint*) pAList->Items[i])->dXValue)+=dX;
      }
  return true;    
};


float median3(float y0,float y1, float y2) // returns median of 3 values
{if(y1<=y0 && y0<=y2) return y0; // y0 is middle value (median)
 if(y0<=y1 && y1<=y2) return y1; // y1 is middle value
 return y2; // y2 must be middle value
}

/*
 *  This Quickselect routine is based on the algorithm described in
 *  "Numerical recipes in C", Second Edition,
 *  Cambridge University Press, 1992, Section 8.5, ISBN 0-521-43108-5
 *  This code by Nicolas Devillard - 1998. Public domain.
 * [ Note the Numerical recipees code works on an array indixed from 1..n rather than the normal c usage of 0...n-1, so while this code uses the same algorithm its implementation is different]
 *  Note this code changes the contents of arr , and assumes n is odd (ie it just finds the middle value)
 *  code has been (significantly) reformated by Peter Miller to make indentation correct !
  * takes time proportional to n (ie is fast!)
 */

#define elem_type float
#define ELEM_SWAP(a,b) { register elem_type t=(a);(a)=(b);(b)=t; }

elem_type quick_select(elem_type arr[], int n)
{
    int low, high ;
    int median;
    int middle, ll, hh;

    low = 0 ; high = n-1 ; median = (low + high) / 2;
    for (;;)
       {
        if (high <= low) /* One element only */
            return arr[median] ;

        if (high == low + 1)
           {  /* Two elements only */
            if (arr[low] > arr[high])
                ELEM_SWAP(arr[low], arr[high]) ;
            return arr[median] ;
           }

        /* Find median of low, middle and high items; swap into position low */
        middle = (low + high) / 2;
        if (arr[middle] > arr[high])    ELEM_SWAP(arr[middle], arr[high]) ;
        if (arr[low] > arr[high])       ELEM_SWAP(arr[low], arr[high]) ;
        if (arr[middle] > arr[low])     ELEM_SWAP(arr[middle], arr[low]) ;

        /* Swap low item (now in position middle) into position (low+1) */
        ELEM_SWAP(arr[middle], arr[low+1]) ;

        /* Nibble from each end towards middle, swapping items when stuck */
        ll = low + 1;
        hh = high;
        for (;;)
                {
                 do ll++; while (arr[low] > arr[ll]) ;
                 do hh--; while (arr[hh]  > arr[low]) ;

                 if (hh < ll)
                        break;

                 ELEM_SWAP(arr[ll], arr[hh]) ;
                }

        /* Swap middle item (in position low) back into correct position */
        ELEM_SWAP(arr[low], arr[hh]) ;

        /* Re-set active partition */
        if (hh <= median)
                low = ll;
        if (hh >= median)
                high = hh - 1;
       }
}

#undef ELEM_SWAP
#undef elem_type


void TScientificGraph::fnMedian_filt(unsigned int median_ahead, int iGraphNumberF ) // apply median filter to graph in place
{// mt=median(mint+,maxt+,mt-1)  - min, max look ahead 2+ . This algorithm provides a fast (if median_ahead is smallish) but good approximation to a true median
 if(median_ahead>1)
        {double m,ymin,ymax;
         SGraph *pAGraph = ((SGraph*) pHistory->Items[iGraphNumberF]);
         TList *pAList = pAGraph->pDataList;
         unsigned int iCount=pAList->Count;
         for (unsigned int i=0; i<iCount; i++)  // for all items in list
                {if(i==0)
                        m=((SDataPoint*)pAList->Items[i])->dYValue; // initial value
                 if(i+1<iCount)
                        {// find min/max up to median_ahead
                         ymax=ymin=((SDataPoint*)pAList->Items[i+1])->dYValue;
                         for (unsigned int j=2;j<=median_ahead && i+j<iCount; j++)
                                {double t= ((SDataPoint*)pAList->Items[i+j])->dYValue;
                                 if(t>ymax) ymax=t;
                                 else if(t<ymin) ymin=t;
                                }
                         if(m>ymax) m=ymax; // median cal
                         else if(m<ymin) m=ymin; // else m is median
                        }
                 ((SDataPoint*)pAList->Items[i])->dYValue=m; // put back filtered value
                }
        }
}


#if 0
 // This code was written from scratch 28/12/2019 Peter Miller
 // code looks ahead from the end of the previous lookahead so that portion of the code examines each x value once.
 // knowing starting and ending value of range (from above) we can take equally spaced samples and then another set of samples with a "prime" period to the 1st
 //  This is functionally the same as fnMedian_filt_time below (just a few optiisations in that code).
 // it should be considered a "generic prototype" to be used to create specific versions of the code rather than to be directly used (other than for trials/debuging).
 //
 void TScientificGraph::fnMedian_filt_time1(double median_ahead_t, int iGraphNumberF, void (*callback)(unsigned int cnt,unsigned int maxcnt)) // apply median filter to graph in place  , lookahead defined in time
{
 // callback() is called periodically to let caller know progress   . This is done based on time (once/sec).
 time_t lastT;
 unsigned int i,j,last_endi=0,endi,jinc ;
 float this_endT,maxy,miny,nearesty,y,m;
 SGraph *pAGraph = ((SGraph*) pHistory->Items[iGraphNumberF]);
 TList *pAList = pAGraph->pDataList;
 unsigned int maxi=pAList->Count;
 lastT=clock(); // used to keep callbacks at uniform time intervals
 if(median_ahead_t>0 && maxi>=3) // need at least 3 points for initial median and need a positive value for the look ahead time
        {m=median3(((SDataPoint*)pAList->Items[0])->dYValue,((SDataPoint*)pAList->Items[1])->dYValue,((SDataPoint*)pAList->Items[2])->dYValue);
         //rprintf("initial median of 1st 3 points (%g,%g,%g) selected as %g\n",((SDataPoint*)pAList->Items[0])->dYValue,((SDataPoint*)pAList->Items[1])->dYValue,((SDataPoint*)pAList->Items[2])->dYValue,m);
         for(i=0;i<maxi;++i)
                {// process all points one by one
                 if(callback!=NULL && (i & 0xffff)==0 && (clock()-lastT)>= CLOCKS_PER_SEC)
                        {lastT=clock();   // update on progress every second (approximately - use i & 0xff to keep average overhead of time() check very low
                         (*callback)(i,maxi);
                        }
                 this_endT=((SDataPoint*)pAList->Items[i])->dXValue+median_ahead_t; // time for the end of the look ahead
                 for(endi=last_endi;endi<maxi && ((SDataPoint*)pAList->Items[endi])->dXValue < this_endT;++endi) // search forward to find i that matches end of look ahead time
                        {
                         if(i==0)
                                {// do real average for 1st point [as we can and it gives us a better starting value, especially for big lookaheads]
                                 m+=(((SDataPoint*)pAList->Items[endi])->dYValue-m)/(float)(endi+1);
                                }
                         }
                 // now we need to process from i to endi, if this is a lot of points limit to max 63 [range 63 jinc=1 (63 points), range 64 jinc=2 (32 points)  ]
                 jinc=1+(endi-i)/64;  // 64 is a power of 2 for efficiency, but also visibally seems to give sensible results, while being faster than the original code
                 last_endi=endi; // ensure we don't check values we have already processed
                 // now search for min,max,nearest y
                 maxy=miny=nearesty= ((SDataPoint*)pAList->Items[i])->dYValue;  // this is 1st value so loop below start on 2nd value
                 for(j=i+jinc;j<maxi && j<endi;j+=jinc)
                        {y= ((SDataPoint*)pAList->Items[j])->dYValue;
                         if(y>maxy) maxy=y;
                         if(y<miny) miny=y;
                         if(fabs(y-m)<fabs(nearesty-m)) nearesty=y;
                        }
                 if(jinc>1) //  if it was 1 we have already sampled every point, otherwise take a 2nd pass
                        {
#if 0
                         // take a 2nd pass - do backwards so we guarantee to include last point and use jinc+1 as step (so swap odd/even and ~ relatively prime)
                         // by eye this is very similar to the potentially more "random" power of 2 -1 algorithm below, but a bit slower to execute, so its not normally used
                         jinc++; // swap odd<->even and adjacent numbers are frequently relatively prime (definately has a different period)
                         for(j=min(endi-1,maxi-1);j>=i+jinc;j-=jinc) // note j is unsigned, test  j>=i+jinc ensures it cannot go negative
                                {y= ((SDataPoint*)pAList->Items[j])->dYValue;
                                 if(y>maxy) maxy=y;
                                 if(y<miny) miny=y;
                                 if(fabs(y-m)<fabs(nearesty-m)) nearesty=y;
                                }
#else
                         // now make another pass taking more samples (but less numerically than loop above) that are not (very) harmonicaly related to those taken above
                         // this is faster than the #else code for big lookaheads as the while loop just deletes low order bits in inc rather than potentially incrementing it a lot of times
                         jinc++;
                         while(jinc&(jinc-1)) jinc&=jinc-1; // decrease jinc by deleting lower order bits until its a power of 2
                         jinc=(jinc<<1)-1; // a power of 2 -1 is frequently prime, at least its unlikley to have common factors with the original jinc used above
                         // for example if jinc was 2 originally on the 2nd pass it will become 3, if it was 3,4,5 or 6  it will become 7
                         for(j=i+jinc;j<maxi && j<endi;j+=jinc)
                                {y= ((SDataPoint*)pAList->Items[j])->dYValue;
                                 if(y>maxy) maxy=y;
                                 if(y<miny) miny=y;
                                 if(fabs(y-m)<fabs(nearesty-m)) nearesty=y;
                                }
#endif
                        }
                 // approximate median m must be between miny and maxy
                 if(m<miny) m=miny;
                 if(m>maxy) m=maxy;
                 ((SDataPoint*)pAList->Items[i])->dYValue=m; // put back filtered value
                }
         }
 }
#elif 1
 //        U S E   T H I S    V E R S I O N  !!!
 //        =====================================  (normally - see comments of this and alternatives)
 // This code was written from scratch 28/12/2019 Peter Miller
 // code looks ahead from the end of the previous lookahead so that portion of the code examines each x value once.
 // knowing starting and ending value of range (from above) we can take equally spaced samples and then another set of samples with a "prime" period to the 1st
 // This uses a linear filter as well as the median - using the linear filter (which looks ahead median_ahead_t) directly unless clipped to stay in the range of the median
 // This also shows a few affects by only sampling on the look ahead - but its probably the best compromise for "median1" as for moderate look aheads it keeps the median in the centre of the "noise band" in regions where y is constant or slowly changing
 //  and for long lookaheads initialisation to the average at the start works well.
 void TScientificGraph::fnMedian_filt_time1(double median_ahead_t, int iGraphNumberF, void (*callback)(unsigned int cnt,unsigned int maxcnt)) // apply median filter to graph in place  , lookahead defined in time
{
 // callback() is called periodically to let caller know progress   . This is done based on time (once/sec).
 time_t lastT;
 unsigned int i,j,last_endi=0,endi,jinc ;
 float this_endT,maxy,miny,nearesty,y,m;
 float f,dt; // linear filtered value (range limited based on median limits)
 float k,lastdt=0,x,lastx=0;
 SGraph *pAGraph = ((SGraph*) pHistory->Items[iGraphNumberF]);
 TList *pAList = pAGraph->pDataList;
 unsigned int maxi=pAList->Count;
 lastT=clock(); // used to keep callbacks at uniform time intervals
 if(median_ahead_t>0 && maxi>=3) // need at least 3 points for initial median and need a positive value for the look ahead time
        {f=m=median3(((SDataPoint*)pAList->Items[0])->dYValue,((SDataPoint*)pAList->Items[1])->dYValue,((SDataPoint*)pAList->Items[2])->dYValue);
         //rprintf("initial median of 1st 3 points (%g,%g,%g) selected as %g\n",((SDataPoint*)pAList->Items[0])->dYValue,((SDataPoint*)pAList->Items[1])->dYValue,((SDataPoint*)pAList->Items[2])->dYValue,m);
         for(i=0;i<maxi;++i)
                {// process all points one by one
                 if(callback!=NULL && (i & 0xffff)==0 && (clock()-lastT)>= CLOCKS_PER_SEC)
                        {lastT=clock();   // update on progress every second (approximately - use i & 0xff to keep average overhead of time() check very low
                         (*callback)(i,maxi);
                        }
                 this_endT=((SDataPoint*)pAList->Items[i])->dXValue+median_ahead_t; // time for the end of the look ahead
                 if(last_endi!=0) lastx=((SDataPoint*)pAList->Items[last_endi-1])->dXValue;
                 for(endi=last_endi;endi<maxi && ((SDataPoint*)pAList->Items[endi])->dXValue < this_endT;++endi) // search forward to find i that matches end of look ahead time
                        { // update linear filter while we do this so it "runs" median_ahead_t in advance of current location so approximately gives average of +/-median_ahead_t around current time
                         if(i!=0) // if i==0 don't do anything here as we will calculate the actual median below and use this to initialise f
                                {x= ((SDataPoint*)pAList->Items[endi])->dXValue;
                                 dt= x - lastx;
                                 lastx=x;
                                 if(dt>0)
                                        {// above if avoids possible divide by zero below
                                         if(dt!=lastdt) // only calculate k when dt changes [ saves ~ 1 sec ]
                                                {k=1.0-exp(-(dt)/(2.0*median_ahead_t)); // *2.0 as we want to look forward and back by median_ahead_t
                                                 lastdt=dt;
                                                }
                                         f+=k*(((SDataPoint*)pAList->Items[endi])->dYValue-f);
                                        }
                                }

                        }
                 if(i==0)
                        {// do exact median by taking a copy of the values, and using quick_select() to find the median
                         float *arr=(float *)calloc(endi,sizeof(float));
                         if(arr!=NULL)
                                {
                                 for(unsigned int k=0; k<endi;++k)   // take a copy of all the y values that we want median of, also get min,max,average
                                        {
                                         arr[k]= ((SDataPoint*)pAList->Items[k])->dYValue;
                                        }
                                 f=quick_select( arr, endi); // initialise with actual median as its quick to calculate
                                free(arr);
                               }
                        }
                 last_endi=endi; // ensure we don't check values we have already processed
                 if(endi>=maxi)
                        {// lookahead goes beyond end of data , in this case we assume the limits are unbounded and leave current median value alone
                         ((SDataPoint*)pAList->Items[i])->dYValue=f; // put back current filtered value
                          continue;
                        }
                 // now we need to process from i to endi, if this is a lot of points limit to max 63 [range 63 jinc=1 (63 points), range 64 jinc=2 (32 points)  ]
                 jinc=1+(endi-i)/64;  // 64 is a power of 2 for efficiency, but also visibally seems to give sensible results, while being faster than the original code
                 // now search for min,max,nearest y
                 maxy=miny=nearesty= ((SDataPoint*)pAList->Items[i])->dYValue;  // this is 1st value so loop below start on 2nd value

                 for(j=i+jinc;j<maxi && j<endi;j+=jinc)
                        {y= ((SDataPoint*)pAList->Items[j])->dYValue;
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
                                {y= ((SDataPoint*)pAList->Items[j])->dYValue;
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
                 ((SDataPoint*)pAList->Items[i])->dYValue=m; // put back filtered value
                }
         }
 }
#elif 0
 // This code was written from scratch 28/12/2019 Peter Miller
 // code looks ahead from the end of the previous lookahead so that portion of the code examines each x value once.
 // knowing starting and ending value of range (from above) we can take equally spaced samples and then another set of samples with a "prime" period to the 1st
 // This uses a linear filter as well as the median - using the linear filter (which looks ahead median_ahead_t) directly unless clipped to stay in the range of the median
 // the nearest value to the linear filter is returned so its always a value thats in the input (just like a real median)
 // This also shows a few affects by only sampling on the look ahead , but mainly it "looks strange" as even with large look aheads the values are noisy as the nearest found changes with time
 //
 void TScientificGraph::fnMedian_filt_time1(double median_ahead_t, int iGraphNumberF, void (*callback)(unsigned int cnt,unsigned int maxcnt)) // apply median filter to graph in place  , lookahead defined in time
{
 // callback() is called periodically to let caller know progress   . This is done based on time (once/sec).
 time_t lastT;
 unsigned int i,j,last_endi=0,endi,jinc ;
 float this_endT,maxy,miny,nearesty,y,m;
 float f,dt; // linear filtered value (range limited based on median limits)
 float k,lastdt=0;
 SGraph *pAGraph = ((SGraph*) pHistory->Items[iGraphNumberF]);
 TList *pAList = pAGraph->pDataList;
 unsigned int maxi=pAList->Count;
 lastT=clock(); // used to keep callbacks at uniform time intervals
 if(median_ahead_t>0 && maxi>=3) // need at least 3 points for initial median and need a positive value for the look ahead time
        {f=m=median3(((SDataPoint*)pAList->Items[0])->dYValue,((SDataPoint*)pAList->Items[1])->dYValue,((SDataPoint*)pAList->Items[2])->dYValue);
         //rprintf("initial median of 1st 3 points (%g,%g,%g) selected as %g\n",((SDataPoint*)pAList->Items[0])->dYValue,((SDataPoint*)pAList->Items[1])->dYValue,((SDataPoint*)pAList->Items[2])->dYValue,m);
         for(i=0;i<maxi;++i)
                {// process all points one by one
                 if(callback!=NULL && (i & 0xffff)==0 && (clock()-lastT)>= CLOCKS_PER_SEC)
                        {lastT=clock();   // update on progress every second (approximately - use i & 0xff to keep average overhead of time() check very low
                         (*callback)(i,maxi);
                        }
                 this_endT=((SDataPoint*)pAList->Items[i])->dXValue+median_ahead_t; // time for the end of the look ahead

                 for(endi=last_endi;endi<maxi && ((SDataPoint*)pAList->Items[endi])->dXValue < this_endT;++endi) // search forward to find i that matches end of look ahead time
                        { // update linear filter while we do this so it "runs" median_ahead_t in advance of current location so approximately gives average of +/-median_ahead_t around current time
                         if(i==0)
                                {// do real average for 1st point [as we can and it gives us a better starting value, especially for big lookaheads]
                                 f+=(((SDataPoint*)pAList->Items[endi])->dYValue-f)/(float)(endi+1);
                                }
                         else
                                {dt=((SDataPoint*)pAList->Items[endi])->dXValue - ((SDataPoint*)pAList->Items[endi-1])->dXValue;
                                 if(dt>0)
                                        {// above if avoids possible divide by zero below
                                         if(dt!=lastdt) // only calculate k when dt changes [ saves ~ 1 sec ]
                                                {k=1.0-exp(-(dt)/(2.0*median_ahead_t)); // *2.0 as we want to look forward and back by median_ahead_t
                                                }
                                         f+=k*(((SDataPoint*)pAList->Items[endi])->dYValue-f);
                                        }
                                }

                        }
                 // now we need to process from i to endi, if this is a lot of points limit to max 63 [range 63 jinc=1 (63 points), range 64 jinc=2 (32 points)  ]
                 jinc=1+(endi-i)/64;  // 64 is a power of 2 for efficiency, but also visibally seems to give sensible results, while being faster than the original code
                 last_endi=endi; // ensure we don't check values we have already processed
                 // now search for min,max,nearest y
                 maxy=miny=nearesty= ((SDataPoint*)pAList->Items[i])->dYValue;  // this is 1st value so loop below start on 2nd value

                 for(j=i+jinc;j<maxi && j<endi;j+=jinc)
                        {y= ((SDataPoint*)pAList->Items[j])->dYValue;
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
                         // for example if jinc was 2 originally on the 2nd pass it will become 3, if it was 3,4,5 or 6  it will become 7
                         for(j=i+jinc;j<maxi && j<endi;j+=jinc)
                                {y= ((SDataPoint*)pAList->Items[j])->dYValue;
                                 if(y>maxy) maxy=y;
                                 if(y<miny) miny=y;
                                 if(fabs(y-f)<fabs(nearesty-f)) nearesty=y;
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
                            {m=nearesty; //  use nearest value found to current filtered value
                            }
                       }
                 ((SDataPoint*)pAList->Items[i])->dYValue=m; // put back filtered value
                }
         }
 }

#elif 1
 // This has become a "test bed" for various median approximations
 // it includes an exact calculation of the median for comparison
 // see rprint outputs for results
 //
 /* another version - this uses an iterative approximation to the median for initialisation rather than the average
    Algorithm from
    https://stackoverflow.com/questions/1058813/on-line-iterator-algorithms-for-estimating-statistical-median-mode-skewnes
    the algorithm is:
    cumadev += abs(sample-median)
    eta = 1.5*cumadev/(k*k), where k is the number of samples seen so far
             note that cumadev/k is average dev so eta=1.5*adev/k  , and it may be better to use an incremental average rather than cumadev.
    median += eta * sgn(sample - median) where sgn is the signum function which returns one of {-1, 0, 1}  [here it just needs to return -1 or 1 ]

    While this does give a reasonably sensible result there seems to be no reason to use it over "average" which is a well known and robust algorithm.
 */
 //  this is a version of the generic code above that used the above algorithm for the initial initialisation instead of using the average

int floatcomp(const void* elem1, const void* elem2) // needed to sort floats with qsort function
{
    if(*(const float*)elem1 < *(const float*)elem2)
        return -1;
    return *(const float*)elem1 > *(const float*)elem2; // returns 0 on equal
}



 void TScientificGraph::fnMedian_filt_time1(double median_ahead_t, int iGraphNumberF, void (*callback)(unsigned int cnt,unsigned int maxcnt)) // apply median filter to graph in place  , lookahead defined in time
{
 // callback() is called periodically to let caller know progress   . This is done based on time (once/sec).
 time_t lastT;
 unsigned int i,j,last_endi=0,endi,jinc ;
 float this_endT,maxy,miny,nearesty,y,m;
 double cumadev=0,eta;
 SGraph *pAGraph = ((SGraph*) pHistory->Items[iGraphNumberF]);
 TList *pAList = pAGraph->pDataList;
 unsigned int maxi=pAList->Count;
 lastT=clock(); // used to keep callbacks at uniform time intervals
 if(median_ahead_t>0 && maxi>=3) // need at least 3 points for initial median and need a positive value for the look ahead time
        {m=median3(((SDataPoint*)pAList->Items[0])->dYValue,((SDataPoint*)pAList->Items[1])->dYValue,((SDataPoint*)pAList->Items[2])->dYValue);
         //rprintf("initial median of 1st 3 points (%g,%g,%g) selected as %g\n",((SDataPoint*)pAList->Items[0])->dYValue,((SDataPoint*)pAList->Items[1])->dYValue,((SDataPoint*)pAList->Items[2])->dYValue,m);
         for(i=0;i<maxi;++i)
                {// process all points one by one
                 if(callback!=NULL && (i & 0xffff)==0 && (clock()-lastT)>= CLOCKS_PER_SEC)
                        {lastT=clock();   // update on progress every second (approximately - use i & 0xff to keep average overhead of time() check very low
                         (*callback)(i,maxi);
                        }
                 this_endT=((SDataPoint*)pAList->Items[i])->dXValue+median_ahead_t; // time for the end of the look ahead
                 for(endi=last_endi;endi<maxi && ((SDataPoint*)pAList->Items[endi])->dXValue < this_endT;++endi) // search forward to find i that matches end of look ahead time
                        {
                         if(i==0)
                                {
#if 1
                                 // use new approximation for the median rather than the average here
#define sgn(x) ((x)<0?-1.0:1.0)
                                 cumadev+=fabs(((SDataPoint*)pAList->Items[endi])->dYValue-m);
                                 eta=1.5*cumadev/((float)(endi+1)*(float)(endi+1));
                                 m+=eta*sgn(((SDataPoint*)pAList->Items[endi])->dYValue-m);
#else
                                 m+=(((SDataPoint*)pAList->Items[endi])->dYValue-m)/(float)(endi+1);
#endif
                                }
                         }
                 if(i==0)
                        {
#if 0
                         // calculate exact median
                         if(endi>=maxi) endi=maxi-1;
                         float *arr=(float *)calloc(endi,sizeof(float));
                         if(arr!=NULL)
                                {
                                 for(unsigned int k=0; k<endi;++k)   // take a copy of all the y values that we want median of, also get min,max,average
                                        {
                                         arr[k]= ((SDataPoint*)pAList->Items[k])->dYValue;
                                        }
                                 m=quick_select( arr, endi); // initialise with actual median as its quick to calculate
                                free(arr);
                               }
#else
                         // do exact median by taking a copy of the values, qsorting them and using the mid element
                         float *arr=(float *)calloc(endi,sizeof(float));
                         float qs;
                         unsigned int mid=(endi>>1)-1;// as array is 0..endi-1 eg if 10 elements (endi=10) array is 0..9, mid=10/2-1=4 (and mid+1=5)
                         float meany;
                         for(unsigned int k=0; k<endi;++k)   // take a copy of all the y values that we want median of, also get min,max,average
                                {y= ((SDataPoint*)pAList->Items[k])->dYValue;
                                 arr[k]= y;
                                 if(k==0)
                                        {maxy=miny=meany=y;
                                        }
                                 else
                                        {if(y>maxy) maxy=y;
                                         if(y<miny) miny=y;
                                         meany+=(y-meany)/(float)(k+1);
                                        }
                                }
                         rprintf("quick_select() starting\n");
                         lastT=clock();
                         qs=quick_select( arr, endi);
                         rprintf("quick_select() complete after %.2f secs\n median=%g\n",(clock()-lastT)/CLOCKS_PER_SEC,qs);
                         rprintf("qsort of %u elements starting\n",endi);
                         lastT=clock(); // warning - we start with the array as left by quick_select which will be different to the original array in order
                         qsort(arr, endi, sizeof(float), floatcomp);
                         rprintf("qsort complete after %.2f secs",(clock()-lastT)/CLOCKS_PER_SEC);
                         rprintf(" middle element [%u]=%g mid+1=%g.\n Median= average of [mid],[mid+1]=%g\n",mid,arr[mid],arr[mid+1],0.5*(arr[mid]+arr[mid+1]) );

                         rprintf("Approximate median by eta algorithm is %g\n",m);
                         rprintf("Average value =%g min=%g max=%g (min+max)/2=%g\n",meany,miny,maxy,0.5*(miny+maxy) );

                         m= 0.5*(arr[mid]+arr[mid+1]); // given we have actually calculated it, use actual median for the rest of the code below.
                         free(arr);
#endif
                        }
                 // now we need to process from i to endi, if this is a lot of points limit to max 63 [range 63 jinc=1 (63 points), range 64 jinc=2 (32 points)  ]
                 jinc=1+(endi-i)/64;  // 64 is a power of 2 for efficiency, but also visibally seems to give sensible results, while being faster than the original code
                 last_endi=endi; // ensure we don't check values we have already processed
                 // now search for min,max,nearest y
                 maxy=miny=nearesty= ((SDataPoint*)pAList->Items[i])->dYValue;  // this is 1st value so loop below start on 2nd value
                 for(j=i+jinc;j<maxi && j<endi;j+=jinc)
                        {y= ((SDataPoint*)pAList->Items[j])->dYValue;
                         if(y>maxy) maxy=y;
                         if(y<miny) miny=y;
                         if(fabs(y-m)<fabs(nearesty-m)) nearesty=y;
                        }
                 if(jinc>1) //  if it was 1 we have already sampled every point, otherwise take a 2nd pass
                        {
#if 0
                         // take a 2nd pass - do backwards so we guarantee to include last point and use jinc+1 as step (so swap odd/even and ~ relatively prime)
                         // by eye this is very similar to the potentially more "random" power of 2 -1 algorithm below, but a bit slower to execute, so its not normally used
                         jinc++; // swap odd<->even and adjacent numbers are frequently relatively prime (definately has a different period)
                         for(j=min(endi-1,maxi-1);j>=i+jinc;j-=jinc) // note j is unsigned, test  j>=i+jinc ensures it cannot go negative
                                {y= ((SDataPoint*)pAList->Items[j])->dYValue;
                                 if(y>maxy) maxy=y;
                                 if(y<miny) miny=y;
                                 if(fabs(y-m)<fabs(nearesty-m)) nearesty=y;
                                }
#else
                         // now make another pass taking more samples (but less numerically than loop above) that are not (very) harmonicaly related to those taken above
                         // this is faster than the #else code for big lookaheads as the while loop just deletes low order bits in inc rather than potentially incrementing it a lot of times
                         jinc++;
                         while(jinc&(jinc-1)) jinc&=jinc-1; // decrease jinc by deleting lower order bits until its a power of 2
                         jinc=(jinc<<1)-1; // a power of 2 -1 is frequently prime, at least its unlikley to have common factors with the original jinc used above
                         // for example if jinc was 2 originally on the 2nd pass it will become 3, if it was 3,4,5 or 6  it will become 7
                         for(j=i+jinc;j<maxi && j<endi;j+=jinc)
                                {y= ((SDataPoint*)pAList->Items[j])->dYValue;
                                 if(y>maxy) maxy=y;
                                 if(y<miny) miny=y;
                                 if(fabs(y-m)<fabs(nearesty-m)) nearesty=y;
                                }
#endif
                        }
                 // approximate median m must be between miny and maxy
                 if(m<miny) m=miny;
                 if(m>maxy) m=maxy;
                 ((SDataPoint*)pAList->Items[i])->dYValue=m; // put back filtered value
                }
         }
 }

#endif

#if 1
// This code was written from scratch 28/12/2019 Peter Miller
 // code looks ahead from the end of the previous lookahead so that portion of the code examines each x value once.
 // knowing starting and ending value of range (from above) we can take equally spaced samples and then another set of samples with a "prime" period to the 1st
 //
 //
 void TScientificGraph::fnMedian_filt_time(double median_ahead_t, int iGraphNumberF, void (*callback)(unsigned int cnt,unsigned int maxcnt)) // apply median filter to graph in place  , lookahead defined in time
{
 // callback() is called periodically to let caller know progress   . This is done based on time (once/sec).
 time_t lastT;
 unsigned int i,j,last_endi=0,endi,jinc ;
 float this_endT,maxy,miny,y,m;
 SGraph *pAGraph = ((SGraph*) pHistory->Items[iGraphNumberF]);
 TList *pAList = pAGraph->pDataList;
 unsigned int maxi=pAList->Count;
 lastT=clock(); // used to keep callbacks at uniform time intervals
 if(median_ahead_t>0 && maxi>=3) // need at least 3 points for initial median and need a positive value for the look ahead time
        {m=median3(((SDataPoint*)pAList->Items[0])->dYValue,((SDataPoint*)pAList->Items[1])->dYValue,((SDataPoint*)pAList->Items[2])->dYValue);
         //rprintf("initial median of 1st 3 points (%g,%g,%g) selected as %g\n",((SDataPoint*)pAList->Items[0])->dYValue,((SDataPoint*)pAList->Items[1])->dYValue,((SDataPoint*)pAList->Items[2])->dYValue,m);
         for(i=0;i<maxi;++i)
                {// process all points one by one
                 if(callback!=NULL && (i & 0xffff)==0 && (clock()-lastT)>= CLOCKS_PER_SEC)
                        {lastT=clock();   // update on progress every second (approximately - use i & 0xff to keep average overhead of time() check very low
                         (*callback)(i,maxi);
                        }
                 this_endT=((SDataPoint*)pAList->Items[i])->dXValue+median_ahead_t; // time for the end of the look ahead
                 for(endi=last_endi;endi<maxi && ((SDataPoint*)pAList->Items[endi])->dXValue < this_endT;++endi); // search forward to find i that matches end of look ahead time
                 if(i==0)
                        {// do exact median by taking a copy of the values, and using quick_select() to find the median
                         float *arr=(float *)calloc(endi,sizeof(float));
                         if(arr!=NULL)
                                {
                                 for(unsigned int k=0; k<endi;++k)   // take a copy of all the y values that we want median of, also get min,max,average
                                        {
                                         arr[k]= ((SDataPoint*)pAList->Items[k])->dYValue;
                                        }
                                 m=quick_select( arr, endi); // initialise with actual median as its quick to calculate
                                free(arr);
                               }
                        }
                 last_endi=endi; // ensure we don't check values we have already processed
                 if(endi>=maxi)
                        {// lookahead go beyond end of data , in this case we assume the limits are unbounded and leave current median value alone
                         ((SDataPoint*)pAList->Items[i])->dYValue=m; // put back current median value
                          continue;
                        }
                 // now we need to process from i to endi, if this is a lot of points limit to max 63 [range 63 jinc=1 (63 points), range 64 jinc=2 (32 points)  ]
                 jinc=1+(endi-i)/64;  // 64 is a power of 2 for efficiency, but also visibally seems to give sensible results, while being faster than the original code

                 // now search for min,max
                 maxy=miny= ((SDataPoint*)pAList->Items[i])->dYValue;  // this is 1st value so loop below start on 2nd value
                 for(j=i+jinc;j<maxi && j<endi;j+=jinc)
                        {y= ((SDataPoint*)pAList->Items[j])->dYValue;
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
                                {y= ((SDataPoint*)pAList->Items[j])->dYValue;
                                 if(y>maxy) maxy=y;
                                 if(y<miny) miny=y;
                                }
                        }
                 // approximate median m must be between miny and maxy
                 if(m<miny) m=miny;
                 if(m>maxy) m=maxy;
                 ((SDataPoint*)pAList->Items[i])->dYValue=m; // put back filtered value
                }
         }
 }

#else
void TScientificGraph::fnMedian_filt_time(double median_ahead_t, int iGraphNumberF, void (*callback)(unsigned int cnt,unsigned int maxcnt)) // apply median filter to graph in place  , lookahead defined in time
{// mt=median(mint+,maxt+,mt-1)  - min, max look ahead 2+ . This algorithm provides a reasonably fast but good approximation to a true median
 // look ahead based on time is designed to be robust to rounding errors (as x values are floats) - the limit value is average(x[i+j],x[i+j+1])
 // callback() is called periodically to let caller know progress
 if(median_ahead_t>0)
        {double m,ymin,ymax;
         double lastx,xij;
         SGraph *pAGraph = ((SGraph*) pHistory->Items[iGraphNumberF]);
         TList *pAList = pAGraph->pDataList;
         unsigned int iCount=pAList->Count;
         if(iCount<2) return; // not enough data in graph to process
         m=((SDataPoint*)pAList->Items[0])->dYValue; // initial value
         for (unsigned int i=0; i<iCount; i++)  // for all items in list
                {
                 if(callback!=NULL && (i & 0x3ffff)==0)
                        (*callback)(i,iCount); // update on progress
                 if(i+2<iCount)
                        {// find min/max up to median_ahead
                         ymax=ymin=((SDataPoint*)pAList->Items[i+1])->dYValue;
                         lastx=((SDataPoint*)pAList->Items[i])->dXValue;
                         xij=((SDataPoint*)pAList->Items[i+2])->dXValue;
                         unsigned int inc=1,nos_samples=0;
                         for (unsigned int j=2;
                                i+j+1<iCount &&
                                median_ahead_t>0.5*(xij+ ((SDataPoint*)pAList->Items[i+j+1])->dXValue)-lastx;
                                j+=inc)
                                {xij=((SDataPoint*)pAList->Items[i+j+1])->dXValue;
                                 double t= ((SDataPoint*)pAList->Items[i+j])->dYValue;
                                 if(t>ymax) ymax=t;
                                 else if(t<ymin) ymin=t;
                                 // above loop can require lots of iterations if sample time is small (eg 1ms), start skipping values if we have already sampled a reasonable number
                                 nos_samples++;
                                 if((nos_samples&0xf)==0)
                                        {if(inc&0xfffffff)
                                          inc<<=1; // *2 , if test stops overflow
                                        }
                                }
                         if(m>ymax) m=ymax; // median cal
                         else if(m<ymin) m=ymin; // else m is median
                        }
                 ((SDataPoint*)pAList->Items[i])->dYValue=m; // put back filtered value
                }
        }
}
#endif

void TScientificGraph::fnLinear_filt_time(double tc, int iGraphNumberF, void (*callback)(unsigned int cnt,unsigned int maxcnt)) // apply linear filter to graph in place
{// f(t)+=k*(y(t)+f(t-1))
 // k=1-exp(-dt/tc) where dt is time between samples
 // callback() is called periodically to let caller know progress
 if(tc>0)
        {double m;
         double lastx,x,y,k;
		 SGraph *pAGraph = ((SGraph*) pHistory->Items[iGraphNumberF]);
		 TList *pAList = pAGraph->pDataList;
		 unsigned int iCount=pAList->Count;
		 if(iCount<2) return; // not enough data in graph to process
		 m=((SDataPoint*)pAList->Items[0])->dYValue; // initial value
		 lastx=((SDataPoint*)pAList->Items[0])->dXValue;
         for (unsigned int i=0; i<iCount; i++)  // for all items in list
                {
				 y=((SDataPoint*)pAList->Items[i])->dYValue;
				 x=((SDataPoint*)pAList->Items[i])->dXValue;
                 if(x>lastx)
                        {// above if avoids possible maths error below
                         k=1.0-exp(-(x-lastx)/tc);
                         m+=k*(y-m);
                        }
                 lastx=x;
				 if(callback!=NULL && (i & 0x3fffff)==0)
						(*callback)(i,iCount); // update on progress
				 ((SDataPoint*)pAList->Items[i])->dYValue=m; // put back filtered value
                }
        }
}

void TScientificGraph::fnLinreg(int iGraphNumberF, void (*callback)(unsigned int cnt,unsigned int maxcnt))
 // apply 1st order least squares linear regression (y=mx+c) to graph in place
{// to save copying data this is done inline
 SGraph *pAGraph = ((SGraph*) pHistory->Items[iGraphNumberF]);
 TList *pAList = pAGraph->pDataList;
 unsigned int iCount=pAList->Count;

 double meanx=0,meany=0; /* initial values set to mean that N=0 or N=1 do not need to be treated as special cases below */
 double meanx2=0,meanxy=0,meany2=0; /* mean x^2 , mean x*y and mean y^2 */
 double xi,yi;
 double m,c,r2; // results of least squares fit
 unsigned int i,N=0; /* N is count of items */
 if(iCount<2) return; // not enough data in graph to process
 for(i=0;i<iCount;++i) /* only use 1 pass here - to calculate means directly */
		{++N;
		 xi=((SDataPoint*)pAList->Items[i])->dXValue;
		 yi=((SDataPoint*)pAList->Items[i])->dYValue;
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
		 m=rm;
		 c=meany-rm*meanx; /* y=mx+c so c=y-mx */
		 rt=(meanxy-meanx*meany);
		 rb=(meany2-meany*meany);
		 // rprintf("rm=%g rt=%g rb=%g rt/rb=%g \n",rm,rt,rb,rt/rb);
		 if(rb!=0)     /* trap divide by zero */
			r2= rm * (rt/rb) ;
		  else
			r2=1.0;/* should be in range 0-1 */
		 }
 rprintf("Best Least squares straight line is Y=%g*X+%g which has an R^2 of %g\n",m,c,r2);
 // now put new y values back, calculated as y=m*x+c
 for(i=0;i<iCount;++i)
	{
	 xi=((SDataPoint*)pAList->Items[i])->dXValue;
	 ((SDataPoint*)pAList->Items[i])->dYValue=m*xi+c;
	 if(callback!=NULL && (i & 0x3fffff)==0)
		(*callback)((i>>1) + (iCount>>1),iCount); // update on progress , this is 2nd pass through data so goes 50% - 100%
    }
}


bool TScientificGraph::fnPolyreg(unsigned int order,int iGraphNumberF, void (*callback)(unsigned int cnt,unsigned int maxcnt))
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
 TList *pAList = pAGraph->pDataList;
 unsigned int iCount=pAList->Count;
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
 if(order> (iCount>>1))
	{order=iCount>>1; // imperical observation is order > 1/2 total number of points then bad things happen numerically, so trap that here
	}
 try{ /* the code below may fail if given a high enough order so trap that here */
	minx=((SDataPoint*)pAList->Items[0])->dXValue;  // xvalues are sorted into order
	maxx=((SDataPoint*)pAList->Items[iCount-1])->dXValue;
	miny=maxy=((SDataPoint*)pAList->Items[0])->dYValue;
	for(i=0;i<iCount;++i)
		{y=((SDataPoint*)pAList->Items[i])->dYValue;
		 if(y>maxy) maxy=y;
		 if(y<miny) miny=y;
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
			{x=((SDataPoint*)pAList->Items[i])->dXValue;
			 y=((SDataPoint*)pAList->Items[i])->dYValue;
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
				{x=((SDataPoint*)pAList->Items[i])->dXValue;
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
	{x= ((SDataPoint*)pAList->Items[i])->dXValue;
	 y= ((SDataPoint*)pAList->Items[i])->dYValue;
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
			maxep=fabs(err); // calculate max abs error
		 meane2p+=(err*err-meane2p)/(long double)(i+1);  // incremental update for mean(error^2)
		}
	 // now do for orthogonal poly  . This method should be accurate for all orders of polynomial.
	 x=x*xm+xc;    // scale x
	 pv=0;
	 qv=1.0;
	 sum=sy[0];
	 for(j=1;j<=order;++j)   // calc orthogonal poly at x
		{double t;
		 t=pv;pv=qv;qv=t; // swap(pv,qv)
		 qv=((x-sx[j-1])*pv)-(v[j-1]*qv);
		 sum+=sy[j]*qv;
		}

	  sum=(sum-yc)/ym; // inverse scale y so sum is now new value of y calculated from the polynomial

	  err=sum-y;// error when using orthogonal poly
	  if(fabs(err) > maxe)
		maxe=fabs(err); // calculate max abs error
	  meane2+=(err*err-meane2)/(long double)(i+1);  // incremental update for mean(error^2)
	 ((SDataPoint*)pAList->Items[i])->dYValue=sum; // put calculated value (orthogonal poly as that should be the most accurate) back
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

bool TScientificGraph::fnFFT(bool dBV_result,bool Hanning,int iGraphNumberF, void (*callback)(unsigned int cnt,unsigned int maxcnt)) // apply FFT to data. returns true if OK, false if failed.
{// real fft on data - assumes time steps are equal and in secs
 // actual FFT is done by KISS FFT
 // average value is subtracted from y values before fft & replaced afterwards to help dynamic range.
 // returns power spectrum (correctly scaled).
 // if dBV is true returns result in dBV (ie 20*log10(magnitude))
 // if Hanning is true use a Hanning (Hann) window - this is the recommended general purpose window at https://download.ni.com/evaluation/pxi/Understanding%20FFTs%20and%20Windowing.pdf
 //    which says "In general, the Hanning window is satisfactory in 95 percent of cases. It has good frequency resolution and reduced spectral leakage. "
 //
 SGraph *pAGraph = ((SGraph*) pHistory->Items[iGraphNumberF]);
 TList *pAList = pAGraph->pDataList;
 unsigned int iCount=pAList->Count;
 double x,y;
 float lastx,xmin,xmax,xinc_min,xinc_max;
 double xinc,xinc_av,y_av,freq,freq_step;
 double y2_av;// av( y^2) for rms
 unsigned int i,nfft;
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
 kiss_fftr_state = kiss_fftr_alloc(nfft,0,0,0);
 if(kiss_fftr_state==NULL)
	{free(rin);
	 free(sout);
	 rprintf("Sorry- not enough ram for FFT\n");
	 return false;
	}
  // get "stats" from input data
  lastx=xmin= ((SDataPoint*)pAList->Items[0])->dXValue; // x values are sorted
  xmax=((SDataPoint*)pAList->Items[iCount-1])->dXValue;
  xinc=(((SDataPoint*)pAList->Items[1])->dXValue)-xmin;
  xinc_min=xinc_av=xinc_max=xinc;
  y_av=((SDataPoint*)pAList->Items[0])->dYValue;
  y2_av=y_av*y_av;
  for (i=1;i<iCount;++i)
	{
	 x=((SDataPoint*)pAList->Items[i])->dXValue;
	 y=((SDataPoint*)pAList->Items[i])->dYValue;
	 xinc=x-lastx;
	 if(xinc>xinc_max) xinc_max=xinc;
	 if(xinc<xinc_min) xinc_min=xinc;
	 xinc_av+=(xinc-xinc_av)/(double)(i);
	 y_av+=(y-y_av)/(double)(i+1); // i+1 is correct as one less xinc value
	 y2_av+=(y*y-y2_av)/(double)(i+1);
	 lastx=x;
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
		 rin[i] = window*(((SDataPoint*)pAList->Items[i])->dYValue - y_av);
		}
	 else
		{
		 rin[i] = ((SDataPoint*)pAList->Items[i])->dYValue - y_av;
		}
	}
 // rest of rin array needs to be filled with zero - this has already been done by calloc()
 if(callback!=NULL)
	(*callback)(2,4); // update on progress  - crude but fft is quick
 kiss_fftr(kiss_fftr_state,rin,sout); // actually do fft
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
	float a,b;// a=max, b=min
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
	if(a!=0.0f)
		{b/=a;
		 y=a*sqrt(1.0f+b*b);
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
	 ((SDataPoint*)pAList->Items[i])->dYValue=y;  // |result|
	 ((SDataPoint*)pAList->Items[i])->dXValue=freq; // freq in Hz, starting at DC
	}
 // we put less values back - now need to free space for now unused points
 for(;i< iCount;i++)
		{
		 delete (SDataPoint*)pAList->Items[i]; // delete all the now unused x,y points
		}
 if((nfft/2)+1<iCount)
	pAList->Count=(nfft/2)+1; // shrink array to number of values put back
 free(kiss_fftr_state);
 free(rin);
 free(sout);
 kiss_fft_cleanup(); // final cleanup for fft functions
 return true; // good return
}

void TScientificGraph::compress_y(int iGraphNumberF) // compress by deleting points with equal y values except for 1st and last in a row
{// needs to be done on sorted x values
 // makes just 1 pass over the array of points, with 2 pointers i (to the item being tested) and j (j<=i) where items will be moved to (current end of compressed list)
 // at end items >=j need to be deleted (that is done at the end of this function)
 double lasty,lastx,x,y;
 SGraph *pAGraph = ((SGraph*) pHistory->Items[iGraphNumberF]);
 TList *pAList = pAGraph->pDataList;
 unsigned int iCount=pAList->Count;
 unsigned int i,j;
 bool skipy=false; // set to true while we are skipping equal y values
 if(iCount<2) return; // not enough data in graph to process
 lasty=((SDataPoint*)pAList->Items[0])->dYValue;
 lastx=((SDataPoint*)pAList->Items[0])->dXValue;
 for (i=j=1; i<iCount; i++)  // for all items in list except 1st, i is where we read from, j is where we write to
		{
		 y=((SDataPoint*)pAList->Items[i])->dYValue;
		 x=((SDataPoint*)pAList->Items[i])->dXValue;
         if(y==lasty)
                {// in block of repeats
                 skipy=true;
                 lasty=y;   // keep track of last point
                 lastx=x;
                 continue; // keep going till we find the end of the block
                }
         if(skipy)
                {// we have skipped some values, put the last one in
                 ((SDataPoint*)pAList->Items[j])->dYValue=lasty;
                 ((SDataPoint*)pAList->Items[j])->dXValue=lastx;
                 ++j;
                 skipy=false;
                }
         ((SDataPoint*)pAList->Items[j])->dYValue=y;// y value is different, copy point over
         ((SDataPoint*)pAList->Items[j])->dXValue=x;
         ++j;
         lasty=y;
         lastx=x;
        }
 // now delete values not used    [ have used array elements from 0 to j-1 ]
 rprintf("compress: %u point(s) removed from trace\n",iCount-j);
 for(i=j;i< iCount;i++)
		{
		 delete (SDataPoint*)pAList->Items[i]; // delete all the now unused x,y points
		}
 pAList->Count=j;// resize array that holds points  (frees up memory space in that as well)
}


int __fastcall xcompare(void * Item1, void * Item2)    /* define comparison order for sort below */
{double diff=((TScientificGraph::SDataPoint*)Item1)->dXValue - ((TScientificGraph::SDataPoint*)Item2)->dXValue;
 if(diff==0) return 0; // same
 else if(diff<0) return -1;
 else return 1;
}

void TScientificGraph::sortx( int iGraphNumberF) // sort ordered on x values
{
 SGraph *pAGraph = ((SGraph*) pHistory->Items[iGraphNumberF]);
 TList *pAList = pAGraph->pDataList;
 pAList->Sort(xcompare);
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
{((SGraph*)pHistory->Items[iGraphNumberF])->ucStyle=ucStyleF;}

void TScientificGraph::fnSetPointStyle(unsigned short ucStyleF,
                                                              int iGraphNumberF)
{((SGraph*)pHistory->Items[iGraphNumberF])->ucPointStyle=ucStyleF;}

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
  if(sScaleX.dMax-nextafterf(sScaleX.dMin)<=0)
        sScaleX.dMax= nextafterf(sScaleX.dMin); // limit amount of zoom
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
  if(sScaleX.dMax-nextafterf(sScaleX.dMin)<=0)
        sScaleX.dMax= nextafterf(sScaleX.dMin); // limit amount of zoom
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
  if(sScaleY.dMax-nextafterf(sScaleY.dMin)<=0)
        sScaleY.dMax= nextafterf(sScaleY.dMin); // limit amount of zoom
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
  if(sScaleY.dMax-nextafterf(sScaleY.dMin)<=0)
        sScaleY.dMax= nextafterf(sScaleY.dMin); // limit amount of zoom
  // now X
  dInterval = sScaleX.dMax
              -sScaleX.dMin;
  dDiff=(1-dInX)*dInterval/2;
  sScaleX.dMin=sScaleX.dMin+dDiff;
  sScaleX.dMax=sScaleX.dMax-dDiff;
  if(sScaleX.dMax-nextafterf(sScaleX.dMin)<=0)
        sScaleX.dMax= nextafterf(sScaleX.dMin); // limit amount of zoom

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
  if(sScaleY.dMax-nextafterf(sScaleY.dMin)<=0)
        sScaleY.dMax= nextafterf(sScaleY.dMin); // limit amount of zoom
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
#if 1
void TScientificGraph::fnClearAll()                //clear all graphs
{
  int i;
  for (i=iNumberOfGraphs-1; i>=0; i--)
  {
   fnDeleteGraph(i);// delete graphs 1 by 1, this changes  iNumberOfGraphs whihc is why for loop above is "strange"
  }
  pHistory->Clear();
  iNumberOfGraphs=0;
};
#else
// original code - did not free all ram
void TScientificGraph::fnClearAll()                //clear all graphs
{
  int i;
  for (i=0; i<iNumberOfGraphs; i++)
  {
    ((SGraph*) pHistory->Items[i])->pDataList->Clear();
    delete ((SGraph*) pHistory->Items[i])->pDataList;
  }
  pHistory->Clear();
  iNumberOfGraphs=0;
};
#endif
//------------------------------------------------------------------------------
void TScientificGraph::fnDeleteGraph(int iGraphNumberF)     //deletes graph
{
/*
        SGraph *pAGraph = ((SGraph*) pHistory->Items[iGraphNumberF]);
         TList *pAList = pAGraph->pDataList;
         unsigned int iCount=pAList->Count;
         if(iCount<2) return; // not enough data in graph to process
         m=((SDataPoint*)pAList->Items[0])->dYValue; // initial value
         lastx=((SDataPoint*)pAList->Items[0])->dXValue;
         for (unsigned int i=0; i<iCount; i++)  // for all items in list
                {
                 y=((SDataPoint*)pAList->Items[i])->dYValue;
                 x=((SDataPoint*)pAList->Items[i])->dXValue;

*/
  if ((iGraphNumberF<iNumberOfGraphs)&&(iGraphNumberF>=0))
  { SGraph *pAGraph = ((SGraph*) pHistory->Items[iGraphNumberF]);
    TList *pAList = pAGraph->pDataList;
    unsigned int iCount=pAList->Count;
    for(unsigned int i=0;i< iCount;i++)
        {
         delete (SDataPoint*)pAList->Items[i]; // delete all the x,y points
        }
    ((SGraph*) pHistory->Items[iGraphNumberF])->pDataList->Clear(); // clear list that held points
    delete ((SGraph*) pHistory->Items[iGraphNumberF])->pDataList;   // then delete it
    delete (SGraph*) pHistory->Items[iGraphNumberF]; //  delete SGraph structure (see fnAddgraph() below)
    pHistory->Delete(iGraphNumberF); // remove item from list
    pHistory->Capacity=pHistory->Count; // resize list
    iNumberOfGraphs--;
  }
}
//------------------------------------------------------------------------------

int TScientificGraph::fnAddGraph()              //insert new graph
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

  pGraph->pDataList = new TList;               //allocate data container
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
{((SGraph*)pHistory->Items[iGraphNumberF])->Caption=Caption;}
//------------------------------------------------------------------------------
bool TScientificGraph::fnInScaleY(double d)
{ return (d<sScaleY.dMax && d>sScaleY.dMin);}
//------------------------------------------------------------------------------
bool TScientificGraph::fnInScaleX(double d)
{ return (d<sScaleX.dMax && d>sScaleX.dMin);}
//------------------------------------------------------------------------------
void TScientificGraph::fnAutoScale()
{
  int i,j;
  float dXMin, dXMax,dYMin,dYMax,d;
  int max_graph=-1,min_graph=-1;  // graph for min/max
  float X_for_minY,X_for_maxY;  // location of min/max
  SGraph *aGraph;

  if (iNumberOfGraphs>0)
  {
#if 1
    // there might not be an Items[0] if the length is zero
    dXMax=dYMax=-MAXFLOAT;
    dXMin=dYMin=MAXFLOAT;
#else  // assume item 0 exists [ it might not!]
    aGraph=(SGraph*) pHistory->Items[0];
    dXMin=((SDataPoint*)aGraph->pDataList->Items[0])->dXValue;
    dXMax=((SDataPoint*)aGraph->pDataList->Items[0])->dXValue;
    dYMin=((SDataPoint*)aGraph->pDataList->Items[0])->dYValue;
    dYMax=((SDataPoint*)aGraph->pDataList->Items[0])->dYValue;
    max_graph=0;
    min_graph=0;
#endif
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
    if(aGraph->pDataList->Count==0) continue; // if no values for this graph skip further processing
#if 1 /* know x values are sorted so can move finding xmin/max outside of the loop for speed */
    if (((SDataPoint*)aGraph->pDataList->Items[0])->dXValue<dXMin)
        {dXMin=((SDataPoint*)aGraph->pDataList->Items[0])->dXValue;
        }
    if (((SDataPoint*)aGraph->pDataList->Items[(aGraph->pDataList->Count)-1])->dXValue>dXMax)
        {dXMax=((SDataPoint*)aGraph->pDataList->Items[(aGraph->pDataList->Count)-1])->dXValue;
        }
    // now loop over all elemnts to find y min/max
    for (j=0; j<aGraph->pDataList->Count; j++)
    { float y=((SDataPoint*)aGraph->pDataList->Items[j])->dYValue;
      if (y<dYMin)
        {dYMin=y;
         min_graph=i;
         X_for_minY= ((SDataPoint*)aGraph->pDataList->Items[j])->dXValue;
        }
      if (y>dYMax)
        {dYMax=y;
         max_graph=i;
         X_for_maxY= ((SDataPoint*)aGraph->pDataList->Items[j])->dXValue;
        }
    }
#else
    for (j=0; j<aGraph->pDataList->Count; j++)
    {
      if (((SDataPoint*)aGraph->pDataList->Items[j])->dXValue<dXMin)
        {dXMin=((SDataPoint*)aGraph->pDataList->Items[j])->dXValue;
        }
      if (((SDataPoint*)aGraph->pDataList->Items[j])->dXValue>dXMax)
        {dXMax=((SDataPoint*)aGraph->pDataList->Items[j])->dXValue;
        }
      if (((SDataPoint*)aGraph->pDataList->Items[j])->dYValue<dYMin)
        {dYMin=((SDataPoint*)aGraph->pDataList->Items[j])->dYValue;
         min_graph=i;
         X_for_minY= ((SDataPoint*)aGraph->pDataList->Items[j])->dXValue;
        }
      if (((SDataPoint*)aGraph->pDataList->Items[j])->dYValue>dYMax)
        {dYMax=((SDataPoint*)aGraph->pDataList->Items[j])->dYValue;
         max_graph=i;
         X_for_maxY= ((SDataPoint*)aGraph->pDataList->Items[j])->dXValue;
        }
    }
#endif
  }
  if(max_graph>=0)
        {aGraph=(SGraph*) pHistory->Items[max_graph];
         rprintf("Maximum value = %g found on trace %d (%s) at X=%g\n",dYMax,max_graph,aGraph->Caption.c_str(),X_for_maxY);
        }
  else  dXMax=dYMax=1;
  if(min_graph>=0)
        {aGraph=(SGraph*) pHistory->Items[min_graph];
         rprintf("Minimum value = %g found on trace %d (%s) at X=%g\n",dYMin,min_graph,aGraph->Caption.c_str(),X_for_minY);
        }
  else dXMin=dYMin=0;
  d=dYMax-dYMin;     //space to axis
  d=d*0.1;
  fnSetScales(dXMin,dXMax,dYMin-d
  ,dYMax+d);      //actualize scales
  // save actual min/max values
  actual_dXMin=dXMin;
  actual_dXMax=dXMax;
  actual_dYMin=dYMin-d;
  actual_dYMax=dYMax+d;

  fnOptimizeGrids();                                    //optimize grids

}
//------------------------------------------------------------------------------
long int TScientificGraph::fnGetNumberOfDataPoints(int iGraphNumberF)
{
  TList *pAList;

                                      //adress of data list
  pAList = ((SGraph*) pHistory->Items[iGraphNumberF])->pDataList;
  pAList->Pack();                     //set count to real value,no Null-pointers
  return (pAList->Count);
}
//------------------------------------------------------------------------------
double TScientificGraph::fnGetDataPointYValue(long int iChannelF,
                                                   int iGraphNumberF)
{
  TList *pAList;

  if ((pHistory->Count-1)>=iGraphNumberF)
  {
                                      //adress of data list
    pAList = ((SGraph*) pHistory->Items[iGraphNumberF])->pDataList;
    if ((pAList->Count-1)>=iChannelF)
    {
      return (((SDataPoint*)pAList->Items[iChannelF])->dYValue);
    }
    else {return (-1);}
  }
  else {return (-1);}
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
#if 0 /* not now needed as better trapping of max zoom in code elsewhere and this code limits much earlier than necessary */
  // limit amount of zooming to a sensible value
  if( (actual_dXMax-actual_dXMin)/(dXMax-dXMin)>MAXINT)
        {// zoom too big , clip it
          dXMax= dXMin+ (actual_dXMax-actual_dXMin)/MAXINT;
          dXMin=dXMax-2.0*(actual_dXMax-actual_dXMin)/MAXINT;
        }
  if( (actual_dYMax-actual_dYMin)/(dYMax-dYMin)>MAXINT)
        {// zoom too big , clip it
          dYMax= dYMin+ (actual_dYMax-actual_dYMin)/MAXINT;
          dYMin= dYMax- 2.0* (actual_dYMax-actual_dYMin)/MAXINT;
        }
#endif
#if 1    /* expand a little so if we are very close to a power of 10 a tick will appear where we would expect one */
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
  pBitmap->Canvas->LineTo(pPoint->x,pPoint->y-iTickLength*dScaling);
  fnKoord2Point(pPoint,dADoub,sScaleY.dMax);
  pBitmap->Canvas->MoveTo(pPoint->x,pPoint->y);
  pBitmap->Canvas->LineTo(pPoint->x,pPoint->y+iTickLength*dScaling);
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
  pBitmap->Canvas->LineTo(pPoint->x+iTickLength*dScaling,pPoint->y);
  fnKoord2Point(pPoint,sScaleX.dMax,dADoub);
  pBitmap->Canvas->MoveTo(pPoint->x,pPoint->y);
  pBitmap->Canvas->LineTo(pPoint->x-iTickLength*dScaling,pPoint->y);
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
  if(sScaleX.dMax-nextafterf(sScaleX.dMin)<=0)
        {sScaleX.dMax= nextafterf(sScaleX.dMin); // limit amount of zoom
         sScaleX.dMin=sScaleX.dMin-(sScaleX.dMax-sScaleX.dMin);
        }
#else
  if(sScaleX.dMax-nextafterf(sScaleX.dMin)<=0)
        sScaleX.dMax= nextafterf(sScaleX.dMin); // limit amount of zoom
#endif
  if(sScaleX.dMax < sScaleX.dMin)
        {double t=sScaleX.dMax ; // wrong order - swap so max> min
         sScaleX.dMax =sScaleX.dMin;
         sScaleX.dMin =t;
        }

  if(sScaleY.dMax> MAXFLOAT) sScaleY.dMax=MAXFLOAT;
  if(sScaleY.dMin< -MAXFLOAT) sScaleY.dMin=-MAXFLOAT;
#if 1
  if(sScaleY.dMax-nextafterf(sScaleY.dMin)<=0)
        {sScaleY.dMax= nextafterf(sScaleY.dMin); // limit amount of zoom
         sScaleY.dMin=sScaleY.dMin-(sScaleY.dMax-sScaleY.dMin);
        }
#else
  if(sScaleY.dMax-nextafterf(sScaleY.dMin)<=0)
        sScaleY.dMax= nextafterf(sScaleY.dMin); // limit amount of zoom
#endif
  if(sScaleY.dMax < sScaleY.dMin)
        {double t=sScaleY.dMax ; // wrong order - swap so max> min
         sScaleY.dMax =sScaleY.dMin;
         sScaleY.dMin =t;
        }
}

bool TScientificGraph::SaveCSV(char *filename,char *x_axis_name)
{          // save data into specified csv filename
 int i,j;
 SGraph *aGraph,*xGraph;
 FILE *fp;
 char errorstr[256]; // buffer for sprintf strings
 errorstr[sizeof(errorstr)-1]=0; // make sure null terminated
 if(iNumberOfGraphs==0)
        {snprintf(errorstr,sizeof(errorstr)-1,"Error on CSV save: Nothing to save!");    // sizeof -1 as last character set to null above
         ShowMessage(errorstr);
         return false;
        }
 // check that all graphs have the same number of points in them
 for (i=0; i<iNumberOfGraphs; i++)              //for all graphs
  {
    aGraph=(SGraph*) pHistory->Items[i];
    if(i==0)
        {j=aGraph->pDataList->Count ; // nos items in trace 0
        }
    else
        {if(j!= aGraph->pDataList->Count)
                {snprintf(errorstr,sizeof(errorstr)-1,"Error on CSV save: lines on chart have different numbers of points (%d vs %d)!",aGraph->pDataList->Count,j);    // sizeof -1 as last character set to null above
                 ShowMessage(errorstr);
                 return false;
                }
        }
   }
 // all graphs have the same number of elements - open file
 fp=fopen(filename,"wt");
 if(fp==NULL)
        {snprintf(errorstr,sizeof(errorstr)-1,"Error on CSV save: cannot create file %s",filename);    // sizeof -1 as last character set to null above
         ShowMessage(errorstr);
         return false;
        }
 // now write header line for csv file with names for each column
 fprintf(fp,"\"%s\"",x_axis_name);
 for (i=0; i<iNumberOfGraphs; i++)              //for all graphs
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
 for (j=0; j<xGraph->pDataList->Count; j++)
	{ fprintf(fp,"%.9g", ((SDataPoint*)xGraph->pDataList->Items[j])->dXValue);    // %.9g (9sf) gives max resolution for a float
	  for (i=0; i<iNumberOfGraphs; i++)              //for all graphs
		{aGraph=(SGraph*) pHistory->Items[i];
		 fprintf(fp,",%.9g",((SDataPoint*)aGraph->pDataList->Items[j])->dYValue);
        }
      fprintf(fp,"\n");
    }
 fclose(fp);
 return true; // good exit
}
