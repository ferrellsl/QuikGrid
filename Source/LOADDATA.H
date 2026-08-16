// loaddata.h
void LoadTestData( HWND &hwnd );
void SetStartupState( HWND &hwnd );
int LoadFileMetricData( HWND &hwnd, char FileName[] );
int LoadFileLatLonData( HWND &hwnd, char FileName[] );
// int LoadFileMedsData( HWND &hwnd, char FileName[] );
int LoadFileDCAData( HWND &hwnd, char FileName[] );
int LoadFileNOSData( HWND &hwnd, char FileName[] );
int LoadFileNOAALatLonData( HWND &hwnd, char FileName[] );
int LoadFileDXFData( HWND &hwnd, char FileName[] );
int LoadFileOutlineData( HWND &hwnd, char FileName[] );
int LoadGridData( HWND &hwnd, char FileName[] );
int LoadUSGSDEMData( HWND &hwnd, char FileName[] );
int LoadERMapperGrid( HWND &hwnd, char FileName[] );
int  GridSizeRatio();
int  GridSizeRatio( int );
void LoadNormalization( double &dx, double &dy );
int IgnoreZzero( ) ; 
int IgnoreZzero( int i );
int KeepEveryNth() ;
int KeepEveryNth( int NewValue );
void SetSurfaceTitle( HWND &hwnd, char szFileName[],
                      float x = 0, float y = 0, int ix = -1, int iy = -1 );
int GridMaxRatio() ;
int GridMaxRatio( int NewRatio ) ;
void SetTemplate( int nx, double xs, double xi,
						int ny, double ys, double yi, int LatLon ) ;
int LoadSubmetrixSXPData( HWND &hwnd, char FileName[] );
void SetNiceColourMap( const float mina, const float maxa );                

