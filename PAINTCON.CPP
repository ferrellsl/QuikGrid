//******************************************************************
//                 P A I N T C O N
//
// Functions primarily concerned with displaying graphics (not text).
//
//                       Copyright (c) 1993 - 2005 by John Coulthard
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
// Sept. 23/98: Insert code to support grid resolution.
// Jan. 18/99: Improve xyInfo2d by making sure point being sought
//             is visible - prevents program expection.
// Jan. 19/99: Add InTheGrid function (is mouse in the grid?).
// Jan. 29/99: Change display of coordinate info to use common functions.
// Feb. 4/99: Implement 3d axes.
//            Only draw visible grid in 2d mode.
// Feb. 10/99: Add ZoomIn and ZoomOut.
// Feb. 13/99: Add panning support for arrow keys.
// Feb. 14/99: Always display mark if comment is present.
//             Display axes if selected.
// Mar. 2/99: Change BluePen and ScatPen to use 0 width (1 pizel).
//            (means skinny lines at all times).
//            Change Mark to scale to window size.
// Mar. 23/99: Change to label/highlight starting at 0'th line
//             (instead of at the highlight'th line).
//             Change to have NiceContourLinesSet handle secondary line concept.
// Mar. 28/99: Change ContourLinesSet to set contour lines starting at zmin.
//             Change NiceContoursSet to reinitialize #contours and # between.
// May 22/99:  Remove bug - right mouse click bad for any grid resolution.
// May 25-27/99:  Improve drawing speed (check line ends for visible & buffer).
// Jun 4/99: Fix bug in PaintSurfaceRatio - error checking upper limit.
// Jun 5/99: Changed CloseTo2d code to do direct calculation (speed increase).
// Jun 5/99: Move hdc communication with QuikGrid to global extern. Fix
//           but in optimization code having to do with pen updates.
// Jun 11/99: Moving code over to drawstuf.cpp. (Polybuffer, Plotto).
// Oct 25/99: Initial code to show meterology stuff on display.
// Oct 29/99: Change to make contour lines optional in 2d mode.
// Feb. 2/00: Add support for outlines.
// Mar, 26/00: Check for outline or data points not on the defined grid.
// Jun 13/00: Assert( xyRange == 0 ) checks commented out.
//            if condition sets xyRange to 1 in these circumstances. 
//            It appears that xyRange will drop to zero under circumstances
//            where an outline is loaded for a grid and the two differ so
//            extremely in magnitude that underflows occur. This is probably
//            a user error - but I am not sure how it can easily be detected in a
//            straight forward fashion. Also never had a field report on it so
//            it probably is not very important to address this "problem?"
// Jun 15/00: Made Margin and shift available globally so they could be changed
//            on the fly from a command file.
// Sep 5/00: check for xyRange checks for <= xyRangeMin (10.0) instead of zero.
// Sep 11/00: Try xyRangeMin of 2 to correct scaling when outline only loaded.
// Sep 25/00: Moving contouring stuff to condraw.cpp.
// Nov. 4/00: Moving references to ContourLines[i] to ContourLineValue function
// Nov 8-12/00: Many changes to allow custom contour colours.
// Nov 26/00: Make the outline a bold pen.
// Dec. 4/00: Make outline bold an option.
// Feb. 14/01: 3D outline was always bold - fixed!
// Mar. 13/01: Implement Axes always on top.
//             Implement ColourLegend
// Mar. 14/01: Implement DataPointsOnGridOnly
// Oct. 3/01:  Fix bug in colour legend if no grid to display.
// Dec. 4/01: Change precision for the display of the z coordinate
//            for corner intersections or comment attached to data point
//            from 3 to 5 (why I set it to 3 originally I can't remember
//            but suspect it was due to small screens of the day).
// Sep. 01/09: Implement user control of speed arrow length.
//             (create "ArrowSize" external).
//*********************************************************************
//
//  Candidates to move to external source module.
//  DoLineTo,
//   Moving save2d and save3d lines to global would help.

#include <windows.h>
#include <iostream>
#include <strstream>
#include <iomanip>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include "assert.h"
#include "surfgrid.h"
#include "xygrid.h"
#include "scatdata.h"
#include "saveline.h"
#include "xpand.h"
#include "quikgrid.h"
#include "paintcon.h"
#include "loaddata.h"
#include "dxfwrite.h"
#include "utilitys.h"
#include "rc.h"
#include "rotate.h"
#include "contour.h"
#include "hatch.h"
#include "labelc.h"
#include "drawstuf.h"
#include "condraw.h"
using namespace std;
extern HWND WindProcHwnd; 
extern HWND hGridLocationDlgBox;
extern HDC         hdc; 
extern int Zooming;
extern ContourOptionsType DrawingOptions;
extern int LatLonData;
extern int GridGeneratedOK;
extern int GridLock;
extern ScatData OutLine;
extern SaveLineType Save2dContours,
						  Save3dContours;
extern int ColourContours;
extern int ColouredGrid;
extern float CgridZadjust; 
extern int BoldOutline;
extern UINT PaintState;
extern bool CreatingAMetaFile;
extern int QuikGridChangedImage;
extern int NeverHideContours; 
extern int LabelNDigits;  // Defined in Labelc.cpp
extern double ColourZmin, ColourZmid, ColourZmax;

// Global definition grids:

SurfaceGrid Sgrid(2,2),  // The meteorological speed grid.
            Dgrid(2,2),  // The meteorological direction grid (0-360).
            Cgrid(2,2);  // The colour grid.

float ZoomRatio = 1.5 ;  // Default amount of change on zoom.
float PanRatio = 0.1; // Default for panning with arrow keys.
int IncludeColourLegend = TRUE;
int DataPointsOnGridOnly = TRUE; // Only show scattered data points if they
                                 // are on the defined grid area.
int SmallMark = TRUE; // Size of the mark for the data point.
int DisplayVolumeDifference = 1;
int TextSize = 80;
float ArrowSize = 2.; // Arrow length twice maximum grid side.
float ZmaxLabel = 0.0, ZminLabel = 0.0; // For drawing the axes
//*************************************************************************
extern double VolumeDifference,
              VolumeDifferencePositive,
              VolumeDifferenceNegative;

extern int LabelNthLine,
           AxesOnTop = FALSE;
//*************************************************************************
static xyGridClass xyGrid(2,2); // initial dimensions only.
static int xyGridOK = 0;

static const int MarkSize = 30, // length of a tic to mark a point.
		 MarkFraction = MarkSize/10;

static ContourType ContourMode = TwoD;
static int BlackAndWhite; 
//static HBRUSH hBrush;
static int SpecialPenInUse = FALSE;
static int SpecialPenWasUsed = FALSE;

static long ColourLegendBoxSize;
static long ColourLegendBoxSizeInPixels = 20;

static int BoldWidth; // Width of a bold line - dependant on window size. 

HPEN hGridPen,
     hArrowPen,
     hContourPen,
     hBoldContourPen,
     hDataPointPen,
     hOutlinePen,
     hConnectPointsPen,
     hBlackPen,
     hOldPen,
     hBoldPen,
     hBackgroundPen,
     hAxisPen,
     hSpecialPen,
     hPenInUse;

COLORREF GridPen = RGB( 0, 0, 255),
         ArrowPen = RGB( 0, 0, 255),
         ContourPen = RGB( 0, 0, 0),
         BoldContourPen = RGB( 0, 0, 0),
         DataPointPen = RGB( 255, 0, 0),
         OutlinePen = RGB( 255, 0, 0),
         ConnectPointsPen  = RGB( 255, 0, 0),
         BlackPen = RGB( 0, 0, 0),
         BackgroundPen = RGB( 255, 255, 255 ),
         TextPen = RGB( 0, 0, 0),
         AxisPen = RGB( 0, 0, 0);

static HFONT hFont;

static int   ContourValueIndex;
static float ContourValue = 0.,
             Real3DContourValue, 
				 zRatio = .6;

// xmin... yMax define the limits of the current image in "user" units.
// (not screen units). For the 2d view this is normally the same as the
// extents for the Z grid. For the 3d view this is the extents of the
// grid after rotating, including adjustment for the Zratio.

static float xMin, xMax, yMin, yMax;
static float zMin, zMax,
				 xRange,
				 yRange,
				 zRange,
				 xyRange;
// XminZgrid... zMaxZgrid define the extents of the unrotated native
// Z grid.

static float xMinZgrid, xMaxZgrid,
             yMinZgrid, yMaxZgrid,
             zMinZgrid, zMaxZgrid;


float Margin = 400.0,
      shift = Margin*2;

static const float MaxScale = 6000.0,
//   What is the following? I think it was created to prevent
// underflows when the xyRange was close to zero. I'm not even
// sure it is needed - it can result in too small a display. 
                   xyRangeMin = 0.000010;  //* Previously 0.01

static int PercentzRatio = 60,
			  TitleXposn = Margin,
			  TitleYposn = 0,    // will be replaced.
			  PenMode = 1;

static long GridInfoIX,
	         GridInfoIY;

static inline int ScaleX(float x)
   { return ( x - xMin)/xyRange*MaxScale + Margin;}
static inline int ScaleY(float y) // Flip y to go from 6000 -> 0; 
   { return -( ( y - yMin)/xyRange*MaxScale + Margin) + MaxScale+shift;}

static inline float unScaleX( int ix )
   { return (ix-Margin)*xyRange/MaxScale + xMin; }
static inline float unScaleY( int iy )
   { return (-(iy-MaxScale-shift)-Margin)*xyRange/MaxScale + yMin; }

struct PORT {
				  int xr;   // x range
	           int yr;   // y range
	           int xo;   // x origin
	           int yo;   // y origin
	         } ;

static PORT CurrWin,
	         CurrView,
	         ZoomWin,
	         PreviousWin,
	         PreviousView;

RECT ClientRect, ClipRect;

//********************************************************
//      C r e a t e   P e n s
//********************************************************
void CreatePens( )
{
  hGridPen = CreatePen( PS_SOLID, 0, GridPen);
  hArrowPen = CreatePen( PS_SOLID, 0, ArrowPen);
  hContourPen = CreatePen( PS_SOLID, 0, ContourPen);
  hBoldContourPen = CreatePen( PS_SOLID, 0, BoldContourPen);
  hDataPointPen = CreatePen( PS_SOLID, 0, DataPointPen);
  hOutlinePen = CreatePen( PS_SOLID, 0, OutlinePen);
  hConnectPointsPen = CreatePen( PS_SOLID, 0, ConnectPointsPen);
  hBlackPen = CreatePen( PS_SOLID, 0, ContourPen);
  hAxisPen = CreatePen( PS_SOLID, 0, AxisPen);
  hBackgroundPen = (HPEN)CreateSolidBrush( BackgroundPen ) ;
}
//********************************************************
//      D e s t r o y  P e n s
//********************************************************
void DestroyPens()
{
  DeleteObject( hGridPen );
  DeleteObject( hArrowPen );
  DeleteObject( hContourPen );
  DeleteObject( hBoldContourPen );
  DeleteObject( hDataPointPen );
  DeleteObject( hOutlinePen );
  DeleteObject( hConnectPointsPen );
  DeleteObject( hBlackPen);
  DeleteObject( hAxisPen);
  DeleteObject( hBackgroundPen);
}

