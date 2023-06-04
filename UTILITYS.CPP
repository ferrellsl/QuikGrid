//****************************************************************
//                    U t i l i t y s
//
//                       Copyright (c) 1993 - 2001 by John Coulthard
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
// Oct. /96: Reduce required registration name length to 7.
// Jan. 19/99: Add function LatLonToDegMin
// Jan. 29/99: Add functions FormatCoord. Change to use sprinf to format.
// Feb. 1/99: Add functions FormatLat and FormatLon.
//            Support output and input of NS/EW
// Feb. 11/99: Change to accept numeric unlock code.
// Feb. 26/99: Add function StringIsBlank( String )
// Mar. 22/99: Add funtion AtPrecision
// Apr. 20/99: Set MessageBox to use MB_SETFOREGROUND
//             Set MessageBox to use MB_SYSTEMMODAL istead of MB_TASKMODAL.
// Dec. 15/99: Implement GetPair... functions.
// Mar. 3/00: Fixed bug in StringIsBlank - only worked to string 128 long.
// Mar 24/01: More flavours of NotifyUser.
//*****************************************************************
#include <windows.h>
#include <stdlib.h> 
#include <iostream>
#include <strstream>
#include <iomanip>
#include <string.h>
#include <commdlg.h>
#include <direct.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#pragma hdrstop
#include "assert.h"
#include "surfgrid.h"
#include "scatdata.h"
#include "xygrid.h"
#include "quikgrid.h"
#include "rc.h"
#include "utilitys.h"
using namespace std;
extern HWND WindProcHwnd; 
extern int NicePrecision;
extern int QuikGridChangedImage;
static char szBuffer[256],
            szLoadString[256],
            szPrint[256];

//****************************************************************
//                    F l i p    O p t i o n
//****************************************************************
void FlipOption( HMENU hMenu, int &option, UINT MenuId )
{ if ( option )
  { CheckMenuItem( hMenu, MenuId, MF_UNCHECKED);
    option = FALSE;
  }
  else
  { CheckMenuItem( hMenu, MenuId, MF_CHECKED);
    option = TRUE;
  }
}
//****************************************************************
//           P i c t u r e   C h a n g e d
//****************************************************************
//  Comes here if QuikGrid has changed the image
//  (As opposed to Windows doing it due to resize etc.).
//  *ALL* changes caused by QuikGrid should be routed here.
void PictureChanged( HWND &hwnd )
{
      InvalidateRect( hwnd, NULL, FALSE);
      QuikGridChangedImage = TRUE; 
}

//**********************************************************
//               At Precision
//**********************************************************
// Returns a float number with the given precision.
float AtPrecision( float value, int precision )
{
  static char String[30];
  sprintf( String, "%.*g", precision, value );  // Write it out at precision
  return atof( String );
}

//**********************************************************
//               S t r i n g I s B l a n k
//**********************************************************
bool StringIsBlank( const char String[] )
{
  static int StringLength;
  static int i;
  StringLength = strlen( String );
  if( StringLength < 1 ) return true; // Null string is blank.
  for( i = 0; i < StringLength; i++ )
     if( !isspace( String[i] ) ) return false;
  return true;
}

//**********************************************************
//               S t r i n g I s I n t e g e r
//**********************************************************
// Check to see if string is composed of the digits 0 - 9.
bool StringIsInteger( const char String[] )
{
  static int StringLength, i;
  StringLength = strlen( String );
  if( StringLength < 1 ) return false; // Null string is bad.
  for( i = 0; i < StringLength; i++ )
     if( String[i] < '0' || String[i] > '9' ) return false;
  return true;
}

//**********************************************************
//               L a t  L o n  T o  D e g  M i n
//**********************************************************
// Convert a double precision number in format ddd.fffff where
// ddd = degrees and ffff is fractions of a degree.
// return integer degrees and floating point minutes
// and fractions of a minute. (Makes it easy to display).

