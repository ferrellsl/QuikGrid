//**********************************************************************
//    R i g h t  M o u s e B u t t o n  D i a l o g  B o x
//                   P r o c e d u r e s
//
//              Copyright (c) 1993 - 2000 by  John Coulthard
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
//     GridLocationDlgProc        - full information
//     RightMouseOptionsDlgProc   - Options for right mouse stuff.
//     GridLocnTerseDlgProc       - terse information
//
//  March 16/00: Stuff transfered from dialogbx.cpp
//               RightMouseOptions and GridLocnTerse... created. 
//  Nov. 6/01: Fix failure bug - Did not check to stop adding a comment to
//             a grid intesection. 
//**********************************************************************
#include <windows.h>
#include <HtmlHelp.h>
#include <math.h>
#include "utilitys.h"
#include "rc.h"
#include "prefer.h"
#include "surfgrid.h"
#include "scatdata.h"
#include "quikgrid.h"
#include "paintcon.h"
#include "loaddata.h"
#include "dlgutil.h"
#include "rotate.h"
#include "helpmap.h"
#include "condraw.h" 

extern ContourOptionsType DrawingOptions;
extern HWND hGridLocationDlgBox;

char RightMouseDialogTitle[200] = "Information for ";
int  RightMouseDialogTerse = FALSE;

static long PointFound;
//**********************************************************************
//         G r i d   L o c a t i o n   D i a l o g   B o x  (Modeless)
//**********************************************************************
//      I n i t   G r i d  I n f o  B o x
//**********************************************************************
static void InitGridInfoBox( HWND hDlg )   // Service routine
{
 SetDlgItemText( hDlg, IDD_POINTTYPE, "Searching..." );
 SetDlgItemText( hDlg, IDD_GRIDLOCATIONX, " " );
 SetDlgItemText( hDlg, IDD_GRIDLOCATIONY, " " );
 SetDlgItemText( hDlg, IDD_GRIDVALUEX, " ");
 SetDlgItemText( hDlg, IDD_GRIDVALUEY," ");
 SetDlgItemText( hDlg, IDD_GRIDVALUEZ, " ");
 SetDlgItemText( hDlg, IDD_CLOSECONTOUR, " " ) ;
 SetDlgItemText( hDlg, IDD_COMMENT, " ");
 CheckDlgButton( hDlg, IDD_IGNOREPOINT, 0 );
}
//**********************************************************************
//           F i l l   G r i d   I n f o   B o x
//**********************************************************************
static long ix, iy;
static float zadjust;
static void FillGridInfoBox(HWND hDlg )     // Service routine for this function.
  {
    int i, CloseIndex;
    float CloseDistance, temp, zvalue;
    double xadjust, yadjust;
    GetxyInfo( ix, iy );
    LoadNormalization( xadjust, yadjust);
    zadjust = ScatterData.zAdjust();
    PointFound = -1;

    SetWindowText( hDlg, RightMouseDialogTitle );
    CheckDlgButton( hDlg, IDD_IGNOREPOINT, 0 );
    CheckDlgButton( hDlg, IDD_DISPLAYPOINT, 0 );

    if( iy < 0 )  // display information on data point.
      {
        PointFound = ix; 
        SetDlgItemText( hDlg, IDD_POINTTYPE, "Data Point:" );
        SetDlgItemLong( hDlg, IDD_GRIDLOCATIONX, PointFound+1 );
        SetDlgItemText( hDlg, IDD_GRIDLOCATIONY, " " ); 
        SetDlgItemFloatXY( hDlg, IDD_GRIDVALUEX, ScatterData.x(ix)+xadjust);
        SetDlgItemFloatXY( hDlg, IDD_GRIDVALUEY, ScatterData.y(ix)+yadjust);
        zvalue = ScatterData.z(ix); 
	     SetDlgItemFloat( hDlg, IDD_GRIDVALUEZ, zvalue-zadjust);
	     SetDlgItemText( hDlg, IDD_COMMENT, ScatterData.comment(ix));
	     if( (ScatterData.flags(ix)&1) )
	         CheckDlgButton( hDlg, IDD_IGNOREPOINT, 1 );
      }

    else   // display information on grid location. 
     {
       SetDlgItemText( hDlg, IDD_POINTTYPE, "Grid Intersection:" );
       SetDlgItemText( hDlg, IDD_COMMENT, " ");
       SetDlgItemInt( hDlg, IDD_GRIDLOCATIONX, ix+1, FALSE );
       SetDlgItemInt( hDlg, IDD_GRIDLOCATIONY, iy+1, FALSE );
       if( LatLonData )
       {
         SetDlgItemText( hDlg, IDD_GRIDVALUEX, FormatLon(Zgrid.x(ix)+xadjust) );
         SetDlgItemText( hDlg, IDD_GRIDVALUEY, FormatLat(Zgrid.y(iy)+yadjust) );
       }
       else
       {
         SetDlgItemFloatXY( hDlg, IDD_GRIDVALUEX, Zgrid.x(ix)+xadjust );
         SetDlgItemFloatXY( hDlg, IDD_GRIDVALUEY, Zgrid.y(iy)+yadjust );
       }
       zvalue = Zgrid.z( ix, iy ); 
       if( zvalue < 0 )
         SetDlgItemText( hDlg, IDD_GRIDVALUEZ, "Undefined" );
       else
         SetDlgItemFloat( hDlg, IDD_GRIDVALUEZ, zvalue-zadjust );

     }
	 // Now find the closest contour line.
   if( zvalue < 0 )
      SetDlgItemText( hDlg, IDD_CLOSECONTOUR, "Undefined" ) ;
   else
     { 
       CloseDistance = fabs( ContourLineValue(0) - zvalue);
       CloseIndex = 0;
       for( i=1; i<NumberOfContours; i++ )
        {
          temp = fabs( ContourLineValue(i) - zvalue);
          if( temp < CloseDistance )
            { CloseDistance = temp; CloseIndex = i; }
        }
       SetDlgItemFloat( hDlg, IDD_CLOSECONTOUR, ContourLineValue(CloseIndex) - zadjust );
     }
  }
