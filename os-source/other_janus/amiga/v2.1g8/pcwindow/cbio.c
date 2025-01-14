
/**** cbio.c ********************************************************
 *   
 *  Program name:  cbio
 *
 *  Purpose:  To provide standard clipboard device interface routines
 *         such as Open, Post, Read, Write, etc.
 *
 * 18 jul 89   -BK         Added function entry debug kprintfs to EVERY
 *                         function. The Exorcism of RJ begins!
 * Modified by RJ on 18 Dec 86 to be more in the Amiga standard
 *
 ******************************************************************* */

#include "zaphod.h"
#include <devices/clipboard.h>
#include <proto/exec.h>
#include <string.h>

#define  DBG_CB_OPEN_ENTER                1
#define  DBG_CB_OPEN_RETURN               1
#define  DBG_CB_CLOSE_ENTER               1
#define  DBG_CB_CLOSE_RETURN              1
#define  DBG_CB_SATISFY_POST_ENTER        1
#define  DBG_CB_SATISFY_POST_RETURN       1
#define  DBG_WRITE_LONG_ENTER             1
#define  DBG_WRITE_LONG_RETURN            1
#define  DBG_CB_WRITE_TEXT_ENTER          1
#define  DBG_CB_WRITE_TEXT_RETURN         1
#define  DBG_CB_READ_SETUP_ENTER          1
#define  DBG_CB_READ_SETUP_RETURN         1
#define  DBG_CB_READ_ENTER                1
#define  DBG_CB_READ_RETURN               1
#define  DBG_CB_READ_CLEANUP_ENTER        1
#define  DBG_CB_READ_CLEANUP_RETURN       1
#define  DBG_CB_CURRENT_READ_ID_ENTER     1
#define  DBG_CB_CURRENT_READ_ID_RETURN    1
#define  DBG_CB_CURRENT_WRITE_ID_ENTER    1
#define  DBG_CB_CURRENT_WRITE_ID_RETURN   1
#define  DBG_CB_READ_EQUALS_WRITE_ENTER   1
#define  DBG_CB_READ_EQUALS_WRITE_RETURN  1
#define  DBG_CB_CHECK_SATISFY_ENTER       1
#define  DBG_CB_CHECK_SATISFY_RETURN      1
#define  DBG_CB_CUT_ENTER                 1
#define  DBG_CB_CUT_RETURN                1
#define  DBG_CB_POST_ENTER                1
#define  DBG_CB_POST_RETURN               1

struct IOClipReq clipboardIO = {0};
struct MsgPort clipboardMsgPort = {0};
LONG lastWriteID = -1;

/****i* PCWindow/CBOpen ******************************************
*
*   NAME   
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/

SHORT CBOpen(SHORT unit)
{
   SHORT error;

#if (DEBUG1 & DBG_CB_OPEN_ENTER)
   kprintf("cbio.c:       CBOpen(unit=0x%lx)\n",unit);
#endif

   /* open the clipboard device */
   if ((error = (SHORT)OpenDevice("clipboard.device", unit, (struct IORequest *)&clipboardIO, 0))
         != 0)
      return(error);

   /* Set up the message port in the I/O request */
   clipboardMsgPort.mp_Node.ln_Type = NT_MSGPORT;
   clipboardMsgPort.mp_Flags = 0;
   clipboardMsgPort.mp_SigBit = AllocSignal(-1);
   clipboardMsgPort.mp_SigTask = FindTask(NULL);
   AddPort(&clipboardMsgPort);
   clipboardIO.io_Message.mn_ReplyPort = &clipboardMsgPort;

#if (DEBUG2 & DBG_CB_OPEN_RETURN)
   kprintf("cbio.c:       CBOpen: Returns(0)\n");
#endif

   return(0);
}

