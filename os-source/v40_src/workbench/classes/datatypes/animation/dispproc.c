/* dispproc.c
 * Written by David N. Junod
 *
 */

/*****************************************************************************/

#include "classbase.h"
#include "classdata.h"

#include <hardware/custom.h>
#include <hardware/dmabits.h>
#include <hardware/intbits.h>

/*****************************************************************************/

#define	DB(x)	;
#define	DC(x)	;

/*****************************************************************************/

void ASM XLateBitMap (REG (a0) struct BitMap * sbm, REG (a1) struct BitMap * dbm, REG (a2) char *table1, REG (a3) char *table2, REG (d0) ULONG width);
void ASM CopyBitMap  (REG (a6) struct ClassBase *cb, REG (a2) struct BitMap *bm1, REG (a3) struct BitMap *bm2, REG (d2) ULONG width);

/*****************************************************************************/

struct FrameNode *DiscardAllBut (struct ClassBase *cb, struct localData *lod, ULONG frame, ULONG mode)
{
    struct FrameNode *rfn = NULL;
    struct FrameNode *fn;

    /* Stop the load process */
    Signal ((struct Task *) lod->lod_LoadProc, SIGBREAKF_CTRL_F);

    ObtainSemaphore (&lod->lod_DLock);
    ObtainSemaphore (&lod->lod_FLock);

    /* Pull from the In list */
    while (fn = (struct FrameNode *) RemHead (&lod->lod_FrameList))
    {
	/* Subtract from the in list */
	lod->lod_InList--;

	/* See if we have a match */
	if ((fn->fn_Frame.alf_Frame == frame) && (rfn == NULL))
	    rfn = fn;
	else
	{
	    lod->lod_OutList++;
	    AddTail (&lod->lod_DiscardList, (struct Node *) fn);
	}
    }

    ReleaseSemaphore (&lod->lod_FLock);
    ReleaseSemaphore (&lod->lod_DLock);

    return (rfn);
}

/*****************************************************************************/

void DiscardAllFrames (struct ClassBase *cb, struct localData *lod)
{
    struct FrameNode *fn;

    /* Stop the load process */
    Signal ((struct Task *) lod->lod_LoadProc, SIGBREAKF_CTRL_F);

    /* Give it a chance to actually stop */
    Delay (5);

    /* Move the frames to the discard list */
    ObtainSemaphore (&lod->lod_DLock);
    ObtainSemaphore (&lod->lod_FLock);
    while (fn = (struct FrameNode *) RemHead (&lod->lod_FrameList))
    {
	lod->lod_InList--;
	lod->lod_OutList++;
	AddTail (&lod->lod_DiscardList, (struct Node *) fn);
    }
    ReleaseSemaphore (&lod->lod_FLock);
    ReleaseSemaphore (&lod->lod_DLock);
}

/*****************************************************************************/

void FreeFrameLists (struct ClassBase * cb, Object * o, struct localData * lod)
{
    struct FrameNode *fn;
    struct adtFrame *alf;

    /* Free the frame list */
    while (fn = (struct FrameNode *) RemHead (&lod->lod_FrameList))
    {
	/* Unload the frame */
	alf = &fn->fn_Frame;
	alf->MethodID = ADTM_UNLOADFRAME;
	DoMethodA (o, (Msg) alf);

	/* Free our portion */
	FreeVec (fn);
    }

    /* Free the discarded frame list */
    while (fn = (struct FrameNode *) RemHead (&lod->lod_DiscardList))
    {
	/* Unload the frame */
	alf = &fn->fn_Frame;
	alf->MethodID = ADTM_UNLOADFRAME;
	DoMethodA (o, (Msg) alf);

	/* Free our portion */
	FreeVec (fn);
    }
    lod->lod_InList = lod->lod_OutList = 0;
}

/*****************************************************************************/

