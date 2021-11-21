//*******************************************************************
//       S t a t i s t i c s    D i a l o g    B o x
//
//                   Copyright (c) 1998 - 2000 by John Coulthard
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
// Nov. 8/98: Created.
// Jan. 29/00: Added recognition for speed and direction grids.
// Feb. 2/00: Added code to display the outline statistics.
// Feb. 4/00: Set the precision to 5.
// Jun. 7/00: Show gridlock status.
// Jun 18/00: Show z values for Speed and Direction grids. 
//*******************************************************************
#include <windows.h>
#include <iostream>
#include <strstream>
#include <iomanip>
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
using namespace std;
extern SurfaceGrid Zgrid;
extern ScatData OutLine; 
// Sgrid defines a wind or current speed grid.
// Dgrid defines the wind or current direction.
extern SurfaceGrid Sgrid, Dgrid, Cgrid;
extern float CgridZadjust;
extern double VolumeDifference;
extern int   DisplayVolumeDifference;
extern HWND hGridLocationDlgBox;
extern int Registered, GridGeneratedOK;
extern UINT PaintState;
extern ContourOptionsType DrawingOptions;
extern int GridLock;

static double xadjust, yadjust, zadjust;

//**********************************************************************
//    D i s p l a y    S t a t i s t i c s   D i a l o g   B o x
//**********************************************************************
BOOL FAR PASCAL StatisticsDlgProc( HWND hDlg, UINT message, WPARAM wParam,
					 LPARAM lParam)
{
//  HWND hwnd;
  int NumberOfDataPoints;

  switch (message)
  {
   case WM_INITDIALOG :
       {
        NumberOfDataPoints = ScatterData.Size();
        ostrstream Buf;
        Buf << setprecision(5);

        LoadNormalization( xadjust, yadjust);
        zadjust = ScatterData.zAdjust();

        if( NumberOfDataPoints < 3 )
          Buf << "No scattered data points are loaded. \r\n" ;
          else
          {

            Buf << NumberOfDataPoints << " Scattered data points are loaded. ";
            if( LatLonData ) Buf << "\r\nThey represent latitude and longtitude " ;

            Buf << "\r\nIn X" ;
              if( LatLonData ) Buf << "(longtitude)";
            Buf << " they range from " <<
              FormatCoord( ScatterData.xMin()+xadjust) << " to ";
            Buf <<  FormatCoord( ScatterData.xMax()+xadjust) <<  "."
              << "\r\nIn Y";
              if( LatLonData ) Buf << "(latitude)";
            Buf << " they range from " ;
            Buf <<  FormatCoord( ScatterData.yMin()+yadjust) << " to " ;
            Buf <<  FormatCoord( ScatterData.yMax()+yadjust) << "."
              << "\r\nIn the Z direction they range from "
              << (ScatterData.zMin()-zadjust)
              << " to " << (ScatterData.zMax()-zadjust) << ".\r\n" ;
          }
        if( GridGeneratedOK )
          {
            Buf << "\r\nA grid of size "
              << Zgrid.xsize() << " by "  <<  Zgrid.ysize()
              << " is defined. \r\nIn the X direction it goes from "
              << FormatCoord(Zgrid.xmin()+xadjust) << " to " ;
            Buf << FormatCoord(Zgrid.xmax()+xadjust) ;
            Buf  << " in steps of " << FormatCoord(Zgrid.x(1)-Zgrid.x(0))
              << ". \r\nIn the Y direction it goes from " ;
            Buf  << FormatCoord(Zgrid.ymin()+yadjust) << " to " ;
            Buf  << FormatCoord(Zgrid.ymax()+yadjust);
            Buf  << " in steps of " << FormatCoord(Zgrid.y(1)-Zgrid.y(0))
              << ". \r\nIn the Z direction the data goes from "
              << (Zgrid.zmin()-zadjust) << " to "
              << (Zgrid.zmax()-zadjust) << ".\r\n" ;
                        }
          else
          Buf << "\r\nThere is no grid. \r\n" ;

       if( GridLock&&GridGeneratedOK ) Buf << "\r\nThe grid is locked as a template.\r\n " ;
       if( GridLock&&(!GridGeneratedOK) )
           Buf << "\r\nGrid coordinates are defined and locked as a template.\r\n " ;

       NumberOfDataPoints = OutLine.Size();
       if( NumberOfDataPoints < 3 )
          Buf << "\r\nNo outline data is loaded. \r\n" ;
          else
          {
            Buf << "\r\n" << NumberOfDataPoints << " Outline data points are loaded. ";
            Buf << "\r\nIn X" ;
             if( LatLonData ) Buf << "(longtitude)";
            Buf << " they range from " <<
              FormatCoord( OutLine.xMin()+xadjust) << " to ";
            Buf <<  FormatCoord( OutLine.xMax()+xadjust) <<  "."
              << "\r\nIn Y";
              if( LatLonData ) Buf << "(latitude)";
            Buf << " they range from " ;
            Buf <<  FormatCoord( OutLine.yMin()+yadjust) << " to " ;
            Buf <<  FormatCoord( OutLine.yMax()+yadjust) << "."
              << "\r\nIn the Z direction they range from "
              << (OutLine.zMin()-zadjust)
              << " to " << (OutLine.zMax()-zadjust) << ".\r\n" ;
          }

       if( Sgrid.xsize() > 2 )
          Buf << "\r\nA speed/magnitude grid is defined."
              << "\r\nThe values ranges from " << Sgrid.zmin()
              << " to " << Sgrid.zmax() << "\r\n" ;

       if( Dgrid.xsize() > 2 )
          Buf << "\r\nA direction grid is defined."
              << "\r\n\The direction ranges between "
              << Dgrid.zmin() << " and " << Dgrid.zmax()<< "\r\n";


       if( Cgrid.xsize() > 2 )
          Buf << "\r\nA colour grid is defined."
              << "\r\n\The z value ranges between "
              << (Cgrid.zmin()-CgridZadjust) << " and "
              << (Cgrid.zmax()-CgridZadjust) <<"\r\n";

       Buf << "\r\nThe normalization constants are x: " << xadjust << " y: " << yadjust;
       Buf << ends ;
       char *szBuf = Buf.str();
       SetDlgItemText( hDlg, IDD_STATISTICS, szBuf);
       delete szBuf;
		 return TRUE;
       }

	 case WM_COMMAND :
		 switch (wParam)
	    {
		  case IDOK :

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

