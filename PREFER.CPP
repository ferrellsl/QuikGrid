//*****************************************************************
//               P r e f e r e n c e s   R o u t i n e s
//
// Stuff to save and load preferences.
//                         Copyright 1999-2009  by John Coulthard
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
// Feb. 13/99: Implementation.
// Oct. 12/99: Default Template installed.
// 2000-2009: Add preferences as required.
//*****************************************************************
#include <windows.h>
#include <stdio.h>
#include "rc.h"
#include "surfgrid.h"
#include "scatdata.h"
#include "quikgrid.h"
#include "paintcon.h"
#include "grid.h"
#include "utilitys.h"
#include "xpand.h"
#include "loaddata.h"

extern float ZoomRatio;
extern float PanRatio;
extern int UsePolyline;
extern int FlipXY;
extern int ReverseSignOfZ; 
extern int NewInputStyle;
extern int IgnoreComments;
extern int IgnoreErrorLines;
extern ContourOptionsType DrawingOptions;
extern UINT PaintState;
extern char SeparatorCharacter; 
extern SurfaceGrid Zgrid;
extern int OutputUndefinedValues;
extern double UndefinedValue;
extern int UseScatteredDataLayers;
extern int DXFFaceScaleZ;
extern int HighDXFOutputPrecision;
extern char RightMouseDialogTitle[200];
extern int  RightMouseDialogTerse;
extern int LabelNthLine;
extern int LabelNDigits;
extern int LabelDistance;
extern int ColourContours;
extern int ColouredGrid;
extern int BoldOutline;
extern int NicexyCoordinates;
extern int AxesOnTop;
extern int IncludeColourLegend;
extern int DataPointsOnGridOnly;
extern int SubmetrixSelectZ;
extern COLORREF ShadePen;
extern int BlendedColours;
extern int ReverseColourMap;
extern int DisplayVolumeDifference;
extern int RestoreScreenFromMemory;
extern int AlwaysDrawDirectlyToScreen;
extern int NeverHideContours;
extern int NeverLabelDXFContours;
int UseThreads;
extern int TextSize;
extern float ZmaxLabel, ZminLabel;
extern int VRMLtriangles;
extern float ArrowSize;

extern COLORREF GridPen,
                ArrowPen,
                ContourPen,
                BoldContourPen,
                DataPointPen,
                OutlinePen,
                BlackPen,
                BackgroundPen,
                TextPen,
                AxisPen;

static double xadjust, yadjust;
static char szTemp[100], *endptr;
static int UseTabSeparator;
extern int GridlineColouredGrid;
extern int SmallMark;
extern float DXFTextHeightRatio;
extern int InputFieldPosition[4] ;
extern int NOAAInputFieldPosition[4] ;

//**************************************************************************
static BOOL WriteProfileInt( char szSection[], char szKey[], int i)
{
  sprintf( szTemp, "%i", i );
  return WritePrivateProfileString( szSection, szKey, szTemp, INIfileName() );
}
//**************************************************************************
static BOOL WriteProfileDouble( char szSection[], char szKey[], double d )
{
  sprintf( szTemp, "%.10g", d );
  return WritePrivateProfileString( szSection, szKey, szTemp, INIfileName() );
}
//**************************************************************************
static int GetProfileInt( char szSection[], char szKey[], char szDefault[] )
{
  GetPrivateProfileString
	 ( szSection, szKey, szDefault, szTemp, sizeof( szTemp) , INIfileName() );
  return strtol( szTemp, &endptr, 10 );
}
//**************************************************************************
static double GetProfileDouble( char szSection[], char szKey[], char szDefault[] )
{
  GetPrivateProfileString
	 ( szSection, szKey, szDefault, szTemp, sizeof( szTemp) , INIfileName() );
  return strtod( szTemp, &endptr );
}
//**************************************************************************
static int GetMenuProfileInt( HMENU &hMenu, const UINT &Item,
										char szSection[], char szKey[], char szDefault[] )
{
  static UINT result;
  static UINT Checked;
  GetPrivateProfileString
	 ( szSection, szKey, szDefault, szTemp, sizeof( szTemp) , INIfileName() );
  result = strtol( szTemp, &endptr, 10 );
  Checked = MF_UNCHECKED;
  if( result) Checked = MF_CHECKED;
  CheckMenuItem( hMenu, Item, Checked);
  return result;
}
//**************************************************************************
//      L o a d   P r e f e r e n c e s
//**************************************************************************

