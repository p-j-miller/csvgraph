# csvgraph
Csvgraph is designed to allow quick viewing of graphs of potentially very large (GB) csv files (for comparison most spreadsheets are limited to 1,048,576 rows). Csvgraph has no built-in limits, but ultimately it is limited by your available RAM (the 32-bit version will it will use up to 4GB of RAM if its available, while the 64-bit version will use all the available RAM and virtual memory).  Even with extremely large files reading is fast and zooming is normally instantaneous.

These csv files are assumed to have column headers on their first line so a typical csv file would start:
~~~
  "Time(sec)","Col-2","Col-3","Col-4","Col-5"
  99950,20,0,20,20
  99950.1,10,1,11,12
~~~

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

3v8 4/7/2023
csvsave where 2nd trace had less points than 1st trace caused an error - fixed.

3v9 26/2/2024

Can now open a file that excel already has open (and error messages are better on failing to open files).
Better trapping of user pressing a "command" button while a previous command is still running.
Derivative now uses 17-point Savitzky Golay algorithm with user specified order (1->10 is actually used, can be set 1->infinity by user).
Savitzky Golay smoothing added as a filtering option (25 points, with user specified order, 1->10 is actually used, can be set 1->infinity by user).
Added 2nd derivative (d2y/d2x) to list of filters which uses a 25-point Savitzky Golay algorithm with user specified order (1->10 is actually used, can be set 1->infinity by user).
If a number is missing in a column referred to in an expression this will be set to nan.
Added constant "nan" for expressions.
If an expression evaluates to nan the line is skipped so this can be a powerful way to select points for csvgraph to display.
Added "variables" x and line to expressions. x is current x value, and line is current line number.
Updated expression handler so nan==nan and nan!=nan work as expected in expressions.

3v10 6/4/2024  Smoothing spline filter added.

4v0 3/7/2024
	Csvgraph now works with Unicode text (uft-8) so any characters can be used. That means that a csv file that is uft-8 encoded will be correctly read, and its BOM (Byte Order Mark) if present correctly processed. Column headings, the main titles and axis titles can all include utf8 characters. Utf-8 is backwards compatible with 7-bit ASCII so “conventional” csv files should be processed exactly as before. When saving a csv file, you have the option to specify that the file is created with a BOM. Filenames and the paths to files may also contain Unicode characters (utf-8).  Note that saving a CSV file with recent versions of Excel creates a utf-8 format csv file with a BOM, and a BOM is required for Excel to recognise the csv file if you try and open it.
Other changes:
 •	Screen images can be saved as BMP, jpg, png, gif, tiff and wdg format. 
 •	? operator can be used freely in expressions e.g. ($1==0?0:1) is now accepted as a valid expression (previously it would only work with the brackets removed)
 •	for regression in polynomials of sqrt(x), points with negative x are now ignored.
 •	Allowed X as well as x in expressions (useful as cut and paste of equations [ e.g. from a curve fit] may use X)

4v1 7/11/2024
Central moving average and Kalman filters added.
Standard median filter implementation improved (faster & more accurate).
If equal x values found in the input Csvgraph now allows user to leave them as is or move 
repeated values slightly so the x values always increase.
Minimum absolute/relative error line fit improved (previously occasionally it would not find 
the best straight line).
GMR line fit could in rare situations generate a line with the wrong slope – fixed.
Changed to using C++ Builder 12.1 compiler.

4v2 2/9/2025
Cepstrum analysis added. This is useful for seeing the harmonic structure of a waveform. It 
has been used for defect detection, measuring the existance and delay time of echo’s etc.
Window for FFT changed from Hanning to Nuttall Figure 12 (see 7.30 FFT in the manual).
Prompt relating to duplicate X values (when they were present in the input csv file) deleted 
– duplicates are left (and will be saved if the “save-as CSV” function is used).
Right Clicking on a point in the graph now displays 1/X as well as X (useful for Cepstrum 
analysis where 1/X is frequency).
1st version built with Target Platform "Windows 64-bit (Modern)" of the C++ Builder 12.1 
compiler (supplied as csvgraph64x.exe). This has resulted in a significantly larger executable, 
but has also provided faster execution of many functions. It has also resulted in the “RAM 
used” shown being different to that shown in Task Manager (the increments when traces 
are loaded are the same in both).
Updated icon – which better scales to different sizes.
Latest sorting code from https://github.com/p-j-miller/yasort-and-yamedian used. This is a 
little faster.
Per-Monitor (V2) DPI awareness has been fully implemented, this needs Windows 10 1703 
or above. This gives crisper looking text & graphics on screens with a scaling factor of >100%.
A side effect of this is that the font size for the axis titles can now be set over a wider range.
The right hand controls panel can now be hidden/restored with a left mouse click (previoulsy 
it had to be first undocked , then it could be hidden). 

4v3 22/4/2026

There is no new functionality in 4v3.

The latest version of ya_sprintf (2v3) from https://github.com/p-j-miller/ya-sprintf is now 
used, in particular the function ya_shortf() is used in “save as CSV” which means this 
operation is much faster (over 5* faster on large files when writing to a SSD). A side effect of 
this change is that the CSV file created will be smaller (a file that was previously 6GB was 
reduced to 5GB, while a 64KB file reduced to 50KB). Writing a csv file in this way and then 
reading it back is now guaranteed to be “round trip exact” and this has been validated for all 
possible values.

# Installation
Cvsgraph is a portable program which does not need installation.

Copy the file csvgraph.exe to any location on your computer (or run it from a USB-stick or similar).
From version 3.2 two versions are available, 32-bit and 64-bit – please select the version that 
matches the operating system version you are running. This can be found in settings – System –
About under system type. If you are unsure the 32-bit version will run on both versions of the 
operating system but will be limited to using 4GB of memory (which will be adequate for all but 
extremely large files).
In version 4v2 and above three versions are available, the “64x” version has been compiled with a 
more recent version of the compiler/linker and now uses the UCRT (Universal C Runtime) runtime. 
The UCRT should be present on all computers running Windows 10 or 11 and is available for 
previous versions back to Windows XP – see for example https://learn.microsoft.com/en￾us/cpp/windows/universal-crt-deployment?view=msvc-170 .
 Adding a trace from a 12GB csv file was 32% faster with the 64x version compared to the 64-bit version 
 (this file was too big to load with the 32-bit version) so using the 64x version is recommended.
Functionality should be identical on all 3 versions. 
Please report any issues with via github as usual.

If you wish the Help/Manual function to work then copy csvgraph.pdf to the same directory 
(location) as csvgraph.exe. If you change the name of csvgraph.exe (e.g., to csvgraph64.exe) then the 
name of the pdf file also has to change (to csvgraph64.pdf in the case).

A shortcut on your desktop makes it simple to execute csvgraph.

The first time you run csvgraph you may see a Windows warning “The Publisher could not be 
verified. Are you sure you want to run this software” (or similar), you can either run anyway (the 
executable from github - https://github.com/p-j-miller/csvgraph should be safe) or compile your 
own executable from the source files (a free version of the required Builder C++ compiler is available 
at www.embarcadero.com/products/cbuilder/starter ). Builder C++ 10.4 is required to build the 64-
bit version. As of version 3.7, csvgraph is built with Builder C++ 11.3, while version 4v1 and above is 
built with Builder C++ 12.1.

You may also get a similar message from your pdf reader the first time you use Menu/Help/Manual 
function, again you can accept this as you know why the pdf reader was invoked.

See the file LICENSE for details but csvgraph is free for both commercial and non-commercial use.
