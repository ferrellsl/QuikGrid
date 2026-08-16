//****************************************************************
//                        S u r f a c e   G r i d
//
//                 Copyright (c) 1993-2003 by W. John Coulthard
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
// Defines the QuikGrid 2 dimensional array that is generated from
// the scattered data points.
//
// 
//*****************************************************************

#include "assert.h"
#include "surfgrid.h"
#define max(a,b) (((a)>(b))?(a):(b))
#define min(a,b) (((a)<(b))?(a):(b))

void SurfaceGrid::Initialize()
{
  minz = maxz = 0.0;
  scalez = 1.0;
  View.xStart = View.yStart = 0;
  View.xIncrement = View.yIncrement = 1;
  View.xEnd = nx;
  View.yEnd = ny;
  CurrentView = UnitView = OldView = View; 
  adjustz = adjustx = adjusty = 0.0;
  first = 1;
}

SurfaceGrid::SurfaceGrid( const int i, const int j )
{
  int k;
  nx=i;
  ny=j;
  assert( nx <= MaxXYdimension );
  assert( ny <= MaxXYdimension );
  Initialize();
  zgrid = new float[(long)nx*(long)ny];
  assert( zgrid != 0 );
  xvect = new float[nx];
  assert( xvect != 0 );
  yvect = new float[ny];
  assert( yvect != 0 );
  for( k = 0; k<nx; k++ ) xvect[k] = k ;
  for( k = 0; k<ny; k++ ) yvect[k] = k;
}

void SurfaceGrid::Reset()
{
	Initialize();
}

void SurfaceGrid::New( const int i, const int j )
{
  int k;
  Initialize();
  if( (i==nx)&&(j==ny) ) return;
  delete[] zgrid; delete[] xvect; delete[] yvect;
  nx=i;
  ny=j;
  assert( nx <= MaxXYdimension );
  assert( ny <= MaxXYdimension );
  Initialize(); // Intialize again as things have changed. 
  zgrid = new float[(long)nx*(long)ny];
  assert( zgrid != 0 );
  xvect = new float[nx];
  assert( xvect != 0 );
  yvect = new float[ny];
  assert( yvect != 0 );
  for( k = 0; k<nx; k++ ) xvect[k] = k ;
  for( k = 0; k<ny; k++ ) yvect[k] = k;
}

void  SurfaceGrid::zset( const int i, const int j, const float a )
{
  assert( (i>=0) && (j>=0) );
  int offset = (long)j*nx + (long)i;
  zgrid[offset] = a ;
  if( a < 0.0 ) return;
  if (first){ minz=maxz=a; first=0; }
  else { if( a > maxz ) maxz = a;
         if( a < minz ) minz = a; 
       }
}

void  SurfaceGrid::xset( const int i, const float a )
{
  assert( (i<nx) && (i>=0) );
  xvect[i] = a ;
}

void  SurfaceGrid::yset( const int i, const float a )
{
  assert( (i<ny) && (i>=0) );
  yvect[i] = a ;
}

float SurfaceGrid::z( const int i, const int j)
{
 long ix =  View.xStart + i*View.xIncrement;
 long iy =  View.yStart + j*View.yIncrement;
 return  (zgrid[iy*nx+ix]-adjustz)*scalez;
}

void SurfaceGrid::zratio( const float ratio )
// Provides two services.
// 1. Normalizes x,y,z to zero so rotation works nicelyh
// 2. Scales z coordinates so that they span "ratio" of the
//    distance for the x axis.
{
  if( ratio == 0.0 )  // Turn off all adjustments and scaling.
   { adjustz = adjustx = adjusty = 0.0; scalez = 1.0; return; }

  adjustz = minz;      // Normalize z, x and y so that they
  adjustx = xvect[0];  // start at zero
  adjusty = yvect[0];
// if ratio positive scale the z units. 
  if( ratio > 0.0 ) scalez = ratio*(xvect[nx-1]-xvect[0])/(maxz - minz);
}

void SurfaceGrid::map( float &x, float &y, float &z )
{
  x = x - adjustx;
  y = y - adjusty;
  z = (z - adjustz)*scalez;
}