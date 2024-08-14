/***********

    GameSelect.c

    W.D.L 930618

************/

/*
 * COPYRIGHT: Unless otherwise noted, all files are Copyright (c) 1993
 * Commodore-Amiga, Inc.  All rights reserved.
 *
 * DISCLAIMER: This software is provided "as is".  No representations or
 * warranties are made with respect to the accuracy, reliability, performance,
 * currentness, or operation of this software, and all use is at your own risk.
 * Neither commodore nor the authors assume any responsibility or liability
 * whatsoever with respect to your use of this software.
 */

// Tab size is 8!

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/interrupts.h>

#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/dostags.h>

#include <utility/tagitem.h>

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <intuition/imageclass.h>
#include <intuition/intuitionbase.h>

#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>
#include <graphics/text.h>

#include <hardware/intbits.h>

#include <libraries/asl.h>
#include <libraries/lowlevel.h>

#include "devices/cd.h"
#include <devices/mpeg.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/utility_protos.h>
#include <clib/dos_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/asl_protos.h>
#include <clib/graphics_protos.h>
#include <clib/alib_protos.h>
#include <clib/lowlevel_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/asl_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/lowlevel_pragmas.h>

#include "stdio.h"
#include <stdlib.h>
#include <math.h>	// For min() & max()
#include <string.h>	// for setmem()

#include "disp_def.h"
#include "retcodes.h"

#include "gameselect_rev.h"

#include "debugsoff.h"

// Uncomment to get debug output turned on
#define KPRINTF
//#include "debugson.h"

#define	GAMESELVER	VERSTAG " Wayne D. Lutz"

UBYTE	* Version = GAMESELVER;

// Comment out to NOT play cdaudio
#define		DO_CD_AUDIO
#define		DO_HANGCOUNTER

// argument parsing 
#define TEMPLATE    "FROM/A,SECONDS/K/N,MINUTES/K/N,REBOOTLOOPS/K/N,AUDIOTRACK/K/N"
#define	OPT_FROM	0
#define	OPT_SECONDS	1
#define	OPT_MINUTES	2
#define	OPT_REBOOTLOOPS	3
#define	OPT_AUDIOTRACK	4
#define	OPT_COUNT	5

#define GAD_LEFT	15
#define	GAD_TOP		50
#define	GAD_HT		(attr->ta_YSize + 8)
#define	GAD_LEFT_SEP	32
#define	GAD_SEP		2

#define COLUMN_SEP	32
#define COLUMNS		2

#define	PANEL_DEPTH	2


#define FIRST_ID	100

#define LMB_DOWN (!((*((BYTE *) 0xbfe001) & 192)==192))
#define S(MASK) (MASK & state )

			// 5 min * 60 sec * VB/S
#define	TIMEOUT		(5*60*(DisplayIsPal ? 50 : 60))

// protos

VOID CloseDisplay( DISP_DEF * disp_def );
int  DoDisplay( UBYTE * ilbmname );
int  DoQuery( UBYTE * filename, DISP_DEF * disp_def );
int  ScrWinOpen( DISP_DEF * disp_def, UBYTE * ilbmfile, LONG );
int  PlayTrack( ULONG track );
VOID CDDeviceTerm( VOID );
BOOL CDDeviceInit( ULONG * opened );
VOID CDAudioTerm( VOID );
VOID StripIntuiMessages( struct MsgPort * mp, struct Window * win );
VOID ColorCycle( DISP_DEF * disp_def );

UBYTE outcon[80] = 
	{"CON:10/10/600/200/GameSelect Output Window/AUTO/CLOSE/WAIT"};

UBYTE * DoWB = "execute >nil: sys:s/startup-sequence.wb";
UBYTE * DoSECRET = "cdgsxl >nil: Demos:xl/secret.xl MULTIPAL BLIT SDBL NOPOINTER XLSPEED 150 FIREABORT";

BOOL	DisplayIsPal;
BOOL	DoingAutoDemo;	// Var that reflects if a game/demo was started by
			// the automatic demo mechanism as opposed to being
			// explicitly selected by the user.

ULONG	AudioTrack;


struct Interrupt	* VBint;

typedef struct intdata
{
    LONG	count1;
    LONG	count2;
    LONG	TimeOut;
    BOOL	DoTimeOut;
    ULONG	HangCounter;

} INTDATA;

INTDATA		IntData;

LONG		  opts[OPT_COUNT];
struct RDArgs	* rdargs;


DISP_DEF		  DisplayDef;
UBYTE			* ILBMname;

struct Screen		* PanelScreen;
struct Window		* PanelWindow;

struct Screen		* MaskScreen;

struct VisualInfo	* PanelVI;
struct Gadget		* PanelGlist;
struct Image		* PanelFrame;

struct Library		* GadToolsBase;
struct Library		* UtilityBase;
struct Library		* IntuitionBase;
struct Library		* GfxBase;
struct Library		* IFFParseBase;
struct Library		* FreeAnimBase;
struct Library		* LowLevelBase;


STATIC struct Device	* CDDevice;
STATIC struct MsgPort	* CDPort;

STATIC struct IOStdReq	* CDDeviceMReq;

STATIC LONG		  CDPortSignal = -1;

STATIC BOOL		  DoReboot;
STATIC int		  rebootloops;


typedef struct DataNode
{
    struct DataNode *Next;
    ULONG Size;
    ULONG Type;

} DATA_NODE;

typedef struct GameConfig
{
    DATA_NODE	  dnode;
    LONG	  timeout;
    ULONG	  flags;
    UBYTE	* name;
    UBYTE	* command;

} GAMECONFIG;

// GAMECONFIG flags
#define		CONFIG_ALWAYS_TIMEOUT	0x00000001
#define		CONFIG_UNLOAD		0x00000002

GAMECONFIG * HeadConfig,* CurConfig;

typedef struct TimeOutNode
{
    DATA_NODE	  dnode;
    GAMECONFIG	* config;

} TIMEOUTNODE;

typedef struct TimeOutControl
{
    TIMEOUTNODE	* timeouts;
    TIMEOUTNODE * cur;

} TIMEOUTCONTROL;

TIMEOUTCONTROL	TimeOutControl;

struct Rectangle MarqueeRect;
struct Gadget	* CurrentGad,* GadColumn1,* GadColumn2;
SHORT  CurrentID = FIRST_ID;

struct TextAttr Topaz =
{
    "topaz.font",
    9,
    FSF_BOLD,
    0,
};


struct TextAttr GameSelectAttr =
{
    "GameSelect",
    18,
    FSF_BOLD,
    0,
};


IMPORT UBYTE		far GSFontName;
IMPORT struct TextFont	far GSTextFont;
STATIC BOOL		FontAdded;

STATIC USHORT chip InvisiblePointer[]= {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000};

SHORT GSXY1[] = {
    0,0,
    0,0,
    0,0,
    0,0,
    0,0,
    0,0,
    0,0,
    0,0
};

STATIC struct Border	GSBorder1 =
{
    0,0,	// LeftEdge, TopEdge,
    7,0,	// FrontPen,BackPen
    JAM1,	// DrawMode
    8,		// Count
    GSXY1,	// XY
    NULL	// NextBorder
};


SHORT GSXY2[] = {
    0,0,
    0,0,
    0,0,
    0,0,
    0,0,
    0,0,
    0,0,
    0,0
};


STATIC struct Border	GSBorder2 =
{
    0,0,	// LeftEdge, TopEdge,
    2,0,	// FrontPen,BackPen
    JAM1,	// DrawMode
    8,		// Count
    GSXY2,	// XY
    &GSBorder1	// NextBorder
};


// Left Top selected border

SHORT SelXYLT[] = {
    0,0,
    0,0,
    0,0,
    0,0,
    0,0,
    0,0,
    0,0,
    0,0,
    0,0,
    0,0,
};


STATIC struct Border	GSSelectedBorderLT =
{
    0,0,	// LeftEdge, TopEdge,
    2,0,	// FrontPen,BackPen
    JAM1,	// DrawMode
    10,		// Count
    SelXYLT,	// XY
    NULL	// NextBorder
};


// Right Bottom selected border

SHORT SelXYRB[] = {
    0,0,
    0,0,
    0,0,
    0,0,
    0,0,
    0,0,
    0,0,
    0,0,
    0,0,
    0,0,
};


STATIC struct Border	GSSelectedBorderRB =
{
    0,0,	// LeftEdge, TopEdge,
    7,0,	// FrontPen,BackPen
    JAM1,	// DrawMode
    10,		// Count
    SelXYRB,	// XY
    &GSSelectedBorderLT	// NextBorder
};


void __regargs _CXBRK(void) { }  /* Disable SAS CTRL/C handling */
void __regargs chkabort(void) { }  /* really */
void __regargs Chk_Abort(void) { }  /* truly */


#ifdef	DEBUG	// [
#include "time.h"

VOID
PrintDateStr( VOID )
{
    struct tm	* date;
    static char	  string[20];
    char	  Date[20],* ptr;
    long	  t,century = 1900;
    SHORT	  i;


    t = time(NULL);
    date = (struct tm *)localtime(&t);
    for(i = 0;i < 20;++i)
	string[i] = '\0';

    D(PRINTF("tm_mon=%ld, mday=%ld, tm_year=%ld",date->tm_mon,date->tm_mday);)
 
    if( date->tm_year < 90 )
	century = 2000;

    sprintf(Date,"%ld",date->tm_year + century);
    strcpy(string,Date);

    sprintf(Date,"%02u", date->tm_mon + 1);
    strcat( string, Date );

    sprintf(Date,"%02u",date->tm_mday );
    strcat( string, Date );

    D(PRINTF("Date = '%ls'\n",string);)

} // PrintDateStr()


VOID
PrintTimeStr( VOID )
{
    struct tm	* clock;
    static char	  string[10];
    long	  t;

    t = time(NULL);
    clock = localtime(&t);
    sprintf(string,"%02d:%02d:%02d",(short)clock->tm_hour,(short)clock->tm_min,
	(short)clock->tm_sec);

    D(PRINTF("Time='%ls'\n",string);)

} // PrintTimeStr()

VOID
MemAvailable( VOID )
{
    UBYTE * ptr;

    if ( ptr = AllocMem( 1024 * 0xFFFF, 0 ) )
	FreeMem( ptr, 1024 * 0xFFFF );

    PrintDateStr();
    PrintTimeStr();
    D(PRINTF("Memory Available: PUBLIC %ld\n", AvailMem(MEMF_PUBLIC) );)
    D(PRINTF("Memory Available: LARGEST %ld\n", AvailMem(MEMF_LARGEST) );)


} // MemAvailiable()
#endif		// ]

/*
 * Close every thing down.
 */
