/*** dispatch.c ***********************************************************
*
*   dispatch.c	- Gadget Toolkit dispatch routines
*
*   Copyright 1989, Commodore-Amiga, Inc.
*
*   $Id: dispatch.c,v 39.18 93/02/11 10:48:39 vertex Exp $
*
**************************************************************************/

/*------------------------------------------------------------------------*/

#include "gtinclude.h"

/*------------------------------------------------------------------------*/

/* Function Prototypes: */

/* Public: */
struct IntuiMessage * __asm
LIB_GT_GetIMsg (register __a0 struct MsgPort *iport);

void __asm
LIB_GT_ReplyIMsg (register __a1 struct IntuiMessage *imsg);

void __asm
LIB_GT_RefreshWindow (register __a0 struct Window *win);

void __asm
LIB_GT_BeginRefresh (register __a0 struct Window *win);

void __asm
LIB_GT_EndRefresh (register __a0 struct Window *win,
		  register __d0 BOOL complete);

struct IntuiMessage * __asm
LIB_GT_FilterIMsg (register __a1 struct IntuiMessage *imsg);

struct IntuiMessage *__asm
LIB_GT_PostFilterIMsg (register __a1 struct IntuiMessage *imsg);

void __asm
LIB_GT_SetGadgetAttrsA (register __a0 struct ExtGadget *gad,
		       register __a1 struct Window *win,
		       register __a3 struct TagItem *taglist);

struct ExtGadget * __asm
LIB_CreateContext (register __a0 struct ExtGadget **glistptr);

/* Internal: */
static struct ExtGadget *FindContext (struct Window *win);
static void WalkRefreshGadgets (struct Window *win, BOOL refresh);

/*------------------------------------------------------------------------*/

/****** gadtools.library/GT_GetIMsg ******************************************
*
*   NAME
*	GT_GetIMsg -- get an IntuiMessage, with GadTools processing. (V36)
*
*   SYNOPSIS
*	imsg = GT_GetIMsg(intuiport)
*	D0                A0
*
*	struct IntuiMessage *GT_GetIMsg(struct MsgPort *);
*
*   FUNCTION
*	Use GT_GetIMsg() in place of the usual exec.library/GetMsg() when
*	reading IntuiMessages from your window's UserPort.  If needed,
*	the GadTools dispatcher will be invoked, and suitable processing
*	will be done for gadget actions.  This function returns a pointer
*	to a modified IntuiMessage (which is a copy of the original,
*	possibly with some supplementary information from GadTools).
*	If there are no messages (or if the only messages are meaningful
*	only to GadTools, NULL will be returned.
*
*   INPUTS
*	intuiport - the Window->UserPort of a window that is using the
*	            Gadget Toolkit.
*
*   RESULT
*	imsg - pointer to modified IntuiMessage, or NULL if there are
*	       no applicable messages.
*
*   NOTES
*	Be sure to use GT_ReplyIMsg() and not exec.library/ReplyMsg() on
*	messages obtained with GT_GetIMsg().
*	If you intend to do more with the resulting message than read
*	its fields, act on it, and reply it, you may find GT_FilterIMsg()
*	more appropriate.
*
*	Starting with V39, this function actually returns a pointer to an
*	ExtIntuiMessage structure, but the prototype was not changed for
*	source code compatibility with older software.
*
*   SEE ALSO
*	GT_ReplyIMsg(), GT_FilterIMsg()
*
******************************************************************************
*
* Created:  10-Nov-89, Peter Cherna
* Modified: 10-Nov-89, Peter Cherna
*
*/

struct IntuiMessage * ASM LIB_GT_GetIMsg(REG(a0) struct MsgPort *iport)
{
struct IntuiMessage *clientmsg;
struct IntuiMessage *imsg;

    /* Get a message until there is one that isn't eaten by the toolkit,
     * or until no more are available:
     */
    while (imsg = (struct IntuiMessage *)GetMsg(iport))
    {
	DP(("GT_GM: Win $%lx, have imsg of class $%lx, IAddress $%lx\n",imsg->IDCMPWindow, imsg->Class, imsg->IAddress));

	/* Do the dispatch */
	if (clientmsg = GT_FilterIMsg((struct IntuiMessage *)imsg))
	    return(clientmsg);

        /* Message was eaten, so ReplyMsg() it and get another one
         * if available
         */
	ReplyMsg((struct Message *)imsg);
    }

    return(NULL);
}


/*------------------------------------------------------------------------*/

/****** gadtools.library/GT_ReplyIMsg ****************************************
*
*   NAME
*	GT_ReplyIMsg -- reply a message obtained with GT_GetIMsg(). (V36)
*
*   SYNOPSIS
*	GT_ReplyIMsg(imsg)
*	             A1
*
*	VOID GT_ReplyIMsg(struct IntuiMessage *);
*
*   FUNCTION
*	Reply a modified IntuiMessage obtained with GT_GetIMsg().
*	If you use GT_GetIMsg(), use this function where you would normally
*	have used exec.library/ReplyMsg().
*	You may safely call this routine with a NULL pointer (nothing
*	will be done).
*
*   INPUTS
*	imsg - a modified IntuiMessage obtained with GT_GetIMsg(), or NULL
*	       in which case this function does nothing
*
*   NOTES
*	When using GadTools, you MUST explicitly GT_ReplyIMsg()
*	all messages you receive.  You cannot depend on CloseWindow()
*	to handle messages you have not replied.
*
*	Starting with V39, this function actually expects a pointer to an
*	ExtIntuiMessage structure, but the prototype was not changed for
*	source code compatibility with older software.
*
*   SEE ALSO
*	GT_GetIMsg()
*
******************************************************************************
*
* Created:  14-Oct-89, Peter Cherna
* Modified: 09-Nov-89, Peter Cherna
*
*/

VOID ASM LIB_GT_ReplyIMsg(REG(a1) struct IntuiMessage *imsg)
{
    if (imsg)
    {
	/* Reply the REAL IntuiMessage that was in play here: */
	ReplyMsg((struct Message *)GT_PostFilterIMsg(imsg));
    }
}


/*------------------------------------------------------------------------*/

/*/ WalkRefreshGadgets()
 *
 * Function to walk a window's gadget list and do all the special
 * refresh for the toolkit gadgets.  The boolean parameter tells
 * whether the refresh functions are expected to do Begin/EndRefresh()
 *
 * Notes:  When we go with custom gadgets, this function won't
 * be necessary, since our gadgets will have the same powers of
 * refresh as regular gadgets.
 *
 * Created:  17-Oct-89, Peter Cherna
 * Modified: 17-Nov-89, Peter Cherna
 *
 */

