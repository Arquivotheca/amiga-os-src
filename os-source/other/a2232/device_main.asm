**
**	$Header: Work:src/cerial/RCS/device_main.asm,v 1.1 91/08/09 21:24:17 bryce Exp $
**
**	Generic device driver: Init/Open/Close/Expunge code
**
**	(C) Copyright 1989 Commodore-Amiga, Inc.
**	    All Rights Reserved
**
	SECTION main,CODE

;---------- Includes --------------------------------------------------------
	IFND	PREASSEMBLED_INCLUDES
	NOLIST
	INCLUDE "exec/types.i"
	INCLUDE "exec/nodes.i"
	INCLUDE "exec/lists.i"
	INCLUDE "exec/libraries.i"
	INCLUDE "exec/io.i"
	INCLUDE "exec/errors.i"
	LIST
	INCLUDE "exec/alerts.i"
	NOLIST
	XREF	_LVOAlert
	INCLUDE "intuition/preferences.i"
	LIST
	ENDC

	INCLUDE "device_base.i"
	INCLUDE "macros.i"

;---------- Exports ---------------------------------------------------------
	XDEF Device_ColdInit	;Callback for RTF_AUTOINIT ROMTag
	XDEF Device_Open	;Open unit
	XDEF Device_Close	;Close unit
	XDEF Device_Expunge	;Memory panic
	XDEF Device_Null	;Future use
	XDEF do_IntuiName

;---------- Domestic imports ------------------------------------------------
	XREF dataTable
	XREF funcTable

;---------- Foreign imports -------------------------------------------------
	XREF _MD_ColdInit
	XREF _MD_Expunge
	XREF _MD_Open
	XREF _MD_Close
	XREF _MD_BeginIO
	XREF _MD_AbortIO



*****i* multidev/Device_Init ************************************************
*
*   NAME
*	initRoutine - Called from a ROMTag for the first OpenDevice()
*
*   FUNCTION
*	Start the device from scratch.	The system will guarantee
*	single-threading of this call.
*
*	If this function needed to fail, it would have to free the device
*	base allocted by RTF_AUTOINIT.	We just return "sucess" and let
*	expunge clear us out.
*
*   INPUTS
*	D0 - device pointer (if RTF_AUTOINIT was used in the ROMTag)
*	A0 - SegList (if loaded from disk)
*	A6 - Exec
*
*    RESULTS
*	D0 - Return zero to be unloaded
*	     Return the device pointer if all is ok
*
********************************************************************************
*
*  REGISTERS
*	A3-stash of SegList
*	A5-DeviceBase
*
Device_ColdInit:
		PRINTF	2,<10,'----Generic Init. DeviceBase at %lx----'>,d0
		PUSHM	a2/d2/a5
		move.l	d0,a5
		move.l	a0,md_SegList(a5)
		move.l	a6,md_SysBase(a5) ;For fast non-chip access

		;[a5-DeviceBase  a6-ExecBase.  a2/d2 scratch]
		JSR	_MD_ColdInit	 ;Device-specific cold init

		POPM
		rts		    ;(exit)



*****i* multidev/Device_Open ************************************************
*
*   NAME
*	Open - Given a blank IORequest, open the device.
*
*   SYNOPSIS
*	Error = OpenDevice(unit,ioRequest,flags),base
*	D0		   D0	A1	  D1	 A6
*
*	BYTE OpenDevice(ULONG,struct ioRequest,ULONG);
*
*   FUNCTION
*	Called from exec/OpenDevice.
*	Open the device.  ramlib will single-thread the call.  Currently
*	Forbid() is used (but this may change).
*
*   RESULTS
*	io_Device   - Set this to point to this device (-1 for error)
*	io_Unit     - Set this to a unit-specific pointer
*	io_Error    - return error condition code here.  Zero indicates ok
*
*   INPUTS
*	D0 - unit
*	D1 - flags
*	A1 - IORequest
*	A6 - DeviceBase
*
*   RESULTS
*	d0 - Zero means ok, else return an error code (exec/errors.i).
****************************************************************************
*	A6 - Exec
*	A5 - DeviceBase
*	A1 - IORequest
*	D1 - Flags

