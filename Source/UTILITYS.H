//*******************************************************************
//                 U t i l i t y s . h
//*******************************************************************
void PictureChanged( HWND &hwnd );
void FlipOption( HMENU hMenu, int &option, UINT MenuId );

// NotifyUser and all of it's varients. 
void NotifyUserError( int MessageNumber );

// NotifyUser - Load string versions.
void NotifyUser( int MessageNumber);
void NotifyUser( int MessageNumber, char szString[] );
void NotifyUser( int MessageNumber, char szString1[], char szString2[] );
void NotifyUser( int MessageNumber, char szString1[], int i, char szString2[]);
void NotifyUser( int MessageNumber, char szString1[], char szString2[], int i );
void NotifyUser( int MessageNumber, int i, char szString1[], char szString2[] );
void NotifyUser( int MessageNumber, int i, char szString[] );
void NotifyUser( int MessageNumber, char szString1[], int i1, int i2, char szString2[], char szString3[] );

// NotifyUser - string as argument versions. 
void NotifyUser( char szNote[]);
void NotifyUser( char szNote[], const int arg1);
void NotifyUser( char szNote[], const int arg1, const int arg2 );
void NotifyUser( char szNote[], const int arg1, const int arg2, const int arg3 );
void NotifyUser( char szNote[], const float arg1);
void NotifyUser( char szNote[], const float arg1, const float arg2 );
void NotifyUser( char szNote[], const float arg1, const float arg2, const float arg3 );
void NotifyUser( char szNote[], char szString[] );

int GetReadFile( char szFile[], HWND &hwnd, char* szOpenTitle );
int GetReadFile( char szFile[], HWND &hwnd, char* szOpenTitle, char szFilter[] );
int GetReadFileMultiple( char szFile[], HWND &hwnd, char* szOpenTitle, char szFilter[] );
int GetSaveFile( char szFile[], HWND &hwnd, char* szOpenTitle  );
int GetSaveFile( char szFile[], HWND &hwnd, char* szOpenTitle, char szFilter[], char szExtension[] );

void SetWaitCursor();
void RestoreCursor();
int LonToDouble( double &result, const char szInput[] );
int LatToDouble( double &result, const char szInput[] );
float MinPart( const double x ); 
void LatLonToDegMin( const double x, int &degrees, double &minutes );
double ConvertDegDec( long x ); 
void   ConvertDecDeg( double x, int &degrees, int & minutes, int &seconds);
long   ConvertDecDeg( double x );
void FormatXY( char szResult[], const int length, const double x, const double y );
void FormatCoord( char szResult[], const int &length, const double &value );
char *FormatCoord( const double &value );
char *FormatLon( const double &value, int Pretty = TRUE );
char *FormatLat( const double &value, int Pretty = TRUE ) ;
char *INIfileName();
bool StringIsBlank( const char String[] );
bool StringIsInteger( const char String[] );
float AtPrecision( float value, int precision );
int GetPairInt( char szString[], char szSearch[], int &Fail );
double GetPairFloat( char szString[], char szSearch[], int &Fail );
int TabEdit( char String[], int MaxLength ); 
