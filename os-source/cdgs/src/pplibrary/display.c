/* $Id: display.c,v 1.10 93/04/06 15:19:37 peter Exp $ */

#include <exec/types.h>
#include <exec/memory.h>
#include <hardware/intbits.h>

#include <graphics/gfxbase.h>
#include <intuition/intuition.h>
#include <intuition/pointerclass.h>
#include <internal/librarytags.h>

#include <libraries/debox.h>

#include "display.h"
#include "screencoords.h"
#include "display_protos.h"

#include "playerprefs.h"


#include "playerprefsbase.h"
#include "playerprefs_pragmas.h"
#include "playerprefs_protos.h"
#include <clib/debox_protos.h>
#include <pragmas/debox_pragmas.h>

#include <clib/alib_protos.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>
#include <clib/battclock_protos.h>
#include <clib/intuition_protos.h>

#define EXEC_PRIVATE_PRAGMAS
struct Library *TaggedOpenLibrary(ULONG tag);
#include "playerprefs_private_pragmas.h"
#include <pragmas/exec_old_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/battclock_pragmas.h>
#include <pragmas/intuition_pragmas.h>

extern struct Library          *IntuitionBase;
extern struct GfxBase          *GfxBase;
extern struct DeBoxBase        *DeBoxBase;
extern void                    *SysBase;
extern struct PlayerPrefsBase  *PlayerPrefsBase;

struct IBuffer *CurrentIB;
struct IBuffer *WorkIB;
struct IBuffer *MasterIB;
struct MsgPort  *dbufport;
struct Screen *PlayerScreen;
struct Window *PlayerWindow;
struct ColorChanger *PlayerColorChanger;

struct BMInfo   *bmi;
struct BMInfo   *buttonBMI;
struct BitMap   *buttonBM;
struct BitMap   *bm;
UBYTE           *mask;
struct RastPort WorkRP;
UBYTE                   *bmask;
struct BitMap blankpointerbm;
APTR blankpointer;

WORD XOFF;
WORD YOFF;

struct ColorPack PlayerColorPack;
struct ColorRange *PlayerColorRange;

static ULONG Replicate(ULONG colorByte);
static struct IBuffer * AllocIBuffer(struct Screen *sc, struct BMInfo *bmi, int maxgad, int bufferid );
static struct ColorChanger *AllocColorChanger( struct Screen *sc, struct BMInfo *bmi );
static void __saveds taskLoop( void );
static struct Screen * OpenScreenOnlyTags( ULONG firsttag, ... );
static struct Window * OpenWindowOnlyTags( ULONG firsttag, ... );
static APTR myNewObject( struct IClass *classPtr, UBYTE *classID, ULONG firsttag, ... );

extern ULONG __asm LibAnimationControlA(register __a0 struct View *,
    register __a1 struct ViewPort *, register __a2 struct TagItem *, register __a6 struct GfxBase *GfxBase );


/* Here is the color table for scrdata.pic, stored here because debox
 * crashes when decompressing the picture if the new sbox is used,
 * but using the old sbox limits us to 4 bits-per-gun color resolution.
 * This is just taken from a "type scrdata.pic hex" dump, and mangled
 * with an editor macro.
 */

