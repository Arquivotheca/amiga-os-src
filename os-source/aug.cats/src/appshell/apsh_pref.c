/* apsh_pref.c
 * Copyright (C) 1990 Commodore-Amiga, Inc.
 * written by David N. Junod
 *
 * control module for preferences.
 *
 */

#include "appshell_internal.h"
#include "dbugmac.h"

void kprintf (void *,...);

#define	DB(x)	;
#define	DP(x)	;
#define	DS(x)	;
#define	DF(x)	;
#define DOE(x)  ;
#define DPS(x)	x

/* Preference message handling routines */
BOOL open_pref (struct AppInfo *, struct MsgHandler *, struct TagItem *);
BOOL handle_pref (struct AppInfo *, struct MsgHandler *, struct TagItem *);
BOOL close_pref (struct AppInfo *, struct MsgHandler *, struct TagItem *);
BOOL shutdown_pref (struct AppInfo *, struct MsgHandler *, struct TagItem *);
struct MsgHandler *setup_prefA (struct AppInfo *, struct TagItem *);
VOID FreshenScreenMode (struct AppInfo * ai, struct ScreenModePref *);
VOID FreshenWindow (struct AppInfo * ai, struct TagItem *);
VOID FreshenPalette (struct AppInfo * ai, struct PalettePref *);
VOID FreshenPointer (struct AppInfo * ai, struct PointerPref *);
VOID FreshenUser (struct AppInfo * ai, struct Prefs *);
VOID KPT (char *, struct TagItem *);

/****** appshell.library/setup_notifyA ****************************************
*
*   NAME
*	setup_notifyA - Initializes the notification message handler.
*
*   SYNOPSIS
*	mh = setup_notifyA (ai, attrs)
*	D0                  D0  D1
*
*	struct MsgHandler *mh;
*	struct AppInfo *ai;
*	struct TagItem *attrs;
*
*   FUNCTION
*	This is the low-level function used to initialize the file
*	notification message handler.  This handler is responsible for
*	dispatching events when marked files are modified.
*
*	NOTE: This call should not be called directly by the application,
*	but is automatically initialized by all AppShell applications.
*
*	Valid tagitems to use at OPEN time:
*
*	    APSH_NameTag, <file name>
*	    Complete file name of file to watch.
*
*	    APSH_CmdID, <function ID>
*	    Function to execute when file is modified.  Examine APSH_NameTag
*	    to get the name of the file modified.
*
*   NOTES
*	This handler is used by the AppShell to automatically handle preference
*	files, with notification, for the application.
*
*	When the application closes, all notification events will be
*	automatically terminated.
*
*   EXAMPLE
*
*	\* Start notification on a file *\
*	VOID WatchClipFile (struct AppInfo *ai, STRPTR cmd, struct TagItem *attrs)
*	{
*	    \* Start the notification *\
*	    HandlerFunc (ai,
*			 APSH_Handler,	"NOTIFY",
*			 APSH_Command,	APSH_MH_OPEN,
*			 APSH_NameTag,	"ram:temp",
*			 APSH_CmdID,	ClipFileChangeID,
*			 TAG_DONE);
*	}
*
*   INPUTS
*	ai	- Pointer to the application's AppInfo structure.
*	attrs	- Pointer to an array of TagItem attributes for the
*		  function.
*
*   RESULT
*	mh	- Pointer to a MsgHandler structure.
*
*   SEE ALSO
*
*********************************************************************
*
* Created:  18-Mar-90, David N. Junod
*
*/

struct MsgHandler *setup_prefA (struct AppInfo * ai, struct TagItem * tl)
{
    register struct MsgHandler *mh;
    register struct MHObject *mho;
    ULONG msize;

    DBUG_ENTER("setup_prefA");

    /* calculate size of memory block */
    msize = sizeof (struct MsgHandler) +
      (4L * sizeof (ULONG));