void LatLonToDegMin( const double x, int &degrees, double &minutes )
{
  degrees = x;
  minutes = std::fabs(x)-std::fabs((double)degrees)*60.;
//  if( degrees > 0.0 ) minutes = (x-degrees)*60. +.5;
//  else                minutes = (x-degrees)*60 + .5;
}
//*********************************************************
//             F o r m a t L o n
//*********************************************************
char *FormatLon( const double &value, int Pretty )
{
  static char szText[50], EW;
  static int degrees;
  static double minutes;
  LatLonToDegMin( value, degrees, minutes);
  EW = 'E';
  if( degrees < 0.0 ) { EW = 'W'; degrees = -degrees; }
  if( Pretty ) sprintf( szText, "%c%i°%g'", EW, degrees, minutes );
  else         sprintf( szText, "%c%i%g", EW, degrees, minutes );
  return szText;
}
//*********************************************************
//             F o r m a t L a t
//*********************************************************
char *FormatLat( const double &value, int Pretty  )
{
  static char szText[50], NS;
  static int degrees;
  static double minutes;
  LatLonToDegMin( value, degrees, minutes);
  NS = 'N';
  if( degrees < 0.0 ) { NS = 'S'; degrees = -degrees; }
  if( Pretty ) sprintf( szText, "%c%i°%g'", NS, degrees, minutes );
  else         sprintf( szText, "%c%i%g", NS, degrees, minutes );
  return szText;
}
//*********************************************************
//             F o r m a t C o o r d
//*********************************************************
// Routine to format x or y coorindate. Result is placed
// in area szResult of maximum length .
void FormatCoord( char szResult[], const int &length, const double &value )
{
 static int degrees;
 static double minutes;
 if( LatLonData )
  {
    LatLonToDegMin( value, degrees, minutes);
	 sprintf( szResult, "%i°%g'", degrees, minutes );
  }
 else
  {
	 sprintf( szResult, "%.*g", NicePrecision, value );
  }
}

//*********************************************************
//             F o r m a t C o o r d
//*********************************************************
//  Same as above but returns pointer to character string.
//
char *FormatCoord( const double &value )
{
  static char szText[50];
  FormatCoord( szText, sizeof( szText), value ) ;
  return szText;
}
//*********************************************************
//            F o r m a t X Y
//
// Jan. 18/04: NW EW reversed fixed.
//*********************************************************
// Routine to format x and y pair ah so (......,......)
void FormatXY( char szResult[], const int length, const double x, const double y )
{
 static char NS, EW;
 static int xdegrees, ydegrees;
 static double xminutes, yminutes;

 if( LatLonData )
 {
   LatLonToDegMin( x, xdegrees, xminutes);
   LatLonToDegMin( y, ydegrees, yminutes);
   NS = 'N';
   EW = 'E';
   if( xdegrees < 0.0 ) { EW = 'W'; xdegrees = -xdegrees; }
   if( ydegrees < 0.0 ) { NS = 'S'; ydegrees = -ydegrees; }
   sprintf( szResult, "(%c%i°%g',%c%i°%g')",
             //  EW, ydegrees, yminutes, NS, xdegrees, xminutes);
             NS, ydegrees, yminutes, EW, xdegrees, xminutes);
 }
 else
   sprintf( szResult, "(%.*g,%.*g)", NicePrecision, x, NicePrecision, y );
}

//*********************************************************
//           L a t  T o  D o u b l e
//*********************************************************
// Convert string containing a lat <or> lon in the
// format ddmm.mmmm to a double precision number dd.ddddd.
// Method: Scan for ".", back up two and scoop to end
// If no "." back up two from very end.
// as float degrees. Back to beginning and scoop 2 or 3
// digit integer with optional sign.
// returns success or failure, on failure result = 0
//
// Yet to add: Allow NS for +- and EW for -+ (LonToDouble reverses the sign). 

