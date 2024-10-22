**
**	$Id: mapansi.asm,v 36.8 91/02/12 10:58:53 darren Exp $
**
**      keymap library ANSI to IECLASS_RAWKEY conversion
**
**      (C) Copyright 1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**
	SECTION	keymap

**	Includes

	INCLUDE	"kldata.i"

	INCLUDE	"devices/inputevent.i"


**	Exports

	XDEF	MapANSI


**	Locals

MAXDEADPAIRS	EQU	15

 STRUCTURE	mapANSI,0
    LABEL   ma_BestWords		; long containing following 2 words
    UWORD   ma_BestLength
    UWORD   ma_BestCount
    LABEL   ma_Parameters
    ULONG   ma_Count			; from d0
    ULONG   ma_Length			; from d1
    CPTR    ma_String			; from a0
    CPTR    ma_Buffer			; from a1
    CPTR    ma_Keymap			; from a2
    LONG    ma_Actual			; result
    ULONG   ma_Local			; local store for non-indexed entry
    STRUCT  ma_DeadPairs,MAXDEADPAIRS*2
    LABEL   ma_DeadWords		; long containing following 2 words
    UWORD   ma_DeadFactor		;
    UWORD   ma_DoubleDeadFactor		; 
    LABEL   mapANSI_SIZEOF

**	Assumptions

	IFNE	ma_BestWords+ma_BestLength
	FAIL	"ma_BestWords, ma_BestLength not zero, recode"
	ENDC
	IFNE	km_LoKeyMapTypes
	FAIL	"km_LoKeyMapTypes not zero, recode"
	ENDC


******* keymap.library/MapANSI ***************************************
*
*   NAME
*	MapANSI -- Encode an ANSI string into keycodes. (V36)
*
*   SYNOPSIS
*	actual = MapANSI( string, count, buffer, length, keyMap )
*	D0                A0      D0     A1      D1      A2
*
*	LONG MapANSI( STRPTR, LONG, STRPTR, LONG, struct KeyMap * );
*
*   FUNCTION
*	This console function converts an ANSI byte string to the
*	code/qualifier pairs of type IECLASS_RAWKEY that would
*	generate the string and places those pairs in a buffer.
*	A code/qualifier pair is a pair of bytes suitable for
*	putting in the ie_Code and low byte of ie_Qualifier,
*	and for subsequent events, shifted to the ie_Prev1DownCode/
*	ie_Prev1DownQual then ie_Prev2DownCode/ie_Prev2DownQual
*	pairs for any dead or double dead key mapping.
*	
*
*   INPUTS
*	string - the ANSI string to convert.
*	count - the number of characters in the string.
*	buffer - a byte buffer large enough to hold all anticipated
*	    code/qualifier pairs generated by this conversion.
*	length - maximum anticipation, i.e. the buffer size in bytes
*	    divided by two (the size of the code/qualifier pair).
*	keyMap - a KeyMap structure pointer, or null if the default
*	    key map is to be used.
*
*   RESULT
*	actual - the number of code/qualifier pairs in the buffer,
*	    or negative to describe an error (see below).
*
*   EXAMPLE
*	...
*	#include <devices/inputevent.h>
*	
*	#define	STIMSIZE	3	\* two dead keys, one key *\
*	unsigned char rBuffer[STIMSIZE*2];
*	...
*	    KeymapBase = (struct Library *) OpenLibrary("keymap.library", 0);
*	    ...
*	    event.ie_NextEvent = 0;
*	    event.ie_Class = IECLASS_RAWKEY;
*	    event.ie_SubClass = 0;
*	
*	    \* prove keymap code completeness and MapANSI reversibility *\
*	    for (code = 0; code < 256; code++) {
*		buffer[0] = code;
*		actual = MapANSI(buffer, 1, rBuffer, STIMSIZE, 0);
*		r = rBuffer;
*		event.ie_Prev2DownCode = 0;
*		event.ie_Prev2DownQual = 0;
*		event.ie_Prev1DownCode = 0;
*		event.ie_Prev1DownQual = 0;
*		switch (actual) {
*		    case -2:
*			printf("MapANSI internal error");
*			goto reportChar;
*		    case -1:
*			printf("MapANSI overflow error");
*			goto reportChar;
*		    case 0:
*			printf("MapANSI ungeneratable code");
*			goto reportChar;
*	
*		    case 3:
*			event.ie_Prev2DownCode = *r++;
*			event.ie_Prev2DownQual = *r++;
*		    case 2:
*			event.ie_Prev1DownCode = *r++;
*			event.ie_Prev1DownQual = *r++;
*		    case 1:
*			event.ie_Code = *r++;
*			event.ie_Qualifier = *r;
*	
*			actual = MapRawKey(&event, buffer, BUFFERLEN, 0);
*			if ((actual != 1) || (buffer[0] != code)) {
*			    printf("MapANSI not reversible");
*			    for (i = 0; i < actual; i++)
*				ReportChar(buffer[i]);
*			    printf(" from");
*	reportChar:
*			    ReportChar(code);
*			    printf("\n");
*			}
*		}
*	    }
*	...
*
*   ERRORS
*	if actual is 0, a character in the string was not generatable
*	    from the keyMap.
*	if actual is -1, a buffer overflow condition was detected.
*	if actual is -2, an internal error occurred (e.g. no memory)
*
*   SEE ALSO
*	devices/inputevent.h, devices/keymap.h
*
**********************************************************************
qualifierOnesCount:			; number of bits set in a nibble
		dc.b	0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4



