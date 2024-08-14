*	   ************************************************
*	   **						 **
*	   **	   --------------------------------	 **
*	   **	    ROM OPERATING SYSTEM EXECUTIVE	 **
*	   **	   --------------------------------	 **
*	   **						 **
*	   **		Exec-specific startup		 **
*	   **						 **
*	   ************************************************

*************************************************************************
*									*
*   Copyright 1984,85,88,89,90,91 Commodore-Amiga, Inc.			*
*   All rights reserved.  No part of this program may be reproduced,	*
*   transmitted, transcribed, stored in retrieval system, or		*
*   translated into any language or computer language, in any form	*
*   or by any means, electronic, mechanical, magnetic, optical, 	*
*   chemical, manual or otherwise, without the prior written		*
*   permission of Commodore-Amiga, Incorporated.			*
*									*
*************************************************************************

**********************************************************************
*
*   Source Control:
*
*	$Id: startexec.asm,v 39.14 92/11/13 09:13:34 mks Exp $
*
*	$Log:	startexec.asm,v $
* Revision 39.14  92/11/13  09:13:34  mks
* Made unallocated quick interrupts point at an alert.
* 
* Revision 39.13  92/10/09  11:11:54  mks
* Added the clearing of the exception vectors...
*
* Revision 39.12  92/06/08  15:48:15  mks
* Removed conditional INCLUDE_WACK since it no longer exists
*
* Revision 39.11  92/05/28  19:04:02  mks
* Changed NEWLIST a0 to a bsr NewList to save ROM space
*
* Revision 39.10  92/05/20  16:18:56  mks
* Added code to set MEMF_KICK attribute to all memory that is
* available before KickTags are used
*
* Revision 39.9  92/04/09  04:30:33  mks
* Now shares the scanBounds from another location...
*
* Revision 39.8  92/04/07  14:08:07  mks
* Added the storage and initialization of the Alert Timeout
*
* Revision 39.7  92/03/13  10:49:35  mks
* Now stores a colour for graphics.
*
* Revision 39.6  92/02/28  14:37:13  mks
* Removed the colour flashes from exec...
*
* Revision 39.5  92/02/19  15:45:18  mks
* Changed ALERT macro to MYALERT
*
* Revision 39.4  92/01/27  14:25:49  mks
* Removed the CDTV custom exec as it is no longer needed...
* (New magic in CDTV ROM)
*
* Revision 39.3  92/01/21  14:57:01  mks
* Folded in the CDTV_CR stuff...
*
* Revision 39.2  91/12/19  19:53:54  mks
* Added the patch to CachePostDMA so that only 68030 and better
* CPUs have the DCache clearing call.
*
* Some minor code cleanup during the execbase swap
*
* Added the initialization of the MemHandler list...
*
* Revision 39.1  91/10/28  11:30:41  mks
* Added supervisor stack kludge to V39 source tree
*
* Revision 39.0  91/10/15  08:28:56  mks
* V39 Exec initial checkin
*
**********************************************************************


;****** Exported Functions *******************************************

	XDEF	StartExec

;****** Include Files ************************************************

	NOLIST
	INCLUDE "types.i"
	INCLUDE "nodes.i"
	INCLUDE "lists.i"
	INCLUDE "interrupts.i"
	INCLUDE "libraries.i"
	INCLUDE "execbase.i"
	INCLUDE "ables.i"
	TASK_ABLES
	INCLUDE "resident.i"
	INCLUDE "alerts.i"
	INCLUDE "memory.i"
	INCLUDE "tasks.i"

	INCLUDE "constants.i"
	INCLUDE "romconstants.i"
	INCLUDE "assembly.i"
	INCLUDE "calls.i"

	INCLUDE "hardware/cia.i"
	INCLUDE "hardware/custom.i"
	INCLUDE "hardware/intbits.i"
	INCLUDE "hardware/dmabits.i"
	LIST


;****** Imported *****************************************************

	EXTERN_DATA	_ciaapra
	EXTERN_DATA	_custom
	EXTERN_DATA	_color

	EXTERN_DATA	ExecOrigin

	EXTERN_DATA	ExcTab
	EXTERN_DATA	Spcl010Excpt
	EXTERN_DATA	VERNUM
	EXTERN_DATA	REVNUM
	EXTERN_DATA	EXECBASETOTAL
	EXTERN_DATA	MAXSYSFUNC
	EXTERN_DATA	SysLibTab

	EXTERN_STR	TitleStr
	EXTERN_STR	ExecIdStr
	EXTERN_STR	ExecName