//*********************************************************
//            S e t   N e w  W i n
//*********************************************************
static void SetNewWin( HDC &hdc, PORT NewWin )
{
	SetWindowExtEx( hdc, NewWin.xr, NewWin.yr, NULL );
	SetWindowOrgEx( hdc, NewWin.xo, NewWin.yo, NULL );
   PreviousWin = CurrWin;
   CurrWin = NewWin;
}
//*********************************************************
//            S e t   N e w    V i e w
//*********************************************************
static void SetNewView( HDC &hdc, PORT NewView )
{
 SetViewportExtEx( hdc, NewView.xr, NewView.yr, NULL );
 SetViewportOrgEx( hdc, NewView.xo, NewView.yo, NULL );
 PreviousView = CurrView;
 CurrView = NewView;
}
//********************************************************
//         S e t   N e w   W i n   V i e w
//********************************************************
static void SetNewWinView( HDC &hdc, PORT NewWin, PORT NewView )
{
   SetNewWin( hdc, NewWin);
   SetNewView( hdc, NewView);
}

//*********************************************************
//        S e t   C u r r e n t   W i n   V i e w
//*********************************************************
void SetCurrentWinView( HDC &hdc )
{
  SetWindowExtEx( hdc, CurrWin.xr, CurrWin.yr, NULL );
  SetWindowOrgEx( hdc, CurrWin.xo, CurrWin.yo, NULL );
  SetViewportExtEx( hdc, CurrView.xr, CurrView.yr, NULL );
  SetViewportOrgEx( hdc, CurrView.xo, CurrView.yo, NULL );
}

//********************************************************
//              S e t   C u r r e n t   W i n
//********************************************************
void SetCurrentWin( HDC &hdc )
{
  SetWindowExtEx( hdc, CurrWin.xr, CurrWin.yr, NULL );
  SetWindowOrgEx( hdc, CurrWin.xo, CurrWin.yo, NULL );
}
//********************************************************
//              Z o o m   V i e w   P r e v i o u s 
//********************************************************
void ZoomViewPrevious()
{
     ZoomWin = PreviousWin;
}

//********************************************************
//              Z o o m   V i e w   D o u b l e
//********************************************************
int ZoomViewDouble( const POINT ZoomCentre, const int ZoomFactor )
{
	  static int TempZoom;
	  static float TempZoomRatio;
     static POINT OldZoomCentre;
     OldZoomCentre.x = ZoomWin.xo + ZoomWin.xr/2;
     OldZoomCentre.y = ZoomWin.yo+ZoomWin.yr/2;
     TempZoom = ZoomFactor;


     assert( ZoomFactor != 0 );

     if( ZoomWin.xr < 200  && ZoomFactor > 0 ) TempZoom = 1;
	  if( ZoomWin.xr > 8000 && ZoomFactor < 0 ) TempZoom = -1;


     if( abs( TempZoom) == 1 && OldZoomCentre.x == ZoomCentre.x &&
                                OldZoomCentre.y == ZoomCentre.y ) return 0;

	  TempZoomRatio = ZoomRatio;
	  if( abs( TempZoom ) == 1 ) TempZoomRatio = 1. ;
	  if( TempZoom < 0 ) TempZoomRatio = -TempZoomRatio;

     if( ZoomFactor > 0 )
		 {
         assert( TempZoomRatio > 0.0 ); 
			ZoomWin.xr = ZoomWin.xr/TempZoomRatio;
			ZoomWin.yr = ZoomWin.yr/TempZoomRatio;
       }
	  else   // ZoomFactor < 0
		 {
			ZoomWin.xr = ZoomWin.xr*(-TempZoomRatio);
	      ZoomWin.yr = ZoomWin.yr*(-TempZoomRatio);
       }

     ZoomWin.xo = ZoomCentre.x - ZoomWin.xr/2;
     ZoomWin.yo = ZoomCentre.y - ZoomWin.yr/2;

     return 1; 
}
//**************************************************************
//            Z o o m   I n
//**************************************************************
int ZoomIn(  )
{
     static POINT ZoomCentre;
     ZoomCentre.x = ZoomWin.xo + ZoomWin.xr/2;
     ZoomCentre.y = ZoomWin.yo + ZoomWin.yr/2;

     return ZoomViewDouble( ZoomCentre, 2 );
}
//**************************************************************
//            Z o o m   O u t
//**************************************************************
int ZoomOut(  )
{
     static POINT ZoomCentre;
	  ZoomCentre.x = ZoomWin.xo+ZoomWin.xr/2;
	  ZoomCentre.y = ZoomWin.yo+ZoomWin.yr/2;

	  return ZoomViewDouble( ZoomCentre, -2 );
}
//**************************************************************
//            P A N   V I E W
//**************************************************************
int PanView( const int xDirection, const int yDirection )
{
	  static POINT ZoomCentre;
	  static int xMotion, yMotion;

	  ZoomCentre.x = ZoomWin.xo + ZoomWin.xr/2;
	  ZoomCentre.y = ZoomWin.yo + ZoomWin.yr/2;

	  xMotion = ZoomWin.xr*PanRatio*2;
	  yMotion = ZoomWin.yr*PanRatio*2;

	  ZoomCentre.x = ZoomCentre.x + xMotion*xDirection;
	  ZoomCentre.y = ZoomCentre.y + yMotion*yDirection;

	  return ZoomViewDouble( ZoomCentre, 1 ) ;
}
//********************************************************
//            G e t Z o o m e d U s e r C o o r d 
//********************************************************
int GetZoomedUser( float &xmin, float &xmax,
		   float &ymin, float &ymax )
{
  xmin = unScaleX( ClientRect.left );
  xmax = unScaleX( ClientRect.right );
  ymin = unScaleY( ClientRect.bottom );
  ymax = unScaleY( ClientRect.top );
  // Constrain to the space of the actual data.
  xmin = max( xmin, ScatterData.xMin());
  xmax = min( xmax, ScatterData.xMax());
  ymin = max( ymin, ScatterData.yMin());
  ymax = min( ymax, ScatterData.yMax());
  return 1;  
}
//********************************************************
//            V i s i b l e
//
// Returns TRUE if x,y are inside the clipping window
//********************************************************
int Visible( const int &ix, const int &iy )
{
  if( CreatingAMetaFile ) return 1;

  if( ix < ClipRect.left ) return 0;
  if( iy < ClipRect.top ) return 0;
  if( ix > ClipRect.right ) return 0;
  if( iy > ClipRect.bottom ) return 0;

  return 1;
}

//********************************************************
//            V i s i b l e X
//
// Returns -1 0 +1 if X is less than equal or greater than.
//********************************************************
int VisibleX( const int &ix )
{
  if( CreatingAMetaFile ) return 0;

  if( ix < ClipRect.left ) return -1;
  if( ix > ClipRect.right ) return 1;
  return 0;
}

//********************************************************
//            V i s i b l e Y
//
// Returns -1 0 +1 if Y is less than equal or greater than.
//********************************************************
int VisibleY( const int &iy )
{
  if( CreatingAMetaFile ) return 0;

  if( iy < ClipRect.top ) return -1;
  if( iy > ClipRect.bottom ) return 1;
  return 0;
}

//********************************************************
//            I n   T h e   G r i d
//
// Returns true if x,y are inside the defined grid.
//********************************************************
int InTheGrid( const int &ix, const int &iy )
// Screen position integer version.
{
  static float x, y;
  if( !Visible( ix, iy ) ) return 0;
  y = unScaleY( iy );
  x = unScaleX( ix );
  if( x < Zgrid.xmin() ) return 0;
  if( y < Zgrid.ymin() ) return 0;
  if( x > Zgrid.xmax() ) return 0;
  if( y > Zgrid.ymax() ) return 0;
  return 1;
}

static int InTheGrid( const float x, const float y )
// Internal grid only. 
// Check floating point x and y
{
  if( x < xMinZgrid ) return 0;
  if( y < yMinZgrid ) return 0;
  if( x > xMaxZgrid ) return 0;
  if( y > yMaxZgrid ) return 0;

  return 1;
}
//********************************************************
//   P a i n t   S u r f a c e R a t i o: Get and Set zRatio
//********************************************************
int PaintSurfaceRatio() { return PercentzRatio; }
int PaintSurfaceRatio( int NewRatio )
{ float OldRatio = PercentzRatio;
  PercentzRatio = NewRatio ;
  if( PercentzRatio < 0 ) PercentzRatio = 0; 
  if( PercentzRatio > 200  ) PercentzRatio = 200;
  zRatio = (float) PercentzRatio*.01;
  RotateReset();
  Save3dContours.Reset();
  return OldRatio;
}

//********************************************************
//   A p p l y  z R a t i o
//********************************************************
static void ApplyzRatio()
{
// A zRatio of zero is used within QuikGrid as meaning to
// use unscaled units, which is not how the zratio function
// of the class works. It expects a zero ratio to mean unscaled
// z coordinates. So this function maps 0. to -1.0.

  if( zRatio == 0.0 ) Zgrid.zratio( -1.0 ) ;  // unscaled z units.
  else Zgrid.zratio( zRatio );
}


//********************************************************
//          S e t   E x t e n t s
//********************************************************
void SetExtents( HDC &hdc )
{
  if( xyRange <= xyRangeMin ) xyRange = xyRangeMin;
  static PORT Extents;
  Extents.xr = ScaleX(xMax)-ScaleX(xMin) + shift;
  Extents.yr = abs(ScaleY(yMax)-ScaleY(yMin)) + shift;
  Extents.xo = 0;
  Extents.yo = ScaleY(yMax) - Margin;
  SetNewWin( hdc, Extents );
}