void LoadPreferences( HMENU &hMenu)
{
//        Do the Zoom preferences.

  GetPrivateProfileString
	 ( "Zoom", "Zoom ratio", "1.5", szTemp, sizeof( szTemp) , INIfileName() );
	ZoomRatio = strtod( szTemp, &endptr );

  GetPrivateProfileString
	 ( "Zoom", "Pan ratio", ".1", szTemp, sizeof( szTemp), INIfileName() );
  PanRatio = strtod( szTemp, &endptr );

// Do View Data Point options.

  DrawingOptions.showmarks = GetProfileInt( "DataView", "Show marks", "1" );
  SmallMark = GetProfileInt( "DataView", "Use small marks", "0" );
  DrawingOptions.pointnumber = GetProfileInt( "DataView", "Show point number", "0" );
  DrawingOptions.numbers = GetProfileInt("DataView", "Show Z value", "0");
  DrawingOptions.xyplot = GetProfileInt( "DataView", "Show xyplot", "0"  );
  DrawingOptions.comments = GetProfileInt( "DataView", "Show comments","0" );
  DrawingOptions.lines = GetProfileInt( "DataView", "Show lines", "0" );
  DataPointsOnGridOnly = GetProfileInt( "DataView", "DataPointsOnGridOnly", "0" );

// Do View menu  and View... Edit view options preferences

  DrawingOptions.marks = GetMenuProfileInt( hMenu, IDM_DATAPLOT,"View", "Data points", "0") ;
  DrawingOptions.blackwhite = GetProfileInt( "View", "Blackwhite", "0");
  DrawingOptions.showcorners = GetProfileInt("View", "Corners","0" ) ;
  DrawingOptions.contours = GetMenuProfileInt( hMenu, IDM_CONTOUR,"View", "Contours", "1") ;
  DrawingOptions.labelcontours = GetMenuProfileInt( hMenu, IDM_CONTOURLABELS,"View", "Contour labels", "1") ;
  DrawingOptions.grid = GetMenuProfileInt( hMenu, IDM_GRIDPLOT,"View", "Grid", "0" ) ;
  DrawingOptions.hiddengrid = GetMenuProfileInt( hMenu, IDM_GRIDHIDE,"View", "Hiddengrid", "1" ) ;
  ColouredGrid = GetMenuProfileInt( hMenu, IDM_COLOUREDGRID,"View", "ColouredGrid", "0" ) ;
  GridlineColouredGrid = GetProfileInt("View", "GridlineColouredGrid", "0" ) ;
  BoldOutline  = GetProfileInt(  "View", "BoldOutline", "0" );
  AxesOnTop  = GetProfileInt( "View", "AxesOnTop", "0" );
  IncludeColourLegend  = GetProfileInt( "View", "IncludeColourLegend", "1" );
  DrawingOptions.threedaxes = GetProfileInt( "View", "3Daxes", "1") ;
  NeverHideContours = GetProfileInt( "View", "Never hide contours", "0");
  DisplayVolumeDifference = GetProfileInt( "View", "DisplayVolumeDifference", "0" ); 
  RestoreScreenFromMemory = GetProfileInt( "View", "RestoreScreenFromMemory", "0" );
  AlwaysDrawDirectlyToScreen = GetProfileInt( "View", "AlwaysDrawDirectlyToScreen", "0" );
  TextSize = GetProfileInt( "View", "Text size", "80");
  ZminLabel = GetProfileDouble( "View", "Zmin label", "0.0");
  ZmaxLabel = GetProfileDouble( "View", "Zmax label", "0.0");
  ArrowSize = GetProfileDouble( "View", "Arrow size", "2.0");

  sprintf( szTemp, "%i", IDM_2DSURFACE );
  PaintState = GetProfileInt( "View", "Viewtype", szTemp ) ;

  CheckMenuItem( hMenu, IDM_2DSURFACE, MF_UNCHECKED);
  CheckMenuItem( hMenu, IDM_3DSURFACE, MF_UNCHECKED);
  if( PaintState == IDM_2DSURFACE )
		 CheckMenuItem( hMenu, IDM_2DSURFACE, MF_CHECKED);
  else
    {
      CheckMenuItem( hMenu, IDM_3DSURFACE, MF_CHECKED);
      EnableMenuItem( hMenu, IDM_GENGRIDZOOM, MF_GRAYED);
    }

// Set Grid Generation options.

  XpandScanRatio( GetProfileInt( "Grid generation", "Scan Bandwidth", "16" ));
  XpandDensityRatio(GetProfileInt( "Grid generation", "Distance cutoff","150" ) );
  XpandEdgeFactor(GetProfileInt( "Grid generation", "Edge sensitivity","100")  );
  XpandSample( GetProfileInt( "Grid generation", "Sample rate", "1" ) );

// Set Input/Output preferences

  GridSizeRatio(GetProfileInt( "Input Output", "Automatic grid size ratio", "4"  ));
  GridMaxRatio(GetProfileInt( "Input Output", "Maximum automatic grid size", "200"  ));
  UsePolyline = GetProfileInt( "Input Output", "DXF Polylines", "0" );
  FlipXY = GetProfileInt( "Input Output", "Reverse x and y", "0"  );
  ReverseSignOfZ = GetProfileInt( "Input Output", "Reverse sign of z", "0" );
  IgnoreZzero(GetProfileInt( "Input Output", "Ignore DCA Z zero", "1" ) );
  IgnoreComments = GetProfileInt( "Input Output", "Ignore comments", "0" );
  IgnoreErrorLines = GetProfileInt( "Input Output", "Ignore error lines", "0" );
  NewInputStyle = GetProfileInt( "Input Output", "Use new style metric input", "1" );
  NicexyCoordinates = GetProfileInt( "Input Output", "Calculate nice xy coordinates", "1" );
  KeepEveryNth(GetProfileInt( "Input Output", "Read every nth point", "1" ) );
  UseTabSeparator = GetProfileInt( "Input Output", "Use tab separator", "0" );
  SeparatorCharacter = ' ';
  if( UseTabSeparator ) SeparatorCharacter = '\t';
  OutputUndefinedValues = GetProfileInt( "Input Output", "Output undefined values", "0" );
  UseScatteredDataLayers = GetProfileInt( "Input Output", "Output scattered data in DXF layers", "0" );
  UndefinedValue = GetProfileDouble( "Input Output", "Undefined value", "-99999." );
  DXFFaceScaleZ = GetProfileInt( "Input Output", "DXF3DFACE Scale Z", "1" );
  VRMLtriangles = GetProfileInt( "Input Output", "VRML triangles", "1" );
  HighDXFOutputPrecision = GetProfileInt( "Input Output", "High DXF output precision", "1" );
  SubmetrixSelectZ = GetProfileInt( "Input Output", "SubmetrixZ", "1" );
  DXFTextHeightRatio = GetProfileDouble( "Input Output", "DXF Text Height Ratio", ".02");
  NeverLabelDXFContours = GetProfileInt(  "Input Output", "Never label DXF contours", "0" );
  UseThreads = GetProfileInt(  "Input Output", "Use threads", "1" );
  InputFieldPosition[0] = GetProfileInt( "Input Output", "X Input Position", "0" );
  InputFieldPosition[1] = GetProfileInt( "Input Output", "Y Input Position", "1" );
  InputFieldPosition[2] = GetProfileInt( "Input Output", "Z Input Position", "2" );
  InputFieldPosition[3] = GetProfileInt( "Input Output", "C Input Position", "3" );
  NOAAInputFieldPosition[0] = GetProfileInt( "Input Output", "NOAA X Input Position", "0" );
  NOAAInputFieldPosition[1] = GetProfileInt( "Input Output", "NOAA Y Input Position", "3" );
  NOAAInputFieldPosition[2] = GetProfileInt( "Input Output", "NOAA Z Input Position", "6" );
  NOAAInputFieldPosition[3] = GetProfileInt( "Input Output", "NOAA C Input Position", "7" );

// Set Surface z ratio

  PaintSurfaceRatio( GetProfileInt( "Surface", "Z ratio", "60" ));

// Pick up the template
	int nx = GetProfileInt( "Template", "Xsize", "0" );
	if( nx > 0 )
	{
	  double Xstart = GetProfileDouble( "Template", "Xstart", "0" );
	  double Xincrement = GetProfileDouble( "Template", "Xincrement", "0" );
	  int ny = GetProfileInt( "Template", "Ysize", "0" );
	  double Ystart = GetProfileDouble( "Template", "Ystart", "0" );
	  double Yincrement = GetProfileDouble( "Template", "Yincrement", "0" );
	  int LatLon = GetProfileInt( "Template", "LatLonData", "0" );
	  SetTemplate( nx, Xstart, Xincrement, ny, Ystart, Yincrement, LatLon );
     CheckMenuItem( hMenu, IDM_GRIDLOCK, MF_CHECKED);
	}

// Pick up the 3d rotation options.
	Projection = GetProfileDouble( "3D Options", "Projection", "5" );
	Aspect = GetProfileDouble( "3D Options", "Aspect", "0" );
	Tilt = GetProfileDouble( "3D Options", "Tilt", "35" );
	Turn = GetProfileDouble( "3D Options", "Turn", "35" );

// Get the color options
   GridPen = GetProfileInt( "Color", "GridPen", "16711680");
   ArrowPen = GetProfileInt( "Color", "ArrowPen", "16711680");
   ContourPen = GetProfileInt( "Color", "ContourPen", "0");
   BoldContourPen = ContourPen;
   DataPointPen = GetProfileInt( "Color", "DataPointPen", "255");
   OutlinePen = GetProfileInt( "Color", "OutlinePen", "255");
   BackgroundPen = GetProfileInt( "Color", "BackgroundPen", "16777215");
   TextPen = GetProfileInt( "Color", "TextPen", "0");
   AxisPen = GetProfileInt( "Color", "AxisPen", "0");

// Get colour mapping options.

   BlendedColours = GetProfileInt( "Colour mapping", "BlendedColours", "1" );
   ShadePen = GetProfileInt( "Colour mapping", "Shaded colour", "0");
   ReverseColourMap = GetProfileInt( "Colour mapping", "Reverse colour map", "0");

   // Right mouse button options.
   RightMouseDialogTerse = GetProfileInt( "Right Button", "Terse", "0");
   GetPrivateProfileString
   ( "Right Button", "Title", "Information for", 
       RightMouseDialogTitle, sizeof(RightMouseDialogTitle), INIfileName() );

   // Contour options
   
  LabelNDigits = GetProfileInt( "Contour", "LabelPrecision", "5");
  LabelDistance= GetProfileInt( "Contour", "LabelSeparation", "1200");
  ColourContours = GetProfileInt( "Contour", "AutomaticColours", "0"); 
  }
