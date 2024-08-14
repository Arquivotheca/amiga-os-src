/* $Id: commoncode.c,v 1.12 93/03/31 15:05:34 peter Exp $ */

#include <exec/types.h>
#include <exec/interrupts.h>
#include <exec/libraries.h>
#include <exec/io.h>
#include <exec/memory.h>
#include <devices/input.h>
#include <devices/inputevent.h>
#include <resources/battclock.h>

#include <graphics/gfxbase.h>

#include <libraries/debox.h>
#include "screensaver/screensaver.h"

#include "playerprefs.h"
#include "display.h"
#include "playerprefsbase.h"
#include "playerprefs_pragmas.h"
#include "playerprefs_protos.h"
#include "display_protos.h"
#include <clib/debox_protos.h>
#include <pragmas/debox_pragmas.h>

#include <clib/alib_protos.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>
#include <clib/battclock_protos.h>

#include "playerprefs_private_pragmas.h"
#include <pragmas/exec_old_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/battclock_pragmas.h>

struct Library          *IntuitionBase;
struct GfxBase          *GfxBase;
struct Library          *DeBoxBase;
struct Library          *LowLevelBase;
void                    *SysBase;
struct PlayerPrefsBase  *PlayerPrefsBase;

struct TimeClock                 CDTVTime = {1991,2,28,12,00};
struct CDTVPrefs                 CDTVPrefs;
WORD                             ViewCenterX,ViewCenterY;

extern UBYTE           *mask;
extern struct RastPort WorkRP;
extern UBYTE                   *bmask;

struct CDTVInputInfo    input_data;

char __far TimerDeviceName[] = "timer.device";
char __far  BattClock_Name[] = BATTCLOCKNAME;

extern struct IBuffer *CurrentIB;
extern struct IBuffer *WorkIB;
extern struct IBuffer *MasterIB;

extern WORD XOFF;
extern WORD YOFF;

typedef int (*RENDERFUNC)( struct RastPort *, struct BitMap *, struct GadDir *, WORD, WORD );
typedef int (*BOXFUNC)(struct RastPort *, struct GadDir *, int , UWORD );

int UpdateDisplay( void );

void removehandler(struct PlayerPrefsBase *, struct IOStdReq *, struct Interrupt *);

struct IOStdReq * installhandler(struct PlayerPrefsBase *, struct Interrupt *);

struct IOStdReq *
installhandler(struct PlayerPrefsBase *PlayerPrefsBase, struct Interrupt *ihdata)
{
struct MsgPort         *reply;
struct IOStdReq        *io;

    if (!(reply = (struct MsgPort *)CreateMsgPort())) return (0);

    if (!(io = (struct IOStdReq *)CreateStdIO (reply))) return (0);

    if (OpenDevice ("input.device", 0L, io, 0L))
    {
	io->io_Device = NULL;
	return (0);
    }

    io->io_Command  = IND_ADDHANDLER;
    io->io_Data     = (APTR)ihdata;
    if (!DoIO (io)) return (io);

    if (io)
    {
	if (io->io_Device) CloseDevice(io);
	DeleteStdIO (io);
    }

    if (reply) DeleteMsgPort(reply);

    return (NULL);
}



void
removehandler(struct PlayerPrefsBase *PlayerPrefsBase, struct IOStdReq *io,
    struct Interrupt *ihdata)
{
    if (io)
    {
	if (io->io_Device)
	{
	    io->io_Command = IND_REMHANDLER;
	    io->io_Data = (APTR) ihdata;
	    DoIO (io);
	    CloseDevice (io);
	}

	if (io->io_Message.mn_ReplyPort) DeleteMsgPort(io->io_Message.mn_ReplyPort);

	DeleteStdIO(io);
    }
}


GADSTDBox(struct RastPort *rp, struct GadDir *g, int val, UWORD state)
{
struct GADSTDBox   *gb;
struct GADImageLoc *gl;

int x,y;

    gb = (struct GADSTDBox *)g->BoxData;
    gl = (val ? &gb->oni:&gb->offi);

    x = g->LeftEdge + gb->xoffset + XOFF;
    y = g->TopEdge + gb->yoffset + YOFF;
    BltMaskBitMapRastPort( MasterIB->ib_ButtonBM, gl->xloc, gl->yloc,
	rp, x, y, gb->width, gb->height, (ABC|ABNC|ANBC), bmask);

    return( 0 );
}



