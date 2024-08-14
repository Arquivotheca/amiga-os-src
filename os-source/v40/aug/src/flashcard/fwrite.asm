******************************************************************
**
** Flash ROM support code
**
******************************************************************

	INCLUDE	"exec/types.i"
	INCLUDE	"exec/macros.i"
	INCLUDE	"exec/ables.i"
	INCLUDE	"exec/memory.i"

	INCLUDE "hardware/intbits.i"
	INCLUDE "hardware/cia.i"

	INCLUDE "resources/card.i"
	INCLUDE	"debug.i"

	XREF	_intena
	XREF	_CheckAbort

EXECBASE	EQU	4

  STRUCTURE ZoneHandle,0
	APTR	zh_to
	APTR	zh_from
	ULONG	zh_size
	APTR	zh_timer
	APTR	zh_signals
	ULONG	zh_sigmask
	APTR	zh_cv
	LABEL	ZoneHandle_SIZEOF


DELAY		MACRO	;us
		move.w	#((\1*100)/139),d1
		bsr	BusyDelay
		ENDM

TENUS	EQU	(((10)*100)/139)
SIXUS	EQU	(((6)*100)/139)

** FlashROM commands

WRITE		EQU	$40
WRITEVERIFY	EQU	$c0
ERASE		EQU	$20
ERASEVERIFY	EQU	$a0
READMODE	EQU	$00

**
** Stabilize 1ms after Vpp change
**
 		XDEF	_Stabilize1MS
_Stabilize1MS:

	movea.l	a2,a1
	move.l	4(sp),a2
	bsr.s	BusyDelay
	movea.l	a1,a2
	rts



** Delay function for longer delays
**
** void BusyDelay( ciaticks )
**		   D1
**
BusyDelay:
	movem.l	d0/d1/a0,-(sp)
	move.w	d1,d0
	lsr.w	#8,d0
	move.l	zh_timer(a2),a0

	move.b	d1,(a0)			;timer low
	move.b	d0,$100(a0)		;timer high (and start)

BusyWaitTimer:

	tst.b	(a0)			;wait 2.8us, long enough to count once
	tst.b	(a0)

	cmp.b	(a0),d1			;check low byte
	bne.s	BusyWaitTimer

	cmp.b	$100(a0),d0		;then high
	bne.s	BusyWaitTimer

	movem.l	(sp)+,d0/d1/a0
	rts


** Erase a zone (variable size)
**
** success = EraseZone(zh)
** D0
**
	XDEF	_EraseZone

_EraseZone:

* zh_from points to zone of 0's for initial write of 0's before erase

	PRINTF	DBG_ENTRY,<'EraseZone'>


	move.l	4(sp),d0
	movem.l	d2-d7/a2-a6,-(sp)
	move.l	d0,a2
	
	move.l	zh_signals(a2),a4
	move.l	zh_sigmask(a2),d6
	swap	d6

	move.l	zh_timer(a2),a3

	moveq	#0,d3			;assume fail

	moveq	#2,d4
	moveq	#0,d5

	move.w	#3000,d2		;plscnt

	moveq	#SIXUS,d1		;6us delay

	move.l	EXECBASE,a6

EraseOddBytes:

	move.l	zh_to(a2),a1
	add.w	d5,a1

	move.l	zh_size(a2),d0
	lsr.l	#1,d0			;do even bytes

* erase even bytes first 

EraseAgain:

* Pig!! But very safe way to erase for 10milliseconds


	DISABLE

	move.b	#ERASE,(a1)
	move.b	#ERASE,(a1)

	DELAY	10000			;10000 us max

	move.b	#ERASEVERIFY,(a1)

	ENABLE
	
	bra.s	VerifyAfterErase

EraseVerifyNext:

	move.b	#ERASEVERIFY,(a1)

VerifyAfterErase:

	move.b	d1,ciatalo(a3)
	move.b	#0,$100(a3)		;start

	tst.b	(a3)
	tst.b	(a3)

wait6us:
	cmp.b	(a3),d1
	bne.s	wait6us

	cmp.b	#$FF,(a1)
	bne.s	EraseByteRetry

	adda.w	d4,a1			;skip every other byte

	move.w	(a4),d7			;check for signals
	and.w	d6,d7
	bne.s	EraseCheckAbort

EraseContinue:

	subq.l	#1,d0
	bne.s	EraseVerifyNext

	add.b	#1,d5
	cmp.b	#2,d5
	bcs.s	EraseOddBytes

	moveq	#01,d3			;success

	bra.s	EraseWriteDone

EraseByteRetry:

	subq.w	#1,d2
	bne.s	EraseAgain

	moveq	#00,d3			;failed

