//**********************************************************************
//                  V R M L
//  Save grid in WRML (World Reality Meta Language)
//                Copyright (c) 2001-2003 by John Coulthard
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
// April 11/01: Start by cloning DXFFACE code.
// April 22/01: Reflect Colour grid capability.
// May 8/01: Playing around with clockwise vs counterclockwise vertices.
// May 15/01: Try shape type = UNKNOWN_SHAPE_TYPE
// Nov. 13/03: Force all Material out for each triangle to make it
//             work with TrueSpace (Increases file size a lot!).
//***********************************************************************

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
#include "condraw.h" 
using namespace std;
extern int DXFFaceScaleZ;
int VRMLtriangles = TRUE;
int VRML2format = FALSE;

static ofstream WRLoutFile(" ", ios::out );
static double dxnormalize, dynormalize;
static float zadjust, AverageZ;
static int WRLOutputFailed = FALSE;
static int nx, ny;
static OPENFILENAME ofn;
static float zRatio;
static float zRange, zMin, xRange, zScale;
static COLORREF Colour;
extern COLORREF GridPen,
                AxisPen,
                OutlinePen;
extern float ZmaxLabel, ZminLabel; 
extern int ColouredGrid;
extern SurfaceGrid Cgrid;
extern ContourOptionsType DrawingOptions;
extern ScatData OutLine;
static int ci;

