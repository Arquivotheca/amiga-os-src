/* $Id: player.c,v 1.28 93/04/07 23:37:36 peter Exp $ */

#include <exec/types.h>
#include <exec/io.h>
#include <exec/execbase.h>
#include <exec/memory.h>

#include <graphics/gfxbase.h>
#include <devices/input.h>
#include <devices/inputevent.h>

#include <cd/cd.h>

#include <cdtv/cdtvprefs.h>
#include <libraries/debox.h>

#include "/screensaver/screensaver.h"

#include "/screencoords.h"
#include "/playerprefs.h"
#include "/display.h"
#include "/playerprefsbase.h"

#include "/playerprefs_pragmas.h"
#include "/playerprefs_protos.h"
#include "/display_protos.h"
#include "/other_protos.h"

#include <clib/alib_protos.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/debox_protos.h>
#include <cdg/cdg_cr_protos.h>

#include <pragmas/exec_old_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/debox_pragmas.h>
#include <cdg/cdg_cr_pragmas.h>

#include <cdg/cdgprefs.h>

#include <libraries/lowlevel.h>
#include <clib/lowlevel_protos.h>
#include <pragmas/lowlevel_pragmas.h>

#include <internal/playerlibrary.h>
#include <clib/player_protos.h>
#include <pragmas/player_pragmas.h>

#include <string.h>

/*************************** External Functions ****************************/

extern BOOL             CDGBegin();
extern VOID             CDGEnd();
extern VOID             CDGChannel(ULONG);
extern VOID             EndPPScreen(VOID);
extern VOID             InitVarBase(struct PlayerPrefsBase *ppb);
extern UWORD *          DoAnimation(UWORD *ins, ULONG update );
extern ULONG NOOP(void);

/********************* External Structures/Variables ***********************/

extern struct GfxBase          *GfxBase;
extern struct Library          *IntuitionBase;
extern struct DeBoxBase        *DeBoxBase;
extern struct ExecBase         *SysBase;
extern struct PlayerPrefsBase  *PlayerPrefsBase;
extern struct Library          *LowLevelBase;

extern struct IBuffer          *CurrentIB;
extern struct IBuffer          *WorkIB;
extern struct IBuffer          *MasterIB;

extern UWORD volatile __far     vhposr;                     /* Custom chip register. */
extern char __far               TimerDeviceName[];

extern UBYTE __far              scrdata[];

/******************************* Functions *********************************/

static void  PlayerLoop(struct CDGPrefs *);
static void  DoKeyStroke(UWORD);
void  UpdateGadgets(void);
static UWORD *SetupPlayer(void);
static void  ShutDown(void);
static void  DoReset(void);
static void  AbortQCode(void);
static int SubmitKeys(UWORD event);
static void SetPlayList( ULONG enable );
static void PreparePlayList( void );
static void UnpreparePlayList( void );

/************************* Structures/Variables ****************************/

struct BuildBg_Info __far PlayerBg =
{
    NULL,		/* struct Header *BackHeader */
    NULL,		/* UBYTE *BackData (strip) */
    NULL,		/* struct Header *ForHeader */
    scrdata,		/* UBYTE *ForData (foreground image) */
    320, 200, 6,	/* Width, Height, Depth */
    0, 0,		/* ForXOff, ForYOff */
    NULL		/* Flags */
};

UBYTE  IntroLastTrack = 0;
UWORD  IntroCount;
WORD  IntroFlashTicks = 0;
WORD noscrolltracks = 0;
UBYTE  LastPlayState, LastPlayMode;
LONG ShuffleMode = 0;
LONG StopCount = 0;
LONG haveCD = 0;
LONG playertofront = 0;
extern ULONG randomseed;
UBYTE TotalMinutes = 100;
UBYTE TotalSeconds = 100;
ULONG NumSelectedTracks = 0;

union  CDTOC         *TOC;
struct QCode          qcode, copyqcode;
int                   qvalid, qoutstanding, toutstanding;
struct MsgPort       *cdreply, *qcodereply, *TimeReplyPort;
struct IOStdReq      *cdio, *cdqcode;
struct timerequest   *TimeReq;

struct PlayerOptions  PlayerOptions, OptionsSet;
struct PlayerState    PlayerState;
struct PlayList      *PlayList;
struct PlayList      *UndoPlayList;

struct Library       *CDGBase;
struct PlayerBase    *PlayerBase;

/* CDGstate definitions.  The values form a progression, with
 * each state implying the previous (except CDG_UNAVAILABLE is not
 * implied, of course).
 * CDG_UNAVAIABLE means CDGBegin() failed, and we're not to try
 *	any CDG...() calls.
 * CDG_AVAILABLE means CDGBegin() succeeded, but we have not turned
 *	on subcode processing (by calling CDGPlay()), or that we
 *	have turned it off (by calling CDGStop()).
 * CDG_SUBCODES means that CDGPlay() has been called, and therefore
 *	subcodes are being processed by cdg.library.  This state
 *	indicates that we don't have (or don't yet know that we have)
 *	CD+G packets in those subcodes.
 * CDG_HAVEGRAPHICS means we know that CD+G packets are coming off
 *	this disk.
 * CDG_SHOWGRAPHICS means that CD+G packets are coming in, and we
 *	have the CD+G screen frontmost.
 */

#define CDG_UNAVAILABLE		0
#define CDG_AVAILABLE		1
#define CDG_SUBCODES		2
#define CDG_HAVEGRAPHICS	3
#define CDG_SHOWGRAPHICS	4
int CDGstate;

UBYTE  CDGchannel = 1;

ULONG  currsec;

UBYTE  KeyPressed = PKSF_RELEASE;

/********************************* Tables **********************************/

UWORD IntroData[] =
{
    PLGAD_DISK,24, 0xffff,
    PLGAD_DISK,23, PLGAD_COUNTER, COUNTER_FLYING + 0, 0xffff,
    PLGAD_DISK,22, PLGAD_COUNTER, COUNTER_FLYING + 1, 0xffff,
    PLGAD_DISK,21, PLGAD_COUNTER, COUNTER_FLYING + 2, 0xffff,
    PLGAD_DISK,20, PLGAD_COUNTER, COUNTER_FLYING + 3, 0xffff,
    PLGAD_DISK,19, PLGAD_COUNTER, COUNTER_FLYING + 4, 0xffff,
    PLGAD_DISK,18, PLGAD_COUNTER, COUNTER_FLYING + 5, 0xffff,
    PLGAD_DISK,17, PLGAD_COUNTER, COUNTER_FLYING + 6, 0xffff,
    PLGAD_DISK,16, PLGAD_COUNTER, COUNTER_FLYING + 7, 0xffff,
    PLGAD_DISK,15, PLGAD_COUNTER, COUNTER_FLYING + 8, 0xffff,
    PLGAD_DISK,14, PLGAD_COUNTER, COUNTER_FLYING + 9, 0xffff,
    PLGAD_DISK,13, PLGAD_COUNTER, COUNTER_FLYING +10, 0xffff,
    PLGAD_DISK,12, PLGAD_COUNTER, COUNTER_FLYING +11, 0xffff,
    PLGAD_DISK,11, PLGAD_COUNTER, COUNTER_FLYING +12, 0xffff,
    PLGAD_DISK,10, PLGAD_COUNTER, COUNTER_FLYING +13, 0xffff,
    PLGAD_DISK, 9, PLGAD_COUNTER, COUNTER_FLYING +14, 0xffff,
    PLGAD_DISK, 8, PLGAD_COUNTER, COUNTER_FLYING +15, 0xffff,
    PLGAD_DISK, 7, PLGAD_COUNTER, COUNTER_FLYING +16, 0xffff,
    PLGAD_DISK, 6, PLGAD_COUNTER, COUNTER_FLYING +17, 0xffff,
    PLGAD_DISK, 5, PLGAD_COUNTER, COUNTER_FLYING +18, 0xffff,
    PLGAD_DISK, 4, PLGAD_COUNTER, COUNTER_FLYING +19, 0xffff,
    PLGAD_DISK, 3, PLGAD_COUNTER, COUNTER_FLYING +20, 0xffff,
    PLGAD_DISK, 2, PLGAD_COUNTER, COUNTER_FLYING +21, 0xffff,
    PLGAD_DISK, 1, PLGAD_COUNTER, 0,         0xffff,
    0xffff
};