UBYTE ColorTable8[] =
{
    0x3C, 0x6E, 0xC8,
    0x44, 0x44, 0x44,
    0x77, 0x77, 0x77,
    0x88, 0x88, 0x88,
    0x99, 0x99, 0x99,
    0xAA, 0xAA, 0xAA,
    0xBB, 0xBB, 0xBB,
    0xCC, 0xCC, 0xCC,
    0xDD, 0xDD, 0xDD,
    0xFF, 0xFF, 0xFF,
    0xFF, 0x99, 0x66,
    0xEE, 0xBB, 0x00,
    0xFF, 0x33, 0x00,
    0x00, 0xDD, 0xEE,
    0x3C, 0x6E, 0xC2,
    0x35, 0x5E, 0xB4,
    0x2E, 0x4D, 0xA7,
    0x28, 0x3F, 0x99,
    0x22, 0x33, 0x8B,
    0x1C, 0x25, 0x7F,
    0x17, 0x1A, 0x71,
    0x99, 0x99, 0x99,
    0x77, 0x77, 0x77,
    0x55, 0x55, 0x55,
    0x77, 0x77, 0x77,
    0x99, 0x99, 0x99,
    0xCC, 0xCC, 0xCC,
    0xFF, 0xFF, 0xFF,
    0x00, 0xCC, 0x00,
    0x77, 0x99, 0xCC,
    0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF,
    0x06, 0x08, 0x08,
    0x22, 0x22, 0x22,
    0x3B, 0x3B, 0x3B,
    0xD4, 0xD4, 0xD4,
    0xC3, 0xC3, 0xC3,
    0xB2, 0xB2, 0xB2,
    0x55, 0x55, 0x55,
    0x66, 0x66, 0x66,
    0xA1, 0xA1, 0xA1,
    0x90, 0x90, 0x90,
    0x7F, 0x4C, 0x33,
    0x8F, 0x6E, 0x00,
    0x7F, 0x19, 0x00,
    0x00, 0x6E, 0x77,
    0x3B, 0x55, 0x7F,
    0x7F, 0x7F, 0x7F,
    0x4C, 0x55, 0x6E,
    0x63, 0x4A, 0x00,
    0x6E, 0x6E, 0x6E,
    0x5D, 0x5D, 0x5D,
    0x6E, 0x55, 0x4C,
    0x4C, 0x4C, 0x4C,
    0x3B, 0x3B, 0x3B,
    0x2A, 0x2A, 0x2A,
    0x3B, 0x3B, 0x3B,
    0x4C, 0x4C, 0x4C,
    0x66, 0x66, 0x66,
    0x7F, 0x7F, 0x7F,
    0x00, 0x66, 0x00,
    0x4C, 0x4C, 0x4C,
    0xFF, 0x00, 0x00,
    0x11, 0x11, 0x11,
};

/*----------------------- DISPLAY FRAMES ------------------------*/


/* InitIBuffer()
 *
 * Clear all fields of the IBuffer structure.
 * If you pass a non-zero BMInfo pointer, then it sets that, allocates a
 *	ScreenBuffer,
 * It always allocates MAX_PLGAD gadgets, and sets the MaxGad and BoxNo fields.
 */
static struct IBuffer *
AllocIBuffer(struct Screen *sc, struct BMInfo *bmi, int maxgad, int bufferid )
{
    struct IBuffer *ib;
    int ok = 0;
    ULONG sbflags;

    if ( ib = AllocMem( sizeof( struct IBuffer ), MEMF_CLEAR ) )
    {
	ib->ib_Screen = sc;
	if ( ib->ib_GState = AllocMem( maxgad*sizeof( WORD ), MEMF_CLEAR ) )
	{
	    ib->ib_MaxGad = maxgad;
	    ib->ib_BoxNo = -1;

	    sbflags = SB_SCREEN_BITMAP;
	    
	    if ( ib->ib_BMInfo = bmi )
	    {
		sbflags = SB_COPY_BITMAP;
	    }
	    if ( ib->ib_ScreenBuffer = AllocScreenBuffer( sc, NULL, sbflags ) )
	    {
		ib->ib_ScreenBuffer->sb_DBufInfo->dbi_UserData1 = (APTR)bufferid;
		ok = 1;
	    }
	}
	if ( !ok )
	{
#if INCLUDE_CLEANUP
	    FreeIBuffer( ib );
#endif /* INCLUDE_CLEANUP */
	    ib = NULL;
	}
    }
    return( ib );
}

#if INCLUDE_CLEANUP
static VOID
FreeIBuffer( struct IBuffer *ib )
{
    if ( ib )
    {
	FreeScreenBuffer( ib->ib_Screen, ib->ib_ScreenBuffer );
	if ( ib->ib_GState )
	{
	    FreeMem( ib->ib_GState, ib->ib_MaxGad * sizeof( WORD ) );
	}
	FreeMem( ib, sizeof( struct IBuffer ) );
    }
}
#endif /* INCLUDE_CLEANUP */

VOID
SwapIBuffer( VOID )
{
    struct IBuffer *temp;

    temp = CurrentIB;
    CurrentIB = WorkIB;
    WorkIB = temp;
    WorkRP.BitMap = WorkIB->ib_ScreenBuffer->sb_BitMap;
    WaitBlit();
    CurrentIB->ib_ScreenBuffer->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = dbufport;
    *(ULONG *)&(CurrentIB->ib_ScreenBuffer->sb_DBufInfo->dbi_UserData1) |= IBUFFER_INUSE;

    ChangeScreenBuffer( CurrentIB->ib_Screen, CurrentIB->ib_ScreenBuffer );
}

/* Wait until WorkIB is safe to render into
 * NB: We check CurrentIB (the other one), since the SafeMessage
 * comes back when it's safe to render to the OTHER buffer.
 */