//**************************************************************************
//           G r i d   L o c a t i o n   D l g  P r o c
//**************************************************************************
BOOL FAR PASCAL GridLocationDlgProc( HWND hDlg, UINT message, WPARAM wParam,
			       LPARAM lParam)
 {
  static int ButtonCheck, PointIgnore;
  static char OldComment[255], NewComment[255], oldZvalue[30], newZvalue[30];
  static double zValue;

  switch (message)
  {
    case WM_INITDIALOG:
	     InitGridInfoBox( hDlg );
	     return TRUE;

    case WM_COMMAND:
	 switch (wParam)
	    {
	     case IDD_HELP:
	       if( HtmlHelp( hDlg, "quikgrid.chm::html/gridinfo.htm", HELP_CONTEXT, GridInfo ) == 0 )
			 //if (HtmlHelp(hDlg, "quikgrid.chm::html/gridinfo.htm", HELP_CONTEXT, 0L) == 0)
		    NotifyUser( IDS_HELP);
	       return TRUE;

	     case IDD_GETINFO:
	       FillGridInfoBox( hDlg );
          GetDlgItemText( hDlg, IDD_COMMENT, OldComment, 255 );
          GetDlgItemText( hDlg, IDD_GRIDVALUEZ, oldZvalue, 30 );
	       return TRUE;

	     case IDD_GETINFOINIT:
	       InitGridInfoBox( hDlg );
          return TRUE;

	     case IDOK:
	       if( PointFound >= 0 )
            {
            // Query ignore point button for points.
              int flags = ScatterData.flags(PointFound);
              PointIgnore = flags&1;
              ButtonCheck = 0;
              if( IsDlgButtonChecked( hDlg, IDD_IGNOREPOINT ) ) ButtonCheck = 1;
              if( PointIgnore+ButtonCheck == 1 )
              {
		         // Been a change of state.
		         if( ButtonCheck )
		              ScatterData.SetFlags( PointFound, (1|flags) );
		         else ScatterData.SetFlags( PointFound, (2&flags) );
               HWND hwnd = GetParent( hDlg );
		         PostMessage( hwnd, WM_COMMAND, IDM_REGENGRID, NULL );
		        } // end if( PointIgnore...
              flags = ScatterData.flags(PointFound);
		        if( IsDlgButtonChecked( hDlg, IDD_DISPLAYPOINT) )
		           ScatterData.SetFlags( PointFound, (2|flags) );
		        else
		          ScatterData.SetFlags( PointFound, (1&flags) );
	       } // end if (PointFound...
        GetDlgItemText( hDlg, IDD_COMMENT, NewComment, 255 );
	     if( strcmp(OldComment, NewComment) != 0 )
         {
           if( PointFound < 0 ) // It is a grid intersection being changed.
             NotifyUser( IDS_GRIDCOMMENTERROR );
           else
           {
		       ScatterData.SetComment( PointFound, NewComment );
             if( DrawingOptions.comments )
		       {
		        HWND hwnd = GetParent( hDlg );
		        PictureChanged( hwnd );
             } // end if( DrawingOptions.comments)
           } // end else if( PointFound <0 - grid intersection selected.
         }// end if( strcmp... new comment.

         GetDlgItemText( hDlg, IDD_GRIDVALUEZ, newZvalue, 30 ); // New z value? 
         if( strcmp(oldZvalue, newZvalue) != 0 )
         {
            if( GetDlgItemFloat( hDlg, IDD_GRIDVALUEZ, zValue)) return TRUE;
            zValue += zadjust; // adjust to be an internal number.
            if( PointFound < 0 ) // It is a grid intersection being changed.
             {
               if( zValue < Zgrid.zmin() ) zValue = Zgrid.zmin();
               Zgrid.zset(ix, iy, zValue ) ;
               PaintContourReset();
               RotateReset();
               HWND hwnd = GetParent( hDlg );
               PictureChanged( hwnd );
             }
            else // It is a scattered data point.
             {
               if( zValue < ScatterData.zMin() ) zValue = ScatterData.zMin(); 
               ScatterData.SetZ( PointFound, zValue-zadjust );
               HWND hwnd = GetParent( hDlg );
		         PostMessage( hwnd, WM_COMMAND, IDM_REGENGRID, NULL );
               PictureChanged( hwnd );
             }
         }
         DestroyWindow( hDlg);
	      hGridLocationDlgBox = NULL;
	      return TRUE;

	     case IDCANCEL:
	       DestroyWindow( hDlg);
	       hGridLocationDlgBox = NULL;
	       return TRUE;
	    }    //end switch( wParam...
	  break;

      case WM_CLOSE :
         DestroyWindow( hDlg );
         hGridLocationDlgBox = NULL;
         return TRUE;
     } // end switch(message...
  return FALSE;
 }