;------ deadCount ----------------------------------------------------
;
;	d0	type
;
;	d2,ccr	last index for dead entry, or negative for not KCB_DEAD
;	a0	preserved
;
deadCount:
		moveq	#0,d2
		btst	#KCB_NOP,d0
		bne.s	dcNotDead
		btst	#KCB_DEAD,d0
		beq.s	dcNotDead

		;-- type is dead determine how many entries
		moveq	#0,d1
		move.b	d0,d1
		and.b	#7,d1		; only KC SHIFT, ALT, and CONTROL count
		move.b	qualifierOnesCount(pc,d1.w),d1
		bset.b	d1,d2
		add.w	d2,d2		; 2 bytes for each entry
dcNotDead:
		subq.w	#2,d2		; start w/ last index
		rts


;------ checkQualifierOnes -------------------------------------------
;
;	d0	qual1
;	d1	qual2
;
;	ccr	qual1 ? qual2
;	a0	preserved
;
checkQualifierOnes:
		and.w	#$ff,d0
		move.b	qualifierOnesCount(pc,d0.w),d0
		and.w	#$ff,d1
		sub.b	qualifierOnesCount(pc,d1.w),d0
		rts


;------ maxDeadIndex -------------------------------------------------
;
;	d0	type
;	a0	entry
;	a6	mapANSI variables
;
maxDeadIndex:
		move.l	d2,-(a7)
		bsr.s	deadCount
		blt.s	mdiDone

mdiLoopEntry:
		btst	#DPB_DEAD,0(a0,d2.w)
		beq.s	mdiNextEntry
		;-- entry is DPF_DEAD
		move.b	1(a0,d2.w),d1	; get index
		move.b	d1,d0
		and.w	#DP_2DINDEXMASK,d0 ; get portion that is dead factor
		cmp.w	ma_DeadFactor(a6),d0
		ble.s	mdiChkDouble
		move.w	d0,ma_DeadFactor(a6)	; new maximum dead factor
mdiChkDouble:
		lsr.b	#DP_2DFACSHIFT,d1
		beq.s	mdiNextEntry
		;-- entry participates in double dead keys
		cmp.w	ma_DoubleDeadFactor(a6),d0
		ble.s	mdiNextEntry
		move.w	d0,ma_DoubleDeadFactor(a6) ; new max double dead factor
mdiNextEntry:
		subq.w	#2,d2
		bge.s	mdiLoopEntry

mdiDone:
		move.l	(a7)+,d2
		rts


