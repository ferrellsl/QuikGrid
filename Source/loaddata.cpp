//***********************************************************
//                  L o a d D a t a
//  Routines having to primarily to do with reading data
//                                                     
//                Copyright (c) 1993 - 2005 by John Coulthard
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
// Special note:
//         Borland C++ stream input of floating point numbers
//         will not work if it is running under Win95 which has the
//         original version of MS Plus installed. This is a MS bug
//         and there is a patch for it. Nonetheless, due to this bug,
//         QuikGrid completely avoids the use of C++ floating point
//         input (*sigh*). This explains the use of function "strtod"
//         for all floating point conversions, and perhaps what may look
//         like some strange constructions in the code.
//
// Sept. 19/96: Modified to use "strtod" to do floating point
//              conversions (Avoids need to use MS Service Pack 1).
// Oct. 14/96: Fixed failure in "Load Metric Data" to detect
//             last "z" terminated by an eof.
// Oct. 26/96: Changed DXF output to use "default" instead of
//             "scientific" notation - for ER MAPPER, perhaps
//             other packages as well. Also for contour layer names.
// Oct. 27/96: Deleted unused 3DDXFOUTPUT function.
// Dec. 12/96: Change Load grid to accept up to 1000x1000.
// Jan. 7/97: Remove OutputDXF code to own module DXFWRITE.CPP.
//            Remove Grid output code to own module GRDWRITE.CPP
//            Initial implementation of DXF input.
// Jan. 8/97: Clean up code - move common code to service routines.
//            Initial implementation of MEDS input.
//            LoadDXF deletes second of two identical points.
// Jan. 9/97: track separately: LinesRead, PointsRead, PointsKept.
//            Initial hooks for Load USGSDEMData installed.
// Feb. ?/97: Implement read DXF input. 
// Mar. 11/97: Correct bug in Read DXF input.
// Mar. 12/97: Change to handle blank lines in the DXF input correctly.
// Apr. 22/98: Implement FlipXY option.
// Jun. 27/98: Correct bug in Read DXF input (warning about more than
//             100 data points would be generated twice).
// Jan. 16/99: Comment out "Load File Meds Data"
// Jan. 17/99: Implement new style input (allow commas as a delimiter
//             enforces 1 xyz triplet/line and trailing comments)
// Jan 18/99:  SetSurface Title changed to show cursor position.
// Feb. 1/99:  Change to distinguis lat from lon on input.
// Feb. 3/99:  Change to handle lat/lon input better error checking.
// Feb. 9/99:  Change Urestricted registered data points to 10000.
// Feb. 10/99: Change metric input to use newstyle blank/comma input.
// Feb. 24/99: Check for null line as last line and ignore if so.
// Feb. 26/99: Change to use function StringIsBlank
//             (Lat/Lon, Metric, Report line short).
// Mar. 4/99: Add ReverseSignOfZ to StoreDataPoint routine.
// Apr. 20/99: Set MessageBox to use MB_SETFOREGROUND
//             Set MessageBox to use MB_SYSTEMMODAL istead of MB_TASKMODAL.
//             Changes to support cancelling of input.
//             Add function to ignore comments during input.
// May 14/99: Start coding internals for reading DEM files.
// Jun 1/99: Changes to support Grid Locking.
// Jun 10/99: Fixed possible problem re use of "comp" in read DEM data.
// Oct. 12/99: SetTemplate (for default template code) installed.
// Oct. 29/99: Ignore lines with errors or blank fields installed.
//             Title will show values from Speed and/or Direction grids
//                 if they exist. (2d mode only).
//             Some leading blanks allowed in fixed format input fields.
// Dec. 15/99: Implement load ER Mapper Grid.
// Dec. 30/99: Add TabEdit - Fixed columns works with tabs.
// Jan. 31/00: Add load outline code.
// Feb. 2/00: Add LockNormalization code. Change outline code to load to
//            the "outline" array.
// Feb. 3/00: Make title bar display speed and direction better.
// May 13/00: Outline input will handle Z coordinates.
//            Also reset itself better if second outline data set read.
// Jun. 11/00: Fixed bug in outline input which invalidated the grid.
//            (Triggered by outlines with negative z value).
// Sep. 11/00: Play around with scaling for dummy grid and data points
//            when outline alone is loaded.
// Sep. 17/00: Fixed error in fixed format input Lat and Lon not
//             interpreted correctly. Also maade "IgnoreLines in error"
//             work for lat lon input.
// Mar. 10/01: Add support for Colour maping by initing colours when
//             loading is complete. (InitializeColourMap...).
// Mar. 21-25/01: Implementation of LoadSubmetrixSXP Data.
// Mar. 24-25/01: Implement setting of "nice" colour map numbers.
// Apr. 18/01: Implement reading multiple files for sxp & xyz metric input.
// Oct. 3/01: Changed convert float to ignore lines if a blank field onlyl
//            iff IgnoreErrorLines is set.
// Nov. 11/01: Change messagebox to use NotifyUser instead.
// Dec. 15/01: DXF input - skip first point after polyline identified.
// Apr. 2005: Add support for NOAA formatted lat/lon numbers. 
//*******************************************************************
//
// Code candidates to move to external module(s).
//  SetPrecision,    ResetSurfaceTitle,     SetStartupState,
//  SetSurfaceTitle.

#include <windows.h> 
#include <iostream>
#include <iomanip>
#include <fstream>
#include <strstream>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "surfgrid.h"
#include "scatdata.h"
#include "xygrid.h"
#include "utilitys.h"
#include "rc.h"
#include "quikgrid.h"
#include "paintcon.h"
#include "loaddata.h"
#include "assert.h"
#include "grid.h"
#include "rotate.h"
#include "condraw.h"
#include "gridview.h"
using namespace std;
static ifstream inFile( " ", ios::in );

extern int GridGeneratedOK, GridLock;
extern int DisplayVolumeDifference;
extern float Turn, Tilt;
extern UINT PaintState;
extern SurfaceGrid Sgrid, Dgrid, Cgrid;
extern float CgridZadjust;

static 	int  IgnoreZero = 1;
static 	int		  KeepEveryNthPoint = 1;

static long LinesRead, 
				PointsRead,
				MaxInputData;

static double dxnormalize, dynormalize,
				  dx, dy, dz;

static char szdx[50], szdy[50], szdz[50],
            szNOAAdx[3][50], szNOAAdy[3][50],
				InputLine[200], Comment[200],
            szPrint[400], szLoadString[255];

static int   Xstart, Xlength, NewXstart, // Start posn and length of
             Ystart, Ylength, NewYstart, // szdx...comment in InputLine.
             Zstart, Zlength, NewZstart,
             Cstart, Clength, NewCstart;

static int FixedStarts = FALSE, // Indicates that Xstart... are unchanging.
			  IgnoreThisLine = FALSE; // Used with IgnoreErrorLines 

static char *endptr;

// Used by ConstructFilename et. all to handle multiple file names.
static int NameOffset, PathOffset;
static char szFileName[512];

//         ***************  G L O B A L S    *******************

ScatData OutLine;
long PointsKept;
int CancelLoadData;
int FlipXY = FALSE;
int NewInputStyle = TRUE;
int ReverseSignOfZ = FALSE;
int NicePrecision = 6;
int IgnoreComments = FALSE;
int DEMZeroUndefined = TRUE;
int IgnoreErrorLines = FALSE;
int LeadingBlanks = 3;
int LockNormalization = FALSE;
int NiceRatio = 4,
    NiceRatioMaxLines = 200;
int SubmetrixSelectZ = 1;
int InputFieldPosition[4]= {0, 1, 2, 3};
int NOAAInputFieldPosition[4] = {0, 3, 6, 7};

//**********************************************************
//             C o n s t r u c t  F i l e n a m e
//**********************************************************
// For handling multiple files... this routine is called
// for the first file in a multiple file selection. It
// copies over the path name and first file name and sets up
// the initial offsets to mark the end of the path and location
// of the next (if any) of the files in the set.
static void ConstructFilename( char FileName[] )
{
  strcpy( szFileName, FileName );    // copy over path and maybe the file name.
  NameOffset = strlen( FileName );

  if( FileName[ NameOffset+1 ] != NULL ) // This is a multiple file selection.
  {
    szFileName[NameOffset] = '\\';   // append separator to the path name.
    strcpy( &szFileName[NameOffset+1], &FileName[NameOffset+1] ); // and concatenate the file name.
    PathOffset = NameOffset + 1; // Save size of path in filename.
    strlen( &FileName[NameOffset+1])+1 ;
    NameOffset += strlen( &FileName[NameOffset+1]) + 2;  // Store offset of next filename in input.
  }
}

//**********************************************************
//             N o   M o r e   F i l e s 
//**********************************************************
// For handling multiple files... This routine returns
// true if there are no more files available in the multiple
// file selection. If there are it sets up the file name, closes
// the old file and opens the new.
static bool NoMoreFiles( char FileName[], bool Binary=false  )
{
  static char LocalFileName[200];
  if( FileName[NameOffset+1] == NULL ) return true;
  strcpy( LocalFileName, &FileName[NameOffset]);
  if( StringIsBlank(LocalFileName )) return true; 
  strcpy( &szFileName[PathOffset], &FileName[NameOffset]);

  NameOffset += strlen( &FileName[NameOffset])+1;
  inFile.close();
  if( Binary ) inFile.open( szFileName, ios::binary );
  else         inFile.open( szFileName, ios::in     );
  if( !inFile )
  { NotifyUser( IDS_OPENINPUTFILEFAIL, szFileName );
    return true; }
  LinesRead = 1; 
  return false;
}
//**********************************************************
//               S e t   P r e c i s i o n
//**********************************************************
// Sets a nice precision to use to display numbers.
// Some data sets carry much more than 6 digits of accuracy.
// This routine will set the global NicePrecision so it is between
// 5 and 16 depending on the input data.
static void SetPrecision()
{
  static double Temp, LogTemp;
  Temp = fabs(dxnormalize)/fabs(ScatterData.xMax()-ScatterData.xMin() );
  NicePrecision = 6;     // default setting.
  if( Temp == 0 ) return;
  LogTemp = log10( Temp );
  if( LogTemp < 1. ) return;
  NicePrecision =  NicePrecision + (int)LogTemp;
  if( NicePrecision > 13 ) NicePrecision = 13;
  return;
}
//*************************************************************
//            S c o o p
//*************************************************************
// Scan InputLine starting at position Start for a numeric type string
// at posn start with length length.
static void Scoop( char input[], const int initial, int &start, int &length)
{
  static char c;
  length = 0;
  if( input[initial]==NULL ) return;
  start = initial;
  // Determine start == next position not a blank, comma, tab char or colon.
  while (true)
  {
    c = input[start];
    if(  (c==NULL) ) return;   // ran into end of line.
    if( (c == ' ') || (c == ',') || (c =='\t') || c==':' ) { start++; continue;}
    break;
  }
  length = strcspn( &input[start], " ,\t:" ) ;
}
//*************************************************************
//        M o v e   F i x e d  F i e l d
//*************************************************************
// Internal service routine:
// Moves text from Start in Inputline to the field.
// A small number of leading blanks will be allowed.
static void MoveFixedField( char szField[], const int Start, const int InputLength )
{
 static int length, start, i;
 start = Start;
 strcpy ( szField, " " ); // Initialize with a blank line.
 if( start < InputLength)
 {
	for( i = 0; i < LeadingBlanks; i++ )
			 if( InputLine[start] == ' '||InputLine[start]=='\t' ) start++;  // check for whitespace
	length = strcspn( &InputLine[start], " ,\t" ) ;
	if( length > 0 )
	 {
		strncpy( szField, &InputLine[start], length ) ;
		szField[length] = NULL;
     }
  }
}
//*************************************************************
//        G e t    3    S t r i n g s
//*************************************************************
//
// Scans InputLine for 3 strings (szdx, szdy, szdz)
// delimited by whitespace or commas.
// Used by LoadFileOutlineData) routine only.
//
static int Get3Strings()
{
	static int i, InputLength;
   strcpy( szdx, "0" );
   strcpy( szdy, "0" );
   strcpy( szdz, "0" );

   if( StringIsBlank (InputLine) ) return 1; // Blank line is ok will be a move.
   InputLength = strlen( InputLine );

	Scoop( InputLine, 0, Xstart, Xlength );
	if( Xlength < 1 || Xlength > 50 ) return 0;
	strncpy( szdx, &InputLine[Xstart], Xlength ) ;
	szdx[Xlength] = NULL;

	Scoop( InputLine, Xstart+Xlength, Ystart, Ylength);
	if( Ylength < 1 || Ylength > 50 ) return 0;
	strncpy( szdy, &InputLine[Ystart], Ylength ) ;
	szdy[Ylength] = NULL;

   strcpy( Comment, " " ); // Initialize comment as well.

	Scoop( InputLine, Ystart+Ylength, Zstart, Zlength);
	if( Zlength < 1 || Zlength > 20 ) return 1;  // z missing is ok for outlines.
	strncpy( szdz, &InputLine[Zstart], Zlength ) ;
	szdz[Zlength] = NULL;

   Cstart = Zstart+Zlength;
	Clength = 0;
	if( InputLine[Cstart]==NULL ) return 1;
	Clength = strlen( &InputLine[Cstart]);
	// skip leading blanks for the free format comment field.
	for ( i = 0; i < Clength; i++)
	 {
		 if( InputLine[Cstart+i] != ' ' ){ Cstart += i; break ;}
	 }
	if( InputLine[Cstart]==NULL ) return 1;
	strcpy ( Comment, &InputLine[Cstart] ) ;
	Clength = strlen( Comment );
   // Now trim trailing blanks
   for( i = Clength-1; i > 0; i--)
      if( Comment[i] != ' ' ) {Clength = i+1; break; }
   Comment[Clength] = NULL;

   return 1;
}
//*************************************************************
//        S t r i n g   P o s i t i o n   M a p
//*************************************************************
// Utility scans the input line and determines the position
// of the strings it contains (start location & length)
// Everything is left in the common data structure "StringMap"
struct StringMapType
 {  int Start;
    int Length; };