#if 0
static void ASM CopyBitMap (
    REG (a6) struct ClassBase *cb,
    REG (a2) struct BitMap *bm1, REG (a3) struct BitMap *bm2, REG (d2) ULONG width)
{
    ULONG bpr1 = bm1->BytesPerRow;
    ULONG bpr2 = bm2->BytesPerRow;
    register UBYTE *src;
    register UBYTE *dst;
    register LONG r;
    register LONG p;

    for (p = bm1->Depth - 1; p >= 0; p--)
    {
	src = (BYTE *) bm1->Planes[p];
	dst = (BYTE *) bm2->Planes[p];

	for (r = bm1->Rows - 1; r >= 0; r--)
	{
	    CopyMem (src, dst, width);
	    src += bpr1;
	    dst += bpr2;
	}
    }
}
#endif

/*****************************************************************************/

ULONG notify (struct ClassBase * cb, Object * o, struct localData *lod, ULONG tag1, ...)
{
    struct opUpdate opu;

    /* Fill in the message */
    opu.MethodID     = OM_NOTIFY;
    opu.opu_AttrList = (struct TagItem *)&tag1;
    opu.opu_GInfo    = NULL;
    opu.opu_Flags    = NULL;

    /* Send the notification message */
    return DoGadgetMethodA ((struct Gadget *) o, lod->lod_Window, NULL, (Msg)&opu);
}

/*****************************************************************************/

/* They must have made sure there is a adtFrame and a alf_BitMap */
void DisplayFrame (struct ClassBase *cb, Object *o, struct localData *lod, struct FrameNode *fn)
{
    struct DTSpecialInfo *si = (struct DTSpecialInfo *) G (o)->SpecialInfo;
    struct adtFrame *alf = &fn->fn_Frame;
    LONG width, height;
    struct IBox *b;

    /* Get the bitmap */
    DB (kprintf ("df %ld\n", alf->alf_Frame));
    if (lod->lod_Flags & LODF_REMAP)
    {
	DB (kprintf (" xlate\n"));
	XLateBitMap (alf->alf_BitMap, lod->lod_BMap, lod->lod_ColorTable, lod->lod_ColorTable2,	lod->lod_BMHD.bmh_Width);
    }
    else if (TypeOfMem (alf->alf_BitMap->Planes[0]) == MEMF_CHIP)
    {
	DB (kprintf (" share\n"));
	lod->lod_BMap =	alf->alf_BitMap;
    }
    else
    {
	DB (kprintf (" copy\n"));
	CopyBitMap (cb, alf->alf_BitMap, lod->lod_BMap, lod->lod_BltWidth);
    }

    /* Don't let them resize while we are rendering */
    LockLayer (0, lod->lod_Window->WLayer);

    /* Get the size */
    GetAttr (DTA_Domain, o, (ULONG *) & b);
    width  = (ULONG) MIN (lod->lod_Width, b->Width);
    height = (ULONG) MIN (lod->lod_BltHeight, b->Height);

    /* Blit the	image on-screen	*/
    DC (kprintf ("%ld %08lx\n", alf->alf_Frame, lod->lod_BMap));
    BltBitMapRastPort (lod->lod_BMap, si->si_TopHoriz, si->si_TopVert,
		       lod->lod_Window->RPort, b->Left, b->Top, width, height, 0xC0);

    /* Unlock the layer */
    UnlockLayer (lod->lod_Window->WLayer);

    /* Remember the update position */
    si->si_OTopVert  = si->si_TopVert;
    si->si_OTopHoriz = si->si_TopHoriz;
}

/*****************************************************************************/

