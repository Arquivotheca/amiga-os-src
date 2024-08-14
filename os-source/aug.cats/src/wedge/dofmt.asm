*************************************************************************
*
*	general purpose string formatter
*
*	(C) Copyright 1985,1989,1991 Commodore-Amiga, Inc.
*	All Rights Reserved
*
*	$Id: dofmt.asm,v 39.2 92/05/12 09:55:27 mks Exp $
*
*	$Log:	dofmt.asm,v $
* Revision 39.2  92/05/12  09:55:27  mks
* Added support for the uppercase %d, %u, and %x such that programs
* can use those even without locale.  The ROM RawDoFmt() will not
* act differently for those other than accepting the upper case
* versions.
* 
* Revision 39.1  92/03/16  20:13:40  mks
* Fixed the %03ld and -5 problem
* Fixed the empty BSTR problem
*
* Revision 39.0  91/10/15  08:26:48  mks
* V39 Exec initial checkin
*
*************************************************************************

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

RAWDOFMTKLUDGE	EQU	1
	NOLIST
	INCLUDE "assembly.i"
	LIST

******o* Exports *******

	XDEF	RawDoFmt


*------ Ascii2Binary -------------------------------------------------
*
*   PURPOSE
*	Convert an unsigned ASCII number string to a long binary value
*
*   ENTRY
*	a4  String pointer
*   EXIT
*	d0  Binary value
*	d1  destroyed
*	d2  Next byte beyond ASCII number
*	a4  Pointer to next byte beyond ASCII number
*   notE
*	d2 and a4 do not follow standard calling protocol
*
*---------------------------------------------------------------------
Ascii2Binary:
		moveq.l #0,d0
		moveq.l #0,d2
a2BLoop:
		move.b	(a4)+,d2
		cmpi.b	#'0',d2
		bcs.s	a2BDone
		cmpi.b	#'9',d2
		bhi.s	a2BDone
		add.l	d0,d0		; *2
		move.l	d0,d1
		add.l	d0,d0		; *4
		add.l	d0,d0		; *8
		add.l	d1,d0		; *10
		sub.b	#'0',d2
		add.l	d2,d0
		bra.s	a2BLoop

a2BDone:
		subq.l	#1,a4
		rts


*------ FormatD ------------------------------------------------------
*
*   PURPOSE
*	Format a long signed binary to decimal ASCII
*
*   ENTRY
*	d4  number
*	a5  ASCII buffer
*   EXIT
*	a5  Ptr to byte beyond high digit (or sign) of ASCII number
*	d0,d1,d2,d4,a0	destroyed
*   notE
*	d2, d4 and a5 do not follow standard calling protocol
*
*--------------------------------------------------------------------

; New format D and U...

FormatD:	tst.l	d4		; Check the input number
		bpl.s	FormatU		; If not negative, skip this
		move.b	#'-',(a5)+	; Output the sign
		neg.l	d4		; Swap to positive value...
FormatU:	moveq.l	#'0',d0		; Assume we need a 0...
		lea	FDTable(pc),a0	; Get table...
FormatLoop:	move.l	(a0)+,d1	; Get current field
		beq.s	FormatDone	; We be done with format...
		moveq.l	#'0'-1,d2	; Start at '0'-1...
1$:		addq.l	#1,d2		; Bump up one...
		sub.l	d1,d4		; Drop the digit...
		bcc.s	1$		; Keep doin it until too far...
		add.l	d1,d4		; Add it back in...
		cmp.l	d0,d2		; Check for leading 0...
		beq.s	FormatLoop	; Continue if it is one...
		moveq.l	#0,d0		; Clear leading 0 flag...
		move.b	d2,(a5)+	; Save the character
		bra.s	FormatLoop	; Do next digit...
FDDone:
FormatDone:	moveq.l	#'0',d0		; Get the byte to add...
		add.b	d0,d4		; Add it to the last digit...
		move.b	d4,(a5)+	; Same it...
		rts			; we are done.

FDTable:
		dc.l	1000000000	; 1,000,000,000
		dc.l	100000000	;   100,000,000
		dc.l	10000000	;    10,000,000
		dc.l	1000000		;     1,000,000
		dc.l	100000		;       100,000
		dc.l	10000		;	 10,000
		dc.l	1000		;	  1,000
		dc.l	100		;           100
		dc.l	10		;            10
		dc.l	0		; End of table / unit digit default