//********************************************************
//          C e n t r e   D i s p l a y
//********************************************************
void CentreDisplay( HDC &hdc, RECT &rect, RECT &cliprect, int Zooming )
// rect is the view rectangle for the entire client area of the window.
// cliprect is the update or clipping rectangle
{

  static PORT CentreView;
  static int CentreShift;

  if( xyRange <= xyRangeMin ) xyRange = xyRangeMin; // Try to avoid scaling under/over flows

  SetMapMode( hdc, MM_ISOTROPIC );

  // Save coordinates of rectangle
  ClientRect = rect;
  ClipRect = cliprect;
  //char szBuffer[200];
  //sprintf( szBuffer, "Client rectangle: %i %i %i %i Clip rectangle %i %i %i %i",
  //          ClientRect.left, ClientRect.right, ClientRect.bottom, ClientRect.top,
  //          ClipRect.left, ClipRect.right, ClipRect.bottom, ClipRect.top);
  //NotifyUser( szBuffer );


  if( !Zooming ) SetExtents( hdc ); // Set new window if not zooming.
  CentreView.xr = rect.right;
  CentreView.yr = rect.bottom;
  
  // Centre the thing in the window.

   if( rect.bottom <= 0 ) // Pathological case - null height.
      { CentreView.xo = CentreView.yo = 0; 
        SetNewView( hdc, CentreView ); } // Non centering call.
        
   else
   {
	  if( (float)rect.right/rect.bottom < (float)CurrWin.xr/CurrWin.yr )
      { CentreView.xo = 0;                                // tall
		  CentreShift = (rect.bottom-(rect.right*(long)CurrWin.yr)/CurrWin.xr)/2;
        CentreView.yo = CentreShift; 
      }
     else                                                 // wide
      { CentreShift = (rect.right-(rect.bottom*(long)CurrWin.xr)/CurrWin.yr)/2;
        CentreView.xo = CentreShift;
	     CentreView.yo = 0;
      }

     if( !Zooming ) SetNewView( hdc, CentreView );
     else           SetNewWinView( hdc, ZoomWin, CentreView );
   }

  DPtoLP( hdc, (LPPOINT)&ClientRect, 2);
  DPtoLP( hdc, (LPPOINT)&ClipRect, 2);
  //hBrush = CreateSolidBrush( BackgroundPen ) ;
  FillRect( hdc, &ClientRect, (HBRUSH)hBackgroundPen );

  ZoomWin = CurrWin;
  //assert( DeleteObject( hBrush));
}

//********************************************************
//         R o t a t e   G r i d   R e s e t
//********************************************************
void RotateReset() { xyGridOK = 0; }

//********************************************************
//        R o t a t e  Grid
//********************************************************
// Routine to rotate zGrid and save as int 2d grid.
void RotateGrid()
{
  if( xyGridOK ) return;
  xyGridOK = 1; 
  float x, y, z;
  int xsize = Zgrid.xsize();
  int ysize = Zgrid.ysize();
  xyGrid.New( xsize, ysize);

  for( int i = 0; i<xsize; i++ )
   {
     for( int j = 0; j<ysize; j++)
      {
       z = Zgrid.z(i,j);
       if ( z < 0.0 ) xyGrid.set( i, j, 0, 0 );
       else
         { 
            x = Zgrid.x(i);
            y = Zgrid.y(j);
				Rotate( y, x, z);
				if( Projection != 0.0 ) Project( x, y, z);
				xyGrid.set( i, j, ScaleX( x ), ScaleY( y ) );
	      } 
      }
   }
    
}

//********************************************************
//             S c a l e  2 d   R e c t a n g l e
//********************************************************
void Scale2dRectangle()
{
  xMin = Zgrid.xmin();
  yMin = Zgrid.ymin();
  xMax = Zgrid.xmax();
  yMax = Zgrid.ymax();

  xRange = xMax - xMin;
  yRange = yMax - yMin;
  xyRange = max( xRange , yRange);
  if( xyRange <= xyRangeMin ) xyRange = xyRangeMin;
  assert( xyRange > 0.0 );
 // NotifyUser( "Xmin-max %f %f ", xMin, xMax);
 // NotifyUser( "Ymin-max %f %f", xMin, xMax) ;
}


//**********************************************************
//          D r a w   2 d  G r i d   L i n e
//**********************************************************
// Service routine.
// Delete multiple moves and draws. 
static void Draw2dGridLine( int xScaled, int yScaled, float z, int &Draw )
{
   static int xOld, yOld,
              DrawOld = 0;

   if(  DrawOld != Draw )  // new scan line started.
     {
       LineTo( hdc, xOld, yOld );
       DrawOld = 0;
     }

   if( z<0.0 ) // z forces Draw to 0 
   {  
      if( DrawOld ) LineTo( hdc, xOld, yOld );
      Draw = 0;
   }
   else  // z > 0.0 
   {
		if( !DrawOld ) MoveToEx( hdc, xScaled, yScaled, NULL );
      Draw = 1;
   }
   xOld = xScaled; yOld = yScaled; DrawOld = Draw;

}
//********************************************************
//     F o r m a t   D a t a   P o i n t   T e x t
//********************************************************
char* FormatDataPointText( long i )
{
  static double xadjust, yadjust;
  char szTemp[30];

       ostrstream Buf;
       Buf << setprecision(5) ;
       if( DrawingOptions.pointnumber ) Buf << (i+1) ;

       if( DrawingOptions.xyplot )
       {
	     LoadNormalization(xadjust, yadjust);
        FormatXY( szTemp, sizeof(szTemp), ScatterData.x(i)+xadjust,
                                          ScatterData.y(i)+yadjust ) ;
        if( DrawingOptions.pointnumber ) Buf << "," ;
        Buf << szTemp;
       }

       if( DrawingOptions.numbers)
       {
         if( DrawingOptions.pointnumber) Buf << ",";
	      Buf << " " << (ScatterData.z(i)-ScatterData.zAdjust() );
       }

       if( DrawingOptions.comments )
        {
			 if( ScatterData.comment(i)[0] != NULL )
              {
               if( DrawingOptions.pointnumber || DrawingOptions.numbers )
                                             Buf << "," ;
               // Buf << "\"" << ScatterData.comment(i) << "\"" ;
               if( DrawingOptions.pointnumber || DrawingOptions.numbers  ||
                   DrawingOptions.xyplot ) Buf << "\"" ;
               Buf << ScatterData.comment(i) ;
               if( DrawingOptions.pointnumber || DrawingOptions.numbers ||
                   DrawingOptions.xyplot ) Buf << "\"" ;
              }
        }
       Buf << ends;

  return Buf.str();
}
//********************************************************
//                D r a w  M a r k
//********************************************************
// ... and xy position and z value and comments if selected.
//     and the point number if selected.
//
static void DrawMark(const int &x, const int &y, const long &i )
{
  static char *szBuf;
  static int ScaledMarkSize, SmallWidth;
  ScaledMarkSize = CurrWin.xr/90;
  SmallWidth = BoldWidth/128;
  if( SmallWidth < 1 ) SmallWidth = 1;
  if( ScaledMarkSize < 2 ) ScaledMarkSize = 2;
  if( ScaledMarkSize > MarkSize ) ScaledMarkSize = MarkSize;
  if( !Visible( x, y ) ) return;

  if( DrawingOptions.showmarks ||
      (DrawingOptions.comments && ScatterData.comment(i)[0] != NULL ) )
    {
      if( SmallMark )
      {
        MoveToEx( hdc, x, y, NULL); LineTo( hdc, x+SmallWidth, y);
		  MoveToEx( hdc, x, y, NULL); LineTo( hdc, x, y+SmallWidth);
      }
      else
      {
		  MoveToEx( hdc, x-ScaledMarkSize, y, NULL); LineTo( hdc, x+ScaledMarkSize, y);
		  MoveToEx( hdc, x, y-ScaledMarkSize, NULL); LineTo( hdc, x, y+ScaledMarkSize);
      }
    }

  if( DrawingOptions.pointnumber||
      DrawingOptions.numbers||
      DrawingOptions.comments||
      DrawingOptions.xyplot)
    {
       szBuf = FormatDataPointText(i); 
		 TextOut( hdc, x+MarkFraction, y+MarkFraction, szBuf, strlen(szBuf) );
       delete szBuf;

    } // end if DrawingOptions ... 

}
//********************************************************
//             C l o s e T o 2 d G r i d
//
// Finds the closest grid intersection to (x,y) and returns
// it's coordinates as (IX,IY). The distance between (x,y)
// and (IX,IY), squared, is returned as the function value.
//********************************************************
static float CloseTo2dGrid( const float x, const float y, long &IX, long &IY )
 {
  static float xDistance, yDistance, incr;
  static const float round = 0.5;
  static int xSize, ySize;

  xSize = Zgrid.xsize();
  ySize = Zgrid.ysize();

  incr = Zgrid.x(1)-Zgrid.x(0);
  IX = (x - Zgrid.x(0))/incr + round;
  if( IX < 0 ) IX = 0;
  if( IX >= xSize ) IX = xSize-1;
  xDistance = x - Zgrid.x(IX);

  incr = Zgrid.y(1)-Zgrid.y(0);
  IY = (y - Zgrid.y(0))/incr + round;
  if( IY < 0 ) IY = 0;
  if( IY >= ySize ) IY = ySize-1;
  yDistance = y - Zgrid.y(IY);

  return xDistance*xDistance + yDistance*yDistance;
 }
