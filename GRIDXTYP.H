//**************************************************************
//                   G r i d X T y p e. h
//         Copyright (c) 1993 - 1998 by John Coulthard
//
// Routine used by XPAND to store the data that associates data
// points to their nearest grid location. Hence, after sorting, it is
// a very fast lookup to find all data points near to a give grid
// interesection.
//
// Sept. 21/98: Converted to Win32 - long variables.
// Apr. 8/99: Convert to use lookup table.
//**************************************************************

class GridXType 
{ protected:
	enum { Shift = 32768 } ;  // Increased from 4096 for larger grids.

	struct LocateStructure { long intersection;
									 long DataLocation; } ;
	LocateStructure *FindPoints;
	long Size,             // Number of data points.
		  nx, ny,           // Size of grid being generated.
		  *Lookup,          // Lookup grid (nx by ny in size)
        LookupSize, 
		  PreviousSearch,   // Remembers result from previous search.
		  np;               // Actual number of points used in search.
  
  public:
	GridXType( const long iSize, const long nx, const long ny);
	void New( const long iSize, const long nx, const long ny );
   void Sort();
	long Search( const int i, const int j, const int n );

	long x( long i) { return FindPoints[i].intersection/Shift; }
	long y( long i) { return FindPoints[i].intersection%Shift; }
	long location( const long i)  { return FindPoints[i].DataLocation; }
	void setnext( long i, int ix, int iy )
     { FindPoints[np].DataLocation = i;
       FindPoints[np].intersection = (long)ix*(long)Shift+(long)iy;
       np++; }


	~GridXType() { delete[] FindPoints; delete Lookup; }
};
