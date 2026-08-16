//***************************************************************
//           I n p u t   C o m m a n d   F i l e
//
//                     Copyright (c) 2000-2007 by John Coulthard
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
// Feb. 9 - Mar 3/00: Initial implementation.
// Mar. 26/00: Ignored blank lines
//             Handles lines terminated by eof.
// Jun 14/00: Implemented WINDOWSIZE
// Jun 15/00: Implemented MARGIN
// Jun 25/00: Implemented EXITTIMER
// Jul 9/00: Implement CONTOURLINES OFF
// Nov 7-8/00: Implement CONTOUR (for custom contour lines).
// Mar 1/01: GetLastString added - will handle file names with embedded blanks.
// Mar 9/01: Allow * for file name on input - and will prompt interactively.
// Mar. 10/01: Add Colormap command.
// Apr. 2/01: Add copy grid to colour commands.
// May 4/01: Implement multiple word commands. SubmetrixZ data selection
// May 5/01: Recognize # to read previous file again.
// June 12/01: Implement edit colour map blended/monochrome...
// Sept/01: more commands - clean up code. Organize by menu function.
// Nov. 4/01: Implement write DXF, 3DFACE DXF, VRML
// Nov. 8/01: Implement Grid Automatic Grid Resolution on/off
// Nov. 9/01: Window size bug if command placed early in command file.
// Nov. 16/01: View colour legend & Axes implemented.
// Nov. 17/01: GridTemplate generates grid if scattered data available.
// and etc. and etc. and etc. 
//***************************************************************
#include <windows.h> 
#include <iostream>
#include <fstream>
#include <stdio.h> 
#include "utilitys.h"
#include "loaddata.h"
#include "surfgrid.h"
#include "scatdata.h"
#include "rc.h"
#include "gridres.h"
#include "grid.h"
#include "xpand.h"
#include "gridview.h"
#include "quikgrid.h"
#include "paintcon.h"
#include "condraw.h"
#include "grdwrite.h"
#include "rotate.h"
#include "dxfface.h"
#include "dxfwrite.h"
#include "vrml.h"
using namespace std;
 HINSTANCE hInst;
void FlipOption( HMENU hMenu, int &option, UINT MenuId );

static ifstream inFile( " ", ios::in );
static char szCommand[500],
            szWord[50], 
            szFileName[2000],
            szSubmetrixFileName[2000],
            InputLine[500],
            szLoadString[255],
            szPrint[1000];
static int ScanPosition, LinesRead;

//extern int LabelNthLine;
extern int ColouredGrid; 
extern int LabelNDigits;
extern int GridGeneratedOK, GridLock;
extern ScatData OutLine;
extern int LockNormalization;
extern int Zooming;
extern float Margin, shift;
extern ContourOptionsType DrawingOptions;
extern UINT PaintState;
extern int CaptureBitmapAndExit;
extern int CaptureMetaFileAndExit;
extern COLORREF ContourPen;
extern int SubmetrixSelectZ;
extern int FlipXY;
extern int ReverseSignOfZ;
extern int BlendedColours;
extern COLORREF ShadePen;
extern int ReverseColourMap;
extern int DisplayVolumeDifference;
extern int GridResAuto;
extern int IncludeColourLegend;
extern int InputFieldPosition[4];
extern int IgnoreComments;
extern int TextSize;
extern float ZmaxLabel, ZminLabel;
extern char szTitle[];
extern int ColourContours;

struct CommandTableType
 {  char *Command;
    int  Map; };

//***************************************************************
//   C o m m a n d   M a p   T a b l e
//***************************************************************
// Define a macro to assign a unique constant to each command.
//  - assign the source line number 
#define NEXT __LINE__ 
static const int
// File... commands
 FILEINPUTOUTLINE = NEXT,   // File input outline file.
 FILEINPUTMETRIC = NEXT,
 FILEINPUTLATLON = NEXT,
 FILEINPUTSUBMETRIX = NEXT,
 FILELOADQUIKGRID = NEXT,
 FILEWRITEGRIDQG = NEXT,
 FILESAVEGRIDGRD = NEXT,
 FILEEXPORTDXF = NEXT,
 FILEEXPORT3DFACE = NEXT,
 FILEEXPORTVRML = NEXT,
 FILECLEAROUTLINE = NEXT,
 TERMINATE = NEXT,

 // View... commands
 VIEWLATLON = NEXT,
 VIEW2D = NEXT,
 VIEW3D = NEXT,
 VIEWHIDDENCOLOURGRID = NEXT,
 VIEWNOGRID = NEXT,
 VIEWTRANSPARENTGRID = NEXT,
 VIEWHIDDENGRID = NEXT,
 VIEWVOLUMEDIFFERENCE = NEXT,
 VIEWCONTOURLABELS = NEXT,
 VIEWCONTOURLINES = NEXT,
 VIEWCOLOURLEGEND = NEXT,
 VIEWAXES = NEXT,
 VIEWTEXTSIZE = NEXT,
 VIEWZLABELS = NEXT,

 // Edit... commands
 EDITCOLOURMAP = NEXT,
 EDITCONTOURLINES = NEXT,
 EDITCONTOURLINESCOLOUR = NEXT,
 EDITINPUTSUBMETRIXZ = NEXT,
 EDITGRIDGENERATIONDISTANCE = NEXT,
 EDITGRIDGENERATIONEDGE = NEXT,
 EDITGRIDGENERATIONSAMPLE = NEXT,
 EDITINPUTREVERSEXY = NEXT,
 EDITINPUTREVERSEZ = NEXT,
 EDITINPUTSAMPLENTH = NEXT,
 EDITINPUTPOSITION = NEXT,
 EDITANGLE = NEXT,
 EDITCOLOURMAPBLENDED = NEXT,
 EDITCOLOURMAPMONO = NEXT,
 EDITCOLOURMAPMONOCOLOUR = NEXT,
 EDITCOLOURMAPREVERSE = NEXT,
 EDITZRATIO = NEXT,
 EDITTITLE = NEXT,

// Grid... commands
 GRIDLOCK = NEXT,
 AUTOMATICGRIDRESOLUTION = NEXT,

// Function... commands
 FUNCTIONCOPYGRIDTOSPEED = NEXT,
 FUNCTIONCOPYGRIDTODIRECTION = NEXT,
 FUNCTIONCOPYGRIDTOCOLOUR = NEXT,
 FUNCTIONCOLOURLAYERMINUSGRID = NEXT,

// Commands with no equivalent menu action
 WINDOWSIZE = NEXT,
 MARGIN = NEXT,
 EXITTIMER = NEXT,
 CAPTUREBITMAPANDEXIT = NEXT,
 CAPTUREMETAFILEANDEXIT = NEXT,
 DESTROYMENU = NEXT,
 DEFINECONTOURLINE = NEXT,
 GRIDTEMPLATE = NEXT,

 ENDOFLIST = NEXT;