UWORD ShutdownAnim[] =
{
    PLGAD_STOP,0,PLGAD_REV,0,PLGAD_PLAY,0,PLGAD_FF,0,
    PLGAD_SHUFF,0,PLGAD_CDG,0, PLGAD_COLON, 0, PLGAD_MIN, 100, PLGAD_SEC, 100, PLGAD_NEG, 0, 0xffff,
    PLGAD_20, 0, 0xffff,
    PLGAD_19, 0, 0xffff,
    PLGAD_18, 0, 0xffff,
    PLGAD_17, 0, 0xffff,
    PLGAD_16, 0, 0xffff,
    PLGAD_15, 0, 0xffff,
    PLGAD_14, 0, 0xffff,
    PLGAD_13, 0, 0xffff,
    PLGAD_12, 0, 0xffff,
    PLGAD_11, 0, 0xffff,
    PLGAD_10, 0, 0xffff,
    PLGAD_9 , 0, 0xffff,
    PLGAD_8 , 0, 0xffff,
    PLGAD_7 , 0, 0xffff,
    PLGAD_6 , 0, 0xffff,
    PLGAD_5 , 0, 0xffff,
    PLGAD_4 , 0, 0xffff,
    PLGAD_3 , 0, 0xffff,
    PLGAD_2 , 0, 0xffff,
    PLGAD_1 , 0, 0xffff,
    0xffff
};


UWORD DiskOutAnim[] =
{
    PLGAD_DISK, 1, 0xffff,
    PLGAD_DISK, 2, 0xffff,
    PLGAD_DISK, 3, 0xffff,
    PLGAD_DISK, 4, 0xffff,
    PLGAD_DISK, 5, 0xffff,
    PLGAD_DISK, 6, 0xffff,
    PLGAD_DISK, 7, 0xffff,
    PLGAD_DISK, 8, 0xffff,
    PLGAD_DISK, 9, 0xffff,
    PLGAD_DISK,10, 0xffff,
    PLGAD_DISK,11, 0xffff,
    PLGAD_DISK,12, 0xffff,
    PLGAD_DISK,13, 0xffff,
    PLGAD_DISK,14, 0xffff,
    PLGAD_DISK,15, 0xffff,
    PLGAD_DISK,16, PLGAD_COUNTER,COUNTER_FLYING +21, 0xffff,
    PLGAD_DISK,17, PLGAD_COUNTER,COUNTER_FLYING +20, 0xffff,
    PLGAD_DISK,18, PLGAD_COUNTER,COUNTER_FLYING +19, 0xffff,
    PLGAD_DISK,19, PLGAD_COUNTER,COUNTER_FLYING +18, 0xffff,
    PLGAD_DISK,20, PLGAD_COUNTER,COUNTER_FLYING +17, 0xffff,
    PLGAD_DISK,21, PLGAD_COUNTER,COUNTER_FLYING +16, 0xffff,
    PLGAD_DISK,22, PLGAD_COUNTER,COUNTER_FLYING +15, 0xffff,
    PLGAD_DISK,23, PLGAD_COUNTER,COUNTER_FLYING +14, 0xffff,
    PLGAD_DISK,24, PLGAD_COUNTER,COUNTER_FLYING +13, 0xffff,
    PLGAD_COUNTER,COUNTER_FLYING +12, 0xffff,
    PLGAD_COUNTER,COUNTER_FLYING +11, 0xffff,
    PLGAD_COUNTER,COUNTER_FLYING +10, 0xffff,
    PLGAD_COUNTER,COUNTER_FLYING + 9, 0xffff,
    PLGAD_COUNTER,COUNTER_FLYING + 8, 0xffff,
    PLGAD_COUNTER,COUNTER_FLYING + 7, 0xffff,
    PLGAD_COUNTER,COUNTER_FLYING + 6, 0xffff,
    PLGAD_COUNTER,COUNTER_FLYING + 5, 0xffff,
    PLGAD_COUNTER,COUNTER_FLYING + 4, 0xffff,
    PLGAD_COUNTER,COUNTER_FLYING + 3, 0xffff,
    PLGAD_COUNTER,COUNTER_FLYING + 2, 0xffff,
    PLGAD_COUNTER,COUNTER_FLYING + 1, 0xffff,
    PLGAD_COUNTER,COUNTER_FLYING + 0, 0xffff,
    0xffff
};


/****** playerprefs.library/DoPlayer ****************************************
*
*   NAME
*   DoPlayer -- Start CD audio control panel.
*
*   SYNOPSIS
*   DoPlayer();
*
*   VOID DoPlayer(void);
*
*   FUNCTION
*   Starts the CD audio control panel.  This function never returns.
*
*   RESULT
*   Like the fabled DETONATE instruction, the function returning
*   indicates an error.
*
*   EXAMPLE
*   DoPlayer();
*   error_exit();
*
*   NOTES
*
*   BUGS
*   Yes.
*
*   SEE ALSO
*
*****************************************************************************
*/


