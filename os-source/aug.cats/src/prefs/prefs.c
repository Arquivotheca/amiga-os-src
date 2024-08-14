/* prefs.c
 * preference file shared system library
 * Written by David N. Junod
 */

#include "prefsbase.h"

void kprintf (void *,...);

#define	DB(x)	;
#define	DF(x)	;
#define	DR(x)	;

extern struct Library *UtilityBase, *IFFParseBase, *DOSBase;

/* Private functions */
struct ScreenModePref *
GetScreenModePref (BPTR drawer, STRPTR name, VOID * p, struct TagItem * attrs);
struct PalettePref *
GetPalettePref (BPTR drawer, STRPTR name, VOID * p, struct TagItem * attrs);

/*
 * This routine compairs the locks given and reports if they are
 * on the same object.  Returns TRUE if they are the same.
 */
SHORT CompairLocks (BPTR Lock1, BPTR Lock2)
{
    register struct FileLock *lock1;
    register struct FileLock *lock2;
    register SHORT flag;

    lock1 = (struct FileLock *) BADDR (Lock1);
    lock2 = (struct FileLock *) BADDR (Lock2);

    flag = ((lock1->fl_Key) == (lock2->fl_Key));
    flag &= ((lock1->fl_Task) == (lock2->fl_Task));
    flag &= ((lock1->fl_Volume) == (lock2->fl_Volume));

    return (flag);
}

/****** prefs/GetPrefRecord ************************************************
*
*   NAME
*	GetPrefRecord - Obtain a specific preference record from a list.
*
*   SYNOPSIS
*	pref = GetPrefRecord (list, kind);
*	d0		      a1    d0
*
*	VOID *pref;
*	struct List *list;
*	LONG kind;
*
*   FUNCTION
*	This function will search through a list of preference records and
*	return a pointer to the requested kind.
*
*   INPUTS
*	list	- Pointer to a list whose nodes represent preference records.
*	kind	- A preference record kind as defined in
*		  <libraries/prefs_lib.h>
*
*   RETURNS
*	pref	- Pointer to the appropriate preference structure.
*
*   SEE ALSO
*	GetPref()
*
*********************************************************************
*
* Created:  14-Sep-90, David N. Junod
*
*/

VOID * __asm GetPrefRecord (
	register __a1 struct List *list,
	register __d0 LONG kind)
{
    if (list)
    {
	/* See if there is anything in the list */
	if (list->lh_TailPred != (struct Node *) list)
	{
	    struct Node *node, *next;

	    /* Get a pointer to the first node */
	    node = list->lh_Head;

	    /* Step through the list */
	    while (next = node->ln_Succ)
	    {
		struct Prefs *p;

		/* Get a pointer to the Prefs header portion */
		p = &(((struct ScreenModePref *) node)->smp_Header);

		if (p->p_Kind == kind)
		{
		    return ((VOID *)node);
		}

		/* Get a pointer to the next record */
		node = next;
	    }
	}
    }

    return (NULL);
}