static CommandTableType CommandTable[] =
   {
     {"Outline",          FILEINPUTOUTLINE},
     {"FileInputOutline", FILEINPUTOUTLINE},
     {"MetricInput",     FILEINPUTMETRIC},
     {"FileInputMetric", FILEINPUTMETRIC},
     {"CopyGridToSpeed", FUNCTIONCOPYGRIDTOSPEED},
     {"FunctionCopyGridSpeed",     FUNCTIONCOPYGRIDTOSPEED},
     {"CopyGridToDirection",       FUNCTIONCOPYGRIDTODIRECTION},
     {"FunctionCopyGridDirection", FUNCTIONCOPYGRIDTODIRECTION},
     {"LatLonInput",      FILEINPUTLATLON},
     {"FileInputLat/Lon", FILEINPUTLATLON},
     {"FileInputLatLon",  FILEINPUTLATLON},
	  {"GridLock", GRIDLOCK},
     {"GridLockGridTemplate", GRIDLOCK},
     {"GridLockGrid", GRIDLOCK}, 
	  {"GridTemplate", GRIDTEMPLATE},
     {"EditGridTemplate", GRIDTEMPLATE},
     {"GridAutomaticGridResolution", AUTOMATICGRIDRESOLUTION},
	  {"ContourLines",     EDITCONTOURLINES},
     {"EditContourLines", EDITCONTOURLINES},
     {"EditContourLinesColour", EDITCONTOURLINESCOLOUR},
     {"LatLon",                 VIEWLATLON},
     {"ViewLatitude/Longitude", VIEWLATLON},
     {"ViewLatitudeLongitude",  VIEWLATLON},
     {"ViewLat/Lon",            VIEWLATLON},
     {"ViewLatLon",             VIEWLATLON},
     {"ClearOutline",     FILECLEAROUTLINE},
     {"FileClearOutline", FILECLEAROUTLINE},
     {"WindowSize",     WINDOWSIZE},
     {"EditWindowSize", WINDOWSIZE},
	  {"Margin",     MARGIN },
     {"EditMargin", MARGIN },
	  {"DieTimer",  EXITTIMER },  // obsolete - undocumented
     {"ExitTimer", EXITTIMER }, // obsolete - undocumented.
     {"CaptureExit",    CAPTUREBITMAPANDEXIT },
     {"WindowCopyClipboardBitmapExit", CAPTUREBITMAPANDEXIT },
     {"WindowCopyClipboardMetaFileExit", CAPTUREMETAFILEANDEXIT },
	  {"Contour",           DEFINECONTOURLINE },
     {"DefineContourLine", DEFINECONTOURLINE },
     {"CreateContour", DEFINECONTOURLINE },
	  {"WriteGridQG",          FILEWRITEGRIDQG }, // Compatibility with old.
     {"FileSaveGridQuikGrid", FILEWRITEGRIDQG },
     {"FileSaveGridQuikGridFile", FILEWRITEGRIDQG },
     {"FileSaveGridGRD", FILESAVEGRIDGRD },
     {"FileSaveGridGRDFile", FILESAVEGRIDGRD },
     {"FileExportDXFFile",  FILEEXPORTDXF },
     {"FileExportDXF3DFaceFile", FILEEXPORT3DFACE },
     {"FileExportVRMLFile", FILEEXPORTVRML },
	  {"Terminate", TERMINATE },
     {"FileExit",  TERMINATE },
	  {"DestroyMenu",    DESTROYMENU }, // Renamed to Restricted Menu.
     {"RestrictedMenu", DESTROYMENU },
     {"ColourMap",                   EDITCOLOURMAP },
     {"ColorMap",                    EDITCOLOURMAP},
     {"EditColourFeaturesColourMap", EDITCOLOURMAP},
     {"EditColourMap",               EDITCOLOURMAP}, 
     {"Zratio",                  EDITZRATIO },
     {"EditVerticalScaleFactor", EDITZRATIO },
     {"CONTOURLABELS",         VIEWCONTOURLABELS},
     {"ViewLabelContourLines", VIEWCONTOURLABELS},
     {"ViewContourLabels",     VIEWCONTOURLABELS},
     {"SubmetrixInput",     FILEINPUTSUBMETRIX},
     {"FileInputSubmetrix", FILEINPUTSUBMETRIX},
     {"FileInputSubmetrixFile", FILEINPUTSUBMETRIX}, 
     {"FunctionCopyGridColour",   FUNCTIONCOPYGRIDTOCOLOUR},
     {"FunctionColourColourGrid", FUNCTIONCOLOURLAYERMINUSGRID},
     {"CopyGridToColour", FUNCTIONCOPYGRIDTOCOLOUR},
     {"CopyGridToColor",  FUNCTIONCOPYGRIDTOCOLOUR},
     {"EditInputSubmetrixZ", EDITINPUTSUBMETRIXZ},
     {"EditGridGenerationDistance", EDITGRIDGENERATIONDISTANCE},
     {"EditGridGenerationEdge",   EDITGRIDGENERATIONEDGE},
     {"EditGridGenerationSample", EDITGRIDGENERATIONSAMPLE},
     {"EditAngle", EDITANGLE},
     {"EditInputReverseXY", EDITINPUTREVERSEXY},
     {"EditInputReverseZ",  EDITINPUTREVERSEZ},
     {"EditInputSample", EDITINPUTSAMPLENTH},
     {"EditInputPosition", EDITINPUTPOSITION},
     {"View2d", VIEW2D},
     {"View3d", VIEW3D},
     {"ViewHiddenColourGrid", VIEWHIDDENCOLOURGRID},
     {"ViewHiddenColouredGrid", VIEWHIDDENCOLOURGRID},
     {"ViewTransparentGrid", VIEWTRANSPARENTGRID },
     {"ViewNoGrid",     VIEWNOGRID },
     {"ViewHiddenGrid", VIEWHIDDENGRID},
     {"ViewVolumeDifference", VIEWVOLUMEDIFFERENCE},
     {"ViewContourLines", VIEWCONTOURLINES},
     {"ViewContours",     VIEWCONTOURLINES},
     {"ViewColourLegend", VIEWCOLOURLEGEND},
     {"ViewAxes", VIEWAXES},
     {"ViewTextSize", VIEWTEXTSIZE},
     {"ViewZaxisLabels", VIEWZLABELS},
     {"ViewZaxesLabels", VIEWZLABELS},
     {"FileLoadQuikGridGrid", FILELOADQUIKGRID},
     {"FileLoadQuikGridGridFile", FILELOADQUIKGRID},
     {"EditColourMapBlended", EDITCOLOURMAPBLENDED},
     {"EditColourMapMonochrome", EDITCOLOURMAPMONO},
     {"EditColourMapMonochromeColour", EDITCOLOURMAPMONOCOLOUR},
     {"EditColourMapReverse", EDITCOLOURMAPREVERSE},
     {"EditColourMapReverseColourMap", EDITCOLOURMAPREVERSE},
     {"Title", EDITTITLE},
     {"EditTitle", EDITTITLE},
   };

