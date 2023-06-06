//*******************************************************************
//       D i a l o g  C o l o r   C h o o s e r
//
//                   Copyright (c) 2000-2001 by John Coulthard
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
// Feb. 25-28/00: Initial implementation.
// Mar. 10/01: Add Colour mapping support.
// June 7/01: Add support for colour mapping dialog box.
//*******************************************************************
#include <windows.h>
#include <HtmlHelp.h>
#include "surfgrid.h"
#include "scatdata.h"
#include "utilitys.h"
#include "rc.h"
#include "prefer.h"
#include "helpmap.h"
#include "dlgutil.h"
#include "condraw.h"

extern ScatData ScatterData;
extern SurfaceGrid Cgrid;
extern float CgridZadjust;
extern int BlendedColours;
extern int ReverseColourMap;
extern int GridGeneratedOK;

extern HPEN hGridPen,
            hArrowPen,
            hContourPen,
            hBoldContourPen,
            hDataPointPen,
            hOutlinePen,
            hAxisPen;

extern COLORREF GridPen,
                ArrowPen,
                ContourPen,
                BoldContourPen,
                DataPointPen,
                OutlinePen,
                BackgroundPen,
                TextPen,
                AxisPen;

extern COLORREF ShadePen;

extern double ColourZmax,
             ColourZmid,
             ColourZmin;

static CHOOSECOLOR ColorStructure;

static COLORREF ColorChosen,
                crTemp[16] = { 16*0 };  // Custom colors.

static HWND hwnd;
//**********************************************************************
//   Choose A C o l o r
//**********************************************************************
// Internal service routine.
static int ChooseAColor( HWND &hDlg, COLORREF &Pen)
{
   static int First = TRUE;
   if( First)
     {
       crTemp[0] = GridPen,
       crTemp[1] = ContourPen,
       crTemp[2] = OutlinePen,
       crTemp[3] = ArrowPen,
       crTemp[4] = DataPointPen,
       crTemp[5] = AxisPen,
       crTemp[6] = TextPen,
       crTemp[7] = BackgroundPen;
       crTemp[8] = ShadePen;
       crTemp[9] = RGB( 255, 255, 255);
       crTemp[10] = RGB( 255, 0, 0);
       crTemp[11] = RGB( 0, 255, 0);
       crTemp[12] = RGB( 0, 0, 255);
     }
   hwnd = GetParent( hDlg );
   ColorStructure.lStructSize = sizeof( CHOOSECOLOR);
   ColorStructure.hwndOwner = hwnd;
   ColorStructure.hInstance = 0;
   ColorStructure.rgbResult = Pen;
   ColorStructure.lpCustColors = crTemp;
   ColorStructure.Flags = CC_RGBINIT;
   ColorStructure.lCustData = 0L;
   ColorStructure.lpfnHook = NULL;
   ColorStructure.lpTemplateName = NULL;
   return ChooseColor( &ColorStructure );
}
//**********************************************************************
//    G e t   C o l o r
//**********************************************************************
// Internal service routine.
static void GetColor( HWND &hDlg, COLORREF &Pen, HPEN &hPen )
{
   if( ChooseAColor( hDlg, Pen ) )
     {
        ColorChosen = ColorStructure.rgbResult;
        DeleteObject( hPen );
        hPen = CreatePen( PS_SOLID, 0, ColorChosen );
        Pen = ColorChosen;
        PictureChanged( hwnd );
     }
   return;
}
//**********************************************************************
//    C o l o r  F e a t u r e s  D i a l o g  B o x
//**********************************************************************
BOOL FAR PASCAL ColorFeaturesDlgProc( HWND hDlg, UINT message, WPARAM wParam,
					 LPARAM lParam)
 {

  switch (message)
  {
	 case WM_INITDIALOG :
		 return TRUE;

	 case WM_COMMAND :
		 switch (wParam)
	    {
        case IDHELP:
	       if( HtmlHelp( hDlg, "quikgrid.chm::1ku6po.htm", HELP_CONTEXT, EditColourFeatures ) == 0 )
			   //if (HtmlHelp(hDlg, "quikgrid.chm::html/1ku6po.htm", HELP_CONTEXT, 0L) == 0)
		     NotifyUser( IDS_HELP);
	       return TRUE;

		  case IDOK :
          EndDialog(hDlg,0);
			 return 0;

		  case IDCANCEL :
			  EndDialog( hDlg, 0);
			  return TRUE;

        case IDPREFERENCE:
           SaveColorOptions();
           return TRUE;

        case IDD_COLORGRIDLINE:
           GetColor( hDlg, GridPen, hGridPen ) ;
           return TRUE;

        case IDD_COLORDATAPOINTS:
           GetColor( hDlg, DataPointPen, hDataPointPen ) ;
           return TRUE;

        case IDD_COLORARROWS:
           GetColor( hDlg, ArrowPen, hArrowPen ) ;
           return TRUE;

        case IDD_COLOROUTLINE:
           GetColor( hDlg, OutlinePen, hOutlinePen ) ;
           return TRUE;

        case IDD_COLORAXIS:
           GetColor( hDlg, AxisPen, hAxisPen ) ;
           return TRUE;

        case IDD_COLORTEXT:
           if( ChooseAColor( hDlg, TextPen ) )
                                   TextPen = ColorStructure.rgbResult;
           PictureChanged( hwnd );
           return TRUE;

        case IDD_COLORCONTOURLINE:
           GetColor( hDlg, ContourPen, hContourPen );
           DeleteObject( hBoldContourPen );
           hBoldContourPen = CreatePen( PS_SOLID, 0, ColorChosen );
           BoldContourPen = ColorChosen;
           return TRUE;

        case IDD_COLORBACKGROUND:
           if( ChooseAColor( hDlg, BackgroundPen ) )
                                   BackgroundPen = ColorStructure.rgbResult;
           PictureChanged( hwnd );
           return TRUE;
        }
		 break;
     }
  return FALSE;
 }
