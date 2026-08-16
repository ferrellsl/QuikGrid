//***********************************************************
//                  D X F F A C E
//  Save grid in DXF 3D FACE  format.
//                Copyright (c) 1997 - 2000 by John Coulthard
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
// Feb. 26/97: Created.
// Mar. 4/97: Tidy up.
// Oct. 12/99: Change so calling routine supplies file name.
// Mar. 1/00:  Support scaling of the z coordinate.
// Mar. 15/00: Change to output 12 digits of accuracy. 
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
using namespace std;
int DXFFaceScaleZ = FALSE;
int HighDXFOutputPrecision = TRUE;

static ofstream DXFoutFile(" ", ios::out );
static double dxnormalize, dynormalize;
static float zadjust;
static int DXFOutputFailed = FALSE;
static OPENFILENAME ofn;
static float zRatio;
static float zRange, zMin, xRange, zScale;

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
//     O u t p u t D X F F a c e
//**************************************************************
// Output a DXF line - also provide service of adjusting
// x,y,z values before output. 
static void OutputDXFFace(float x, float y, float za,
								  float x2, float y2, float za2,
								  float x3, float y3, float za3)
{
   static float z, z2, z3;
   z  = za;
   z2 = za2;
   z3 = za3;

	if( z < 0. || z2 < 0. || z3 < 0. ) return;

   if( DXFFaceScaleZ &&( zRatio!=0.0) )  // Scale z axis if requested.
    {
      z  = (z-zMin) *zScale;
      z2 = (z2-zMin)*zScale;
      z3 = (z3-zMin)*zScale;
    }

   DXFoutFile  << "  0" << endl
			 << "3DFACE"   << endl
	       << "  8"      << endl
			 << "GRID"     << endl ;


	DXFoutFile	<< " 10"      << endl
			 << ((double)x+dxnormalize) << endl
			 << " 20"           << endl
			 << ((double)y+dynormalize) << endl
			 << " 30"           << endl
			 << (z-zadjust)     << endl;

	DXFoutFile  << " 11"      << endl  // next vertice.
			 << ((double)x2+dxnormalize)<< endl
			 << " 21"           << endl
			 << ((double)y2+dynormalize)<< endl
			 << " 31"           << endl
			 << (z2-zadjust)    << endl;

	DXFoutFile  << " 12"      << endl  // next vertice.
			 << ((double)x3+dxnormalize)<< endl
			 << " 22"           << endl
			 << ((double)y3+dynormalize)<< endl
			 << " 32"           << endl
			 << (z3-zadjust)    << endl;

	DXFoutFile  << " 13"      << endl  // next vertice.
			 << ((double)x3+dxnormalize)<< endl       // Repeat the last one.
			 << " 23"           << endl
			 << ((double)y3+dynormalize)<< endl
			 << " 33"           << endl
			 << (z3-zadjust)    << endl;
}

//**************************************************************
//      G e t  D X F 3 D F A C E  f i l e N a m e
//**************************************************************
int GetDXF3DFaceFileName( HWND &hwnd, char szFile[] )
{
 szFile[0] = '\0';

 memset( &ofn, 0, sizeof(OPENFILENAME) ); // zero structure members

 ofn.lStructSize = sizeof(OPENFILENAME);
 ofn.hwndOwner = hwnd;  // =NULL if no owner required.
 //ofn.lpstrFilter = "DXF files\0*.DXF\0 3DF\0*.3DF\0All files\0*.*\0\0";
 ofn.lpstrFilter = "DXF files\0*.dxf\0All files\0*.*\0\0";
 ofn.nFilterIndex = 1;
 ofn.lpstrFile = szFile;
 ofn.nMaxFile = 255;        //sizeof(szFile);
 ofn.Flags = OFN_PATHMUSTEXIST|OFN_OVERWRITEPROMPT;
 ofn.lpstrTitle = "Generate DXF 3D FACE output to file" ;
 ofn.lpstrDefExt = "dxf";
 if( !GetSaveFileName( &ofn ) ) return 0;
 return 1;
}
//**************************************************************
//       O u t p u t  D X F 3 D F A C E  f i l e
//**************************************************************
int OutputDXF3DFaceFile( HWND &hwnd, char szFile[] )
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

 DXFoutFile << "999" << endl   // Output Header information.
	 << "DXF 3D FACE output created by " ;

 DXFoutFile << "QuikGrid." << endl;

 if( HighDXFOutputPrecision) DXFoutFile << setprecision( 12 ) ;

 DXFoutFile << "  0"      << endl      // Geometric entities go in an ...
	 << "SECTION"  << endl
	 << "  2"      << endl
         << "ENTITIES" << endl;  // ... ENTITIES section.

 int nx = Zgrid.xsize();
 int ny = Zgrid.ysize();

 zRatio = (float)PaintSurfaceRatio()*.01;
 xRange = Zgrid.x(nx-1) - Zgrid.x(0);
 zMin   = Zgrid.zmin();
 zRange = Zgrid.zmax() - Zgrid.zmin();
 zScale = zRatio*(xRange/zRange);

 static int i, j;

 for (   i = 0; i<(nx-1); i++ )
 { for ( j = 0; j<(ny-1); j++ )
	{
	// Draw two triangles per qrid square.. clockwise.
	  OutputDXFFace( Zgrid.x(i), Zgrid.y(j), Zgrid.z(i,j),
						  Zgrid.x(i), Zgrid.y(j+1), Zgrid.z(i, j+1),
						  Zgrid.x(i+1), Zgrid.y(j+1), Zgrid.z(i+1,j+1) );

	  OutputDXFFace( Zgrid.x(i), Zgrid.y(j), Zgrid.z(i,j),
						  Zgrid.x(i+1), Zgrid.y(j), Zgrid.z(i+1, j),
						  Zgrid.x(i+1), Zgrid.y(j+1), Zgrid.z(i+1,j+1) );

		if( DXFoutFile.fail() )
			{ AbortWriteDXFFile( szFile );  DXFoutFile.close(); return 0; }
	}
 }

 DXFoutFile << "  0"    << endl    // Terminate ENTITIES section.
		 << "ENDSEC" << endl;

 DXFoutFile << "  0"    << endl    // Output end of file indicator.
            << "EOF"    << endl;

 DXFoutFile.close();
 RestoreCursor();
 return 1; 
}

