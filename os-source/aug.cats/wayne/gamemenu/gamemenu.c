/***********

    GameMenu.c

    W.D.L 930820

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

#include <hardware/custom.h>
#include <hardware/intbits.h>

#include <libraries/asl.h>
#include <libraries/lowlevel.h>

#include "devices/cd.h"

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
#include "anbr.h"

#include "gamemenu_rev.h"

#include "debugsoff.h"

// Uncomment to get debug output turned on
#define KPRINTF
//#include "debugson.h"

#define	GAMEMENULVER	VERSTAG "NTSC Wayne D. Lutz"

#define	HIPRIORITY	50L

UBYTE	* Version = GAMEMENULVER;

ULONG	AudioTrack;

IMPORT struct Custom		  far custom;

// Comment out to NOT play cdaudio
#define		DO_CD_AUDIO

// argument parsing 
#define TEMPLATE    "FROM/A,AUDIOTRACK/K/N,ANBR/K"
#define	OPT_FROM	0
#define	OPT_AUDIOTRACK	1
#define	OPT_ANBR	2
#define	OPT_COUNT	3

#define GAD_LEFT	15
#define	GAD_TOP		50
#define	GAD_HT		(attr->ta_YSize + 4)
#define	GAD_LEFT_SEP	32

#define COLUMN_SEP	32
#define COLUMNS		2

#define	PANEL_DEPTH	2


#define FIRST_ID	100

#define LMB_DOWN (!((*((BYTE *) 0xbfe001) & 192)==192))
#define S(MASK) (MASK & state )

			// 5 min * 60 sec * VB/S
#define	TIMEOUT		(5*60*(DisplayIsPal ? 50 : 60))

#define	LEFT_EAR	(JPF_BTN6)
#define	RIGHT_EAR	(JPF_BTN5)

// protos

VOID CloseDisplay( DISP_DEF * disp_def );
int  DoDisplay( UBYTE * ilbmname );
int  DoQuery( UBYTE * filename, DISP_DEF * disp_def );
int  ScrWinOpen( DISP_DEF * disp_def, UBYTE * ilbmfile, LONG );
int  PlayTrack( ULONG track );
VOID CDDeviceTerm( VOID );
BOOL CDDeviceInit( ULONG * opened );
VOID CDAudioTerm( VOID );
VOID ColorCycle( DISP_DEF * disp_def );


UBYTE outcon[80] = 
	{"CON:10/10/600/200/GameMenu Output Window/AUTO/CLOSE/WAIT"};

UBYTE * DoWB = "execute >nil: sys:s/startup-sequence.wb";

BOOL	DisplayIsPal;


LONG		  opts[OPT_COUNT];
struct RDArgs	* rdargs;


DISP_DEF		  DisplayDef;
UBYTE			* ILBMname;

struct Screen		* PanelScreen;
struct Window		* PanelWindow;

struct Screen		* MaskScreen;


struct Library		* UtilityBase;
struct IntuitionBase	* IntuitionBase;
struct GfxBase		* GfxBase;
struct Library		* IFFParseBase;
struct Library		* FreeAnimBase;
struct Library		* LowLevelBase;


STATIC struct Device	* CDDevice;
STATIC struct MsgPort	* CDPort;

STATIC struct IOStdReq	* CDDeviceMReq;

STATIC LONG	CDPortSignal	= -1;

STATIC LONG	CDAnimpri;

typedef struct DataNode
{
    struct DataNode *Next;
    ULONG Size;
    ULONG Type;

} DATA_NODE;

typedef struct GameConfig
{
    DATA_NODE		  dnode;
    ULONG		  buttonmask;
    struct Rectangle	  rect;
    UBYTE		* name;
    UBYTE		* command;

} GAMECONFIG;

GAMECONFIG * HeadConfig;

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
GAMECONFIG	* CurrentConfig;

struct TextAttr Topaz =
{
    "topaz.font",
    9,
    FSF_BOLD,
    0,
};

STATIC USHORT chip InvisiblePointer[]= {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000};


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
    D(PRINTF("Memory Available: %ld\n", AvailMem(MEMF_PUBLIC) );)


} // MemAvailiable()
#endif		// ]

/*
 * Close every thing down.
 */