void LocateFrame (struct ClassBase *cb, Object *o, struct localData *lod, ULONG which)
{
    struct FrameNode *fn;
    struct adtFrame *alf;

    /* Locate the frame */
    DB (kprintf ("l %ld\n", which));
    lod->lod_Ticks = 0;
    lod->lod_Frame = lod->lod_Which = which;
    lod->lod_ClockTime = lod->lod_Frame * lod->lod_TicksPerFrame;
    lod->lod_Player->pl_MetricTime = lod->lod_ClockTime;

    /* Tell them we are busy */
    notify (cb, o, lod, GA_ID, G (o)->GadgetID, DTA_Busy, TRUE, TAG_DONE);

    /* Show that we need to locate */
    lod->lod_Flags |= LODF_LOCATE;

    /* Try to get the frame */
    ObtainSemaphore (&lod->lod_DLock);
    if (fn = DiscardAllBut (cb, lod, lod->lod_Frame, 0))
    {
    }
    else if (fn = (struct FrameNode *) RemHead (&lod->lod_DiscardList))
	lod->lod_OutList--;
    else
	fn = AllocVec (sizeof (struct FrameNode), MEMF_CLEAR);
    ReleaseSemaphore (&lod->lod_DLock);

    /* Make sure we have a frame node */
    if (fn)
    {
	/* Cache the pointer */
	alf = &fn->fn_Frame;

#if 1
	/* See if we need to load the frame */
	if (alf->alf_Frame != lod->lod_Frame)
	{
	    DB (kprintf ("alf %ld, lod %ld, bm %08lx\n", alf->alf_Frame, lod->lod_Frame, alf->alf_BitMap));
	    alf->MethodID      = ADTM_LOADFRAME;
	    alf->alf_TimeStamp = lod->lod_Frame;
	    alf->alf_Frame     = lod->lod_Frame;
	    DoMethodA (o, (Msg) alf);
	}
#else
	/* See if we need to load the frame */
	if ((alf->alf_Frame != lod->lod_Frame) && alf->alf_BitMap)
	{
	    alf->MethodID      = ADTM_LOADFRAME;
	    alf->alf_TimeStamp = lod->lod_Frame;
	    alf->alf_Frame     = lod->lod_Frame;
	    DoMethodA (o, (Msg) alf);
	}
#endif

	/* Make sure we have the bitmap */
	if (alf->alf_BitMap)
	{
	    DisplayFrame (cb, o, lod, fn);
	}
	else
	{
	    DB (kprintf ("l no bitmap\n"));
	}

	/* Start the load process */
	Signal ((struct Task *) lod->lod_LoadProc, SIGBREAKF_CTRL_D);

	/* Add the node to the end of the list (should be sorted by timestamp) */
	ObtainSemaphore (&lod->lod_FLock);
	lod->lod_InList++;
	AddTail (&lod->lod_FrameList, (struct Node *)fn);
	ReleaseSemaphore (&lod->lod_FLock);
    }
    else
    {
	DB (kprintf ("l no frame\n"));
    }

    /* Tell them we aren't busy */
    notify (cb, o, lod, GA_ID, G (o)->GadgetID, DTA_Busy, FALSE, TAG_DONE);
}

/*****************************************************************************/

ULONG ASM playerDispatch (REG (a0) struct Hook * h, REG (a2) struct Player * pi, REG (a1) Msg msg)
{
#ifdef	SysBase
#undef	SysBase
#endif
    struct ExecBase *SysBase = (*((struct ExecBase **) 4));
    struct localData *lod = (struct localData *) h->h_Data;
    struct pmTime *pmt = (struct pmTime *) msg;

    switch (pmt->pmt_Method)
    {
	case PM_SHUTTLE:
	    /* Clear ticks */
	    lod->lod_Ticks = 0;

	    /* Turn on shuttle flag */
	    lod->lod_Flags |= LODF_SHUTTLE;
	    pi->pl_MetricTime = lod->lod_Frame = pmt->pmt_Time / lod->lod_TicksPerFrame;
	    Signal ((struct Task *) lod->lod_DispProc, SIGBREAKF_CTRL_F);
	    break;

	case PM_TICK:
	    lod->lod_Ticks++;
	    if (lod->lod_Ticks >= lod->lod_TicksPerFrame)
	    {
		pi->pl_MetricTime = ++lod->lod_Frame;
		lod->lod_Ticks = 0;
		Signal ((struct Task *) lod->lod_DispProc, SIGBREAKF_CTRL_F);
	    }
	    break;

	case PM_STATE:
	    /* Turn off shuttle */
	    lod->lod_Flags &= ~LODF_SHUTTLE;

	    /* Show that we are a state change */
	    lod->lod_Flags |= LODF_STATE;

	    /* Signal state change */
	    Signal ((struct Task *) lod->lod_DispProc, SIGBREAKF_CTRL_F);
	    break;
    }

    return (0);
}

