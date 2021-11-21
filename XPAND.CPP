//*******************************************************************
//                              X P A N D
//*******************************************************************

// Function to generate a rectangular grid representing the surface
// presented by a set of scattered 3d data points. The use of functions
// XpandInit and XpandPoint are to allow generation of the grid in
// the background under MS Windows 3.1 .
//
//                       Copyright (c) 1993 - 2007 by W. John Coulthard
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
// April 12/99: Changed to user newer Gridxtype class. Xpand now passes
//              the size of the grid to be generated through.
//              Commented out "assert" statements.
// April 15/2002: Increase PercentEdge and Density ratio limit to 10,000.
//
// Feb. 27/2007 - Many thanks to Matt Gessner, a software engineer from Ohio
//              who identified an error in the SelectPoints function.
//              The calculation of the Start and End variables was not being
//              done correctly for all situations in the previous version.
//              This caused an array overflow in the gridxtyp class.
//*********************************************************************
//                                     
#include <windows.h>
#include <iostream> 
#include <math.h>
#include <stdlib.h>
//#include "assert.h"
#include "surfgrid.h"   // The generated grid. 
#include "scatdata.h"   // The scattered data input data.
#include "xpand.h"      // Defines Xpand entry points.
#include "gridxtyp.h"   // Lookup tables for grid expansion.

// Forward declare some functions used by Xpand.
 static void SelectPoints( ScatData &Data, SurfaceGrid &Grid);
 static float WeightedAverage( ScatData &Data);
 static int IncludeGridPoint();
 static void LocateInit( ScatData &Data, SurfaceGrid &Grid );
 static int  LocateGridX( int &i, int &j, float, float, SurfaceGrid &Grid);
 static void PutInOctant( const float x, const float y, ScatData &Data, long DataIndex);


// Shared Variables. 
 static int NumXcoords,
	    NumYcoords,
	    xIndex,
	    yIndex,
	    PercentEdgeFactor = 100,
	    PercentDensityRatio = 150;

 static float PointDistSquared[8],   // Distance**2 from point.
              ClosestSquared,        // Overall closest point.
              ScanStopRatio = 16.,
	      DensityStopRatio = 1.5,
	      EdgeSenseFactor = 1.,
              EdgeSenseDistSq, 
	      Radius,
	      Diameter,
              DiameterSq, 
	      xyStopDistSq,
	      xGridMax, yGridMax, xGridMin, yGridMin,
	      UndefinedZ = -99999.;

 static long  PointInOctant[8],  // Number of point closest in Octant
              TotalRange,
	      NumberDone,
	      TotalShells,
	      NumDataPoints,
	      NoFound,
	      TotalGridPoints,
	      nSample = 1;          // Sample every n'th point.

 static GridXType Locator(0, 0, 0);

 const float pi = 3.14159; 

//**************************************************************
//        X p a n d
//                     Generate the entire grid in one call. 
//*************************************************************
void Xpand( SurfaceGrid &Zgrid, ScatData &RandomData)
{
 XpandInit( Zgrid, RandomData );
 while( XpandPoint( Zgrid, RandomData) ) ;
}

//******************************************************************
//                    X p a n d   P o i n t
//
//  Evaluates one grid intersection only and advances indexes so
//       the next call will evaluate the next one. 
//******************************************************************
int XpandPoint( SurfaceGrid &Zgrid, ScatData &RandomData )
{  static float Zvalue; 
	//assert( xIndex < NumXcoords );
   //assert( yIndex < NumYcoords );
   if( NumDataPoints < 3 ) return 0;

   // Select the closest point in each octant surround the grid point.
   SelectPoints( RandomData, Zgrid) ;

   // Check if point can be included and if so.. calculate it. 
   if ( IncludeGridPoint() > 0 ) Zvalue = WeightedAverage(RandomData); 
   else                          Zvalue = UndefinedZ;

   Zgrid.zset(xIndex, yIndex, Zvalue ) ;

   // Move to next grid intersection....
   ++xIndex;   
   if( xIndex >= NumXcoords )  { ++yIndex; xIndex= 0; } 

	if( yIndex >= NumYcoords ){ Locator.New(0, NumXcoords, NumYcoords); return 0; }
   ++NumberDone;
   //assert( NumberDone <= TotalGridPoints);

   return 1; 
}

//***************************************************************
//     Set and Query  Xpand  Status  and parameters
//**************************************************************
int XpandPercentDone( ) {  return (NumberDone*100)/TotalGridPoints; }