* restore default Vpp

EraseWriteDone:

	move.l	zh_to(a2),a0
	move.b	#READMODE,(a0)		;set read mode

EraseFailure:
	move.l	d3,d0
	movem.l	(sp)+,d2-d7/a2-a6

EraseWriteFailure:

	PRINTF	DBG_EXIT,<'%ld=EraseZone()'>,D0

	rts				;return (TRUE | FALSE)



**
** EraseCheckAbort -- get out for a moment to check for Aborted operation
**

EraseCheckAbort:

	movem.l	d0-d1/a0-a1/a5-a6,-(sp)

	move.l	zh_cv(a2),a1

	jsr	_CheckAbort

	tst.l	d0
	bne.s	AbortErase

	movem.l	(sp)+,d0-d1/a0-a1/a5-a6
	bra.s	EraseContinue

AbortErase:
	movem.l	(sp)+,d0-d1/a0-a1/a5-a6
	moveq	#00,d3
	bra.s	EraseFailure


**
** WriteCheckAbort -- get out for a moment to check for Aborted operation
**

WriteCheckAbort:

	movem.l	d0-d1/a0-a1/a5-a6,-(sp)

	move.l	zh_cv(a2),a1

	jsr	_CheckAbort

	tst.l	d0
	bne.s	AbortWrite

	movem.l	(sp)+,d0-d1/a0-a1/a5-a6
	bra.s	WriteContinue

AbortWrite:
	movem.l	(sp)+,d0-d1/a0-a1/a5-a6
	moveq	#00,d3
	bra	WriteDone

** Write a zone (variable size)
**
** success = WriteZone(zh)
** D0
**
	XDEF	_WriteZone

_WriteZone:
	move.l	4(sp),d0

	PRINTF	DBG_ENTRY,<'WriteZone'>

	movem.l	d2-d7/a2-a6,-(sp)

	move.l	d0,a2

	move.l	zh_timer(a2),a3

	move.l	zh_signals(a2),a4
	move.l	zh_sigmask(a2),d6
	swap	d6

	moveq	#1,d3			;true

	move.l	zh_from(a2),a0
	move.l	zh_to(a2),a1

	move.l	zh_size(a2),d0

	moveq	#SIXUS,d1		;6us delay

WriteLoop:

	move.w	(a4),d7			;check for signals
	and.w	d6,d7
	bne.s	WriteCheckAbort

WriteContinue:

	subq.l	#1,d0
	bmi.s	WriteDone

	moveq	#(25-1),d2		;retry count

WriteRetry:
	move.b	#WRITE,(a1)		;command write
	move.b	(a0),(a1)		;data

	move.b	#TENUS,(a3)
	move.b	#0,$100(a3)		;start

	tst.b	(a3)
	tst.b	(a3)

wait10us:
	cmp.b	#TENUS,(a3)
	bne.s	wait10us

	move.b	#WRITEVERIFY,(a1)	;command write verify

	move.b	d1,(a3)
	move.b	#0,$100(a3)		;start

	tst.b	(a3)
	tst.b	(a3)

wait6us2:
	cmp.b	(a3),d1
	bne.s	wait6us2

	cmp.b	(a0)+,(a1)+
	beq	WriteLoop

	subq.l	#1,a0
	subq.l	#1,a1

	dbf	d2,WriteRetry

	moveq	#00,d3			;failed

WriteDone:

	move.l	d3,d0			;result
	movem.l	(sp)+,d2-d7/a2-a6

	PRINTF	DBG_EXIT,<'%ld=WriteZone()'>,D0

	rts				;return (TRUE | FALSE)


** Check Blank State
**
** success = CheckBlank(zh)
** D0
**
	XDEF	_CheckBlank

_CheckBlank:

	move.l	4(sp),a0
	move.l	zh_to(a0),a1
	move.l	zh_size(a0),d1

	moveq	#-1,d0

CheckNextBlank:

	subq.l	#1,d1
	bmi.s	IsBlank

	cmp.b	(a1)+,d0
	beq.s	CheckNextBlank

	moveq	#00,d0		;not blank

IsBlank:
	rts


** Compare from/to memory
**
** success = VerifyWrite(zh)
** D0
**
	XDEF	_VerifyWrite

_VerifyWrite:

	move.l	4(sp),a0
	move.l	zh_to(a0),a1
	move.l	zh_size(a0),d1
	move.l	zh_from(a0),a0

	moveq	#01,d0		;assume success

NextMatch:
	subq.l	#1,d1
	bmi.s	IsMatched

	cmp.b	(a0)+,(a1)+
	beq.s	NextMatch

	moveq	#00,d0		;failed

IsMatched:

	rts