//******************************************************************
//         A b o r t   W r i t e  WRL  F i l e
//******************************************************************
static void AbortWriteWRLFile( char FileName[] )
{
  ostrstream Buf;

  Buf << "Error writing VRML output to the file "
      << FileName
      << ". Output terminated. " << ends;

  char *szBuf = Buf.str();
  NotifyUser( szBuf );
  delete szBuf;

  RestoreCursor();
}
//**************************************************************
//      W r i t e   G r i d   S q u a r e   C o l o u r
//**************************************************************
static void WriteGridSquareColour( int i, int j )
{
  // i,j define the corner of the grid square to be output.
  // Draw two triangles per qrid square. clockwise.
  if( Zgrid.z(i,j) < 0.0 ) return;     // Don't draw triangle
  if( Zgrid.z(i+1,j+1) < 0.0 ) return; // if a node is undefined.

  if( VRMLtriangles ) // Output triangles.
  {
    if(Zgrid.z(i,j+1) >= 0.0)
    {
      AverageZ = (Zgrid.z(i,j)+Zgrid.z(i+1,j+1)+Zgrid.z(i,j+1))/3.;
      // if Cgrid exists use colour values from it instead.
      if( Cgrid.xsize() > 2 )
      {
        if( Cgrid.z(i,j) < 0.0 ) return;  // Unless undefined.
        if( Cgrid.z(i+1,j+1) < 0.0 ) return;
        if( Cgrid.z(i,j+1) < 0.0 ) return;
        AverageZ = (Cgrid.z(i,j)+Cgrid.z(i+1,j+1)+Cgrid.z(i,j+1))/3.;
      }
      Colour = ChooseZColour( AverageZ );
      WRLoutFile << "    "
                 << ((float)GetRValue(Colour)/255.) << " "
                 << ((float)GetGValue(Colour)/255.) << " "
                 << ((float)GetBValue(Colour)/255.) << ", \n";
      ci++;
    } // end if( zgrid.z...
    if(Zgrid.z(i+1,j) >= 0.0)
    {
      AverageZ = (Zgrid.z(i,j)+Zgrid.z(i+1,j+1)+Zgrid.z(i+1,j))/3.;
      if( Cgrid.xsize() > 2 )
      {
        if( Cgrid.z(i,j) < 0.0 ) return;
        if( Cgrid.z(i+1,j+1) < 0.0 ) return;
        if( Cgrid.z(i+1,j) < 0.0 ) return;
        AverageZ = (Cgrid.z(i,j)+Cgrid.z(i+1,j+1)+Cgrid.z(i+1,j))/3.;
      }
      Colour = ChooseZColour( AverageZ );
      WRLoutFile << "    "
                 << ((float)GetRValue(Colour)/255.) << " "
                 << ((float)GetGValue(Colour)/255.) << " "
                 << ((float)GetBValue(Colour)/255.) << ", \n";
      ci++;
    }  // end if( zgrid.z
  } // end if( VRMLtriangles )
  else // output squares
  {
    if(Zgrid.z(i,j+1) < 0.0) return;
    if(Zgrid.z(i+1,j) < 0.0) return;
    AverageZ = (Zgrid.z(i,j)+Zgrid.z(i+1,j+1)+Zgrid.z(i,j+1)+Zgrid.z(i+1,j))/4.;
    if( Cgrid.xsize() > 2 ) // if colour grid exists take colour from it.
    {
      if( Cgrid.z(i,j) < 0.0 ) return;  // Unless undefined.
      if( Cgrid.z(i+1,j+1) < 0.0 ) return;
      if( Cgrid.z(i,j+1) < 0.0 ) return;
      if( Cgrid.z(i+1,j) < 0.0 ) return;
      AverageZ = (Cgrid.z(i,j)+Cgrid.z(i+1,j+1)+Cgrid.z(i+1,j)+Cgrid.z(i+1,j))/4.;
    } // end if Cgrid.xsize
    Colour = ChooseZColour( AverageZ );
    WRLoutFile << "    "
               << ((float)GetRValue(Colour)/255.) << " "
               << ((float)GetGValue(Colour)/255.) << " "
               << ((float)GetBValue(Colour)/255.) << ", \n";
    ci++;
  } // end else - output squares.
}
//**************************************************************
//      W r i t e   G r i d   S q u a r e   C o o r d i n a t e s
//**************************************************************
static void WriteGridSquareCoordinates(int i, int j)
{
  float z = Zgrid.z(i, j);
  if (z < 0) z = 0; // Set undefined nodes to zero (they will be ignored anyhow).
  else if (DXFFaceScaleZ && (zRatio != 0.0))  // Scale z axis if requested.
      z = (z - zMin) * zScale;
  float x = Zgrid.x(i) + dxnormalize;
  float y = Zgrid.y(j) + dynormalize;
  WRLoutFile << "    " << x << " " << y << " " << z << ",\n";
}
//**************************************************************
//      W r i t e   G r i d   S q u a r e   C o o r d i n a t e   I n d i c e s
//**************************************************************
static void WriteGridSquareCoordinateIndices(int i, int j)
{
  // Draw two triangles per qrid square.. clockwise.
  if( Zgrid.z(i,j) < 0.0 ) return;     // Don't draw triangle
  if( Zgrid.z(i+1,j+1) < 0.0 ) return; // if a node is undefined.

  int ij = i*ny+j;            // Calculate the offsets for the 4 vertices
  int i1j = (i+1)*ny+j;       // of the grid square oriented at "ij".
  int ij1 =  i*ny+j+1;        // i1j == vertice at "i+1", "j" etc.
  int i1j1 = (i+1)*ny+j+1;

  if (!ColouredGrid || VRMLtriangles) {
    if (Zgrid.z(i,j+1) >= 0.0)
      WRLoutFile << "    " << ij << ", " << ij1 << ", " << i1j1 << ", -1,\n";  // Clockwise.

    if (Zgrid.z(i+1,j) >= 0.0)
      WRLoutFile << "    " << ij << ", " << i1j1 << ", " << i1j << ", -1,\n";  // Clockwise
  }
  else {
    if(Zgrid.z(i,j+1) < 0.0) return;
    if(Zgrid.z(i+1,j) < 0.0) return;
    WRLoutFile << "    " << ij << ", " << ij1 << ", " << i1j1 << ", " << i1j << ", -1,\n"; // clockwise
  }
}
//**************************************************************
//      G e t   V R M L   f i l e N a m e
//**************************************************************
int GetVRMLFileName( HWND &hwnd, char szFile[] )
{
  szFile[0] = '\0';

  memset( &ofn, 0, sizeof(OPENFILENAME) ); // zero structure members

  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = hwnd;  // =NULL if no owner required.
  ofn.lpstrFilter = "VRML files\0*.wrl\0All files\0*.*\0\0";
  ofn.nFilterIndex = 1;
  ofn.lpstrFile = szFile;
  ofn.nMaxFile = 255;        //sizeof(szFile);
  ofn.Flags = OFN_PATHMUSTEXIST|OFN_OVERWRITEPROMPT;
  ofn.lpstrTitle = "Generate VRML output to file" ;
  ofn.lpstrDefExt = "wrl";
  if( !GetSaveFileName( &ofn ) ) return 0;
  return 1;
}
//**************************************************************
//      Output   V R M L 1   F o r m a t
//**************************************************************
static int OutputVRML1Format()
{
  WRLoutFile << "#VRML V1.0 ascii\n"   // Output Header information.
             << "Separator {\n"
             << " DEF SceneInfo Info { string \"Generated by"
             << " QuikGrid.\" } # end SceneInfo\n"
             << " ShapeHints {\n"
             << "  vertexOrdering CLOCKWISE\n"
             << "  shapeType UNKNOWN_SHAPE_TYPE\n"
             << "  faceType CONVEX\n"
             << "  creaseAngle 0.0\n"
             << " } # end ShapeHints\n";

  WRLoutFile << " DEF X1_X2_GRID Separator {\n"
             << "  Material {\n"
             << "   diffuseColor ";

  nx = Zgrid.xsize();
  ny = Zgrid.ysize();

  zRatio = (float)PaintSurfaceRatio()*.01;
  xRange = Zgrid.x(nx-1) - Zgrid.x(0);
  zMin   = Zgrid.zmin();
  zRange = Zgrid.zmax() - Zgrid.zmin();
  zScale = zRatio*(xRange/zRange);

  float xmax = Zgrid.xmax() + dxnormalize;          // For axes on the generated grid.
  float ymax = Zgrid.ymax() + dynormalize;
  float xmin = Zgrid.xmin() + dxnormalize;
  float ymin = Zgrid.ymin() + dynormalize;

  // Z axes get special treatment - if overridden or scaled.

  float zmax = Zgrid.zmax();
  float zmin = Zgrid.zmin();

  // if the axes are to be plotted at a different z minimum.
  if ((ZmaxLabel != ZminLabel) &&
      (ZmaxLabel > zmin - zadjust) &&
      (ZminLabel < zmax - zadjust))
  {
      zmin = ZminLabel + zadjust;
      zmax = ZmaxLabel + zadjust;
  }

  if (DXFFaceScaleZ && (zRatio != 0.0))  // Scale z axis if requested.
  {
      zmin = (zmin - zMin) * zScale;
      zmax = (zmax - zMin) * zScale;
  }

  // end special treatment for z axis.

  if (ColouredGrid)
  {
    WRLoutFile << "[\n";
    // Output a Coloured Grid - each grid triangle is it's own colour
    for ( int i = 0; i<(nx-1); i++ )
    { for ( int j = 0; j<(ny-1); j++ )
      {
        WriteGridSquareColour(i, j);
        if (WRLoutFile.fail()) return 0;
      }
    } // End nested for's.
    WRLoutFile << "   ]\n";
  } // end if( ColouredGrid)
  else // Each grid triangle is the same colour - more compact output.
  {
    WRLoutFile << ((float)GetRValue(GridPen)/255.) << " "
               << ((float)GetGValue(GridPen)/255.) << " "
               << ((float)GetBValue(GridPen)/255.) << endl;
  } // end else - output non coloured grid.
  WRLoutFile << "   ambientColor .2 .2 .2\n"
             << "   specularColor 0 0 0\n"
             << "   emissiveColor 0 0 0\n"
             << "   shininess .2\n"
             << "   transparency 0\n"
             << "  } # end Material\n"
             << "  MaterialBinding { value PER_FACE }\n";

  // Output the grid coordinates.
  WRLoutFile << "  Coordinate3 { point [\n"
             << "   # Grid coordinates follow\n";
  for ( int i = 0; i<nx; i++ )
  { for ( int j = 0; j<ny; j++ )
    {
     WriteGridSquareCoordinates(i, j);
     if (WRLoutFile.fail()) return 0;
    }
  }
  WRLoutFile << "   ] # end point\n"
             << "  } # end Coordinate 3\n";

  WRLoutFile << " IndexedFaceSet { coordIndex [\n";

  for ( int i = 0; i<(nx-1); i++ )
  { for ( int j = 0; j<(ny-1); j++ )
    {
      WriteGridSquareCoordinateIndices(i, j);
      if (WRLoutFile.fail()) return 0;
    }
  }  // end nested for's

  WRLoutFile << "   ] # end coordIndex\n";

  if (ColouredGrid)
  {
    WRLoutFile << "   materialIndex [\n";
    WRLoutFile << "    ";
    for ( int i = 0; i<ci; i++ )
    {
      WRLoutFile << i << ", ";
      if (i && (i % 20) == 0) WRLoutFile << "\n    ";
    }
    WRLoutFile << "\n";
    if (WRLoutFile.fail()) return 0;

    WRLoutFile << "   ] # end materialIndex\n";
  }
  WRLoutFile << "  } # end IndexedFaceSet\n"
             << " } # end GRID Separator\n";

  // Grid is done - output the 3d Axes (three simple lines).
  if (DrawingOptions.threedaxes)
  {
    int iAxesOrigin = 0;
    int iAxesZmax = 1;
    int iAxesXmax = 2;
    int iAxesYmax = 3;
    WRLoutFile << " DEF AXES Separator {\n"
               << "  Material {\n"
               << "   diffuseColor "
               << ((float)GetRValue(AxisPen)/255.) << " "
               << ((float)GetGValue(AxisPen)/255.) << " "
               << ((float)GetBValue(AxisPen)/255.) << endl
               << "   ambientColor .2 .2 .2\n"
               << "   specularColor 0 0 0\n"
               << "   emissiveColor 0 0 0\n"
               << "   shininess .2\n"
               << "   transparency 0\n"
               << "  } # end Material\n"
               << "  Coordinate3 { point [\n"
               << "   # Axes coordinates follow\n"
               << "   " << xmin << " " << ymin << " " << zmin << ",\n"
               << "   " << xmin << " " << ymin << " " << zmax << ",\n"
               << "   " << xmax << " " << ymin << " " << zmin << ",\n"
               << "   " << xmin << " " << ymax << " " << zmin << ",\n"
               << "   ] # end point\n"
               << "  } # end Coordinate 3\n"
               << "  IndexedLineSet { coordIndex [\n"
               << "   " << iAxesOrigin << ", " << iAxesZmax << ", -1,\n"
               << "   " << iAxesOrigin << ", " << iAxesXmax << ", -1,\n"
               << "   " << iAxesOrigin << ", " << iAxesYmax << ", -1 ] }\n"
               << " } # end AXES Separator\n";
  }

  // Now output the outline.
  int SizeOfOutline = OutLine.Size();
  if( SizeOfOutline > 1 )
  {
    WRLoutFile << " DEF OUTLINE Separator {\n"
               << "  Material {\n"
               << "   diffuseColor "
               << ((float)GetRValue(OutlinePen)/255.) << " "
               << ((float)GetGValue(OutlinePen)/255.) << " "
               << ((float)GetBValue(OutlinePen)/255.) << endl
               << "   ambientColor .2 .2 .2\n"
               << "   specularColor 0 0 0\n"
               << "   emissiveColor 0 0 0\n"
               << "   shininess .2\n"
               << "   transparency 0\n"
               << "  } # end Material\n";

    WRLoutFile << "  Coordinate3 { point [\n"
               << "   # Outline coordinates follow\n";

    // Output the Outline Coordinates.

    // Set z to zmin if no Z coordinate read for outline.
    bool NoZ = false;
    if (OutLine.zMin() == OutLine.zMax()) NoZ = true;
    for ( int i = 0; i < SizeOfOutline; i++ )
    {
      float x = OutLine.x(i) + dxnormalize;
      float y = OutLine.y(i) + dynormalize;
      float z = zmin + zadjust;
      if (!NoZ) z = OutLine.z(i) + zadjust;
      if (DXFFaceScaleZ && (zRatio != 0.0))  // Scale z axis if requested.
        z = (z - zMin) * zScale;

      WRLoutFile << "   " << x << " " << y << " " << z << ",\n";
    } // End for ( i =

    WRLoutFile << "  ] # end point\n"
               << " } # end Coordinate 3\n";

    WRLoutFile << " IndexedLineSet { coordIndex [\n";
    for ( int i = 0; i < SizeOfOutline; i++ )
    {
      if( OutLine.flags(i) & 1 ) WRLoutFile << " -1, \n"; // Move here?
      WRLoutFile << i << ", \n";
    }
    WRLoutFile << " -1 ] }\n"
               << " } # end OUTLINE Separator\n";
  } // End if( OutlineSize > 1 )

  WRLoutFile << "} # end VRML Separator\n";
  return 1;
}
//**************************************************************
//      Output   V R M L 2   F o r m a t
//**************************************************************
static int OutputVRML2Format()
{
  WRLoutFile << "#VRML V2.0 utf8\n"   // Output Header information.
             << "Group {\n"
             << " children [\n"
             << "  DEF SceneInfo WorldInfo { title \"Generated by"
             << " QuikGrid.\" }, # end SceneInfo\n";

  WRLoutFile << "  DEF X1_X2_GRID Group {\n"
             << "   children\n"
             << "    DEF _shape Shape {\n"
             << "     appearance Appearance {\n"
             << "      material Material {\n"
             << "       diffuseColor "
             << ((float)GetRValue(GridPen) / 255.) << " "
             << ((float)GetGValue(GridPen) / 255.) << " "
             << ((float)GetBValue(GridPen) / 255.) << "\n"
             << "      }\n"
             << "     }\n"
             << "     geometry IndexedFaceSet {\n"
             << "      coord Coordinate {\n"
             << "       point [\n"
             << "        # Grid coordinates follow\n";

  nx = Zgrid.xsize();
  ny = Zgrid.ysize();

  zRatio = (float)PaintSurfaceRatio()*.01;
  xRange = Zgrid.x(nx-1) - Zgrid.x(0);
  zMin   = Zgrid.zmin();
  zRange = Zgrid.zmax() - Zgrid.zmin();
  zScale = zRatio*(xRange/zRange);

  float xmax = Zgrid.xmax() + dxnormalize;          // For axes on the generated grid.
  float ymax = Zgrid.ymax() + dynormalize;
  float xmin = Zgrid.xmin() + dxnormalize;
  float ymin = Zgrid.ymin() + dynormalize;

  // Z axes get special treatment - if overridden or scaled.

  float zmax = Zgrid.zmax();
  float zmin = Zgrid.zmin();

  // if the axes are to be plotted at a different z minimum.
  if ((ZmaxLabel != ZminLabel) &&
      (ZmaxLabel > zmin - zadjust) &&
      (ZminLabel < zmax - zadjust))
  {
      zmin = ZminLabel + zadjust;
      zmax = ZmaxLabel + zadjust;
  }

  if (DXFFaceScaleZ && (zRatio != 0.0))  // Scale z axis if requested.
  {
      zmin = (zmin - zMin) * zScale;
      zmax = (zmax - zMin) * zScale;
  }

  // end special treatment for z axis.

  // Output the grid coordinates.
  for ( int i = 0; i<nx; i++ )
  { for ( int j = 0; j<ny; j++ )
    {
     WriteGridSquareCoordinates(i, j);
     if (WRLoutFile.fail()) return 0;
    }
  }
  WRLoutFile << "       ] # end point\n"
             << "      } # end Coordinate\n"
             << "      normal Normal {\n"
             << "       vector [ ]\n"
             << "      } # end Normal\n";

  if (ColouredGrid)
  {
    WRLoutFile << "      color Color {\n"
               << "       color [\n";
    // Output a Coloured Grid - each grid triangle is it's own colour
    for ( int i = 0; i<(nx-1); i++ )
    { for ( int j = 0; j<(ny-1); j++ )
      {
        WriteGridSquareColour(i, j);
        if (WRLoutFile.fail()) return 0;
      }
    } // End nested for's.
    WRLoutFile << "       ] # end color\n"
               << "      } # end Color\n"
               << "      colorPerVertex FALSE\n";
  } // end if( ColouredGrid)

  WRLoutFile << "      coordIndex [\n";

  for ( int i = 0; i<(nx-1); i++ )
  { for ( int j = 0; j<(ny-1); j++ )
    {
      WriteGridSquareCoordinateIndices(i, j);
      if (WRLoutFile.fail()) return 0;
    }
  }  // end nested for's

  WRLoutFile << "      ] # end coordIndex\n";

  if (ColouredGrid)
  {
    WRLoutFile << "      colorIndex [\n";
    WRLoutFile << "       ";
    for ( int i = 0; i<ci; i++ )
    {
      WRLoutFile << i << ", ";
      if (i && (i % 20) == 0) WRLoutFile << "\n       ";
    }
    WRLoutFile << "\n";
    if (WRLoutFile.fail()) return 0;

    WRLoutFile << "      ] # end colorIndex\n";
  }
  WRLoutFile << "      ccw FALSE\n"
             << "      solid FALSE\n"
             << "      convex TRUE\n"
             << "      creaseAngle 0\n"
             << "     } # end IndexedFaceSet\n"
             << "    } # end Shape\n"
             << "  } # end GRID Group\n";

  // Grid is done - output the 3d Axes (three simple lines).
  if (DrawingOptions.threedaxes)
  {
    int iAxesOrigin = 0;
    int iAxesZmax = 1;
    int iAxesXmax = 2;
    int iAxesYmax = 3;
    WRLoutFile << "  ,\n"
               << "  DEF AXES Group {\n"
               << "   children\n"
               << "    DEF _shape Shape {\n"
               << "     appearance Appearance {\n"
               << "      material Material {\n"
               << "      diffuseColor "
               << ((float)GetRValue(AxisPen) / 255.) << " "
               << ((float)GetGValue(AxisPen) / 255.) << " "
               << ((float)GetBValue(AxisPen) / 255.) << "\n"
               << "      }\n"
               << "     }\n"
               << "     geometry IndexedLineSet {\n"
               << "      coord Coordinate {\n"
               << "       point [\n"
               << "        # Axes coordinates follow\n"
               << "        " << xmin << " " << ymin << " " << zmin << ",\n"
               << "        " << xmin << " " << ymin << " " << zmax << ",\n"
               << "        " << xmax << " " << ymin << " " << zmin << ",\n"
               << "        " << xmin << " " << ymax << " " << zmin << ",\n"
               << "       ] # end point\n"
               << "      } # end Coordinate\n"
               << "      coordIndex [\n"
               << "       " << iAxesOrigin << ", " << iAxesZmax << ", -1,\n"
               << "       " << iAxesOrigin << ", " << iAxesXmax << ", -1,\n"
               << "       " << iAxesOrigin << ", " << iAxesYmax << ", -1\n"
               << "      ]\n"
               << "     } # end IndexedLineSet\n"
               << "    } # end Shape\n"
               << "  } # end AXES Group\n";
  }

  // Now output the outline.
  int SizeOfOutline = OutLine.Size();
  if( SizeOfOutline > 1 )
  {
    WRLoutFile << "  ,\n"
               << "  DEF OUTLINE Group {\n"
               << "   children\n"
               << "    DEF _shape Shape {\n"
               << "     appearance Appearance {\n"
               << "      material Material {\n"
               << "       diffuseColor "
               << ((float)GetRValue(OutlinePen) / 255.) << " "
               << ((float)GetGValue(OutlinePen) / 255.) << " "
               << ((float)GetBValue(OutlinePen) / 255.) << "\n"
               << "      }\n"
               << "     }\n"
               << "     geometry IndexedLineSet {\n"
               << "      coord Coordinate {\n"
               << "       point [\n"
               << "        # Outline coordinates follow\n";

    // Output the Outline Coordinates.

    // Set z to zmin if no Z coordinate read for outline.
    bool NoZ = false;
    if (OutLine.zMin() == OutLine.zMax()) NoZ = true;
    for ( int i = 0; i < SizeOfOutline; i++ )
    {
      float x = OutLine.x(i) + dxnormalize;
      float y = OutLine.y(i) + dynormalize;
      float z = zmin + zadjust;
      if (!NoZ) z = OutLine.z(i) + zadjust;
      if (DXFFaceScaleZ && (zRatio != 0.0))  // Scale z axis if requested.
        z = (z - zMin) * zScale;

      WRLoutFile << "       " << x << " " << y << " " << z << ",\n";
    } // End for ( i =

    WRLoutFile << "       ] # end point\n"
               << "      } # end Coordinate\n"
               << "      coordIndex [\n";

    WRLoutFile << "       ";
    for ( int i = 0; i < SizeOfOutline; i++ )
    {
      if( OutLine.flags(i) & 1 ) WRLoutFile << " -1, \n"; // Move here?
      WRLoutFile << i << ", ";
      if (i && (i % 20) == 0) WRLoutFile << "\n       ";
    }
    WRLoutFile << "      ]\n"
               << "     } # end IndexedLineSet\n"
               << "    } # end Shape\n"
               << "  } # end OUTLINE Group\n";
  } // End if( OutlineSize > 1 )

  WRLoutFile << " ] # end children\n"
             << "} # end Group\n";
  return 1;
}
//**************************************************************
//       O u t p u t   V R M L   f i l e
//**************************************************************
int OutputVRMLFile( char szFile[] )
{
  ci = 0;

  WRLoutFile.open( szFile, ios::out );

  if( !WRLoutFile )
  { NotifyUser( IDS_NOOUTPUTFILE );
    return 0;
  }

  LoadNormalization( dxnormalize, dynormalize );
  zadjust = ScatterData.zAdjust();

  SetWaitCursor();

  Zgrid.zratio(0);
  WRLOutputFailed = FALSE;

  int success = VRML2format ? OutputVRML2Format() : OutputVRML1Format();
  if (!success) AbortWriteWRLFile(szFile);

  WRLoutFile.close();
  RestoreCursor();
  return success;
}