//**************************************************************************
//        S a v e    Z o o m    O p t i o n s
//**************************************************************************

void SaveZoomOptions()
{
  sprintf( szTemp, "%g", ZoomRatio );
  WritePrivateProfileString
			 ( "Zoom", "Zoom ratio", szTemp, INIfileName() );
  sprintf( szTemp, "%g", PanRatio );
  WritePrivateProfileString
			 ( "Zoom", "Pan ratio", szTemp, INIfileName() );
}
//**************************************************************************
//    S a v e   V i e w   D a t a   O p t i o n s
//**************************************************************************

void SaveViewDataOptions()
{
 WriteProfileInt( "DataView", "Show marks", DrawingOptions.showmarks) ;
 WriteProfileInt( "DataView", "Use small marks", SmallMark ); 
 WriteProfileInt( "DataView", "Show point number", DrawingOptions.pointnumber);
 WriteProfileInt( "DataView", "Show Z value", DrawingOptions.numbers) ;
 WriteProfileInt( "DataView", "Show XY", DrawingOptions.xyplot ) ;
 WriteProfileInt( "DataView", "Show comments", DrawingOptions.comments) ;
 WriteProfileInt( "DataView", "Show lines", DrawingOptions.lines ) ;
 WriteProfileInt( "DataView", "DataPointsOnGridOnly", DataPointsOnGridOnly ) ;
}
//**************************************************************************
//    S a v e   V i e w   M e n u   O p t i o n s
//**************************************************************************

