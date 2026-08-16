//****************************************************************
//                      G r i d
// Routines to assist in grid management.
//                Copyright (c) 1993 - 2001 by John Coulthard
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
// Sept. 23/98: Remove code to limit zapgrid to 125x125 grid.
// Sept. 27/98: Removed obsoliet GridMethod code.
// Mar. 6/99: Added to InitializeGrid code to set zmax and min.
// May 25/99: Added function SetZUndefined (set something to undefined). 
// Jun 1/99: Added code to implement GridLock (Template concept).
// Jun 2/99: Coded routine to set undefined z values to something.
// Jun 4/99: Code to snap xy cordinates to grid value.
// Oct 25/99: Fix up Initialize Grid code so better behaved with GridLock.
// Mar. 12/01: Make changes to nice coordinates used for initial grid.
//****************************************************************
#include <windows.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#pragma hdrstop
#include "assert.h" 
#include "surfgrid.h"
#include "scatdata.h"
#include "xygrid.h"
#include "xpand.h" 
#include "quikgrid.h"
#include "paintcon.h"
#include "utilitys.h"
#include "rc.h"
#include "loaddata.h"
#include "rotate.h"
#include "condraw.h" 
using namespace std;
static float xmin,
				 xincrement,
             xmax,
				 ymin,
				 yincrement,
             ymax;

extern int GridLock,
           NiceRatio,
           NiceRatioMaxLines;

int NicexyCoordinates = 1;
//float DefaultZmax = 0.0, DefaultZmin = 0.0;


//************************************************************
//                 A u t o   G r i d   S i z e
//   Generate "nice" dimensions for the grid to be generated
//   based on the number of scattered input points.
//************************************************************
// The routine calling this *must* follow through and generate
// a new grid - as the storage space for the grid has been re-allocated.
// Internal service routine
static void AutoGridSize()
{
  static int NiceSize;
  if( GridLock ) return;
  NiceSize = sqrt((float) ScatterData.Size()*(float)NiceRatio);
  NiceSize = max( 10, NiceSize);
  NiceSize = min( NiceRatioMaxLines, NiceSize);
  Zgrid.New( NiceSize, NiceSize );
}

//********************************************************************
//                     Z A P Z G R I D
//  Routine to reset z grid coordinates to -999999 (Undefined)
//********************************************************************
void ZapZgrid()
{
 if( GridLock ) return;
 int XGridSize = Zgrid.xsize();
 int YGridSize = Zgrid.ysize();

 for( int i = 0; i<XGridSize; i++) // Fill grid...
	{ for( int j = 0; j<YGridSize; j++) Zgrid.zset(i,j, -99999.) ;}
}

//********************************************************************
//                     F I L L G R I D X Y
// Routine to fill in the xy grid coordinate locations for Zgrid. 
//********************************************************************
void FillGridXY(  )
{
 if( GridLock ) return;
 int XGridSize = Zgrid.xsize();
 int YGridSize = Zgrid.ysize();
 float yvalue = ymin;
 float xvalue = xmin;
 int i; 

   for( i = 0; i<YGridSize; i++)
     {  Zgrid.yset( i, yvalue );
        yvalue += yincrement; 
     }

   for( i = 0; i<XGridSize; i++)
     {  Zgrid.xset( i, xvalue );
		  xvalue += xincrement;
     }
}
//********************************************************************
//      S E T G R I D X Y
// Routine sets up new x y grid coordinate locations. 
//********************************************************************
void SetGridXY( float xstart, float xincr, float ystart, float yincr)
{
 if( GridLock ) return;
 xmin = xstart;
 xincrement = xincr;
 ymin = ystart;
 yincrement = yincr;
 ZapZgrid();   // Set z locations to -9999. 
 FillGridXY(); // And fill in new xy grid coordinates. 
}

