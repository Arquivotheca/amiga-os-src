/*** ism.h *****************************************************************
 *
 *  Intuition State Machine include file
 *
 *  $Id: ism.h,v 40.0 94/02/15 17:36:33 davidj Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1989, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/

#define WAITTOF_OPTIMIZE	0

/* input token command codes
 * these codify "types" of input event and other commands which
 * are to be presented to the state machine.  For each, we
 * document the fields in the InputToken which will be valid.
 * In each case, the InputEvent will be valid somehow.
 */
enum ITCommand
{
    /* This first set corresponds roughly to input events,
     * the other InputToken fields are not used.
     */
    itMENUDOWN,		/* 0 */
    itMENUUP,		/* 1 */
    itSELECTDOWN,	/* 2 */
    itSELECTUP,		/* 3 */
    itMOUSEMOVE,	/* 4 */
			/* it_Box.Left/Top are scaled delta mouse ticks	*/
			/* it_IE->ie_xy are old-coords deltas for
			 * DELTAMOVE messages
			 */
    itRAWKEY,		/* 5 */
    itTIMER,		/* 6 */
    itDISKCHANGE,	/* 7 : (ULONG) it_Object1 = IECLASS_DISKINSERTED/REMOVED*/

    /* These are commands.  The InputEvent field is used only for
     * passing along input events
     */
    itACTIVATEWIN,	/* 8 : it_Object1 = window, it_Object2 = AWIN_ code from
			 * intuinternal.h.  it_SubCommand = menu code for
			 * AWIN_LENDMENUKEY.
			 */
    itACTIVATEGAD,	/* 9 : it_Object1 = window, it_Object2 = gadget	*/
			/* sorry: requester parameter is stuck in
			 * it_SubCommand
			 */
    itSETREQ,		/* 10 : it_Object1 = window, it_Object2 = requester	*/
    itCLEARREQ,		/* 11 : it_Object1 = window, it_Object2 = requester	*/
    itMETADRAG,		/* 12 : nothing	*/
    /**** I DON'T THINK WE NEED OPENWINDOW: OpenWindow() might
     **** do a deferred ActivateWindow though
     ****/
/*13*/    itOPENWIN,	/* 13 : it_Object1 = window, it_Object2 = screen	*/
    itCLOSEWIN,		/* 14 : it_Object1 = window				*/
    itVERIFY,		/* 15 : it_Object1 = window, it_SubCommand = code	*/
    itREMOVEGAD,	/* 16 : it_Object1 = gadget, it_Object2 = firstgadget
			 * it_SubCommand = numgad
			 */
    itSETMENU,		/* 17 : it_Object1 = window, it_Object2 = menu	*/
    itCLEARMENU,	/* 18 : it_Object1 = window				*/
    itCHANGEWIN,	/* 19 : it_Object1 = window, it_Box = pos/dims
			 * it_SubCommand = listed below as CWSUB_*
			 */
    itDEPTHWIN,		/* 20 : it_Object1 = window
			 * it_SubCommand = front/back/infrontof
			 * it_Object2 = infrontof window.
			 */
/*21*/    itMOVESCREEN,	/* 21 : it_Object1 = screen, it_Object2 = &point 	*/
    itDEPTHSCREEN,	/* 22 : it_Object1 = screen, it_SubCommand = f/b	*/

    itCHANGESCBUF,	/* 23 : it_Object1 = screen, it_Object2 = screenbuffer */

    itOPENSCREEN,	/* 24 : it_Object1 = screen				*/
    itCLOSESCREEN,	/* 25 : it_Object1 = screen				*/
    itNEWPREFS,		/* 26 : nothing	*/
    itMODIFYIDCMP,	/* 27 : it_Object1 = window, it_Object2 = Flags (cast)*/
/*28*/    itZOOMWIN,	/* 28 : it_Object1 = window, it_SubCommand = verify? */
    itCLOSEWB,		/* 29 : it_Object1 = msg				*/
    itUNKNOWNIE,	/* 30 : nothing	*/
    itSHOWTITLE,	/* 31 : it_Object1 = screen, it_SubCommand = showit	*/
    itONOFFMENU,	/* 32 : it_Object1 = w, it_Object2 = (UWORD!) menunum,
			 *  it_SubCommand = onOrOff (true means ON)
			 */
    itCOPYSCBUF,	/* 33 : it_Object1 = screen, it_Object2 = ScreenBuffer */
    itSETPOINTER,	/* 34 : it_Object1 = window, it_Object2 = pointer */
    itGADGETMETHOD,	/* 35 : it_Object1 = window, it_Object2 = gadget,
			 * it_SubCommand = message
			 */
    itMODIFYPROP,	/* 36 : it_Object1 = window, it_Object2 = gadget,
			 * it_SubCommand = PropPacket
			 */
#if WAITTOF_OPTIMIZE
    itFREECPRLIST,	/* 37 : it_Object1 = cprlist 1, it_Object2 = cprlist 2,
			 * it_SubCommand = VBlank counter
			 */
    itFREESPRITEDATA,	/* 38 : it_Object1 = ExtSprite, it_SubCommand = VBlank counter */
#endif
};