//********************************************************
//    D r a w 2 d I n t e r s e c t i o n
//********************************************************
static void Draw2dIntersection( HDC &hdc, long ix, long iy, UINT align )
{
  static double xadjust, yadjust;
  LoadNormalization( xadjust, yadjust );
  char szTemp[50];

  ostrstream Buf;

  LoadNormalization(xadjust, yadjust);
  FormatXY( szTemp, sizeof(szTemp), Zgrid.x(ix)+xadjust,
                                       Zgrid.y(iy)+yadjust  ) ;
  Buf << setprecision(5) << szTemp ;
  if( Zgrid.z(ix,iy) >= 0.0 )
               Buf << " " << (Zgrid.z(ix,iy)-ScatterData.zAdjust());
  Buf << ends;

  char *szBuf = Buf.str();
  SetTextAlign( hdc, align );
  int x = ScaleX(Zgrid.x(ix));
  int y = ScaleY(Zgrid.y(iy));
  TextOut( hdc, x, y, szBuf, strlen(szBuf) );
  delete szBuf;

  MoveToEx( hdc, x-MarkSize, y, NULL); LineTo( hdc, x+MarkSize, y);
  MoveToEx( hdc, x, y-MarkSize, NULL); LineTo( hdc, x, y+MarkSize);

}
//********************************************************
//     D r a w 2 d C o r n e r s 
//********************************************************
static void Draw2dCorners( HDC &hdc ) 
{
  float xmin, xmax, ymin, ymax;
  long ix, iy; 
  GetZoomedUser( xmin, xmax, ymin, ymax );
  CloseTo2dGrid( xmin, ymin, ix, iy );
  Draw2dIntersection( hdc, ix, iy, TA_BOTTOM );
  CloseTo2dGrid( xmin, ymax, ix, iy );
  Draw2dIntersection( hdc, ix, iy, TA_TOP );
  CloseTo2dGrid( xmax, ymin, ix, iy );
  Draw2dIntersection( hdc, ix, iy, TA_BOTTOM|TA_RIGHT );
  CloseTo2dGrid( xmax, ymax, ix, iy );
  Draw2dIntersection( hdc, ix, iy, TA_TOP|TA_RIGHT );
  SetTextAlign( hdc, TA_TOP|TA_LEFT );
}
//****************************************************************************
//            D r a w  2 d  X Y A x e s
//****************************************************************************
static void Draw2dXYAxes(HDC &hdc)
{
   const static int Shift = 30;
	static int xmax, ymax, xmin, ymin, TextAlign;
   static double xadjust, yadjust;
   static char label[30];
   LoadNormalization( xadjust, yadjust);
	xmax = ScaleX(xMaxZgrid);          // For axes on the generated grid.
	ymax = ScaleY(yMaxZgrid);
	xmin = ScaleX(xMinZgrid);
	ymin = ScaleY(yMinZgrid);

   // Do the x axis. First the line.
   ymax += Shift;  // Shift the x axis away from the image a tiny bit.
   ymin += Shift;
	MoveToEx( hdc, xmin, ymin, NULL );
   LineTo( hdc, xmax, ymin);
   // The labels.
   FormatCoord( label, sizeof(label), xMinZgrid+xadjust);
   TextOut( hdc, xmin, ymin+Shift, label, strlen(label) );

   TextAlign = SetTextAlign( hdc, TA_TOP|TA_LEFT );
	if( LatLonData)TextOut( hdc, xmax, ymin, " E", 2 );
	else        TextOut( hdc, xmax, ymin, " X", 2 );
   FormatCoord( label, sizeof(label), Zgrid.xmax()+xadjust);
   SetTextAlign( hdc, TA_TOP|TA_RIGHT);
   TextOut( hdc, xmax, ymin+Shift , label, strlen(label) );
   // Now do the y axes.

   xmax -= Shift;  // Shift y axis back
   xmin -= Shift;
   ymax -= Shift;  // And move the x axis.
   ymin -= Shift;
	MoveToEx( hdc, xmin, ymin, NULL );
	LineTo( hdc, xmin, ymax );
   SetTextAlign( hdc, TA_BOTTOM|TA_RIGHT);
   FormatCoord( label, sizeof(label), Zgrid.ymin()+yadjust );
   TextOut( hdc, xmin, ymin, label, strlen(label));
	SetTextAlign( hdc, TA_BOTTOM|TA_LEFT );
	if( LatLonData) TextOut( hdc, xmin, ymax, " N", 2 );
	else         TextOut( hdc, xmin, ymax, " Y", 2 );
   SetTextAlign( hdc, TA_BOTTOM|TA_RIGHT);
   FormatCoord( label, sizeof(label), Zgrid.ymax()+yadjust);
   TextOut( hdc, xmin, ymax, label, strlen(label) );
	TextAlign = SetTextAlign( hdc, TextAlign); //restore text alignment.
}

//****************************************************************************
//            D r a w    L e g e n d   Box
//****************************************************************************
static void DrawLegendBox(HDC &hdc, long x, long y, double z)
{
  static POINT corners[4];
  static COLORREF Colour;
  static HBRUSH hBrush, hOldBrush;
  static HPEN hPen, hOldPen;
  static char label[20];
  static double zadjust;

  corners[0].x = x;
  corners[0].y = y;
  corners[1].x = x+ColourLegendBoxSize;
  corners[1].y = y;
  corners[2].x = x+ColourLegendBoxSize;
  corners[2].y = y+ColourLegendBoxSize;
  corners[3].x = x;
  corners[3].y = y+ColourLegendBoxSize;
  
  Colour = ChooseZColour( z );
  hBrush = CreateSolidBrush( Colour );
  hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
  hPen = CreatePen( PS_SOLID, 0, Colour);
  hOldPen   = (HPEN)SelectObject( hdc, hPen );

  Polygon( hdc, corners, 4 );

  SelectObject( hdc, hOldBrush );
  assert( DeleteObject( hBrush));
  SelectObject( hdc, hOldPen );
  assert( DeleteObject( hPen ) );
  
  zadjust = CgridZadjust;
  sprintf( label, "%.5g", (double) z-zadjust );
  TextOut( hdc, x+ColourLegendBoxSize*3/2,
                y, label, strlen(label) ) ;
}

//************************************************************
//       Z g r i d    I s   D i s p l a y a b l e
//***********************************************************
static bool ZgridIsDisplayable()
{
  static int xSize, ySize, i, j;
  if( Zgrid.xsize() <= 3 ) return false; // there is no grid.

  if( Zgrid.zmin() != Zgrid.zmax() ) return true; // a good sign coordinates are defined.

  xSize = Zgrid.xsize();                          // See if any grid points defined.
  ySize = Zgrid.ysize();
  for( i = 0; i < xSize; i++ )
  { for( j = 0; j < ySize; j++ ) if( Zgrid.z(i,j) >= 0.0 ) return true; } // if anything defined ok.

  return  false; // The entire grid is undefined.
}

//****************************************************************************
//            D r a w    C o l o u r  L e g e n d
//****************************************************************************
static void DrawColourLegend(HDC &hdc)
{
  static int LegendXposn, LegendYposn, i;
  static double increment, max, min;
  static RECT LegendRect;
  static long LittleBit;
  static int NumberOfBoxes;

  if( !ColouredGrid || !IncludeColourLegend ) return;
  if( !ZgridIsDisplayable() ) return;

 static POINT temp[2];  // Determine size of legend box in pixels.
 temp[0].x = 0;
 ColourLegendBoxSizeInPixels = 20 ;
 if( TextSize > 120 )  ColourLegendBoxSizeInPixels = TextSize/6;
 temp[1].x = ColourLegendBoxSizeInPixels;
 temp[0].y = temp[1].y = 0;
 DPtoLP( hdc, (LPPOINT)&temp, 2 );
 ColourLegendBoxSize = labs( temp[1].x - temp[0].x);
 LittleBit = ColourLegendBoxSize/5;

  LegendXposn = ClientRect.right - ColourLegendBoxSize*4;
  if( PaintState == IDM_3DSURFACE ) LegendXposn -= ColourLegendBoxSize/2;

  max = ColourZmax;
  min = ColourZmin;
  increment = (max - min)/9.0 ;           // Look for roundoff problems here?
  increment = AtPrecision( increment, 1 );
  if( increment == 0.0 ) NumberOfBoxes = 1;
  else NumberOfBoxes = int((max-min)/increment)+1;
  if( min+increment*NumberOfBoxes < max ) NumberOfBoxes +=1;

  LegendYposn = ClientRect.bottom + (ClientRect.top-ClientRect.bottom)/2
                - ColourLegendBoxSize*(NumberOfBoxes/2);

  LegendRect.bottom = LegendYposn - LittleBit;
  LegendRect.left  = LegendXposn - LittleBit;
  LegendRect.top    = LegendYposn + ColourLegendBoxSize*NumberOfBoxes + LittleBit;
  LegendRect.right   = LegendXposn + ColourLegendBoxSize*4 - LittleBit*2;
  //hBrush = CreateSolidBrush( BackgroundPen ) ;
  FillRect( hdc, &LegendRect,(HBRUSH) hBackgroundPen );

  LegendYposn = LegendYposn + ColourLegendBoxSize*(NumberOfBoxes-1);
  for( i = 0; i<NumberOfBoxes; i++ )
    DrawLegendBox( hdc, LegendXposn, LegendYposn - i*ColourLegendBoxSize, min+i*increment );
}
//****************************************************************************
//            D r a w    V o l u m e  D i f f e r e n c e
//****************************************************************************
static void DrawVolumeDifference(HDC &hdc)
{
  static long Xposn, Yposn, DistanceFromEdge, VerticalSpace;
  static char label[50];
  if( !DisplayVolumeDifference ) return;
  if( Cgrid.xsize() < 3 ) return; // If no colour layer don't do it.
  static POINT temp[2];  // Determine size of legend box in pixels.
  temp[0].x = 0;
  temp[1].x = 13;   // Character vertical spacing.
  temp[0].y = temp[1].y = 0;
  DPtoLP( hdc, (LPPOINT)&temp, 2 );
  VerticalSpace = labs( temp[1].x - temp[0].x);
  DistanceFromEdge = VerticalSpace*3;
  Xposn = ClientRect.right  - DistanceFromEdge*4;
  Yposn = ClientRect.bottom - DistanceFromEdge*2;
  TextOut( hdc, Xposn, Yposn, "Volume Difference " , 18);
  Yposn += VerticalSpace;
  sprintf( label, "Positive =   %.*g ", 9, VolumeDifferencePositive) ;
  TextOut( hdc, Xposn, Yposn, label, strlen(label) );
  sprintf( label, "Negative = %.*g ", 9, VolumeDifferenceNegative) ;
  Yposn += VerticalSpace;
  TextOut( hdc, Xposn, Yposn, label, strlen(label) );
  sprintf( label, "Total  =      %.*g ", 9, VolumeDifference) ;
  Yposn += VerticalSpace;
  TextOut( hdc, Xposn, Yposn, label, strlen(label) );
}
//********************************************************
//        A p p l y  V i e w
//********************************************************
static void ApplyView()
{
 Zgrid.applyview();
 if( Sgrid.xsize() > 2 ) Sgrid.applyview( );
 if( Dgrid.xsize() > 2 ) Dgrid.applyview( );
}
//********************************************************
//        R e m o v e  V i e w
//********************************************************
static void RemoveView()
{
 Zgrid.removeview();
 if( Sgrid.xsize() > 2 ) Sgrid.removeview( );
 if( Dgrid.xsize() > 2 ) Dgrid.removeview( );
}

//********************************************************
//        S c a t t e r d a t a  I s  O n   G r i d
//********************************************************
static bool ScatterdataIsOnGrid(  )
{
  if( ScatterData.Size() < 3 ) return true; // No points are on grid by definition?
  if( ScatterData.xMax() < xMinZgrid ||
  		ScatterData.xMin() > xMaxZgrid ||
	 	ScatterData.yMax() < yMinZgrid ||
		ScatterData.yMin() > yMaxZgrid )
   {
		if( GridLock )
       { if( QuikGridChangedImage ) PostMessage( WindProcHwnd, WM_COMMAND, IDM_NOTIFYUSER, IDS_SCATDATANOTONGRIDLOCK );}
		else
       { if( QuikGridChangedImage )  PostMessage( WindProcHwnd, WM_COMMAND, IDM_NOTIFYUSER, IDS_SCATDATANOTONGRID ); } 
      return false;
    }
  return true;
}
//************************************************************
//       S c a t t e r d a t a  I s  D r a w a b l e
//***********************************************************
static bool ScatterdataIsDrawable( )
{
  if( ScatterData.Size() < 3 ) return false;
  if( !ScatterdataIsOnGrid( ) ) return false;
  return true;
}