//*******************************************************************
//         N I C E   X Y    C O O R D S
//*******************************************************************
// Try to pretty up the numbers for the grid.
static void NicexyCoords( float &xmin, float &xincr, float &ymin, float &yincr )
{
 static double dxnormalize, dynormalize;

 LoadNormalization(dxnormalize, dynormalize);
 xincr = AtPrecision( xincr, 1 );
 yincr = AtPrecision( yincr, 1 );
 xmin = double(int(((double)xmin+(double)dxnormalize)/(double)xincr)) * (double)xincr - (double)dxnormalize;
 ymin = double(int(((double)ymin+(double)dynormalize)/(double)yincr)) * (double)yincr - (double)dynormalize;
}

//*******************************************************************
//         D E F A U L T X Y C O O R D S
//*******************************************************************
void DefaultxyCoords()
{
 static float xRange, yRange;
 static int nx, ny;

 if( GridLock ) return;

 xmin = ScatterData.xMin();
 ymin = ScatterData.yMin();
 xmax = ScatterData.xMax();
 ymax = ScatterData.yMax();


 xRange = xmax - xmin;
 yRange = ymax - ymin;

 if( (xRange==0.0) || (yRange==0.0) )
      { NotifyUser( IDS_DATANORANGE ); return; }  

 xincrement = xRange/(Zgrid.xsize()-1);
 yincrement = yRange/(Zgrid.ysize()-1);
  
 if( LatLonData ) return; // Don't try nice coordinates if lat/lon data.
 if( !NicexyCoordinates ) return;

 // Set up really nice looking coordinates (if possible).

 NicexyCoords( xmin, xincrement, ymin, yincrement );

// Now see if nice coords still define the whole grid & if not adjust.

 if( xmin > ScatterData.xMin() ) xmin -= xincrement;
 if( ymin > ScatterData.yMin() ) ymin -= yincrement;

 xRange = xmax - xmin;      // New xmin that is.
 yRange = ymax - ymin;

 nx = xRange/xincrement + 1;
 ny = yRange/yincrement + 1;

 if( (xmin + xincrement*(nx-1)) < xmax ) nx += 1;
 if( (ymin + yincrement*(ny-1)) < ymax ) ny += 1;

 if( (nx > Zgrid.xsize()) && (nx < Zgrid.MaximumXY())
  && (ny > Zgrid.ysize()) && (ny < Zgrid.MaximumXY()) );
       Zgrid.New( nx, ny );

 }

//*******************************************************************
//                  I n i t i a l i z e   G r i d
//*******************************************************************
int InitializeGrid( HWND &hwnd )
{
 static long NumberOfPoints;
// The size of the grid has been determined by now. 

 XpandUndefinedZ( -99999. ) ;

 SetWaitCursor();

 AutoGridSize();
          // Create a grid of reasonable size.
 if( !GridLock)
  {
     DefaultxyCoords();   // Calculate default x and y coordinate positions
     FillGridXY( );      // And fill the grid x y coordinates with the defaults.
   }

 ZapZgrid();               // Set all z coordinates to -99999.

 NumberOfPoints = ScatterData.Size();
 if( NumberOfPoints < 3 )
 {
    NotifyUser( IDS_TOOFEWPOINTS );
    SetStartupState( hwnd );
    return 0;
 }

  Zgrid.Znew(); 
 Zgrid.zset(0,0, ScatterData.zMax() );   // Initialize max and min. for grid.
 Zgrid.zset(0,0, ScatterData.zMin() );   // (Will be replaced during grid generation).

 NumberOfContours = 10; // re-initialize number of contours.
 NiceContourLinesSet();
 ContourBoldLabelSet( PenHighLite );
 PaintContourReset();
 RotateReset();

 RestoreCursor();
 return 1; 
}
//***************************************************************************
//          S e t  Z  U n d e f i n e d
//***************************************************************************
// This little utility will scan the grid for "value" and set the intersections
// to undefined. WITH THE EXCEPTION: "value" intersections adjacent to
// intersections that are not undefined or are not set to "value" are left
// alone. In effect intersections of "value" on the border to other values
// are not changed.

