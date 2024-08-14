
	TTL	'STRMARK.ASM'


*************************************************************************	
*                                                                    	*
*   Copyright 1990, Joseph Katz/Mark Barton.  All rights reserved.	*
*   No part of this program may be reproduced, transmitted, or stored   *
*   in any language or computer system, in any form, whatsoever,	*
*   without the prior written permission of the authors.   		*
*                                                                    	*
*************************************************************************

          SECTION      speech

**************************************************
*
* Main module STRMARK
*
* Marks the appropriate consonants to the left
* of a stressed vowel as stressed
*
* Note 1 - Oct 10, 1989  JK  -- Changed LEA's to MOVE.L's.  PHON, STRESS, and
*				DUR are now allocated rather than fixed bfrs.
**************************************************


 
	INCLUDE   'exec/types.i'
        INCLUDE   'featequ.i'
        INCLUDE   'gloequs.i'
        INCLUDE   'pcequs.i'

        XREF      FEAT
        XDEF      STRMARK

STRESSED	equ	$10	;stress bit
UNSTRESSED	equ	$ef	;complement of stress bit
STRESSBIT	equ	4

*-------- Initialize pointers

STRMARK:
	MOVE.L	PHON(a5),a0					;*** Note 1
	MOVE.L	STRESS(a5),a1					;*** Note 1
	LEA	FEAT,a4						;*** Note 1

	subq.l	#1,a0		;pt to previous phon
	bra.s	str1
strloop	addq.l	#1,a0		;next phon
	addq.l	#1,a1		;next stress
str1	moveq	#0,d0
	move.b	(a0),d0		;get previous phon
	move.b	(a1),d1		;get current stress
	beq	strloop		;not stressed, next phon
	cmp.b	#$ff,d1		;last phon?
	beq	strend		;yes, branch
	moveq	#0,d7		;no, get features of prev phon
	move.b	d0,d7
	lsl.w	#2,d7
	move.l	0(a4,d7),d2
	btst	#CONSBIT,d2	;consonant?
	beq	strloop		;no, next phon
	or.b	#STRESSED,-1(a1) ;yes, mark as stressed

* Start backup stress marking

	move.l	a0,a2		;transfer phon pointer
	move.l	a1,a3		;transfer stress pointer
	subq.l	#1,a3		;align stress with prev phon
	btst	#LIQUIDBIT,d2	;liquid?
	bne.s	liqgli		;yes, branch
	btst	#GLIDEBIT,d2	;glide?
	beq.s	checkNOSE	;no, branch
liqgli	or.b	#STRESSED,(a3)	;it's a liqgli, mark as stressed (redundant)
	subq.l	#1,a3		;backup stress
	move.b	-(a2),d0	;backup & get phon
	moveq	#0,d7
	move.b	d0,d7
	lsl.w	#2,d7
	move.l	0(a4,d7),d2	;get features
	bra.s	checkPLOS
checkNOSE:
	btst	#NASALBIT,d2	;nasal?
	beq.s	checkPLOS	;no, branch
markit	or.b	#STRESSED,(a3)	;yes, mark as stressed	
	subq.l	#1,a3		;backup stress
	move.b	-(a2),d0	;backup & get phon
	moveq	#0,d7
	move.b	d0,d7
	lsl.w	#2,d7
	move.l	0(a4,d7),d2	;get features
	bra.s	checkFRIC
checkPLOS:
	btst	#PLOSBIT,d2	;plosive?
	bne	markit		;yes, branch
	btst	#PLOSABIT,d2	;no, plosive aspirant?
	bne	markit		;yes, mark as stressed, then check fric
checkFRIC:
	btst	#FRICBIT,d2	;fricative?
	beq.s	checkAFFRIC	;no, branch
	btst	#VOICEDBIT,d2	;voiced fricative?
	bne.s	destress	;yes, branch to destressing loop
	or.b	#STRESSED,(a3)	;no, it's an unvoiced fric, mark as stressed
	bra.s	destress
checkAFFRIC:
	btst	#AFFRICBIT,d2	;affricate?
	beq.s	destress	;no, branch to destressing loop
	or.b	#STRESSED,(a3)	;yes, mark as stressed
destress:

	bra	strloop		;experimental, don't destress at all

	btst	#STRESSBIT,(a3)
	beq.s	des1
	addq.l	#1,a2		;start forward destressing loop
	addq.l	#1,a3		; by skipping the first accented consonant
des1	addq.l	#1,a2
	addq.l	#1,a3
	move.b	(a2),d0		;get phon
	moveq	#0,d7
	move.b	d0,d7
	lsl.w	#2,d7
	move.l	0(a4,d7),d2	;get features
	btst	#VOWELBIT,d2	;vowel?
	bne	strloop		;yes, next case
*	btst	#LIQUIDBIT,d2	;no,liquid?
*	bne	strloop		;yes, don't destress, next case
	and.b	#UNSTRESSED,(a3) ;no, destress
	bra	des1

strend	rts
	END
	
