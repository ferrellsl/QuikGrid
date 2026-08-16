//******************************************************************
//                       S w i t c h e s

//  Contouring routine to get and set bit switches. The purpose is
//  flag any reference point in the grid defining the surface if
//  a contour has been traced past it.
//
//                   Copyright (c) 1993-2002 by W. John Coulthard
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
//********************************************************************

#include <iostream>
#pragma hdrstop
//#include "assert.h"

static unsigned char ON = 1;  
static long CharPosition, BitOffset;
static unsigned char *BitArray = 0;
static unsigned long SizeOfBitArray = 0;
static const long BitsPerChar = 8 ;
static long FirstDimension = 0;

//****************************************************************
//               S w i t c h    C l e a r
//****************************************************************
/*void SwitchClear( int i, int j)
{


 static long SizeNeeded;
 SizeNeeded = ((long)i*(long)j)/BitsPerChar+1;

 if( SizeNeeded <= 1 ) 
   { if (BitArray != 0 ) delete[] BitArray; 
     BitArray = 0; SizeOfBitArray = 0; 
     return; 
   } 

 FirstDimension = i;
 //assert(FirstDimension > 0 );

 if( SizeNeeded != SizeOfBitArray )
   { if( BitArray != 0 ) delete[] BitArray;
     BitArray = new unsigned char[SizeNeeded];
     //assert( BitArray != 0 );
     SizeOfBitArray = SizeNeeded;
   }

 memset( BitArray, 0, SizeOfBitArray);

}*/
//*********************************************************
//          S w i t c h   P o s n
//*********************************************************
static void SwitchPosn(int i, int j)
{
//  Calculate the location of the switch for SwitchSet and SwitchGet.

 static long BitPosition;
     BitPosition = (long)j*FirstDimension + (long)i;
     CharPosition = BitPosition/BitsPerChar;
     BitOffset = BitPosition - CharPosition*BitsPerChar;
     //assert( CharPosition <= SizeOfBitArray );
}
//*********************************************************
//            S w i t c h   S e t
//*********************************************************
/* int SwitchSet( int i, int j)
{


 SwitchPosn( i, j );

 if( *(BitArray+CharPosition)&(ON<<BitOffset) ) return 1;

 *(BitArray+CharPosition) |= (ON<<BitOffset);
 return 0;

}*/
//***********************************************************
//             S w i t c h   G e t
//***********************************************************
/*int SwitchGet( int i, int j)
{
// Returns the value of switch (i,j).
 SwitchPosn( i, j );
 if( *(BitArray+CharPosition)&(ON<<BitOffset) ) return 1;
 return 0;
}*/
