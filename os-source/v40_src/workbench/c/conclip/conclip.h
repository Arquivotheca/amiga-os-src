#ifndef CONCLIP_H
#define CONCLIP_H


/*****************************************************************************/


#include <exec/types.h>
#include <exec/lists.h>
#include <exec/nodes.h>


/*****************************************************************************/


/* Linked list of CHRS blocks - NULL terminated
 * Initially the text starts immediately after this structure,
 * in other words, I do an AllocVec big enough to hold the data,
 * and the structure, but...
 *
 * In the future I may end up allocating the text, and structures
 * separately.  In this case the 'cc_FreeHandle' field will hold a pointer
 * to the text block, so if 'cc_FreeHandle' is non-null, call FreeVec()
 * with the 'cc_FreeHandle' pointer, and then call FreeVec() on the
 * structure.  Otherwise just call FreeVec() on the structure.
 *
 * Always get at the text via the 'cc_Text' pointer.
 *
 * Use RemHead() to remove these structures from the replied
 * message.  You can use something else, but since these put in
 * the message as a linked list of nodes, its the proper way
 * to go.
 *
 * 'cc_Size' tells you how many characters there are in this
 * CHRSChunk ... though as it turns out, the text is NULL
 * terminated, so you may not need to know the size.
 *
 * The 'cc_Flags' field is currently set to 0; means its a
 * standard block of text to be inserted.  This field may have
 * new meanings in the future.
 *
 * The 'cc_UserData' field is there for application use.
 */

struct CHRSChunk
{
    struct MinNode  cc_Link;
    STRPTR          cc_Text;		/* pointer to start of text */
    APTR            cc_FreeHandle;	/* if non-zero, call FreeVec() with this pointer */
    ULONG           cc_Size;		/* total # of characters in this block */
    ULONG           cc_Flags;		/* may end up needing this at some time */
    ULONG           cc_UserData;	/* User can do whatever they want with this */
    char            cc_Data[1];		/* really 'n' # of bytes		*/
};


/*****************************************************************************/


/* This is the message structure that CON: will send to
 * CONCLIP.  Only the cm_Type field needs to be filled in.
 *
 * In the simple implementation, check the cm_type field for
 * a value of TYPE_REPLY, check cm_Error for ERR_NOERROR.  If both
 * of these tests pass, cm_Chunks should contain a linked list
 * of CHRSChunk structures.
 */

struct CmdMsg
{
    struct Message cm_Msg;
    LONG 	   cm_Type;	/* type of message, and return validation */
    long 	   cm_Error;	/* error code return */

    struct MinList cm_Chunks;

    /* used by TYPE_ARGS only */
    LONG           cm_Unit;
    BOOL           cm_Shutdown;
};


/* These are used as keys when sending, and replying to
 * messages via conclips public port - these are random
 * numbers (odd values)
 *
 * Conclip shares a message port for commands received
 * from CON:, and commands generated when the user
 * runs Conclip again (e.g., to turn it off, and change unit #'s)
 *
 * TYPE_REPLY is provided as a means of verifying that the reply
 * message really came from CONCLIP, and not just someone else
 * who happens to be using the same name for a public port.
 */

#define TYPE_ARGS  23		/* passing arguments     */
#define TYPE_READ  25		/* read the clipboard	 */
#define TYPE_REPLY 27		/* reply to a TYPE_READ  */


/*****************************************************************************/


/* Values for CmdMsg.cm_Error and used as return value for many of the
 * functions in this program.
 */
#define  ERR_NOERROR	  0	/* worked good                 */
#define  ERR_NOIFFPARSE	  1	/* hmmm, couldnt open IFFPARSE */
#define  ERR_NOCLIPBOARD  2	/* couldn't open clipboard     */
#define  ERR_NOMEMORY	  3	/* not enough memory           */
#define  ERR_BADIFF	  4	/* some kind of IFF error      */
#define  ERR_OTHER	  5	/* something else went wrong   */


/*****************************************************************************/


#pragma libcall ConsoleBase GetConSnip 36 0
#pragma libcall ConsoleBase SetConSnip 3c 801
#pragma libcall ConsoleBase AddConSnipHook 42 801
#pragma libcall ConsoleBase RemConSnipHook 48 801

VOID AddConSnipHook(struct Hook *hook);
VOID RemConSnipHook(struct Hook *hook);
STRPTR GetConSnip(VOID);


/*****************************************************************************/


#endif /* CONCLIP_H */