#ifdef INCLUDE_CLEANUP
LONG __asm
DoPlayer(register __a6 struct PlayerPrefsBase *ppb)
#else
void __asm
DoPlayer(register __a6 struct PlayerPrefsBase *ppb)
#endif
{
extern struct GadDir                PlayGADList[];
extern UBYTE  __far                 scrbuttons[];
extern struct BuildBg_Info __far    PlayerBg;
#define LVOScreenDepth -0x312

struct CDGPrefs     cdp;
int                 i;

    /* Initilize the variables */
    InitVarBase(ppb);
    SetChipRev( SETCHIPREV_BEST );

    /* Patching ScreenDepth() to fail prevents Amiga-M/N from having
     * any effect.  This is required to support CD+G.  We could
     * alternately apply an input-handler to catch those keys.
     * An input handler is easier to remove, but the SetFunction()
     * is easier to apply.  Since we exit through reboot, this
     * seems good enough.
     */
    SetFunction( IntuitionBase, LVOScreenDepth, NOOP );

    randomseed = (-vhposr & 0x7fff) ^ 0x1D6539A3;

    if (!StartPPScreen(PlayGADList, &PlayerBg, scrbuttons))
    {
	for ( i=PLGAD_FIRSTBUTTON; i<=PLGAD_LASTBUTTON; i++ )
	{
	    WorkIB->ib_GState[i] = CurrentIB->ib_GState[i] = (UWORD) ~0;
	}
	WorkIB->ib_GState[PLGAD_MIN] = CurrentIB->ib_GState[PLGAD_MIN] = 100;
	WorkIB->ib_GState[PLGAD_SEC] = CurrentIB->ib_GState[PLGAD_SEC] = 100;

	if (cdreply = (struct MsgPort *)CreateMsgPort())
	{
	    if (qcodereply = (struct MsgPort *)CreateMsgPort())
	    {
		if (cdio = (struct IOStdReq *)CreateStdIO(cdreply))
		{
		    if (cdqcode = (struct IOStdReq *)CreateStdIO(qcodereply))
		    {
			if (!OpenDevice("cd.device", 0, cdio, 0))
			{
			    CopyMem(cdio, cdqcode, sizeof(*cdio));

			    cdio->io_Message.mn_ReplyPort   = cdreply;
			    cdqcode->io_Message.mn_ReplyPort = qcodereply;
			    qoutstanding = toutstanding = qvalid = 0;

			    if (TOC = AllocMem(sizeof(union CDTOC) * NENTRIES, MEMF_PUBLIC))
			    {
				if (UndoPlayList = AllocMem(sizeof(struct PlayList), MEMF_PUBLIC))
				{
				    if (TimeReplyPort = CreateMsgPort())
				    {
					if (TimeReq = (struct timerequest *)
					    CreateExtIO(TimeReplyPort, sizeof(struct timerequest)))
					{
					    if (!OpenDevice("timer.device", UNIT_VBLANK, TimeReq, 0))
					    {
						if (PlayerBase = (struct PlayerBase *)OpenLibrary("player.library", NULL))
						{
						    struct TagItem add_createkeys[] =
						    {
							SCON_AddCreateKeys, 0,
							SCON_AddCreateKeys, 1,
							TAG_DONE,
						    };

						    if ( !SystemControlA( add_createkeys ) )
						    {

							if (CDG_Open(&cdp))
							{
							    if (UnpackSprites(&cdp))
							    {
								PlayerLoop(&cdp);
/* Don't bother cleaning up since we exit via reboot
 * The CLEAN() macro selectively includes or excludes the code,
 * based on the state of the INCLUDE_CLEANUP definition.
 */
#ifdef INCLUDE_CLEANUP
#define CLEAN(x)	x
#else
#define CLEAN(x)	;
#endif
								CLEAN( CDG_Close() );
							    }
							}
#ifdef INCLUDE_CLEANUP
							{
							    struct TagItem rem_createkeys[] =
							    {
								SCON_RemCreateKeys, 0,
								SCON_RemCreateKeys, 1,
								TAG_DONE,
							    };

							    SystemControlA( remove_createkeys );
							}
#endif
						    }

						    CLEAN( CloseLibrary(PlayerBase) );
						}

						CLEAN( CloseDevice(TimeReq) );
					    }

					    CLEAN( DeleteExtIO(TimeReq) );
					}

					CLEAN( DeleteMsgPort(TimeReplyPort) );
				    }
				    CLEAN( FreeMem(UndoPlayList, sizeof(struct PlayList)) );
				}

				CLEAN( FreeMem(TOC, sizeof(union CDTOC) * NENTRIES) );
			    }

			    CLEAN( AbortQCode() );

			    CLEAN( CloseDevice(cdio) );
			}

			CLEAN( DeleteStdIO(cdqcode) );
		    }

		    CLEAN( DeleteStdIO(cdio) );
		}

		CLEAN( DeleteMsgPort(qcodereply) );
	    }

	    CLEAN( DeleteMsgPort(cdreply) );
	}

	CLEAN( EndPPScreen() );
    }


#ifdef INCLUDE_CLEANUP
    return(20);
#else
    /* If we get here, it's because we failed to init, so let's die */
    ColdReboot();
#endif
}




/************************ Main Player Event Loop ***************************/

static void
PlayerLoop(struct CDGPrefs *cdp)
{

ULONG event;
UWORD *anim;

    CDGstate = CDG_UNAVAILABLE;
    if ( CDGBegin( cdp ) )
    {
	CDGstate = CDG_AVAILABLE;
    }

    anim = SetupPlayer();

    while(1)
    {
	if ( haveCD )
	{
	    while( event = GetIDCMPEvent() )
	    {
		DoKeyStroke( event );
	    }

	    /* -1 indicates a non-audio disk.  Let's not reboot if
	     * the user is changing audio disks.
	     */
	    if ( PlayerState.AudioDisk != 1 )
	    {
		/* If the intro animation was in progress, it's best
		 * to let it complete before we shut down.
		 */
		while ( anim )
		{
		    anim = DoAnimation( anim, 1 );
		}
		ShutDown();
		if ( PlayerState.AudioDisk == ~0 )
		{
		    DoReset();
		}
	    }
	    else
	    {
		/* We scroll the track table to put the currently
		 * playing track into view.  However, if the
		 * user is visiting the track table while the disk
		 * is playing, we don't want to fight over who gets
		 * to control the scrolling, the user's movements or
		 * the desire to put the current track into view.
		 * Here we decrement the counter, which, while non-zero,
		 * prevents auto-scrolling of the track table.
		 * If we're not playing or paused, nuke the variable.
		 */
		if ( noscrolltracks )
		{
		    noscrolltracks--;
		}
		if ( PlayerState.PlayState < PLS_PLAYING )
		{
		    noscrolltracks = 0;
		}

		UpdateGadgets();

		/* Since the state is really controlled by player.library,
		 * we do this little thing to detect whenever we transition
		 * to stopped state.  We count off a few VBlanks for
		 * safety, horrible though that is.
		 */
		if ( PlayerState.PlayState != PLS_STOPPED )
		{
		    StopCount = 5;
		}
		else if ( ( StopCount ) && ( --StopCount == 0 ) )
		{
		    if ( ShuffleMode )
		    {
			UnShuffleGrid();
			ShuffleMode = 0;
		    }
		    UnpreparePlayList();
		}

		/* If subcodes are on, then call CDGDraw() */
		if ( CDGstate >= CDG_SUBCODES )
		{
		    MasterIB->ib_GState[PLGAD_CDG] = CDGDraw( CDGF_GRAPHICS|CDGF_MIDI );
		}

		/* Did CD+G just turn itself on? */
		if ( ( MasterIB->ib_GState[PLGAD_CDG] & CDGF_GRAPHICS ) &&
		    ( CDGstate == CDG_SUBCODES ) )
		{
		    CDGClearScreen();

		    if ( PlayerState.PlayState == PLS_PAUSED )
		    {
			CDGPause();
		    }

		    AbleColorChanger( FALSE );
		    CDGFront();
		    CDGstate = CDG_SHOWGRAPHICS;

		    if ( ( PlayerState.PlayMode == PLM_FFWD ) || ( PlayerState.PlayMode == PLM_SKIPFWD ) )
		    {
			CDGFastForward();
		    }
		    else if ( ( PlayerState.PlayMode == PLM_FREV ) || ( PlayerState.PlayMode == PLM_SKIPREV ) )
		    {
			CDGRewind();
		    }
		}

		if ( CDGstate != CDG_SHOWGRAPHICS )
		{
		    UpdateDisplay();
		    if ( playertofront )
		    {
			playertofront = 0;
			RethinkDisplay();
			AbleColorChanger( TRUE );
		    }
		}
	    }
	}
	else	/* ( haveCD == FALSE ) */
	{
	    /* This function just strips IntuiMessages.  The function
	     * only strips events older than a few seconds.  The benefit
	     * is that we get a kind of "type-ahead", in that if
	     * the user selects play a second or two before the
	     * player.library tells us about the insertion of the
	     * disk, the play event is still queued for processing
	     * in the normal (audio disk is present) part of this
	     * event loop.
	     */
	    StripIDCMPEvents();
	    {
	    }
	    /* Waiting for a CD to be inserted into the drive */
	    GetPlayerState(&PlayerState);
	    if ( PlayerState.AudioDisk == 1 )
	    {
		anim = SetupPlayer();
	    }
	    else if ( PlayerState.AudioDisk == ~0 )
	    {
		DoReset();
	    }
	}

	if ( anim )
	{
	    anim = DoAnimation( anim, 0 );
	}
	else
	{
	    WaitTOF();
	}
    }
}