;****** Imported Functions *******************************************

	EXTERN_CODE	DfltTaskTrap
	EXTERN_CODE	AddLibrary
	EXTERN_CODE	ExitTask	;Generic task exit code
	EXTERN_CODE	WackInitCode
	EXTERN_CODE	privTrap10
	EXTERN_CODE	Supervisor000
	EXTERN_CODE	Supervisor010
	EXTERN_CODE	InitServers
	EXTERN_CODE	Switch020
	EXTERN_CODE	Launch
	EXTERN_CODE	Launch020
	EXTERN_CODE	FindCodeAfter
	EXTERN_CODE	CacheClearU_020
	EXTERN_CODE	CacheClearE_020
	EXTERN_CODE	CachePreDMA_040
	EXTERN_CODE	CachePostDMA_030
	EXTERN_CODE	NewList
	EXTERN_CODE	Bad_Quick_Int

	EXTERN_SYS	MakeFunctions
	EXTERN_SYS	InitCode
	EXTERN_SYS	SumLibrary
	EXTERN_SYS	AllocMem
	EXTERN_SYS	CopyMem
	EXTERN_SYS	FreeMem
	EXTERN_SYS	AllocEntry
	EXTERN_SYS	AddHead
	EXTERN_SYS	Remove
	EXTERN_SYS	AddTask
	EXTERN_SYS	Alert
	EXTERN_SYS	Debug
	EXTERN_SYS	Supervisor		; for LVO fixup
	EXTERN_SYS	GetCC			; for LVO fixup
	EXTERN_SYS	Switch			; for LVO fixup
	EXTERN_SYS	Dispatch		; for LVO fixup
	EXTERN_SYS	CacheClearU		; for LVO fixup
	EXTERN_SYS	CacheClearE		; for LVO fixup
	EXTERN_SYS	CachePreDMA		; for LVO fixup
	EXTERN_SYS	CachePostDMA		; for LVO fixup

	XREF		scanBounds

******************************************************************************
*
* Start up exec (after cold initialization is complete)
*
*	Exec is the lowest priority RTF_SINGLETASK item.
StartExec:
;------ Find all resident tags
	NPRINTF 10,<'Calling FindCodeAfter'>
	LEA.L	scanBounds(PC),A0
	BSR	FindCodeAfter
	MOVE.L	D0,ResModules(A6)	;Note: tosses old ResModules


*************************o* Swizzle memory ***********************************
;	A6-Temp ExecBase
;	A5-New ExecBase
;

;------ try to get FAST memory for ExecBase
	move.l	#EXECBASETOTAL,d0		;size
	move.l	#MEMF_CLEAR!MEMF_PUBLIC,D1	;flags
	JSRSELF AllocMem
	tst.l	d0
	beq	se_NoBetterMemory
	move.l	d0,a5
	sub.w	#MAXSYSFUNC,a5			;MAXSYSFUNC is negative

;------ put in some static data
	LEA.L	LN_TYPE(a5),A1
	LEA.L	se_exectemplate(PC),A0
	MOVEQ	#((se_exectemplate_end-se_exectemplate)>>1)-1,D0
1$:	MOVE.W	(A0)+,(A1)+
	DBRA	D0,1$

;------ build the function table
	MOVE.L	a5,A0
	LEA.L	SysLibTab(PC),A1
	MOVE.L	A1,A2
	JSRSELF MakeFunctions
	MOVE.W	D0,LIB_NEGSIZE(a5)      ;patch up func table size
	;:TODO: A bit sloppy with the cache here.  The cache has been
	;enabled and flushed, but the cpu specific code not yet in.
	;We depend that new values written to memory are not yet cached.