int LatToDouble( double &result, const char szInput[] )
{
 static char string[100], *scanposition, *endptr, NSEW, last;
 static long degrees, i, stringlength;
 static double minutes;
 result = 0.0;
 NSEW = ' ';

 // Move it inside so I can alter it.
 strncpy( string, szInput, sizeof( string )-1 );
 string[sizeof(string)-1] = 0; // null terminate it just in case
 stringlength = strlen( string );
 last = string[stringlength-1];

 if( last == 'N' || last == 'E' ||
     last == 'S' || last == 'W' )
      { NSEW = last; string[last] = NULL; }
 // scan for the "." or a ' or end of the line.
 scanposition = strchr( string, '.' );
 if( scanposition == NULL ) scanposition = strchr( string, '\'' ); 
 if( scanposition == NULL ) return 0; // no period, no quote - error.
 scanposition -= 2; // back up two characters.
 if(*scanposition=='^'||*scanposition=='°') scanposition ++; 
 minutes = strtod(scanposition, &endptr);
 if( fabs(minutes) > 60. ) return 0;
 if( !( *endptr == NULL||isspace(*endptr)||*endptr=='\'') )
                           return 0;
 *scanposition = NULL; // terminate string at start of minutes.
 // Check for NS or EW - change sign accordingly.
 scanposition = &string[0];
 stringlength = strlen( string );
 for( i = 0; i < stringlength; i++ )
  {
    if( isspace( string[i]) ) continue;
    scanposition = &string[i];
    break;
  }
 if( *scanposition == 'N' || *scanposition == 'E' ||
     *scanposition == 'S' || *scanposition == 'W' )
       { NSEW = *scanposition; scanposition++ ; }

 degrees = strtol( scanposition, &endptr, 10 ); // and scoop of the degrees.
 if( std::fabs((double)degrees) > 360 ) return 0;
 if( !( (*endptr == NULL)||isspace(*endptr)||(*endptr=='°')||(*endptr=='^') ) )
                           return 0;
 result = degrees;
 result += minutes/60. ;
 if( degrees < 0 ) result = -result;
 //
 // The conversion for lat and lon are identical except for the sign of
 // lon. Postive lon is westerly and is carried as negative in the program.
 // All LonToDouble does is call this routine and reverse the sign.
 // So here we set E as negative.
 // Of course that means that Nddmmm.mm is a valid longtitude which
 // is wrong - will clean this up later after I know all this stuff is robust. 
 //
 if( NSEW == 'S' || NSEW == 'E' ) result = -result;
 return 1;
}
//****************************************************************
//        M i n  P a r t
//****************************************************************
float MinPart( const double x )
{
  return (fabs(x)-std::fabs((double)x))*60.;
}
//*********************************************************
//           L o n   T o   D o u b l e
//*********************************************************
// Latitude is positive west so reverse the sign.
// Otherwise handle as for longtitude.
int LonToDouble( double &result, const char szInput[] )
{
  static int returncode;
  returncode = LatToDouble( result, szInput );
  result = -result;
  return returncode;
}
//**********************************************************
//               C o n v e r t   D e c   D e g
//**********************************************************
// Convert a double precision number in format ddd.fffff where
// ddd = degrees and ffff is fractions of a degree.
// format desired is ddd mm ssss
// ddd = degrees; mm = minutes; ssss = ss.ss seconds.