static const int SizeOfMap = 30; // Number of strings to locate on line.
static StringMapType StringMap[SizeOfMap];

static void StringPositionMap()
{
  static int i, Start, Length, ScanPosition;
  for (i = 0; i < SizeOfMap; i++ ) // Initialize Position Map
     StringMap[i].Start = StringMap[i].Length = 0;
  ScanPosition = 0;
  for( i = 0; i < SizeOfMap; i++)
  {
    Scoop( InputLine, ScanPosition, Start, Length );
    if( Length < 1 ) return;
    StringMap[i].Start = Start;
    StringMap[i].Length = Length;
    ScanPosition = Start + Length;
  }
}
//*************************************************************
//        G e t    4    S t r i n g s
//*************************************************************
//  Routine scans InputLine for 4 strings (szdx, szdy, szdz, comment)
//
// Used by LatLon, NOS and Metric input.
static int Get4Strings()
{
  static int InputLength, i;

  szdx[0] = szdy[0] = szdz[0] = Comment[0] = NULL;

  if( FixedStarts )
  {
	TabEdit( InputLine, sizeof(InputLine) );
	InputLength = strlen( InputLine );
	MoveFixedField( szdx, Xstart, InputLength);
	MoveFixedField( szdy, Ystart, InputLength);
	MoveFixedField( szdz, Zstart, InputLength);
   // Don't want to allow leading blanks for comments???             <- NOTE
   strcpy ( Comment, " " );
	if( Cstart >= 0 ) MoveFixedField( Comment, Cstart, InputLength);
   return 1;
  } // end FixedStarts

// All the rest handles positional input.

	if( LinesRead == 1 ) TabEdit( InputLine, sizeof(InputLine) );
   StringPositionMap(); // Determine where the strings are in the input.
	InputLength = strlen( InputLine );

   Xstart = StringMap[InputFieldPosition[0]].Start;
   Xlength = StringMap[InputFieldPosition[0]].Length;

   if( Xlength < 1 || Xlength > 50 ) return 0;
	strncpy( szdx, &InputLine[Xstart], Xlength ) ;
	szdx[Xlength] = NULL;

   Ystart = StringMap[InputFieldPosition[1]].Start;
   Ylength = StringMap[InputFieldPosition[1]].Length;

	if( Ylength < 1 || Ylength > 50 ) return 0;
	strncpy( szdy, &InputLine[Ystart], Ylength ) ;
	szdy[Ylength] = NULL;

   Zstart = StringMap[InputFieldPosition[2]].Start;
   Zlength = StringMap[InputFieldPosition[2]].Length;

	if( Zlength < 1 || Zlength > 20 ) return 0;
	strncpy( szdz, &InputLine[Zstart], Zlength ) ;
	szdz[Zlength] = NULL;

   Cstart = StringMap[InputFieldPosition[3]].Start;
   Clength = StringMap[InputFieldPosition[3]].Length;
   strcpy( Comment, " " );
   if( Clength < 1 ) return 1;
   if( Cstart < 0 ) return 1;
	if( InputLine[Cstart]==NULL ) return 1;
   if( Clength > sizeof(Comment) ) Clength = sizeof(Comment) - 1 ;

   //NotifyUser( "Comment at %i of length at least %i ", Cstart, Clength );
// Check if comment last field take all to end of the input line.
// - If so take comment to the end of the line.

   if( InputFieldPosition[3] > InputFieldPosition[0] &&
       InputFieldPosition[3] > InputFieldPosition[1] &&
       InputFieldPosition[3] > InputFieldPosition[2]  )
   {     // Take to end of the line ...
	   Clength = strlen( &InputLine[Cstart]);
      if( Clength > sizeof(Comment) ) Clength = sizeof(Comment) - 1 ;
     	strncpy ( Comment, &InputLine[Cstart], Clength ) ;
      Comment[Clength] = NULL;
      // Now trim trailing blanks
      for( i = Clength-1; i > 0; i--)
         if( Comment[i] != ' ' ) {Clength = i+1; break; }
      Comment[Clength] = NULL;
    }  // end if Take to first break character only.
    else
      {
        strncpy( Comment, &InputLine[Cstart], Clength);
        Comment[Clength] = NULL;
      }

return 1;
}
//*************************************************************
//        G e t    N O A A   S t r i n g s
//*************************************************************
//  Routine scans InputLine for 8 strings (szdx, szdy, szdz, comment)
//
// Used for NOAA type lat lon input only.
static int GetNOAAStrings()
{
  static int InputLength, i;

  szdx[0] = szdy[0] = szdz[0] = Comment[0] = NULL;

// Don't handle fixed starts in initial implementation.

//  if( FixedStarts )
//  {
//	TabEdit( InputLine, sizeof(InputLine) );
//	InputLength = strlen( InputLine );
//	MoveFixedField( szdx, Xstart, InputLength);
//	MoveFixedField( szdy, Ystart, InputLength);
//	MoveFixedField( szdz, Zstart, InputLength);
   // Don't want to allow leading blanks for comments???             <- NOTE
//   strcpy ( Comment, " " );
//	if( Cstart >= 0 ) MoveFixedField( Comment, Cstart, InputLength);
//   return 1;
//  } // end FixedStarts

// All the rest handles positional input.

	if( LinesRead == 1 ) TabEdit( InputLine, sizeof(InputLine) );
   StringPositionMap();      // Determine where the strings are in the input.
	InputLength = strlen( InputLine );



//   For NOAA input we need three strings for lat and three for lon.
//                     Form is "dd mm ss.ssss"
//
// Perhaps here just pass back the input field start position.
// Maybe two string arrays of length three (one for each part?).

   for( i = 0; i < 3; i++ )
   {
     Xstart = StringMap[NOAAInputFieldPosition[0]+i].Start;
     Xlength = StringMap[NOAAInputFieldPosition[0]+i].Length;
     if( Xlength < 1 || Xlength > 8 ) return 0;
	  strncpy( &szNOAAdx[i][0], &InputLine[Xstart], Xlength ) ;
	  szNOAAdx[i][Xlength] = NULL;
   }

   for( i = 0; i < 3; i++ )
   {
     Ystart = StringMap[NOAAInputFieldPosition[1]+i].Start;
     Ylength = StringMap[NOAAInputFieldPosition[1]+i].Length;
     if( Ylength < 1 || Ylength > 8 ) return 0;
	  strncpy( &szNOAAdy[i][0], &InputLine[Ystart], Ylength ) ;
	  szNOAAdy[i][Ylength] = NULL;
   }

   Zstart = StringMap[NOAAInputFieldPosition[2]].Start;
   Zlength = StringMap[NOAAInputFieldPosition[2]].Length;

	if( Zlength < 1 || Zlength > 10 ) return 0;
	strncpy( szdz, &InputLine[Zstart], Zlength ) ;
	szdz[Zlength] = NULL;

   Cstart = StringMap[NOAAInputFieldPosition[3]].Start;
   Clength = StringMap[NOAAInputFieldPosition[3]].Length;
   strcpy( Comment, " " );
   if( Clength < 1 ) return 1;
   if( Cstart < 0 ) return 1;
	if( InputLine[Cstart]==NULL ) return 1;
   if( Clength > sizeof(Comment) ) Clength = sizeof(Comment) - 1 ;

   //NotifyUser( "Comment at %i of length at least %i ", Cstart, Clength );
// Check if comment last field take all to end of the input line.
// - If so take comment to the end of the line.

   if( NOAAInputFieldPosition[3] > NOAAInputFieldPosition[0] &&
       NOAAInputFieldPosition[3] > NOAAInputFieldPosition[1] &&
       NOAAInputFieldPosition[3] > NOAAInputFieldPosition[2]  )
   {     // Take to end of the line ...
	   Clength = strlen( &InputLine[Cstart]);
      if( Clength > sizeof(Comment) ) Clength = sizeof(Comment) - 1 ;
     	strncpy ( Comment, &InputLine[Cstart], Clength ) ;
      Comment[Clength] = NULL;
      // Now trim trailing blanks
      for( i = Clength-1; i > 0; i--)
         if( Comment[i] != ' ' ) {Clength = i+1; break; }
      Comment[Clength] = NULL;
    }  // end if Take to first break character only.
    else
      {
        strncpy( Comment, &InputLine[Cstart], Clength);
        Comment[Clength] = NULL;
      }

return 1;
}
//****************************************************************
//   A n a l y z e   S t a r t
//***************************************************************
// Used to analyze first line of input only to determine if it
// is being used to specify the location of fixed input data.
// Used by "CheckForFixedStarts", following....
static void AnalyzeStart( char string[], int start )
{
  if( (string[0]=='X')||(strncmp(string,"LA",2)==0) ) NewXstart = start;
  if( (string[0]=='Y')||(strncmp(string,"LO",2)==0) ) NewYstart = start;
  if( string[0]=='Z' )                             NewZstart = start;
  if( string[0]=='C' )                             NewCstart = start;
}
//*************************************************************
//        C h e c k   F o r   F i x e d   S t a r t s
//*************************************************************
// Check to see if the strings contain one of LA, LO, X, Y, C, Z.
// and if so set up the starting position as fixed.

