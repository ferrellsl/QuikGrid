//*****************************************************
//             G r i d   X   T y p e 
//                       
//               Copyright (c) 1993 - 1998 by John Coulthard
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
// Routine used by XPAND to store the data that associates data
// points to their nearest grid location. Hence, after sorting, it is
// a very fast lookup to find all data points near to a give grid
// interesection.
//
// Sept. 21/98: Converted over to Win32 - long variables
// Apr. 9-12/99: Convert to use lookup tables.
//******************************************************
#include <stdlib.h>
#include "assert.h"
#include "GridXTyp.h"

GridXType::GridXType(const long iSize, const long ax, const long ay)
{
  nx = ax; ny = ay;
  Size = iSize;
  np = 0; 
  FindPoints = 0;
  Lookup = 0;
  PreviousSearch = -1;  
  if( Size == 0 ) return;
  assert( (Size > 0));
  FindPoints = new LocateStructure[Size];
  assert( FindPoints != 0 );
  LookupSize = nx * ny;
  Lookup = new long [ LookupSize ] ;
  assert( Lookup != 0 );
}

void GridXType::New( const long iSize, const long ax, const long ay )
{
  PreviousSearch = -1;
  np = 0; 
  if( iSize == Size && ax == nx && ay == ny ) return;
  nx = ax; ny = ay;
  if( FindPoints != 0 ) delete[] FindPoints;
  FindPoints = 0;
  if( Lookup != 0 ) delete[] Lookup;
  Lookup = 0;
  Size = iSize;
  if( Size == 0 ) return;
  assert( (Size > 0) );
  FindPoints = new LocateStructure[iSize];
  assert( FindPoints != 0);
  LookupSize = nx * ny ;
  Lookup = new long [ LookupSize ] ;
  assert( Lookup != 0 );
}

static int LocateInitSort( const void *a, const void *b )
{ return ( *( long *)a - *( long *)b ); }

void GridXType::Sort()
{
 static long i, ix, iy, ixiy, ixiyold, ixiyLook;
 qsort( FindPoints, np, sizeof(FindPoints[0]), LocateInitSort);

 // Build lookup table. -1 means no data points attached to that node.
 //char string[30];
 //sprintf( string, "%i  %i", FindPoints[0].intersection, FindPoints[1].intersection );
 //MessageBeep(MB_ICONQUESTION);
 //MessageBox( NULL, string, "QuikGrid",
 //		  MB_ICONINFORMATION|MB_OK|MB_TASKMODAL);
 for( i = 0; i < LookupSize; i++ ) { Lookup[i] = -1; }

 ixiyold = -1;
 for( i = 0; i < np; i++ )
 {
	 ixiy = FindPoints[i].intersection;
	 if( ixiy == ixiyold ) continue;
	 ixiyold = ixiy;
	 ix = ixiy/Shift;
	 iy = ixiy%Shift;
	 ixiyLook = iy*nx + ix;
	 assert( ixiyLook >= 0 && ixiyLook < LookupSize );
	 Lookup[ ixiyLook] = i ;
 }

}
//****************************************************************
long GridXType::Search( const int i, const int j, const int n )
{ // returns location of n th point belonging to intersection i,j

 // Here is the new code
 static long ixiy, newij, result;
 ixiy = (long)j*nx + (long)i;
 assert( ixiy >= 0 && ixiy < LookupSize );
 result = Lookup[ixiy];

 if( result == -1 ) return -1;
 //assert( result >= 0 && result < np ) ;
 newij = (long)i*(long)Shift+(long)j;
 //assert( newij == FindPoints[result].intersection );
 
 result += n;
 if( result >= np )  return -1;

 if( newij == FindPoints[result].intersection)
			return FindPoints[result].DataLocation;
 return -1;

 // Old code follows
 /*
 static long ij, test, top, bot;
 ij = (long)i*(long)Shift+(long)j;
 test = PreviousSearch;
 if( ij != PreviousSearch )
 {
   PreviousSearch = -1; 
	top = np;
	bot = -1;
   while( (top-bot) > 1 )
   {
     test = (top-bot)/2 + bot;
     if( ij == FindPoints[test].intersection ) break; 
     if( ij >  FindPoints[test].intersection ) bot = test;
     else                                      top = test;
   }

   if( ij != FindPoints[test].intersection ) return -1;

   while( test > 0 ) 
    { test--; 
      if( ij != FindPoints[test].intersection) { test++; break;}
    }
   PreviousSearch = test; 
  }
 test += n;
 if( ij == FindPoints[test].intersection)
			return FindPoints[test].DataLocation;
 return -1;
 //  */
}