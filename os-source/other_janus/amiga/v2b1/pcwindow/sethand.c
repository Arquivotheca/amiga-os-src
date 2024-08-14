
/* **************************************************************************
 * 
 * Input Handler Setup Routines  --  Zaphod Project
 *
 * Copyright (C) 1988, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * HISTORY    Name         Description
 * ---------  -----------  -------------------------------------------------
 * 25 JAN 88  =	RJ Mical=   Created this file
 *
 * *************************************************************************/

#include "zaphod.h"
#include <exec/devices.h>
#include <devices/input.h>
#include <devices/inputevent.h>



struct MsgPort *InputHandlerPort = NULL;
struct IOStdReq *InputRequestPtr = NULL;
struct Interrupt HandlerData = {0};
BOOL InputHandlerOpen = FALSE;



struct InputEvent *IHandler();



BOOL MakeHandler()
{
	BOOL success;

	InputHandlerPort = (struct MsgPort *)CreatePort(0, 0);
	if (InputHandlerPort == NULL) goto DONE;

	InputRequestPtr = (struct IOStdReq *)CreateStdIO(InputHandlerPort);
	if (InputRequestPtr == NULL) goto DONE;

	if (OpenDevice("input.device", 0, InputRequestPtr, 0) == 0)
		InputHandlerOpen = TRUE;
	else goto DONE;

	HandlerData.is_Data = NULL;
	HandlerData.is_Code = IHandler;
	HandlerData.is_Node.ln_Pri = 51; /* This is greater than Intuition */

	InputRequestPtr->io_Command = IND_ADDHANDLER;
	InputRequestPtr->io_Data    = (APTR)&HandlerData;
 
	/* ... and install the handler */
	DoIO(InputRequestPtr);

	success = TRUE;

DONE:
	if (NOT success) UnmakeHandler();
	return(success);
}



UnmakeHandler()
{
	/* Remove our input handler and device from the chain */ 
	if (InputRequestPtr)
		{
		InputRequestPtr->io_Command = IND_REMHANDLER;
		InputRequestPtr->io_Data    = (APTR)&HandlerData;
		DoIO(InputRequestPtr);
		if (InputHandlerOpen)
			CloseDevice(InputRequestPtr);
		DeleteStdIO(InputRequestPtr);
		}

	if (InputHandlerPort) DeletePort(InputHandlerPort);
}



 
