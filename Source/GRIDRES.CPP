//**************************************************************
//    G R I D   R E S O L U T I O N    R O U T I N E S
//
//                  Copyright (c) 1996 - 2000 by John Coulthard
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
// Nov. 5/98: Created
// May 13/99: Improved accuracy of Automatic grid Resolution.
// May 14/99: Handle zooming differently from increase/decrease.
// Feb 10/00: Make it handle Sgrid and Dgrid at the same time.
// Jan 8/02: Removed "DrawMenuBar" from GridResMenus - not needed. 
//**************************************************************
#include <windows.h>
#include "surfgrid.h"
#include "xygrid.h"
#include "scatdata.h" 
#include "quikgrid.h" 
#include "paintcon.h"
#include "rotate.h"
#include "rc.h"
#include "assert.h"
#include "condraw.h"
#include "utilitys.h"

#define max(a,b) (((a)>(b))?(a):(b))

extern SurfaceGrid Zgrid, Sgrid, Dgrid;
extern float ZoomRatio;

static int BestLines = 125,
           LeastLines = 10;
static float FloatBestIncr = 1.0 ;
static GridViewType CurrentView;

//
//**************************************************************
//   A p p l y   V i e w
//             Internal service routine.
//**************************************************************
static void ApplyView()
{
 Zgrid.view( CurrentView);
 if( Sgrid.xsize() > 2 ) Sgrid.view( CurrentView );
 if( Dgrid.xsize() > 2 ) Dgrid.view( CurrentView );
}
//**************************************************************
//   G r i d   R e s e t
//             Internal service routine.
//**************************************************************
static void GridReset(HWND &hwnd)
{
   PaintContourReset();
   RotateReset(); 
   PictureChanged( hwnd );
}
//**************************************************************
//    G r i d   R e s   M e n u s
//**************************************************************
void GridResMenus( HWND &hwnd, HMENU &hMenu)
{
   static int size, Incr;
   size = min( Zgrid.xsize(), Zgrid.ysize() );
   CurrentView = Zgrid.view();
   Incr = max( CurrentView.xIncrement, CurrentView.yIncrement);

   if( Incr == 1 )
     {
       EnableMenuItem( hMenu, IDM_GRIDRESFULL, MF_GRAYED);
       EnableMenuItem( hMenu, IDM_GRIDRESINCREASE, MF_GRAYED);
       EnableMenuItem( hMenu, IDM_GRIDRESDECREASE, MF_ENABLED);
       if( size/Incr <= LeastLines )
                   EnableMenuItem( hMenu, IDM_GRIDRESDECREASE, MF_GRAYED);
     }
   else // Incr > 1
     {
        EnableMenuItem( hMenu, IDM_GRIDRESFULL, MF_ENABLED);
        EnableMenuItem( hMenu, IDM_GRIDRESINCREASE, MF_ENABLED);
        EnableMenuItem( hMenu, IDM_GRIDRESDECREASE, MF_ENABLED);
        if( size/Incr <= LeastLines )
               EnableMenuItem( hMenu, IDM_GRIDRESDECREASE, MF_GRAYED);
     }
  //DrawMenuBar( hwnd ); // Don't need to do this here - will be picked up next time. 
  return;
}
//**************************************************************
//    G r i d   R e s   O p t i m u m
//**************************************************************
void GridResOptimum(HWND &hwnd, HMENU &hMenu)
{
   static int size, BestIncr;
   size = min( Zgrid.xsize(), Zgrid.ysize() );
   BestIncr = size / BestLines;
   if( BestIncr < 1 ) BestIncr = 1;

   CurrentView = Zgrid.view();
   CurrentView.xIncrement = BestIncr;
   CurrentView.yIncrement = BestIncr;
   ApplyView();
   GridResMenus( hwnd, hMenu);
   GridReset( hwnd );
   FloatBestIncr = BestIncr;
   return;
}
//**************************************************************
//    G r i d   R e s   D e c r e a s e
//**************************************************************
void GridResDecrease(HWND &hwnd, HMENU &hMenu, int Zooming )
{
   static float NewIncr;
   CurrentView = Zgrid.view();
   if( min( Zgrid.xsize(), Zgrid.ysize() )/
       max( CurrentView.xIncrement, CurrentView.yIncrement) <= LeastLines )
        { GridResMenus( hwnd, hMenu); return;}
   //CurrentView.xIncrement += 1;
   //CurrentView.yIncrement += 1;
   if( Zooming )
   {
     NewIncr = FloatBestIncr*ZoomRatio;
     assert( FloatBestIncr >= 1.0 );
     CurrentView.yIncrement = CurrentView.xIncrement = NewIncr;
   }
   else
     NewIncr = CurrentView.xIncrement = CurrentView.yIncrement += 1;

   if( CurrentView.xStart + 3*CurrentView.xIncrement >= Zgrid.xsize() )
       { GridResMenus( hwnd, hMenu); return;}
   if( CurrentView.yStart + 3*CurrentView.yIncrement >= Zgrid.ysize() )
       { GridResMenus( hwnd, hMenu); return;}

   FloatBestIncr = NewIncr;
   ApplyView();
   GridResMenus( hwnd, hMenu);
   GridReset( hwnd );
}
//**************************************************************
//     G r i d   R e s   I n c r e a s e 
//**************************************************************
void GridResIncrease(HWND &hwnd, HMENU &hMenu, int Zooming )
{
   CurrentView = Zgrid.view();
   if( CurrentView.xIncrement+CurrentView.yIncrement == 2 )
     { GridResMenus( hwnd, hMenu); return; }
   //if( CurrentView.xIncrement > 1 ) CurrentView.xIncrement -= 1;
   //if( CurrentView.yIncrement > 1 ) CurrentView.yIncrement -= 1;
   if( Zooming )
   {
      FloatBestIncr = FloatBestIncr/ZoomRatio;
      if( FloatBestIncr < 1.0 ) FloatBestIncr = 1.0; 
   }
   else
      FloatBestIncr = CurrentView.yIncrement = CurrentView.xIncrement -= 1;

   if( CurrentView.xIncrement > 1 )
       CurrentView.yIncrement = CurrentView.xIncrement = FloatBestIncr;

   ApplyView();
   GridResMenus( hwnd, hMenu);
   GridReset( hwnd );
  }
//**************************************************************
//    G r i d   R e s   F u l l 
//**************************************************************
void GridResFull(HWND &hwnd, HMENU &hMenu )
{
   CurrentView = Zgrid.view();
   FloatBestIncr = 1.0;
   if( CurrentView.xIncrement + CurrentView.yIncrement == 2 )
       { GridResMenus( hwnd, hMenu); return; }
   CurrentView.xIncrement = CurrentView.yIncrement = 1;
   ApplyView();
   GridResMenus( hwnd, hMenu);
   GridReset( hwnd );
   return ;
}
//**************************************************************
//    G r i d   R e s  V a l u e  (may not be needed)... 
//**************************************************************
int GridResValue()
{
   CurrentView = Zgrid.view();
   return max( CurrentView.xIncrement, CurrentView.yIncrement); 
}