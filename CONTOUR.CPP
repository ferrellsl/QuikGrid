//***************************************************************
//                     Contour plotting of a grid.
//                               
//               Copyright (c) 1993 - 2002 by John Coulthard
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
//************************************************************************
//
//   This code is a transcription of a FORTRAN IV subroutine called
//   CNTOUR that I wrote in the late 60's. (that is why it contains
//   goto's). The last known bug was removed from CNTOUR in 1972 and I
//   did not wish to change the structure of such a stable piece of code.
//
//   The source code for CNTOUR was published in the SHARE user's group
//   library. The code was widely distributed and heavily used.
//
//***************************************************************
// Requires a simple class defined in surfgrid.h and surfgrid.cpp to
// supply the grid. 
//

#include "surfgrid.h"  // The simple class mentioned above.
#include <memory.h>       // For memset function in switches
#include "assert.h"

// Forward declare functions in switches.cpp.
void SwitchClear(int i, int j);
int SwitchGet  (int i, int j);
int SwitchSet  (int i, int j);


static int XReferencePoint,  // We are contouring between a
           YReferencePoint,  //    Reference point and a
           XSubPoint,        //    Sub point. These are used to scan
	        YSubPoint,        //    the grid and mark beginings of lines.
           NumberOfX,        // Number of X and Y grid lines.
	        NumberOfY,
           Drawing;

static float ContourValue;    // Value we are contouring.

// Forward declare functions in contour.cpp
static void TraceContour( SurfaceGrid &Zgrid );
static void Interpolate( float, float, float, float, float, float);

// DoLineTo must be supplied by the calling routine. 
void DoLineTo( float x, float y, int drawtype );

//****************************************************************
//                    C o n t o u r
//****************************************************************

void Contour( SurfaceGrid &Zgrid, float TourValue )

// This function conducts a search of all of the elements of the
// rectangular grid looking for existence of the contour line
// represented by TourValue.

// Once the value is found function TraceContour is called to
// actually trace the contour through the grid. It ultimately
// calls function DoLineTo which must be supplied to actually
// draw the line.

// An array of switches is maintained to flag whether a given
// grid location has been already contoured or not (to prevent
// tracing a contour line more than once.

{
   NumberOfX = Zgrid.xsize();
   NumberOfY = Zgrid.ysize();
   SwitchClear(NumberOfX, NumberOfY);
   ContourValue = TourValue;

// Search for a bottom edge contour
   
   YReferencePoint = 0;
   YSubPoint       = 0;

   for( XReferencePoint = 1; XReferencePoint<NumberOfX; XReferencePoint++ )
   {
     if( Zgrid.z(XReferencePoint,0) <= ContourValue ) continue;
     XSubPoint = XReferencePoint - 1;
     if( Zgrid.z(XSubPoint,0) > ContourValue)   continue;
     TraceContour( Zgrid );
   }

// Search for a right edge contour
   
   XReferencePoint = NumberOfX - 1;
   XSubPoint       = XReferencePoint;

   for( YReferencePoint =1; YReferencePoint<NumberOfY; YReferencePoint++)
   {
    if( Zgrid.z(NumberOfX -1, YReferencePoint) <= ContourValue) continue;
    YSubPoint = YReferencePoint -1;
    if (Zgrid.z(NumberOfX -1, YSubPoint) > ContourValue) continue;
    TraceContour( Zgrid );
   }

// Search the top edge.
    
   YReferencePoint = NumberOfY-1;
   YSubPoint       = YReferencePoint;

   for( XReferencePoint = NumberOfX-2; XReferencePoint>=0; XReferencePoint--)
   {
    if( Zgrid.z( XReferencePoint,NumberOfY-1) <= ContourValue) continue;
    XSubPoint = XReferencePoint + 1;
    if( Zgrid.z( XSubPoint,      NumberOfY-1) >  ContourValue) continue;
    
    TraceContour( Zgrid );
    
   }

// Search the left edge
   
   XReferencePoint = 0;
   XSubPoint       = 0;

   for ( YReferencePoint = NumberOfY-2; YReferencePoint>=0;YReferencePoint--)
   {
    if( Zgrid.z( 0, YReferencePoint) <= ContourValue ) continue;
    YSubPoint = YReferencePoint + 1;
    if( Zgrid.z( 0, YSubPoint) > ContourValue ) continue;
    TraceContour( Zgrid );
   }

// Search the interior of the array
   
   for( YReferencePoint = 1; YReferencePoint<NumberOfY-1; YReferencePoint++)
   {
    for( XReferencePoint =1; XReferencePoint<NumberOfX;   XReferencePoint++)
    {
     XSubPoint = XReferencePoint-1;
     if( Zgrid.z(XReferencePoint,YReferencePoint)<=ContourValue) continue;
     if( Zgrid.z(XSubPoint,      YReferencePoint)> ContourValue) continue;
     if( SwitchGet(XReferencePoint,YReferencePoint)) continue;

     YSubPoint = YReferencePoint;
     TraceContour( Zgrid );
    }
   }
 SwitchClear( 0, 0 ); // All done! delete memory for switches.
}