;------ indexedType --------------------------------------------------
;
;	d0.b	type
;	d1.b	index
;
;	a0	preserved
;
indexedType:
		movem.l	d2/d3,-(a7)
		move.b	d0,d2		; cache type
		moveq	#0,d0		; initialize result
itLoopTest:
		tst.b	d1		; test index
itLoopBranch:
		beq.s	itDone
		move.b	d2,d3		; find msb set in remaining type
		neg.b	d3		;
		and.b	d2,d3		;
		eor.b	d3,d2		; clear this bit in type
		lsr.b	#1,d1		; shift and test msb in index
		bcc.s	itLoopBranch	; index bit clear, result unaffected
		or.b	d3,d0		; index bit set, set appro result bit
		bra.s	itLoopTest

itDone:
		movem.l	(a7)+,d2/d3
		rts


;------ findDeadPairs ------------------------------------------------
;
;	d0.b	type
;	d1.b	code
;	a0	entry
;	a6	mapANSI variables
;
findDeadPairs:
		movem.l	d2-d5,-(a7)
		move.w	d0,d4
		move.b	d1,d5
		bsr	deadCount
		blt.s	fdpDone

fdpLoopEntry:
		btst	#DPB_DEAD,0(a0,d2.w)
		beq.s	fdpNextEntry
		;-- entry is DPF_DEAD
		moveq	#0,d3
		move.b	1(a0,d2.w),d3			; get index
		and.b	#DP_2DINDEXMASK,d3
		subq.w	#1,d3				; origin from 1.. to 0..
		add.w	d3,d3				; byte pairs
		tst.b	ma_DeadPairs(a6,d3.w)
		bmi.s	fdpFirstEntry
		;-- check for better qualifier situation
		move.b	d2,d0
		move.b	ma_DeadPairs+1(a6,d3.w),d1
		bsr	checkQualifierOnes
		bge.s	fdpNextEntry
fdpFirstEntry:
		move.b	d5,ma_DeadPairs(a6,d3.w)	; deadPairs[index].code
		move.b	d4,d0				; recover type
		move.b	d2,d1				; convert pair offset to
		lsr.b	#1,d1				;   index for type
		bsr	indexedType
		move.b	d0,ma_DeadPairs+1(a6,d3.w)	; deadPairs[index].qual
fdpNextEntry:
		subq.w	#2,d2
		bge.s	fdpLoopEntry

fdpDone:
		movem.l	(a7)+,d2-d5
		rts


;------ scanMap ------------------------------------------------------
;
;	d0.b	type
;	d1	code
;	a0	entry
;	a5	buffer for code/qual pairs
;	a6	mapANSI variables
;
;------
;
;	d2	map entry type offset
;	d3	map entry offset limit
;	d4	dead index
;	d5	char
;	d6	type
;	d7	code
;	a2	entry
;
scanMap:
		movem.l	d2-d7/a2,-(a7)
		btst	#KCB_NOP,d0
		bne	smDone
		move.w	d0,d6		; cache type
		move.w	d1,d7		; cache code
		move.l	a0,a2		; cache entry

		;-- generate 2**<number of bits set in type>
		and.w	#$f,d0
		lea	qualifierOnesCount(pc),a0
		move.b	0(a0,d0.w),d0
		moveq	#0,d3
		bset	d0,d3
		moveq	#0,d2

		btst	#KCB_STRING,d6
		beq.s	smChkBestWidth

		;-- KCF_STRING type: look for string match
smsLoop:
		moveq	#0,d4
		move.b	d2,d4
		add.b	d4,d4
		move.b	0(a2,d4.w),d4	; string size
		beq.s	smsNext

		cmp.w	ma_BestCount(a6),d4
		blt.s	smsNext		; not as good as match already got
		cmp.l	ma_Count(a6),d4
		bgt.s	smsNext		; not this many bytes in string to match

		moveq	#0,d1
		move.b	d2,d1
		add.b	d1,d1
		move.b	1(a2,d1.w),d1	; string offset
		ext.w	d1
		lea	0(a2,d1.w),a0	; string address

		move.l	ma_String(a6),a1
		;--	string compare
		move.w	d4,d1
		moveq	#0,d0		; clear ccr to eq
		bra.s	smsMatchDBF
