//*******************************************************************
//       D i a l o g      B o x     P r o c e d u r e s
//
//                   Copyright (c) 1993 - 2001 by John Coulthard
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
// April 12/98: Aboutbox - no longer sets version type. The
//              "Big Data" version will be distributed only.
// April 13/98: Add UsePolyline item.
// April 22/98: Add FlipXY option to input/output options.
// May   16/98: Add help button to Input/Output dialog box.
// Sept. 21/98: Change About to say Windows 95 version.
// Sept. 22/98: Remove the "collapse" function. (old virtual grid code).
// Jan.  15/99: Implement "Use 0ld style input" in Read data options.
// Feb. 1/99:   Add display point number to view data point options.
// Feb. 3/99:  GetDlgItemInt strange behavious returning negative int
//             to be converted to float. Use temporary int in Rotation
//             dialog box.
// Feb. 13/99: Implement Zoom and Pan options and preferences.
// Feb. 15/99: Implement Grid Generation options preferences.
// Feb. 22/99: Move Automatic grid size stuff to Input/Output.
// Feb. 24/99: Fix bug in set undefz option (Technical parameters).
// Feb. 25/99: Change Register dialog box to sleep for 1 second on
//             entry of a incorrect code.
// Mar. 23/99: Change Contour Lines dialog to do nicer increments.
// Mar. 29/99: GridGen dialog box no longer sets contour line values.
//             (So user set lines are more "sticky" across grid regens.
// Apr. 20/99: Support Ignore Comments.
//             Add support to Read Data box for cancelling and tracking.
// Jun 4/99: Add save preferences to Zratio dialog box.
// Dec. 31/99: Add save preferences to 3d rotation dialog box.
// Mar. 16/00: GridInfo Dialog stuff moved to dlgright.cpp.
// Nov. 8/00: Identify customized contour lines in Contour dialog box.
// Feb 13/00: Don't refresh screen if marks are not being displayed
//            (In Edit data point options dialog box)
//*******************************************************************
#include <windows.h>
#include <HtmlHelp.h>
#include <iostream>
#include <strstream>
#include <iomanip>
#include <math.h>
#include <string.h>
#include <dos.h>
#include "surfgrid.h"
#include "scatdata.h"
#include "xygrid.h"
#include "xpand.h"
#include "quikgrid.h"
#include "paintcon.h"
#include "loaddata.h"
#include "utilitys.h"
#include "rc.h"
#include "helpmap.h"
#include "assert.h"
#include "dlgutil.h"
#include "rotate.h"
#include "prefer.h"
#include "condraw.h"
#include "gridview.h"
#include "grid.h"
using namespace std;
extern SurfaceGrid Zgrid;
extern HWND hGridLocationDlgBox;
extern int GridGeneratedOK;
extern UINT PaintState;
extern ContourOptionsType DrawingOptions;
extern int UsePolyline;
extern int FlipXY;
extern int ReverseSignOfZ;
//extern int LabelNthLine;
extern int LabelNDigits;
extern int LabelDistance;
extern int NewInputStyle;
extern int IgnoreComments;
extern float ZoomRatio;
extern float PanRatio;
extern char SeparatorCharacter;
extern long PointsKept;
extern int CancelLoadData;
extern int IgnoreErrorLines;
extern int OutputUndefinedValues;
extern double UndefinedValue;
extern int UseScatteredDataLayers;
extern int DXFFaceScaleZ;
extern int HighDXFOutputPrecision;
extern int CustomLines;   // non zero if custom contour lines in use.
extern int ColourContours; // for continuously coloured contours.
extern int NicexyCoordinates;
extern int DataPointsOnGridOnly;
extern int SubmetrixSelectZ;
extern double VolumeDifference;
extern int SmallMark;
extern float DXFTextHeightRatio;
extern int NeverLabelDXFContours;
extern int UseThreads;
extern int InputFieldPosition[] ;
extern int NOAAInputFieldPosition[] ;
extern int VRMLtriangles;
extern int VRML2format;

static char szBuffer[256];

//**********************************************************************
//                A B O U T   D I A L O G    B O X
//**********************************************************************
    BOOL FAR PASCAL AboutDlgProc( HWND hDlg, UINT message, WPARAM wParam,
			       LPARAM lParam)
 {
  switch (message)
     {
      case WM_INITDIALOG :
		  LoadString((HINSTANCE)hInst, IDS_ABOUTVERSION, szBuffer, 255);
      SetDlgItemText( hDlg, IDD_DISCLAIMER, szBuffer);

	 return TRUE;
      case WM_COMMAND :
	 switch (wParam)
	    {
        case IDD_HELP:
				if( HtmlHelp( hDlg, "quikgrid.chm::disclaimer.htm", HELP_CONTEXT, Disclaimer ) == 0 )
					//if (HtmlHelp(hDlg, "quikgrid.chm::html/disclaimer.htm", HELP_CONTEXT, 0L) == 0)
		     NotifyUser( IDS_HELP);
	     return TRUE;

	     case IDOK :
	     case IDCANCEL :
	       EndDialog( hDlg, 0);
	       return TRUE;
	    }
	 break;
     }
  return FALSE;
 }