    /* allocate the memory block */
    if (mh = (struct MsgHandler *) AllocVec (msize, MEMF_CLEAR | MEMF_PUBLIC))
    {
	/* get a pointer to the embedded object header */
	mho = &(mh->mh_Header);

	/* initialize the object header */
	mho->mho_Node.ln_Type = APSH_MH_HANDLER_T;
	mho->mho_Node.ln_Pri = APSH_MH_HANDLER_P;
	mho->mho_Node.ln_Name = "NOTIFY";

	/* initialize the object list */
	NewList (&(mho->mho_ObjList));

	/* set up the ID */
	mho->mho_ID = APSH_MAIN_ID;

	/* Initialize the preference list */
	NewList (&(ai->ai_PrefList));

	/* number of functions in handler */
	mh->mh_NumFuncs = 4;
	mh->mh_Func = MEMORY_FOLLOWING (mh);
	mh->mh_Func[APSH_MH_OPEN] = open_pref;
	mh->mh_Func[APSH_MH_HANDLE] = handle_pref;
	mh->mh_Func[APSH_MH_CLOSE] = close_pref;
	mh->mh_Func[APSH_MH_SHUTDOWN] = shutdown_pref;

	/* Create the port for notification */
	if (mh->mh_Port = CreatePort (NULL, NULL))
	{
	    struct ScreenModePref * smp;
	    struct TagItem attrs[5];

	    /* Remember the signal bits */
	    mh->mh_SigBits = (1L << mh->mh_Port->mp_SigBit);

	    /* Set up the tag attributes */
	    attrs[0].ti_Tag = PREFS_NOTIFY_A;
	    attrs[0].ti_Data = (LONG) mh->mh_Port;
	    attrs[1].ti_Tag = PREFS_CALLBACK_A;
	    attrs[2].ti_Tag = PREFS_LIST_A;
	    attrs[2].ti_Data = (LONG) & (ai->ai_PrefList);
	    attrs[3].ti_Tag = TAG_IGNORE;
	    attrs[4].ti_Tag = TAG_DONE;

	    /* Get the default pointer */
	    attrs[1].ti_Data = (LONG) FreshenPointer;
	    GetPref (PREFS_POINTER, ai->ai_ConfigDir, attrs);

	    /* Get the busy pointer */
	    GetPref (PREFS_BUSYPOINTER, ai->ai_ConfigDir, attrs);

	    /* Get the screen mode preferences */
	    attrs[1].ti_Data = (LONG) FreshenScreenMode;
	    if (smp = GetPref (PREFS_SCREENMODE, ai->ai_ConfigDir, attrs))
	    {
		attrs[3].ti_Tag = PREFS_DEPTH;
		attrs[3].ti_Data = smp->smp_Depth;
		DF (kprintf ("init palette: depth %ld\n", (LONG) smp->smp_Depth));
	    }

	    /* Get the color palette */
	    attrs[1].ti_Data = (LONG) FreshenPalette;
	    GetPref (PREFS_PALETTE, ai->ai_ConfigDir, attrs);

	    DBUG_RETURN(mh);
	}
	else
	{
	    ai->ai_Pri_Ret = RETURN_FAIL;
	    ai->ai_Sec_Ret = APSH_CLDNT_CREATE_PORT;
	    ai->ai_TextRtn = PrepText (ai, APSH_MAIN_ID, ai->ai_Sec_Ret, NULL);
	}

	FreeVec ((APTR) mh);
	mh = NULL;
    }
    else
    {
	ai->ai_Pri_Ret = RETURN_FAIL;
	ai->ai_Sec_Ret = APSH_NOT_ENOUGH_MEMORY;
	ai->ai_TextRtn = PrepText (ai, APSH_MAIN_ID, ai->ai_Sec_Ret, NULL);
    }

    DBUG_RETURN(mh);
}

