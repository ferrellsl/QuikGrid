//***********************************************************
//                  D X F W R I T E  
//  Save display in DXF output format. 
//                Copyright (c) 1997 - 2002  by John Coulthard
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
// Jan. 8/97: Changed to select .dxf in file dialog box.
// April 13/98: Add hooks to output DXF polylines.
// April 29/98: Modularize DXF Layer name code.
//              Implement DXF polyline support.
// Oct. 12/99: Change so calling routine supplies filename.
// Jan. 29/00: Change to put scattered data points on special layers.
// Feb. 2/00:  Refine schema to choose layer name.
// Feb. 11/02: Include contour labels.
//             and Bold face lines.
// Aug. 10/02: Restored "70 32" flags for polyline vertex
//             (but why did I delete them?). 
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
#include "contour.h"
#include "condraw.h"
using namespace std;
// Externals...
// Sgrid defines a wind or current speed grid.
// Dgrid defines the wind or current direction.
extern SurfaceGrid Sgrid, Dgrid;
extern ScatData OutLine;
extern int BoldOutline;
extern int HighDXFOutputPrecision;  // defined in DXF3DFACE.
extern int LabelDistance;
extern float ArrowSize;

// Globals...
int UsePolyline = FALSE,
    UseScatteredDataLayers = FALSE,
    NeverLabelDXFContours = FALSE;

float DXFTextHeightRatio = .02;

// Local variables...
static ofstream DXFoutFile(" ", ios::out );

static double dxnormalize, dynormalize,
              Thickness,
              GapStartX, GapStartY, GapEndX, GapEndY,
              GapHeight, GapWidth;

static float zadjust,
				 DXFxold, DXFyold, DXFzold,
             ContourValue,
             DXFTextHeight,
             xold, yold, zold;

static int DXFOutputFailed = FALSE,
           CurrentContourLineNumber,
			  PolyLining = FALSE,
			  NumberOfVertices = 0,
			  nx, ny,
           ContourLine;

static char DXFLayerName[255],
            szFile[256];

static bool BoldLine = false,
            Gapping = false;

//******************************************************************
//         O u t p u t  D X F  T e x t
//******************************************************************
static void OutputDXFText( char Text[], double x, double y, float z )
{
 DXFoutFile
    << "  0"           << endl
    << "TEXT"          << endl
    << "  8"           << endl
    << DXFLayerName    << endl
    << " 10"           << endl
    << (x+dxnormalize) << endl
    << " 20"           << endl
    << (y+dynormalize) << endl
    << " 30"           << endl
    << (z-zadjust)     << endl
    << " 40"           << endl
    << DXFTextHeight   << endl
    << "  1"           << endl
    << Text            << endl;
}

//**************************************************************
//     O u t p u t D X F L i n e 
//**************************************************************
// Output a DXF line - also provide service of adjusting
// x,y,z values before output. 
static void OutputDXFLine(float x, float y, float z,
	 float x2, float y2, float z2 )
{
	DXFoutFile
          << "  0"           << endl
			 << "LINE"          << endl
			 << "  8"           << endl
			 << DXFLayerName    << endl;

   if( BoldLine )
     DXFoutFile
          << " 39"           << endl  // Width.
          << Thickness       << endl;
          
	DXFoutFile
          << " 10"                    << endl
			 << ((double)x+dxnormalize)  << endl
			 << " 20"                    << endl
			 << ((double)y+dynormalize)  << endl
			 << " 30"                    << endl
			 << (z-zadjust)              << endl;

	DXFoutFile
          << " 11"                    << endl  // next vertice.
			 << ((double)x2+dxnormalize) << endl
			 << " 21"                    << endl
			 << ((double)y2+dynormalize) << endl
			 << " 31"                    << endl
			 << (z2-zadjust)             << endl;

}
//******************************************************************
//         C o n t i n u e P o l y L i n e
//******************************************************************
static void ContinuePolyLine( float x, float y, float z )
{
  assert( PolyLining == TRUE );
  DXFoutFile
			 << "  0"           << endl
			 << "VERTEX"        << endl
			 << "  8"           << endl
			 << DXFLayerName    << endl ;

  DXFoutFile
			 << " 10"           << endl
			 << ((double)x+dxnormalize)   << endl
			 << " 20"           << endl
			 << ((double)y+dynormalize)   << endl
			 << " 30"           << endl
			 << (z-zadjust)     << endl

			 << " 70"           << endl  // Flags
			 << "    32"        << endl; // 3d Polyline vertex.

  if( BoldLine ) DXFoutFile
          << " 39"           << endl  // Width.
          << Thickness       << endl
          << " 40"           << endl  // Width.
          << Thickness        << endl
          << " 41"           << endl  // Width.
          << Thickness        << endl;

  NumberOfVertices += 1;
  DXFxold = x; DXFyold = y; DXFzold = z;
}
//******************************************************************
//         E n d P o l y L i n e
//******************************************************************
static void EndPolyLine ( )
{
	if( PolyLining )
  {
	  if( NumberOfVertices < 2 )
		 ContinuePolyLine( DXFxold, DXFyold, DXFzold );

	  DXFoutFile
			 << "  0"           << endl
			 << "SEQEND"        << endl
			 << "  8"           << endl
			 << DXFLayerName    << endl;
  }
  PolyLining = FALSE ;
  NumberOfVertices = 0;
}

