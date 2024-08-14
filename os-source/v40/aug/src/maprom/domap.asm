*
* $Id: domap.asm,v 39.4 92/08/26 10:43:43 mks Exp $
*
*******************************************************************************
*
* Called with A0 pointing to ROM buffer...
*
* Since this is really slimy hardware ROM remapping, we need to just do
* slimy tricks...  I guess this is just to try the remap...
* (Very 512K kickstart required as the hardware does only 512K)
*
* This must be in CHIP memory...  (To work correctly)
*
	SECTION	DoMap,CODE,CHIP
*
ROM_MAP			equ	$07F80000
ROM_MAP_READ		equ	$00F80000
ROM_SIZE		equ	$00080000
ROM_MAP_SET		equ	$80F80000
ROM_CRASH		equ	$00F800D0	; Really slimy trick...
*
A3000_BusTimeoutMode	equ	$00de0000	;0=DSACK 1=BERR|0=ok 1=timed out
A3000_BusTimeoutEnable	equ	$00de0001	;Bit 7  0=enable 1=disable

*
		xref	_LVOSupervisor
		xref	_intena
		xref	_intreq
*
_DoMap:		xdef	_DoMap
		movem.l	a5/a6,-(sp)		; Save
		move.l	4,a6			; Get execbase
		lea	DoMap(pc),a5		; Place to go...
		jsr	_LVOSupervisor(a6)	; Into supervisor mode
		movem.l	(sp)+,a5/a6		; Restore
		rts				; We did not map...
*
DoMap:		or.w	#$0700,SR		; DISABLE (restored via RTE)
*
* Now, check if the mapping can work...
*
		move.b	#0,A3000_BusTimeoutMode	; We want no BERR...
		move.b	#0,A3000_BusTimeoutEnable
*
		nop				; Pipe line flush
		move.l	#1,ROM_MAP_SET		; Turn on the mapping...
		nop				; Pipe line flush
		move.l	ROM_MAP,d0		; Get RAM version...
		cmp.l	ROM_MAP_READ,d0		; Did it check?
		bne.s	No_Map			; No map hardware
		move.l	d0,d1			; Get other value...
		not.l	d1			; to test with
		move.l	d1,ROM_MAP		; Store it
		cmp.l	ROM_MAP_READ,d1		; Check it
		beq.s	Yes_Map			; If same, we have a map...
		move.l	d0,ROM_MAP		; Restore the old value
No_Map:		move.b	#$80,A3000_BusTimeoutMode	; Back to normal?
		rte				; Exit (no mapping hardware)
*
* We get here only if the mapping hardware exists
*
Yes_Map:	move.l	#ROM_MAP,a1		; Destination...
		move.l	#ROM_SIZE/4,d0		; ROM Size... (LONGs)
		bra.s	Forward1		; Jump into the loop...
ForwardLoop:	swap	d0			; Swap back up...
ForwardLoop1:	move.l	(a0)+,(a1)+		; Copy...
Forward1:	dbra.s	d0,ForwardLoop1		; Loop...
		swap	d0			; High word of count
		dbra.s	d0,ForwardLoop		; Some more...
*
* I wish we could call ColdCrash but that does not work since
* the ROM just was changed...
*
CopyDone:	move.b	#$80,A3000_BusTimeoutMode	; Back to normal?
		move.b	#$80,A3000_BusTimeoutEnable
		move.w	#$4000,_intena		; Disable interrupts...
		move.w	#$3fff,_intreq		; Clear interrupts...
		lea	ROM_CRASH,a0		; Where to jump to...
		cnop	0,4			; Correct alignment...
		reset				; CPU reset	(first word)
		jmp	(a0)			; System reset... (second word)
*
*******************************************************************************
*
		END
