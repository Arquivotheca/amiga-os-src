
/* includes */
#include <exec/types.h>
#include <intuition/intuitionbase.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <dos.h>

/* prototypes */
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>

/* direct ROM interface */
#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>

/* application includes */
#include "resetwb.h"
#include "texttable.h"
#include "iprefs.h"


/*****************************************************************************/


extern struct IntuitionBase *IntuitionBase;
extern struct Library       *SysBase;
extern struct Library	    *DOSBase;
extern struct Task          *parentTask;
extern char __far            WBName[];


/*****************************************************************************/


#define POLL_NUMTRIES	(3)

/* global counts retries due to snooping */
static LONG trycount;


/*****************************************************************************/


/* return screen pointer of Workbench if the screen is currently opened */
static struct Screen *FindWorkbench(VOID)
{
struct Screen        *wb = NULL;
struct PubScreenNode *node;

    if (node = (struct PubScreenNode *)FindName(LockPubScreenList(),WBName))
        wb = node->psn_Screen;

    UnlockPubScreenList();

    return(wb);
}


/*****************************************************************************/


struct Screen *LockWB(VOID)
{
ULONG          ilock;
struct Screen *wb;

    ilock = LockIBase(0);

    if (wb = FindWorkbench())
        wb = LockPubScreen(WBName);

    UnlockIBase(ilock);

    return(wb);
}


/*****************************************************************************/


BOOL CouldCloseWB(struct Window *refwindow)
{
ULONG          lock;
struct Screen *sp;
struct Window *wp;
BOOL           result = TRUE;

    geta4();     /* since we're called from CloseWorkBench() */

    lock = LockIBase(0);

    if (sp = FindWorkbench())
    {
	/* look for windows which are not either:
	 * - WBENCHWINDOWS
	 * - refwindow
	 */
	for (wp = sp->FirstWindow; wp; wp = wp->NextWindow)
	{
	    if ( (!(wp->Flags & WBENCHWINDOW)) && (wp != refwindow))
	    {
		result = FALSE;
		break;
	    }
	}
    }

    UnlockIBase(lock);
    return(result);
}


/*****************************************************************************/


/* pulls easy-request but keeps checking for
 * conditions where it thinks it might be able
 * to resetwb successfully.  When that happens,
 * it will pull down the autorequest and return TRUE,
 * just as it will if the user selects RETRY in the
 * requester.
 */
static LONG RequestPoll(VOID)
{
struct Window    *wp;
LONG 	          result;
struct EasyStruct ez;

    OpenCat();

    ez.es_StructSize    = sizeof(struct EasyStruct);
    ez.es_Flags         = 0;
    ez.es_Title         = NULL;
    ez.es_TextFormat    = GetStr(MSG_IP_CLOSEWINDOW_PROMPT);
    ez.es_GadgetFormat  = GetStr(MSG_IP_CLOSEWINDOW_GADGETS);

    SignalParent();

    wp = BuildEasyRequestArgs(NULL,&ez,0,NULL);

    while (TRUE)
    {
         Delay(5);

         result = SysReqHandler(wp,NULL,FALSE);

         /* return if Retry selected */
         if (result >= 0)
            break;

         if ((trycount < POLL_NUMTRIES) && CouldCloseWB(wp))
         {
             trycount++;
             result = 1;         /* pretend that Retry was selected */
             break;
         }
    }

    FreeSysRequest(wp);

    CloseCat();

    return(result);
}


/*****************************************************************************/


BOOL ResetWB(VOID)
{
BOOL result = FALSE;

    trycount = 0;

    do
    {
	if (CloseWorkBench())
	{
	    OpenWorkBench();
	    result = TRUE;
	    break;
	}
	else if (FindWorkbench() == NULL)
	{
	    /* don't open WB if it's not already open */
	    result = TRUE;
	    break;
	}
    } while (RequestPoll());

    return(result);
}