/* sub commands for itCHANGEWIN	*/
#define CWSUB_CHANGE		0x0001	/* just change to it_Box		*/
#define CWSUB_SIZEDELTA 	0x0002	/* it_Box.Width/Height holds deltas	*/
#define CWSUB_MOVEDELTA 	0x0003	/* it_Box.Left/Top holds deltas		*/

#define CWSUB_SUBCOMMAND	0x0003	/* command-part */

/* Here are some qualifiers that can be applied: */
#define CWSUBQ_SIZING		0x0004	/* involves sizing (incl. zoom)		*/
#define CWSUBQ_PROGRAMMATIC	0x0008	/* operation not user-initiated		*/

/* combination of InputEvent and old DistantEcho:
 * contains parameters for a command which passes through
 * the state machine, as well as input event which is either
 * directly from input.device or constructed from last received
 * InputEvent
 *
 * Very important lesson:  You cannot make it_Node into a MinNode
 * because the pool stuff (pools.c) uses ln_Name as a back-pointer
 * to the list.  Remember that.
 */
struct InputToken	{
    struct Node	it_Node;	/* in list				*/
    UWORD	it_Flags;	/* below				*/
    enum ITCommand  it_Command;	/* from enum above			*/
    ULONG	it_SubCommand;	/* such as front/back, ...		*/
    CPTR	it_Object1;	/* window, screen, gadget, parameter	*/
    CPTR	it_Object2;	/* secondary pointer parameter		*/
    union
    {
	struct IBox it_box;	/* pos/dim parameters for itCHANGEWIN	*/
	struct LongPoint it_longmouse;	/* For itMOUSEMOVE		*/
    } it_Position;

    struct InputEvent	*it_IE;	/* copy of input event			*/
    struct Task	*it_Task;	/* who to signal for ITF_SIGNAL 	*/
    struct TabletData *it_Tablet;	/* Tablet data if any		*/
    UBYTE	it_Code;	/* result of RawKeyConvert for RAWKEY	*/
    ULONG	it_Error;	/* return code				*/
};

#define it_Box		it_Position.it_box
#define it_LongMouse	it_Position.it_longmouse

/* InputToken Flag values	*/
#define ITF_NOTIFYDONE	(0x0001) /* wants to know when event is finished */

#define ITF_CALLBACK	(0x0002) /* notify by function callback		*/
/* NOT  CURRENTLY SUPPORTED */

#define ITF_SIGNAL	(0x0004) /* notify by signalling		*/

#define ITF_DEFERRED	(0x0008) /* this token has been deferred	*/
#define ITF_DONOTDEFER	(0x0010) /* for debugging			*/
#define ITF_NOPOOL	(0x0020) /* should not be returned to pool	*/
#define ITF_QUICK	(0x0040) /* like IOQUICK: try it right away	*/
#define ITF_REUSE	(0x0080) /* don't also set ITF_DEFERRED		*/

enum IntuitionStates	{
    sNoWindow,
    sIdleWindow,
    sRequester,
    sMenu,
    sDMRTimeout,
    sScreenDrag,
    sVerify,
    sGadget,
    sWSizeDrag,		/* 8 */
};

/* states are folded together: let know which */
#define WSD_SIZE	0
#define WSD_DRAG	1

#define IT	(IBase->InputToken)
#define IE	(IBase->InputToken->it_IE)

#if 0	/* they're in there.	*/
/* ZZZ: remember to claim this signal */
#define SIGB_INTUITION	(5)
#define SIGF_INTUITION 	(1<< SIGB_INTUITION)
#endif
