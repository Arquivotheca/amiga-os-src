/*
 * $Id: infomisc.c,v 38.2 93/02/04 11:00:58 mks Exp $
 *
 * $Log:	infomisc.c,v $
 * Revision 38.2  93/02/04  11:00:58  mks
 * Just some minor cleanup here...
 * 
 * Revision 38.1  91/06/24  11:36:29  mks
 * Initial V38 tree checkin - Log file stripped
 *
 */

/*------------------------------------------------------------------------*/

#include <string.h>
#include <exec/memory.h>
#include <dos/datetime.h>

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>

#include "info.h"
#include "support.h"

#include <clib/gadtools_protos.h>
#include "proto.h"
#include "quotes.h"

/*------------------------------------------------------------------------*/

/*  Function Prototypes: */

void datetostring(STRPTR string, struct DateStamp *ds);
struct Node *AllocNode(STRPTR name);
void EnableGadget(struct Window *win, struct Gadget *gad);
void DisableGadget(struct Window *win, struct Gadget *gad);

/*------------------------------------------------------------------------*/

/*/ datetostring()
 *
 * Converts the supplied datestamp into a string of the form
 * dd-mmm-yy hh:mm:ss, such as "04-Sep-89 21:04:10", which gets
 * stuffed into the supplied buffer.
 *
 * Created:  04-Sep-89, Peter Cherna
 * Modified: 10-Apr-90, Peter Cherna
 *
 */

void datetostring(string, ds)

    STRPTR string;
    struct DateStamp *ds;

{
struct DateTime dt;
char date[LEN_DATSTRING];
char time[LEN_DATSTRING];

    strcpy(string,Quote(Q_VOL_IS_UNKNOWN));
    if (ds)
    {
        dt.dat_Stamp = *ds;
        DP(("%ld %ld %ld\n", dt.dat_Stamp.ds_Days,dt.dat_Stamp.ds_Minute, dt.dat_Stamp.ds_Tick));

        dt.dat_Format = FORMAT_DOS;
        dt.dat_Flags = NULL;
        dt.dat_StrDay = NULL;
        dt.dat_StrDate = date;
        dt.dat_StrTime = time;

        if (DateToStr(&dt))
        {
            strcpy(string, date);
            strcat(string, " ");
            strcat(string, time);
        }
    }
}


/*------------------------------------------------------------------------*/

/*/ AllocNode
 *
 * Function to allocate space for a node and its text string.
 *
 * Created:  17-Jul-89, Peter Cherna
 * Modified: 17-Jul-89, Peter Cherna
 *
 * Returns:  pointer to node or NULL if memory allocation failed.
 *
 * Bugs:  Should use strncpy() or some such overrun protection
 *
 */

struct Node *AllocNode(STRPTR name)
{
struct Node *node;

	node = ALLOCVEC(sizeof(struct Node) + TOOLTYPESLENGTH + 1, MEMF_CLEAR|MEMF_PUBLIC);

	if (node)
	{

	node->ln_Name = (char *) (node + 1);
	/*  stccpy is Lattice's ANSI function that copies at most n
	    characters from the second string into the first, but
	    guarantees null-termination, unlike strncpy */

		stccpy(node->ln_Name, name, (TOOLTYPESLENGTH+1));
	}

    return(node);
}

/*------------------------------------------------------------------------*/
/* FreeNode() became FREEVEC() */

/*------------------------------------------------------------------------*/

/*/ EnableGadget()
 *
 * Function to enable a particular gadget if it is not already so.
 *
 * Created:  17-Jul-89, Peter Cherna
 * Modified: 17-Jul-89, Peter Cherna
 *
 * Returns: none (void)
 *
 * Bugs:
 *
 */

void EnableGadget(win, gad)

    struct Window *win;
    struct Gadget *gad;

    {
    GT_SetGadgetAttrs(gad,win,NULL,GA_Disabled,FALSE,TAG_DONE);
    }


/*------------------------------------------------------------------------*/

/*/ DisableGadget()
 *
 * Function to disable a particular gadget if it is not already so.
 *
 * Created:  17-Jul-89, Peter Cherna
 * Modified: 19-Jul-89, Peter Cherna
 *
 * Returns: none (void)
 *
 * Bugs:
 *
 */

void DisableGadget(win, gad)

    struct Window *win;
    struct Gadget *gad;

    {
    GT_SetGadgetAttrs(gad,win,NULL,GA_Disabled,TRUE,TAG_DONE);
    }


/*------------------------------------------------------------------------*/
