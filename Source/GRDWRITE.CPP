//***********************************************************
//                  G R D W R I T E
//  Save the grid in QuikGrid or XYZ  or Surfer GRD format.
//                Copyright (c) 1996 - 1999 by John Coulthard
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
// Jan. 7/97: Code extracted from loadata.cpp to this module.
// Mar. 5/99: Increased precision of output. x and y coordinates
//            bumped to 12 digits.
// Mar. 13/99: Change to not use scientific notation by default.
//             Put out more precision in XYZ output code.
// Jun 1.99: Change to output data as lat lon if that form.
// Oct. 12/99: Change to have calling routine supply filename.
// Apr. 24/01: Fix serious bug - Output's undefined nodes all the
//             time. Doesn't update x coordinate correctly.
// Dec/04: Implement output MapInfo vertical grid file.
// Apr. 3/05: Remove (comment out MapInfo code). 
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
char SeparatorCharacter = '\t';
int OutputUndefinedValues = FALSE;
double UndefinedValue = -99999.;

static double dxnormalize, dynormalize;
static float zadjust;

//******************************************************************
//         A b o r t   W r i t e  G r i d   F i l e
//******************************************************************
static void AbortWriteGridFile( char FileName[] )
{
  ostrstream Buf;

  Buf << "Error writing Grid output to the file "
	   << FileName
           << ". Output terminated. " << ends;

  char *szBuf = Buf.str();
  NotifyUser( szBuf );
  delete szBuf;

  RestoreCursor();

}
//******************************************************************
//   S a v e   G r i d   D a t a  - Q u i k G r i d  f o r m a t
//******************************************************************
int SaveGridData( char FileName[] )
{
  ofstream outFile( FileName, ios::out );
  if( !outFile )
    { NotifyUser( IDS_NOOUTPUTFILE );
      return 0;
    }

 LoadNormalization( dxnormalize, dynormalize );

 SetWaitCursor();
 int nx = Zgrid.xsize();
 int ny = Zgrid.ysize();
 int i;

 outFile << nx << " " << ny << " " <<  ScatterData.zAdjust();
 if( LatLonData ) outFile << " LatLonData ";
 outFile << endl;
 outFile << setprecision(12);

 for( i = 0; i < nx; i++ )
  {
	 outFile <<  ((double)Zgrid.x(i)+dxnormalize) << " " ;
    if( (i%10) == 9 ) outFile << endl;
    if( outFile.fail() ){ AbortWriteGridFile( FileName );
                          outFile.close();
			  return 0; }
  }
 outFile<< endl;

 for( i = 0; i < ny; i++ )
  {
	 outFile << ((double)Zgrid.y(i)+dynormalize) << " " ;
    if( (i%10) == 9 ) outFile << endl;
    if( outFile.fail() ){ AbortWriteGridFile( FileName );
                          outFile.close();
			  return 0; }
  }
 outFile << endl;
 outFile << setprecision(6);

 int n = 0;
 for( i = 0; i < nx; i++ )
  { for( int j = 0; j < ny; j++ )
     { outFile << Zgrid.z(i,j) << " " ;
       if( (n%10) == 9 ) outFile << endl;
       n++;
       if( outFile.fail() ){ AbortWriteGridFile( FileName );
                             outFile.close();
			     return 0; }
     }
  }

 outFile.close();
 RestoreCursor();
 return 1;
}