*------ FormatX ------------------------------------------------------
*
*   PURPOSE
*	Format a long signed binary to hex ASCII
*
*   ENTRY
*	d4  number
*	a5  ASCII buffer
*   EXIT
*	a5  Pointer to byte beyond high digit of ASCII number
*	d0,d1,d2,d4  destroyed
*   notE
*	d2, d4 and a5 do not follow standard calling protocol
*
*---------------------------------------------------------------------
FormatX:
		tst.l	d4
		beq.s	FDDone
		clr.w	d1
		btst	#2,d3
		bne.s	FXLong
		moveq	#3,d2
		swap	d4
		bra.s	FX16sLoop
FXLong:
		moveq	#7,d2
FX16sLoop:
		rol.l	#4,d4
		move.b	d4,d0
		and.b	#$0F,d0
		bne.s	FXYesOut
		tst.w	d1
		beq.s	FX16sDbf
FXYesOut:
		moveq	#-1,d1
		cmp.b	#9,d0
		bhi.s	FXLetter
		add.b	#'0',d0
		bra.s	FXMoveOut
FXLetter:
		add.b	#('A'-10),d0
FXMoveOut:
		move.b	d0,(a5)+

FX16sDbf:
		dbf	d2,FX16sLoop
		rts


*****o* exec.library/RawDoFmt ***************************************
*
*   NAME
*	RawDoFmt -- format data into a character stream (V36 change).
*
*   SYNOPSIS
*	APTR RawDoFmt(FormatString, DataStream, PutChProc, PutChData);
*		 a0	       a1	   a2	  a3
*	APTR RawDoFmt(char *,APTR,void (*)(),APTR);
*
**********************************************************************
*
*   IMPLEMENTATION
*	d0  format character
*	d1  temporary (subroutines)
*	d2  temporary
*	d3  flags:  0:1 left justify, 1:1 zero fill, 2:1 long integer
*	d4  data longword
*	d5  specified string length limit
*	d6  specified field width
*	a0  temporary (subroutines)
*	a2  PutChProc
*	a3  PutChData
*	a4  formatStream next character
*	a5  stack buffer pointer
*	a6  stack buffer permanent pointer
*
*---------------------------------------------------------------------
RawDoFmt:
		movem.l d2-d6/a2-a5,-(a7)
		link	a6,#-16
		move.l	a1,-(a7)        ; top word on stack
		move.l	a0,a4
		bra.s	DFLoop

;---------- insert finish code here
DFFinish:
		jsr	(a2)            ; put final null character
		;V36 addition - return working DataStream pointer
		move.l	(a7),d0
		;set stack pointer from A6, then restore A6
		unlk	a6
		movem.l (a7)+,d2-d6/a2-a5
		rts

;---------
DFVanilla:	;-- not % format
		jsr	(a2)
DFLoop:
		move.b	(a4)+,d0        ; get next format character
		beq.s	DFFinish
		cmpi.b	#'%',d0
		bne.s	DFVanilla

;------------------------------------
;------- we just found a %! ---------
;------------------------------------
		lea.l	-16(a6),a5
		clr.w	d3
		cmpi.b	#'-',(a4)
		bne.s	DFF1

*	;-- left justified
		bset	#0,d3
		addq.l	#1,a4
DFF1:
		cmpi.b	#'0',(a4)
		bne.s	DFF2

*	;-- zero pad
		bset	#1,d3

*	;-- get integer width
DFF2:
		bsr	Ascii2Binary
		move.w	d0,d6

*	;-- get limiting width (reals are not supported)
		moveq.l #0,d5
		cmpi.b	#'.',(a4)
		bne.s	DFFDataSize
		addq.l	#1,a4
		bsr	Ascii2Binary
		move.w	d0,d5

*	;--
DFFDataSize:
		cmpi.b	#'l',(a4)
		bne.s	DFFGetCode
		bset	#2,d3
		addq.l	#1,a4


*	;-- get convert code
DFFGetCode:
		move.b	(a4)+,d0
		cmpi.b	#'d',d0
		beq.s	DFF_do_d
		cmpi.b	#'D',d0		; Check for upper case D too...
		bne.s	DFFX
		;-- 'D': decimal number