//**********************************************************************
//                G R I D G E N   D I A L O G    B O X  (Modeless)
//**********************************************************************
	 BOOL FAR PASCAL GridgenDlgProc( HWND hDlg, UINT message, WPARAM wParam,
			       LPARAM lParam)
 {
  switch (message)
     {
      case WM_INITDIALOG :
         SetDlgItemInt( hDlg, IDD_SAMPLENUMBER, XpandSample(), FALSE);
			SetDlgItemInt( hDlg, IDD_PERCENTDONE, 0, FALSE);
			SetDlgItemInt( hDlg, IDD_GRIDSIZEX, Zgrid.xsize(), FALSE);
			SetDlgItemInt( hDlg, IDD_GRIDSIZEY, Zgrid.ysize(), FALSE);
			SetDlgItemInt( hDlg, IDD_SCANBANDWIDTH, 0, FALSE);
			SetDlgItemLong( hDlg, IDD_NUMDATAPOINTS, ScatterData.Size() );
			SetDlgItemInt( hDlg, IDD_PERCENTSHELL, 0, FALSE);
		return TRUE;

	 case WM_COMMAND :
	 switch (wParam)
	    {
	     case IDD_HELP:
	       if( HtmlHelp( hDlg, "quikgrid.chm::gridgeneration.htm", HELP_CONTEXT, GridGenDialogBox ) == 0 )
			   //if (HtmlHelp(hDlg, "quikgrid.chm::html/gridgeneration.htm", HELP_CONTEXT, 0L) == 0)
		    NotifyUser( IDS_HELP);
	       return TRUE;

	     case IDCANCEL :   goto CloseCode;

	     case IDD_STARTTIMER :
	      if( !SetTimer( hDlg, 1, 1000, NULL)) NotifyUser( IDS_TOOMANYTIMERS );
	      SetDlgItemText( hDlg, IDD_GRIDGENSTATUS, "Generating...");
         return TRUE;
	    }  // end switch  (wParam)
	 break;

      case WM_TIMER:
		  if( DoGenerateGrid )
			 { SetDlgItemInt( hDlg, IDD_PERCENTDONE, XpandPercentDone(), FALSE);
				SetDlgItemInt( hDlg, IDD_SCANBANDWIDTH, XpandBandWidth(), FALSE);
				SetDlgItemInt( hDlg, IDD_PERCENTSHELL, XpandPercentShell(), FALSE);
			  }
        return TRUE;

      case WM_CLOSE: goto CloseCode;
       {

       }  // end case WM_CLOSE
     }
  return FALSE;

   CloseCode:
        HWND hwnd = GetParent( hDlg );
        PaintContourReset();
        RotateReset();
        PictureChanged( hwnd );
        HMENU hMenu = GetMenu(hwnd);
        for( int i = 0; i < 6; i++)
          EnableMenuItem( hMenu, i, MF_ENABLED|MF_BYPOSITION);
        if( PaintState == IDM_2DSURFACE)
        {
          EnableMenuItem( hMenu, IDM_GENGRIDZOOM, MF_ENABLED);
          EnableMenuItem( hMenu, IDM_CROP2DVIEW, MF_ENABLED);
        }
	     DrawMenuBar( hwnd );
        if( __argc > 2 )  PostMessage( hwnd, WM_COMMAND, IDM_WRITEFILEQUIT, NULL );
        DestroyWindow( hDlg);
        hGridgenDlgBox = 0;
        DoGenerateGrid = 0;
        KillTimer( hDlg, 1);
        ColourLayerVolume();

        return TRUE;
 }

//**********************************************************************
//          Rotation and Perspective Projection Dialog Box
//**********************************************************************
    BOOL FAR PASCAL RotateDlgProc( HWND hDlg, UINT message, WPARAM wParam,
			       LPARAM lParam)
 {
  HWND hwnd;
  static int iTurn;
  switch (message)
     {
      case WM_INITDIALOG :
			SetDlgItemInt( hDlg, IDD_ABOVE, Tilt, FALSE );
			SetDlgItemInt( hDlg, IDD_ABOUT, Turn, TRUE );
         SetDlgItemInt( hDlg, IDD_PROJECTION, Projection, FALSE );
    	 return TRUE;
      case WM_COMMAND :
	 switch (wParam)
	    {
	     case IDD_HELP:
				if( HtmlHelp( hDlg, "quikgrid.chm::09nrwe.htm", HELP_CONTEXT, EditViewPoint ) == 0 )
					//if (HtmlHelp(hDlg, "quikgrid.chm::html/09nrwe.htm", HELP_CONTEXT, 0L) == 0)
		     NotifyUser( IDS_HELP);
	     return TRUE;

		  case IDPREFERENCE:
		  case IDOK :
			 Tilt = (float)GetDlgItemInt( hDlg, IDD_ABOVE, NULL, FALSE);
			 iTurn = GetDlgItemInt( hDlg, IDD_ABOUT, NULL, TRUE);
          Turn = iTurn;
			 Projection = (float)GetDlgItemInt( hDlg, IDD_PROJECTION, NULL, FALSE);
	       //RotateInitialize( Turn, Tilt, Aspect);
			 PaintContourReset();
			 hwnd = GetParent( hDlg );
          RotateInitialize( hwnd );
          RotateReset();
			 PictureChanged( hwnd );
          if( wParam==IDPREFERENCE )
										 { Save3dOptions(); return TRUE; }
			 EndDialog( hDlg, 0 );
			 return TRUE;

		  case IDCANCEL :
	       EndDialog( hDlg, 0);
	       return TRUE;
	    }
	 break;
     }
  return FALSE;
 }
 //**********************************************************************
