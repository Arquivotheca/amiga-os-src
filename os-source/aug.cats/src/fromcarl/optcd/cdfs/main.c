/***********************************************************************
****************                                        ****************
****************        -=  CDTV FILE SYSTEM  =-        ****************
****************                                        ****************
************************************************************************
***                                                                  ***
***   Written by Carl Sassenrath for Commodore International, Ltd.   ***
*** Copyright (C) 1991 Commodore Electronics Ltd. All rights reserved***                                                                  ***
***                                                                  ***
***         Confidential and Proprietary System Source Code          ***
***                                                                  ***
***********************************************************************/

#include "main.h"
#include "fse.h"	/* ewhac/KMY (2707) */

IMPORT	struct Message *GetMsg();

#include "packets.h"

/***********************************************************************
***
***  Main Entry
***
***	This is the main loop for the file handler.  It
***	accepts and interprets packets for as long as the
***	handler is running.
***
***********************************************************************/
Main()
{
/*	if (FSProc != 0x12344321) Debug0("CDFS DATA SEGMENT MISALIGNED!");*/

	Init();

	while (TRUE)
	{
		WaitPacket();
		if (DispatchAction()) ReplyPacket();
	}
}

/***********************************************************************
***
***  WaitPacket
***
***	Wait for the next packet, but also wake up if someone ejects
***	or inserts a disk.
***
***	ewhac/KMY (2414) (defends against ADDCHANGEINT failing)
***
***********************************************************************/
WaitPacket()
{
	REG struct Message *msg;

again:
	while (!(msg = GetMsg(FSPort)))	
	{
		Wait(1<<(FSPort->mp_SigBit));

		if (InsertFlag && InhibitCount == 0) ReMount(); /* V34.2 Inhib added */
	}

	/*
	 * If the ADDCHANGEINT command fails, it will be returned to FSPort.
	 * This code defends against this eventuality.
	 */
	if ( msg != &DevChgReq )
		Packet = (struct DosPacket *)(msg->mn_Node.ln_Name);
	else
		{
		Debug1( "%s doesn't support ADDCHANGEINT.\r\n", DeviceName );
		goto again;	/*  Look up.  */
		}
}

/***********************************************************************
***
***  DispatchAction
***
***	If we can handle the action, dispatch a function to do so.
***
***********************************************************************/
DispatchAction()
{
	REG UWORD action;
	REG struct s_Actions *a = &Actions[0];

	action = (UWORD)(Packet->dp_Type);
	Debug2("\tA%ld\n",action);	

	/* This message comes from within the FS... */
	if (action == ACTION_DISK_CHANGE) /* CES 21.0 */
	{
		InputPends = FALSE;
		return FALSE; /* Just getting return of packet, do not reply! */
	}

	/* Scan action table */
	while (a->action && a->action != action) a++;

	/* Check for a bad packet action */
	if (!a->action)
	{
		PktRes2 = ERROR_ACTION_NOT_KNOWN;
		PktRes1 = DOS_FALSE;
		return TRUE;
	}

	/* Convert and cache args (in data segment) */
	ConvertArgs(a->argflags);

	Debug2("\ta%lx.%lx.%lx\n",PktArg1,PktArg2,PktArg3);	

	/* ewhac/KMY (2707/2721) */
	FSE( FSEF_PACKETS, (LONG) action, PktArg1, PktArg2, PktArg3 );

	/* Execute action function: */
	PktRes2 = NULL;
	PktRes1 = a->func();

	return TRUE;
}

/***********************************************************************
***
***  ReplyPacket
***
***	Provide DOS's screwy method of replying to the packet.
***	Why did I trust Tim King?
***
***********************************************************************/
ReplyPacket()
{
	REG DOSPKT *pkt = Packet;
	REG struct Message *msg;
	REG struct MsgPort *port;

	MUST(pkt);
	MUST(msg = pkt->dp_Link);
	MUST(port = pkt->dp_Port);
	pkt->dp_Res1 = PktRes1;
	pkt->dp_Res2 = PktRes2;
	pkt->dp_Port = FSPort; 
	msg->mn_Node.ln_Name = (char *)pkt;
	PutMsg(port,msg); 

	Debug2("\tR%lx.%lx\n",PktRes1,PktRes2);
}