//************************************************************
//       O u t l i n e  I s  D r a w a b l e
//***********************************************************
static int OutlineIsDrawable(  )
{
  static float xRange, yRange;
  if( OutLine.Size() < 2 ) return FALSE; // no outline data.

    xRange = xMaxZgrid-xMinZgrid;
    yRange = yMaxZgrid-yMinZgrid;

    if( OutLine.xMax() < xMinZgrid-xRange ||
		  OutLine.xMin() > xMaxZgrid+xRange ||
		  OutLine.yMax() < yMinZgrid-yRange ||
 		  OutLine.yMin() > yMaxZgrid+yRange )
  {
       { if( QuikGridChangedImage ) PostMessage( WindProcHwnd, WM_COMMAND, IDM_NOTIFYUSER, IDS_OUTLINENOTONGRID ); }
	 return FALSE;
  }
 return TRUE;
}

//********************************************************
//        Q G S e l e c t  F o n t /  Q G D e l e t e  F o n t
//********************************************************
static void QGSelectFont()
{
 FLOAT Dpi ;
 LOGFONT lf ;
 POINT pt ;

 SaveDC(hdc) ;

 SetGraphicsMode(hdc, GM_ADVANCED) ;
 ModifyWorldTransform(hdc, NULL, MWT_IDENTITY) ;
 SetViewportOrgEx(hdc, 0, 0, NULL) ;
 SetWindowOrgEx(hdc, 0, 0, NULL) ;

 Dpi = (FLOAT)GetDeviceCaps(hdc, LOGPIXELSY) ;
 pt.x = 0; 
 pt.y = (int)(TextSize * Dpi / 72) ;
 DPtoLP(hdc, &pt, 1) ;

 lf.lfHeight         = - (long) std::fabs ((pt.y) / 10.0 + 0.5) ;
 lf.lfWidth          = 0 ;
 lf.lfEscapement     = 0 ;
 lf.lfOrientation    = 0 ;
 lf.lfWeight         = 0 ;
 lf.lfItalic         = FALSE ;
 lf.lfUnderline      = FALSE ;
 lf.lfStrikeOut      = FALSE;
 lf.lfCharSet        = 0 ;
 lf.lfOutPrecision   = 0 ;
 lf.lfClipPrecision  = 0 ;
 lf.lfQuality        = 0 ;
 lf.lfPitchAndFamily = 0 ;

 strcpy(lf.lfFaceName, "Times New Roman") ;

 hFont = CreateFontIndirect(&lf) ;

 RestoreDC(hdc, -1) ;
 assert( SelectObject( hdc, hFont )!=NULL);
 assert( SelectObject( hdc, hBackgroundPen )!=NULL);
 SetBkColor( hdc, BackgroundPen );
 SetTextColor( hdc, TextPen );

}

static void QGDeleteFont( )
{
  assert( DeleteObject( hFont ));
}

//****************************************************************
//        Q G S e l e c t   B o l d   P e n
//****************************************************************
static HPEN QGSelectPen( const COLORREF &Colour)
{
// Try to select a pen width of several pixels.
 static POINT temp[2];
 temp[0].x = 0;
 temp[1].x = 2;
 temp[0].y = temp[1].y = 0;
 DPtoLP( hdc, (LPPOINT)&temp, 2 );
 BoldWidth = abs(temp[1].x-temp[0].x);
 if( !BlackAndWhite )
 return CreatePen( PS_SOLID, BoldWidth, Colour );
 else
 return CreatePen( PS_SOLID, BoldWidth, BlackPen );
}

//********************************************************
//        A p p l y    P e n
//********************************************************
static void ApplyPen( const HPEN &hPen  )
{
  if( !BlackAndWhite ) assert( SelectObject( hdc, hPen )!=NULL);
  hPenInUse = hPen;
}

//********************************************************
//        c r e a t e   S p e c i a l   P e n
//********************************************************

static void CreateSpecialPen( const COLORREF &Colour, const int &Bold )
{
  if( BlackAndWhite || (!ColourContours) ) return;
  if( Colour != ContourPen )
       {
         if( Bold ) hSpecialPen = QGSelectPen( Colour );
			else       hSpecialPen = CreatePen( PS_SOLID, 0, Colour);
         SpecialPenInUse = SpecialPenWasUsed = TRUE;
       }
}

//********************************************************
//        R e m o v e   S p e c i a l   P e n
//********************************************************

static void RemoveSpecialPen( const COLORREF &Colour )
{
  if( BlackAndWhite || (!ColourContours) ) return;
  SelectObject( hdc, hBlackPen);
  if( Colour != ContourPen ) assert( DeleteObject( hSpecialPen ));
  SpecialPenInUse = FALSE;
}

//********************************************************
//      S e l e c t   P e n
//********************************************************
//  - we want to choose the Bold face alternative.
//
void SelectPen( int pen )
{
  if( !BlackAndWhite)
   {
      if( SpecialPenInUse) assert(SelectObject( hdc, hSpecialPen)!=NULL);
      else {
             if( pen == 2 )  assert(SelectObject( hdc, hBoldPen )!=NULL);
             else            assert(SelectObject( hdc, hPenInUse )!=NULL);
           }
   }
  else  // Is black and white display.
   {
      if( pen == 2 )  SelectObject( hdc, hBoldPen );
      else            SelectObject( hdc, hBlackPen );
   }
}

//************************************************************
//       D r a w   2 d   O u t l i n e
//***********************************************************
static void Draw2dOutline()
{
   static int ScaledMarkSize;

	if( !OutlineIsDrawable() ) return;
	PolyBuffer( 0, 0, -1 ) ;
	if( BoldOutline) hOutlinePen = QGSelectPen( OutlinePen);
	else hOutlinePen = CreatePen( PS_SOLID, 0, OutlinePen);
   ApplyPen( hOutlinePen);
	long SizeOfOutline = OutLine.Size();

   for ( long i = 0; i < SizeOfOutline; i++ )
   {
     int x = ScaleX(OutLine.x(i));
     int y = ScaleY(OutLine.y(i));
     if( OutLine.flags(i)&1 ) PolyBuffer( x, y, 0 );
     else PolyBuffer( x, y, 1 );
     if( OutLine.comment(i)[0] != NULL )
     {
       TextOut( hdc, x+MarkFraction, y+MarkFraction, OutLine.comment(i), strlen(OutLine.comment(i)) );
       ScaledMarkSize = CurrWin.xr/200;

       hOldPen = (HPEN)SelectObject( hdc, hBlackPen );
       MoveToEx( hdc, x-ScaledMarkSize, y, NULL); LineTo( hdc, x+ScaledMarkSize, y);
		 MoveToEx( hdc, x, y-ScaledMarkSize, NULL); LineTo( hdc, x, y+ScaledMarkSize);
       SelectObject( hdc, hOldPen ); 
     }
   }
	PolyBuffer( 0, 0, -1 );
	SelectObject( hdc, hBlackPen);
	assert( DeleteObject( hOutlinePen ));
}

