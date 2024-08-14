
*************************************************************************	
*                                                                    	*
*   Copyright 1990, Joseph Katz/Mark Barton.  All rights reserved.	*
*   No part of this program may be reproduced, transmitted, or stored   *
*   in any language or computer system, in any form, whatsoever,	*
*   without the prior written permission of the authors.   		*
*                                                                    	*
*************************************************************************

******  convert.asm  *********************************
*
* Step size to index conversion tables
*
* and the code to use them
*
* Also adds 3Hz LPF noise perturbations
* to the F0 periods
*
******************************************************

	cseg
	public CONVERT

	INCLUDE 'assembly.i'
	INCLUDE 'exec/types.i'
	INCLUDE 'exec/nodes.i'
	INCLUDE 'exec/lists.i'
	INCLUDE 'exec/strings.i'
	INCLUDE 'exec/initializers.i'
	INCLUDE 'exec/memory.i'
	INCLUDE 'exec/interrupts.i'
	INCLUDE 'exec/ports.i'
	INCLUDE 'exec/libraries.i'
	INCLUDE 'exec/devices.i'
	INCLUDE 'exec/io.i'
	INCLUDE 'exec/tasks.i'
	INCLUDE 'exec/resident.i'
	INCLUDE 'hardware/custom.i'
	INCLUDE 'hardware/dmabits.i'
	INCLUDE 'exec/execbase.i'
	INCLUDE 'exec/ables.i'
	INCLUDE 'exec/errors.i'

	include 'gloequs.i'
	include 'narrator.i'
	include 'f0equs.i'

* Coef frame offsets
F1	equ	0	;sometimes not used
F2	equ	1
F3	equ	2
AMP1	equ	3
AMP2	equ	4
AMP3	equ	5
FLAGS	equ	6
F0	equ	7

* LPF delay elements
*LPF	equ	2

* Noise element initialization
NOISEINIT	EQU	$C08D			;Start at a random spot


* Flag bits
FRIC	equ	6
ASPIR	equ	5
VBNASAL	equ	4

VOICEBAR equ	27		;voicebar is f1 sample #27

SAMPRATE equ	11100		;for the f0 computation


* Number of formant table entries
F1MAX	equ	31
F2MAX	equ	31
F3MAX	equ	21


CONVERT:
	move.l	COEFPTR(a5),a0	;get ptr to coef buffer
	lea	conv1,a1	;point to tables
	lea	conv2,a2
	lea	conv3,a3
	lea	dblinear,a4
*	lea	noireg,a6	;noise register & filter delays
*	clr.l	LPF(a6)		;clear both filter delays

	CLR.W	FILTER1DELAY(A5)		;Zero filter 1 delay element
	CLR.W	FILTER2DELAY(A5)		;Zero filter 2 delay element
	MOVE.W	#NOISEINIT,NOISEREGISTER(A5)	;Initialize noise register



* Prepare sampling rate modified by pitch parameter constant
	move.l	#F0BOR*SAMPRATE/2,d1	;sampling rate/2 (because 2Hz steps)
	divu	FREQ(a5),d1	;divide by user pitch parm
	and.l	#$ffff,d1	;and out remainder
	move.l	d1,KHZ12(a5)	;save

start	moveq	#0,d1
	moveq	#0,d2
	moveq	#0,d3
	moveq	#0,d4
	move.b	(a0),d1		;get f1 from coef
	cmp.b	#$ff,d1		;end?
	bne.s	con1		;no, branch
	rts			;return

con1	move.l	USERIORB(a5),a6 	;get ptr to user iorb
	move.b	NDI_FLAGS(a6),d5	;get iorb flags

	move.b	F2(a0),d2	;get f2
	move.b	F3(a0),d3	;get f3
	move.b	FLAGS(a0),d4	;get flags
	move.b	0(a1,d1),(a0)	;replace F1 with indexed table value

* Special bogus female code to be removed when proper targets are in place

	cmp.w	#FEMALE,NDI_SEX(a6)	;female?
	bne.s	con1b		;no, branch
	cmp.w	#5,d1		;a special sound? (VB nasal etc)
	ble.s	con1a		;yes, branch
	addq.b	#2,(a0)		;no, translate up 10%	