;------ Copy in select data from temporary ExecBase
;------ :TODO: Make much smaller by block copying.
	move.w	SoftVer(a6),SoftVer(a5)
	move.l	MaxLocMem(a6),MaxLocMem(a5)
	move.l	MaxExtMem(a6),MaxExtMem(a5)
	movem.l LastAlert(a6),d0/d1/d2/a0	; All 4 long words!
	movem.l d0/d1/d2/a0,LastAlert(a5)
	move.w	AttnFlags(a6),AttnFlags(a5)
	move.l	ResModules(a6),ResModules(a5)
	movem.l	KickMemPtr(a6),d0/d1/d2
	movem.l	d0/d1/d2,KickMemPtr(a5)
	NPRINTF 100,<'StartExec KickMem %lx,%lx,%lx'>,d0,d1,d2
	move.l	CoolCapture(a6),CoolCapture(a5)

	lea.l	MemList(a6),a2  ;old list
	lea.l	MemList(a5),a3  ;new list
	bsr.s	new_SwapLists

	lea.l	LibList(a6),a2  ;old list
	lea.l	LibList(a5),a3  ;new list
	; Just fall into the swaplist code...
	;
	; Trick to let the code just fall into here and
	; return out the bottom...
	pea	new_ExecBase(pc)
new_SwapLists:
	move.l	(a2),a0         ;LH_HEAD
	move.l	a0,(a3)         ;LH_HEAD
	move.l	a3,LN_PRED(a0)
	move.l	LH_TAILPRED(a2),a0
	move.l	a0,LH_TAILPRED(a3)
	move.l	a3,(a0)         ;LN_SUCC
	addq.l	#4,(a0)
	rts

new_ExecBase:
	exg.l	a5,a6		;Swap in new ExecBase

	; Store the colour we want to start up with for graphics...
	move.w	#OK_HARDWARE,ex_Pad0(a6)	; Colour to use...

	NPRINTF 19,<'Relocations: Exec: $%lx '>,a6

	;------ supervisor stack
*******************************************************************************
*
* Ugly kludge:  We want to be in fastest memory.  But, we can not just
* allocate the memory since on a CHIP only system, we want to be in the
* highest memory...  So, we try with MEMF_FAST and then, if that does
* not work, we try with MEMF_REVERSE
*
	move.l	#SYSSTK_SIZE,d2
	move.l	#MEMF_CLEAR|MEMF_FAST,d1	;type
*
sstk_again:
	move.l	d2,d0				;size
	JSRSELF AllocMem
	move.l	d0,SysStkLower(a6)
	bne.s	sstk_gotit			; Did we get it?
*
* No fast memory, so try again with CHIP and REVERSE
* If this does not work, we do not care that we get stuck
* since nothing would work anyway...
*
	move.l	#MEMF_CLEAR|MEMF_REVERSE,d1	; type
	bra.s	sstk_again
*
sstk_gotit:
	add.l	d2,d0			;Get top of stack
	move.l	d0,SysStkUpper(a6)
	move.l	d0,SP			;Set up supervisor stack!
	NPRINTF 19,<'Super stack: $%lx '>,SP

	move.l	a5,a1
	moveq	#0,d0
	move.w	LIB_NEGSIZE(a5),d0
	suba.l	d0,a1
	add.w	LIB_POSSIZE(a5),d0
	NPRINTF 20,<'FreeMem $%lx size %lx'>,a1,d0
	JSRSELF FreeMem

;:TODO: - copy resident tag arary
;:TODO: - sort out memory allocation failure paths
; Free stack based on size.
;

;------------------------
; There is no better memory for our use.
;
se_NoBetterMemory:
*********************o* End Swizzle memory ***********************************

;------ set default sysbase pointer & sysbase check pointer
	MOVE.L	A6,ABSEXECBASE	;Location 4
	MOVE.L	A6,D0
	NOT.L	D0
	MOVE.L	D0,ChkBase(A6)

;------- setup system lists:
;
;:TODO: Could be made into a byte table, offset then type.  Would save space
;
se_newList:
	LEA	 ListMakeListList(PC),A1
2$:
	MOVE.W	 (A1)+,D0
	BEQ.S	 se_cont
	LEA.L	 0(A6,D0.W),A0
	bsr      NewList	; (a0 preserved)
	MOVE.W	 (A1)+,D0
	MOVE.B	 D0,LH_TYPE(A0)
	BRA.S	 2$