void ConvertDecDeg( double x, int &degrees, int &minutes, int &seconds )
{
  degrees = x;
  x -= degrees;
  x = x*60.;
  minutes = x;
  x -= minutes;
  x = x*60. ;
  if( x > 0.0 ) seconds = x*100.+ .5;
  else          seconds = x*100. -.5;
}
//**********************************************************
//               C o n v e r t   D e c   D e g
//**********************************************************
// Same as above but combine degminsec so one big integer. 
long ConvertDecDeg( double x )
{
 int Deg, Min, Sec;
 ConvertDecDeg( x, Deg, Min, Sec );
 return long(Deg)*1000000 + long(Min)*10000 + Sec;
}
//**********************************************************
//               C o n v e r t   D e g   D e c
//**********************************************************
// convert a *big* integer representing lat, long from
// degree min seconds to a double precision float number
// in degrees and fractions of a degree. 
double ConvertDegDec( long x )
{
  long seconds =  x%10000;
  if( abs(seconds)>6000) return 999.;
  long minutes = (x%1000000)/10000;
  if( abs(minutes)>60) return 999.;
  long degrees =           x/1000000;
  if( abs(degrees)>720) return 999.;
  double ddegrees = degrees;
  double dminutes = (double) minutes/60.;
  double dseconds = (double) seconds*.01/3600.;
  return ddegrees+dminutes+dseconds;
}
//*********************************************************************
//              D o A s s e r t
//                  Adapted from: Windows++ by Paul Dilascia
//*********************************************************************
void DoAssert( char msg[], char file[], unsigned int line)
{
  ostrstream Buf;

  Buf << "at line " << line << " in file " << file
      << ": \"" << msg << "\"" <<
      "\n\n *** INTERNAL QUIKGRID CONSISTENCY ERROR. " <<
      "\n *** IT IS STRONGLY RECOMMENDED THAT YOU CLICK ON CANCEL." << ends;
       
  char *szBuf = Buf.str();
  MessageBeep(0);
  if( MessageBox( WindProcHwnd, szBuf, "Assertion Failure",
     MB_OKCANCEL|MB_ICONSTOP|MB_SYSTEMMODAL|MB_SETFOREGROUND)!=IDOK) exit(-1);
  delete szBuf;
}   
//*********************************************************************
//                  N o t i f y    U s e r
//*********************************************************************
void NotifyUser( char szNote[] )
{
 MessageBeep(MB_ICONQUESTION);
 //MessageBox( WindProcHwnd, szNote, "QuikGrid", MB_ICONINFORMATION|MB_OK|MB_TASKMODAL);
 MessageBox( WindProcHwnd, szNote, "QuikGrid", MB_ICONINFORMATION|MB_OK);
}
//*********************************************************************
//                  N o t i f y    U s e r   E r r o r (Loadstring version)
//*********************************************************************
void NotifyUserError( int MessageNumber )
{
  if( LoadString((HINSTANCE) hInst, IDS_MISSINGMESSAGENUMBER, szLoadString, 255 ) )
                                          sprintf( szPrint , szLoadString, MessageNumber );
  else sprintf( szPrint, "Missing message number %i should appear here (internal error).", MessageNumber);
  NotifyUser( szPrint );
 }