void SetZUndefined( double value )
{
 static int XGridSize, YGridSize, i, j, ii, jj, DoIt;

 XGridSize = Zgrid.xsize();
 YGridSize = Zgrid.ysize();

 for( i = 0; i<XGridSize; i++)
 for( j = 0; j<YGridSize; j++)
   {
     if( Zgrid.z(i,j) == value )
     {
       DoIt = TRUE;
       for( ii = i-1; ii <= i+1; ii++ )
       for( jj = j-1; jj <= j+1; jj++ )
        {
          if( ii < 0 || jj < 0 || ii >= XGridSize || jj >= YGridSize) continue;
          if( Zgrid.z(ii,jj ) == value || Zgrid.z(ii,jj) < 0.0 ) continue;
          DoIt = FALSE;
        }
       if( DoIt ) Zgrid.zset(i,j, -99999.) ;
     }
   }
}

//************************************************************************
//    S e t  U n d e f i n e d   T o 
//************************************************************************
void SetUndefinedTo( double value )
{
 static int XGridSize, YGridSize, i, j;

 XGridSize = Zgrid.xsize();
 YGridSize = Zgrid.ysize();

 for( i = 0; i<XGridSize; i++)
 for( j = 0; j<YGridSize; j++)
   {
     if( Zgrid.z(i,j) < 0.0 ) Zgrid.zset(i, j, value );
   }
}

//*****************************************************************
//        M a p X Y to Z
//*****************************************************************
// returns z value on closest grid intersection to (x,y)
// Cannot return undefined so return midpoint z value.
static float MapXYtoZ( float x, float y)
{
 static float incr, zmid, result;
 static int xSize, ySize, IX, IY;
 static const float round = .5;
 //static char szText[256];
 
 xSize = Zgrid.xsize();
 ySize = Zgrid.ysize();
 zmid = (Zgrid.zmax() + Zgrid.zmin())*.5;

 incr = Zgrid.x(1)-Zgrid.x(0);
 IX = (x - Zgrid.x(0))/incr + round;
 if( IX < 0 ) IX = 0;
 if( IX >= xSize ) IX = xSize-1;

 incr = Zgrid.y(1)-Zgrid.y(0);
 IY = (y - Zgrid.y(0))/incr + round;
 if( IY < 0 ) IY = 0;
 if( IY >= ySize ) IY = ySize-1;

 //sprintf( szText, " %g, %g, maps to grid location %i, %i ", x, y, IX, IY);
 //NotifyUser( szText);

 result = Zgrid.z(IX,IY);
 if( result < 0.0 ) result = zmid;
 return result; 
}
//*****************************************************************
//       S n a p   X Y Z  D a t a
//*****************************************************************
// Routine to set scattered data point z values to grid values.
void SnapXYZdata()
{
  static long NumberOfPoints;
  static int i;
  static float x, y, z, zadjust;

 NumberOfPoints = ScatterData.Size();
 if( NumberOfPoints < 3 ) return; // There are no data points.
 SetWaitCursor();
 zadjust = ScatterData.zAdjust();
 for( i = 0; i < NumberOfPoints; i++ )
 {
   x = ScatterData.x(i);
   y = ScatterData.y(i);
   z = MapXYtoZ( x, y ) ;
   ScatterData.SetZ( i , z-zadjust );
 }
 RestoreCursor();
}
//*****************************************************************
//       S e t   G r i d   E d g e  Z
//*****************************************************************
void SetGridEdgeZ( float value )
{
 static int i, xSize, ySize;
 xSize = Zgrid.xsize();
 ySize = Zgrid.ysize();
 for( i = 0; i < xSize; i++){ Zgrid.zset(i, 0, value );
                              Zgrid.zset(i, ySize-1, value ); }
 for( i = 0; i < ySize; i++){ Zgrid.zset(0, i, value );
                              Zgrid.zset(xSize-1, i, value ); }
}
//*****************************************************************
//       I n i t i a l i z e   Z g r i d
//*****************************************************************
// Routine sets Zgrid back to basic state
// will be place to add default max and min.
void InitializeZgrid( )
{
  Zgrid.Reset();
  //if( (DefaultZmax == 0.0) && (DefaultZmin == 0.0) ) return;
  //Zgrid.zset(0,0, DefaultZmax );   // Initialize max and min. for grid.
  //Zgrid.zset(0,0, DefaultZmin );
}
