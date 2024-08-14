**
** $Id: bonus.asm,v 1.6 92/02/27 17:52:39 mks Exp $
**
** The general bonus module.  This contains the hardware specific code
** needed on various machines.
**
**  (C) Copyright 1991,1992 Commodore-Amiga, Inc.
**      All Rights Reserved
**
*******************************************************************************

	SECTION	bonus,code

MACHINE_A3000	equ	1

Main:		move.l	4,a6		; Get execbase...


*
* The A3000 hardware currently only needs the SCRAM test
*
	IFD	MACHINE_A3000
SCRAM	EQU	1
	ENDC
*
* The A1000jr also only needs the SCRAM test
*
	IFD	MACHINE_A1000
SCRAM	EQU	1
	ENDC
*
	INCLUDE	"bonus_rev.i"
*
* If we need scram testing...
*
	IFD	SCRAM
*
*
	INCLUDE "exec/types.i"
	INCLUDE "exec/nodes.i"
	INCLUDE "exec/macros.i"
	INCLUDE	"exec/execbase.i"
	INCLUDE "exec/memory.i"
	INCLUDE "exec/resident.i"
	INCLUDE "v:include/internal/a3000_hardware.i"
*
RAMEND		EQU $08000000
*
initcode:
*
* Now allocate and copy the code to CHIP memory to do the test...
*
		moveq.l	#SCRAMtestE-SCRAMtest,d0	; Test size
		moveq	#MEMF_CHIP,d1			; memory type
		JSRLIB	AllocMem			; Allocate...
		tst.l	d0				; check if we got it
		beq.s	scram_Bail			; if not, we bail out
		move.l	d0,a1				; Get pointers to it
		move.l	d0,a5				; (a5 for supervisor)
		lea.l	SCRAMtest(pc),a0		; Code in ROM...
		moveq	#((SCRAMtestE-SCRAMtest)/2)-1,d0
scram_Copy:	move.w	(a0)+,(a1)+			; Copy it in words...
		dbra	d0,scram_Copy			; keep going...
		JSRLIB	CacheClearU			; Clear the caches...
*
* Now, we will need a 0xFF mask if we are not a 68040 or better...
*
		btst.b	#AFB_68040,AttnFlags+1(a6)
		seq	d3		; d3==0 if 68040...  -1 otherwise
*
* Now, we turn off all caches for a while...
*
		moveq.l	#0,d0		; No caches on...
		moveq.l	#-1,d1		; Do this for all of them
		JSRLIB	CacheControl	; Turn them all off
		move.l	d0,-(sp)	; Store old bits...
*
* Ok, so call the test routine that is now in CHIP memory...
*
		JSRLIB	Supervisor	; (a5/a6 are not trashed)
		move.l	d0,d2		; Store result...
*
* Ok, now turn them on again...
*
		move.l	(sp)+,d0	; Get old bits
		move.l	d0,d1		; All of them
		JSRLIB	CacheControl	; Put them back...
*
* Free the memory...
*
		move.l	a5,a1				; a5 from before...
		moveq.l	#SCRAMtestE-SCRAMtest,d0	; test size
		JSRLIB	FreeMem				; Goodby...
*
* Restore and exit...
*
scram_Bail:	;move.l	d2,d0	; Get result
		;ext.w	d0
		;ext.l	d0
	moveq.l	#0,d0
		rts


