//*******************************************************************
//       E d i t   V i e w   O p t i o n s  D i a l o g
//
//                   Copyright (c) 2001 by John Coulthard
//
//    This file is part of QuikGrid.
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
// Feb. 14/01: Initial implementation.
// Sep. 01/09: Add control for the length of the Speed Arrow. 
//*******************************************************************
#include <windows.h>
#include <HtmlHelp.h>
#include "surfgrid.h"
#include "scatdata.h"
#include "quikgrid.h"
#include "utilitys.h"
#include "rc.h"
#include "prefer.h"
#include "helpmap.h"
#include "condraw.h"
#include "gridview.h"
#include "dlgutil.h"  

extern DLGPROC lpfnDataPointsDlgProc;
//extern HINSTANCE hInst;
extern UINT PaintState;
extern ContourOptionsType DrawingOptions;
extern int ColouredGrid,
	        BoldOutline,
           GridlineColouredGrid,
           AxesOnTop,
           IncludeColourLegend,
           DisplayVolumeDifference;
extern int RestoreScreenFromMemory;
extern int AlwaysDrawDirectlyToScreen;
extern int NeverHideContours;
extern int TextSize;
extern float ArrowSize;
extern float ZmaxLabel, ZminLabel; 
static HMENU hMenu;
static HWND hwnd;
static double zAdjust; 

//**********************************************************************
//    Check Menu service routine.
//**********************************************************************
static void CheckMenu( int Value, int State )
{
   if( State ) CheckMenuItem( hMenu, Value, MF_CHECKED );
   else        CheckMenuItem( hMenu, Value, MF_UNCHECKED );
   return;
}