void SaveViewMenuOptions()
{
  WriteProfileInt( "View", "Data points", DrawingOptions.marks) ;
  WriteProfileInt( "View", "Blackwhite", DrawingOptions.blackwhite);
  WriteProfileInt( "View", "Corners", DrawingOptions.showcorners) ;
  WriteProfileInt( "View", "Contours", DrawingOptions.contours) ;
  WriteProfileInt( "View", "Contour labels", DrawingOptions.labelcontours) ;
  WriteProfileInt( "View", "Grid", DrawingOptions.grid) ;
  WriteProfileInt( "View", "Hiddengrid", DrawingOptions.hiddengrid) ;
  WriteProfileInt( "View", "Colouredgrid", ColouredGrid) ;
  WriteProfileInt( "View", "GridlineColouredgrid", GridlineColouredGrid) ;
  WriteProfileInt( "View", "BoldOutline", BoldOutline) ;
  WriteProfileInt( "View", "3Daxes", DrawingOptions.threedaxes) ;
  WriteProfileInt( "View", "AxesOnTop", AxesOnTop) ;
  WriteProfileInt( "View", "IncludeColourLegend", IncludeColourLegend) ;
  WriteProfileInt( "View", "Viewtype",  PaintState) ;
  WriteProfileInt( "View", "DisplayVolumeDifference", DisplayVolumeDifference );
  WriteProfileInt( "View", "RestoreScreenFromMemory", RestoreScreenFromMemory );
  WriteProfileInt( "View", "AlwaysDrawDirectlyToScreen", AlwaysDrawDirectlyToScreen );
  WriteProfileInt( "View", "Never hide contours", NeverHideContours );
  WriteProfileInt( "View", "Text size", TextSize );
  WriteProfileDouble( "view", "Arrow size", ArrowSize); 
  WriteProfileDouble( "View", "Zmax label", ZmaxLabel);
  WriteProfileDouble( "View", "Zmin label", ZminLabel);
}
//**************************************************************************
//    S a v e   T e c h n i c a l   O p t i o n s
//**************************************************************************