//STATIC
VOID
closedown( VOID )
{
#ifdef	OUTT	// [
    if ( MaskScreen ) {

	if ( MaskScreen->FirstWindow )
	    CloseWindow( MaskScreen->FirstWindow );

	CloseScreen( MaskScreen );
	MaskScreen = NULL;
    }
#endif		// ]
    if ( IntuitionBase )
	CloseLibrary( (struct Library *)IntuitionBase );

    if ( GfxBase )
	CloseLibrary( (struct Library *)GfxBase );

    if ( IFFParseBase )
	CloseLibrary( IFFParseBase );
/*
    if ( FreeAnimBase )
	CloseLibrary( FreeAnimBase );
*/
    if ( LowLevelBase )
	CloseLibrary( LowLevelBase );

} // closedown()


/*
 * Open all of the required libraries.
 */
init( BOOL iff )
{
    if ( !(IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 39L)) ) {
	printf("Could NOT open intuition.library! V39\n");
	return( RC_FAILED );
    }

    if(!(GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",39L)) ) {
	printf("Could NOT open graphics.library V39!\n");
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
/*
    D(PRINTF("Opening freeanim.library\n");)
    FreeAnimBase = OpenLibrary("freeanim.library",0L);
    D(PRINTF("After opening freeanim.library\n");)
*/
    return( RC_OK );

} // init()


VOID
ClosePanel( VOID )
{

    D(PRINTF("ClosePanel() 1\n");)

    if ( DisplayDef.Flags & DISP_OPEN ) {
	ScreenToBack( DisplayDef.screen );
	CloseDisplay( &DisplayDef );
    }

    PanelScreen = NULL;
    PanelWindow = NULL;

    D(PRINTF("ClosePanel() END\n");)

} // ClosePanel()


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


VOID
Marquee( struct RastPort * rp, struct Rectangle * rect, BOOL erase )
{
STATIC int	pen = 254;
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

    pen = (pen == 254) ? 180 : 254;


} // Marquee()


VOID
CurrConfig( GAMECONFIG * config )
{
    if ( !config ) {
	D(PRINTF("CurrConfig NULL gad!!!\n");)
	return;
    }

    Marquee( PanelWindow->RPort, &MarqueeRect, TRUE);

    MarqueeRect = config->rect; // structure copy

    Marquee( PanelWindow->RPort, &MarqueeRect, FALSE);

    CurrentConfig = config;

} // CurrConfig()


BOOL
docommand( UBYTE *command )
{
    struct TagItem	  tags[5];
    LONG		  fh;
//    struct Task		* cdanimtask;

    D(PRINTF("\ndocommand ENTERED\n");)
    D(MemAvailable();)

#ifdef	DO_CD_AUDIO	// [
    CDAudioTerm();
#endif			// ]

/*
    Forbid();
    if ( cdanimtask = FindTask( "Flipper" ) ) {
	CDAnimpri = SetTaskPri( cdanimtask, CDAnimpri );
	D(PRINTF("Got cdanimtask, pri= %ld\n",CDAnimpri);)
    }
    Permit();
*/

#ifdef	OUTT	// [
    ScreenToFront( MaskScreen );
    if ( MaskScreen->FirstWindow )
	ActivateWindow( MaskScreen->FirstWindow );
#else		// ][
    if ( MaskScreen )
	 ScreenToBack( MaskScreen );
    if ( PanelScreen )
	 ScreenToBack( PanelScreen );
#endif		// ]

    ClosePanel();

    WaitBlit();
    Delay( 50 );
    WaitBlit();

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
	printf("docommand - Can't open output window\n");
    }

    return( TRUE );

} // docommand()


BOOL
SelectConfig( ULONG state )
{
    GAMECONFIG	* config;

    D(PRINTF("SelectConfig() 1\n");)

    for ( config = HeadConfig; config; config = (GAMECONFIG *)config->dnode.Next ) {
	if ( S(config->buttonmask) )
	    break;
    }

    if ( CurrentConfig = config ) {

	return( TRUE );
    }

    return( FALSE );

} // SelectConfig()


