/* Prototypes for functions defined in
gamecontrol.c
 */

extern struct Opts opts;

extern struct RDArgs * rdargs;

extern struct Library * IntuitionBase;

extern struct Library * GfxBase;

extern struct Library * LowLevelBase;

extern struct Library * KeymapBase;

extern struct Window * win;

extern struct RastPort * rp;

extern struct EasyStruct easyStruct;

extern struct EasyStruct * ez;

extern UBYTE ezBuf[100];

extern BOOL cliStart;

extern BOOL keysOn;

extern struct Rectangle grid[31][9];

extern int curX;

extern int curY;

void main(int , UBYTE ** );

BOOL turnOnLLKeys(BOOL );

void changeBox(int , int , int , int );

void flashBox(int , int );

void gridBox(int , int );

void liteBox(int , int );

void drawGrid(void);

void initGrid(void);

void ezReq(STRPTR , APTR , ...);