//*********************************************************************
//       G e t  L o a d   S t r i n g (internal service routine)
//*********************************************************************
static bool GetLoadString( int MessageNumber )
{
	if (LoadString((HINSTANCE)hInst, MessageNumber, szLoadString, 255)) return true;
 NotifyUserError( MessageNumber );
 return false;
}
//*********************************************************************
//                  N o t i f y    U s e r (LoadString version)
//*********************************************************************
void NotifyUser( int MessageNumber )
{
  if( !GetLoadString( MessageNumber ) ) return;
  NotifyUser( szLoadString );
}
void NotifyUser( int MessageNumber, char szString[] )
{
  if( !GetLoadString( MessageNumber ) ) return;
  sprintf( szPrint, szLoadString, szString );
  NotifyUser( szPrint );
}
void NotifyUser( int MessageNumber, char szString1[], char szString2[] )
{
  if( !GetLoadString( MessageNumber ) ) return;
  sprintf( szPrint, szLoadString, szString1, szString2 );
  NotifyUser( szPrint );
}
void NotifyUser( int MessageNumber, int i, char szString[] )
{
  if( !GetLoadString( MessageNumber ) ) return;
  sprintf( szPrint, szLoadString, i, szString );
  NotifyUser( szPrint );
}
void NotifyUser( int MessageNumber, char szString1[], int i, char szString2[])
{
  if( !GetLoadString( MessageNumber ) ) return;
  sprintf( szPrint, szLoadString, szString1, i, szString2 );
  NotifyUser( szPrint );
}
void NotifyUser( int MessageNumber, char szString1[], char szString2[],int i )
{
  if( !GetLoadString( MessageNumber ) ) return;
  sprintf( szPrint, szLoadString, szString1, szString2, i );
  NotifyUser( szPrint );
}
void NotifyUser( int MessageNumber, int i, char szString1[], char szString2[] )
{
  if( !GetLoadString( MessageNumber ) ) return;
  sprintf( szPrint, szLoadString, i, szString1, szString2 );
  NotifyUser( szPrint );
}
void NotifyUser( int MessageNumber, char szString1[], int i1, int i2, char szString2[], char szString3[] )
{
  if( !GetLoadString( MessageNumber ) ) return;
  sprintf( szPrint, szLoadString, szString1, i1, i2, szString2, szString3 );
  NotifyUser( szPrint );
}
//*********************************************************************
//                  N o t i f y    U s e r  (string supplied versions)
//*********************************************************************
void NotifyUser( char szNote[], const int arg1 )
{
  sprintf( szBuffer, szNote, arg1);
  NotifyUser( szBuffer );
}
void NotifyUser( char szNote[], const int arg1, const int arg2 )
{
  sprintf( szBuffer, szNote, arg1, arg2);
  NotifyUser( szBuffer );
}
void NotifyUser( char szNote[], const int arg1, const int arg2, const int arg3 )
{
  sprintf( szBuffer, szNote, arg1, arg2, arg3);
  NotifyUser( szBuffer );
}
void NotifyUser( char szNote[], const float arg1 )
{
  sprintf( szBuffer, szNote, arg1);
  NotifyUser( szBuffer );
}
void NotifyUser( char szNote[], const float arg1, const float arg2 )
{
  sprintf( szBuffer, szNote, arg1, arg2);
  NotifyUser( szBuffer );
}
void NotifyUser( char szNote[], const float arg1, const float arg2, const float arg3 )
{
  sprintf( szBuffer, szNote, arg1, arg2, arg3);
  NotifyUser( szBuffer );
}
void NotifyUser( char szNote[], const float arg1, const float arg2, const float arg3, const float arg4 )
{
  sprintf( szBuffer, szNote, arg1, arg2, arg3, arg4);
  NotifyUser( szBuffer );
}
void NotifyUser( char szNote[], char szString[] )
{
  sprintf( szBuffer, szNote, szString );
  NotifyUser( szBuffer );
}

//*********************************************************************
//                  G e t   R e a d  F i l e
//*********************************************************************
int GetReadFile( char szFile[], HWND &hwnd, char* szOpenTitle )
{
 static OPENFILENAME ofn;
 UINT i, cbString;
 char chReplace;  // string separator for szFilter
 char szFilter[256];
 //HFILE hf;

 szFile[0] = '\0';

 cbString = LoadString((HINSTANCE)hInst, IDS_FILTERSTRING,
		       szFilter, sizeof(szFilter) );

 assert( cbString != 0 );

 chReplace = szFilter[cbString-1]; // retrieve wildcard.

 for( i = 0; szFilter[i] != '\0'; i++ )
    { if( szFilter[i] == chReplace)
         szFilter[i] = '\0'; 
    }

 memset( &ofn, 0, sizeof(OPENFILENAME) ); // zero structure members

 ofn.lStructSize = sizeof(OPENFILENAME);
 ofn.hwndOwner = hwnd;  // =NULL if no owner required. 
 ofn.lpstrFilter = szFilter;
 ofn.nFilterIndex = 0;
 ofn.lpstrFile = szFile;
 ofn.nMaxFile = 255;        //sizeof(szFile);
 ofn.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;
 ofn.lpstrTitle = szOpenTitle;

 if( GetOpenFileName( &ofn ) ) return 1;
 return 0;  
}

