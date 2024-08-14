
/*********************************************************************/
/*                                                                   */
/*                     Copyright (c) 1985                            */
/*                    Commodore-Amiga, Inc.                          */
/*                    All rights reserved.                           */
/*                                                                   */
/*     No part of this program may be reproduced, transmitted,       */
/*     transcribed, stored in retrieval system, or translated        */
/*     into any language or computer language, in any form or        */
/*     by any means, electronic, mechanical, magnetic, optical,      */
/*     chemical, manual or otherwise, without the prior written      */
/*     permission of:                                                */
/*                     Commodore-Amiga, Inc.                         */
/*                     983 University Ave #D                         */
/*                     Los Gatos, CA. 95030                          */
/*                                                                   */
/*********************************************************************/

/*********************************************************************/
/*                                                                   */
/*  Program name:  cdio                                              */
/*                                                                   */
/*  Purpose:  To provide standard console device interface routines  */
/*            such as Open, GetChar, PutChar, etc.                   */
/*                                                                   */
/*  Arguments:  (window)    - The window to associate the console    */
/*                            device with. Used in CDOpen.           */
/*              (character) - The char to output. Used in CDPutChar  */
/*              (string)    - The string to output. Used in CDPutStr */
/*                                                                   */
/*  Programer:  Stan Shepard                                         */
/*                                                                   */
/*  Date Released:  01-Jun-85                                        */
/*  Revised 12-Oct-85 for native Lattice-C Compiler (C.Scheppner)    */     
/*********************************************************************/

/* #include <exec/types.h>
   #include "standard.h"
   #include <exec/nodes.h>
   #include <exec/lists.h>
   #include <exec/ports.h>
   #include <exec/io.h>
   #include <devices/console.h>
   #include <intuition/intuition.h> */
#include <intuition/intuition.h>

extern	void	CDPutStrL();
extern	void	queue_read();
extern	void	CDPutChar();
extern	void	CDPutStr();
extern	void	CDClose();

struct IOStdReq consoleIO       = {0};
struct IOStdReq consoleREADIO   = {0};
struct MsgPort  consoleMsgPort  = {0};

UBYTE consoleReadChar  = 0;
UBYTE OutChar = 0;

int CDOpen(window)
struct Window *window;

{
   int error;
      
/* Open the console device */
   consoleIO.io_Data = (APTR) window;
   consoleIO.io_Length = sizeof(*window);
   if ((error = OpenDevice("console.device", 0, &consoleIO, 0)) != 0)
   {
      printf("CDInit OpenDevice error: %d.\n", error);
      return(error);
   }

/* Set up the message port in the I/O request */
   consoleMsgPort.mp_Node.ln_Type = NT_MSGPORT;
   consoleMsgPort.mp_Node.ln_Name = "CDIO";
   consoleMsgPort.mp_Flags = 0;
   consoleMsgPort.mp_SigBit = AllocSignal(-1);
   consoleMsgPort.mp_SigTask = (struct Task *) FindTask((char *) NULL);
   AddPort(&consoleMsgPort);
   consoleIO.io_Message.mn_ReplyPort = &consoleMsgPort;

/* Start reading */
   consoleREADIO = consoleIO;
   queue_read();

   return(0);
}

void CDClose()
{
   CloseDevice(&consoleIO);
   FreeSignal(consoleMsgPort.mp_SigBit);
   RemPort(&consoleMsgPort);
}

int CDMayGetChar()
{
   register int temp;

   if ( GetMsg(&consoleMsgPort) == NULL ) return(-1);
   temp = consoleReadChar;
   queue_read();
   return(temp);
}

void queue_read()
{
   consoleREADIO.io_Command = CMD_READ;
   consoleREADIO.io_Data = (APTR)&consoleReadChar;
   consoleREADIO.io_Length = 1;
   SendIO(&consoleREADIO);           /* Due to bug in DoIO */
}

UBYTE CDGetChar()
{
   register temp;

   while (GetMsg(&consoleMsgPort) == NULL) ; /* during console IO */
   temp = consoleReadChar;
   queue_read();
   return((UBYTE)temp);
}

void CDPutChar(character)
char character;
{
   OutChar = character;
   consoleIO.io_Command = CMD_WRITE;
   consoleIO.io_Data = (APTR)&OutChar;
   consoleIO.io_Length = 1;
   DoIO(&consoleIO);
}

void CDPutStr(string)
char *string;      /*  NULL terminated string to output */
{
   consoleIO.io_Command = CMD_WRITE;
   consoleIO.io_Data = (APTR)string;
   consoleIO.io_Length = -1;
   DoIO(&consoleIO);
}

void CDPutStrL(string,length)
char *string;      /* output string of length length */
int length;
{
   consoleIO.io_Command = CMD_WRITE;
   consoleIO.io_Data = (APTR)string;
   consoleIO.io_Length = length;
   DoIO(&consoleIO);
}