//******************************************************************
//         I n i t P o l y L i n e
//******************************************************************
static void InitPolyLine( float x, float y, float z )
{
  if( PolyLining ) EndPolyLine();

  DXFoutFile
			 << "  0"           << endl
			 << "POLYLINE"      << endl
			 << "  8"           << endl
			 << DXFLayerName    << endl
			 << " 66"           << endl  // Vertices Follow flag.
			 << "     1"        << endl 

			 << " 70"           << endl  // Flags are...
			 << "     8"        << endl; // 3d Polyline.;

  if( BoldLine ) DXFoutFile
          << " 39"           << endl  // Width.
          << Thickness        << endl
          << " 40"           << endl  // Width.
          << Thickness        << endl
          << " 41"           << endl  // Width.
          << Thickness        << endl;

  PolyLining = TRUE;
  NumberOfVertices = 0;
  ContinuePolyLine( x, y, z ) ;

}


//******************************************************************
//         G a p  I n i t
//******************************************************************
static void GapInit( double x, double y )
{
  Gapping = true;
  GapStartX = x;
  GapStartY = y;
  GapHeight = DXFTextHeight;
  GapWidth = GapHeight*strlen( szContourLineLabel(ContourLine));
  EndPolyLine();
}

//******************************************************************
//         G a p  P r o c e s s
//******************************************************************
static bool GapProcess( double x, double y)
{
  static double Xpos, Ypos,
                Xdiff, Ydiff,
                XdiffAbs, YdiffAbs;
  if( !Gapping) return false;

  // Determine if moved enough to create a gap for the label.

  Xdiff = GapStartX-x;
  Ydiff = GapStartY-y;
  XdiffAbs = fabs( Xdiff );
  YdiffAbs = fabs( Ydiff );
  if( XdiffAbs < GapWidth && YdiffAbs < GapHeight ) return true;

  GapEndX = x;   // Initially just delete the vector and let it go.
  GapEndY = y;

  // Place the label in the gap - in the centre between
  // Start and End.

  Xpos = GapStartX + (GapEndX-GapStartX)*.5 - GapWidth*.5;
  Ypos = GapStartY + (GapEndY-GapStartY)*.5 - GapHeight*.5;

  OutputDXFText( szContourLineLabel(ContourLine),
                              Xpos, Ypos, ContourValue);

   if( UsePolyline ) InitPolyLine( GapEndX, GapEndY, ContourValue );
   xold = GapEndX;
   yold = GapEndY;
   zold = ContourValue;
   Gapping = false;
   return false;
}


//******************************************************************
//         L a b e l   D X F C o n t o u r   L i n e s
//******************************************************************
struct LabelType { double x; double y; int ContourLine;} ;
static LabelType LabelStore[4000];

static double Distance,
              dLabelDistance,
              FirstDistance, // Distance to move after start of line
			     BigDistance,
			     SmallDistance, // Separate labels by this amount.
              oldx, oldy;

