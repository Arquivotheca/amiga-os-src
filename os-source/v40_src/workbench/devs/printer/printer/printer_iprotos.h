
/* copen.c */
int OpenPrinter ( void );
int ExpungePrinter ( void );
int strcmpnc ( char *s1 , char *s2 );

/* yexr.c */
int YEnlargeXReduce ( struct PrtInfo *PInfo , int pass );

/* cdumprport.c */
int PCDumpRPort ( struct IODRPReq *ior );
int roundit ( int number );
int GetSpecialDensity ( ULONG *special );

/* cwrite.c */
UBYTE PCWrite ( struct IOStdReq *ior );
int ProcessCSI ( int Length , unsigned char OutBuffer []);
int ProcessString ( int Length , unsigned char OutBuffer []);
int GetCommand ( BYTE ThisCommand , int Length , UBYTE OutBuffer [], UBYTE values [], BYTE callFlag );
unsigned char GetParms ( int *Position , int Length , UBYTE values [], int *flag );
unsigned char ReadBuffer ( int *Position , int Length );
int FindLastEscape ( void );

/* pixel.c */
int InitHamArray ( struct PrtInfo *PInfo );
int ScanConvertPixelArray ( struct PrtInfo *PInfo , UWORD y );
int ConvertPixelArray ( struct PrtInfo *PInfo , UWORD y , int sum );
int CompactPixelArray ( struct PrtInfo *PInfo , UWORD sytotal );
int TransferPixelArray ( UWORD ypos , struct PrtInfo *PInfo , int pass );
int CheckBuf ( UWORD y , int (*render )(), int flag );
int GetBlack ( union colorEntry *ce , UWORD flags );
int SwapScaleX ( struct PrtInfo *PInfo );
int FixScalingVars ( struct PrtInfo *PInfo );
int Force1to1Scaling ( struct PrtInfo *PInfo );
int SwapInts ( struct PrtInfo *PInfo );

/* yrxe.c */
int YReduceXEnlarge ( struct PrtInfo *PInfo , int pass );
int AvgPixelArray ( struct PrtInfo *PInfo , UWORD total );

/* awrite.c */
VOID OpenAlpha ( int start );
int savepPref ( void );
int cmppPref ( void );
BYTE PCPrtCommand ( struct IOPrtCmdReq *ior );
int translateCommand ( UWORD point , char outputBuffer [], UBYTE Parms []);

/* cdefault.c */
int DoNothing ( void );
int DefaultDoSpecial ( void );

/* yexe.c */
int YEnlargeXEnlarge ( struct PrtInfo *PInfo , int pass );

/* error.c */
int Backout ( int error , struct IODRPReq *ior , struct PrtInfo *PInfo );

/* fixcolors.c */
int FixColorMap ( struct PrtInfo *PInfo );
int FixColorsPixelArray ( struct PrtInfo *PInfo );
int FixColorsYMC ( struct PrtInfo *PInfo , union colorEntry *ce , int n );
int FixColorsRGB ( struct PrtInfo *PInfo , union colorEntry *ce , int n );

/* mrp.c */
int MyReadPixelArray ( UWORD y , struct PrtInfo *PInfo , WORD *buf );

/* query.c */
int PCQuery ( struct IOStdReq *pio );

/* floyd.c */
int FloydPixelArray ( struct PrtInfo *PInfo , int flag );
int FloydSwapDest ( struct PrtInfo *PInfo );

/* alias.c */
int AliasSwapBufs ( struct PrtInfo *PInfo );
int AliasPixelArray ( UWORD y , UWORD sy , struct PrtInfo *PInfo , int flag );

/* yrxr.c */
int YReduceXReduce ( struct PrtInfo *PInfo , int pass );

/* time.c */
ULONG ReadVBlankTime ( void );
void ReadVertTime ( struct mytimeval *t );
void ElapsedVertTime ( struct mytimeval *t1 , struct mytimeval *t2 , struct mytimeval *total );
void PrintAvgTime ( char *s , struct mytimeval *total , struct mytimeval *sum );
