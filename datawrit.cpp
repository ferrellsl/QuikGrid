//***********************************************************
//                  D A T A W R I T
//  Save the scattered data points in XYZ - Comment format.
//                Copyright (c) 1999 by John Coulthard
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
// Jan. 28/99: Initial implementation. (Adapted from GridWrit).
// Feb. 1/99:  Handle both metric and lat/lon data.
// Jun 1/99: Change to use format conversion routines for lat/lon.
// Jan 9/02: Use double precision for metric output.
//***********************************************************

#include <windows.h> 
#include <iostream>
#include <iomanip>
#include <fstream>
#include <strstream>
#include <math.h>
#include <string.h>
#include <commdlg.h>
#include <time.h>
#pragma hdrstop
#include "surfgrid.h"
#include "scatdata.h"
#include "xygrid.h"
#include "utilitys.h"
#include "rc.h"
#include "quikgrid.h"
#include "paintcon.h"
#include "erwrite.h"
#include "assert.h"
#include "loaddata.h"
using namespace std;
extern char SeparatorCharacter;

//******************************************************************
//         A b o r t   W r i t e   F i l e
//******************************************************************
static void AbortWriteFile( char FileName[] )
{
  ostrstream Buf;

  Buf << "Error writing data points to the file "
	   << FileName 
           << ". Output terminated. " << ends; 

  char *szBuf = Buf.str();
  NotifyUser( szBuf );
  delete szBuf;

  RestoreCursor();

}
//******************************************************************
//                    S a v e   D a t a   P o i n t s
//******************************************************************
int OutputDataPointFile( HWND &hwnd )
{
 char FileName[256];

 if( !GetSaveFile( FileName, hwnd, "Save data points to file" ) ) return 0;

  ofstream outFile( FileName, ios::out );
  if( !outFile ) 
    { NotifyUser( IDS_NOOUTPUTFILE );
      return 0;
    }

 static int i, NumberOfDataPoints;
 static double xadjust, yadjust, zadjust;

 NumberOfDataPoints = ScatterData.Size();
 LoadNormalization( xadjust, yadjust);
 zadjust = ScatterData.zAdjust();

 SetWaitCursor();
 outFile << setprecision(12);

 for( i = 0; i < NumberOfDataPoints; i++ )
  {
   if( !(ScatterData.flags(i)&1) )  // Don't write out ignored points.
   {
     if( LatLonData )
     {
       outFile << FormatLat( ScatterData.y(i)+yadjust, FALSE )
               << SeparatorCharacter
               << FormatLon( ScatterData.x(i)+xadjust, FALSE )
               << SeparatorCharacter ;
      }
      else
      {
		 outFile << (ScatterData.x(i)+xadjust) << SeparatorCharacter
					<< (ScatterData.y(i)+yadjust) << SeparatorCharacter;
      }
	  outFile << ( ScatterData.z(i)-zadjust ) << SeparatorCharacter ;
     outFile << ScatterData.comment(i); 
     outFile << endl;
     if( outFile.fail() ){ AbortWriteFile( FileName );
                           outFile.close();
			                  return 0; }
   }
  }

 outFile.close();
 RestoreCursor();
 return 1;
}

