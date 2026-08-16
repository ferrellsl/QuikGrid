//******************************************************************
//                 C O N D R A W
// Functions primarily concerned with drawing contours.
//
//                       Copyright (c) 2000 - 2001 by John Coulthard
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
// Sept. 25/00: Creation date.
//              Incorporate PaintContourReset, ContourLinesSet and
//              NiceContourLinesSet from Paintcon.
// Nov 4-8/00: Implement functions to replace direct access to ContourLines
//           array and new Custom-Colour feature.
// Nov. 9-17/00: Implement variable colours.
// Mar. 8/01: Change variable colours to use Scattered data points range. 
// Mar. 10/01: Implement Colour mapping.
// Mar. 17/01: Check maximun number of contours in ContourLinesSet.
// Jan. 15/04: Do more double precision in contour lines set.
//*******************************************************************
#include <windows.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "rc.h" 
#include "assert.h"
#include "surfgrid.h"
#include "utilitys.h"
#include "scatdata.h"
#include "saveline.h"
#include "quikgrid.h" 
using namespace std;
// define externals for contour line drawing.

double ColourZmin,
       ColourZmid,
       ColourZmax;
float CgridZadjust;
int ColourContours = FALSE;
int BlendedColours = TRUE;
int ReverseColourMap = FALSE;
const int MaxContourLines = 1000;
int NumberOfContours = 10,
    PenHighLite = 5;
  // Want to get these out of the global domain.

COLORREF ShadePen;
HPEN     hShadePen;
SaveLineType Save2dContours,
             Save3dContours;

// Declare global's needed here.

extern int LabelNDigits;  // Defined in Labelc.cpp
extern SurfaceGrid Zgrid, Cgrid;
extern ScatData ScatterData;
extern COLORREF ContourPen;

// Local storage follows

struct ContourLinesType { double Value;
                          char szLabel[20];
                          int Bold;
                          int Label;
								  COLORREF Colour; } ;

static ContourLinesType ContourLines[MaxContourLines];

static double zAdjust;

static float ColourTopRange,
             ColourBottomRange,
             ColourRange,
             NewColourTopRange,
             NewColourBottomRange,
             NewColourRange,
             NewColourZmin,
             NewColourZmid,
             NewColourZmax;
int CustomLines = 0; // Count of custom contours.


//********************************************************
//   C o n t o u r  L i n e  V a l u e
//********************************************************
float ContourLineValue( const int i )
{
 return ContourLines[i].Value;
}

void ContourLineValue( const int i, float x )
{
  assert( i < MaxContourLines );
  ContourLines[i].Value = x;
  return ;
}
//********************************************************
//   C o n t o u r  L i n e  B o l d
//********************************************************
int ContourLineBold( const int i )
{
  return ContourLines[i].Bold;
}

//********************************************************
//   C o n t o u r  L i n e  C o l o u r
//********************************************************
COLORREF ContourLineColour( const int i )  
{
  return ContourLines[i].Colour;
}

//********************************************************
//   C o n t o u r  L i n e  L a b e l
//********************************************************
// returns TRUE if the line is to be labelled.
// Currently we only label bold faced lines.
int ContourLineLabel( const int i )
{
  return ContourLines[i].Label;
}

//********************************************************
//   C o n t o u r  L i n e  L a b e l
//********************************************************
// Returns a pointer to the actual text string to
// label the contour line with.
char *szContourLineLabel( const int i )
{
  return ContourLines[i].szLabel;
}


//********************************************************
//      C o l o u r   M a p
//********************************************************
// Set the max, min and mid point for the colour mapping.
int ColourMap( const double min, const double mid, const double max)
{
   if( !( (mid >= min) && (max >= mid) ) )
    { NotifyUser( IDS_COLOURMAPPINGERROR );
    //"Colour mapping error. Max less than Min? Values ignored.");
      return FALSE;  }
   ColourZmin = min;
   ColourZmid = mid;
   ColourZmax = max;
   ColourTopRange = max - mid;
   ColourBottomRange = mid - min;
   ColourRange = max - min;
   if( Cgrid.xsize() < 3 ) 
   CgridZadjust = ScatterData.zAdjust();// For color legend labelling.
   return TRUE;
}