static VOID WalkRefreshGadgets(struct Window *win,
                               BOOL refresh)
{
struct ExtGadget *gad;

    /* Walk the list of gadgets in the supplied window: */

    gad = (struct ExtGadget *)win->FirstGadget;
    while (gad)
    {
        MP(("GT_RW:  gadget at $%lx", gad));
        /* Is this a Toolkit gadget? */
        if (gad->GadgetType & GADTOOL_TYPE)
        {
            MP((" is a toolkit gadget"));
            /* Does it have a refresh function? */
            if (SGAD(gad)->sg_Refresh)
            {
                MP((", refresh fcn at $%lx\n", SGAD(gad)->sg_Refresh));
                /* Do the refresh: */
                (*SGAD(gad)->sg_Refresh)(gad, win, refresh);
            }
        }
        MP(("\n"));
        gad = (struct ExtGadget *)gad->NextGadget;
    }
}

/*------------------------------------------------------------------------*/

/****** gadtools.library/GT_RefreshWindow ************************************
*
*   NAME
*	GT_RefreshWindow -- refresh all GadTools gadgets in a window. (V36)
*
*   SYNOPSIS
*	GT_RefreshWindow(win, req)
*	                 A0   A1
*
*	VOID GT_RefreshWindow(struct Window *, struct Requester *);
*
*   FUNCTION
*	Perform the initial refresh of all the GadTools gadgets you have
*	created.  After you have opened your window, you must call this
*	function.  Or, if you have opened your window without gadgets,
*	you add the gadgets with intuition.library/AddGList(),
*	refresh them using intuition.library/RefreshGList(), then call
*	this function.
*	You should not need this function at other times.
*
*   INPUTS
*	win - pointer to the Window containing GadTools gadgets.
*	req - reserved for future use, should always be NULL
*
*   SEE ALSO
*	GT_BeginRefresh()
*
******************************************************************************
*
* Created:  17-Oct-89, Peter Cherna
* Modified:  5-Dec-89, Peter Cherna
*
* NOTES
*	Since the requester parameter is not currently used, the actual
*	function does not define this parameter which makes the code
*	smaller.
*/

VOID ASM LIB_GT_RefreshWindow(REG(a0) struct Window *win)
{
    /* Do the refresh, but inform that we're not inside Begin/EndRefresh() */
    WalkRefreshGadgets(win, FALSE);
}


/*------------------------------------------------------------------------*/

/****** gadtools.library/GT_BeginRefresh *************************************
*
*   NAME
*	GT_BeginRefresh -- begin refreshing friendly to GadTools. (V36)
*
*   SYNOPSIS
*	GT_BeginRefresh(win)
*	                A0
*
*	VOID GT_BeginRefresh(struct Window *);
*
*   FUNCTION
*	Invokes the intuition.library/BeginRefresh() function in a manner
*	friendly to the Gadget Toolkit.  This function call permits the
*	GadTools gadgets to refresh themselves at the correct time.
*	Call GT_EndRefresh() function when done.
*
*   INPUTS
*	win - pointer to Window structure for which a IDCMP_REFRESHWINDOW
*	      IDCMP event was received.
*
*   NOTES
*	The nature of GadTools precludes the use of the IDCMP flag
*	WFLG_NOCAREREFRESH.  You must handle IDCMP_REFRESHWINDOW events
*	in at least the minimal way, namely:
*
*		case IDCMP_REFRESHWINDOW:
*		    GT_BeginRefresh(win);
*		    GT_EndRefresh(win, TRUE);
*		    break;
*
*   SEE ALSO
*	intuition.library/BeginRefresh()
*
******************************************************************************
*
* Created:  21-Nov-89, Peter Cherna
* Modified: 21-Nov-89, Peter Cherna
*
*/

VOID ASM LIB_GT_BeginRefresh(REG(a0) struct Window *win)
{
    BeginRefresh(win);
    WalkRefreshGadgets(win, TRUE);
}


/*------------------------------------------------------------------------*/

/****** gadtools.library/GT_EndRefresh ***************************************
*
*   NAME
*	GT_EndRefresh -- end refreshing friendly to GadTools. (V36)
*
*   SYNOPSIS
*	GT_EndRefresh(win, complete)
*	              A0   D0
*
*	VOID GT_EndRefresh(struct Window *, BOOL complete);
*
*   FUNCTION
*	Invokes the intuition.library/EndRefresh() function in a manner
*	friendly to the Gadget Toolkit.  This function call permits
*	GadTools gadgets to refresh themselves at the correct time.
*	Call this function to EndRefresh() when you have used
*	GT_BeginRefresh().
*
*   INPUTS
*	win - pointer to Window structure for which a IDCMP_REFRESHWINDOW
*	      IDCMP event was received.
*	complete - TRUE when done with refreshing.
*
*   SEE ALSO
*	intuition.library/EndRefresh()
*
******************************************************************************
*
* Created:  14-Dec-89, Peter Cherna
* Modified: 14-Dec-89, Peter Cherna
*
*/

VOID ASM LIB_GT_EndRefresh(REG(a0) struct Window *win, REG(d0) BOOL complete)
{
    /* Do nothing special function.  Put here by popular demand for
     * symmetry
     */
    EndRefresh(win, complete);
}

/*------------------------------------------------------------------------*/