smsMatchLoop:
		cmp.b	(a0)+,(a1)+
smsMatchDBF:
		dbne	d1,smsMatchLoop
		bne.s	smsNext

		;--	string matched
		cmp.w	ma_BestCount(a6),d4
		bne.s	smsCacheResult
		cmp.w	#1,(a6)		; ma_BestLength(a6)
		bne.s	smsCacheResult

		;--	check for better qualifier situation
		move.b	d2,d0
		move.b	1(a5),d1	; buffer[0] qual
		bsr	checkQualifierOnes
		bge.s	smsNext

		;--	save this match
smsCacheResult:
		move.w	#1,(a6)		; ma_BestLength(a6)
		move.w	d4,ma_BestCount(a6)
		move.b	d7,(a5)
		move.b	d6,d0
		move.b	d2,d1
		bsr	indexedType
		move.b	d0,1(a5)

smsNext:
		addq.w	#1,d2
		cmp.w	d2,d3
		bgt.s	smsLoop
		bra	smDone


		;-- no single character generation can beat an existing multi
smChkBestWidth:
		cmp.w	#1,ma_BestCount(a6)
		bgt	smDone

		;-- get character to match
		move.l	ma_String(a6),a0
		move.b	(a0),d5

smChkDead:
		btst	#KCB_DEAD,d6
		beq	smPlain

		;-- KCF_DEAD type

smdLoop:
		move.w	d2,d1
		add.b	d1,d1
		move.b	0(a2,d1.w),d0
		bne.s	smdChkDeadMod

		;-- simple character here
		cmp.b	1(a2,d1.w),d5
		bne	smdNext
smdSingleMatch:
		bsr.s	smSingleMatch
		bra	smdNext


;------ smSingleMatch
		;--	single character matched
smSingleMatch:
		;--	check if already exists a single character match
		cmp.w	#1,(a6)		; ma_BestLength(a6)
		bne.s	smsmCache

		;--	compare for better qualifier situation
		move.b	d2,d0
		move.b	1(a5),d1	; buffer[0] qual
		bsr	checkQualifierOnes
		bge.s	smsmRts

smsmCache
		;--	cache single character
		move.l	#$00010001,(a6)	; ma_BestLength = ma_BestCount = 1
		move.b	d7,(a5)
		move.b	d6,d0
		move.b	d2,d1
		bsr	indexedType
		move.b	d0,1(a5)
smsmRts:
		rts
;.......


smdChkDeadMod:
		btst	#DPB_MOD,d0
		beq	smdNext

		;-- DPF_MOD character here
		move.b	1(a2,d1.w),d1
		ext.w	d1
		lea	0(a2,d1.w),a0

		cmp.b	(a0)+,d5
		beq.s	smdSingleMatch	; un-dead character matched

		cmp.w	#1,(a6)		; ma_BestLength(a6)
		beq	smdNext		; cannot beat 1 w/ a dead key pair

		moveq	#1,d4

		;-- dead+ character testing
smdDeadLoop:
		cmp.b	(a0)+,d5
		bne.s	smdDeadNext

		;--	dead+ character matched
		tst.w	ma_BestCount(a6) ; check if any match so far
		beq.s	smdCacheDead

		;--	check if there already exists a dead+ match
		cmp.w	#2,(a6)		; ma_BestLength(a6)
		bne.s	smdCacheDead

		;--	compare for better qualifier situation
		move.b	d2,d0
		move.b	3(a5),d1	; buffer[1] qual
		bsr	checkQualifierOnes
		bge.s	smdDeadNext

		;--	try to cache dead+ pair
smdCacheDead:
		moveq	#2,d0
		move.w	d0,(a6)		; ma_BestLength(a6)

		;--	check that the destination is large enough
		cmp.l	ma_Length(a6),d0
		bgt	smdNext		; don't try any more dead keys

		;--	cache dead+ pair
		move.w	#1,ma_BestCount(a6)
		lea	ma_DeadPairs(a6),a1
		move.w	d4,d0
		add.w	d0,d0
		move.b	-2(a1,d0.w),(a5)
		move.b	-1(a1,d0.w),1(a5)
		move.b	d7,2(a5)
		move.b	d6,d0
		move.b	d2,d1
		bsr	indexedType
		move.b	d0,3(a5)
