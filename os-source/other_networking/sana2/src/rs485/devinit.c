
/*
 * rs485Init/rs485Open/rs485Close/rs485Expunge
 *
 * These functions perform device specific processing for their counterparts
 * in libinit.c
 */

#ifndef RS485_H
#include "rs485.h"
#endif

/*
 * OpenLibrary() processing
 */
void __saveds
rs485Open(struct rs485Device *libbase, struct IOSana2Req *iob,
	     long unit, long flags)
{
struct rs485Unit *up;
struct buffmgmt *functions;

	functions = NULL;
/*
**  fail if asked for promicuous mode
*/
	if(flags & SANA2OPB_PROM){
		iob->ios2_Req.io_Error = S2ERR_NOT_SUPPORTED;
		return;
	}
	/* init unit list if first time through */
	if(!rs485Units.lh_Head){
		NewList(&rs485Units);
	}
/*
**  discover what boards are in the system
*/
	iob->ios2_Req.io_Error = getboards();
	if(iob->ios2_Req.io_Error != S2ERR_NO_ERROR){
		return;
	}
/*
**  find requested unit on internal list
*/
	FORALL(&rs485Units, up, struct rs485Units *)
	{
		if(up->unitnum == unit)
		{
			break;
		}
	}
	if(!up->node.ln_Succ)
	{
		iob->ios2_Req.io_Error = IOERR_OPENFAIL;
		return;
	}
/*
**  if sole ownership requested or in progress, fail open
*/
	if(((flags | up->flags) & SANA2OPF_MINE) && (up->opencnt != 0))
	{
		iob->ios2_Req.io_Error = IOERR_UNITBUSY;
		goto quit;
	}
/*
**  Alloc Buffer Management Function structure
*/
	if( !(functions = (struct buffmgmt *)AllocMem(sizeof(struct buffmgmt), 0L)) )
	{
		iob->ios2_Req.io_Error = IOERR_OPENFAIL;
		goto quit;
	}
/*
**  Parse TagList, if OK, fill Buffer Management structure
*/
	if(!UtilityBase)
	{
		UtilityBase = OpenLibrary("utility.library", 0L);
		if(!UtilityBase)
		{
			iob->ios2_Req.io_Error = IOERR_OPENFAIL;
			goto quit;
		}
	}
#undef CopyToBuff
#undef CopyFromBuff
	functions->CopyToBuff	= (buffnc)
	   GetTagData(S2_CopyToBuff, NULL, iob->ios2_BufferManagement);
	functions->CopyFromBuff = (buffnc)
	   GetTagData(S2_CopyFromBuff, NULL, iob->ios2_BufferManagement);
	if( !(functions->CopyToBuff && functions->CopyFromBuff) )
	{
		iob->ios2_Req.io_Error = IOERR_OPENFAIL;
		goto quit;
	}
	iob->ios2_BufferManagement = functions;
/*
**  do basic setup of board
*/
	up->libbase = libbase;
	iob->ios2_Req.io_Error = initboard(up);
	if(iob->ios2_Req.io_Error != S2ERR_NO_ERROR)
	{
		goto quit;
	}
/*
**  start unit task, if necessary
*/
	iob->ios2_Req.io_Error = starttask(up);
	if(iob->ios2_Req.io_Error != S2ERR_NO_ERROR)
	{
		goto quit;
	}
/*
**  instantiate int server, if necessary
*/
	iob->ios2_Req.io_Error = interrupts_on(up);
	if(iob->ios2_Req.io_Error != S2ERR_NO_ERROR){
		goto quit;
	}
	up->flags |= flags;
	up->opencnt++;
	UNIT(iob) = up;

quit:	if(iob->ios2_Req.io_Error != S2ERR_NO_ERROR && up->opencnt == 0)
	{
		if(functions)
			FreeMem(functions, sizeof(struct buffmgmt));
		interrupts_off(up);
		stoptask(up);
	}
}


/*
** device specific close processing; if OpenCnt == 0, no reason to keep Util
** Lib open. Take away unit pntr from user, in case a later mistake is made..
**/
void __saveds
rs485Close(struct rs485Device *libbase, struct IOSana2Req *iob)
{
	iob->ios2_Req.io_Error = S2ERR_NO_ERROR;
	if(UNIT(iob))
	{
		UNIT(iob)->opencnt--;
		FreeMem(iob->ios2_BufferManagement, sizeof(struct buffmgmt));
		if(UtilityBase && libbase->ml_Lib.lib_OpenCnt == 0)
		{
			CloseLibrary(UtilityBase);
			UtilityBase = NULL;
		}
		UNIT(iob) = NULL;
	}
}


/*
** expunge processing: free unit structures.  Only called if lib opencnt == 0.
*/
void __saveds
rs485Expunge(struct rs485Device *libbase)
{
struct rs485Unit *up, *next;

	up = (struct rs485Unit *)rs485Units.lh_Head;
	while(up->node.ln_Succ)
	{
		next = (struct rs485Unit *)up->node.ln_Succ;
		shutupboard(up);
		stoptask(up);
		interrupts_off(up);
		free_alltracked(up);
		freeunit(up);
		up = next;
	}
	NewList(&rs485Units);
}
