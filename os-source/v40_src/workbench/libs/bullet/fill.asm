**
**	$Id: fill.asm,v 9.0 91/04/09 20:00:12 kodiak Exp $
**
**      intellifont math support
**
**      (C) Copyright 1991 Robert R. Burns
**          All Rights Reserved
**
	SECTION	bullet

**	Included Files

	INCLUDE	"exec/types.i"

**	Exported Names

	XDEF	_fill

**	Imported Names

	XREF	_or_on

off_tab:
		dc.b	$00,$01,$03,$02,$07,$06,$04,$05
		dc.b	$0F,$0E,$0C,$0D,$08,$09,$0B,$0A
		dc.b	$1F,$1E,$1C,$1D,$18,$19,$1B,$1A
		dc.b	$10,$11,$13,$12,$17,$16,$14,$15
		dc.b	$3F,$3E,$3C,$3D,$38,$39,$3B,$3A
		dc.b	$30,$31,$33,$32,$37,$36,$34,$35
		dc.b	$20,$21,$23,$22,$27,$26,$24,$25
		dc.b	$2F,$2E,$2C,$2D,$28,$29,$2B,$2A
		dc.b	$7F,$7E,$7C,$7D,$78,$79,$7B,$7A
		dc.b	$70,$71,$73,$72,$77,$76,$74,$75
		dc.b	$60,$61,$63,$62,$67,$66,$64,$65
		dc.b	$6F,$6E,$6C,$6D,$68,$69,$6B,$6A
		dc.b	$40,$41,$43,$42,$47,$46,$44,$45
		dc.b	$4F,$4E,$4C,$4D,$48,$49,$4B,$4A
		dc.b	$5F,$5E,$5C,$5D,$58,$59,$5B,$5A
		dc.b	$50,$51,$53,$52,$57,$56,$54,$55
		dc.b	$FF,$FE,$FC,$FD,$F8,$F9,$FB,$FA
		dc.b	$F0,$F1,$F3,$F2,$F7,$F6,$F4,$F5
		dc.b	$E0,$E1,$E3,$E2,$E7,$E6,$E4,$E5
		dc.b	$EF,$EE,$EC,$ED,$E8,$E9,$EB,$EA
		dc.b	$C0,$C1,$C3,$C2,$C7,$C6,$C4,$C5
		dc.b	$CF,$CE,$CC,$CD,$C8,$C9,$CB,$CA
		dc.b	$DF,$DE,$DC,$DD,$D8,$D9,$DB,$DA
		dc.b	$D0,$D1,$D3,$D2,$D7,$D6,$D4,$D5
		dc.b	$80,$81,$83,$82,$87,$86,$84,$85
		dc.b	$8F,$8E,$8C,$8D,$88,$89,$8B,$8A
		dc.b	$9F,$9E,$9C,$9D,$98,$99,$9B,$9A
		dc.b	$90,$91,$93,$92,$97,$96,$94,$95
		dc.b	$BF,$BE,$BC,$BD,$B8,$B9,$BB,$BA
		dc.b	$B0,$B1,$B3,$B2,$B7,$B6,$B4,$B5
		dc.b	$A0,$A1,$A3,$A2,$A7,$A6,$A4,$A5
		dc.b	$AF,$AE,$AC,$AD,$A8,$A9,$AB,$AA

off_nxt:
		dc.b	0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0
		dc.b	1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1
		dc.b	1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1
		dc.b	0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0
		dc.b	1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1
		dc.b	0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0
		dc.b	0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0
		dc.b	1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1
		dc.b	1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1
		dc.b	0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0
		dc.b	0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0
		dc.b	1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1
		dc.b	0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0
		dc.b	1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1
		dc.b	1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1
		dc.b	0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0

;------	fill ---------------------------------------------------------
;
;	void fill(UBYTE *srcptr, UBYTE *dstptr,
;		WORD width, WORD depth, UBYTE *or_buffer,
;		BOOL making_bold, UBYTE *bold_buf2);
;	
 STRUCTURE	parms,0
    STRUCT saveRegs,(6+2)*4
    APTR    retAddr
    APTR    srcptr
    APTR    dstptr
    LONG    width
    LONG    depth
    APTR    or_buffer
    LONG    making_bold
    APTR    bold_buf2

_fill:
		movem.l	d2-d7/a2-a3,-(a7)
;   if (making_bold) {
;	fill_bold(srcptr, or_buffer, bold_buf2, dstptr, width, depth);
;	return;
;   }
		tst.l	making_bold(a7)
		bne.s	fillBold

;   dptr = dstptr;			/* remember start of destination */

		;-- get parameters:
		;   a0  srcptr
		;   a1  dstptr
		;   d2  width counter
		;   d3  width
		;   d4  depth counter
		movem.l	srcptr(a7),a0/a1
		move.w	width+2(a7),d3
		move.w	depth+2(a7),d4
		lea	off_tab(pc),a2
		lea	off_nxt(pc),a3
		moveq	#0,d0		; clear high bytes of p

