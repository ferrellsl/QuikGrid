//****************************************************************
//                      L a b e l c . c p p
// Routines to assist in handling labelling for contours.
//                Copyright (c) 1999-2000 by John Coulthard
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
// Mar. 7/99:  Created.
// Mar. 7/99:  Fine tuned.
// Mar. 10/99: Change centering of label - TrueType fonts.
// Mar. 13/99: And more fine tuning and playing with parameters.
// Nov. 5/00: Convert to get labels using a contour line index
//            (Preparation for custom contour line labels).
//****************************************************************
#include <windows.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <stdio.h> 
#include "utilitys.h"
#include "scatdata.h"
#include "condraw.h" 
using namespace std;
extern ScatData ScatterData;
int LabelNDigits = 5;
int LabelDistance = 1200;

struct LabelType { long x; long y; int ContourLine;} ;
LabelType LabelStore[200];

static long Distance,
            FirstDistance = 200*200,   // Distance to move after start of line
			   BigDistance = LabelDistance*LabelDistance,
			   SmallDistance = 350*350, // Separate labels by this amount.
            oldx, oldy ;

static int NumberOfLabels = 0,
			  First = TRUE,
			  Active = FALSE;  // Distance travelled to trigger label.

static float oldContourLine;

//********************************************************************
//          L a b e l c   O K
//********************************************************************
// Check to see if this label is far away enough from other labels
// To plot.
static int LabelcOK( int x, int y, int ContourLine )
{
	static int i;
   static long xd, yd, d;
   BigDistance = LabelDistance*LabelDistance;

   xd = x - oldx; yd = y - oldy;
   d = xd*xd + yd*yd;  // d = distance squared.
	// First check to see if far enough from label on same line.
   if( d < Distance ) return FALSE;
	// Now check to see if far enough from all other labels.
   for( i = 0; i<NumberOfLabels; i++ )
	{
    xd = x - LabelStore[i].x; yd = y - LabelStore[i].y; d = xd*xd + yd*yd;
    if( d < SmallDistance ) return FALSE;
	 // if same contour line it has to be even farther away.
	 if( (ContourLine == LabelStore[i].ContourLine) && (d < Distance) )  return FALSE;
	}
   return TRUE;
}
//********************************************************************
//          L a b e l c   I n i t
//********************************************************************
void LabelcInit(  )
{
  NumberOfLabels = 0;
  First = TRUE;
  Active = FALSE;
  Distance = FirstDistance;
}

//********************************************************************
//          L a b e l c   S t o p
//********************************************************************
void LabelcStop(  )
{
	 Active = FALSE;
}

//********************************************************************
//          L a b e l c   S t a r t
//********************************************************************
void LabelcStart(  )
{
	 Active = TRUE;
}

//********************************************************************
//          L a b e l c   I n t e r c e p t
//********************************************************************
// If distance moved is greater than "distance" save value for display later.
void LabelcIntercept( int x, int y, int Pen, int ContourLine )
{
	static int Notified = FALSE;
	if( !Active ) return;
	if( First )
	 {
		First = FALSE;
		Notified = FALSE;
		oldx = x, oldy = y;
//      oldValue = Value;
      oldContourLine = ContourLine;
      Distance = FirstDistance;
		return;
	 }
	if( Notified ) return; // Storage overflowed and message given.
//    if( Pen == 0 || Value != oldvalue )   // Indicates new contour segment started.
    if( Pen == 0 || ContourLine != oldContourLine )
	 {
	 	oldx = x, oldy = y;
//      oldValue = Value;
      oldContourLine = ContourLine;
      Distance = FirstDistance;
		return;
	 }
//	if( !LabelcOK( x, y, Value ) ) return;
   if( !LabelcOK( x, y, ContourLine ) ) return;
	Distance = BigDistance;
	NumberOfLabels ++;
	if( NumberOfLabels > sizeof(LabelStore) )
	 {
		NotifyUser( "Storage for contour labels full");
		Notified = TRUE;
		return;
	 }
	LabelStore[NumberOfLabels-1].x = x;
	LabelStore[NumberOfLabels-1].y = y;
//	LabelStore[NumberOfLabels-1].value = Value;
   LabelStore[NumberOfLabels-1].ContourLine =  ContourLine;
//   oldx = x, oldy = y; oldvalue = Value;
   oldx = x, oldy = y; oldContourLine = ContourLine;
}

//********************************************************************
//          L a b e l c   D i s p l a y
//********************************************************************
void LabelcDisplay( HDC &hdc )
{
  static int i, TextAlign;
//  static char string[30];
//  static float zAdjust;
  static SIZE size;
  GetTextExtentPoint32( hdc, "123", 3, &size ); // get height in logical units.
//  zAdjust = ScatterData.zAdjust();
  TextAlign = SetTextAlign( hdc, TA_CENTER );

  for( i = 0; i<NumberOfLabels; i++ )
	{
	 //sprintf( string, "%.*g", LabelNDigits, LabelStore[i].value-zAdjust );
    //TextOut( hdc, LabelStore[i].x, LabelStore[i].y-(size.cy/2), string, strlen(string) );
    TextOut( hdc, LabelStore[i].x, LabelStore[i].y-(size.cy/2),
             szContourLineLabel(LabelStore[i].ContourLine),
      strlen(szContourLineLabel(LabelStore[i].ContourLine)) );
	}
  TextAlign = SetTextAlign( hdc, TextAlign); // restore text alignment
}