/****** gadtools.library/GT_FilterIMsg ***************************************
*
*   NAME
*	GT_FilterIMsg -- filter an IntuiMessage through GadTools. (V36)
*
*   SYNOPSIS
*	modimsg = GT_FilterIMsg(imsg)
*	D0                      A1
*
*	struct IntuiMessage *GT_FilterIMsg(struct IntuiMessage *);
*
*   FUNCTION
*	NOTE WELL:  Extremely few programs will actually need this function.
*	You almost certainly should be using GT_GetIMsg() and GT_ReplyIMsg()
*	only, and not GT_FilterIMsg() and GT_PostFilterIMsg().
*
*	GT_FilterIMsg() takes the supplied IntuiMessage and asks the
*	Gadget Toolkit to consider and possibly act on it.  Returns
*	NULL if the message was only of significance to a GadTools gadget
*	(i.e. not to you), else returns a pointer to a modified IDCMP
*	message, which may contain additional information.
*
*	You should examine the Class, Code, and IAddress fields of
*	the returned message to learn what happened.  Do not make
*	interpretations based on the original imsg.
*
*	You should use GT_PostFilterIMsg() to revert to the original
*	IntuiMessage once you are done with the modified one.
*
*   INPUTS
*	imsg - an IntuiMessage you obtained from a Window's UserPort.
*
*   RESULT
*	modimsg - a modified IntuiMessage, possibly with extra information
*	          from GadTools, or NULL. When NULL, the message passed in to
*		  the function should be sent back to Intuition via ReplyMsg()
*
*   NOTES
*	Starting with V39, this function actually expects and returns
*	pointers to ExtIntuiMessage structures, but the prototype was not
*	changed for source code compatibility with older software.
*
*   WARNING
*	If this function returns NULL, you must call ReplyMsg() on the
*	IntuiMessage you passed in to GT_FilterIMsg(). That is, if the
*	message was processed by the toolkit you must reply this message
*	to Intuition since gadtools will not do this automatically.
*
*   SEE ALSO
*	GT_GetIMsg(), GT_PostFilterIMsg()
*
******************************************************************************
*
* Created:  10-Nov-89, Peter Cherna
* Modified: 10-Nov-89, Peter Cherna
*
* Function that interprets an IntuiMessage and dispatches it to the
* appropriate handler routine as necessary.  Returns the address of
* the modified message if the message wasn't eaten, else returns NULL.
*
* Created:  10-Nov-89, Peter Cherna (from old SpecialGetIMsg())
* Modified: 22-Mar-90, Peter Cherna
*/

struct IntuiMessage * ASM LIB_GT_FilterIMsg(REG(a1) struct IntuiMessage *imsg)
{
struct ExtGadget      *gad;
struct QuasiMessage   *quasi;
/* Pointer to context  data, which contains active gadget. */
struct ExtGadget      *ctgad;
/* Assume we're eating this message, until proven otherwise: */
BOOL                   msgforclient = FALSE;

    MP(("GT_FIM: Enter\n"));

    /* Obtain the shared QuasiIntuiMessage, or allocate a new one */

    /* Get window's context IData.  Use its embedded QuasiMessage, if available. */
    if (( ctgad = FindContext(imsg->IDCMPWindow) ) && ( !CTID(ctgad)->ctid_QuasiUsed ))
    {
	/* Mark the embedded QuasiMessage as used */
	CTID(ctgad)->ctid_QuasiUsed++;
	quasi = &CTID(ctgad)->ctid_QuasiMessage;
	quasi->qm_ContextGadget = ctgad;
	DP((" GT_FIM:  Marking the embedded quasi as used\n"));
    }
    else
    {
	/* If the context IData can't be found, or the embedded
	 * QuasiMessage is in use, allocate a new one
	 */
	quasi = AllocVec(sizeof(struct QuasiMessage), MEMF_CLEAR);
	DP((" GT_FIM:  QM unavailable!  AllocVec()'d one at $%lx\n", quasi));
	/* NOTE WELL: qm_ContextGadget is NULL.  We key off this to
	 * know to free this later
	 */
    }

    if (quasi)
    {
	/* Make QuasiMessage contain a copy of the IntuiMessage, and
	 * a pointer to the original IntuiMessage:
	 */
	quasi->qm_IMessage = *(struct ExtIntuiMessage *)imsg;
	quasi->qm_OrigMessage = imsg;

	gad = NULL;

	if ((imsg->Class == IDCMP_GADGETUP) || (imsg->Class == IDCMP_GADGETDOWN))
	{
#ifdef DEBUGGING
	    if (imsg->Class == IDCMP_GADGETUP)
		DP(("GT_FIM: Have a IDCMP_GADGETUP\n"));
	    else
		DP(("GT_FIM: Have a IDCMP_GADGETDOWN\n"));
#endif
	    /* Found a gadget event, identify the gadget: */
	    gad = (struct ExtGadget *) imsg->IAddress;
	}
	/* If we have a IDCMP_MOUSEMOVE/IDCMP_INTUITICKS, and there is an ActiveGadget: */
	else if ( ((imsg->Class == IDCMP_MOUSEMOVE)
                || (imsg->Class == IDCMP_INTUITICKS)
                || (imsg->Class == IDCMP_MOUSEBUTTONS)) &&
	    (ctgad) && (gad = CTID(ctgad)->ctid_ActiveGadget) )
	{
	    /* Since IDCMP_MOUSEMOVE's and IDCMP_INTUITICKS don't have the responsible
	     *	gadget in the IAddress field, we are sure to stuff the
	     *	address of the active gadget into our context instance
	     *	data, so that we can extract it here.
	     *	It would be NULL for a Window REPORTMOUSE-generated IDCMP_MOUSEMOVE,
	     *	or a regular IDCMP_INTUITICKS.
	     */
	    /* We have either a IDCMP_MOUSEMOVE or an IDCMP_INTUITICKS, and the gadget
	     * wants one or the other, or both.  If we have one that the
	     * gadget doesn't want, we don't want to send it a message:
	     */
	    if (((imsg->Class == IDCMP_MOUSEMOVE) &&
		(!(SGAD(gad)->sg_Flags & SG_MOUSEMOVE))) ||
		((imsg->Class == IDCMP_INTUITICKS) &&
		(!(SGAD(gad)->sg_Flags & SG_INTUITICKS))) ||
		((imsg->Class == IDCMP_MOUSEBUTTONS) &&
		(!(SGAD(gad)->sg_Flags & SG_MOUSEBUTTONS))))
	    {
		gad = NULL;
	    }
#ifdef DEBUGGING
	    if (imsg->Class == IDCMP_MOUSEMOVE)
		DP(("GT_FIM:  Have a Gadget IDCMP_MOUSEMOVE, gad $%lx\n",gad));
	    else
		DP(("GT_FIM:  Have a Gadget IDCMP_INTUITICKS, gad $%lx\n",gad));
#endif
	}
	D(if (gad)
	{
	    DP(("GT_FIM: GadgetID = $%lx, EventHandler = $%lx\n",
		gad->GadgetID, SGAD(gad)->sg_EventHandler));
	    DP(("GT_FIM: Activation = $%lx\n", gad->Activation));
	});

	/* Do we have a gadget, is it a toolkit gadget, and if so,
	 * does it have a handler?
	 */
	if ((gad) && (gad->GadgetType & GADTOOL_TYPE) &&
	    (SGAD(gad)->sg_EventHandler) )
	{
	    /* If this is a toolkit gadget that cares about IDCMP_MOUSEMOVE or
	     * IDCMP_INTUITICKS events, set or clear the context ActiveGadget
	     * upon IDCMP_GADGETDOWN or IDCMP_GADGETUP respectively:
	     */
	    if (SGAD(gad)->sg_Flags & (SG_MOUSEMOVE | SG_INTUITICKS | SG_MOUSEBUTTONS))
	    {
		if ((imsg->Class == IDCMP_GADGETUP)
                ||  ((imsg->Class == IDCMP_MOUSEBUTTONS) && (imsg->Code == SELECTUP)))
		{
		    DP(("GT_FIM:  Clearing active gadget\n"));
		    if (ctgad)
		    {
			CTID(ctgad)->ctid_ActiveGadget = NULL;
		    }
		}
		else if (imsg->Class == IDCMP_GADGETDOWN)
		{
		    DP(("GT_FIM:  Setting active gadget\n"));
		    if (ctgad)
		    {
			CTID(ctgad)->ctid_ActiveGadget = gad;
		    }
		}
	    }
	    DP(("GT_FIM: Calling handler at $%lx\n", SGAD(gad)->sg_EventHandler));
	    /* Handler returns TRUE if the message is to be passed down
	     * to the client, FALSE if it is to be eaten.  msgforclient
	     *	is this value:
	     */
	    msgforclient = (*SGAD(gad)->sg_EventHandler)(gad, (struct IntuiMessage *)quasi);
	    DP(("GT_FIM: Back from handler\n"));
	}
	else if ((imsg->Class == IDCMP_MOUSEBUTTONS) && (imsg->Code == SELECTUP))
	{
	    /* This is where boolean FOLLOWMOUSE gadgets get turned
	     * off if the user rolls off them and then releases the
	     * mouse:
	     */
	    if (ctgad)
	    {
		CTID(ctgad)->ctid_ActiveGadget = NULL;
	    }
	    msgforclient = TRUE;
	}
	else
	{
	    /* A class of message that the toolkit doesn't care about,
	     * so send it down to the client:
	     */
	    msgforclient = TRUE;
	}
    }

