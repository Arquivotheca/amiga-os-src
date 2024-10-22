*	   ************************************************
*	   **						 **
*	   **	   --------------------------------	 **
*	   **	    ROM OPERATING SYSTEM EXECUTIVE	 **
*	   **	   --------------------------------	 **
*	   **						 **
*	   **		 Exception Processing		 **
*	   **						 **
*	   ************************************************

*************************************************************************
*									*
*   Copyright 1984,85,88,89,91 Commodore-Amiga, Inc.			*
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
*	$Id: exceptions.asm,v 39.4 92/04/07 21:12:51 mks Exp $
*
*	$Log:	exceptions.asm,v $
* Revision 39.4  92/04/07  21:12:51  mks
* First pass at using real op-codes for 680x0 processors with
* the new HX68  (did some of it already)
* 
* Revision 1.1  92/03/26  13:30:53  mks
* Initial revision
* 
* Revision 39.3  91/12/19  19:49:53  mks
* Added a CachePostDMA_030 to the calls that are patched...
*
* Revision 39.2  91/12/19  09:43:31  mks
* Fixed bug in CachePostDMA and lesser processors ( cpu < 68030 )
*
* Revision 39.1  91/12/13  19:11:03  mks
* Removed (for now) the 68040 setting of the TC register...
*
* Revision 39.0  91/10/15  08:26:56  mks
* V39 Exec initial checkin
*
**********************************************************************


;****** Included Files ***********************************************

	NOLIST
	INCLUDE	"assembly.i"
	INCLUDE	"calls.i"
	INCLUDE	"constants.i"
	INCLUDE	"types.i"
	INCLUDE "nodes.i"
	INCLUDE "lists.i"
	INCLUDE "interrupts.i"
	INCLUDE "libraries.i"
	INCLUDE	"execbase.i"
	INCLUDE "ables.i"
	INT_ABLES
	INCLUDE	"tasks.i"
	LIST


;****** Imported Globals *********************************************


;****** Imported Functions *******************************************

    EXTERN_CODE SysIntr1
    EXTERN_CODE SysIntr2
    EXTERN_CODE SysIntr3
    EXTERN_CODE SysIntr4
    EXTERN_CODE SysIntr5
    EXTERN_CODE SysIntr6
    EXTERN_CODE SysIntr7

    EXTERN_CODE Alert
    EXTERN_CODE FatalProc

    EXTERN_SYS	Supervisor
    EXTERN_SYS	CacheClearU
    EXTERN_SYS	CacheClearE
    EXTERN_SYS	CachePreDMA
    EXTERN_SYS	CachePostDMA
    EXTERN_SYS	CacheControl


;****** Exported Functions *******************************************

    XDEF	ExcTab
    XDEF	Spcl010Excpt
    XDEF	Supervisor
    XDEF	Supervisor000
    XDEF	Supervisor010
    XDEF	CacheClearU
    XDEF	CacheClearE
    XDEF	CacheClearU_020
    XDEF	CacheClearE_020
    XDEF	CachePreDMA
    XDEF	CachePreDMA_040
    XDEF	CachePostDMA
    XDEF	CachePostDMA_030
    XDEF	CacheControl
    XDEF	TypeOfCPU
    XDEF	privTrap10
    XDEF	ColdReboot
    XDEF	GoAway

*----------------------------------------------------------------
*
*   Coldstart Exception Vectors
*
*	These exception vectors are loaded into low memory
*	on coldstart.
*
*----------------------------------------------------------------

LOCFUNC     MACRO
	    DC.W    \1-ExcTab
	    ENDM

EXTFUNC     MACRO
	    DC.W    \1+(*-ExcTab)
	    ENDM