//********************************************************************
//     R i g h t   M o u s e  B u t t o n    D I A L O G    B O X
//**********************************************************************
BOOL FAR PASCAL RightMouseOptionsDlgProc( HWND hDlg, UINT message, WPARAM wParam,
			       LPARAM lParam)  
 {
  switch (message)
  {
    case WM_INITDIALOG :
	       SetDlgItemText( hDlg, IDD_RIGHTBUTTONBOXTITLE, RightMouseDialogTitle );
          CheckDlgButton( hDlg, IDD_SHOWCOMMENTSONLY, RightMouseDialogTerse );
	       return TRUE;

    case WM_COMMAND :
	  switch (wParam)
	    {
	     case IDHELP:
	        if( HtmlHelp( hDlg, "quikgrid.chm::html/1w_tidp.htm", HELP_CONTEXT, RightButtonOptions ) == 0 )
				//if (HtmlHelp(hDlg, "quikgrid.chm::html/1w_tidp.htm", HELP_CONTEXT, 0L) == 0)
		     NotifyUser( IDS_HELP);
	     return TRUE;

        case IDPREFERENCE:
           GetDlgItemText( hDlg, IDD_RIGHTBUTTONBOXTITLE, RightMouseDialogTitle, 200 );
           RightMouseDialogTerse = IsDlgButtonChecked( hDlg, IDD_SHOWCOMMENTSONLY );
           SaveRightButtonOptions();
           return TRUE;

	     case IDOK :
	       GetDlgItemText( hDlg, IDD_RIGHTBUTTONBOXTITLE, RightMouseDialogTitle, 200 );
          RightMouseDialogTerse = IsDlgButtonChecked( hDlg, IDD_SHOWCOMMENTSONLY );
	     case IDCANCEL :
	       EndDialog( hDlg, 0);
	       return TRUE;
	    }
	 break;
     }
  return FALSE;
 }

//**********************************************************************
//   G r i d   L o c a t i o n   T e r s e D I A L O G    B O X
//**********************************************************************
// Displays only x,y,z and comment for a data point location.
BOOL FAR PASCAL GridLocnTerseDlgProc( HWND hDlg, UINT message, WPARAM wParam,
			       LPARAM lParam)  
 {
  static long ix, iy;
  double xadjust, yadjust;
  switch (message)
  {
   case WM_INITDIALOG :
      SetWindowText( hDlg, RightMouseDialogTitle );
      SetDlgItemText( hDlg, IDD_COMMENT, ScatterData.comment(ix));
      SetDlgItemText( hDlg, IDD_GRIDVALUEX, " ");
      SetDlgItemText( hDlg, IDD_GRIDVALUEY," ");
      SetDlgItemText( hDlg, IDD_COMMENT, " ");
	   return TRUE;

   case WM_COMMAND :
	 switch (wParam)
	    {
	     case IDHELP:
           NotifyUser( "Help link not implemented yet." );
	        //if( WinHelp( hDlg, "quikgrid.hlp", HELP_CONTEXT, EditTitle ) == 0 )
		     //NotifyUser( IDS_HELP);
	     return TRUE;

        case IDD_GETINFO:
          GetxyInfo( ix, iy );
          LoadNormalization( xadjust, yadjust);
          if( iy < 0 )  // display information on data point.
           {
            SetDlgItemFloatXY( hDlg, IDD_GRIDVALUEX, ScatterData.x(ix)+xadjust);
            SetDlgItemFloatXY( hDlg, IDD_GRIDVALUEY, ScatterData.y(ix)+yadjust);
            SetDlgItemText( hDlg, IDD_COMMENT, ScatterData.comment(ix));
           }
          else SetDlgItemText( hDlg, IDD_COMMENT, "Selected point is not a data point");  
	       return TRUE;

	     case IDD_GETINFOINIT:
          SetDlgItemText( hDlg, IDD_GRIDVALUEX, " ");
          SetDlgItemText( hDlg, IDD_GRIDVALUEY," ");
          SetDlgItemText( hDlg, IDD_COMMENT, " ");
          return TRUE;

	     case IDOK :
	     case IDCANCEL :
          DestroyWindow( hDlg);
          hGridLocationDlgBox = NULL;
	       return TRUE;
	    }
	 break;

    case WM_CLOSE :
         DestroyWindow( hDlg );
         hGridLocationDlgBox = NULL;
         return TRUE;
     }
  return FALSE;
 }