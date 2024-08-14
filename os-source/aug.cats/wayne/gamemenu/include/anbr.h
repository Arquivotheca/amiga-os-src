/**************
    anbr.h

 W.D.L 930818

***************/


typedef struct AnimFrame
{
    struct AnimFrame	* next;
    UBYTE		* delta;
    ULONG		  flags;
    LONG		  reltime;

} ANIMFRAME;


typedef struct BltData
{
    struct bltnode	  bltnode;
    ULONG		  flags;
    ULONG		  signal;
    struct Task		* task;
    struct anbr		* anbr;

} BLTDATA;


typedef struct anbr
{
    struct anbr		* next;
    long		* multTable;  /* Multiply table */
    struct BitMap	* bitmap;
    UBYTE		* mask;
    ANIMFRAME		* animframes;
    ANIMFRAME		* curframe;
    struct Rectangle	  rect;
    short		  width;
    short		  bwidth;
    short		  height;
    short		  xpos;
    short		  ypos;
    ULONG		  flags;
    LONG		  numframes;
    LONG		  reltime;
    ULONG		  timer;
    BLTDATA		  bltdata;

} ANBR;

// ANBR->flags
#define	 ANBR_NEED_2_DRAW	0x00000010


typedef struct AnimControl
{
    ANBR	* firstanbr;
    ULONG	  flags;
    ULONG	  timer;

} ANIMCONTROL;


#define	BLIT_QUED	0x00000001


/********* protos **********/

VOID	decode_xriff( UBYTE *, char *, SHORT, LONG *, short, short, short );
int	DoQuery( UBYTE * filename, DISP_DEF * disp_def );
int	ReadAnbr( UBYTE * filename, ANBR **anbr );
VOID	FreeAnbr( ANBR * anbr );
int	OpenDisplay( DISP_DEF * disp_def, UBYTE * ilbmfile );
VOID	CloseDisplay( DISP_DEF * disp_def );