//*********************************************************************
//                  G e t   R e a d  F i l e   (with file filter option)
//*********************************************************************
int GetReadFile( char szFile[], HWND &hwnd, char* szOpenTitle, char szFilter[] )
{
 static OPENFILENAME ofn;

 memset( &ofn, 0, sizeof(OPENFILENAME) ); // zero structure members

 ofn.lStructSize = sizeof(OPENFILENAME);
 ofn.hwndOwner = hwnd;  // =NULL if no owner required. 
 ofn.lpstrFilter = szFilter;
 ofn.nFilterIndex = 0;
 ofn.lpstrFile = szFile;
 ofn.nMaxFile = 255;        //sizeof(szFile);
 ofn.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;
 ofn.lpstrTitle = szOpenTitle;

 if( GetOpenFileName( &ofn ) ) return 1;
 return 0;  
}

//*********************************************************************
//            G e t   F i l e   N a m e  M u l t i p l e
//*********************************************************************
// retrieve more than one file name in one go.
int GetReadFileMultiple( char szFile[], HWND &hwnd, char* szOpenTitle, char szFilter[] )
{
 static OPENFILENAME ofn;

 memset( &ofn, 0, sizeof(OPENFILENAME) ); // zero structure members

 ofn.lStructSize = sizeof(OPENFILENAME);
 ofn.hwndOwner = hwnd;  // =NULL if no owner required. 
 ofn.lpstrFilter = szFilter;
 ofn.nFilterIndex = 0;
 ofn.lpstrFile = szFile;
 ofn.nMaxFile = 5000;        //sizeof(szFile);
 ofn.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST|OFN_EXPLORER|OFN_ALLOWMULTISELECT;
 ofn.lpstrTitle = szOpenTitle;

 if( GetOpenFileName( &ofn ) ) return 1;
 return 0;  
}
//*********************************************************************
//                  G e t  S a v e F i l e 
//*********************************************************************
int GetSaveFile( char szFile[], HWND &hwnd, char* szOpenTitle )
{
 static OPENFILENAME ofn;
 UINT i, cbString;
 char chReplace;  // string separator for szFilter
 char szFilter[256];
 //HFILE hf;
   
 szFile[0] = '\0';

 cbString = LoadString((HINSTANCE)hInst, IDS_FILTERSTRING,
                       szFilter, sizeof(szFilter) );

 assert( cbString != 0 );

 chReplace = szFilter[cbString-1]; // retrieve wildcard.

 for( i = 0; szFilter[i] != '\0'; i++ )
    { if( szFilter[i] == chReplace)
         szFilter[i] = '\0'; 
    }

 memset( &ofn, 0, sizeof(OPENFILENAME) ); // zero structure members

 ofn.lStructSize = sizeof(OPENFILENAME);
 ofn.hwndOwner = hwnd;  // =NULL if no owner required. 
 ofn.lpstrFilter = szFilter;
 ofn.nFilterIndex = 1;
 ofn.lpstrFile = szFile;
 ofn.nMaxFile = 255;        //sizeof(szFile);
 ofn.Flags = OFN_PATHMUSTEXIST|OFN_OVERWRITEPROMPT;
 ofn.lpstrTitle = szOpenTitle;

 if( GetSaveFileName( &ofn ) ) return 1;
 return 0;
}

