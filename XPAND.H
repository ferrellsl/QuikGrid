// Xpand include file.

void XpandInit( SurfaceGrid &Zgrid, ScatData &Data );

int  XpandPoint( SurfaceGrid &Zgrid, ScatData &RandomData );

int XpandPercentDone();
int XpandPercentShell();
int XpandBandWidth();
int XpandScanRatio();
int XpandScanRatio( const int NewRatio);
int XpandDensityRatio();
int XpandDensityRatio( const int NewRatio);
int XpandEdgeFactor();
int XpandEdgeFactor( const int NewRatio );
float XpandUndefinedZ();
float XpandUndefinedZ( const float z );
long XpandSample();
long XpandSample( const long i );

void Xpand( SurfaceGrid &Zgrid, ScatData &RandomData );