static const int NumberOfCommands = sizeof(CommandTable)/sizeof(CommandTable[0]);
//*****************************************************************
//   C o m m a n d  M a p
//*****************************************************************
static int CommandMap( char szText[] )
{
  for ( int i = 0; i < NumberOfCommands; i++ )
  {
	 if(  stricmp( CommandTable[i].Command, szText ) == 0 )
                         return CommandTable[i].Map;
  }
 return 0;
}
//*****************************************************************
//          G e t   S t r i n g
//*****************************************************************
// Scans string InputLine starting at "start" and returns next
// string delimited by whitespace is placed in found.
// if string start is a " scan for next " .
static int GetString(  char found[], int size )
{
  static int i, StringLength, LineLength;
  found[0] = ' '; found[1] = NULL;
  LineLength = strlen( InputLine );
  if( ScanPosition >= LineLength ) return FALSE;
  if( InputLine[ScanPosition]==NULL ) return FALSE;
  for( i = ScanPosition; i < LineLength+1; i++ ) if( !isspace(InputLine[i]) ) break;
  ScanPosition = i;
  if( i >= LineLength )return FALSE;
  if( InputLine[i] == NULL )  return FALSE;
  StringLength = strcspn( &InputLine[i], " ,\t" ) ;
  if( StringLength < 1 )return FALSE;
  if( StringLength > (size-2)  )
    {NotifyUser( IDS_COMMANDWORDLARGE ); return FALSE;}
  strncpy( found, &InputLine[i], StringLength ) ;
  found[StringLength] = NULL;
  found[StringLength+1] = NULL;
  ScanPosition += StringLength+1;
  return  TRUE;
}

//*****************************************************************
//          G e t   L a s t   S t r i n g
//*****************************************************************
// Scans string InputLine starting at "start" and returns next
// string terminated by the end of the line. It will trim trailing
// blanks but otherwise will return embedded blanks.
// Used to return file names with embedded blanks.
static int GetLastString(  char found[], int size )
{
  int i, Last, StringLength, LineLength;
  found[0] = ' '; found[1] = NULL;
  LineLength = strlen( InputLine );
  if( ScanPosition >= LineLength ) return FALSE;
  if( InputLine[ScanPosition]==NULL ) return FALSE;
  for( i = ScanPosition; i < LineLength+1; i++ ) if( !isspace(InputLine[i]) ) break;
  ScanPosition = i;
  if( i >= LineLength )return FALSE;
  if( InputLine[i] == NULL )  return FALSE;
  // if enclosed in quotes look for enclosing quote.
  // Also consider: keep on going for more strings and contruct an explorer like
  // multiple file string "string\0string\0string\0\0 .
  
  if( InputLine[i] == '\"' )
       {
         i += 1;
         StringLength = strcspn( &InputLine[i], "\"" );
       }
  else     // Scan backward from end of line for non blank character. 
       {
          for( Last = LineLength-1; Last > i; Last--)
             if( !isspace( InputLine[Last])) break;
           StringLength = Last - i + 1;
       }
  if( StringLength < 1 )return FALSE;
  if( StringLength > size  )
  {NotifyUser( "Command file internal error - string too large?"); return FALSE;}
  strncpy( found, &InputLine[i], StringLength ) ;
  found[StringLength] = NULL;
  found[StringLength+1] = NULL;
  ScanPosition = LineLength;
  return  TRUE;
}

//*****************************************************************
//     R e p o r t   F l o a t   F a i l
//*****************************************************************
// Internal service routine. 
static void ReportFloatFail(  char szField[] )
{
   NotifyUser( IDS_COMMANDFLOATERROR, szField, InputLine );
}


//*****************************************************************
//     C o n v e r t   I n t e g e r
//*****************************************************************
// Internal service routine.
static bool ConvertInteger( int &result, char szText[] )
{
 if( !StringIsInteger( szText ) )
   {
     NotifyUser( IDS_COMMANDINTEGERERROR, szText, InputLine );
     return false;
   }
 result = atoi( szText );
 return true;
}

//*****************************************************************
//     C o n v e r t   R G B Integer
//*****************************************************************
// Internal service routine.
static bool ConvertRGBInteger( int &result,  char szText[] )
{
 if( !ConvertInteger( result, szText ) ) return false;

 if( result < 0 || result > 255 )
   {
     NotifyUser( IDS_COMMANDRBGERROR, szText, InputLine );
     return false;
   }
 return true;
}
 
//*****************************************************************
//     C o n v e r t   F l o a t
//*****************************************************************
// Internal service routine.
static bool ConvertFloat( double &result,  char szText[] )
{
 static char *endptr;
 result = strtod(szText, &endptr);
 if( *endptr == NULL || *endptr == ','||*endptr=='\t' ) return true;
 if( *endptr == 'D' ) *endptr = 'e';  // Perhaps exponent is +Dnn??
 result = strtod(szText, &endptr);    // so change to e and try again.
 if( *endptr == NULL || *endptr == ','||*endptr=='\t' ) return true;
 ReportFloatFail( szText );
 return false;
}
//*******************************************************************
//    S e t   C o m m a n d   T i t l e   B a r
//*******************************************************************
static void SetCommandTitleBar( HWND &hwnd, int MessageNumber, char szArg[]=" ")
{

 if( MessageNumber == 0 ){ SetWindowText( hwnd, szArg ); return; }
 if( !LoadString( (HINSTANCE)hInst, MessageNumber, szLoadString, 255 ) )
                                             NotifyUserError( MessageNumber );
 sprintf( szPrint, szLoadString, szArg );
 SetWindowText( hwnd, szPrint );
}
//*****************************************************************
//   G e n e r a t e   G r i d
//*****************************************************************
static int GenerateGrid( HWND &hwnd, HMENU &hMenu )
{

 if( ScatterData.Size() < 3)
				{
              NotifyUser( IDS_GENGRIDNODATA );
				  SetStartupState( hwnd );
              RestoreCursor();
				  return FALSE;
            }

 SetCommandTitleBar( hwnd, IDS_GENERATINGGRIDFROM, szFileName );
 ZapZgrid();
 InitializeZgrid();
 XpandInit( Zgrid, ScatterData );
 GridResMenus( hwnd, hMenu );
 Xpand( Zgrid, ScatterData );
 GridGeneratedOK = TRUE;
 SetWindowText( hwnd, szFileName );
 ColourLayerVolume();
 return TRUE;
}