/****i* PCWindow/CBClose ******************************************
*
*   NAME   
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/

VOID CBClose()
{
#if (DEBUG1 & DBG_CB_CLOSE_ENTER)
   kprintf("cbio.c:       CBClose(VOID)\n");
#endif

   RemPort(&clipboardMsgPort);
   if (clipboardMsgPort.mp_SigBit)
      FreeSignal(clipboardMsgPort.mp_SigBit);
   CloseDevice((struct IORequest *)&clipboardIO);

#if (DEBUG2 & DBG_CB_CLOSE_RETURN)
   kprintf("cbio.c:       CBClose: Returns(VOID)\n");
#endif
}

/****i* PCWindow/CBSatisyfyPost ******************************************
*
*   NAME   
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/

CBSatisfyPost(string, length)
char *string;
LONG length;
{
#if (DEBUG1 & DBG_CB_SATISFY_POST_ENTER)
   kprintf("cbio.c:       CBSatisfyPost(string=0x%lx,length=0x%lx)\n",string,length);
#endif

   clipboardIO.io_Offset = 0;
   writeLong((LONG *)"FORM");
   length += 12;
   writeLong(&length);
   writeLong((LONG *)"FTXT");
   writeLong((LONG *)"CHRS");
   length -= 12;
   writeLong(&length);

   clipboardIO.io_Command = CMD_WRITE;
   clipboardIO.io_Data = (STRPTR)string;
   clipboardIO.io_Length = length;
   DoIO((struct IORequest *)&clipboardIO);

   lastWriteID = clipboardIO.io_ClipID;

   clipboardIO.io_Command = CMD_UPDATE;

#if (DEBUG2 & DBG_CB_SATISFY_POST_RETURN)
   kprintf("cbio.c:       CBSatisfyPost: Returns(???)\n");
#endif

   return(DoIO((struct IORequest *)&clipboardIO));
}

/****i* PCWindow/writeLong ******************************************
*
*   NAME   
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/

VOID writeLong(LONG *ldata)
{

#if (DEBUG1 & DBG_WRITE_LONG_ENTER)
   kprintf("cbio.c:       writelong(ldata=0x%lx)\n",ldata);
#endif

   clipboardIO.io_Command = CMD_WRITE;
   clipboardIO.io_Data = (STRPTR)ldata;
   clipboardIO.io_Length = 4;
   DoIO((struct IORequest *)&clipboardIO);

#if (DEBUG2 & DBG_WRITE_LONG_RETURN)
   kprintf("cbio.c:       writelong: Returns(VOID)\n");
#endif
}

/****i* PCWindow/CBWriteText ******************************************
*
*   NAME   
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/

CBWriteText(string,length)
char *string;
SHORT length;
{
#if (DEBUG1 & DBG_CB_WRITE_TEXT_ENTER)
   kprintf("cbio.c:       CBWriteText(string=0x%lx,length=0x%lx)\n",string,length);
#endif

   clipboardIO.io_ClipID = 0;

#if (DEBUG2 & DBG_CB_WRITE_TEXT_RETURN)
   kprintf("cbio.c:       CBWriteText: Returns(VOID)\n");
#endif

   return(CBSatisfyPost(string,length));
}

/****i* PCWindow/CBReadSetup ******************************************
*
*   NAME   
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
* To do a Clipboard read, call:
*   - CBReadSetup();
*   - CBRead(); as many times as necessary.
*   - CBReadCleanup();
* 
* Sets up the Clipboard for a read, returns the length of text to be read.
* If length < 0, error.
*
*/

SHORT CBReadSetup()
{
   LONG length;
   SHORT status;
   UBYTE string[16];

#if (DEBUG1 & DBG_CB_READ_SETUP_ENTER)
   kprintf("cbio.c:       CBReadSetup(VOID)\n");
#endif

   status = 0;
   length = -1;

   clipboardIO.io_Command = CMD_READ; /* get the FORM */
   clipboardIO.io_Data = string;
   clipboardIO.io_Length = 4;
   clipboardIO.io_Offset = 0;
   clipboardIO.io_ClipID = 0;
   status -= DoIO((struct IORequest *)&clipboardIO);
   string[4]='\0';

   if(!strcmp(string,"FORM"))
      { /* iff form */
      clipboardIO.io_Command = CMD_READ; /* get the total length */
      clipboardIO.io_Data = (STRPTR)&length;
      clipboardIO.io_Length = 4;
      status -=DoIO((struct IORequest *)&clipboardIO);

      clipboardIO.io_Command = CMD_READ; /* read the chunk and body */
      clipboardIO.io_Data = string;
      clipboardIO.io_Length = 8;
      status -=DoIO((struct IORequest *)&clipboardIO);
      string[8]='\0';

      if (!strcmp(string,"FTXTCHRS"))
         {
         clipboardIO.io_Command = CMD_READ; /* get the length of the data */
         clipboardIO.io_Data = (STRPTR)&length;
         clipboardIO.io_Length = 4;
         status -=DoIO((struct IORequest *)&clipboardIO);
         }
      else status = -1;
      }

   if (!status) return((SHORT)length);
   else return(-1);

#if (DEBUG2 & DBG_CB_READ_SETUP_RETURN)
   kprintf("cbio.c:       CBReadSetup: Returns(???)\n");
#endif
}