OpenMaskScreen( VOID )
{
    struct Window * win;
    USHORT	  * invisiblePtr;
    struct ColorSpec	colors[] = 
			{
			    {0,0,0,0},
			    {0,0,0,0},
			    {0,0,0,0},
			    {0,0,0,0},
			    {0,0,0,0},
			    {-1,0,0,0}
			};

    if ( MaskScreen = OpenScreenTags(NULL,

	SA_Height,	4,
	SA_Depth,	1,
	SA_Width,	40,
	SA_Top,		600,
	SA_Left,	-100,
	SA_Overscan,	OSCAN_MAX,
	SA_DisplayID,	LORES_KEY,
	SA_ShowTitle,	FALSE,
	SA_Quiet,	TRUE,
	SA_Behind,	TRUE,
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
	    if ( invisiblePtr = (USHORT *)AllocMem( sizeof (InvisiblePointer), MEMF_CHIP ) ) {
		D(PRINTF("sizeof (InvisiblePointer)= %ld\n",sizeof (InvisiblePointer));)
		CopyMem( InvisiblePointer, invisiblePtr, sizeof (InvisiblePointer) );
		SetPointer( win, invisiblePtr, 1, 1, 0, 0 );
	    } else {
		D(PRINTF("Allocation of invisible pointer failed!!\n");)
	    }
	}

	D(PRINTF("MaskScreen->Height= %ld, Rows= %ld\n",MaskScreen->Height,MaskScreen->RastPort.BitMap->Rows);)
	return( RC_OK );
    }

} // OpenMaskScreen()


DoDisplay( UBYTE * ilbmname )
{
    int		  ret;

    setmem( &DisplayDef, sizeof (DISP_DEF), 0 );

    D(PRINTF("DoDisplay ENTERED with ilbmname= '%ls'\n",
	ilbmname ? ilbmname : "");)

    if ( !(ilbmname && *ilbmname) )
	return( RC_MISSING_FILE );

    DisplayDef.Flags |= DISP_SCREEN;
    DisplayDef.Flags |= (DISP_INTERLEAVED|DISP_BACKGROUND);
    DisplayDef.Flags |= DISP_CENTERX;
    DisplayDef.Flags |= DISP_NOPOINTER;

    if ( !DoQuery( ilbmname, &DisplayDef ) ) {
	D(PRINTF("DoDisplay() DoQuery returned FALSE\n");)
	return( RC_BAD_ILBM );
    }


    if ( !(ret = ScrWinOpen( &DisplayDef, ilbmname, NULL ) ) ) {

	PanelScreen = DisplayDef.screen;
	PanelWindow = PanelScreen->FirstWindow;

	return( RC_OK );
    }

    setmem( &DisplayDef, sizeof (DISP_DEF) ,0 );

    return( ret );

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
    if ( pred == NULL ) {
	pred = (DATA_NODE *)head;
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


AddGameConfig( UBYTE * namebuf, UBYTE * commandbuf, ULONG buttonmask )
{
    GAMECONFIG	* config,* pred;
    TIMEOUTNODE * tnode,* tpred;
    UBYTE	* ptr;
    LONG	  size = sizeof (GAMECONFIG) + strlen(namebuf) + 1 + strlen(commandbuf) + 1;
    BOOL	  timeoutconfig;

    if ( namebuf[0] == '@' ) {
	size--;
	timeoutconfig = TRUE;
	ptr = &namebuf[1];
    } else {
	timeoutconfig = FALSE;
	ptr = namebuf;
    }

    if ( config = (GAMECONFIG *)AllocDataNode( size, MEMF_CLEAR ) ) {
	config->name = (UBYTE*)(config + 1);
	config->command = config->name + strlen(namebuf) + 1;
	strcpy( config->name, ptr );
	strcpy( config->command, commandbuf );

	for ( pred = HeadConfig; pred && pred->dnode.Next; pred = (GAMECONFIG *)pred->dnode.Next );

	DNodeInsert( (DATA_NODE **)&HeadConfig, (DATA_NODE *)config, (DATA_NODE *)pred );

	if ( timeoutconfig ) {
	    if ( tnode = (TIMEOUTNODE *)AllocDataNode( sizeof (*tnode), MEMF_CLEAR ) ) {
		for ( tpred = TimeOutControl.timeouts; tpred && tpred->dnode.Next; tpred = (TIMEOUTNODE *)tpred->dnode.Next );
		DNodeInsert( (DATA_NODE **)&TimeOutControl.timeouts, (DATA_NODE *)tnode, (DATA_NODE *)tpred );
		tnode->config = config;
	    }
	}
	config->buttonmask = buttonmask;
	CurrentConfig = config;
	return( RC_OK );
    }

    return( RC_NO_MEM );

} // AddGameConfig()


/*
#define	OSCAR_COMMAND	"execute >NIL: Scripts:dostartup CD0:Oscar"
#define	DIGG_COMMAND	"execute >NIL: Scripts:dostartup CD0:Diggers"
*/

#define	GAME1_COMMAND	"execute >NIL: Scripts:dostartup Game1:"
#define	GAME2_COMMAND	"execute >NIL: Scripts:dostartup Game2:"

SetUpConfig( VOID )
{
    int		  ret;

			// WAS DIGG_COMMAND
    if ( ret = AddGameConfig( "", GAME2_COMMAND, LEFT_EAR ) ) {
	D(PRINTF("SetUpConfig() Out of Memory\n");)
	return( ret );
    }
    CurrentConfig->rect.MinX = 34; //54;
    CurrentConfig->rect.MaxX = CurrentConfig->rect.MinX + 291;
    CurrentConfig->rect.MinY = 173; //220;
    CurrentConfig->rect.MaxY = CurrentConfig->rect.MinY + 235;


			// WAS OSCAR_COMMAND
    if ( ret = AddGameConfig( "", GAME1_COMMAND, RIGHT_EAR ) ) {
	D(PRINTF("SetUpConfig() Out of Memory\n");)
	return( ret );
    }
    CurrentConfig->rect.MinX = 375; //394;
    CurrentConfig->rect.MaxX = CurrentConfig->rect.MinX + 291;
    CurrentConfig->rect.MinY = 173; //220;
    CurrentConfig->rect.MaxY = CurrentConfig->rect.MinY + 235;

    CurrentConfig = HeadConfig;

    return( ret );

} // SetUpConfig()


VOID
FreeConfig( VOID )
{
    DNodeListFree( (DATA_NODE *)HeadConfig );
    DNodeListFree( (DATA_NODE *)TimeOutControl.timeouts );

} // FreeConfig()



STATIC BOOL
ReadPort( ULONG port )
{
    STATIC	ULONG state = 0L, oldstate;

    oldstate = state;
    state = ReadJoyPort( port );

    if ( 1 /*oldstate != state*/) {
	switch ( JP_TYPE_MASK & state ) {

	    case (JP_TYPE_GAMECTLR):
		if ( SelectConfig( state ) )
		    return( TRUE );
		break;
/*
	    case (JP_TYPE_MOUSE):
		if (S(JPF_BTN1))
		    return( TRUE );
		break;
*/
	}
    }

    return( FALSE );

} // ReadPort()


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
	D(PRINTF("SendIOR() ERROR!!! io_Error= %ld\n",req->io_Error);)
	return( FALSE );
    } else {
	return( TRUE );
    }

} // DoIOR()


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