//STATIC
VOID
closedown( VOID )
{
    if ( MaskScreen ) {

	if ( MaskScreen->FirstWindow )
	    CloseWindow( MaskScreen->FirstWindow );

	CloseScreen( MaskScreen );
	MaskScreen = NULL;
    }

    if ( IntuitionBase )
	CloseLibrary( IntuitionBase );

    if ( GfxBase )
	CloseLibrary( GfxBase );

    if ( GadToolsBase )
	CloseLibrary( GadToolsBase );

    if ( IFFParseBase )
	CloseLibrary( IFFParseBase );

    if ( FreeAnimBase )
	CloseLibrary( FreeAnimBase );

    if ( LowLevelBase )
	CloseLibrary( LowLevelBase );

} // closedown()


/*
 * Open all of the required libraries.
 */
init( BOOL iff )
{
    if ( !(IntuitionBase = OpenLibrary("intuition.library", 39L)) ) {
	printf("Could NOT open intuition.library! V39\n");
	return( RC_FAILED );
    }

    if(!(GfxBase = OpenLibrary("graphics.library",39L)) ) {
	printf("Could NOT open graphics.library V39!\n");
	return( RC_FAILED );
    }

    if ( !(GadToolsBase = OpenLibrary( "gadtools.library", 39L )) ) {
	printf("Could NOT open gadtools.library V39!\n");
	return( RC_FAILED );
    }

    if ( !(LowLevelBase = OpenLibrary( "lowlevel.library", 39L )) ) {
	printf("Could NOT open lowlevel.library!\n");
	return( RC_FAILED );
    }


    if ( iff ) {
	D(PRINTF("opening iffparse.library!\n");)
	if(!(IFFParseBase = OpenLibrary("iffparse.library",0L)) ) {
	    printf("Could NOT open iffparse.library!\n");
	    return( RC_FAILED );
	}
    }

#ifdef	OUTT	// [
    D(PRINTF("Opening freeanim.library\n");)
    FreeAnimBase = OpenLibrary("freeanim.library",0L);
    D(PRINTF("After opening freeanim.library\n");)
#endif		// ]

    return( RC_OK );

} // init()


#define	MPEG_NAME	"cd32mpeg.device"
BOOL MPEGModule;

BOOL
MPEGCapable( VOID )
{
    struct MsgPort	* MPEGPort;
    struct IOMPEGReq	* MPEGReq;   
    BOOL		  present = FALSE;

#ifdef	OUTT	// [
    return( MPEGModule = TRUE );	

#else		// ][

    D(
     PRINTF("Before Open\n");
     MemAvailable();
    )

    if ( MPEGPort = CreateMsgPort() ) {
	D(PRINTF("MPEGCapable() GOT MPEGPort\n");)

	if ( MPEGReq = CreateIORequest( MPEGPort, sizeof (struct IOMPEGReq)) ) {
	    D(PRINTF("MPEGCapable() GOT MPEGDeviceMReq\n");)

	    if ( !OpenDevice( MPEG_NAME, 0, (struct IORequest *)MPEGReq, 0 ) ) {
		D(PRINTF("MPEGCapable() Got a Device\n");)
		present = TRUE;

		D(
		 PRINTF("After Open\n");
		 MemAvailable();
		)

		CloseDevice( (struct IORequest *)MPEGReq );
	    }
	    DeleteIORequest( MPEGReq );
	}
	DeleteMsgPort( MPEGPort );
    }

    D(
     PRINTF("After Close\n");
     PRINTF("Before Expunge: PUBLIC %ld\n", AvailMem(MEMF_PUBLIC) );
     PRINTF("Before Expunge: LARGEST %ld\n", AvailMem(MEMF_LARGEST) );
     MemAvailable();
    )

    return( MPEGModule = present );	
#endif		// ]

} // MPEGCapable()


VOID
ClosePanel( VOID )
{
    struct Gadget	* gad;
    struct IntuiText	* it;

    D(PRINTF("ClosePanel() 1\n");)

    if ( FontAdded ) {
	Forbid();
        GSTextFont.tf_Accessors--;
	RemFont( &GSTextFont );
	Permit();
	FontAdded = FALSE;
    }

    if ( PanelFrame ) {
	DisposeObject( PanelFrame );
	PanelFrame = NULL;
    }

    if ( DisplayDef.Flags & DISP_OPEN ) {
	if ( PanelVI ) {
	    FreeVisualInfo( PanelVI );
	    PanelVI = NULL;
	}
	CloseDisplay( &DisplayDef );

	if ( PanelGlist ) {
	    for ( gad = PanelGlist; gad; gad = gad->NextGadget ) {
		for ( it = gad->GadgetText; it; it = gad->GadgetText ) {
		    gad->GadgetText = it->NextText;
		    FreeMem( it, sizeof (*it) );
		}
	    }
	    FreeGadgets( PanelGlist);
	    PanelGlist = NULL;
	}
	PanelScreen = NULL;
	PanelWindow = NULL;
	return;
    }

    if ( PanelWindow ) {
	Forbid();
	for ( gad = PanelWindow->FirstGadget; gad; gad = gad->NextGadget ) {
	    for ( it = gad->GadgetText; it; it = gad->GadgetText ) {
		gad->GadgetText = it->NextText;
		FreeMem( it, sizeof (*it) );
	    }
	}
	StripIntuiMessages( PanelWindow->UserPort, PanelWindow );
	ModifyIDCMP( PanelWindow, 0L );
	Permit();
	CloseWindow( PanelWindow );
	PanelWindow = NULL;
    }

    if ( PanelGlist ) {
	FreeGadgets( PanelGlist);
	PanelGlist = NULL;
    }

    if ( PanelScreen ) {
	if ( PanelVI ) {
	    FreeVisualInfo( PanelVI );
	    PanelVI = NULL;
	}
	CloseScreen( PanelScreen );
	PanelScreen = NULL;
    }

    D(PRINTF("ClosePanel() END\n");)

} // ClosePanel()


struct Gadget *
GadCreate( struct NewGadget * ng, struct VisualInfo * vi, struct Gadget * last,
 ULONG kind, ULONG tag, ... )
{
	ng->ng_VisualInfo = vi;

	return( CreateGadgetA(kind,last,ng,(struct TagItem *)&tag) );

} // GadCreate()


#define	BORDER_X	3
#define	BORDER_SLANTX	8
#define	BORDER_SLANTY	(((ng->ng_Height-1) >> 1) - 2)	/*8*/

VOID
SetupGSBorders( struct NewGadget * ng )
{

    GSXY1[0] = BORDER_X;
    GSXY1[1] = (ng->ng_Height-1) >> 1;
    GSXY1[2] = BORDER_X+BORDER_SLANTX;
    GSXY1[3] = GSXY1[1] - BORDER_SLANTY;
    GSXY1[4] = ng->ng_Width - (BORDER_X + BORDER_SLANTX);
    GSXY1[5] = GSXY1[3];
    GSXY1[6] = GSXY1[4] + BORDER_SLANTX;
    GSXY1[7] = GSXY1[1];
    GSXY1[8] = GSXY1[6]-1;
    GSXY1[9] = GSXY1[7];
    GSXY1[10] = GSXY1[4]-1;
    GSXY1[11] = GSXY1[5];
    GSXY1[12] = GSXY1[2]+1;
    GSXY1[13] = GSXY1[3];
    GSXY1[14] = GSXY1[0]+1;
    GSXY1[15] = GSXY1[1];

/*
                       /2,3-------------4,5\
                    / 12,13-------------10,11 \
                 /   /                       \   \
              /   /           GSXY1              \   \
        0,1/   /                                   \   \6,7    
       14,15/                                         \ 8,9

       14,15\                                         / 8,9
        0,1\   \                                   /   /6,7
              \   \           GSXY2             /   /
                 \   \                       /   /
                    \ 12,13-------------10,11 /
                       \2,3-------------4,5/
*/

    GSXY2[0] = BORDER_X;
//    GSXY2[1] = GSXY1[1] + 1;
    GSXY2[1] = (ng->ng_Height) >> 1;
    GSXY2[2] = BORDER_X+BORDER_SLANTX;
    GSXY2[3] = GSXY2[1] + BORDER_SLANTY;
    GSXY2[4] = ng->ng_Width - (BORDER_X + BORDER_SLANTX);
    GSXY2[5] = GSXY2[3];
    GSXY2[6] = GSXY2[4] + BORDER_SLANTX;
    GSXY2[7] = GSXY2[1];
    GSXY2[8] = GSXY2[6]-1;
    GSXY2[9] = GSXY2[7];
    GSXY2[10] = GSXY2[4]-1;
    GSXY2[11] = GSXY2[5];
    GSXY2[12] = GSXY2[2]+1;
    GSXY2[13] = GSXY2[3];
    GSXY2[14] = GSXY2[0]+1;
    GSXY2[15] = GSXY2[1];


// Left Top selected border
   SelXYLT[0] = 1;
   SelXYLT[1] = 1;
   SelXYLT[2] = 1;
   SelXYLT[3] = ng->ng_Height-1;
   SelXYLT[4] = 0;
   SelXYLT[5] = ng->ng_Height;
   SelXYLT[6] = 0;
   SelXYLT[7] = 0;
   SelXYLT[8] = ng->ng_Width-1;
   SelXYLT[9] = 0;


// Left Top selected border
   SelXYRB[0] = ng->ng_Width-1;
   SelXYRB[1] = ng->ng_Height-1;
   SelXYRB[2] = ng->ng_Width-1;
   SelXYRB[3] = 1;
   SelXYRB[4] = ng->ng_Width;
   SelXYRB[5] = 0;
   SelXYRB[6] = ng->ng_Width;
   SelXYRB[7] = ng->ng_Height;
   SelXYRB[8] = 0;
   SelXYRB[9] = ng->ng_Height;

} // SetupGSBorders()


