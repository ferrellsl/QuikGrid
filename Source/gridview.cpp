//*****************************************************************
//               G e n  G r i d  O n   V i e w
//
// Generate new grid on current 2d view.
//                         Copyright 1999 - 2001 by John Coulthard
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
// Jan. 30/99: Implementation.
// May 28/99: Creating CropGridOnView.
// Jun 16/00: In copy to Dgrid or Sgrid adjust z values by zadjust.
//            Check no negative values get stored in Sgrid or Dgrid.
// Mar. 9/01: Fix bug in crop grid view - overflow end coordinates.
// Mar. 14/01: Change crop to crop scattered data points to the new
//             grid extents (not the windows extents). Change to
//             allow crop of full grid to only crop the data points.
// Mar. 16/01: Fix problems in Truncate (in the class - as well as here).
// June /01: Colour layer routines.
// Sept/01:  Volume difference routine.
//*****************************************************************
#include <windows.h>
#include <stdio.h>
#include "surfgrid.h"
#include "scatdata.h"
#include "quikgrid.h"
#include "paintcon.h"
#include "grid.h"
#include "utilitys.h"
#include "assert.h"
#include "loaddata.h"
#include "rotate.h"
#include "condraw.h" 

extern ScatData ScatterData;
extern SurfaceGrid Zgrid;
extern SurfaceGrid Sgrid, Dgrid, Cgrid;
extern UINT PaintState;
extern float CgridZadjust;
double VolumeDifference, VolumeDifferencePositive, VolumeDifferenceNegative;
extern int DisplayVolumeDifference; 