se_cont:
;------ Clear the user exception vectors (so we can allocate them later)
	move.l	#Bad_Quick_Int,d1
	move.w	#($400-$108)/4-1,d0	; We need to do all of the user vectors
	move.l	#$400,a0		; Point at the vectors...
3$:	move.l	d1,-(a0)		; Store quick vector
	dbra.s	d0,3$			; Do all of them...

;------ setup task defaults:
	LEA	DfltTaskTrap(PC),A0
	MOVE.L	A0,TaskTrapCode(A6)
	MOVE.L	A0,TaskExceptCode(A6)
	MOVE.L	#ExitTask,TaskExitCode(A6)
	MOVE.L	#SYS_SIGALLOC,TaskSigAlloc(A6)
	MOVE.W	#SYS_TRAPALLOC,TaskTrapAlloc(A6)
	MOVEQ	#DEFAULT_QUANTUM,D0
	MOVE.W	D0,Quantum(A6)
	MOVE.W	D0,Elapsed(A6)

;------ setup system exceptions:
	NPRINTF 16,<'setting up exceptions & CPU specific stuff...'>
	LEA	ExcTab(PC),A0           * coldstart exception table
	MOVE.L	A0,A1
	MOVE.W	#BusErrorVector,A2	* (sign extends to 32 bits)
	BRA.S	20$

	;------ do a ExcTab relative load
10$:
	LEA	0(A0,D0.W),A3   ; use A3 as a temp ptr
	MOVE.L	A3,(A2)+                ; assign to memory
20$:
	MOVE.W	(A1)+,D0                ; null terminated table
	BNE.S	10$

;------ Patch Exec vector table for new realities.  Care must be taken
;------ to ensure Exec library properly summed.
	BSET.B	#LIBB_CHANGED,LIB_FLAGS(A6)	; something changed...

	;------ set default GetCC, Supervisor00, LaunchPoint
	;------ Supervisor is a placebo until patched for '000 or '010
	MOVE.L	#$40C04e75,_LVOGetCC(A6)        ; (MOVE.W SR,D0;RTS)
	MOVE.L	#Supervisor000,_LVOSupervisor+2(A6)
	MOVE.L  #Launch,ex_LaunchPoint(A6)

	;------ fix things bus and address errs for 68010 and 68020
	MOVE.W	AttnFlags(A6),D0
	BTST.L	#AFB_68010,D0
	BEQ	se_No010
	;[D0-AttnFlags]

	;------ this must not be a 68000.  set bus and address errors
	LEA	Spcl010Excpt(PC),A0
	MOVE.W	#BusErrorVector,A1
	MOVE.L	A0,(A1)+                ; reset bus error vector
	MOVE.L	A0,(A1)+                ; reset address error vector

	IFNE	A3000_ROMS
	IFLT	INFODEPTH-200
	move.l	#_EasyBusError,BusErrorVector
	ENDC
	ENDC
	IFGE	INFODEPTH-200
	move.l	#_NoisyBusError,BusErrorVector
	ENDC

	;------ set different Supervisor(), GetCC(), and PrivTrap handler
	move.l	#privTrap10,PrivTrapVector
	MOVE.L	#Supervisor010,_LVOSupervisor+2(A6)
	MOVE.L	#$42C04e75,_LVOGetCC(A6)        ; (MOVE.W CCR,D0 RTS)

	;------ set up different cache calls
	;[D0-AttnFlags]
	BTST.L	#AFB_68020,D0		;Also set for 68030/68040
	BEQ.S	se_No020
	MOVE.L	#CacheClearU_020,_LVOCacheClearU+2(A6)
	MOVE.L	#CacheClearE_020,_LVOCacheClearE+2(A6)

	;[D0-AttnFlags]
	btst.l	#AFB_68030,D0		;Also set for 68040
	beq.s	se_NoDCache
	move.l	#CachePostDMA_030,_LVOCachePostDMA+2(a6)

se_NoDCache:
	;[D0-AttnFlags]
	MOVE.W	D0,D1
	AND.W	#AFF_FPU40!AFF_68882!AFF_68881,D1
	BEQ.S	se_NoFPU

	MOVE.L	#Switch020,_LVOSwitch+2(A6)	;Really "switch FPU"
	MOVE.L  #Launch020,ex_LaunchPoint(A6)	;Really "launch FPU"

	IFNE	AFB_PRIVATE-15
	FAIL	'AFP_PRIVATE must be 15 for code to work!!!'
	ENDC
	bset.b	#AFB_PRIVATE-8,AttnFlags(a6)	;Flag "FPU switch in use"

