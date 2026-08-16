#ifndef XYGRID_H
#define XYGRID_H

class xyGridClass
// Define a simple class to handle the definition of a rectangular grid
// representing a surface - this one rotated.
{
  private:
    struct point2d {  int ix;  int iy; } ;
    point2d *zgrid;       // z[i][j]
    int   nx, ny;       // save actual size of the array.

  public:
    xyGridClass(int i,int j);
    void  New(int i, int j);
    void  set( int i, int j,  int x,  int y);

    int x( int i, int j )  { return zgrid[j*nx+i].ix ;}
    int y( int i, int j )  { return zgrid[j*nx+i].iy ;}
    int   xsize()                   { return nx; }
    int   ysize()                   { return ny; }

    ~xyGridClass() { delete[] zgrid;}
};

#endif