GADInsDraw(struct RastPort *rp, struct GadDir *g, int val, UWORD state)
{
static   BYTE  inslen[INS_MAX+1] = { 1, 3, 3, 3, 1, 1, 1, 1 };
BYTE *ptr;
int   x,y;

    /* If this gadget has the GDFLAGS_DISABLABLE attribute and the state
     * is not max, then do nothing, else draw the highlight normally
     */
    if ( ( g->Flags & GDFLAGS_DISABLABLE ) && ( state != g->MaxStates-1 ) )
    {
	return( 1 );
    }
    else
    {
	x = g->LeftEdge + XOFF;
	y = g->TopEdge + YOFF;

	ptr = (UBYTE *)g->BoxData;

	SetDrMd(rp, JAM1);

	while(*ptr != INS_END)
	{
	    switch (*ptr)
	    {
		case INS_PEN:   SetAPen(rp, (long)(val ? ptr[2]:ptr[1]));   break;

		case INS_MOVE:  Move(rp, x+ptr[1],y+ptr[2]);                break;

		case INS_DRAW:  Draw(rp, x+ptr[1],y+ptr[2]);                break;

		case INS_END:                                               return(0);

#if 0 /* Unused */
		case INS_ON_REDRAW:

		    if (val)
		    {
			((RENDERFUNC)g->RenderFunc)(rp, MasterIB->ib_ButtonBM, g, state, state);
			return(0);
		    }

		     else break;
#endif

		case INS_OFF_REDRAW:

		    if (!val)
		    {
			((RENDERFUNC)g->RenderFunc)(rp, MasterIB->ib_ButtonBM, g, state, state);
			return(0);
		    }

		     else break;

#if 0 /* Unused */
		case INS_ONSKIP:
		case INS_OFFSKIP:                                           break;
#endif
	    }

	    ptr += (*ptr < INS_MAX ? inslen[*ptr]:1);
	}

    }
    return(0);
}

static
PowerTen(UBYTE dig)
{
static WORD dignumarr[] = { 0, 1, 10, 100, 1000, 10000 };

    return((int)dignumarr[dig]);
}


static
getdigit(struct GADNumberInfo *gi, UWORD data, WORD digit, int last)
{
int dignum,dig;

    if (data == -1) return(-1);

    if ((gi->flags & GNIF_HALF) && (data & GNIF_HALF) ) dignum = 10;
    else                                                dignum = 0;

    data &= 0xfff;
    dig = (data % PowerTen(digit+1))/PowerTen(digit);

    /* if GNIF_BLANKLEAD is set (request to blank leading zeros)
     *     Blank if next higher digit (last) was blank, and
     *     this isn't the units digit, and the digit is zero.
     * if GNIF_BLANKZERO is set (request to blank the entry zero)
     *     Blank if the whole number is zero.
     * if GNIF_BLANKMAX is set (request to blank when value maxes out)
     *     Blank if the number can't be represented in the alloted digits.
     */
    if (((gi->flags & GNIF_BLANKLEAD) && (last == gi->offnum) && (digit != 1) && (dig==0))
	|| ((gi->flags & GNIF_BLANKZERO) && !data)
	|| ((gi->flags & GNIF_BLANKMAX) && (data >= PowerTen(gi->ndigits+1)))) return((int)gi->offnum);

    else return((dignum + dig));
}


GADNumber(struct RastPort *rp, struct BitMap *buttonBM, struct GadDir *g, WORD old, WORD new)
{
struct GADNumberInfo *gi;
int                            i;
WORD                           o1, n1;
int                            xloc, w, x, y, offx;

    gi = (struct GADNumberInfo *)g->RenderData;

    i    = gi->ndigits;
    n1   =
    o1   = gi->offnum;
    offx = XOFF+g->LeftEdge+gi->xoffset-(gi->flags & GNIF_LEADONE ? gi->nwidth:0);
    y    = YOFF+g->TopEdge + gi->yoffset;

    while(i)
    {
	o1 = getdigit( gi, old, i, o1 );
	n1 = getdigit( gi, new, i, n1 );

	if (o1 != n1)
	{
	    if (n1 < 0) continue;
	    if ((i == gi->ndigits) && (gi->flags & GNIF_LEADONE))
	    {
	        xloc = (n1==1 ? gi->oneloc:gi->blankoneloc);
	        w    = gi->onewidth;
	        x    = XOFF+g->LeftEdge + gi->oneoffset;
	    }

	    else
	    {
	        xloc = gi->xloc + (n1 * gi->width);
	        w    = gi->width;
	        x    = offx;
	    }

	    BltBitMap(buttonBM, xloc, gi->yloc, rp->BitMap, x, y, w, gi->height, 0xc0, 0xff, NULL);
	}

	offx += gi->nwidth;
	i--;
    }

    return(g->Flags & GDFLAGS_SELECTABLE);
}



