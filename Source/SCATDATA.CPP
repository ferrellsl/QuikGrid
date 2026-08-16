//******************************************************
//                  S c a t  D a t a
//                       
//     Copyright (c) 1993 - 1999 by John Coulthard
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
// April 7/99 : Change to get data in blocks of 64000.
// April 14/99: Increase limit to 12 million points.
// March 16/01: Change truncate to recalculate max's and min's. 
//******************************************************

#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <windows.h>
#include "assert.h" 
#include "scatdata.h"

ScatData::ScatData()
{
  //MaxPoints = 128000000; // 128 million!
  MaxPoints = SIZE_MAX; // Use OS limit
 // sizeincrement = 16000; // Initial size to get.
  sizeincrement = 1600000; // Initial size to get.
  blank = "";
  size = 0;
  xmax=xmin=ymax=ymin=zmax=zmin=zadjust=0.0;
  X = new float[sizeincrement];
  Y = new float[sizeincrement];
  Z = new float[sizeincrement];
  Comment = new (char (*[sizeincrement]) );
  Flags = new unsigned char[sizeincrement];
  assert( X!=0 && Y!=0 && Z!=0 && Comment != 0 && Flags != 0 );
  memsize = sizeincrement;
}

ScatData::~ScatData()
{ delete [] X;
  delete [] Y;
  delete [] Z;
  for( long i = 0; i<size; i++)
   { if( Comment[i] != blank ) delete [] Comment[i]; }
  delete [] Comment;      
}

void ScatData::SetNext( float x, float y, float z, char comarg[], unsigned char c )
{
  if( size == memsize )
    { // out of space - get bigger area and copy existing to it.
      memsize = memsize + memsize/2;

      tempXYZ = new float[memsize];
      assert( tempXYZ!= 0 );
		memcpy( tempXYZ, X, sizeof(X[0])*size);
      delete [] X;
      X = tempXYZ;

      tempXYZ = new float[memsize];
      assert( tempXYZ!= 0 );
      memcpy( tempXYZ, Y, sizeof(Y[0])*size);
      delete [] Y;
      Y = tempXYZ;

      tempXYZ = new float[memsize];
      assert( tempXYZ!= 0 );
      memcpy( tempXYZ, Z, sizeof(Z[0])*size);
      delete [] Z;
      Z = tempXYZ;

      tempComment = new (char (*[memsize]) );
      assert( tempComment!= 0 );
		memcpy( tempComment, Comment, sizeof(Comment[0])*size );
      delete [] Comment;
      Comment = tempComment;

      tempFlags = new ( unsigned char[memsize] );
      assert ( tempFlags != 0 );
      memcpy( tempFlags, Flags, sizeof(Flags[0])*size );
      delete [] Flags;
      Flags = tempFlags;

    }
  assert( size < memsize );

  X[size] = x;
  Y[size] = y;
  Z[size] = z;
  Flags[size] = c;
  if( size == 0 )
   { xmax = xmin = x;
     ymax = ymin = y;
     zmax = zmin = z;
   }
  else
   { if( x > xmax ) xmax = x;
     if( x < xmin ) xmin = x;
     if( y > ymax ) ymax = y;
     if( y < ymin ) ymin = y;
     if( z > zmax ) zmax = z;
     if( z < zmin ) zmin = z;
   }
  Comment[size] = blank;
  ++size;
  if( comarg == NULL ) return;
  if( comarg[0] == NULL ) return;
  if( strspn(comarg, " ") == strlen( comarg ) ) return; // Ignore if blank line.

  Comment[size-1] = new char[strlen(comarg)+1];
  strcpy( Comment[size-1], comarg);
}

void ScatData::Reset()
{ for( long i = 0; i< size; i++ )
  {
    if( Comment[i] != blank ) delete [] Comment[i];
  }
  size = 0;
  xmax=xmin=ymax=ymin=zmax=zmin=zadjust=0.0;
}

void ScatData::SetFlags( const long i, unsigned char c )
{
  assert( i < size && i >= 0);
  Flags[i] = c;
}

void ScatData::SetZ( const long i, const float z )
{
  assert( i < size && i >= 0 );
  Z[i] = z;
  if( z > zmax ) zmax = z;
  if( z < zmin ) zmin = z; 
}


void ScatData::SetComment( const long i, char comarg[] )
{
 assert( i < size ); 
 if( Comment[i] != blank ) delete Comment[i] ;
 Comment[i] = blank;
 if( comarg == NULL ) return;
 if( comarg[0] == NULL ) return;
 if( strspn(comarg, " ") == strlen( comarg ) ) return; // Ignore if blank line.
 Comment[i] = new char[strlen(comarg)+1];
 strcpy( Comment[i], comarg);
}

void ScatData::move( const long i, const long j)
{
 assert( i >= 0 && i < size );
 assert( j >= 0 && j < size );
 if( i == j ) return;
 assert( j > i ); 
 X[i] = X[j];
 Y[i] = Y[j];
 Z[i] = Z[j];
 Flags[i] = Flags[j];
 if( Comment[i] != blank ) delete [] Comment[i];
 Comment[i] = Comment[j];
 Comment[j] = blank; 
}

void ScatData::truncate( const long i)
{
 if( i == size ) return;
 assert( i >= 0 && i < size );
 for( long j = i; j<size; j++)
   { if( Comment[j] != blank ) { delete [] Comment[j]; Comment[j] = blank; }  }
 size = i ;
 // We need to recompute the maximum and minimum x, y and z.
 xmax = xmin = X[0];
 ymax = ymin = Y[0];
 zmax = zmin = Z[0];
 for( long j = 1; j<size; j++)
 {
     float x = X[j];
     float y = Y[j];
     float z = Z[j];

     if( x > xmax ) xmax = x;
     if( x < xmin ) xmin = x;
     if( y > ymax ) ymax = y;
     if( y < ymin ) ymin = y;
     if( z > zmax ) zmax = z;
     if( z < zmin ) zmin = z;
 }
}
