# QuikGrid v5.4 for 64-bit editions of MS Windows
Contour mapping and 3D surface modeling app

![alt text](https://github.com/ferrellsl/QuikGrid/blob/main/Screenshots/Wiz1.png?raw=true)

![alt text](https://github.com/ferrellsl/QuikGrid/blob/main/Screenshots/Wiz2.png?raw=true)

![alt text](https://github.com/ferrellsl/QuikGrid/blob/main/Screenshots/Wiz3.png?raw=true)

![alt text](https://github.com/ferrellsl/QuikGrid/blob/main/Screenshots/Wiz4.png?raw=true)

![alt text](https://github.com/ferrellsl/QuikGrid/blob/main/Screenshots/wiz5.png?raw=true)

![alt text](https://github.com/ferrellsl/QuikGrid/blob/main/Screenshots/wiz6.png?raw=true)

![alt text](https://github.com/ferrellsl/QuikGrid/blob/main/Screenshots/wiz7.png?raw=true)

![alt text](https://github.com/ferrellsl/QuikGrid/blob/main/Screenshots/wiz-5ft-gradient.png?raw=true)

![alt text](https://github.com/ferrellsl/QuikGrid/blob/main/Screenshots/DXF-contours.png?raw=true)

A 64-Bit Windows installer for QuikGrid can be found in the release section of this repo.

This repo is for QuikGrid, the Windows-based application originally developed by John Coulthard at https://www.galiander.ca/quikgrid/

This is the full Windows application including the HelpFile in CHM format, not the command-line grid generation app, although it incorporates some of the command-line grid generation code within the Windows app itself.  The gridding alogrithm, also referred to by some coders as QuikGrid, can be found in various other repos for command-line use and for use within python scripts.  The app in this repo is the complete Windows application for contouring, gridding, and 3D mapping.  It supports several output formats such as AutoCAD DXF, Surfer grid files, XYZ triplets, ER Mapper files, and VRML.  Input formats include ASCII XYZ files, NOAA LAT/LON files, Submetrix SXP files, and DCA files.

QuikGrid started out in life as a 32-bit Windows MFC application developed under the Borland C/C++ compiler.  It was my goal to import the code into a modern version of Visual Studio and generate a 64-bit binary and to create a new CHM based help file for QuikGrid as the old HLP format was no longer supported under modern versions of MS Windows.

I was shocked by how easily the Borland code imported into Visual Studio. Even the resource files generated for Borland C imported properly into Visual Studio without any issues.  I initially used Visual Studio 2013 and generated a working 32-bit EXE as a proof-of-concept project.  My next goal was to port all the 32-bit code to 64-bit code in order to process data files that exceeded 4GB in size.  The 32-bit EXE will process approximately 128 million points, but I had data sets that easily exceeded 500 million points pulled from LIDAR files.  I have successfully loaded a point cloud of 796,007,674 points (32GB ASCII XYZ file) and there's no reason why it can't load clouds composed of billions of points.

Porting to 64-bit was mostly straightforward.  I had to modify all the windowing, help and mouse functions to work properly on a 64-bit version of Windows.  Once that was accomplished I next needed to modify arrays and loops so that they could exceed 32-bit limits.  This was probably the most tedious portion of my porting efforts as far as the C/C++ code goes.  

In regard to the obsolete HLP file, I looked for a tool or tools that would simply translate or convert the old HLP file into a CHM file, but I wasn't successful in finding anything that would work and Microsoft's HTML Workshop would continually crash every time I attempted to read the HLP file.  So eventually I found an app that would de-compile the HLP file into its respective components (http://download.jgsoft.com/helpscribble/helpdc21.zip). I was then able to import these components into an application called HelpScribble (https://www.helpscribble.com/). HelpScribble took the de-compiled HLP file components and generated a working CHM file.  Everyting imported just fine into HelpScribble except for the Index/TOC Links which I had to go through individually and manually rebuild.  That was rather tedious and took the better part of 12 hours.

I have since found an application called HelpNDoc (https://www.helpndoc.com/) that's free for personal use that will import HLP files and generate CHM help files with zero effort. The CHM file created by HelpNDoc is included in the v5.4 release.

The code has now been moved over to Visual Studio 2019 and I decided it was time to put this release on github as John and I are not young men anymore.  We're both retired but I won't divulge our ages!  LOL!

KNOWN BUGS:  

1.  If the user tries to access the help menu from within QuikGrid after opening a data file or a grid file, they'll receive an error message that help isn't available.  The workaround for this is to open the help file from within QuikGrid BEFORE importing/loading a dataset or grid file.  Help will then function properly for the rest of your session even if you choose to close and re-open the Help file.  Another option is to just double-click on the QuikGrid.chm file whenever you need it.  I'm unsure why this happens and haven't been motivated enough to find the cause and fix it.

2.  On some systems, QuikGrid refuses to run unless it has been configured to run under "Windows 8 Compatibility Mode".  I'm unsure why this occurs, but after running it successfully in compatibility mode, you can go back and set it to run natively and it will continue to run properly thereafter.

BUILDING:

If you're using Visual Studio 2019 or later, double-click the Visual Studio Solution file in the QuikGrid sub-folder and build the EXE.  This app is a Microsoft Foundation Class (MFC) application so building a native Linux or MacOS version is probably out of the question without a significant re-write of the GUI code. It would be nice if some coding gurus out there removed the MFC dependencies and replaced them with something cross-platform such as QT, wxWidgets or even GTK.  I currently have no plans to do so though.  I'm happy running this on Windows.

FOR QUESTIONS or COMMENTS:

I can be reached at: sferrellblue >at< yahoo.com if you have any questions or comments.