* End bogus code

con1b	cmp.w	#5,d1		;a special sound?
	ble.s	con1a		;yes, branch
	btst	#NDB_NEWIORB,d5	;no, new IORB?
	beq.s	con1a		;No, skip
	move.b	NDI_F1ADJ(a6),d0 ;Yes, add in F1 adjustment
	add.b	(a0),d0         ;
	bgt.s	con1xa		;clamp at zero
	move.b	#0,(a0)		;
	bra.s	con1a		;
con1xa	cmp.b	#F1MAX,d0	;   and at F1MAX
	ble.s	con1xb		;
	move.b	#F1MAX,(a0)	;
	bra.s	con1a		;
con1xb	move.b	d0,(a0)		;

	
con1a	btst	#FRIC,d4
	beq.s	con2		;branch if not a fricative
	subq.b	#1,d2		;first fric recording is actually #0
	lsl.b	#3,d2		;pre-shift 3 (synth shifts 6 more)
	move.b	d2,F2(a0)
	bra.s	con2a
con2	move.b	0(a2,d2),F2(a0)

* More bogus female code
	cmp.w	#FEMALE,NDI_SEX(a6)	;female?
	bne.s	con2b		;no, branch
	addq.b	#3,F2(a0)	;yes, translate up 15%

con2b	btst	#NDB_NEWIORB,d5	  ;New IORB?
	beq.s	con2a		  ;No, skip
	move.b	NDI_F2ADJ(a6),d2  ;Yes, add in F2 adjustment
	add.b	F2(a0),d2         ;
	bgt.s	con2xa	  	  ;clamp at zero
	move.b	#0,F2(a0)	  ;
	bra.s	con2a		  ;
con2xa	cmp.b	#F2MAX,d2	  ;   and at F2MAX
	ble.s	con2xb		  ;
	move.b	#F2MAX,F2(a0)	  ;
	bra.s	con2a		  ;
con2xb	move.b	d2,F2(a0)	  ;


con2a	btst	#ASPIR,d4
	beq.s	con3		;branch if not an aspirant
	subq.b	#1,d3
	lsl.b	#3,d3		;pre-shift 3 (synth shifts 6 more)
	move.b	d3,F3(a0)
	bra.s	con3a
con3	move.b	0(a3,d3),F3(a0)

* Even more bogus female code
	cmp.w	#FEMALE,NDI_SEX(a6)	;female?
	bne.s	con3e		;no, branch
	addq.b	#3,F3(a0)	;yes, translate up 15%

con3e	btst	#NDB_NEWIORB,d5	  ;New IORB?
	beq.s	con3a		  ;No, skip
	move.b	NDI_F3ADJ(a6),d3  ;Yes, add in F3 adjustment
	add.b	F3(a0),d3         ;
	bgt.s	con3xa		  ;clamp at zero
	move.b	#0,F3(a0)	  ;
	bra.s	con3a		  ;
con3xa	cmp.b	#F3MAX,d3	  ;   and at F3MAX
	ble.s	con3xb	  	  ;
	move.b	#F3MAX,F3(a0)	  ;
	bra.s	con3a		  ;
con3xb	move.b	d3,F3(a0)	  ;

* Convert amplitudes in db form to linear coefficients
con3a	move.b	AMP1(a0),d0	  ;get F1 amplitude in db
	ext.w	d0		  ;extend for signed indexing
	btst	#NDB_NEWIORB,d5
	beq.s	con3a1
	move.b	NDI_A1ADJ(a6),d3
	ext.w	d3
	add.w	d3,d0
con3a1	move.b	0(a4,d0),AMP1(a0) ;replace with linear equivalent
	move.b	AMP2(a0),d0	  ;do same for F2 & F3
	ext.w	d0
	btst	#NDB_NEWIORB,d5
	beq.s	con3a2
	move.b	NDI_A2ADJ(a6),d3
	ext.w	d3
	add.w	d3,d0
con3a2	move.b	0(a4,d0),AMP2(a0)	
	move.b	AMP3(a0),d0
	ext.w	d0
	btst	#NDB_NEWIORB,d5
	beq.s	con3a3
	move.b	NDI_A3ADJ(a6),d3
	ext.w	d3
	add.w	d3,d0