    if (msgforclient)
    {
	MP(("GT_FIM: Message of class $%lx, IAddress $%lx  for client\n",
	    quasi->qm_IMessage.Class, quasi->qm_IMessage.IAddress));
	return((struct IntuiMessage *)quasi);
    }
    else
    {
	/* Release the QuasiMessage */
	GT_PostFilterIMsg( (struct IntuiMessage *)quasi );

	DP(("GT_FIM: Returning Empty-handed\n"));
	return(NULL);
    }
}


/*------------------------------------------------------------------------*/

/****** gadtools.library/GT_PostFilterIMsg ***********************************
*
*   NAME
*	GT_PostFilterIMsg -- return the unfiltered message after
*	                     GT_FilterIMsg() was called, and clean up. (V36)
*
*   SYNOPSIS
*	imsg = GT_PostFilterIMsg(modimsg)
*	D0                       A1
*
*	struct IntuiMessage *GT_PostFilterIMsg(struct IntuiMessage *);
*
*   FUNCTION
*	NOTE WELL:  Extremely few programs will actually need this function.
*	You almost certainly should be using GT_GetIMsg() and GT_ReplyIMsg()
*	only, and not GT_FilterIMsg() and GT_PostFilterIMsg().
*
*	Performs any clean-up necessitated by a previous call to
*	GT_FilterIMsg().  The original IntuiMessage is now yours to handle.
*	Do not interpret the fields of the original IntuiMessage, but
*	rather use only the one you got from GT_FilterIMsg().  You
*	may only do message related things at this point, such as queueing
*	it up or replying it.  Since you got the message with
*	exec.library/GetMsg(), your responsibilities do include replying
*	it with exec.library/ReplyMsg(). This function may be safely
*	called with a NULL parameter.
*
*   INPUTS
*	modimsg - a modified IntuiMessage obtained with GT_FilterIMsg(),
*	          or NULL in which case this function does nothing and
*	          returns NULL
*
*   RESULT
*	imsg - a pointer to the original IntuiMessage, if GT_FilterIMsg()
*	       returned non-NULL.
*
*   NOTES
*	Be sure to use exec.library/ReplyMsg() on the original IntuiMessage
*	you obtained with GetMsg(), (which is the what you passed to
*	GT_FilterIMsg()), and not on the parameter of this function.
*
*	Starting with V39, this function actually expects and returns
*	pointers to ExtIntuiMessage structures, but the prototype was not
*	changed for source code compatibility with older software.
*
*   SEE ALSO
*	GT_FilterIMsg()
*
******************************************************************************
*
* Created:  09-Nov-89, Peter Cherna (from SpecialReplyMsg())
* Modified: 21-Nov-89, Peter Cherna
*
*/

struct IntuiMessage * ASM LIB_GT_PostFilterIMsg(REG(a1) struct IntuiMessage *imsg)
{
struct IntuiMessage *realimsg = NULL;
struct ExtGadget    *ctgad;

    if (imsg)
    {
	/* Extract original IntuiMessage */
	realimsg = ((struct QuasiMessage *)imsg)->qm_OrigMessage;
	/* Peter 16-May-91: The way to cancel MENUVERIFY is to
	 * change the Code field to MENUCANCEL.  Therefore,
	 * we need to copy the code field back to allow Intuition
	 * to hear it.
	 */
	realimsg->Code = imsg->Code;
	if ( ctgad = ((struct QuasiMessage *)imsg)->qm_ContextGadget )
	{
	    /* Mark the embedded QuasiMessage as not in use */
	    DP((" GT_PFIM:  Marking embedded quasi as unused\n"));
	    CTID(ctgad)->ctid_QuasiUsed = 0;
	    /* DelayedFree means that FreeGadgets() was called while
	     * a modified IntuiMessage was still unreplied.  The
	     * context gadget must be freed here and now.
	     */
	    if (CTID(ctgad)->ctid_DelayedFree)
	    {
		FreeVec(ctgad);
	    }
	}
	else
	{
	    /* This was an allocated QuasiMessage, so free it */
	    DP((" GT_PFIM:  FreeVec()ing quasi at $%lx\n", imsg));
	    FreeVec(imsg);
	}
    }

    return(realimsg);
}


/*------------------------------------------------------------------------*/