ExcTab: 				; Vect Description
	    LOCFUNC busError		* (02) bus error
	    LOCFUNC addrError		* (03) address error
	    LOCFUNC illglInst		* (04) illegal instruction
	    LOCFUNC zeroDiv		* (05) zero divide
	    LOCFUNC chkInst		* (06) CHK instruction
	    LOCFUNC trapvInst		* (07) TRAPV instruction
	    LOCFUNC privTrap		* (08) privilege violation
	    LOCFUNC trace		* (09) trace
	    LOCFUNC line1010		* (10) line 1010 emulator
	    LOCFUNC line1111		* (11) line 1111 emulator
	    LOCFUNC resvdV12		* (12) not used
	    LOCFUNC resvdV13		* (13) not used
	    LOCFUNC formatErr		* (14) format error 68010
	    LOCFUNC uninitIntr		* (15) uninitialized interrupt
	    LOCFUNC resvdV16		* (16) reserved
	    LOCFUNC resvdV17		* (17) reserved
	    LOCFUNC resvdV18		* (18) reserved
	    LOCFUNC resvdV19		* (19) reserved
	    LOCFUNC resvdV20		* (20) reserved
	    LOCFUNC resvdV21		* (21) reserved
	    LOCFUNC resvdV22		* (22) reserved
	    LOCFUNC resvdV23		* (23) reserved
	    LOCFUNC resvdV24		* (24) reserved
	    EXTFUNC SysIntr1		* (25) autovectors 1
	    EXTFUNC SysIntr2		* (26) autovectors 2
	    EXTFUNC SysIntr3		* (27) autovectors 3
	    EXTFUNC SysIntr4		* (28) autovectors 4
	    EXTFUNC SysIntr5		* (29) autovectors 5
	    EXTFUNC SysIntr6		* (30) autovectors 6
	    EXTFUNC SysIntr7		* (31) non-maskable
	    LOCFUNC Trap0		* (32-47) trap instructions
	    LOCFUNC Trap1
	    LOCFUNC Trap2
	    LOCFUNC Trap3
	    LOCFUNC Trap4
	    LOCFUNC Trap5
	    LOCFUNC Trap6
	    LOCFUNC Trap7
	    LOCFUNC Trap8
	    LOCFUNC Trap9
	    LOCFUNC TrapA
	    LOCFUNC TrapB
	    LOCFUNC TrapC
	    LOCFUNC TrapD
	    LOCFUNC TrapE
	    LOCFUNC TrapF
	    LOCFUNC resvdV48		* (48) reserved
	    LOCFUNC resvdV49		* (49) reserved
	    LOCFUNC resvdV50		* (50) reserved
	    LOCFUNC resvdV51		* (51) reserved
	    LOCFUNC resvdV52		* (52) reserved
	    LOCFUNC resvdV53		* (53) reserved
	    LOCFUNC resvdV54		* (54) reserved
	    LOCFUNC resvdV55		* (55) reserved
	    LOCFUNC resvdV56		* (56) reserved
	    LOCFUNC resvdV57		* (57) reserved
	    LOCFUNC resvdV58		* (58) reserved
	    LOCFUNC resvdV59		* (59) reserved
	    LOCFUNC resvdV60		* (60) reserved
	    LOCFUNC resvdV61		* (61) reserved
	    LOCFUNC resvdV62		* (62) reserved
	    LOCFUNC resvdV63		* (63) reserved
	    DC.W    0	;save 2 bytes


*----------------------------------------------------------------
*
*   Exception Processing
*
*	The following code handles both system and task exceptions.
*	Fatal exceptions will cause a system reset, and will
*	display an error message.Task exceptions will invoke the
*	task's exception handler.  If a task exception occurs while
*	the system is in supervisor mode, a fatal exception will be
*	generated.
*
*---------------------------------------------------------------------

*------ system exception table:

xBase:
	    BSR.S   fatalExcpt		* (00) sysPanic
	    BSR.S   fatalExcpt		* (01) exception when super (fatalTrap)
busError:   BSR.S   spclExcpt		* (02) bus error
addrError:  BSR.S   spclExcpt		* (03) address error
illglInst:  BSR.S   taskExcpt		* (04) illegal instruction
zeroDiv:    BSR.S   taskExcpt		* (05) zero divide
chkInst:    BSR.S   taskExcpt		* (06) CHK instruction
trapvInst:  BSR.S   taskExcpt		* (07) TRAPV instruction
	    BSR.S   taskExcpt		* (08) privilege violation (privViol)