con3a3	move.b	0(a4,d0),AMP3(a0)

* If VBNASAL flag set, reduce AMP1 and zero AMP2 & AMP3
	btst	#VBNASAL,d4
	beq.s	con3b
	tst.b	d1		;voicebar?  (d1 still has ID number) 
	bne.s	con4a		;no, branch
	move.b	#2,AMP1(a0)	;yes, force amp1
	bra.s	con4a
*con4	move.b	AMP1(a0),d0	;nasals are quieter than 
*	lsr.b	#1,d0		; their articulatory targets
*	move.b	d0,AMP1(a0)	; MAYBE?
con4a	btst	#FRIC,d4
	bne.s	con4b		;
	clr.b	AMP2(a0)	;clear amp2 if voicebar and not fricative
con4b	btst	#ASPIR,d4
	bne.s	con3b		;clear amp3 if voicebar and not aspirant
	clr.b	AMP3(a0)	;

* Convert F0 entry from 2Hz steps to samples per pitch pulse
* An entry of 22 (44Hz) is the lowest allowable pitch.
* Adjust pitch proportionately in accordance to user pitch parameter

con3b	move.l	KHZ12(a5),d1	;get grand constant (for 2Hz steps)
	moveq	#0,d0
	cmp.w	#ROBOTICF0,F0MODE(a5)	;robotic mode?
	bne.s	con3ba		;no, branch
*	add.l	d1,d1		;yes, double because FREQ(a5) is in 1Hz steps
	divu	#F0BOR,d1	;divide by F0BOR to retrieve user pitch
	bra.s	con3bb
con3ba	move.b	F0(a0),d0	;get f0 frequency
	beq.s	con3c		;if zero do special stuff
	divu	d0,d1		;divide it into the samprate/2
con3bb	move.b	d1,F0(a0)	;move f0 period back into coef
	bra.s	con3d
con3c	move.b	#254,F0(a0)	;special zero frequency value (22Hz) 

* Compute a noise sample by computing 4 uniform random numbers
* and averaging to produce a pseudo-Gaussian distribution

con3d	btst	#NDB_NEWIORB,d5	;new iorb?
	beq	con3f		;no, skip perburbation code		
	moveq	#3,d1		;yes, loop count = 4
	moveq	#0,d0		;output sum = 0

*	move.l	a1,-(sp)	;save a1
*	lea	noireg,a1	;restore noise register & filter delays
*	move.w	(a1),d3		;pick up shift register
	MOVE.W	NOISEREGISTER(A5),D3		;Pick up shift register


noiloop	asl.w	#1,d3		;shift it
	bcc.s	noxor		;do that pseudo random
	eor.w	#$1d87,d3	;	 magic
noxor	ext.l	d3
	add.l	d3,d0		;add into sum
	dbf	d1,noiloop	;next noise value
	asr.l	#2,d0		;justify sum to WORD size

*	move.w	d3,(a1)		;save the shift register
	MOVE.W	D3,NOISEREGISTER(A5)		;Save shift register

	move.w	d0,d3		;move to filter input reg
* Low-pass filter at 3Hz
*	move.w	LPF(a1),d1	;pick up delay element
*	move.w	LPF+2(a1),d2	;pick up next delay element
	MOVE.W	FILTER1DELAY(A5),D1		;Pick up first delay element
	MOVE.W	FILTER2DELAY(A5),D2		;Pick up second delay element

	move.w	d1,d4		;d1 to out
	sub.w	d3,d4		;d1 - in
	muls	#31860,d4	;m1(d1 - in)	(3 Hz, bw=.6)
*	muls	#30114,d4	;m1(d1 - in)	(3 Hz)
*	muls	#31911,d4	;		(.5Hz)
	asl.l	#2,d4
	swap	d4
	add.w	d3,d4		;in + m1(d1 - in)
	move.w	d2,d7		;d2 to scratch
	sub.w	d3,d7		;d2 - in
	muls	#15877,d7	;m2(d2 - in)	(3 Hz, bw=.6)
*	muls	#14341,d7	;m2(d2 - in)	(3 Hz)
*	muls	#15549,d7	;		(.5Hz)
	asl.l	#2,d7
	swap	d7
	sub.w	d7,d4		;filter done

