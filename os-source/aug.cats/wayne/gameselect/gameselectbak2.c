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

#include <libraries/asl.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/utility_protos.h>
#include <clib/dos_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/asl_protos.h>
#include <clib/graphics_protos.h>
#include <clib/alib_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/asl_pragmas.h>
#include <pragmas/graphics_pragmas.h>

#include "stdio.h"
#include <stdlib.h>
#include <math.h>	// For min() & max()
#include <string.h>	// for setmem()

#include "retcodes.h"

#include "debugsoff.h"

// Uncomment to get debug output turned on
#define KPRINTF
//#include "debugson.h"


#define GAD_LEFT	15
#define	GAD_TOP		6
#define	GAD_HT		14

#define	PANEL_HT	140
#define	PANEL_DEPTH	2
#define	PANEL_HOME	(GAD_HT+(2*GAD_TOP))

#define	XL_TOP_BORDER	4

#define SCREEN_MODE_ID	100
#define OPEN_ID		101
#define PLAY_ID		102
#define QUIT_ID		103
#define STILL_ID	104
#define FRAME_ID	105
#define FRAME_DISP_ID	106
#define STEP_L_ID	107
#define STEP_R_ID	108
#define INFO_ID		109
#define SAVE_ID		110
#define OPEN_NEW_ID	111
#define CLOSE_ID	112
#define SAVE_ILBM_ID	113
#define SAVE_VID_RAW_ID	114
#define SAVE_AUD_8SVX_ID 115
#define SAVE_AUD_RAW_ID	116
#define FRAME_LOAD_ID	117
#define FRAME_DEL_ID	118
#define FRAME_UNDEL_ID	119
#define SAVE_AS_ID	120
#define CREATE_ID	121
#define SAVE_AS_FLIK_ID	122

#define LMB_DOWN (!((*((BYTE *) 0xbfe001) & 192)==192))


BOOL	DisplayIsPal;

STATIC struct Image	* PanelFrame;

//LONG		  opts[OPT_COUNT];
struct RDArgs	* rdargs;

struct Library		* UtilityBase;
struct Screen		* PanelScreen;
struct Window		* PanelWindow;

struct IntuitionBase	* IntuitionBase;
struct GfxBase		* GfxBase;
struct Library		* IFFParseBase;
struct Library		* FreeAnimBase;


typedef struct DataNode
{
    struct DataNode *Next;
    ULONG Size;
    ULONG Type;

} DATA_NODE;

typedef struct GameConfig
{
    DATA_NODE	  dnode;
    UBYTE	* name;
    UBYTE	* command;

} GAMECONFIG;


GAMECONFIG * HeadConfig;

struct TextAttr Topaz80 =
{
    "topaz.font",
    8,
    0,
    0,
};

STATIC USHORT chip InvisiblePointer[]= {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000};

/*
 * Close every thing down.
 */
//STATIC
VOID
closedown( VOID )
{
    if ( IntuitionBase )
	CloseLibrary( (struct Library *)IntuitionBase );

    if ( GfxBase )
	CloseLibrary( (struct Library *)GfxBase );

    if ( IFFParseBase )
	CloseLibrary( IFFParseBase );

    if ( FreeAnimBase )
	CloseLibrary( FreeAnimBase );

} // closedown()


/*
 * Open all of the required libraries.
 */
init()
{
    if ( !(IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 39L)) ) {
	printf("Could NOT open intuition.library!\n");
	return( RC_FAILED );
    }

    if(!(GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",39L)) ) {
	printf("Could NOT open graphics.library!\n");
	return( RC_FAILED );
    }
/*
    if ( iff ) {
	D(PRINTF("opening iffparse.library!\n");)
	if(!(IFFParseBase = OpenLibrary("iffparse.library",0L)) ) {
	    printf("Could NOT open iffparse.library!\n");
	    return( RC_FAILED );
	}
    }
*/
    kprintf("Opening freeanim.library\n");
    FreeAnimBase = OpenLibrary("freeanim.library",0L);
    kprintf("After opening freeanim.library\n");

    return( RC_OK );

} // init()