//********************************************************
//        P a i n t    2 d    S u r f a c e
//********************************************************
void Paint2dSurface( HDC &hdc, const ContourOptionsType &Options )
{
 static int i, j,
            Draw,
            xold, yold,
            TextAlign,
            xSize, ySize,
				temp,
				xoff, yoff,
				xirange, yirange, irange;

 static const float RadianConversionFactor = .017453 ;

 static float ArrowScale, radians;
 static int *xScaled, *yScaled;
 static long NumberOfPoints;

 if( Zgrid.xsize() < 3 ) return; // there is no grid.

 ApplyView();

 xMaxZgrid = Zgrid.xmax();    // Set up original zgrid max & mins as globals.
 yMaxZgrid = Zgrid.ymax();
 zMaxZgrid = Zgrid.zmax();
 xMinZgrid = Zgrid.xmin();
 yMinZgrid = Zgrid.ymin();
 zMinZgrid = Zgrid.zmin();

 SpecialPenWasUsed = FALSE;
 QGSelectFont( );
 hBoldPen = QGSelectPen( ContourPen );
 BlackAndWhite = Options.blackwhite;

 ApplyPen( hBlackPen );


 xSize = Zgrid.xsize();
 ySize = Zgrid.ysize();

 NumberOfPoints = ScatterData.Size();
 if( CreatingAMetaFile )
 {
    // Do a move to grid extents to force constant image sizing for metafiles.
   static const int ExtraSpace = 0;
   MoveToEx( hdc, ScaleX(Zgrid.x(0))-ExtraSpace, ScaleY(Zgrid.y(0))-ExtraSpace, NULL );
   LineTo(   hdc, ScaleX(Zgrid.x(0))-ExtraSpace, ScaleY(Zgrid.y(0))-ExtraSpace);
   MoveToEx( hdc, ScaleX(Zgrid.x(xSize-1))+ExtraSpace, ScaleY(Zgrid.y(ySize-1))+ExtraSpace, NULL );
   LineTo(   hdc, ScaleX(Zgrid.x(xSize-1))+ExtraSpace, ScaleY(Zgrid.y(ySize-1))+ExtraSpace);
 } // End if( CreatingAMetaFile )

 if( Options.threedaxes)
     {
       ApplyPen( hAxisPen );
       Draw2dXYAxes( hdc );
     }

 if( (Options.grid||Options.hiddengrid||ColouredGrid) && ZgridIsDisplayable() )
  {
    PolyBuffer( 0, 0, -1 );
    ApplyPen( hGridPen );
	 // Precompute scaled x and y coordinate positions.
	 xScaled = new int[xSize];
	 assert( xScaled != 0 );
	 yScaled = new int[ySize];
	 assert( yScaled != 0 );
	 for( i=0; i<xSize; i++ ) xScaled[i]=ScaleX(Zgrid.x(i));
    for( i=0; i<ySize; i++ ) yScaled[i]=ScaleY(Zgrid.y(i));

	 if( ColouredGrid ) Hatch2D( xScaled, yScaled, hdc );
    
	 else
	 {
	  for(  i=0; i< xSize; i++)
         {
		    Draw = 0;  temp = xScaled[i];
          // Don't draw line if outside the visible screen.
          if( (temp > ClientRect.left && temp < ClientRect.right)||CreatingAMetaFile )
           {
		       for( j=0; j<ySize; j++)
			     Draw2dGridLine( temp, yScaled[j], Zgrid.z(i,j), Draw);
		      }
         } // end for( i=0....

	  for( j=0; j< ySize; j++)
      {
		  Draw = 0; temp = yScaled[j];
        // Don't draw line if outside the visible screen.
        if( (temp > ClientRect.top && temp < ClientRect.bottom)||CreatingAMetaFile )
        {
		   for( i=0; i<xSize; i++)
			 Draw2dGridLine( xScaled[i], temp, Zgrid.z(i,j),Draw);
			}
		} // end for j=0...
	 Draw = 0; // force Draw2dLine and PolyBuffer to flush outstanding draw.
	 Draw2dGridLine( 0, 0, -99., Draw );
	 PolyBuffer( 0, 0, -1 );
	 } // end if(ColouredGrid... Else...
	delete [] xScaled;
	delete [] yScaled;
  } // end if( options.grid...

  // Do contours in 2d mode.
	if( Options.contours && ZgridIsDisplayable())
  {
	long SavedSize = Save2dContours.Size();
	PlotTo( 0, 0, -9 ); // reset PlotTo so will pick up pen change.
   ApplyPen( hContourPen );
   if( SavedSize > 0 )
     {                 // Redraw contours from saved memory (gotta save colours sometime).
       for( long i =0; i<SavedSize; i++ )
		 PlotTo( Save2dContours.x(i), Save2dContours.y(i), Save2dContours.Pen(i));
		 if( Options.labelcontours ) LabelcDisplay( hdc ); // gotta save colours here too.
     }  // End if( SaveidSize > 0
	else               // Draw contours from scratch -
	  {
		 ContourMode = TwoD;  // function DoLineTo will save them.
		 if( Options.labelcontours) LabelcInit(); // Initialize label collection.
		 for( int i=0; i<NumberOfContours; i++ )
			 {
             ContourValue = ContourLineValue(i);
             ContourValueIndex = i;
             PenMode = 1; 
				 if( ContourLineBold(i) ) PenMode = 2;
             if( ContourLineLabel(i)&&Options.labelcontours ) LabelcStart();
             CreateSpecialPen( ContourLineColour(i), ContourLineBold(i) );
				 Contour( Zgrid, ContourValue );
             PolyBuffer( 0, 0, -1 );  // Flush line buffer.
             if( ContourLineBold(i) ) PenMode = 1;
             if( ContourLineLabel(i)&&Options.labelcontours ) LabelcStop();
             RemoveSpecialPen( ContourLineColour(i));
			 }
		 if( Options.labelcontours) { LabelcDisplay( hdc); LabelcStop(); }
	  } // end else for if( SavedSize > 0
 PolyBuffer( 0, 0, -1 );
}   // end if( Options.contours && . . .

// Insert Kludge to display wind speed and direction.
  if( (Sgrid.xsize() > 2) && (Dgrid.xsize() > 2)
   && (Dgrid.zmax() >0.0) && (Sgrid.zmax() >0.0)  ) // Do if the grids exist.
     {
		 // Draw arrows for grid coordinates instead of the grid itself.
		 // Determine scale for the speed. (Make it a side of a grid square).
		 // Precompute scaled x and y coordinate positions.
      PolyBuffer( 0, 0, -1 );
      ApplyPen( hArrowPen );
		xScaled = new int[xSize];
		assert( xScaled != 0 );
		yScaled = new int[ySize];
		assert( yScaled != 0 );
		for( i=0; i<xSize; i++ ) xScaled[i]=ScaleX(Zgrid.x(i));
		for( i=0; i<ySize; i++ ) yScaled[i]=ScaleY(Zgrid.y(i));
		xirange = abs(xScaled[0]-xScaled[1]);
		yirange = abs(yScaled[0]-yScaled[1]);
		irange = max( xirange, yirange);
		float SpeedMax = Sgrid.zmax();
		assert( SpeedMax > 0.0 );
		ArrowScale = (irange/SpeedMax)*ArrowSize ; // Length of Speed Arrow here.
		for( i = 0; i < xSize; i++ )
		  for( j = 0; j < ySize; j++ )
		  { if( Dgrid.z(i,j) > 0.0 && Sgrid.z(i,j) > 0.0 )
         {
			 assert( MoveToEx( hdc, xScaled[i], yScaled[j], NULL ));
			 radians = -(Dgrid.z(i,j)+180.)*RadianConversionFactor;
			 xoff = (sin( radians ) * Sgrid.z(i,j))*ArrowScale;
			 yoff = (cos( radians ) * Sgrid.z(i,j))*ArrowScale;
			 assert(LineTo( hdc, xScaled[i]+xoff, yScaled[j]+yoff ));
          // Now put half an arrow on to indicate direction.
			 radians -= .24; // about 14 degrees.
			 xoff = (sin( radians ) * Sgrid.z(i,j))*(ArrowScale*.7);
			 yoff = (cos( radians ) * Sgrid.z(i,j))*(ArrowScale*.7);
          assert(LineTo( hdc, xScaled[i]+xoff, yScaled[j]+yoff ));
         }
		  }
		PolyBuffer( 0, 0, -1 );
		delete [] xScaled;
		delete [] yScaled;
} // end display wind speed and direction grids.

 if( ScatterdataIsDrawable()&&Options.marks )
 {
  ApplyPen( hDataPointPen );

  long nSample = XpandSample();
  for( long i = 0; i<NumberOfPoints; i += nSample )
   {
     float xf = ScatterData.x(i);
     float yf = ScatterData.y(i);
	  int x = ScaleX(xf);
     int y = ScaleY(yf);

	  if( !DataPointsOnGridOnly || InTheGrid( xf, yf ) ) DrawMark( x, y, i );

     
     if( Options.lines )
       {
         if( i == 0 )
			  { xold = x; yold = y; }
         else
			  { MoveToEx( hdc, xold, yold, NULL );
             if( !(ScatterData.flags(i)&1) ) LineTo( hdc, x, y );
	          xold = x; yold = y; }
		 }  // end if (Options.lines
    }  // end for( long i = 0, i<NumberOfPoints...

 } // end if( ScatterdataIsDrawable() &&options.marks...

 TextAlign = SetTextAlign( hdc, TA_BOTTOM|TA_LEFT );
 TitleYposn = ScaleY(yMax);
 TextOut( hdc, TitleXposn, TitleYposn, szTitle, lstrlen( szTitle ) );
 TextAlign = SetTextAlign( hdc, TextAlign); //restore text alignment.

 DrawColourLegend( hdc );
 DrawVolumeDifference( hdc );

 SelectObject( hdc, hBlackPen );
 if( Options.showcorners ) Draw2dCorners( hdc );

 Draw2dOutline();

 RemoveView();
 QGDeleteFont();
 DeleteObject( hBoldPen );
 if( SpecialPenWasUsed ) PaintContourReset();
}

//*****************************************************************
//    S e t D X F o u t p u t  M o d e
//*****************************************************************
void SetDXFoutputMode()
{
  ContourMode = DXFoutput ;
}
//*****************************************************************
//            D o   L i n e   T o
//   Called by the Contour function to do the actual drawing 
//*****************************************************************
void DoLineTo( float x, float y, int Drawing)
{
  static int xplot, yplot, pen;
  static float zRotated, xRotated, yRotated;

  switch ( ContourMode )
  {
    case ThreeD:  
	  zRotated = ContourValue;
     xRotated = x;
     yRotated = y;
     Rotate( yRotated, xRotated, zRotated ); // x <-> y deliberate...
     if(Projection != 0.0 ) Project( xRotated, yRotated, zRotated);
     pen = 0;
     if( Drawing ) pen = PenMode;
     xplot = ScaleX( xRotated );
     yplot = ScaleY( yRotated );
	  if( !SpecialPenWasUsed) Save3dContours.SetNext( xplot, yplot, (unsigned char) pen);
	  PlotTo( xplot, yplot, pen);
     LabelcIntercept( xplot, yplot, pen, ContourValueIndex);
     break;   

   case TwoD:
     pen = 0;
     if( Drawing ) pen = PenMode;
     xplot = ScaleX( x );
     yplot = ScaleY( y );
     if( !SpecialPenWasUsed) Save2dContours.SetNext( xplot, yplot, (unsigned char) pen);
	  PlotTo( xplot, yplot, pen);
	  LabelcIntercept( xplot, yplot, pen, ContourValueIndex);
     break;

   case DXFoutput: 
     OutputDXFContourLine( x, y, Drawing );
     break;

   default:
     assert( FALSE );

  }
}