BOOL open_pref (struct AppInfo * ai, struct MsgHandler * mh, struct TagItem * tl)
{
    BOOL retval = FALSE;
    STRPTR name;
    LONG FuncID;

    DBUG_ENTER("open_pref");

    /* Get the name and function */
    if ((name = (STRPTR) GetTagData (APSH_NameTag, NULL, tl)) &&
	(FuncID = GetTagData (APSH_CmdID, NULL, tl)))
    {
	struct List *list = &(ai->ai_PrefList);

	/* Make sure it doesn't already exist */
	if (!(FindNameI (list, name)))
	{
	    struct NotifyRequest *nr;
	    struct Prefs *p = NULL;
	    LONG msize;

	    /* How much memory do we need */
	    msize = sizeof (struct Prefs) + strlen (name) + 1L;

	    /* Allocate the record */
	    if (p = (struct Prefs *) AllocVec (msize, MEMF_CLEAR))
	    {
		/* Copy the name */
		p->p_Node.ln_Name = MEMORY_FOLLOWING (p);
		strcpy (p->p_Node.ln_Name, name);

		/* Set the callback function */
		p->p_Func = FreshenUser;
		p->p_Kind = FuncID;

		/* Calculate the size of the notification request */
		msize = sizeof (struct NotifyRequest) + strlen (name) + 1L;

		/* Allocate a notification request */
		if (nr = (struct NotifyRequest *) AllocVec (msize, MEMF_CLEAR))
		{
		    BOOL stat;

		    /* Remember the newly allocate NotifyRequest struct */
		    p->p_NR = nr;

		    /* Copy the name */
		    nr->nr_Name = MEMORY_FOLLOWING (nr);
		    strcpy (nr->nr_Name, name);

		    /* Set up the UserData to point back at ourselves */
		    nr->nr_UserData = (ULONG) p;

		    /* Set the appropriate flags */
		    nr->nr_Flags = NRF_SEND_MESSAGE | NRF_WAIT_REPLY;

		    /* Set the message port */
		    nr->nr_stuff.nr_Msg.nr_Port = mh->mh_Port;

		    /* Start the notification */
		    stat = StartNotify (nr);

		}		/* Allocate notification request */

		AddTail (list, (struct Node *) p);
		retval = TRUE;
	    }
	}
    }

    DBUG_RETURN(retval);
}

BOOL handle_pref (struct AppInfo * ai, struct MsgHandler * mh, struct TagItem * tl)
{
    struct NotifyMessage *msg;

    DBUG_ENTER("handle_pref");

    while (msg = (struct NotifyMessage *) GetMsg (mh->mh_Port))
    {
	struct NotifyRequest *nr;
	struct Prefs *p;

	nr = msg->nm_NReq;
	p = &(((struct ScreenModePref *) nr->nr_UserData)->smp_Header);

	(*(p->p_Func)) ((VOID *) ai, (VOID *) nr->nr_UserData);

	ReplyMsg ((struct Message *) msg);
    };

    DBUG_RETURN(TRUE);
}

BOOL close_pref (struct AppInfo * ai, struct MsgHandler * mh, struct TagItem * tl)
{
    BOOL retval = FALSE;
    struct Prefs *p;
    STRPTR name;

    DBUG_ENTER("close_pref");

    /* Get the name and function */
    if (name = (STRPTR) GetTagData (APSH_NameTag, NULL, tl))
    {
	struct List *list = &(ai->ai_PrefList);

	/* Make sure it doesn't already exist */
	if (p = (struct Prefs *) FindNameI (list, name))
	{
	    /* Remove the name from the list */
	    Remove ((struct Node *) p);

	    /* Free the notification, etc. */
	    FreePref (p);

	    /* Indicate success */
	    retval = TRUE;
	}
    }

    DBUG_RETURN(retval);
}

BOOL shutdown_pref (struct AppInfo * ai, struct MsgHandler * mh, struct TagItem * tl)
{
    struct MHObject *mho;

    DBUG_ENTER("shutdown_pref");

    if (mh && (mho = &(mh->mh_Header)))
    {
	/* Remove the node */
	Remove ((struct Node *) mh);

	/* Free the list of preference records */
	FreePrefList (&(ai->ai_PrefList));

	/* Delete the message port */
	if (mh->mh_Port)
	{
	    DeletePort (mh->mh_Port);
	}

	/* Free the memory */
	FreeVec ((APTR) mh);
    }
    DBUG_RETURN(TRUE);
}

/* A user notification request */
VOID FreshenUser (struct AppInfo * ai, struct Prefs * p)
{
    DBUG_ENTER("FreshenUser");

    if (p)
    {
	LONG FuncID = p->p_Kind;

	if (FuncID != NO_FUNCTION)
	{
	    struct TagItem attrs[2];

	    attrs[0].ti_Tag = APSH_NameTag;
	    attrs[0].ti_Data = (LONG) p->p_NR->nr_FullName;
	    attrs[1].ti_Tag = TAG_DONE;

	    PerfFunc (ai, FuncID, NULL, attrs);
	}
    }

    DBUG_VOID_RETURN;
}

BOOL lockscreen (struct AppInfo * ai, struct IDCMPInfo * md, STRPTR name)
{
	DBUG_ENTER("lockscreen");

    /* Get a lock on the screen */
    DPS(kprintf("LS\tName[%s]\n",name));
    if (md->ii_PubScreen = LockPubScreen (name))
    {

    DPS(kprintf("LS\tPubScreen Addr[%ld]\n",
    			md->ii_PubScreen
    			));

	/* Set the appropriate fields */
	md->ii_Screen = md->ii_PubScreen;
	md->ii_ScrType = PUBLICSCREEN;

	/* Bring the screen to front */
	ScreenToFront (md->ii_PubScreen);

	DBUG_RETURN(TRUE);
    }
    else
    {
	DBUG_RETURN(FALSE);
    }
}