Device_Open:	PRINTF	49,<'%s ($%lx) unit %ld open'>,LN_NAME(A6),A6,D0
		PRINTF	50,<',flags $%lx,IORequest $%lx'>,D1,A1



		PUSHM	a5/a6
		move.l	md_SysBase(a6),a5
		exg.l	a5,a6



		;
		;   Something on this task's schedule could call AllocMem(),
		;   which could loop back to Expunge()
		;
		addq.w	#1,LIB_OPENCNT(a5)  ;Prevent accidental expunge



		;
		; reroute to the preference's "default" unit
		;
		tst.l	d0
		bne.s	do_NotDefault
		movem.l d1/a1,-(sp)
		lea.l	-pf_SIZEOF(sp),sp
		  moveq   #LIBRARY_MINIMUM,d0
		  lea.l   do_IntuiName(pc),A1
		  JSRLIB  OpenLibrary
		  tst.l   d0
		  beq.s   do_NoIntui	  ;D0=0
		  move.l  sp,a0 	  ;pf_SIZEOF bytes of storage
		  move.l  d0,a6
		  move.l  #pf_SIZEOF,D0
		  JSRLIB  GetPrefs	  ;If opened, get Pref baud offset
		  move.l  a6,a1
		  move.l  md_SysBase(a5),a6 ;restore A6
		  JSRLIB  CloseLibrary
		  moveq   #0,d0
		  move.b  pf_RowSizeChange-1(sp),d0   ;Get new unit number
do_NoIntui:	lea.l	pf_SIZEOF(sp),sp
		movem.l (sp)+,d1/a1
		addq.l	#1,d0			    ;prefs offsets by one
		PRINTF	20,<'--preferences default unit %lx--'>,d0
do_NotDefault:	;
		;[d0-unit number,d1-flags,a1-IORequest]
		;



		;
		; serial.device pass-through kludge ("unit 1")
		; d0 is non-zero
		;
		subq.l	#1,d0
		bne.s	do_RegularUnit
		lea.l	do_SerialName(pc),a0    ;"serial.device",0
		;[A1-IORequest]
		;[D0-unit number <=zero>]
		;[D1-flags]
		JSRLIB	OpenDevice  ;a0-name, a1-IORequest, d0-unit, d1-flags
		bra	do_exit
do_RegularUnit: ;
		;[d0-(unit number-1)]
		;



		;
		; Normal multiserial open
		;
		subq.l	#1,d0
		move.b	#IOERR_OPENFAIL,IO_ERROR(a1)
		cmp.l	md_NumUnits(a5),d0
		bhs.s	do_RangeBad

		tst.l	md_UnitArray(a5)
		beq.s	do_noopen	    ;Zombie mode...

		exg.l	a0,d1		    ;Exchange scratch for flags
		move.w	md_UnitSize(a5),d1  ;Bytes per array entry
		mulu.w	d0,d1		    ;Generate offset into UnitArray
		add.l	md_UnitArray(a5),d1 ;Add in base pointer
		exg.l	a0,d1		    ;restore flags, unit <EA> in A0
		move.l	a0,IO_UNIT(a1)      ;Set unit <ea>
		move.l	a5,IO_DEVICE(a1)    ;Default device pointer

		;[A1-IORequest D1-flags A5-Device A6-SysBase d0=IO_ERROR]
		move.l	a1,-(sp)
		JSR	_MD_Open	    ;Device-specific open
		move.l	(sp)+,a1
		move.b	d0,IO_ERROR(a1)
		bne.s	do_noopen
		addq.w	#1,LIB_OPENCNT(a5)  ;Mark device as open (for real)

do_exit:	PRINTF	20,<'OpenDevice returns %lx'>,d0
		subq.w	#1,LIB_OPENCNT(a5)  ;drop expunge protection counter
		POPM
		moveq	#0,d0	    ;Don't unload ME!
		rts	;(exit)



;-----On open failure, make request unusable, decrement OPENCNT twice-----
do_RangeBad:	PRINTF	20,<'--Bad unit number (%ld)--'>,d0
do_noopen:	moveq	#-1,d1
		move.l	d1,IO_DEVICE(a1)    ;trash pointer
		move.l	d1,IO_UNIT(a1)      ;trash pointer
		bra	do_exit

do_SerialName:	dc.b	'oldser.device',0
do_IntuiName:	dc.b	'intuition.library',0
		CNOP	0,2