Config2Gadgets( struct VisualInfo * vi, struct Gadget **glist )
{
    GAMECONFIG		* config;
    struct TextAttr	* attr /*= &Topaz*/;
    struct Gadget	* prev;
    struct NewGadget	  ng;
    struct IntuiText	  it,* git;
    SHORT		  i = 0,len,cnt,rows,columns;
    SHORT		  id = FIRST_ID,max = 0,min = PanelScreen->Width;
    SHORT		  gadtop;

    setmem( &ng, sizeof ( ng ), 0 );
    setmem( &it, sizeof (it), 0 );

    *glist = NULL;

    GadColumn1 = GadColumn2 = NULL;

    if ( !(prev = CreateContext(glist)) )
	return( FALSE );

    GameSelectAttr.ta_YSize = GSTextFont.tf_YSize;
    strcpy( &GSFontName, "GameSelect" );
    Forbid();
    AddFont( &GSTextFont );
    GSTextFont.tf_Accessors++;
    Permit();
    attr = &GameSelectAttr;
    FontAdded = TRUE;

#define	SHINE_OFF 1 /* was 1 for Topaz */

    it.ITextFont = attr;

    cnt = 0;
    for ( config = HeadConfig; config; config = (GAMECONFIG *)config->dnode.Next ) {
	it.IText = config->name;
	len = IntuiTextLength( &it );
	it.LeftEdge = (PanelScreen->Width >> 1) - (len >> 1);
	max = (len > max) ? len : max;
	min = (it.LeftEdge < min) ? it.LeftEdge : min;
	cnt++;
    }

    columns = (cnt > (DisplayIsPal ? 7 : 6)) ? COLUMNS : 1;

    rows = (cnt-1)/columns;

    ng.ng_Width = max + GAD_LEFT_SEP;

    ng.ng_LeftEdge = (PanelScreen->Width >> 1) - (((columns*ng.ng_Width) + GAD_LEFT_SEP) >> 1) - 8;

    if ( rows > 1 ) {
	int	ht = DisplayIsPal ? 240 : 200;
	int	maxtop = ht - 10;
	int	room = maxtop - GAD_TOP;


	gadtop = ng.ng_TopEdge = GAD_TOP + ((room - (rows * (GAD_HT+4))) >> 1);
    } else {
	gadtop = ng.ng_TopEdge = GAD_TOP;
    }
/*
    if ( gadtop < 73 ) {
	gadtop = ng.ng_TopEdge = 73;
    }
*/

    if ( gadtop < (DisplayIsPal ? 85 : 78) ) {
	gadtop = ng.ng_TopEdge = (DisplayIsPal ? 85 : 78);
    }

    ng.ng_Height = GAD_HT;
    ng.ng_TextAttr = it.ITextFont = attr;
    ng.ng_VisualInfo = vi;
    ng.ng_Flags = NULL;

    SetupGSBorders( &ng );

    for ( config = HeadConfig; config; config = (GAMECONFIG *)config->dnode.Next ) {
	ng.ng_GadgetText = NULL;
	ng.ng_GadgetID = id++;
	if ( prev = GadCreate( &ng, vi, prev, BUTTON_KIND, TAG_DONE ) ) {

	    prev->UserData = (APTR)config;
	    if ( prev->GadgetText = (struct IntuiText *)AllocMem( sizeof (it), MEMF_CLEAR )) {
		prev->GadgetText->ITextFont = attr;
		prev->GadgetText->IText = config->name;
		prev->GadgetText->FrontPen = 0;
		prev->GadgetText->DrawMode = JAM1;
		len = IntuiTextLength( prev->GadgetText );
		prev->GadgetText->LeftEdge = (prev->Width >> 1) - (len >> 1);
		prev->GadgetText->TopEdge = (prev->Height >> 1) - (attr->ta_YSize >>1) + 1;

		if ( git = (struct IntuiText *)AllocMem( sizeof (it), MEMF_CLEAR )) {
		    *git = *prev->GadgetText;
		    prev->GadgetText->FrontPen = 7;
		    prev->GadgetText->LeftEdge = (prev->Width >> 1) - (len >> 1) + SHINE_OFF;
		    prev->GadgetText->TopEdge = (prev->Height >> 1) - (attr->ta_YSize >>1) + SHINE_OFF;
		    prev->GadgetText->NextText = git;
	    	}

	    }
	    D(PRINTF("GadgetText= 0x%lx\n",prev->GadgetText);)

	    if ( prev->TopEdge == gadtop ) {
		if ( !GadColumn1 ) {
		    GadColumn1 = prev;
		} else {
		    GadColumn2 = prev;
		}
	    }
	}
	if ( (rows > 1) && (++i > rows) ) {
	    ng.ng_TopEdge = gadtop;
	    ng.ng_LeftEdge += (ng.ng_Width) + COLUMN_SEP;
	    i = 0;
	} else {
	    ng.ng_TopEdge += GAD_HT + GAD_SEP;
	}
    }

    if ( !prev ) {
	FreeGadgets( *glist );
	*glist = NULL;
	return( FALSE );
    }

    return( TRUE );

} // Config2Gadgets()


struct Gadget	*
TraverseGList ( ULONG ID, struct Gadget * GList)
{
    while (GList) {
	if (GList->GadgetID == ID)	
	    return( GList );

	GList = GList->NextGadget;
    }		

    return ( NULL );

} // TraverseGList()


struct Gadget	*
GetGadget( struct Window * Window, ULONG ID )
{
    return ( TraverseGList ( ID, Window->FirstGadget ) );

} // GetGadget()


VOID
RubberBox( struct RastPort * rp, struct Rectangle * rect )
{
    int x1,y1,x2,y2;
    int i;

    x1 = rect->MinX;
    y1 = rect->MinY;
    x2 = rect->MaxX;
    y2 = rect->MaxY;

    if (x1 > x2) {
	i = x1;
	x1 = x2;
	x2 = i;
    }

    if (y1 > y2) {
	i = y1;
	y1 = y2;
	y2 = i;
    }

    Move(rp,x1,y1);
    Draw(rp,x2,y1);
    Move(rp,x2,y1+1);
    Draw(rp,x2,y2);
    Move(rp,x2-1,y2);
    Draw(rp,x1,y2);
    Move(rp,x1,y2-1);
    Draw(rp,x1,y1+1);

} // RubberBox()

#ifdef	OUTT	// [

VOID
Marquee( struct RastPort * rp, struct Rectangle * rect, BOOL redraw )
{
    static int patidx = 0;
    static int pat[4] = {0xcccc,0x6666,0x3333,0x9999};
    struct Rectangle trect;

    trect.MinX = rect->MinX - 1;
    trect.MaxX = rect->MaxX + 1;
    trect.MinY = rect->MinY - 1;
    trect.MaxY = rect->MaxY + 1;

    SetDrPt( rp, pat[patidx] );
    RubberBox( rp, rect );
    RubberBox( rp, &trect );

    if ( redraw ) {

	if ( (++patidx) == 4 )
		patidx = 0;

	SetDrPt( rp,pat[patidx] );

	RubberBox( rp, rect );
	RubberBox( rp, &trect );
    }

    SetDrPt( rp, 0xffff );

} // Marquee()
#else		// ][

VOID
Marquee( struct RastPort * rp, struct Rectangle * rect, BOOL erase )
{
STATIC int	pen = 33; //7;
    struct Rectangle trect;


    trect.MinX = rect->MinX - 1;
    trect.MaxX = rect->MaxX + 1;
    trect.MinY = rect->MinY - 1;
    trect.MaxY = rect->MaxY + 1;

    SetDrPt( rp, 0xffff );

    SetAPen( rp, erase ? 0 : pen );
    RubberBox( rp, rect );
    RubberBox( rp, &trect );
/*
    if ( ++pen > 3 )
	pen = 2;
*/

//    pen = (pen == 7) ? 4 : 7;


} // Marquee()

#endif			// ]


BOOL
docommand( UBYTE *command, BOOL restart )
{
    struct TagItem tags[4];
    LONG fh;

    D(PRINTF("\ndocommand ENTERED\n");)
    D(MemAvailable();)

#ifdef	DO_CD_AUDIO	// [
    D(PRINTF("Before stopping audio\n");)
    IntData.HangCounter = ((DisplayIsPal ? 50 : 60)*10);
    CDAudioTerm();
    IntData.HangCounter = 0;
    D(PRINTF("After stopping audio\n");)
#endif			// ]

    ScreenToFront( MaskScreen );
    if ( MaskScreen->FirstWindow )
	ActivateWindow( MaskScreen->FirstWindow );
    ClosePanel();

    WaitBlit();
    Delay( 50 );
    WaitBlit();

    if(fh = Open(outcon,MODE_OLDFILE)) {
    	tags[0].ti_Tag	= SYS_Input;
    	tags[0].ti_Data 	= fh;
    	tags[1].ti_Tag	= SYS_Output;
    	tags[1].ti_Data 	= NULL;
    	tags[2].ti_Tag	= NP_Priority;
    	tags[2].ti_Data 	= 0L;
    	tags[3].ti_Tag	= TAG_DONE;

#ifdef	OUTT	// [
	if ( DoingAutoDemo )	// Wait 10 minutes before rebooting if hung
	    IntData.HangCounter = ((DisplayIsPal ? 50 : 60)* (60*10));
#else		// ][

	if ( CurConfig ) {
	    if ( (CurConfig->flags & CONFIG_ALWAYS_TIMEOUT) || DoingAutoDemo ) {
		IntData.HangCounter = ((DisplayIsPal ? 50 : 60) * CurConfig->timeout);
	    } else {
		IntData.HangCounter = 0;
	    }
	    D(PRINTF("CurConfig= 0x%lx, timeout= %ld, HangCounter= %ld\n",
		CurConfig,CurConfig->timeout,IntData.HangCounter);)
	} else {
	    D(PRINTF("CurConfig is NULL\n");)
	}

#endif		// ]

	D(PRINTF("Doing System() with '%ls'\n",command);)

	if( (System(command, tags) ) == -1 ) {
	    D(PRINTF("After System() -1\n");)
	    Close(fh);
	} else {
	    D(PRINTF("After System()\n");)
	    Close(fh);
	}

    } else {
	D(PRINTF("docommand - Can't open output window\n");)
    }

    if ( MaskScreen->FirstWindow )
	ActivateWindow( MaskScreen->FirstWindow );

    IntData.count1 = 0;	// Reset for TIMEOUT

    if ( !restart ) {
	IntData.HangCounter = 0;
	return( TRUE );
    }

    if ( DoingAutoDemo ) {
	if ( DoReboot ) {
	    if ( !TimeOutControl.cur->dnode.Next ) {
		if ( --rebootloops <= 0 )
		    D(PRINTF("Calling ColdReboot!\n");)
		    ColdReboot();
		}
	}
    }

    DoDisplay( ILBMname );

//    Delay( 300 );

#ifdef	DO_CD_AUDIO	// [
    D(PRINTF("Before starting audio\n");)
    if ( CDDeviceInit( NULL ) ) {
	PlayTrack( AudioTrack );
    }
    D(PRINTF("After starting audio\n");)
#endif			// ]

    IntData.HangCounter = 0;

    D(PRINTF("docommand END\n");)
    D(MemAvailable();)

    return( TRUE );

} // docommand()


BOOL
AsyncCommand( UBYTE *command )
{
    struct TagItem	  tags[5];
    LONG		  fh;

    ClosePanel();

    SetTaskPri( FindTask( NULL ), 0L );

    if(fh = Open(outcon,MODE_OLDFILE)) {
    	tags[0].ti_Tag	= SYS_Input;
    	tags[0].ti_Data = fh;
    	tags[1].ti_Tag	= SYS_Output;
    	tags[1].ti_Data = NULL;
    	tags[2].ti_Tag	= NP_Priority;
    	tags[2].ti_Data = 0L;
    	tags[3].ti_Tag	= SYS_Asynch;
    	tags[3].ti_Data	= TRUE;
    	tags[4].ti_Tag	= TAG_DONE;

	if( (System(command, tags) ) == -1 ) {
	    Close(fh);
	    Delay( 100 );
	    ColdReboot();
	}
    } else {
	printf("AsyncCommand() - Can't open output window\n");
    }

    return( TRUE );

} // AsyncCommand()


