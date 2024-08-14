/* dnh.c
 *
 */

#include "hyperbrowser.h"

/*****************************************************************************/

#define	DB(x)	;

/*****************************************************************************/

STRPTR nodeNames[] =
{
    "MAIN",
    "MEMORY",

    "LIBRARYLIST",
    "LIBRARY",
    "DEVICELIST",
    "DEVICE",

    "CLASSLIST",
    "CLASS",
    "SCREENLIST",
    "SCREEN",
    "WINDOW",
    "GADGETLIST",
    "GADGET",
    NULL,
};

STRPTR nodeTitles[] =
{
    "HyperBrowser",
    "Memory",

    "Exec Library List",
    "Library Information",
    "Exec Device List",
    "Device Information",

    "Intuition BOOPSI Class List",
    "Intuition BOOPSI Class Information",
    "Intuition Screen List",
    "Screen Information",
    "Window Information",
    "Gadget List",
    "Gadget Information",
};

enum
{
    NM_MAIN,
    NM_MEMORY,

    NM_LIBRARYLIST,
    NM_LIBRARY,
    NM_DEVICELIST,
    NM_DEVICE,

    NM_CLASSLIST,
    NM_CLASS,
    NM_SCREENLIST,
    NM_SCREEN,
    NM_WINDOW,
    NM_GADGETLIST,
    NM_GADGET,
};

#define	BEGIN	'('
#define	END	')'

/*****************************************************************************/

LONG Tokenize (struct GlobalData * gd, STRPTR name, ULONG * address)
{
    UBYTE buffer[32];
    BOOL good = TRUE;
    ULONG total = 0;
    LONG i, j, k;
    BYTE digit;
    BYTE ch;

    /* See if it is the MAIN node */
    if (Stricmp (name, nodeNames[0]) == 0)
	return NM_MAIN;

    /* Chop the prefix from the node name */
    strcpy (buffer, &name[strlen (BASENAME) + 1]);

    i = k = 0;
    j = strlen (buffer);
    while (i < j)
    {
	/* Search for an address */
	if (buffer[i++] == BEGIN)
	{
	    /* Terminate the buffer */
	    buffer[(i - 2)] = 0;

	    /* Build the hex value string */
	    while ((i < j) && (buffer[i] != END) && good)
	    {
		ch = ToUpper (buffer[i++]);
		digit = ch - '0';
		if (digit < 0 || digit > 9)
		{
		    digit = ch - 'A';
		    if (digit >= 0 && digit < 6)
			digit += 10;
		    else
			good = FALSE;
		}
		total = UMult32 (total, 16) + digit;
	    }

	    /* Able to convert it? */
	    if (good)
		*address = total;

	    /* All done parsing */
	    i = j;
	}
    }

    /* Find the token */
    for (i = 0; nodeNames[i]; i++)
	if (Stricmp (buffer, nodeNames[i]) == 0)
	    return i;
    return -1;
}

/*****************************************************************************/

ULONG ASM nodehost (REG (a0) struct Hook * h, REG (a2) STRPTR o, REG (a1) Msg msg)
{
    struct GlobalData *gd = (struct GlobalData *) h->h_Data;
    struct opFindHost *ofh = (struct opFindHost *) msg;
    struct opNodeIO *onm = (struct opNodeIO *) msg;
    ULONG retval = FALSE;
    ULONG address;
    LONG token;
    ULONG id;

    switch (msg->MethodID)
    {
	case HM_FINDNODE:
	    DB (kprintf ("find : \"%s\" : ", ofh->ofh_Node));
	    /* We have to have a MAIN node, even if we never use it */
	    if (Stricmp (ofh->ofh_Node, nodeNames[0]) == 0)
	    {
		/* Show that it is ours */
		DB (kprintf ("found\n"));
		retval = TRUE;
	    }
	    /* Is it one of our nodes? */
	    else if ((Strnicmp (ofh->ofh_Node, BASENAME, BASENAME_LENGTH) == 0) &&
		     ((token = Tokenize (gd, ofh->ofh_Node, &address)) >= 0))
	    {
		/* Show that it is ours */
		DB (kprintf ("found\n"));
		retval = TRUE;

		/* Set the node title */
		ofh->ofh_Title = nodeTitles[token];

		/* Don't let any browsing occur */
		ofh->ofh_Next = ofh->ofh_Prev = ofh->ofh_Node;
	    }
	    else
	    {
		DB (kprintf ("not found\n"));
	    }
	    break;

	case HM_OPENNODE:
	    DB (kprintf ("open %s\n", onm->onm_Node));

	    /* Set the buffer pointer */
	    gd->gd_Node[0] = 0;

	    /* Build the document */
	    switch (token = Tokenize (gd, onm->onm_Node, &address))
	    {
		case NM_MEMORY:
		    showmemory (gd, address);
		    break;

		    /* EXEC INFORMATION */
		case NM_LIBRARYLIST:
		    showlibrarylist (gd);
		    break;

		case NM_LIBRARY:
		    showlibrary (gd, address);
		    break;

		case NM_DEVICELIST:
		    showdevicelist (gd);
		    break;

		case NM_DEVICE:
		    showdevice (gd, address);
		    break;

		    /* INTUITION INFORMATION */
		case NM_CLASSLIST:
		    showclasslist (gd);
		    break;

		case NM_SCREENLIST:
		    showscreenlist (gd);
		    break;

		case NM_SCREEN:
		    showscreen (gd, address);
		    break;

		case NM_WINDOW:
		    showwindow (gd, address);
		    break;
	    }

	    /* Prepare the buffer information */
	    onm->onm_DocBuffer = gd->gd_Node;
	    onm->onm_BuffLen = strlen (onm->onm_DocBuffer);

	    /* Remove the node from the database when done */
	    onm->onm_Flags |= HTNF_CLEAN;

	    /* Show successful open */
	    retval = TRUE;
	    break;

	case HM_CLOSENODE:
	    DB (kprintf ("close %s\n", onm->onm_Node));
	    retval = TRUE;
	    break;

	case HM_EXPUNGE:
	    break;
    }
    return retval;
}