//*********************************************************
//      R o t a t e  M a x  M i n
//*********************************************************
// Service routine used only by Paint3dContour & Paint3dHideGrid.
static void RotateMaxMin( float x, float y, float z)
{
  Rotate( y, x, z ); // y <-> x deliberate.
  xMin = min( xMin, x );
  xMax = max( xMax, x );
  yMin = min( yMin, y );
  yMax = max( yMax, y );
  zMin = min( zMin, z );
  zMax = max( zMax, z );
}
//*********************************************************
//   S c a l e 3 d C u b e
//*********************************************************
void Scale3dCube()
{
 // Determine scaling factors by rotating the cube corners.
 // *MUST* be called previous to Paint3dSurface (in quikgrid.cpp).

  // If zmax - zmin == 0 there is no surface but need to set xyRange anyhow.
  // to avoid divide by zero exceptions.
  if( (Zgrid.zmax()-Zgrid.zmin()) == 0.0 )
  	  {// NotifyUser( "Attempt to scale grid with no z range ignored");
        { if( QuikGridChangedImage ) PostMessage( WindProcHwnd, WM_COMMAND, IDM_NOTIFYUSER, IDS_GRIDNOZRANGE );}
       xyRange = xyRangeMin; return; }

  xMaxZgrid = Zgrid.xmax();    // Set up original zgrid max & mins as globals.
  yMaxZgrid = Zgrid.ymax();
  zMaxZgrid = Zgrid.zmax();
  xMinZgrid = Zgrid.xmin();
  yMinZgrid = Zgrid.ymin();
  zMinZgrid = Zgrid.zmin();

  ApplyzRatio();

  xMin = Zgrid.xmin();
  yMin = Zgrid.ymin();
  zMin = Zgrid.zmin();
  Rotate( yMin, xMin, zMin);
  xMax = xMin;
  yMax = yMin;
  zMax = zMin;
  RotateMaxMin( Zgrid.zmin(), Zgrid.ymin(), Zgrid.zmax() );
  RotateMaxMin( Zgrid.xmin(), Zgrid.ymax(), Zgrid.zmin() );
  RotateMaxMin( Zgrid.xmin(), Zgrid.ymax(), Zgrid.zmax() );
  RotateMaxMin( Zgrid.xmax(), Zgrid.ymin(), Zgrid.zmin() );
  RotateMaxMin( Zgrid.xmax(), Zgrid.ymin(), Zgrid.zmax() );
  RotateMaxMin( Zgrid.xmax(), Zgrid.ymax(), Zgrid.zmin() );
  RotateMaxMin( Zgrid.xmax(), Zgrid.ymax(), Zgrid.zmax() );

  xRange = xMax - xMin;
  yRange = yMax - yMin;
  zRange = zMax - zMin;
  xyRange = max( xRange , yRange);
  if( xyRange <= xyRangeMin ) xyRange = xyRangeMin;
  assert( xyRange > 0.0 );
}
//**********************************************************
//          D r a w   3 d  G r i d   L i n e
//**********************************************************
// Service routine.
static void Draw3dGridLine( int i, int j, int &Draw)
{   int x, y;
   x = xyGrid.x(i,j);
   y = xyGrid.y(i,j);
     if( x == 0 ) Draw = 0;
     else
      {
	    if( Draw == 0 )
		 {
			PlotTo( x, y, 0 );
	      Draw = 1;
       }
		 else
         PlotTo( x, y, 1 );
      }
}
//**********************************************************
//          D r a w   3 d  G r i d   L i n e
//**********************************************************
// Service routine.
static void Draw3dGridLine( const float xa, const float ya, const float za,
                            const int Draw, const char c[], HDC &hdc )
{
	static int ix, iy;
   static float x, y, z;
   x = xa;
   y = ya;
   z = za; 
	Rotate( y, x, z);
	if( Projection != 0.0 ) Project( x, y, z);

	ix = ScaleX( x );
	iy = ScaleY( y );

	if( Draw == 0 ) PlotTo( ix, iy, 0 );
	else PlotTo( ix, iy, 1 );
   if( c[0] != ' ' )
    { PolyBuffer(0,0,-1); TextOut( hdc, ix, iy, c, strlen(c) );}
}
//************************************************************
//            D r a w  3 d   C o n t o u r s
//************************************************************
//  Service routine.....
static void Draw3dContours( HDC &hdc, const ContourOptionsType &Options   )
{
  static long SavedSize;

  ContourMode = ThreeD;
  ApplyPen( hContourPen );
  SavedSize = Save3dContours.Size();
  PlotTo( 0, 0, -9 );  // reset plotto for pen changes. 
  if( SavedSize == 0 )
	{
    if( Options.labelcontours) LabelcInit(); // Initialize label collection.
    for( int i = 0; i<NumberOfContours; i++)
     {
      Real3DContourValue = ContourLineValue(i);
      ContourValueIndex = i; 
      ContourValue = Zgrid.zmap( Real3DContourValue );
      PenMode = 1; 
      if( ContourLineBold(i) ) PenMode = 2;
      if( ContourLineLabel(i)&&Options.labelcontours ) LabelcStart();
      CreateSpecialPen( ContourLineColour(i), ContourLineBold(i) );
      Contour( Zgrid, ContourValue );
      PolyBuffer( 0, 0, -1 );
      if( ContourLineBold(i) ) PenMode = 1;
      if( ContourLineLabel(i)&&Options.labelcontours ) LabelcStop();
      RemoveSpecialPen( ContourLineColour(i));
	  }
	  if( Options.labelcontours) { LabelcDisplay( hdc); LabelcStop(); }
   }
   else
   {
    for( long i =0; i<SavedSize; i++ )
		PlotTo( Save3dContours.x(i), Save3dContours.y(i), Save3dContours.Pen(i));
	 if( Options.labelcontours ) LabelcDisplay( hdc );
   }
  PolyBuffer( 0, 0, -1 ); 
}
//********************************************************
//        C l o s e T o 3 d G r i d
//********************************************************
static float CloseTo3dGrid( const int ix, const int iy, long &IX, long &IY )
 {
  // Nothing for it but to brute force it. Run through the entire
  // rotated grid looking for the closest intersection.
 float DistanceSquared, TestSquared, xdist, ydist;
 int xSize = Zgrid.xsize();
 int ySize = Zgrid.ysize();
 DistanceSquared = 32000.0*32000.0;
 if (DrawingOptions.grid||DrawingOptions.hiddengrid)
 {

  IX = IY = -1;
  for   (  int i = 0; i<xSize; i++ )
   { for(  int j = 0; j<ySize; j++ )
     { if ( (xyGrid.x(i,j) != 0) && Visible( xyGrid.x(i,j),xyGrid.y(i,j) ) )
       {
          xdist = ix - xyGrid.x(i,j);
          ydist = iy - xyGrid.y(i,j);
			 TestSquared = xdist*xdist + ydist*ydist ;
          if( TestSquared < DistanceSquared )
				 {DistanceSquared = TestSquared; IX = i; IY = j ;}
       }
     }
   }
  }
  return DistanceSquared;
 }
//********************************************************
//    D r a w 3 d I n t e r s e c t i o n
//********************************************************
static void Draw3dIntersection( HDC &hdc, long ix, long iy, UINT align )
{
  static double xadjust, yadjust;
  char szTemp[50];

  LoadNormalization( xadjust, yadjust );

  ostrstream Buf;

  FormatXY( szTemp, sizeof(szTemp), Zgrid.x(ix)+xadjust,
                                    Zgrid.y(iy)+yadjust ) ;
  Buf << szTemp << " " << (Zgrid.z(ix,iy)-ScatterData.zAdjust());
  Buf << ends;

  char *szBuf = Buf.str();
  SetTextAlign( hdc, align );
  int x = xyGrid.x(ix,iy);
  int y = xyGrid.y(ix,iy);
  TextOut( hdc, x, y, szBuf, strlen(szBuf) );
  delete szBuf;

  MoveToEx( hdc, x-MarkSize, y, NULL); LineTo( hdc, x+MarkSize, y);
  MoveToEx( hdc, x, y-MarkSize, NULL); LineTo( hdc, x, y+MarkSize);

}
//********************************************************
//     D r a w 3 d C o r n e r s
//********************************************************
static void Draw3dCorners( HDC &hdc )
{
  int xmin, xmax, ymin, ymax;
  long ix, iy;
  xmin = ClientRect.left;
  xmax = ClientRect.right;
  ymin = ClientRect.bottom;
  ymax = ClientRect.top;
  CloseTo3dGrid( xmin, ymin, ix, iy );
  if( ix >= 0 ) Draw3dIntersection( hdc, ix, iy, TA_BOTTOM );
  CloseTo3dGrid( xmin, ymax, ix, iy );
  if( ix >= 0 ) Draw3dIntersection( hdc, ix, iy, TA_TOP );
  CloseTo3dGrid( xmax, ymin, ix, iy );
  if( ix >= 0 ) Draw3dIntersection( hdc, ix, iy, TA_BOTTOM|TA_RIGHT );
  CloseTo3dGrid( xmax, ymax, ix, iy );
  if( ix >= 0 ) Draw3dIntersection( hdc, ix, iy, TA_TOP|TA_RIGHT );
  SetTextAlign( hdc, TA_TOP|TA_LEFT );
}
//****************************************************************************
//            D r a w  3 d  X Y A x e s
//****************************************************************************
static void Draw3dXYAxes(HDC &hdc)
{
   static float xmax, ymax, xmin, ymin, zmax, zmin, xtemp, ytemp, zadjust;
   static int TextAlign;
	xmax = Zgrid.xmax();          // For axes on the generated grid.
	ymax = Zgrid.ymax();
	xmin = Zgrid.xmin();
	ymin = Zgrid.ymin();
	zmax = Zgrid.zmax();
	zmin = Zgrid.zmin();
   zadjust = ScatterData.zAdjust();

// if the axes are to be plotted at a different z minimum.
   if( (ZmaxLabel != ZminLabel) &&
       (ZmaxLabel > zMinZgrid-zadjust) &&
       (ZminLabel < zMaxZgrid-zadjust) )
  {
    xtemp = ytemp = 0.0;
    zmin = ZminLabel+zadjust;
    Zgrid.map( xtemp, ytemp, zmin );
  }

   Draw3dGridLine( xmin, ymin, zmin, 0, " ", hdc);
   if( LatLonData)Draw3dGridLine( xmax, ymin, zmin, 1, "E", hdc);
   else Draw3dGridLine( xmax, ymin, zmin, 1, "X", hdc);
   Draw3dGridLine( xmin, ymin, zmin, 0, " ", hdc);
   TextAlign = SetTextAlign( hdc, TA_BOTTOM|TA_RIGHT );
   if( LatLonData) Draw3dGridLine( xmin, ymax, zmin, 1, "N", hdc);
   else Draw3dGridLine( xmin, ymax, zmin, 1, "Y", hdc);
   TextAlign = SetTextAlign( hdc, TextAlign); //restore text alignment.
   PolyBuffer( 0, 0, -1 ) ;
}
//****************************************************************************
//            D r a w  3 d  Z A x i s
//****************************************************************************
static void Draw3dZAxis(HDC &hdc)
{
	static float xmin, ymin, zmax, zmin, zadjust, xtemp, ytemp, ZminLabelTemp, ZmaxLabelTemp;
   static char label[20];
	xmin = Zgrid.xmin();
	ymin = Zgrid.ymin();
	zmin = Zgrid.zmin();
	zmax = Zgrid.zmax();
   zadjust = ScatterData.zAdjust();
   ZminLabelTemp = zMinZgrid-zadjust;
   ZmaxLabelTemp = zMaxZgrid-zadjust;

// if customized max and min's are wanted....
   if( (ZmaxLabel != ZminLabel) &&
       (ZmaxLabel > zMinZgrid-zadjust) &&
       (ZminLabel < zMaxZgrid-zadjust) )
  {
    xtemp = ytemp = 0.0;
    zmin = ZminLabel+zadjust;
    zmax = ZmaxLabel+zadjust;
    Zgrid.map( xtemp, ytemp, zmin );
    //NotifyUser( "Zmin at z axis is %g ", zmin);
    Zgrid.map( xtemp, ytemp, zmax );
    ZminLabelTemp = ZminLabel;
    ZmaxLabelTemp = ZmaxLabel;
  }   

   sprintf( label, "%g", ZminLabelTemp );
   Draw3dGridLine( xmin, ymin, zmin, 0, label, hdc);
   sprintf( label, "%g", ZmaxLabelTemp );
   Draw3dGridLine( xmin, ymin, zmax, 1, label, hdc);

// Could draw tic marks on the axis at every contour line interval. 

   PolyBuffer( 0, 0, -1 );
}

