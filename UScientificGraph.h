//---------------------------------------------------------------------------
// This version by Peter Miller 18/7/2019 as csv file grapher
//
//  Loosely based on an original public domain version by  Frank heinrich, mail@frank-heinrich.de
//---------------------------------------------------------------------------
#ifndef UScientificGraphH
#define UScientificGraphH

#include <Graphics.hpp>
// #define CHECK_DEPTH /* if defined check depth of recursion in myqsort() */

enum LinregType  {LinLin,LinLin_GMR,LogLin,LinLog,LogLog,RecipLin,LinRecip,RecipRecip,SqrtLin,Nlog2nLin};

// class for scientific plots

class TScientificGraph
{
public:

protected:

  struct  SInterval                   //interval structure for scales
  {
    double dMin;
    double dMax;
  };

  struct SGraph                       //structure for single graph
  {
	float *x_vals;                    // x values for this graph
	float *y_vals;                    // y values for this graph
	size_t size_vals_arrays;    // actual size of above arrays (in floats)
	size_t nos_vals;            // how many items currently in x/y_vals arrays
    TColor ColDataPoint;              //color data points
    TColor ColErrorBar;               //color error bars
    TColor ColLine;                   //color graph line
    TPenStyle LineStyle;              //line style
    int iSizeDataPoint;               //size of data points
    int iWidthErrorBar;               //width of error bars
    int iWidthLine;                   //width of graph line
    unsigned char ucStyle;            //graph style
                                      //bit0 - datapoint y/n
                                      //bit1 - errorbar y/n
                                      //bit2 - line y/n
    unsigned char ucPointStyle;       //data point style
                                      //bit 0-1 shape
                                      //   00 - circle
                                      //   01 - square
                                      //   10 - triangle up
                                      //   11 - triangle down
                                      //bit 2 filled y/n
    AnsiString  Caption;              //legend
    int iTextSize;                    //text size of legend
  };

  int iBitmapWidth;                   //bitmap settings
  int iBitmapHeight;
  int iNumberOfGraphs;                //number of initialized graphs

  TList *pHistory;                    //container for graphs

  SInterval sSizeX;                   //Scales
  SInterval sSizeY;
  SInterval sScaleX;
  SInterval sScaleY;

                              //Calculates The Bitmap Coordinates of a datapoint
  bool fnKoord2Point(TPoint *pPoint, double dXValueF, double dYValueF);
  void graph_line(double xe,double ye,double xmin,double xmax,double ymin,double ymax); // draw line from xs,ys to xe,ye
  bool fnInScaleX(double dX);         //Test functions, points in scales?
  bool fnInScaleY(double dY);

  double fnMakeANiceNumber(double d);
                                      // d number for grid distance
                                      // -> return value is a nice number
  void fnPaintGridX(double dADoub);                   //paints grids and ticks
  void fnPaintGridY(double dADoub);
  void fnPaintTickX(double dADoub, double dScaling);
  void fnPaintTickY(double dADoub, double dScaling);
  void fnPaintDataPoint(TRect Rect, unsigned char ucStyle);  //paints data point

public:
  Graphics::TBitmap *pBitmap;         //Bitmap

  float fLeftBorder;                  //Borders in %/100 (plot in bitmap)
  float fRightBorder;
  float fTopBorder;
  float fBottomBorder;
                                      //Properties Plot
  TColor ColBackGround;               //color background
  TColor ColGrid;                     //color grids
  TColor ColAxis;                     //color axis
  TColor ColText;                     //color text
  int iTextSize;                      //text size for most text
  int aTextSize;                      // text size for x and y axis titles
  TPenStyle PSAxis;                   //pen style for axis
  TPenStyle PSGrid;                   //pen style for grids
  int iTickLength;                    //ticklength in pixels
  int iPenWidthAxis;                  //pen width for axis
  int iPenWidthGrid;                  //pen width for grid
  int iTextOffset;                    //text offset from axis

  bool bGrids;                        //show grids?
  bool bZeroLine;                     //show zeroline if no grids?