//*******************************************************************
//                  T r a c e   C o n t o u r
//********************************************************************

static void TraceContour( SurfaceGrid &Zgrid )
{
  static const int XLookupTable[9] = {  0, 1, 1,  0, 9, 0, -1, -1, 0};
  static const int YLookupTable[9] = { -1, 0, 0, -1, 9, 1,  0,  0, 1};
  static const int DiagonalTest[9] = {  0, 1, 0,  1, 0, 1,  0,  1, 0};

  static int XTracePoint,
             YTracePoint,
             XTraceSubPoint,
             YTraceSubPoint;
	       
  static float XMidPoint,
               YMidPoint,
               ZMidPoint;

  static int Locate,
             XNext,
             YNext,
             XMidNext,
             YMidNext;

  XTracePoint = XReferencePoint;
  YTracePoint = YReferencePoint;
  XTraceSubPoint = XSubPoint;
  YTraceSubPoint = YSubPoint;
  Drawing = 0 ;
  
  HorizontalOrVerticalCode:               // Warning - target of a goto!!!!

  Interpolate( Zgrid.x( XTracePoint), Zgrid.y( YTracePoint),
	       Zgrid.z( XTracePoint, YTracePoint),
	       Zgrid.x( XTraceSubPoint), Zgrid.y( YTraceSubPoint),
	       Zgrid.z( XTraceSubPoint, YTraceSubPoint)  ) ;

  Locate = 3*(YTracePoint-YTraceSubPoint) +
		  XTracePoint-XTraceSubPoint + 4;
      //assert( Locate >=0 && Locate <9 ); 
  
  XNext = XTraceSubPoint + XLookupTable[ Locate ];
  YNext = YTraceSubPoint + YLookupTable[ Locate ];

  // Test to see if the next point is past an edge.

  if( (XNext >= NumberOfX) || (XNext < 0) ||
      (YNext >= NumberOfY) || (YNext < 0)   ) return; 
  
  // Check - if vertical line and been contoured before - all done.

  if( (Locate == 5) && SwitchSet(XTracePoint,YTracePoint) ) return;


  if( !DiagonalTest[Locate] )
  {
    assert(true);  // The code should never reach here (always a diagonal). 
  if( Zgrid.z( XNext,YNext) > ContourValue )
    { XTracePoint = XNext;
      YTracePoint = YNext; } 
    else
    { XTraceSubPoint = XNext;
      YTraceSubPoint = YNext; }
                                              goto HorizontalOrVerticalCode;  
   }
  
  // Diagonals get special treatment = the midpoint of the rectangle
  // has a midpoint which is calculated and used as a contour point.

  XMidPoint = ( Zgrid.x( XTracePoint) + Zgrid.x(XNext) )*0.5;
  YMidPoint = ( Zgrid.y( YTracePoint) + Zgrid.y(YNext) )*0.5;

  Locate = 3*(YTracePoint-YNext) + XTracePoint - XNext + 4;
      //assert( (Locate >= 0) && (Locate <9) );

  XMidNext = XNext + XLookupTable[Locate];
  YMidNext = YNext + YLookupTable[Locate];
  //assert( ( XMidNext >= 0) && (XMidNext < NumberOfX) );
  //assert( ( YMidNext >= 0) && (YMidNext < NumberOfY) );

  ZMidPoint = ( Zgrid.z(XTracePoint,YTracePoint) +
		Zgrid.z(XTraceSubPoint, YTraceSubPoint) +
		Zgrid.z(XNext, YNext) +
		Zgrid.z(XMidNext, YMidNext ) )*0.25;

  if( ZMidPoint > ContourValue)                  goto MidPointGTContourCode;
  // Midpoint less than contour value 
  Interpolate( Zgrid.x(XTracePoint), Zgrid.y(YTracePoint),
	       Zgrid.z(XTracePoint,YTracePoint),
	       XMidPoint, YMidPoint, ZMidPoint);

  if( Zgrid.z(XMidNext,YMidNext) <= ContourValue )

  // Turn off sharp right.... 
  { XTraceSubPoint = XMidNext;
    YTraceSubPoint = YMidNext;
                                              goto HorizontalOrVerticalCode; 
  }

  Interpolate( Zgrid.x(XMidNext), Zgrid.y(YMidNext),
	       Zgrid.z(XMidNext, YMidNext),
	       XMidPoint, YMidPoint, ZMidPoint);

  if( Zgrid.z(XNext,YNext) <= ContourValue)
  // Continue straight through.... 
  { XTracePoint = XMidNext;
    YTracePoint = YMidNext;
    XTraceSubPoint = XNext;
    YTraceSubPoint = YNext;
                                              goto HorizontalOrVerticalCode; 
  }
  // Wide left turn.
  Interpolate(Zgrid.x(XNext), Zgrid.y(YNext),
	      Zgrid.z(XNext,YNext),
	      XMidPoint, YMidPoint, ZMidPoint);

  XTracePoint = XNext;
  YTracePoint = YNext;
                                              goto HorizontalOrVerticalCode;

MidPointGTContourCode:                                  // Target of a goto!

  Interpolate( XMidPoint, YMidPoint, ZMidPoint,
	       Zgrid.x(XTraceSubPoint), Zgrid.y(YTraceSubPoint),
	       Zgrid.z(XTraceSubPoint,YTraceSubPoint) );

  // It may be a sharp left turn.
  if( Zgrid.z(XNext,YNext) > ContourValue)
  {XTracePoint = XNext;
   YTracePoint = YNext;
                                              goto HorizontalOrVerticalCode;  
  }
  // no 
  Interpolate( XMidPoint, YMidPoint, ZMidPoint,
	       Zgrid.x(XNext), Zgrid.y(YNext),
	       Zgrid.z(XNext,YNext) );
  // Continue straight through? 
  if( Zgrid.z( XMidNext,YMidNext) > ContourValue )
  // yes 
  { XTraceSubPoint = XNext;
    YTraceSubPoint = YNext;
    XTracePoint = XMidNext;
    YTracePoint = YMidNext;
                                              goto HorizontalOrVerticalCode;
   }

  // Wide right turn
  Interpolate( XMidPoint, YMidPoint, ZMidPoint,
	       Zgrid.x(XMidNext), Zgrid.y(YMidNext),
	       Zgrid.z(XMidNext,YMidNext) );

  XTraceSubPoint = XMidNext;
  YTraceSubPoint = YMidNext;
                                              goto HorizontalOrVerticalCode;

}
//**********************************************************************
//                     I n t e r p o l a t e
//**********************************************************************
static void Interpolate( float XRef, float YRef, float ZRef,
                         float XSub, float YSub, float ZSub ) 
{
 // This routine interpolates between the two points and calls to
 // plot the line tracing out the contour.

 static float Xdistance,
              Ydistance,
              temp,
              Fraction,
              XPlotLocation,
              YPlotLocation;
               
 if( ZSub < 0.0 ) { Drawing = 0; return; }// Don't contour negative areas.

 Xdistance = XRef - XSub;
 Ydistance = YRef - YSub;
 temp = ZRef - ZSub; // watch out- underflow!!!!!
 if( temp > 0.0 )    
      Fraction = (ZRef - ContourValue)/ temp ; 
 else Fraction = 0.0;
 if ( Fraction > 1.0 ) Fraction = 1.0;
 XPlotLocation = XRef - Fraction*Xdistance;
 YPlotLocation = YRef - Fraction*Ydistance;
 
 DoLineTo( XPlotLocation, YPlotLocation, Drawing );
 Drawing = 1;
 
}