RNormal(struct RastPort *rp, struct BitMap *buttonBM, struct GadDir *g, WORD old, WORD new)
{
UWORD *d;

    d = (UWORD *)g->RenderData;

    BltBitMapRastPort(buttonBM, d[2*new], d[2*new+1],
	rp, g->LeftEdge+XOFF, g->TopEdge+YOFF,
	g->RightEdge-g->LeftEdge, g->BottomEdge-g->TopEdge, 0xc0);

    return(g->Flags & GDFLAGS_SELECTABLE);
}



/*----------------------- ANIMATION ----------------------*/

UWORD *
DoAnimation(UWORD *ins, ULONG update )
{
    if (*ins == 0xffff) return(NULL);

    while (*ins != 0xffff)
    {
	MasterIB->ib_GState[ *ins ] = ins[1];

	ins += 2;
    }

    if ( update )
    {
	UpdateDisplay();
    }
    ins++;
    return(ins);
}



/************************************************************************
* UpdateDisplay()                                                       *
*                                                                       *
* This routine is responsible for all the updating of the display.  The *
* goal of the routine is to get 'df' looking exactly like 'md' in as    *
* few moves as possible                                                 *
*                                                                       *
* However, we also have to consider if the Master state is different    *
* from the Current state, even if it isn't different from the Work      *
* state.                                                                *
************************************************************************/

/*
 * Master is different from Current; therefore we *must*
 * flip the view.
 **
 * Note: A "more efficient" method of this would involve
 * checking MasterIB against CurrentIB; if there are any
 * differences, then a full loop would be run to synchronize
 * WorkIB with MasterIB.  Otherwise, no rendering to WorkIB is
 * done, even though MasterIB may differ from WorkIB (but not
 * from CurrentIB).
 */

static int
DrawDisplay( void )
{
struct GadDir *g;
UWORD *masterstate, *workstate, *currentstate, oldworkstate;
int i;
int ans = 0;

    /* Wait for WorkIB to be safe to render into */
    MakeWorkIBSafe();

    g = MasterIB->ib_GadList;
    masterstate = MasterIB->ib_GState;
    workstate = WorkIB->ib_GState;
    currentstate = CurrentIB->ib_GState;

    for ( i=0; i<MasterIB->ib_MaxGad; i++, g++, masterstate++, workstate++, currentstate++ )
    {
	if (*masterstate != *currentstate)
	{
	    ans = 1;
	}

	/* Master is the same as Work; so we do no work for this one. */
	if (*masterstate == *workstate)
	{
	    continue;
	}

	/* Handle states wrapping */
	if ( ( *masterstate & 0x0fff ) >= g->MaxStates )
	{
	    *masterstate = (*masterstate & 0xf000)+(*masterstate % g->MaxStates);
	}
	if (*masterstate < 0)
	{
	    *masterstate = g->MaxStates-1;
	}

	/* Update the work state first, since this would allow the gadget
	 * rendering function to override if needed.  (PLGAD_TTRACK does!)
	 */
	oldworkstate = *workstate;
	*workstate = *masterstate;
	if ( ((RENDERFUNC)g->RenderFunc)( &WorkRP, MasterIB->ib_ButtonBM, g,
	    oldworkstate, *masterstate ) && ( i == WorkIB->ib_BoxNo ) )
	{
	    /* If the highlight has moved, there won't be a need to erase
	     * the highlight off the currently highlighted one since we
	     * just repainted it
	     */
	    WorkIB->ib_BoxNo = -1;
	}
    }

    if (MasterIB->ib_BoxNo != WorkIB->ib_BoxNo)
    {
	/* If there was an old highlight, erase it */
	if (WorkIB->ib_BoxNo >= 0)
	{
	    UWORD gstate = MasterIB->ib_GState[WorkIB->ib_BoxNo];
	    g = &MasterIB->ib_GadList[ WorkIB->ib_BoxNo ];
	    ((BOXFUNC)g->BoxFunc)( &WorkRP, g, 0, gstate );
	}

	/* If there is a new highlight, render it */
	if (MasterIB->ib_BoxNo >= 0)
	{
	    UWORD gstate = MasterIB->ib_GState[MasterIB->ib_BoxNo];
	    g = &MasterIB->ib_GadList[ MasterIB->ib_BoxNo ];
	    ((BOXFUNC)g->BoxFunc)( &WorkRP, g, 1, gstate );
	}

	WorkIB->ib_BoxNo = MasterIB->ib_BoxNo;
	ans = 1;
    }

    if (ans) MasterIB->ib_Updated = 1;

    return(ans);
}