/*********************** Interpret KeyStroke Event *************************/

static void
DoKeyStroke(UWORD event)
{
    if ( CDGstate != CDG_SHOWGRAPHICS )
    {
	/* If it's an up or down stroke of the select button, then
	 * figure out what event current gadget would like to send
	 * instead.
	 */
	if ( ( event == KEY_SELECT ) || ( event == ( KEY_SELECT | IECODE_UP_PREFIX ) ) ||
	    ( event == GC0_SELECT ) || ( event == ( GC0_SELECT | IECODE_UP_PREFIX ) ) ||
	    ( event == GC1_SELECT ) || ( event == ( GC1_SELECT | IECODE_UP_PREFIX ) ) )
	{
	    event = GetBoxKey( MasterIB, event );
	}

	if (!SubmitKeys(event)) switch (event)
	{
	    case KEY_UP:
	    case GC0_UP:
	    case GC1_UP:
		DoBoxMove(KEY_UP);
		break;

	    case KEY_DOWN:
	    case GC0_DOWN:
	    case GC1_DOWN:
		DoBoxMove(KEY_DOWN);
		break;

	    case KEY_LEFT:
	    case GC0_LEFT:
	    case GC1_LEFT:
		DoBoxMove(KEY_LEFT);
		break;

	    case KEY_RIGHT:
	    case GC0_RIGHT:
	    case GC1_RIGHT:
		DoBoxMove(KEY_RIGHT);
		break;

	    case KEY_GBUTTON:
		togglegnum();
		break;

	    case KEY_SHUFFLE:
	    case GC0_SHUFFLE:
	    case GC1_SHUFFLE:
		/* We want shuffle to scramble the tracks and
		 * initiate a fresh play.  The ShuffleGrid()
		 * function does call UpdatePlayTime(), which
		 * sends a stop key, so we need not worry here.
		 */
		ShuffleGrid( ShuffleMode );
		ShuffleMode = 1;
		if ( PlayerState.PlayState == PLS_STOPPED )
		{
		    /* PKS_EJECT actually is absolute play,
		     * i.e. never toggle to pause.
		     */
		    PreparePlayList();
		    SubmitKeyStroke(KeyPressed = PKS_EJECT|PKSF_PRESS);
		    SubmitKeyStroke(KeyPressed = PKS_EJECT|PKSF_RELEASE);
		}
		break;

	    /* Change time display mode. */
	    case KEY_TIMEMODE:
		{
		    static const UBYTE NextTime[ 4 ] =
		    {
			1,	/* track elapsed -> track remaining */
			3,	/* track remaining -> disk remaining */
			0,	/* disk elapsed -> track elapsed */
			2,	/* disk remaining -> disk elapsed */
		    };

		    GetOptions(&OptionsSet);
		    OptionsSet.TimeMode = NextTime[ OptionsSet.TimeMode ];
		    OptionsSet.Loop = OptionsSet.Intro = OptionsSet.Subcode = -1;
		    SetOptions(&OptionsSet);
		}
		break;

	    /* Playlist loop */
	    case KEY_REPEAT:
	    case GC0_REPEAT:
	    case GC1_REPEAT:
		GetOptions(&OptionsSet);
		OptionsSet.Loop = !OptionsSet.Loop;
		OptionsSet.TimeMode = OptionsSet.Intro = OptionsSet.Subcode = -1;
		SetOptions(&OptionsSet);
		break;

	    /* Intro mode */
	    case KEY_INTRO:
		GetOptions(&OptionsSet);
		OptionsSet.Intro = !OptionsSet.Intro;
		IntroFlashTicks = ~0;	/* Signifies initial */
		OptionsSet.Loop = OptionsSet.TimeMode = OptionsSet.Subcode = -1;
		SetOptions(&OptionsSet);

		IntroLastTrack  = 0;
		IntroCount = ((UWORD) ~0);

		if (OptionsSet.Intro && PlayerState.PlayState == PLS_STOPPED)
		{
		    /* PKS_EJECT actually is absolute play,
		     * i.e. never toggle to pause
		     */
		    PreparePlayList();
		    SubmitKeyStroke(KeyPressed = PKS_EJECT|PKSF_PRESS);
		    SubmitKeyStroke(KeyPressed = PKS_EJECT|PKSF_RELEASE);
		}

		break;

	    /* Keystroke echoed by player.library */
	    case ECHOKEY_PREVTRAK:
		MasterIB->ib_GState[PLGAD_REV] = 1;
		if ( CDGstate >= CDG_HAVEGRAPHICS )
		{
		    CDGClearScreen();
		}
		break;

	    /* Keystroke echoed by player.library */
	    case ECHOKEY_NEXTTRAK:
		MasterIB->ib_GState[PLGAD_FF] = 1;
		if ( CDGstate >= CDG_HAVEGRAPHICS )
		{
		    CDGClearScreen();
		}
		break;

	    /* Keystroke echoed by player.library */
	    case (ECHOKEY_PREVTRAK | IECODE_UP_PREFIX):
		MasterIB->ib_GState[PLGAD_REV] = 0;
		break;

	    /* Keystroke echoed by player.library */
	    case (ECHOKEY_NEXTTRAK | IECODE_UP_PREFIX):
		MasterIB->ib_GState[PLGAD_FF] = 0;
		break;

	    case KEY_GOCDG:
		if ( CDGstate == CDG_HAVEGRAPHICS )
		{
		    CDGstate = CDG_SHOWGRAPHICS;
		    AbleColorChanger( FALSE );
		    CDGFront();
		    if (PlayerState.PlayState == PLS_PAUSED)
		    {
			CDGPause();
		    }

		    if ( ( PlayerState.PlayMode == PLM_FFWD ) || ( PlayerState.PlayMode == PLM_SKIPFWD ) )
		    {
			CDGFastForward();
		    }
		    else if ( ( PlayerState.PlayMode == PLM_FREV ) || ( PlayerState.PlayMode == PLM_SKIPREV ) )
		    {
			CDGRewind();
		    }
		}
		break;

#if DEBUGGING
	    case RAWKEY_ESC:
		{
		    int i;
		    static char *modes[] =
		    {
			"Normal",
			"FFwd",
			"FRev",
			"SkipFwd",
			"SkipRev",
		    };
		    static char *states[] =
		    {
			"Stopped",
			"Selected",
			"NumEntry",
			"Playing",
			"Paused",
		    };

		    kprintf("Debugging:\n");
		    kprintf("State: %s, Mode: %s\n", states[ PlayerState.PlayState ], modes[ PlayerState.PlayMode ] );
		    kprintf("Disk: %ld, Tracks: %ld, ListIndex: %ld, Track: %ld, Time: %ld:%ld\n",
			PlayerState.AudioDisk, PlayerState.Tracks, PlayerState.ListIndex,
			PlayerState.Track, PlayerState.Minute, PlayerState.Second );
		    kprintf("PlayList EntryCount %ld", PlayList->EntryCount );
		    for ( i = 0; i < PlayList->EntryCount; i++ )
		    {
			if ( ( i % 10 ) == 0 )
			{
			     kprintf("\n    ");
			}
			kprintf("%s%2ld%s ",
			PlayList->Entry[ i ] & PLEF_ENABLE ? " " : "(",
			PlayList->Entry[ i ] & PLEF_TRACK,
			PlayList->Entry[ i ] & PLEF_ENABLE ? " " : ")" );
		    }
		    kprintf("\nShuffleMode: %ld\n\n", ShuffleMode);

		    {
			struct MinNode *node;
			struct PlayerLibrary
			{

			    struct Library          PlayerLib;

			    APTR                    SegList;

			    struct Task            *PlayerTask;
			    struct MsgPort         *TaskMsgPort;
			    struct MsgPort         *TaskReplyPort;

			    struct PlayerOptions    PlayerOptions;
			    struct PlayerState      PlayerState;
			    struct PlayList         PlayList;

			    struct SignalSemaphore  PlayListSemaphore;
			    struct SignalSemaphore  PlayStateSemaphore;
			} *pl = (struct PlayerLibrary *) PlayerBase;
			kprintf("PlayerTask: %lx\n", pl->PlayerTask);
			kprintf("PlayListSem: \"%s\"\n    Owner %lx, NestCount %ld, QueueCount %ld\n",
				pl->PlayListSemaphore.ss_Link.ln_Name,
				pl->PlayListSemaphore.ss_Owner, pl->PlayListSemaphore.ss_NestCount,
				pl->PlayListSemaphore.ss_QueueCount );
			for ( node = pl->PlayListSemaphore.ss_WaitQueue.mlh_Head;
			    node->mln_Succ; node = node->mln_Succ )
			{
			    kprintf("%lx ", ((struct SemaphoreRequest *)node)->sr_Waiter );
			}
			kprintf("PlayStateSem: \"%s\"\n    Owner %lx, NestCount %ld, QueueCount %ld\n",
				pl->PlayStateSemaphore.ss_Link.ln_Name,
				pl->PlayStateSemaphore.ss_Owner, pl->PlayStateSemaphore.ss_NestCount,
				pl->PlayStateSemaphore.ss_QueueCount );
			for ( node = pl->PlayStateSemaphore.ss_WaitQueue.mlh_Head;
			    node->mln_Succ; node = node->mln_Succ )
			{
			    kprintf("%lx ", ((struct SemaphoreRequest *)node)->sr_Waiter );
			}
		    }
		}
		break;
#endif /* DEBUGGING */

	    /* Numeric keypad */
	    case KEY_ADVANCE:
		if ( ( MasterIB->ib_BoxNo >= PLGAD_1 ) &&
		    ( MasterIB->ib_BoxNo <= PLGAD_20 ) )
		{
		    DoBoxMove(KEY_RIGHT);
		}

		break;

	    default:
		break;
	}
    }
    else /* ( CDGstate == CDG_SHOWGRAPHICS ) */
    {
	if (!SubmitKeys(event)) switch (event)
	{
	    /* Playlist loop */
	    case KEY_REPEAT:
	    case GC0_REPEAT:
	    case GC1_REPEAT:
		GetOptions(&OptionsSet);
		OptionsSet.Loop = !OptionsSet.Loop;
		OptionsSet.TimeMode = OptionsSet.Intro = OptionsSet.Subcode = -1;
		SetOptions(&OptionsSet);
		break;

	    /* Intro mode */
	    case KEY_INTRO:
		GetOptions(&OptionsSet);
		OptionsSet.Intro = !OptionsSet.Intro;
		OptionsSet.Loop = OptionsSet.TimeMode = OptionsSet.Subcode = -1;
		SetOptions(&OptionsSet);

		IntroLastTrack  = 0;
		IntroCount = ((UWORD) ~0);
		break;

	    case KEY_SHUFFLE:
	    case GC0_SHUFFLE:
	    case GC1_SHUFFLE:
		/* We want shuffle to scramble the tracks and
		 * initiate a fresh play.  The ShuffleGrid()
		 * function does call UpdatePlayTime(), which
		 * sends a stop key, so we need not worry here.
		 */
		ShuffleGrid( ShuffleMode );
		ShuffleMode = 1;
		if ( PlayerState.PlayState == PLS_STOPPED )
		{
		    /* PKS_EJECT actually is absolute play,
		     * i.e. never toggle to pause.
		     */
		    PreparePlayList();
		    SubmitKeyStroke(KeyPressed = PKS_EJECT|PKSF_PRESS);
		    SubmitKeyStroke(KeyPressed = PKS_EJECT|PKSF_RELEASE);
		}
		break;

	    /* Keystroke echoed by player.library */
	    case ECHOKEY_PREVTRAK:
		CDGRewind();
		break;

	    /* Keystroke echoed by player.library */
	    case ECHOKEY_NEXTTRAK:
		CDGFastForward();
		break;

	    /* Keystroke echoed by player.library */
	    case (ECHOKEY_PREVTRAK | IECODE_UP_PREFIX):
		if (PlayerState.PlayState == PLS_PAUSED)
		{
		    CDGPause();
		}
		else
		{
		    CDGPrevTrack();
		}
		break;

	    /* Keystroke echoed by player.library */
	    case (ECHOKEY_NEXTTRAK | IECODE_UP_PREFIX):
		if (PlayerState.PlayState == PLS_PAUSED)
		{
		    CDGPause();
		}
		else
		{
		    CDGNextTrack();
		}
		break;

	    case KEY_SELECT:
	    case KEY_GOCDG:
	    case GC0_SELECT:
	    case GC1_SELECT:
		CDGstate = CDG_HAVEGRAPHICS;
		MasterIB->ib_BoxNo = PLGAD_GOCDG;
		/* Since IntroFlashTicks doesn't count down when CD+G
		 * graphics are being displayed, we don't want to set
		 * it then either.  If we did, the flash would be "saved up"
		 * until the user exited the CD+G screen.
		 */
		IntroLastTrack = PlayerState.Track;
		IntroFlashTicks = 0;
		CDGBack();
		SetTrackCounter(1);
		playertofront = 1;
		break;

	    case KEY_UP:
	    case GC0_UP:
	    case GC1_UP:
		if ( CDGchannel++ == 15 )
		{
		    CDGchannel = 1;
		}
		CDGChannel( CDGchannel );
		break;

	    case KEY_DOWN:
	    case GC0_DOWN:
	    case GC1_DOWN:
		if ( --CDGchannel == 0 )
		{
		    CDGchannel = 15;
		}
		CDGChannel( CDGchannel );
		break;

	    case ( KEY_UP | IECODE_UP_PREFIX ):
	    case ( GC0_UP | IECODE_UP_PREFIX ):
	    case ( GC1_UP | IECODE_UP_PREFIX ):
	    case ( KEY_DOWN | IECODE_UP_PREFIX ):
	    case ( GC0_DOWN | IECODE_UP_PREFIX ):
	    case ( GC1_DOWN | IECODE_UP_PREFIX ):
		/* For some reason, if the last keystroke sent was a downstroke
		 * we send the corresponding upstroke.  This was here before,
		 * so I'm loath to delete it even though I don't see why
		 * it's here...
		 */
		if ((KeyPressed & PKSF_STROKEDIR) == PKSF_PRESS)
		    SubmitKeyStroke(KeyPressed = ((KeyPressed & PKSF_KEY) | PKSF_RELEASE));
		break;
	}
    }
}