int XpandBandWidth()
    { if( NumberDone == 0 ) return 0; 
      return TotalRange/NumberDone; }

int XpandPercentShell()
    { if( NumberDone == 0 ) return 0; 
      return TotalShells/NumberDone; }

int XpandScanRatio() { return ScanStopRatio; }
int XpandScanRatio( const int NewRatio )
  { float OldRatio = ScanStopRatio;
    ScanStopRatio=NewRatio;
    if( ScanStopRatio < 1. ) ScanStopRatio = 1.;
    if( ScanStopRatio > 100. ) ScanStopRatio = 100.;
    return OldRatio; }

int XpandDensityRatio() { return PercentDensityRatio; }
int XpandDensityRatio(  const int NewRatio )
   { int OldRatio = PercentDensityRatio;
     PercentDensityRatio = NewRatio;
     if( PercentDensityRatio < 1 ) PercentDensityRatio = 1;
     if( PercentDensityRatio > 10000) PercentDensityRatio = 10000;
     DensityStopRatio = (float) PercentDensityRatio*.01;  
     return OldRatio; }

int XpandEdgeFactor() { return PercentEdgeFactor; }
int XpandEdgeFactor(const int NewFactor )
    { int OldFactor = PercentEdgeFactor;
      PercentEdgeFactor = NewFactor;
      if( PercentEdgeFactor < 1 ) PercentEdgeFactor = 1;
      if( PercentEdgeFactor > 10000 ) PercentEdgeFactor = 10000;
      EdgeSenseFactor = (float) PercentEdgeFactor*.01; 
      return OldFactor;
    }
float XpandUndefinedZ() { return UndefinedZ; }
float XpandUndefinedZ( const float z )
{ float OldZ = UndefinedZ;
  UndefinedZ = z;
  return OldZ; 
}
long XpandSample() { return nSample; }
long XpandSample( const long i )
{
  long oldSample = nSample;
  nSample = i;
  if( nSample < 1 ) nSample = 1;
  return oldSample; 
}
//***************************************************************
//            X p a n d   I n i t
//***************************************************************
void XpandInit( SurfaceGrid &Zgrid, ScatData &Data )
{
 NumXcoords = Zgrid.xsize();
 NumYcoords = Zgrid.ysize();
 
 TotalGridPoints = (long)NumXcoords*(long)NumYcoords;
 xIndex = yIndex = 0;
 NumberDone = TotalRange = TotalShells = 0;
 NumDataPoints = Data.Size();
 if( NumDataPoints < 3 ) return; 

 xGridMin = Zgrid.xmin();
 xGridMax = Zgrid.xmax();
 yGridMin = Zgrid.ymin();
 yGridMax = Zgrid.ymax(); 
 float xRange = Data.xMax()-Data.xMin();
 float yRange = Data.yMax()-Data.yMin();
 float Volume = xRange*yRange;
 float VolPerPoint = Volume/((float)NumDataPoints/(float)nSample);
 Radius = sqrt( VolPerPoint/pi);
 Diameter = Radius*2; 
 DiameterSq = Diameter*Diameter;
 xyStopDistSq = DiameterSq*DensityStopRatio*DensityStopRatio;
 EdgeSenseDistSq = DiameterSq*EdgeSenseFactor*EdgeSenseFactor;

 LocateInit( Data, Zgrid ); 

}

//**********************************************************************
//             S c a n    O n e   G r i d
//**********************************************************************
static void ScanOneGrid( int i, int j, ScatData &Data, SurfaceGrid &Grid)
{
 static long GotOne; 
 for( long n = 0; TRUE; n++) 
 {
  GotOne = Locator.Search(i, j, n);
  if( GotOne < 0 ) return;
  PutInOctant( Grid.x(xIndex), Grid.y(yIndex), Data, GotOne );
  TotalRange++;
 } 
}

//****************************************************************
//                 P u t   I n    O c t a n t
//****************************************************************
static void PutInOctant( const float x, const float y, ScatData &Data, long DataIndex)
{  
   float Xdistance =  Data.x(DataIndex) - x;
   float Ydistance =  Data.y(DataIndex) - y;

   // Select the octant the data point is in. They are labeled:
   //        31
   //       2  0
   //       6  4
   //        75

   int Octant = 0;
   if( Ydistance < 0.0 ) Octant = 4;
   if (Xdistance < 0.0 ) Octant += 2;
   if ( fabs(Xdistance) < fabs(Ydistance) ) Octant += 1;

   // and get the distance squared and save that information
   // for that Octant iff this is the first point found or closer than
   // the point previously saved for that octant.

   float DistSquared = Xdistance*Xdistance + Ydistance*Ydistance;
   if( NoFound == 0 ) ClosestSquared = DistSquared;
   if( DistSquared < ClosestSquared) ClosestSquared = DistSquared;
   NoFound++;

   if ( PointInOctant[ Octant ] == -1 ||
	DistSquared < PointDistSquared[Octant] )
	{
	  PointInOctant[ Octant] = DataIndex ;
	  PointDistSquared[Octant] = DistSquared;
	}
}

