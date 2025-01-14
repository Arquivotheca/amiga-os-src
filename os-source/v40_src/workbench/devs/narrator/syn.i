
*************************************************************************	
*                                                                    	*
*   Copyright 1990, Joseph Katz/Mark Barton.  All rights reserved.	*
*   No part of this program may be reproduced, transmitted, or stored   *
*   in any language or computer system, in any form, whatsoever,	*
*   without the prior written permission of the authors.   		*
*                                                                    	*
*************************************************************************

*****************************************
*					*
* The individual synthesis routines     *
* broken up for purposes of SPEEEEED    *
*					*
*****************************************
*
* Totally voiced (vowel like)
*
F123:
	moveq	#0,d7
	btst	#RECEND,d4	;end of formant record?
	bne.s	F123a		;yes, branch
	moveq	#0,d6		;no, make F1,2,3 sample
	move.b	(a1)+,d6	;F1 record sample
	or.w	d1,d6		;or in amplitude
	move.b	0(a4,d6),d7	;get product from mult table
	moveq	#0,d6
	move.b	(a2)+,d6	;F2
	or.w	d2,d6
	sub.b	0(a4,d6),d7
	moveq	#0,d6
	move.b	(a3)+,d6	;F3
	or.w	d3,d6
	add.b	0(a4,d6),d7
F123a	move.b	d7,(a0)+	;move sample to output buffer
	move.b	d7,(a0)+
	subq.w	#1,SAMPCNT(a5)
	bne.s	F123b
	bsr	Output
	BNE	SYN_AbortIt	;For now, we only recognize aborts
F123b	subq.w	#1,d4		;decr microframe count
	bne.s	F123c
	move.w	SAMPERFRAME(a5),d4	;restore microframe count
	addq.w	#1,UPDATE(a5)	;incr frame update flag/count
F123c	subq.w	#1,RECLEN(a5)
	bne.s	F123d
	bset	#RECEND,d4	;set record end flag if end is reached
F123d	dbf	d0,F123		;if not at end of pitch pulse, do next sample
	bra	NewPulse	;if at end, branch
*
* Nasal/Voicebar  (F1 only)
* 
AN:
	moveq	#0,d7
	btst	#RECEND,d4	;end of formant record?
	bne.s	ANa		;yes, branch
	moveq	#0,d6		;no, make F1,2,3 sample
	move.b	(a1)+,d6	;F1 record sample
	or.w	d1,d6		;or in amplitude
	move.b	0(a4,d6),d7	;get product from mult table
ANa	move.b	d7,(a0)+	;move sample to output buffer
	move.b	d7,(a0)+
	subq.w	#1,SAMPCNT(a5)
	bne.s	ANb
	bsr.s	Output
	BNE	SYN_AbortIt	;For now, we only recognize aborts
ANb	subq.w	#1,d4		;decr microframe count
	bne.s	ANc
	move.w	SAMPERFRAME(a5),d4	;restore microframe count
	addq.w	#1,UPDATE(a5)	;incr frame update flag/count
ANc	subq.w	#1,RECLEN(a5)
	bne.s	ANd
	bset	#RECEND,d4	;set record end flag if end is reached
ANd	dbf	d0,AN		;if not at end of pitch pulse, do next sample
	bra	NewPulse	;if at end, branch
*
* Fricative
*
FF:
	moveq	#0,d6
	move.b	0(a2,d5),d6	;get fric sample using reflecting index (d5)
	or.w	d2,d6
	move.b	0(a4,d6),d7
	move.b	d7,(a0)+
	move.b	d7,(a0)+
	subq.w	#1,SAMPCNT(a5)
	bne.s	FFa
	bsr	Output
	BNE	SYN_AbortIt	;For now, we only recognize aborts
FFa	subq.w	#1,d4		;decr microframe
	bne.s	FFb
	move.w	SAMPERFRAME(a5),d4
	addq.w	#1,UPDATE(a5)
FFb	add.w	FRINC(a5),d5
	subq.w	#1,FRICLEN(a5)
	bne.s	FFc
	move.w	#511,FRICLEN(a5)
	neg.w	FRINC(a5)
FFc	dbf	d0,FF
	bra	NewPulse
*
* Aspirant
*
AH:
	moveq	#0,d6
	move.b	0(a3,d5),d6	;get aspir sample using reflecting index (d5)
	or.w	d3,d6
	move.b	0(a4,d6),d7
	asr.b	#3,d7		;attenuate aspir 18db
	move.b	d7,(a0)+
	move.b	d7,(a0)+
	subq.w	#1,SAMPCNT(a5)
	bne.s	AHa
	bsr	Output
	BNE	SYN_AbortIt	;For now, we only recognize aborts
AHa	subq.w	#1,d4		;decr microframe
	bne.s	AHb
	move.w	SAMPERFRAME(a5),d4
	addq.w	#1,UPDATE(a5)
AHb	add.w	FRINC(a5),d5
	subq.w	#1,FRICLEN(a5)
	bne.s	AHc
	move.w	#511,FRICLEN(a5)
	neg.w	FRINC(a5)
AHc	dbf	d0,AH
	bra	NewPulse