void SaveTechnicalOptions()
{
  WriteProfileInt( "Grid generation", "Scan Bandwidth", XpandScanRatio() );
  WriteProfileInt( "Grid generation", "Grid size ratio", GridSizeRatio() );
  WriteProfileInt( "Grid generation", "Distance cutoff", XpandDensityRatio() );
  WriteProfileInt( "Grid generation", "Edge sensitivity", XpandEdgeFactor() );
  WriteProfileInt( "Grid generation", "Sample rate" , XpandSample() );
}
//**************************************************************************
//    S a v e   I n p u t   O p t i o n s
//**************************************************************************

void SaveInputOptions()
{
  WriteProfileInt( "Input Output", "Automatic grid size ratio", GridSizeRatio() );
  WriteProfileInt( "Input Output", "Maximum automatic grid size", GridMaxRatio() );
  WriteProfileInt( "Input Output", "Reverse x and y", FlipXY );
  WriteProfileInt( "Input Output", "Ignore comments", IgnoreComments );
  WriteProfileInt( "Input Output", "Ignore error lines", IgnoreErrorLines );
  WriteProfileInt( "Input Output", "Reverse sign of z", ReverseSignOfZ );
  WriteProfileInt( "Input Output", "Ignore DCA Z zero", IgnoreZzero() );
  WriteProfileInt( "Input Output", "Use new style metric input", NewInputStyle );
  WriteProfileInt( "Input Output", "Read every nth point", KeepEveryNth() );
  WriteProfileInt( "Input Output", "Calculate nice xy coordinates", NicexyCoordinates);
  WriteProfileInt( "Input Output", "SubmetrixZ", SubmetrixSelectZ );
  WriteProfileInt( "Input Output", "Use threads", UseThreads );
  WriteProfileInt( "Input Output", "X Input Position", InputFieldPosition[0] );
  WriteProfileInt( "Input Output", "Y Input Position", InputFieldPosition[1] );
  WriteProfileInt( "Input Output", "Z Input Position", InputFieldPosition[2] );
  WriteProfileInt( "Input Output", "C Input Position", InputFieldPosition[3] );
  WriteProfileInt( "Input Output", "NOAA X Input Position", NOAAInputFieldPosition[0] );
  WriteProfileInt( "Input Output", "NOAA Y Input Position", NOAAInputFieldPosition[1] );
  WriteProfileInt( "Input Output", "NOAA Z Input Position", NOAAInputFieldPosition[2] );
  WriteProfileInt( "Input Output", "NOAA C Input Position", NOAAInputFieldPosition[3] );
}
//**************************************************************************
//    S a v e   O u t p u t  O p t i o n s
//**************************************************************************