/****** gadtools.library/CreateContext ***************************************
*
*   NAME
*	CreateContext -- create a place for GadTools context data. (V36)
*
*   SYNOPSIS
*	gad = CreateContext(glistpointer);
*	D0                  A0
*
*	struct Gadget *CreateContext(struct Gadget **);
*
*   FUNCTION
*	Creates a place for GadTools to store any context data it might
*	need for your window.  In reality, an unselectable invisible
*	gadget is created, with room for the context data.
*	This function also establishes the linkage from a glist type
*	pointer to the individual gadget pointers.  Call this function
*	before any of the other gadget creation calls.
*
*   INPUTS
*	glistptr - address of a pointer to a Gadget, which was previously
*	           set to NULL.  When all the gadget creation is done, you may
*	           use that pointer as your NewWindow.FirstGadget, or
*	           in intuition.library/AddGList(),
*	           intuition.library/RefreshGList(), FreeGadgets(), etc.
*
*   RESULT
*	gad - pointer to context gadget, or NULL if failure.
*
*   EXAMPLE
*
*	struct Gadget *gad;
*	struct Gadget *glist = NULL;
*	gad = CreateContext(&glist);
*	\*  Other creation calls go here *\
*	if (gad)
*	{
*	    myNewWindow.FirstGadget = glist;
*	    if ( myWindow = OpenWindow(&myNewWindow) )
*	    {
*		GT_RefreshWindow(win,NULL);
*		\* other stuff *\
*		CloseWindow(myWindow);
*	    }
*	}
*	FreeGadgets(glist);
*
******************************************************************************
*
* Created:  08-Nov-89, Peter Cherna
* Modified: 20-Nov-89, Peter Cherna
*
*/

struct ExtGadget * ASM LIB_CreateContext(REG(a0) struct ExtGadget **glistptr)
{
struct ExtGadget *gad;
struct NewGadget  ng;

    if (glistptr)
    {
        memset(&ng, NULL, sizeof(struct NewGadget));

        /* !!! CreateGadgetA() requires a non-zero VisualInfo.  However, the
         * GENERIC_KIND gadget (when ng_GadgetText is NULL) MUST work
         * without referring to the VisualInfo if we expect to create
         * the context gadget with it.  So to fool it, we set a bogus
         * but non-zero value of VisualInfo, knowing it won't be needed
         * on the other side, but just to fool the middle-man !!!
         */
        ng.ng_VisualInfo = IGNORE_VISUALINFO;

        if (gad = CreateGenericBase((struct ExtGadget *)glistptr,&ng,CONTEXT_IDATA_SIZE,NULL))
        {
           gad->Flags = GFLG_GADGHNONE;
           SGAD(gad)->sg_Flags = SG_CONTEXT;
           return(gad);
        }
    }

    return(NULL);
}

/*------------------------------------------------------------------------*/