/****i* PCWindow/CBRead ******************************************
*
*   NAME   
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/

SHORT CBRead(string, length)
UBYTE *string;
SHORT length;
{
   SHORT status;

#if (DEBUG1 & DBG_CB_READ_ENTER)
   kprintf("cbio.c:       CBRead(string=0x%lx,length=0x%lx)\n",string,length);
#endif

   status = 0;

   clipboardIO.io_Command = CMD_READ;
   clipboardIO.io_Data = string;
   clipboardIO.io_Length = length;
   status -=DoIO((struct IORequest *)&clipboardIO);
   if (!status)return(length);
   else return(-1);

#if (DEBUG2 & DBG_CB_READ_RETURN)
   kprintf("cbio.c:       CBRead: Returns(???)\n");
#endif
}

/****i* PCWindow/CBReadCleanup ******************************************
*
*   NAME   
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/

VOID CBReadCleanup()
{
#if (DEBUG1 & DBG_CB_READ_CLEANUP_ENTER)
   kprintf("cbio.c:       CBReadCleanup(VOID)\n");
#endif

   clipboardIO.io_Command = CMD_READ;
   clipboardIO.io_Length = 1;
   clipboardIO.io_Data = 0;
   DoIO((struct IORequest *)&clipboardIO);

#if (DEBUG2 & DBG_CB_READ_CLEANUP_RETURN)
   kprintf("cbio.c:       CBReadCleanup: Returns(VOID)\n");
#endif
}

/****i* PCWindow/CBCurrentReadID ******************************************
*
*   NAME   
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/

SHORT CBCurrentReadID()
{
#if (DEBUG1 & DBG_CB_CURRENT_READ_ID_ENTER)
   kprintf("cbio.c:       CBCurrentReadID(VOID)\n");
#endif

   clipboardIO.io_Command = CBD_CURRENTREADID;
   DoIO((struct IORequest *)&clipboardIO);

#if (DEBUG2 & DBG_CB_CURRENT_READ_ID_RETURN)
   kprintf("cbio.c:       CBCurrentReadID: Returns(0x%lx)\n",clipboardIO.io_ClipID);
#endif

   return((SHORT)clipboardIO.io_ClipID);
}

/****i* PCWindow/CBCurrentWriteID ******************************************
*
*   NAME   
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/

SHORT CBCurrentWriteID()
{
#if (DEBUG1 & DBG_CB_CURRENT_WRITE_ID_ENTER)
   kprintf("cbio.c:       CBCurrentWriteID(VOID)\n");
#endif

   clipboardIO.io_Command = CBD_CURRENTWRITEID;
   DoIO((struct IORequest *)&clipboardIO);

#if (DEBUG2 & DBG_CB_CURRENT_WRITE_ID_RETURN)
   kprintf("cbio.c:       CBCurrentWriteID: Returns(0x%lx)\n",clipboardIO.io_ClipID);
#endif

   return((SHORT)clipboardIO.io_ClipID);
}

/****i* PCWindow/CBReadEqualsWrite ******************************************
*
*   NAME   
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/

BOOL CBReadEqualsWrite()
{
#if (DEBUG1 & DBG_CB_READ_EQUALS_WRITE_ENTER)
   kprintf("cbio.c:       CBReadEqualsWrite(VOID)\n");
#endif

#if (DEBUG2 & DBG_CB_READ_EQUALS_WRITE_RETURN)
   kprintf("cbio.c:       CBReadEqualsWrite: Returns(???)\n");
#endif

   return((BOOL)(lastWriteID == CBCurrentReadID()));
}

#if 0

/*
BOOL CBCheckSatisfy(idVar)
SHORT *idVar;
{
   struct SatisfyMsg *sm;

#if (DEBUG1 & DBG_CB_CHECK_SATISFY_ENTER)
   kprintf("cbio.c:       CBCheckSatisfy(idVar=0x%lx)\n",idVar);
#endif

   if (*idVar == 0)
      return(TRUE);
   if (*idVar < CBCurrentWriteID()) 
      {
      *idVar = 0;
      return(TRUE);
      }
   if (sm = (struct SatisfyMsg *) GetMsg(&satisfyMsgPort)) 
      {
      if (*idVar == sm->sm_ClipID)
         return(TRUE);
      }

#if (DEBUG2 & DBG_CB_CHECK_SATISFY_RETURN)
   kprintf("cbio.c:       CBCheckSatisfy: Returns(???)\n");
#endif

   return(FALSE);
}

CBCut(stream, length)
char *stream;
SHORT length;
{
#if (DEBUG1 & DBG_CB_CUT_ENTER)
   kprintf("cbio.c:       CBCut(stream=0x%lx,length=0x%lx)\n",stream,length);
#endif

   clipboardIO.io_Command = CMD_WRITE;
   clipboardIO.io_Data = stream;
   clipboardIO.io_Length = length;
   clipboardIO.io_Offset = 0;
   clipboardIO.io_ClipID = 0;
   DoIO(&clipboardIO);
   clipboardIO.io_Command = CMD_UPDATE;
   DoIO(&clipboardIO);

#if (DEBUG2 & DBG_CB_CUT_RETURN)
   kprintf("cbio.c:       CBCut: Return(???)\n");
#endif
}

SHORT CBPost()
{
#if (DEBUG1 & DBG_CB_POST_ENTER)
   kprintf("cbio.c:       CBPost(VOID)\n");
#endif

   clipboardIO.io_Command = CBD_POST;
   clipboardIO.io_Data = &satisfyMsgPort;
   clipboardIO.io_ClipID = 0;
   DoIO(&clipboardIO);

#if (DEBUG2 & DBG_CB_POST_RETURN)
   kprintf("cbio.c:       CBPost: Returns(???)\n");
#endif

   return(clipboardIO.io_ClipID);
}
*/
#endif
