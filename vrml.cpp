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
int DXFFaceScaleZ = FALSE;
int VRMLtriangles = TRUE;

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

//******************************************************************
//         A b o r t   W r i t e  WRL  F i l e
//******************************************************************
static void AbortWriteWRLFile( char FileName[] )
{
  ostrstream Buf;

  Buf << "Error writing WRNL output to the file "
	   << FileName 
           << ". Output terminated. " << ends; 

  char *szBuf = Buf.str();
  NotifyUser( szBuf );
  delete szBuf;

  RestoreCursor();
 }
  
//**************************************************************
//      W r i t e   T r i a n g l e
//**************************************************************
static void WriteTriangle( int i, int j, int k )
{
 Colour = ChooseZColour( AverageZ );
 WRLoutFile << " Material { " << endl
            << "   diffuseColor "
            << ((float)GetRValue(Colour)/255.)<< " "
            << ((float)GetGValue(Colour)/255.)<< " "
            << ((float)GetBValue(Colour)/255.)<< endl
            << "   ambientColor .2 .2 .2 " << endl
            << "   specularColor 0 0 0 " << endl
            << "   emissiveColor 0 0 0 " << endl
            << "   shininess .2 " << endl
            << "   transparency 0 } # end Material " << endl
            << " IndexedFaceSet { coordIndex [ " ;
 WRLoutFile << (i) << ", " // i,j,k point to the cordners of the triangle
            << (j) << ", "
            << (k) << ", -1,]}" << endl;
}
//**************************************************************
//      W r i t e   S q u a r e
//**************************************************************
static void WriteSquare( int i, int j, int k, int l )
{
 Colour = ChooseZColour( AverageZ );
 WRLoutFile << " Material { " << endl
            << "   diffuseColor "
            << ((float)GetRValue(Colour)/255.)<< " "
            << ((float)GetGValue(Colour)/255.)<< " "
            << ((float)GetBValue(Colour)/255.)<< endl
            << "   ambientColor .2 .2 .2 " << endl
            << "   specularColor 0 0 0 " << endl
            << "   emissiveColor 0 0 0 " << endl
            << "   shininess .2 " << endl
            << "   transparency 0 } # end Material " << endl
            << " IndexedFaceSet { coordIndex [ " ;
 WRLoutFile << (i) << ", " // i,j,k, l point to the cordners of the triangle
            << (j) << ", "
            << (k) << ", "
            << (l) << ", -1,]}" << endl;
}
//**************************************************************
//      W r i t e   G r i d    s q u a r e
//**************************************************************
static void WriteGridSquare( int i, int j )
{

   // i,j define the corner of the grid square to be output.
 	// Draw two triangles per qrid square.. clockwise.
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
    WriteTriangle( i*ny+j, i*ny+j+1,(i+1)*ny+j+1); // clockwise
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
    WriteTriangle( i*ny+j,(i+1)*ny+j+1, (i+1)*ny+j); // clockwise
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
  WriteSquare( i*ny+j, i*ny+j+1,(i+1)*ny+j+1, (i+1)*ny+j); // clockwise
} // end else - output squares.
}
//**************************************************************
//      G e t  V R M L  f i l e N a m e
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
//       O u t p u t  V R M L   f i l e
//**************************************************************
int OutputVRMLFile( char szFile[] )
{
  static int i, j, ij, i1j, i1j1, ij1,
             AxesOffset, OutlineOffset,
             iAxesOrigin, iAxesZmax, iAxesXmax, iAxesYmax,
             SizeOfOutline;
  static float x, y, z,
               xmin, xmax, ymin, ymax, zmin, zmax;
  static bool NoZ; 


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

 WRLoutFile << "#VRML V1.0 ascii" << endl   // Output Header information.
	 << "Separator { " << endl
	 << "   DEF SceneInfo Info { string \"Generated by ";

 WRLoutFile << "QuikGrid.\" } # end SceneInfo" <<  endl;

 WRLoutFile << "ShapeHints { "      << endl
	 << "   vertexOrdering CLOCKWISE"  << endl
    << "   shapeType UNKNOWN_SHAPE_TYPE"      << endl
    << "   faceType CONVEX" << endl
    << "   creaseAngle 0.0 } # end ShapeHints " << endl;

 WRLoutFile << "DEF X1_X2_GRID Separator { " << endl
            << "  Material { " << endl
            << "    diffuseColor "
                << ((float)GetRValue(GridPen)/255.)<< " "
                << ((float)GetGValue(GridPen)/255.)<< " "
                << ((float)GetBValue(GridPen)/255.)<< endl
            << "    ambientColor .2 .2 .2 " << endl
            << "    specularColor 0 0 0 " << endl
            << "    emissiveColor 0 0 0 " << endl
            << "    shininess .2 " << endl
            << "    transparency 0 } # end Material " << endl;

 WRLoutFile << " Coordinate3 {  point [ " << endl;

 nx = Zgrid.xsize();
 ny = Zgrid.ysize();

 zRatio = (float)PaintSurfaceRatio()*.01;
 xRange = Zgrid.x(nx-1) - Zgrid.x(0);
 zMin   = Zgrid.zmin();
 zRange = Zgrid.zmax() - Zgrid.zmin();
 zScale = zRatio*(xRange/zRange);

 // Output the grid coordinates.
 WRLoutFile << "# Grid coordinates follow - Offset 0 " << endl;
 for (   i = 0; i<nx; i++ )
 { for ( j = 0; j<ny; j++ )
	{
    z = Zgrid.z(i,j);
    if( z < 0 ) z = 0; // Set undefined nodes to zero (they will be ignored anyhow).
    else if( DXFFaceScaleZ &&( zRatio!=0.0) )  // Scale z axis if requested.
             z  = (z-zMin) *zScale;
    x = Zgrid.x(i)+dxnormalize;
    y = Zgrid.y(j)+dynormalize;
    WRLoutFile << x << " " << y << " " << z << "," << endl;
   }
 }

 // Output the Axes coordinates.

 AxesOffset = nx*ny;
 xmax = Zgrid.xmax()+ dxnormalize;          // For axes on the generated grid.
 ymax = Zgrid.ymax()+ dynormalize;
 xmin = Zgrid.xmin()+ dxnormalize;
 ymin = Zgrid.ymin()+ dynormalize;

 // Z axes get special treatment - if overridden or scaled.

 zmax = Zgrid.zmax();
 zmin = Zgrid.zmin();

 // if the axes are to be plotted at a different z minimum.
   if( (ZmaxLabel != ZminLabel) &&
       (ZmaxLabel > zmin-zadjust) &&
       (ZminLabel < zmax-zadjust) )
  {
   zmin = ZminLabel+zadjust;
   zmax = ZmaxLabel+zadjust; 
  }

  if( DXFFaceScaleZ &&( zRatio!=0.0) )  // Scale z axis if requested.
            {zmin  = (zmin-zMin) *zScale;
             zmax  = (zmax-zMin) *zScale; }

 // end special treatment for z axis.

 WRLoutFile << "# Axes coordinates follow - Offset " <<  AxesOffset << endl;
 WRLoutFile << xmin << " " << ymin << " " << zmin << "," << endl;
 WRLoutFile << xmin << " " << ymin << " " << zmax << "," << endl;
 WRLoutFile << xmax << " " << ymin << " " << zmin << "," << endl;
 WRLoutFile << xmin << " " << ymax << " " << zmin << "," << endl;
 iAxesOrigin = AxesOffset;
 iAxesZmax   = AxesOffset+1;
 iAxesXmax   = AxesOffset+2;
 iAxesYmax   = AxesOffset+3;

 OutlineOffset = AxesOffset + 4;

 // Output the Outline Coordinates.

 SizeOfOutline = OutLine.Size();
 if( SizeOfOutline > 1 )
 {
   WRLoutFile << "# Outline coordinates follow - Offset " <<  OutlineOffset << endl;
   // Set z to zmin if no Z coordinate read for outline.
   NoZ = false;
   if( OutLine.zMin() == OutLine.zMax() ) NoZ = true;
   for ( i = 0; i < SizeOfOutline; i++ )
   {
     x = OutLine.x(i)+ dxnormalize;
     y = OutLine.y(i)+ dynormalize;
     z = zmin + zadjust;
     if( !NoZ ) z = OutLine.z(i)+ zadjust;
     if( DXFFaceScaleZ &&( zRatio!=0.0) )  // Scale z axis if requested.
            z  = (z-zMin) *zScale;
             
     WRLoutFile << x << " " << y << " " << z << "," << endl;
   } // End for ( i =
} // End if Outline() < 2

 WRLoutFile << "   ] # end point " << endl
            << " } # end Coordinate 3" << endl;

 if( ColouredGrid)
 {
   // Output a Coloured Grid - each grid triangle is it's own colour
   for ( i = 0; i<(nx-1); i++ )
  { for ( j = 0; j<(ny-1); j++ )
	{
     WriteGridSquare( i, j );
     if( WRLoutFile.fail() )
			{ AbortWriteWRLFile( szFile );  WRLoutFile.close(); return 0; }
	}
 } // End nested for's.
 } // end if( ColouredGrid)
 else // Each grid triangle is the same colour - more compact output.
 {
 WRLoutFile << " IndexedFaceSet {  coordIndex [ " << endl;

 for (   i = 0; i<(nx-1); i++ )
 { for ( j = 0; j<(ny-1); j++ )
	{
	// Draw two triangles per qrid square.. clockwise.
     if( Zgrid.z(i,j) < 0.0 ) continue;     // Don't draw triangle
     if( Zgrid.z(i+1,j+1) < 0.0 ) continue; // if a node is undefined.

         ij = i*ny+j;            // Calculate the offsets for the 4 vertices
         i1j = (i+1)*ny+j;       // of the grid square oriented at "ij".
         ij1 =  i*ny+j+1;        // i1j == vertice at "i+1", "j" etc.
         i1j1 = (i+1)*ny+j+1;

         if(Zgrid.z(i,j+1) >= 0.0)
         WRLoutFile << ij << ", " << ij1 << ", " << i1j1 << ", -1," << endl;  // Clockwise.

         if(Zgrid.z(i+1,j) >= 0.0)
         WRLoutFile << ij << ", " << i1j1 << ", " << i1j << ", -1," << endl;  // Clockwise

     if( WRLoutFile.fail() )
			{ AbortWriteWRLFile( szFile );  WRLoutFile.close(); return 0; }
	}
 }  // end nested for's

 WRLoutFile << "    ] # end coordIndex " << endl
            << "  } # end IndexedFaceSet " << endl;
 } // end else - output non coloured grid.

 // Grid is done - output the 3d Axes (three simple lines).
 if( DrawingOptions.threedaxes )
 {
   WRLoutFile << endl << " # The 3d Axes follow " << endl;
   Colour = AxisPen;
   WRLoutFile << " Material { " << endl
            << "   diffuseColor "
            << ((float)GetRValue(Colour)/255.)<< " "
            << ((float)GetGValue(Colour)/255.)<< " "
            << ((float)GetBValue(Colour)/255.)<< endl
            << "   ambientColor .2 .2 .2 " << endl
            << "   specularColor 0 0 0 " << endl
            << "   emissiveColor 0 0 0 " << endl
            << "   shininess .2 " << endl
            << "   transparency 0 } # end Material " << endl
            << " IndexedLineSet { coordIndex [ " << endl;
    WRLoutFile << iAxesOrigin << ", " << iAxesZmax << ", -1," << endl;

    WRLoutFile << iAxesOrigin << ", " << iAxesXmax << ", -1," << endl;
    WRLoutFile << iAxesOrigin << ", " << iAxesYmax << ", -1 ] }" << endl;
  }

  // Now output the outline.

if( SizeOfOutline > 1 )
{
  WRLoutFile << endl << " # The Outline follows " << endl;
 Colour = OutlinePen;
 WRLoutFile << " Material { " << endl
            << "   diffuseColor "
            << ((float)GetRValue(Colour)/255.)<< " "
            << ((float)GetGValue(Colour)/255.)<< " "
            << ((float)GetBValue(Colour)/255.)<< endl
            << "   ambientColor .2 .2 .2 " << endl
            << "   specularColor 0 0 0 " << endl
            << "   emissiveColor 0 0 0 " << endl
            << "   shininess .2 " << endl
            << "   transparency 0 } # end Material " << endl;

 WRLoutFile << " IndexedLineSet { coordIndex [ " << endl;
 for ( i = 0; i < SizeOfOutline; i++ )
   {
     if( OutLine.flags(i)&1 )WRLoutFile << " -1, " << endl; // Move here? 
     WRLoutFile << (i+OutlineOffset) << ", " << endl;
   }
   WRLoutFile << " -1 ] }" << endl;
 } // End if( OutlineSize > 1 )


 WRLoutFile << " } # end QuikGrid Separator " << endl
            << "} # end VRML Separator "    << endl;

 WRLoutFile.close();
 RestoreCursor();
 return 1;
}