smdDeadNext:
		addq.w	#1,d4
		cmp.w	ma_DeadFactor(a6),d4
		blt.s	smdDeadLoop


		cmp.w	#2,(a6)		; ma_BestLength(a6)
		beq	smdNext		; cannot beat 2 w/ a dead key triple
		bra.s	smdDDeadNext	; (know that next is skipped anyway)

		;-- dead++ character testing
smdDDeadLoop:
		cmp.b	(a0)+,d5
		bne.s	smdDDeadNext

		;--	dead+ character matched
		tst.w	ma_BestCount(a6) ; check if any match so far
		beq.s	smdCacheDDead

		;--	check if there already exists a dead++ match
		cmp.w	#3,(a6)		; ma_BestLength(a6)
		bne.s	smdCacheDDead

		;--	compare for better qualifier situation
		move.b	d2,d0
		move.b	5(a5),d1	; buffer[2] qual
		bsr	checkQualifierOnes
		bge.s	smdDDeadNext

		;--	try to cache dead++ pair
smdCacheDDead:
		moveq	#3,d0
		move.w	d0,(a6)		; ma_BestLength(a6)

		;--	check that the destination is large enough
		cmp.l	ma_Length(a6),d0
		bgt.s	smdNext		; don't try any more dead keys

		;--	cache dead+ pair
		move.w	#1,ma_BestCount(a6)
		lea	ma_DeadPairs(a6),a1
		moveq	#0,d0
		move.w	d4,d0
		divu	ma_DeadFactor(a6),d0
		add.w	d0,d0
		move.b	-2(a1,d0.w),(a5)
		move.b	-1(a1,d0.w),1(a5)
		swap	d0
		add.w	d0,d0
		move.b	-2(a1,d0.w),2(a5)
		move.b	-1(a1,d0.w),3(a5)
		move.b	d7,4(a5)
		move.b	d6,d0
		move.b	d2,d1
		bsr	indexedType
		move.b	d0,5(a5)

smdDDeadNext:
		addq.w	#1,d4
		cmp.w	ma_DoubleDeadFactor(a6),d4
		bge.s	smdNext
		;--	ensure this is not the beginning of a set
		moveq	#0,d0
		move.l	d4,d0
		divu	ma_DeadFactor(a6),d0
		swap	d0
		tst.w	d0
		bne.s	smdDDeadLoop
		bra.s	smdDDeadNext	; cannot reach this character

smdNext:
		addq.w	#1,d2
		cmp.w	d2,d3
		bgt	smdLoop
		bra	smDone

qualifierExtraBytes:			; number of extra bytes associated
					; with non-special qualifier patterns
		dc.b	0,0,0,0,0,0,0,0	; the last is the special VANILLA case
		dc.b	0,0,0,8,0,8,8,16

		dc.w	%1110100000000000	; 

smPlain:
		;-- check if entry contains the data itself
		move.w	#%1110100000000000,d0
		btst	d6,d0
		bne.s	smpLoop

		;--	entry contains data itself
		move.l	a2,d1		; reverse the byte order
		rol.w	#8,d1
		swap	d1
		rol.w	#8,d1
		lea	ma_Local(a6),a2
		move.l	d1,(a2)		; make entry data usable with pointer

		;-- check for special vanilla case
		cmp.b	#KC_VANILLA,d6
		bne.s	smpLoop

		;--	KC_VANILLA has only 4 entries, not 1<<3 == 8
		moveq	#4,d3

smpLoop:
		;-- check for character match
		move.b	0(a2,d2.w),d0
		cmp.b	d5,d0
		bne.s	smpChkVanilla
		bsr	smSingleMatch

		;-- check for special vanilla case
