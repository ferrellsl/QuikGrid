//**************************************************************
//                   S a v e  L i n e   . h
//                             Class implementation
//                             John Coulthard
//              Copyright (c) 1993 - 1999 by John Coulthard
//**************************************************************
#ifndef SAVELINE_H
#define SAVELINE_H 

static const long SaveLineMax = 10000000,
                  SaveLineBlock =2000;

class SaveLineType
{ private:
   struct LineType
   {
       int x;
	    int y;
	    int pen;
   } ;
   LineType *array[SaveLineMax/SaveLineBlock] ;
   long size;
   int xmax, xmin, ymax, ymin;

  public:
   SaveLineType();
   ~SaveLineType();
   void SetNext(  const int x,  const int y, const unsigned char pen );
   void Reset();

   long Size() { return size; }
   long MaxSize() { return SaveLineMax; }
   int xMax() { return xmax; }
   int xMin() { return xmin; }
   int yMax() { return ymax; }
   int yMin() { return ymin; }
   int Pen( long i ) { return array[i/SaveLineBlock][i%SaveLineBlock].pen; }
   int x( long i) { return array[i/SaveLineBlock][i%SaveLineBlock].x; }
   int y( long i) { return array[i/SaveLineBlock][i%SaveLineBlock].y; }
};

#endif