//                    Contour Lines Dialog Box
//**********************************************************************
    BOOL FAR PASCAL ContoursDlgProc( HWND hDlg, UINT message, WPARAM wParam,
			       LPARAM lParam)
 {
   HWND hwnd, hwndCtrl;
   BOOL OK;
   static int NumberOfSecondaryLines;
   static int Posn = LabelDistance;
   static double start, increment, PrimaryIncrement, zAdjust;
   switch (message)
     {
      case WM_INITDIALOG :
         CheckDlgButton( hDlg, IDD_COLOURCONTOURS, ColourContours ) ;
			SetDlgItemInt( hDlg, IDD_NCONTOURS, NumberOfContours, FALSE );

         SetDlgItemInt( hDlg, IDD_LABELNDIGITS, LabelNDigits, FALSE );
         zAdjust = ScatterData.zAdjust();
			SetDlgItemFloat( hDlg, IDD_ZMIN, Zgrid.zmin()-zAdjust );
			SetDlgItemFloat( hDlg, IDD_ZMAX, Zgrid.zmax()-zAdjust );
         if( CustomLines != 0 )
          {
           SetDlgItemText( hDlg, IDD_CONTOURINCR, "Custom");
           SetDlgItemText( hDlg, IDD_SECONDARYLINES, " ");
          }
         else
         {
          if( PenHighLite == MaxContourLines )
           {
            SetDlgItemFloat( hDlg, IDD_CONTOURINCR, 0 ) ;
            SetDlgItemInt( hDlg, IDD_SECONDARYLINES, NumberOfContours, FALSE);
           }
          else
           {
            SetDlgItemInt( hDlg, IDD_SECONDARYLINES, PenHighLite-1, FALSE);
            SetDlgItemFloat( hDlg, IDD_CONTOURINCR,
                             (ContourLineValue(1)-ContourLineValue(0))*PenHighLite );
           }
         }
         hwndCtrl = GetDlgItem( hDlg, IDD_SCROLLDISTANCE );
         ShowScrollBar( hwndCtrl, SB_CTL, TRUE);
         SetScrollRange( hwndCtrl, SB_CTL, 300, 3000, TRUE);
			SetScrollPos( hwndCtrl, SB_CTL, Posn, TRUE );

			return TRUE;

       case WM_HSCROLL:
       switch ( LOWORD (wParam) )
       {
        case SB_LINEUP:
          Posn -= 20;
          break;

        case SB_LINEDOWN:
          Posn += 20;
          break;

        case SB_PAGEUP:
          Posn -= 200;
          break;

        case SB_PAGEDOWN:
          Posn += 200;
          break;

        case SB_THUMBPOSITION:
          Posn = HIWORD( wParam);
          break;

        default:
          return 0;
       }
       hwndCtrl = GetDlgItem( hDlg, IDD_SCROLLDISTANCE );
       SetScrollPos( hwndCtrl, SB_CTL, Posn, TRUE );
       return 0;


    case WM_COMMAND :
	 switch (LOWORD(wParam))
	    {

	     case IDD_HELP:
	        if( HtmlHelp( hDlg, "quikgrid.chm::aucb3k.htm", HELP_CONTEXT, ContourIntervals ) == 0 )
				//if (HtmlHelp(hDlg, "quikgrid.chm::html/aucb3k.htm", HELP_CONTEXT, 0L) == 0)
		        NotifyUser( IDS_HELP);
	        return TRUE;

        case IDD_COLOURCONTOURS:
          ColourContours = IsDlgButtonChecked( hDlg, IDD_COLOURCONTOURS) ;
          if( ColourContours ) ContourAutoColour(); 
          return TRUE;

        case IDPREFERENCE:
	     case IDOK :
          NumberOfSecondaryLines = GetDlgItemInt( hDlg, IDD_SECONDARYLINES, &OK, FALSE);
          if( !OK || NumberOfSecondaryLines < 0  )
            { NotifyUser( IDS_INVALIDNUMBEROFLINES );
              return TRUE; }
          if( NumberOfSecondaryLines < 0 ) NumberOfSecondaryLines = 0;
          PenHighLite = NumberOfSecondaryLines + 1;
          if( PenHighLite < 1 )   PenHighLite = 1;
	       if( PenHighLite > MaxContourLines )
                                     PenHighLite = MaxContourLines;

          if( GetDlgItemFloat( hDlg, IDD_CONTOURINCR, PrimaryIncrement )) return TRUE;
          if( PrimaryIncrement <= 0.0 )
            {
              NumberOfContours = NumberOfSecondaryLines;
              ContourLinesSet( );
              ContourBoldLabelSet( PenHighLite ); 
              PenHighLite = MaxContourLines;
            }
          else
            {
              increment = PrimaryIncrement/PenHighLite;
              zAdjust = ScatterData.zAdjust();
              start = (float)( (int)((Zgrid.zmin()-zAdjust)/PrimaryIncrement) )
                                                   *PrimaryIncrement + zAdjust;
              ContourLinesSet( start, increment, Zgrid.zmax());
              ContourBoldLabelSet( PenHighLite ); 
            }
          ContourBoldLabelSet( PenHighLite );
          LabelNDigits =  GetDlgItemInt( hDlg, IDD_LABELNDIGITS, NULL, FALSE);
          if( LabelNDigits < 1 )   LabelNDigits = 1;
	       if( LabelNDigits > 6 ) LabelNDigits = 6;
          LabelDistance = Posn;
          if( LOWORD(wParam) == IDPREFERENCE )
           {
             SaveContourOptions();
             return TRUE;
           }

	       hwnd = GetParent( hDlg );
	       PictureChanged( hwnd );
	       EndDialog( hDlg, 0 );
	       return TRUE;

	     case IDCANCEL :
	       EndDialog( hDlg, 0);
	       return TRUE;
	    }
	 break;
     }
  return FALSE;
 }

 //*************************************************************************
 //   T e c h n i c a l    P a r a m e t e r s    D i a l o g
 //     ** i.e.  G r i d   G e n e r a t i o n  O p t i o n s **
 //*************************************************************************
 static int Changed;
 static int UndefZButton = 0;
 static double UndefZ, OldUndefZ, NewUndefZ;
 static int CheckAndStashTechnical( HWND hDlg )
 {
   static int NewValue, OldValue;
      // First benign technical parameters. Grid regeneration
		// required to see changes but is not necessary.
	   Changed = 0;
	   if( IsDlgButtonChecked( hDlg, IDD_SETGRIDUNDEF) )
	     {
	      if(!GetDlgItemFloat( hDlg, IDD_VALUEUNDEF, NewUndefZ) )
		{
		 if( NewUndefZ != OldUndefZ || UndefZButton == 0 )
		   {
		    ScatterData.zAdjust( 0.0 );
		    if( NewUndefZ < ScatterData.zMin() )
		      {
		        ScatterData.zAdjust( -NewUndefZ );
                        UndefZ = 0.0;
                      }
		    else
		      {
			ScatterData.zAdjust( -ScatterData.zMin() );
			UndefZ = NewUndefZ + ScatterData.zAdjust();
                      }
		    assert( UndefZ >= 0.0 );
		    XpandUndefinedZ( UndefZ );
		    //Zgrid.Reset();
          InitializeZgrid();
		    Changed = 1;
                   }
		} // endif !GetDlg...
                  else return TRUE;
	     } // endif IdsDlgButton...
	   else
	     { if( UndefZButton == 1 )
	      {
           ScatterData.zAdjust( 0.0 );
	        ScatterData.zAdjust( -ScatterData.zMin() );
		     XpandUndefinedZ( -99999. );
		     //Zgrid.Reset();
           InitializeZgrid();
		     Changed = 1;
	      }
             }

	   NewValue = GetDlgItemInt( hDlg, IDD_BANDWIDTHCUTOFF, NULL, FALSE);
	   OldValue = XpandScanRatio( NewValue );
      if( NewValue != OldValue ) Changed = 1;
	   NewValue = GetDlgItemInt( hDlg, IDD_DENSITYRATIO, NULL, FALSE);
	   OldValue = XpandDensityRatio( NewValue);
      if( NewValue != OldValue ) Changed = 1;
	   NewValue = GetDlgItemInt( hDlg, IDD_EDGESENSITIVITY, NULL, FALSE);
	   OldValue = XpandEdgeFactor( NewValue );
	   if( NewValue != OldValue ) Changed = 1;
		NewValue = GetDlgItemInt( hDlg, IDD_INTERNALSAMPLE, NULL, FALSE);
      if( NewValue < 1 ) NewValue = 1;
		if( (ScatterData.Size()/(long)NewValue < 3) && (NewValue != 1) )
		  {
			 NotifyUser( IDS_SAMPLESIZESMALL );
		  }
		else
	     {
			  OldValue = XpandSample( NewValue);
			  if( NewValue != OldValue ) Changed = 1;
	     }

  return TRUE;
 }
 //****************************************************************************
	  BOOL FAR PASCAL TechnicalDlgProc( HWND hDlg, UINT message, WPARAM wParam,
			       LPARAM lParam)
 {
   HWND hwnd;
   static int Regenerate;
   switch (message)
   {
    case WM_INITDIALOG :
      Regenerate = FALSE;
      SetDlgItemInt( hDlg, IDD_BANDWIDTHCUTOFF, XpandScanRatio(), FALSE );
      SetDlgItemInt( hDlg, IDD_DENSITYRATIO, XpandDensityRatio(), FALSE);
      SetDlgItemInt( hDlg, IDD_EDGESENSITIVITY, XpandEdgeFactor(), FALSE);
      SetDlgItemInt( hDlg, IDD_INTERNALSAMPLE, XpandSample(), FALSE );
      UndefZ = XpandUndefinedZ();
		if( UndefZ < 0.0 )
	   {
        UndefZButton = 0;
	     CheckDlgButton( hDlg, IDD_SETGRIDUNDEF, UndefZButton );
	     SetDlgItemFloatXY( hDlg, IDD_VALUEUNDEF, 0.0 );
      }
		else
	   {
        UndefZButton = 1;
	     CheckDlgButton( hDlg, IDD_SETGRIDUNDEF, UndefZButton );
	     SetDlgItemFloatXY( hDlg, IDD_VALUEUNDEF, UndefZ-ScatterData.zAdjust() );
	     GetDlgItemFloat( hDlg, IDD_VALUEUNDEF, OldUndefZ);
	   }
      return TRUE;

    case WM_COMMAND :
      switch (wParam)
 {
	 case IDD_HELP:
	     if( HtmlHelp( hDlg, "quikgrid.chm::kb32tk.htm", HELP_CONTEXT, TechnicalParameters ) == 0 )
			 //if (HtmlHelp(hDlg, "quikgrid.chm::html/kb32tk.htm", HELP_CONTEXT, 0L) == 0)
		     NotifyUser( IDS_HELP);
	     return TRUE;

//	 int Changed;

    case IDPREFERENCE:
        CheckAndStashTechnical( hDlg );
        SaveTechnicalOptions();
        if( Changed ) Regenerate = TRUE;
        return TRUE;

	 case IDOK :
      CheckAndStashTechnical( hDlg );

	  if( Changed||Regenerate )
	   {
		hwnd = GetParent( hDlg );
		PostMessage( hwnd, WM_COMMAND, IDM_REGENGRID, NULL );
	   }

	 case IDCANCEL :
	   EndDialog( hDlg, 0);
	   return TRUE;
       }
       break;
     }
  return FALSE;
 }