// ***************************************************************
//                    S e l e c t    P o i n t s
//*****************************************************************
static void SelectPoints( ScatData &Data, SurfaceGrid &Grid)

// This routine will search the array of Data points looking for
// the closest point in each of the 8 octants surrounding the grid
// coordinate currently being evaluated (at location x, y).

{
 int Start,
     End,
     Row,
     Column;

 float TestDist;

 int TopDone = 0;      // These are logical flags controlling the 
 int BottomDone = 0;   // Shelling process. 
 int LeftDone = 0;
 int RightDone = 0;
 int i; 

// Zero out the arrays which keep track of closest point and its distance.
 for( i = 0; i< 8; i++ ){ PointInOctant[i] = -1;
			      PointDistSquared[i] = 0.0; }
 NoFound = 0;

 ScanOneGrid( xIndex, yIndex, Data, Grid ); // Do home grid first.

 for( int shell = 1; 1; shell++ ) // Now shell outwards from home. 
 {
  Start = xIndex - shell; if( Start < 0 ) Start = 0;
  End   = xIndex + shell+1; if( End > NumXcoords) End = NumXcoords;
 // Do top row.
   if( !TopDone )
   { 
     Row = yIndex + shell;
     if( Row >= NumYcoords )TopDone=1; else
      {
       TestDist = Grid.y(Row) - Grid.y(yIndex);
       TestDist = TestDist*TestDist;
       if( ((NoFound > 0) &&((TestDist > ClosestSquared*ScanStopRatio))||
         (TestDist > xyStopDistSq))) TopDone = 1; else 
         for( i=Start; i<End; i++) ScanOneGrid( i, Row, Data, Grid );
      }
    }

 // Do bottom row.
   if( !BottomDone )
   {
     Row = yIndex - shell;
     if( Row < 0 ) BottomDone = 1; else
      {
       TestDist = Grid.y(yIndex) - Grid.y(Row);
       TestDist = TestDist*TestDist;
       if( ((NoFound>0)&&((TestDist > ClosestSquared*ScanStopRatio))||
           (TestDist > xyStopDistSq)) ) BottomDone = 1; else 
           for( i = Start; i<End; i++) ScanOneGrid( i, Row, Data, Grid );
      }
   }
  Start = yIndex-shell+1; if( Start < 0 ) Start = 0;
  End   = yIndex+shell;   if( End > NumYcoords) End = NumYcoords;
 // Do left column.
   if( !LeftDone )
   {
     //Start = yIndex-shell+1; if( Start < 0 ) Start = 0;
     //End   = yIndex+shell;   if( End > NumYcoords) End = NumYcoords;
     Column = xIndex - shell;
     if( Column < 0 ) LeftDone = 1; else
     {
       TestDist = Grid.x(xIndex) - Grid.x(Column);
       TestDist = TestDist*TestDist;
       if( ((NoFound>0)&&((TestDist > ClosestSquared*ScanStopRatio))||
           (TestDist > xyStopDistSq)) ) LeftDone = 1; else 
	    for( i = Start; i<End; i++) ScanOneGrid( Column, i, Data, Grid);
     }
   }
 // Do right column.
   if( !RightDone )
   {
     Column = xIndex + shell;
     if( Column >= NumXcoords ) RightDone=1; else
     {
       TestDist = Grid.x(Column) - Grid.x(xIndex);
       TestDist = TestDist*TestDist;
       if( ((NoFound>0)&&((TestDist > ClosestSquared*ScanStopRatio))||
           (TestDist > xyStopDistSq)) ) RightDone = 1; else 
	   for( i = Start; i<End; i++) ScanOneGrid( Column, i, Data, Grid);
     }
   }
   if( TopDone&&BottomDone&&LeftDone&&RightDone )
       { TotalShells += shell; break; }
 }

}

