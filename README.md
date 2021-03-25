# csvgraph
Csvgraph is designed to allow quick viewing of graphs of potentially very large (GB) csv files (for comparison most spreadsheets are limited to 1,048,576 rows). Csvgraph has no built-in limits, but ultimately it is limited by your available RAM (it will use up to 4GB of RAM if its available). Even with extremely large files reading is fast and zooming is normally instantaneous.

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



# Installation
Cvsgraph is a portable program which does not need installation.

Copy the file csvgraph.exe to any location on your computer (or run it from a USB-stick or similar).
Add csvgraph.pdf to the same directory to allow access to the manual from within csvgraph (menu help/manual).

A shortcut on your desktop makes it simple to execute csvgraph.

See the file LICENSE for details, but csvgraph is free for both commercial and non-commercial use.

csvgraph.exe runs on Windows (10 32 or 64 bit). It should run on earlier versions (to Vista) or under Wine on Linux but this is untested.

# source code
The source code was compiled with Embarcadero® C++Builder 10.2 - a suitable project file is included in the archive. It should compile with no errors or warnings.
It should be easy to move to a more recent version of C++Builder (at least for a 32 bit Windows target), but equally it would not be very hard to revert to earlier versions (back to Builder C++ V5) if necessary. 

Many parts of the source code could be of interest if you don't use C++Builder - for example atof.c is part of ya-sprintf see https://github.com/p-j-miller/ya-sprintf

expr-code.cpp contains many routines that are generally useful (and most of this file is pure C) - including a fast general expression evaluator for expressions stored in strings.

UScientificGraph.cpp contains an efficient median filter implementation and a polynomial fit implementation both of which work well with a very large number of data points and the implementations for these are believed to be novel. 
It also contains a fast line clipping algorithm that allows "infinite" zoom in/out. 
