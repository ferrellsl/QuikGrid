//*******************************************************************
//       D i a l o g  B o x  G r i d   L i n e   F u n c t i o n s 
//
//                   Copyright (c) 1994 - 1999 by John Coulthard
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
// October 10/96 - Call to ZapZgrid moved to QUIKGRID.CPP
// Feb. 8/99: Deleted Calculate Incro button in NewGridLines.
//*******************************************************************
#include <windows.h>
#include <HtmlHelp.h>
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
#include "grid.h"

extern SurfaceGrid Zgrid;
extern HWND hGridLocationDlgBox;
extern int Registered, GridGeneratedOK;
extern UINT PaintState;
extern ContourOptionsType DrawingOptions;

//**********************************************************************
//    N u m b e r  O f  G r i d   L i n e s   D I A L O G    B O X
//**********************************************************************
BOOL FAR PASCAL NumberGridLinesDlgProc( HWND hDlg, UINT message, WPARAM wParam,
					 LPARAM lParam)
 {
  long   MaxXY, xLinesNew, yLinesNew;
  HWND hwnd;

  switch (message)
  {
	 case WM_INITDIALOG :
		 SetDlgItemInt( hDlg, IDD_XGRIDLINES, Zgrid.xsize(), FALSE );
		 SetDlgItemInt( hDlg, IDD_YGRIDLINES, Zgrid.ysize(), FALSE );
		 return TRUE;

	 case WM_COMMAND :
		 switch (wParam)
	    {
        case IDD_HELP:
	     if( HtmlHelp( hDlg, "quikgrid.chm::html/fx3bh7.htm", HELP_CONTEXT, NumberGridLines ) == 0 )
			 //if (HtmlHelp(hDlg, "quikgrid.chm::html/fx3bh7.htm", HELP_CONTEXT, 0L) == 0)
		     NotifyUser( IDS_HELP);
	     return TRUE;

		  case IDOK :
			 xLinesNew = GetDlgItemInt( hDlg, IDD_XGRIDLINES, NULL, FALSE);
	       yLinesNew = GetDlgItemInt( hDlg, IDD_YGRIDLINES, NULL, FALSE);
	       if( xLinesNew < 3 || yLinesNew < 3 )
				{ NotifyUser( IDS_GRIDLINESLT3 ); return TRUE; }
	       MaxXY = Zgrid.MaximumXY();
	       if( xLinesNew > MaxXY || yLinesNew >MaxXY )
				{ NotifyUser(IDS_GRIDTOOLARGE); return TRUE; }
			 Zgrid.New( xLinesNew, yLinesNew);
          ZapZgrid();
			 DefaultxyCoords();
			 FillGridXY();

          GridGeneratedOK = FALSE;
			 hwnd = GetParent( hDlg );
			 PictureChanged( hwnd );
			 PostMessage( hwnd, WM_COMMAND, IDM_REGENGRID, NULL );
			 EndDialog( hDlg, 0);
			 return 0;

		  case IDCANCEL :
			  EndDialog( hDlg, 0);
			  return TRUE;
	    }
		 break;
     }
  return FALSE;
 }
