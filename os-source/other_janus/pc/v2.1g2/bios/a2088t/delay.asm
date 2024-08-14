	NAME	DELAY
	TITLE	Delay subroutine for 100æs
	PAGE	59,132
;***************************************************************************
;* Copyright (C) 1985 by Phoenix Software Associates Ltd.  This program	   *
;* contains proprietary and confidential information.  All rights reserved *
;* except as may be permitted by prior written consent.			   *

;************************************************************************
;*	DLY100 - Delay 100 uS						*
;*	CALLED BY: everyone and his brother				*
;*									*
;*	Input:	none							*
;*	Output:	none							*
;************************************************************************

	INCLUDE ROMOPT.INC
	INCLUDE ROMEQU.INC
	INCLUDE	MACROS.INC		; ROM Macros file

OUT1	<DELAY - Delay subroutine for 100æs>


DGROUP	GROUP	ROMDAT
ROMDAT	SEGMENT AT 0040h	;The following are relative to Segment 40h
	EXTRN	DLY_CNT:BYTE		; DELAY COUNT
ROMDAT	ENDS
;
;------------------------------------------------------------------------------
;
CGROUP	GROUP	CODE
CODE	SEGMENT BYTE PUBLIC 'CODE'
	ASSUME	CS:CODE,DS:NOTHING,ES:NOTHING

        PUBLIC  DLY100
DLY100  PROC    NEAR


			;  8254 channel 0 and is independent of divisor (but 
			;  not mode) reprogramming
TIXIN	equ	189	; number of "tickettes" (timer 0 cycles) in 100 usec - 
			;  nominally ??? input clocks, but tune this sucker
; requires counter divisor (often 64K) >> TIXIN (~120) >> # tickettes / sample
;  (~13 on 80386+80385, usually more)
	push	ax
	push	bx
	push	dx
;*			    countlast = countfirst = current tickette
	call	gettickette
	mov	bx, ax
	sub	bx, TIXIN/2		; BX = countfirst-TIXIN
					;  may be < 0
	mov	dx, ax
;*			    do {
dlyloop:
;*				count = current tickette
	call	gettickette
;*				if rollover, countfirst += count
	cmp	dx, ax
	jae	rollovereif
	add	bx, ax
rollovereif:
;*				countlast = current tickette
	mov	dx, ax
;*			     } while countfirst - count < TIXIN
	cmp	bx, ax			; countfirst-TIXIN < count ?
	jl	dlyloop			; use signed, because countfirst-TIXIN
					;  may be < 0
;*			    exit
	pop	dx
	pop	bx
	pop	ax
	ret
;
DLY100	ENDP			; END PROCEDURE DEFINITION


;************************************************************************
;*	GETTICKETTE - get tickette					*
;*	This routine is used to get the current count at the 8254	*
;*	channel 0.							*
;*									*
;*	Input:	none							*
;*	Output:	none							*
;************************************************************************
ifdef NNNNEW                    ; for block move enhancements
public gettickette
endif ; NNNNEW
gettickette	proc near	; returns current counter 0 count / 2
	mov    	al, 0			; set counter latch
	cli
	out	PITMD,al
	WAFORIO
	in	al,PIT0
	xchg	al, ah
	WAFORIO
	in	al,PIT0			; read high byte
	sti				; enable ints
	xchg	al, ah
	shr	ax, 1			; /2
	ret
gettickette	endp

;---------------------------------------------------------------------------

	SYNC	BASIC_INT		; creates ORG at $C003 in BIOS segment

	TOTAL
;
CODE	ENDS
	END