se_NoFPU:
	;[D0-AttnFlags]
	btst.l	#AFB_68040,D0
	beq.s	se_No040
	move.l	#CachePreDMA_040,_LVOCachePreDMA+2(A6)
	dc.w	$f4f8		;CPUSHA	(IC,DC)

se_No040:
	dc.w	$4E7A,$0002	;MOVEC CACR,D0
	or.w	#CACRF_ClearI!CACRF_ClearD,D0
	dc.w	$4E7B,$0002	;MOVEC D0,CACR
se_No020:
se_No010:

	MOVE.L	A6,A1
	JSRSELF SumLibrary	;Fix bad sum bug -Bryce
;------ end vector patching

;------ Set up the memory handler lists...
	lea	ex_MemHandlers(a6),a0
	bsr	NewList		; (a0)

;------ *** What a kludge...  Well, it is the right way to do it...
; What we do here is mark all of the memory that is in the system now
; as MEMF_KICK which means that it can be used for KickMem/KickTag
; memory.  This is not the same as MEMF_LOCAL since MEMF_LOCAL is
; non-reset memory.
	move.l	MemList(a6),d0	; Get MemList start...
se_markMem:
	move.l	d0,a0		; Get address...
	move.l	(a0),d0		; Get next address...
	beq.s	se_markMemDone	; Done if NULL...
	or.w	#MEMF_KICK,MH_ATTRIBUTES(a0)	; Set the bit...
	bra.s	se_markMem
se_markMemDone:

;------ and actually add the library to the system
	MOVE.L	A6,A1
	BSRSELF AddLibrary
	NPRINTF 16,<'exec library has been added!  Setting up interrupts.'>

;------ initialize default interrupt servers:
	BSR	InitServers

;------ enable master DMA and master interrupts:
	NPRINTF 16,<'enabling DMA and interrupts...'>
	MOVE.W	#-1,IDNestCnt(A6) * and TDNestCnt
	LEA	_custom,A0
	MOVE.W	#(DMAF_SETCLR!DMAF_MASTER),dmacon(A0)
	MOVE.W	#(INTF_SETCLR!INTF_INTEN),intena(A0)

;------ setup wackbase and entry traps:
	BSR	WackInitCode

;------ set execbase special checksum:
	CLEAR	D1
	LEA	SoftVer(A6),A0
	MOVE.W	#(ChkSum-2-SoftVer)/2,D0        ;neil checksum fix
se_setchk:
	ADD.W	(A0)+,D1
	DBF	D0,se_setchk
	NOT.W	D1
	MOVE.W	D1,ChkSum(A6)


************************************************************************
*
*	Create System Default Task
*
************************************************************************

*------ hand crafted default task:  (we count on being the first)
	LEA	se_taskMemList(PC),A0
	JSRSELF AllocEntry		* absolutely must succeed !
	MOVE.L	D0,A1
	NPRINTF 30,<'exec.task AllocEntry return $%lx'>,d0

	;------ We check for an error here, but can't do anything about it
	TST.L	D0
	BPL.S	allocEntry_OK

	MYALERT	AG_NoMemory!AO_ExecLib!AT_DeadEnd

allocEntry_OK:
	;------ setup task control block
	; A1 - memlist
	; A2 - TCB for exec.task
	; A0 - Stack Pointer
	MOVE.L	ME_ADDR+ML_SIZE(A1),A0  ; extract memory pointer (SP LOWER)
	LEA	USRSTK_SIZE+8(A0),A2    ; calculate TCB
	MOVE.L	A0,TC_SPLOWER(A2)
	LEA.L	USRSTK_SIZE(A0),A0
	MOVE.L	A0,TC_SPUPPER(A2)
	MOVE.L	A0,TC_SPREG(A2)
	MOVE.L	A0,USP

	MOVE.W	#(NT_TASK<<8)+(0),LN_TYPE(A2) ; Hits LN_PRI also. Fixed old bug.
	MOVE.L	#ExecName,LN_NAME(A2)
	LEA	TC_MEMENTRY(A2),A0
	bsr	NewList		; (a0 - preserved)
	JSRSELF AddHead 	* task memory list


	MOVE.L	A2,A1
	MOVE.L	A2,ThisTask(A6) * Mark exec.task as running
	SUB.L	A2,A2		* clear initialPC
	MOVE.L	A2,A3		* clear finalPC
	JSRSELF AddTask
	NPRINTF 400,<'---- exec.library added to library list'>

	;------ remove from ready list and mark it running:
	MOVE.L	ThisTask(A6),A1
	MOVE.B	#TS_RUN,TC_STATE(A1)
	JSRSELF Remove

	;------ switch to system task:
	AND	#0,SR

	;------ let the good times roll...
	FORBID
	PERMIT
	NPRINTF 2,<'---- let the good times roll... multitasking enabled!'>