// Return true only if processed a header line and it was ok.
//     (true means calling routine should read another line to replace it).

static bool CheckForFixedStarts()
{
 static char c;
 if( LinesRead != 1 ) return false;  // Only check first line in a file.
 c = szdx[0];
 if( !( c=='L'||c=='C'||c=='X'||c=='Y'||c=='Z') ) return false;
 NewXstart = NewYstart = NewZstart = 0;
 NewCstart = -1;
 AnalyzeStart( szdx, Xstart );
 AnalyzeStart( szdy, Ystart );
 AnalyzeStart( szdz, Zstart );
 AnalyzeStart( Comment, Cstart );
 if( NewXstart == NewYstart ) return false;
 if( NewXstart == NewZstart ) return false;
 if( NewYstart == NewZstart ) return false;
 FixedStarts = TRUE;
 Xstart = NewXstart;
 Ystart = NewYstart;
 Zstart = NewZstart;
 Cstart = NewCstart;
 LinesRead++;
 return true;
}
//*********************************************************************
//       G e t  L o a d   S t r i n g (internal service routine)
//*********************************************************************
static bool GetLoadString( int MessageNumber )
{
 if( LoadString( (HINSTANCE)hInst, MessageNumber, szLoadString, 255 ) ) return true;
 NotifyUserError( MessageNumber );
 return false;
}
//*************************************************************
//        R e s e t S u r f a c e T i t l e
//*************************************************************
//
// Internal service routine.
static void ResetSurfaceTitle( HWND &hwnd )
{
  MaxInputData = ScatterData.MaximumPoints();

  if( !GetLoadString( IDS_QUIKGRID ) ) return; 

  SetWindowText( hwnd, szLoadString );
}
//*************************************************************
//        S e t R e a d i n g S u r f a c e T i t l e
//*************************************************************
//
// Internal service routine.
static void SetReadingSurfaceTitle( HWND &hwnd, char FileName[] )
{
  if( !GetLoadString( IDS_READINGINPUTFROM ) ) return;
  sprintf( szPrint, szLoadString, FileName );
  SetWindowText( hwnd, szPrint );
}

//******************************************************************
//          S e t    S t a r t u p    S t a t e
//******************************************************************
void SetStartupState( HWND &hwnd )
{
  Zgrid.New(2,2);
  ScatterData.Reset();
  Zgrid.zset(0,0,-99); Zgrid.zset(0,1,-99);
  Zgrid.zset(1,0,-99); Zgrid.zset(1,1,-99);
  Zgrid.xset(0,0); Zgrid.xset(1,1);
  Zgrid.yset(0,0); Zgrid.yset(1,1);
  ContourLinesSet( 0, 1, NumberOfContours );
  ContourBoldLabelSet(PenHighLite); 
  PaintContourReset();
  RotateReset();
  szTitle[0] = NULL;
  dxnormalize = dynormalize = 0.0;
  ResetSurfaceTitle( hwnd );
  CancelLoadData = FALSE;
  PictureChanged( hwnd );
  GridGeneratedOK = FALSE;
}


//******************************************************************
//             C h e c k  L i m i t s
//******************************************************************
// Internal service routine. 
static int CheckLimits( const long &i )
{
  if( i == MaxInputData )
       { NotifyUser( IDS_TOOMUCHDATA );
			return 1; }

  if( CancelLoadData ) return 1;

  return 0; 
}
//*****************************************************************
//     R e p o r t   C o n v e r s i o n    F a i l
//*****************************************************************
// Internal service routine. 
static void ReportConversionFail(  char szText[], char szType[] )
{
   NotifyUser( IDS_CONVERSIONFAIL, szType, szText, LinesRead );
}
//*****************************************************************
//     R e p o r t   F l o a t   F a i l
//*****************************************************************
// Internal service routine. 
static void ReportFloatFail(  char szText[] )
{
  NotifyUser( IDS_FLOATINGPOINTFAIL, szText, LinesRead, szFileName );
}

//*****************************************************************
//     R e p o r t   L i n e  S h o r t
//*****************************************************************
// Internal service routine. 
static void ReportLineShort(  )
{
   static char TempSpace[10]; 
   if( StringIsBlank( InputLine ) )
   {
     if( IgnoreErrorLines) { IgnoreThisLine = TRUE; return; }
     // read another line and see if we get eof.  If so no error.
     inFile.getline( TempSpace, sizeof( TempSpace));
     if( inFile.eof() ) return;
   }
   NotifyUser( IDS_NOTENOUGHNUMBERS, LinesRead, szFileName, InputLine );

 }
//*****************************************************************
//     R e p o r t  E O F 
//*****************************************************************
// Internal service routine. 
static void ReportEOF( )
{
   NotifyUser( IDS_UNEXPECTEDEOF, LinesRead+1, szFileName );
}
//*****************************************************************
//     C o n v e r t   F l o a t
//*****************************************************************
// Internal service routine.
static int ConvertFloat( double &result,  char szText[] )
{
 if( StringIsBlank( szText ) )
  {
    result = 0.0;
    if( IgnoreErrorLines) IgnoreThisLine = TRUE;
    return TRUE;
  } 
 result = strtod(szText, &endptr);
 if( *endptr == NULL || *endptr == ','||*endptr=='\t' ) return TRUE;
 if( *endptr == 'D' ) *endptr = 'e';  // Perhaps exponent is +Dnn??
 result = strtod(szText, &endptr);    // so change to e and try again.
 if( *endptr == NULL || *endptr == ','||*endptr=='\t' ) return TRUE;
 if( !IgnoreErrorLines) { ReportFloatFail( szText ); return FALSE; }
 IgnoreThisLine = TRUE;
 return TRUE;
}

//*****************************************************************
//             I n i t i a l i z e  I n p u t 
//*****************************************************************
// Internal service routine
static int InitializeInput(HWND &hwnd, char FileName[], bool binary=false )
{

  CancelLoadData = FALSE;
  ResetSurfaceTitle( hwnd );
  szTitle[0] = NULL; 

  if( !binary) inFile.open( FileName, ios::in );
  else         inFile.open( FileName, ios::binary );
  if( !inFile )
    {
      NotifyUser( IDS_OPENINPUTFILEFAIL , FileName );
      return FALSE;
    }
  SetReadingSurfaceTitle( hwnd, FileName );
  ScatterData.Reset();
  SetWaitCursor();

  PointsRead = PointsKept = LinesRead = 0;
  LatLonData = FALSE;
  FixedStarts = FALSE;

  return TRUE;
}
//*****************************************************************
//             S e t  N i c e  C o l o u r   M a p
//*****************************************************************
void SetNiceColourMap( const float mina, const float maxa )
{
  static double min, incr, max;
  static double zadjust;
  min = mina;
  max = maxa;
  incr = ((double)max-(double)min)/9.;
  zadjust = ScatterData.zAdjust();
  incr = AtPrecision( incr, 1 );  // take increment to 1 digit.
  // Make min & max a divisor of increment.
  if( incr != 0.0 )
  {
    min = double( int( ((double)min-zadjust) / incr)*incr) + zadjust;
    max = double( int( ((double)max-zadjust) / incr)*incr) + zadjust;
  }
  DataColourMap( min, min+(max-min)*.5, max);
}
//*****************************************************************
//             I n i t i a l i z e   C o l o u r   M a p
//*****************************************************************
// Internal service routine.
static void InitializeColourMap()
{
  SetNiceColourMap( ScatterData.zMin(), ScatterData.zMax() );
}

//*****************************************************************
//             S h u t d o w n  I n p u t
//*****************************************************************
// Internal service routine. (not used by all routines).
static int ShutdownInput(HWND &hwnd,  char FileName[] )
{

  inFile.close();
  if( ScatterData.Size() < 3 )
	 {
      NotifyUser( IDS_TOOFEWPOINTS );
		SetStartupState( hwnd);
		return FALSE;
	 }
  ScatterData.zAdjust( -ScatterData.zMin() ); // Normalize to zero.
  if( ScatterData.zMin() == ScatterData.zMax() )
    {
     NotifyUser( "Warning: The z values for the input data span no distance (all are = %g)",
           ScatterData.zMin()-ScatterData.zAdjust());
    }
  if( LatLonData) NicePrecision = 6;
  else SetPrecision();
  SetSurfaceTitle( hwnd, FileName );
  RestoreCursor();
  InitializeColourMap();


  if( !InitializeGrid( hwnd ) ) return FALSE;

  return TRUE;
}
//*************************************************************
//        S t o r e D a t a P o i n t
//*************************************************************
// Internal service routine
static void StoreDataPoint( int StoreComment = FALSE, unsigned char Ignore=0 )
{
  static double temp;
  if( IgnoreThisLine ) return;
  if( (PointsRead%KeepEveryNthPoint) != 0 ) { PointsRead++; return;}
  if( FlipXY )
	 {
      if( LatLonData ) { temp = dx; dx = -dy; dy = -temp; }
      else          { temp = dx; dx = dy; dy = temp; }
    }
  if( ReverseSignOfZ ) dz = -dz;
  if( (PointsKept == 0) && (!GridLock) && (!LockNormalization) )
                           	{ dxnormalize = dx; dynormalize = dy; }

  if( IgnoreComments || (!StoreComment) )
       ScatterData.SetNext( dx-dxnormalize, dy-dynormalize, dz, NULL, Ignore );
  else ScatterData.SetNext( dx-dxnormalize, dy-dynormalize, dz, Comment, Ignore );

  PointsKept++;
  PointsRead++;

}
//*************************************************************
//        K e e p  E v e r y  N t h  P o i n t 
//*************************************************************
int KeepEveryNth() { return KeepEveryNthPoint; }
int KeepEveryNth( int NewValue )
{
  int OldValue = KeepEveryNthPoint;
  KeepEveryNthPoint = NewValue;
  if (KeepEveryNthPoint < 1 ) KeepEveryNthPoint = 1;
  return OldValue;
}
//*************************************************************
//        I g n o r e  Z  z e r o
//*************************************************************
int IgnoreZzero( ) { return IgnoreZero ; }
int IgnoreZzero( int i )
{
  int oldIgnoreZero = IgnoreZero;
  IgnoreZero = i;
  return oldIgnoreZero; 
}

//*************************************************************
//       S e t   S u r f a c e   T i t l e
//*************************************************************
void SetSurfaceTitle( HWND &hwnd, char szFileName[], float x, float y, int ix, int iy )
{
  static int i;
  static char *FileNameOnly;
  static GridViewType CurrentView;
  
  ostrstream Buf;
  Buf << "QuikGrid - ";
  FileNameOnly = &szFileName[0];
  for( i = (strlen(szFileName) - 1); i > 0; i-- )
    {
      if( szFileName[i] == '\\' )
                            { FileNameOnly = &szFileName[i+1]; break; }
    }

  Buf << FileNameOnly;
// Now optionally something about the state of resolution:

  CurrentView = Zgrid.view();
  if( CurrentView.xIncrement > 1 )
   {
     Buf << " <Grid Resolution 1/" << CurrentView.xIncrement << ">";
   }
  if( PaintState == IDM_2DSURFACE)
    { 
      if( (ix > -1) && (iy > -1) )   // Show cursor position?
       {
         if( LatLonData)
         {
           Buf << ": " << FormatLat( y+dynormalize) << " " ;
           Buf << FormatLon( x+dxnormalize) ;
         }
			else Buf << ": " << setprecision( NicePrecision)
						<< (x+dxnormalize) << " " << (y+dynormalize) ;
			Buf << setprecision(5);
			if( Zgrid.z(ix,iy) >= 0.0 )  // Display if valid.
						 Buf << " Z:" << (Zgrid.z(ix,iy) - ScatterData.zAdjust()) ;
         Buf << setprecision(3);
			if( Dgrid.xsize() > 2 && Dgrid.z(ix,iy) >= 0.0 ) // No z adjust for
						 Buf << " D:" << (Dgrid.z(ix,iy)) ;
			if( Sgrid.xsize() > 2 && Sgrid.z(ix,iy) >= 0.0 ) // Dgrid or Sgrid!!!
						 Buf << " S:" << (Sgrid.z(ix,iy)) ;
         Buf << setprecision(5);
         if( Cgrid.xsize() > 2 && Cgrid.z(ix,iy) >= 0.0 ) // Colour grid.
						 Buf << " C:" << (Cgrid.z(ix,iy) - CgridZadjust) ;
       }  // end if( (ix...
     } // end if (PaintState...

  else  // PaintState is 3D
    Buf << " Angle about: " <<  setprecision(3)
        << Turn << " above: " << Tilt << ends; 

  Buf << ends;

  char *szBuf = Buf.str();
  SetWindowText( hwnd, szBuf );
  delete szBuf;

}