/* Returns the integer position of the item in the list or 0 if not found.
*/
LONG
GListItemPosition( struct Gadget * first, struct Gadget * last, struct Gadget * gad )
{
    LONG		  pos=0;
    struct Gadget	* g;

    for ( g = first; g && (g != last); g = g->NextGadget )  {
	pos++;
	if ( g == gad )
	    return ( pos );
    }
    return (0);

} // GListItemPosition()


VOID
RefreshGSList( struct Gadget * gad, struct Window * win, struct Requester * req,
 SHORT num )
{

    while ( num-- && gad ) {
	// The first gad is the one returned from CreateContext. It should be
	// the only one that is NOT a GTYP_BOOLGADGET.
	if ( (gad->GadgetType & GTYP_GTYPEMASK) == GTYP_BOOLGADGET ) {
	    RefreshGList( gad, win, req, 1 );
	    D(PRINTF("Drawing Border Left= %ld, Top= %ld, Flags= 0x%lx, Type= 0x%lx\n",
		gad->LeftEdge, gad->TopEdge,gad->Flags,gad->GadgetType );)
	    DrawBorder( win->RPort, &GSBorder2, gad->LeftEdge, gad->TopEdge );
	}
	gad = gad->NextGadget;
    }

} // RefreshGSList()


VOID
OffGSList( struct Gadget * gad, struct Window * win, struct Requester * req )
{
    while ( gad ) {
	gad->Flags |= GFLG_DISABLED;
//	GT_SetGadgetAttrs( gad, win, req, GA_Disabled, TRUE, TAG_END );
	gad = gad->NextGadget;
    }

} // OffGSList()


VOID
CurrGad( struct Gadget * gad )
{
    if ( !gad ) {
	D(PRINTF("CurrGad NULL gad!!!\n");)
	return;
    }

    Marquee( PanelWindow->RPort, &MarqueeRect, TRUE);

    MarqueeRect.MinY = gad->TopEdge - 1;
    MarqueeRect.MaxY = MarqueeRect.MinY + gad->Height + 1;

    MarqueeRect.MinX = gad->LeftEdge - 1;
    MarqueeRect.MaxX = MarqueeRect.MinX + gad->Width + 1;

    Marquee( PanelWindow->RPort, &MarqueeRect, FALSE);

    CurrentGad = gad;
    CurrentID = gad->GadgetID;

} // CurrGad()


BOOL
SelectGad( VOID )
{
    GAMECONFIG		* config;
    struct IntuiText	* it;
    UBYTE		  pen;

    D(PRINTF("SelectGad() 1\n");)

    if ( CurrentGad && (config = (GAMECONFIG *)CurrentGad->UserData ) ) {

	if ( it = (struct IntuiText *)CurrentGad->GadgetText ) {
	    it->FrontPen = 0;
	    if ( it = it->NextText )
		it->FrontPen = 7;
	}

	SetABPenDrMd( PanelWindow->RPort, 154, 154, JAM2 );
	RectFill( PanelWindow->RPort, CurrentGad->LeftEdge, CurrentGad->TopEdge,
	    CurrentGad->LeftEdge+CurrentGad->Width,CurrentGad->TopEdge+CurrentGad->Height);

	PrintIText( PanelWindow->RPort, (struct IntuiText *)CurrentGad->GadgetText,
	    CurrentGad->LeftEdge, CurrentGad->TopEdge );

	DrawBorder( PanelWindow->RPort, &GSSelectedBorderRB,
	    CurrentGad->LeftEdge, CurrentGad->TopEdge );

	DrawBorder( PanelWindow->RPort, &GSBorder2,
	    CurrentGad->LeftEdge, CurrentGad->TopEdge );

	if ( it = (struct IntuiText *)CurrentGad->GadgetText ) {
	    it->FrontPen = 7;
	    if ( it = it->NextText )
		it->FrontPen = 0;
	}

	Delay( 20 );
	CurConfig = config;
	docommand( config->command, TRUE );
	CurConfig = NULL;
	return( TRUE );
    }

    return( FALSE );

} // SelectGad()


VOID
PrevGad( VOID )
{
    int		    id;
    struct Gadget * gad;

    if ( CurrentGad ) {
	if ( (id = CurrentGad->GadgetID) == FIRST_ID ) {
	    for ( gad = CurrentGad; gad && gad->NextGadget; gad = gad->NextGadget );
    	    CurrGad( gad );
	} else {
    	    CurrGad( GetGadget( PanelWindow, --id ) );
	}
    } else {
	CurrGad( GetGadget( PanelWindow, FIRST_ID ) );
    }

} // PrevGad()


VOID
NextGad( VOID )
{
    struct Gadget * gad;

    if ( CurrentGad && (gad = CurrentGad->NextGadget) ) {
    	    CurrGad( gad );
    } else {
	CurrGad( GetGadget( PanelWindow, FIRST_ID ) );
    }

} // NextGad


VOID
LeftGad( VOID )
{
    struct Gadget * gad,* prev = NULL;
    int		    i,pos;

    if ( CurrentGad ) {
	// if the gadget is in Column2 (the right column)
	if ( pos = GListItemPosition( GadColumn2, NULL, CurrentGad ) ) {
	    gad = GadColumn1;

	    for ( i = 1; gad && (gad != GadColumn2); i++, gad = gad->NextGadget ) {
		if ( i == pos ) {
		    CurrGad( gad );
		    return;
		}
		prev = gad;
	    }
	    if ( (gad == GadColumn2) && prev)
		CurrGad( prev );

	} else {
	// the gadget is in Column1 (the left column) so do a PrevGad
	    PrevGad();
	}

    } else {
	CurrGad( GetGadget( PanelWindow, FIRST_ID ) );
    }


} // LeftGad()


VOID
RightGad( VOID )
{
    struct Gadget * gad,* prev = NULL;
    int		    i,pos;

    if ( CurrentGad ) {
	// if the gadget is in Column1 (the left column)
	if ( pos = GListItemPosition( GadColumn1, GadColumn2, CurrentGad ) ) {
	    gad = GadColumn2;

	    for ( i = 1; gad; i++, gad = gad->NextGadget ) {
		if ( i == pos ) {
		    CurrGad( gad );
		    return;
		}
		prev = gad;
	    }

	    if ( !gad && prev)
		CurrGad( prev );

	} else {
	// the gadget is in Column2 (the right column) so do a NextGad
	    NextGad();
	}

    } else {
	CurrGad( GetGadget( PanelWindow, FIRST_ID ) );
    }


} // RightGad()


VOID
SetUpCurrGad( VOID )
{
    struct Gadget * gad;

    gad = GetGadget( PanelWindow,  FIRST_ID );

    do {
	MarqueeRect.MinY = gad->TopEdge - 1;
	MarqueeRect.MaxY = MarqueeRect.MinY + gad->Height + 1;

	MarqueeRect.MinX = gad->LeftEdge - 1;
	MarqueeRect.MaxX = MarqueeRect.MinX + gad->Width + 1;
	Marquee( PanelWindow->RPort, &MarqueeRect, TRUE);

    } while (gad = gad->NextGadget);

    CurrentGad = GetGadget( PanelWindow, CurrentID );

    MarqueeRect.MinY = CurrentGad->TopEdge - 1;
    MarqueeRect.MaxY = MarqueeRect.MinY + CurrentGad->Height + 1;

    MarqueeRect.MinX = CurrentGad->LeftEdge - 1;
    MarqueeRect.MaxX = MarqueeRect.MinX + CurrentGad->Width + 1;
    Marquee( PanelWindow->RPort, &MarqueeRect, FALSE);

} // SetUpCurrGad()


OpenMaskScreen( VOID )
{
    struct Window * win;
    struct ColorSpec	colors[] = 
			{
			    {0,0,0,0},
			    {0,0,0,0},
			    {-1,0,0,0}
			};

    if ( MaskScreen = OpenScreenTags(NULL,

	SA_Height,	4,
	SA_Depth,	1,
	SA_Width,	40,
//	SA_Overscan,	OSCAN_MAX,
	SA_DisplayID,	LORES_KEY,
	SA_ShowTitle,	FALSE,
	SA_Quiet,	TRUE,
//	SA_Behind,	TRUE,
	SA_Colors,	colors,
	TAG_DONE) )
    {

	if ( win = OpenWindowTags( NULL,
	 WA_Width,		MaskScreen->Width,
	 WA_Height,		MaskScreen->Height,
	 WA_Flags,		BACKDROP | BORDERLESS,
	 WA_Activate,		TRUE,
	 WA_IDCMP,		NULL,
	 WA_CustomScreen,	MaskScreen,
	 TAG_DONE ) ) {
	    SetPointer( win, InvisiblePointer, 1, 1, 0, 0 );
	}

	D(PRINTF("MaskScreen->Height= %ld, Rows= %ld\n",MaskScreen->Height,MaskScreen->RastPort.BitMap->Rows);)
	return( RC_OK );
    }

} // OpenMaskScreen()


