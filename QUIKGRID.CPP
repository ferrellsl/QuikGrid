//**************************************************************
//                   Q U I K G R I D / S U R F A C E
//   A program to visualize scattered data points as a
//   contour map or gridded surface.
//
//                 Copyright (c) 1993 - 2005 by  John Coulthard
//
//    This file is part of QuikGrid.
//
//    QuikGrid is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    QuikGrid is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with QuikGrid (File gpl.txt); if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//    or visit their website at http://www.fsf.org/licensing/licenses/gpl.txt .
//
// Jul. 8/96: Renamed from SURFACE to QUIKGRID.
// Oct. 10/96: Reset grid to undefined in Generate Grid.
//             (if sticks remove ZapZgrid() from many other modules).
// Oct. 17/96: Don't draw screen if window is an Icon.
// Feb. 24/97: Convert to Borland C++ v4.5
//           : Add hooks to output 3D DXF FACE output.
//             Call it version 3.4. (DXF upgrade).
// Mar. 4/97: Upgrade to Version 4 loaddata. Implement
//            Version 4 read DXF data points. Tear out old
//            Blair Allen code.  Clean up DXFACE code some more.
// May 18/98: Accelerators not loading - fixed.
// Jun. 27/98: Changed load grid so not restricted.
// Jul. 19/98: Started version 3.5
//             Doubled saveline.h memory to 128000 segments.
// Sep. 21/98: Converted over to Win32
// Sep. 23/98: Port over grid resoultion code & USGS DEM input.
// Sep. 25/98: Insert hooks for number of grid lines dialog box.
// Nov. 8/98:  Insert code for Statistics dialog box.
//             Prevent invocation of grid lines dialog boxes if no data points.
// Jan. 28/99: Implelent saving of the data points.
// Jan. 30/99: Implement Generating a grid on a zoomed view.
// Feb. 1/99:  Implement Generate grid on original view.
// Feb. 4/99: Implement 3d scroll bars. Axes on the 3d view.
// Feb. 8/99: Deleted ShowCorners menu item.
//            Added PleaseRegister dialog box.
//            Remove restrictions from all but writing out grid, points
//            and printing.
// Feb. 10/99: Restored ShowCorners (someone missed it!)
// Feb. 13/99: Implement Zoom and Pan options and preferences.
//             Implement panning with arrows.
// Mar. 6/99: Implement code changes to display scattered data points
//            when there is no grid generated.
// Mar. 7/99: Implementation begins on contour labels.
// Mar. 20/99:Install multitasking using multithreading.
// Mar. 29/99: Reset the new grid before starting grid re-generation.
//             (to reset grid max and min's.
// Apr. 20/99: Changes to multithread data input.
// Apr. 21/99: More changes for multi tasked data input.
// May 14/99: Change so Grid Resolution works better for Zooming.
// May 16/99: Changes for USGS DEM input. Delete reading data dialog
//             box for grid type input.
// May 29/99: Implement Grid cropping infrastructure.
// June 1-2/99: Code for functions menu and interface to it.
// Oct. 8/99: Implement command line version (handle arguments)
// Oct. 12/99: Default template code installed.
// Oct. 25/99: Hooks for handling display of wind speed & direction.
// Dec. 26/99: Code to load ER Mapper files
// Feb. 9/00: Hooks to implement command file processing.
// Feb. 25/00: Hooks to implement choosing color for features.
// Nov. 8/00: Add PaintContourReset after processing commmand file.
//            PaintContourReset after IDM_NOCOLOR (black/white option).
// Dec. 4/00: Add bold outline flag.
// Feb. 6/01: To Windows 2000
// Feb. 13/01: Add EditViewOptions dialog box.
// Mar. 9/01: Inhibit croping and zoom grid gen when grid coord's locked.
//            Enable (perhaps) croping etc. after processing command file.
// Apr. 1/01: Implement copy grid to colour grid.
// Sep. /01:  Implement support for display of volume difference
// Oct. 26/01: Implement capture metafile and exit.
// Nov. 3-8/01: Implement drawing to a memory DC.
//              & restore screen from memory.
// Nov. 9/01: Inhibit screen updating while processing commands.
//            Force window to foreground if capturebitmap & exit
//               (so window is not obscured).
// Jan 28/03: Move "Please Register Dialog Box" to later in the
//            startup sequence (error ignored in Windows caused
//            problems in WINE).
// Feb. 22/05: Remove "Register" code - more than this module involved. 
//**************************************************************

#include <windows.h>
#include <HtmlHelp.h>
#include <stddef.h>  
#include <process.h>
#include <iostream>
#include <commdlg.h>
#include <string.h>
#include <stdlib.h>
//#include <dir.h>
#include <dos.h>
#include "assert.h"
#include "surfgrid.h"
#include "scatdata.h"
#include "xygrid.h"
#include "xpand.h" 
#include "quikgrid.h"
#include "paintcon.h"
#include "loaddata.h"
#include "grdwrite.h"
#include "dxfwrite.h"
#include "erwrite.h"
#include "utilitys.h"
#include "rc.h"
#include "dxfface.h"
#include "gridres.h"
#include "grid.h"
#include "rotate.h"
#include "datawrit.h"
#include "gridview.h"
#include "prefer.h"
#include "cmdfile.h"
#include "condraw.h"
#include "vrml.h"

// Function definitions that WinMain needs.

BOOL WINAPI AboutDlgProc( HWND hDlg, UINT message, WPARAM wParam,
					 LPARAM lParam);
BOOL WINAPI NumberGridLinesDlgProc( HWND hDlg, UINT message, WPARAM wParam,
					 LPARAM lParam);
BOOL WINAPI NewGridDlgProc( HWND hDlg, UINT message, WPARAM wParam,
			       LPARAM lParam);  
BOOL WINAPI GridgenDlgProc( HWND hDlg, UINT message, WPARAM wParam,
			       LPARAM lParam);
BOOL WINAPI RotateDlgProc( HWND hDlg, UINT message, WPARAM wParam,
			       LPARAM lParam);
BOOL WINAPI ContoursDlgProc( HWND hDlg, UINT message, WPARAM wParam,
			       LPARAM lParam);
BOOL WINAPI TechnicalDlgProc( HWND hDlg, UINT message, WPARAM wParam,
			       LPARAM lParam);
BOOL WINAPI TitleDlgProc( HWND hDlg, UINT message, WPARAM wParam,
			       LPARAM lParam);
BOOL WINAPI ReadingDataDlgProc( HWND hDlg, UINT message, WPARAM wParam,
			       LPARAM lParam);
BOOL WINAPI GridLocationDlgProc( HWND hDlg, UINT message, WPARAM wParam,
			       LPARAM lParam);
BOOL WINAPI DataInputOptionsDlgProc( HWND hDlg, UINT message, WPARAM wParam,
			       LPARAM lParam);
BOOL WINAPI DataOutputOptionsDlgProc( HWND hDlg, UINT message, WPARAM wParam,
			       LPARAM lParam);
BOOL WINAPI ZratioDlgProc( HWND hDlg, UINT message, WPARAM wParam,
					 LPARAM lParam);
BOOL WINAPI ZoomRatioDlgProc( HWND hDlg, UINT message, WPARAM wParam,
					 LPARAM lParam);
BOOL WINAPI DataPointsDlgProc( HWND hDlg, UINT message, WPARAM wParam,
			       LPARAM lParam);
BOOL WINAPI StatisticsDlgProc( HWND hDlg, UINT message, WPARAM wParam,
			       LPARAM lParam);
BOOL WINAPI ColorFeaturesDlgProc( HWND hDlg, UINT message, WPARAM wParam,
			       LPARAM lParam);
BOOL WINAPI ColorMappingDlgProc( HWND hDlg, UINT message, WPARAM wParam,
			       LPARAM lParam);
BOOL WINAPI RightMouseOptionsDlgProc( HWND hDlg, UINT message, WPARAM wParam,
			       LPARAM lParam);
BOOL WINAPI GridLocnTerseDlgProc( HWND hDlg, UINT message, WPARAM wParam,
			       LPARAM lParam);
BOOL WINAPI EditViewDlgProc( HWND hDlg, UINT message, WPARAM wParam,
			       LPARAM lParam);

long WINAPI WndProc( HWND, UINT, WPARAM, LPARAM) ;

static long CommandParam;
static char FileName[5000], OutputFileName[1000];
static int ReadingData = FALSE;
static bool ProcessingCommands = false;

extern int LockNormalization;
extern ScatData OutLine;
extern COLORREF BackgroundPen;
extern HPEN hBackgroundPen;
extern int RightMouseDialogTerse;

// global data definitions.

DLGPROC  lpfnDataPointsDlgProc;

int RestoreScreenFromMemory = TRUE;
int AlwaysDrawDirectlyToScreen  = FALSE;
int GridResAuto = FALSE ;

int CaptureBitmapAndExit = FALSE;
int CaptureMetaFileAndExit = FALSE;
int QuikGridChangedImage = TRUE;
bool CreatingAMetaFile = false;
bool UseThreads = TRUE;

float Turn = 35.,
      Tilt = 35.,
      Aspect = 0.,
      Projection = 5.;

int Zooming = 0,
   // PenHighLite = 5,
    DoGenerateGrid = 0,
    LatLonData = 0,
    GridGeneratedOK = FALSE,
    PictureDrawnOK = FALSE,
	 GridLock = FALSE,
	 ColouredGrid = FALSE,
	 BoldOutline = FALSE;

char szTitle[256];

ContourOptionsType
            DrawingOptions = { 0, 0, 0, 0, 0, 0, 1, 0, 0 ,1 ,0, 0, 1, 0 },
		      OldOptions;
ScatData    ScatterData;
SurfaceGrid Zgrid(2,2);      // these are initial dimensions only.

//HINSTANCE hInst;
HACCEL hAccel;

HWND hGridgenDlgBox,
     hReadingDataDlgBox,
	  hGridLocationDlgBox = NULL;
HWND WindProcHwnd;

UINT PaintState = IDM_2DSURFACE;

HDC  hdc;

