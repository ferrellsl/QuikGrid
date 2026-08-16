#undef assert
#ifdef NODEBUG
#define assert(IGNORE)  ((void)0) 
#else
void DoAssert( char a[], char b[], unsigned int);
#define assert(x) ((void) ((x) || (DoAssert(#x, __FILE__,__LINE__),1)))
#endif