//**********************************************************************
//          N e w  T i t l e    D I A L O G    B O X
//**********************************************************************
    BOOL FAR PASCAL TitleDlgProc( HWND hDlg, UINT message, WPARAM wParam,
			       LPARAM lParam)
 {
    static HWND hwnd;
    switch (message)
     {
      case WM_INITDIALOG :
	 SetDlgItemText( hDlg, IDD_TITLETEXT, szTitle );
	 return TRUE;
      case WM_COMMAND :
	 switch (wParam)
	    {
	     case IDD_HELP:
	       //if( HtmlHelp( hDlg, "quikgrid.chm::html/edittitle.htm", HELP_CONTEXT, EditTitle ) == 0 )
			 if (HtmlHelp(hDlg, "quikgrid.chm::edittitle.htm", HELP_CONTEXT, 0L) == 0)
		     NotifyUser( IDS_HELP);
	     return TRUE;

	     case IDOK :
	       GetDlgItemText( hDlg, IDD_TITLETEXT, szTitle, 255 );
          hwnd = GetParent( hDlg );
          PictureChanged( hwnd );
	     case IDCANCEL :
	       EndDialog( hDlg, 0);
	       return TRUE;
	    }
	 break;
     }
  return FALSE;
 }

//**********************************************************************
//          R e a d i n g  D a t a  D i a l o g   B o x  (Modeless)
//**********************************************************************
    BOOL FAR PASCAL ReadingDataDlgProc( HWND hDlg, UINT message, WPARAM wParam,
			       LPARAM lParam)
 {
  static int i;
  switch (message)
   {
    case WM_INITDIALOG :
      if( !SetTimer( hDlg, 1, 1000, NULL)) NotifyUser( IDS_TOOMANYTIMERS );
      SetDlgItemText( hDlg, IDD_POINTSREAD, " ");
	   return TRUE;

    case WM_COMMAND :
	   switch (wParam)
	    {
	     case IDCANCEL :
        CancelLoadData = TRUE;
        return TRUE;
	    }
       break;

    case WM_TIMER:
      SetDlgItemInt( hDlg, IDD_POINTSREAD, PointsKept, FALSE);
      return TRUE;

    case WM_CLOSE :
      HWND hwnd = GetParent( hDlg );
      HMENU hMenu = GetMenu(hwnd);
      for( i = 0; i < 6; i++)
			EnableMenuItem( hMenu, i, MF_ENABLED|MF_BYPOSITION);
		if( LatLonData) CheckMenuItem( hMenu, IDM_LATLON, MF_CHECKED);
		else         CheckMenuItem( hMenu, IDM_LATLON, MF_UNCHECKED);
		DrawMenuBar( hwnd );
		KillTimer( hDlg, 1);
		DestroyWindow( hDlg );
      return TRUE;
   }
  return FALSE;
 }