//*****************************************************************
//     G r i d   S i z e   R a t i o :  Change Assumptions
//*****************************************************************
int GridSizeRatio() { return NiceRatio; }
int GridSizeRatio( int NewRatio )
  { int OldRatio = NiceRatio;
    NiceRatio=NewRatio;
    if( NiceRatio > 20 )
     { NotifyUser( IDS_GRIDRATIOINVALID ); NiceRatio = 20; }
    if( NiceRatio < 1  )
     { NotifyUser( IDS_GRIDRATIOINVALID ); NiceRatio = 1; }
    return OldRatio; }
//*****************************************************************
//     G r i d   M a x   R a t i o :  Change Assumptions
//*****************************************************************
int GridMaxRatio() { return NiceRatioMaxLines; }
int GridMaxRatio( int NewRatio )
{
  static int OldRatio, MaxLines;
	 MaxLines = Zgrid.MaximumXY();
	 OldRatio = NiceRatioMaxLines;
    NiceRatioMaxLines=NewRatio;
	 if( NiceRatioMaxLines > MaxLines )
	  { NotifyUser( "Maximum lines is too large" );
       NiceRatioMaxLines = 200; }
    if( NiceRatioMaxLines < 20  )
     { NotifyUser( "Maximum lines cannot be smaller than 20" );
       NiceRatioMaxLines = 20; }
	 return OldRatio;
}

//******************************************************************
//             L o a d  T e s t   D a t a
//******************************************************************
void LoadTestData(HWND &hwnd )
{
  ResetSurfaceTitle( hwnd );

  LatLonData = 0; 
  struct Point3d { float x, y, z; char* comment; unsigned char ignore; } ;
  static Point3d TestData[] = {
  {11,11,11,"Comment for point 1",0},
  {14,11.5,12, "2, - a comment",0},
  {11.2,15.4,12, "Comment for point 3",0},
  {13,13,17, "4, Another comment",0},
  {15.1,14.7,13,"Five ",0},
  {18,18,19,"Six",0},
  {20,20,13,"7, The quick brown fox jumped over the lazy dog (a long commment)",0},
  {14.6,19,14,"Eight",0},
  {18.5,16.2,12,"Nine",0},
  {12.2,11.8, 17, "Ten - initially ignored",1}  } ;

  int NumberOfPoints = sizeof(TestData)/sizeof(TestData[0]);
  if( !GridLock && !LockNormalization ) dxnormalize = dynormalize = 0;
  ScatterData.Reset(); 
  for( int i=0;i<NumberOfPoints;i++)
    ScatterData.SetNext( TestData[i].x-dxnormalize,
			 TestData[i].y-dynormalize,
			 TestData[i].z,
			 TestData[i].comment,
			 TestData[i].ignore) ;
  
  if( !GridLock ) Zgrid.New( 10, 10 );

  SetSurfaceTitle( hwnd, "Internal test data." );
  InitializeGrid( hwnd );
  InitializeColourMap();
  SetPrecision();
}

//*****************************************************************
//             L o a d   N o r m a l i z a t i o n
//*****************************************************************
void LoadNormalization( double &dx, double &dy )
{
  dx = dxnormalize;
  dy = dynormalize; 
}

//*****************************************************************
//            C o n v e r t I n t e g e r
//*****************************************************************
static bool ConvertInteger( long &result, char szText[] )
{
 if( !StringIsInteger( szText ) )
   {
     if( !IgnoreErrorLines) { ReportFloatFail( szText ); return FALSE; }
     IgnoreThisLine = TRUE;
     return true;
   }
 result = atoi( szText );
 return true;
}
//*****************************************************************
//            C o n v e r t N O S
//*****************************************************************
static bool ConvertNOS( double &result, char szText[] )
{
    long NOS;
    result = 0.0;
    if( !ConvertInteger( NOS, szText))
     { NotifyUser( IDS_NOSERROR, szText, InputLine );
       return false; }
    result = ConvertDegDec( NOS );
    return true;
}
//*****************************************************************
//             L o a d   F i l e   N O S  D a t a
//*****************************************************************
int LoadFileNOSData( HWND &hwnd, char FileName[] )
{
  long dxNOS, dyNOS;
  static int StoreComment;

if( NewInputStyle )
{
  ConstructFilename( FileName );
  if( !InitializeInput( hwnd, szFileName ) ) return FALSE;
  LatLonData = TRUE;

  while ( TRUE )
   {
		IgnoreThisLine = FALSE;
      LinesRead++;
      inFile.getline( InputLine, sizeof( InputLine) );

      if ( inFile.eof() )
       {
           if( NoMoreFiles( FileName ) ) return ShutdownInput( hwnd, FileName );
           inFile.getline( InputLine, sizeof( InputLine) );
           if( inFile.eof() ) return ShutdownInput( hwnd, FileName );
       }

      Get4Strings();
      if( StringIsBlank( InputLine )  ) continue; // ignore blank line.
      if( StringIsBlank(szdx) || StringIsBlank(szdy) || StringIsBlank(szdz))
            { ReportLineShort(); return ShutdownInput( hwnd, FileName ); }

      if( CheckForFixedStarts() ) continue;

      if( !ConvertNOS( dy, szdx ) ) return ShutdownInput( hwnd, FileName );
		if( !ConvertNOS( dx, szdy ) ) return ShutdownInput( hwnd, FileName );
      dx = -dx;
      if( !ConvertFloat( dz, szdz ) ) return ShutdownInput( hwnd, FileName );

 		StoreComment = TRUE;
      if( StringIsBlank( Comment ) ) StoreComment = FALSE;
		StoreDataPoint( StoreComment );
		if( CheckLimits(PointsKept)) break;

	 } // end while (TRUE)
  } // end if( NewInputStyle

else
{ // Old sytle NOS input follows.

  if( !InitializeInput( hwnd, FileName ) ) return FALSE;
  strcpy( szFileName, FileName );
  LatLonData = TRUE;

  while (inFile >> dxNOS >> dyNOS >> dz)
	 {
		IgnoreThisLine = FALSE;
      LinesRead++; 
      dy = ConvertDegDec( dxNOS );  // Longitude is first.
      dx = -ConvertDegDec( dyNOS );  // Latitude is second.
      if( fabs(dx) > 360. || fabs(dy) > 90. )
       { NotifyUser( IDS_NOTNOSDATA );
         break; } 

		StoreDataPoint( );
      if (CheckLimits(PointsKept)) return ShutdownInput( hwnd, FileName );

     }
  if( !inFile.eof()&&inFile.fail() )
    {
		ostrstream Buf;

		Buf << "Error reading scattered NOS formatted data points from file "
		<< FileName << " at approximately line number " << (LinesRead+1)
			  << ". Input terminated. " << ends;

		char *szBuf = Buf.str();
      NotifyUser( szBuf );
	  delete szBuf;
    }

 } // end else - old style input.
 return ShutdownInput( hwnd, FileName );
}
//*****************************************************************
//             L o a d   F i l e   L a t   L o n
//*****************************************************************
int LoadFileLatLonData( HWND &hwnd, char FileName[] )
{
  static int StoreComment;

  ConstructFilename( FileName );
  if( !InitializeInput( hwnd, szFileName ) ) return FALSE;

	while (TRUE)
	{
      IgnoreThisLine = FALSE;
      LinesRead++;
      inFile.getline( InputLine, sizeof( InputLine) );

      if ( inFile.eof() )
       {
           if( NoMoreFiles( FileName ) ) return ShutdownInput( hwnd, FileName );
           inFile.getline( InputLine, sizeof( InputLine) );
           if( inFile.eof() ) return ShutdownInput( hwnd, FileName );
       }

      Get4Strings();
      if( StringIsBlank( InputLine )  ) continue; // ignore blank line.
      if( StringIsBlank(szdx) || StringIsBlank(szdy) || StringIsBlank(szdz))
            { ReportLineShort(); return ShutdownInput( hwnd, FileName ); }

      if( CheckForFixedStarts() ) continue;

     // if( !ConvertFloat( dx, szdx ) ) return ShutdownInput( hwnd, FileName );
	  //	if( !ConvertFloat( dy, szdy ) ) return ShutdownInput( hwnd, FileName );
     if( !LonToDouble( dx, szdy ) )
			  { if( !IgnoreErrorLines )
				 {
					ReportConversionFail( szdy, "Lat/Lon number" );
					return ShutdownInput( hwnd, FileName );
				 }
             else IgnoreThisLine = TRUE;
				}
		if( !LatToDouble( dy, szdx ) )
			  { if( !IgnoreErrorLines )
				 {
				  ReportConversionFail( szdx, "Lat/Lon number" );
				  return ShutdownInput( hwnd, FileName );
				  }
				 else  IgnoreThisLine = TRUE;
			  }
      if( !ConvertFloat( dz, szdz ) ) return ShutdownInput( hwnd, FileName );

 		StoreComment = TRUE;
      if( StringIsBlank( Comment ) ) StoreComment = FALSE;
		StoreDataPoint( StoreComment );
		if( CheckLimits(PointsKept)) break;

	 } // end while (TRUE)

return ShutdownInput( hwnd, FileName );

/*  if( !InitializeInput( hwnd, FileName ) ) return FALSE;
  strcpy( szFileName, FileName );
  LatLonData = TRUE;

  while ( TRUE )
   {
		IgnoreThisLine = FALSE;
		LinesRead++;
      inFile.getline( InputLine, sizeof( InputLine) );
      if ( inFile.eof() ) break;

		if( !Get4Strings() )
			 { ReportLineShort();
            return ShutdownInput( hwnd, FileName );}

      if( CheckForFixedStarts() ) continue;

		if( !LonToDouble( dx, szdy ) )
			  { if( !IgnoreErrorLines )
				 {
					ReportConversionFail( szdy, "Lat/Lon number" );
					return ShutdownInput( hwnd, FileName );
				 }
             else IgnoreThisLine = TRUE;
				}
		if( !LatToDouble( dy, szdx ) )
			  { if( !IgnoreErrorLines )
				 {
				  ReportConversionFail( szdx, "Lat/Lon number" );
				  return ShutdownInput( hwnd, FileName );
				  }
				 else  IgnoreThisLine = TRUE;
			  }
		if( !ConvertFloat( dz, szdz ) ) return ShutdownInput( hwnd, FileName );

 		StoreComment = TRUE;
      if( StringIsBlank( Comment ) ) StoreComment = FALSE;
		StoreDataPoint( StoreComment );
		if( CheckLimits(PointsKept)) break;
		if( inFile.eof() )  break;  // eof ok here - all done.
	 } // end while (TRUE)


  return ShutdownInput( hwnd, FileName );    // */
}