//**********************************************************************
//    Set Colour Mapping look.
//**********************************************************************
static void SetColourMapLook( HWND hDlg )
{
  if( BlendedColours )
         {
           CheckDlgButton( hDlg, IDD_BLENDEDCOLOUR, TRUE );
           SetDlgItemText( hDlg, IDD_GREEN, "Green");
           if( ReverseColourMap )
           {
             SetDlgItemText( hDlg, IDD_RED, "Blue");
             SetDlgItemText( hDlg, IDD_BLUE, "Red");
           }
           else
           {
             SetDlgItemText( hDlg, IDD_RED, "Red");
             SetDlgItemText( hDlg, IDD_BLUE, "Blue");
           }
         }
       else
         {
            CheckDlgButton( hDlg, IDD_SHADEDMONOCHROME, TRUE);
            SetDlgItemText( hDlg, IDD_GREEN, "Ignored");
            if( ReverseColourMap )
           {
             SetDlgItemText( hDlg, IDD_RED, "Darkest");
             SetDlgItemText( hDlg, IDD_BLUE, "White");
           }
           else
           {
             SetDlgItemText( hDlg, IDD_RED, "White");
             SetDlgItemText( hDlg, IDD_BLUE, "Darkest");
           }
         }
       if( !GridGeneratedOK )
         {
           SetDlgItemText( hDlg, IDD_COLOURMAPHEADING,
           " There is no grid - any values entered below will be overwritten when a grid is generated ");
         }
       else SetDlgItemText( hDlg, IDD_COLOURMAPHEADING," ");
}
//**********************************************************************
//    C o l o r  M a p p i n g   D i a l o g  B o x
//**********************************************************************
 BOOL FAR PASCAL ColorMappingDlgProc( HWND hDlg, UINT message, WPARAM wParam,
					 LPARAM lParam)
 {
    static double NewMax, NewMid, NewMin, zAdjust;
    static int ColourResult, OldBlendedColours, OldReverseColourMap;
    static char szColourZmid[40];
    static HPEN     hShadePen;
    static COLORREF OldShadePen;
   switch (message)
  {
	 case WM_INITDIALOG :
       zAdjust = CgridZadjust;
       SetDlgItemFloat( hDlg, IDD_COLOURZMAX, ColourZmax-zAdjust );
       SetDlgItemFloat( hDlg, IDD_COLOURZMID, ColourZmid-zAdjust );
       SetDlgItemFloat( hDlg, IDD_COLOURZMIN, ColourZmin-zAdjust );
       if( ReverseColourMap )
            CheckDlgButton( hDlg, IDD_REVERSECOLOURMAP, 1 );
       else CheckDlgButton( hDlg, IDD_REVERSECOLOURMAP, 0 );
       SetColourMapLook( hDlg );
       OldBlendedColours = BlendedColours;
       OldReverseColourMap = ReverseColourMap;
       OldShadePen = ShadePen;
		 return TRUE;

	 case WM_COMMAND :
		 switch (wParam)
	    {
        case IDHELP:
			   if (HtmlHelp(hDlg, "quikgrid.chm::1ku6po.htm", HELP_CONTEXT, 0L) == 0)
		     NotifyUser( IDS_HELP);
	       return TRUE;

		  case IDOK :
          ColourResult = GetDlgItemFloat( hDlg, IDD_COLOURZMAX, NewMax )+
                         GetDlgItemFloat( hDlg, IDD_COLOURZMIN, NewMin );
          if( ColourResult != 0 ) return 0;

          GetDlgItemText( hDlg, IDD_COLOURZMID, szColourZmid, sizeof(szColourZmid) );
          if( StringIsBlank( szColourZmid )) NewMid = (NewMax-NewMin)*.5 + NewMin;
          else ColourResult = GetDlgItemFloat( hDlg, IDD_COLOURZMID, NewMid );
          if( ColourResult != 0 ) return 0;
          zAdjust = CgridZadjust;
          if( !ColourMap( NewMin+zAdjust, NewMid+zAdjust, NewMax+zAdjust) ) return 0;
          EndDialog(hDlg,0);
			 return 0;

        case IDD_BLENDEDCOLOUR:
          BlendedColours = TRUE;
          SetColourMapLook( hDlg );
          return 0;

        case IDD_SHADEDMONOCHROME:
          BlendedColours = FALSE;
          SetColourMapLook( hDlg );
          return 0;

        case IDD_CHOOSECOLOUR:
           GetColor( hDlg, ShadePen, hShadePen ) ;
           return TRUE;

        case IDD_REVERSECOLOURMAP:
           if( IsDlgButtonChecked( hDlg, IDD_REVERSECOLOURMAP) )
                ReverseColourMap = TRUE;
           else ReverseColourMap = FALSE;
           SetColourMapLook( hDlg );
           return TRUE;

		  case IDCANCEL :
           BlendedColours = OldBlendedColours;
           ReverseColourMap = OldReverseColourMap;
           ShadePen = OldShadePen; 
			  EndDialog( hDlg, 0);
			  return TRUE;

        case IDPREFERENCE:
           SaveColourMappingOptions();
           return TRUE;

      }
		 break;
     }
  return FALSE;
 }