//**********************************************************************
//          D a t a  I n p u t O p t i o n s
//
// April 13/98: Add UsePolyline item.
// April 22/98: Add FlipXY uption.
// May   16/98: Add help button to dialog box.
// Jan.  16/99: Add "Use Old Style Input".
// Feb. 22/99:  Add Automatic grid size ratio and maximum.
// Mar. 4/99:   Add ReverseSignOfZ.
// Dec. 28/99: Move data output options to separate dialog box.
// Mar. 12/01: Add NicexyCoordinates.
// Jan. 29/03: Add UseThreads
//**********************************************************************
static int UseTabSeparator;
static void CheckAndStashInput( HWND hDlg )
{
  static int i, NewValue, OldValue;
  NewValue = GetDlgItemInt( hDlg, IDD_GRIDSIZERATIO, NULL, FALSE);
  OldValue = GridSizeRatio( );
  if( NewValue != OldValue )
	 {
	   if( (NewValue > 20) || (NewValue < 1) )
		   { NotifyUser( IDS_GRIDRATIOINVALID ); return; }

	   GridSizeRatio( NewValue );
    };
  NewValue = GetDlgItemInt( hDlg, IDD_GRIDSIZERATIOMAX, NULL, FALSE);
  GridMaxRatio( NewValue );
  IgnoreZzero( IsDlgButtonChecked( hDlg, IDD_IGNOREZENABLE ) );
  NewValue = GetDlgItemInt( hDlg, IDD_KEEPEVERY, NULL, FALSE);
  KeepEveryNth( NewValue );
  UsePolyline =     IsDlgButtonChecked( hDlg, IDD_POLYLINES );
  FlipXY =          IsDlgButtonChecked( hDlg, IDD_FLIPXY );
  ReverseSignOfZ =  IsDlgButtonChecked( hDlg, IDD_REVERSEZSIGN );
  NewInputStyle =  !IsDlgButtonChecked( hDlg, IDD_NEWSTYLEINPUT );
  UseTabSeparator = IsDlgButtonChecked( hDlg, IDD_USETABSEPARATOR );
  IgnoreComments =  IsDlgButtonChecked( hDlg, IDD_IGNORECOMMENTS);
  IgnoreErrorLines = IsDlgButtonChecked( hDlg, IDD_IGNOREERRORLINES);
  NicexyCoordinates = IsDlgButtonChecked( hDlg, IDD_NICEXYCOORDS);
  UseThreads =       IsDlgButtonChecked( hDlg, IDD_USETHREADS);
  SubmetrixSelectZ = GetDlgItemInt( hDlg, IDD_SUBMETRIXZ, NULL, FALSE);
   if( (SubmetrixSelectZ < 1) || (SubmetrixSelectZ > 2) )
           {
             SubmetrixSelectZ = 1;
             NotifyUser( IDS_SUBMETRIXNOT1OR2 );
           }
  SeparatorCharacter = ' ';
  if( UseTabSeparator ) SeparatorCharacter = '\t';
  InputFieldPosition[0] = GetDlgItemInt( hDlg, IDD_XFIELDPOSITION, NULL, FALSE) -1 ;
  InputFieldPosition[1] = GetDlgItemInt( hDlg, IDD_YFIELDPOSITION, NULL, FALSE) -1 ;
  InputFieldPosition[2] = GetDlgItemInt( hDlg, IDD_ZFIELDPOSITION, NULL, FALSE) -1 ;
  InputFieldPosition[3] = GetDlgItemInt( hDlg, IDD_CFIELDPOSITION, NULL, FALSE) -1 ;
  NOAAInputFieldPosition[0] = GetDlgItemInt( hDlg, IDD_NOAAXFIELDPOSITION, NULL, FALSE) -1 ;
  NOAAInputFieldPosition[1] = GetDlgItemInt( hDlg, IDD_NOAAYFIELDPOSITION, NULL, FALSE) -1 ;
  NOAAInputFieldPosition[2] = GetDlgItemInt( hDlg, IDD_NOAAZFIELDPOSITION, NULL, FALSE) -1 ;
  NOAAInputFieldPosition[3] = GetDlgItemInt( hDlg, IDD_NOAACFIELDPOSITION, NULL, FALSE) -1 ;
  for( i = 0; i < 3 ; i++ )
   {
    if( InputFieldPosition[i] > 29 || InputFieldPosition[i] < 0 ||
        NOAAInputFieldPosition[i] > 29 || NOAAInputFieldPosition[i] < 0)
     {
       NotifyUser( IDS_DIALOGFIELDPOSITIONBAD);
       InputFieldPosition[i] = 0;
     }
   }
     if( InputFieldPosition[3] > 29 || InputFieldPosition[3] < 0 ) InputFieldPosition[3] = -1;
}
//*****************************************************************************
	 BOOL FAR PASCAL DataInputOptionsDlgProc( HWND hDlg, UINT message, WPARAM wParam,
			       LPARAM lParam)
 {

  switch (message)
     {
		case WM_INITDIALOG :
		  CheckDlgButton( hDlg, IDD_POLYLINES, UsePolyline ) ;
		  CheckDlgButton( hDlg, IDD_FLIPXY, FlipXY ) ;
        CheckDlgButton( hDlg, IDD_REVERSEZSIGN, ReverseSignOfZ ) ;
		  CheckDlgButton( hDlg, IDD_IGNOREZENABLE, IgnoreZzero() );
		  CheckDlgButton( hDlg, IDD_NEWSTYLEINPUT, !NewInputStyle ) ;
		  CheckDlgButton( hDlg, IDD_IGNORECOMMENTS, IgnoreComments);
		  CheckDlgButton( hDlg, IDD_IGNOREERRORLINES, IgnoreErrorLines);
        CheckDlgButton( hDlg, IDD_NICEXYCOORDS, NicexyCoordinates);
        CheckDlgButton( hDlg, IDD_USETHREADS, UseThreads);
		  UseTabSeparator = FALSE;
		  if( SeparatorCharacter == '\t' ) UseTabSeparator = TRUE;
		  CheckDlgButton( hDlg, IDD_USETABSEPARATOR, UseTabSeparator ) ;
		  SetDlgItemInt( hDlg, IDD_KEEPEVERY, KeepEveryNth(), FALSE);
        SetDlgItemInt( hDlg, IDD_GRIDSIZERATIO, GridSizeRatio(), FALSE);
        SetDlgItemInt( hDlg, IDD_GRIDSIZERATIOMAX, GridMaxRatio(), FALSE);
        SetDlgItemInt( hDlg, IDD_SUBMETRIXZ, SubmetrixSelectZ, FALSE);
        SetDlgItemInt( hDlg, IDD_XFIELDPOSITION, InputFieldPosition[0]+1, FALSE);
        SetDlgItemInt( hDlg, IDD_YFIELDPOSITION, InputFieldPosition[1]+1, FALSE);
        SetDlgItemInt( hDlg, IDD_ZFIELDPOSITION, InputFieldPosition[2]+1, FALSE);
        SetDlgItemInt( hDlg, IDD_CFIELDPOSITION, InputFieldPosition[3]+1, FALSE);
        SetDlgItemInt( hDlg, IDD_NOAAXFIELDPOSITION, NOAAInputFieldPosition[0]+1, FALSE);
        SetDlgItemInt( hDlg, IDD_NOAAYFIELDPOSITION, NOAAInputFieldPosition[1]+1, FALSE);
        SetDlgItemInt( hDlg, IDD_NOAAZFIELDPOSITION, NOAAInputFieldPosition[2]+1, FALSE);
        SetDlgItemInt( hDlg, IDD_NOAACFIELDPOSITION, NOAAInputFieldPosition[3]+1, FALSE);
	     return TRUE;

      case WM_COMMAND :
	 switch (wParam)
	    {

        case IDPREFERENCE:
          CheckAndStashInput( hDlg );
			 SaveInputOptions();
          return TRUE;

	     case IDOK:
          CheckAndStashInput( hDlg );
		    EndDialog( hDlg, 0);
	       return TRUE;

		  case IDHELP:
		    if( HtmlHelp( hDlg, "quikgrid.chm::datainputoptions.htm", HELP_CONTEXT, DataInputOptions ) == 0 )
				//if (HtmlHelp(hDlg, "quikgrid.chm::html/datainputoptions.htm", HELP_CONTEXT, 0L) == 0)
			    NotifyUser( IDS_HELP);
		    return TRUE;

	     case IDCANCEL:
	       EndDialog( hDlg, 0);
	       return TRUE;
		 }
	 break;
      case WM_CLOSE:
	      EndDialog( hDlg, 0);
         return TRUE;
     }
  return FALSE;
 }
