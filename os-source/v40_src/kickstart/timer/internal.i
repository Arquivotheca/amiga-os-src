*************************************************************************
*									*
*	Copyright (C) 1985, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************


*************************************************************************
*
* internal.i
*
* Source Control
* ------ -------
* 
* $Id: internal.i,v 36.18 91/04/22 13:49:08 darren Exp $
*
* $Locker:  $
*
* $Log:	internal.i,v $
* Revision 36.18  91/04/22  13:49:08  darren
* Remove fields needed for SetICR() in RethinkMicroTimer()
* 
* Revision 36.17  91/04/09  13:44:51  darren
* Device base changes to support MIDI performance enhancements.
* 
* Revision 36.16  91/03/21  15:02:54  darren
* New byte field (0 upon initilization) used to track if interrupts
* for microtic timer have been enabled, or disabled - quick atomic
* bit set/clr used.
* 
* Revision 36.15  91/03/14  17:29:35  darren
* Some new fields to support our jumpy timer.device.
* 
* Revision 36.14  91/01/25  15:46:10  rsbx
* Change to V37
* 
* Revision 36.13  90/04/01  19:12:50  rsbx
* RCS version change.
* 
* Revision 36.12  89/09/18  18:03:16  rsbx
* Unit 2 (EClock) now works as a delta request instead of as a absolute
* time request. Added unit 4, absolute EClock.
* 
* Revision 36.6  89/08/14  18:11:19  rsbx
* Removed some dead fields.
* 
* Revision 36.5  89/08/09  19:25:31  rsbx
* *** empty log message ***
* 
* Revision 36.4  89/08/09  18:10:22  rsbx
* *** empty log message ***
* 
* Revision 36.2  89/08/09  14:06:56  rsbx
* timer.device base changed to reflect rewritten timer.device
* 
*
*************************************************************************

 STRUCTURE JUMPSTRUCT,0
	UWORD	TJ_OPCODE		; space for jmp opcode
	APTR	TJ_STfmt		; address of STfmt code
	LABEL	TJ_SIZE

 STRUCTURE TIMERUNIT,0
	APTR	TU_UNITLIST		; pointer to the unit delay list
	APTR	TU_ADDREQ		; code to insert a timer request
	APTR	TU_REMREQ		; code to remove a timer request
	LABEL	TU_SIZE

 STRUCTURE TIMERDEVICE,LIB_SIZE
	UWORD	TD_PADDING		;

	APTR	TD_SYSLIB		; ptr to exec

	UWORD	TD_TODHERTZ		; ticks/sec from power supply
	UWORD	TD_TIMEPERTOD		; microsecs per tick for power supply
	ULONG	TD_NUMREQS		; # of reqs since last SYSTIME update
	ULONG	TD_ECLOCKHERTZ		; timerB tics/sec
	ULONG	TD_ECLOCKCONST1		; (eclockhertz*64k)/10^6
	ULONG	TD_ECLOCKCONST2		; (64k*10^6)/eclockhertz
	ULONG	TD_LASTTOD		; track last value ReadTOD()
	STRUCT	TD_SYSTIME,TV_SIZE	; base of current system time
**	STRUCT	TD_PREVREQ,TV_SIZE	; time at last GetSysTime
	STRUCT	TD_ECLOCKTIME,EV_SIZE	; monotomically rising eclock time
	STRUCT	TD_UNIT0,TU_SIZE	; unit zero (MicroHertz)
	STRUCT	TD_UNIT1,TU_SIZE	; unit one (VBlank)
	STRUCT	TD_UNIT2,TU_SIZE	; unit two (EClock)
	STRUCT	TD_UNIT3,TU_SIZE	; unit three (WaitUntil)
	STRUCT	TD_UNIT4,TU_SIZE	; unit four (WaitEClock)
	STRUCT	TD_TERMIOQ,MLH_SIZE	; people who need termio's
	STRUCT	TD_ECLOCKLIST,MLH_SIZE	; units 0 & 2 delay list
	STRUCT	TD_VBLANKLIST,MLH_SIZE	; unit 1 delay list
	STRUCT	TD_SYSTIMELIST,MLH_SIZE	; list of pending "wait untils"
	APTR	TD_TODRESOURCE		; cia resource ptr for TOD int
	APTR	TD_COUNTERRESOURCE	; cia resource ptr for 16 counters
	APTR	TD_JUMPYRESOURCE	; ciab resource - jumpy!
	STRUCT	TD_TODINT,IS_SIZE	; TOD alarm interrupt node
	STRUCT	TD_VBLANKINT,IS_SIZE	; VBlank interrupt node
	STRUCT	TD_CIATBINT,IS_SIZE	; cia timer B interrupt node
	STRUCT	TD_CIATJUMP,IS_SIZE     ; Jumpy timer - Cause() interrupt

	APTR	AReq2STTOD		; pointers to the code associated
	APTR	ASTTOD2Req		;  with the below jumpstructs
	APTR	AGetSTTOD		;
	APTR	AEIncSTTOD		;
	APTR	ATODIncSTTOD		;
	APTR	AAdjSTTOD		;

	APTR	AReq2STEClock		;
	APTR	ASTEClock2Req		;
	APTR	AGetSTEClock		;
	APTR	AEIncSTEClock		;
	APTR	ATODIncSTEClock		;
	APTR	AAdjSTEClock		;

	STRUCT	Req2STfmt,TJ_SIZE	; request to internal system time conv
	STRUCT	STfmt2Req,TJ_SIZE	; internal system time to request conv
	STRUCT	GetSTfmt,TJ_SIZE	; get internal system time
	STRUCT	EIncSTfmt,TJ_SIZE	; EClock interrrupt system time increment
	STRUCT	TODIncSTfmt,TJ_SIZE	; TOD interrupt system time increment
	STRUCT	AdjSTfmt,TJ_SIZE	; SetSysTime system time adjustment

	APTR	TD_JUMPYCIACRX 		; cache jumpy control register
	APTR	TD_JUMPYCIATXLO		; cache jumpy timer low

	LABEL	TD_SIZE