void
MakeWorkIBSafe( void )
{
    int firsttime = 1;
    struct Message *dbmsg;

    while ( ((ULONG)CurrentIB->ib_ScreenBuffer->sb_DBufInfo->dbi_UserData1) & IBUFFER_INUSE )
    {
	if ( !firsttime )
	{
	    WaitTOF();
	}
	else
	{
	    firsttime = 0;
	}
	while ( dbmsg = GetMsg( dbufport ) )
	{
	    *((ULONG *)(dbmsg+1)) &= ~IBUFFER_INUSE;
	}
    }
}

/*-------------------------------------------------------------------------*/


long
StartPPScreen(struct GadDir *gl, struct BuildBg_Info *bg, UBYTE *buttondata)
{
    if ( dbufport = CreateMsgPort() )
    {
	struct Rectangle textoscan;
	struct Rectangle maxoscan;
	WORD extraX, extraY;
	struct ColorSpec screencolors[ NUM_COLORS+1 ];
	int i;

	/*
	 * Center the 320x200 screen inside the current OSCAN_TEXT rectangle,
	 * then blow out the left and top edges to OSCAN_MAX.  We do this
	 * because we have elements which fly off-screen on the left and
	 * the top, and we want them to be able to go as far off as possible
	 * without clipping.
	 */
	QueryOverscan( LORES_KEY, &textoscan, OSCAN_TEXT );
	QueryOverscan( LORES_KEY, &maxoscan, OSCAN_MAX );
	/* We know that TextOScan.MinX and .MinY are always zero */
	extraX = ( ( textoscan.MaxX /*- textoscan.MinX*/ + 1 ) - bg->Width ) / 2;
	extraY = ( ( textoscan.MaxY /*- textoscan.MinY*/ + 1 ) - bg->Height ) / 2;

	/* WARNING:  Due to late changes to the scrdata.pic screen,
	 * the contents are not centered.  It was too late in development
	 * to recenter the screen and change all the coordinates, not all
	 * of which have been moved into this file.  Therefore, we must
	 * add a hard-coded offset (SCRDATA_LEFTOFFSET) to account for
	 * the 32 pixels of blank space on the left.
	 */

	extraX -= SCRDATA_LEFTOFFSET/2;
	XOFF = extraX = /*textoscan.MinX*/ - maxoscan.MinX + extraX;
	YOFF = extraY = /*textoscan.MinY*/ - maxoscan.MinY + extraY;
	/* Now re-use the textoscan rectangle to hold the DClip we're
	 * going to open to in a moment.
	 */
	textoscan.MinX = maxoscan.MinX;
	textoscan.MinY = maxoscan.MinY;

	for ( i = 0; i < NUM_COLORS; i++ )
	{
	    screencolors[i].ColorIndex = i;
	    screencolors[i].Red =
		screencolors[i].Green =
		screencolors[i].Blue = 0;
	}
	screencolors[ i ].ColorIndex = ~0;

	if ( PlayerScreen = OpenScreenOnlyTags(
	    SA_DisplayID, LORES_KEY,
	    SA_DClip, &textoscan,
	    SA_Depth, 6,
	    SA_DisplayID, LORES_KEY,
	    SA_Quiet, TRUE,
	    SA_Exclusive, TRUE,
	    SA_Draggable, FALSE,
	    SA_Interleaved, TRUE,
	    SA_Colors, screencolors,
	    TAG_DONE ) )
	{
	    /* Create a blank mouse pointer for our window */
	    InitBitMap( &blankpointerbm, 2, 16, 0 );
	    if ( blankpointer = myNewObject( NULL, "pointerclass",
		POINTERA_BitMap, &blankpointerbm,
		TAG_DONE ) )
	    {
		if ( PlayerWindow = OpenWindowOnlyTags(
		    WA_Borderless, TRUE,
		    WA_RMBTrap, TRUE,
		    WA_SimpleRefresh, TRUE,
		    WA_NoCareRefresh, TRUE,
		    WA_Pointer, blankpointer,
		    WA_CustomScreen, PlayerScreen,
		    WA_BackFill, LAYERS_NOBACKFILL,
		    WA_Activate, TRUE,
		    WA_IDCMP, IDCMP_RAWKEY|IDCMP_MOUSEMOVE|IDCMP_DELTAMOVE|IDCMP_INTUITICKS,
		    WA_ReportMouse, TRUE,
		    TAG_DONE ) )
		{
		    InitRastPort( &WorkRP );

		    if (BuildBg(bg, &bmi, &bm))
		    {
			mask = NULL;

			if (GrabBm(NULL, buttondata, &buttonBMI, &buttonBM, &bmask))
			{
			    int i;
			    UBYTE *c8_ptr = ColorTable8;
			    ULONG *c32_ptr = (ULONG *)PlayerColorPack.cp_Colors;
			    for ( i = 0; i < NUM_COLORS*3; i++ )
			    {
				*c32_ptr++ = Replicate(*c8_ptr++);
			    }
			    PlayerColorPack.cp_NumColors = NUM_COLORS;
			    PlayerColorPack.cp_FirstColor = 0;
			    PlayerColorPack.cp_EndMarker = 0;

			    /* See note about SCRDATA_LEFTOFFSET above!! */
			    BltBitMap( bm, SCRDATA_LEFTOFFSET, 0,
				PlayerScreen->RastPort.BitMap, extraX+SCRDATA_LEFTOFFSET, extraY,
				bmi->bmi_Width-SCRDATA_LEFTOFFSET, bmi->bmi_Height,
				0xC0, 0xFF, NULL );

			    if ( CurrentIB = AllocIBuffer( PlayerScreen, bmi, MAX_PLGAD, 0 ) )
			    {
				if ( WorkIB = AllocIBuffer( PlayerScreen, bmi, MAX_PLGAD, 1 ) )
				{
				    if ( MasterIB = AllocIBuffer( PlayerScreen, NULL, MAX_PLGAD, 0 ) )
				    {
					/* ZZZ cleanup? */
					if ( PlayerColorRange = AllocVec( sizeof( struct ColorRange ) *
					     ( (PlayerScreen->Height/COPPER_RATE) + 1), MEMF_CLEAR ) )
					{
					    /* ZZZ cleanup? */
					    if ( PlayerColorChanger = AllocColorChanger( PlayerScreen, bmi ) )
					    {
						MasterIB->ib_GadList = gl;
						MasterIB->ib_ButtonBM = buttonBM;
						WorkRP.BitMap = WorkIB->ib_ScreenBuffer->sb_BitMap;

						return(0);
					    }
					}
				    }
				}
			    }
			}
		    }
		}
	    }
	}
    }

#if INCLUDE_CLEANUP
    EndPPScreen();
#endif /* INCLUDE_CLEANUP */
    return(1);
}