                                      // Position Legend in %/100 of Plot Size
  double dLegendStartX, dLegendStartY;
  double dCaptionStartX;              // position for x-axis caption
  double dCaptionStartY;              // position for y-axis caption

  int iSkipLineLevel;              //minimal sum of abs X & Y distances between two
                                      //points in a plot, between a line is
                                      //painted. Otherwise it's skipped and the
                                      //next point is checked (distance in
                                      //pixels)
  double dGridSizeX;                  //difference between to grid lines
  double dGridSizeY;
  int iGridsPerX;                     //number of grid lines per axis
  int iGridsPerY;                     // (for optimize grids function)

  double dInX;                        //Zoom Factors, Shift Factors in %/100
  double dOutX;
  double dInY;
  double dOutY;
  double dShiftFactor;

  WideString YLabel1,YLabel2,XLabel;  //Labels was  AnsiString

  //Constructor, Destructor  , resize bitmap
  TScientificGraph(int iBitmapWidthK, int iBitmapHeightK);
  ~TScientificGraph();

  void resize_bitmap(int iBitmapWidthK, int iBitmapHeightK);

  //Set Properties
  void fnSetColDataPoint(TColor Color, int iGraphNumberF = 0);
  void fnSetColErrorBar(TColor Color, int iGraphNumberF = 0);
  void fnSetColLine(TColor Color, int iGraphNumberF = 0);
  void fnSetSizeDataPoint(int iSize, int iGraphNumberF = 0);
  void fnSetErrorBarWidth(int iWidth, int iGraphNumberF = 0);
  void fnSetLineWidth(int iWidth, int iGraphNumberF = 0);
  void fnSetStyle(unsigned short ucStyleF, int iGraphNumberF = 0);
  void fnSetPointStyle(unsigned short ucStyleF, int iGraphNumberF = 0);
  void fnSetLineStyle(TPenStyle Style, int iGraphNumberF = 0);
  void fnSetCaption(AnsiString Caption, int iGraphNumberF = 0);
  void fnSetGrids(bool b) {bGrids=b;}

  //Add Items
  bool fnAddDataPoint(float dXValueF, float dYValueF,
	   int iGraphNumberF = 0); // returns true if added OK else false
  float fnAddDataPoint_nextx(int iGraphNumberF);    // returns next x value for this graph assuming its the same as the previous graph
  float fnAddDataPoint_thisy(int iGraphNumber);    // returns next y value of iGraphNumber (locn from current graph number)  used to do $T1
  bool fnChangeXoffset(double dX); // change all X values by adding dX to the most recently added graph if at least 2 graphs defined
  void fnKalman_filter(double median_ahead_t, int iGraphNumberF, void (*callback)(size_t cnt,size_t maxcnt)); // apply single variable Kalamn filter with noise variance of median_ahead_t to graph in place
  void fnCentral_moving_average_filter(double median_ahead_t, int iGraphNumberF, void (*callback)(size_t cnt,size_t maxcnt)) ; // central moving average
  void fnMedian_filt(unsigned int median_ahead, int iGraphNumberF = 0); // apply median filter to graph in place , lookahead defined in samples
  void fnMedian_filt_time1(double median_ahead_t, int iGraphNumberF, void (*callback)(size_t cnt,size_t maxcnt)); // new algorithm apply median filter to graph in place  , lookahead defined in time
  void fnMedian_filt_time(double median_ahead_t, int iGraphNumberF, void (*callback)(size_t cnt,size_t maxcnt)); // apply median filter to graph in place  , lookahead defined in time
  void fnLinear_filt_time(double tc, int iGraphNumberF, void (*callback)(size_t cnt,size_t maxcnt)); // apply linear filter to graph in place
  void fnLinreg_origin( int iGraphNumberF, void (*callback)(size_t cnt,size_t maxcnt)); // fit y=mc
  void fnLinreg_abs(bool rel, int iGraphNumberF, void (*callback)(size_t cnt,size_t maxcnt)); // fit y=mx+c with min abs error or min abs relative error
  void fnLinreg_3(int iGraphNumberF, void (*callback)(size_t cnt,size_t maxcnt)); // fit y=a*x+b*sqrt(x)+c
  void fnrat_3(int iGraphNumberF, void (*callback)(size_t cnt,size_t maxcnt)); // fits y=(a+bx)/(1+cx)
  void fnLinreg(enum LinregType type,int iGraphNumberF, void (*callback)(size_t cnt,size_t maxcnt)); // apply 1st order linear regression (y=mx+c) to graph in place
  bool fnPolyreg(unsigned int order,int iGraphNumberF, void (*callback)(size_t cnt,size_t maxcnt)); // fit polynomial of specified order regression to graph in place
  bool fnFFT(bool dBV_result,bool Hanning,int iGraphNumberF, void (*callback)(size_t cnt,size_t maxcnt)); // apply FFT to data. returns true if OK, false if failed.
  void compress_y(int iGraphNumberF); // compress by deleting points with equal y values except for 1st and last in a row
  void fix_dupx(int iGraphNumberF); // "fix" equal x values
  void deriv_filter(unsigned int diff_order,int iGraphNumberF); // smoothed derivative
  void deriv2_filter(unsigned int diff_order,int iGraphNumberF); // smoothed 2nd derivative
  void Savitzky_Golay_smoothing(unsigned int s_order,int iGraphNumberF); // Savitzky Golay smoothing
  void Spline_smoothing(double tc,int iGraphNumberF); // Smoothing spline smoothing
  void sortx( int iGraphNumberF); // sort ordered on x values
  int fnAddGraph(size_t max_points) ;  // create new line for graph with at most max_points

