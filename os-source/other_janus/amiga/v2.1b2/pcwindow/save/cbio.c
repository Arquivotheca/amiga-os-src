
/* *** cbio.c ********************************************************
 *   
 *  Program name:  cbio
 *
 *  Purpose:  To provide standard clipboard device interface routines
 *         such as Open, Post, Read, Write, etc.
 *
 * Modified by RJ on 18 Dec 86 to be more in the Amiga standard
 *
 ******************************************************************* */

#include "exec/types.h"
#include "exec/ports.h"
#include "exec/io.h"
#include "devices/clipboard.h"

void writeLong();
void CBClose();
void CBReadCleanup();

struct IOClipReq clipboardIO = {0};
struct MsgPort clipboardMsgPort = {0};
LONG lastWriteID = -1;
/*???struct MsgPort satisfyMsgPort = {0};*/


struct Task *FindTask();


SHORT   CBOpen(unit)
SHORT   unit;
{
   SHORT   error;

   /* open the clipboard device */
   if ((error = (SHORT)OpenDevice("clipboard.device", unit, &clipboardIO, 0))
         != 0)
      return(error);

   /* Set up the message port in the I/O request */
   clipboardMsgPort.mp_Node.ln_Type = NT_MSGPORT;
   clipboardMsgPort.mp_Flags = 0;
   clipboardMsgPort.mp_SigBit = AllocSignal(-1);
   clipboardMsgPort.mp_SigTask = FindTask(NULL);
   AddPort(&clipboardMsgPort);
   clipboardIO.io_Message.mn_ReplyPort = &clipboardMsgPort;

/*???   satisfyMsgPort.mp_Node.ln_Type = NT_MSGPORT;*/
/*???   satisfyMsgPort.mp_Flags = 0;*/
/*???   satisfyMsgPort.mp_SigBit = AllocSignal(-1);*/
/*???   satisfyMsgPort.mp_SigTask = FindTask(NULL);*/
/*???   AddPort(&satisfyMsgPort);*/
   return(0);
}



void CBClose()
{
/*???   RemPort(&satisfyMsgPort);*/
   RemPort(&clipboardMsgPort);
   if (clipboardMsgPort.mp_SigBit)
      FreeSignal(clipboardMsgPort.mp_SigBit);
   CloseDevice(&clipboardIO);
}



CBSatisfyPost(string, length)
char *string;
LONG length;
{
   clipboardIO.io_Offset = 0;
   writeLong("FORM");
   length += 12;
   writeLong(&length);
   writeLong("FTXT");
   writeLong("CHRS");
   length -= 12;
   writeLong(&length);

   clipboardIO.io_Command = CMD_WRITE;
   clipboardIO.io_Data = (STRPTR)string;
   clipboardIO.io_Length = length;
   DoIO(&clipboardIO);

   lastWriteID = clipboardIO.io_ClipID;

   clipboardIO.io_Command = CMD_UPDATE;
   return(DoIO(&clipboardIO));
}



void writeLong(ldata)
LONG *ldata;
{
   SHORT status;

   clipboardIO.io_Command = CMD_WRITE;
   clipboardIO.io_Data = (STRPTR)ldata;
   clipboardIO.io_Length = 4;
   status=(DoIO(&clipboardIO));
}



CBWriteText(string,length)
char *string;
SHORT length;
{
   clipboardIO.io_ClipID = 0;
   return(CBSatisfyPost(string,length));
}



SHORT CBReadSetup()
/* To do a Clipboard read, call:
 *   - CBReadSetup();
 *   - CBRead(); as many times as necessary.
 *   - CBReadCleanup();
 * 
 * Sets up the Clipboard for a read, returns the length of text to be read.
 * If length < 0, error.
 */
{
   LONG length;
   SHORT status;
   UBYTE string[16];

   status = 0;
   length = -1;

   clipboardIO.io_Command = CMD_READ; /* get the FORM */
   clipboardIO.io_Data = string;
   clipboardIO.io_Length = 4;
   clipboardIO.io_Offset = 0;
   clipboardIO.io_ClipID = 0;
   status -= DoIO(&clipboardIO);
   string[4]='\0';

   if(!strcmp(string,"FORM"))
      { /* iff form */
      clipboardIO.io_Command = CMD_READ; /* get the total length */
      clipboardIO.io_Data = (STRPTR)&length;
      clipboardIO.io_Length = 4;
      status -=DoIO(&clipboardIO);

      clipboardIO.io_Command = CMD_READ; /* read the chunk and body */
      clipboardIO.io_Data = string;
      clipboardIO.io_Length = 8;
      status -=DoIO(&clipboardIO);
      string[8]='\0';

      if (!strcmp(string,"FTXTCHRS"))
         {
         clipboardIO.io_Command = CMD_READ; /* get the length of the data */
         clipboardIO.io_Data = (STRPTR)&length;
         clipboardIO.io_Length = 4;
         status -=DoIO(&clipboardIO);
         }
      else status = -1;
      }

   if (!status) return(length);
   else return(-1);
}



SHORT CBRead(string, length)
UBYTE *string;
SHORT length;
{
   SHORT status;

   status = 0;

   clipboardIO.io_Command = CMD_READ;
   clipboardIO.io_Data = string;
   clipboardIO.io_Length = length;
   status -=DoIO(&clipboardIO);
   if (!status)return(length);
   else return(-1);
}



void CBReadCleanup()
{
   clipboardIO.io_Command = CMD_READ;
   clipboardIO.io_Length = 1;
   clipboardIO.io_Data = 0;
   DoIO(&clipboardIO);
}



SHORT CBCurrentReadID()
{
   clipboardIO.io_Command = CBD_CURRENTREADID;
   DoIO(&clipboardIO);
   return(clipboardIO.io_ClipID);
}



SHORT CBCurrentWriteID()
{
   clipboardIO.io_Command = CBD_CURRENTWRITEID;
   DoIO(&clipboardIO);
   return(clipboardIO.io_ClipID);
}



BOOL CBReadEqualsWrite()
{
/*
   SHORT i;
*/

   return(lastWriteID == CBCurrentReadID());
}



/*
BOOL CBCheckSatisfy(idVar)
SHORT *idVar;
{
   struct SatisfyMsg *sm;

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
   return(FALSE);
}

CBCut(stream, length)
char *stream;
SHORT length;
{
   clipboardIO.io_Command = CMD_WRITE;
   clipboardIO.io_Data = stream;
   clipboardIO.io_Length = length;
   clipboardIO.io_Offset = 0;
   clipboardIO.io_ClipID = 0;
   DoIO(&clipboardIO);
   clipboardIO.io_Command = CMD_UPDATE;
   DoIO(&clipboardIO);
}

SHORT CBPost()
{
   clipboardIO.io_Command = CBD_POST;
   clipboardIO.io_Data = &satisfyMsgPort;
   clipboardIO.io_ClipID = 0;
   DoIO(&clipboardIO);
   return(clipboardIO.io_ClipID);
}
*/