trace:	    BSR.S   taskExcpt		* (09) trace
line1010:   BSR.S   taskExcpt		* (10) line 1010 emulator
line1111:   BSR.S   taskExcpt		* (11) line 1111 emulator
resvdV12:   BSR.S   taskExcpt		* (12) reserved trap
resvdV13:   BSR.S   taskExcpt		* (13) reserved trap
formatErr:  BSR.S   fatalExcpt		* (14) format error
uninitIntr: BSR.S   fatalExcpt		* (15) uninitialized interrupt
resvdV16:   BSR.S   taskExcpt		* (16) reserved trap
resvdV17:   BSR.S   taskExcpt		* (17) reserved trap
resvdV18:   BSR.S   taskExcpt		* (18) reserved trap
resvdV19:   BSR.S   taskExcpt		* (19) reserved trap
resvdV20:   BSR.S   taskExcpt		* (20) reserved trap
resvdV21:   BSR.S   taskExcpt		* (21) reserved trap
resvdV22:   BSR.S   taskExcpt		* (22) reserved trap
resvdV23:   BSR.S   taskExcpt		* (23) reserved trap
resvdV24:   BSR.S   taskExcpt		* (24) reserved trap

SHORT_SR    EQU     4
LONG_SR     EQU     SHORT_SR+8

*---------- system fatal exception:
fatalExcpt:
	    OR.W    #$0700,SR		* disable interrupts
	    SUB.L   #xBase+2,(SP)       * calculate relative offset
	    LSR.W   2(SP)               * exception vector number
	    BRA     FatalProc

*---------- bus and address error exceptions:
Spcl010Excpt:
	    ;------ turn the vector offset into an exception number
	    CLR.L   -(SP)
	    MOVE.W  SHORT_SR+6(SP),2(SP)
	    AND.W   #$FFF,2(SP)
	    LSR.W   2(SP)
	    LSR.W   2(SP)
	    BRA.S   privResume

spclExcpt:
	    SUB.L   #xBase+2,(SP)       * calculate relative offset
	    LSR.W   2(SP)               * exception vector number
	    BTST.B  #5,LONG_SR(SP)      * were we in user mode?
	    BEQ.S   taskTrap
	    BRA     FatalProc


*---------- normal exceptions:
taskExcpt:
	    SUB.L   #xBase+2,(SP)       * calculate relative offset
	    LSR.W   2(SP)               * exception vector number
privResume: BTST.B  #5,SHORT_SR(SP)     * were we in user mode?
	    BEQ.S   taskTrap
	    BRA     FatalProc

*---------- trap instruction exceptions:
trapExcpt:
	    SUB.L   #(tBase+2-$40),(SP) * calculate relative offset
	    LSR.W   2(SP)               * exception vector number
	    BTST.B  #5,SHORT_SR(SP)     * were we in user mode?
	    BEQ.S   taskTrap
	    BRA     FatalProc

tBase:
Trap0:	    BSR.S   trapExcpt		* trap instruction
Trap1:	    BSR.S   trapExcpt		* trap instruction
Trap2:	    BSR.S   trapExcpt		* trap instruction
Trap3:	    BSR.S   trapExcpt		* trap instruction
Trap4:	    BSR.S   trapExcpt		* trap instruction
Trap5:	    BSR.S   trapExcpt		* trap instruction
Trap6:	    BSR.S   trapExcpt		* trap instruction
Trap7:	    BSR.S   trapExcpt		* trap instruction
Trap8:	    BSR.S   trapExcpt		* trap instruction
Trap9:	    BSR.S   trapExcpt		* trap instruction
TrapA:	    BSR.S   trapExcpt		* trap instruction
TrapB:	    BSR.S   trapExcpt		* trap instruction
TrapC:	    BSR.S   trapExcpt		* trap instruction
TrapD:	    BSR.S   trapExcpt		* trap instruction
TrapE:	    BSR.S   trapExcpt		* trap instruction
TrapF:	    BSR.S   trapExcpt		* trap instruction
resvdV48:   BSR.S   trapExcpt		* (48) reserved trap
resvdV49:   BSR.S   trapExcpt		* (49) reserved trap
resvdV50:   BSR.S   trapExcpt		* (50) reserved trap
resvdV51:   BSR.S   trapExcpt		* (51) reserved trap
resvdV52:   BSR.S   trapExcpt		* (52) reserved trap
resvdV53:   BSR.S   trapExcpt		* (53) reserved trap
resvdV54:   BSR.S   trapExcpt		* (54) reserved trap
resvdV55:   BSR.S   trapExcpt		* (56) reserved trap
resvdV56:   BSR.S   trapExcpt		* (56) reserved trap
resvdV57:   BSR.S   trapExcpt		* (57) reserved trap
resvdV58:   BSR.S   trapExcpt		* (58) reserved trap
resvdV59:   BSR.S   trapExcpt		* (59) reserved trap
resvdV60:   BSR.S   trapExcpt		* (60) reserved trap
resvdV61:   BSR.S   trapExcpt		* (61) reserved trap
resvdV62:   BSR.S   trapExcpt		* (62) reserved trap
resvdV63:   BSR.S   trapExcpt		* (63) reserved trap