/* Returns non-zero to indicate the event is consumed, zero if the
 * event is not consumed.  Transport control keys are consumed, since
 * player.library will echo them back to us.  We act on the echoes,
 * not the actual keystrokes.
 */
static int
SubmitKeys(UWORD event)
{
    switch(event)
    {
	case KEY_PLAYPAUSE:
	case GC0_PLAYPAUSE:
	case GC1_PLAYPAUSE:
	    PreparePlayList();
	    SubmitKeyStroke(KeyPressed = PKS_PLAYPAUSE|PKSF_PRESS);
	    SubmitKeyStroke(KeyPressed = PKS_PLAYPAUSE|PKSF_RELEASE);
	    return(1);

	case KEY_STOP:
	case GC0_STOP:
	case GC1_STOP:
	    if ( ShuffleMode )
	    {
		UnShuffleGrid();
		ShuffleMode = 0;
	    }

	    /* If the stop key is pressed while we're well and
	     * truly stopped, then reset the play list, to
	     * dim up any selected tracks.
	     * StopCount hits zero after five consective vblanks
	     * of being stopped.  That should protect us against
	     * brief transitions to stop mode which can occur
	     * when shuffling while playing, for example.
	     * (Ok, so I know it's ugly...)
	     */
	    if ( ( PlayerState.PlayState == PLS_STOPPED ) && ( !StopCount ) )
	    {
		MasterIB->ib_GState[PLGAD_SHUFF] = 0;
		SetPlayList(0);
		UpdateGadgets();
	    }

	    SubmitKeyStroke(KeyPressed = PKS_STOP|PKSF_PRESS);
	    SubmitKeyStroke(KeyPressed = PKS_STOP|PKSF_RELEASE);
	    return(1);

	case KEY_BACK:
	case GC0_BACK:
	case GC1_BACK:
	    /* Since now by default all tracks are off, but that's taken to mean
	     * OK to play all, we must call PreparePlayList() here if stopped.
	     * That function lights all tracks if all tracks are dim.
	     */
	    if ( PlayerState.PlayState == PLS_STOPPED )
	    {
		PreparePlayList();
	    }
	    MasterIB->ib_GState[PLGAD_REV]  = 1;
	    SubmitKeyStroke(KeyPressed = PKS_REVERSE|PKSF_PRESS);
 	    return(1);

	case KEY_FWD:
	case GC0_FWD:
	case GC1_FWD:
	    /* Since now by default all tracks are off, but that's taken to mean
	     * OK to play all, we must call PreparePlayList() here if stopped.
	     * That function lights all tracks if all tracks are dim.
	     */
	    if ( PlayerState.PlayState == PLS_STOPPED )
	    {
		PreparePlayList();
	    }
	    MasterIB->ib_GState[PLGAD_FF]   = 1;
	    SubmitKeyStroke(KeyPressed = PKS_FORWARD|PKSF_PRESS);
	    return(1);

	case (KEY_BACK|IECODE_UP_PREFIX):
	case (GC0_BACK|IECODE_UP_PREFIX):
	case (GC1_BACK|IECODE_UP_PREFIX):
	    MasterIB->ib_GState[PLGAD_REV]  = 0;
	    SubmitKeyStroke(KeyPressed = PKS_REVERSE|PKSF_RELEASE);
	    AbortQCode();
	    return(1);

	case (KEY_FWD|IECODE_UP_PREFIX):
	case (GC0_FWD|IECODE_UP_PREFIX):
	case (GC1_FWD|IECODE_UP_PREFIX):
	    MasterIB->ib_GState[PLGAD_FF]   = 0;
	    SubmitKeyStroke(KeyPressed = PKS_FORWARD|PKSF_RELEASE);
	    AbortQCode();
	    return(1);

	default:
	    return(0);
    }
}