//*******************************************************************
// D a t a  O u t p u t  O p t i o n s  S e r v i c e  r o u t i n e
//*******************************************************************
static void CheckAndStashOutput( HWND hDlg )
{
  static double temp;
  VRMLtriangles = IsDlgButtonChecked(hDlg, IDD_VRMLTRIANGLES);
  VRML2format = IsDlgButtonChecked(hDlg, IDD_VRML2OUTPUT);
  UsePolyline =     IsDlgButtonChecked( hDlg, IDD_POLYLINES );
  UseTabSeparator = IsDlgButtonChecked( hDlg, IDD_USETABSEPARATOR );
  NeverLabelDXFContours = IsDlgButtonChecked( hDlg, IDD_DXFNEVERLABEL );
  UseScatteredDataLayers = IsDlgButtonChecked( hDlg, IDD_USESCATTEREDDATALAYERS );
  DXFFaceScaleZ = IsDlgButtonChecked( hDlg, IDD_DXFFACESCALEZ );
  HighDXFOutputPrecision = IsDlgButtonChecked( hDlg, IDD_HIGHDXFOUTPUTPRECISION );
  SeparatorCharacter = ' ';
  if( UseTabSeparator ) SeparatorCharacter = '\t';
  OutputUndefinedValues = IsDlgButtonChecked( hDlg, IDD_OUTPUTUNDEFINED );
  GetDlgItemFloat( hDlg, IDD_UNDEFINEDVALUE, UndefinedValue );
  GetDlgItemFloat( hDlg, IDD_DXFTEXTHEIGHT, temp );
  DXFTextHeightRatio = temp*.01;
}
//**********************************************************************
//          D a t a  O u t p u t  O p t i o n s
//**********************************************************************
BOOL FAR PASCAL DataOutputOptionsDlgProc( HWND hDlg, UINT message, WPARAM wParam,
			       LPARAM lParam)
{
switch (message)
     {
		case WM_INITDIALOG :
		  CheckDlgButton( hDlg, IDD_POLYLINES, UsePolyline ) ;
		  UseTabSeparator = FALSE;
		  if( SeparatorCharacter == '\t' ) UseTabSeparator = TRUE;
		  CheckDlgButton( hDlg, IDD_USETABSEPARATOR, UseTabSeparator ) ;
		  CheckDlgButton( hDlg, IDD_OUTPUTUNDEFINED, OutputUndefinedValues) ;
		  CheckDlgButton( hDlg, IDD_USESCATTEREDDATALAYERS, UseScatteredDataLayers) ;
        CheckDlgButton( hDlg, IDD_DXFFACESCALEZ, DXFFaceScaleZ ) ;
        CheckDlgButton( hDlg, IDD_HIGHDXFOUTPUTPRECISION, HighDXFOutputPrecision ) ;
        CheckDlgButton( hDlg, IDD_DXFNEVERLABEL, NeverLabelDXFContours );
        CheckDlgButton( hDlg, IDD_VRMLTRIANGLES, VRMLtriangles ); 
        CheckDlgButton( hDlg, IDD_VRML2OUTPUT, VRML2format ); 
		  SetDlgItemFloat( hDlg, IDD_UNDEFINEDVALUE, UndefinedValue );
        SetDlgItemFloat( hDlg, IDD_DXFTEXTHEIGHT, DXFTextHeightRatio*100.);
		  return TRUE;

      case WM_COMMAND :
	 switch (wParam)
	    {

        case IDPREFERENCE:
			 CheckAndStashOutput( hDlg );
          SaveOutputOptions();
          return TRUE;

	     case IDOK:
			 CheckAndStashOutput( hDlg );
		    EndDialog( hDlg, 0);
	       return TRUE;

		  case IDHELP:
		   if( HtmlHelp( hDlg, "quikgrid.chm::dataoutputoptions.htm", HELP_CONTEXT, DataOutputOptions ) == 0 )
			   //if (HtmlHelp(hDlg, "quikgrid.chm::html/dataoutputoptions.htm", HELP_CONTEXT, 0L) == 0)
			    NotifyUser( IDS_HELP);
		   return TRUE;

	     case IDCANCEL:
	       EndDialog( hDlg, 0);
	       return TRUE;
		 }
	 break;
      case WM_CLOSE:
	      EndDialog( hDlg, 0);
         return TRUE;
     }
  return FALSE;
 }