#if INCLUDE_CLEANUP
VOID
EndPPScreen(VOID)
{
    FreeIBuffer( MasterIB );
    FreeIBuffer( WorkIB );
    FreeIBuffer( CurrentIB );

    if ( PlayerWindow )
    {
	CloseWindow( PlayerWindow );
	PlayerWindow = NULL;
    }

    DisposeObject( blankpointer );
    blankpointer = NULL;

    if ( PlayerScreen )
    {
	CloseScreen( PlayerScreen );
	PlayerScreen = NULL;
    }

    if (mask)
    {
	bm->Planes[bm->Depth++] = mask;
	mask = NULL;
    }
    if (bm)
    {
	FreeBitMap(bm);
	bm = NULL;
    }
    if (bmi)
    {
	FreeBMInfo(bmi);
	bmi = NULL;
    }
    if (bmask)
    {
	buttonBM->Planes[buttonBM->Depth++] = bmask;
	bmask = NULL;
    }
    if (buttonBM)
    {
	FreeBitMap(buttonBM);
	buttonBM = NULL;
    }
    if (buttonBMI)
    {
	FreeBMInfo(buttonBMI);
	buttonBMI = NULL;
    }
    if (dbufport)
    {
	DeleteMsgPort( dbufport );
	dbufport = NULL;
    }
}
#endif /* INCLUDE_CLEANUP */

LONG MouseX = 0;
LONG MouseY = 0;
LONG MouseMovedX = 0;
LONG MouseMovedY = 0;

static int
abs( int val )
{
    return( val > 0 ? val : -val );
}

#define MOUSEMOVE_THRESH	2
#define MOUSEJUMP_THRESH	100
#define MOUSEDECAY_RATE		10


/* StripIDCMPEvents() is supposed to strip any pending IDCMP events that
 * are stale, where stale means older than a few seconds.  The idea here
 * is to eventually flush out any ancient (thus spurious) events, but
 * not to discard anything very recent.  We call StripIDCMPEvents()
 * while waiting for an Audio CD to be reinserted.  Because recent
 * events are not GetMsg()d, the user can "type ahead", i.e. hit
 * play around the same time as the CD is inserted, and the event
 * will stay in the queue long enough for us to be informed that
 * the CD was inserted.
 *
 * Empirically, on my system, I struck the play key simultaneously with
 * closing the CD door.  I observed a delay of 4.2-4.8 seconds between
 * the IntuiMessage and the "current time".  It costs about 1 second per
 * 25 entries in the track table, so for a disk with lots of tracks, it
 * should take a bit longer.  So crudely speaking, 7 seconds should cover
 * all the type-ahead we need.
 */ 
