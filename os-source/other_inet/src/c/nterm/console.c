/* console i/o */

#include "exec/types.h"
#include "exec/exec.h"
#include "devices/console.h"
#include "intuition/intuition.h"
#include "term.h"

#define READ_WAIT if (CheckIO(&consoleREADIO)==0) WaitIO(&consoleREADIO)
#define WRITE_WAIT if (CheckIO(&consoleIO)==0) WaitIO(&consoleIO)

struct IOStdReq consoleIO = 0;
struct IOStdReq consoleREADIO = 0;
struct MsgPort consoleOutPort = 0, consoleInPort;
UBYTE consoleReadChar = 0;
ULONG cbit;

int CDOpen(window)
struct Window *window;
{
	int error;
		
/* Open the console device */
	consoleIO.io_Data = (char *) window;
	consoleIO.io_Length = sizeof(*window);
	if ((error = OpenDevice("console.device", 0, &consoleIO, 0)) != 0)
	{
/*		kprintf("CDInit OpenDevice error: %d.\n", error);*/
		return(error);
	}

/* Set up the OUTPUT message port in the I/O request */
	consoleOutPort.mp_Node.ln_Type = NT_MSGPORT;
	consoleOutPort.mp_Node.ln_Name = "CDIO";
	consoleOutPort.mp_Flags = 0;
	consoleOutPort.mp_SigBit = AllocSignal(-1);
	consoleOutPort.mp_SigTask = (struct Task *) FindTask((char *) NULL);
	AddPort(&consoleOutPort);
	consoleIO.io_Message.mn_ReplyPort = &consoleOutPort;
/* Set up the INPUT message port in the I/O request */
	consoleREADIO = consoleIO;
	consoleInPort.mp_Node.ln_Type = NT_MSGPORT;
	consoleInPort.mp_Node.ln_Name = "CDIO";
	consoleInPort.mp_Flags = 0;
	consoleInPort.mp_SigBit = AllocSignal(-1);
	consoleInPort.mp_SigTask = (struct Task *) FindTask((char *) NULL);
	AddPort(&consoleInPort);
	consoleREADIO.io_Message.mn_ReplyPort = &consoleInPort;


/* Start reading */
	queue_read();
	cbit = 1 << consoleInPort.mp_SigBit; /* the console input signal bit */
	return(0);
}

CDClose()
{
	CloseDevice(&consoleIO);
	FreeSignal(consoleOutPort.mp_SigBit);
	FreeSignal(consoleInPort.mp_SigBit);
	RemPort(&consoleOutPort);
	RemPort(&consoleInPort);
}

int CDMayGetChar() /* return a char or -1 (none) */
{
	register temp;

	if (CheckIO(&consoleREADIO)==0) return(-1); /* if no char */
	WaitIO(&consoleREADIO); /* reply */
	temp = consoleReadChar; /* get char */
	queue_read(); /* queue another read */
	return(temp); /* return char */
}

queue_read() /* queue a 1 char read */
{
	consoleREADIO.io_Command = CMD_READ;
	consoleREADIO.io_Data = &consoleReadChar;
	consoleREADIO.io_Length = 1;
	SendIO(&consoleREADIO); /* queue the read */
}

UBYTE CDGetChar() /* get a char */
{
	register temp;

	READ_WAIT;
	temp = consoleReadChar; /* get it */
	queue_read(); /* queue another read */
	return(temp); /* return it */
}

CDPutChar(character) /* write a char */
unsigned char character;
{
/*	WRITE_WAIT; */
	consoleIO.io_Command = CMD_WRITE;
	consoleIO.io_Data = &character;
	consoleIO.io_Length = 1;
/*	SendIO(&consoleIO); /* write it */
	DoIO(&consoleIO); /* write it and reply */
}

CDPutStr(string)
unsigned char *string;		/*  NULL termiinated string to output  */
{
/*	WRITE_WAIT; */
	consoleIO.io_Command = CMD_WRITE;
	consoleIO.io_Data = string;
	consoleIO.io_Length = -1;
/*	SendIO(&consoleIO); /* write it */
	DoIO(&consoleIO);
}

CDPutStrL(string,length)
unsigned char *string;		/* string to output  */
UWORD length;		/* the length */
{
/*	WRITE_WAIT; */
	consoleIO.io_Command = CMD_WRITE;
	consoleIO.io_Data = string;
	consoleIO.io_Length = length;
/*	SendIO(&consoleIO); /* write it */
	DoIO(&consoleIO);
}