/****** gadtools.library/GT_SetGadgetAttrsA **********************************
*
*   NAME
*	GT_SetGadgetAttrsA -- change the attributes of a GadTools gadget. (V36)
*	GT_SetGadgetAttrs -- varargs stub for GT_SetGadgetAttrsA(). (V36)
*
*   SYNOPSIS
*	GT_SetGadgetAttrsA(gad, win, req, tagList)
*	                   A0   A1   A2   A3
*
*	VOID GT_SetGadgetAttrsA(struct Gadget *, struct Window *,
*	                        struct Requester *, struct TagItem *);
*
*	GT_SetGadgetAttrs(gad, win, req, firsttag, ...)
*
*	VOID GT_SetGadgetAttrs(struct Gadget *, struct Window *,
*	                       struct Requester *, Tag, ...);
*
*   FUNCTION
*	Change the attributes of the specified gadget, according to the
*	attributes chosen in the tag list. If an attribute is not provided
*	in the tag list, its value remains unchanged.
*
*   INPUTS
*	gad - pointer to the gadget in question. Starting with V39, this
*	      value may be NULL in which case this function does nothing
*	win - pointer to the window containing the gadget. Starting with V39,
*	      this value may be NULL in which case the internal attributes of
*	      the gadgets are altered but no rendering occurs.
*	req - reserved for future use, should always be NULL
*	tagList - pointer to an array of tags providing optional extra
*		  parameters, or NULL.
*
*   TAGS
*	BUTTON_KIND:
*	GA_Disabled (BOOL) - Set to TRUE to disable gadget, FALSE otherwise.
*	    (V36)
*
*	CHECKBOX_KIND:
*	GA_Disabled (BOOL) - Set to TRUE to disable gadget, FALSE otherwise.
*	    (V36)
*	GTCB_Checked (BOOL) - State of checkbox. (V36)
*
*	CYCLE_KIND:
*	GA_Disabled (BOOL) - Set to TRUE to disable gadget, FALSE otherwise
*	    (V37)
*	GTCY_Active (UWORD) - The ordinal number (counting from zero) of
*	    the active choice of a cycle gadget. (V36)
*	GTCY_Labels (STRPTR *) - Pointer to NULL-terminated array of strings
*	    that are the choices offered by the cycle gadget. (V37)
*
*	INTEGER_KIND:
*	GA_Disabled (BOOL) - Set to TRUE to disable gadget, FALSE otherwise
*	    (V36)
*	GTIN_Number (ULONG) - New value of the integer gadget (V36)
*
*	LISTVIEW_KIND:
*	GA_Disabled (BOOL) - Set to TRUE to disable gadget, FALSE otherwise
*	    (V39)
*	GTLV_Top (WORD) - Top item visible in the listview.  This value
*	    will be made reasonable if out-of-range. (V36)
*	GTLV_MakeVisible (WORD) - Number of an item that should be forced
*	    within the visible area of the listview by doing minimal scrolling.
*	    This tag overrides GTLV_Top. (V39)
*	GTLV_Labels (struct List *) - List of nodes whose ln_Name fields
*	    are to be displayed in the listview.  Use a value of ~0 to
*	    "detach" your List from the display.  You must detach your list
*	    before modifying the List structure, since GadTools reserves
*	    the right to traverse it on another task's schedule.  When you
*	    are done, attach the list by using the tag pair
*	    {GTLV_Labels, list}. (V36)
*	GTLV_Selected (UWORD) - Ordinal number of currently selected
*	    item. Starting with V39, you can provide ~0 to cause the currently
*	    selected item to be deselected. (V36)
*
*	MX_KIND:
*	GA_Disabled (BOOL) - Set to TRUE to disable gadget, FALSE otherwise
*	    (V39)
*	GTMX_Active (UWORD) - The ordinal number (counting from zero) of
*	    the active choice of an mx gadget. (V36)
*
*	NUMBER_KIND:
*	GTNM_Number (LONG) - A signed long integer to be displayed in
*	    the number gadget. (V36)
*	GTNM_FrontPen (UBYTE) - The pen to use when rendering the number. (V39)
*	GTNM_BackPen (UBYTE) - The pen to use when rendering the background
*	    of the number. (V39)
*	GTNM_Justification (UBYTE) - Determines how the number is rendered
*	    within the gadget box. GTJ_LEFT will make the rendering be
*	    flush with the left side of the gadget, GTJ_RIGHT will make it
*	    flush with the right side, and GTJ_CENTER will center the number
*	    within the gadget box. (V39)
*	GTNM_Format (STRPTR) - C-Style formatting string to apply on the number
*	    before display. Be sure to use the 'l' (long) modifier. This string
*	    is processed using exec.library/RawDoFmt(), so refer to that
*	    function for details. (V39)
*
*	PALETTE_KIND:
*	GA_Disabled (BOOL) - Set to TRUE to disable gadget, FALSE otherwise
*	    (V36)
*	GTPA_Color (UBYTE) - Selected color of the palette. This
*	    number is a pen number, and not the ordinal color number within
*	    the palette gadget itself. (V36)
*	GTPA_ColorOffset (UBYTE) - First color to use in palette (V39)
*	GTPA_ColorTable (UBYTE *) - Pointer to a table of pen numbers
*	    indicating  which colors should be used and edited by the palette
*	    gadget. This array must contain as many entries as there are
*	    colors displayed in the palette gadget. The array provided with
*	    this tag must remain valid for the life of the gadget or until a
*	    new table is provided. A NULL table pointer causes a 1-to-1
*	    mapping of pen numbers. (V39)
*
*	SCROLLER_KIND:
*	GA_Disabled (BOOL) - Set to TRUE to disable gadget, FALSE otherwise
*	    (V36)
*	GTSC_Top (WORD) - Top visible in scroller. (V36)
*	GTSC_Total (WORD) - Total in scroller area. (V36)
*	GTSC_Visible (WORD) - Number visible in scroller. (V36)
*
*	SLIDER_KIND:
*	GA_Disabled (BOOL) - Set to TRUE to disable gadget, FALSE otherwise
*	    (V36)
*	GTSL_Min (WORD) - Minimum level for slider. (V36)
*	GTSL_Max (WORD) - Maximum level for slider. (V36)
*	GTSL_Level (WORD) - Current level of slider. (V36)
*	GTSL_LevelFormat (STRPTR) - C-Style formatting string for slider
*	    level.  Be sure to use the 'l' (long) modifier.  This string
*	    is processed using exec.library/RawDoFmt(), so refer to that
*	    function for details. (V39)
*	GTSL_DispFunc ( LONG (*function)(struct Gadget *, WORD) ) - Function
*	    to calculate level to be displayed.  A number-of-colors slider
*	    might want to set the slider up to think depth, and have a
*	    (1 << n) function here.  Your function must
*	    take a pointer to gadget as the first parameter, the level
*	    (a WORD) as the second, and return the result as a LONG. (V39)
*	GTSL_Justification (UBYTE) - Determines how the level display is to
*	    be justified within its alotted space. Choose one of GTJ_LEFT,
*	    GTJ_RIGHT, or GTJ_CENTER. (V39)
*
*	STRING_KIND:
*	GA_Disabled (BOOL) - Set to TRUE to disable gadget, FALSE otherwise
*	    (V36)
*	GTST_String (STRPTR) - New contents of the string gadget,
*	    or NULL to reset the gadget to the empty state. (V36)
*
*	TEXT_KIND:
*	GTTX_Text - New NULL terminated string to be displayed in the text
*	    gadget. NULL means to clear the gadget. (V36)
*	GTTX_FrontPen (UBYTE) - The pen to use when rendering the text. (V39)
*	GTTX_BackPen (UBYTE) - The pen to use when rendering the background
*	    of the text. (V39)
*	GTTX_Justification (UBYTE) - Determines how the text is rendered
*	    within the gadget box. GTJ_LEFT will make the rendering be
*	    flush with the left side of the gadget, GTJ_RIGHT will make it
*	    flush with the right side, and GTJ_CENTER will center the text
*	    within the gadget box. (V39)
*
*   NOTES
*	This function may not be called inside of a GT_BeginRefresh() /
*	GT_EndRefresh() session.  (As always, restrict yourself to simple
*	rendering functions).
*
*   SEE ALSO
*	GT_GetGadgetAttrs()
*
******************************************************************************
*
* Created:   3-Dec-89, Peter Cherna
* Modified:  5-Dec-89, Peter Cherna
*
* NOTES
*	Since the requester parameter is not currently used, the actual
*	function does not define this parameter which makes the code
*	smaller.
*
*/

void __asm
LIB_GT_SetGadgetAttrsA( register __a0 struct ExtGadget *gad,
		        register __a1 struct Window *win,
		        register __a3 struct TagItem *taglist )
{
    DP(("GT_SGA:  Enter\n"));

    /* Is this a Toolkit gadget? */
    if (gad && gad->GadgetType & GADTOOL_TYPE)
    {
	DP(("GT_SGA:  Got a toolkit gadget\n"));
	/* Does it have a SetAttrs function? */
	if (SGAD(gad)->sg_SetAttrs)
	{
	    DP(("SetAttrs fcn at $%lx\n", SGAD(gad)->sg_SetAttrs));
	    /* Do it to it: */
	    (*SGAD(gad)->sg_SetAttrs)(gad, win, taglist);
	}
    }
    DP(("GT_SGA:  Exit\n"));
}


/*------------------------------------------------------------------------*/

/*/ FindContext()
 *
 * Returns pointer to the Context Instance Data, or NULL
 *
 * Created:  08-Nov-89, Peter Cherna
 * Modified:  8-Dec-89, Peter Cherna
 *
 */

static struct ExtGadget *FindContext(struct Window *win)
{
struct ExtGadget *gad;

    MP(("FC:  Enter for win $%lx\n", win));

    gad = (struct ExtGadget *)win->FirstGadget;
    while (gad)
    {
	if ((gad->GadgetType & GADTOOL_TYPE) && (SGAD(gad)->sg_Flags & SG_CONTEXT))
	    return(gad);

	gad = (struct ExtGadget *)gad->NextGadget;
    }

    return(NULL);
}


/*------------------------------------------------------------------------*/

