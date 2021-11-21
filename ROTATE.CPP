//***********************************************************
//               R o t a t e
//                 Copyright (c) 1993 - 2001 by John Coulthard
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
//***********************************************************

#include <math.h>
#include <windows.h>
#include "assert.h"

static float xc1, xc2, xc3, yc1, yc2, yc3, zc1, zc2, zc3,
             xViewPoint, yViewPoint, zViewPoint; 

extern float Turn, Tilt, Aspect;


//************************************************************
//            R o t a t e   I n i t i a l i z e
//************************************************************
void RotateInitialize( float Turn, float Tilt, float Aspect)
{
 static const float RadianConversionFactor = .017453 ;

 float TurnTemp = Turn;
 if( TurnTemp < 0 ) TurnTemp += 360;
 float TurnRadians =  RadianConversionFactor*TurnTemp;
 float TiltRadians =  RadianConversionFactor*(90.-Tilt);
 float AspectRadians = RadianConversionFactor*Aspect;

 float SinTurn =   sin( TurnRadians);
 float CosTurn =   cos( TurnRadians);
 float SinTilt =   sin( TiltRadians);
 float CosTilt =   cos( TiltRadians);
 float SinAspect = sin( AspectRadians);
 float CosAspect = cos( AspectRadians);

 // Calculate the constants.

 xc1 = CosTurn * CosTilt;
 xc2 = SinTurn * CosTilt;
 xc3 = SinTilt;
 yc1 = SinTurn*CosAspect + CosTurn*SinTilt*SinAspect ;
 yc2 = CosTurn*CosAspect - SinTurn*SinTilt*SinAspect;
 yc3 = CosTilt*SinAspect;
 zc1 = SinTurn*SinAspect - CosTurn*SinTilt*CosAspect;
 zc2 = CosTurn*SinAspect + SinTurn*SinTilt*CosAspect;
 zc3 = CosTilt*CosAspect;
}

//************************************************************
//            R o t a t e   I n i t i a l i z e
//************************************************************
void RotateInitialize( HWND hwnd )
{
 if( Tilt < 0 ) Tilt = 0;
 if( Tilt > 90 ) Tilt = 90.;
 SetScrollPos( hwnd, SB_VERT, 90-Tilt, TRUE );
 if( Turn < -180) Turn = 180;
 if( Turn > 180 ) Turn = - 180;
 SetScrollPos( hwnd, SB_HORZ, Turn+180, TRUE );
 RotateInitialize( Turn, Tilt, Aspect);
}

//************************************************************
//                    R o t a t e
//************************************************************
void Rotate( float &x, float &y, float &z )
{
  static float xTemp, yTemp, zTemp; 
  xTemp = x;
  yTemp = y;
  zTemp = z;
  x = xTemp*xc1 - yTemp*xc2 + zTemp*xc3;
  y = xTemp*yc1 + yTemp*yc2 - zTemp*yc3;
  z = xTemp*zc1 + yTemp*zc2 + zTemp*zc3;  // Do I use this anywhere????? (wasted calculation?)  
}

//*************************************************************
//                 P r o j e c t
//*************************************************************
void Project( float &x, float &y, float z)
{
 static float xOrig, yOrig, zDistance, xDistance,
	      DistSquared, a, b, xDistNew, DistSquared2;
 xOrig = x;
 yOrig = y;
 zDistance = zViewPoint - z ;
 xDistance = xOrig - xViewPoint;
 x = xOrig + z*(xOrig-xViewPoint)/(zDistance);
 DistSquared = xDistance*xDistance+zDistance*zDistance;
 assert( DistSquared > 0.0 );
 a = sqrt(DistSquared);
 xDistNew = x - xViewPoint;
 DistSquared2 = xDistNew*xDistNew + zViewPoint*zViewPoint ;
 assert( DistSquared2 > 0.0); 
 b = sqrt(DistSquared2 );
 y = yOrig - (b - a)/a *(yViewPoint-yOrig);
 }
   
//**************************************************************
//           P r o j e c t   I n i t i a l i z e
//**************************************************************
void ProjectInitialize( float x, float y, float z)
{
  xViewPoint = x;
  yViewPoint = y;
  zViewPoint = z;
} 