//*****************************************************************
//            C o n v e r t N O A A
// i = 1 convert x  i = 2 convert y.
//*****************************************************************
static bool ConvertNOAA( double &result, char dd[], char mm[], char ss[] )
{
  double seconds;
  long minutes, degrees;
  
  result = 0.0;

  if( !ConvertFloat( seconds, ss ) )
       { NotifyUser( IDS_NOAAERROR, ss, InputLine );
       return false; }

  if( abs(seconds)>60.00) return 999.;

  if( dd[0] == 'N' || dd[0] == 'n' )  dd[0] = '0';
  if( dd[0] == 'S' || dd[0] == 's' )  dd[0] = '-';

  if( !ConvertInteger( degrees, dd))
       { NotifyUser( IDS_NOAAERROR, dd, InputLine );
       return false; }

  if( abs(degrees)>720) return 999.;
  
  if( mm[0] == 'W' || mm[0] == 'w' )  mm[0] = '0';
  if( mm[0] == 'E' || mm[0] == 'e' )  mm[0] = '-';

  if( !ConvertInteger( minutes, mm))
       { NotifyUser( IDS_NOAAERROR, mm, InputLine );
       return false; }

  if( abs(minutes)>60) return 999.;

  double ddegrees = degrees;
  double dminutes = (double) minutes/60.;
  double dseconds = seconds/3600.;

  result = ddegrees+dminutes+dseconds;
  return true;
}

//*****************************************************************
//       L o a d   F i l e   N O A A  L a t   L o n   D a t a
//*****************************************************************
//
// NOAA style lat lon data is "dd mm ss.ssss" (spaces between fields).
//
int LoadFileNOAALatLonData( HWND &hwnd, char FileName[] )
{
  static int StoreComment;

  ConstructFilename( FileName );
  if( !InitializeInput( hwnd, szFileName ) ) return FALSE;
  LatLonData = TRUE;

  while (TRUE)
	{
      IgnoreThisLine = FALSE;
      LinesRead++;
      inFile.getline( InputLine, sizeof( InputLine) );

      if ( inFile.eof() )
       {
           if( NoMoreFiles( FileName ) ) return ShutdownInput( hwnd, FileName );
           inFile.getline( InputLine, sizeof( InputLine) );
           if( inFile.eof() ) return ShutdownInput( hwnd, FileName );
       }

      GetNOAAStrings();
      if( StringIsBlank( InputLine )  ) continue; // ignore blank line.

//      if( CheckForFixedStarts() ) continue;

//    if( !LonToDouble( dx, szdy ) )
      if( !ConvertNOAA( dx, &szNOAAdy[0][0],&szNOAAdy[1][0],&szNOAAdy[2][0] ) )
			  { if( !IgnoreErrorLines )
				 {
					ReportConversionFail( szdy, "NOAA Lat/Lon number" );
					return ShutdownInput( hwnd, FileName );
				 }
             else IgnoreThisLine = TRUE;
				}
      dx = -dx; 

//		if( !LatToDouble( dy, szdx ) )
      if( !ConvertNOAA( dy, &szNOAAdx[0][0], &szNOAAdx[1][0], &szNOAAdx[2][0] ) )
			  { if( !IgnoreErrorLines )
				 {
				  ReportConversionFail( szdx, "NOAA Lat/Lon number" );
				  return ShutdownInput( hwnd, FileName );
				  }
				 else  IgnoreThisLine = TRUE;
			  }
           
      if( !ConvertFloat( dz, szdz ) ) return ShutdownInput( hwnd, FileName );

 		StoreComment = TRUE;
      if( StringIsBlank( Comment ) ) StoreComment = FALSE;
		StoreDataPoint( StoreComment );
		if( CheckLimits(PointsKept)) break;

	 } // end while (TRUE)

return ShutdownInput( hwnd, FileName );
}
//*****************************************************************
//             L o a d   F i l e   M e t r i c  D a t a
//*****************************************************************
int LoadFileMetricData( HWND &hwnd, char FileName[] )
{
  static int StoreComment;

  ConstructFilename( FileName );
  if( !InitializeInput( hwnd, szFileName ) ) return FALSE;
  if( NewInputStyle )
  {
	while (TRUE)
	{
      IgnoreThisLine = FALSE;
      LinesRead++;
      inFile.getline( InputLine, sizeof( InputLine) );

      if ( inFile.eof() )
       {
           if( NoMoreFiles( FileName ) ) return ShutdownInput( hwnd, FileName );
           inFile.getline( InputLine, sizeof( InputLine) );
           if( inFile.eof() ) return ShutdownInput( hwnd, FileName );
       }

      Get4Strings();
      if( StringIsBlank( InputLine )  ) continue; // ignore blank line.
      if( StringIsBlank(szdx) || StringIsBlank(szdy) || StringIsBlank(szdz))
            { ReportLineShort(); return ShutdownInput( hwnd, FileName ); }

      if( CheckForFixedStarts() ) continue;

      if( !ConvertFloat( dx, szdx ) ) return ShutdownInput( hwnd, FileName );
		if( !ConvertFloat( dy, szdy ) ) return ShutdownInput( hwnd, FileName );
      if( !ConvertFloat( dz, szdz ) ) return ShutdownInput( hwnd, FileName );

 		StoreComment = TRUE;
      if( StringIsBlank( Comment ) ) StoreComment = FALSE;
		StoreDataPoint( StoreComment );
		if( CheckLimits(PointsKept)) break;

	 } // end while (TRUE)
  } // end if( NewInputStyle

  else
  {
   while ( TRUE )
   {
      LinesRead++;
      inFile >> szdx;
      if( inFile.eof() )  break;  // eof ok here - all done.

      inFile >> szdy;
      if( inFile.eof() )    // eof here is unexpected.
   	{ ReportEOF();
	      return ShutdownInput( hwnd, FileName );
   	}

      inFile >> szdz;

		if( !ConvertFloat( dx, szdx ) ) return ShutdownInput( hwnd, FileName );
		if( !ConvertFloat( dy, szdy ) ) return ShutdownInput( hwnd, FileName );
		if( !ConvertFloat( dz, szdz ) ) return ShutdownInput( hwnd, FileName );

		StoreDataPoint( );
		if( CheckLimits(PointsKept)) break;
		if( inFile.eof() )  break;  // eof ok here - all done.
	 } // end while (TRUE)
  } // end else (old sytle input)

  return ShutdownInput( hwnd, FileName );
}
//*****************************************************************
//             S h u t d o w n  O u t l i n e I n p u t
//*****************************************************************
// Internal service routine. (not used by all routines).
static int ShutdownOutlineInput()
{
  static float zMin, zMax;
  inFile.close();
  if( OutLine.Size() < 2 )
	 {
		NotifyUser( "Less than two outline data points loaded. Input ignored. ");
		OutLine.Reset();
      RestoreCursor();
		return FALSE;
	 }
  // iff no grid and no points loaded
  // dummy up a 3x3 grid and data points so the outline will display.
  if( (ScatterData.Size() < 3 ) && (Zgrid.xsize() < 4) )
	{
	  zMin = OutLine.zMin();
     zMax = OutLine.zMax();
     if( zMin == zMax ) zMax += (OutLine.xMax()-OutLine.xMin())*.25;

  // dummy up some scattered data points so z normalization works.

	  ScatterData.Reset();
	  ScatterData.SetNext( OutLine.xMin(), OutLine.yMin(), zMin);
	  ScatterData.SetNext( OutLine.xMax(), OutLine.yMax(), zMax );
	  ScatterData.zAdjust( -zMin );

  // set up a dummy grid.

	  Zgrid.New(3,3);
     SetGridXY( OutLine.xMin(), .5*(OutLine.xMax()-OutLine.xMin()),
					 OutLine.yMin(), .5*(OutLine.yMax()-OutLine.yMin()) ) ;
     Zgrid.zset(0,0,zMin); 
     Zgrid.zset(0,0,zMax);

	  NumberOfContours = 10;
	  NiceContourLinesSet();
     ContourBoldLabelSet( PenHighLite );
	  PaintContourReset();
	  RotateReset();
	  SetPrecision();
	}  // End if (ScatterData...
  LockNormalization = TRUE;
  RestoreCursor();
  return TRUE;
}
//*****************************************************************
//             L o a d   F i l e   O u t l i n e  D a t a
//*****************************************************************
int LoadFileOutlineData( HWND &hwnd, char FileName[] )
{
  static unsigned char MoveNext;

  // strcpy( szFileName, FileName );
  ConstructFilename( FileName );

  CancelLoadData = FALSE;
  inFile.open( szFileName, ios::in );
  if( !inFile )
    {
      NotifyUser( " Unable to open file to input outline data. " );
      return FALSE;
    }

  OutLine.Reset();
  LockNormalization = FALSE;
  SetWaitCursor();
  SetReadingSurfaceTitle( hwnd, FileName );
  PointsRead = PointsKept = LinesRead = 0;
  MoveNext = 1;

  while (TRUE)
	{
      IgnoreThisLine = FALSE;
      LinesRead++;

      inFile.getline( InputLine, sizeof( InputLine) );
      if ( inFile.eof() )
        {
         // break;
           if( NoMoreFiles( FileName ) ) return ShutdownOutlineInput(  );
           inFile.getline( InputLine, sizeof( InputLine) );
           if( inFile.eof() ) return ShutdownOutlineInput(  );
           MoveNext = 1; 
        }
      if( !Get3Strings() )
			 { ReportLineShort(); return ShutdownOutlineInput( );}
      if( szdx[0] == '\#' || szdx[0] == 'n' || StringIsBlank( InputLine) ) MoveNext = 1;
            else
      {
		  if( !ConvertFloat( dx, szdx ) ) return ShutdownOutlineInput( );
		  if( !ConvertFloat( dy, szdy ) ) return ShutdownOutlineInput( );
		  if( !ConvertFloat( dz, szdz ) ) return ShutdownOutlineInput( );
        if( PointsKept == 0 &&         // for 1st data point with
            !GridLock &&               // no gridlock
            ScatterData.Size() < 3 &&  // no data points.
            Zgrid.xsize() < 4  )       // no grid.
            { dxnormalize = dx; dynormalize = dy; }  // Set normalization

        if( StringIsBlank( Comment ) )
          OutLine.SetNext( (float)(dx-dxnormalize), (float)(dy-dynormalize), (float)dz, NULL, MoveNext );
        else
          OutLine.SetNext( (float)(dx-dxnormalize), (float)(dy-dynormalize), (float)dz, Comment, MoveNext );

        PointsKept++;
        PointsRead++;
        MoveNext = 0;
      } // end else.

		if( CheckLimits(PointsKept)) break;
		if( inFile.eof() )  break;  // eof ok here - all done.

	 } // end while (TRUE)

  return ShutdownOutlineInput();
}


//*****************************************************************
//             L o a d   F i l e   D C A   D a t a
//*****************************************************************
// Sample data points: 
// 127,5037.375169,966.621120,29.507,fence
// 129,4985.084287,973.404674,32.341,conc.slab

// IgnoreErrorLines does not apply to DCA data. 

