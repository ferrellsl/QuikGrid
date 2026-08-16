void PaintContourReset();
void ContourLinesSet();
void NiceContourLinesSet();
void ContourLinesSet( const double start, const double increment, const double end);
float ContourLineValue( const int i );
void ContourLineValue( const int i, float x );
int ContourLineBold( const int i );
int ContourLineLabel( const int i );
char *szContourLineLabel( const int i );
void ContourBoldLabelSet( const int EveryNth );
void ContourCustom( const double Value, const int Bold,
                    const int Label, const char szLabel[], const COLORREF Colour );
COLORREF ContourLineColour( const int i ) ;
void ContourAutoColour(  );
COLORREF ChooseZColour( float z );
int ColourMap( const double min, const double mid, const double max );
int DataColourMap( const double min, const double mid, const double max );
void RestoreColourMap(  );
void SaveColourMap(  );