/****** prefs/GetPref ******************************************************
*
*   NAME
*	GetPref - Get user preference information.
*
*   SYNOPSIS
*	pref = GetPref (kind, drawer, attrs)
*	d0		d0    a1      a2
*
*	VOID *pref;
*	LONG kind;
*	BPTR drawer;
*	struct TagItem *attrs;
*
*   FUNCTION
*	This function will return a pointer to the requested type of user
*	preference information.
*
*	Performs the following operations, until it is able to get the
*	specified user preference file:
*
*	  o  Search the specified drawer for the requested type of user
*	     preference file.
*
*	  o  Search ENV:Sys for the requested type of user preference file.
*
*	  o  Search internal table for default system preferences.
*
*	This function can ONLY be called from a process.
*
*   INPUTS
*
*	kind	- Type of user preference information to obtain.  Following
*		  are the valid kinds, defined in <libraries/prefs_lib.h>.
*
*		  PREFS_SCREENMODE
*		  Screen configuration preference file.  Contains the display
*		  mode id, width, height, depth and whether the user prefers
*		  autoscroll or not.
*  
*		  PREFS_PALETTE
*		  Color palette and pen spec.
*
*		  PREFS_POINTER
*		  Mouse pointer image to use for the 'normal' pointer..
*
*		  PREFS_BUSYPOINTER
*		  Mouse pointer image to use for the busy pointer.
*
*		  PREFS_WBCONFIG
*		  For most applications, the only thing to obtain from this
*		  preference structure, is whether to bring a window forward
*		  when it has been double-clicked.
*
*		  PREFS_FONT
*
*	drawer	- A lock on the directory to begin the search for the
*		  user preference file.  This lock should be obtained
*		  by the GetPrefsDrawer() function.
*
*	attrs	- Pointer to a TagItem array.  Following are the valid tags,
*		  which are defined in <libraries/prefs_lib.h>.
*
*		  PREFS_DEFAULT_A, <struct>
*		  Supply a default structure to use in the case that no
*		  file is located.
*
*		  PREFS_QUICKFAIL_A, [TRUE/FALSE]
*		  Indicate that if unable to find the preference file in
*		  the first step, then use the default, or supplied default.
*
*		  PREFS_NAME_A, <name>
*		  Use the supplied name, instead of using the internal
*		  name lookup table.  Usefull for loading a color palette
*		  from any IFF ILBM, for example.
*
*		  PREFS_NOTIFY_A, <port>
*		  Indicate that you want notification on the preference file.
*		  This function will fill in the NotifyRequest structure and
*		  start notification.  The FreePref() function will end
*		  notification and free the structure, among other things.
*
*		  PREFS_FRESHEN_A, <prefs>
*		  Indicate that you wish to freshen an existing preference
*		  record, perhaps due to receiving a file notification message.
*		  The kind field of GetPref() doesn't have to contain a valid
*		  value, as it will get the kind from the prefs record that
*		  is passed.
*
*		  PREFS_LIST_A, <list>
*		  Add the preference record to a list.  Will not allow
*		  duplicate kinds in the list.
*
*   RESULTS
*	pref	- Pointer to the appropriate preference structure.  The
*		  structure must only be freed by the FreePref() function.
*
*		  If you asked for fresher preference record (using the
*		  PREFS_FRESHEN_A tag), then check p_Flags to see if
*		  you need to shutdown to invoke the new preferences (check
*		  for the PREFS_CLOSEALL_F flag being set).
*
*		  If NULL, then use IoErr() to find out what the error was.
*
*   SEE ALSO
*	SetPref(), FreePref()
*
*********************************************************************
*
* Created:  14-Sep-90, David N. Junod
*
*/

