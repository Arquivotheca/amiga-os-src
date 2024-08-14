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
/*********************************************************************/

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/ports.h>
#include <exec/io.h>

#include <devices/console.h>
#include <intuition/intuition.h>


struct IOStdReq consoleIO = 0;
struct IOStdReq consoleREADIO = 0;
struct MsgPort consoleMsgPort = 0;
UBYTE consoleReadChar = 0;

int CDOpen(window)
struct Window *window;
{
	int error;
		
/* Open the console device */
	consoleIO.io_Data = (char *) window;
	consoleIO.io_Length = sizeof(*window);
	if ((error = OpenDevice("console.device", 0, &consoleIO, 0)) != 0)
	{
		kprintf("CDInit OpenDevice error: %d.\n", error);
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

CDClose()
{
	CloseDevice(&consoleIO);
	FreeSignal(consoleMsgPort.mp_SigBit);
	RemPort(&consoleMsgPort);
}

int CDMayGetChar()
{
	register temp;

	if ( GetMsg(&consoleMsgPort) == NULL ) return(-1);
	temp = consoleReadChar;
	queue_read();
	return(temp);
}

queue_read()
{
	consoleREADIO.io_Command = CMD_READ;
	consoleREADIO.io_Data = &consoleReadChar;
	consoleREADIO.io_Length = 1;
	SendIO(&consoleREADIO);			  /* Due to bug in DoIO */
}

UBYTE CDGetChar()
{
	register temp;

	WaitIO(&consoleREADIO);
	temp = consoleReadChar;
	queue_read();
	return(temp);
}

CDPutChar(character)
char character;
{
	consoleIO.io_Command = CMD_WRITE;
	consoleIO.io_Data = &character;
	consoleIO.io_Length = 1;
	DoIO(&consoleIO);
}

CDPutStr(string)
char *string;		/*  NULL termiinated string to output  */
{
	consoleIO.io_Command = CMD_WRITE;
	consoleIO.io_Data = string;
	consoleIO.io_Length = -1;
	DoIO(&consoleIO);
}