/* This routine opens the screen and sets the font & palette */
BOOL OpenEnvironment (struct AppInfo * ai, struct MsgHandler * mh, struct TagItem * tls)
{
    struct MHObject *mho = &(mh->mh_Header);
    struct IDCMPInfo *md = mho->mho_SysData;
    struct ScreenModePref *smp = NULL;
    struct PalettePref *pp = NULL;
    struct TagItem tag[6], *clone = NULL, *tmp;
    struct TagItem *ns1 = NULL, *ns2 = NULL;
    struct TagItem *np = NULL;
    struct TagItem *tl;
    UWORD *palette = NULL;
    LONG num_colors = 0L;
    LONG sname;

    DBUG_ENTER("OpenEnvironment");

    /* Get the correct tag list to work with */
    tl = (struct TagItem *) GetTagData (APSH_WindowEnv, (LONG) tls, tls);
    ns1 = (struct TagItem *) GetTagData (APSH_NewScreen, NULL, tls);
    ns2 = (struct TagItem *) GetTagData (APSH_NewScreenTags, NULL, tls);
    np  = (struct TagItem *) GetTagData (APSH_Palette, NULL, tls);

    DOE( kprintf("**OpenEnv: Entry\n\n") );
    DOE(     KPT("**OpenEnv: Tags",tls ) );
    DOE( kprintf("**OpenEnv: NewScreen     = %lx\n", ns1) );
    DOE( kprintf("**OpenEnv: NewScreenTags = %lx\n", ns2) );
    DOE( kprintf("**OpenEnv: NewPalette    = %lx\n\n", np) );

    /* Clear out any old tags */
    if (md->ii_NSTags)
    {
	FreeTagItems (md->ii_NSTags);
	md->ii_NSTags = NULL;
    }
    md->ii_NS = NULL;

/**************************************************************************/

    /* PUBSCREEN keyword overrides everything else */
    /* Should also be checking the icon!!! */
    if ((sname = WhichOption (ai, "pubscreen/k")) >= 0)
    {
	/* Was the name specified? */
	if (ai->ai_Options[sname])
	{
	    /* Get the screen name */
	    ai->ai_ScreenName = (STRPTR) ai->ai_Options[sname];

	    DS( kprintf("Screen: %s\n",ai->ai_ScreenName));
	    DOE( kprintf("**OpenEnv: PubScreen = %s\n\n",ai->ai_ScreenName) );

		DPS(kprintf("OpenEnv: Calling lockscreen for cli pubscreen\n"));
	    if (lockscreen (ai, md, ai->ai_ScreenName))
	    {
		/* Everything is set up! */
		DBUG_RETURN(TRUE);
	    }
	}
    }

/**************************************************************************/
/* Custom screen overrides screenmode preference */
/**************************************************************************/

    /* Do we have a hard-coded screen? */
    if ( ns1 || ns2 )
    {
	/* Do we have a NewScreen? */
	if (ns1)
	{
	    /* Remember it */
	    DB (kprintf ("Got NewScreen structure\n"));
	    md->ii_NS = (struct NewScreen *) ns1->ti_Data;
	    DOE( kprintf("**OpenEnv: VerNS = %lx\n\n", md->ii_NS) );
	}

	/* Do we have screen tags? */
	if (ns2)
	{
	    /* Make sure we have tags to clone */
	    if (ns2->ti_Data)
	    {
		DOE( KPT("**OpenEnv: Preclone...", ns2) );
		DB (kprintf ("Got NewScreen tag items\n"));
		md->ii_NSTags = clone =
		  CloneTagItems ((struct TagItem *) ns2);
		DOE( KPT("**OpenEnv: Postclone...", clone) );
	    }
	}
    }
    /* Use floating preferences */
    else
    {
	/* See if there is a preference record for the screen mode */
	if (smp = GetPrefRecord (&(ai->ai_PrefList), PREFS_SCREENMODE))
	{
	    struct Prefs *p = &(smp->smp_Header);

	    if (p->p_Flags & PREFS_WHERE_F)
	    {
		DB (kprintf ("No screen mode\n"));
		DOE( kprintf("**OpenEnv: No Pref screen mode\n\n") );
		smp = NULL;
	    }
	    else
	    {
		DB (kprintf ("Got a screen mode\n"));
		DOE( kprintf("**OpenEnv: Pref screen mode found\n\n") );

		tag[0].ti_Tag = SA_DisplayID;
		tag[0].ti_Data = (LONG) smp->smp_ModeID;
		tag[1].ti_Tag = SA_Width;
		tag[1].ti_Data = (LONG) smp->smp_Width;
		tag[2].ti_Tag = SA_Height;
		tag[2].ti_Data = (LONG) smp->smp_Height;
		tag[3].ti_Tag = SA_Depth;
		tag[3].ti_Data = (LONG) smp->smp_Depth;
		tag[4].ti_Tag = SA_AutoScroll;
		tag[4].ti_Data = (LONG) smp->smp_AutoScroll;
		tag[5].ti_Tag = TAG_DONE;

		/* To get one contiguous tag list */
		clone = CloneTagItems (tag);
	    }
	}
    }

/**************************************************************************/

    /* Do we have a hard-code palette? */
    if (np)
    {
	WORD depth = 0;

	DB (kprintf ("Got a palette\n"));

	/* Get the depth of the screen */
	if (ns2 = FindTagItem (SA_Depth, clone))
	{
	    depth = (WORD) ns2->ti_Data;
	}
	else if (md->ii_NS)
	{
	    depth = md->ii_NS->Depth;
	}

	/* Get a pointer to the color palette */
	palette = (UWORD *) np->ti_Data;

	/* Get the number of colors */
	num_colors = (1 << depth);

	/* Make sure we get NewLook set */
	if (!(FindTagItem (SA_Pens, clone)))
	{
	    UWORD penspec[1];

	    penspec[0] = (~0);

	    tag[0].ti_Tag = SA_Pens;
	    tag[0].ti_Data = (ULONG) penspec;
	    tag[1].ti_Tag = (clone) ? TAG_MORE : TAG_IGNORE;
	    tag[1].ti_Data = (LONG) clone;
	    tag[2].ti_Tag = TAG_DONE;

	    /* To get one contiguous tag list */
	    tmp = CloneTagItems (tag);
	    FreeTagItems (clone);
	    clone = tmp;

	    DB (kprintf ("using default pen spec\n"));
	}
    }
    /* Get the floating preference palette */
    else
    {
	if (pp = GetPrefRecord (&(ai->ai_PrefList), PREFS_PALETTE))
	{
	    DB (kprintf ("got a palette preference file\n"));

	    tag[0].ti_Tag = SA_Pens;
	    tag[0].ti_Data = (LONG) pp->pp_Pens;
	    tag[1].ti_Tag = SA_FullPalette;
	    tag[1].ti_Data = TRUE;
	    tag[2].ti_Tag = (clone) ? TAG_MORE : TAG_IGNORE;
	    tag[2].ti_Data = (LONG) clone;
	    tag[3].ti_Tag = TAG_DONE;

	    /* To get one contiguous tag list */
	    tmp = CloneTagItems (tag);
	    FreeTagItems (clone);
	    clone = tmp;

	    /* Get a pointer to the color palette */
	    palette = (UWORD *) pp->pp_CRegs;

	    /* Get the number of colors used */
	    num_colors = pp->pp_NumColors;
	}
    }

/**************************************************************************/

    /* If we have the makings of a new screen, then do it */
    if (md->ii_NS || md->ii_NSTags || smp)
    {
	md->ii_NSTags = clone;

	/* See if they have already specified a public screen name */
	if ((clone == NULL) || !(FindTagItem (SA_PubName, clone)))
	{
	    /* Make a new list */
	    tag[0].ti_Tag = SA_PubName;
	    tag[0].ti_Data = (LONG) ai->ai_BaseName;
	    DS (kprintf ("BaseName [%s]\n", ai->ai_BaseName));
	    DOE( kprintf("**OpenEnv: PubScreenName = %s\n\n",ai->ai_BaseName) );
	    tag[1].ti_Tag = (clone) ? TAG_MORE : TAG_IGNORE;
	    tag[1].ti_Data = (LONG) clone;
	    tag[2].ti_Tag = TAG_DONE;

	    /* To get one contiguous tag list */
	    tmp = CloneTagItems (tag);
	    FreeTagItems (clone);
	    md->ii_NSTags = clone = tmp;
	}

	DOE( kprintf("**OpenEnv: VerScreenTags = %lx\n\n",md->ii_NSTags) );

	/* Fill in the screen name */
	if (ai->ai_ScreenName = (STRPTR) GetTagData (SA_PubName, NULL, clone))
	{
	    DS (kprintf ("ScreenName [%s]\n", ai->ai_ScreenName));
	    /* See if the screen already exists */
		DPS(kprintf("OpenEnv: Calling lockscreen for custom pubscreen\n"));
	    if (lockscreen (ai, md, ai->ai_ScreenName))
	    {
		/* Everything is set up! */
		DBUG_RETURN(TRUE);
	    }
	}
	else
	{
	    DS (kprintf ("No screen name\n"));
	}

	/* Set the screen type */
	md->ii_ScrType = CUSTOMSCREEN;
	if (md->ii_NS)
	{
	    md->ii_ScrType = md->ii_NS->Type;
	}
	md->ii_ScrType = GetTagData (SA_Type, (LONG) md->ii_ScrType, md->ii_NSTags);

	/* Attempt to open the screen */
	if (md->ii_Screen = OpenScreenTagList (md->ii_NS, md->ii_NSTags))
	{
	    struct RastPort *rp = &(md->ii_Screen->RastPort);
	    struct GelsInfo *ginfo = NULL;

	    /*
	     * Don't set up a Gel System if there is already one for this
	     * screen
	     */
	    if (rp->GelsInfo == NULL)
	    {
		/* Setup the Gels for icons */
		ginfo = setupGelSys (rp, 0xFC);
	    }

	    /* See if it is a public screen */
	    if (FindTagItem (SA_PubName, clone))
	    {
		/* Set status here */
		PubScreenStatus (md->ii_Screen, NULL);

		/* Set the screen type */
		md->ii_ScrType = PUBLICSCREEN;
	    }
	}
	else
	{
	    /* Unable to open the screen, so show that */
	    ai->ai_Pri_Ret = RETURN_FAIL;
	    ai->ai_Sec_Ret = APSH_CLDNT_OPEN_SCREEN;
	    ai->ai_TextRtn = PrepText (ai, APSH_MAIN_ID, ai->ai_Sec_Ret, NULL);

	    NotifyUser (ai, ai->ai_TextRtn, NULL);

	    DBUG_RETURN(FALSE);
	}

	if (palette)
	{
	    LoadRGB4 (&(md->ii_Screen->ViewPort), palette, num_colors);
	}

	/* Indicate that we opened the screen */
	md->Open_Screen = TRUE;
    }
    else
    {
	/* Free the unused tags */
	FreeTagItems (clone);

	/* Open on the default public screen */
	DPS(kprintf("OpenEnv: Calling lockscreen for default\n"));
	lockscreen (ai, md, NULL);
    }

    DBUG_RETURN(TRUE);
}