int
UpdateDisplay( void )
{
    DrawDisplay();
    if ( MasterIB->ib_Updated )
    {
	SwapIBuffer();
	MasterIB->ib_Updated = 0;
	return( 1 );
    }
    return( 0 );
}



void
BoxMove(struct IBuffer *ib, UWORD event)
{
struct GadDir *g;

    if (ib->ib_BoxNo < 0)
    {
	ib->ib_BoxNo = 0;
	return;
    }
    g = &ib->ib_GadList[ ib->ib_BoxNo ];

    switch(event)
    {
	case KEY_UP:    event = g->Up;          break;
	case KEY_DOWN:  event = g->Down;        break;
	case KEY_LEFT:  event = g->Left;        break;
	case KEY_RIGHT: event = g->Right;       break;

	default:        return;
    }

    if (event == PLGAD_NL) return;

    ib->ib_BoxNo = event;
}

/*-----------------------------------------------------------------------*/

handlescrsave(struct CycleIntInfo *ci, int blank, struct PlayerPrefsBase *PlayerPrefsBase)
{
    if (blank == -1)
    {
	ScreenSaverCommand( SCRSAV_FAKEIT );
	ci->cintd_DestFade0 = 0;
	return(0);
    }

    if (ScreenSaverCommand(SCRSAV_ISACTIVE))
    {
	if ((blank == 0) && (ci->cintd_Fade0 == 15))
	{
	    blank = 1;
	    ScreenSaverCommand( SCRSAV_UNFAKEIT );
	}

	ci->cintd_DestFade0 = 15;
    }

    else
    {
	if (blank == 1)
	{
	    blank = 0;
	    ScreenSaverCommand( SCRSAV_FAKEIT );
	}

	ci->cintd_DestFade0 = 0;
    }

    return(blank);
}



/*=========================== CLOCK FUNCTIONS ============================*/


static void
MakeDate(ULONG time, struct TimeClock *tc)
{
long n,m,y;

    n = (time/SECS_DAY) + 671;

    if (n <= 0)
    {
	y = 1978 ;
	m = 1 ;
	tc->day = 1 ;
    }

    else
    {
	y       =  (4 * n + 3) / 1461 ;
	n       -= 1461 * y / 4 ;
	y       += 1976;
	m       =  (5 * n + 2) / 153 ;
	tc->day =  n - (153 * m + 2) / 5 + 1 ;
	m       += 3 ;

	if (m > 12)
	{
	    y++;
	    m -= 12;
	}
    }

    tc->month = m;
    tc->year  = y;

    n  = (time % SECS_DAY)/60;

    tc->min  = n % 60;
    tc->hour = n / 60;
}



static
FillCDTVTime(struct TimeClock *tc)
{
struct Library *BattClockBase;
struct TimeClock tempc;
ULONG tempsec;

    if (!(BattClockBase = OpenResource( BattClock_Name ))) return(1);

    tempsec=ReadBattClock();
    MakeDate( tempsec,&tempc );

    CopyMem( &tempc, tc, sizeof(struct TimeClock));

    return(0);
}




/*========================================================================--*
* InitBase() - Init base libs in local varables                             *
*                                                                                                                                                       *
*---------------------------------------------------------------------------*/


VOID
InitVarBase(struct PlayerPrefsBase *ppb)
{
struct View view;

    SysBase         = ppb->ppb_ExecBase;
    GfxBase         = ppb->ppb_GfxBase;
    IntuitionBase   = ppb->ppb_IntuitionBase;
    DeBoxBase       = ppb->ppb_DeBoxBase;
    LowLevelBase    = ppb->ppb_LowLevelBase;
    PlayerPrefsBase = ppb;

    InitView(&view);
    ViewCenterX = view.DxOffset;
    ViewCenterY = view.DyOffset;

    FillCDTVPrefs(&CDTVPrefs);
    FillCDTVTime(&CDTVTime);

    ConfigureCDTV( &CDTVPrefs );
}