//************************************************************
//                  W i n   M a i n
//************************************************************
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
		     LPSTR lpszCmdParam, int nCmdShow)
 {
   static char szAppName[] = "QuikGrid";
   HWND     hwnd;
   MSG      msg;
   WNDCLASSEX wndclass;

	if( !hPrevInstance )
     {
       wndclass.cbSize         = sizeof( wndclass );
       wndclass.style          = CS_VREDRAW | CS_HREDRAW ;  // | CS_SAVEBITS
       wndclass.lpfnWndProc    = (WNDPROC)WndProc;
       wndclass.cbClsExtra     = 0;
       wndclass.cbWndExtra     = 0;
       wndclass.hInstance      = hInstance;
       wndclass.hIcon          = LoadIcon (hInstance, "Surface" );
       wndclass.hCursor        = LoadCursor (NULL, IDC_ARROW);
	   wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
       wndclass.lpszMenuName   = szAppName;
		 wndclass.lpszClassName  = szAppName;
       wndclass.hIconSm        = NULL;
       RegisterClassEx (&wndclass) ;
     }
 hInst = hInstance;

 hwnd = CreateWindow(szAppName,
	     "QuikGrid",             // Window Caption
	     WS_OVERLAPPEDWINDOW|WS_HSCROLL|WS_VSCROLL, // Window style   
	     CW_USEDEFAULT,       // initial x position 
	     CW_USEDEFAULT,       // initial y position 
	     CW_USEDEFAULT,       // initial x size 
	     CW_USEDEFAULT,       // initial y size 
	     NULL,
	     NULL,
	     hInstance,
	     NULL) ;

 ShowWindow( hwnd, nCmdShow );
 UpdateWindow(hwnd);

 
 hAccel = LoadAccelerators( hInstance, "QuikGrid" );

    while( GetMessage( &msg, NULL, 0, 0 ) )
    {
     if( !TranslateAccelerator( hwnd, hAccel, &msg) )
       {
         TranslateMessage( &msg);
	      DispatchMessage( &msg);
       }
    }
 return msg.wParam;
 }

//****************************************************************
//            T h r e a d  G r i d  G e n e r a t e
//****************************************************************
static void ThreadGridGenerate( PVOID pvoid )
{
 while( DoGenerateGrid )
 {
   if( !XpandPoint( Zgrid, ScatterData ) ) break;
 }
 SendMessage( hGridgenDlgBox, WM_CLOSE, NULL, NULL);
}
//****************************************************************
//            T h r e a d  L o a d  D a t a
//****************************************************************
static void ThreadLoadData( PVOID pvoid )
{
 static int i, FileOK;
 static HMENU hMenu;

 // ReadingData = TRUE;      // Probably redundant.
 hMenu = GetMenu( WindProcHwnd );
 for( i=0; i < 6; i++)
					 EnableMenuItem( hMenu, i, MF_GRAYED|MF_DISABLED|MF_BYPOSITION);
 DrawMenuBar( WindProcHwnd );
 switch( CommandParam )
 {
     case IDM_OPENMETRIC :
		  FileOK = LoadFileMetricData( WindProcHwnd, FileName );
		  break;

	  case IDM_OPENNOS :
		  FileOK = LoadFileNOSData( WindProcHwnd , FileName );
        break;

    case IDM_OPENNOAA :
		  FileOK = LoadFileNOAALatLonData( WindProcHwnd , FileName );
        break;

     case IDM_OPENLATLON :
		  FileOK = LoadFileLatLonData( WindProcHwnd , FileName );
        break ;

	  case IDM_OPENDCA :
		  FileOK = LoadFileDCAData( WindProcHwnd , FileName );
		  break ;

	  case IDM_OPENDXF:
		  FileOK = LoadFileDXFData( WindProcHwnd , FileName );
		  break ;

     case IDM_OPENSUBMETRIXSXP:
        FileOK = LoadSubmetrixSXPData( WindProcHwnd , FileName );
        break;

     case IDM_OPENOUTLINE:
        FileOK = LoadFileOutlineData( WindProcHwnd , FileName );
        SendMessage( hReadingDataDlgBox, WM_CLOSE, NULL, NULL);
		  ReadingData = FALSE;
        PictureChanged( WindProcHwnd );
        return;

	  case IDM_OPENGRID:
		 FileOK = LoadGridData(WindProcHwnd, FileName );
       SendMessage( hReadingDataDlgBox, WM_CLOSE, NULL, NULL);
       ReadingData = FALSE;
       if( !FileOK ) return;
       if( PaintState == IDM_2DSURFACE)
       {
         EnableMenuItem( hMenu, IDM_GENGRIDZOOM, MF_ENABLED);
         EnableMenuItem( hMenu, IDM_CROP2DVIEW, MF_ENABLED);
       }
       Zooming = 0;
		 GridGeneratedOK = TRUE;
       if( GridResAuto ) GridResOptimum( WindProcHwnd, hMenu );
       else GridResMenus( WindProcHwnd, hMenu );
		 PictureChanged( WindProcHwnd );
		 return ;

	  case IDM_USGSDEM:
		 FileOK = LoadUSGSDEMData(WindProcHwnd, FileName );
       SendMessage( hReadingDataDlgBox, WM_CLOSE, NULL, NULL);
       ReadingData = FALSE;
       Zooming = 0;
		 GridResOptimum( WindProcHwnd, hMenu );
		 CheckMenuItem( hMenu, IDM_GRIDRESAUTO, MF_CHECKED);
		 if( !FileOK ) return;
       if( PaintState == IDM_2DSURFACE)
       {
         EnableMenuItem( hMenu, IDM_GENGRIDZOOM, MF_ENABLED);
         EnableMenuItem( hMenu, IDM_CROP2DVIEW, MF_ENABLED);
       }
		 GridGeneratedOK = TRUE;
		 PictureChanged( WindProcHwnd );
		 return ;

     case IDM_LOADERMAPPER:
       FileOK = LoadERMapperGrid( WindProcHwnd, FileName );
       SendMessage( hReadingDataDlgBox, WM_CLOSE, NULL, NULL);
       ReadingData = FALSE;
       Zooming = 0;
		 GridResOptimum( WindProcHwnd, hMenu );
		 CheckMenuItem( hMenu, IDM_GRIDRESAUTO, MF_CHECKED);
		 if( !FileOK ) return;       
       if( PaintState == IDM_2DSURFACE)
       {
         EnableMenuItem( hMenu, IDM_GENGRIDZOOM, MF_ENABLED);
         EnableMenuItem( hMenu, IDM_CROP2DVIEW, MF_ENABLED);
       }
		 GridGeneratedOK = TRUE;
		 PictureChanged( WindProcHwnd );
       return; 

 }
 SendMessage( hReadingDataDlgBox, WM_CLOSE, NULL, NULL);
 ReadingData = FALSE;
 if( FileOK )
  {
    if( !GridLock ) GridGeneratedOK = FALSE;
	 PostMessage( WindProcHwnd, WM_COMMAND, IDM_REGENGRID, NULL );
    PictureChanged( WindProcHwnd );
  }
}

//******************************************************************
//              L o a d   D a t a
//******************************************************************
static void LoadData()
{
   if( UseThreads) _beginthread( ThreadLoadData, 0, NULL );
   else                          ThreadLoadData( NULL );

   return;
 }
//******************************************************************
//              S e t  M e m o r y  D i s p l a y
//******************************************************************
static HBITMAP     hbmScreen = 0;
static HDC         hdcDisplay = 0;
static HDC         hdcMemory = 0;

static HDC SetMemoryDisplay( HWND &hwnd )
//
// Select a memory mapped display compatible with the current
// display window. Only get new memory when window size changes.
//
{
      static RECT        rect;
      static long OldRight = 0;
      static long OldBottom = 0;

      GetClientRect( hwnd, &rect); // Get size of screen.

      if( (rect.right != OldRight) || (rect.bottom != OldBottom) )
      {
         if( hdcDisplay != 0 ) ReleaseDC( hwnd, hdcDisplay );
         hdcDisplay = GetDC( hwnd );  // Selects display excluding title, etc.
         if( hdcMemory != 0 ) DeleteDC( hdcMemory );
         hdcMemory = CreateCompatibleDC(hdcDisplay);
         DeleteObject( hbmScreen );
         hbmScreen = CreateCompatibleBitmap(hdcDisplay, rect.right, rect.bottom);
         if (hbmScreen == 0) NotifyUser( "QuikGrid failure: Unable to obtain display memory.");
         if (!SelectObject(hdcMemory, hbmScreen)) NotifyUser( "QuikGrid failure: Unable to select display memory.");
         OldRight = rect.right;
         OldBottom = rect.bottom;
      }
    return hdcMemory;
}

//*****************************************************************
//      C o p y    M e m o r y    T o   D i s p l a y
//*****************************************************************

extern RECT ClientRect, ClipRect;  // Setup up in logical units in PAINTCON

static void CopyMemoryToDisplay()
{
 // Note: BitBlt works in "logical" units.
  SetMapMode( hdcDisplay, MM_ISOTROPIC );  // Set up the display
  SetCurrentWinView( hdcDisplay );

  SetMapMode( hdcMemory, MM_ISOTROPIC );  // Set up memory map.
  SetCurrentWinView( hdcMemory );

  if (!BitBlt(hdcDisplay,
       ClientRect.left, ClientRect.bottom,      // Destination lower left corner.
       ClientRect.right-ClientRect.left, ClientRect.top-ClientRect.bottom,  // Width, Height
       hdcMemory,
       ClientRect.left, ClientRect.bottom,      // Source lower left corner.
       SRCCOPY))  NotifyUser("QuikGrid failure: Copy memory to screen failed");

  ValidateRect(WindProcHwnd, NULL);
}

//*****************************************************************
//      C o p y    D i s p l a y   T o     M e m o r y
//*****************************************************************

static void CopyDisplayToMemory()
{
 // Note: BitBlt works in "logical" units.
  SetMapMode( hdcDisplay, MM_ISOTROPIC );  // Set up the display
  SetCurrentWinView( hdcDisplay );

  SetMapMode( hdcMemory, MM_ISOTROPIC );  // Set up memory map.
  SetCurrentWinView( hdcMemory );

  if (!BitBlt(hdcMemory,
       ClientRect.left, ClientRect.bottom,      // Destination lower left corner.
       ClientRect.right-ClientRect.left, ClientRect.top-ClientRect.bottom,  // Width, Height
       hdcDisplay,
       ClientRect.left, ClientRect.bottom,      // Source lower left corner.
       SRCCOPY))  NotifyUser("QuikGrid failure: Copy display to memory failed");

}