VOID *__asm GetPref (
			 register __d0 LONG kind,
			 register __a1 BPTR drawer,
			 register __a2 struct TagItem * attrs)
{
    struct Process *proc = (struct Process *) FindTask (NULL);
    struct LibBase *lb = ((struct ExtLibrary *) PrefsBase)->el_Base;
    struct TagItem ins[3];
    VOID *enterval = NULL;
    VOID *retval = NULL;

    DB (kprintf ("GP(%ld,0x%lx,0x%lx)\n", kind, drawer, attrs));

    /* Clear the error field */
    proc->pr_Result2 = 0L;

    /*
     * See if we're being asked to freshen up, maybe someone has detected a
     * slight odor.
     */
    if (enterval = (VOID *) GetTagData (PREFS_FRESHEN_A, NULL, attrs))
    {
	struct Prefs *p;

	/* Get a pointer to the Prefs header portion */
	p = &(((struct ScreenModePref *) enterval)->smp_Header);

	/* Clear the close all flag */
	p->p_Flags &= ~PREFS_CLOSEALL_F;

	/* Set the kind */
	kind = p->p_Kind;
    }

    /* Range check the kind */
    if ((kind >= 0L) && (kind <= PREFS_KINDS))
    {
	STRPTR name = NULL;
	BPTR lock;

	/* Make sure we have a directory lock to work with */
	if (drawer)
	{
	    /* Lock onto the directory that was passed */
	    lock = DupLock (drawer);
	}
	else
	{
	    /* Lock onto the system directory */
	    lock = DupLock (lb->lb_SysPrefs);
	}

	/* Make sure we have a name to work with */
	name = (STRPTR) GetTagData (PREFS_NAME_A, (LONG) lb->lb_PrefName[kind], attrs);

	/* Could have a jump table instead... */
	switch (kind)
	{
	    case PREFS_SCREENMODE:
		retval = GetScreenModePref (lock, name, enterval, attrs);
		break;

	    case PREFS_PALETTE:
		retval = GetPalettePref (lock, name, enterval, attrs);
		break;

	    case PREFS_POINTER:
		retval = GetPointerPref (lock, name, enterval, attrs);
		break;

	    case PREFS_BUSYPOINTER:
		ins[0].ti_Tag = PREFS_PRIVATE_1;
		ins[0].ti_Data = TRUE;
		ins[1].ti_Tag = TAG_DONE;
		ins[1].ti_Data = attrs;
		if (attrs)
		    ins[1].ti_Tag = TAG_MORE;

		retval = GetPointerPref (lock, name, enterval, ins);
		break;

	    default:
		proc->pr_Result2 = ERROR_ACTION_NOT_KNOWN;
		break;
	}

	/* See if we have results */
	if (retval)
	{
	    struct List *list;
	    struct MsgPort *mp;
	    struct Prefs *p;

	    /* Get a pointer to the Prefs header portion */
	    p = &(((struct ScreenModePref *) retval)->smp_Header);

	    /* Set the kind */
	    p->p_Kind = kind;

	    /* Set the callback */
	    p->p_Func =	(VOID (*)) GetTagData (PREFS_CALLBACK_A, (LONG) p->p_Func, attrs);

	    /* See if they want us to add the record to a list */
	    if (list = (struct List *) GetTagData (PREFS_LIST_A, NULL, attrs))
	    {
		/* Make sure it doesn't already exist in the list */
		if (!(GetPrefRecord (list, kind)))
		{
		    /* Add the record to the tail of the list */
		    AddTail (list, (struct Node *) retval);
		    DB (kprintf("Add 0x%lx to list\n", retval));
		}
	    }

	    /* See if they want notification on the file */
	    if (mp = (struct MsgPort *) GetTagData (PREFS_NOTIFY_A, NULL, attrs))
	    {
		LONG msize;
		struct NotifyRequest *nr;

		/* Obtain exclusive access to our work buffer */
		ObtainSemaphore (&(lb->lb_Lock));

		/* Get the complete file name from the lock */
		NameFromLock (lock, lb->lb_WorkText, 510);

		/* Build a complete name */
		AddPart (lb->lb_WorkText, name, 510);

		/* Calculate the size of the notification request */
		msize = sizeof (struct NotifyRequest) +
			strlen (lb->lb_WorkText) + 1L;

		/* Allocate a notification request */
		if (nr = (struct NotifyRequest *) AllocVec (msize, MEMF_CLEAR))
		{
		    BOOL stat;

		    /* Remember the newly allocate NotifyRequest struct */
		    p->p_NR = nr;

		    /* Copy the name */
		    nr->nr_Name = MEMORY_FOLLOWING (nr);
		    strcpy (nr->nr_Name, lb->lb_WorkText);

		    /* Set up the UserData to point back at ourselves */
		    nr->nr_UserData = (ULONG) retval;

		    /* Set the appropriate flags */
		    nr->nr_Flags = NRF_SEND_MESSAGE | NRF_WAIT_REPLY;

		    /* Set the message port */
		    nr->nr_stuff.nr_Msg.nr_Port = mp;

		    /* Start the notification */
		    stat = StartNotify (nr);

		}		/* Allocate notification request */

		/* Release the work buffer */
		ReleaseSemaphore (&(lb->lb_Lock));

	    }			/* See if they want notification */
	}			/* End of if successful */

	/* Unlock the temporary lock */
	UnLock (lock);
    }				/* End of kind range check */
    else
    {
	/* Somebody doesn't know what they're doing ... */
	proc->pr_Result2 = ERROR_ACTION_NOT_KNOWN;
    }

    return (retval);
}