/************** Update Screen Gadgets Based on Current State ***************/

void
UpdateGadgets()
{
    GetPlayerState(&PlayerState);

    if (PlayerState.PlayState >= PLS_PLAYING)
    {
	if (toutstanding)
	{
	    if (CheckIO(TimeReq))
	    {
		if (!qoutstanding)
		{
		    SendIO(cdqcode);
		    qoutstanding = 1;
		}

		if (CheckIO(cdqcode))
		{
		    WaitIO(cdqcode);
		    qoutstanding = 0;

		    if (!cdqcode->io_Error)
		    {
			copyqcode = qcode;
			qvalid = 1;
		    }

		    WaitIO(TimeReq);

#if DEBUGGING
		    if (TimeReq->tr_node.io_Error) kprintf("Error %d\n", TimeReq->tr_node.io_Error);
#endif

		    TimeReq->tr_time.tv_secs    = 1;
		    TimeReq->tr_time.tv_micro   = 0;
		    SendIO(TimeReq);
		}
	    }
	}
    }
    if ( ( CDGstate == CDG_SHOWGRAPHICS ) &&
	( LastPlayMode == PlayerState.PlayMode ) &&
	( LastPlayState == PlayerState.PlayState ) )
    {
	return;
    }

    if ( ( CDGstate ) && ( LastPlayState != PlayerState.PlayState ) )
    {
	switch ( PlayerState.PlayState )
	{
	    case PLS_STOPPED:
		if ( CDGstate == CDG_SHOWGRAPHICS )
		{
		    playertofront = 1;
		}
		CDGStop();
		CDGBack();
		CDGstate = CDG_AVAILABLE;
		MasterIB->ib_GState[PLGAD_REV] = MasterIB->ib_GState[PLGAD_FF]  = 0;
		/* Since IntroFlashTicks doesn't count down when CD+G
		 * graphics are being displayed, we don't want to set
		 * it then either.  If we did, the flash would be "saved up"
		 * until the user exited the CD+G screen.
		 */
		IntroLastTrack = PlayerState.Track;
		IntroFlashTicks = 0;
		break;

	    case PLS_PLAYING:
		/* Show the play glyph if we're unpausing */
		if ( LastPlayState == PLS_PAUSED )
		{
		    CDGPlay( 1 );
		}
		else
		{
		    CDGPlay( 0 );
		    CDGstate = CDG_SUBCODES;
		}
		break;

	    case PLS_PAUSED:
		CDGPause();
		break;
	}
    }

    if ( ( CDGstate == CDG_HAVEGRAPHICS ) && ( PlayerState.PlayState >= PLS_PLAYING ) )
    {
	/* We want the GOCDG gadget enabled, so slide the cover off
	 * until it's fully off
	 */
	if ( MasterIB->ib_GState[PLGAD_GOCDG] < GOCDGSTATES )
	{
	    MasterIB->ib_GState[PLGAD_GOCDG]++;
	}
    }
    else
    {
	/* We want the GOCDG gadget disabled, so slide the cover up
	 * until it's fully up.  If the highlight is over the GOCDG
	 * gadget, kick it off as soon as it's fully covered.
	 */
	if ( MasterIB->ib_GState[PLGAD_GOCDG] > 0 )
	{
	    MasterIB->ib_GState[PLGAD_GOCDG]--;
	    if ( ( MasterIB->ib_GState[PLGAD_GOCDG] == 0 ) &&
		( MasterIB->ib_BoxNo == PLGAD_GOCDG ) )
	    {
		MasterIB->ib_BoxNo = PLGAD_TIME;
	    }
	}
    }

    switch ( PlayerState.PlayState )
    {
	case PLS_STOPPED:

	    AbortQCode();
	    break;

	case PLS_PLAYING:
	case PLS_PAUSED:
	    if (!toutstanding)
	    {
		TimeReq->tr_time.tv_secs    = 1;
		TimeReq->tr_time.tv_micro   = 0;
		SendIO(TimeReq);
		toutstanding = 1;
	    }

	    break;
    }

    if ( PlayerState.PlayState == PLS_STOPPED )
    {
	MasterIB->ib_GState[PLGAD_TTRACK] = 0;
    }
    else
    {
	MasterIB->ib_GState[PLGAD_TTRACK] = PlayerState.Track;

	if (PlayerState.Track != IntroLastTrack)
	{
	    if (PlayerState.PlayMode == PLM_SKIPREV) --IntroCount;
	    else                                     IntroCount++;

	    IntroLastTrack = PlayerState.Track;

	    /* The Intro gadget has two primary states: off and on.
	     * When it's on, the whole thing is yellow.  But whenever
	     * we change tracks, we want to wink one of the arrows
	     * for FLASHLENGTH vblanks.  IntroFlashTicks counts that down.
	     * Note that we get here the first time Intro is
	     * turned on, and we don't want to wink the gadget
	     * in that case.  We handle that by detecting
	     * IntroFlashTicks of ~0.
	     */
	    if ( IntroFlashTicks == ~0 )
	    {
		IntroFlashTicks = 0;
	    }
	    else
	    {
		IntroFlashTicks = FLASHLENGTH;
	    }
	}
    }

    if ( PlayerState.AudioDisk == 1 )
    {
	MasterIB->ib_GState[PLGAD_MIN] = PlayerState.Minute;
	MasterIB->ib_GState[PLGAD_SEC] = PlayerState.Second;
	MasterIB->ib_GState[PLGAD_NEG] = PlayerState.Minus;

	if ( ( NumSelectedTracks == 0 ) && ( PlayerState.PlayState < PLS_PLAYING ) )
	{
	    MasterIB->ib_GState[PLGAD_MIN] = TotalMinutes;
	    MasterIB->ib_GState[PLGAD_SEC] = TotalSeconds;
	    MasterIB->ib_GState[PLGAD_NEG] = 0;
	}
	MasterIB->ib_GState[PLGAD_COLON] = 1;
    }
    else
    {
	MasterIB->ib_GState[PLGAD_TTRACK] = 0;
    }

    /* Never set PLGAD_TTRACK if the head gadget is moving */
    if ( MasterIB->ib_GState[PLGAD_COUNTER] & COUNTER_FLYING )
    {
	MasterIB->ib_GState[PLGAD_TTRACK] = 0;
    }

    if ( PlayerState.PlayMode == PLM_NORMAL )
    {
	MasterIB->ib_GState[PLGAD_STOP]  =  (PlayerState.PlayState == PLS_STOPPED)
				    || (PlayerState.PlayState == PLS_SELECTED)
				    || (PlayerState.PlayState == PLS_NUMENTRY);

	if ( PlayerState.PlayState == PLS_PAUSED )
	{
	    MasterIB->ib_GState[PLGAD_PLAY] = 1;
	}
	else if ( PlayerState.PlayState == PLS_PLAYING )
	{
	    MasterIB->ib_GState[PLGAD_PLAY] = 2;
	}
	else
	{
	    MasterIB->ib_GState[PLGAD_PLAY] = 0;
	}
    }

    GetOptions(&PlayerOptions);

    MasterIB->ib_GState[PLGAD_TIME]   = PlayerOptions.TimeMode;

    MasterIB->ib_GState[PLGAD_REPEAT] = PlayerOptions.Loop;
    if ( PlayerOptions.Intro )
    {
	if ( IntroFlashTicks )
	{
	    MasterIB->ib_GState[PLGAD_INTRO] = ( 0x03 & IntroCount ) + 1;
	    IntroFlashTicks--;
	}
	else
	{
	    MasterIB->ib_GState[PLGAD_INTRO] = 5;
	}
    }
    else
    {
	MasterIB->ib_GState[PLGAD_INTRO] = 0;
    }

    if (PlayerState.LastModify) MasterIB->ib_GState[PLGAD_SHUFF] = 0;

    SetTrackCounter(0);

    if (PlayerState.AudioDisk == 1) DisplayGrid();

    LastPlayMode  = PlayerState.PlayMode;
    LastPlayState = PlayerState.PlayState;
}