int LoadFileDCAData( HWND &hwnd, char FileName[] )
{
  static char szBuf1[11], szBuf2[101], szComment[120];
  unsigned char ignore;

  if( !InitializeInput( hwnd, FileName ) ) return FALSE;
  strcpy( szFileName, FileName );
  while ( TRUE )
	{
      IgnoreThisLine = FALSE;
		LinesRead++;
      inFile.get( szBuf1, sizeof( szBuf1 ), ',' );
      if( inFile.fail()||inFile.eof() ) break;
      inFile.ignore();

      inFile.get( szdx, sizeof( szdx ), ',' );
      if( !ConvertFloat( dx, szdx ) ) return ShutdownInput( hwnd, FileName );
      inFile.ignore();

      inFile.get( szdy, sizeof( szdy ), ',' );
      if( !ConvertFloat( dy, szdy ) ) return ShutdownInput( hwnd, FileName );
      inFile.ignore();

      inFile.get( szdz, sizeof( szdz ), ',' );
      if( !ConvertFloat( dz, szdz ) ) return ShutdownInput( hwnd, FileName );
      ignore = 0;

      if( IgnoreZero && (dz == 0.0) ) ignore = 1 ;
      if( inFile.fail() ) break;

      inFile.get( szBuf2, sizeof(szBuf2), '\n' );
      inFile.ignore();  // skip the \n.
      strcpy( szComment, szBuf1 );
      strcat( szComment, szBuf2 );

      if( (PointsRead%KeepEveryNthPoint) != 0 ) { PointsRead++; continue;}
      if( PointsKept == 0 && !GridLock && !LockNormalization )
	     { dxnormalize = dx;
          dynormalize = dy; 
        }
      if( IgnoreComments) szComment[0] = NULL; 
      ScatterData.SetNext( dx-dxnormalize, dy-dynormalize, dz, szComment, ignore );
      PointsKept++;
      PointsRead++;
      if( CheckLimits(PointsKept)) break;

     }
  if( (!inFile.eof()&&inFile.fail() ) )
    {
      ostrstream Buf;

      Buf << "Error reading DCA formatted data points from file "
	   << FileName << " at line number " << (LinesRead+1)
           << ". Input terminated. " << ends;

      char *szBuf = Buf.str();
      NotifyUser( szBuf );
     delete szBuf;
    }

  return ShutdownInput( hwnd, FileName );

}

//*****************************************************************
//             L o a d   F i l e   D X F   D a t a
//*****************************************************************

//*****************************************************************
//     ReadGroupCode - internal service routine for DXF input.
//*****************************************************************
static int ReadCodePair( int &GroupCode, char szValue[] )
{
   static char szGroupCode[50];

   inFile.get( szGroupCode, sizeof(szGroupCode), '\n' );
   if( inFile.fail() ) return FALSE;
   inFile.ignore(999, '\n');
   GroupCode = atoi(szGroupCode);
   inFile.get( szValue, 50, '\n' ); 
   if( inFile.fail() ) return FALSE;
   inFile.ignore(999, '\n');
   LinesRead += 2; 
   return TRUE; 
}

//*****************************************************************
//             L o a d   F i l e   D X F   D a t a
//*****************************************************************
int LoadFileDXFData( HWND &hwnd, char FileName[] )
{
  static char szBuf1[50];
  static int GroupCode;
  static char szLayerName[60];
  static double dxold, dyold, dzold;
  static bool SkipNextPoint;

  if( !InitializeInput( hwnd, FileName ) ) return FALSE;
  strcpy( szFileName, FileName );
  // First step - scan all the way down to the ENTITIES section.
  SkipNextPoint = false;
  while ( TRUE )
  {
    if( !ReadCodePair( GroupCode, szBuf1) )
      { ReportEOF(); return ShutdownInput( hwnd, FileName ); }
    if( (GroupCode == 2 ) && 
    	stricmp( szBuf1, "ENTITIES") == 0 )  break;
  }
   
  // Now want to scoop anything that looks like a 3d coordinate
  // When a z coordinate is scooped stash a data point. 
  while ( TRUE )
  {
    IgnoreThisLine = FALSE; 
    if( !ReadCodePair( GroupCode, szBuf1 ) ) return ShutdownInput( hwnd, FileName );
   // Check to see if a polyline is starting - if so ignore first coordinate.
   if( (GroupCode == 0 ) &&
       stricmp( szBuf1, "POLYLINE") == 0 ) SkipNextPoint = true;
    switch (GroupCode)
      {
        case 8: // Layer name
	       strcpy( szLayerName, szBuf1 );
          break;

        case 10: // X coordinate:
        case 11:
	       if( !ConvertFloat( dx, szBuf1) ) return ShutdownInput( hwnd, FileName );
	       break;

        case 20: // Y coordinate:
        case 21:
	       if( !ConvertFloat( dy, szBuf1) ) return ShutdownInput( hwnd, FileName );
	       break;

        case 30: // Z coordinate:
        case 31:
	       if( !ConvertFloat( dz, szBuf1) ) return ShutdownInput( hwnd, FileName );
	       // if this point same as previous point discard it.
          // if the skipnextpoint flag is not set discard it.
	       if( (dx!=dxold || dy!=dyold || dz!=dzold || PointsKept==0 )
                       && !SkipNextPoint )
	       {
            StoreDataPoint( );
            if( CheckLimits(PointsKept)) return ShutdownInput( hwnd, FileName );
          }
          SkipNextPoint = false;
          dxold = dx; dyold = dy; dzold = dz; 
          break;
        } // End switch( GroupCode)
     } // End while (TRUE)
}