*
* Oh me, oh my, what we have to do is not die...
*
* So, we get into this code and we need to make sure that things do
* not die...  (and do not use the stack!)
*
* Register usage:
*	a4	- pointer to the RAMSEY register...
*	a3	- pointer at memory under test...
*	a2	- temp pointer...
*	d4-d7	- saved memory...  (memory under test)
*	d3	- the 68040 flag... (set from above)
*	d2	- General flag mask for RAMSEY...
*	d0-d1	- scrap...
*
* This is the test for Static Column Dynamic RAM (aka "SCRAM").   Quite
* magic and messy, and that's after improvements.  You basically turn on
* the feature, and if all ram stops working, you've got a system without
* SCRAM.  Since the OS may be running from FAST memory we have this
* problem of the code going away if you don't have SCRAM memory so we
* also copy the test to CHIP memory first.  What a pain!
*
* For future compatiblity, Ramsey registers must be accessed in supervisor
* mode.  This code must run DISABLED, and not cause any MMU table searches.
* Chip ram is ok, chip registers or funny timing is not.  Test must be
* complete before the next refresh cycle.  Must never write while in
* "SCRAM" mode on an A1000jr or may fry the hardware.  Must restore
* any values that may have been trashed in the test.  Must not turn on
* "WRAP TERMINATION" if running a 68040.  If 68040, make sure we keep
* the PAGE mode turned on too!  I'm trapped in a fortune cookie
* factory and can't wake up.
*
SCRAMtest:	or.w	#$0700,sr	; DISABLE, rte will restore SR...
*
* First, set up registers...
*
		lea.l	A3000_RamControl,a4
		lea	RAMEND-(512*1024)-(4*4),a3
		moveq.l	#RAMCF_WRAP!RAMCF_BURST!RAMCF_PAGE,d2	; Get mask...
*
* Check if we have a good RAMSEY
*
		cmp.b	#ANCIENT_RAMSEY,A3000_RamseyVersion-A3000_RamControl(a4)
		beq.s	scram_TooOld
*
* Ok, so first get the data from memory (save it) and write in a new set
*
		movem.l	(a3),d4/d5/d6/d7	; Get the old memory
*
* Now write the test pattern (non-scram mode)
*
		move.l	a3,a2			; Get temp pointer...
		move.l	a2,(a2)+		; Write 4 longs...
		move.l	a2,(a2)+		; Write 4 longs...
		move.l	a2,(a2)+		; Write 4 longs...
		move.l	a2,(a2)+		; Write 4 longs...
*
* Ok, now we need to turn on the burst page mode and read back the values...
*
		move.l	d2,d0			; Get full mode mask
		or.b	(a4),d0			; Get the mask...
		nop				; Must sync the bus... !!!!
		move.b	d0,(a4)			; Set the mode...
*
* Now spin until it takes effect...
*
scram_Spin1:	nop				; Sync the bus...
		move.b	(a4),d1			; Get status...
		and.b	d2,d1			; (mask it)
		cmp.b	d2,d1			; They need to match...
		bne.s	scram_Spin1		; Keep waiting for it...
*
* Ok, now read back the test pattern (in scram mode)
*
		move.l	d2,d0			; Get full mode mask
		moveq.l	#4-1,d1			; Number of longs...
scram_TestLoop:	cmp.l	-(a2),a2		; Check it!
		dbne.s	d1,scram_TestLoop	; test result and loop count
		bne.s	scram_NoGood		; If not good, skip
		moveq.l	#RAMCF_WRAP,d0		;YesSCRAM
		and.b	d3,d0			; Mask with 68040 flag...
		addq.l	#RAMCF_PAGE,d0
*
* Ok, now d0 has the inverted scram settings...  So lets set everything to
* what it needs to be...
*
scram_NoGood:	not.l	d0			; Invert d0
		and.b	(a4),d0			; and in the current settings
		nop				; Must sync the bus... !!!!
		move.b	d0,(a4)			; Set the mode...
		and.b	d2,d0			; (mask it)
*
* Now spin until it takes effect...
*
scram_Spin2:	nop				; Sync the bus...
		move.b	(a4),d1			; Get status...
		and.b	d2,d1			; (maske it)
		cmp.b	d0,d1			; They need to match...
		bne.s	scram_Spin2		; Keep waiting for it...
*
* Ok, now restore what was in memory...
*
		movem.l	d4/d5/d6/d7,(a3)	; Restore old memory values
scram_TooOld:	rte
SCRAMtestE:
*
*******************************************************************************
*
* End of SCRAM testing code...
*
	ENDC
*
* The following name the bonus module based on the machine type
*
BonusName:
	IFD	MACHINE_A3000
	dc.b	'A3000 Bonus',0
	ENDC
*
	IFD	MACHINE_A1000
	dc.b	'A1000 Bonus',0
	ENDC
*
* The version string is the same in all of them...
*
BonusID:	VSTRING
		ds.w	0
*
EndTag:		END