BOOL CloseEnvironment ( struct AppInfo * ai,
			struct MsgHandler * mh,
			struct TagItem * tl )
{
    register struct MHObject *mho;

    DBUG_ENTER("CloseEnvironment");

    /* initialize a few variables */
    mho = &(mh->mh_Header);

    DBUG_RETURN(FALSE);
}

VOID FreshenScreenMode (struct AppInfo * ai, struct ScreenModePref * smp)
{
    struct TagItem tag[2];
    struct Prefs *p;

    DBUG_ENTER("FreshenScreenMode");

    /* Say that we want an update */
    tag[0].ti_Tag = PREFS_FRESHEN_A;
    tag[0].ti_Data = (LONG) smp;
    tag[1].ti_Tag = TAG_DONE;

    /* This will return the same thing as passed... */
    GetPref (PREFS_SCREENMODE, ai->ai_ConfigDir, tag);

    p = &(smp->smp_Header);

    if (p->p_Flags & PREFS_CLOSEALL_F)
    {
	/* Gotta change everything */
	PerfFunc (ai, DeActivateID, NULL, NULL);
	PerfFunc (ai, ActivateID, NULL, NULL);
    }

    DBUG_VOID_RETURN;
}

VOID FreshenPalette (struct AppInfo * ai, struct PalettePref * pp)
{
    struct List *list = &(ai->ai_PrefList);
    struct ScreenModePref *smp;
    struct TagItem tag[3];
    struct Prefs *p;

    DBUG_ENTER("FreshenPalette");

    /* Say that we want an update */
    tag[0].ti_Tag = PREFS_FRESHEN_A;
    tag[0].ti_Data = (LONG) pp;
    tag[1].ti_Tag = TAG_IGNORE;
    tag[2].ti_Tag = TAG_DONE;

    if (smp = GetPrefRecord (list, PREFS_SCREENMODE))
    {
	tag[1].ti_Tag = PREFS_DEPTH;
	tag[1].ti_Data = (ULONG) smp->smp_Depth;
    }

    GetPref (PREFS_PALETTE, ai->ai_ConfigDir, tag);

    p = &(pp->pp_Header);

    if (p->p_Flags & PREFS_CLOSEALL_F)
    {
	/* Gotta change everything */
	PerfFunc (ai, DeActivateID, NULL, NULL);
	PerfFunc (ai, ActivateID, NULL, NULL);
    }
    else
    {
	struct Screen *scr = ai->ai_Screen;

	/* Set the screen colors */
	LoadRGB4 (&(scr->ViewPort), pp->pp_CRegs, pp->pp_NumColors);
    }

    DBUG_VOID_RETURN;
}