//******************************************************************
//         A b o r t   R e a d   G r i d   F i l e 
//******************************************************************
// Internal service routine. 
static int AbortReadGridFile( char FileName[] )
{
  ostrstream Buf;

  Buf << "Error reading a grid from file "
	   << FileName 
	   << ". Input terminated. " << ends;

  char *szBuf = Buf.str();
  NotifyUser( szBuf );
  delete szBuf;
  inFile.close();
  RestoreCursor();

  PaintContourReset();
  RotateReset();
  strcpy( szTitle, " " );
  return FALSE;    
}
//********************************************************************
//       S h u t d o w n G r i d L o a d
//********************************************************************
static void ShutdownGridLoad( float zAdjust )
{
  ScatterData.Reset();
  ScatterData.SetNext( Zgrid.x(0), Zgrid.y(0), Zgrid.zmin()-zAdjust);
  ScatterData.SetNext( Zgrid.x(Zgrid.xsize()-1), Zgrid.y(Zgrid.ysize()-1),
				 Zgrid.zmax()-zAdjust);
  ScatterData.zAdjust( zAdjust );
  NumberOfContours = 10;
  NiceContourLinesSet();
  ContourBoldLabelSet( PenHighLite ); 
  PaintContourReset();
  RotateReset();
  SetPrecision();
  RestoreCursor();
  InitializeColourMap();
  if( DisplayVolumeDifference ) ColourLayerVolume();
}
//********************************************************************
//       L o a d    G r i d    D a t a
//********************************************************************
int LoadGridData( HWND &hwnd, char FileName[] )
{
 int nx, ny;
 float a;
 char szFloat[50], *endptr;
 int FloatFail;
 int i;

 if( !InitializeInput( hwnd, FileName ) ) return FALSE;
 strcpy( szFileName, FileName );
 if( !LockNormalization) dxnormalize = dynormalize = 0;

  float zAdjust;
  inFile >> nx >> ny ;
  if( GridLock )
   { if( (nx != Zgrid.xsize()) || (ny != Zgrid.ysize()) )
     { NotifyUser( IDS_GRIDLOCKEDTEMPLATE);
       return AbortReadGridFile(FileName); }
   // else
   // NotifyUser( "Current grid is locked as template but this grid is the same size - the z coordinates will be replaced.");
   }
  int MaximumGridSize = Zgrid.MaximumXY();
  if( inFile.fail()||nx<3||ny<3||
		nx>MaximumGridSize||ny>MaximumGridSize ) return AbortReadGridFile(FileName);

  if( !GridLock ) Zgrid.New( nx, ny );
  if( !GridLock )ZapZgrid();
  else Zgrid.Znew();

  while (inFile.peek() == ' ') inFile.ignore(); // scan off blanks.
  if (inFile.peek() == '\n' )  // default zAdjust if missing from
       zAdjust = 0.0;          // first line.
  else { inFile >> szFloat; zAdjust = atof( szFloat ); }

  // And repeat for flag for lat lon data.
   LatLonData = FALSE;
  if( inFile.peek() != '\n' )
   {
     while (inFile.peek() == ' ') inFile.ignore(); // scan off blanks.
     if (inFile.peek() != '\n' )
     {
      inFile >> szFloat;
      if( strcmp( szFloat, "LatLonData") == 0 ) LatLonData = TRUE;
     }
   }

  FloatFail = FALSE;
  for( i = 0; i < nx; i++ )
	  { inFile >> szFloat; a = strtod(szFloat, &endptr);
		 if( *endptr != NULL ) FloatFail = TRUE;
		 if( !GridLock && (i == 0) && !LockNormalization ) dxnormalize = a;
       if( !GridLock ) Zgrid.xset( i, a-(float)dxnormalize ) ;
		 if( inFile.fail()||FloatFail )
		      return AbortReadGridFile(FileName);
     }

  for( i = 0; i < ny; i++ )
     { inFile >> szFloat; a = strtod(szFloat, &endptr);
		 if( *endptr != NULL ) FloatFail = TRUE;
       if( !GridLock && (i == 0) && !LockNormalization) dynormalize = a;
       if( !GridLock ) Zgrid.yset( i, a-(float)dynormalize ) ;
		 if( inFile.fail()||FloatFail )
		      return AbortReadGridFile(FileName);
     }
  PointsKept = 0;
  for( i = 0; i<nx; i++ )
   { for( int j = 0; j<ny; j++ )
     { inFile >> szFloat; a = strtod(szFloat, &endptr);
		 if( *endptr != NULL ) FloatFail = TRUE;
		 Zgrid.zset(i,j,a);
       PointsKept ++;
		 if( inFile.fail()||FloatFail ) return AbortReadGridFile(FileName);
       if( CancelLoadData ) break;
     }
    if( CancelLoadData ) break;
   }

  inFile.close();
  ShutdownGridLoad( zAdjust );
  SetSurfaceTitle( hwnd, FileName );

  return TRUE;
}
//***************************************************************
//  D E M   G r i d   S e r v i c e   R o u t i n e s
//***************************************************************
// If size == 0 do free format read.
static int DEMfail;
static int GetDEMint( int size )
{
  static int result;
  static char szText[50], *endptr, szBuf[100];
  if( size == 0 ) inFile >> szText;
  else { inFile.read( szText, size ); szText[size] = NULL; }
  result = strtol( szText, &endptr, 10 );
  if( *endptr == NULL || *endptr == ',' || *endptr == ' ') return result;
  DEMfail = TRUE;
  sprintf( szBuf, "Error reading integer point number %s ", szText );
  NotifyUser( szBuf );
  return 0;
}
//*******************************************************************
static double GetDEMdouble( int size )
{
  static double result;
  static char szText[50], *endptr, szBuf[100] ;
  if( size == 0 ) inFile >> szText;
  else {inFile.read( szText, size ); szText[size] = NULL;}
  result = strtod(szText, &endptr);
  if( *endptr == NULL || *endptr == ',' || *endptr == ' ' ) return result;
  if( *endptr == 'D' ) *endptr = 'e';  // Perhaps exponent is +Dnn??
  result = strtod(szText, &endptr);    // so change to e and try again.
  if( *endptr == NULL || *endptr == ',' ) return result;
  DEMfail = TRUE;
  sprintf( szBuf, "Error reading floating point number %s ", szText );
  NotifyUser( szBuf );
  return 0;
}
//********************************************************************
//       L o a d    D E M   g r i d   d a t a
//********************************************************************
// with respectful thanks to sol Katz - much source lifted from his
// DEM2xyz program.
int LoadUSGSDEMData( HWND &hwnd, char FileName[] )
{
#define     YMAX     2048          /*  max length of an NCIC scan line */
#define     XMAX     2048          /*  max # of NCIC scan lines */

#define MIN(x,y)     (((x) < (y)) ? (x) : (y))
#define MAX(x,y)     (((x) > (y)) ? (x) : (y))

#define SW     0
#define NW     1
#define NE     2
#define SE     3

int             base;
double          westMost, southMost, northMost; 
double          comp;
double          deltaY, deltaX, temp;
//double          eastMost, yCoord;
//int             eastMostSample,  westMostSample;
int             southMostSample, northMostSample;
int             rowCount, columnCount, r, c;
//int	   	colInt, rowInt;
int         firstRow, lastRow;

int             lastProfile;
int             profileID[2], profileSize[2];
double          planCoords[2], localElevation, elevExtremea[2];

  // Define fields for A record.
  static char mapLabel[145], szText[256];
  static int i,j, flushingData, 
             DEMlevel,
             elevationPattern,
             groundSystem,
             groundZone,
             planeUnitOfMeasure,
             elevUnitOfMeasure,
             polygonSizes,
             profileDimension[2],
             accuracyCode;

  static double groundCoords[4][2],
                elevBounds[2],
					 localRotation,
                spatialResolution[3],
                ignore;

  static double zAdjust = 0.0, ScaleFactor;

  if( !InitializeInput( hwnd, FileName ) ) return FALSE;
  strcpy( szFileName, FileName );
  if( !LockNormalization ) dxnormalize = dynormalize = 0;
  PointsKept = 0;

  inFile.read( mapLabel, 144 );  mapLabel[144] = NULL;

// Do fixed length input for the A record.
// 6 characters for integers, 12 for floats, 24 for doubles.

  DEMfail = FALSE;
  DEMlevel = GetDEMint(6);
  elevationPattern = GetDEMint(6);
  groundSystem = GetDEMint(6);
  groundZone = GetDEMint(6);
  if( DEMfail ) return AbortReadGridFile(FileName);
  //sprintf( szText, " %i  %i  %i ", elevationPattern, groundSystem, groundZone);
  //NotifyUser( szText );

  for( i = 0; i<15; i++ ) ignore = GetDEMdouble(24); // ignore 15 double precision zeros.
  planeUnitOfMeasure = GetDEMint(6); // 3 = arc - seconds
  elevUnitOfMeasure = GetDEMint(6);  // 2 = metres. 
  polygonSizes = GetDEMint(6);
  if( DEMfail ) return AbortReadGridFile(FileName);
  if( planeUnitOfMeasure == 3. ) LatLonData = TRUE;
  //sprintf( szText, " %i  %i  %i ", planeUnitOfMeasure, elevUnitOfMeasure, polygonSizes );
  //NotifyUser( szText );
  if( elevUnitOfMeasure != 2 )
   {
     NotifyUser( " Problem reading DEM header. Is this a DEM file?");
     return AbortReadGridFile(FileName);
   }
  for( i = 0; i < 4; i++ )
     for( j = 0; j < 2; j++ ) groundCoords[i][j] = GetDEMdouble(24);
  elevBounds[0] = GetDEMdouble(24);
  elevBounds[1] = GetDEMdouble(24);
  localRotation = GetDEMdouble(24);
  accuracyCode = GetDEMint(6);
  //sprintf( szText, " Rotation %g  Accuracy code %i ", localRotation, accuracyCode );
  //NotifyUser( szText );
  for( i = 0; i < 3; i ++ )  spatialResolution[i] = GetDEMdouble(12);
  profileDimension[0] = GetDEMint(6);
  profileDimension[1] = GetDEMint(6);
  //sprintf( szText, " Dimensions: %i by %i ", profileDimension[0],
  //															profileDimension[1] );
  //NotifyUser( szText);
  inFile.ignore( 160); // Flush past the rest of the A record.

  if( profileDimension[0] != 1 || profileDimension[1] <5 ||
		profileDimension[1] > XMAX )
    {
		NotifyUser( " Problem reading DEM header record - perhaps this is not a DEM file?" );
		return AbortReadGridFile(FileName);
    }


  //  eastMost  = MAX(groundCoords[NE][0], groundCoords[SE][0]);
    westMost  = MIN(groundCoords[NW][0], groundCoords[SW][0]);
    northMost = MAX(groundCoords[NE][1], groundCoords[NW][1]);
    southMost = MIN(groundCoords[SW][1], groundCoords[SE][1]);
  //  eastMostSample =  ((int) (eastMost / spatialResolution[0]))
  //		     * (int) spatialResolution[0];
  //  westMostSample =  ((int) ((westMost + comp) / spatialResolution[0]))
  //		     * (int) spatialResolution[0] ;
	 comp = 0.0 ;
	 if (spatialResolution[0] == 3.0) comp   = 2.0 ;
    northMostSample = ((int) (northMost / spatialResolution[1]))
  		     * (int) spatialResolution[1] ;
    southMostSample = ((int) ((southMost + comp) / spatialResolution[1]))
  		     * (int) spatialResolution[1] ;

    //columnCount = (eastMostSample  - westMostSample)
	 //	  / (int) spatialResolution[0] + 1;
    columnCount = profileDimension[1];
    rowCount    = (northMostSample - southMostSample)
		  / (int) spatialResolution[1] + 1;
	if( DEMfail ) return AbortReadGridFile(FileName);

   //sprintf( szText, "Rows: %d Columns: %d Data range: %g to %g ",
	//	rowCount, columnCount, elevBounds[0], elevBounds[1] );
	//NotifyUser( szText );

	if( rowCount < 3 || rowCount > XMAX ||
		 columnCount < 3 || columnCount > XMAX )
		  {
          NotifyUser( " Invalid number of rows or columns. Is this a DEM file?"); 
			 return AbortReadGridFile(FileName);
		  }

	Zgrid.New( columnCount, rowCount );
	ZapZgrid();

// Fill in the x and y coordinates. The x coordinates will be overwritten
// by the profile record data unless input is terminated permaturely.

 ScaleFactor = 1.;
 if( LatLonData) ScaleFactor = 3600.;
 deltaX = spatialResolution[0]/ScaleFactor;
 temp = westMost/ScaleFactor;
 for( i = 0; i < columnCount; i++ ) {Zgrid.xset( i, temp-dxnormalize); temp += deltaX;}

 deltaY = spatialResolution[1]/ScaleFactor;   // Arc seconds
 temp = southMost/ScaleFactor;
 for( i = 0; i < rowCount; i++ )    {Zgrid.yset( i, temp-dynormalize); temp += deltaY;}

// Now process the profile records. Read them in free format.

   lastProfile = 0;
   //colInt = 1 ;
	//rowInt = 1 ;
   flushingData = FALSE;

   for (c = 1; c <= columnCount; c++)
	{
	 profileID[0] = GetDEMint(0);
    if( DEMfail ) return AbortReadGridFile(FileName);
	 profileID[1] = GetDEMint(0);
	 profileSize[0] = GetDEMint(0);
	 profileSize[1] = GetDEMint(0);
	 if( DEMfail ) return AbortReadGridFile(FileName);
	 planCoords[0] = GetDEMdouble(0);
    if( DEMfail ) return AbortReadGridFile(FileName);
	 planCoords[1] = GetDEMdouble(0);
	 localElevation = GetDEMdouble(0);
	 elevExtremea[0] = GetDEMdouble(0);
	 elevExtremea[1] = GetDEMdouble(0);
    //sprintf( szText, " Plan Coords, elev, .. %g %g %g %g %g ", planCoords[0],
    //      planCoords[1], localElevation, elevExtremea[0], elevExtremea[1]);
    //NotifyUser( szText );

	 if( DEMfail ) return AbortReadGridFile(FileName);
    temp = planCoords[0]/ScaleFactor;
    Zgrid.xset( c-1, temp-dxnormalize );

// Check for consistency and terminate input if problems.

    if ( profileID[1]-1 != lastProfile )
       {
         NotifyUser( " Error in profile record sequencing. Terminating DEM input. ");
	      break; 
       }
    lastProfile =  profileID[1];
    if( profileID[0] != 1 )
       {
        NotifyUser( "Error in profile record row identification. Terminating DEM input.");
        break;
       }
    if( profileSize[1] != 1 ) // Element should be precisely 1.
       {
         NotifyUser( " Error in profile record column number. Terminating DEM input. ");
         break;
       }

    firstRow = ((int) (planCoords[1] - southMostSample))
		  / (int) spatialResolution[1];
    firstRow++;      // firstRow comes to zero often/always without this????
    lastRow = firstRow + profileSize[0] - 1;

 //     read in the data for this column

    for (r = firstRow; r <= lastRow; r++ )
      {
        base = GetDEMint(0);
        if( r < 1 || r > rowCount )
         {
           if( !flushingData )
           {
             sprintf( szText,
             " Ignoring some profile data at column %i row %i", c, r );
             NotifyUser( szText );
             flushingData = TRUE;
           }
         }
        else
          {
            Zgrid.zset( c-1, r-1, (float) base +localElevation ) ;
            PointsKept++;
          }
        if( CancelLoadData ) break;
      }
   if( CancelLoadData ) break;

 }
  inFile.close();
  ShutdownGridLoad( zAdjust );
  // Adjust zratio to approximate true shape of the surface.. . .
  // One Nautical mile = 1 minute of arc = 1.852 km = 1852 metres.
  // If one degree = 60*1852 metres = 111120 metres
  // So Zratio is Height/111120 metres.  - but is is very float very unexciting. 
  //static int Zratio;
  //Zratio = (Zgrid.zmax()-Zgrid.zmin())/111120.*100.;
  //if( Zratio < 1 ) Zratio = 1;
  //PaintSurfaceRatio( Zratio );
  SetSurfaceTitle( hwnd, FileName );
  if( DEMZeroUndefined  ) SetZUndefined( 0.0 );
  strcpy( szTitle, mapLabel );
  szTitle[70] = NULL; // truncate title at 70 characters.
  PointsKept = 0;
  return TRUE;
}

//*****************************************************************
//         S e t  T e m p l a t e
//*****************************************************************
void SetTemplate( int nx, double xs, double xi,
						int ny, double ys, double yi, int LatLon )
{
 if( GridLock )
  { NotifyUser( "Attempt to set a grid template when grid lock is in effect");
    return; }
 int MaximumGridSize = Zgrid.MaximumXY();
 if( nx<3||ny<3||nx>MaximumGridSize||ny>MaximumGridSize )
	{
	  NotifyUser( "Failure establishing Template (Internal failure)");
	  return;
	}
  if( !LockNormalization) dxnormalize = xs;
  if( !LockNormalization) dynormalize = ys;

  Zgrid.New( nx, ny );

  SetGridXY( xs-dxnormalize, xi, ys-dynormalize, yi );
  Zgrid.zset(0,0, xs );      // Initialize max and min. for grid.
  Zgrid.zset(0,0, xs+xi );   // (Will be replaced during grid generation).
  Zgrid.zset(0,0, -99999. );
  LatLonData = LatLon;
  GridLock = TRUE;

  return;
}