smpChkVanilla:
		cmp.b	#KC_VANILLA,d6
		bne.s	smpNext

		;-- check if code is controllable
		move.b	d0,d1
		and.b	#$c0,d1
		cmp.b	#$40,d1
		bne.s	smpNext

		;-- check for control version of vanilla case
		and.b	#$1f,d0
		cmp.b	d5,d0
		bne.s	smpChkAltVanilla

		;--	check if already exists a single character match
		cmp.w	#1,(a6)		; ma_BestLength(a6)
		bne.s	smpCacheVanilla

		;--	compare for better qualifier situation
		move.b	d2,d0
		move.b	1(a5),d1	; buffer[0] qual
		bsr	checkQualifierOnes
		addq.b	#1,d0		; + CTRL
		bge.s	smpNext

		;--	cache single character + CTRL
smpCacheVanilla:
		move.l	#$00010001,(a6) ; ma_BestLength = ma_BestCount = 1
		move.b	d7,(a5)
		move.b	d6,d0
		move.b	d2,d1
		bsr	indexedType
		or.b	#KCF_CONTROL,d0
		move.b	d0,1(a5)
		bra.s	smpNext

smpChkAltVanilla:
		or.b	#$80,d0
		cmp.b	d5,d0
		bne.s	smpNext

		;--	check if already exists a single character match
		cmp.w	#1,(a6)		; ma_BestLength(a6)
		bne.s	smpCacheAltVanilla

		;--	compare for better qualifier situation
		move.b	d2,d0
		move.b	1(a5),d1	; buffer[0] qual
		bsr	checkQualifierOnes
		addq.b	#2,d0		; + ALT + CTRL
		bge.s	smpNext

		;--	cache single character + ALT + CTRL
smpCacheAltVanilla:
		move.l	#$00010001,(a6) ; ma_BestLength = ma_BestCount = 1
		move.b	d7,(a5)
		move.b	d6,d0
		move.b	d2,d1
		bsr	indexedType
		or.b	#KCF_ALT!KCF_CONTROL,d0
		move.b	d0,1(a5)

smpNext:
		addq.w	#1,d2
		cmp.w	d2,d3
		bgt	smpLoop

smDone:
		movem.l	(a7)+,d2-d7/a2
		rts


;------	_MapANSI -----------------------------------------------------
;
;	d0	count for string
;	d1	length for buffer
;	a0	string
;	a1	buffer
;	a2	keyMap
;	a6	keyMapLibrary
;
;	d0	actual
;
;------
;
;	d2	decrementing counter for keyMap entries
;	a2	keyMap
;	a3	incrementing keyMapTypes
;	a4	incrementing keyMap entries
;	a5	buffer
;	a6	mapANSI variables
;
MapANSI:
		movem.l	d2/a2-a6,-(a7)
		move.l	a2,d2
		bne.s	maGetLocals

		;-- use the default keymap
		move.l	kl_KMDefault(a6),a2

		;-- get a store for local variables
maGetLocals:
		sub.w	#mapANSI_SIZEOF,a7
		move.l	a7,a6
		movem.l	d0/d1/a0/a1/a2,ma_Parameters(a6)
		move.l	a1,a5
		clr.l	ma_Actual(a6)

		;-- find the number of dead keys
		clr.l	ma_DeadWords(a6)	; DeadFactor, DoubleDeadFactor
		lea	maxDeadIndex(pc),a2
		bsr.s	maLoopMap

		;-- set up local variables for dead keys
		moveq	#0,d0
		move.w	ma_DeadFactor(a6),d0
		beq	maScanMaps
		bra.s	maInitDeadPairs


;------ maLoopMap
maLoopMap:
		move.l	ma_Keymap(a6),a0
		move.l	(a0),a3			; km_LoKeyMapTypes(a2)
		move.l	km_LoKeyMap(a0),a4
		moveq	#$40-1,d2
maLoLoop:
		move.b	(a3)+,d0
		moveq	#$40-1,d1
		sub.w	d2,d1
		move.l	(a4)+,a0
		jsr	(a2)
		dbf	d2,maLoLoop

		move.l	ma_Keymap(a6),a0
		move.l	km_HiKeyMapTypes(a0),a3
		move.l	km_HiKeyMap(a0),a4
		moveq	#$38-1,d2