#define STALESECONDS 7

/* Returns the first message from the message port without
 * GetMsg()ing it.
 */
static struct Message *
PeekMsg( struct MsgPort *port )
{
    struct Message *headmsg = (struct Message *)port->mp_MsgList.lh_Head;
    if ( !headmsg->mn_Node.ln_Succ )
    {
	headmsg = NULL;
    }
    return( headmsg );
}

/* Returns TRUE if the IntuiMessage is STALESECONDS old or older.
 * Returns FALSE if the IntuiMessage is NULL or recent.
 */
static ULONG
IsStale( struct IntuiMessage *imsg )
{
    ULONG nowsecs, nowmicros, secs;
    ULONG retval = FALSE;

    if ( imsg )
    {
	CurrentTime( &nowsecs, &nowmicros );
	/* Calculate number of seconds that have elapsed since this
	 * message.  We don't actually subtract microseconds, but
	 * we test to see if we need to borrow if we were to subtract,
	 * because borrowing affects the number of seconds.
	 */
	secs = nowsecs - imsg->Seconds;
	if ( nowmicros < imsg->Micros )
	{
	    secs--;
	}
	if ( secs >= STALESECONDS )
	{
	    retval = TRUE;
	}
    }
    return( retval );
}

void
StripIDCMPEvents( void )
{
    while ( IsStale( (struct IntuiMessage *)PeekMsg( PlayerWindow->UserPort ) ) )
    {
	ReplyMsg( GetMsg( PlayerWindow->UserPort ) );
    }
}

UWORD
GetIDCMPEvent( void )
{
    struct IntuiMessage *imsg;
    UWORD event = 0;

    while ( ( event == 0 ) &&
	( imsg = (struct IntuiMessage *)GetMsg( PlayerWindow->UserPort ) ) )
    {
	if ( imsg->Class == IDCMP_RAWKEY )
	{
	    event = imsg->Code;
	    switch ( event )
	    {
		case KEY_UP:
		case KEY_DOWN:
		case KEY_RIGHT:
		case KEY_LEFT:
		case GC0_UP:
		case GC0_DOWN:
		case GC0_RIGHT:
		case GC0_LEFT:
		case GC1_UP:
		case GC1_DOWN:
		case GC1_RIGHT:
		case GC1_LEFT:
		    /* Only these events support repeat */
		    break;
		default:
		    if ( imsg->Qualifier & IEQUALIFIER_REPEAT )
		    {
			/* Nuke repeat key events */
			event = 0;
		    }
		    break;
	    }

	}
	else if ( imsg->Class == IDCMP_MOUSEMOVE )
	{
	    /* If there is any X movement at all, then we note that fact
	     * in a variable we count down with INTUITICKS.  If that countdown
	     * hits zero, we decay the MouseX towards the origin.  But only if
	     * the X motion exceeds a "noise" threshold, then we listen
	     * to that motion.
	     */
	    if ( imsg->MouseX )
	    {
		MouseMovedX = 2;
	    }

	    if ( abs( imsg->MouseX ) > MOUSEMOVE_THRESH )
	    {
		MouseX += imsg->MouseX;
	    }

	    /* If there is any Y movement at all, then we note that fact
	     * in a variable we count down with INTUITICKS.  If that countdown
	     * hits zero, we decay the MouseY towards the origin.  But only if
	     * the Y motion exceeds a "noise" threshold, then we listen
	     * to that motion.
	     */
	    if ( imsg->MouseY )
	    {
		MouseMovedY = 2;
	    }
	    if ( abs( imsg->MouseY ) > MOUSEMOVE_THRESH )
	    {
		MouseY += imsg->MouseY;
	    }

	    /* If the MouseX or MouseY exceeds the jump threshold
	     * then we issue a joystick event and recenter the mouse
	     * on that axis.
	     */
	    if ( MouseX > MOUSEJUMP_THRESH )
	    {
		MouseX = 0;
		event = KEY_RIGHT;
	    }
	    else if ( MouseX < -MOUSEJUMP_THRESH )
	    {
		MouseX = 0;
		event = KEY_LEFT;
	    }
	    else if ( MouseY > MOUSEJUMP_THRESH )
	    {
		MouseY = 0;
		event = KEY_DOWN;
	    }
	    else if ( MouseY < -MOUSEJUMP_THRESH )
	    {
		MouseY = 0;
		event = KEY_UP;
	    }
	}
	else if ( imsg->Class == IDCMP_INTUITICKS )
	{
	    /* If the countdown variable MouseMovedX hits zero,
	     * then we know the mouse hasn't moved in that axis
	     * recently, and we should decay the mouse position
	     * to zero.  We do this to avoid drift when moving
	     * the mouse predominantly along one axis;  we'd like
	     * to avoid the occasional perpendicular hop.
	     /
	    if ( MouseMovedX )
	    {
		MouseMovedX--;
	    }
	    if ( !MouseMovedX )
	    {
		if ( MouseX > 0 )
		{
		    MouseX -= MOUSEDECAY_RATE;
		    if ( MouseX < 0 )
		    {
			MouseX = 0;
		    }
		}
		else if ( MouseX < 0 )
		{
		    MouseX += MOUSEDECAY_RATE;
		    if ( MouseX > 0 )
		    {
			MouseX = 0;
		    }
		}
	    }

	    /* Likewise for MouseMovedY */
	    if ( MouseMovedY )
	    {
		MouseMovedY--;
	    }
	    if ( !MouseMovedY )
	    {
		if ( MouseY > 0 )
		{
		    MouseY -= MOUSEDECAY_RATE;
		    if ( MouseY < 0 )
		    {
			MouseY = 0;
		    }
		}
		else if ( MouseY < 0 )
		{
		    MouseY += MOUSEDECAY_RATE;
		    if ( MouseY > 0 )
		    {
			MouseY = 0;
		    }
		}
	    }
	}
	ReplyMsg( (struct Message *)imsg );
    }
    return( event );
}