//**********************************************************************
//    Process radio buttons for IDOK and IDPREFERENCE
//**********************************************************************
static void CheckAndStash( HWND hDlg )
{
  static int New;
  static double dtemp;

  PaintContourReset();

  if( IsDlgButtonChecked( hDlg, IDM_3DSURFACE) )
  PostMessage( hwnd, WM_COMMAND, IDM_3DSURFACE,  NULL );
  else if ( IsDlgButtonChecked( hDlg, IDM_2DSURFACE) )
  PostMessage( hwnd, WM_COMMAND, IDM_2DSURFACE, NULL );

  DrawingOptions.contours = IsDlgButtonChecked( hDlg, IDM_CONTOUR ) ;
  CheckMenu( IDM_CONTOUR, DrawingOptions.contours );

  DrawingOptions.labelcontours = IsDlgButtonChecked( hDlg, IDM_CONTOURLABELS ) ;
  CheckMenu( IDM_CONTOURLABELS, DrawingOptions.labelcontours );

  DrawingOptions.threedaxes = IsDlgButtonChecked( hDlg, IDM_3DAXES ) ;
  CheckMenu( IDM_3DAXES, DrawingOptions.threedaxes );

  DrawingOptions.grid = IsDlgButtonChecked( hDlg, IDM_GRIDPLOT ) ;

  if( DrawingOptions.grid )
            { DrawingOptions.hiddengrid = FALSE; ColouredGrid = FALSE;
              CheckMenuItem( hMenu, IDM_GRIDPLOT, MF_CHECKED );
			     CheckMenuItem( hMenu, IDM_GRIDHIDE, MF_UNCHECKED );
              CheckMenuItem( hMenu, IDM_COLOUREDGRID, MF_UNCHECKED );}

  DrawingOptions.hiddengrid = IsDlgButtonChecked( hDlg, IDM_GRIDHIDE ) ;
  if( DrawingOptions.hiddengrid )
            { DrawingOptions.grid = FALSE; ColouredGrid = FALSE;
              CheckMenuItem( hMenu, IDM_GRIDPLOT, MF_UNCHECKED );
			     CheckMenuItem( hMenu, IDM_GRIDHIDE, MF_CHECKED );
              CheckMenuItem( hMenu, IDM_COLOUREDGRID, MF_UNCHECKED );}

  ColouredGrid = IsDlgButtonChecked( hDlg, IDM_COLOUREDGRID ) ;
  if( ColouredGrid )
            { DrawingOptions.hiddengrid = FALSE; DrawingOptions.grid = FALSE;
              CheckMenuItem( hMenu, IDM_GRIDPLOT, MF_UNCHECKED );
			     CheckMenuItem( hMenu, IDM_GRIDHIDE, MF_UNCHECKED );
              CheckMenuItem( hMenu, IDM_COLOUREDGRID, MF_CHECKED ); }

  if( !(ColouredGrid||DrawingOptions.hiddengrid||DrawingOptions.grid) )
             { CheckMenuItem( hMenu, IDM_GRIDPLOT, MF_UNCHECKED );
			      CheckMenuItem( hMenu, IDM_GRIDHIDE, MF_UNCHECKED );
               CheckMenuItem( hMenu, IDM_COLOUREDGRID, MF_UNCHECKED ); }

  DrawingOptions.marks = IsDlgButtonChecked( hDlg, IDM_DATAPLOT ) ;
  CheckMenu( IDM_DATAPLOT, DrawingOptions.marks );

  DrawingOptions.showcorners = IsDlgButtonChecked( hDlg, IDM_SHOWCORNERS ) ;
  CheckMenu( IDM_SHOWCORNERS, DrawingOptions.showcorners );

  DrawingOptions.blackwhite = IsDlgButtonChecked( hDlg, IDM_NOCOLOR ) ;
  CheckMenu( IDM_NOCOLOR, DrawingOptions.blackwhite );

  BoldOutline = IsDlgButtonChecked( hDlg, IDM_BOLDOUTLINE ) ;
  CheckMenu( IDM_BOLDOUTLINE, BoldOutline );

  GridlineColouredGrid = IsDlgButtonChecked( hDlg, IDD_GRIDLINECOLOUREDGRID ) ;
  IncludeColourLegend = IsDlgButtonChecked( hDlg, IDD_INCLUDECOLOURLEGEND ) ;
  AxesOnTop = IsDlgButtonChecked( hDlg, IDD_AXESONTOP ) ;
  NeverHideContours = IsDlgButtonChecked( hDlg, IDD_NEVERHIDECONTOURS ); 
  RestoreScreenFromMemory =  IsDlgButtonChecked( hDlg, IDD_SAVERESTOREMEMORY ) ;
  AlwaysDrawDirectlyToScreen =  IsDlgButtonChecked( hDlg, IDD_ALWAYSPAINTSCREEN ) ;

  New = IsDlgButtonChecked( hDlg, IDD_COLOURDIFFERENCE );
  if( New != DisplayVolumeDifference )
   { DisplayVolumeDifference = New;
     if( DisplayVolumeDifference ) ColourLayerVolume();
   }
  TextSize = GetDlgItemInt( hDlg, IDD_TEXTSIZE, NULL, FALSE);
  if( TextSize < 60 )  { TextSize = 60;
    NotifyUser( "The text size too small. It has been set to 60" ); }
  if( TextSize > 200 ) { TextSize = 200;
    NotifyUser( "The text size too large. It has been set to 200" ); }

  if( !GetDlgItemFloat(hDlg, IDD_ARROWSIZE, dtemp) ) ArrowSize = dtemp;
  if( ArrowSize < .2 ) { ArrowSize = .2;
    NotifyUser( "The speed arrow size is too small. It has been set to 0.2" ); }
  if( ArrowSize > 100.0 ) { ArrowSize = 100.0;
    NotifyUser( "The speed arrow size is too large. It has been set to 100.0" ); }
    
  zAdjust = ScatterData.zAdjust();
  if( !GetDlgItemFloat(hDlg, IDD_ZMAXLABEL, dtemp) ) ZmaxLabel = dtemp;
  if( !GetDlgItemFloat(hDlg, IDD_ZMINLABEL, dtemp) ) ZminLabel = dtemp;
  if( ZmaxLabel == 0.0 && ZminLabel ==0.0 ) return;
  if( ZmaxLabel <= ZminLabel )
    {
      NotifyUser( "Z axis label maximum was less than or equal to the mimimum label: Both were set to zero. ");
      ZmaxLabel = ZminLabel = 0.0;
    }
  return;
}