OpenPanel( VOID )
{
    struct ColorSpec	colors[] = 
	{
	    {0,0xAA,0xAA,0xAA},
	    {1,0x00,0x00,0x00},
	    {2,0xFF,0xFF,0xFF},
	    {3,0x00,0x88,0xFF},
	    {-1,0,0,0}
	};

    UWORD pens[] =
	{
	    0, /* DETAILPEN */
	    1, /* BLOCKPEN	*/
	    1, /* TEXTPEN	*/
	    2, /* SHINEPEN	*/
	    1, /* SHADOWPEN	*/
	    3, /* FILLPEN	*/
	    1, /* FILLTEXTPEN	*/
	    0, /* BACKGROUNDPEN	*/
	    2, /* HIGHLIGHTTEXTPEN	*/
    
	    1, /* BARDETAILPEN	*/
	    2, /* BARBLOCKPEN	*/
	    1, /* BARTRIMPEN	*/
    
	    (UWORD)~0,
	};


    if ( PanelScreen = OpenScreenTags(NULL,

//	SA_Height,	PANEL_HT,
	SA_Depth,	PANEL_DEPTH,

	SA_DisplayID,	HIRES_KEY,
	SA_ShowTitle,	FALSE,
	SA_Quiet,	TRUE,
	SA_Behind,	TRUE,
	SA_Colors,	colors,
	SA_Pens,	pens,
	TAG_DONE) )
    {

	D(PRINTF("OpenPanel() PanelScreen->LeftEdge= %ld, TopEdge= %ld, Width= %ld, Height= %ld, Depth= %ld, PanelScreen= 0x%lx\n",
	    PanelScreen->LeftEdge,PanelScreen->TopEdge,PanelScreen->Width,
	    PanelScreen->Height,PanelScreen->RastPort.BitMap->Depth,PanelScreen);)

	PanelVI = GetVisualInfo( PanelScreen, TAG_DONE );

	if ( PanelVI && Config2Gadgets( PanelVI, &PanelGlist ) &&
	   ( PanelWindow = OpenWindowTags( NULL,
	    WA_Width,		PanelScreen->Width,
	    WA_Height,		PanelScreen->Height,
	    WA_IDCMP,		NULL,
	    WA_Flags,		BACKDROP | BORDERLESS,
	    WA_Activate,	TRUE,
	    WA_CustomScreen,	PanelScreen,
	    WA_Gadgets,		PanelGlist,
	    WA_NewLookMenus,	TRUE,
	    TAG_DONE) )) {

	    SetPointer( PanelWindow, InvisiblePointer, 1, 1, 0, 0 );
//	    SetDrMd( PanelWindow->RPort, COMPLEMENT | JAM1 );
	    SetDrMd( PanelWindow->RPort, JAM1 );

	    if ( PanelFrame = NewObject( NULL, "frameiclass",
		IA_FrameType, FRAME_ICONDROPBOX,
		IA_Recessed, TRUE,
		IA_Width, PanelWindow->Width,
		IA_Height, PanelWindow->Height,
		TAG_DONE ) ) {

		DrawImage( PanelWindow->RPort, PanelFrame, 0, 0 );
		RefreshGSList( PanelGlist, PanelWindow, NULL, -1 );
		OffGSList( PanelGlist, PanelWindow, NULL );
	    }

	    SetUpCurrGad();

	    D(PRINTF("OpenPanel() PanelWindow->LeftEdge= %ld, TopEdge= %ld, Width= %ld, Height= %ld, WScreen= 0x%lx\n",
		PanelWindow->LeftEdge,PanelWindow->TopEdge,PanelWindow->Width,
		PanelWindow->Height,PanelWindow->WScreen);)

	    ScreenToFront( PanelScreen );
	    return( RC_OK );
	}

//	CloseScreen( PanelScreen );
//	PanelScreen = NULL;
	ClosePanel();

	return( RC_NO_WIN );
    }

    return( RC_NO_SC );

} // OpenPanel()


DoDisplay( UBYTE * ilbmname )
{
    int		  ret;

    setmem( &DisplayDef, sizeof (DISP_DEF), 0 );

    D(PRINTF("DoDisplay ENTERED with ilbmname= '%ls'\n",
	ilbmname ? ilbmname : "");)

    if ( !(ilbmname && *ilbmname) )
	return( OpenPanel() );

    DisplayDef.Flags |= DISP_SCREEN;
    DisplayDef.Flags |= (DISP_INTERLEAVED|DISP_BACKGROUND);
    DisplayDef.Flags |= DISP_CENTERX;
    DisplayDef.Flags |= DISP_NOPOINTER;

    if ( !DoQuery( ilbmname, &DisplayDef ) ) {
	D(PRINTF("DoDisplay() DoQuery returned FALSE\n");)
	return( OpenPanel() );
    }
/*
    if ( MaskScreen )
	ScreenPosition( MaskScreen, SPOS_ABSOLUTE, 0, -50, 0, 0);
*/
    if ( !(ret = ScrWinOpen( &DisplayDef, ilbmname, NULL ) ) ) {

	PanelScreen = DisplayDef.screen;
	PanelWindow = PanelScreen->FirstWindow;

	if ( PanelVI = GetVisualInfo( PanelScreen, TAG_DONE ) ) {

	    if ( Config2Gadgets( PanelVI, &PanelGlist ) ) {
		AddGList( PanelWindow, PanelGlist, 0, -1, NULL );
		RefreshGSList( PanelGlist, PanelWindow, NULL, -1 );
		OffGSList( PanelGlist, PanelWindow, NULL );
		SetUpCurrGad();
/*
		if ( MaskScreen )
		    ScreenPosition( MaskScreen, SPOS_ABSOLUTE, -100, 600, 0, 0);
*/
		return( RC_OK );
	    }
 	}

    }

    setmem( &DisplayDef, sizeof (DISP_DEF) ,0 );

    D(PRINTF("DoDisplay() END calling OpenPanel\n");)

    return( OpenPanel() );

} // DoDisplay()
 

#define	MIN_ALLOC_SIZE	8

DATA_NODE *
DNodeFree( DATA_NODE * dnode )
{
    DATA_NODE * next;

    D(PRINTF("DNodeFree() 1 dnode= 0x%lx, Size= %ld\n",dnode,dnode->Size);)

    next = dnode->Next;

    FreeMem( dnode, dnode->Size );

    D(PRINTF("DNodeFree() END\n");)

    return( next );

} // DNodeFree()


DATA_NODE *
AllocDataNode( int insize, ULONG flags )
{
    DATA_NODE	* dnode;
    int		  realsize = insize;

    D(PRINTF("AllocDataNode ENTERED with insize= %ld\n",insize);)

    if ( insize % MIN_ALLOC_SIZE )
	realsize = ( (insize / MIN_ALLOC_SIZE) + 1) * MIN_ALLOC_SIZE;

    if ( dnode = AllocMem( realsize, flags ) ) {
	dnode->Size = realsize;
    }

    D(PRINTF("AllocDataNode(): allocated 0x%lx, realsize %ld, dnode->Size %ld\n",
	dnode, realsize, dnode->Size);)

    return( dnode );

} // AllocDataNode()


/****************************************************************************/
/* Returns the preceding item or NULL if no predecessor or item not found.
*/

DATA_NODE *
DNodePred( DATA_NODE * head, DATA_NODE * node )
{
    DATA_NODE	* dn;

    if ( head == node )
	return ( NULL );

    for ( dn = head; dn; dn = dn->Next ) {
	if ( dn->Next == node )
	    return ( dn );
    }

    return ( NULL );

} // DNodePred()


/****************************************************************************/
/* Inserts 'node' into the list after 'pred'.  If pred==NULL, add item at
** head of the list.
*/

VOID
DNodeInsert( DATA_NODE **head, DATA_NODE * node, DATA_NODE * pred )
{
    D(PRINTF("\nDNI() pred= 0x%lx, head= 0x%lx, node= 0x%lx\n",
	pred,head,node);)

    if ( pred == NULL ) {
	pred = (DATA_NODE *)head;
    } else {
	D(PRINTF("DNI() name= '%ls', command= '%ls'\n",
	    ((GAMECONFIG *)pred)->name,((GAMECONFIG *)pred)->command);)
    }

    node->Next = pred->Next;

    pred->Next = node;

} // DNodeInsert()


/****************************************************************************/
/* Unlinks an item from the list.  Returns the preceding item.
*/

DATA_NODE *
DNodeRemove( DATA_NODE **head, DATA_NODE * node )
{
    register DATA_NODE * pred = NULL;

    if ( *head == node ) {
	*head = node->Next;
    } else if ( pred = DNodePred( *head, node ) ) {
	*pred = *node;
    }

    return (pred);

} // DNodeRemove()


VOID
DNodeListFree( DATA_NODE * dnode )
{
    DATA_NODE	* dnode1;

    while( dnode ) {
	dnode1 = dnode->Next;
	D(PRINTF("DNodeListFree: freeing 0x%lx size %ld\n", dnode, dnode->Size ))
	FreeMem( dnode, dnode->Size );
	dnode = dnode1;
    }

} // DNodeListFree()


#ifdef	DEBUG	// [
VOID
DumpGameConfig( VOID )
{
    GAMECONFIG * t;

    D(PRINTF("\nDumping Config List\n\n");)

    for ( t = HeadConfig; t; t = (GAMECONFIG *)t->dnode.Next ) {
	D(PRINTF("name= '%ls' command= '%ls'\n",t->name,t->command);)
    }

} // 
#endif		// ]


AddGameConfig( UBYTE * namebuf, UBYTE * commandbuf )
{
    GAMECONFIG	* config,* pred;
    TIMEOUTNODE * tnode,* tpred;
    UBYTE	* ptr;
    LONG	  size = sizeof (GAMECONFIG) + strlen(namebuf) + 1 + strlen(commandbuf) + 1;
    LONG	  timeout = 0;
    ULONG	  flags = NULL;
    int		  skip;
    BOOL	  timeoutconfig;

    if ( namebuf[0] == '@' ) {
	size--;
	timeoutconfig = TRUE;
	ptr = &namebuf[1];
	D(PRINTF("!!AddGameConfig() found '@'\n");)
	skip = 1;
    } else {
	timeoutconfig = FALSE;
	D(PRINTF("!!AddGameConfig() did NOT find '@'\n");)
	ptr = namebuf;
	skip = 0;
    }

    // '~' denotes that this is an MPEG demo. Check if the MPEG module
    // is attached. If not don't display this button.
    if ( ptr[0] == '~' ) {
	if ( !MPEGModule )
	    return( RC_OK );

	skip++;
	size--;
	ptr++;
    }

    // '-' denotes that this is an NTSC ONLY demo.
    if ( ptr[0] == '-' ) {
	if ( DisplayIsPal )
	    return( RC_OK );

	skip++;
	size--;
	ptr++;
    }

    // '+' denotes that this is a PAL ONLY demo.
    if ( ptr[0] == '+' ) {
	if ( !DisplayIsPal )
	    return( RC_OK );

	skip++;
	size--;
	ptr++;
    }

    if ( ptr[0] == '#' ) {
	UBYTE	* t = strchr( &ptr[1], '#' );

	if ( t ) {
	    timeout = atol( &ptr[1] );
	    skip += (t-ptr)+1;
	    size -= (t-ptr)+1;
	    ptr = t+1;
	    if ( (*(t-1) == '$') || !timeoutconfig )
		flags |= CONFIG_ALWAYS_TIMEOUT;
	}
    }

    // '^' means asyncronously start this command and then unload GameSelect.
    if ( ptr[0] == '^' ) {
	skip++;
	size--;
	ptr++;
	flags |= CONFIG_UNLOAD;
    }

    if ( config = (GAMECONFIG *)AllocDataNode( size, MEMF_CLEAR ) ) {
	config->name = (UBYTE*)(config + 1);
	config->command = config->name + strlen(namebuf) + 1 - skip;
	strcpy( config->name, ptr );
	strcpy( config->command, commandbuf );
	config->timeout = timeout;
	config->flags = flags;
	D(PRINTF("name= '%ls' command= '%ls'\n",
	    config->name,config->command);)

	for ( pred = HeadConfig; pred && pred->dnode.Next; pred = (GAMECONFIG *)pred->dnode.Next );

	D(PRINTF("Calling DNI() with pred= 0x%lx, config= 0x%lx\n",pred,config);)
	DNodeInsert( (DATA_NODE **)&HeadConfig, (DATA_NODE *)config, (DATA_NODE *)pred );

	if ( timeoutconfig ) {
	    if ( tnode = (TIMEOUTNODE *)AllocDataNode( sizeof (*tnode), MEMF_CLEAR ) ) {
		for ( tpred = TimeOutControl.timeouts; tpred && tpred->dnode.Next; tpred = (TIMEOUTNODE *)tpred->dnode.Next );
		D(PRINTF("Calling DNI() with timeoutconfig\n");)
		DNodeInsert( (DATA_NODE **)&TimeOutControl.timeouts, (DATA_NODE *)tnode, (DATA_NODE *)tpred );
		tnode->config = config;
	    }
	}

	return( RC_OK );
    }

    return( RC_NO_MEM );

} // AddGameConfig()


