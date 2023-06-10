# csvgraph
Csvgraph is designed to allow quick viewing of graphs of potentially very large (GB) csv files (for comparison most spreadsheets are limited to 1,048,576 rows). Csvgraph has no built-in limits, but ultimately it is limited by your available RAM (the 32-bit version will it will use up to 4GB of RAM if its available, while the 64-bit version will use all the available RAM and virtual memory).  Even with extremely large files reading is fast and zooming is normally instantaneous.

These csv files are assumed to have column headers on their first line so a typical csv file would start:

  "Time(sec)","Col-2","Col-3","Col-4","Col-5"
  
  99950,20,0,20,20
  
  99950.1,10,1,11,12

Values are read as floating-point numbers so are restricted to numbers between +/-3.4e+38 and the smallest non-zero number is approximately 1.4e-45, with approximately 7 significant digits.

As well as simple plots of the data in csv files the data can be filtered in a number of different ways (including median filters, polynomial fits and FFT's) as well as calculated from the csv data (eg the absolute difference between the data in 2 columns of the csv file can easily be plotted (e.g. "abs($3-$4)" will plot the absolute diference between columns 3 and 4).

The file csvgraph.pdf contains the full manual and a number of examples of its use.

# Versions
1v0 - 1st github release

1v1 - 6/1/2021- fixed potential issue with DC component of FFT. Added Menu/Help/Manual. Updated manual.

1v2 – 24/1/2021 – bug fix - “inf” in csv file would be read as an extremely large number (infinity) 
     which then caused issues when csvgraph tried to scale numbers and draw the 
     graph.	Added many more options for “filtering” including exponential, power, hyperbolic and sqrt.

1v3 – 3/2/2021	- more curve fitting options added, y=mx, y=mx+c with GMR , minimum absolute 
error and minimum relative error, and y=a*x+b*sqrt(x)+c.

2v0 – 17/2/2021 – Major internal changes to reduce RAM usage and improve speed. 
No changes to function.

2v1 – 21/3/2021 – more curve fitting options added;  y=a+b*sqrt(x)+c*x+d*x^1.5, y=(a+bx)/(1+cx), 
y=(a+bx+cx^2)/(1+dx+ex^2), polynomials on sqrt(x) of user defined order and rational functions (polynomial/polynomial) to a user defined order.

2v2 - 5/4/2021 - $T1 to Tn allowed in expressions to use values from existing traces on the graph.
 Traces are numbered from 1. Invalid trace numbers (too big) return 0.
User can now set order of the linear filter. This is implemented as a nth order Butterworth filter (10*order dB/decade). Order=0 gives no filtering. Order =1 gives same filtering as previously.
"filters" for integral and derivative added.
All filters now report progress as a % (previously min. abs error and min. relative error did not report progress and they can be quite slow).
Option to skip N lines before csv header added for cases where csv header is not on the 1st row of the file
Added column numbers to X column and Y column list boxes to make it easier to select columns when names are not very descriptive (or missing).

2v3 - 23-1-2022 added y=m*x*log(x)+c curve fitting.
 Sort and median functions improved to make them faster - sort will use all available processors to improve its execution speed.

2v4 - 3-2-2022 bug fix, using variables $Tn in an expression with a set of x-axis values that were

  not in numerically increasing order (and so needed to be sorted) did not work correctly in 2v2 and 2v3 – sorry.

2v5 - 16-2-2022 Better reporting of invalid lines in csv files. When reading times, made starting at zero time optional.

2v6 – 27/2/2022 – Median1 filter improved, for <=10,000 data points its now exact and it's (much) more for accurate traces with a larger number of data points. Position of trace legends moved left so more text can be seen.

2v7 – 22/3/2022 - prints -3dB frequency for linear filter.
Display to user 1 example of every type of error in csv file.
If dates are present on some lines of the csv file, then flag lines without a date as a potential error when x column is set to time.
New (exact) median (recursive median filter) algorithm, which falls back to sampling if the execution time becomes long. Median and Median 1 renamed as recursive median filter and standard median filter.

2v8 - 23/5/2022 – added ability to read a date and time as the x value
csvsave added % complete and (significantly) sped up writing to file.
csvsave interpolates if required so x values do not need to be identical on all traces.

2v9 – 7/6/2022 – bug fix – if x-offset is not equal to 0 and multiple traces added incorrect x-offset
    was applied to the 2nd trace added onwards.
		Higher resolution internally on reading dates & times, so if “start time from zero” is
    ticked the results may be slightly more accurate.

3v0 – 	15/8/22 - Never released – 1st 64-bit version

3v1 – 17/8/22	- Never released. Source code refactored with “common files”
 (that may be used in other programs) separated out. Some files moved from C++ to C where they were pure C and C++ compiler generated lots of warnings.
 
3v2 – 14/9/2022 – 1st 64-bit release. 64-bit version will use all available memory, while 32-bit version 
is limited to 4GB of ram, apart from that functionality is identical. 
The 64-bit version can read files with more than 2^32 lines, the 32-bit version will run out of RAM before that many lines can be read.

Minor changes:
	When the Scales menu was invoked multiple times previously the scaling would change very slightly – fixed.
	Allowed range of font sizes for main title and X/Y axis titles expanded.
	FFT now makes use of multiple processor cores if they are present to deliver faster results.

3v3 - 26/9/2022 - changes to the 64 bit version to allow loading even larger files (using virtual memory as well as RAM)

3v4 - 1-10-2022 - fixed dpi handling issue when multiple monitors used on 64 bit version.

3v5 – 6/11/2022 – on a right mouse click when a line is selected then the slope of this line (dy/dx) is 
given as well as the coordinates of both end of the line. A filename can be given on the command line (on earlier versions this did not work for the 64-bit version). An expression containing a function is now allowed for the y axis (e.g. max($2,0) ).

3v6 3/6/2023 –
 Long column headers now cause a scroll bar to be automatically added to the X & Y listboxes so they can be fully seen.
Save x range on screen as CSV added to File menu.
Option (tickbox) added to add basename of filename to legends of traces on the graph, which is useful if the same column is read from multiple files.
Y axis title automatically added unless user specifies one (based on column header of 1st trace added).
Added option to load X as Value/60 (sec->min), Val/3600 (sec->hrs), val/86400 (sec->days).
Error handling for X values in a user defined date/time format improved, and trailing whitespace now allowed.

3v7 10/6/2023 –
“Show legends” tick-box added.
Changed to using Builder C++ 11.3 compiler.
Title is now centred above the graph.
Minor changes to sizes, fonts, etc in csvgraph.



# Installation
Cvsgraph is a portable program which does not need installation.

Copy the file csvgraph32.exe or csvgraph64.exe to any location on your computer (or run it from a USB-stick or similar).
Add csvgraph.pdf to the same directory to allow access to the manual from within csvgraph (menu help/manual).

A shortcut on your desktop makes it simple to execute csvgraph.

See the file LICENSE for details, but csvgraph is free for both commercial and non-commercial use.

csvgraph runs on Windows (10 32 or 64 bit). It should run on earlier versions (to Vista) or under Wine on Linux but this is untested.

# source code
From the 3v7 release, the source code was compiled with Embarcadero® C++Builder 11.3 - a suitable project file is included in the archive. It should compile with no errors or warnings.
It should be easy to move to a more recent version of C++Builder (at least for a 32/64 bit Windows target), but equally it would not be very hard to revert to earlier versions (back to Builder C++ V5) if necessary. 

Many parts of the source code could be of interest if you don't use C++Builder - for example atof.c is part of ya-sprintf see https://github.com/p-j-miller/ya-sprintf

expr-code.c contains many routines that are generally useful - including a fast general expression evaluator for expressions stored in strings.

UScientificGraph.cpp contains an efficient median filter implementation and a polynomial fit implementation both of which work well with a very large number of data points and the implementations for these are believed to be novel. 
It also contains a fast line clipping algorithm that allows "infinite" zoom in/out. 