BOOL
PauseCDDA( struct IOStdReq * req, int pause )
{
    req->io_Command = CD_PAUSE;
    req->io_Offset = NULL;
    req->io_Length = pause;	// pause is 1 to pause 0 to unpause
    req->io_Data   = NULL;

    SendIO( (struct IORequest *)req );
    WaitIO(  (struct IORequest *)&req );

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
	D(PRINTF("\nPlayTracks io_Error= %ld\n\n",req.io_Error);)
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
	    D(PRINTF("CDAudioTerm() 2\n");)
	    AbortIO( (struct IORequest *)CDDeviceMReq );
	}
	D(PRINTF("CDAudioTerm() 3\n");)

	WaitIO(  (struct IORequest *)CDDeviceMReq );
	D(PRINTF("CDAudioTerm() 4\n");)
//	PauseCDDA( &req, 0 );

	D(PRINTF("CDAudioTerm() 5\n");)

	CDDeviceTerm();
    }

    D(PRINTF("CDAudioTerm() END\n");)

} /* CDAudioTerm() */

#endif		// ]


__interrupt __asm __saveds
cleanup( register __a0 UBYTE * hardware, register __a1 BLTDATA * bltdata )
{
    D(PRINTF("cleanup: hardware= 0x%lx, bltdata= 0x%lx\n",hardware,bltdata);)
    bltdata->flags &= ~BLIT_QUED;

    return( 0 );

} // cleanup()

__interrupt __asm __saveds
bltfunc( register __a0 UBYTE * hardware, register __a1 BLTDATA * bltdata )
{
    ANBR	* anbr = bltdata->anbr;
    D(UWORD	vhposr = custom.vhposr >> 8;)

    D(PRINTF("%ld ",vhposr);)

    anbr->flags |= ANBR_NEED_2_DRAW;
    Signal( bltdata->task, bltdata->signal );

    D(PRINTF("bltfunc: hardware= 0x%lx, bltdata= 0x%lx, anbr= 0x%lx\n",hardware,bltdata,bltdata->anbr);)

    return( 0 );

} // bltfunc()


