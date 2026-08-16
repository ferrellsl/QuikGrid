//*******************************************************************
//       D i a l o g      B o x    U t i l i t y    F u n c t i o n s 
//
//                        Copyright (c) 1995 - 1999 by John Coulthard
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
// Sept. 19/96: Modified to use "strtod" to do floating point
//              conversions (Avoids need to use MS Service Pack 1).
// Sept. 23/98: Increased SetPrecision to 6 - part of conversion to
//              WIN32 - conversions handled differently.
// Jan. 28/99:  Alter Lat/Lon conversion to use mmddd.ddd format.
//              (LatLonData=TRUE means data is Lat/Lon).
//*******************************************************************
#include <windows.h>
#include <iostream>
#include <strstream>
#include <iomanip>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "utilitys.h"
#include "assert.h"
#include "rc.h"
using namespace std;
extern int LatLonData;

//**********************************************************************
//      R e p o r t  N O S  c o n v e r s i o n  F a i l
//**********************************************************************
static void ReportNOSConversionFail( char szText[255] )
{
   NotifyUser( IDS_LATLONERROR, szText );
}
//**********************************************************************
//    S e t D l g I t e m F l o a t X Y
//**********************************************************************
void SetDlgItemFloatXY( HWND &hDlg, UINT Item, const double &x )
// Output number as metric or Lat/Lon formatted data.
{
  static char szBuf[30];
  FormatCoord( szBuf, sizeof(szBuf), x ) ;
  SetDlgItemText( hDlg, Item, szBuf);
}

//**********************************************************************
//     S e t D l g I t e m F l o a t 
//**********************************************************************
void SetDlgItemFloat( HWND &hDlg, UINT Item, const double &x )
// Output Metric Floating point number only.
{
 ostrstream Buf;
 Buf << setprecision(6) << x << ends;
 char *szBuf = Buf.str();
 SetDlgItemText( hDlg, Item, szBuf);
 delete szBuf;
}
//**********************************************************************
//     S e t D l g I t e m L o n g  
//**********************************************************************
void SetDlgItemLong( HWND &hDlg, UINT Item, const long &i )
// Output long integer number only.
{
 ostrstream Buf;
 Buf << i << ends;  
 char *szBuf = Buf.str();
 SetDlgItemText( hDlg, Item, szBuf);
 delete szBuf;
}
//**********************************************************************
//    G e t D l g I t e m L o n g t i t u d e
//**********************************************************************
int GetDlgItemLongtitude( HWND &hDlg, UINT Item, double &result)
{
  char szText[255];

  GetDlgItemText( hDlg, Item, szText, sizeof( szText ) ) ;

  if( !LonToDouble( result, szText ) )
         { ReportNOSConversionFail( szText ); return 1; }
  return 0; 
}
//**********************************************************************
//    G e t D l g I t e m F l o a t X Y
//**********************************************************************
int GetDlgItemFloatXY( HWND &hDlg, UINT Item, double &result)
{
  char szText[255], *endptr;

  GetDlgItemText( hDlg, Item, szText, sizeof( szText ) ) ;

  if( LatLonData )
   {
    if( !LatToDouble( result, szText ) )
         { ReportNOSConversionFail( szText ); return 1; }
   }
  else
   {
	 result = strtod(szText, &endptr);
	 if( *endptr != NULL )
	  {
        NotifyUser( IDS_FLOATINGPOINTERROR, szText );
        result = 0;
		  return 1;
      }  // end if( *endptr.... 
    }  // end else
  return 0; 
}
//**********************************************************************
//    G e t D l g I t e m F l o a t 
//**********************************************************************
int GetDlgItemFloat( HWND &hDlg, UINT Item, double &result) 
{
  char szText[255], *endptr;

    GetDlgItemText( hDlg, Item, szText, 255 ) ;
	 result = strtod(szText, &endptr);
	 if( *endptr != NULL )
      {
        NotifyUser( IDS_FLOATINGPOINTERROR, szText );
        result = 0;
		  return 1;
      }
  return 0; 
 }