/**************** Initialize Structures/Variables/Gadgets ******************/

static UWORD *
SetupPlayer(void)
{
extern UWORD     IntroData[];

    static int firstinit = 1;

    OptionsSet.Loop     = -1;
    OptionsSet.Intro    = -1;
    OptionsSet.TimeMode = -1;
    OptionsSet.Subcode  = 1;
    SetOptions(&OptionsSet);

    MasterIB->ib_GState[PLGAD_STOP]   = 1;
    MasterIB->ib_GState[PLGAD_TIME]   = 0;
    MasterIB->ib_GState[PLGAD_REPEAT] = 0;
    MasterIB->ib_GState[PLGAD_SHUFF]  = 0;
    MasterIB->ib_GState[PLGAD_CDG] = 0;
    MasterIB->ib_GState[PLGAD_GOCDG] = 0;

    /* The disk head needs to be flying initially, because we key off
     * that to suppress changing PLGAD_TTRACK to anything other
     * than blank.
     * PLGAD_MIN, PLGAD_SEC, and PLGAD_NEG all set to blank.
     */
    MasterIB->ib_GState[PLGAD_COUNTER] = COUNTER_FLYING + 0;
    MasterIB->ib_GState[PLGAD_MIN] = 100;
    MasterIB->ib_GState[PLGAD_SEC] = 100;
    MasterIB->ib_GState[PLGAD_NEG] = 0;
    MasterIB->ib_GState[PLGAD_COLON] = 0;

    ShuffleMode = 0;
    StopCount = 0;

    /* Turn on the speakers; full volume, right now(sorta). */
    cdio->io_Command = CD_ATTENUATE;
    cdio->io_Offset  = 0x7FFF;
    cdio->io_Length  = 0;
    DoIO(cdio);

    /* Initialize the qcode packet. */
    cdqcode->io_Command  = CD_QCODELSN;
    cdqcode->io_Data     = (APTR)&qcode;

    /* Create timer request */
    TimeReq->tr_node.io_Command = TR_ADDREQUEST;
    TimeReq->tr_time.tv_secs    = 1;
    TimeReq->tr_time.tv_micro   = 0;

    GetPlayerState(&PlayerState);

    if (PlayerState.PlayMode == PLM_NORMAL)
    {
	MasterIB->ib_GState[PLGAD_STOP]  =  (PlayerState.PlayState == PLS_STOPPED)
				    || (PlayerState.PlayState == PLS_SELECTED);

	if ( PlayerState.PlayState == PLS_PAUSED )
	{
	    MasterIB->ib_GState[PLGAD_PLAY] = 1;
	}
	else if ( PlayerState.PlayState == PLS_PLAYING )
	{
	    MasterIB->ib_GState[PLGAD_PLAY] = 2;
	}
	else
	{
	    MasterIB->ib_GState[PLGAD_PLAY] = 0;
	}
    }

    GetOptions(&PlayerOptions);

    MasterIB->ib_GState[PLGAD_TIME]   = PlayerOptions.TimeMode;
    MasterIB->ib_GState[PLGAD_REPEAT] = PlayerOptions.Loop;
    MasterIB->ib_GState[PLGAD_INTRO]  = PlayerOptions.Intro * ((0x03 & IntroCount) + 1);

    LastPlayMode  = PlayerState.PlayMode;
    LastPlayState = PlayerState.PlayState;

    UpdateDisplay();

    if ( firstinit )
    {
	FadeIn();
    }

    haveCD = 1;
    AbleColorChanger( TRUE );
    MasterIB->ib_BoxNo = PLGAD_PLAY;

    GetOptions(&PlayerOptions);

    /* Gimme summary. */
    cdio->io_Command  = CD_TOCLSN;
    cdio->io_Offset   = 0;
    cdio->io_Length   = 100;
    cdio->io_Data     = (APTR)TOC;
    DoIO(cdio);

    if (!cdio->io_Error)
    {
	InitTrackCounter();
	PlayList = ObtainPlayList();
	SetPlayList(1);
	GetPlayerState(&PlayerState);
	TotalMinutes = PlayerState.Minute;
	TotalSeconds = PlayerState.Second;
	SetPlayList(0);
    }
    else
    {
	DoReset();
    }

    currsec = TOC[PlayerState.Track].Entry.Position.LSN;
    firstinit = 0;

    return( IntroData );
}