decode_next_frame( ANBR * anbr )
{
    long	* deltadata;
    UBYTE	* deltabyte;
    ANIMFRAME	* frame;
    char	* ptr;
    SHORT	  i;

    D(PRINTF("decode_next_frame() ENTERED with ab= 0x%lx\n",ab);)

    frame = anbr->curframe;

    /* If there are no more frames */
    if (!frame) {
	if ( 1 /*!anbr->loop || (--anbr->loop) > 0*/ ) {
	    anbr->curframe = frame = anbr->animframes;
	}

	if (!frame) {
	    return (RC_DONE);
	}
    }

    /* Display the new frame */

    /* Point at the delta data */
    deltadata = (LONG *)frame->delta;

    if (deltadata) {

	for ( i = 0; i < anbr->bitmap->Depth; i++ ) {
	    if ( deltadata[i] ) {
		deltabyte = (UBYTE *)deltadata + deltadata[i];
		if( ptr = anbr->bitmap->Planes[i] ) {
		    D(PRINTF("Calling riff... i=%ld\n",i);)
		    decode_xriff(deltabyte,ptr,anbr->bwidth,anbr->multTable,anbr->height,0,0);
		}
	    }
	}
    }

    anbr->curframe = frame->next;

    return (RC_OK);

} /* decode_next_frame() */