;   for (d = 0; d < depth; d++) {	/* for each horizontal raster      */
fDepthLoop:
		subq.w	#1,d4
		blt.s	fChkOrOn

;	mode = 0;			/* beam off at start of each raster */
		;-- mode will be kept track of w/ different loop code

;	for (w = 0; w < width; w++) {	/* loop for each byte of the raster */
		move.w	d3,d2		; w
		bra.s	fMode0WidthLoop

fMode0SpecialZero:
		move.b	d0,(a1)+

fMode0WidthLoop:
		subq.w	#1,d2
		blt.s	fDepthLoop

;	    p = *srcptr++;
		move.b	(a0)+,d0
		beq.s	fMode0SpecialZero

;	    if (mode == 0) {
;		*dstptr++ = off_tab[p];
		move.b	0(a2,d0.w),(a1)+

;		mode = off_nxt[p];
		tst.b	0(a3,d0.w)
		beq.s	fMode0WidthLoop
		;--	fall thru to fMode1WidthLoop

fMode1WidthLoop:
		subq.w	#1,d2
		blt.s	fDepthLoop

		move.b	(a0)+,d0

;	    else {
;		*dstptr++ = ~off_tab[p];
		move.b	0(a2,d0.w),d1
		not.b	d1
		move.b	d1,(a1)+

;		mode = (BYTE) 1 - off_nxt[p];
		tst.b	0(a3,d0.w)
		beq.s	fMode1WidthLoop
		bra.s	fMode0WidthLoop

;	    }
;	}
;   }


fChkOrOn:

;   if (or_on) {
		tst.w	_or_on(a4)
		beq.s	fDone

;	tran_size = width * depth;
		mulu	depth+2(a7),d3
		lsr.l	#2,d3
;	for (w = 0; w < tran_size; w++)
		move.l	or_buffer(a7),a0
		move.l	dstptr(a7),a1
		bra.s	fOrDBF
;	    *dptr++ |= *or_buffer++;
fOrLoop:
		move.l	(a0)+,d1
		or.l	d1,(a1)+
fOrDBF:
		dbf	d3,fOrLoop
;   }

fDone:
		movem.l	(a7)+,d2-d7/a2-a3
		rts


;------	fillBold -----------------------------------------------------
fillBold:
		;-- get parameters:
		;   a0  srcptr (y0)
		;   a1  dstptr (dstptr)
		;   d4	depth counter
		;   a2  or_buffer (y1)
		;   a3  bold_buf2 (y2)
		movem.l	srcptr(a7),a0/a1
		move.w	depth+2(a7),d4
		move.l	or_buffer(a7),a2
		move.l	bold_buf2(a7),a3

		;   d2	b2: y2 temp
		;   d3	width counter
		;   d5	bit count
		;   d6	result longword
		;   d7	on_count

;   for (d = 0; d < depth; d++) {	/* for each horizontal raster        */
fbDepthLoop:
		subq.w	#1,d4
		blt.s	fDone

;	on_count = 0;			/* beam off at start of each raster  */
		moveq	#0,d7		; on_count

;	for (w = 0; w < width; w++) {	/* loop for each byte of the raster  */
		move.w	width+2(a7),d3

fbWidthLoop:
		subq.w	#4,d3		; width by longwords, not bytes
		blt.s	fbDepthLoop

;	    bit_mask = 0x80;
;	    b0 = *y0++;
;	    b1 = *y1++;
;	    b2 = *y2++;
;	    b = 0;
		move.l	(a0)+,d0	; by longwords, not bytes
		move.l	(a2)+,d1	; by longwords, not bytes
		move.l	(a3)+,d2	; by longwords, not bytes
		moveq	#0,d6

;	    for (bit = 0; bit < 8; bit++) {
		moveq	#31,d5		; by longwords, not bytes
fbBitLoop:
;		b <<= 1;		/* shift to next destination bit    */
		add.l	d6,d6

;		/* Get next on count change */
;
;		on_change = 0;
;		if (b0 & bit_mask)
;		    on_change++;
		add.l	d0,d0
		bcc.s	fbChkB1
		addq.w	#1,d7
;		if (b1 & bit_mask)
;		    on_change += 2;
fbChkB1:
		add.l	d1,d1
		bcc.s	fbChkB2
		addq.w	#2,d7
;		if (b2 & bit_mask)
;		    on_change += 4;
;		if (on_change > 3)
;		    on_change |= 0xfff8;/* sign extend  */
fbChkB2:
		add.l	d2,d2
		bcc.s	fbChkOnCnt
		subq.w	#4,d7
;		on_count += on_change;
;
;		if (on_count > 0)	/* If we're on                      */
;		    b |= 1;		/* write another bit to destination */
fbChkOnCnt:
		tst.w	d7
		ble.s	fbNextBit
		addq.l	#1,d6
;
;		bit_mask >>= 1;
fbNextBit:
		dbf	d5,fbBitLoop
;	    }
;	    *dstptr++ = b;		/* Write UBYTE to destination        */
		move.l	d6,(a1)+
		bra.s	fbWidthLoop
;	}
;    }
;}
	END