//********************************************************
//      D a t a  C o l o u r   M a p
//********************************************************
// Set the max, min and mid point for the colour mapping.
// Called by Initialize data inside load data.
// If Cgrid exists set aside values in New...
int DataColourMap( const double min, const double mid, const double max )
{
   if( !( (mid >= min) && (max >= mid) ) )
    { NotifyUser( IDS_COLOURMAPPINGERROR );
    //"Colour mapping error. Max less than Min? Values ignored.");
      return FALSE;  }
   if( Cgrid.xsize() > 2 )
    {
      // Just set new map aside if Cgrid exists.
      NewColourZmin = min;
      NewColourZmid = mid;
      NewColourZmax = max;
      NewColourTopRange = max - mid;
      NewColourBottomRange = mid - min;
      NewColourRange = max - min;
      return TRUE;
    }
   else ColourMap( min, mid, max );
   return TRUE;
}

//********************************************************
//      R e s t o r e   C o l o u r   M a p
//********************************************************
// Set the Colour Map - called if Cgrid is destroyed.
void RestoreColourMap(  )
{
   ColourZmin = NewColourZmin;
   ColourZmid = NewColourZmid;
   ColourZmax = NewColourZmax;
   ColourTopRange = NewColourTopRange;
   ColourBottomRange = NewColourBottomRange;
   ColourRange = NewColourRange;
   CgridZadjust = ScatterData.zAdjust();
}
//********************************************************
//      S a v e   C o l o u r   M a p
//********************************************************
// Save the Colour Map - called if Cgrid is created.
void SaveColourMap(  )
{
   NewColourZmin = ColourZmin;
   NewColourZmid = ColourZmid;
   NewColourZmax = ColourZmax;
   NewColourTopRange = ColourTopRange;
   NewColourBottomRange = ColourBottomRange;
   NewColourRange = ColourRange;
}
//********************************************************
//      C h o o s e Z C o l o u r
//********************************************************
COLORREF ChooseZColour( float zArg )
{

 // x defines (0-1) where in the range of colours the z coordinate is.
 // I define the range of colour into 4 regions...
 // Region 1: Blue is max, red is zero, green tapers from 0 to max.
 // Region 2: Green is max, red is zero, blue tapers down to 0.
 // Region 3: Green is max, red tapers up to max, blue is zero.
 // Region 4: Red is max, Green tapers to zero, blue is zero.

	 static int r, g, b;

// Region 1: goes from zero to low,
// Region 2: goes from low to mid.
// Region 3: goes from mid to high.
// Region 4: goes from high to 1.
// max is the highest saturation for a colour value (0-255)

	 static const float max = 255.,low = .33, mid = .5, high = .66;

// Define some constant multipliers to use for each region calculations.

    static const float Mult1 = 3., Mult2 = 6., Mult3 = 6., Mult4 = 3.;

// The range is divided up into thirds.
// First third (Region 1) Blue is max and dominant.
// Second third (Regions 2 & 3 ) Green is max and dominant.
// Last third (Region 4) Red is max and dominant.
// At all times only two colours are being mixed.

    static float x, z;

    z = zArg;
    if( ReverseColourMap) z = ColourZmax - ( z - ColourZmin);

    if( !BlendedColours )
    {
      // In this case return a varying intensity single colour.
      //                             0 0 0 is black
      //                             255 255 255 is white.
      // If no colour range return mid point.
      if( ColourRange <=0.0 ) return RGB( 127, 127, 127 );
      x = (ColourZmax - z)/ColourRange;
      float Gvalue, Rvalue, Bvalue;
      Rvalue = GetRValue(ShadePen);
      Gvalue = GetGValue(ShadePen);
      Bvalue = GetBValue(ShadePen);
      r =  255-x*(255-Rvalue);
      g =  255-x*(255-Gvalue);
      b =  255-x*(255-Bvalue);
      if( r < 0 ) r = 0;
	   if( g < 0 ) g = 0;
	   if( b < 0 ) b = 0;
	   if( r > 255 ) r = 255;
	   if( g > 255 ) g = 255;
	   if( b > 255 ) b = 255;
      return RGB( r , g, b );
    }

    // if zero colour range return green.
    if( ColourRange <= 0.0 ) return RGB( 0, 255, 0 );

    // Separate x out into upper or lower half to accomodate colour mapping.
    if( z > ColourZmid )
     {
      if( ColourTopRange == 0.0 ) x = 1.0;
      else  x = ( z - ColourZmid)/ColourTopRange*.5 + .5 ;
      if( x > 1.0 ) x = 1.0;
      if( x < .5  ) x = .5;
     }
    else
     {
        if( ColourBottomRange == 0.0 ) x = 0.0;
        else x = ( z - ColourZmin)/ColourBottomRange*.5;
        if( x < 0.0 ) x = 0.0;
        if( x > .5  ) x = .5;
     }
 // Region 1: Blue dominant, green increasing.
	 if( x <= low )
		 { r = 0;
			b = max;
			g = x*Mult1*max; }
 // Region 2: Green dominant, blue decreasing.
	 if( (x > low) && (x <= mid) )
		 { r = 0;
			b = (1.-(x-low)*Mult2)*max;
			g = max;}
 // Region 3: Green dominant, red increasing.
	 if( (x > mid) && (x <= high) )
		 { r = (x-mid)*Mult3*max;
         b = 0;
			g = max;}
 // Region 4: Red dominant, green decreasing.
	 if( x > high )
		 { r = max;
			b = 0;
			g = (1.-(x-high)*Mult4)*max; }

    if( r < 0 ) r = 0;
	 if( g < 0 ) g = 0;
	 if( b < 0 ) b = 0;
	 if( r > 255 ) r = 255;
	 if( g > 255 ) g = 255;
	 if( b > 255 ) b = 255;
	 return RGB( r, g, b );
}
//********************************************************
//                   A u t o   C o l o u r
//********************************************************
// Play around with schemes to assign continuous color range.
void ContourAutoColour(  )
{
  for( int i = 0; i < NumberOfContours; i ++ )
	 ContourLines[i].Colour = ChooseZColour( ContourLines[i].Value );
}

