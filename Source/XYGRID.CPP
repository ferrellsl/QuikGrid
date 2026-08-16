//*****************************************************************
//                      X Y G r i d
//
//                     Copyright (c) 1993 by W. John Coulthard
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
//*****************************************************************
#pragma hdrstop
#include "assert.h"
#include "xygrid.h"

xyGridClass::xyGridClass( int i, int j )
{
  nx=i;
  ny=j;
  zgrid = new point2d[nx*ny];
  assert( zgrid != 0 );
}

void xyGridClass::New( int i, int j )
{
  if( (i==nx)&&(j==ny) ) return; 
  delete[] zgrid; 
  nx=i;
  ny=j;
  zgrid = new point2d[nx*ny];
  assert( zgrid != 0 );
}

void  xyGridClass::set( int i, int j,  int ia,  int ib )
{ assert( (i<nx) && (i>=0) );
  assert( (j<ny) && (j>=0) );
  int offset = j*nx + i;
  assert( offset <= nx*ny ); 
  zgrid[offset].ix = ia ;
  zgrid[offset].iy = ib ; 
}

