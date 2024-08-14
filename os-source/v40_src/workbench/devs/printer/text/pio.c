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

#include	"exec/types.h"
#include	"exec/nodes.h"
#include	"exec/lists.h"
#include	"exec/ports.h"
#include	"exec/tasks.h"
#include	"exec/io.h"
#include	"devices/printer.h"

union {
    struct IOStdReq ios;
    struct IODRPReq iodrp;
    struct IOPrtCmdReq iopc;
} printerIO;

struct MsgPort replyMsgPort;

int pOpen(signal) /* open the printer */
int signal;
{
    int error;
	    
    if ((error = OpenDevice("printer.device", 0, &printerIO, 0)) != 0)
	return(error);

    /* set up the message port in the I/O request */
    replyMsgPort.mp_Node.ln_Type = NT_MSGPORT;
    replyMsgPort.mp_Node.ln_Name = "PIO";
    replyMsgPort.mp_Flags = 0;
    replyMsgPort.mp_SigBit = signal;
    replyMsgPort.mp_SigTask = (struct Task *) FindTask((char *) NULL);
    AddPort(&replyMsgPort);

    printerIO.ios.io_Message.mn_ReplyPort = &replyMsgPort;
    return(0);
}

pClose() /* close the printer */
{
    CloseDevice(&printerIO);
    RemPort(&replyMsgPort);
}

int
pDumpRPort(rastPort, colorMap, modes, sx,sy, sw,sh, dc,dr, s)
struct RastPort *rastPort;
struct ColorMap *colorMap;
ULONG modes;
UWORD sx, sy, sw, sh;
LONG dc, dr;
UWORD s;
{
    printerIO.iodrp.io_Command = PRD_DUMPRPORT;
    printerIO.iodrp.io_RastPort = rastPort;
    printerIO.iodrp.io_ColorMap = colorMap;
    printerIO.iodrp.io_Modes = modes;
    printerIO.iodrp.io_SrcX = sx;
    printerIO.iodrp.io_SrcY = sy;
    printerIO.iodrp.io_SrcWidth = sw;
    printerIO.iodrp.io_SrcHeight = sh;
    printerIO.iodrp.io_DestCols = dc;
    printerIO.iodrp.io_DestRows = dr;
    printerIO.iodrp.io_Special = s;
    return(DoIO(&printerIO));
}

int
pWrite(buffer) /* write to the printer */
char *buffer;
{
    int count;
    char *b;

    b = buffer;
    count = 0;
    while (*b++ != '\0') count++;
    /* queue a printer write */
    printerIO.ios.io_Command = CMD_WRITE;
    printerIO.ios.io_Data = buffer;
    printerIO.ios.io_Length = count;
    return(DoIO(&printerIO));
}

int
pRawWrite(buffer) /* write to the printer w/o escape parsing */
char *buffer;
{
    int count;
    char *b;

    b = buffer;
    count = 0;
    while (*b++ != '\0') count++;
    /* queue a printer raw write */
    printerIO.ios.io_Command = PRD_RAWWRITE;
    printerIO.ios.io_Data = buffer;
    printerIO.ios.io_Length = count;
    return(DoIO(&printerIO));
}

pPrtCommand(command, p0, p1, p2, p3) /* perform a printer command */
int command, p0, p1, p2, p3;
{
    /* queue a printer command */
    printerIO.iopc.io_Command = PRD_PRTCOMMAND;
    printerIO.iopc.io_PrtCommand = command;
    printerIO.iopc.io_Parm0 = p0;
    printerIO.iopc.io_Parm1 = p1;
    printerIO.iopc.io_Parm2 = p2;
    printerIO.iopc.io_Parm3 = p3;
    return(DoIO(&printerIO));
}