#define	BUF_LEN	100
LoadConfig( UBYTE * file )
{
    FILE		* fp;
    UBYTE		  inbuf[256],namebuf[BUF_LEN],commandbuf[BUF_LEN],*bufptr,* ptr;
    int			  i,len,ret;

    D(PRINTF("LoadConfig ENTERED\n");)

    if( !(fp = fopen(file,"r")) ) {
	D(PRINTF("Could not open file '%s'\n",file);)
	return( RC_CANT_FIND );
    }

    // DO this BEFORE MPEGCapable(), as opening the device, causes the 
    // anim to stutter.

    FreeAnimBase = OpenLibrary("freeanim.library",0L);
    CloseLibrary( FreeAnimBase );
    FreeAnimBase = NULL;

    // Check for the presence of the MPEG module.
    MPEGCapable();

    ret = RC_OK;

    while( !feof( fp ) ) {

	if( !(fgets(inbuf,sizeof(inbuf),fp)) ) {
	    fclose( fp );
	    D(PRINTF("LoadConfig END\n");)
	    D(DumpGameConfig();)
	    return( RC_OK );
	}

	i = strlen( inbuf );
	inbuf[ i-1 ] = 0;
	bufptr = inbuf;

	D(PRINTF("bufptr= '%ls'\n",bufptr);)

	// skip white space
	for ( ; *bufptr && ((*bufptr == ' ') || (*bufptr == '\t')); bufptr++ );
	if ( !*bufptr || (*bufptr == ';') )
	    continue;

	if ( ptr = strchr( bufptr, '=' ) ) {
	    len = (int)ptr - (int)bufptr;
	    if ( len < (BUF_LEN-1) ) {
		strncpy( namebuf, bufptr, len );
		namebuf[len] = 0;
	    } else {
		continue;
	    }

	    ptr++;
	    if ( !*ptr )
		continue;

	    strncpy( commandbuf, ptr, (BUF_LEN-1) );
	    commandbuf[(BUF_LEN-1)] = 0;
	} else {
	    continue;
	}

	if ( ret = AddGameConfig( namebuf, commandbuf ) ) {
	    D(PRINTF("LoadConfig() Out of Memory\n");)
	    break;
	}
	D(DumpGameConfig();)

    }


    fclose( fp );
    return( ret );

} // LoadConfig()


VOID
FreeConfig( VOID )
{
    DNodeListFree( (DATA_NODE *)HeadConfig );
    DNodeListFree( (DATA_NODE *)TimeOutControl.timeouts );

} // FreeConfig()

#ifdef	OUTT	// [

STATIC BOOL
ReadPort( ULONG port )
{
    STATIC	ULONG state = 0L, oldstate, wbcount = 0, secret = 0;

    oldstate = state;
    state = ReadJoyPort( port );

    if ( oldstate != state ) {
	wbcount = 0;
	secret = 0;

	switch ( JP_TYPE_MASK & state ) {

	    case (JP_TYPE_GAMECTLR):
		D(PRINTF("\nGame controller:\n");)
		if (S(JPF_BTN1)) { 
		    D(PRINTF(" Right\n");)
		    IntData.count1 = 0;	// Reset for TIMEOUT
		}
		if (S(JPF_BTN2)) { 
		    D(PRINTF(" Fire\n");)
		    SelectGad();
		    IntData.count1 = 0;	// Reset for TIMEOUT
		}
		if (S(JPF_BTN3)) {
		    D(PRINTF(" JPF_BTN3\n");)
		    IntData.count1 = 0;	// Reset for TIMEOUT
		}
		if (S(JPF_BTN4)) {
		    D(PRINTF(" JPF_BTN4\n");)
		    IntData.count1 = 0;	// Reset for TIMEOUT
		}
/*
		if (S(JPF_BTN6) && S(JPF_BTN5) && S(JPF_BTN7)) {
		    D(PRINTF(" left ear, Middle, right ear\n");)
		}
*/
		if (S(JPF_BTN7)) {
		    D(PRINTF(" Middle\n");)
		    IntData.count1 = 0;	// Reset for TIMEOUT
		}

		if (S(JPF_UP)) {
		    D(PRINTF(" Up direction\n");)
		    PrevGad();
		    IntData.count1 = 0;	// Reset for TIMEOUT
		    break;
		}

		if (S(JPF_DOWN)) { 
		    D(PRINTF(" Down direction\n");)
		    NextGad();
		    IntData.count1 = 0;	// Reset for TIMEOUT
		    break;
		}

		if (S(JPF_LEFT)) {
		    D(PRINTF(" LEFT direction\n");)
		    LeftGad();
		    IntData.count1 = 0;	// Reset for TIMEOUT
		    break;

		}

		if (S(JPF_RIGHT)) {
		    D(PRINTF(" RIGHT direction\n");)
		    RightGad();
		    IntData.count1 = 0;	// Reset for TIMEOUT
		    break;
		}
		break;

	}
    } else if (S(JPF_BTN6) && S(JPF_BTN4) && S(JPF_BTN1)) {
	D(PRINTF(" left ear, Blue, Red... LoadWB wbcount= %ld, target= %ld\n",wbcount,((DisplayIsPal ? 50 : 60)*3));)
	secret = 0;

	if ( ++wbcount > ((DisplayIsPal ? 50 : 60)*3) ) {
	    if ( VBint ) {
		RemIntServer(INTB_VERTB, VBint);
		Delay( 10 );
		FreeMem(VBint, sizeof(struct Interrupt));
		VBint = NULL;
	    }
	    docommand( DoWB, FALSE );
	    return( TRUE );
	}
    } else {
	if ( S(JPF_RIGHT) && S(JPF_BTN6) && S(JPF_BTN5) && S(JPF_BTN4) ) {
	    if ( ++secret > ((DisplayIsPal ? 50 : 60)*3) ) {
		DisplayBeep( NULL );
	    }
	} else if ( (secret > ((DisplayIsPal ? 50 : 60)*3)) && S(JPF_LEFT) && S(JPF_BTN6) && S(JPF_BTN5) && S(JPF_BTN4) ) {
	    if ( ++secret > ((DisplayIsPal ? 50 : 60)*6) ) {
		docommand( DoSECRET, TRUE );
		return( FALSE );
	    }
	} else {
	    secret = 0;
	}
	wbcount = 0;
    }

    return( FALSE );

} // ReadPort()

#else		// ][


STATIC BOOL
ReadPort( ULONG port )
{
    STATIC	ULONG state = 0L, oldstate, wbcount = 0, secret = 0, delay = 0,i = 3;
#ifndef	OUTT	// [
    ULONG	s;

    // Take out later. Only for debugging on my A4000.
    if ( !CDDevice ) {
	s = ReadJoyPort( 0 );

	if ( (JP_TYPE_MASK & s) == JP_TYPE_MOUSE ) {
	    if (JPF_BTN1 & s )
		return( TRUE );
	}
    }
#endif		// ]

    oldstate = state;
    state = ReadJoyPort( port );

    if (S(JPF_BTN6) && S(JPF_BTN4) && S(JPF_BTN1)) {
	D(PRINTF(" left ear, Blue, Red... LoadWB wbcount= %ld, target= %ld\n",wbcount,((DisplayIsPal ? 50 : 60)*3));)
	secret = 0;
	IntData.count1 = 0;	// Reset for TIMEOUT

	if ( ++wbcount > ((DisplayIsPal ? 50 : 60)*3) ) {
	    if ( VBint ) {
		RemIntServer(INTB_VERTB, VBint);
		Delay( 10 );
		FreeMem(VBint, sizeof(struct Interrupt));
		VBint = NULL;
	    }
	    docommand( DoWB, FALSE );
	    return( TRUE );
	}
    } else if ( S(JPF_RIGHT) && S(JPF_BTN6) && S(JPF_BTN5) && S(JPF_BTN4) ) {
	wbcount = 0;
	IntData.count1 = 0;	// Reset for TIMEOUT

	if ( ++secret > ((DisplayIsPal ? 50 : 60)*3) ) {
	    if ( secret == (((DisplayIsPal ? 50 : 60)*3)+1) ) {
		DisplayBeep( NULL );
	    } else {
		secret--;
	    }
	}
    } else if ( (secret > ((DisplayIsPal ? 50 : 60)*3)) && /*S(JPF_LEFT) &&*/ S(JPF_BTN6) && S(JPF_BTN5) && S(JPF_BTN3) ) {
	wbcount = 0;
	IntData.count1 = 0;	// Reset for TIMEOUT

	if ( ++secret > ((DisplayIsPal ? 50 : 60)*6) ) {
	    docommand( DoSECRET, TRUE );
	    secret = 0;
	    return( FALSE );
	}

    } else /*if ( oldstate != state )*/ {
/*
	kprintf("\nsecret= %ld, target= %ld\n",secret,((DisplayIsPal ? 50 : 60)*3));

	if ( S(JPF_BTN1) ) {
	    kprintf("BTN1... BLUE\n");
	}
	if ( S(JPF_BTN2) ) {
	    kprintf("BTN2... RED\n");
	}
	if ( S(JPF_BTN3) ) {
	    kprintf("BTN3... YELLOW\n");
	}
	if ( S(JPF_BTN4) ) {
	    kprintf("BTN4... GREEN\n");
	}
	if ( S(JPF_BTN5) ) {
	    kprintf("BTN5...\n");
	}
	if ( S(JPF_BTN6) ) {
	    kprintf("BTN6...\n");
	}
	if ( S(JPF_BTN7) ) {
	    kprintf("BTN7...\n");
	}
*/
	wbcount = 0;
//	secret = 0;

	if ( oldstate == state ) {
	    wbcount = 0;
	    secret = 0;
	    if ( ++delay < ((DisplayIsPal ? 8 : 11)*i) ) {
		return( FALSE );
	    }
	    i = 1;
	    delay = 0;
	} else {
	    i = 3;
	    delay = 0;
	}

	switch ( JP_TYPE_MASK & state ) {

	    case (JP_TYPE_GAMECTLR):
		D(PRINTF("\nGame controller:\n");)
		if (S(JPF_BTN1)) { 
		    D(PRINTF(" Right\n");)
		    IntData.count1 = 0;	// Reset for TIMEOUT
		}
		if (S(JPF_BTN2)) { 
		    GAMECONFIG	* config;

		    D(PRINTF(" Fire\n");)

		    if ( CurrentGad && (config = (GAMECONFIG *)CurrentGad->UserData ) ) {
			if ( config->flags & CONFIG_UNLOAD ) {
			    CurConfig = config;
			    return( TRUE );
			}
		    }
		    SelectGad();
		    IntData.count1 = 0;	// Reset for TIMEOUT
		}
		if (S(JPF_BTN3)) {
		    D(PRINTF(" JPF_BTN3\n");)
		    IntData.count1 = 0;	// Reset for TIMEOUT
		}
		if (S(JPF_BTN4)) {
		    D(PRINTF(" JPF_BTN4\n");)
		    IntData.count1 = 0;	// Reset for TIMEOUT
		}
/*
		if (S(JPF_BTN6) && S(JPF_BTN5) && S(JPF_BTN7)) {
		    D(PRINTF(" left ear, Middle, right ear\n");)
		}
*/
		if (S(JPF_BTN7)) {
		    D(PRINTF(" Middle\n");)
		    IntData.count1 = 0;	// Reset for TIMEOUT
		}

		if (S(JPF_UP)) {
		    D(PRINTF(" Up direction\n");)
		    secret = 0;
		    PrevGad();
		    IntData.count1 = 0;	// Reset for TIMEOUT
		    break;
		}

		if (S(JPF_DOWN)) { 
		    D(PRINTF(" Down direction\n");)
		    secret = 0;
		    NextGad();
		    IntData.count1 = 0;	// Reset for TIMEOUT
		    break;
		}

		if (S(JPF_LEFT)) {
		    D(PRINTF(" LEFT direction\n");)
		    secret = 0;
		    IntData.count1 = 0;	// Reset for TIMEOUT
		    if ( !GadColumn2 )
			break;
		    LeftGad();
		    break;

		}

		if (S(JPF_RIGHT)) {
		    D(PRINTF(" RIGHT direction\n");)
		    secret = 0;
		    IntData.count1 = 0;	// Reset for TIMEOUT
		    if ( !GadColumn2 )
			break;
		    RightGad();
		    break;
		}
		break;

	}
    }/* else {
	wbcount = 0;
	secret = 0;
    }*/

    return( FALSE );

} // ReadPort()

