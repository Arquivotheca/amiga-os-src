/********************************************************************* 
 *                                                                   *
 *                     Copyright (c) 1985                            *
 *                    Commodore-Amiga, Inc.                          *
 *                    All rights reserved.                           *
 *                                                                   *
 *     No part of this program may be reproduced, transmitted,       *
 *     transcribed, stored in retrieval system, or translated        *
 *     into any language or computer language, in any form or        *
 *     by any means, electronic, mechanical, magnetic, optical,      *
 *     chemical, manual or otherwise, without the prior written      *
 *     permission of:                                                *
 *                     Commodore-Amiga, Inc.                         *
 *                     983 University Ave #D                         *
 *                     Los Gatos, CA. 95030                          *
 *                                                                   *
 *********************************************************************/

/*********************************************************************
 *                                                                   *
 *  Program name:  cdio                                              *
 *                                                                   *
 *  Purpose:  To provide standard console device interface routines  *
 *            such as Open, GetChar, PutChar, etc.                   *
 *                                                                   *
 *  Arguments:  (window)    - The window to associate the console    *
 *                            device with. Used in CDOpen.           *
 *              (character) - The char to output. Used in CDPutChar  *
 *              (string)    - The string to output. Used in CDPutStr *
 *                                                                   *
 *********************************************************************/

#include "ed.h"

#if	AMIGA

struct IOStdReq consoleIO = {0};
struct IOStdReq consoleREADIO = {0};
struct MsgPort consoleMsgPort = {0};
UBYTE consoleReadChar = 0;

#ifdef FUNNYSTRING
UBYTE stringbuffer[256];
#endif

int CDOpen(window)
struct Window *window;
{
	int error;
		
/* Open the console device */
	consoleIO.io_Data = (APTR) window;
	consoleIO.io_Length = 1; /* sizeof(*window); */
	if ((error = OpenDevice("console.device", 0, &consoleIO, 0)) != 0)
		return(error);

/* Set up the message port in the I/O request */
	consoleMsgPort.mp_Node.ln_Type = NT_MSGPORT;
	consoleMsgPort.mp_Node.ln_Name = "CON_EMACS";
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
	while(GetMsg(&consoleMsgPort));;
	RemPort(&consoleMsgPort);
        FreeSignal(consoleMsgPort.mp_SigBit);
	CloseDevice(&consoleIO);
}

int CDMayGetChar()
{
	 int temp;

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
	SendIO(&consoleREADIO);			  /* Due to bug in DoIO */
}

UBYTE CDGetChar()
{
	 int temp;

	while (GetMsg(&consoleMsgPort) == NULL) ; /* during console IO */
	temp = consoleReadChar;
	queue_read();
	return((UBYTE)temp);
}

void CDPutChar(character)
UBYTE character;
{
	consoleIO.io_Command = CMD_WRITE;
	consoleIO.io_Data = (APTR)&character;
	consoleIO.io_Length = 1;
	DoIO(&consoleIO);
}

void CDPutStr(string)
UBYTE *string;		/*  NULL terminated string to output  */
{
#ifdef FUNNYSTRING
strcpy(stringbuffer,"\033[1v");
strcpy(&stringbuffer[4],string);
strcat(&stringbuffer[4],"\033[v");
#endif
	consoleIO.io_Command = CMD_WRITE;
#ifdef FUNNYSTRING
	consoleIO.io_Data = (APTR)stringbuffer;
#else
	consoleIO.io_Data = (APTR)string;
#endif
	consoleIO.io_Length = -1;
	DoIO(&consoleIO);
}

void CDPutStrL(string,length)
UBYTE *string;		/*  output a string of length length */
int length;
{
#ifdef FUNNYSTRING
Lstrcpy(&stringbuffer[4],string,length);
stringbuffer[0]=stringbuffer[length+4]='\033';
stringbuffer[1]=stringbuffer[length+5]='[';
stringbuffer[2]='1';
stringbuffer[3]=stringbuffer[length+6]='v';
#endif
	consoleIO.io_Command = CMD_WRITE;
#ifdef FUNNYSTRING
	consoleIO.io_Data = (APTR)stringbuffer;
	consoleIO.io_Length = length+7;
#else
	consoleIO.io_Data = (APTR)string;
	consoleIO.io_Length = length;
#endif
	DoIO(&consoleIO);
}

#endif