//**********************************************************************
//          V i e w  D a t a  P o i n t s    D I A L O G    B O X
// Feb. 1/99:  Add pointnumber options.
//**********************************************************************
    BOOL FAR PASCAL DataPointsDlgProc( HWND hDlg, UINT message, WPARAM wParam,
			       LPARAM lParam)
 {
   static ContourOptionsType OldDrawingOptions;
   static int OldDataPointsOnGridOnly, OldSmallMark;
  switch (message)
     {
  case WM_INITDIALOG :
	 CheckDlgButton( hDlg, IDD_MARKPLOT, DrawingOptions.showmarks );
    CheckDlgButton( hDlg, IDD_SMALLMARK, SmallMark );
	 CheckDlgButton( hDlg, IDD_ZPLOT , DrawingOptions.numbers );
	 CheckDlgButton( hDlg, IDD_XYPLOT  , DrawingOptions.xyplot );
	 CheckDlgButton( hDlg, IDD_COMMENTPLOT , DrawingOptions.comments );
	 CheckDlgButton( hDlg, IDD_LINEPLOT    , DrawingOptions.lines );
    CheckDlgButton( hDlg, IDD_NUMBERPLOT  , DrawingOptions.pointnumber );
    CheckDlgButton( hDlg, IDD_DATAPOINTSONGRIDONLY  , DataPointsOnGridOnly );
	 return TRUE;

  case WM_COMMAND :
	 switch (wParam)
	    {
	     case IDD_HELP:
			 if (HtmlHelp(hDlg, "quikgrid.chm::15q.8f2.htm", HELP_CONTEXT, ViewDataPointOptions) == 0)
				 //if (HtmlHelp(hDlg, "quikgrid.chm::html/15q.8f2.htm", HELP_CONTEXT, 0L) == 0)
			  NotifyUser( IDS_HELP);
	     return TRUE;

		  case IDPREFERENCE:
         OldDrawingOptions = DrawingOptions;
         OldDataPointsOnGridOnly = DataPointsOnGridOnly;
         OldSmallMark = SmallMark;

			DrawingOptions.showmarks = IsDlgButtonChecked( hDlg, IDD_MARKPLOT);
         SmallMark = IsDlgButtonChecked( hDlg, IDD_SMALLMARK);
			DrawingOptions.pointnumber = IsDlgButtonChecked( hDlg, IDD_NUMBERPLOT);
			DrawingOptions.numbers = IsDlgButtonChecked( hDlg, IDD_ZPLOT) ;
			DrawingOptions.xyplot = IsDlgButtonChecked( hDlg, IDD_XYPLOT) ;
			DrawingOptions.comments = IsDlgButtonChecked( hDlg, IDD_COMMENTPLOT) ;
			DrawingOptions.lines = IsDlgButtonChecked( hDlg, IDD_LINEPLOT) ;
         DataPointsOnGridOnly = IsDlgButtonChecked( hDlg, IDD_DATAPOINTSONGRIDONLY);
			SaveViewDataOptions();
         if( !DrawingOptions.marks ) return TRUE; // marks are not being displayed
			if( OldDrawingOptions.showmarks != DrawingOptions.showmarks ||
				 OldDrawingOptions.numbers != DrawingOptions.numbers ||
				 OldDrawingOptions.pointnumber != DrawingOptions.pointnumber ||
				 OldDrawingOptions.xyplot != DrawingOptions.xyplot ||
				 OldDrawingOptions.comments != DrawingOptions.comments ||
				 OldDrawingOptions.lines != DrawingOptions.lines||
             OldDataPointsOnGridOnly != DataPointsOnGridOnly||
             OldSmallMark != SmallMark )
			{
			  HWND hwnd = GetParent( hDlg );
			  PictureChanged( hwnd );
			}
			return TRUE;

	     case IDOK :
			OldDrawingOptions = DrawingOptions;
         OldDataPointsOnGridOnly = DataPointsOnGridOnly;
         OldSmallMark = SmallMark;

			DrawingOptions.showmarks = IsDlgButtonChecked( hDlg, IDD_MARKPLOT);
         SmallMark = IsDlgButtonChecked( hDlg, IDD_SMALLMARK);
			DrawingOptions.pointnumber = IsDlgButtonChecked( hDlg, IDD_NUMBERPLOT);
			DrawingOptions.numbers = IsDlgButtonChecked( hDlg, IDD_ZPLOT) ;
			DrawingOptions.xyplot = IsDlgButtonChecked( hDlg, IDD_XYPLOT) ;
			DrawingOptions.comments = IsDlgButtonChecked( hDlg, IDD_COMMENTPLOT) ;
			DrawingOptions.lines = IsDlgButtonChecked( hDlg, IDD_LINEPLOT) ;
         DataPointsOnGridOnly = IsDlgButtonChecked( hDlg, IDD_DATAPOINTSONGRIDONLY);
         if( !DrawingOptions.marks )
             { EndDialog( hDlg, 0); return TRUE;} // marks are not being displayed
			if( OldDrawingOptions.showmarks != DrawingOptions.showmarks ||
				 OldDrawingOptions.numbers != DrawingOptions.numbers ||
				 OldDrawingOptions.pointnumber != DrawingOptions.pointnumber ||
				 OldDrawingOptions.xyplot != DrawingOptions.xyplot ||
				 OldDrawingOptions.comments != DrawingOptions.comments ||
				 OldDrawingOptions.lines != DrawingOptions.lines ||
             OldDataPointsOnGridOnly != DataPointsOnGridOnly ||
             OldSmallMark != SmallMark )
			    {
			     HWND hwnd = GetParent( hDlg );
			     PictureChanged( hwnd );
			    }
            EndDialog( hDlg, 0);
            return TRUE;

	     case IDCANCEL :
	       EndDialog( hDlg, 0);
	       return TRUE;
	    }
	 break;
     }
  return FALSE;
 }
