//************************************************************
//                      H a t c h
// Hatch3d: draw grid with hidden lines - using polygon fill.
// Hatch2d: draw coloured grid squares using polygon fill for 2d case.
// Assumptions: - all scaling has been done before call.
//
//      Copyright (c) 1993 - 2002 by W. John Coulthard
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
// Feb 3./99: Fix bug in Hatch to handle negative Turn.
// Nov 15/00: Implement variable colours in the grid squares.
// Mar 12/01: Allow gridlines to show on coloured grids.
// Apr. 22/01: Finish implementation of display for special coloured grid.
// Oct. 3/01: Check columns for visbility to increase speed.
// Jan. 7-8/02: Code to draw contours on the grid squarein Hatch3d.
//************************************************************

#include <windows.h>
#include <iostream>
#include "assert.h"
#include "surfgrid.h"
#include "scatdata.h"
#include "xygrid.h" 
#include "quikgrid.h"
#include "paintcon.h"
#include "condraw.h"
#include "utilitys.h"
using namespace std;
// External variables.
extern int ColouredGrid;
extern int NumberOfContours;
extern SurfaceGrid Cgrid;
extern HPEN hBlackPen,
            hBoldPen;
extern ContourOptionsType DrawingOptions;

// Global variables defined here.
int GridlineColouredGrid = FALSE;
int NeverHideContours = FALSE;

static float AverageZ, HighZ, LowZ ;
static POINT corners[4]; // The screen coordinates of the grid.

// Local variables. 
static float Zcorners[4];  // The original data corners of the grid.


//**************************************************************
//        D r a w  C o r n e r s
//**************************************************************
// A service routine for Hatch2D and Hatch3D.