//*****************************************************************
//   F a i l
//*****************************************************************
static int Fail( HWND &hwnd, char szInfo[]=" " )
{
  NotifyUser( IDS_COMMANDFAIL, szInfo, LinesRead, ScanPosition, szWord, InputLine );
  RestoreCursor();
  inFile.close();

  SetCommandTitleBar( hwnd, IDS_COMMANDTERMINATED );

  if( !GridGeneratedOK ) SetStartupState( hwnd );
  return FALSE;
}
static int Fail( HWND &hwnd, int MessageNumber )
{
  static char szInfo[256];
  if (!LoadString((HINSTANCE)hInst, MessageNumber, szInfo, 255))
                                             NotifyUserError( MessageNumber );
  NotifyUser( IDS_COMMANDFAIL, szInfo, LinesRead, ScanPosition, szWord, InputLine );
  RestoreCursor();
  inFile.close();

  SetCommandTitleBar( hwnd, IDS_COMMANDTERMINATED );

  if( !GridGeneratedOK ) SetStartupState( hwnd );
  return FALSE;
}
//*****************************************************************
//         G e t    C o m m a n d
//*****************************************************************
// This routine assembles a unique command identifier.
// e.g. Edit Input SubmetrixZ
// becomes "EditInputSubmetrixZ" as returned in szCommand.
// preamble words like Edit get assembled.
static int GetCommand(  )
{

 static int PreambleType;
 static const int Copy = 1,
                  Noise = 2,
                  Last = 3,
                  Comment = 4,
                  Unknown = 5;
 static CommandTableType PreambleTable[] =
   {
     {"//", Comment},
     {".sxp", Noise},
     {"=", Noise},
     {"-", Noise}, 
     {"2D", Copy},
     {"3D", Copy},
     {"3DFACE", Copy},
     {"a", Noise},
     {"an", Noise},
     {"and", Noise},
     {"angle", Copy}, 
     {"as", Noise},
     {"automatic", Copy},
     {"axes", Copy},
     {"axis", Copy},
     {"bitmap", Copy }, 
     {"blended", Copy},
     {"Capture", Copy},
     {"CaptureAndExit", Last},
     {"clear", Copy},
     {"clearoutline", Last},
     {"clipboard", Copy},
     {"colour", Copy},
     {"coloured", Copy}, 
     {"ColourMap", Last},
     {"ColorMap", Last },
     {"contour", Copy},
     {"ContourLabels", Last },
     {"ContourLines", Last},
     {"Contours", Copy }, 
     {"Copy", Copy},
     {"CopyGridToColour", Last},
     {"CopyGridToColor", Last},
     {"CopyGridToDirection", Last},
     {"CopyGridToSpeed", Last},
     {"Current", Noise},
     {"Cutoff", Noise },
     {"data", Noise},
     {"Define", Copy},
     {"DestroyMenu", Last },
     {"dietimer", Last},
     {"difference", Copy},
     {"direction", Copy},
     {"distance", Copy},
     {"DXF", Copy},
     {"Edge", Copy}, 
     {"Edit", Copy},
     {"Every", Noise},
     {"Exit", Copy},
     {"exittimer", Last},
     {"export", Copy },
     {"Face", Copy},
     {"Factor", Copy},
     {"Features", Copy},
     {"Field", Noise}, 
     {"File", Copy},
     {"format", Noise}, 
     {"from", Noise},
     {"Function", Copy},
     {"Generation", Copy},
     {"GRD", Copy}, 
     {"Grid", Copy},
     {"Gridlock", Last},
     {"GridTemplate", Last},
     {"Hidden", Copy },
     {"in", Noise},
     {"Input", Copy},
     {"label", Copy},
     {"labels", Copy}, 
     {"latLon", Copy},
     {"latloninput", Last },
     {"layer", Noise},
     {"Legend", Copy},
     {"Line", Copy},
     {"lines", Copy},
     {"load", Copy},
     {"lock", Copy},
     {"Map", Copy}, 
     {"Margin", Copy},
     {"Meta", Copy},
     {"MetaFile", Copy}, 
     {"metric", Copy},
     {"metricinput", Last},
     {"minus", Noise}, 
     {"monochrome", Copy},
     {"no", Copy},
     {"of", Noise},
     {"options", Noise},
     {"outline", Copy},
     {"Output", Copy},
     {"point", Noise},
     {"points", Noise},
     {"position", Copy },
     {"QuikGrid", Copy },
     {"resolution", Copy },
     {"RestrictedMenu", Last},
     {"Reverse", Copy},
     {"Sample", Copy},
     {"Save", Copy},
     {"scale", Copy},
     {"scattered", Noise},
     {"sensitivity", Noise},
     {"sign", Noise},
     {"size", Copy},  
     {"speed", Copy},
     {"submetrix", Copy},
     {"submetrixinput", Copy},
     {"surface", Noise},
     {"surfer", Noise},
     {"Template", Copy},  
     {"Terminate", Last},
     {"Text", Copy},
     {"Textsize", Copy},
     {"th", Noise},
     {"the", Noise},
     {"timer", Copy},
     {"title", Last},
     {"to", Noise},
     {"transparent", Copy},
     {"view", Copy},
     {"vertical", Copy},
     {"volume", Copy},
     {"VRML", Copy}, 
     {"windowsize", Copy },
     {"window", Copy },
     {"with", Noise}, 
     {"WriteGridQG", Last},
     {"x", Copy},
     {"xy", Copy},
     {"y", Copy}, 
     {"z", Copy },
     {"zratio", Last } 
   };

static const int NumberOfPreambles = sizeof(PreambleTable)/sizeof(PreambleTable[0]);
static int OldScanPosition;
 szCommand[0] = NULL;
 while ( true )
 {
   OldScanPosition = ScanPosition;
   if( !GetString( szWord, sizeof( szWord ) ) ) // Probably eol so all done.
      {
        ScanPosition = OldScanPosition;
        return 1;
      }
   if( stricmp( szWord, "color"   ) == 0 ) strcpy( szWord, "colour" );
   if( stricmp( szWord, "colored" ) == 0 ) strcpy( szWord, "coloured" );
   PreambleType = Unknown;
   for ( int i = 0; i < NumberOfPreambles; i++ )
     {
	   if(  stricmp( PreambleTable[i].Command, szWord ) == 0 )
                        PreambleType = PreambleTable[i].Map;
     }
   switch (PreambleType)
	 {
     case Copy:
     strcat( szCommand, szWord ); // Concatenate to command string and continue.
     break;

     case Noise: // Ignore
     break;

     case Last: // all done add word to command string and return.
     strcat( szCommand, szWord );
     return 1;

     case Comment:  // all done - but back up and truncate at the comment.
     ScanPosition = OldScanPosition;
     InputLine[ScanPosition] = NULL;
     InputLine[ScanPosition+1] = NULL;
     return 1;

     case Unknown:
     if( szCommand[0] == NULL ) return 0; // First word is bad. 
     ScanPosition = OldScanPosition; // back up....
     return 1;

     default:  
     NotifyUser( "Internal error scanning command file - command file processing terminated" );
     return 0;
    }
 }
}

//*****************************************************************
//   N e x t    S t r i n g   O n
//*****************************************************************
static bool NextStringON()
{
 static char szString[10];
 if( !GetString( szString, sizeof(szString) )) return true; // if missing true.
 if( (stricmp( szString, "ON" ) == 0 ))        return true;
 return false;
}