//********************************************************
//   P a i n t   C o n t o u r    R e s e t
//********************************************************
// Should be reset after any change to...
// angle, grid dimensions, contour intervals, scatter data. grid...
void PaintContourReset() { Save2dContours.Reset(); Save3dContours.Reset(); }


//********************************************************
//     C o n t o u r  L i n e s  S e t
//********************************************************
void ContourLinesSet( const double start, const double increment, const double end)
{
	assert( start <= end );
   assert( increment > 0.0 );
   CustomLines = 0;
	ContourLines[0].Value = start;
   if( (end-start)/increment > MaxContourLines )
    NotifyUser( IDS_TOOMANYCONTOURLINES );
	for( NumberOfContours = 1; NumberOfContours < MaxContourLines; NumberOfContours++ )
	 {
      //ContourLines[NumberOfContours].Value =
      //                    ContourLines[NumberOfContours-1].Value + increment;
      ContourLines[NumberOfContours].Value =
                         ContourLines[0].Value + increment*NumberOfContours;
      zAdjust = ScatterData.zAdjust();
      sprintf( ContourLines[NumberOfContours].szLabel, "%.*g", LabelNDigits,
               ContourLines[NumberOfContours].Value-zAdjust );
      ContourLines[NumberOfContours].Bold = FALSE;
      ContourLines[NumberOfContours].Label = FALSE;
      ContourLines[NumberOfContours].Colour = ContourPen;
		if(ContourLines[NumberOfContours].Value > (end+increment*.5) ||
	      ContourLines[NumberOfContours].Value > Zgrid.zmax() ) break;
	 }
   if( ColourContours ) ContourAutoColour( );
	PaintContourReset();
}