//******************************************************************
//                       W N D P R O C
//******************************************************************

   long FAR PASCAL WndProc( HWND hwnd, UINT message, WPARAM wParam,
                            LPARAM lParam)
   {
     static PAINTSTRUCT ps;
     static RECT        rect, cliprect;
     static long OldRight = 0;
     static long OldBottom = 0;

     static HANDLE   hInstance;
     static HMENU hMenu ;

     static DLGPROC
          lpfnAboutDlgProc,
			 lpfnGridgenDlgProc,
			 lpfnNumberGridLinesDlgProc,
		    lpfnNewGridDlgProc,
		    lpfnRotateDlgProc,
		    lpfnContoursDlgProc,
		    lpfnTechnicalDlgProc,
		    lpfnTitleDlgProc,
		    lpfnReadingDataDlgProc,
		    lpfnGridLocationDlgProc,
		    lpfnDataInputOptionsDlgProc,
          lpfnDataOutputOptionsDlgProc,
			 lpfnZratioDlgProc,
			 lpfnZoomRatioDlgProc,
          lpfnStatisticsDlgProc,
          lpfnColorFeaturesDlgProc,
          lpfnColorMappingDlgProc,
          lpfnRightMouseOptionsDlgProc,
          lpfnGridLocnTerseDlgProc,
          lpfnEditViewDlgProc;
     
	  static int i,
					 ZoomFactor = 2,
                ZoomLevel = 1,
		          AutoGridGen = 1,
                MouseResolution = 20; // Throw away first 20 mouse moves.

     static POINT ZoomCentre, MouseLocation;
	  static long Oldix = -1, Oldiy = -1;

 WindProcHwnd = hwnd;

 // ****** MAIN SWITCH ****************

 switch (message)
 {
	case WM_CREATE :
	   hInstance = (( LPCREATESTRUCT) lParam) -> hInstance ;
	   lpfnAboutDlgProc = (DLGPROC) AboutDlgProc;
		lpfnGridgenDlgProc = (DLGPROC) GridgenDlgProc;
		lpfnNumberGridLinesDlgProc = (DLGPROC) NumberGridLinesDlgProc;
		lpfnNewGridDlgProc = (DLGPROC) NewGridDlgProc;
	   lpfnRotateDlgProc  = (DLGPROC) RotateDlgProc;
	   lpfnContoursDlgProc = (DLGPROC) ContoursDlgProc;
	   lpfnTechnicalDlgProc = (DLGPROC) TechnicalDlgProc;
	   lpfnTitleDlgProc = (DLGPROC) TitleDlgProc;
	   lpfnReadingDataDlgProc = (DLGPROC) ReadingDataDlgProc;
	   lpfnGridLocationDlgProc = (DLGPROC) GridLocationDlgProc;
	   lpfnDataInputOptionsDlgProc = (DLGPROC) DataInputOptionsDlgProc;
      lpfnDataOutputOptionsDlgProc = (DLGPROC) DataOutputOptionsDlgProc;
		lpfnZratioDlgProc = (DLGPROC) ZratioDlgProc;
      lpfnZoomRatioDlgProc = (DLGPROC) ZoomRatioDlgProc;
	   lpfnDataPointsDlgProc = (DLGPROC) DataPointsDlgProc,
      lpfnStatisticsDlgProc = (DLGPROC) StatisticsDlgProc;
      lpfnColorFeaturesDlgProc = (DLGPROC) ColorFeaturesDlgProc;
      lpfnColorMappingDlgProc = (DLGPROC) ColorMappingDlgProc;
      lpfnRightMouseOptionsDlgProc = (DLGPROC) RightMouseOptionsDlgProc;
      lpfnGridLocnTerseDlgProc = (DLGPROC) GridLocnTerseDlgProc;
      lpfnEditViewDlgProc = (DLGPROC) EditViewDlgProc;

	   ShowScrollBar( hwnd, SB_BOTH, FALSE);
	   SetStartupState( hwnd );
 // set menu defaults correctly (undone in 5.2?)
      hMenu = GetMenu( hwnd );
      GridResMenus( hwnd, hMenu );
      
		LoadPreferences( hMenu);
      CreatePens( );
      RotateInitialize( Turn, Tilt, Aspect );
		if( PaintState == IDM_3DSURFACE )
		 {
			ShowScrollBar( hwnd, SB_BOTH, TRUE);
			SetScrollRange( hwnd, SB_VERT, 0, 90, TRUE);
			SetScrollRange( hwnd, SB_HORZ, 0, 360, TRUE);
			SetScrollPos( hwnd, SB_VERT, 90-Tilt, TRUE );
		   SetScrollPos( hwnd, SB_HORZ, Turn+180, TRUE );
		 }
      EnableMenuItem( hMenu, IDM_GENGRIDZOOM, MF_GRAYED);
      EnableMenuItem( hMenu, IDM_CROP2DVIEW, MF_GRAYED);
		DrawMenuBar( hwnd );

		PostMessage( hwnd, WM_COMMAND, IDM_STARTUP, NULL);

		return 0;

//*********** COMMAND SWITCH ************************

	case WM_COMMAND:
     hMenu = GetMenu( hwnd );

	  switch ( LOWORD(wParam) )
	  {

	  case IDM_STARTUP:
	  {
		  if( __argc < 2 )  return 0; // there are no command line arguments.
		  strcpy( FileName, __argv[1] );
        int LengthName = strlen(FileName);
        FileName[LengthName+1] = NULL; // Place second null on file name.
        for( i = LengthName-1; i > 0; i--)
								  if( FileName[i] == '.' ) break;
       // Test for command file first.
        if( strnicmp( &FileName[i], ".CMD", 4 ) == 0 ||
            strnicmp( &FileName[i], ".QCF", 4 ) == 0 ) 
          {
             ProcessingCommands = true;
             InputCommandFile( hwnd , FileName, hMenu );
             if( PaintState == IDM_2DSURFACE)
              {
                EnableMenuItem( hMenu, IDM_GENGRIDZOOM, MF_ENABLED);
                EnableMenuItem( hMenu, IDM_CROP2DVIEW, MF_ENABLED);
              }
             Zooming = 0;
             PictureChanged( hwnd );
             PaintContourReset();
             ProcessingCommands = false;
             return 0;
          }
        // Iff not a command file we do data input. 
        ReadingData = TRUE;
		  hReadingDataDlgBox = CreateDialog
			 ( (HINSTANCE)hInstance, "ReadingData", hwnd, lpfnReadingDataDlgProc);
		  CommandParam = IDM_OPENMETRIC;
		  if( strnicmp( &FileName[i], ".NOS",4 ) == 0 ) CommandParam = IDM_OPENNOS;
        if( strnicmp( &FileName[i], ".LST",4 ) == 0 ||
            strnicmp( &FileName[i], ".NGS",4 ) == 0 ||
            strnicmp( &FileName[i], ".NOA",4 ) == 0 ) CommandParam = IDM_OPENNOAA;
		  if( strnicmp( &FileName[i], ".LL", 3 ) == 0 ) CommandParam = IDM_OPENLATLON;
        if( strnicmp( &FileName[i], ".SXP", 3 ) == 0 ) CommandParam = IDM_OPENSUBMETRIXSXP;
		  if( strnicmp( &FileName[i], ".DXF",4 ) == 0 ||
		      strnicmp( &FileName[i], ".3DF",4 ) == 0 ) CommandParam = IDM_OPENDXF;
		  if( strnicmp( &FileName[i], ".DCA",4 ) == 0 ) CommandParam = IDM_OPENDCA;
		  if( strnicmp( &FileName[i], ".DEM",4 ) == 0 )
																 { CommandParam = IDM_USGSDEM;
																	GridGeneratedOK = FALSE;
																	GridResAuto = TRUE; }
		  if( strnicmp( &FileName[i], ".QG", 3 ) == 0 )
																 { CommandParam = IDM_OPENGRID;
																	GridGeneratedOK = FALSE; }
        if( strnicmp( &FileName[i], ".ERS", 4 ) == 0 )
																 { CommandParam = IDM_LOADERMAPPER;
																	GridGeneratedOK = FALSE; }
        if( strnicmp( &FileName[i], ".OUT",4 ) == 0 ) CommandParam = IDM_OPENOUTLINE;
		  _beginthread( ThreadLoadData, 0, NULL );
		  return 0;
	  }

	  case IDM_WRITEFILEQUIT:
     // DIALOGBX sets this up after a grid has been generated *and*
     // if there are two arguments on the command line
		  if( !GridGeneratedOK)
			  NotifyUser( "QuikGrid failure: No grid available to write out");
		  strcpy( OutputFileName, __argv[2] );
		  for( i = strlen(OutputFileName)-1; i > 0; i--)
								                 if( OutputFileName[i] == '.' ) break ;
		  if( strnicmp( &OutputFileName[i], ".QG", 3 ) == 0 )
													        SaveGridData( OutputFileName ) ;
		  else if( strnicmp( &OutputFileName[i], ".ERS", 4 ) == 0 )
											  OutputERMapperFile( hwnd, OutputFileName ) ;
		  else if( strnicmp( &OutputFileName[i], ".3DF", 4 ) == 0 )
											 OutputDXF3DFaceFile( hwnd, OutputFileName ) ;
        else if( strnicmp( &OutputFileName[i], ".WRL", 4 ) == 0 )
						                           OutputVRMLFile( OutputFileName ) ;
        else if( strnicmp( &OutputFileName[i], ".GRD", 4 ) == 0 )
						                            OutputGRDfile( OutputFileName ) ;
		  else if( strnicmp( &OutputFileName[i], ".DXF", 4 ) == 0 )
						OutputDXFfile( DrawingOptions, PaintState, OutputFileName ) ;

		  else OutputXYZfile( OutputFileName );
		  PostMessage( hwnd, WM_DESTROY, NULL, NULL );
		  return 0;


//****************** FILE MENU COMMANDS **********************

	  case IDM_INPUTCOMMANDFILE:
        if( !GetReadFile( FileName,  hwnd, "Open command file" ) ) return 0;
        ProcessingCommands = true;
		  InputCommandFile( hwnd , FileName, hMenu );
        if( PaintState == IDM_2DSURFACE)
              {
                EnableMenuItem( hMenu, IDM_GENGRIDZOOM, MF_ENABLED);
                EnableMenuItem( hMenu, IDM_CROP2DVIEW, MF_ENABLED);
              }
		  Zooming = 0;
        PictureChanged( hwnd );
        PaintContourReset();
        ProcessingCommands = false;
        return 0;

	  case IDM_LOADTESTDATA:
		  Zooming = 0;
		  LoadTestData( hwnd );
		  strcpy( FileName, "Internal test data" );
		  if( !GridLock ) GridGeneratedOK = FALSE;
        CheckMenuItem( hMenu, IDM_LATLON, MF_UNCHECKED);
		  PictureChanged( hwnd );
		  PostMessage( hwnd, WM_COMMAND, IDM_REGENGRID, NULL );
		  return 0;

	  case IDM_OPENMETRIC :
		  if( !GetReadFileMultiple( FileName,  hwnd, "Open metric XYZ data file",
                                  "All files\0*.*\0\0" ) ) return 0;
        ReadingData = TRUE;
		  hReadingDataDlgBox = CreateDialog
		    ((HINSTANCE) hInstance, "ReadingData", hwnd, lpfnReadingDataDlgProc);
		  CommandParam = LOWORD( wParam );
        LoadData();
        //_beginthread( ThreadLoadData, 0, NULL );
        return 0;

	  case IDM_OPENNOS :
		 // if( !GetReadFile( FileName, hwnd, "Open NOS Lat/Lon data file" ) ) return 0;
        if( !GetReadFileMultiple( FileName,  hwnd, "Open NOS data file",
                                  "All files\0*.*\0\0" ) ) return 0;
		  ReadingData = TRUE;
		  hReadingDataDlgBox = CreateDialog
			  ((HINSTANCE)hInstance, "ReadingData", hwnd, lpfnReadingDataDlgProc);
		  CommandParam = LOWORD( wParam );
		  //_beginthread( ThreadLoadData, 0, NULL );
        LoadData();
		  return 0;

     case IDM_OPENNOAA :
        if( !GetReadFileMultiple( FileName,  hwnd, "Open NOAA Lat/Lon data file",
                                  "All files\0*.*\0\0" ) ) return 0;
		  ReadingData = TRUE;
		  hReadingDataDlgBox = CreateDialog
			  ((HINSTANCE)hInstance, "ReadingData", hwnd, lpfnReadingDataDlgProc);
		  CommandParam = LOWORD( wParam );
		  //_beginthread( ThreadLoadData, 0, NULL );
        LoadData();
		  return 0;

     case IDM_OPENLATLON :
        //if( !GetReadFile( FileName, hwnd, "Open Lat/Lon data file" ) ) return 0;
        if( !GetReadFileMultiple( FileName,  hwnd, "Open Lat/Lon data file",
                                  "All files\0*.*\0\0" ) ) return 0;
        ReadingData = TRUE;
		  hReadingDataDlgBox = CreateDialog
			  ((HINSTANCE)hInstance, "ReadingData", hwnd, lpfnReadingDataDlgProc);
		  CommandParam = LOWORD( wParam );
        //_beginthread( ThreadLoadData, 0, NULL );
        LoadData();
		  return 0;

     case IDM_OPENOUTLINE:
        if( !GetReadFileMultiple( FileName,  hwnd, "Open outline data file",
                                  "All files\0*.*\0\0" ) ) return 0;
		  ReadingData = TRUE;
		  hReadingDataDlgBox = CreateDialog
			  ((HINSTANCE)hInstance, "ReadingData", hwnd, lpfnReadingDataDlgProc);
		  CommandParam = LOWORD( wParam );
		  //_beginthread( ThreadLoadData, 0, NULL );
        LoadData();
        return 0;

     case IDM_CLEAROUTLINE:
        OutLine.Reset();
        LockNormalization = FALSE;
        PictureChanged( WindProcHwnd );
        return 0;

	  case IDM_OPENDCA :
		  if( !GetReadFile( FileName, hwnd, "Open DCA data file") ) return 0;
        ReadingData = TRUE;
		  hReadingDataDlgBox = CreateDialog
			  ((HINSTANCE)hInstance, "ReadingData", hwnd, lpfnReadingDataDlgProc);
		  CommandParam = LOWORD( wParam );
        //_beginthread( ThreadLoadData, 0, NULL );
        LoadData();
		  return 0;

	  case IDM_OPENDXF:
		  if( !GetReadFile( FileName, hwnd, "Open DXF data file") ) return 0;
        ReadingData = TRUE;
		  hReadingDataDlgBox = CreateDialog
			  ((HINSTANCE)hInstance, "ReadingData", hwnd, lpfnReadingDataDlgProc);
		  CommandParam = LOWORD( wParam );
        //_beginthread( ThreadLoadData, 0, NULL );
        LoadData();
		  return 0;

     case IDM_OPENSUBMETRIXSXP:
        if( !GetReadFileMultiple( FileName, hwnd, "Open Submetrix .sxp data file",
              "SXP files\0*.sxp\0All files\0*.*\0\0") ) return 0;
        ReadingData = TRUE;
		  hReadingDataDlgBox = CreateDialog
			  ((HINSTANCE)hInstance, "ReadingData", hwnd, lpfnReadingDataDlgProc);
        CommandParam = LOWORD( wParam );
        //_beginthread( ThreadLoadData, 0, NULL );
        LoadData();
        return 0;

     case IDM_USGSDEM:
       if( GridLock )
        { //NotifyUser( "Current grid is locked as template - request ignored.");
        NotifyUser( IDS_GRIDLOCKEDTEMPLATE );
          return 0; }
		 if( !GetReadFile( FileName, hwnd, "Open USGS DEM data file" ) ) return 0;
       ReadingData = TRUE;
		 hReadingDataDlgBox = CreateDialog
			 ((HINSTANCE)hInstance, "ReadingData", hwnd, lpfnReadingDataDlgProc);
		 GridGeneratedOK = FALSE;
		 GridResAuto = TRUE;
		 CommandParam = LOWORD( wParam );
       //_beginthread( ThreadLoadData, 0, NULL );
       LoadData();
		 return 0;

	  case IDM_OPENGRID:
		 if( !GetReadFile( FileName, hwnd, "Open QuikGrid grid data file" ) ) return 0;
       ReadingData = TRUE;
		 hReadingDataDlgBox = CreateDialog
			 ((HINSTANCE)hInstance, "ReadingData", hwnd, lpfnReadingDataDlgProc);
		 GridGeneratedOK = FALSE;
		 CommandParam = LOWORD( wParam );
       //_beginthread( ThreadLoadData, 0, NULL );
       LoadData();
		 return 0;

     case IDM_LOADERMAPPER:
      {
       if( GridLock )
        { //NotifyUser( "Current grid is locked as template - request ignored.");
          NotifyUser( IDS_GRIDLOCKEDTEMPLATE );
          return 0; }
       if( !GetReadFile( FileName, hwnd, "Open ER Mapper raster file .ers" ) ) return 0;
       ReadingData = TRUE;
       hMenu = GetMenu( WindProcHwnd );
       for( i=0; i < 6; i++)
					 EnableMenuItem( hMenu, i, MF_GRAYED|MF_DISABLED|MF_BYPOSITION);
       DrawMenuBar( WindProcHwnd );
		 hReadingDataDlgBox = CreateDialog
			 ((HINSTANCE)hInstance, "ReadingData", hwnd, lpfnReadingDataDlgProc);
		 GridGeneratedOK = FALSE;
  		 CommandParam = LOWORD( wParam );
       //_beginthread( ThreadLoadData, 0, NULL );
       LoadData();
       return 0;
      }

     case IDM_SAVE:  // Save in QuikGrid format.
        if( !GridGeneratedOK ) {NotifyUser( IDS_GRIDNOTGENERATED ); return 0; }
        if( !GetSaveFile( OutputFileName, hwnd, "Save grid data in QuikGrid format to file" ) )
                                return 0;
		  SaveGridData( OutputFileName );
		  return 0;

	  case IDM_DXF3DFACE:
		  if( !GridGeneratedOK ) {NotifyUser( IDS_GRIDNOTGENERATED ); return 0; }
		  if( !GetDXF3DFaceFileName( hwnd, OutputFileName ) ) return 0;
		  OutputDXF3DFaceFile( hwnd, OutputFileName ) ;
		  return 0;

	  case IDM_OUTPUTDXF:
		  if( !GridGeneratedOK ) {NotifyUser( IDS_GRIDNOTGENERATED ); return 0; }
		  if( !GetDXFfilename( hwnd, OutputFileName ) ) return 0;
		  OutputDXFfile( DrawingOptions, PaintState, OutputFileName ) ;
		  return 0;

     case IDM_OUTPUTWRL:
		  if( !GridGeneratedOK ) {NotifyUser( IDS_GRIDNOTGENERATED ); return 0; }
		  if( !GetVRMLFileName( hwnd, OutputFileName ) ) return 0;
		  OutputVRMLFile( OutputFileName ) ;
        return 0;

	  case IDM_SAVEXYZ:
		  if( !GridGeneratedOK ) {NotifyUser( IDS_GRIDNOTGENERATED ); return 0; }
		  if( !GetSaveFile( OutputFileName, hwnd, "Save grid in XYZ format to file" ) )
                  return 0;
		  OutputXYZfile( OutputFileName ) ;
		  return 0;

      case IDM_SAVEGRD:
		  if( !GridGeneratedOK ) {NotifyUser( IDS_GRIDNOTGENERATED ); return 0; }
		  if( !GetSaveFile( OutputFileName, hwnd, "Save grid in GRD format to file",
                  "GRD files\0*.grd\0All files\0*.*\0\0", "grd"  ) )
                  return 0;
		  OutputGRDfile( OutputFileName ) ;
		  return 0;

//     case IDM_SAVEMAPINFO:
//		  if( !GridGeneratedOK ) {NotifyUser( IDS_GRIDNOTGENERATED ); return 0; }
//		  if( !GetSaveFile( OutputFileName, hwnd, "Save grid in Mapinfo Vertical Mapper format to file",
//                  "txt files\0*.txt\0All files\0*.*\0\0", "txt"  ) )
//                 return 0;
//		  OutputMapinfoFile( OutputFileName ) ;
//		  return 0;

     case IDM_SAVEDATAPOINTS:
        if( ScatterData.Size() > 2 )OutputDataPointFile( hwnd ) ;
        else NotifyUser( " Cannot write out file. There are no data points. ");
        return 0;

	  case IDM_ERMAPPER:
		  if( !GridGeneratedOK ) {NotifyUser( IDS_GRIDNOTGENERATED ); return 0; }
		  if( !GetERMapperFileName(  hwnd, OutputFileName ) ) return 0;
		  OutputERMapperFile( hwnd, OutputFileName );
		  return 0;
        
	 case IDM_PRINT:
    {
		 if( DoGenerateGrid || !PictureDrawnOK || (Zgrid.xsize()<3) )
			{NotifyUser(" There is no display to print???"); return 0; }
       CreatingAMetaFile = false; 
		 static PRINTDLG pd;
		 static DOCINFO di = { sizeof(DOCINFO), "QuikGrid: Printing", NULL };
		 memset( &pd, 0, sizeof(PRINTDLG));
		 pd.lStructSize = sizeof(PRINTDLG);
		 pd.hwndOwner = hwnd;
		 pd.Flags = PD_RETURNDC|PD_HIDEPRINTTOFILE|PD_NOSELECTION;
		 if( !PrintDlg(&pd) ) return 0;
         StartDoc( pd.hDC, &di );
         StartPage( pd.hDC );
		   SetWaitCursor();
		   rect.right = GetDeviceCaps( pd.hDC, HORZRES );
		   rect.bottom = GetDeviceCaps( pd.hDC, VERTRES );
         rect.top = 0;
         rect.left = 0;

		   HRGN hRgn = CreateRectRgnIndirect( &rect );
         cliprect = rect;
         SelectClipRgn( pd.hDC, hRgn );

         hdc = pd.hDC;
		   switch ( PaintState )
		   {
			  case IDM_2DSURFACE:
		       Scale2dRectangle();
		       CentreDisplay( pd.hDC, rect, cliprect, Zooming);
		       Paint2dSurface( pd.hDC, DrawingOptions);
             break;

		     case IDM_3DSURFACE:
		       Scale3dCube();
				 CentreDisplay( pd.hDC, rect, cliprect, Zooming);
		       Paint3dSurface( pd.hDC, DrawingOptions );
		       break;
			 } // end switch ( PaintState...

         DeleteObject( hRgn );

         RestoreCursor();
         EndPage( pd.hDC );
         EndDoc( pd.hDC);
		   DeleteDC( pd.hDC );
		   if( pd.hDevMode != NULL ) GlobalFree(pd.hDevMode);
		   if( pd.hDevNames!= NULL ) GlobalFree(pd.hDevNames);
		 return 0;
      }

	    case IDM_EXIT:
	    	 PostMessage( hwnd, WM_DESTROY, NULL, NULL );
	   	 return 0;


//***************** VIEW COMMANDS ****************************

     case IDM_2DSURFACE :
		  if( PaintState == IDM_2DSURFACE) return 0;
		  Zooming = 0;
		  CheckMenuItem( hMenu, PaintState, MF_UNCHECKED);
		  PaintState = IDM_2DSURFACE;
		  CheckMenuItem( hMenu, PaintState, MF_CHECKED);
        EnableMenuItem( hMenu, IDM_GENGRIDZOOM, MF_ENABLED);
        EnableMenuItem( hMenu, IDM_CROP2DVIEW, MF_ENABLED);
		  DrawMenuBar( hwnd );
		  PictureChanged( hwnd); 
        ShowScrollBar( hwnd, SB_BOTH, FALSE);
        if( GridResAuto ) GridResOptimum( hwnd, hMenu );
        else GridResMenus( hwnd, hMenu );
        if( DrawingOptions.contours && DrawingOptions.labelcontours) PaintContourReset();
		  return 0;

	  case IDM_3DSURFACE :

		  if( PaintState == IDM_3DSURFACE) return 0;
		  Zooming = 0;
		  CheckMenuItem( hMenu, PaintState, MF_UNCHECKED);
		  PaintState = IDM_3DSURFACE;
		  CheckMenuItem( hMenu, PaintState, MF_CHECKED);
        EnableMenuItem( hMenu, IDM_GENGRIDZOOM, MF_GRAYED);
        EnableMenuItem( hMenu, IDM_CROP2DVIEW, MF_GRAYED);
		  DrawMenuBar( hwnd );
		  PictureChanged( hwnd); 
        ShowScrollBar( hwnd, SB_BOTH, TRUE);
        SetScrollRange( hwnd, SB_VERT, 0, 90, TRUE);
        SetScrollRange( hwnd, SB_HORZ, 0, 360, TRUE);
        SetScrollPos( hwnd, SB_VERT, 90-Tilt, TRUE );
        SetScrollPos( hwnd, SB_HORZ, Turn+180, TRUE );
        if( GridResAuto ) GridResOptimum( hwnd, hMenu );
        else GridResMenus( hwnd, hMenu );
        if( DrawingOptions.contours && DrawingOptions.labelcontours) PaintContourReset();
		  return 0;

	 case IDM_GRIDPLOT:
		 OldOptions = DrawingOptions;
		 FlipOption( hMenu, DrawingOptions.grid, IDM_GRIDPLOT);
		 if( DrawingOptions.grid )
			{ DrawingOptions.hiddengrid = ColouredGrid = FALSE;
							CheckMenuItem( hMenu, IDM_GRIDHIDE, MF_UNCHECKED);
                     CheckMenuItem( hMenu, IDM_COLOUREDGRID, MF_UNCHECKED);
		   }
		 if( ((OldOptions.grid||OldOptions.hiddengrid)!=
		      (DrawingOptions.grid||DrawingOptions.hiddengrid))&&
		       PaintState==IDM_2DSURFACE )
			         PictureChanged( hwnd ); 
		 if( PaintState==IDM_3DSURFACE) PictureChanged( hwnd ); 
		 return 0;

	 case IDM_GRIDHIDE:
		 OldOptions = DrawingOptions;
		 FlipOption( hMenu, DrawingOptions.hiddengrid, IDM_GRIDHIDE);
		 if( DrawingOptions.hiddengrid )
			{ DrawingOptions.grid = ColouredGrid = FALSE;
							CheckMenuItem( hMenu, IDM_GRIDPLOT, MF_UNCHECKED);
							CheckMenuItem( hMenu, IDM_COLOUREDGRID, MF_UNCHECKED);
		   }
		 if( ((OldOptions.grid||OldOptions.hiddengrid)!=
		      (DrawingOptions.grid||DrawingOptions.hiddengrid))&&
				 PaintState==IDM_2DSURFACE ) PictureChanged( hwnd );
		 if( PaintState==IDM_3DSURFACE) PictureChanged( hwnd );
		 return 0;

	 case IDM_COLOUREDGRID:
		 FlipOption( hMenu, ColouredGrid, IDM_COLOUREDGRID);
		 if( ColouredGrid )
		  {
			 DrawingOptions.grid = DrawingOptions.hiddengrid = FALSE;
			 CheckMenuItem( hMenu, IDM_GRIDPLOT, MF_UNCHECKED );
			 CheckMenuItem( hMenu, IDM_GRIDHIDE, MF_UNCHECKED );
		  }
		 PictureChanged( hwnd );
		 return 0;

	  case IDM_BOLDOUTLINE:
		 FlipOption( hMenu, BoldOutline, IDM_BOLDOUTLINE );
       PictureChanged( hwnd ); 
		 return 0;

     case IDM_VIEWPREFERENCE:
        SaveViewMenuOptions();
		  return 0;

     case IDM_VIEWSTATISTICS:
		 DialogBox((HINSTANCE)hInstance, "Statistics", hwnd, lpfnStatisticsDlgProc);
        return 0;

	  case IDM_DATAPLOT:
		 FlipOption( hMenu, DrawingOptions.marks, IDM_DATAPLOT);
		 PictureChanged( hwnd); 
		 return 0;

	  case IDM_NOCOLOR:
		 FlipOption( hMenu, DrawingOptions.blackwhite, IDM_NOCOLOR);
		 PictureChanged( hwnd); 
       PaintContourReset();
		 return 0;

     case IDM_COLORFEATURES:
		 DialogBox((HINSTANCE)hInstance, "ColorFeatures", hwnd, lpfnColorFeaturesDlgProc);
       PictureChanged( hwnd); 
       return 0;

     case IDM_COLOURMAPPING:
		 DialogBox((HINSTANCE)hInstance, "ColourMapping", hwnd, lpfnColorMappingDlgProc);
       PictureChanged( hwnd); 
       return 0;

	  case IDM_SHOWCORNERS:
		 FlipOption( hMenu, DrawingOptions.showcorners, IDM_SHOWCORNERS);
		 if( (PaintState == IDM_3DSURFACE) &&
		      DrawingOptions.hiddengrid &&
		      DrawingOptions.showcorners)
				NotifyUser( "Note: This option will not display in 3d hidden surface mode.");
		 if( (PaintState == IDM_3DSURFACE) &&
		      DrawingOptions.hiddengrid )  return 0;
		 PictureChanged( hwnd); 
		 return 0;

	 case IDM_CONTOUR:
		 FlipOption( hMenu, DrawingOptions.contours, IDM_CONTOUR);
       PictureChanged( hwnd);
		 return 0;

	 case IDM_CONTOURLABELS:
		 FlipOption( hMenu, DrawingOptions.labelcontours, IDM_CONTOURLABELS);
       PaintContourReset();
		 PictureChanged( hwnd);
		 return 0;

	 case IDM_3DAXES:
		FlipOption( hMenu, DrawingOptions.threedaxes, IDM_3DAXES);
		PictureChanged( hwnd);
		return 0;

	 case IDM_LATLON:
		FlipOption( hMenu, LatLonData, IDM_LATLON);
      PictureChanged( hwnd); 
		return 0;

    case IDM_SAVEVIEWOPTIONS:
       SaveViewMenuOptions();
       return 0;



//************** EDIT COMMANDS *******************************

	  case IDM_TITLE:
		  DialogBox((HINSTANCE)hInstance, "Title", hwnd, lpfnTitleDlgProc);
		  return 0;

    case IDM_EDITVIEWOPTIONS:
		DialogBox((HINSTANCE)hInstance, "EditViewOptions", hwnd, lpfnEditViewDlgProc);
        return 0;

	 case IDM_TECHNICAL:   // aka Grid Generation Options
		 DialogBox((HINSTANCE)hInstance, "Technical", hwnd, lpfnTechnicalDlgProc);
		 return 0;

	  case IDM_EDITZRATIO:
		  DialogBox((HINSTANCE)hInstance, "Zratio", hwnd, lpfnZratioDlgProc);
		  return 0;

	  case IDM_ZOOMRATIO:
		  DialogBox((HINSTANCE)hInstance, "ZoomRatio", hwnd, lpfnZoomRatioDlgProc);
		  return 0;

	  case IDM_CONTVALS:
		 if( !GridGeneratedOK ) { NotifyUser( IDS_GRIDNOTGENERATED ); return 0; }
		 DialogBox((HINSTANCE)hInstance, "Contours", hwnd, lpfnContoursDlgProc);
		 PaintContourReset();
		 return 0;

	  case IDM_ANGLE:
		  DialogBox((HINSTANCE)hInstance, "Rotate", hwnd, lpfnRotateDlgProc);
		 RotateReset();
		 return 0;

     case IDM_COLOURMAPGRIDEXTENTS:
        SetNiceColourMap( Zgrid.zmin(), Zgrid.zmax());
        PictureChanged( hwnd );
        return 0;

	  case IDM_NUMBEROFGRIDLINES :
		  if( ScatterData.Size() < 3 ) {NotifyUser( IDS_NOINPUTDATA );return 0;}
        if( GridLock )
		   { //NotifyUser ("Function not available while grid coordinates are locked");
           NotifyUser( IDS_FUNCTIONNOTAVAILABLE );
		     return 0;}
		if (DialogBox((HINSTANCE)hInstance, "NumberOfGridLines", hwnd, lpfnNumberGridLinesDlgProc)
				== -1 ) NotifyUser( "Dialog Box create failed");
		  return 0;

	  case IDM_GRIDDIM :
        if( ScatterData.Size() < 3 ) {NotifyUser( IDS_NOINPUTDATA );return 0;}
        if( GridLock )
		   { //NotifyUser ("Function not available while grid coordinates are locked");
           NotifyUser( IDS_FUNCTIONNOTAVAILABLE );
		     return 0;}
		if (DialogBox((HINSTANCE)hInstance, "NewGridLines", hwnd, lpfnNewGridDlgProc)
				== -1 ) NotifyUser( "Dialog Box create failed");
		  return 0;

     case IDM_DATAPOINTOPTIONS:
		 DialogBox((HINSTANCE)hInstance, "ViewDataPointOptions", hwnd, lpfnDataPointsDlgProc);
		     return 0;

	  case IDM_DATAINPUTOPTIONS:
		  DialogBox((HINSTANCE)hInstance, "DataInputOptions", hwnd, lpfnDataInputOptionsDlgProc);
		  return 0;

     case IDM_DATAOUTPUTOPTIONS:
		  DialogBox((HINSTANCE) hInstance, "DataOutputOptions", hwnd, lpfnDataOutputOptionsDlgProc);
		  return 0;

     case IDM_RIGHTCLICKOPTIONS:
		 DialogBox((HINSTANCE)hInstance, "RightButtonOptions", hwnd, lpfnRightMouseOptionsDlgProc);
        return 0;

//**************** WINDOW COMMANDS ****************************

	 case IDM_COPYMETA:    // Also case IDM_COPY
		 if( DoGenerateGrid || !PictureDrawnOK || (Zgrid.xsize()<3) )
					{NotifyUser(" There is no display to copy???"); return 0; }
       SetWaitCursor();
       CreatingAMetaFile = true;
		 hdc = CreateEnhMetaFile( NULL, NULL, NULL, NULL );
		 SetCurrentWin( hdc );
		 if( PaintState==IDM_2DSURFACE)
			{ Scale2dRectangle();
		     Paint2dSurface(hdc, DrawingOptions); }
		 if( PaintState==IDM_3DSURFACE)
		   { Scale3dCube();
		     Paint3dSurface(hdc, DrawingOptions); }
		 static HENHMETAFILE hmf;
		 hmf = CloseEnhMetaFile( hdc );
		 OpenClipboard( hwnd);
		 EmptyClipboard();
		 SetClipboardData( CF_ENHMETAFILE, hmf);
		 CloseClipboard();
       CreatingAMetaFile = false;
       if( CaptureMetaFileAndExit ) PostMessage( hwnd, WM_DESTROY, NULL, NULL );
       RestoreCursor();
		 return 0;

	 case IDM_COPYBITMAP:
     {
      // Here we copy the screen o the clipboard using BitBlt.
      // We seem to be able to get away with letting the mapping mode
      // default and deal with the image in native pixels.

      static HDC hdcClipboardScreen, hdcClipboardCompatible;
      static HBITMAP hbmClipboardScreen;
		if( DoGenerateGrid || !PictureDrawnOK || (Zgrid.xsize()<3) )
					{NotifyUser(" There is no display to copy???"); return 0; }
      SetWaitCursor();
      OpenClipboard( hwnd);
		EmptyClipboard();

		hdcClipboardScreen = GetDC( hwnd );

		hdcClipboardCompatible = CreateCompatibleDC(hdcClipboardScreen);
      GetClientRect( hwnd, &rect); // Get size of screen.
      hbmClipboardScreen = CreateCompatibleBitmap(hdcClipboardScreen, rect.right, rect.bottom);
		if (hbmClipboardScreen == 0) NotifyUser( "QuikGrid: Copy bitmap failure 1");

		// Select the bitmaps into the compatible DC.

	   if (!SelectObject(hdcClipboardCompatible, hbmClipboardScreen)) NotifyUser( "QuikGrid: Copy bitmap failure 2");
	   if (!BitBlt(hdcClipboardCompatible, 0,0,
					rect.right, rect.bottom, // Width, Height,
					hdcClipboardScreen, 0,0,
					SRCCOPY))  NotifyUser("QuikGrid: Copy screen to clipboard as bitmap failed.");

	   DeleteDC( hdcClipboardCompatible );
      ReleaseDC( hwnd, hdcClipboardScreen );

      SetClipboardData( CF_BITMAP, hbmClipboardScreen);
	   CloseClipboard();

      if( CaptureBitmapAndExit ) PostMessage( hwnd, WM_DESTROY, NULL, NULL );
      RestoreCursor();
		return 0;
     }

	  case IDM_UNZOOM :
		  if( Zooming == 0 ) return 0;
		  Zooming = 0;
        ZoomLevel = 1;
        if( GridResAuto ) GridResOptimum( hwnd, hMenu );
		  PictureChanged( hwnd );
		  return 0;

	  case IDM_ZOOMFREEZE :
		  if( abs( ZoomFactor ) == 1 )
		    { ZoomFactor = ZoomFactor*2;
            CheckMenuItem( hMenu, IDM_ZOOMFREEZE, MF_UNCHECKED);
          }
		  else
		    { ZoomFactor = ZoomFactor/2;
		      CheckMenuItem( hMenu, IDM_ZOOMFREEZE, MF_CHECKED);
          }
		  return 0;

	  case IDM_PREVIOUSZOOM :
        if( ZoomFactor > 0 ) ZoomLevel --;
        else ZoomLevel ++;
		  ZoomViewPrevious();
		  PictureChanged( hwnd );
		  return 0;

	  case IDM_NEGATIVEZOOM :
		  if( ZoomFactor > 0 )
		    { ZoomFactor = -2;
		      CheckMenuItem( hMenu, IDM_NEGATIVEZOOM, MF_CHECKED);
          }
		  else
		    { ZoomFactor = 2;
		      CheckMenuItem( hMenu, IDM_NEGATIVEZOOM, MF_UNCHECKED);
          }
        CheckMenuItem( hMenu, IDM_ZOOMFREEZE, MF_UNCHECKED);
		  return 0;

     case IDM_ZOOMIN:
        if( ZoomIn() )
      {
        Zooming = 1;
        PictureDrawnOK = FALSE;
	     PictureChanged( hwnd );
        Oldix = Oldiy = -1 ;
        ZoomLevel ++;
		  if( GridResAuto) GridResIncrease( hwnd, hMenu, TRUE );
      }
        return 0;

     case IDM_ZOOMOUT:
         if( ZoomOut( ) )
      {
        Zooming = 1;
		  PictureDrawnOK = FALSE;
		  PictureChanged( hwnd );
		  Oldix = Oldiy = -1 ;
        ZoomLevel --;
		  if( GridResAuto) GridResDecrease( hwnd, hMenu, TRUE );
      }
		  return 0;

	  case IDM_PANUP:
		  PanView( 0, -1 );
		  Zooming = 1;
		  PictureDrawnOK = FALSE;
		  PictureChanged( hwnd );
		  Oldix = Oldiy = -1 ;
		  return 0;

	  case IDM_PANDOWN:
		  PanView( 0, +1 );
		  Zooming = 1;
		  PictureDrawnOK = FALSE;
		  PictureChanged( hwnd );
		  Oldix = Oldiy = -1 ;
		  return 0;

	  case IDM_PANRIGHT:
		  PanView( +1, 0 );
		  Zooming = 1;
		  PictureDrawnOK = FALSE;
		  PictureChanged( hwnd );
		  Oldix = Oldiy = -1 ;
		  return 0;

     case IDM_PANLEFT:
		  PanView( -1, 0 );
		  Zooming = 1;
		  PictureDrawnOK = FALSE;
		  PictureChanged( hwnd );
		  Oldix = Oldiy = -1 ;
		  return 0;


//***************** GRID COMMANDS ******************************

	  case IDM_GRIDRESINCREASE:
		  GridResIncrease( hwnd, hMenu );
		  return 0;
	  case IDM_GRIDRESDECREASE:
		  GridResDecrease( hwnd, hMenu );
		  return 0;
	  case IDM_GRIDRESFULL:
		  GridResFull( hwnd, hMenu );
		  return 0;
	  case IDM_GRIDRESAUTO:
        if( GridResAuto )
          { GridResAuto = FALSE;
            CheckMenuItem( hMenu, IDM_GRIDRESAUTO, MF_UNCHECKED); }
        else
          {  GridResAuto = TRUE;
             CheckMenuItem( hMenu, IDM_GRIDRESAUTO, MF_CHECKED);
             GridResOptimum( hwnd, hMenu );}
		  return 0;

    case IDM_CROP2DVIEW:
        if( !GridGeneratedOK )
          { NotifyUser( " There is no grid. Crop request ignored. "); return 0; }
         if( GridLock )
		   { NotifyUser( IDS_FUNCTIONNOTAVAILABLE );
		     return 0;}
        CropGridOnView();
        Zooming = 0;
		  GridGeneratedOK = TRUE;
        if( GridResAuto ) GridResOptimum( hwnd, hMenu );
        else GridResMenus( hwnd, hMenu );
        PictureChanged( hwnd );
        return 0;

	 case IDM_GENGRIDZOOM:
		  if( !GridGeneratedOK )
          { NotifyUser( " There is no grid. Cannot defined a zoomed grid. "); return 0; }
         if( GridLock )
		   { NotifyUser( IDS_FUNCTIONNOTAVAILABLE );
		     return 0;}
		  //Zgrid.Reset();
        InitializeZgrid();
		  GenGridOnView();
        GridGeneratedOK = FALSE;
        PictureChanged( hwnd );
	     PostMessage( hwnd, WM_COMMAND, IDM_REGENGRID, NULL );
        return 0;

	  case IDM_GENGRIDORIGINAL:
        if( ScatterData.Size() < 3 ) {NotifyUser( IDS_NOINPUTDATA );return 0;}
        //Zgrid.Reset();
        InitializeZgrid();
        DefaultxyCoords();   // Calculate default x and y coordinate positions
        FillGridXY( );    // And fill the grid x y coordinates with the defaults.
        GridGeneratedOK = FALSE;
        PictureChanged( hwnd );
	     PostMessage( hwnd, WM_COMMAND, IDM_REGENGRID, NULL );
        return 0;

    case IDM_GRIDLOCK:
      if( !GridLock && !GridGeneratedOK )
       {NotifyUser( "Grid lock failed - there is no grid to lock "); return 0;}
		FlipOption( hMenu, GridLock, IDM_GRIDLOCK);
      if( GridLock ) return 0;
      KillLayerGrids();
      PictureChanged( hwnd );
		return 0;

	 case IDM_SAVETEMPLATE:
		if( !GridGeneratedOK )
			{NotifyUser( "There is no grid - cannot set template"); return 0; }
		SaveTemplate();
		if( !GridLock ) FlipOption( hMenu, GridLock, IDM_GRIDLOCK);
		return 0;

	 case IDM_CLEARTEMPLATE:  // Should put the following in a function!!!
		ClearTemplate();
      if( !GridLock ) return 0;
      FlipOption( hMenu, GridLock, IDM_GRIDLOCK);
      KillLayerGrids();
      PictureChanged( hwnd );
		return 0;


//******************* FUNCTION COMMANDS ************************

	  case IDM_SNAPXYZDATA:  // No longer on menu selection.
        if( !GridGeneratedOK ) NotifyUser( IDS_GRIDNOTGENERATED );
        SnapXYZdata();
        PictureChanged( hwnd );
        return 0;

    case IDM_COPYGRIDSPEED:
      if( !GridGeneratedOK )
			{NotifyUser( "There is no grid - cannot copy it"); return 0; }
      if( !GridLock ) FlipOption( hMenu, GridLock, IDM_GRIDLOCK);
      CopyGridToSpeed();
      PictureChanged( hwnd );
      return 0;

    case IDM_COPYGRIDDIRECTION:
      if( !GridGeneratedOK )
			{NotifyUser( "There is no grid - cannot copy it"); return 0; }
      if( !GridLock ) FlipOption( hMenu, GridLock, IDM_GRIDLOCK);
      CopyGridToDirection();
      PictureChanged( hwnd );
      return 0;

    case IDM_COPYGRIDCOLOUR:
      if( !GridGeneratedOK )
			{NotifyUser( "There is no grid - cannot copy it"); return 0; }
      if( !GridLock ) FlipOption( hMenu, GridLock, IDM_GRIDLOCK);
      CopyGridToColour();
      PictureChanged( hwnd );
      return 0;

    case IDM_COLOURLAYERDIFFERENCE:
      if( !GridGeneratedOK )
			{NotifyUser( "There is no grid - cannot compare it"); return 0; }
      ColourLayerCompare();
      PictureChanged( hwnd );
      return 0;
      
	  case IDM_GRIDSKIRTMIN:
		  if( !GridGeneratedOK ) NotifyUser( IDS_GRIDNOTGENERATED );
		  SetGridEdgeZ( Zgrid.zmin() );
        PaintContourReset();
        RotateReset();
        PictureChanged( hwnd );
        return 0;

	  case IDM_GRIDSKIRTMAX:
		  if( !GridGeneratedOK ) NotifyUser( IDS_GRIDNOTGENERATED );
        SetGridEdgeZ( Zgrid.zmax() );
        PaintContourReset();
        RotateReset();
        PictureChanged( hwnd );
        return 0;


	  case IDM_SETMINUNDEFINED:
		  if( !GridGeneratedOK ) NotifyUser( IDS_GRIDNOTGENERATED );
        SetZUndefined( Zgrid.zmin() );
        PaintContourReset();
        RotateReset();
        PictureChanged( hwnd );
        return 0;

	  case IDM_SETMAXUNDEFINED:
		  if( !GridGeneratedOK ) NotifyUser( IDS_GRIDNOTGENERATED );
		  SetZUndefined( Zgrid.zmax() );
        PaintContourReset();
        RotateReset();
        PictureChanged( hwnd );
        return 0;

	  case IDM_SETUNDEFINEDMINZ:
		  if( !GridGeneratedOK ) NotifyUser( IDS_GRIDNOTGENERATED );
        SetUndefinedTo( Zgrid.zmin());
        PaintContourReset();
        RotateReset();
        PictureChanged( hwnd );
        return 0;

     case IDM_SETUNDEFINEDMAXZ:
		  if( !GridGeneratedOK ) NotifyUser( IDS_GRIDNOTGENERATED );
		  SetUndefinedTo( Zgrid.zmax());
        PaintContourReset();
        RotateReset();
        PictureChanged( hwnd );
        return 0;

//********************* ABOUT  REGISTER HELP *******************

	  case IDM_ABOUT:
		  DialogBox((HINSTANCE)hInstance, "AboutBox", hwnd, lpfnAboutDlgProc);
		  return 0;

	  case IDM_HELP :
		  //if (HtmlHelp((HWND)GetDesktopWindow(), "QUIKGRID.CHM::html/contents.htm", HH_DISPLAY_TOPIC, 0L) == 0)
			  if (HtmlHelp((HWND)hwnd, "QUIKGRID.CHM::html/contents.htm", HH_DISPLAY_TOPIC, 0L) == 0)
		  NotifyUser( IDS_HELP);
		  return 0;

//*********** MISCELLANEOUS  & NON MENU COMMANDS ******************************

	  case IDM_REGENGRID :
	     if( !AutoGridGen ) return 0;
	  case IDM_GENGRID :
		{ Zooming = 0;
        assert ( !DoGenerateGrid );
        if( ScatterData.Size() < 3)
				{
              NotifyUser( IDS_GENGRIDNODATA );
				  SetStartupState( hwnd );
				  return 0;
            }
		  for( int i=0; i < 6; i++)
					 EnableMenuItem( hMenu, i, MF_GRAYED|MF_DISABLED|MF_BYPOSITION);
		  DrawMenuBar( hwnd );
		  hGridgenDlgBox = CreateDialog
			  ((HINSTANCE)hInstance, "GridgenBox", hwnd, lpfnGridgenDlgProc);
		  PostMessage( hGridgenDlgBox, WM_COMMAND, IDD_STARTTIMER, NULL);
		  SetWaitCursor();
		  ZapZgrid();
        //if( !GridLock ) Zgrid.Reset(); // If GridLock do we want to do this?????
        //Zgrid.Reset();
        InitializeZgrid();
		  XpandInit( Zgrid, ScatterData );

        if( GridResAuto ) GridResOptimum( hwnd, hMenu );
        else GridResMenus( hwnd, hMenu );
		  RestoreCursor();

        PictureDrawnOK = FALSE;
		  GridGeneratedOK = TRUE;  // Actually not true but soon will be.
        DoGenerateGrid=1;   // Turn on background calculations
        _beginthread( ThreadGridGenerate, 0, NULL );
		  return 0;
       }

	  case IDM_AUTOGRIDGEN:
		  FlipOption( hMenu, AutoGridGen, IDM_AUTOGRIDGEN );
		  return 0;

   case IDM_NOTIFYUSER:  // Used by Paintcon to avoid redraw loops. 
        NotifyUser( lParam ) ;
        return 0;

   case IDD_TILTUP:
       Tilt += 10;
       RotateInitialize( hwnd );
       PaintContourReset();
       RotateReset();
		 PictureChanged( hwnd );
       return 0;

    case IDD_TILTDOWN:
       Tilt -= 10;
       RotateInitialize( hwnd );
       PaintContourReset();
       RotateReset();
		 PictureChanged( hwnd );
       return 0;

    case IDD_TURNRIGHT:
       Turn += 10;
       RotateInitialize( hwnd );
       PaintContourReset();
       RotateReset();
		 PictureChanged( hwnd );
       return 0;

    case IDD_TURNLEFT:
       Turn -= 10;
       RotateInitialize( hwnd );
       PaintContourReset();
       RotateReset();
		 PictureChanged( hwnd );
       return 0;

    case IDD_TILTUPTINY:
       Tilt += 1;
       RotateInitialize( hwnd );
       PaintContourReset();
       RotateReset();
		 PictureChanged( hwnd );
       return 0;

    case IDD_TILTDOWNTINY:
       Tilt -= 1;
       RotateInitialize( hwnd );
       PaintContourReset();
       RotateReset();
		 PictureChanged( hwnd );
       return 0;

    case IDD_TURNRIGHTTINY:
       Turn += 1;
       RotateInitialize( hwnd );
       PaintContourReset();
       RotateReset();
		 PictureChanged( hwnd );
       return 0;

    case IDD_TURNLEFTTINY:
       Turn -= 1;
       RotateInitialize( hwnd );
       PaintContourReset();
       RotateReset();
		 PictureChanged( hwnd );
       return 0;

	 case IDM_GETINFO:
		 if( PaintState == IDM_2DSURFACE )
		      xyInfo2d( MouseLocation.x, MouseLocation.y, DrawingOptions);
		 else xyInfo3d( MouseLocation.x, MouseLocation.y, DrawingOptions);
		 PostMessage( hGridLocationDlgBox, WM_COMMAND, IDD_GETINFO, NULL);
		 return 0;

    case IDM_XYZINTITLE:
         // Place xyz information in the title box.
       {
          static long ix, iy ;
          static float x, y;
          if( !PictureDrawnOK || (Zgrid.xsize()<3) ) return 0;
          ContourOptionsType const GridOnly = { 0,0,0,0,0,0,1,0,0,0,0, 0};
          ix = iy = -1;
          if( InTheGrid( MouseLocation.x, MouseLocation.y ) )
             {
               xyInfo2d( MouseLocation.x, MouseLocation.y, GridOnly);
               GetxyData( x, y, MouseLocation.x, MouseLocation.y );
               GetxyInfo( ix, iy );
             }
          SetSurfaceTitle( hwnd, FileName, x, y, ix, iy);
          Oldix = ix; Oldiy = iy;
          return 0 ;
        }


	 }   // End switch WM_COMMAND

//************* END COMMAND SWITCH ***************************

    case WM_VSCROLL:
      if( DoGenerateGrid || !PictureDrawnOK || (Zgrid.xsize()<3) || ReadingData )
	         return DefWindowProc (hwnd, message, wParam, lParam);
      switch ( LOWORD (wParam) )
       {
        case SB_LINEUP:
          Tilt += 1;
          break;

        case SB_LINEDOWN:
          Tilt -= 1;
          break;

        case SB_PAGEUP:
          Tilt += 10;
          break;

        case SB_PAGEDOWN:
          Tilt -= 10;
          break;

        case SB_THUMBPOSITION:
          Tilt = 90 - HIWORD( wParam);
          break;

        default:
          return 0;
       }
       RotateInitialize( hwnd );
	    PaintContourReset();
       RotateReset();
	    PictureChanged( hwnd );

      return 0;

    case WM_HSCROLL:
		 if( DoGenerateGrid || !PictureDrawnOK || (Zgrid.xsize()<3) || ReadingData )
				return DefWindowProc (hwnd, message, wParam, lParam);
       switch ( LOWORD (wParam) )
       {
        case SB_LINEUP:
          Turn -= 1;
          break;

        case SB_LINEDOWN:
          Turn += 1;
          break;

        case SB_PAGEUP:
          Turn -= 10;
          break;

        case SB_PAGEDOWN:
          Turn += 10;
          break;

        case SB_THUMBPOSITION:
          Turn = HIWORD( wParam)-180;
          break;

        default:
          return 0;
       }

       RotateInitialize( hwnd );
	    PaintContourReset();
       RotateReset();
		 PictureChanged( hwnd );
      return 0;

    case WM_MOUSEMOVE:
      // In 2D mode place the location of the cursor in the Title.
      MouseResolution--; // don't look at every move.
      if( MouseResolution > 0 )
						return DefWindowProc (hwnd, message, wParam, lParam);
      MouseResolution = 2;
		if( DoGenerateGrid || !PictureDrawnOK || (Zgrid.xsize()<3) || ReadingData )
				return DefWindowProc (hwnd, message, wParam, lParam);
      if( PaintState != IDM_2DSURFACE )
				return DefWindowProc (hwnd, message, wParam, lParam);

      MouseLocation.x = LOWORD( lParam );
	   MouseLocation.y = HIWORD( lParam );

	   hdc = GetDC( hwnd );

	   SetMapMode( hdc, MM_ISOTROPIC );
	   SetCurrentWinView( hdc );

	   DPtoLP( hdc, (LPPOINT)&MouseLocation, 1);

	   ReleaseDC( hwnd, hdc );
      PostMessage( hwnd, WM_COMMAND, IDM_XYZINTITLE, NULL );

      return 0;

	case WM_RBUTTONDOWN:  // Print data about current location.

      if( DoGenerateGrid || (Zgrid.xsize()<3) || ReadingData )
	         return DefWindowProc (hwnd, message, wParam, lParam);
	   if( DrawingOptions.marks + DrawingOptions.numbers +
	       DrawingOptions.grid + DrawingOptions.hiddengrid + ColouredGrid
	       == 0 ) return DefWindowProc (hwnd, message, wParam, lParam);

	   MouseLocation.x = LOWORD( lParam );
	   MouseLocation.y = HIWORD( lParam );

	   hdc = GetDC( hwnd );

	   SetMapMode( hdc, MM_ISOTROPIC );
	   SetCurrentWinView( hdc );

	   DPtoLP( hdc, (LPPOINT)&MouseLocation, 1);

	   ReleaseDC( hwnd, hdc );

	   if( hGridLocationDlgBox == NULL )
	    {
          if( RightMouseDialogTerse )
           hGridLocationDlgBox = CreateDialog
		   ((HINSTANCE)hInstance, "GridCoordinateInfoTerse", hwnd, lpfnGridLocnTerseDlgProc);
          else  hGridLocationDlgBox = CreateDialog
			  ((HINSTANCE)hInstance, "GridCoordinateInfo", hwnd, lpfnGridLocationDlgProc);
       }
	   PostMessage( hGridLocationDlgBox, WM_COMMAND, IDD_GETINFOINIT, NULL);
		PostMessage( hwnd, WM_COMMAND, IDM_GETINFO, NULL );
		return 0;

	case WM_LBUTTONDOWN:  // Do a Zoommmmm......

		if( DoGenerateGrid || (Zgrid.xsize()<3) || ReadingData )
	       //return DefWindowProc (hwnd, message, wParam, lParam);
          return 0;

	   ZoomCentre.x = LOWORD( lParam );
	   ZoomCentre.y = HIWORD( lParam );
	   Zooming = 1;

		hdc = GetDC( hwnd );

	   SetMapMode( hdc, MM_ISOTROPIC );
	   SetCurrentWinView( hdc );

	   DPtoLP( hdc, (LPPOINT)&ZoomCentre, 1);

	   ReleaseDC( hwnd, hdc );

	   if( ZoomViewDouble( ZoomCentre, ZoomFactor ) )
      {
        //PictureDrawnOK = FALSE;
	     PictureChanged( hwnd );
        Oldix = Oldiy = -1 ;
		  if( GridResAuto )
		  {
			  if(ZoomFactor > 1)
						{ ZoomLevel ++;  GridResIncrease( hwnd, hMenu, TRUE ); }
			  if(ZoomFactor < -1)
                  { ZoomLevel --;  GridResDecrease( hwnd, hMenu, TRUE ); }
		  }
      }
	   return 0;


	case WM_PAINT :

   // If an icon or a null update rectangle or processing a command file just forget it.
     if( IsIconic(hwnd) || (!GetUpdateRect( hwnd, NULL, FALSE))||ProcessingCommands )
       //{ ValidateRect(hwnd, NULL); return 0; }
         { BeginPaint( hwnd, &ps);ValidateRect(hwnd, NULL); EndPaint (hwnd, &ps) ;return 0; }
   // If busy or no grid defined . . .
      if( DoGenerateGrid || ReadingData || Zgrid.xsize() < 3 )
      // Just forget it!!!
       // { ValidateRect(hwnd, NULL); QuikGridChangedImage = TRUE; return 0; }
       { BeginPaint( hwnd, &ps);ValidateRect(hwnd, NULL); EndPaint (hwnd, &ps);
         QuikGridChangedImage = TRUE; return 0; }
       /* // Blank the screen!!!
	    {
        InvalidateRect( hwnd, NULL, TRUE ); // get entire client area.
        GetClientRect( hwnd, &rect);
        hdc = BeginPaint( hwnd, &ps);
	     FillRect( hdc, &rect, hBackgroundPen );
        EndPaint (hwnd, &ps) ;
        QuikGridChangedImage = TRUE; 
        return 0;
		 }
       // */
      CreatingAMetaFile = false;
      GetClientRect( hwnd, &rect);
      GetUpdateRect( hwnd, &cliprect, FALSE );

   // Refresh from memory DC if possible.
      if( (!QuikGridChangedImage) &&
          (rect.right == OldRight) &&
          (rect.bottom == OldBottom) &&
           RestoreScreenFromMemory       )
            { CopyMemoryToDisplay(); return 0; } 

      PictureDrawnOK = FALSE;

      if( RestoreScreenFromMemory && (!AlwaysDrawDirectlyToScreen) )
                                    hdc = SetMemoryDisplay( hwnd );
      else                          hdc = BeginPaint( hwnd, &ps);
      
      if( CaptureBitmapAndExit ) SetForegroundWindow( hwnd );
	   SetWaitCursor();
	   switch (PaintState )
	   {
		 case IDM_2DSURFACE:
			SetSurfaceTitle( hwnd, FileName);
			Scale2dRectangle();
			CentreDisplay( hdc , rect, cliprect, Zooming );
			Paint2dSurface( hdc, DrawingOptions);
			break;

		 case IDM_3DSURFACE:
			SetSurfaceTitle( hwnd, FileName);
			Scale3dCube();
			CentreDisplay( hdc, rect, cliprect, Zooming );
			Paint3dSurface( hdc, DrawingOptions );
			break;

		 default:
			GetClientRect( hwnd, &rect );
			FillRect( hdc, &rect,(HBRUSH) hBackgroundPen );
			DrawText (hdc, "Internal QuikGrid error: Unsupported view type requested.", -1, &rect,
			DT_SINGLELINE|DT_CENTER|DT_VCENTER);
			break;
		} // end switch (PaintState...

      PictureDrawnOK = TRUE;

      if( RestoreScreenFromMemory && (!AlwaysDrawDirectlyToScreen) )
            CopyMemoryToDisplay();
      else  EndPaint (hwnd, &ps) ;
      if( RestoreScreenFromMemory && AlwaysDrawDirectlyToScreen)
            { SetMemoryDisplay( hwnd ); CopyDisplayToMemory(); }

      if( CaptureBitmapAndExit )PostMessage(hwnd, WM_COMMAND, IDM_COPYBITMAP, NULL );
      if( CaptureMetaFileAndExit ) PostMessage(hwnd, WM_COMMAND, IDM_COPYMETA, NULL );

      QuikGridChangedImage = FALSE;
      OldRight = rect.right;
      OldBottom = rect.bottom;
      RestoreCursor();
		return 0;

	case WM_TIMER:
	  KillTimer( hwnd, 2 );  // The TIMEOUT timer.
	  PostMessage( hwnd, WM_DESTROY, NULL, NULL ); 
	  break;

	case WM_DESTROY :
		KillTimer( hwnd, 1 );  // What timer is this?????
		KillTimer( hwnd, 2 );  // The TIMEOUT timer.
	   if( DoGenerateGrid )
		      SendMessage( hGridgenDlgBox, WM_CLOSE, NULL, NULL);
	   //HtmlHelp((HWND)GetDesktopWindow(), "QUIKGRID.CHM", HH_DISPLAY_TOPIC, 0L);
	  // HtmlHelp((HWND)hwnd, "QUIKGRID.CHM", HH_DISPLAY_TOPIC, 0L);
	   DestroyPens();
	   PostQuitMessage(0);
	   return 0;
  }  // end switch message.

return DefWindowProc (hwnd, (LRESULT)message, wParam, lParam);
}