//******************************************************************
//                       S w i t c h e s
//
//  Contouring routine to get and set bit switches. The purpose is
//  flag any reference point in the grid defining the surface if
//  a contour has been traced past it.
//
//********************************************************************

static unsigned char ON = 1;
static long CharPosition, BitOffset;
static unsigned char *BitArray = 0;
static unsigned long SizeOfBitArray = 0;
static const long BitsPerChar = 8 ;
static long FirstDimension = 0;

//****************************************************************
//               S w i t c h    C l e a r
//****************************************************************
void SwitchClear( int i, int j)
{
// Routine allocates space for i by j bit switches and clears
// them all to zero.

// If i and j are zero just delete the previous array of switches. 

 static long SizeNeeded;
 SizeNeeded = ((long)i*(long)j)/BitsPerChar+1;

 if( SizeNeeded <= 1 ) 
   { if (BitArray != 0 ) delete[] BitArray; 
     BitArray = 0; SizeOfBitArray = 0; 
     return; 
   } 

 FirstDimension = i;
 //assert(FirstDimension > 0 );

 if( SizeNeeded != SizeOfBitArray )
   { if( BitArray != 0 ) delete[] BitArray;
     BitArray = new unsigned char[SizeNeeded];
     //assert( BitArray != 0 );
     SizeOfBitArray = SizeNeeded;
   }

 memset( BitArray, 0, SizeOfBitArray); // Could use a loop to do this a byte at a time. 

}
//*********************************************************
//          S w i t c h   P o s n
//*********************************************************
static void SwitchPosn(int i, int j)
{
//  Calculate the location of the switch for SwitchSet and SwitchGet.

 static long BitPosition;
     BitPosition = (long)j*FirstDimension + (long)i;
     CharPosition = BitPosition/BitsPerChar;
     BitOffset = BitPosition - CharPosition*BitsPerChar;
     //assert( CharPosition <= SizeOfBitArray );
}
//*********************************************************
//            S w i t c h   S e t
//*********************************************************
int SwitchSet( int i, int j)
{
// Set's switch (i,j) on . It returns the old value of the switch.

 SwitchPosn( i, j );

 if( *(BitArray+CharPosition)&(ON<<BitOffset) ) return 1;

 *(BitArray+CharPosition) |= (ON<<BitOffset);
 return 0;

}
//***********************************************************
//             S w i t c h   G e t
//***********************************************************
int SwitchGet( int i, int j)
{
// Returns the value of switch (i,j).
 SwitchPosn( i, j );
 if( *(BitArray+CharPosition)&(ON<<BitOffset) ) return 1;
 return 0;
}