//********************************************************
//    C o n t o u r  L i n e s   S e t
//********************************************************
void ContourLinesSet()
{
  static double zDataRange, increment, start;
  if( NumberOfContours > MaxContourLines )
   {
     NumberOfContours = MaxContourLines;
     NotifyUser( IDS_TOOMANYCONTOURLINES );
   }
  assert( NumberOfContours >= 0);
  CustomLines = 0;
  zDataRange = Zgrid.zmax() - Zgrid.zmin();
  if( zDataRange <= 0.0 ) return;  // Grid is probably all undefined.
  increment = zDataRange/NumberOfContours;
  assert( increment >= 0 );

  start = Zgrid.zmin();
  ContourLines[0].Value = start;
  for( int i = 1; i < NumberOfContours; i ++ )
  {
	 //ContourLines[i].Value = ContourLines[i-1].Value + increment;
    ContourLines[i].Value = ContourLines[0].Value + increment*i;
    zAdjust = ScatterData.zAdjust();
    sprintf( ContourLines[NumberOfContours].szLabel, "%.*g", LabelNDigits,
             ContourLines[NumberOfContours].Value-zAdjust );
    ContourLines[NumberOfContours].Bold = FALSE;
    ContourLines[NumberOfContours].Label = FALSE;
    ContourLines[NumberOfContours].Colour = ContourPen;
  }
  if( ColourContours ) ContourAutoColour( );
}

//********************************************************
//    N i c e C o n t o u r  L i n e s   S e t
//********************************************************
void NiceContourLinesSet()
{
  static double zDataRange, zAdjust, increment, PrimaryIncrement, start;

  NumberOfContours = 10;
  PenHighLite = 5;
  zDataRange = Zgrid.zmax() - Zgrid.zmin();
  if( zDataRange <= 0.0 ) return;  // Grid is probably all undefined.
  zAdjust = zDataRange*.1;
  PrimaryIncrement = (zDataRange-zAdjust)/NumberOfContours*PenHighLite;
  PrimaryIncrement = AtPrecision( PrimaryIncrement, 1 );
  increment = PrimaryIncrement/PenHighLite ;
  assert( increment > 0 );

  zAdjust = ScatterData.zAdjust();
  start = (float)( (int)((Zgrid.zmin()-zAdjust)/PrimaryIncrement) )
								*PrimaryIncrement + zAdjust;

  ContourLinesSet( start, increment, Zgrid.zmax() );
}

//********************************************************
//    C o n t o u r  B o l d  L a b e l  S e t
//********************************************************
void ContourBoldLabelSet( const int EveryNth )
{
   for( int i = 1; i < NumberOfContours; i ++ )
   {
     if( i%EveryNth == 0 ) ContourLines[i].Bold = TRUE;
     else                  ContourLines[i].Bold = FALSE;
     ContourLines[i].Label = ContourLines[i].Bold;
   }
}

//********************************************************
//    C o n t o u r  C u s t o m
//********************************************************
// Sets a custom contour line.
void ContourCustom( const double Value, const int Bold,
                    const int Label, const char szLabel[], const COLORREF Colour )
{
  assert( CustomLines < MaxContourLines );
  ContourLines[CustomLines].Value = Value;
  ContourLines[CustomLines].Bold  = Bold;
  ContourLines[CustomLines].Label = Label;
  ContourLines[CustomLines].Colour = Colour;
  if( szLabel[0] == ' ' )
  {
    zAdjust = ScatterData.zAdjust();
    sprintf( ContourLines[CustomLines].szLabel, "%.*g", LabelNDigits,
             Value-zAdjust );
  }
  else
   {
     ContourLines[CustomLines].Label = TRUE;
     strncpy( ContourLines[CustomLines].szLabel, szLabel, sizeof(ContourLines[0].szLabel) );
   }
  CustomLines++;
  NumberOfContours = CustomLines;
  ColourContours = TRUE;
  return;
}
