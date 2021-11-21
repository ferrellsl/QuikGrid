#ifndef QUIKGRID_H
#define QUIKGRID_H

//#define max(a,b) (((a)>(b))?(a):(b))
//#define min(a,b) (((a)<(b))?(a):(b))

struct ContourOptionsType
 { int contours,
       marks,
       xyplot,
       numbers,
       comments,
       grid,
       hiddengrid,
       lines,
       blackwhite,
       showmarks,
       showcorners,
       pointnumber,
		 threedaxes,
		 labelcontours; };

enum ContourType {ThreeD, TwoD, DXFoutput } ; // Used by DoLineTo.

extern int DoGenerateGrid;
extern HWND hGridgenDlgBox;
extern SurfaceGrid Zgrid;
extern ScatData ScatterData; 
extern const int MaxContourLines;
extern int NumberOfContours;
extern int PenHighLite;
extern HINSTANCE hInst ;
extern float Turn, Tilt, Aspect, Projection;
extern char szTitle[];
extern int LatLonData;

#endif