VOID
main( LONG argc,char * argv[] )
{
    int			  ret,checkplay = 0,i=0;
    struct DisplayInfo	  disp;
    BOOL		  DoAudio;
    LONG		  sigbit = -1,signal;
    UBYTE		* anbrname;
    ANBR		* anbr = NULL;
//    struct Task		* cdanimtask;

    D(PRINTF("argc= %ld, argv[0]= '%ls', argv[1]= '%ls'\n",
	argc,argv[0],argv[1]);)

    D(
     for ( j = 0; j < argc; j++ )
	D(PRINTF("argv[%ld]= '%ls'\n",j,argv[j]);)
    )

    // workbench
    if ( argc == 0 )
	exit( RETURN_ERROR );

    setmem( opts, sizeof (opts) ,0 );

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

    if ( opts[OPT_ANBR] ) {
	anbrname = (UBYTE *)opts[OPT_ANBR];
    } else {
	anbrname = NULL;
    }

    D(PRINTF("main() ILBMname from CommandLine= '%ls'\n",
	ILBMname ? ILBMname : "");)


    if ( !(ILBMname && *ILBMname) )
	ILBMname = "Demos:GameMenu.pic";

    D(PRINTF("main() ILBMname= '%ls'\n",
	ILBMname ? ILBMname : "");)


    if ( ret = init( (ILBMname && *ILBMname) ? TRUE : FALSE ) ) {
	FreeArgs( rdargs );
	closedown();
	exit( RETURN_ERROR );
    }

    ReadPort( 1 ); // Need to do this once outside of the interrupt
    Delay( 10 );
    ReadPort( 1 );
    Delay( 10 );

    if ( ret = SetUpConfig() ) {
	printf("Error Setting up ret= %ld\n",ret);
	FreeArgs( rdargs );
	closedown();
	exit( RETURN_ERROR );
    }

    if ( ReadPort( 1 ) )
	goto exit;

    DisplayIsPal = FALSE;
    if ( GetDisplayInfoData(NULL, (UBYTE *)&disp, sizeof(struct DisplayInfo), DTAG_DISP, LORES_KEY) ) {
	if ( disp.PropertyFlags & DIPF_IS_PAL )
	    DisplayIsPal = TRUE;
    }

//    SetTaskPri( FindTask( NULL ), HIPRIORITY );

/*
    CloseLibrary( FreeAnimBase );
    FreeAnimBase = NULL;
*/

    if ( anbrname && (ret = ReadAnbr( anbrname, &anbr )) ) {
	D(PRINTF("ReadAnbr() RETURNED ret= %ld, anbr= 0x%lx\n",ret,anbr);)
	anbr = NULL;
	ret = RC_OK;
    }

    if ( ( sigbit = AllocSignal( -1 ) ) == -1 ) {
	ret =  RC_NO_MEM;
	goto exit;
    }

    if ( ret = OpenMaskScreen() )
	goto exit;

    if ( opts[OPT_AUDIOTRACK] ) {
	AudioTrack = *(LONG *)opts[OPT_AUDIOTRACK];
	DoAudio = TRUE;
    } else {
	DoAudio = FALSE;
    }

#ifdef	DO_CD_AUDIO	// [
    if ( DoAudio ) {
	if ( !CDDeviceInit( NULL ) ) {
	    printf("CDDeviceInit() FAILED\n");
	    DoAudio = FALSE;
	}
    }
#else			// ][
    DoAudio = FALSE;
#endif			// ]

    if ( !DoDisplay( ILBMname ) ) {
	struct View * vlord = ViewAddress();
	struct Task * reboototask;

	if ( DoAudio ) {
	    DoAudio = PlayTrack( AudioTrack );
	} else {
	    D(PRINTF("DoAudio is FALSE\n");)
	}

	if ( reboototask = FindTask( "RebootoTask" ) ) {
	    D(PRINTF("Found RebootoTask!!! sending it a CTRL_C\n");)
	    Signal( reboototask, SIGBREAKF_CTRL_C );
	}

/*
	Forbid();
	if ( cdanimtask = FindTask( "Flipper" ) ) {
	    CDAnimpri = SetTaskPri( cdanimtask, -127 );
	    D(PRINTF("Got cdanimtask, pri= %ld\n",CDAnimpri);)
	} else {
	    D(PRINTF("Did NOT get cdanimtask, pri= %ld\n",CDAnimpri);)
	}
	Permit();
*/
	SetTaskPri( FindTask( NULL ), HIPRIORITY );

	anbr->xpos = 378;//384;
	anbr->ypos = 60;//90;
	anbr->bltdata.bltnode.function = (int (* )())bltfunc;
	anbr->bltdata.bltnode.beamsync = (anbr->height + vlord->DyOffset + anbr->ypos) << 1;
	anbr->bltdata.bltnode.cleanup = (int (* )())cleanup;
	anbr->bltdata.bltnode.stat = CLEANUP;
	anbr->bltdata.signal = signal = (1 << sigbit);
	anbr->bltdata.task = FindTask( NULL );
	anbr->bltdata.anbr = anbr;
	QBSBlit( (struct bltnode *)&anbr->bltdata );

	while ( !ReadPort( 1 ) ) {
	    if ( anbr ) {
		Wait( signal );
		BltBitMap( anbr->bitmap, 0, 0, DisplayDef.screen->RastPort.BitMap,
		    anbr->xpos,anbr->ypos,anbr->width,anbr->height,0xC0,0xFF,NULL);
	    } else {
		WaitTOF();
	    }
	    ColorCycle( &DisplayDef );

	    if ( DoAudio /*&& !(++checkplay % 120)*/ ) {
		if ( CheckIO( (struct IORequest *)CDDeviceMReq ) ) {
		    WaitIO(  (struct IORequest *)CDDeviceMReq );
		    SetTaskPri( FindTask( NULL ), 0L );
		    PlayTrack( AudioTrack );
		    SetTaskPri( FindTask( NULL ), HIPRIORITY );
		}
	    }
	    if ( anbr ) {
		WaitBlit();
		decode_next_frame( anbr );
		QBSBlit( (struct bltnode *)&anbr->bltdata );
	    }
	}

	for ( i = 0; i < 40; i++ ) {
	    if ( anbr ) {
		Wait( signal );

		BltBitMap( anbr->bitmap, 0, 0, DisplayDef.screen->RastPort.BitMap,
		    anbr->xpos,anbr->ypos,anbr->width,anbr->height,0xC0,0xFF,NULL);
	    } else {
		WaitTOF();
	    }
	    ColorCycle( &DisplayDef );

	    if ( !(i % 10) )
		Marquee( PanelWindow->RPort, &CurrentConfig->rect, FALSE);

	    if ( anbr ) {
		WaitBlit();
		decode_next_frame( anbr );
		QBSBlit( (struct bltnode *)&anbr->bltdata );
	    }
	}
	ClosePanel();
    }


exit:

   if ( anbr ) {
	while ( anbr->bltdata.flags & BLIT_QUED )
	    WaitTOF();

	FreeAnbr( anbr );
    }


    if( sigbit != -1 ) {
	FreeSignal( sigbit );
    }

    CDAudioTerm();

    docommand( CurrentConfig->command );

    FreeConfig();

    CloseLibrary( UtilityBase );
    closedown();

    FreeArgs( rdargs );

    if ( !ret ) {
	ret = RETURN_OK;
    } else {
	ret = RETURN_FAIL;
    }

    exit( ret );

} // main()