*****i* multidev/Device_Close ***********************************************
*
*   NAME
*	Close - Given a opened IORequest, close the device.
*
*   SYNOPSIS
*	SegList = Close(ioRequest),base
*	D0		A1	   A6
*
*	BPTR Close(struct ioRequest *);
*
*   FUNCTION
*	Called from exec/CloseDevice.
*	Close the device.  ramlib will single-thread the call.
*
*   RESULTS
*	io_Device   - destroy
*	io_Unit     - destroy
*	io_Error    - return error condition code here.  Zero indicates ok
*	d0 -	Return NULL to stay.  Return the SegList to be unloaded.
*		Note that if you do return the SegList, you will be
*		immediatly unloaded.  It is usually better to always return
*		zero here, and let Expunge do the unloading when memory is
*		low.
****************************************************************************
*
*   A5 - DeviceBase
*   A6 - ExecBase
*
*   If LIB_OPENCNT==1, _MD_Close may take special action.
*

Device_Close:	PRINTF	50,<'%s close. A1=%lx'>,LN_NAME(A6),A1

		PUSHM	a5/a6
		move.l	md_SysBase(a6),a5
		exg.l	a5,a6

		clr.b	IO_ERROR(a1)
		;[A5-DeviceBase  A6-SysBase  A1-IORequest]
		move.l	a1,-(sp)
		JSR	_MD_Close
		move.l	(sp)+,a1

;---- invalidate closed IORequest
		moveq	#-1,d0
		move.l	d0,IO_DEVICE(a1)    ;trash pointer
		move.l	d0,IO_UNIT(a1)      ;trash pointer
		subq.w	#1,LIB_OPENCNT(a5)  ;Mark closed after expunge danger
		bvc.s	cl_Ok
		ALERT	AN_Unknown!AG_CloseDev!AO_Unknown  ;too many closes
cl_Ok:		moveq	#0,d0	    ;Don't unload ME!
		POPM
		rts



*****i* multidev/Device_Expunge *********************************************
*
*   NAME
*	Expunge - Called during system memory panic.
*
*   SYNOPSIS
*	segList = Expunge(deviceBase)
*	D0		  A6-(scratch)
*
*	BPTR Expunge(struct Device *);
*
*   FUNCTION
*	This vector is called when the system is in a memory panic, and
*	needs to free space.  Do whatever can be done to free up memory.
*	If the device is currently closed, the device should be removed
*	from the list, the memory should be freed, and finally the
*	SegList should be returned.
*
*	In most cases the device should have already freed all resources
*	at the time of the last Close.
*
*   WARNING
*	This routine must be fast, and as it is called from the allocator,
*	must never Wait(), or call anything that might eventually call
*	Wait(). This call is guaranteed to be single-threaded.
*
*    RESULTS
*	d0 - Return zero to stay.  Return the SegList to be unloaded.
****************************************************************************

Device_Expunge: IFGE	DEBUG_DETAIL-4
		MOVE.W	LIB_OPENCNT(A6),D0  ;Debug
		EXT.L	D0
		PRINTF	4,<'%s expunge.  Open count %lx'>,LN_NAME(A6),D0
		ENDC
		moveq	#0,d0		    ;Return code
		tst.w	LIB_OPENCNT(a6)     ;Everyone out?  Expunge it...
		beq.s	ex_FlushIt
		rts



ex_FlushIt:	PUSHM	a5/a6
		move.l	md_SysBase(a6),a5
		exg.l	a5,a6

		;[A5-DeviceBase A6-ExecBase]
		JSR	_MD_Expunge

		move.l	a5,a1
		JSRLIB	Remove			;Remove device node

		move.l	md_SegList(a5),-(sp)
		moveq	#0,d0
		move.w	LIB_NEGSIZE(a5),d0
		move.l	a5,a1
		suba.l	d0,a1			;DeviceBase-NegSize=MemoryBase
		add.w	LIB_POSSIZE(a5),d0      ;NegSize+PosSize=MemorySize
		JSRLIB	FreeMem
		move.l	(sp)+,d0                ;get back seglist

		POPM
		rts



*****i* multidev/Device_Null ***********************************************
*
*   NAME
*	Null - return zero
*
*   SYNOPSIS
*	zero = Null()
*	D0
*
*	ULONG Null(void);
*
*   FUNCTION
*	Return zero.
*
*****************************************************************************

Device_Null:
		PRINTF	2,<'%s Device_Null point called!!!'>,LN_NAME(A6)
		moveq	#0,d0
		rts

		END