//********************************************************************
//         L o a d   E R  M a p p e r  G r i d
//********************************************************************
int LoadERMapperGrid( HWND &hwnd, char FileName[] )
{
// Check filename suffix.
  int StringLength, Fail, nx, ny, period;
  char FileDescription[2000], *ptr;
  double Eastings, Northings, xstart, ystart, xincr, yincr, zmin, zAdjust;
  char czValue[4];
  float zValue; 
  strcpy( szFileName, FileName );
  StringLength = strlen( FileName );
  for( period = StringLength -1; period > 0; period--)
								  if( FileName[period] == '.' ) break;
  if( strnicmp( &FileName[period], ".ERS", 4 ) != 0 )
   {
     NotifyUser(" File name does not end in .ers??? " );
     return FALSE;
   }

//  P r o c e s s   D e s c r i p t o r   F i l e

  if( !InitializeInput( hwnd, FileName ) ) return FALSE;

  inFile.get( FileDescription, sizeof( FileDescription ), NULL );
  ptr = strstr( FileDescription, "Raster" );
  if( ptr == NULL )
     { NotifyUser( "ER Mapper file does not appear to be a raster type");
       inFile.close();
       return FALSE; }
  ptr = strstr( FileDescription, "IEEE4ByteReal" );
  if( ptr == NULL )
     { NotifyUser( "ER Mapper file does not appear to be type IEEE4ByteReal");
       inFile.close();
       return FALSE; }

  Fail = FALSE;
  ny = GetPairInt( FileDescription, "NrOfLines", Fail );
  nx = GetPairInt( FileDescription, "NrOfCellsPerLine", Fail ) ;
  xincr = GetPairFloat( FileDescription, "Xdimension", Fail );
  yincr = GetPairFloat( FileDescription, "Ydimension", Fail );
  Eastings = GetPairFloat( FileDescription, "Eastings", Fail );
  Northings = GetPairFloat( FileDescription, "Northings", Fail );

  inFile.close();
  if( Fail||(nx==0)||(ny==0)||(xincr==0.0)||(yincr==0.0) )
   {
     NotifyUser( "Error encountered reading ER Mapper File definition" );
     return FALSE;
   }
  Zgrid.New( nx, ny ) ;
  xstart = 0;
  if( !LockNormalization)
  {
    dxnormalize = Eastings + .5*xincr;
    dynormalize = (Northings - .5*yincr) - (ny-1)*yincr;
  }
  ystart = 0;
  SetGridXY( xstart, xincr, ystart, yincr );

//        P r o c e s s    B i n a r y    F i l e

  FileName[period] = NULL; // chop off ".ers"

  inFile.open( FileName, ios::binary );
  if( !inFile )
    { NotifyUser( IDS_NOINPUTFILE );
      return FALSE;
    }
  zmin = 0.0 ;
  PointsKept = 0;
  for( int j = ny-1; j >= 0 ; j-- )
   { for( int i = 0; i < nx; i++ )
     {
       inFile.read( czValue, 4 );
       if( inFile.eof() ) { NotifyUser( " Permature end reading binary file ");
                       return FALSE; }
       PointsKept++;
       memcpy( &zValue, czValue, 4 );
       if( zValue != -99999. )
         { Zgrid.zset( i, j, zValue ) ;
           if( zValue < zmin ) zmin = zValue;
         }
     }
  }
  zAdjust = 0.0;
  if( zmin < 0.0 )
   {
     zAdjust = zmin;
     Zgrid.Znew();
     for( int j = 0; j <ny ; j++ )
     { for( int i = 0; i < nx; i++ )
      {
       zValue = Zgrid.z(i,j);
       if( zValue != -99999. ) Zgrid.zset( i, j, zValue-zAdjust ) ;
      }
     }
   }

  inFile.close();

  FileName[period] = '.'; // restore ".ers"
  ShutdownGridLoad( zAdjust);
  SetSurfaceTitle( hwnd, FileName );
  return TRUE;
}

static int MaxTransducers,
           PingBlockCount,
           SXPBlockCount;
//******************************************************************
//         L o a d  S u b m e t r i x  S X P  D a t a
//******************************************************************
// Implementation time:
//   Intial boilerplate: 15-30 min.
static int ShutdownSubmetrix(HWND &hwnd,  char FileName[] )
{
  //NotifyUser( " Submetrix input done. \n Ping Blocks read = %i \n Maximum Transducers = %i\n Block count = %i",
  //             PingBlockCount, MaxTransducers, SXPBlockCount );
  return ShutdownInput( hwnd, FileName );
}
int LoadSubmetrixSXPData( HWND &hwnd, char FileName[] )
{
  // Definitions
static const int SXP_HEADER_DATA   = 0x01df01df;	// 2000 Series processed data file
static const int SBP_XYZA_PING		= 0x28; 		// Ping data for processed data xyza: auxiliary data
static const int SXF_MAXSAMPS = 0xff00;			// Samples per ping
static const int SXF_MAXCHANS = 4;					// Number of transducer channels
static const int SXF_MAXBLOCKSIZE = SXF_MAXSAMPS * SXF_MAXCHANS * 8;	// Max. size of a data block
//static const int MAX_TX_INFO = 4;					// Number of transducer blocks
static const int MAX_TX        = 15;				// Number of transducers
static const int MAX_TX_SLOTS  = MAX_TX + 1;		// Size of transducer array

static int numread;				// Number of bytes read from file
static int blocksize;				// Size of data block read
static int i, j;					// loop counters
static char databuff[SXF_MAXBLOCKSIZE];
static const bool Binary = true;
static int ScanPosition, HeaderType;
static int SoftwareVersion, FileFormatVersion;
static int NumberOfTransducers, PingNumber ;
static int NoSampsFile, NoSampsOrig;
static float z;
static char szText[300];

static const int PINGSIZE = 128,
                 TRANSDUCERSIZE = 128,
                 POINTSIZE = 40;


MaxTransducers = 0;
PingBlockCount = 0;
SXPBlockCount = 0;
  // For debugging.
  static bool debug;
  debug = false;

// Call this section "ConstructFilename?".


  ConstructFilename( FileName ); 
  if( !InitializeInput( hwnd, szFileName, Binary ) ) return FALSE;

  // Keep reading headers
  while( true )
  {
    inFile.read( databuff, 8 ); // read in the header.
    if( inFile.eof() ) // If eof check to see if more files to load. 
      {
        if( NoMoreFiles( FileName, Binary) ) return ShutdownSubmetrix( hwnd, FileName );
        inFile.read( databuff, 8 ) ;
        if ( inFile.eof() )  return ShutdownSubmetrix( hwnd, FileName );
      }
    SXPBlockCount += 1;
    // The size of the following data block is in the second part of the header
    memcpy( &blocksize, &databuff[4], 4 );
    memcpy( &HeaderType, &databuff[0], 4 );
    if( debug ) NotifyUser( " Header read: Blocksize = %i  ", blocksize );
    // Check against errors in size
    if ((blocksize <= 0) || (blocksize > (SXF_MAXBLOCKSIZE)))
     {
       NotifyUser( " Error reading Submetrix .sxp file. Bad blocksize = %i\n ", blocksize );
       return ShutdownSubmetrix( hwnd, FileName );
     }
    // Read the data block
    inFile.read( databuff, blocksize);
    numread = inFile.gcount();
    // File error
    if( inFile.fail() || (numread < blocksize))
    {
       //perror("Read error");
       NotifyUser(
       "Error reading Submetrix .sxp file. Blocksize = %i, Bytes read = %i ",
         blocksize, numread);
       return ShutdownSubmetrix( hwnd, FileName );
    }

    // End of file
    if( inFile.eof())
       {
          NotifyUser( "Unexpected end of file reading data block - input terminated.");
          return ShutdownSubmetrix( hwnd, FileName );
       }
    // The type of data block is in the first part of the header
    switch (HeaderType)
    {
      case SXP_HEADER_DATA:
        memcpy( &SoftwareVersion, &databuff[0], 4 );
        memcpy( &FileFormatVersion, &databuff[4], 4 );
        if( debug )
           NotifyUser( "Header Block: Software version:   %d\n File format version: %d",
                         SoftwareVersion, FileFormatVersion);
        break;

      case SBP_XYZA_PING:
        {
          PingBlockCount += 1;
          // The data block starts with the ping data
          // Sanity check on transducer data
          memcpy( &NumberOfTransducers, &databuff[56], 4 );
          memcpy( &PingNumber, &databuff[40], 4 );
          if( debug ) { sprintf( szText,
             "Ping data: Line name = \"%40s\"\n  Ping number = %i\n  No. of transducers = %i ",
             databuff, PingNumber, NumberOfTransducers );  NotifyUser( szText ); } 
          if( NumberOfTransducers > MAX_TX_SLOTS || NumberOfTransducers <= 0 )
           {
             NotifyUser(
             "Submetrix error. Bad ping data: Ping number = %i\n  No. of transducers = %i ",
             PingNumber, NumberOfTransducers );
             return ShutdownSubmetrix( hwnd, FileName );
           }
          if( NumberOfTransducers > MaxTransducers) MaxTransducers = NumberOfTransducers;
          // Read transducer data
          // Point to first transducer data block, by skipping the size of the ping data

           ScanPosition = PINGSIZE;   // End of the ping header.
           for (i = 0; i < NumberOfTransducers; i++)
           {
             memcpy( &NoSampsFile, &databuff[ScanPosition+36], 4 );
             memcpy( &NoSampsOrig, &databuff[ScanPosition+32], 4);
             if( debug ) NotifyUser( " No of samples processed = %i\n No of samples read = %i ",
                      NoSampsFile, NoSampsOrig );
             // Check the sample counts. The number of good samples must not exceed the total
             if (NoSampsFile > NoSampsOrig ||
                 NoSampsOrig > SXF_MAXSAMPS ||
                 NoSampsFile < 0 ||
                 NoSampsOrig < 0 )
             {
                NotifyUser( " Sample count inconsistency: \n No of samples processed = %i\n No of samples read = %i ",
                      NoSampsFile, NoSampsOrig );
                return ShutdownSubmetrix( hwnd, FileName );
              }
              // Read the samples
              // Note that the number of samples actually written is stored in
              // txerdata->m_noSampsFile.
              // m_noSampsOrig is the number of samples that were read in real-time. If the file
              // was recorded with the option of removing rejected samples enabled, then
              // m_noSampsFile will be less than m_noSampsOrig. Otherwise, the two numbers will be
              // equal, but some of the samples will be rejected. Use m_status to determine which
              // samples were rejected: a value of 1 is good, 0 is bad.
              ScanPosition += TRANSDUCERSIZE;
              for (j = 0; j < NoSampsFile; j++)
              {
                 static const int Xoffset = 16; // My x is their easting (y)
                 static const int Yoffset = 8;  // My y is their northing (x).
                 static const int Zoffset = 24;
                 static const int Aoffset = 30;
                 static unsigned short int Amplitude;
                 memcpy( &dx, &databuff[ScanPosition+Xoffset], 8);
                 memcpy( &dy, &databuff[ScanPosition+Yoffset], 8);
                 memcpy( &z,  &databuff[ScanPosition+Zoffset], 4);
                 memcpy( &Amplitude, &databuff[ScanPosition+Aoffset], 2);
                 dz = -z;
                 if( SubmetrixSelectZ == 2 ) dz = Amplitude;
                 StoreDataPoint();
                 if( debug )
                 {
                   NotifyUser( " Value stored: %g   %g    %g ",
                      (float)dx, (float)dy, z );
                   if( PointsRead > 10 ) return ShutdownSubmetrix( hwnd, FileName );
                 }
                 ScanPosition += POINTSIZE;
               }   // End for j
             // Point to the next transducer data block
              ScanPosition += POINTSIZE;
             } // end for i
          } // end case SBP_XYZA_PING:
          break;

          default:
            NotifyUser( "Unknown Submetrix datatype encountered - input terminated.");
            return ShutdownSubmetrix( hwnd, FileName );
    } // end switch.
  } // end while reading.
}