//************************************************************
//   I n c l u d e   G r i d   P o i n t
//************************************************************
static int IncludeGridPoint( )
{
// Routine to decide whether to evaluate a Grid point.
// Return 0 = reject; 1 = accept.

 static const int LookUpTable[11] = {0,1,3,2,6,7,5,4,0,1,3}; 

 if( NoFound == 0 )  return 0; // No points found.
 if( ClosestSquared > xyStopDistSq ) return 0; // Closest too far away
 if( ClosestSquared < EdgeSenseDistSq ) return 1; // 

 int NumConsecutiveOctants = 0; // code looks for 4 consecutive
 for ( int i = 0; i<11; i++)    // empty octants.
   {
    if ( PointInOctant[ LookUpTable[i] ] > -1 ) NumConsecutiveOctants = 0;
    else { NumConsecutiveOctants += 1;
	   if (NumConsecutiveOctants == 4 ) return 0 ; }
   }
 return 1;
}

//************************************************************
//       W e i g h t e d    A v e r a g e
//************************************************************
static float WeightedAverage( ScatData &Data)

// function calculates the weighted average value for the
// specified grid point. The needed data is all in PointInOctant
// and PointDistSquared.


{
 float SumSquared = 0.0;
 float SumWeights = 0.0;
 for( int i = 0; i<8; i++)
 {
  long Point = PointInOctant[i];
  if ( Point > -1 )
  {
     float DistanceSquared = PointDistSquared[i] ;
			 // Check for point right on a grid intersection.
     if( DistanceSquared == 0.0 ) return Data.z(Point);
     SumSquared += 1.0/DistanceSquared;
     SumWeights += (1.0/DistanceSquared)*Data.z(Point);
  }
 }
 return SumWeights/SumSquared;
}

//**************************************************************
//             L o c a t e    I n i t
//**************************************************************
static void LocateInit( ScatData &Data, SurfaceGrid &Grid )
{
  int ix, iy;
  Locator.New( NumDataPoints, NumXcoords, NumYcoords );

  for( long i = 0; i < NumDataPoints; i += nSample )
  {
  if( !(Data.flags(i)&1) ) // check the ignore bit in the flags. 
   {
    if( LocateGridX( ix, iy, Data.x(i), Data.y(i), Grid ) )
           Locator.setnext( i, ix, iy );
   }
  }
  Locator.Sort();
}

//**************************************************************
//             L o c a t e   G r i d   X
//**************************************************************
static int LocateGridX( int &ix, int &iy, float xLocn, float yLocn,
                              SurfaceGrid &Grid )
{
// Finds the closest grid intersection to data point i.
// Return's 1 if successful,
//          0 if data point is too far away from the intersection. 
  static long test,
	      nx, ny,
	      top,
	      bot;
  static float distance;
  
  // check to see if point too far outside the grid perimeter.
  distance = xGridMin - xLocn;
  if( xLocn < xGridMin && distance*distance > xyStopDistSq ) return 0;
  distance = xGridMax - xLocn;
  if( xLocn > xGridMax && distance*distance > xyStopDistSq ) return 0;
  distance = yGridMin - yLocn;
  if( yLocn < yGridMin && distance*distance > xyStopDistSq ) return 0;
  distance = yGridMax - yLocn;
  if( yLocn > yGridMax && distance*distance > xyStopDistSq ) return 0;

  
  // Binary search for closest in x. 
  nx = NumXcoords;
  top = nx-1;
  bot = 0;

  while ( (top-bot) > 1 )
  {
    test = (top-bot)/2 + bot;
    if( xLocn > Grid.x(test)) bot = test;
    else                      top = test;
  }

  float dist = fabs( xLocn-Grid.x(test) );
  if( test < (nx-1) ) 
     if( fabs( xLocn-Grid.x(test+1) ) < dist ) test++;
  if( test > 0 ) 
     if( fabs( xLocn-Grid.x(test-1) ) < dist ) test--;
  //assert( test >= 0 && test < nx );
  ix = test;

  // Do the same for closest in y. 
  ny = NumYcoords;
  top = ny - 1;
  bot = 0;
  while ( (top-bot) > 1 )
  {
    test = (top-bot)/2 + bot;
    if( yLocn > Grid.y(test)) bot = test;
    else                      top = test;
  }
  dist = fabs( yLocn - Grid.y(test));
  if( test < (ny-1) ) 
     if( fabs( yLocn-Grid.y(test+1) ) < dist ) test++;
  if( test > 0 ) 
     if( fabs( yLocn-Grid.y(test-1) ) < dist ) test--;
  //assert( test >= 0 && test < ny);
  iy = test;
  return 1; 
}