************************************************************************
*
*	Initialize all other Coldstart Resident Modules
*
************************************************************************

se_coolCapture:
	MOVE.L	CoolCapture(A6),D0
	BEQ.S	se_initCode
	NPRINTF 4,<'StartExec starting CoolCapture $%lx'>,D0
	MOVE.L	D0,A0
	JSR	(A0)
	NPRINTF 4,<'CoolCapture returned'>

se_initCode:

	;----- resident code initialization:
	NPRINTF 18,<'About to init resident modules...'>
	MOVEQ.L #RTF_COLDSTART,D0	* indicates all coldstart code
	MOVEQ.L #0,D1
	JSRSELF InitCode	;(D0,D1)


************************************************************************
*
*	"Never, never land".  We should never get here.
*
*	InitCode will eventually hit strap.  strap will end up calling DOS.
*	DOS will startup its first Process then RemTask(0), thus killing
*	exec.task.
*
************************************************************************

	MOVE.W	#CC_NOMODULES,_color
	NPRINTF 2,<'After RTF_COLDSTART!!!! (how did we get here?)...'>

	;------ call resident system debugger:
se_debug:
;;	MOVE.L	ABSEXECBASE,A6
;;	JSRSELF Debug		;via vector table
	BRA.S	se_debug


;---------------------------- table area ------------------------------------
;
;	:TODO: Save space
;
MAKELIST    MACRO   * SYMBOL,TYPE
	    DC.W    \1,\2
	    ENDM

ListMakeListList:
;	MAKELIST MemList,NT_MEMORY
	MAKELIST ResourceList,NT_RESOURCE
	MAKELIST DeviceList,NT_DEVICE
;	MAKELIST LibList,NT_LIBRARY
	MAKELIST PortList,NT_MSGPORT
	MAKELIST TaskReady,NT_TASK
	MAKELIST TaskWait,NT_TASK
	MAKELIST IntrList,NT_INTERRUPT
	MAKELIST SoftInts,NT_SOFTINT
	MAKELIST (SoftInts+SH_SIZE),NT_SOFTINT
	MAKELIST (SoftInts+SH_SIZE*2),NT_SOFTINT
	MAKELIST (SoftInts+SH_SIZE*3),NT_SOFTINT
	MAKELIST (SoftInts+SH_SIZE*4),NT_SOFTINT
	MAKELIST SemaphoreList,NT_SIGNALSEM
	DC.W	 0


se_exectemplate:
	;------ definitions for exec library static data
	DC.B	NT_LIBRARY			; LN_TYPE
	DC.B	-100				; LN_PRI
	DC.L	ExecName			; LN_NAME
	DC.B	LIBF_SUMUSED!LIBF_CHANGED	; LIB_FLAGS
	DC.B	0				; LIB_pad
	DC.W	0				; LIB_NEGSIZE
	DC.W	SYSBASESIZE			; LIB_POSSIZE
	DC.W	VERNUM				; LIB_VERSION
	DC.W	REVNUM				; LIB_REVISION
	DC.L	ExecIdStr			; LIB_IDSTRING
	DC.L	0				; LIB_SUM
	DC.W	1				; LIB_OPENCNT
se_exectemplate_end:


se_taskMemList:
	DS.B	LN_SIZE
	DC.W	1
	DC.L	MEMF_PUBLIC!MEMF_CLEAR
	DC.L	TC_SIZE+USRSTK_SIZE+8