//**********************************************************************
//			Z r a t i o    D I A L O G    B O X
//
//  Change the shape of the surface. (What I call the Z ratio).
//**********************************************************************
	 BOOL FAR PASCAL ZratioDlgProc( HWND hDlg, UINT message, WPARAM wParam,
					 LPARAM lParam)
 {
  int NewValue, OldValue;
  switch (message)
     {
		case WM_INITDIALOG :
			  SetDlgItemInt( hDlg, IDD_ZRATIO, PaintSurfaceRatio(), FALSE);
      return TRUE;

	case WM_COMMAND :
		switch (wParam)
	    {
		  case IDD_HELP:
		  if( HtmlHelp( hDlg, "quikgrid.chm::zratio.htm", HELP_CONTEXT, ZRatio ) == 0 )
			  //if (HtmlHelp(hDlg, "quikgrid.chm::html/zratio.htm", HELP_CONTEXT, 0L) == 0)
			  NotifyUser( IDS_HELP);
		  return TRUE;

        case IDPREFERENCE:
          NewValue = GetDlgItemInt( hDlg, IDD_ZRATIO, NULL, FALSE );
			 OldValue = PaintSurfaceRatio();
          if( OldValue != NewValue )
			  {
		       PaintSurfaceRatio( NewValue );
		       if( PaintState != IDM_2DSURFACE )
              {
			      HWND hwnd = GetParent( hDlg );
			      PictureChanged( hwnd ); // let cancel.
			     }
		     }
			 SaveZratioOptions();
			 return TRUE;

		  case IDOK :
			 NewValue = GetDlgItemInt( hDlg, IDD_ZRATIO, NULL, FALSE );
			 OldValue = PaintSurfaceRatio();
			 if( OldValue != NewValue )
			  {
		       PaintSurfaceRatio( NewValue );
		       if( PaintState != IDM_2DSURFACE )
              {
			      HWND hwnd = GetParent( hDlg );
			      PictureChanged( hwnd ); // let cancel.
			     }
		     }
		  case IDCANCEL :
			 EndDialog( hDlg, 0);
			 return TRUE;
		 }
	 break;
	  }
  return FALSE;
 }
 //**********************************************************************
//			Z o o m   R a t i o   D I A L O G    B O X
//
//  Change the value for the amount of zooming to do.
//**********************************************************************
static int NewZoom, OldZoom, NewPan, OldPan;
static int CheckAndStashZoom( HWND hDlg )
  {
	 NewZoom = GetDlgItemInt( hDlg, IDD_ZOOMRATIO, NULL, FALSE );
	 if( NewZoom < 1 || NewZoom > 300 )
	  { NotifyUser( IDS_ZOOMCHANGEBAD );
		 return 0; }
	 if( OldZoom != NewZoom ) ZoomRatio = (float) NewZoom/100;
	 NewPan = GetDlgItemInt( hDlg, IDD_PANRATIO, NULL, FALSE );
	 if( NewPan < 1 || NewPan > 50 )
		{ NotifyUser( IDS_PANCHANGEBAD );
		  return 0; }
	 if( OldPan != NewPan ) PanRatio = (float) NewPan/100;
	 return 1;
  }
//************************************************************************
BOOL FAR PASCAL ZoomRatioDlgProc( HWND hDlg, UINT message, WPARAM wParam,
					 LPARAM lParam)
 {
  switch (message)
     {
		case WM_INITDIALOG :
		  OldZoom = ZoomRatio*100;
		  SetDlgItemInt( hDlg, IDD_ZOOMRATIO, OldZoom , FALSE);
		  OldPan = PanRatio*100;
		  SetDlgItemInt( hDlg, IDD_PANRATIO, OldPan , FALSE);
		return TRUE;

		case WM_COMMAND :
		switch (wParam)
	    {
		  case IDD_HELP:
			  if (HtmlHelp(hDlg, "quikgrid.chm::zooming.htm", HELP_CONTEXT, ZoomRatioHelp) == 0)
				  //if (HtmlHelp(hDlg, "quikgrid.chm::html/zooming.htm", HELP_CONTEXT, 0L) == 0)
						 NotifyUser( IDS_HELP);
			  return TRUE;

		  case IDPREFERENCE:
			 if( CheckAndStashZoom ( hDlg ) ) SaveZoomOptions();
			 return TRUE;

		  case IDOK :
			 if( CheckAndStashZoom( hDlg ) ) EndDialog( hDlg, 0);
			 return TRUE;

		  case IDCANCEL :
			 EndDialog( hDlg, 0);
			 return TRUE;
		 }
	 break;
	  }
  return FALSE;
 }