/* remove and reply all IntuiMessages on a port that
 * have been sent to a particular window
 * ( note that we don't rely on the ln_Succ pointer
 *  of a message after we have replied it )
 */
VOID
StripIntuiMessages( struct MsgPort * mp, struct Window * win )
{
    struct IntuiMessage *msg;
    struct Node *succ;

    if ( !(mp && win ) )
	return;

    msg = ( struct IntuiMessage * ) mp->mp_MsgList.lh_Head;

    while( succ =  msg->ExecMessage.mn_Node.ln_Succ ) {

	if( msg->IDCMPWindow ==  win ) {

	    /* Intuition is about to free this message.
	     * Make sure that we have politely sent it back.
	     */
	    Remove( ( struct Node * )msg );

	    ReplyMsg( ( struct Message * )msg );
	}
	    
	msg = ( struct IntuiMessage * ) succ;
    }

} // StripIntuiMessages()

VOID
ClosePanel( VOID )
{


    if ( PanelFrame ) {
	DisposeObject( PanelFrame );
	PanelFrame = NULL;
    }

    if ( PanelWindow ) {
	Forbid();
	StripIntuiMessages( PanelWindow->UserPort, PanelWindow );
	ModifyIDCMP( PanelWindow, 0L );
	Permit();
	CloseWindow( PanelWindow );
	PanelWindow = NULL;
    }

    if ( PanelScreen ) {
	CloseScreen( PanelScreen );
	PanelScreen = NULL;
    }

} // ClosePanel()


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


	if ( PanelWindow = OpenWindowTags( NULL,
	    WA_Width,		PanelScreen->Width,
	    WA_Height,		PanelScreen->Height,
	    WA_IDCMP,		IDCMP_GADGETUP|IDCMP_GADGETDOWN|IDCMP_MOUSEBUTTONS|IDCMP_MENUPICK,
	    WA_Flags,		BACKDROP | BORDERLESS,
	    WA_Activate,	TRUE,
	    WA_CustomScreen,	PanelScreen,
	    WA_NewLookMenus,	TRUE,
	    TAG_DONE) ) {

	    SetPointer( PanelWindow, InvisiblePointer, 1, 1, 0, 0 );
	    SetDrMd( PanelWindow->RPort, COMPLEMENT | JAM1 );

	    if ( PanelFrame = NewObject( NULL, "frameiclass",
		IA_FrameType, FRAME_ICONDROPBOX,
		IA_Recessed, TRUE,
		IA_Width, PanelWindow->Width,
		IA_Height, PanelWindow->Height,
		TAG_DONE ) ) {

		DrawImage( PanelWindow->RPort, PanelFrame, 0, 0 );
	    }

	    kprintf("OpenPanel() PanelWindow->LeftEdge= %ld, TopEdge= %ld, Width= %ld, Height= %ld, WScreen= 0x%lx\n",
		PanelWindow->LeftEdge,PanelWindow->TopEdge,PanelWindow->Width,
		PanelWindow->Height,PanelWindow->WScreen);

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

/************************************************************************
**	NextToken.c - MEW 900430
**
**	Scans a whitespace-delimited string and returns the length of and a
**	pointer to the next token in the string.  Any whitespace enclosed
**	in quotes is considered part of the token.  Any whitespace preceeding
**	a token is ignored.  If there isn't another token, this returns NULL.
**
**	On exit, this function returns a pointer to the token in your string.
**	Also, your string pointer will be modified to point immediately after
**	the token, and the token's length is returned.
**
**	Please note: Your original string is not modified; ie: the token
**	string will NOT be null-terminated.
**	
*/

STATIC UBYTE *
NextToken( UBYTE ** str, int * len )
{
    UBYTE	*scan = *str, *tok = NULL;
    UBYTE	quote = 0;

    *len = 0;
    FOREVER  {
	switch ( *scan )  {
	    case 0:
		goto NextToken_exit;

	    case ' ':
	    case '\t':
		scan++;
		break;

	    case '\"':
		quote = *(scan++);
		goto NextToken_1;

	    default:
		goto NextToken_1;
	}
    }

NextToken_1:
    tok = scan;
    FOREVER  {
	switch ( *scan )  {

	    case 0:
		goto NextToken_exit;

	    case ' ':
	    case '\t':
		if ( !quote )
			goto NextToken_exit;
		scan++;
		break;

	    case '\"':
		if ( *(scan++) == quote )
			goto NextToken_exit;
		break;

	    default:
		scan++;
		break;
	}
	(*len)++;
    }

NextToken_exit:
    *str = scan;
    return ( tok );

}  /* NextToken() */


#define	MIN_ALLOC_SIZE	8

DATA_NODE *
DNodeFree( DATA_NODE * dnode )
{
    DATA_NODE * next;

    kprintf("DNodeFree() 1 dnode= 0x%lx, Size= %ld\n",dnode,dnode->Size);

    next = dnode->Next;

    kprintf("DNodeFree() 2\n");

    FreeMem( dnode, dnode->Size );

    kprintf("DNodeFree() END\n");

    return( next );

} // DNodeFree()


DATA_NODE *
AllocDataNode( int insize, ULONG flags )
{
    DATA_NODE	* dnode;
    int		  realsize = insize;

    kprintf("AllocDataNode ENTERED with insize= %ld\n",insize);

    if ( insize % MIN_ALLOC_SIZE )
	realsize = ( (insize / MIN_ALLOC_SIZE) + 1) * MIN_ALLOC_SIZE;

    if ( dnode = AllocMem( realsize, flags ) ) {
	dnode->Size = realsize;
    }

    kprintf("AllocDataNode(): allocated 0x%lx, realsize %ld, dnode->Size %ld\n",
	dnode, realsize, dnode->Size);

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


AddGameConfig( UBYTE * namebuf, UBYTE * commandbuf )
{
    GAMECONFIG	* config,* pred;
    LONG	  size = sizeof (GAMECONFIG) + strlen(namebuf) + 1 + strlen(commandbuf) + 1;

    if ( config = (GAMECONFIG *)AllocDataNode( size, MEMF_CLEAR ) ) {
	config->name = (UBYTE*)(config + 1);
	config->command = config->name + strlen(namebuf) + 1;
	strcpy( config->name, namebuf );
	strcpy( config->command, commandbuf );

	for ( pred = HeadConfig; pred && pred->dnode.Next; pred = (GAMECONFIG *)pred->dnode.Next );

	DNodeInsert( (DATA_NODE **)&HeadConfig, (DATA_NODE *)config, (DATA_NODE *)pred );

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
	kprintf("Could not open file '%s'\n",file);
	return( RC_CANT_FIND );
    }

    ret = RC_OK;

    while( !feof( fp ) ) {

	if( !(fgets(inbuf,sizeof(inbuf),fp)) ) {
	    fclose( fp );
	    return( RC_OK );
	}

	i = strlen( inbuf );
	inbuf[ i-1 ] = 0;
	bufptr = inbuf;

	D(PRINTF("bufptr= '%ls'\n",bufptr);)

	if ( ptr = NextToken( &bufptr, &len ) ) {
	    D(PRINTF("NextToken returned ptr= '%ls', len= %ld\n",ptr,len);)
	    if ( len < (BUF_LEN-1) ) {
		strncpy( namebuf, ptr, len );
		namebuf[len] = 0;
	    } else {
		continue;
	    }
	} else {
	    kprintf("LoadConfig() namebuf.. NextToken returned NULL!!!\n");
	    continue;
	}

	if ( ptr = NextToken( &bufptr, &len ) ) {
	    D(PRINTF("NextToken returned ptr= '%ls', len= %ld\n",ptr,len);)
	    if ( len < (BUF_LEN-1) ) {
		strncpy( commandbuf, ptr, len );
		commandbuf[len] = 0;
	    } else {
		continue;
	    }
	} else {
	    kprintf("LoadConfig() commandbif.. NextToken returned NULL!!!\n");
	    continue;
	}

	if ( ret = AddGameConfig( namebuf, commandbuf ) ) {
	    kprintf("LoadConfig() Out of Memory\n");
	    break;
	}
    }

    fclose( fp );
    return( ret );

} // LoadConfig()


VOID
FreeConfig( VOID )
{
    DNodeListFree( (DATA_NODE *)HeadConfig );

} // FreeConfig()


VOID
PrintConfig( struct Window * win, struct Rectangle * rect )
{
    GAMECONFIG		* config;
    struct TextAttr	* attr = &Topaz80;
    SHORT		  len,max = 0, min = win->Width;
    struct IntuiText	  it;

    setmem( &it, sizeof (it), 0 );
    it.ITextFont = attr;
    it.DrawMode = JAM1;
    it.FrontPen = 2;
    it.TopEdge = 15;

    rect->MinY = it.TopEdge - 1;
    rect->MaxY = rect->MinY + attr->ta_YSize + 1;

    for ( config = HeadConfig; config; config = (GAMECONFIG *)config->dnode.Next ) {
	kprintf("name= '%ls', command= '%ls'\n",config->name,config->command);
	it.IText = config->name;
	len = IntuiTextLength( &it );
	it.LeftEdge = (win->Width >> 1) - (len >> 1);
	PrintIText( win->RPort, &it, 0, 0 );
	it.TopEdge += attr->ta_YSize + 4;
	max = (len > max) ? len : max;
	min = (it.LeftEdge < min) ? it.LeftEdge : min;
    }

    rect->MinX = min - 1;
    rect->MaxX = rect->MinX + max + 1;

} // PrintConfig()


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
Marquee( struct RastPort * rp, struct Rectangle * rect, BOOL redraw )
{
    static int patidx = 0;
    static int pat[4] = {0xcccc,0x6666,0x3333,0x9999};

    SetDrPt( rp, pat[patidx] );
    RubberBox( rp, rect );

    if ( redraw ) {

	if ( (++patidx) == 4 )
		patidx = 0;

	SetDrPt( rp,pat[patidx] );

	RubberBox( rp, rect );
    }

    SetDrPt( rp, 0xffff );

} // Marquee()


VOID
main( LONG argc,char * argv[] )
{
    int			ret;
    struct DisplayInfo	disp;
    struct Rectangle	rect;

    // workbench
    if ( argc == 0 )
	exit( RETURN_ERROR );

//    setmem( opts, sizeof (opts) ,0 );

    if ( !(UtilityBase = OpenLibrary("utility.library", 0L)) ) {
	printf("Could NOT open utility.library\n");
	exit( RETURN_ERROR );
    }
/*
    rdargs = ReadArgs(TEMPLATE, opts, NULL);

    if ( !rdargs ) {
	PrintFault(IoErr(), NULL);
	exit( RETURN_ERROR );
    }
*/

    kprintf("main() 1\n");

    if ( ret = init() ) {
	kprintf("main() init FAILED\n");


	closedown();
	exit( RETURN_ERROR );
    }

    if ( ret = LoadConfig( "PROGDIR:GameSelect.config" ) ) {
	kprintf("Error loading GameSelect.config file ret= %ld\n",ret);
	closedown();
	exit( RETURN_ERROR );
    }

    if ( GetDisplayInfoData(NULL, (UBYTE *)&disp, sizeof(struct DisplayInfo), DTAG_DISP, LORES_KEY) ) {
	if ( disp.PropertyFlags & DIPF_IS_PAL )
	    DisplayIsPal = TRUE;
    }

    kprintf("main() 3\n");
    CloseLibrary( FreeAnimBase );
    FreeAnimBase = NULL;

    if ( !OpenPanel() ) {
	kprintf("main() 5\n");

	PrintConfig( PanelWindow, &rect );

	Marquee( PanelWindow->RPort, &rect, FALSE);
	while ( !LMB_DOWN ) {
	    WaitTOF();
	    WaitTOF();
	    Marquee( PanelWindow->RPort, &rect, TRUE );
	    WaitTOF();
	    WaitTOF();
	}
	Marquee( PanelWindow->RPort, &rect, FALSE);

	ClosePanel();
    }

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