#endif		// ]

BOOL __saveds
Check4ReBoot( VOID )
{
    STATIC	ULONG state = 0L, oldstate;

//    geta4();

    oldstate = state;
    state = ReadJoyPort( 1 );

    if ( 1 /*(oldstate != state)*/ ) {
	switch ( JP_TYPE_MASK & state ) {
	    case (JP_TYPE_GAMECTLR):
		if (S(JPF_BTN6) && S(JPF_BTN5) && S(JPF_BTN7)) {
		    return( TRUE );
		}
	    break;
	}
    }

    return( FALSE );

} // Check4ReBoot()

__interrupt __asm __saveds
VertBServer( register __a1 INTDATA * idata )
{

//    geta4();

    if ( !(++(idata->count1) % 45) ) {
	D(PRINTF("* ");)
	if ( Check4ReBoot() ) {
	    if ( ++(idata->count2) > 3) {
		D(PRINTF("REBOOT!\n");)
		ColdReboot();
	    }
	} else {
	    idata->count2 = 0;
	}
    }

#ifdef	DO_HANGCOUNTER	// [
    if ( idata->HangCounter ) {
	if ( idata->HangCounter == 1 ) {
	    D(PRINTF("\nGameSelect Exiting DoingAutoDemo= %ld\n",DoingAutoDemo);)
	    ColdReboot();
	}
	idata->HangCounter--;
    }
#endif			// ]

    return( 0 );

} // VertBServer()


/*
 *  SendIOR -- asynchronously execute a device command
 */
BOOL
SendIOR( struct IOStdReq * req, LONG cmd, ULONG off, ULONG len, APTR data)
{
    req->io_Command = cmd;
    req->io_Offset = off;
    req->io_Length = len;
    req->io_Data   = data;

    SendIO( (struct IORequest *)req);

    if ( req->io_Error ) {
	D(PRINTF("SendIOR() ERROR!!! io_Error= %ld\n",req->io_Error);)
	return( FALSE );
    } else {
	return( TRUE );
    }

} // SendIOR()


/*
 *  DoIOR -- synchronously execute a device command
 */
BOOL
DoIOR( struct IOStdReq * req, LONG cmd, ULONG off, ULONG len, APTR data)
{
    req->io_Command = cmd;
    req->io_Offset = off;
    req->io_Length = len;
    req->io_Data   = data;

    DoIO( (struct IORequest *)req);

    if ( req->io_Error ) {
	D(PRINTF("DoIOR() ERROR!!! io_Error= %ld\n",req->io_Error);)
	return( FALSE );
    } else {
	return( TRUE );
    }

} // DoIOR()


#ifdef	OUTT	// [
/*
 * Send a CD_INFO command to cd.device. The info is stored in the cdinfo structure.
 */
BOOL
GetCDInfo( struct CDInfo * cdi )
{
    struct  IOStdReq	__aligned req = *CDDeviceMReq;

    SendIOR( &req, CD_INFO, NULL, sizeof ( struct CDInfo ), cdi );
    WaitIO(  (struct IORequest *)&req );

    if ( !(req.io_Error ) ) {

	D(PRINTF("\nReadXLSpeed= %ld, MaxSpeed= %ld\nSectorSize= %ld, XLECC= %ld\nStatus= %ld\n\n",
	cdi->ReadXLSpeed,cdi->MaxSpeed,cdi->SectorSize,cdi->XLECC,cdi->Status);)

	return( TRUE );
    } else {
	D(PRINTF("CD_INFO ERROR!!! io_Error= %ld\n",req.io_Error);)
	return( FALSE );
    }

} // GetCDInfo()

#else		// ][
/*
 * Send a CD_INFO command to cd.device. The info is stored in the cdinfo structure.
 */
BOOL
GetCDInfo( struct CDInfo * cdi )
{
    SendIOR( CDDeviceMReq, CD_INFO, NULL, sizeof ( struct CDInfo ), cdi );
    WaitIO( (struct IORequest *)CDDeviceMReq );

    if ( !((struct IORequest *)CDDeviceMReq->io_Error ) ) {

	D(PRINTF("\nReadXLSpeed= %ld, MaxSpeed= %ld\nSectorSize= %ld, XLECC= %ld\nStatus= %ld\n\n",
	cdi->ReadXLSpeed,cdi->MaxSpeed,cdi->SectorSize,cdi->XLECC,cdi->Status);)

	return( TRUE );
    } else {
	D(PRINTF("CD_INFO ERROR!!! io_Error= %ld\n",(struct IORequest *)CDDeviceMReq->io_Error);)
	return( FALSE );
    }

} // GetCDInfo()

#endif		// ]


BOOL
PauseCDDA( struct IOStdReq * req, int pause )
{
    req->io_Command = CD_PAUSE;
    req->io_Offset = NULL;
    req->io_Length = pause;	// pause is 1 to pause 0 to unpause
    req->io_Data   = NULL;

    SendIO( (struct IORequest *)req );
    WaitIO( (struct IORequest *)req );

    if ( !(req->io_Error ) ) {

	return( TRUE );
    } else {
	D(PRINTF("CD_PAUSE ERROR!!! io_Error= %ld\n",req->io_Error);)
	return( FALSE );
    }

} // PauseCDDA()


PlayTrack( ULONG track )
{
    struct CDInfo   __aligned cdinfo;

    D(PRINTF("PlayTrack() ENTERED\n");)
    GetCDInfo( &cdinfo );

    if ( cdinfo.Status & CDSTSF_PAUSED ) {
	D(PRINTF("PlayTrack() UNPAUSING\n");)
	PauseCDDA( CDDeviceMReq, 0 ); // Unpause it
    }

    SendIOR( CDDeviceMReq, CD_PLAYTRACK, track, 1, NULL );

    if ( CDDeviceMReq->io_Error ) {
	D(PRINTF("\nPlayTracks io_Error= %ld\n\n",CDDeviceMReq->io_Error);)
	return( FALSE );
    }

    D(PRINTF("PlayTrack() END\n");)

    return( TRUE );

} // PlayTrack()


/*
 * Close cd.device if opened.
 */
VOID
CDDeviceTerm( VOID )
{
    if ( CDDeviceMReq ) {
	if ( CDDevice ) {
	    CloseDevice( (struct IORequest *)CDDeviceMReq );
	    CDDevice = NULL;
	}

	if ( CDPort ) {
	    while( GetMsg( CDPort ) );
	}
	DeleteStdIO( CDDeviceMReq );
	CDDeviceMReq = NULL;
    }

    if ( CDPort ) {
	while( GetMsg( CDPort ) );
	DeleteMsgPort( CDPort );
	CDPort = NULL;
    }

    CDPortSignal = -1;

} // CDDeviceTerm()


/*
 * Attempts to open cd.device if not already opened.
 * Returns:
 *	retcode: (BOOL) reflects device's open state.
 *  *opened: (BOOL) TRUE if opened by this call.
 */
BOOL
CDDeviceInit( ULONG * opened )
{
    if ( opened )
	*opened = FALSE;

    if ( !CDDevice ) {
	D(PRINTF("CDDeviceInit() have to prep CDDevice!");)

	if ( CDPort = CreateMsgPort() ) {
	    D(PRINTF("CDDeviceInit() GOT CDPort\n");)
	    if ( CDDeviceMReq = CreateStdIO( CDPort ) ) {
		D(PRINTF("CDDeviceInit() GOT CDDeviceMReq\n");)

		if ( !OpenDevice( "cd.device", 0, (struct IORequest *)CDDeviceMReq, 0 ) ) {
		    D(PRINTF("CDDeviceInit() Got a Device\n");)
		    CDDevice = CDDeviceMReq->io_Device;
		}
	    }
	}

	if ( !CDDevice ) {
	    D(PRINTF("CDDeviceInit() Failed!! port 0x%lx request 0x%lx device 0x%lx\n",
	    CDPort, CDDeviceMReq, CDDevice );)

	    CDDeviceTerm();
	    return( FALSE );
	}

	CDPortSignal = ( 1 << CDPort->mp_SigBit );
	if ( opened )
	    *opened = TRUE;
    }

    return( TRUE );	

} // CDDeviceInit()