DFF_do_d:	bsr.s	DFGetData
		bsr	FormatD
		bra	DFFGot

DFFX:
		cmpi.b	#'x',d0
		beq.s	DFF_do_x
		cmpi.b	#'X',d0		; Check for upper case X too...
		bne.s	DFFS
		;-- 'X': hex number
DFF_do_x:	bsr.s	DFGetData
		bsr	FormatX
		bra.s	DFFGot


*	;(-- DFGetData)
DFGetData:
		btst	#2,d3		; check if word or long
		bne.s	DFLongData
		move.l	4(a7),a1        ; get data pointer
		move.w	(a1)+,d4        ; get data
		move.l	a1,4(a7)        ;   and update pointer
		ext.l	d4
		rts
DFLongData:
		move.l	4(a7),a1        ; get data pointer
		move.l	(a1)+,d4        ; get data
		move.l	a1,4(a7)        ;   and update pointer
		tst.l	d4		;set CC's
		rts
*	;--

DFFS:
		cmpi.b	#'s',d0
		bne.s	DFFB
		;-- 'S': string
		bsr.s	DFLongData	; get string address
		beq.s	ToDFLoop	; ignore null string pointer
		move.l	d4,a5
		bra.s	DFFLenGet

*	;--

DFFB:
		cmpi.b	#'b',d0
		bne.s	DFFU
		;-- 'B': BSTR
		bsr.s	DFLongData	; get BPTR
		beq.s	ToDFLoop		; ignore null BSTR pointer
		lsl.l	#2,d4		; convert to CPTR
		move.l	d4,a5
		moveq	#0,d2
		move.b	(a5)+,d2        ; get string length
		beq.s	ToDFLoop		; ignore null length...
		tst.b	-1(a5,d2.w)     ; check for null terminated BSTR
		bne.s	DFFGotLen
		subq.w	#1,d2
		bra.s	DFFGotLen


DFFU:
		cmpi.b	#'u',d0
		beq.s	DFF_do_u
		cmpi.b	#'U',d0		; Check for upper case U too...
		bne.s	DFFC
		;-- 'U': unsigned decimal number
DFF_do_u:	bsr.s	DFGetData
		bsr	FormatU
		bra.s	DFFGot


DFFC:		cmpi.b	#'c',d0
		bne	DFVanilla
		;-- 'C': character
		bsr.s	DFGetData
		move.b	d4,(a5)+


*	;-- adjust the formatted characters
DFFGot:
		clr.b	(a5)
		lea	-16(a6),a5

*	;-- get the length of the data
DFFLenGet:
		move.l	a5,a0
		moveq	#-1,d2
DFFLenCount:
		tst.b	(a0)+
		dbeq.s	d2,DFFLenCount
		not.l	d2		; null character is not in string
DFFGotLen:
		tst.w	d5		; check the limiting length
		beq.s	DFFActual	;   not there
		cmp.w	d5,d2		; compare with actual length
		bhi.s	DFFPad		;   keep greater limiting length
DFFActual:
		move.w	d2,d5		; use actual string length
DFFPad:
		sub.w	d5,d6		; determine # of pad characters
		bpl.s	DFFPosPad	; ensure pad is not negative
		clr.w	d6

*	;-- put the formatted bytes
DFFPosPad:
		btst	#0,d3		; check justification
		bne.s	DFOutFDbf
		bsr.s	DFPad
		bra.s	DFOutFDbf

DFOutFLoop:
		move.b	(a5)+,d0
		jsr	(a2)
DFOutFDbf:
		dbra.s	d5,DFOutFLoop
		btst	#0,d3
		beq.s	ToDFLoop
		bsr.s	DFPad
ToDFLoop:	bra	DFLoop

DFPad:
		move.b	#' ',d2
		btst	#1,d3
		beq.s	DFPDbf
*
* Magic fix for the 0-5 for %03d of -5...
*
		cmp.b	#'-',(a5)	; Check if negative
		bne.s	DFPNotNeg
		move.b	(a5)+,d0	; Get byte...
		subq.w	#1,d5		; Count it...
		jsr	(a2)		; Push it...
*
DFPNotNeg:	move.b	#'0',d2
		bra.s	DFPDbf
DFPLoop:
		move.b	d2,d0
		jsr	(a2)
DFPDbf:
		dbra.s	d6,DFPLoop
		rts

	END