static int NumberOfLabels = 0;

static bool ContourLabelling, // Local flag to indicate if labelling wanted.
            First = true,
			   Active = false;  // Distance travelled to trigger label.

static float oldContourLine;

//********************************************************************
//          L a b e l    O K
//********************************************************************
// Check to see if this label is far away enough from other labels
// To plot.
static int LabelOK( double x, double y, int ContourLine )
{
	static int i;
   static double xd, yd, d;

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
//          L a b e l    I n i t
//********************************************************************
static void LabelInit(  )
{
  static double LabelDistanceRatio;
  NumberOfLabels = 0;
  First = true;
  Active = false;
  LabelDistanceRatio = LabelDistance/5300.0;
  //NotifyUser( "Label distance ratio %g " , (float) LabelDistanceRatio );
  //dLabelDistance = (ScatterData.yMax()-ScatterData.yMin() )*.3;
  dLabelDistance = (ScatterData.yMax()-ScatterData.yMin() )*LabelDistanceRatio;
  //FirstDistance = (dLabelDistance*LabelDistanceRatio*.5)*(dLabelDistance*LabelDistanceRatio*.5);
  FirstDistance = (dLabelDistance*.15)*(dLabelDistance*.15);
  SmallDistance = FirstDistance*2.;
  Distance = FirstDistance;
  BigDistance = dLabelDistance*dLabelDistance;
}

//********************************************************************
//          L a b e l    S t o p
//********************************************************************
static void LabelStop(  )
{
	 Active = false;
}

//********************************************************************
//          L a b e l    S t a r t
//********************************************************************
static void LabelStart(  )
{
	 Active = true;
}

//********************************************************************
//          L a b e l   I n t e r c e p t
//********************************************************************
// If distance moved is greater than "distance" display contour and save location.
// so another label is not placed too close to it.
static void LabelIntercept( double x, double y, int draw )
{
	static bool Notified = false;
	if( !Active ) return;
   if( Gapping ) return;
	if( First )
	 {
		First = false;
		Notified = false;
		oldx = x, oldy = y;
      oldContourLine = ContourLine;
      Distance = FirstDistance;
		return;
	 }
	if( Notified ) return; // Storage overflowed and message given.
    if( draw == 0 || ContourLine != oldContourLine )
	 {
	 	oldx = x, oldy = y;
      oldContourLine = ContourLine;
      Distance = FirstDistance;
		return;
	 }
   if( !LabelOK( x, y, ContourLine ) ) return;
	Distance = BigDistance;
	NumberOfLabels ++;
	if( NumberOfLabels > sizeof(LabelStore) )
	 {
		NotifyUser( "Storage for DXF contour line labels full");
		Notified = true;
		return;
	 }
	LabelStore[NumberOfLabels-1].x = x;
	LabelStore[NumberOfLabels-1].y = y;
   LabelStore[NumberOfLabels-1].ContourLine =  ContourLine;
   oldx = x, oldy = y;
   oldContourLine = ContourLine;

   // We can put a label here so start creating a gap in the contour line for it.

   if( UsePolyline )   
    {
       if( draw ) ContinuePolyLine( x, y, ContourValue ); // draw up to here though.
    }
   else
    {
       if( draw ) OutputDXFLine( xold, yold, zold, x, y, ContourValue);
    }
   xold = x;
   yold = y;
   zold = ContourValue;
   GapInit( x, y );

}

//******************************************************************
//         S e t   D X F  L a y e r   N a m e
//******************************************************************
static void SetDXFLayerName( char Name[] )
{
  strcpy( DXFLayerName, Name );
}
static void SetDXFLayerName( char Name[], float Value )
// This version concatenates the "Value" as a floating point
// number to the text. The floating point number is then manipulated
// to make it a legal DXF layer name.
{
	  ostrstream Buf;
	  Buf << Name << Value << ends ;
	  char *szBuf = Buf.str();
	  char *ptr;

	  ptr = strstr( szBuf, "e+00" );  // truncate zero exponent
	  if( ptr ) *ptr = '\0';
	  ptr = strchr( szBuf, '.' );  // convert . to _..
	  if( ptr ) *ptr = '_';
	  ptr = strchr( szBuf, '-' );  // convert - to m.
	  if( ptr ) *ptr = 'm';
	  ptr = strchr( szBuf, '-' );  // convert - to m. again for exponent
	  if( ptr ) *ptr = 'm';
	  ptr = strchr( szBuf, '+' );  // convert + to _.
	  if ( ptr ) *ptr = '_';

	  strcpy( DXFLayerName, szBuf );

	  delete szBuf;
}
//******************************************************************
//         A b o r t   W r i t e  D X F  F i l e  
//******************************************************************
static void AbortWriteDXFFile( char FileName[] )
{
  ostrstream Buf;

  Buf << "Error writing DXF output to the file "
	   << FileName 
           << ". Output terminated. " << ends; 

  char *szBuf = Buf.str();
  NotifyUser( szBuf );
  delete szBuf;

  RestoreCursor();

}


//**************************************************************
//       O u t p u t  D X F  L I N E  G r i d
//**************************************************************
static int OutputDXFLINEgrid()
{
	 for ( int i = 0; i<(nx-1); i++ )
	 {
		for (int j = 0; j<(ny); j++ )
		{
		  if( ( Zgrid.z(  i,   j) >= 0.0 ) &&
				( Zgrid.z(i+1,   j) >= 0.0 )     )
		  {
			OutputDXFLine( Zgrid.x(i), Zgrid.y(j), Zgrid.z(i,j),
				 Zgrid.x(i+1), Zgrid.y(j), Zgrid.z(i+1, j));
			if( DXFoutFile.fail() ){ AbortWriteDXFFile( szFile );
				 DXFoutFile.close();
				 return 0; }
		  } // end if... Zgrid.z...
		} // end for  (int j...
	 }  // end for  (int i....

	for ( int j = 0; j<(ny-1); j ++ )
	{
	  for (int i = 0; i<(nx); i++ )
	  {
		 if( ( Zgrid.z(  i,   j  ) >= 0.0 ) &&
			( Zgrid.z(  i,   j+1) >= 0.0 )  )
		 {
			OutputDXFLine( Zgrid.x(i), Zgrid.y(j), Zgrid.z(i,j),
				 Zgrid.x(i), Zgrid.y(j+1), Zgrid.z(i,j+1));
			if( DXFoutFile.fail() ){ AbortWriteDXFFile( szFile );
				 DXFoutFile.close();
				 return 0; }
		 }  // end if ( .. Zgrid.z...
	  }  // end for (int i
	} // end for int j
  return 1;
}
//**************************************************************
//       O u t p u t  D X F  P O L Y L I N E  G r i d
//**************************************************************
static int OutputDXFPOLYLINEgrid()
{
	 for ( int i = 0; i<(nx); i++ )
	 {
		for ( int j = 0; j<(ny); j++ )
		{
		  if( Zgrid.z( i, j ) >= 0.0 )
			{ if( PolyLining )
				  ContinuePolyLine( Zgrid.x(i), Zgrid.y(j), Zgrid.z( i, j ) );
			  else InitPolyLine  ( Zgrid.x(i), Zgrid.y(j), Zgrid.z( i, j ) );
			}  // end if( ( Zgrid.z
		  else   // z is less than zero - an undefined junction
			 if( PolyLining ) EndPolyLine();

		 if( DXFoutFile.fail() ){ AbortWriteDXFFile( szFile );
				 DXFoutFile.close();
				 return 0; }
		} // end for  (int j...
		EndPolyLine();
	 }  // end for  (int i....
	 EndPolyLine();
	 for ( int j = 0; j<(ny); j++ )
	{
	  for (int i = 0; i<(nx); i++ )
	  {
			if( Zgrid.z( i, j ) >= 0.0 )
			{ if( PolyLining )
				  ContinuePolyLine( Zgrid.x(i), Zgrid.y(j), Zgrid.z( i, j ) );
			  else InitPolyLine  ( Zgrid.x(i), Zgrid.y(j), Zgrid.z( i, j ) );
			}  // end if( ( Zgrid.z
		  else   // z is less than zero - an undefined junction
			 if( PolyLining ) EndPolyLine();

			if( DXFoutFile.fail() ){ AbortWriteDXFFile( szFile );
				 DXFoutFile.close();
				 return 0; }
	  }  // end for (int i
	  EndPolyLine();
	} // end for int j
	EndPolyLine();
	return 1;
}

//**************************************************************
//       O u t p u t  D X F  S P E E D  C U R R E N T  G r i d
//**************************************************************
static int OutputDXFSPEEDCURRENTgrid()
{
   static const float RadianConversionFactor = .017453 ;
   float xrange = fabs(Zgrid.x(0)-Zgrid.x(1));
   float yrange = fabs(Zgrid.y(0)-Zgrid.y(1));
   float range = max( xrange, yrange);
   float scale = (range/Sgrid.zmax())*ArrowSize ;
   for ( int i = 0; i<(nx); i++ )  // Why nx-1 ??? 
	 {
		for (int j = 0; j<(ny); j++ )
		{
		  if( Dgrid.z(i,j) > 0.0 && Sgrid.z(i,j) > 0.0 )
		  {
         float radians = (Dgrid.z(i,j))*RadianConversionFactor;
         float xoff = (sin( radians ) * Sgrid.z(i,j))*scale;
         float yoff = (cos( radians ) * Sgrid.z(i,j))*scale;
         float xend = Zgrid.x(i)+xoff;
         float yend = Zgrid.y(j)+yoff;
         OutputDXFLine( Zgrid.x(i), Zgrid.y(j), Zgrid.z(i,j),
				 xend, yend, Zgrid.z(i,j));
          // Now put half an arrow on to indicate direction.
			radians -= .24; // about 14 degrees.
			xoff = (sin( radians ) * Sgrid.z(i,j))*(scale*.7);
			yoff = (cos( radians ) * Sgrid.z(i,j))*(scale*.7);
         OutputDXFLine( xend, yend, Zgrid.z(i,j),
				 Zgrid.x(i)+xoff, Zgrid.y(j)+yoff, Zgrid.z(i,j)); 
			if( DXFoutFile.fail() ){ AbortWriteDXFFile( szFile );
				 DXFoutFile.close();
				 return 0; }
		  } // end if... Zgrid.z...
		} // end for  (int j...
	 }  // end for  (int i....
  return 1;
}
//**************************************************************
//       G e t  D X F  f i l e   N a m e
//**************************************************************
int GetDXFfilename( HWND hwnd, char szFile[] )
{
 static OPENFILENAME ofn;

 memset( &ofn, 0, sizeof(OPENFILENAME) ); // zero structure members

 ofn.lStructSize = sizeof(OPENFILENAME);
 ofn.hwndOwner = hwnd;  // =NULL if no owner required.
 ofn.lpstrFilter = "DXF files\0*.dxf\0All files\0*.*\0\0";
 ofn.nFilterIndex = 1;
 ofn.lpstrFile = szFile;
 ofn.nMaxFile = 255;        //sizeof(szFile);
 ofn.Flags = OFN_PATHMUSTEXIST|OFN_OVERWRITEPROMPT;
 ofn.lpstrTitle = "Generate DXF output to file" ;
 ofn.lpstrDefExt = "dxf";
 if( !GetSaveFileName( &ofn ) ) return 0;
 return 1;
}

//**************************************************************
//       D r a w   D X F  O u t l i n e
//**************************************************************
static void OutputDXFOutline()
{
	if( OutLine.Size() < 2 ) return; // no outline data.

	if( BoldOutline )  BoldLine = true; else BoldLine = false;

   SetDXFLayerName( "OUTLINE" );

	long SizeOfOutline = OutLine.Size();

   if ( UsePolyline )
	{
     InitPolyLine( OutLine.x(0), OutLine.y(0), OutLine.z(0) );
     for ( long i = 1; i < SizeOfOutline; i++ )
     {
	  if( OutLine.flags(i)&1  )
          InitPolyLine( OutLine.x(i), OutLine.y(i), OutLine.z(i) );
	  else ContinuePolyLine( OutLine.x(i), OutLine.y(i), OutLine.z(i) ); 
     }
     EndPolyLine();
	} // end if ( UsePolyline );
   else // Use line DXF primitive.
   {
     for ( long i = 1; i < SizeOfOutline; i++ )
      {
       if( OutLine.flags(i)&1 ) continue; // do a move
       OutputDXFLine( OutLine.x(i-1), OutLine.y(i-1), OutLine.z(i-1),
				 OutLine.x(i), OutLine.y(i), OutLine.z(i) );
      }
   }  // end else // Use line DXF primitive.
}
//**************************************************************
//    O u t p u t  D X F  C o n n e c t e d  D a t a 
//**************************************************************
static void OutputDXFConnectedData()
{
   static long i;
	long NumberOfPoints = ScatterData.Size();
   if( NumberOfPoints < 3 ) return;

   SetDXFLayerName( "SCATDATA" );

   if ( UsePolyline )
	{
     InitPolyLine( ScatterData.x(0), ScatterData.y(0), ScatterData.z(0) );
     for ( i = 1; i < NumberOfPoints; i++ )
      ContinuePolyLine( ScatterData.x(i), ScatterData.y(i), ScatterData.z(i) );
     EndPolyLine();
	} // end if ( UsePolyline );
   else // Use line DXF primitive.
   {
     for ( long i = 1; i < NumberOfPoints; i++ )
      {
       OutputDXFLine( ScatterData.x(i-1), ScatterData.y(i-1), ScatterData.z(i-1),
                      ScatterData.x(i),   ScatterData.y(i),   ScatterData.z(i) );
      }
   }  // end else // Use line DXF primitive.
}
//**************************************************************
//       O u t p u t  D X F  f i l e
//**************************************************************
int OutputDXFfile(ContourOptionsType Options, UINT PaintState, char szFile[] )
{

 DXFoutFile.open( szFile, ios::out );

  if( !DXFoutFile )
    { NotifyUser( IDS_NOOUTPUTFILE );
      return 0;
    }

 LoadNormalization( dxnormalize, dynormalize );
 zadjust = ScatterData.zAdjust();

 SetWaitCursor();

 Zgrid.zratio(0);
 DXFOutputFailed = FALSE;
 zadjust = ScatterData.zAdjust();

 DXFTextHeight = (ScatterData.yMax()-ScatterData.yMin() )*DXFTextHeightRatio;
 Thickness = (ScatterData.yMax()-ScatterData.yMin() )*.001;
 //Thickness = .1; 

 DXFoutFile << "999" << endl   // Output Header information.
	 << "DXF output created by " ; 

 DXFoutFile << "QuikGrid." << endl;

 if( HighDXFOutputPrecision) DXFoutFile << setprecision( 12 ) ;

 DXFoutFile << "  0"      << endl      // Geometric entities go in an ... 
	 << "SECTION"  << endl
	 << "  2"      << endl
         << "ENTITIES" << endl;  // ... ENTITIES section. 
	   
 nx = Zgrid.xsize();
 ny = Zgrid.ysize();

 SetDXFLayerName( "ARROWS" );
 if( Sgrid.xsize() > 2 && Dgrid.xsize() > 2 )
  { if( OutputDXFSPEEDCURRENTgrid() == 0 ) return 0; }

 if ( Options.grid || Options.hiddengrid )
  {
   SetDXFLayerName( "GRID" );
   if (UsePolyline)
	    { if (OutputDXFPOLYLINEgrid() == 0 ) return 0; }
   else
	    {if ( OutputDXFLINEgrid() == 0 ) return 0; }
  }

 long NumberOfPoints = ScatterData.Size();

 if( Options.marks && (NumberOfPoints>2) )
 {
	strcpy( DXFLayerName, "SCATDATA" );
   float zadjust = ScatterData.zAdjust();
   for( long i = 0; i<NumberOfPoints; i++ )
	{
	  if( UseScatteredDataLayers )
	  {
       // Determine label for point by contour line less than value.
       // Code determines the "closest" contour line.
       float zvalue = ScatterData.z(i);
       int CloseIndex = 0;

       // Code determines contour closest contour line less than the z value.
       for( int j=1; j<NumberOfContours; j++ )
        {
         if( (zvalue < ContourLineValue(j)) && (zvalue >= ContourLineValue(j-1)) )
                         {  CloseIndex = j-1; break;}
        }  
       // build layer name from that.
       ContourValue = ContourLineValue(CloseIndex);
	    SetDXFLayerName( "SCATDATA", ContourValue-zadjust );
	  }   // end if( UseScatteredDataLayers...
     
	  DXFoutFile << "  0"            << endl
	     << "POINT"          << endl
	     << "  8"            << endl // put points on layer
		  << DXFLayerName       << endl // SCATDATA
		  << " 10"            << endl
		  << (ScatterData.x(i)+dxnormalize) << endl
	     << " 20"            << endl
	     << (ScatterData.y(i)+dynormalize) << endl
	     << " 30 "           << endl
		  << (ScatterData.z(i)-zadjust) << endl;
		  if( DXFoutFile.fail() ){ AbortWriteDXFFile( szFile );
				 DXFoutFile.close();
				 return 0; }
     if( Options.numbers ||
         Options.comments||
	 Options.xyplot)        // Output the Text. 
     {
	     char* szBuf = FormatDataPointText( i );
        OutputDXFText( szBuf, (ScatterData.x(i)),
                              (ScatterData.y(i)),
                              (ScatterData.z(i)));
	     delete szBuf;

	     if( DXFoutFile.fail() ){ AbortWriteDXFFile( szFile );
				 DXFoutFile.close();
				 return 0; }
     } // end if( Options... 

   }
   if( Options.lines )  OutputDXFConnectedData();
 } // endif Options.marks && (NumberOfPoints>2)

 if( Options.contours )
 {
   SetDXFoutputMode();
   ContourLabelling = Options.labelcontours;  // Move to local space.
   if ( NeverLabelDXFContours ) ContourLabelling = false; 
   if( ContourLabelling ) LabelInit(); // Initialize label collection.
   for( int i = 0; i<NumberOfContours; i++ )
   {
     if( DXFOutputFailed )  { AbortWriteDXFFile( szFile );
			      DXFoutFile.close();
					return 0; }
     ContourLine = CurrentContourLineNumber = i;
	  ContourValue = ContourLineValue(i);
     if( ContourLineLabel(i)&&Options.labelcontours ) LabelStart();
	  SetDXFLayerName( "CONTOUR", ContourValue-zadjust );
     if( ContourLineBold(i) ) BoldLine = true; else BoldLine = false;
	  Contour( Zgrid, ContourValue );
     if( ContourLineLabel(i)&&Options.labelcontours )LabelStop();
     Gapping = false; 
	  EndPolyLine();

	} // end for( int i = 0 ...
    if( ContourLabelling ) LabelStop();
 } // end if( Options.contours...

OutputDXFOutline(); 

 DXFoutFile << "  0"    << endl    // Terminate ENTITIES section.
		 << "ENDSEC" << endl;

 DXFoutFile << "  0"    << endl    // Output end of file indicator.
            << "EOF"    << endl;

 DXFoutFile.close();
 RestoreCursor(); 
 return 1; 
}
//**************************************************************
//       O u t p u t D X F C o n t o u r L i n e
//**************************************************************
void OutputDXFContourLine( float x, float y, int draw )
{
	if( DXFOutputFailed ) return;
   if( ContourLabelling )
   {
      if( GapProcess( x, y)) return; // Skip if making space for a label.
      LabelIntercept( x, y, draw );  // Check if time to create a label space.
      if( Gapping ) return;          // May have already started a gap for label.
   } // end if ( ContourLabelling )
	if ( UsePolyline )
	{
	  if( draw ) ContinuePolyLine( x, y, ContourValue );
	  else InitPolyLine( x, y, ContourValue);
	} // end if ( UsePolyline)
	else  // if ( !UsePolyline )
	{
	  if( draw )
	  {
		 OutputDXFLine( xold, yold, zold, x, y, ContourValue);
		 if( DXFoutFile.fail() ) DXFOutputFailed = TRUE;
	  }
	} // end else
   xold = x;
   yold = y;
   zold = ContourValue;
}