/* Should actually lock IntuitionBase while doing this... */
VOID FreshenPointer (struct AppInfo * ai, struct PointerPref * pp)
{
    VOID SetPPointer (struct Window *, struct PointerPref *);
    struct Screen *scr = ai->ai_Screen;
    struct Window *win = scr->FirstWindow;
    struct TagItem tag[2];

    DBUG_ENTER("FreshenPointer");

    /* Say that we want an update */
    tag[0].ti_Tag = PREFS_FRESHEN_A;
    tag[0].ti_Data = (LONG) pp;
    tag[1].ti_Tag = TAG_DONE;

    GetPref (NULL, ai->ai_ConfigDir, tag);

    while (win)
    {
	struct ObjectInfo *oi = (struct ObjectInfo *) win->UserData;
	struct WinNode *wn = (struct WinNode *) oi->oi_SystemData;

	/* See if this window belongs to us */
	if (wn && (wn->wn_Header.mho_Node.ln_Type == APSH_MHO_WINDOW))
	{
	    /* Update the pointer... */
	    if (wn->wn_PP == pp)
		SetPPointer (win, pp);
	}

	/* Get a pointer to the next window */
	win = win->NextWindow;
    }

    DBUG_VOID_RETURN;
}

/*
 * This is a generic debugging routine in search of a home.  KPT() is similar
 * to kprintf() but serves to output a nicely formatted tag list using kprintf()
 * to the serial port.
 *
 */