/****** prefs/SetPref ******************************************************
*
*   NAME
*	SetPref - Write a preference record to disk.
*
*   SYNOPSIS
*	success = SetPref (kind, drawer, pref, attrs)
*	d0		   d0    a1      a2    a3
*
*	BOOL success;
*	LONG kind;
*	BPTR drawer;
*	VOID *pref;
*	struct TagItem *attrs;
*
*   FUNCTION
*	This function will write the given preference record to disk.
*
*	Currently, this function is not implemented.
*
*   INPUTS
*	kind	- A preference record kind as defined in
*		  <libraries/prefs_lib.h>
*
*	drawer	- A lock on the directory to write the record to.
*
*	pref	- Pointer to the preference record to write.
*
*	attrs	- Pointer to a TagItem array.
*
*   RETURNS
*	success	- TRUE indicates success.  FALSE indicates error, check
*		  IoErr() to determine reason for failure.
*
*   SEE ALSO
*	GetPref()
*
*********************************************************************
*
* Created:  14-Sep-90, David N. Junod
*
*/

BOOL __asm SetPref (
		        register __d0 LONG kind,
		        register __a1 BPTR drawer,
		        register __a2 VOID * pref,
		        register __a3 struct TagItem * attrs)
{
    struct Process *proc = (struct Process *) FindTask (NULL);

    /* Clear the error field */
    proc->pr_Result2 = 0L;

    return (FALSE);
}

/****** prefs/FreePref ********************************************************
*
*   NAME
*	FreePref - Free a preference record obtained by GetPref.
*
*   SYNOPSIS
*	FreePref (pref);
*		  a1
*
*	VOID *pref;
*
*   FUNCTION
*	This function will properly deallocate a preference record.  This
*	also stop notification on preference files that are being watched.
*
*   INPUTS
*	pref	- Pointer to the preference record to free.
*
*   SEE ALSO
*	FreePrefList(), GetPref()
*
*********************************************************************
*
* Created:  14-Sep-90, David N. Junod
*
*/

VOID __asm FreePref (register __a1 VOID * pref)
{
    DR (kprintf("FreePref 0x%lx\n", pref));
    if (pref)
    {
	struct Prefs *p = &(((struct ScreenModePref *) pref)->smp_Header);
	struct PointerPref *pp = (struct PointerPref *) pref;
	struct NotifyRequest *nr;

	/* See if there is a notification request structure */
	if (nr = p->p_NR)
	{
	    /* Turn off notification on the file */
	    EndNotify (nr);

	    /* Free the notification request structure */
	    FreeVec ((APTR) nr);
	    p->p_NR = NULL;
	}

	switch (p->p_Kind)
	{
	    case PREFS_POINTER:
	    case PREFS_BUSYPOINTER:
		if (pp->pp_DSize)
		{
		    FreeVec ((APTR) pp->pp_PData);
		}
		break;

	    default:
		break;
	}

	/* Free the preference structure */
	DR (kprintf("FreeVec 0x%lx\n", pref));
	FreeVec ((APTR) pref);
    }
}

/****** prefs/FreePrefList ****************************************************
*
*   NAME
*	FreePrefList - Free a list of preference records obtained by GetPref.
*
*   SYNOPSIS
*	FreePrefList (list);
*		      a1
*
*	struct List *list;
*
*   FUNCTION
*	This function will step through a list of preference records, and
*	deallocate each record.  This will also stop notification on the
*	appropriate preference files.
*
*   INPUTS
*	list	- Pointer to a list whose nodes represent preference records.
*
*   SEE ALSO
*	FreePref(), GetPref()
*
*********************************************************************
*
* Created:  14-Sep-90, David N. Junod
*
*/

VOID __asm FreePrefList (register __a1 struct List * list)
{
    struct Node *node, *next;

    if (list)
    {
	/* See if there is anything in the list */
	if (list->lh_TailPred != (struct Node *) list)
	{
	    /* Get a pointer to the first node */
	    node = list->lh_Head;

	    /* Step through the list */
	    while (next = node->ln_Succ)
	    {
		/* Remove the node from the list */
		Remove (node);

		/* Free the preference record */
		FreePref (node);

		/* Get a pointer to the next record */
		node = next;
	    }
	}
    }
}