static void DrawCorners( HDC &hdc )
{
  static HBRUSH hBrush, hOldBrush;
  static HPEN hPen, hOldPen;
  static COLORREF Colour;
  static int k;

 // if any corner is visible draw the grid rectangle.
 for( k = 0; k<4; k++ )
 {
  if( Visible(corners[k].x, corners[k].y)  )
  {
	 if( ColouredGrid)
	 {
		Colour = ChooseZColour( AverageZ );
		hBrush = CreateSolidBrush( Colour );
		if( !GridlineColouredGrid )
      {
         hPen = CreatePen( PS_SOLID, 0, Colour);
         assert( hPen );
         hOldPen   = (HPEN)SelectObject( hdc, hPen );
      } // end if( !GridLine...
		assert( hBrush );
		hOldBrush = (HBRUSH)SelectObject( hdc, hBrush);
	 } // end if( ColouredGrid...

    Polygon( hdc, corners, 4 );

    if( ColouredGrid)  // Clean up coloured grid stuff.
	 {
		SelectObject( hdc, hOldBrush );
		assert( DeleteObject( hBrush));
      if( !GridlineColouredGrid )
      {
         SelectObject( hdc, hOldPen );
		   assert( DeleteObject( hPen ) );
      }
	 } // end if( ColouredGrid...

  return; //                             ---> Exit function once grid is drawn!
  } // end if ( Visible...
 } // end for...
}
//**************************************************************
//        H a t c h  2 D
//**************************************************************
void Hatch2D( int xScaled[], int yScaled[], HDC &hdc )
{
  static int xsize, ysize, i, j;
  xsize = Zgrid.xsize();
  ysize = Zgrid.ysize();
  if( Cgrid.xsize() < 3 )
  { // Main grid should be used if Cgrid is not there.
   for( i = 1; i < xsize; i++ )
   {
    // Test to see if column is outside drawing rectangle.
    corners[0].x = xScaled[i];
    // If less than skip
    if( VisibleX( corners[0].x ) < 0 ) continue;
    corners[1].x = xScaled[i-1];
    // If greater than all done.
    if( VisibleX( corners[1].x ) > 0 ) return;
    corners[2].x = xScaled[i-1];
    corners[3].x = xScaled[i];
    for( j = 1; j < ysize; j++ )
    {
	 if( Zgrid.z(i,j)   < 0 || Zgrid.z(i-1,j)   <0 ) continue;
	 if( Zgrid.z(i,j-1) < 0 || Zgrid.z(i-1,j-1) <0 ) continue;
	 corners[0].y = yScaled[j];
	 corners[1].y = yScaled[j];
	 corners[2].y = yScaled[j-1];
	 corners[3].y = yScaled[j-1];
	 AverageZ = (Zgrid.z(i,j)+Zgrid.z(i,j-1)+Zgrid.z(i-1,j-1)+Zgrid.z(i-1,j))/4.;
	 DrawCorners( hdc );
    } // End for j
   } // End for i
  } // end if( Cgrid...
  else  // The Cgrid exists so use it. 
  {
    for( i = 1; i < xsize; i++ )
   {
    // Test to see if column is outside drawing rectangle.
    corners[0].x = xScaled[i];
    // If less than skip
    if( VisibleX( corners[0].x ) < 0 ) continue;
    corners[1].x = xScaled[i-1];
    // If greater than all done.
    if( VisibleX( corners[1].x ) > 0 ) return;
    corners[2].x = xScaled[i-1];
    corners[3].x = xScaled[i];
    for( j = 1; j < ysize; j++ )
    {
	 if( Cgrid.z(i,j)   < 0 || Cgrid.z(i-1,j)   <0 ) continue;
	 if( Cgrid.z(i,j-1) < 0 || Cgrid.z(i-1,j-1) <0 ) continue;
	 corners[0].y = yScaled[j];
	 corners[1].y = yScaled[j];
	 corners[2].y = yScaled[j-1];
	 corners[3].y = yScaled[j-1];
    AverageZ = (Cgrid.z(i,j)+Cgrid.z(i,j-1)+Cgrid.z(i-1,j-1)+Cgrid.z(i-1,j))/4.;
	 DrawCorners( hdc );
    } // End for j
   } // End for i 
  } // end Else...
}
//**************************************************************
//        C o n t o u r   3 D   G r i d   S q u a r e
//**************************************************************
// Contours the grid square described by Zcorners and corners.
// a service routine for routine DrawGridSquare
static void Contour3dGridSquare(  HDC &hdc)
{
 static int i, j, jMinus1, Ref, Sub, x, y, xDistance, yDistance;
 static bool drawing;
 static float ContourValue, Real3DContourValue, Fraction, Temp;
 static HPEN oldPen;

 if( NeverHideContours || !DrawingOptions.contours ) return;

 for( i = 0; i < NumberOfContours; i++ )
 {
   Real3DContourValue = ContourLineValue(i);
   ContourValue = Zgrid.zmap( Real3DContourValue );
   // Contour line above or below grid square - skip it.
   if( ContourValue < LowZ || ContourValue > HighZ ) continue;
   // Don't want to contour "flat" areas.
   if( Zcorners[0] == Zcorners[1] &&   // If all corners the same .
       Zcorners[0] == Zcorners[2] &&   // and the same as the contour line.
       Zcorners[0] == Zcorners[3] &&
       Zcorners[0] == ContourValue ) continue;  // don't contour the grid.

   drawing = false;

   if( ContourLineBold(i) ) oldPen = (HPEN)SelectObject( hdc, hBoldPen);
   else oldPen = (HPEN)SelectObject( hdc, hBlackPen);

   for( j = 0; j < 4; j++ )
   {
      jMinus1 = j-1;
      if( jMinus1 < 0 ) jMinus1 = 3;
      if( Zcorners[j] > Zcorners[jMinus1] ) { Ref = j; Sub = jMinus1; }
      else                                  { Ref = jMinus1; Sub = j; }

      // if the two points are the same as the contour value just connect them.
      if( (Zcorners[Ref] == Zcorners[Sub]) && (Zcorners[Ref] == ContourValue) )
      {
        MoveToEx( hdc, corners[Ref].x, corners[Ref].y, NULL );
        LineTo(   hdc, corners[Sub].x, corners[Sub].y );
        continue;
      }
      // If the contour line lies between the two corners. . .
      if( ContourValue < Zcorners[Sub] || ContourValue >= Zcorners[Ref]) continue;
      //Interpolate and calculate x and y coordinate.
      xDistance = corners[Ref].x - corners[Sub].x;
      yDistance = corners[Ref].y - corners[Sub].y;
      Temp = Zcorners[Ref] - Zcorners[Sub];
      if( Temp > 0.0 ) Fraction = (Zcorners[Ref] - ContourValue)/Temp;
      else                 Fraction = 0.0 ;
      if( Fraction > 1.0 ) Fraction = 1.0;
      x = corners[Ref].x - Fraction*xDistance;
      y = corners[Ref].y - Fraction*yDistance;

      if( !drawing )MoveToEx( hdc, x, y, NULL );
      else          LineTo( hdc, x, y );
      drawing = !drawing;
      
   } // end for( j = 1...
 SelectObject( hdc, oldPen );
 } // end for ( i = 0...
}

