#ifndef SCATDATA_H
#define SCATDATA_H
//**************************************************************
//                   S c a t   D a t a  . h
//
//        Copyright (c) 1993 - 1999 by John Coulthard
//
// Apr. 6/99:  Change to allocate memory constants in scatdata.cpp.
//**************************************************************

class ScatData
{ protected:
   float *X, *Y, *Z, *tempXYZ;
   char **Comment, **tempComment, *blank;
   unsigned char *Flags, *tempFlags;
	long size, memsize, MaxPoints, sizeincrement ;
   float xmax, xmin, ymax, ymin, zmax, zmin, zadjust; 

  public:
	ScatData();
	void SetNext( float x, float y, float z, char[] = NULL, unsigned char=NULL );
	void Reset();
   void SetZ( const long i, const float z); 
	void SetFlags( const long i, unsigned char c ) ;
	void SetComment( const long i, char[] = NULL) ;
   void move( const long i, const long j);
   void truncate( const long i); 

   long MaximumPoints() { return MaxPoints; }
	long  Size() { return size; }
	float xMax() { return xmax; }
	float xMin() { return xmin; }
	float yMax() { return ymax; }
	float yMin() { return ymin; }
	float zMax() { return zmax+zadjust; }
	float zMin() { return zmin+zadjust; }
	float zAdjust() {return (zadjust); }
	void  zAdjust( float a) { zadjust = a; }
	float x( const long i) { return X[i]; }
	float y( const long i) { return Y[i]; }
	float z( const long i) { return Z[i]+zadjust; }
	char *comment( const long i ) { return Comment[i];  }
	unsigned char flags( const long i ) { return Flags[i] ; }

   ~ScatData();
};

#endif 