void
AbleColorChanger( LONG on )
{
    struct CycleRange *cr = PlayerColorChanger->Ranges;
    int range;

    for ( range = 0; range < PlayerColorChanger->NumRanges; range++ )
    {
	cr->Enabled = on;
	cr++;
    }
}

static struct ColorChanger *
AllocColorChanger( struct Screen *sc, struct BMInfo *bmi )
{
    static STRPTR name = "PlayerCycler";
    struct Task *cycletask;
    struct ColorChanger *cc;

    if ( cc = AllocVec( sizeof(struct ColorChanger) +
	bmi->bmi_NumRanges * sizeof( struct CycleRange ), MEMF_CLEAR ) )
    {
	struct CycleRange *cr;
	struct RangeInfo *rgi;
	int range;

	cc->ViewPort = &sc->ViewPort;
	cc->NumRanges = bmi->bmi_NumRanges;

	cr = cc->Ranges = ( struct CycleRange *)( cc + 1 );
	rgi = bmi->bmi_RangeInfo;
	for ( range = 0; range < cc->NumRanges; range++ )
	{
	    ULONG *ptr;

	    cr->Enabled = 0;
	    cr->Length = rgi->rgi_High - rgi->rgi_Low + 1;
	    /* Use NTSC values for microseconds per frame, since it
	     * still gives appealing results in PAL
	     */
	    cr->Countdown = cr->NumFrames = rgi->rgi_MicroSeconds / 16666;

	    /* ZZZ failure */
	    if ( ptr = AllocVec( cr->Length * 3 * sizeof( ULONG ) +
		2 * sizeof( ULONG ), MEMF_CLEAR ) )
	    {
		int color;

		cr->LoadTable = ptr;

		*ptr++ = ( cr->Length << 16 ) + rgi->rgi_Low;
		for ( color = rgi->rgi_Low; color <= rgi->rgi_High; color++ )
		{
		    *ptr++ = PlayerColorPack.cp_Colors[ color ].Red;
		    *ptr++ = PlayerColorPack.cp_Colors[ color ].Green;
		    *ptr++ = PlayerColorPack.cp_Colors[ color ].Blue;
		}
		*ptr = 0;	/* terminates LoadRGB32() table */
	    }

	    cr++;
	    rgi++;
	}

	if ( cycletask = CreateTask( name, 21, taskLoop, 4000 ) )
	{
	    cycletask->tc_UserData = cc;
	    Signal( cycletask, SIGBREAKF_CTRL_C );
	}
	else
	{
	    FreeVec( cc );
	}
    }
    return( cc );
}