//******************************************************************
//   S a v e   G r i d   D a t a  - S u r f e r G R D  f o r m a t
//******************************************************************
int OutputGRDfile( char FileName[] )
{

  static double z, zraw;
  static int n, i, j, nx, ny;

  ofstream outFile( FileName, ios::out );
  if( !outFile )
    { NotifyUser( IDS_NOOUTPUTFILE );
      return 0;
    }

 LoadNormalization( dxnormalize, dynormalize );
 zadjust = ScatterData.zAdjust();

 SetWaitCursor();
 nx = Zgrid.xsize();
 ny = Zgrid.ysize();

 outFile << "DSAA" << endl;  // GRD ASCII file identifier

 outFile << nx << " " << ny << " " << endl;

 outFile << setprecision(12);

 outFile << ((double)Zgrid.x(0)+dxnormalize) << " " << ((double)Zgrid.x(nx-1)+dxnormalize) << endl;
 outFile << ((double)Zgrid.y(0)+dynormalize) << " " << ((double)Zgrid.y(ny-1)+dynormalize) << endl;
 outFile << ((double)Zgrid.zmin()-zadjust)   << " " << ((double)Zgrid.zmax()-zadjust)      << endl;


 for( j = 0; j < ny; j++ ) // Put out in rows of constant Y.
 {
   n = 0;
   for( i = 0; i < nx; i++ )
     {
       zraw = Zgrid.z(i,j);
       if( zraw >= 0.0 ) z = zraw-zadjust;
			  else z = UndefinedValue;
       outFile << z << " " ;
       if( (n%10) == 9 ) outFile << endl;
       n++;
       if( outFile.fail() ){ AbortWriteGridFile( FileName );
                             outFile.close();
			     return 0; }
     } // end for i (y row).

     outFile << endl;
     outFile << " " << endl;
  }

 outFile.close();
 RestoreCursor();
 return 1;
}
/* Comment out MapInfo output code - probably will never be used. 
//******************************************************************
//   S a v e   G r i d   D a t a  - M a p i n f o  f o r m a t
//******************************************************************
int OutputMapinfoFile( char FileName[] )
{

  static double z, zraw, cellsizex, cellsizey, cellsize;
  static int i, j, nx, ny;

  ofstream outFile( FileName, ios::out );
  if( !outFile )
    { NotifyUser( IDS_NOOUTPUTFILE );
      return 0;
    }

 LoadNormalization( dxnormalize, dynormalize );
 zadjust = ScatterData.zAdjust();

 SetWaitCursor();
 nx = Zgrid.xsize();
 ny = Zgrid.ysize();

 // For MapInfo output similar to Surfer GRD file but rows written in reverse. 
 // ncols
 // nrows
 // xllcenter
 // yllcenter
 // cellsize
 // NoData value = -99999

 // outFile << "DSAA" << endl;  // GRD ASCII file identifier not needed for MapInfo?

 outFile << "ncols         " << nx << endl
         << "nrows         " << ny << " " << endl;

 outFile << setprecision(12);

 cellsizex = Zgrid.x(1)-Zgrid.x(0);
 cellsizey = Zgrid.y(1)-Zgrid.y(0);
 cellsize = (cellsizex+cellsizey)*0.5;

// if( cellsizex != cellsizey)
 //NotifyUser( "Warning Grid cells not square. Average cellsize will be used. " );

 // want the centre of the lower left grid cell.
 outFile << "xllcorner     " << (((double)Zgrid.x(0)+dxnormalize) + (cellsizex*.5)) << endl;
 outFile << "yllcorner     " << (((double)Zgrid.y(0)+dynormalize) + (cellsizey*.5)) << endl; 
 outFile << "cellsize      " << cellsize << endl;  // cellsize

 outFile << "NODATA_value  -99999" << endl;

 for( j = ny-1; j >=0; j-- ) // Put out in rows of constant Y in reverse.
 {
//   n = 0;    // Use "n" for count to limit to 10 entries per text line.
//   Mapinfo wants one row on one line???? 
   for( i = 0; i < nx; i++ )
     {
       zraw = Zgrid.z(i,j);
       if( zraw >= 0.0 ) z = zraw-zadjust;
			  else z = UndefinedValue;
       outFile << z << " " ;
//       if( (n%10) == 9 ) outFile << endl;
//       n++;
       if( outFile.fail() ){ AbortWriteGridFile( FileName );
                             outFile.close();
			     return 0; }
     } // end for i (y row).

     outFile << endl;
     outFile << " " << endl;
  }

 outFile.close();
 RestoreCursor();
 return 1;
}
*/  // End comment out Mapinfo Output code.
//******************************************************************
//                   O u t p u t   X Y Z   f i l e
//******************************************************************
int OutputXYZfile( char FileName[] )
{
 static double x, y, z, zraw;
 static int nx, ny, i, j;

  ofstream outFile( FileName, ios::out );
  if( !outFile )
    { NotifyUser( IDS_NOOUTPUTFILE );
      return 0;
    }

 LoadNormalization( dxnormalize, dynormalize );
 zadjust = ScatterData.zAdjust();

 SetWaitCursor();
 nx = Zgrid.xsize();
 ny = Zgrid.ysize();

 for(  i = 0; i < nx; i++ )
  { for(  j = 0; j < ny; j++ )
	  {
			zraw = Zgrid.z(i,j);
			if( zraw >= 0.0 ) z = zraw-zadjust;
			  else z = UndefinedValue;
			if( zraw >= 0.0 || OutputUndefinedValues )
         {
          x = Zgrid.x(i)+dxnormalize;
			 y = Zgrid.y(j)+dynormalize;

			  if( LatLonData )
				{
				 outFile << FormatLat( y, FALSE ) << SeparatorCharacter
                   << FormatLon( x, FALSE ) << SeparatorCharacter
                   << setprecision(6)
					    << z
					    << endl ;
			   } // end if (LatLonData).
			  else
			  {
				 outFile << setprecision(12)
                   << x  << SeparatorCharacter
					    << y  << SeparatorCharacter
					    << setprecision(6)
					    << z
					    << endl ;
			  }  // end if( LatLonData else....
			} // end if( zraw >= 0.0...
         if( outFile.fail() ){ AbortWriteGridFile( FileName );
                               outFile.close();
			       return 0; }
	 }    // end for( int j...
  }   // end for( int i... 

 outFile.close();
 RestoreCursor(); 
 return 1;
}