;-------------------------- end table area ------------------------------



	IFNE	A3000_ROMS
;----------------------------------------------------------------------------
;
;   "Easy" Bus error handler.
;	    "see no evil, hear no evil, say no evil, no!"
;
_EasyBusError:	bclr.b	#8-8,$a+0(sp)	;clear DF bit
		beq.s	eb_NonData	;Not a data cycle fault...
		btst.b	#6-0,$a+1(sp)
		beq.s	eb_Ignore	;Ignore writes
eb_Kludges:	clr.l	$2c(sp)         ;Nicest read data possilbe
eb_Ignore:	rte			;(exit 1)


;
;   Not a data fault?  Too bad, we'll have to crash you.  The illegal
;   instruction vector points to the same place as bus error used to.
;
eb_NonData:	movem.l a1/a0,-(sp)         ;stack A0 lowest, A1 placeholder
		dc.w	$4e7a,$8801	    ;movec.l vbr,a0
		move.l	IllegalInstructionVector(a0),4(sp)
		move.l	(sp)+,a0
		rts			    ;rts to value we left on stack
	ENDC




	IFGE	INFODEPTH-50

;
;   SLAVE COPY (Old Enforcer, 68851/68030 compatible only)
;
		INCLUDE "libraries/dosextens.i"


STKOFFSET		EQU	8

_NoisyBusError:
_BusError:	movem.l a0/d0,-(sp)
		BLINK			;Toggle power L.E.D.

		moveq	#-1,d0
		move.l	STKOFFSET+$2(sp),d0
		NPRINTF	10,<10,'Program counter     =$%lx'>,d0

		moveq	#-1,d0
		move.l	STKOFFSET+$10(sp),d0
		NPRINTF	10,<'Fault address       =$%lx'>,d0

		movem.l d0-d7/a0-a7,$180
		ZPRINTF 10,<'Address: %08lx %08lx %08lx %08lx %08lx %08lx %08lx %08lx'>,$180
		ZPRINTF 10,<'Data   : %08lx %08lx %08lx %08lx %08lx %08lx %08lx %08lx'>,$1a0

		moveq	#-1,d0
		move.l	STKOFFSET+$18(sp),d0
		NPRINTF	10,<'Data output buffer  =$%lx'>,d0

		moveq	#-1,d0
		dc.w	$4e7a,$0800	;MOVEC.L    USP,D0
		NPRINTF	10,<'User stack pointer  =$%lx'>,d0



		moveq	#0,d0
		move.w	STKOFFSET+$a(sp),d0
		NPRINTF	11,<'Special status word =$%lx'>,d0
		move.w	STKOFFSET+$a(sp),d0
		and.w	#$0100,d0
		beq.s	no_data_fault
		and.w	#$feff,STKOFFSET+$a(sp)        ;Clear DF bit (don't re-run)

		move.w	STKOFFSET+$a(sp),d0
		and.w	#$0040,d0
		beq.s	not_read
		move.l	#$feedface,STKOFFSET+$2c(sp)   ;Clear input data register
not_read:
no_data_fault:

		move.w	STKOFFSET+$a(sp),d0
		btst.l	#6,d0
		bne.s	read
		NPRINTF	10,<' (WRITE)'>
		bra.s	skipread

read		NPRINTF	10,<' (READ)'>
skipread



		moveq	#0,d0
		move.w	STKOFFSET+$0(sp),d0
		NPRINTF	10,<'Status register     =$%lx'>,d0

		move.l	4,a0
		move.l	$114(a0),a0     ;ThisTask
		NPRINTF	10,<'Task address        =%lx'>,a0
		NPRINTF	10,<'Task name           =%s'>,LN_NAME(a0)

		cmp.b	#NT_PROCESS,LN_TYPE(a0)
		bne.s	notcli
		move.l	pr_CLI(a0),d0
		asl.l	#2,d0
		beq.s	notcli
		move.l	d0,a0
		move.l	cli_CommandName(a0),d0
		asl.l	#2,d0
		beq.s	notcli
		addq.l	#1,d0
		move.l	d0,a0
		NPRINTF	10,<'CLI command name    =%s'>,a0
notcli:

		movem.l (sp)+,a0/d0
		rte


	ENDC


	END
