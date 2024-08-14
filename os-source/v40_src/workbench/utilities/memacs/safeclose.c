#include "ed.h"

#if AMIGA
#include "exec/types.h"
#include "exec/nodes.h"
#include "exec/lists.h"
#include "exec/ports.h"
#include "intuition/intuition.h"

/* this function closes an intuition window that shares a port with
* other intuition windows.
*
* It is careful to set the UserPort to null before closing, and to
* free any messages that it might have been sent.
*/

void CloseWindowSafely( win )
struct Window *win;
{
	/* we forbid here to keep out of race conditions with intuition */
    Forbid();

	/* send back any messages for this window 
	* that have not yet been processed
	*/
    StripIntuiMessages( win->UserPort, win );

	/* clear UserPort so intuition will not free it */
    /*win->UserPort = NULL;*/

	/* tell inuition to stop sending more messages */
    ModifyIDCMP( win, 0 );

	/* turn tasking back on */
    Permit();

	/* and really close the window */

    CloseWindow( win );
}

void StripIntuiMessages( mp, win )
struct MsgPort *mp;
struct Window *win;
{
struct IntuiMessage *msg;
struct IntuiMessage *succ;

msg = (struct IntuiMessage *)mp->mp_MsgList.lh_Head;

while( succ = (struct IntuiMessage *)msg->ExecMessage.mn_Node.ln_Succ ) {

	if( msg->IDCMPWindow ==  win ) {
	/* intuition is about to rudely free this message.
	* Make sure that we have politely sent it back.
	*/
	Remove( msg );

	ReplyMsg( msg );
	}

msg = succ;
}
}
#endif