//*********************************************************************
//                  G e t  S a v e F i l e   (with filter option).
//*********************************************************************
int GetSaveFile( char szFile[], HWND &hwnd, char* szOpenTitle, char szFilter[], char szExtension[] )
{
 static OPENFILENAME ofn;
 UINT i, cbString;

 szFile[0] = '\0';

 memset( &ofn, 0, sizeof(OPENFILENAME) ); // zero structure members

 ofn.lStructSize = sizeof(OPENFILENAME);
 ofn.hwndOwner = hwnd;  // =NULL if no owner required. 
 ofn.lpstrFilter = szFilter;
 ofn.nFilterIndex = 1;
 ofn.lpstrFile = szFile;
 ofn.nMaxFile = 255;        //sizeof(szFile);
 ofn.Flags = OFN_PATHMUSTEXIST|OFN_OVERWRITEPROMPT;
 ofn.lpstrTitle = szOpenTitle;
 ofn.lpstrDefExt = szExtension;
 if( GetSaveFileName( &ofn ) ) return 1;
 return 0;
}

//******************************************************
//   Set Wait Cursor and Restore cursor.
//******************************************************
static HCURSOR hCursor;
void SetWaitCursor()
{
 
 hCursor = SetCursor( LoadCursor( NULL, IDC_WAIT ) );
 ShowCursor( TRUE ); 
}
//********************************************************
void RestoreCursor()
{
 ShowCursor( FALSE );
 SetCursor( hCursor);
}
//******************************************************
//  Format name of ini file
//******************************************************
char *INIfileName()
{
  static char szINIfile[1024];
  szINIfile[0] = '\0';
  char* szUserProfile=0;
  size_t sz = 0;
  _dupenv_s(&szUserProfile, &sz, "USERPROFILE");
  strcpy(szINIfile, szUserProfile);
  strcat(szINIfile, "\\QUIKGRID.INI");
  free(szUserProfile);
  return szINIfile;
}

//********************************************************
//        G e t  P a i r  I n t
//********************************************************
// Routine returns result from " string = value".
int GetPairInt( char szString[], char szSearch[], int &Fail )
{
  char *ptr;
  ptr = strstr( szString, szSearch );
  if( ptr == NULL ) { Fail = TRUE; return 0; }
  ptr = strstr( ptr, "=" );
  if( ptr == NULL ) { Fail = TRUE; return 0; }
  ptr++;
  return atoi( ptr );
}
//********************************************************
//        G e t  P a i r  F l o a t
//********************************************************
// Routine returns result from " string = value".
double GetPairFloat( char szString[], char szSearch[], int &Fail )
{
  char *ptr, *endptr;
  double result;
  ptr = strstr( szString, szSearch );
  if( ptr == NULL ) { Fail = TRUE; return 0; }
  ptr = strstr( ptr, "=" );
  if( ptr == NULL ) { Fail = TRUE; return 0; }
  ptr++;
  result = strtod(ptr, &endptr);
  if( !( *endptr == NULL||isspace(*endptr)||*endptr=='\n') )
                  {Fail = TRUE; return 0; }
  return result;
}

//********************************************************
//        T a b   E d i t
//********************************************************
// Routine to expand tabs to blanks
int TabEdit( char String[] , int MaxLength )
{
	static char CopyString[300], Spaces[10] = "         ";
	static int Length, Source, Target, NumberOfSpaces;

	Length = strlen( String );
	Target = 0;
	for( Source=0; Source<Length; Source++ )
	{
	  if( String[Source] == '\t' )
	  {
	  // Copy a series of padding blanks.  How many will be something mod 8;
	  NumberOfSpaces = 8-(Target%8);
	  if( Target >= (MaxLength-NumberOfSpaces-1) )
                          { NotifyUser( IDS_TABEXPANDERROR );  return FALSE;}
	  strncpy( &CopyString[Target], Spaces, NumberOfSpaces );
	  Target += NumberOfSpaces;
	  }
	  else
	  {
		 CopyString[Target] = String[Source];
		 Target++;
		 if( Target >= (MaxLength-1) )
		                    { NotifyUser( IDS_TABEXPANDERROR );  return FALSE;}
	  }
	}
	CopyString[Target] = NULL;
	strcpy( String, CopyString);
	return TRUE;
}
