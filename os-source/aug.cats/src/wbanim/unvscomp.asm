;
; This file is presented exactly as found in the file riffsrc.arc on the BIX
; listings area for the Amiga except for the addition of the README file
; from the same arc file at the front end.  This was added to justify the
; PD nature of this code despite the existence of the copyright notice in
; the original file.
;
; *** README file ***
;
; Hello
; 	here's the source code in Aztec C and assembler for Playriff.
; It must be compiled with 32-bit-ints.  Though the code is PD and may
; be put to whatever use you see fit, I'm not giving away free consulting
; time to support it.  Nevertheless here's how to get in touch with me
; if you have some questions.
;
;	Jim Kent
;	Dancing Flame
;	739A 16th Ave.
;	San Francisco, CA 94118
;	or jim_kent on BIX.
;
; *** unvscomp.asm file ***
;
; unvscomp.asm  Copyright 1987 Dancing Flame all rights reserved.
;
; This file contains a single function which is set up to be called from
; C.  Ie the parameters are on the stack instead of registers.
;       decode_vkplane(in, out, linebytes)
; where in is a bit-plane's worth of vertical-byte-run-with-skips data
; and out is a bit-plane that STILL has the image from last frame on it.
; Linebytes is the number of bytes-per-line in the out bitplane, and it
; should certainly be noted that the external pointer variable ytableptr
; must be initialized to point to a multiplication table of
; 0*linebytes, 1*linebytes ... n*linebytes  before this routine is called.
;
; The format of "in":
;   Each column of the bitplane is compressed separately.  A 320x200
;   bitplane would have 40 columns of 200 bytes each.  The linebytes
;   parameter is used to count through the columns, it is not in the
;   "in" data, which is simply a concatenation of columns.
;
;   Each columns is an op-count followed by a number of ops.
;   If the op-count is zero, that's ok, it just means there's no change
;   in this column from the last frame.
;   The ops are of three classes, and followed by a varying amount of
;   data depending on which class.
;       1. Skip ops - this is a byte with the hi bit clear that says how many
;          rows to move the "dest" pointer forward, ie to skip. It is non-
;          zero
;       2. Uniq ops - this is a byte with the hi bit set.  The hi bit is
;          masked down and the remainder is a count of the number of bytes
;          of data to copy literally.  It's of course followed by the
;          data to copy.
;       3. Same ops - this is a 0 byte followed by a count byte, followed
;          by a byte value to repeat count times.
;   Do bear in mind that the data is compressed vertically rather than
;   horizontally, so to get to the next byte in the destination (out)
;   we add linebytes instead of one!
;

	XDEF _DecodeVKPlane

; LONG ASM DecodeVKPlane (REG (a0) char *in, REG(a2) char *out, REG (d2) LONG linebytes, REG (a3) WORD *ytable);

_DecodeVKPlane:
	movem.l	a2/a3/d2-d5,-(sp)	; save registers
	move.w	d2,d4			; make a copy of linebytes to use as a counter
	bra	zdcp			; And go to the "columns" loop

dcp
	move.l	a2,a1			; get copy of dest pointer
	clr.w	d0			; clear hi byte of op_count
	move.b	(a0)+,d0		; fetch number of ops in this column
	bra	zdcvclp			; and branch to the "op" loop.

dcvclp	clr.w	d1			; clear hi byte of op
	move.b	(a0)+,d1		; fetch next op
	bmi.s	dcvskuniq		; if hi-bit set branch to "uniq" decoder
	beq.s dcvsame			; if it's zero branch to "same" decoder

skip					; otherwise it's just a skip
	add.w	d1,d1			; use amount to skip as index into word-table
	adda.w	0(a3,d1),a1
	dbra	d0,dcvclp		; go back to top of op loop
	addq.l	#1,a2
	dbra	d4,dcp
	movem.l	(sp)+,a2/a3/d2-d5
	rts

dcvsame					; here we decode a "vertical same run"
	move.b	(a0)+,d1		; fetch the count
	move.b	(a0)+,d3		; fetch the value to repeat
	move.w	d1,d5			; and do what it takes to fall into a "tower"
	asr.w	#3,d5			; d5 holds # of times to loop through tower
	and.w	#7,d1			; d1 is the remainder
	add.w	d1,d1
	add.w	d1,d1
	neg.w	d1
	jmp	32+same_tower(pc,d1)	; size of tower

same_tower
	move.b	d3,(a1)
	adda.w	d2,a1
	move.b	d3,(a1)
	adda.w	d2,a1
	move.b	d3,(a1)
	adda.w	d2,a1
	move.b	d3,(a1)
	adda.w	d2,a1
	move.b	d3,(a1)
	adda.w	d2,a1
	move.b	d3,(a1)
	adda.w	d2,a1
	move.b	d3,(a1)
	adda.w	d2,a1
	move.b	d3,(a1)
	adda.w	d2,a1
	dbra	d5,same_tower
	dbra	d0,dcvclp
	addq.l	#1,a2
	dbra	d4,dcp
	movem.l	(sp)+,a2/a3/d2-d5
	rts

dcvskuniq				; here we decode a "unique" run
	and.b	#$7f,d1			; setting up a tower as above....
	move.w	d1,d5
	asr.w	#3,d5
	and.w	#7,d1
	add.w	d1,d1
	add.w	d1,d1
	neg.w	d1
	jmp	32+uniq_tower(pc,d1)
uniq_tower
	move.b	(a0)+,(a1)
	adda.w	d2,a1
	move.b	(a0)+,(a1)
	adda.w	d2,a1
	move.b	(a0)+,(a1)
	adda.w	d2,a1
	move.b	(a0)+,(a1)
	adda.w	d2,a1
	move.b	(a0)+,(a1)
	adda.w	d2,a1
	move.b	(a0)+,(a1)
	adda.w	d2,a1
	move.b	(a0)+,(a1)
	adda.w	d2,a1
	move.b	(a0)+,(a1)
	adda.w	d2,a1
	dbra	d5,uniq_tower		; branch back up to "op" loop
zdcvclp dbra	d0,dcvclp		; branch back up to "column loop"

					; now we've finished decoding a single column
z1dcp	addq.l	#1,a2			; so move the dest pointer to next column
zdcp	dbra	d4,dcp			; and go do it again what say?
	movem.l	(sp)+,a2/a3/d2-d5	; restore registers
	rts