//************************************************************
//       D r a w   3 d   O u t l i n e
//***********************************************************
static void Draw3dOutline()
{
 static int NoZ, ix, iy, ScaledMarkSize;
 static float x, y, z, GridZmin;
 static long i, SizeOfOutline;
 if( OutlineIsDrawable() ) // Draw the outline if it exists.
  {
	PolyBuffer( 0, 0, -1 ) ;
   if( BoldOutline) hOutlinePen = QGSelectPen( OutlinePen);
	else hOutlinePen = CreatePen( PS_SOLID, 0, OutlinePen);
   ApplyPen( hOutlinePen);

   SizeOfOutline = OutLine.Size();

   GridZmin = Zgrid.zmin();  // Set z to zmin if no Z coordinate read for outline.
   NoZ = FALSE;
   if( OutLine.zMin() == OutLine.zMax() ) NoZ = TRUE;
   for ( i = 0; i < SizeOfOutline; i++ )
   {
     x = OutLine.x(i);
     y = OutLine.y(i);
     z = GridZmin;
     if( !NoZ ) z = OutLine.z(i) + ScatterData.zAdjust(); // Normalize to data.;
     Zgrid.map( x, y, z);
     Rotate( y, x, z);

     if( Projection != 0.0 ) Project( x, y, z);
     ix = ScaleX( x );
     iy = ScaleY( y );
     if( OutLine.flags(i)&1 ) PolyBuffer( ix, iy, 0 );
     else PolyBuffer( ix, iy, 1 );
     if( OutLine.comment(i)[0] != NULL )
     {
       TextOut( hdc, ix+MarkFraction, iy+MarkFraction, OutLine.comment(i), strlen(OutLine.comment(i)) );
       ScaledMarkSize = CurrWin.xr/200;

       hOldPen = (HPEN)SelectObject( hdc, hBlackPen );
       MoveToEx( hdc, ix-ScaledMarkSize, iy, NULL); LineTo( hdc, ix+ScaledMarkSize, iy);
		 MoveToEx( hdc, ix, iy-ScaledMarkSize, NULL); LineTo( hdc, ix, iy+ScaledMarkSize);
       SelectObject( hdc, hOldPen ); 
     }
   } // End for ( i = ...
   PolyBuffer( 0, 0, -1 );
   SelectObject( hdc, hBlackPen);
	assert( DeleteObject( hOutlinePen ));

  } // end if( OutLine ... end drawing the outline.
}
//*********************************************************
//      P a i n t   3 d    S u r f a c e
//*********************************************************
void Paint3dSurface( HDC &hdc, const ContourOptionsType &Options  )
{
  // Must be called before this:
  //     Scale3dCube: sets up max's min's etc.
  //     CentreDisplay: set up rest of scaling. (done by quikgrid.cpp)

  static int xold, yold, Draw, xSize, ySize, ix, iy ;
  static long i, j, nSample, NumberOfPoints;
  static float x, y, z, xf, yf;

  //if( zRange == 0.0 ) return;
  if( (Zgrid.zmax()-Zgrid.zmin()) == 0.0 ) return;
  if( Zgrid.xsize() <= 2 ) return; // there is no grid.

  SpecialPenWasUsed = FALSE;
  QGSelectFont();
  hBoldPen = QGSelectPen( ContourPen );
  ApplyView();
  BlackAndWhite = Options.blackwhite;
  xSize = Zgrid.xsize();
  ySize = Zgrid.ysize();

  if( Projection != 0 )
    {
      float test = xRange*xRange + yRange*yRange;
      assert( test >= 0.0 );
      float distance = sqrt( test );
      float xViewPoint = xRange/2. + xMin;
      float yViewPoint = yRange/2. + yMin;
      float zViewPoint = zRange - zMin + distance*Projection;
      ProjectInitialize( xViewPoint, yViewPoint, zViewPoint);
    }
   PolyBuffer( 0, 0, -1 );   // Flush the polyline buffers. 
	// Do the X and Y axis now.
	if( Options.threedaxes )
      {
        ApplyPen( hAxisPen );
        if( !AxesOnTop )Draw3dXYAxes( hdc );
        if( !AxesOnTop && (fabs(Turn) > 90) ) Draw3dZAxis( hdc );
      }
   // Generate the Grid.
   PolyBuffer( 0, 0, -1);  // flush the line buffers.
	if( (Options.grid||Options.hiddengrid||ColouredGrid) && ZgridIsDisplayable())
   {
    ApplyPen( hGridPen );

    RotateGrid();  // This rotates the grid and generates the 2d projection
                   // of it in xyGrid.

    if( Options.hiddengrid||ColouredGrid )
      {
         Zgrid.zratio( 0.0 );    // Turn off zratio so that Hatch3d
         Hatch3D( xyGrid, hdc ); // can get at the true z values to
         ApplyzRatio();          // determine the grid square colour.
      }
    else
     {  // Paint the transparent grid.

      for( i=0; i< xSize; i++)
        { Draw = 0;
          for( j = 0; j< ySize; j++ ) Draw3dGridLine( i, j, Draw);
        }

      for( j=0; j< ySize; j++)
       { Draw = 0;
         for( i = 0; i< xSize; i++ ) Draw3dGridLine( i, j, Draw);
       }

     } // end if( Options... else...
    PolyBuffer( 0, 0, -1);

   }  // end if( Options.grid||Options.hiddengrid....

// Depending on viewing angle do the Z axis here.
	if( Options.threedaxes )
      {
        ApplyPen( hAxisPen );
        if( (fabs(Turn) <= 90) || AxesOnTop )Draw3dZAxis( hdc );
        if( AxesOnTop ) Draw3dXYAxes( hdc );
      }

// Generate the contour lines. 

  if( ( Options.contours && ZgridIsDisplayable()&&
       !Options.hiddengrid && !ColouredGrid ) || NeverHideContours ) Draw3dContours( hdc, Options );

  // Generate the data points and optional numbers.
  if( ScatterdataIsDrawable()&&Options.marks ) // do optional marks and numbers
 {
  NumberOfPoints = ScatterData.Size();

  PolyBuffer(0, 0, -1); 
  ApplyPen( hDataPointPen );

  nSample = XpandSample();
  for( i = 0; i<NumberOfPoints; i+=nSample )
   {
     x = xf = ScatterData.x(i);
     y = yf = ScatterData.y(i);
     z = ScatterData.z(i);
     Zgrid.map( x, y, z);
     Rotate( y, x, z);

     if( Projection != 0.0 ) Project( x, y, z);
     ix = ScaleX( x ) ;
     iy = ScaleY( y );
	  if( !DataPointsOnGridOnly || InTheGrid( xf, yf ) ) DrawMark( ix, iy, i );

     // This is to connect the points with lines.
     //    Change to do a move to an "ignored" point.
     if( Options.lines )
       {
			if( i == 0 ){ xold = ix; yold = iy; }
         else
			  {
             MoveToEx( hdc, xold, yold, NULL ); // Move to previous point.
             if( !(ScatterData.flags(i)&1) ) LineTo( hdc, ix, iy ) ;
	          xold = ix; yold = iy;
           }
       }  // end if (Options.lines
    }  // end for ( i =
 } // end if( Options.marks...

 TitleYposn = ScaleY(yMax);
 TextOut( hdc, TitleXposn, TitleYposn, szTitle, lstrlen( szTitle ) );

 DrawColourLegend( hdc );
 DrawVolumeDifference( hdc ); 

 Draw3dOutline();

 Zgrid.zratio( 0 );

 SelectObject( hdc, hBlackPen );
 if( Options.showcorners&&Options.grid ) Draw3dCorners( hdc );

 RemoveView();
 QGDeleteFont();
 assert( DeleteObject( hBoldPen ));
 if( SpecialPenWasUsed ) PaintContourReset();
}
//**********************************************************************
//            G E T X Y I N F O
//**********************************************************************
void GetxyInfo( long &ix, long &iy)
{
  ix = GridInfoIX;
  iy = GridInfoIY;
}
//**********************************************************************
//            G E T X Y D A T A
//**********************************************************************
void GetxyData( float &x, float &y, long &ix, long &iy)
{
  y = unScaleY( iy );
  x = unScaleX( ix );
}
//**********************************************************************
//           X Y I N F O 2 D
// Put up a window with info about grid location near ix,iy (2dview)
//**********************************************************************
void xyInfo2d( int ix, int iy, const ContourOptionsType &Options )
{
  static long i,
	           nSample,
              NumberOfPoints;

  static float xDistance,
               yDistance,
	            DistanceSquared,
	            test,
	            x, y;
  static GridViewType CurrentView;

  GridInfoIX = GridInfoIY = -1;
  if( !Visible( ix, iy ) ) return;

  SetWaitCursor();
  ApplyView();
  y = unScaleY( iy );
  x = unScaleX( ix );

  // Find closest x coordinate to ix - search grid coordinates if
  // displayed. 
 if (Options.grid||Options.hiddengrid||ColouredGrid)
    DistanceSquared = CloseTo2dGrid( x, y, GridInfoIX, GridInfoIY );
  CurrentView = Zgrid.view();
  if( GridInfoIX != -1 ) GridInfoIX = GridInfoIX*CurrentView.xIncrement;
  if( GridInfoIY != -1 ) GridInfoIY = GridInfoIY*CurrentView.yIncrement;
 
 // Now search scattered data points.
 NumberOfPoints = ScatterData.Size(); 
 if( (Options.marks)&&NumberOfPoints>2 )
 {
   if( GridInfoIY == -1 )
     { xDistance = x - ScatterData.x(0) ;
       yDistance = y - ScatterData.y(0) ;
       DistanceSquared = xDistance*xDistance + yDistance*yDistance;
       GridInfoIX = 0;
     }
   nSample = XpandSample();
   for( i = 0; i < NumberOfPoints; i+=nSample )
     {
       xDistance = x - ScatterData.x(i);
       yDistance = y - ScatterData.y(i);
       test = xDistance*xDistance + yDistance*yDistance;
       if( test < DistanceSquared )
	{ DistanceSquared = test; GridInfoIX = i; GridInfoIY = -1; }
     }
 }
 assert( GridInfoIX != -1 );   // Will fail if outline loaded and no data points.
 RemoveView();
 RestoreCursor(); 
}

//**********************************************************************
//           X Y I N F O 3 D
// Put up a window with info about grid location new ix,iy (3d view)
//**********************************************************************
void xyInfo3d( int ix, int iy, const ContourOptionsType &Options )
{
  static GridViewType CurrentView;
  static float xdist,
	ydist,
	TestSquared,
	DistanceSquared,
	x, y, z;

  static long i,
	      NumberOfPoints,
	      nSample;


 if( Options.hiddengrid ||ColouredGrid )
  {
   NotifyUser( IDS_NO3DINFORMATION );
   if( hGridLocationDlgBox != NULL )
     PostMessage( hGridLocationDlgBox, WM_COMMAND, IDCANCEL, NULL);
   return; 
  }
  SetWaitCursor();
  ApplyView();
  GridInfoIX = GridInfoIY = -1;
  DistanceSquared = CloseTo3dGrid( ix, iy, GridInfoIX, GridInfoIY);
  CurrentView = Zgrid.view();
  if( GridInfoIX != -1 ) GridInfoIX = GridInfoIX*CurrentView.xIncrement;
  if( GridInfoIY != -1 ) GridInfoIY = GridInfoIY*CurrentView.yIncrement;
  RemoveView();
   // Now search scattered data points.
  if( (Options.marks)&&(ScatterData.Size()>2) )
 {
   ApplyzRatio();
   NumberOfPoints = ScatterData.Size(); 

   nSample = XpandSample();
   for( i = 0; i < NumberOfPoints; i+=nSample )
     { x = ScatterData.x(i);
       y = ScatterData.y(i);
       z = ScatterData.z(i);
       Zgrid.map( x, y, z );
       Rotate( y, x, z );
       if( Projection != 0.0 ) Project (x, y, z);
       xdist = ix - ScaleX(x);
       ydist = iy - ScaleY(y); 
       TestSquared = xdist*xdist + ydist*ydist ;
       if( TestSquared < DistanceSquared )
         {DistanceSquared = TestSquared; GridInfoIX = i; GridInfoIY = -1 ;}
     }

   Zgrid.zratio(0);
   RestoreCursor();
 }
}