//***************************************************************
//        G e n  G r i d  O n  V i e w
//***************************************************************
void GenGridOnView()
 {
   static float xmin, xmax, ymin, ymax, xincr, yincr;
   static int xLines, yLines; 

	GetZoomedUser( xmin, xmax, ymin, ymax );
	xLines = Zgrid.xsize();
	yLines = Zgrid.ysize();
	xincr = ((double)xmax-(double)xmin)/(xLines-1);
	yincr = ((double)ymax-(double)ymin)/(yLines-1);
	SetGridXY( xmin, xincr, ymin, yincr);
}
//***************************************************************
//        C r o p   G r i d   O n   V i e w
//***************************************************************
void CropGridOnView()
{
  SurfaceGrid TempGrid(2,2);
  static float xmin, xmax, ymin, ymax, x, y;
  static long i, j,
              xLines, yLines,
              xStart, xEnd, yStart, yEnd,
              NewXsize, NewYsize,
              NoPoints;
  static double xadjust, yadjust, zadjust;
  //static char szText[300];    // Used for debugging output only. 

  LoadNormalization( xadjust, yadjust);
        zadjust = ScatterData.zAdjust();

  GetZoomedUser( xmin, xmax, ymin, ymax );
  xLines = Zgrid.xsize();
  yLines = Zgrid.ysize();

  for( xStart = 0; Zgrid.x(xStart) <= xmin ; xStart++);
  if( xStart > 0 ) xStart--;
  assert( xStart >= 0); assert( xStart < xLines );

  //sprintf( szText, " xrange, yrange, ... %g, %g, %g, %g, %g, %i", xmin, xmax, ymin, ymax, Zgrid.x(0), xStart );
  //NotifyUser( szText );

  for( xEnd = 0; xEnd < xLines; xEnd++ ) if (Zgrid.x(xEnd) > xmax ) break;
  if( xEnd >= xLines ) xEnd = xLines -1;
  assert( xEnd > 0);

  for( yStart = 0; Zgrid.y(yStart) <= ymin ; yStart++);
  if( yStart > 0 ) yStart--;
  assert( yStart >= 0); assert( yStart < yLines );

  for( yEnd = 0; yEnd < yLines; yEnd++ ) if (Zgrid.y(yEnd) > ymax ) break;
  if( yEnd >= yLines ) yEnd = yLines -1;
  assert( yEnd > 0 );

  if( !(yStart == 0 && xStart == 0 &&
        yEnd == yLines-1 && xEnd == xLines-1 ) )
      //{ NotifyUser( " Request to crop entire grid ignored "); return;}
  {
    //sprintf( szText, " New grid coordinates: %i, %i, %i, %i ", xStart, xEnd, yStart, yEnd);
     //NotifyUser( szText );

    // now define size of the new grid and move it over to the temp.
    NewXsize = xEnd - xStart + 1 ;
    NewYsize = yEnd - yStart + 1 ;
    TempGrid.New( NewXsize, NewYsize);

    //sprintf( szText, " New grid size is: %i, %i ", NewXsize,NewYsize );
    //NotifyUser( szText );

    for( i = 0; i<NewXsize; i++ ) TempGrid.xset(i,Zgrid.x(i+xStart)) ;
    for( i = 0; i<NewYsize; i++ ) TempGrid.yset(i,Zgrid.y(i+yStart))  ;

    for( i = 0; i<NewXsize; i++ )
      for( j = 0; j<NewYsize; j++ ) TempGrid.zset(i,j,Zgrid.z(i+xStart,j+yStart));

    Zgrid.New(NewXsize,NewYsize);
    for( i = 0; i<NewXsize; i++ ) Zgrid.xset(i,TempGrid.x(i)) ;
    for( i = 0; i<NewYsize; i++ ) Zgrid.yset(i,TempGrid.y(i))  ;

    for( i = 0; i<NewXsize; i++ )
    for( j = 0; j<NewYsize; j++ ) Zgrid.zset(i,j,TempGrid.z(i,j));

    TempGrid.New(2,2); // Dump the memory for tempgrid.

    NumberOfContours = 10;
    NiceContourLinesSet();
    PaintContourReset();
    RotateReset();
 } // End if !cropping entire grid.

  if( ScatterData.Size() < 3 )
  {
    ScatterData.Reset();
    ScatterData.SetNext( Zgrid.x(0),
				 Zgrid.y(0),
				 Zgrid.zmin()-zadjust);
    ScatterData.SetNext( Zgrid.x(NewXsize-1),
				 Zgrid.y(NewYsize-1),
				 Zgrid.zmax()-zadjust);
    ScatterData.zAdjust( zadjust );
    return;
  }

  // There are scattered data points to be cropped.
  // Crop to the new grid, not the window.

  xmin = Zgrid.xmin();
  xmax = Zgrid.xmax();
  ymin = Zgrid.ymin();
  ymax = Zgrid.ymax();
  NoPoints = ScatterData.Size();

  //sprintf( szText, " No datapoints: %i ", NoPoints ); NotifyUser( szText);

  j = 0;
  for( i = 0; i < NoPoints; i++)
  {
    x = ScatterData.x(i);
    y = ScatterData.y(i);
    //sprintf( szText, " x, y: %g, %g ", x, y ); NotifyUser( szText);
    if( x >= xmin && x <= xmax &&
        y >= ymin && y <= ymax ) { ScatterData.move( j, i); j++;}
  }
  // if crop contains no scattered data points retain 2.
  j -= 1;
  if( j < 2 ) j = 2;
  if( j < NoPoints ) ScatterData.truncate(j);

}
//**************************************************************************
//        C o p y   G r i d   T o   S p e e d
//**************************************************************************
void CopyGridToSpeed()
{
  static int i, j, nx, ny;
  static float Adjust, z;
  nx = Zgrid.xsize();
  ny = Zgrid.ysize();
  Sgrid.New( nx, ny);
  Adjust = ScatterData.zAdjust();
  if( (Sgrid.zmin() - Adjust ) < 0.0 )
   NotifyUser( "Warning: Speed grid contains negative values which will be set to zero" );
  for( i = 0; i<nx; i++ )
  for( j = 0; j<ny; j++ )
  {
   z = Zgrid.z(i,j);
   if( z >= 0.0) // iff grid intersection is defined.
   {
     z -= Adjust;    // adjust to real value.
     if( z < 0.0 ) z = 0.0;
   } // end if z >= 0.0...
   Sgrid.zset(i,j,z);
  } // end for loops.
}