/*****************************************************************************/

void DispHandler (void)
{
    struct ExecBase *SysBase = (*((struct ExecBase **) 4));
    struct DTSpecialInfo *si;
    struct localData *lod;
    struct StartupMsg *sm;
    struct ClassBase *cb;
    struct adtStart asa;
    struct gpRender gpr;
    struct Process *pr;
    struct List *list;
    ULONG sigm;
    ULONG sigs;
    ULONG sigr;
    Class *cl;
    Object *o;

    BOOL stm_locate = FALSE;
    BOOL refresh = FALSE;
    BOOL locate = FALSE;
    BOOL play = FALSE;
    BOOL display;

    struct adtFrame *alf = NULL;
    struct FrameNode *fn;

    struct FrameNode *nfn;
    struct adtFrame *naf;

    /* Find out who we are */
    pr = (struct Process *) FindTask (NULL);

    /* Get the startup message */
    WaitPort (&pr->pr_MsgPort);
    sm = (struct StartupMsg *) GetMsg (&pr->pr_MsgPort);

    /* Pull all the information from the startup message */
    cb = sm->sm_CB;
    cl = sm->sm_Class;
    o = sm->sm_Object;
    lod = INST_DATA (cl, o);
    si = (struct DTSpecialInfo *) G (o)->SpecialInfo;

    /* Initialize the Render message */
    gpr.MethodID = GM_RENDER;
    gpr.gpr_GInfo = NULL;
    gpr.gpr_Redraw = GREDRAW_REDRAW;

    /* Show that we are RUNNING */
    lod->lod_Flags |= LODF_DRUNNING;

    /* Create the message port */
    if (lod->lod_MP = CreateMsgPort ())
    {
	/* Initialize the RealTime hook */
	lod->lod_Hook.h_Entry = ((ULONG (*)()) (playerDispatch));
	lod->lod_Hook.h_Data = lod;

	/* Create the player */
	if (lod->lod_Player = createplayer (cb,
					    PLAYER_Name,	(ULONG) "Anim",
					    PLAYER_Hook,	&lod->lod_Hook,
					    PLAYER_Priority,	0,
					    PLAYER_Conductor,	(ULONG) "Main",
					    TAG_DONE))
	{
	    /* Cache a pointer to the conductor */
	    lod->lod_Conductor = lod->lod_Player->pl_Source;

	    /* All the signals that we're waiting on */
	    sigm = 1L << lod->lod_MP->mp_SigBit;
	    sigs = sigm | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D | SIGBREAKF_CTRL_E | SIGBREAKF_CTRL_F;

	    /* Cache a pointer to the frame list */
	    list = &lod->lod_FrameList;

	    /****************************************/
	    /* Keep going until we are told to stop */
	    /****************************************/
	    while (lod->lod_Flags & LODF_DGOING)
	    {
		/* Are we supposed to start playing? */
		if (play)
		{
		    play = FALSE;
		    locate = TRUE;
		}

		/* Are we supposed to locate a frame? */
		if (locate)
		{
		    /* Set the locate information */
		    lod->lod_Which = lod->lod_Frame;

		    /* Clear the locate flag */
		    locate = FALSE;

		    /* Show that we need a refresh */
		    refresh = TRUE;

#if 0
		    /* See if we should restart */
		    if (lod->lod_Which >= lod->lod_NumFrames - 1)
		    {
			kprintf ("rewind : %ld\n", lod->lod_Conductor->cdt_State);
			LocateFrame (cb, o, lod, 0);
			kprintf ("located 0, now starting\n");
		    }
#endif
		    /* Get the blit width */
		    lod->lod_BltWidth = GetBitMapAttr (lod->lod_KeyFrame, BMA_WIDTH) / 8;

		    /* Start the animation */
		    asa.MethodID  = ADTM_START;
		    asa.asa_Frame = lod->lod_Which;
		    DoMethodA (o, (Msg) & asa);
		}

		/* Are we supposed to refresh the image */
		if (lod->lod_Window && refresh)
		{
		    /* Remove the next frame to display */
		    ObtainSemaphore (&lod->lod_FLock);
		    if (fn = (struct FrameNode *) RemHead (&lod->lod_FrameList))
			lod->lod_InList--;
		    ReleaseSemaphore (&lod->lod_FLock);

		    if (fn)
		    {
			refresh = FALSE;

			/* Cache the pointer to the message */
			alf = &fn->fn_Frame;

			/* Set the sound */
			nfn = NULL;
			ObtainSemaphore (&lod->lod_FLock);
			if (list->lh_TailPred != (struct Node *) list)
			    nfn = (struct FrameNode *) list->lh_Head;
			ReleaseSemaphore (&lod->lod_FLock);

			if (nfn)
			{
			    naf = &nfn->fn_Frame;
			    if (lod->lod_SndObj)
			    {
				setattrs (cb, (struct Gadget *) lod->lod_SndObj,
					  SDTA_Sample, naf->alf_Sample,
					  TAG_DONE);
			    }
			}

			display = TRUE;
			if (alf->alf_BitMap == NULL)
			    display = FALSE;
			else if (lod->lod_Window->WScreen->ViewPort.Modes & VP_HIDE)
			    display = FALSE;
			else if (si->si_Flags & DTSIF_LAYOUT)
			    display = FALSE;
#if 0
			else if (alf->alf_Frame < lod->lod_Frame - 5)
			    display = FALSE;
#endif

			if (display)
			{
			    /* Display the frame */
			    DisplayFrame (cb, o, lod, fn);

			    /* Update the current frame */
			    if (lod->lod_Flags & LODF_CONTROLS)
			    {
				/* Set the current frame */
				setattrs (cb, lod->lod_TapeDeck, TDECK_CurrentFrame, alf->alf_Frame, TAG_DONE);

				/* Initialize the message */
				gpr.MethodID   = GM_RENDER;
				gpr.gpr_GInfo  = NULL;
				gpr.gpr_RPort  = lod->lod_Window->RPort;
				gpr.gpr_Redraw = 100;
				DoGadgetMethodA (lod->lod_TapeDeck, lod->lod_Window, NULL, (Msg) &gpr);
				gpr.gpr_Redraw = GREDRAW_REDRAW;
			    }
			}

			/* Add the node to the discard list */
			ObtainSemaphore (&lod->lod_DLock);
			lod->lod_OutList++;
			AddTail (&lod->lod_DiscardList, (struct Node *) fn);
			ReleaseSemaphore (&lod->lod_DLock);
		    }

		    /* All done playing now */
		    if ((alf == NULL) ||
			(alf && (alf->alf_Frame >= lod->lod_NumFrames - 1)) ||
			(fn && fn->fn_Node.ln_Type))
		    {
			/* Stop the animation */
			asa.MethodID = ADTM_STOP;
			DoMethodA (o, (Msg) & asa);
		    }
		}

		/********************************/
		/* Wait for something to happen */
		/********************************/
		sigr = Wait (sigs);
		/********************************/
		/********************************/
		/********************************/

		/* Handle player information */
		if (lod->lod_Window && (sigr & SIGBREAKF_CTRL_F))
		{
		    /* Handle states */
		    switch (lod->lod_Conductor->cdt_State)
		    {
			case CONDSTATE_METRIC:
			    break;

			case CONDSTATE_SHUTTLE:
			    break;

			case CONDSTATE_STOPPED:
			case CONDSTATE_PAUSED:
			    if (lod->lod_Flags & LODF_SHUTTLE)
			    {
				lod->lod_Which = lod->lod_Frame;
				if (lod->lod_Which > lod->lod_NumFrames)
				{
				    lod->lod_Which = lod->lod_NumFrames;
				}
				else
				{
				    stm_locate = TRUE;
#if 1
				    if (lod->lod_Flags & LODF_CONTROLS)
					SetTapeDeckAttrs (cb, lod, TDECK_CurrentFrame, lod->lod_Which, TAG_DONE);
#else
				    if ((lod->lod_Flags & LODF_CONTROLS) && (lod->lod_Active == NULL))
					SetTapeDeckAttrs (cb, lod, TDECK_CurrentFrame, lod->lod_Which, TAG_DONE);
#endif
				}
				break;
			    }
			    else if (lod->lod_Flags & LODF_PLAYING)
			    {
				DoMethod (o, ADTM_PAUSE);
			    }
			    break;

			case CONDSTATE_LOCATE:
			    /* Start with a clean slate */
			    DiscardAllFrames (cb, lod);

			    /* Figure out what frame to display and display it */
			    lod->lod_ClockTime = lod->lod_Conductor->cdt_ClockTime;
			    lod->lod_Frame = lod->lod_ClockTime / lod->lod_TicksPerFrame;
			    if (lod->lod_Frame > lod->lod_NumFrames)
				lod->lod_Frame = lod->lod_NumFrames;
			    lod->lod_Which = lod->lod_Frame;

			    locate = TRUE;
			    break;

			case CONDSTATE_RUNNING:
			    if (!(lod->lod_Flags & LODF_PLAYING))
				break;

			    lod->lod_Which = lod->lod_Frame;
			    if (lod->lod_Which > lod->lod_NumFrames)
				lod->lod_Which = lod->lod_NumFrames;
			    refresh = TRUE;
			    break;
		    }

		    /* Clear the state change flag */
		    lod->lod_Flags &= ~LODF_STATE;
		}

		/* Start playing? */
		if (lod->lod_Window && (sigr & SIGBREAKF_CTRL_D))
		    play = TRUE;

		/* Dispose of the object */
		if (sigr & SIGBREAKF_CTRL_C)
		{
		    lod->lod_Flags &= ~(LODF_DGOING | LODF_ATTACHED);
		    lod->lod_Window = NULL;
		    WaitBlit ();
		}

		/* Time to remove the animation? */
		if (sigr & SIGBREAKF_CTRL_E)
		{
		    /* Free the frame lists */
		    FreeFrameLists (cb, o, lod);

		    lod->lod_Flags &= ~LODF_ATTACHED;
		    lod->lod_Window = NULL;
		    WaitBlit ();
		}

		/* Handle trigger messages */
		if (sigr & sigm)
		{
		    struct TriggerMsg *tm;

		    /* Pull all the trigger messages */
		    while (tm = (struct TriggerMsg *) GetMsg (lod->lod_MP))
		    {
			switch (tm->tm_Function)
			{
			    case STM_PLAY:
				play = TRUE;
				break;

			    case STM_PAUSE:
				DoMethod (o, ADTM_PAUSE);
				break;

			    case STM_STOP:
				DoMethod (o, ADTM_STOP);
				break;

			    case STM_REWIND:
				break;

			    case STM_FASTFORWARD:
				break;

			    /* We are supposed to load this frame and
			     * display it, and then start preloading. */
			    case STM_LOCATE:
				stm_locate = TRUE;

				/* Compute the clock */
				lod->lod_Which = lod->lod_Frame = (ULONG) tm->tm_Data;
				break;
			}

			FreeVec (tm);
		    }
		}

		/* Are we supposed to locate a frame */
		if (stm_locate)
		{
		    stm_locate = FALSE;
		    if (lod->lod_Window)
			LocateFrame (cb, o, lod, lod->lod_Which);
		}
	    }

	    /****************************************/
	    /*           All done playing           */
	    /****************************************/

	    DeletePlayer (lod->lod_Player);
	}

	DeleteMsgPort (lod->lod_MP);
    }

    /* Exit now */
    Forbid ();

    /* Show that we aren't running anymore */
    lod->lod_Flags &= ~LODF_DRUNNING;
}