#ifdef	OUTT	// [
VOID
CDAudioTerm( VOID )
{
    struct CDInfo __aligned cdinfo;

    D(PRINTF("CDAudioTerm() 1\n");)

    if( CDDeviceMReq ) {
	D(PRINTF("CDAudioTerm() 1.2\n");)

	GetCDInfo( &cdinfo );

	if ( cdinfo.Status & CDSTSF_PLAYING ) {

//	    struct IOStdReq	__aligned req = *CDDeviceMReq;

	    D(PRINTF("CDAudioTerm() 1.2\n");)
//	    PauseCDDA( &req, 1 );

	    if ( !CheckIO( (struct IORequest *)CDDeviceMReq ) ) {
		D(PRINTF("CDAudioTerm() 2\n");)
		AbortIO( (struct IORequest *)CDDeviceMReq );
	    }
	    D(PRINTF("CDAudioTerm() 3\n");)

	    WaitIO(  (struct IORequest *)CDDeviceMReq );
	    D(PRINTF("CDAudioTerm() 4\n");)
//	    PauseCDDA( &req, 0 );
	}

	D(PRINTF("CDAudioTerm() 5\n");)

	CDDeviceTerm();
    }

    D(PRINTF("CDAudioTerm() END\n");)

} /* CDAudioTerm() */
#else		// ][
VOID
CDAudioTerm( VOID )
{
//    struct CDInfo __aligned cdinfo;

    D(PRINTF("CDAudioTerm() 1\n");)

    if( CDDeviceMReq ) {
	D(PRINTF("CDAudioTerm() 1.2\n");)


//	struct IOStdReq	__aligned req = *CDDeviceMReq;

	D(PRINTF("CDAudioTerm() 1.2\n");)
//	PauseCDDA( &req, 1 );

	if ( !CheckIO( (struct IORequest *)CDDeviceMReq ) ) {
	    D(PRINTF("CDAudioTerm() 2 doing AbortIO()\n");)
	    AbortIO( (struct IORequest *)CDDeviceMReq );
	}
	D(PRINTF("CDAudioTerm() 3 Before WaitIO()\n");)
	D(PRINTF("ior= 0x%lx, io_Command= 0x%lx, io_Flags= 0x%lx, io_Error= %ld\nio_Actual= %ld, io_Length= %ld, io_Data= 0x%lx, io_Offset= %ld, ln_Type= 0x%lx\n",
		CDDeviceMReq,CDDeviceMReq->io_Command,
		CDDeviceMReq->io_Flags,CDDeviceMReq->io_Error,
		CDDeviceMReq->io_Actual,CDDeviceMReq->io_Length,
		CDDeviceMReq->io_Data,CDDeviceMReq->io_Offset,
		CDDeviceMReq->io_Message.mn_Node.ln_Type);)

	WaitIO(  (struct IORequest *)CDDeviceMReq );
	D(PRINTF("CDAudioTerm() After WaitIO()\n");)
//	PauseCDDA( &req, 0 );

	D(PRINTF("CDAudioTerm() 5\n");)

	CDDeviceTerm();
    }

    D(PRINTF("CDAudioTerm() END\n");)

} /* CDAudioTerm() */

#endif		// ]

VOID
main( LONG argc,char * argv[] )
{
    int			  ret,checkplay = 0,i=0,j;
    struct DisplayInfo	  disp;
    BOOL		  DoAudio;
/*
    BOOL		  DoReboot;
    int			  rebootloops;
*/
    D(PRINTF("argc= %ld, argv[0]= '%ls', argv[1]= '%ls'\n",
	argc,argv[0],argv[1]);)

    // workbench
    if ( argc == 0 )
	exit( RETURN_ERROR );

    setmem( opts, sizeof (opts) ,0 );
    setmem( &IntData, sizeof (IntData) ,0 );

    if ( !(UtilityBase = OpenLibrary("utility.library", 0L)) ) {
	D(PRINTF("Could NOT open utility.library\n");)
	exit( RETURN_ERROR );
    }

    rdargs = ReadArgs(TEMPLATE, opts, NULL);

    if ( !rdargs ) {
	PrintFault(IoErr(), NULL);
	D(PRINTF("main() ReadArgs FAILED\n");)
	exit( RETURN_ERROR );
    }

    ILBMname = (UBYTE *)opts[OPT_FROM];

    D(PRINTF("main() ILBMname from CommandLine= '%ls'\n",
	ILBMname ? ILBMname : "");)


    if ( !(ILBMname && *ILBMname) )
	ILBMname = "Demos:GameSelect.pic";

    D(PRINTF("main() ILBMname= '%ls'\n",
	ILBMname ? ILBMname : "");)


    if ( ret = init( (ILBMname && *ILBMname) ? TRUE : FALSE ) ) {
	FreeArgs( rdargs );
	closedown();
	exit( RETURN_ERROR );
    }

    ReadPort( 1 ); // Need to do this once outside of the interrupt


    DisplayIsPal = FALSE;
    if ( GetDisplayInfoData(NULL, (UBYTE *)&disp, sizeof(struct DisplayInfo), DTAG_DISP, LORES_KEY) ) {
	if ( disp.PropertyFlags & DIPF_IS_PAL )
	    DisplayIsPal = TRUE;
    }

    if ( ret = LoadConfig( "Scripts:GameSelect.config" ) ) {
	printf("Error loading GameSelect.config file ret= %ld\n",ret);
	FreeArgs( rdargs );
	closedown();
	exit( RETURN_ERROR );
    }


//    SetTaskPri( FindTask( NULL ), 5L );

#ifdef	OUTT	// [
    if ( GSTextFont.tf_CharSpace ) {
	SHORT	  k,l = (GSTextFont.tf_HiChar - GSTextFont.tf_LoChar);
	WORD	* chspace = (WORD *)GSTextFont.tf_CharSpace;

	for ( k = 1; k < l; k++ ) {
	    chspace[k] += 2;
	}
    }
#endif		// ]

/*
    FreeAnimBase = OpenLibrary("freeanim.library",0L);
    CloseLibrary( FreeAnimBase );
    FreeAnimBase = NULL;
*/
    if ( ret = OpenMaskScreen() )
	goto exit;

#ifdef	DO_CD_AUDIO	// [
    if ( !CDDeviceInit( NULL ) ) {
	printf("CDDeviceInit() FAILED\n");
	DoAudio = FALSE;
    } else {
	DoAudio = TRUE;

	if ( opts[OPT_AUDIOTRACK] ) {
	    AudioTrack = *(LONG *)opts[OPT_AUDIOTRACK];
	} else {
	    AudioTrack = 4;
	}
    }
#else			// ][
    DoAudio = FALSE;
#endif			// ]

    if ( opts[OPT_REBOOTLOOPS] ) {
	rebootloops = *(LONG *)opts[OPT_REBOOTLOOPS];
	DoReboot = TRUE;
    } else {
	DoReboot = FALSE;
    }


#define DO_VBINT
#ifdef	DO_VBINT	// [
    /* Allocate memory for interrupt node */
    if (VBint = AllocMem(sizeof(struct Interrupt), MEMF_PUBLIC|MEMF_CLEAR)) {
	LONG	secs,mins;

        /* initialize the node */
        VBint->is_Node.ln_Type = NT_INTERRUPT;
        VBint->is_Node.ln_Pri = 9;
        VBint->is_Node.ln_Name = "VertReset";
        VBint->is_Data = &IntData;
        VBint->is_Code = (VOID (* )())VertBServer;

	if ( opts[OPT_SECONDS] ) {
	    secs = *(LONG *)opts[OPT_SECONDS];
	} else {
	    secs = 0;
	}

	if ( opts[OPT_MINUTES] ) {
	    mins = *(LONG *)opts[OPT_MINUTES];
	} else {
	    mins = 0;
	}

	if( (secs != -1) && (mins != -1) ) {
	    if ( (mins == 0) && (secs == 0) )
		mins = 2;

	    IntData.DoTimeOut = TRUE;
	    IntData.TimeOut =	(mins * 60 * (DisplayIsPal ? 50 : 60)) + 
				(secs * (DisplayIsPal ? 50 : 60));
	}

        /* kick this interrupt server to life*/
        AddIntServer(INTB_VERTB, VBint);
    }
#endif			// ]

    if ( !DoDisplay( ILBMname ) ) {
	struct Task * reboototask;

//	Delay( 300 );
	if ( DoAudio ) {
	    DoAudio = PlayTrack( AudioTrack );
	} else {
	    D(PRINTF("DoAudio is FALSE\n");)
	}

	if ( reboototask = FindTask( "RebootoTask" ) ) {
	    D(PRINTF("Found RebootoTask!!! sending it a CTRL_C\n");)
	    Signal( reboototask, SIGBREAKF_CTRL_C );
	}

	while ( !ReadPort( 1 ) ) {
	    if ( IntData.DoTimeOut && (IntData.count1 > IntData.TimeOut) ) {
		D(PRINTF("TimeOut!!\n");)

		if ( !TimeOutControl.cur ) {
#ifdef	OUTT	// [
		    if ( DoReboot ) {
			if ( --rebootloops < 0 )
			    /*ColdReboot()*/ ; // Now done in DoCommand
		    }
#endif		// ]
		    TimeOutControl.cur = TimeOutControl.timeouts; 
		}

		if ( TimeOutControl.cur ) {
		    struct Gadget * gad = GetGadget( PanelWindow, FIRST_ID );

		    while ( gad && ( TimeOutControl.cur->config != (GAMECONFIG *)gad->UserData ) )
			gad = gad->NextGadget;

		    if ( gad && ( TimeOutControl.cur->config == (GAMECONFIG *)gad->UserData ) ) {
			CurrGad( gad );
		    }

		    for ( j = 0; j < 90; j++ ) {
			WaitTOF();
			ColorCycle( &DisplayDef );
			if ( ++i > 20 ) {
			    Marquee( PanelWindow->RPort, &MarqueeRect, FALSE );
			    i = 0;
			}
		    }
		    DoingAutoDemo = TRUE;
		    SelectGad();
		    DoingAutoDemo = FALSE;

		    TimeOutControl.cur = (TIMEOUTNODE *)TimeOutControl.cur->dnode.Next;
		}
	    }
	    WaitTOF();
	    ColorCycle( &DisplayDef );
	    if ( ++i > 20 ) {
		Marquee( PanelWindow->RPort, &MarqueeRect, FALSE );
		i = 0;
	    }
	    if ( DoAudio && !(++checkplay % 120) ) {
		if ( CheckIO( (struct IORequest *)CDDeviceMReq ) ) {
		    WaitIO( (struct IORequest *)CDDeviceMReq );
		    PlayTrack( AudioTrack );
		}
	    }
	}

	ClosePanel();
    }

exit:

    FreeArgs( rdargs );
    rdargs = NULL;

    CDAudioTerm();

    if ( VBint ) {
	RemIntServer(INTB_VERTB, VBint);
	FreeMem(VBint, sizeof(struct Interrupt));
    }
    CloseLibrary( UtilityBase );

    if ( CurConfig && (CurConfig->flags & CONFIG_UNLOAD) ) {
	MaskScreen = NULL;		// so closedown() doesn't close MaskScreen.
	D(PRINTF("Calling AsyncCommand\n");)
	AsyncCommand( CurConfig->command );
    } else {
	Delay( 200 );
    }

    closedown();

    if ( rdargs )
	FreeArgs( rdargs );

    if ( !ret ) {
	ret = RETURN_OK;
    } else {
	ret = RETURN_FAIL;
    }

    FreeConfig();

    D(PRINTF("main calling exit\n");)
    exit( ret );

} // main()