*----------------------------------------------------------------
*
*   taskTrap -- call the current task's trap handler
*
*   registers:
*	no registers effected
*
*   stack frame (of task's handler):
*	00(SP) == exception number
*	04(SP) == 68000 exception frame
*
*   processor priority
*	no change
*
*----------------------------------------------------------------

taskTrap:
	    MOVEM.L A0/A1,-(SP)         * A1 just for RTS below
	    MOVE.L  SysBase,A0
	    MOVE.L  ThisTask(A0),A0
	    MOVE.L  TC_TRAPCODE(A0),4(SP)
	    MOVE.L  (SP)+,A0
	    RTS


*****o* exec.library/Supervisor ************************************************
*
*   NAME
*	Supervisor -- trap to a short supervisor mode function
*
*   SYNOPSIS
*	result = Supervisor(userFunc)
*	Rx		     A5
*	ULONG Supervisor(void *);
*
*   FUNCTION
*	Allow a normal user-mode program to execute a short assembly language
*	function in the supervisor mode of the processor.  Supervisor() does
*	not modify or save registers; the user function has full access to the
*	register set.  The user function must end with an RTE instruction.
*
*   EXAMPLE
*		;Obtain the Exception Vector base.  68010 or greater only!
*		MOVECtrap:	movec.l VBR,d0	;$4e7a,$0801
*				rte
*
*   INPUTS
*	userFunc - A pointer to a short assembly language function ending
*		  in RTE.  The function has full access to the register set.
*
*   RESULTS
*	result	 - Whatever values the userFunc left in the registers.
*
*   SEE ALSO
*	SuperState/UserState
********************************************************************************
*
* just a comment, may 88, neil: if we do an "illegal" instruction,
* instead of a "privledged" instruction, we could get by with
* a single supervisor call (no special handling needed for 010's).
* the price would be a few more cycles if you are calling Supervisor
* when you are already privledged.
*
* Comment from Bryce. "Supervisor" is a placebo used before the system is
* up.  Either Supervisor000 or Supervisor010 will be stuffed in place before
* tasking starts.
*


;---- Supervisor for 68000 style exception frames ----
Supervisor000:
superEnter: OR.W    #$2000,SR		; cause a priv violation if user mode
	    ;if we make it here, we were already in supervisor mode!
	    PEA     superExit(pc)       ; fake trap stack frame
	    MOVE    SR,-(SP)            ;  "    "     "    "
	    JMP     (A5)

privTrap:   CMP.L   #superEnter,2(SP)
	    BNE.S   notSuper
	    MOVE.L  #superExit,2(SP)	;:OPTIMIZE: Put (SP) here, save RTS
	    JMP     (A5)


; ---- Supervisor for 68010 style exception frames ----
	    CNOP    0,4 	;Nice alignment for 68020
Supervisor010:
	    OR.W    #$2000,SR
	    ;if we make it here, we were already in supervisor mode!
	    SUBQ.L  #8,SP	; Create frame without affecting SR. It's
				; slow, but keeps longword alignment of SSP
	    MOVE    SR,(SP)
	    MOVE.L  #superExit,2(SP)
	    MOVE.W  #$20,6(SP)  ; format 0, vector offset $20
	    JMP     (A5)

privTrap10: CMP.L   #Supervisor010,2(SP)
	    BNE.S   notSuper
	    MOVE.L  #superExit,2(SP)	;:OPTIMIZE: Put (SP) here, save RTS
	    JMP     (A5)

; ---- common functions ----
Supervisor:
superExit:  RTS

notSuper:   OR.W    #$0700,SR		;disable all interrupts
	    MOVE.L  #8,-(SP)            ;fake exception #8 (priv violation)
	    BRA     privResume




*****i* Internal/Exec/TypeOfCPU ***********************************************
*
*	result = TypeOfCPU(),anything
*	d0		     a6
*
*	Test for 68000,68010,68020,68030,68040,68881,68882 CPUs.
*	As soon as we take an exception, this function exits with the
*	current test results in D0.   For the FPU test an FPU register
*	is read and compared with zero.  This should detect boards that
*	don't generate F-line exceptions when there is no FPU.
*
*	The result is a word ready to be placed in ExecBase->AttnFlags
*
*	The 68030 test tries to set the "data cache freeze bit".  If this
*	bit echos, it's an '030.  If it has a CACR, but no I cache, must
*	be a 68040.
*
*	The "Write-Allocate" mode must always be set for use with the Amiga
*	since the cache is not flushed during supervisor <-> user mode
*	context switches.  Without write allocate the following can happen:
*
*		Some user mode code writes to a location and data is cached.
*		Some supervisor mode code writes to the same location.	Since
*		the tag includes some of the function codes, the cache misses,
*		and the cache data is not updated.
*		The stale data is then read by user mode code.
*
*	The instruction cache, instruction burst and write-allocate are
*	enabled by this function.
*
*	The 68040 test is designed to get an '040 to "limp up".  We're working
*	with early chips; not everything works yet.
*
*	WARNING:	Lots of asumptions are made about the exact format
*	WARNING:	of the CACR register.  If new bits are defined in a
*	WARNING:	future CPU, the functions TypeOfCPU and CacheControl
*	WARNING:	will need to change.
*
*******************************************************************************
*
*	A0-old IllegalInstructionVector
*	A1-old SP
*	A2-old Line1111Vector
*	A3-scratch
*	D0-result
*	D1-scratch
*
	;
	; Turn on 68040 assembler mode...
	;
	opt	p=68040

TypeOfCPU:
	MOVEM.L A2/A3,-(SP)
	MOVE.L	IllegalInstructionVector,A0	; save old
	MOVE.L	Line1111Vector,A2
	LEA	cs_illegal(pc),A1       ; set special vectors
	MOVE.L	A1,IllegalInstructionVector
	MOVE.L	A1,Line1111Vector
	MOVE.L	SP,A1		; save stack pointer


;------ 68010 test: Does it have a VBR?
;
;	Note: If >=68010, and someone moved the VBR, then soft rebooted,
;	we come here with the VBR pointing somewhere funny.  For 68000's
;	this code takes an exception via the low memory vectors.  For
;	68010's no exceptions can be taken until after this instruction
;	completes.
;
	CLEAR	D0		; prepare result AND new VBR value
	movec.l	d0,VBR		; Clear VBR...  (Fails if only 68000)
	BSET.L	#AFB_68010,D0
;------ if we made it this far, we are 68010 compatible


;------ Combined 68020/30/40 test: Does it have a CACR?  What bits work?
;
	MOVE.L	#CACRF_EnableI!CACRF_ClearI!CACRF_FreezeD!CACRF_ClearD,D1
	movec.l	d1,CACR			; Set the caches
	movec.l	CACR,d1			; Get results back...
	BSET.L	#AFB_68020,D0		; if we got this far, there is a CACR


	BTST.L	#CACRB_FreezeD,D1
	BEQ.S	cs_Not030
	BSET.L	#AFB_68030,D0		; If it has a D cache, must be 030!
cs_Not030:


	BTST.L	#CACRB_EnableI,D1
	BNE.S	cs_Not040
	OR.W	#AFF_68030!AFF_68040,D0	; Old I cache bit not working? 040
	NPRINTF	300,<'68040 detected - setting safe defaults'>

	cinva	bc		; Caches may be trash

	;
	; This effectively turns off the MMU (if there is one)
	; Since the EC 68040 does not have an MMU, we must only
	; do it via ITT/DTT registers and leave the MMU alone...
	; The 68040 manual states that if the logical address
	; shows as mapped in the ITTx or DTTx registers that
	; the ATC is completely ignored.  Let us hope that they
	; are right.  ;^)
	;
	move.l	#$0000c040,d1	;[Nocache,serialized, 16 meg]
	movec.l	d1,dtt0		; Data below 16meg line is serial non-cached
	move.l	#$00ffc000,d1	;[Cached,Writethrough, 4 gig]
	movec.l	d1,dtt1		; Data above the 16meg line is cached
	movec.l	d1,itt0		; All instructions are cached...
	movec.l	d1,itt1		; (Redundant, just in case ;^)


cs_Not040:
	;68040- Turns on I-Cache
	;680XX- Turns on & Clear I-Cache, Set WA mode
	MOVE.L #CACRF_040_ICache!CACRF_EnableI!CACRF_ClearI!CACRF_WriteAllocate,D1

	movec.l	d1,CACR

;------ end test CPU test



;------ start FPU test
	;
	; The 68040 has a built in, but somewhat different, FPU.  Since
	; it is different, we don't want to set the standard FPU bits
	; at this time.
	;
	btst.l	#AFB_68040,d0
	beq.s	cs_NoFPU40
	;
	; Now, since we do have a 68040, we need to check if it is a
	; real 68040.  That is, the 68EC040 does not have any FPU
	; so the following test is needed to set the FPU40 bit for the
	; 68040...  To do this, we fsave and frestore the FPU.  If
	; this works, we set the FPU40 bit to indicate that the 68040
	; has a working FPU.  If the fsave and frestore cause a
	; exception, the bit will not get set as the exception never
	; returns.
	;
	fsave	-(sp)		; Try to save a stackframe...
	frestore (sp)+		; and restore it...
	bset.l	#AFB_FPU40,d0	; Set the FPU bit...
	bra.s	cs_EndFPU	; End of test...
	;
cs_NoFPU40:
	;
	; We will use FMOVE FPCR,d1 and examine results now
	; this should cause an exception if no '881 is present.
	; Some bad, bad, boards simply terminate the bus cycle,
	; so we check that the result is sane.
	;
	; Since CPU RESET will not clear the FPU registers;
	; I'll write a zero to the FPCR register first. (B4463)
	;
	moveq	#0,d1
	fmove	d1,FPCR			; Write it
	fmove	FPCR,d1			; Get it back...
	tst.l	d1			; was it zero?
	bne.s	cs_EndFPU		; no coprocessor here
	BSET.L	#AFB_68881,D0

	fsave	-(sp)			; Save a 68881 stackframe...
	cmp.b	#$18,1(SP)              ;Check for '881 frame size
	beq.s	cs_Is68881		;(as recommended by Motorola)
	BSET.L	#AFB_68882,D0
cs_Is68881:
	frestore (sp)+

cs_EndFPU:
;------ end FPU test


;This function is executed directly, or hit by the exception.  In either
;case it gets the old SP,restores vectors, then does an RTS.
cs_illegal:
	MOVE.L	A1,SP				* restore stack pointer
	MOVE.L	A0,IllegalInstructionVector	* restore old vector
	MOVE.L	A2,Line1111Vector
	MOVEM.L (SP)+,A2/A3
	RTS

	;
	; Turn off 68040 assembler mode...
	;
	opt	p=68000

****************************************************************************


*****o* exec.library/CacheClearU ******************************************
*
*   NAME
*	CacheClearU - User callable simple Cache clearing (V37)
*
*   SYNOPSIS
*	CacheClearU()
*
*	void CacheClearU();
*
******************************************************************************

*****o* exec.library/CacheClearE ******************************************
*
*   NAME
*	CacheClearE - Cache clearing with extended control (V37)
*
*   SYNOPSIS
*	CacheClearE(address,length,caches)
*	            a0      d0     d1
*
*	void CacheClearE(APTR,ULONG,ULONG);
*
******************************************************************************

*****o* exec.library/CachePostDMA ********************************************
*
*   NAME
*	CachePostDMA - Take actions after to hardware DMA  (V37)
*
*   SYNOPSIS
*	CachePostDMA(vaddress,&length,flags)
*	             a0       a1      d0
*
*	CachePostDMA(APTR,LONG *,ULONG);
*
******************************************************************************
;
;	The 68000 and 68010 have no cache.
;
CacheClearE:
CacheClearU:
CachePostDMA:
			rts


CacheClearU_020:	move.l	#CACRF_ClearD!CACRF_ClearI,d1
			bra.s	cce_Common

CachePostDMA_030:	btst.l	#DMAB_NoModify,d0
			beq.s	cce_GotaDoIt
			rts

cce_GotaDoIt:		move.l	#CACRF_ClearD,d1

CacheClearE_020:
cce_Common:		move.l	a5,a0		;save A5
			lea.l	cce_Super020Func(pc),a5
			JMPSELF	Supervisor	;tail recursion
cce_Super020Func:	btst.b	#AFB_68040,AttnFlags+1(a6)
			bne.s	cce_Super040Func
			and.l	#CACRF_ClearD!CACRF_ClearI,d1
			or.w	#$0700,SR	;DISABLE
			dc.w	$4E7A,$0002	;MOVEC CACR,D0
			or.l	d1,d0
			dc.w	$4E7B,$0002	;MOVEC D0,CACR
			move.l	a0,a5		;restore A5
			rte			;rte restores SR

*
*	Warning: On a 68040 with a copyback data cache enabled, an
*	instruction cache invalidate may not be enough.  Data stored
*	in copyback data cache pages may need to be flushed to memory
*	before the I cache will notice it.
*
*	This is NOT documented in the manuals, but Moto swears it to
*	be true.
*
cce_Super040Func:	btst.b	#CACRB_ClearI,d1
			bne.s	cce_ClearIandD
			dc.w	$f478		;CPUSHA	(DC)
			move.l	a0,a5		;restore A5
			rte
cce_ClearIandD:		dc.w	$f4f8		;CPUSHA	(IC,DC)
			move.l	a0,a5		;restore A5
			rte



*****o* exec.library/CachePreDMA **********************************************
*
*   NAME
*	CachePreDMA - Take actions prior to hardware DMA  (V37)
*
*   SYNOPSIS
*	paddress = CachePreDMA(vaddress,&length,flags)
*	d0                     a0       a1      d0
*
*	APTR CachePreDMA(APTR,LONG *,ULONG);
*
******************************************************************************


CachePreDMA_040:	move.l	a5,a1		;Save A5
			lea.l	cpd_PushDCache(pc),a5
			JSRSELF	Supervisor	;tail recursion
CachePreDMA:		move.l	a0,d0		;Copy input address to return
			rts
cpd_PushDCache:		dc.w	$f478		;CPUSHA	(DC)
			move.l	a1,a5		;Restore A5
			rte



;			dc.w	$f4f8		;CPUSHA	(IC,DC)
;			dc.w	$f4b8		;CPUSHA	(IC)
;			dc.w	$f478		;CPUSHA	(DC)
;
;CacheSync_040:	tst.l	d0
;		bmi.s	cs_everything
;		move.l	d0,a1
;		adda.l	a0,a1		;length+address=end
;		;	a0-base
;		;	a1-end
;		move.l	#PAGESIZE_040-1,d1
;		move.l	a0,d0		;hack
;		and.l	d1,d0		; off
;		move.l	d0,a0		;  extra bits
;		addq.l	#1,d1
;cs_flushem:	dc.w	$f4f0		;CPUSHP	(IC DC),(A0)
;		add.l	d1,a0
;		cmpa.l	a0,a1
;		blo.s	cs_flushem
;		rts
;cs_everything:	dc.w	$f4f8		;CPUSHA (IC DC)
;		rts

*****o* exec.library/CacheControl ********************************************
*
*   NAME
*	CacheControl - Instruction & data cache control
*
*   SYNOPSIS
*	oldBits = CacheControl(cacheBits,cacheMask)
*	D0                     D0        D1
*
*	ULONG CacheControl(ULONG,ULONG);
*
******************************************************************************


		IFEQ	1	;Test for 68040 silicon bug
CacheControl:	btst.b	#AFB_68020,AttnFlags+1(a6)
		beq.s	cc_exit
		move.l	a5,-(sp)
		lea.l	cc_SuperFunc(pc),a5
		JSRSELF	Supervisor
		move.l	(sp)+,a5
cc_exit:	rts

cc_SuperFunc:	or.w	#$0700,SR	;DISABLE
		dc.w	$4E7A,$0002	;MOVEC CACR,D0
		and.l	#CACRF_040_ICache,D0
		dc.w	$4E7B,$0002	;MOVEC D0,CACR
		rte			;rte restores SR
		ENDC




		IFEQ	0
cacheBits	EQUR	d0
cacheMask	EQUR	d1

CacheControl:	movem.l	d2/d3/d4/a5,-(sp)
		moveq	#0,d3
		move.w	AttnFlags(a6),d4
		btst.b	#AFB_68020,d4
		beq.s	cc_exit
		and.l	cacheMask,cacheBits	;destroy irrelevant bits
		or.w	#CACRF_ClearD!CACRF_ClearI,cacheBits ;add required bits
		not.l	cacheMask		;change mask to preserve bits
		lea.l	cc_SuperFunc(pc),a5
		JSRSELF	Supervisor
cc_exit:	move.l	d3,d0
		movem.l	(sp)+,d2/d3/d4/a5
		rts
;
;	d0-mask	d1-bits	d2-scratch d3-result d4-AttnFlags
;
cc_SuperFunc:	or.w	#$0700,SR	;DISABLE
		dc.w	$4E7A,$2002	;MOVEC CACR,D2


		btst.b	#AFB_68040,d4
		beq.s	cc_Not040
				;10987654321098765432109876543210
				;D000000000000000I000000000000000
		swap	d2	;I000000000000000D000000000000000 CACRF_040
		ror.w	#8,d2	;I00000000000000000000000D0000000 CACRF_040
		rol.l	#1,d2	;00000000000000000000000D0000000I CACRF_040
*
* Now, set the burst modes too...  (040 always bursts the cache)
*
		move.l	d2,d3		; Move it over...
		rol.l	#4,d3		; Shift cache info into burst info
		or.l	d3,d2		; Store with the burst bits as needed
cc_Not040:

		;CACR value is normalized to the 020/030 values
		move.l	d2,d3		;Set result
		and.l	cacheMask,d2
		or.l	cacheBits,d2


		btst.b	#AFB_68040,d4
		beq.s	cc_StilNot040
				;10987654321098765432109876543210
				;XXXXXXXXXXXXXXXXXXXXXXXDXXXXXXXI
		ror.l	#1,d2	;IXXXXXXXXXXXXXXXXXXXXXXXDXXXXXXX CACRF_040
		rol.w	#8,d2	;IXXXXXXXXXXXXXXXDXXXXXXXXXXXXXXX CACRF_040
		swap	d2	;DXXXXXXXXXXXXXXXIXXXXXXXXXXXXXXX CACRF_040
		and.l	#CACRF_040_ICache!CACRF_040_DCache,d2 ;!BIT ASUMPTIONS!
		nop			;68040 BUG KLUDGE. Mask 14D43B
		dc.w	$f4f8		;CPUSHA	(IC,DC)
cc_StilNot040:
		nop			;68040 BUG KLUDGE. Mask 14D43B
		dc.w	$4E7B,$2002	;MOVEC D2,CACR
		nop			;68040 BUG KLUDGE. Mask 14D43B
		rte			;rte restores SR
		ENDC

*****o* exec.library/ColdReboot ********************************************
*
*   NAME
*	ColdReboot - reboot the Amiga
*
*   SYNOPSIS
*	ColdReboot()
*
*	void ColdReboot(void);
*
*   FUNCTION
*	Reboot the machine.  All external memory and periperals will be
*	RESET, and the machine will start its power up diagnostics.
*
*	The MagicResetCode must be used EXACTLY as specified here
*	The MagicResetCode must be longword aligned.
*
*   RESULT
*	This function never returns.
*
****************************************************************************
*
*	Note that the system reset used by Alerts reall should flush
*	the caches.  Can't think of a good way to do this, however.
*
ColdReboot:	move.w	#$4000,_intena		;DISABLE
		moveq	#0,d0
		moveq	#-1,d1
		JSRSELF	CacheControl
		lea.l	GoAway(pc),a5		;Location of code to RUN
		jsr	_LVOSupervisor(a6)      ;RUN code in Supervisor mode
	;[NOTE: The jsr is required even though Supervisor never returns.]

MAGIC_ROMEND	    EQU $01000000   ;End of Kickstart ROM
MAGIC_SIZEOFFSET    EQU -$14	    ;Offset from end of ROM to Kickstart size

;-------------- MagicResetCode ---------DO NOT CHANGE-----------------------
		CNOP	0,4			;IMPORTANT! Longword align!
GoAway: 	lea.l	MAGIC_ROMEND,a0 	;(end of ROM)
		sub.l	MAGIC_SIZEOFFSET(a0),a0 ;(end of ROM)-(ROM size)=PC
		move.l	4(a0),a0                ;Get Initial Program Counter
		subq.l	#2,a0			;now points to second RESET
		reset				;first RESET instruction
		jmp	(a0)                    ;CPU Prefetch executes this
		;NOTE: the RESET and JMP instructions must share a longword!
;---------------------------------------DO NOT CHANGE-----------------------

		END
