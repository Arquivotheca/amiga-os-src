#include <intuition/intuition.h>
#include "ddefs.h"

extern	int	stop_test;		/* Not 0 means <ctrl-c> typed */
extern	struct	Window *Window;
	ULONG	IMClass;
	struct	IntuiMessage *message;
extern	int	abort;			/* Not 0 means exit program */
/*
 * This routine checks to see if any key has been typed.
 * If one was, it is input, and compared to <ctrl-c>.
 * If <ctrl-c> is typed, then stop_test is set to 1
 * It also checks to see if the CLOSE gadget was selected, and
 * sets ABORT if it was.
 */

check_stop_test()
{
        stop_test = (stop_test || ('\003' == CDMayGetChar()));

		/* Set ABORT if close gadget was selected */

       while( message = (struct IntuiMessage *)GetMsg(Window->UserPort)) {
         IMClass = message->Class;        /* Correspond to IDCMP flags */
         ReplyMsg(message);         /* reply when info is obtained */
         abort = abort || ((IMClass & CLOSEWINDOW) !=0);
         }
	return(stop_test || abort);
}