void SaveOutputOptions()
{
  WriteProfileInt( "Input Output", "DXF Polylines", UsePolyline );
  UseTabSeparator = FALSE;
  if( SeparatorCharacter == '\t' ) UseTabSeparator = TRUE;
  WriteProfileInt( "Input Output", "Use tab separator", UseTabSeparator );
  WriteProfileInt( "Input Output", "Output undefined values", OutputUndefinedValues );
  WriteProfileInt( "Input Output", "Output scattered data in DXF layers", UseScatteredDataLayers);
  WriteProfileDouble( "Input Output", "Undefined value", UndefinedValue);
  WriteProfileInt( "Input Output", "DXF3DFACE Scale Z", DXFFaceScaleZ );
  WriteProfileInt( "Input Output", "VRML triangles", VRMLtriangles );
  WriteProfileInt( "Input Output", "High DXF output precision", HighDXFOutputPrecision );
  WriteProfileDouble( "Input Output", "DXF Text Height Ratio", DXFTextHeightRatio );
  WriteProfileInt( "Input Output", "Never label DXF contours", NeverLabelDXFContours );
}
//**************************************************************************
//    S a v e   Z r a t i o  O p t i o n s
//**************************************************************************

void SaveZratioOptions()
{
 WriteProfileInt( "Surface", "Z ratio", PaintSurfaceRatio() );
}

//**************************************************************************
//    S a v e    T e m p l a t e
//**************************************************************************

void SaveTemplate()
{
  LoadNormalization( xadjust, yadjust);
  WriteProfileInt( "Template", "Xsize", Zgrid.xsize());
  WriteProfileDouble( "Template", "Xstart", Zgrid.x(0)+xadjust);
  WriteProfileDouble( "Template", "Xincrement", Zgrid.x(1)-Zgrid.x(0));
  WriteProfileInt( "Template", "Ysize", Zgrid.ysize());
  WriteProfileDouble( "Template", "Ystart", Zgrid.y(0)+yadjust);
  WriteProfileDouble( "Template", "Yincrement", Zgrid.y(1)-Zgrid.y(0));
  WriteProfileInt( "Template", "LatLonData", LatLonData);
}
//**************************************************************************
//    C l e a r   T e m p l a t e
//**************************************************************************

void ClearTemplate()
{
  WriteProfileInt( "Template", "Xsize", 0);
}
//**************************************************************************
//    S a v e   3 d   O p t i o n s
//**************************************************************************

void Save3dOptions()
{
  WriteProfileDouble( "3D Options", "Turn", Turn );
  WriteProfileDouble( "3D Options", "Tilt", Tilt );
  WriteProfileDouble( "3D Options", "Aspect", Aspect );
  WriteProfileDouble( "3D Options", "Projection", Projection );
}

//**************************************************************************
//    S a v e   C o l o r   O p t i o n s
//**************************************************************************

void SaveColorOptions()
{
  WriteProfileInt( "Color", "GridPen", GridPen);
  WriteProfileInt( "Color", "ArrowPen", ArrowPen);
  WriteProfileInt( "Color", "ContourPen", ContourPen);
  WriteProfileInt( "Color", "DataPointPen", DataPointPen);
  WriteProfileInt( "Color", "OutlinePen", OutlinePen);
  WriteProfileInt( "Color", "BackgroundPen", BackgroundPen);
  WriteProfileInt( "Color", "TextPen", TextPen);
  WriteProfileInt( "Color", "AxisPen", AxisPen);

}

//**************************************************************************
//    S a v e  C o l o r   M a p p i n g  O p t i o n s
//**************************************************************************
void SaveColourMappingOptions()
{
  WriteProfileInt( "Colour mapping", "BlendedColours", BlendedColours);
  WriteProfileInt( "Colour mapping", "Shaded colour", ShadePen);
  WriteProfileInt( "Colour mapping", "Reverse colour map", ReverseColourMap);
}

//**************************************************************************
//    S a v e   R i g h t  B u t t o n  O p t i o n s
//**************************************************************************
void SaveRightButtonOptions()
{
 WriteProfileInt( "Right Button", "Terse", RightMouseDialogTerse);
 WritePrivateProfileString
			 ( "Right Button", "Title", RightMouseDialogTitle, INIfileName() );
}

//**************************************************************************
//    S a v e   C o n t o u r  O p t i o n s
//**************************************************************************
void SaveContourOptions()
{
  WriteProfileInt( "Contour", "LabelPrecision", LabelNDigits);
  WriteProfileInt( "Contour", "LabelSeparation", LabelDistance);
  WriteProfileInt( "Contour", "AutomaticColours", ColourContours);
}