/****** gadtools.library/GT_GetGadgetAttrsA **********************************
*
*   NAME
*	GT_GetGadgetAttrsA -- request the attributes of a GadTools gadget. (V39)
*	GT_GetGadgetAttrs -- varargs stub for GT_GetGadgetAttrsA(). (V39)
*
*   SYNOPSIS
*	numProcessed = GT_GetGadgetAttrsA(gad, win, req, taglist)
*	                                  A0   A1   A2   A3
*
*	LONG GT_GetGadgetAttrsA(struct Gadget *, struct Window *,
*	                        struct Requester *, struct TagItem *);
*
*	numProcessed = GT_GetGadgetAttrs(gad, win, req, firsttag, ...)
*
*	LONG GT_GetGadgetAttrs(struct Gadget *, struct Window *,
*                              struct Requester *, Tag, ...);
*
*   FUNCTION
*	Retrieve the attributes of the specified gadget, according to the
*	attributes chosen in the tag list.  For each entry in the tag list,
*	ti_Tag identifies the attribute, and ti_Data is a pointer to
*	the long variable where you wish the result to be stored.
*
*   INPUTS
*	gad - pointer to the gadget in question. May be NULL, in which case
*	      this function returns 0
*	win - pointer to the window containing the gadget.
*	req - reserved for future use, should always be NULL
*	taglist - pointer to TagItem list.
*
*   TAGS
*	BUTTON_KIND:
*	GA_Disabled (BOOL) - TRUE if this gadget is disabled,
*	    FALSE otherwise. (V39)
*
*	CHECKBOX_KIND:
*	GA_Disabled (BOOL) - TRUE if this gadget is disabled,
*	    FALSE otherwise. (V39)
*	GTCB_Checked (BOOL) - TRUE if this gadget is currently checked,
*	    FALSE otherwise. (V39)
*
*	CYCLE_KIND:
*	GA_Disabled (BOOL) - TRUE if this gadget is disabled,
*	    FALSE otherwise. (V39)
*	GTCY_Active (UWORD) - The ordinal number (counting from zero) of
*	    the active choice of a cycle gadget. (V39)
*	GTCY_Labels (STRPTR *) - The NULL-terminated array of strings
*	    that are the choices offered by the cycle gadget. (V39)
*
*	INTEGER_KIND:
*	GA_Disabled (BOOL) - TRUE if this gadget is disabled,
*	    FALSE otherwise. (V39)
*	GTIN_Number (ULONG) - The contents of the integer gadget. (V39)
*
*	LISTVIEW_KIND:
*	GA_Disabled (BOOL) - TRUE if this gadget is disabled,
*	    FALSE otherwise. (V39)
*	GTLV_Top (WORD) - Ordinal number of the top item visible
*	    in the listview. (V39)
*	GTLV_Labels (struct List *) - The list of nodes whose ln_Name fields
*	    are displayed in the listview. (V39)
*	GTLV_Selected (UWORD) - Ordinal number of currently selected
*	    item. Returns ~0 if no item is selected. (V39)
*
*	MX_KIND:
*	GA_Disabled (BOOL) - TRUE if this gadget is disabled,
*	    FALSE otherwise. (V39)
*	GTMX_Active (UWORD) - The ordinal number (counting from zero) of
*	    the active choice of an mx gadget. (V39)
*
*	NUMBER_KIND:
*	GTNM_Number - The signed long integer that is displayed in
*	    the read-only number. (V39)
*
*	PALETTE_KIND:
*	GA_Disabled (BOOL) - TRUE if this gadget is disabled,
*	    FALSE otherwise. (V39)
*	GTPA_Color (UBYTE) - The selected color of the palette. (V39)
*	GTPA_ColorOffset (UBYTE) - First color used in palette. (V39)
*	GTPA_ColorTable (UBYTE *) - Pointer to a table of pen numbers
*	    indicating  which colors should be used and edited by the palette
*	    gadget. May be NULL, which causes a 1-to-1 mapping of pen numbers.
*	    (V39)
*
*	SCROLLER_KIND:
*	GA_Disabled (BOOL) - TRUE if this gadget is disabled,
*	    FALSE otherwise. (V39)
*	GTSC_Top (WORD) - Top visible in scroller. (V39)
*	GTSC_Total (WORD) - Total in scroller area. (V39)
*	GTSC_Visible (WORD) - Number visible in scroller. (V39)
*
*	SLIDER_KIND:
*	GA_Disabled (BOOL) - TRUE if this gadget is disabled,
*	    FALSE otherwise. (V39)
*	GTSL_Min (WORD) - Minimum level for slider. (V39)
*	GTSL_Max (WORD) - Maximum level for slider. (V39)
*	GTSL_Level (WORD) - Current level of slider. (V39)
*
*	STRING_KIND:
*	GA_Disabled (BOOL) - TRUE if this gadget is disabled,
*	    FALSE otherwise. (V39)
*	GTST_String (STRPTR) - Returns a pointer to the string gadget's
*	    buffer. (V39)
*
*	TEXT_KIND:
*	GTTX_Text - Pointer to the string to be displayed in the
*	    read-only text-display gadget. (V39)
*
*   RESULT
*	numProcessed - the number of attributes successfully filled in.
*
*   EXAMPLE
*		long top = 0;
*		long selected = 0;
*		long result;
*		result = GT_GetGadgetAttrs( listview_gad, win, NULL,
*			GTLV_Top, &top,
*			GTLV_Selected, &selected,
*			TAG_DONE );
*		if ( result != 2 )
*		{
*			printf( "Something's wrong!" );
*		}
*
*   WARNING
*	The pointers you provide within the tag list to store the return
*	values MUST POINT TO LONGWORDS. That is, even if the type of a
*	return value is defined as (UWORD *), you must pass a pointer to
*	a longword of memory.
*
*   SEE ALSO
*	GT_SetGadgetAttrs()
*
******************************************************************************
*
* NOTES
*	Since the requester parameter is not currently used, the actual
*	function does not define this parameter which makes the code
*	smaller.
*
*/