//**************************************************************************
//        C o p y   G r i d   T o   D i r e c t i o n
//**************************************************************************
void CopyGridToDirection()
{
  static int i, j, nx, ny;
  static float Adjust, z;
  nx = Zgrid.xsize();
  ny = Zgrid.ysize();
  Dgrid.New( nx, ny);
  Adjust = ScatterData.zAdjust();
  if( (Dgrid.zmin() - Adjust) < -360.0 || (Dgrid.zmax()-Adjust) > 361.0 )
  {
    NotifyUser( "Grid contains numbers that do not look like angles(degrees) - copy request ignored." );
    Dgrid.New( 2, 2 );
    return;
  }   
  for( i = 0; i<nx; i++ )
  for( j = 0; j<ny; j++ )
  {
   z = Zgrid.z(i,j);
   if( z >= 0.0) // iff grid intersection is defined.
   {
     z -= Adjust;    // adjust to real value.
     if( z < 0.0 )  z += 360.0 ; // Which leaves it still a big negative number if undefined.

   } // end if z >= 0.0...
   Dgrid.zset(i,j,z);
  } // end for loops.
}
//**********************************************************************
//    G r i d  S q u a r e   V o l u m e
//**********************************************************************
// Calculate the volume difference between the colour layer and the
// current layer for one grid square only.
static float Area;
static int SquaresDefined, SquaresNegative, SquaresTotal;
static float GridSquareVolume( int i, int j )
{
// if any corner undefined leave it.
 static float Caverage, Zaverage, Volume;
 static double zadjust;
 zadjust = ScatterData.zAdjust();
 SquaresTotal += 1;
 if( Cgrid.z(i,j) < 0.0 ) return 0.0;
 if( Cgrid.z(i+1,j) < 0.0 ) return 0.0;
 if( Cgrid.z(i+1,j+1) < 0.0 ) return 0.0;
 if( Cgrid.z(i,j+1) < 0.0 ) return 0.0;
 if( Zgrid.z(i,j) < 0.0 ) return 0.0;
 if( Zgrid.z(i+1,j) < 0.0 ) return 0.0;
 if( Zgrid.z(i+1,j+1) < 0.0 ) return 0.0;
 if( Zgrid.z(i,j+1) < 0.0 ) return 0.0;
 SquaresDefined += 1;
 Area = (Zgrid.x(i+1)-Zgrid.x(i))*(Zgrid.y(j+1)-Zgrid.y(j)); // Should be the same for all grid squares...
 // Take the z value as the average of all the corners.
 Caverage = (Cgrid.z(i,j)+Cgrid.z(i,j+1)+Cgrid.z(i+1,j+1)+Cgrid.z(i+1,j))*.25 - CgridZadjust;
 Zaverage = (Zgrid.z(i,j)+Zgrid.z(i,j+1)+Zgrid.z(i+1,j+1)+Zgrid.z(i+1,j))*.25 - zadjust;
 Volume = (Caverage-Zaverage)*Area;

 return Volume;
}
//**********************************************************************
//    C o l o u r   L a y e r   V o l u m e
//**********************************************************************
// Calculate the volume difference between the colour layer and the
// current layer.
void ColourLayerVolume()
{

  SquaresDefined = SquaresNegative = SquaresTotal = 0.0;
  VolumeDifference = VolumeDifferencePositive = VolumeDifferenceNegative = 0.0;
  static int i, j, xsize, ysize ;
  static double Volume;
  if( Cgrid.xsize() < 3 ) return;  // Colour grid doesn't exist.
  if( !DisplayVolumeDifference ) return; // Information not being displayed.
  xsize = Zgrid.xsize();
  ysize = Zgrid.ysize();
  for( i = 0; i<(xsize-1); i++ )
  for( j = 0; j<(ysize-1); j++ )
  {
    Volume = GridSquareVolume( i, j ) ;
    if( Volume < 0.0 ) VolumeDifferenceNegative += Volume;
    else               VolumeDifferencePositive += Volume;
  }
  VolumeDifference = VolumeDifferencePositive + VolumeDifferenceNegative;
//  static char szBuf[200];
//  sprintf( szBuf, " Square area = %g  Total squares = %i  Defined squares = %i Negative squares = %i ",
//       Area, SquaresTotal, SquaresDefined, SquaresNegative );
//  NotifyUser( szBuf );
}