static void __saveds
taskLoop( void )
{
    int dead = FALSE;
    struct ColorChanger *cc;
    struct CycleRange *cr;
    int range;
    struct Library *GfxBase = TaggedOpenLibrary(OLTAG_GRAPHICS);

    Wait( SIGBREAKF_CTRL_C );
    cc = FindTask( NULL )->tc_UserData;

    while ( !dead )
    {
	/* Look ma!  We're just like a vblank server now */
	WaitTOF();

	if ( SetSignal( 0, SIGBREAKF_CTRL_C ) & SIGBREAKF_CTRL_C )
	{
	    dead = TRUE;
	}
	cr = cc->Ranges;    
	for ( range = 0; range < cc->NumRanges; range++ )
	{
	    if ( ( cr->Enabled ) && ( !--cr->Countdown ) )
	    {
		ULONG *src = &cr->LoadTable[ 3 * cr->Length ];
		ULONG *dest = src;
		ULONG lastblue = *src--;
		ULONG lastgreen = *src--;
		ULONG lastred = *src--;
		int color;

		for ( color = 0; color < 3*(cr->Length - 1); color++ )
		{
		    *dest-- = *src--;
		}
		*dest-- = lastblue;
		*dest-- = lastgreen;
		*dest = lastred;
		cr->Countdown = cr->NumFrames;
		LoadRGB32( cc->ViewPort, cr->LoadTable );
	    }
	    cr++;
	}
    }
    CloseLibrary( GfxBase );
}

static struct Screen *
OpenScreenOnlyTags( ULONG firsttag, ... )
{
     return( OpenScreenTagList( NULL, (struct TagItem *)&firsttag ) );
}

static struct Window *
OpenWindowOnlyTags( ULONG firsttag, ... )
{
     return( OpenWindowTagList( NULL, (struct TagItem *)&firsttag ) );
}

static APTR
myNewObject( struct IClass *classPtr, UBYTE *classID, ULONG firsttag, ... )
{
     return( NewObjectA( classPtr, classID, (struct TagItem *)&firsttag ) );
}





/* We want a copper list which makes color 0 have the following vertical
 * spread:
 */
#define START_RED   60     /* Original CDTV background: 119 */
#define START_GREEN 110    /*                           170 */
#define START_BLUE  200    /*                           255 */
#define END_RED     18     /*                           221 */
#define END_GREEN   8      /*                           170 */
#define END_BLUE    60     /*                           153 */

#if START_RED > END_RED
#define RED_DIFF (START_RED - END_RED)
#else
#define RED_DIFF (END_RED - START_RED)
#endif

#if START_GREEN > END_GREEN
#define GREEN_DIFF (START_GREEN - END_GREEN)
#else
#define GREEN_DIFF (END_GREEN - START_GREEN)
#endif

#if START_BLUE > END_BLUE
#define BLUE_DIFF (START_BLUE - END_BLUE)
#else
#define BLUE_DIFF (END_BLUE - START_BLUE)
#endif


/*****************************************************************************/


static ULONG Replicate(ULONG colorByte)
{
    return ((colorByte << 24) | (colorByte << 16) | (colorByte << 8) | colorByte);
}


/*****************************************************************************/

VOID FadeIn( void )
{
ULONG            i,j,r,g, b;
ULONG            rInc, gInc, bInc;
struct ColorPack colors;
struct TagItem   tags[2];
ULONG copperlines = PlayerScreen->Height/COPPER_RATE;

    colors.cp_NumColors  = NUM_COLORS;
    colors.cp_FirstColor = 0;
    colors.cp_EndMarker  = 0;

    for (i = 0; i < copperlines; i++)
    {
        /* Allocated MEMF_CLEAR:
         * PlayerColorRange[i].cor_Pen    = 0;
         * PlayerColorRange[i].cor_WAIT_X = 0;
         */
        PlayerColorRange[i].cor_WAIT_Y = i*COPPER_RATE;
        PlayerColorRange[i].cor_Flags  = CORF_MODIFY;
    }
	
    PlayerColorRange[i].cor_Pen = -1;

    rInc = Replicate(RED_DIFF) / (copperlines - 1);
    gInc = Replicate(GREEN_DIFF) / (copperlines - 1);;
    bInc = Replicate(BLUE_DIFF) / (copperlines - 1);

    tags[0].ti_Tag  = ACTAG_ColorRange;
    tags[0].ti_Data = (ULONG)PlayerColorRange;
    tags[1].ti_Tag  = TAG_DONE;

    for (i = 0; i <= NUM_FADE_STEPS; i++)
    {
        for (j = 0; j < NUM_COLORS; j++)
        {
            colors.cp_Colors[j].Red   = (PlayerColorPack.cp_Colors[j].Red / NUM_FADE_STEPS) * i;
            colors.cp_Colors[j].Green = (PlayerColorPack.cp_Colors[j].Green / NUM_FADE_STEPS) * i;
            colors.cp_Colors[j].Blue  = (PlayerColorPack.cp_Colors[j].Blue / NUM_FADE_STEPS) * i;
        }

        r = Replicate(START_RED);
        g = Replicate(START_GREEN);
        b = Replicate(START_BLUE);
        for (j = 0; j < copperlines; j++)
        {
            PlayerColorRange[j].cor_Red   = (r / NUM_FADE_STEPS) * i;
            PlayerColorRange[j].cor_Green = (g / NUM_FADE_STEPS) * i;
            PlayerColorRange[j].cor_Blue  = (b / NUM_FADE_STEPS) * i;

#if END_RED > START_RED
            r += rInc;
#else
	    r -= rInc;
#endif

#if END_GREEN > START_GREEN
            g += gInc;
#else
	    g -= gInc;
#endif

#if END_BLUE > START_BLUE
            b += bInc;
#else
	    b -= bInc;
#endif
        }

        WaitTOF();
        LibAnimationControlA(ViewAddress(),&PlayerScreen->ViewPort,tags,GfxBase);
        LoadRGB32(&PlayerScreen->ViewPort,(ULONG *)&colors);

        if (i == 0)
        {
            PlayerColorRange[0].cor_Flags = CORF_MODIFY|CORF_ANIMATE;
            RemakeDisplay();
        }
    }

    LoadRGB32(&PlayerScreen->ViewPort,(ULONG *)&PlayerColorPack);
}