*
* Fricative + aspirant
*
FFAH:
	moveq	#0,d6
	move.b	0(a3,d5),d6	;get aspir sample using reflecting index (d5)
	or.w	d3,d6
	move.b	0(a4,d6),d7
	asr.b	#3,d7		;attenuate aspir 18db
	moveq	#0,d6
	move.b	0(a2,d5),d6	;get fric sample
	or.w	d2,d6
	add.b	0(a4,d6),d7	;add in fric product
	move.b	d7,(a0)+
	move.b	d7,(a0)+
	subq.w	#1,SAMPCNT(a5)
	bne.s	FFAHa
	bsr	Output
	BNE	SYN_AbortIt	;For now, we only recognize aborts
FFAHa	subq.w	#1,d4		;decr microframe
	bne.s	FFAHb
	move.w	SAMPERFRAME(a5),d4
	addq.w	#1,UPDATE(a5)
FFAHb	add.w	FRINC(a5),d5
	subq.w	#1,FRICLEN(a5)
	bne.s	FFAHc
	move.w	#511,FRICLEN(a5)
	neg.w	FRINC(a5)
FFAHc	dbf	d0,FFAH
	bra	NewPulse
*
* Voiced fricative
*
F1FF:
	moveq	#0,d6
	move.b	0(a2,d5),d6	;get fric sample
	or.w	d2,d6		;or in amplitude
	move.b	0(a4,d6),d7	;get fric product
	btst	#RECEND,d4	;end of formant record?
	bne.s	F1FFa		;yes, branch
	moveq	#0,d6		;no, make F1
	move.b	(a1)+,d6	;get F1 record sample
	or.w	d1,d6		;or in amplitude
	add.b	0(a4,d6),d7	;add in F1 product
F1FFa	move.b	d7,(a0)+	;move sample into output buffer
	move.b	d7,(a0)+
	subq.w	#1,SAMPCNT(a5)
	bne.s	F1FFb
	bsr	Output
	BNE	SYN_AbortIt	;For now, we only recognize aborts
F1FFb	subq.w	#1,d4		;decr microframe counter
	bne.s	F1FFc
	move.w	SAMPERFRAME(a5),d4
	addq.w	#1,UPDATE(a5)
F1FFc	subq.w	#1,RECLEN(a5)
	bne.s	F1FFd
	bset	#RECEND,d4
F1FFd	add.w	FRINC(a5),d5
	subq.w	#1,FRICLEN(a5)
	bne.s	F1FFe
	move.w	#511,FRICLEN(a5)
	neg.w	FRINC(a5)
F1FFe	swap	d0		;posn 1/2 pitch count
	subq.w	#1,d0		;decr 1/2 pitch count
	bne.s	F1FFf		;branch if not at 1/2 pitch period end
	swap	d2		;posn -6db fricative amplitude
F1FFf	swap	d0		;posn pitch
	dbf	d0,F1FF		;next sample
	bra	NewPulse
*
* Voiced aspirant
*
F1AH:
	moveq	#0,d6
	move.b	0(a3,d5),d6	;get aspir sample
	or.w	d3,d6		;or in amplitude
	move.b	0(a4,d6),d7	;get aspir product
	asr.b	#3,d7		;attenuate aspirant 18 db
	btst	#RECEND,d4	;end of formant record?
	bne.s	F1AHa		;yes, branch
	moveq	#0,d6		;no, make F1
	move.b	(a1)+,d6	;get F1 record sample
	or.w	d1,d6		;or in amplitude
	add.b	0(a4,d6),d7	;add in F1 product
F1AHa	move.b	d7,(a0)+	;move sample into output buffer
	move.b	d7,(a0)+
	subq.w	#1,SAMPCNT(a5)
	bne.s	F1AHb
	bsr	Output
	BNE	SYN_AbortIt	;For now, we only recognize aborts
F1AHb	subq.w	#1,d4		;decr microframe counter
	bne.s	F1AHc
	move.w	SAMPERFRAME(a5),d4
	addq.w	#1,UPDATE(a5)
F1AHc	subq.w	#1,RECLEN(a5)
	bne.s	F1AHd
	bset	#RECEND,d4
F1AHd	add.w	FRINC(a5),d5
	subq.w	#1,FRICLEN(a5)
	bne.s	F1AHe
	move.w	#511,FRICLEN(a5)
	neg.w	FRINC(a5)
F1AHe	dbf	d0,F1AH		;next sample
	bra	NewPulse
*
* Silence (fill output buffer with zeros)
*
SIL:
	clr.b	(a0)+		;zero output buffer
	clr.b	(a0)+
	subq.w	#1,SAMPCNT(a5)
	bne.s	SILa
	bsr	Output
	BNE	SYN_AbortIt	;For now, we only recognize aborts
SILa	subq.w	#1,d4		;decr microframe count
	bne.s	SILb
	move.w	SAMPERFRAME(a5),d4	;restore microframe count
	addq.w	#1,UPDATE(a5)	;incr frame update flag/count
SILb	dbf	d0,SIL		;if not at end of pitch pulse, do next sample
	bra	NewPulse	;if at end, branch

	NOP			;Put here in case next line is "NewPulse"