//**************************************************************
//        D r a w    G r i d    S q u a r e
//**************************************************************
// draws a polygon filled grid identified by i,j
// a service routine for routine Hatch3D.
static void DrawGridSquare( xyGridClass &grid, const int i, const int j, HDC &hdc )
{

 static float TryZ;
 int k;

 corners[0].x = grid.x(i  ,j  );   // Get screen coordinates of grid square
 corners[0].y = grid.y(i  ,j  );
 corners[1].x = grid.x(i-1,j  );
 corners[1].y = grid.y(i-1,j  );
 corners[2].x = grid.x(i-1,j-1);
 corners[2].y = grid.y(i-1,j-1);
 corners[3].x = grid.x(i  ,j-1);
 corners[3].y = grid.y(i  ,j-1);

 // if any corner is undefined don't draw the grid rectangle.
 for( k = 0; k<4; k++ ) if( corners[k].x==0) return;

 Zcorners[0] = Zgrid.z(i  ,j  );
 Zcorners[1] = Zgrid.z(i-1,j  );
 Zcorners[2] = Zgrid.z(i-1,j-1);
 Zcorners[3] = Zgrid.z(i  ,j-1);

 AverageZ = (Zcorners[0]+Zcorners[1]+Zcorners[2]+Zcorners[3])/4.;
 HighZ = LowZ = Zcorners[0];
 for( k = 1; k<4; k++ )
  {
    TryZ = Zcorners[k];
    if( TryZ > HighZ ) HighZ = TryZ;
    if( TryZ < LowZ  ) LowZ  = TryZ;
  }
 // Now see if coloured grid is available.

 if( Cgrid.xsize() < 3 )
    {                      // Coloured grid is NOT available.
      DrawCorners( hdc );
      Contour3dGridSquare( hdc );
      return;
    }

 // Yes it is available so do intersecton with coloured grid.
 // 1. If coloured grid is undefined skip it.
 if( Cgrid.z(i,j)   < 0 || Cgrid.z(i-1,j)   <0 ||
     Cgrid.z(i,j-1) < 0 || Cgrid.z(i-1,j-1) <0 )
     {
       Contour3dGridSquare( hdc );  // Contour it anyhow.
       return;
     }
 // 2. And replace average z with value from coloured grid.
 AverageZ = (Cgrid.z(i,j)+Cgrid.z(i,j-1)+Cgrid.z(i-1,j-1)+Cgrid.z(i-1,j))/4.;
 DrawCorners( hdc );
 Contour3dGridSquare( hdc );
 return;
}

//*****************************************************************
//                     H a t c h 3 D
//*****************************************************************
void Hatch3D( xyGridClass &grid, HDC &hdc )
// Method: Draw the squares in order - farthest from the viewer
//         to closest - using polygon fill. Closer polygons
//         "hide" the farther ones..... 
{
//  Determine the Octant the grid is being viewed from.
//       * 4*3 *
//       5* * *2
//       *******
//		   6* * *1
//(0,0)->* 7*0 *
//

  static int Octant;
  static float TurnTemp;

  TurnTemp = Turn;
  if( TurnTemp < 0 ) TurnTemp += 360;
  Octant = TurnTemp/45.;
  Octant = Octant%8; 

  struct DirectType {  // stuff to handle looping control
     int xstart;
     int xend;
     int xincr;
     int ystart;
     int yend;
     int yincr;
     int loop;         // loop: =1: x vary fastest =2: y vary fastest. 
                    };
  DirectType table[8] =    {
    { 1,9, 1,8,0,-1,1}, // 0   the 9's will be replaced by  the
    { 1,9, 1,8,0,-1,2}, // 1       grid dimensions appropriately.
    { 1,9, 1,1,9, 1,2}, // 2
    { 1,9, 1,1,9, 1,1}, // 3   the 8's are replace by the
    { 8,0,-1,1,9, 1,1}, // 4       grid dimensions-1.
    { 8,0,-1,1,9, 1,2}, // 5
    { 8,0,-1,8,0,-1,2}, // 6   loop termination values don't get processed.
    { 8,0,-1,8,0,-1,1}  // 7
                            };


   int xsize = grid.xsize();
   int ysize = grid.ysize();

   for( int i =0; i<4; i++) // Fill table values in for xsize,ysize. 
     {
      table[i].xend= xsize;
      table[i+4].xstart = xsize-1;
      int j = i + 2;
      table[j].yend = ysize;
      table[((j+4)%8)].ystart = ysize-1;
     }

   if( table[Octant].loop == 1 )
   {  // x vary fastest loop:
     for( int j = table[Octant].ystart; j != table[Octant].yend; j += table[Octant].yincr)
      { for( int i = table[Octant].xstart; i != table[Octant].xend; i += table[Octant].xincr)
        DrawGridSquare( grid, i, j, hdc ); 
      }
   }
   else
   { // y vary fastest loop:
     for( int i = table[Octant].xstart; i != table[Octant].xend; i += table[Octant].xincr)
      { for( int j = table[Octant].ystart; j != table[Octant].yend; j += table[Octant].yincr)
        DrawGridSquare( grid, i, j, hdc ); 
      }
   }
}