maHiLoop:
		move.b	(a3)+,d0
		moveq	#$40+$38-1,d1
		sub.w	d2,d1
		move.l	(a4)+,a0
		jsr	(a2)
		dbf	d2,maHiLoop
		rts
;......


maInitDeadPairs:
		;--	initialize dead pair codes to "uninitialized"
		lea	ma_DeadPairs(a6),a0
		bra.s	maUninitDeadPairsDBF

maUninitDeadPairsLoop:
		move.w	#$ffff,(a0)+		; init pair code to negative
maUninitDeadPairsDBF:
		dbf	d0,maUninitDeadPairsLoop

		;--	initialize dead pair codes
		lea	findDeadPairs(pc),a2
		bsr.s	maLoopMap

		;--	adjust dead factors to reflect size of dead entries
		addq.w	#1,ma_DeadFactor(a6)	; deadFactor = deadFactor + 1;
		move.w	ma_DoubleDeadFactor(a6),d0
		beq.s	maScanMaps
		addq.w	#1,d0			; doubleDeadFactor = deadFactor
		mulu	ma_DeadFactor(a6),d0	;   * (doubleDeadFactor + 1)
		move.w	d0,ma_DoubleDeadFactor(a6)

maScanMaps:
		tst.l	ma_Count(a6)		; more string to convert?
		ble.s	maFixQuals
		tst.l	ma_Length(a6)		; more room to put result?
		ble.s	maOverflow

		;-- scan the keymaps
		clr.l	(a6)			; ma_BestLength, ma_BestCount

		lea	scanMap(pc),a2
		bsr.s	maLoopMap

		;-- check the results
		moveq	#0,d0
		move.w	(a6),d0			; ma_BestLength(a6)
		bne.s	maGeneratable

		;--	string not generatable
		moveq	#0,d0
		bra.s	maError

maGeneratable:
		cmp.l	ma_Length(a6),d0
		ble.s	maEnoughBuffer

		;-- not enough room in buffer for this solution
maOverflow:
		moveq	#-1,d0
maError:
		move.l	d0,ma_Actual(a6)
		bra.s	maDone


		;-- increment the pointers
maEnoughBuffer:
		sub.l	d0,ma_Length(a6)
		add.l	d0,ma_Actual(a6)
		add.l	d0,d0		; convert to byte pairs
		add.l	d0,a5		; increment buffer
		move.w	ma_BestCount(a6),d0
		sub.l	d0,ma_Count(a6)
		add.l	d0,ma_String(a6)
		bra.s	maScanMaps


		;-- convert keymap qualifiers to inputevent qualifiers
maFixQuals:
		move.l	ma_Actual(a6),d2
		move.l	ma_Buffer(a6),a0
		bra.s	maFixQualsDBF

maFixQualsLoop:
		move.w	(a0),d0
		btst	#KCB_DOWNUP,d0
		beq.s	maFQConvert
		bset	#IECODEB_UP_PREFIX+8,d0
maFQConvert:
		move.b	d0,d1
		and.w	#7,d1
		move.b	type2QualifierMap(pc,d1.w),d0
		move.w	d0,(a0)+

maFixQualsDBF:
		dbf	d2,maFixQualsLoop

maDone:
		move.l	ma_Actual(a6),d0
		add.w	#mapANSI_SIZEOF,a7
		movem.l	(a7)+,d2/a2-a6
		rts

type2QualifierMap:			; map from type to input event qualifier
		dc.b	0,IEQUALIFIER_LSHIFT,IEQUALIFIER_LALT
		dc.b	IEQUALIFIER_LSHIFT+IEQUALIFIER_LALT
		dc.b	IEQUALIFIER_CONTROL
		dc.b	IEQUALIFIER_LSHIFT+IEQUALIFIER_CONTROL
		dc.b	IEQUALIFIER_LALT+IEQUALIFIER_CONTROL
		dc.b	IEQUALIFIER_LSHIFT+IEQUALIFIER_LALT+IEQUALIFIER_CONTROL

	END