/***************************** Shutdown Routine ****************************/

static void
ShutDown()
{
UWORD *anim;

    TotalMinutes = 100;
    TotalSeconds = 100;
    SetTrackCounter( 0 );
    AbleColorChanger( FALSE );
    haveCD = 0;
    if ( CDGstate )
    {
	CDGchannel = 1;
	CDGChannel( CDGchannel );
	CDGDiskRemoved();
	CDGstate = CDG_AVAILABLE;
    }
    anim = ShutdownAnim;
    MasterIB->ib_BoxNo = -1;

    /* Shutdown anim turns all gadgets to off.  Do that simultaneously
     * to bringing the disk-head back to the outside.
     */
    while ( ( MasterIB->ib_GState[PLGAD_COUNTER] ) ||
	( MasterIB->ib_GState[PLGAD_GOCDG] ) || ( anim ) )
    {
	if ( MasterIB->ib_GState[PLGAD_COUNTER] )
	{
	    MasterIB->ib_GState[PLGAD_COUNTER]--;
	}

	if ( MasterIB->ib_GState[PLGAD_GOCDG] )
	{
	    MasterIB->ib_GState[PLGAD_GOCDG]--;
	}

	if (anim)
	{
	    anim = DoAnimation( anim, 1 );
	}
	else
	{
	    UpdateDisplay();
	}
    }

    /* The DiskOutAnim makes the disk head fly up and the disk itself
     * fly left
     */
    anim = DiskOutAnim;
    while(anim = DoAnimation(anim,1));
}



/****************************** Fade and Reset *****************************/

static void
DoReset()
{
    FadeOut();
    ColdReboot();
}



/*********************** Abort Q-Code related requests *********************/

static void
AbortQCode(void)
{
    if (qoutstanding)
    {
	AbortIO(cdqcode);
	WaitIO(cdqcode);
	qoutstanding = 0;
    }

    if (toutstanding)
    {
	AbortIO(TimeReq);
	WaitIO(TimeReq);
	toutstanding = 0;
    }

    qvalid = 0;
}

/********************** Reset Default PlayList ***********************/

/* Create a default play-list, all enabled or all disabled */
static void
SetPlayList( ULONG enable )
{
WORD           i;

    /* Set playlist back to zero */
    UpdatePlayTime(); while (!ModifyPlayList(1));

    NumSelectedTracks = 0;
    PlayList->EntryCount = 0;

    for ( i = 0; i < TOC[0].Summary.LastTrack; i++ )
    {
	PlayList->Entry[i] = (UBYTE)(i+1);
	if ( enable )
	{
	    PlayList->Entry[i] |= PLEF_ENABLE;
	}
    }
    PlayList->EntryCount = TOC[0].Summary.LastTrack;
    if ( enable )
    {
	NumSelectedTracks = PlayList->EntryCount;
    }


    ModifyPlayList(0);
}

/* Enable all tracks in the PlayList if none of them are lit */
static void
PreparePlayList( void )
{
    WORD i;
    if ( NumSelectedTracks == 0 )
    {
	for ( i = 0; i < PlayList->EntryCount; i++ )
	{
	    PlayList->Entry[i] |= PLEF_ENABLE;
	}
	NumSelectedTracks = PlayList->EntryCount;
    }
}

/* Disable all tracks in the PlayList if all of them are lit */
static void
UnpreparePlayList( void )
{
    WORD i;
    if ( NumSelectedTracks == PlayList->EntryCount )
    {
	for ( i = 0; i < PlayList->EntryCount; i++ )
	{
	    PlayList->Entry[i] &= ~PLEF_ENABLE;
	}
	NumSelectedTracks = 0;
    }
}