LONG __asm
LIB_GT_GetGadgetAttrsA( register __a0 struct ExtGadget *gad,
		        register __a1 struct Window *win,
		        register __a3 struct TagItem *taglist )
{
    LONG result = 0;
    struct GetTable *table, *entry;
    struct TagItem *tstate = taglist;
    struct TagItem *ti;
    LONG attr;
    LONG done;

    DP(("GT_GGA:  Enter\n"));
    /* Is this a Toolkit gadget? */
    if (gad && gad->GadgetType & GADTOOL_TYPE)
    {
	/* Does it have a GetTable? */
	if ( table = SGAD(gad)->sg_GetTable )
	{
	    while ( ti = NextTagItem( &tstate ) )
	    {
		DP(("TagItem = {%lx,%lx}\n", ti->ti_Tag, ti->ti_Data));
		if ( ti->ti_Tag == GA_Disabled )
		{
		    DP(("  This is GA_Disabled!\n"));
		    attr = ATTR_DISABLED;
		}
		else
		{
		    attr = ti->ti_Tag - ((ULONG)GT_TagBase);
		}
		if ( attr > 0 )
		{
		    DP(("  attr = %lx, table at %lx\n", attr, table));
		    entry = table;
		    done = FALSE;
		    while ( !done )
		    {
			DP(("    entry %lx = (%lx,%lx)\n", entry, entry->gtab_tag, entry->gtab_descriptor));
			if ( attr == entry->gtab_tag )
			{
			    LONG answer, offset;

			    offset = REAL_OFFSET(entry->gtab_descriptor);
			    if ( attr == ATTR_DISABLED )
			    {
				struct ExtGadget *checkme = gad;
				DP(("    ** gadget = %lx\n", checkme ));
				if ( entry->gtab_descriptor )
				{
				    checkme = *((struct ExtGadget **) (((UBYTE *) gad) + offset));
				    DP(("    ** But checking gadget = %lx\n", checkme ));
				}
				answer = FALSE;
				if ( checkme->Flags & GFLG_DISABLED )
				{
				    answer = TRUE;
				}
			    }
			    else
			    {
				if ( entry->gtab_descriptor & LONG_ATTR )
				{
				    answer = *((LONG *) (((UBYTE *) gad) + offset));
				}
				else
				{
				    answer = *((WORD *) (((UBYTE *) gad) + offset));
				}
			    }
			    *((LONG *)ti->ti_Data) = answer;
			    result++;
			    DP(("    ** Matched: answer = %ld, result = %ld\n", answer,result));
			    done = TRUE;
			}
			/* Terminate on an ATTR_DISABLED or ATTR_END entry */
			if ( entry->gtab_tag >= ATTR_DISABLED )
			{
			    done = TRUE;
			}
			entry++;
		    }
		}
		D(else kprintf("Not GT: attr = %lx\n", attr ));
	    }
	}
    }
    DP(("GT_GGA:  Exit\n"));
    return( result );
}

/*------------------------------------------------------------------------*/

/* NOTE VERY WELL:
 *
 * The GETTABLE_ATTR() macro is designed to FAIL with a compiler error
 * should the offset in the instance data be too great to encode
 * in the table-format.  If this happens, just juggle the parameters
 * to nearer the top of the instance data.
 *
 * The current offset range is 0x7F words into the instance data,
 * so that should be good enough for a while.
 */
struct GetTable Button_GetTable[] =
{
    GETTABLE_DISABLED
};

struct GetTable Checkbox_GetTable[] =
{
    GETTABLE_ATTR(GTCB_Checked,	CBID, cbid_Checked,		WORD_ATTR),
    GETTABLE_DISABLED
};

struct GetTable Integer_GetTable[] =
{
    GETTABLE_ATTR(GTIN_Number,	STID, stid_StringInfo.LongInt,	LONG_ATTR),
    GETTABLE_DISABLED
};

struct GetTable Listview_GetTable[] =
{
    GETTABLE_ATTR(GTLV_Labels,	LVID, lvid_Labels,		LONG_ATTR),
    GETTABLE_ATTR(GTLV_Top,	LVID, lvid_Top,			WORD_ATTR),
    GETTABLE_ATTR(GTLV_Selected,LVID, lvid_Selected,		WORD_ATTR),
    GETTABLE_DISABLED_MEMBER(LVID, lvid_ListGad)
};

struct GetTable MX_GetTable[] =
{
    GETTABLE_ATTR(GTMX_Active,	MXID, mxid_Active,		WORD_ATTR),
    GETTABLE_DISABLED_MEMBER(MXID, mxid_FirstGadget)
};

struct GetTable Number_GetTable[] =
{
    GETTABLE_ATTR(GTNM_Number,	      NMID, nmid_Number,	 LONG_ATTR),
    GETTABLE_END
};

struct GetTable Cycle_GetTable[] =
{
    GETTABLE_ATTR(GTCY_Labels,	CYID, cyid_Labels,		LONG_ATTR),
    GETTABLE_ATTR(GTCY_Active,	CYID, cyid_Active,		WORD_ATTR),
    GETTABLE_DISABLED
};

struct GetTable Palette_GetTable[] =
{
    GETTABLE_ATTR(GTPA_Color,	    PAID, paid_Color,		WORD_ATTR),
    GETTABLE_ATTR(GTPA_ColorOffset, PAID, paid_ColorOffset,	WORD_ATTR),
    GETTABLE_ATTR(GTPA_ColorTable,  PAID, paid_ColorTable,	LONG_ATTR),
    GETTABLE_DISABLED
};

struct GetTable Scroller_GetTable[] =
{
    GETTABLE_ATTR(GTSC_Top,	SCID, scid_Top,			WORD_ATTR),
    GETTABLE_ATTR(GTSC_Total,	SCID, scid_Total,		WORD_ATTR),
    GETTABLE_ATTR(GTSC_Visible,	SCID, scid_Visible,		WORD_ATTR),
    GETTABLE_DISABLED_MEMBER(SCID, scid_Prop)
};

struct GetTable Slider_GetTable[] =
{
    GETTABLE_ATTR(GTSL_Min,	SLID, slid_Min,			WORD_ATTR),
    GETTABLE_ATTR(GTSL_Max,	SLID, slid_Max,			WORD_ATTR),
    GETTABLE_ATTR(GTSL_Level,	SLID, slid_Level,		WORD_ATTR),
    GETTABLE_DISABLED_MEMBER(SLID, slid_Prop)
};

struct GetTable String_GetTable[] =
{
    GETTABLE_ATTR(GTST_String,	STID, stid_StringInfo.Buffer,	LONG_ATTR),
    GETTABLE_DISABLED
};

struct GetTable Text_GetTable[] =
{
    GETTABLE_ATTR(GTTX_Text,	      TXID, txid_IText.IText,	 LONG_ATTR),
    GETTABLE_END
};

/*------------------------------------------------------------------------*/