VOID	KPT( char *prompt, struct TagItem *tags )
{

	Tag 		cur_tag = 0L;

	struct TagItem 	*nxt_tag = tags;

	ULONG		cur_data = 0L,
			done = 0L;

	/*
	 * kprintf() the prompt string
	 *
	 */

	kprintf("%s...\n",prompt);
	kprintf("START ADDR = 0x%lx\nTAGS:\n", nxt_tag);

	/*
	 * followed by the tags themselves.  Since we can't provide names
	 * for the tags in any reasonable size, we'll just print the numerics
	 * aside from the (very) few we do know about.
	 *
	 */

	while (!done)
	{

		cur_tag = nxt_tag->ti_Tag;
		cur_data = nxt_tag->ti_Data;

		switch (cur_tag)
		{
			case TAG_DONE:

				kprintf( "%lx:\t  TAG_DONE,\t***\n\n",
				         nxt_tag );
				done = TRUE;
				nxt_tag = NULL;
				break;

			case TAG_IGNORE:

				kprintf( "%lx:\tTAG_IGNORE,\t%lu\n",
				         nxt_tag,
				         cur_data );
				nxt_tag++;
				break;

			case TAG_MORE:

				kprintf( "%lx:\t  TAG_MORE,\t%lx\n",
				         nxt_tag,
				         cur_data );
				nxt_tag = (struct TagItem *) cur_data;
				break;

			case TAG_SKIP:

				kprintf( "%lx:\t  TAG_SKIP,\t%lu\n",
				         nxt_tag,
				         cur_data );
				nxt_tag = nxt_tag + cur_data;
				break;

			default:

				kprintf( "%lx:\t  TAG_USER,\t%10lu,\t%lu\n",
				         nxt_tag,
				         cur_tag,
				         cur_data );
				nxt_tag++;

		}
	}
}