/*****************************************************************************/


VOID FadeOut( void )
{
ULONG            i,j;
struct ColorPack colors;
ULONG            r,g,b;
ULONG            rInc, gInc, bInc;
struct TagItem   tags[2];
ULONG copperlines = PlayerScreen->Height/COPPER_RATE;

    colors.cp_NumColors  = NUM_COLORS;
    colors.cp_FirstColor = 0;
    colors.cp_EndMarker  = 0;

    rInc = Replicate(RED_DIFF) / (copperlines - 1);
    gInc = Replicate(GREEN_DIFF) / (copperlines - 1);;
    bInc = Replicate(BLUE_DIFF) / (copperlines - 1);

    tags[0].ti_Tag  = ACTAG_ColorRange;
    tags[0].ti_Data = (ULONG)PlayerColorRange;
    tags[1].ti_Tag  = TAG_DONE;

    for (i = 0; i <= NUM_FADE_STEPS; i++)
    {
        for (j = 0; j < NUM_COLORS; j++)
        {
            colors.cp_Colors[j].Red   = (PlayerColorPack.cp_Colors[j].Red / NUM_FADE_STEPS) * (NUM_FADE_STEPS - i);
            colors.cp_Colors[j].Green = (PlayerColorPack.cp_Colors[j].Green / NUM_FADE_STEPS) * (NUM_FADE_STEPS - i);
            colors.cp_Colors[j].Blue  = (PlayerColorPack.cp_Colors[j].Blue / NUM_FADE_STEPS) * (NUM_FADE_STEPS - i);
        }

        r = Replicate(START_RED);
        g = Replicate(START_GREEN);
        b = Replicate(START_BLUE);
        for (j = 0; j < copperlines; j++)
        {
            PlayerColorRange[j].cor_Red   = (r / NUM_FADE_STEPS) * (NUM_FADE_STEPS - i);
            PlayerColorRange[j].cor_Green = (g / NUM_FADE_STEPS) * (NUM_FADE_STEPS - i);
            PlayerColorRange[j].cor_Blue  = (b / NUM_FADE_STEPS) * (NUM_FADE_STEPS - i);
#if END_RED > START_RED
            r += rInc;
#else
	    r -= rInc;
#endif

#if END_GREEN > START_GREEN
            g += gInc;
#else
	    g -= gInc;
#endif

#if END_BLUE > START_BLUE
            b += bInc;
#else
	    b -= bInc;
#endif
        }

        WaitTOF();
        LibAnimationControlA(ViewAddress(),&PlayerScreen->ViewPort,tags,GfxBase);
        LoadRGB32(&PlayerScreen->ViewPort,(ULONG *)&colors);

    }

    for (i = 0; i < NUM_COLORS; i++)
    {
        colors.cp_Colors[i].Red   = 0;
        colors.cp_Colors[i].Green = 0;
        colors.cp_Colors[i].Blue  = 0;
    }
    LoadRGB32(&PlayerScreen->ViewPort,(ULONG *)&colors);
}
