#ifndef SURFGRID_H
#define SURFGRID_H
//******************************************************************
//              S u r f a c e    G r i d
//                    Copyright (c) 1993 - 2002 by John Coulthard
//
// Define the class to store the grid representing the surface.
//
// Sept. 21/98: Convert to Win32.
// Nov. 18/98: Increase max dimensions to 1500x1500.
// Jan. 18/99: Increase max dimensions to 2000x2000.
// Feb. 25/99: Increase max dimensions to 4000x4000.
// Dec. 30/02: Increase mas dimensions to 8000x8000.
// May 1/2019: Increase max dimensions to 12000x12000.
//*******************************************************************

struct GridViewType
    { int xStart,
          xIncrement,
          xEnd,
          yStart,
          yIncrement,
			 yEnd; } ;

class SurfaceGrid
// Define a simple class to handle the definition of a rectangular grid
// representing a surface.

{ private:

  enum { MaxXYdimension = 12000 };  // declare constants
 //			MaxZarrayXY    = 4000 };

  float *zgrid,       // z[i][j]
		  *xvect,       // x[i]
		  *yvect;       // y[j]

  long  nx, ny;     // size of the array.

  int	first;        // flag for first time a z value is set.

  GridViewType View,
					UnitView,
					OldView,
					CurrentView; // View a subset of the grid.

  float maxz, minz,
		  adjustx, adjusty, adjustz,
		  scalez;

  void Initialize();

  public:
	 SurfaceGrid(const int i, const int j);
	 void  New(const int i, const int j);
	 void  Reset();
	 void  zset( const int i, const int j, const float a );
	 void  xset( const int i, const float a );
	 void  yset( const int i, const float a );
	 void  zratio( const float ratio);
    void  map( float &x, float &y, float &z);
    float z   ( const int i, const int j );  

	 GridViewType view(const GridViewType &v)
							 { OldView = View; CurrentView = v; return OldView;}
    GridViewType view()      { return CurrentView; }
	 GridViewType removeview()
							 { OldView = View; View = UnitView; return OldView;}
	 GridViewType applyview()
							 { OldView = View; View = CurrentView; return OldView;}

    int MaximumXY() { return MaxXYdimension; }
	 float x( const int i ) { return xvect[View.xStart +i*View.xIncrement] - adjustx;}
    float y( const int i ) { return yvect[View.yStart +i*View.yIncrement] - adjusty;}
    int   xsize()          { return (View.xEnd-View.xStart-1)/View.xIncrement+1; }
    int   ysize()          { return (View.yEnd-View.yStart-1)/View.yIncrement+1; }

    float xmax()                { return xvect[nx-1] - adjustx;}
    float xmin()                { return xvect[0] - adjustx; }
	 float ymax()                { return yvect[ny-1] - adjusty; }
    float ymin()                { return yvect[0] - adjusty; }
    float zmax()                { return (maxz-adjustz)*scalez; }
    float zmin()                { return (minz-adjustz)*scalez; }
	 float zmap( const float a )       { return (a-adjustz)*scalez; }
    void  Znew()          { first = 1; return; } 
	 ~SurfaceGrid() { delete[] zgrid; delete[] xvect; delete[] yvect;}
};

#endif
