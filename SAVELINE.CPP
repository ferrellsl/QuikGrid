//*****************************************************
//                   Save Line 
// To save calls describing lines. Class implementation
//                 CopyRight 1993 by John Coulthard
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
//******************************************************

#include <windows.h>
#pragma hdrstop 
#include "assert.h"  
#include "saveline.h"
#include "utilitys.h"
#include "rc.h" 

//#define max(a,b) (((a)>(b))?(a):(b))
//#define min(a,b) (((a)<(b))?(a):(b))

SaveLineType::SaveLineType()
{
  size = 0;
  xmax=xmin=ymax=ymin = 0.0;  
  for( int i = 0; i< SaveLineMax/SaveLineBlock ; i++ ) array[i] = 0;
}


void SaveLineType::SetNext(  const int x,  const int y, const unsigned char pen )
{
  if( size >= SaveLineMax ) return;
   
  int block = size/SaveLineBlock;
  assert( block < SaveLineMax/SaveLineBlock );

  int offset = size%SaveLineBlock;
  if( array[block] == 0 ) array[block] = new LineType[SaveLineBlock];
  assert( array[block] != 0 );
  array[block][offset].x = x;
  array[block][offset].y = y;
  array[block][offset].pen = pen;
  if( size == 0 )
   { xmax = xmin = x;
     ymax = ymin = y;
   }
  else
   {
     xmax = max( xmax, x);
     xmin = min( xmin, x);
     ymax = max( ymax, y);
     ymin = min( ymin, y);
   }
  if( size == (SaveLineMax-1) ) NotifyUser( IDS_TOOMANYCONTOURS );
  ++size;
}

void SaveLineType::Reset()
{ size = 0;
  xmax=xmin=ymax=ymin=0.0;
  for( int i =1; i< SaveLineMax/SaveLineBlock; i++) // Leave the 1st block around.
    if( array[i] !=0 ) { delete[] array[i]; array[i] = 0; }
}

SaveLineType::~SaveLineType()
{ 
  for( int i = 0; i < SaveLineMax/SaveLineBlock; i++ )
    if( array[i] != 0 ) delete[] array[i];
}