*	move.w	d4,LPF(a1)	;d1 = out -- store delay elements
*	move.w	d1,LPF+2(a1)	;d2 = d1
	MOVE.W	D4,FILTER1DELAY(A5)		;Store updated delay element
	MOVE.W	D1,FILTER2DELAY(A5)		;Store updated delay element


	moveq	#0,d0
	move.b	NDI_F0PERTURB(a6),d0
	lsl.w	#6,d0
	muls	d0,d4		;mult by perturb user coefficient
	swap	d4
	asr.w	#3,d4
	add.b	d4,F0(a0)	;add to pitch period in coef
*	move.l	(sp)+,a1	;restore a1

con3f	addq.l	#8,a0		;next coef frame
	bra	start
	
*noireg	dc.w	$c08d		;start at a random spot
*	dc.w	0,0		;filter delays

* Decibel to linear amplitude conversion table
	dc.w	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	;negative pad
	dc.w	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	;negative pad
dblinear:
	dc.w	0,0,0,0,0,0,0,0,0,0,0,0		;0-23
	dc.b	1,1,1,1,1,1,1,1,1,1,2,2,2,2,2
	dc.b	3,3,3,4,4,5,6,6,7,8,9,10,11,12
	dc.b	14,16,17,20,22,25,28,31		;line ends at 60db
	dc.b	31,31,31,31,31,31,31,31,31,31,31,31,31 ;positive pad

* Formant index tables
conv1:
   dc.b   31,32,33,34,0,0,0,0,0,0,0,0,0,1,2,4
   dc.b   5,6,7,9,10,11,12,12,13,14,15,16,17,17,18,19
   dc.b   19,20,21,21,22,22,23,23,24,24,25,25,26,26,27,27
   dc.b   28,28,28,29,29,30,30,30,30,30,30,30,30,30,30,30
   dc.b   30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30
   dc.b   30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30
   dc.b   30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30
   dc.b   30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30
   dc.b   30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30
   dc.b   30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30
   dc.b   30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30
   dc.b   30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30
   dc.b   30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30
   dc.b   30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30
   dc.b   30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30
   dc.b   30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30
* 31 = voicebar
* 32 = /M/
* 33 = /N/
* 34 = /NX/

conv2:
   dc.b   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
   dc.b   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
   dc.b   0,0,0,0,0,0,0,1,1,2,2,3,3,4,4,5
   dc.b   5,5,6,6,7,7,7,8,8,9,9,9,10,10,10,11
   dc.b   11,11,12,12,12,12,13,13,13,14,14,14,14,15,15,15
   dc.b   16,16,16,16,17,17,17,17,17,18,18,18,18,19,19,19
   dc.b   19,19,20,20,20,20,21,21,21,21,21,21,22,22,22,22
   dc.b   22,23,23,23,23,23,23,24,24,24,24,24,25,25,25,25
   dc.b   25,25,25,26,26,26,26,26,26,26,26,26,26,26,26,26
   dc.b   26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26
   dc.b   26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26
   dc.b   26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26
   dc.b   26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26
   dc.b   26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26
   dc.b   26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26
   dc.b   26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26
conv3:
   dc.b   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
   dc.b   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
   dc.b   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
   dc.b   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
   dc.b   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
   dc.b   0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3
   dc.b   3,4,4,4,4,4,5,5,5,5,5,6,6,6,6,6
   dc.b   7,7,7,7,7,7,8,8,8,8,8,8,9,9,9,9
   dc.b   9,9,10,10,10,10,10,10,11,11,11,11,11,11,11,12
   dc.b   12,12,12,12,12,12,13,13,13,13,13,13,13,13,14,14
   dc.b   14,14,14,14,14,15,15,15,15,15,15,15,15,15,16,16
   dc.b   16,16,16,16,16,16,17,17,17,17,17,17,17,17,17,18
   dc.b   18,18,18,18,18,18,18,18,18,19,19,19,19,19,19,19
   dc.b   19,19,19,20,20,20,20,20,20,20,20,20,20,20,20,20
   dc.b   20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20
   dc.b   20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20

   END