//**********************************************************************
//    E d i t   V i e w   D i a l o g    B o x
//**********************************************************************
BOOL FAR PASCAL EditViewDlgProc( HWND hDlg, UINT message, WPARAM wParam,
					 LPARAM lParam)
{

  switch (message)
  {
	 case WM_INITDIALOG :
       if( PaintState == IDM_3DSURFACE)
           CheckDlgButton( hDlg, IDM_3DSURFACE, TRUE ) ;
           else CheckDlgButton( hDlg, IDM_3DSURFACE, FALSE ) ;
       if( PaintState == IDM_2DSURFACE)
           CheckDlgButton( hDlg, IDM_2DSURFACE, TRUE ) ;
           else CheckDlgButton( hDlg, IDM_2DSURFACE, FALSE ) ;
       CheckDlgButton( hDlg, IDM_CONTOUR, DrawingOptions.contours );
       CheckDlgButton( hDlg, IDM_CONTOURLABELS, DrawingOptions.labelcontours );
       CheckDlgButton( hDlg, IDM_3DAXES, DrawingOptions.threedaxes );

       CheckDlgButton( hDlg, IDM_GRIDPLOT, DrawingOptions.grid );
       CheckDlgButton( hDlg, IDM_GRIDHIDE, DrawingOptions.hiddengrid );
       CheckDlgButton( hDlg, IDM_COLOUREDGRID, ColouredGrid );

       if( DrawingOptions.grid||DrawingOptions.hiddengrid||ColouredGrid )
            CheckDlgButton( hDlg, IDM_NOGRIDPLOT, FALSE );
       else CheckDlgButton( hDlg, IDM_NOGRIDPLOT, TRUE  );

       CheckDlgButton( hDlg, IDM_DATAPLOT, DrawingOptions.marks );
       CheckDlgButton( hDlg, IDM_SHOWCORNERS, DrawingOptions.showcorners );
       CheckDlgButton( hDlg, IDM_NOCOLOR, DrawingOptions.blackwhite );
       CheckDlgButton( hDlg, IDM_BOLDOUTLINE, BoldOutline );
       CheckDlgButton( hDlg, IDD_GRIDLINECOLOUREDGRID, GridlineColouredGrid );
       CheckDlgButton( hDlg, IDD_INCLUDECOLOURLEGEND, IncludeColourLegend );
       CheckDlgButton( hDlg, IDD_AXESONTOP, AxesOnTop );
       CheckDlgButton( hDlg, IDD_COLOURDIFFERENCE, DisplayVolumeDifference );
       CheckDlgButton( hDlg, IDD_SAVERESTOREMEMORY, RestoreScreenFromMemory );
       CheckDlgButton( hDlg, IDD_ALWAYSPAINTSCREEN, AlwaysDrawDirectlyToScreen );
       CheckDlgButton( hDlg, IDD_NEVERHIDECONTOURS, NeverHideContours );
       SetDlgItemInt(  hDlg, IDD_TEXTSIZE, TextSize, FALSE);
       SetDlgItemFloat(  hDlg, IDD_ARROWSIZE, ArrowSize);
       zAdjust = ScatterData.zAdjust();
       SetDlgItemFloat( hDlg, IDD_ZMAXLABEL, ZmaxLabel );
       SetDlgItemFloat( hDlg, IDD_ZMINLABEL, ZminLabel );

		 return TRUE;

	 case WM_COMMAND :
       hwnd = GetParent( hDlg );
       hMenu = GetMenu(hwnd);
		 switch (wParam)
	    {
        case IDHELP:
	       if( HtmlHelp( hDlg, "quikgrid.chm::1347xo_.htm", HELP_CONTEXT, MapViewOptions ) == 0 )
			   //if (HtmlHelp(hDlg, "quikgrid.chm::html/1347xo_.htm", HELP_CONTEXT, 0L) == 0)
		          NotifyUser( IDS_HELP);
	       return TRUE;

		  case IDOK :
          CheckAndStash( hDlg );
          PictureChanged( hwnd );
			 EndDialog( hDlg, 0);
			 return 0;

		  case IDCANCEL :
			  EndDialog( hDlg, 0);
			  return TRUE;

        case IDPREFERENCE:
           CheckAndStash( hDlg );
           PostMessage( hwnd, WM_COMMAND, IDM_SAVEVIEWOPTIONS, NULL );
           return TRUE;

        case IDM_DATAPOINTOPTIONS:
           HWND hwnd = GetParent( hDlg );
		   DialogBox((HINSTANCE)hInst, "ViewDataPointOptions", hwnd, lpfnDataPointsDlgProc);
		     return 0;

        } // end switch ( wParam )
		 break;
     } // end switch(message)
  return FALSE;
 }