  //Scale Functions
  void fnResize();
  void fnShiftXPlus();
  void fnShiftXMinus();
  void fnShiftYPlus();
  void fnShiftYMinus();
  void fnZoomInX();
  void fnZoomInXFromLeft();
  void fnZoomOutX();
  void fnZoomOutXFromLeft();
  void fnZoomInY();
  void fnZoomIn();           // zoom both x and y
  void fnZoomOut();           // zoom both x and y
  void fnZoomInYFromBottom();
  void fnZoomOutY();
  void fnZoomOutYFromBottom();
  void fnOptimizeGrids();
  void fnScales2Size();
  void fnAutoScale();
  void fnSetScales(double dXMin, double dXMax, double dYMin, double dYMax);
  void fnCheckScales();

  //Clear functions
  void fnDeleteGraph(int iGraphNumberF = 0);
  void fnClearAll();

  //Get functions
  size_t fnGetNumberOfDataPoints(int iGraphNumberF = 0);
  size_t fnGetxyarr(float **x_arr,float **y_arr,int iGraphNumberF = 0); // allow access to x and y arrays, returns nos points
  double fnGetDataPointYValue(size_t iChannelF, int iGraphNumberF = 0);
  double fnGetScaleXMin() {return sScaleX.dMin;}
  double fnGetScaleXMax() {return sScaleX.dMax;}
  double fnGetScaleYMin() {return sScaleY.dMin;}
  double fnGetScaleYMax() {return sScaleY.dMax;}


  //Text functions
  void fnTextOut(double dx, double dy, AnsiString Text,TFontStyle Style,
                        int iSize=8, TColor Color=clWhite);
  void fnTextOut(double dx, double dy, AnsiString Text,
                        int iSize=8, TColor Color=clWhite);

  //calculates Bitmap position to coordinates
  bool fnPoint2Koord(int iPointX, int iPointY, double &dKoordX,
                     double &dKoordY);
  //gives bitmap border positions of plot
  int fnLeftBorder()   {return (int)(iBitmapWidth*fLeftBorder);}
  int fnRightBorder()  {return (int)(iBitmapWidth*(1-fRightBorder));}
  int fnTopBorder()    {return (int)(iBitmapHeight*fTopBorder);}
  int fnBottomBorder() {return (int)(iBitmapHeight*(1-fBottomBorder));}

  // save functions
  bool SaveCSV(char *filename,char *x_axis_name, double xmin, double xmax);
  //Repaint
  void fnPaint();
};
//---------------------------------------------------------------------------
#endif


