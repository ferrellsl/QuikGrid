//********************************************************
//          D r a w s t u f f
//
//  Routines in support of drawing stuff.
//
//             Copyright (c) 1999 - 2002 by John Coulthard
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
// Nov. 8/00: Change Polybuffer to force reset of pen after
//            flush of the buffer (olddrawpen == 0 )
// Feb. 24/02: Draw line if it might cross through the
//            rectangle even though both ends are hidden.
//********************************************************
#include <windows.h>
#include "surfgrid.h"
#include "scatdata.h"
#include "quikgrid.h"
#include "paintcon.h"
#include "assert.h"

extern HDC         hdc;
static int olddrawpen = 0;
//********************************************************
//    P o l y B u f f e r
//********************************************************
void PolyBuffer( int x, int y, int draw )
{
  static const int size = 1000;
  static POINT buffer[size];
  static int posn = 0;

  // Uncomment the following to disable polyline generation.
  /*
  static int disable = TRUE;
  if ( disable ) // disable line buffering - to help testing. .
  {
    if( draw == 0 ) MoveToEx( hdc, x, y, NULL );
    if( draw == 1 ) LineTo(   hdc, x, y );
    return;
  }
  //  */

  if( draw < 0 ) // flush the polyline buffer by doing a move to the last posn.
   {
     olddrawpen = 0;
     if( posn > 1 )
     {
       Polyline( hdc, buffer, posn );
       buffer[0] = buffer[posn-1]; // initialize with move to last location.
       posn = 1;
     }
     return;
   }
  if( draw == 1 )
  {
	 assert( posn > 0 ); // should be a move to the first location....
	 buffer[posn].x = x;
	 buffer[posn].y = y;
	 posn++;
	 if( posn == size )
	 {
      Polyline( hdc, buffer, posn );
		buffer[0] = buffer[posn-1]; // initialize with move to last location.
		posn = 1;
	 }
  }
  else // it is a move
  {
    if( posn > 1 ) Polyline( hdc, buffer, posn );
	 buffer[0].x = x;
	 buffer[0].y = y;
	 posn = 1;
  }
  return;
}

//********************************************************
//    P l o t T o
//********************************************************
void PlotTo( int x, int y, int pen )
// Local service routine.
// If pen = 0 do a move to x, y
// If pen > 0 do a draw to x, y with different pens
// if pen < 0 initialize things.
// If both line segments not visible don't draw it.
// Modify so if both ends not visible make sure they don't intersect
// the visible rectangle. (i.e. both to and from are outside on the same side.
{
 static int oldx, oldy, oldpen = 0;
 static int oldvisible;

 if( pen < 0 ) { olddrawpen = 0; return; }

// Uncomment the following to disable polylining at this level.
/*
 static int disable = TRUE;
 if( disable )
 {
  if( pen == 0 ) { MoveToEx( hdc, x, y, NULL ); return; }
  if( pen == 1 && pen != olddrawpen )
	 SelectPen( 1 );
    //{ NotifyUser( "Selecting pen 1 " ); SelectObject( hdc, hSolidPen );}
  if( pen == 2 && pen != olddrawpen )
	 SelectPen( 2 );
	 // { NotifyUser( "Selecting pen 2 " ); SelectObject( hdc, hHighLitePen );}
  LineTo(hdc, x, y );
  olddrawpen = pen;
  return; 
 }
 // */

 if( pen == 0 )  //   This is a move.
	{
	  oldvisible = Visible( x, y );
	  if( oldvisible) PolyBuffer( x, y, 0 ); // do move if visible
	  oldx = x; oldy = y;
	  return;
	}

	// This is a draw
 if( pen == 1 && pen != olddrawpen)   // Select the right pen.
	 { PolyBuffer(0,0,-1);SelectPen(1);}
 if( pen == 2 && pen != olddrawpen)
     { PolyBuffer(0,0,-1);SelectPen(2);}
 oldpen = olddrawpen = pen; 

 if( Visible(x, y) )  // The "to" point is visible.
	{
	  if( !oldvisible ) PolyBuffer( oldx, oldy, 0 );  // if necessary do a move to old invisible point.
	  PolyBuffer( x, y, 1 ); // then draw to "to" point and exit.
	  oldvisible = TRUE;
     oldx = x; oldy = y;
     return;
	}

 // The "to" point is not visible.

 if( oldvisible) // but the old one is.
   {
     PolyBuffer( x, y, 1); // Draw to invisible "to" point anyhow.
     //PolyBuffer( x, y, 0); // and do a move to same invisible location.
     oldvisible = FALSE;
     oldx = x; oldy = y;
     return;
   }

 // both ends are invisible - make sure can't draw through rectangle.

 if( (VisibleX( x ) == VisibleX( oldx ) && VisibleX( x ) != 0 ) ||    // Make sure that x or
     (VisibleY( y ) == VisibleY( oldy ) && VisibleY( y ) != 0 ))     // y are outside on same side.
   {    // The line is really outside. Just save it and exit.
     oldvisible = FALSE;
     oldx = x; oldy = y;
     return;
   }

   // The line *might* go through the rectangle - don't take a chance.

 PolyBuffer( oldx, oldy, 0 ); // Move to old invisible point.
 PolyBuffer( x, y, 1);        // Draw to the "to" point.
 oldvisible = FALSE;
 oldx = x; oldy = y;
 return;
}
