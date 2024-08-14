/*----------------------------------------------------------------------*
 * prototypes for lib functions 										*
 *----------------------------------------------------------------------*/


VOID FillCDTVPrefs(	struct CDTVPrefs * );
int	 GrabBm(	struct Header *,UBYTE *,struct BMInfo **,struct BitMap **,
				UBYTE **);

UBYTE *CreateMask(struct BitMap *,UBYTE,UBYTE *);

VOID FreeGrabBm(struct BMInfo **,struct BitMap **,UBYTE **);

int FillBackDrop(struct BitMap *,struct Header *,UBYTE *);

int FillForeground(struct BitMap *,struct Header *,UBYTE *,WORD,WORD,
				   struct BMInfo **);
					 
int BuildBg(struct BuildBg_Info *,struct BMInfo **,struct BitMap ** );

int DoTitle(void);

LONG InstallScreenSaver(void);
LONG ScreenSaverCommand(ULONG cmd);

VOID LoadCycleView(struct CycleIntInfo *,struct View *,UWORD **);
VOID CloseViewMgr(struct CycleIntInfo *);
struct CycleIntInfo *StartViewMgr(struct CycleIntInfo *,struct BMInfo *,
									 struct View *,UWORD **,UBYTE);
VOID FindViewRGB(struct View *, UWORD **, WORD );
VOID LoadFoundRGB( UWORD **, UWORD *,WORD );
VOID LoadFoundRGBFade(UWORD **,UWORD *,WORD,WORD,WORD,UWORD *,ULONG);

UWORD LevelColor(UWORD color, BYTE lv );
UWORD BetweenColor( UWORD color0, UWORD color1, BYTE lv );

int WriteTinyStr(struct BitMap *,WORD x,WORD y,UBYTE color,UBYTE *str); 

int SafeWaitIO(	struct IORequest *);

VOID FadeCMap( UWORD *,UWORD *,UWORD *,WORD,WORD );
VOID CenterCDTVView( struct CDTVPrefs *,struct View *,WORD,WORD);


int SaveCDTVPrefs(struct CDTVPrefs *);
int DeleteBookMark(ULONG mpic);
int WriteBookMark( void *,ULONG size, ULONG mpic, BYTE pri );
int ReadBookMark(void *data, ULONG size, ULONG mpic );

int PrepareTM(UBYTE *,LONG);
int DisplayTM(void);
VOID FreeTM(void);

int DecompTradeMark(UBYTE **,LONG *);

LONG InstallKeyClick(void);
LONG KeyClickCommand (ULONG, void *, LONG, LONG, LONG, LONG);

void ConfigureCDTV( struct CDTVPrefs * );

VOID *AllocIORequest(char *,ULONG,ULONG,ULONG);
VOID FreeIORequest( VOID *);
LONG SafeDoIO (  struct IORequest * );