//**************************************************************************
//        C o p y   G r i d   T o   C o l o u r
//**************************************************************************
void CopyGridToColour()
{
  static int i, j, nx, ny;
  //static float Adjust, z;
  static float z;
  SaveColourMap();
  nx = Zgrid.xsize();
  ny = Zgrid.ysize();
  Cgrid.New( nx, ny);

  for( i = 0; i<nx; i++ ) // Initialize Cgrid to undefined.
  for( j = 0; j<ny; j++ ) Cgrid.zset(i,j, -99999.);
  Cgrid.Znew(); // And reset for new Z values.

  for( i = 0; i<nx; i++ )
  for( j = 0; j<ny; j++ )
  {
   z = Zgrid.z(i,j);
   if( z >= 0.0) Cgrid.zset(i,j,z);
  } // end for loops.
  CgridZadjust = ScatterData.zAdjust(); 
  ColourLayerVolume(); 
}
//**************************************************************************
//        K i l l   L a y e r   G r i d s
//**************************************************************************
void KillLayerGrids()
{
  Sgrid.New(2,2);
  Dgrid.New(2,2);
  if( Cgrid.xsize() > 2 ) { Cgrid.New(2,2); RestoreColourMap(); }
}
//**************************************************************************
//      C o l o u r  L a y e r  C o m p a r e
//**************************************************************************
void ColourLayerCompare()
// Replace colour layer with difference from the grid.
{
 static int i, j, nx, ny;
 static float z, c, Zadjust, Minimum, Maximum, Difference, NewCgridZadjust;
 static bool first;
 first = true;
 SaveColourMap();
 Zadjust = ScatterData.zAdjust();
 if( Cgrid.xsize() < 3 )
  { NotifyUser( " Colour layer does not exist - command ignored" ); 
    return; }
 nx = Zgrid.xsize();
 ny = Zgrid.ysize();
 // Pass 1 - determine smallest value in the difference.
 for( i = 0; i<nx; i++ )
  for( j = 0; j<ny; j++ )
  {
    z = Zgrid.z(i,j);
    c = Cgrid.z(i,j);
    if( (z < 0.0) || (c < 0.0) ) continue; // Undefined doesn't count.
    Difference =  (c-CgridZadjust) - (z-Zadjust);
    if( first ) { Minimum = Maximum = Difference; first = false; }
    if( Difference < Minimum ) Minimum = Difference;
    if( Difference > Maximum ) Maximum = Difference;
  } // end for i,j loops.

   if( first || (Maximum - Minimum) == 0.0 )
    { NotifyUser( " No difference between colour layer and grid " ) ;
      return; } 

  // Pass 2 - set the difference.
// NotifyUser( " Minimum = %g  Maximum = %g ", Minimum, Maximum ); 
 Cgrid.Znew();
 NewCgridZadjust = -Minimum;
 for( i = 0; i<nx; i++ )
  for( j = 0; j<ny; j++ )
  {
   z = Zgrid.z(i,j);
   c = Cgrid.z(i,j);
   if( (z < 0.0) || (c < 0.0) ) Cgrid.zset( i, j, -99999.);
   else
        { Difference =  (c-CgridZadjust) - (z-Zadjust);
          // if( Difference == 0.0 )
              // Cgrid.zset( i, j, -99999.);
          // else
          Cgrid.zset( i, j, Difference + NewCgridZadjust); }
     } // end for loops.
  CgridZadjust = NewCgridZadjust; 
 // Set nice colour map 
  static float min, incr, max;
  min = Cgrid.zmin();
  max = Cgrid.zmax();
  incr = ((double)max-(double)min)/9.;
  incr = AtPrecision( incr, 1 );  // take increment to 1 digit.
  // Make min & max a divisor of increment.
  min = double( int( ((double)min-CgridZadjust) / incr)*incr) + CgridZadjust;
  max = double( int( ((double)max-CgridZadjust) / incr)*incr) + CgridZadjust;
  ColourMap( min, min+(max-min)*.5, max);
  //DisplayVolumeDifference = FALSE;
   return; 
}