//**********************************************************************
//          N e w G r i d L i n e s   D I A L O G    B O X
//**********************************************************************
    BOOL FAR PASCAL NewGridDlgProc( HWND hDlg, UINT message, WPARAM wParam,
			       LPARAM lParam)  
 {
  static double xadjust, yadjust;
  long   MaxXY, xLinesNew, yLinesNew, ReturnCode;
  double xstart, xincr, xend,
			ystart, yincr, yend;
  float xmin, xmax, ymin, ymax;

  switch (message)
	  {
		case WM_INITDIALOG:
			SetDlgItemInt( hDlg, IDD_XGRIDLINES, Zgrid.xsize(), FALSE );
			SetDlgItemInt( hDlg, IDD_YGRIDLINES, Zgrid.ysize(), FALSE );
			LoadNormalization( xadjust, yadjust);
         SetDlgItemFloatXY( hDlg, IDD_XRANGE,
									 ScatterData.xMax()-ScatterData.xMin() );
         SetDlgItemFloatXY( hDlg, IDD_YRANGE,
	                         ScatterData.yMax()-ScatterData.yMin() );
         SetDlgItemFloatXY( hDlg, IDD_XINCREMENT, Zgrid.x(1)-Zgrid.x(0) );
         SetDlgItemFloatXY( hDlg, IDD_YINCREMENT, Zgrid.y(1)-Zgrid.y(0) );
         if( LatLonData)
         {
          SetDlgItemText( hDlg, IDD_XMIN, FormatLon(ScatterData.xMin()+xadjust) );
          SetDlgItemText( hDlg, IDD_XMAX, FormatLon(ScatterData.xMax()+xadjust) );
			 SetDlgItemText( hDlg, IDD_YMIN, FormatLat(ScatterData.yMin()+yadjust) );
			 SetDlgItemText( hDlg, IDD_YMAX, FormatLat(ScatterData.yMax()+yadjust) );
          SetDlgItemText( hDlg, IDD_XSTART, FormatLon(Zgrid.x(0)+xadjust) );
          SetDlgItemText( hDlg, IDD_YSTART, FormatLat(Zgrid.y(0)+yadjust) );
			 SetDlgItemText( hDlg, IDD_XEND, FormatLon(Zgrid.x(Zgrid.xsize()-1)+xadjust) );
			 SetDlgItemText( hDlg, IDD_YEND, FormatLat(Zgrid.y(Zgrid.ysize()-1)+yadjust) );
         }
         else
         {
			 SetDlgItemFloatXY( hDlg, IDD_XMIN, ScatterData.xMin()+xadjust);
			 SetDlgItemFloatXY( hDlg, IDD_XMAX, ScatterData.xMax()+xadjust);
			 SetDlgItemFloatXY( hDlg, IDD_YMIN, ScatterData.yMin()+yadjust);
			 SetDlgItemFloatXY( hDlg, IDD_YMAX, ScatterData.yMax()+yadjust);
          SetDlgItemFloatXY( hDlg, IDD_XSTART, Zgrid.x(0)+xadjust );
          SetDlgItemFloatXY( hDlg, IDD_YSTART, Zgrid.y(0)+yadjust );
			 SetDlgItemFloatXY( hDlg, IDD_XEND, Zgrid.x(Zgrid.xsize()-1)+xadjust );
			 SetDlgItemFloatXY( hDlg, IDD_YEND, Zgrid.y(Zgrid.ysize()-1)+yadjust );
         }
	 return TRUE;

      case WM_COMMAND :
	 switch (wParam)
	    {
	     case IDD_HELP:
	     if( HtmlHelp( hDlg, "quikgrid.chm::html/kb32tk.htm", HELP_CONTEXT, EditGridDimensions ) == 0 )
			 //if (HtmlHelp(hDlg, "quikgrid.chm::html/kb32tk.htm", HELP_CONTEXT, 0L) == 0)
		     NotifyUser( IDS_HELP);
	     return TRUE;

	     case IDD_CALCULATEGRID:
	        if( PaintState != IDM_2DSURFACE )
			{ NotifyUser(
				 " This calculation is only available in 2D Contour view mode.");
																					 return TRUE;  }
			 GetZoomedUser( xmin, xmax, ymin, ymax );
	       xLinesNew = GetDlgItemInt( hDlg, IDD_XGRIDLINES, NULL, FALSE);
	       yLinesNew = GetDlgItemInt( hDlg, IDD_YGRIDLINES, NULL, FALSE);
	       if( xLinesNew < 3 || yLinesNew < 3 )
										{ NotifyUser( IDS_GRIDLINESLT3 ); return TRUE; }
	       MaxXY = Zgrid.MaximumXY();
	       if( xLinesNew > MaxXY || yLinesNew >MaxXY )
										 { NotifyUser(IDS_GRIDTOOLARGE);  return TRUE; }
			 xincr = ((double)xmax-(double)xmin)/(xLinesNew-1);
			 yincr = ((double)ymax-(double)ymin)/(yLinesNew-1);
	       LoadNormalization( xadjust, yadjust);
          SetDlgItemFloatXY( hDlg, IDD_XINCREMENT, xincr );
	       SetDlgItemFloatXY( hDlg, IDD_YINCREMENT, yincr );
          if( LatLonData)
          {
            SetDlgItemText( hDlg, IDD_XSTART, FormatLon(xmin+xadjust) );
			   SetDlgItemText( hDlg, IDD_YSTART, FormatLat(ymin+yadjust) );
			   SetDlgItemText( hDlg, IDD_XEND, FormatLon(xmax+xadjust) );
	         SetDlgItemText( hDlg, IDD_YEND, FormatLat(ymax+yadjust) );
          }
          else
          {
	         SetDlgItemFloatXY( hDlg, IDD_XSTART, xmin+xadjust );
			   SetDlgItemFloatXY( hDlg, IDD_YSTART, ymin+yadjust );
			   SetDlgItemFloatXY( hDlg, IDD_XEND, xmax+xadjust );
	         SetDlgItemFloatXY( hDlg, IDD_YEND, ymax+yadjust );
          }
	     return TRUE;

	     case IDD_CALCULATERESET:
	       xmin = ScatterData.xMin();
	       xmax   = ScatterData.xMax();
	       ymin = ScatterData.yMin();
	       ymax   = ScatterData.yMax();
	       xLinesNew = GetDlgItemInt( hDlg, IDD_XGRIDLINES, NULL, FALSE);
	       yLinesNew = GetDlgItemInt( hDlg, IDD_YGRIDLINES, NULL, FALSE);
	       if( xLinesNew < 3 || yLinesNew < 3 )
		       { NotifyUser( IDS_GRIDLINESLT3 ); return TRUE; }
	       MaxXY = Zgrid.MaximumXY();
	       if( xLinesNew > MaxXY || yLinesNew >MaxXY )
		      { NotifyUser(IDS_GRIDTOOLARGE); return TRUE; }
	       xincr = (xmax-xmin)/(xLinesNew-1);
	       yincr = (ymax-ymin)/(yLinesNew-1);
	       LoadNormalization( xadjust, yadjust);
          SetDlgItemFloatXY( hDlg, IDD_XINCREMENT, xincr );
	       SetDlgItemFloatXY( hDlg, IDD_YINCREMENT, yincr );
          if( LatLonData)
          {
            SetDlgItemText( hDlg, IDD_XSTART, FormatLon(xmin+xadjust) );
            SetDlgItemText( hDlg, IDD_YSTART, FormatLat(ymin+yadjust) );
	         SetDlgItemText( hDlg, IDD_XEND, FormatLon(xmax+xadjust) );
	         SetDlgItemText( hDlg, IDD_YEND, FormatLat(ymax+yadjust) );
          }
          else
          {
	         SetDlgItemFloatXY( hDlg, IDD_XSTART, xmin+xadjust );
            SetDlgItemFloatXY( hDlg, IDD_YSTART, ymin+yadjust );
	         SetDlgItemFloatXY( hDlg, IDD_XEND, xmax+xadjust );
	         SetDlgItemFloatXY( hDlg, IDD_YEND, ymax+yadjust );
          }
	     return TRUE;

	     case IDD_CALCULATELINE:
          ReturnCode = GetDlgItemFloatXY( hDlg, IDD_XINCREMENT, xincr);
          ReturnCode += GetDlgItemFloatXY( hDlg, IDD_YINCREMENT, yincr);
          ReturnCode += GetDlgItemFloatXY( hDlg, IDD_YSTART, ystart);
	       ReturnCode += GetDlgItemFloatXY( hDlg, IDD_YEND, yend);
          if( LatLonData )
          {
           ReturnCode +=  GetDlgItemLongtitude( hDlg, IDD_XSTART, xstart);
	        ReturnCode += GetDlgItemLongtitude( hDlg, IDD_XEND, xend);
          }
          else
          {
	        ReturnCode +=  GetDlgItemFloatXY( hDlg, IDD_XSTART, xstart);
	        ReturnCode += GetDlgItemFloatXY( hDlg, IDD_XEND, xend);
          }

	       if( ReturnCode != 0 ) return TRUE;
	       xLinesNew = ((xend-xstart)/xincr)+1.5; // may overflow.  
	       yLinesNew = ((yend-ystart)/yincr)+1.5;
	       SetDlgItemInt( hDlg, IDD_XGRIDLINES, xLinesNew, FALSE );
	       SetDlgItemInt( hDlg, IDD_YGRIDLINES, yLinesNew, FALSE );
	       if( xLinesNew < 3 || yLinesNew < 3 )
				{ NotifyUser( IDS_GRIDLINESLT3 ); return TRUE; }
	       MaxXY = Zgrid.MaximumXY();
	       if( xLinesNew > MaxXY || yLinesNew >MaxXY )
				{ NotifyUser(IDS_GRIDTOOLARGE); return TRUE; }
	       xend = xstart +(double)(xLinesNew-1)*xincr;
	       yend = ystart +(double)(yLinesNew-1)*yincr;
          if( LatLonData)
          {
           SetDlgItemText( hDlg, IDD_XEND, FormatLon(xend));
	        SetDlgItemText( hDlg, IDD_YEND, FormatLat(yend));
          }
          else
          {
	        SetDlgItemFloatXY( hDlg, IDD_XEND, xend);
	        SetDlgItemFloatXY( hDlg, IDD_YEND, yend);
          }
	     return TRUE;

	     case IDOK :
		  {
          MaxXY = Zgrid.MaximumXY();
	       if( ScatterData.Size() < 3 )
				 { NotifyUser( IDS_NOINPUTDATA );
					EndDialog( hDlg, 0); return TRUE; }
	       ReturnCode =  GetDlgItemFloatXY( hDlg, IDD_XINCREMENT, xincr);
          ReturnCode += GetDlgItemFloatXY( hDlg, IDD_YINCREMENT, yincr);
          ReturnCode += GetDlgItemFloatXY( hDlg, IDD_YSTART, ystart);
	       ReturnCode += GetDlgItemFloatXY( hDlg, IDD_YEND, yend);
          if( LatLonData )
          {
           ReturnCode += GetDlgItemLongtitude( hDlg, IDD_XSTART, xstart);
	        ReturnCode += GetDlgItemLongtitude( hDlg, IDD_XEND, xend);
          }
          else
          {
	        ReturnCode +=  GetDlgItemFloatXY( hDlg, IDD_XSTART, xstart);
	        ReturnCode += GetDlgItemFloatXY( hDlg, IDD_XEND, xend);
          }

	       if( ReturnCode != 0 ) return TRUE;
	       if( (xincr <= 0) || (yincr <= 0) )
				{ NotifyUser( IDS_GRIDINCRNEGATIVE );
				  return TRUE;
				}
	       LoadNormalization( xadjust, yadjust);

	       xLinesNew = ((xend-xstart)/xincr)+1.5; 
	       yLinesNew = ((yend-ystart)/yincr)+1.5; 

	       SetDlgItemInt( hDlg, IDD_XGRIDLINES, xLinesNew, FALSE );
	       SetDlgItemInt( hDlg, IDD_YGRIDLINES, yLinesNew, FALSE );

	       if( xLinesNew < 3 || yLinesNew < 3 )
				{ NotifyUser( IDS_GRIDLINESLT3 ); return TRUE; }
	       if( xLinesNew > MaxXY || yLinesNew >MaxXY )
				 { NotifyUser(IDS_GRIDTOOLARGE); return TRUE; }

			 Zgrid.New( xLinesNew, yLinesNew);

			 SetGridXY( xstart-xadjust, xincr, ystart-yadjust, yincr);

			 GridGeneratedOK = FALSE;

			 HWND hwnd = GetParent( hDlg );
			 PictureChanged( hwnd );
			 PostMessage( hwnd, WM_COMMAND, IDM_REGENGRID, NULL );
			 EndDialog( hDlg, 0);
			 return TRUE;
        }

	     case IDCANCEL :
	       EndDialog( hDlg, 0);
	       return TRUE;
	    }
	 break;
     }
  return FALSE;
 }