//****************************************************************
//         F i l e n a m e C o p y
//****************************************************************
// File names must be terminated by a double null.
static void FilenameCopy( char target[], char source[] )
{
  strcpy( target, source) ;
  target[ strlen( source )+1 ] = NULL ;  // append a second null. 
}
//*****************************************************************
//   I n p u t  C o m m a n d   F i l e
//*****************************************************************
int InputCommandFile( HWND &hwnd, char FileName[], HMENU &hMenu )
{
   static char szString[500];
   static float zAdjust;
   static COLORREF Colour;
   static double ContourValue, dResult;
   static int ContourBold, ContourLabel, r, g, b, iResult, i;
   static char szContourLabel[50];
   static UINT Checked;
   
  inFile.open( FileName, ios::in );
  if( !inFile )
    { NotifyUser( IDS_OPENINPUTFILEFAIL, FileName );
      return FALSE;
    }
  SetWaitCursor();
  LinesRead = 0;
  szFileName[0]=NULL; szFileName[1]=NULL;
  szSubmetrixFileName[0] = NULL; szSubmetrixFileName[1] = NULL;
  while (TRUE)
  {
	 ScanPosition = 0;
	 LinesRead++;
	 inFile.getline( InputLine, sizeof( InputLine) );
    InputLine[sizeof(InputLine)-1] = NULL;
	 if( StringIsBlank( InputLine )&& inFile.eof() ) break;
    if( StringIsBlank( InputLine ) ) continue;

	 if( !GetCommand( ) ) return Fail(hwnd);
    if( StringIsBlank (szCommand ) ) continue;
    strcpy( szWord, " ");
	 switch (CommandMap( szCommand))
	 {
     // File... commands
       case FILEINPUTMETRIC:

		   GetLastString( szString, sizeof(szString) );
         // If name is '#' use previous file name (leave previous name alone).
         if( strcmp( szString, "#" ) != 0 ) FilenameCopy( szFileName, szString);
         
         // If the file name is * prompt for the file.
         if( strcmp( szString, "*" ) == 0 || StringIsBlank( szString) )
           {
             szFileName[0] = NULL;
             if( !GetReadFileMultiple( szFileName,  hwnd, "Open metric XYZ data file",
                  "All files\0*.*\0\0" ) ) return Fail(hwnd);
           }
         if( !LoadFileMetricData( hwnd, szFileName ) ) return Fail(hwnd);
         if( !GenerateGrid( hwnd, hMenu ) ) return Fail(hwnd);
         break;

       case FILEINPUTSUBMETRIX:
		   GetLastString( szString, sizeof(szString) );
         // If name is '#' use previous file name (leave previous name alone).
         if( strcmp( szString, "#" ) != 0 ) FilenameCopy( szSubmetrixFileName, szString);
         if( strcmp( szString, "*" ) == 0||StringIsBlank( szString )) // If * prompt for file name.
           {
             szFileName[0] = NULL;
             if( !GetReadFileMultiple( szSubmetrixFileName,  hwnd, "Open Submetrix .sxp data file",
                  "SXP files\0*.sxp\0All files\0*.*\0\0" ) ) return Fail(hwnd);
           }

         if( !LoadSubmetrixSXPData( hwnd, szSubmetrixFileName ) ) return Fail(hwnd);
         if( !GenerateGrid( hwnd, hMenu ) ) return Fail(hwnd);
         break;

       case FILEINPUTLATLON:
		   GetLastString( szString, sizeof(szString) );
         // If name is '#' use previous file name (leave previous name alone).
         if( strcmp( szString, "#" ) != 0 ) FilenameCopy( szFileName, szString);
         if( strcmp( szString, "*" ) == 0|| StringIsBlank( szString ) )
           { if( !GetReadFile( szFileName,  hwnd, "Open Lat/Lon data file" ) ) return Fail(hwnd);}
         if( !LoadFileLatLonData( hwnd, szFileName ) ) return Fail(hwnd);
         if( !GenerateGrid( hwnd, hMenu ) ) return Fail(hwnd);
         break;

       case FILELOADQUIKGRID:
         SetWindowText( hwnd, "Processing Load QuikGrid grid command" );
         GetLastString( szFileName, sizeof(szFileName) );
         if( strcmp( szFileName, "*" ) == 0 || StringIsBlank( szFileName ) )
           { if( !GetReadFile( szFileName,  hwnd, "Open QuikGrid Grid file" ) ) return Fail(hwnd);}
         if( !LoadGridData( hwnd, szFileName ) ) return Fail(hwnd);
		   GridGeneratedOK = TRUE;
         break;

		 case FILEINPUTOUTLINE:
         SetWindowText( hwnd, "Processing Input Outline Data command" );
         GetLastString( szFileName, sizeof(szFileName) );
         if( strcmp( szFileName, "*" ) == 0 || StringIsBlank( szFileName ) )
         	{
              szFileName[0] = NULL;
              if( !GetReadFileMultiple( szFileName,  hwnd, "Open outline data file",
                                  "All files\0*.*\0\0" ) ) return Fail(hwnd);
            }
         if( !LoadFileOutlineData( hwnd , szFileName )) return Fail(hwnd);
		   break;

       case FILECLEAROUTLINE:
		   OutLine.Reset();
         LockNormalization = FALSE;
		   break;

		 case FILEWRITEGRIDQG:
         SetWindowText( hwnd, "Processing Save Grid in QuikGrid format command" );
			GetLastString( szFileName, sizeof(szFileName) );
         // If the file name is * or blank prompt for the file.
         if( strcmp( szFileName, "*" ) == 0|| StringIsBlank( szFileName) )
           {
             szFileName[0] = NULL;
             if( !GetSaveFile( szFileName, hwnd, "Save grid data in QuikGrid format to file" ) )
                    return Fail(hwnd);
           }
			if( !SaveGridData( szFileName )) return Fail(hwnd);

			break;

       case FILESAVEGRIDGRD:
         SetWindowText( hwnd, "Processing Save Grid in grd format command" );
			GetLastString( szFileName, sizeof(szFileName) );
         // If the file name is * or blank prompt for the file.
         if( strcmp( szFileName, "*" ) == 0|| StringIsBlank( szFileName) )
           {
             szFileName[0] = NULL;
             if( !GetSaveFile( szFileName, hwnd, "Save grid in GRD format to file",
                  "GRD files\0*.grd\0All files\0*.*\0\0", "grd"  ) )
                  return Fail(hwnd);
           }
         if( !OutputGRDfile( szFileName )) return Fail(hwnd);
			break;

       case FILEEXPORTDXF:
         SetWindowText( hwnd, "Processing Export a DXF file command" );
         GetLastString( szFileName, sizeof(szFileName) );
         // If the file name is * or blank prompt for the file.
         if( strcmp( szFileName, "*" ) == 0|| StringIsBlank( szFileName) )
           {
             szFileName[0] = NULL;
             if( !GetDXFfilename( hwnd, szFileName ) ) return Fail(hwnd);
           }
         OutputDXFfile( DrawingOptions, PaintState, szFileName ) ;
         break;

       case FILEEXPORT3DFACE:
         SetWindowText( hwnd, "Processing Export a DXF 3D FACE file command" );
         GetLastString( szFileName, sizeof(szFileName) );
         // If the file name is * or blank prompt for the file.
         if( strcmp( szFileName, "*" ) == 0|| StringIsBlank( szFileName) )
           {
             szFileName[0] = NULL;
             if( !GetDXF3DFaceFileName( hwnd, szFileName ) ) return Fail(hwnd);
           }
         OutputDXF3DFaceFile( hwnd, szFileName ) ;
         break;

       case FILEEXPORTVRML:
         SetWindowText( hwnd, "Processing Export a VRML file command" );
         GetLastString( szFileName, sizeof(szFileName) );
         // If the file name is * or blank prompt for the file.
         if( strcmp( szFileName, "*" ) == 0|| StringIsBlank( szFileName) )
           {
             szFileName[0] = NULL;
             if( !GetVRMLFileName( hwnd, szFileName ) ) return Fail(hwnd);
           }
         OutputVRMLFile( szFileName ) ;
         break;

     // View... commands
       case VIEWHIDDENCOLOURGRID:
         if( NextStringON() )   ColouredGrid = TRUE;
         else                   ColouredGrid = FALSE;
		   if( ColouredGrid )
		    {
			 DrawingOptions.grid = DrawingOptions.hiddengrid = FALSE;
			 CheckMenuItem( hMenu, IDM_GRIDPLOT, MF_UNCHECKED );
			 CheckMenuItem( hMenu, IDM_GRIDHIDE, MF_UNCHECKED );
          CheckMenuItem( hMenu, IDM_COLOUREDGRID, MF_CHECKED );
		    }
         break;

       case VIEWNOGRID:
         DrawingOptions.grid = DrawingOptions.hiddengrid = ColouredGrid = FALSE;
         CheckMenuItem( hMenu, IDM_GRIDPLOT, MF_UNCHECKED );
			CheckMenuItem( hMenu, IDM_GRIDHIDE, MF_UNCHECKED );
         CheckMenuItem( hMenu, IDM_COLOUREDGRID, MF_UNCHECKED );
         break;

       case VIEWTRANSPARENTGRID:
         DrawingOptions.grid = TRUE;
         DrawingOptions.hiddengrid = ColouredGrid = FALSE;
         CheckMenuItem( hMenu, IDM_GRIDPLOT, MF_CHECKED );
			CheckMenuItem( hMenu, IDM_GRIDHIDE, MF_UNCHECKED );
         CheckMenuItem( hMenu, IDM_COLOUREDGRID, MF_UNCHECKED );
         break;

       case VIEWHIDDENGRID:
         DrawingOptions.grid = FALSE;
         DrawingOptions.hiddengrid = TRUE; 
         ColouredGrid = FALSE;
         CheckMenuItem( hMenu, IDM_GRIDPLOT, MF_UNCHECKED );
			CheckMenuItem( hMenu, IDM_GRIDHIDE, MF_CHECKED );
         CheckMenuItem( hMenu, IDM_COLOUREDGRID, MF_UNCHECKED );
         break;

       case VIEW2D:
          PostMessage( hwnd, WM_COMMAND, IDM_2DSURFACE, NULL );
          break;

       case VIEW3D:
          PostMessage( hwnd, WM_COMMAND, IDM_3DSURFACE,  NULL );
          break;

       case VIEWCONTOURLABELS:
         if( NextStringON() ) Checked = MF_CHECKED;
         else Checked = MF_UNCHECKED;
         CheckMenuItem( hMenu, IDM_CONTOURLABELS, Checked);
         DrawingOptions.labelcontours = 0;
         if( Checked == MF_CHECKED ) DrawingOptions.labelcontours = 1;
         break;

       case VIEWCONTOURLINES:
         if( NextStringON() )
          { if( !DrawingOptions.contours ) FlipOption( hMenu, DrawingOptions.contours, IDM_CONTOUR);}
           else
          { if( DrawingOptions.contours)   FlipOption( hMenu, DrawingOptions.contours, IDM_CONTOUR);}
         break;

       case VIEWLATLON:
         if( NextStringON() )
         { if( !LatLonData ) FlipOption( hMenu, LatLonData, IDM_LATLON);}
           else
         { if( LatLonData)   FlipOption( hMenu, LatLonData, IDM_LATLON);}
         break;

       case VIEWVOLUMEDIFFERENCE:
         if( NextStringON() ) DisplayVolumeDifference = TRUE;
         else                 DisplayVolumeDifference = FALSE;
         break;

       case VIEWCOLOURLEGEND:
         if( NextStringON() ) IncludeColourLegend = TRUE;
         else                 IncludeColourLegend = FALSE;
         break;

       case VIEWAXES:
         if( NextStringON() ) DrawingOptions.threedaxes = TRUE;
         else                 DrawingOptions.threedaxes = FALSE;
         break;

       case VIEWTEXTSIZE:
         GetString( szWord, sizeof( szWord ) );
         if( !ConvertInteger( iResult, szWord )) return Fail( hwnd );
         if( iResult >= 60 && iResult <= 200 ) TextSize = iResult;
         else return Fail( hwnd, "Bad Textsize in View Textsize command. It should be between 60 and 200.");
         break;

       case VIEWZLABELS:
         GetString( szWord, sizeof( szWord ) );
         if( !ConvertFloat( dResult, szWord )) return Fail( hwnd );
         ZminLabel = dResult;
         GetString( szWord, sizeof( szWord ) );
         if( !ConvertFloat( dResult, szWord )) return Fail( hwnd );
         ZmaxLabel = dResult;
         if( ZmaxLabel <= ZminLabel )
         {
           ZmaxLabel = ZminLabel = 0.0;
           return Fail( hwnd, "Zmax label less than or equal to Zmin Label: Both were set to zero.");
         } 
         break;


     // Edit... commands
       case EDITCOLOURMAPREVERSE:
         if( NextStringON() ) ReverseColourMap = TRUE;
         else                 ReverseColourMap = FALSE;
         break;

       case EDITCOLOURMAPBLENDED:
         BlendedColours = TRUE;
         break;

       case EDITCOLOURMAPMONO:
         BlendedColours = FALSE;
         break;

       case EDITCOLOURMAPMONOCOLOUR:
         // Get the RGB values.
         GetString( szWord, sizeof( szWord ) );
         if( !ConvertRGBInteger( iResult, szWord )) return Fail( hwnd );
         r = iResult;
         GetString( szWord, sizeof( szWord ) );
         if( !ConvertRGBInteger( iResult, szWord )) return Fail( hwnd );
         g = iResult;
         GetString( szWord, sizeof( szWord ) );
         if( !ConvertRGBInteger( iResult, szWord )) return Fail( hwnd );
         b = iResult;
         ShadePen = RGB( r, g, b );
         BlendedColours = FALSE; 
         break;

       case EDITANGLE:
          if( !GetString( szWord, sizeof( szWord ) ) ) return Fail( hwnd, IDS_MISSINGANGLE );
          if( !ConvertFloat( dResult, szWord )) return Fail( hwnd );
          if( dResult < (-360.) || dResult > 360. ) return Fail( hwnd, IDS_ANGLEVALUEERROR );
          Turn = dResult;
          if( GetString( szWord, sizeof(szWord) ) )
             { if( !ConvertFloat( dResult, szWord )) return Fail( hwnd );
               if( dResult < 0. || dResult > 90. ) return Fail( hwnd, IDS_ANGLEVALUEERROR);
               Tilt = dResult; }
          if( GetString( szWord, sizeof(szWord) ) )
             { if( !ConvertFloat( dResult, szWord )) return Fail( hwnd );
               if( dResult < 0. ) return Fail( hwnd, IDS_PROJECTIONVALUEERROR); 
               Projection = dResult; }
          RotateInitialize( hwnd);
          break;

       case EDITINPUTREVERSEXY:
          if( NextStringON() )  FlipXY = 1;
          else FlipXY = 0;
          break;

       case EDITINPUTREVERSEZ:
          if( NextStringON() )  ReverseSignOfZ = 1;
          else ReverseSignOfZ = 0;
          break;

       case EDITINPUTSAMPLENTH:
          GetString( szWord, sizeof( szWord ) );
          if( !ConvertInteger( iResult, szWord )) return Fail( hwnd );
          if( iResult < 0 || iResult > 999 ) return Fail( hwnd, IDS_BADSAMPLINGVALUE ); 
          KeepEveryNth( iResult );
          break;


       case EDITINPUTPOSITION:
        for( i = 0; i < 3; i ++ )
        {
         GetString( szWord, sizeof( szWord ) );
         if( !ConvertInteger( iResult, szWord )) return Fail( hwnd );
         if( iResult > 30  || iResult < 1 ) return Fail( hwnd, IDS_COMMANDFIELDPOSITIONBAD);
         InputFieldPosition[i] = iResult-1;
        }
        if( !GetString( szWord, sizeof( szWord ) )  )
          { IgnoreComments = 1; InputFieldPosition[4] = -1; }
        else
          {
            if( !ConvertInteger( iResult, szWord )) return Fail( hwnd );
            if( iResult > 30  || iResult < 1 ) return Fail( hwnd, IDS_COMMANDFIELDPOSITIONBAD);
            InputFieldPosition[4] = iResult-1;
          }
        break; 

       case EDITGRIDGENERATIONDISTANCE:
          GetString( szWord, sizeof( szWord ) );
          if( !ConvertInteger( iResult, szWord ) ) return Fail( hwnd );
          XpandDensityRatio( iResult );
          break;

       case EDITGRIDGENERATIONEDGE:
          GetString( szWord, sizeof( szWord ) );
          if( !ConvertInteger( iResult, szWord )) return Fail( hwnd );
          XpandEdgeFactor( iResult );
          break;

       case EDITGRIDGENERATIONSAMPLE:
          GetString( szWord, sizeof( szWord ) );
          if( !ConvertInteger( iResult, szWord )) return Fail( hwnd );
          XpandSample( iResult );
          break;

       case EDITINPUTSUBMETRIXZ:
          GetString( szWord, sizeof( szWord ) );
          if( !ConvertInteger( iResult, szWord )) return Fail( hwnd );
          SubmetrixSelectZ = iResult;
          if( (SubmetrixSelectZ < 1) || (SubmetrixSelectZ > 2) )
           {
             SubmetrixSelectZ = 1;
             NotifyUser( IDS_SUBMETRIXNOT1OR2 );
           }
          break;

       case EDITZRATIO:
           if( !GetString( szWord, sizeof( szWord) )) return Fail(hwnd);
           if( !ConvertInteger( iResult, szWord )) return Fail( hwnd );
           PaintSurfaceRatio( iResult );
           break;

       case EDITCOLOURMAP:
        {
           static double min, mid, max;
           zAdjust = ScatterData.zAdjust();
           if( !GetString( szWord, sizeof( szWord) )) return Fail(hwnd);
           if( !ConvertFloat( min, szWord ) ) return Fail(hwnd);
           if( !GetString( szWord, sizeof( szWord) )) return Fail(hwnd);
           if( !ConvertFloat( mid, szWord ) ) return Fail(hwnd);
           // If no last number make max = min and calculate mid.
           if( !GetString( szWord, sizeof( szWord) ))
              { max = mid; mid = min + (max-min)*.5 ; }
           else if( !ConvertFloat( max, szWord ) ) return Fail(hwnd);
           if( !ColourMap( min+zAdjust, mid+zAdjust, max+zAdjust ) ) return Fail(hwnd);
         }
           break;

      case EDITCONTOURLINES:
         SetWindowText( hwnd, "Processing ContourLines command" );
			static double LabeledDistance;
			static int InterspersedLines;
			if( !GetString( szWord, sizeof( szWord) )) return Fail(hwnd);
			if( stricmp( szWord, "OFF" ) == 0  )
			 {
            if( DrawingOptions.contours ) FlipOption( hMenu, DrawingOptions.contours, IDM_CONTOUR);
				break;
          }
         if( !DrawingOptions.contours ) FlipOption( hMenu, DrawingOptions.contours, IDM_CONTOUR);
			if( !ConvertFloat( LabeledDistance, szWord ) ) return Fail(hwnd);
         InterspersedLines = 0 ;
			if( GetString( szWord, sizeof( szWord) ))
           { if( !ConvertInteger( InterspersedLines, szWord )) return Fail(hwnd); }
			if( GetString( szWord, sizeof( szWord) ))
           { if( !ConvertInteger( LabelNDigits, szWord )) return Fail( hwnd) ; }
         if( LabelNDigits < 1 ) LabelNDigits = 1;
	      if( LabelNDigits > 6 ) LabelNDigits = 6;
			PenHighLite = InterspersedLines + 1;
         if( PenHighLite < 1 )   PenHighLite = 1;
	      if( PenHighLite > MaxContourLines )
                    PenHighLite = MaxContourLines;

			if( LabeledDistance <= 0.0 )
            {
				  NumberOfContours = InterspersedLines;
				  ContourLinesSet( );
				  PenHighLite = MaxContourLines;
            }
         else
            {
				  double increment = LabeledDistance/PenHighLite;
				  zAdjust = ScatterData.zAdjust();
				  float start = (float)( (int)((Zgrid.zmin()-zAdjust)/LabeledDistance) )
																	*LabeledDistance + zAdjust;
              ContourLinesSet( start, increment, Zgrid.zmax());
				}
          ContourBoldLabelSet( PenHighLite );
		   break;

      case EDITCONTOURLINESCOLOUR:
            if( NextStringON() )   ColourContours = TRUE;
            else                   ColourContours = FALSE;
         break;

      case GRIDTEMPLATE:
         static int nx, ny, MaxXY;
			static double xstart, xincrement, xend, ystart, yincrement, yend;
		   SetWindowText( hwnd, "Processing GridTemplate command" );
         GridLock = FALSE;
         KillLayerGrids();
         MaxXY = Zgrid.MaximumXY();
			if( !GetString( szWord, sizeof( szWord) )) return Fail(hwnd);
         if( !ConvertFloat( xstart, szWord ) ) return Fail(hwnd);
			if( !GetString( szWord, sizeof( szWord))) return Fail(hwnd);
         if( !ConvertFloat( xincrement, szWord ) ) return Fail(hwnd);
			if( !GetString( szWord, sizeof( szWord))) return Fail(hwnd);
         if( !ConvertFloat( xend, szWord) ) return Fail(hwnd);

         if( !GetString( szWord, sizeof( szWord) )) return Fail(hwnd);
         if( !ConvertFloat( ystart, szWord ) ) return Fail(hwnd);
			if( !GetString( szWord, sizeof( szWord))) return Fail(hwnd);
         if( !ConvertFloat( yincrement, szWord ) ) return Fail(hwnd);
			if( !GetString( szWord, sizeof( szWord))) return Fail(hwnd);
         if( !ConvertFloat( yend, szWord ) ) return Fail(hwnd);

         if( (xincrement <= 0) || (yincrement <= 0) )
				  return Fail( hwnd, IDS_GRIDINCREMENTERROR );
         nx = ((xend-xstart)/xincrement)+1.5;
	      ny = ((yend-ystart)/yincrement)+1.5;
	      if( nx < 3 || ny < 3 ) return Fail(hwnd,IDS_GRIDLINESERROR ); 
	      if( ny > MaxXY || ny >MaxXY )return Fail(hwnd, IDS_GRIDLINESERROR );
			SetTemplate( nx, xstart, xincrement, ny, ystart, yincrement, FALSE);
			CheckMenuItem( hMenu, IDM_GRIDLOCK, MF_CHECKED);
         // If scattered data points available - generate grid.
         if( ScatterData.Size() > 2) GenerateGrid( hwnd, hMenu );
		   break;

      case EDITTITLE:
         GetLastString( szTitle, sizeof(szString) );
         break;

      // Grid.. commands
      case GRIDLOCK:
         if( NextStringON() )
          { if( !GridLock ) FlipOption( hMenu, GridLock, IDM_GRIDLOCK ); }
         else
          { if( GridLock )  FlipOption( hMenu, GridLock, IDM_GRIDLOCK ); }
         if( !GridLock ) KillLayerGrids();
         break;

      case AUTOMATICGRIDRESOLUTION:
        if( NextStringON() )
           {
             GridResAuto = TRUE;
             CheckMenuItem( hMenu, IDM_GRIDRESAUTO, MF_CHECKED);
             GridResOptimum( hwnd, hMenu );
           }
         else
           {
             GridResAuto = FALSE;
             CheckMenuItem( hMenu, IDM_GRIDRESAUTO, MF_UNCHECKED);
           }
        break;

       // Function...  commands
       case FUNCTIONCOLOURLAYERMINUSGRID:
         if( !GridGeneratedOK )
             {NotifyUser( IDS_NOGRIDTOCOPY ); RestoreCursor(); return FALSE; }
         ColourLayerCompare();
         break;

       case FUNCTIONCOPYGRIDTOSPEED:
         if( !GridGeneratedOK )
			{NotifyUser( IDS_NOGRIDTOCOPY ); RestoreCursor(); return FALSE; }
         if( !GridLock ) FlipOption( hMenu, GridLock, IDM_GRIDLOCK);
         CopyGridToSpeed();
         break;

       case FUNCTIONCOPYGRIDTODIRECTION:
         if( !GridGeneratedOK )
			{NotifyUser( IDS_NOGRIDTOCOPY ); RestoreCursor();return FALSE; }
         if( !GridLock ) FlipOption( hMenu, GridLock, IDM_GRIDLOCK);
         CopyGridToDirection();
         break;

       case FUNCTIONCOPYGRIDTOCOLOUR:
         if( !GridGeneratedOK )
			{NotifyUser( IDS_NOGRIDTOCOPY ); RestoreCursor(); return FALSE; }
         if( !GridLock ) FlipOption( hMenu, GridLock, IDM_GRIDLOCK);
         CopyGridToColour();
         break;

       // Commands with no equivalent menu action.
		 case CAPTUREBITMAPANDEXIT:
         CaptureBitmapAndExit = TRUE;
         break;

       case CAPTUREMETAFILEANDEXIT:
         CaptureMetaFileAndExit = TRUE;
         break;

       case DEFINECONTOURLINE:
         // Contour value BOLD colour r g b  Label szlabel 
         zAdjust = ScatterData.zAdjust();
         if( !GetString( szWord, sizeof( szWord) )) return Fail(hwnd);
         if( !ConvertFloat( ContourValue, szWord ) ) return Fail(hwnd);
         // The rest can default:
         ContourBold = ContourLabel = FALSE;
         strcpy( szContourLabel, " " ) ;
         Colour = ContourPen;
         while( GetString( szWord, sizeof( szWord ) ) )
         {
            if( stricmp( szWord, "BOLD" ) == 0 )ContourBold  = TRUE;

            else if( stricmp( szWord, "LABEL" ) == 0 )
              {
                 ContourLabel = TRUE;
                 GetString( szContourLabel, sizeof( szContourLabel) );
              }

            else if( stricmp( szWord, "COLOUR" ) == 0 || stricmp( szWord, "COLOR" ) == 0 )
              {
                // Get the RGB values.
                GetString( szWord, sizeof( szWord ) );
                if( !ConvertRGBInteger( iResult, szWord )) return Fail( hwnd );
                r = iResult;
                GetString( szWord, sizeof( szWord ) );
                if( !ConvertRGBInteger( iResult, szWord )) return Fail( hwnd );
                g = iResult;
                GetString( szWord, sizeof( szWord ) );
                if( !ConvertRGBInteger( iResult, szWord )) return Fail( hwnd );
                b = iResult;
                Colour = RGB( r, g, b );
              }
         }
         ContourCustom( ContourValue+zAdjust, ContourBold,
                        ContourLabel, szContourLabel, Colour );
         break;

		 case DESTROYMENU:
			//DestroyMenu( hMenu );
         DeleteMenu( hMenu, 0, MF_BYPOSITION);
         DeleteMenu( hMenu, 1, MF_BYPOSITION);
         {for( int i=2; i < 6; i++)
            DeleteMenu( hMenu, 2, MF_BYPOSITION);}
         DeleteMenu( hMenu, IDM_EDITVIEWOPTIONS, MF_BYCOMMAND );
         DrawMenuBar( hwnd );
			break;

		 case EXITTIMER:
         static int Seconds;
			if( !GetString( szWord, sizeof( szWord))) return Fail(hwnd);
         if( !ConvertInteger( Seconds, szWord )) return Fail( hwnd);
			if( Seconds < 1 || Seconds > 100 )
                                    return Fail(hwnd, IDS_EXITTIMERERROR );
			SetTimer( hwnd, 2, Seconds*1000, NULL );
			break;

       case MARGIN:
         static double dMargin;
			if( !GetString( szWord, sizeof( szWord))) return Fail(hwnd);
			if( !ConvertFloat( dMargin, szWord ) ) return Fail(hwnd);
         if( dMargin > 1500. || dMargin < -1500. )
              return Fail( hwnd, IDS_MARGINERROR );
         Margin = dMargin;
         shift = Margin*2;
         break;

       case WINDOWSIZE:
          static int Xsize, Ysize;
          if( !GetString( szWord, sizeof( szWord) )) return Fail(hwnd);
          if( !ConvertInteger( Xsize, szWord )) return Fail( hwnd );
			 if( !GetString( szWord, sizeof( szWord))) return Fail(hwnd);
          if( !ConvertInteger( Ysize, szWord ) ) return Fail( hwnd );
          if( Xsize < 50 || Xsize > 3000 ||
              Ysize < 50 || Ysize > 3000 ) return Fail( hwnd, IDS_WINDOWSIZEERROR );
          MoveWindow( hwnd, 0, 0, Xsize, Ysize, TRUE );
         break;

	 case TERMINATE:
			PostMessage( hwnd, WM_DESTROY, NULL, NULL );
			RestoreCursor();
			inFile.close();
			return TRUE;

		 default:
       return Fail( hwnd,IDS_INVALIDCOMMAND ); 
    }
	if ( inFile.eof() ) break;
  }

 RestoreCursor();
 inFile.close();
 return TRUE;

}
