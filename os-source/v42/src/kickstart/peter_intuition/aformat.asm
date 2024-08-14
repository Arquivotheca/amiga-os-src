
*** afmt.asm *****************************************************************
*
*  layout format routines.
*
*  $Id: aformat.asm,v 38.0 91/06/12 14:13:19 peter Exp $
*
*  Confidential Information: Commodore-Amiga, Inc.
*  Copyright (C) 1989, Commodore-Amiga, Inc.
*  All Rights Reserved
*
****************************************************************************/

	INCLUDE	'exec/types.i'
	INCLUDE 'intuition/intuition.i'


; call rawdofmt	a0: fmstring, a1: datastream, a2: pcproc, a3: pcdata

	XREF	_LVORawDoFmt
	XREF	_AbsExecBase
	XREF	_Debug

	XDEF	_fmtPass1
	XDEF	_fmtPass2
	XDEF	_jsprintf

*******************************************************
; Pass 2
; does formatting into 'buffer', replacing separators with '\0',
; and pointing to components a list of IntuiText structs.
;
; returns next member in arglist
;
;	fmtPass2( fmtstring, arglist, buffer, itextlist, separator )
;		    +4(sp)  +8(sp)    +12(sp)	+16(sp)
_fmtPass2:
	move.l	16(sp),a0	; get the first itext
	move.l	a0,d0
	beq.s	pass2bail
	move.l	12(sp),it_IText(a0)	; point it at beginning of buffer

    	lea.l	pass2,a0
	bra.s	bothPasses		; call RawDoFmt
pass2bail
	rts

	
*******************************************************
; Pass 1
; count up total characters in formatted string,
; also count up number of 'separated' string components
; where the separator is '|'
;	fmtPass1( fmtstring, arglist, charcountp, itcountp, separator )
;		    +4(sp)  +8(sp)    +12(sp)	   +16(sp)
_fmtPass1:
	move.l	16(sp),a0		; init itext count to 1
	move.w	#1,(a0)

    	lea.l	pass1,a0		; call RawDoFmt
	; fall through

*******************************************************
	; procedure for RawDoFmt to call is in a0
bothPasses:
	movem.l	a2/a3/a6,-(sp)

	move.l	a0,a2		; procedure

	move.l	12+4(sp),a0	; formatstring
	move.l	12+8(sp),a1	; arglist
	lea.l	12+12(sp),a3	; address of other params

	move.l	_AbsExecBase,a6
	jsr	_LVORawDoFmt(a6)

	; leave return value from RawDoFmt

	movem.l	(sp)+,a2/a3/a6
	rts


*******************************************************
; count chars and number of separated components
; called by RawDoFmt, d0:8: char,
; a3: address of  {&charcount,&itextcount,separator}

pass1:	
	move.l	8(a3),d1	; get separator

	move.l	(a3),a0		; get address of charcount
	addq.w	#1,(a0)		; increment

	; increment "separated string" count when I see the separator
	cmp.b	d1,d0
	bne.s	pass1done

	move.l	4(a3),a0
	addq.w	#1,(a0)

	; jsr	_LVODebug

pass1done:
	rts

*******************************************************
; copy chars and link in itexts
; called by RawDoFmt, d0:8: char,
; a3: address of  {buffer pointer,&itextlist,separator}

pass2:	
	move.l	8(a3),d1	; get separator

	; copy the character into the buffer
	move.l	(a3),a0
	move.b	d0,(a0)		; hold onto a0, I might overwrite char
	addq.l	#1,(a3)		; bump buffer pointer

	; when I encounter the separator, I overwrite it
	; in the buffer with \0, ending the current itext.
	; I then link to the next itext, and start it off
	; on the next buffer position (yet to be filled).
	cmp.b	d1,d0
	bne.s	pass2done

	move.b	#0,(a0)		; overwrite separator in buffer with \0
	; late fix for beta 1, wasn't using immediate #0, but rather 
	; just contents at 0

	move.l	4(a3),a1	; on to next itext link
	move.l	it_NextText(a1),a1
	move.l	a1,4(a3)

	move.l	a1,d0		; make sure the itext chain didn't end
	beq.s	pass2done
	move.l	(a3),it_IText(a1) ; link the new one into new buffer pos

pass2done:
	rts

_jsprintf:
	movem.l	a2/a3/a6,-(sp)
	move.l	4*4(sp),a3
	move.l	5*4(sp),a0
	lea.l	6*4(sp),a1
	lea.l	stuffChar(pc),a2
	move.l	_AbsExecBase,a6
	jsr	_LVORawDoFmt(a6)
	movem.l	(sp)+,a2/a3/a6
	rts

stuffChar:	
	move.b	d0,(a3)+
	rts

	end

