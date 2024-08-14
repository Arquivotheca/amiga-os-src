*****************************************************************************
*
* Patterns.s
*
*	Copyright (c) 1987-89 by ARP Authors.
*	Copyright (c) 1989 by Commodore-Amiga, Inc.
*	All Rights Reserved.
*
* This source file includes the pattern matching functions for AmigaDOS,
* PreParse(), PatternMatch(), FindFirst(), FindNext(), FreeAnchorChain().
*
* This file was created from the file "wildcard.s" from arp.library V1.3.0
* (V39.0 of arp.library)
*
* Functions here assume A6 as DOSBASE.
*
*	Created 7/5/89 by cdh
*	History:
*		11/20/89 cdh	Modify FindNext to insure an_Lock is valid
*		12/6/89  cdh	Args for DOS D1/D2. Error codes SetIoErr.
*				ParsePattern has buf length, fix NOT.
*		
*
*****************************************************************************

AbsExecBase	EQU	4


;	include	"exec/types.i"

;	include "exec/memory.i"

;[ rsd
;FUNCDEF     MACRO   * function
;_LVO\1	    EQU	    FUNC_CNT
;FUNC_CNT    SET     FUNC_CNT-6
;            ENDM
;
;FUNC_CNT    SET     LIB_NONSTD
;] rsd

;	include	"exec/exec_lib.i"

;	INCLUDE	"exec/libraries.i"
*	include	"dos/dos_lib.i"

;	include	"dos/dosasl.i"

*	include "exec/nodes.i"
*	include "exec/alerts.i"
*	include "exec/initializers.i"
*	include "exec/resident.i"
*	include "exec/ports.i"
*	include "exec/execbase.i"
*	include	"exec/ables.i"
;	include "dos/dosextens.i"

P_ANY		EQU	$80	; Token for '*' or '#?
P_SINGLE	EQU	$81	; Token for '?'
P_ORSTART	EQU	$82	; Token for '('
P_ORNEXT	EQU	$83	; Token for '|'
P_OREND 	EQU	$84	; Token for ')'
P_NOT		EQU	$85	; Token for '~'
P_NOTEND	EQU	$86	; Token for
P_NOTCLASS	EQU	$87	; Token for '^'
P_CLASS 	EQU	$88	; Token for '[]'
P_REPBEG	EQU	$89	; Token for '['
P_REPEND	EQU	$8A	; Token for ']'
P_STOP		EQU	$8B


;[ rsd
;	xdef	FindFirst,FindNext,FreeAnchorChain
;	xdef	PreParse,PreParseNoCase
;	xdef	_FindFirst,_MatchPattern,_MatchPatternNoCase
;
;DOS	macro
;	XREF	\2
;_\1	EQU	\2
;	ENDM
;
;	DOS	Examine,_examine
;	DOS	UnLock,_freeobj
;	DOS	ExNext,_exnext
;	DOS	IoErr,@getresult2
;	DOS	SetIoErr,@SetIoErr
;	DOS	ParentDir,_parent
;	DOS	Lock,_lock
;
;	XREF	@toUpper
;	XREF	Toupper
;	XREF	@rootstruct
;	XREF	_PathPart
;	XREF	_FilePart
;	XREF	_CurrentDir
;] rsd

;[ rsd
	xdef	_xParsePattern,_xMatchPattern
;] rsd

*****************************************************************************
*
* cdh 9/7/89
*	This macro allows use of the new dos.library function calls which
* are not yet available as LVO callouts.
*	The macro will call the ROM functions if they have been defined,
* in libraries/dos_lib.i, or if the functions have not yet been defined
* the macro will call them as xref'ed functions by the name of the function
* preceeded by an underscore.
*
*****************************************************************************

*
*	If these are changed, check state of a6 - some SYSCALLS don't have it
*

;[ rsd
;SYSCALL	macro
;	IFD	_LVO\1
;	ERROR! using lvo!
;	jsr	_LVO\1(A6)
;	ENDC
;	IFND	_LVO\1
;*	ERROR!!! You must supply _LVO\1
;*	xref	_\1
;	jsr	_\1
;	ENDC
;	ENDM
;
;SYSJMP	macro
;	IFD	_LVO\1
;	ERROR! using lvo!
;	jmp	_LVO\1(A6)
;	ENDC
;	IFND	_LVO\1
;*	ERROR!!! You must supply _LVO\1
;*	xref	_\1
;	jmp	_\1
;	ENDC
;	ENDM
;
;LINKEXE MACRO
;	move.l	a6,-(sp)
;	move.l	AbsExecBase,a6
;	jsr	_LVO\1(a6)
;	move.l	(sp)+,A6
;	ENDM
;] rsd

**********************************************************************
*
*	NAME
*	   PreParse -- Creat a tokenized string for `MatchPattern'
*
*	SYNOPSIS
*	   Status = PreParse(Source,Dest,DestLength)
*	     d0                D1    D2     D3
*
*	FUNCTION
*
*	INPUTS
*	  char *source - unparsed wildcard string to search for.
*         char *dest - output string, gets tokenized version of input.
*
*	RESULT
*	  BOOL IsWild - whether there were any wildcards in the string
*	
*	BUGS
*	  None known.
*
*	SEE ALSO
*	   MatchPattern, FindFirst/FindNext
*
*	Author:	CDH based on FindFirst by JAT
*	  edited 1/13/88 cdh add ' escape char
*	  total rewrite 11/2/88 cdh, changed everything from scratch
*
***********************************************************************

Ptr	equr	A2
Dest	equr	A1
WPending equr	D2
Buflen	equr	D3
TYpe	equr	D4
Curchr	equr	D1
NotPending equr D5

**
*
* IsWild: determine if Curchr is a wildcard character, and return the offset
*	  in the wildcard array.  NOTE: returns cc as equal for a wildcard,
*	  cc as not equal for no match!!!!!  Wierd!
*

NUM_WILDCARDS	EQU	10

*** NOTE - WildZ IS REFERENCED FOR FindAll !!!!
*** Careful: tables must match, as well as jumptable!

WildZ:	dc.b	P_ANY,P_NOT,P_CLASS,']',P_REPBEG
	dc.b	P_SINGLE,P_ORSTART,P_OREND,P_ORNEXT,0,0
**
	dc.b	"*~[]#?()|%'"
Wildstuff:	ds.w	0

IsWild:	lea	Wildstuff,A0 
	moveq	#NUM_WILDCARDS,D0
1$:	cmp.b	-(A0),Curchr
	 dbeq	D0,1$		; doesn't affect cc's!
	rts

*** Handle #?
ppAny:
	addq	#1,Ptr
	move.b	#P_ANY,Curchr
	bra	ppstore

*****
*
*	Here's the ParsePattern entrypoint.....
*
;[ rsd
;PreParseNoCase:
;	movem.l	D2-D6/Ptr/A3/a4,-(sp)
;	moveq	#1,d6
;	bra.s	preparse_common
;] rsd

_xParsePattern:
*	   Status = PreParse(Source,Dest,DestLength)
*	     d0                D1    D2     D3
	movem.l	d2-d7/a2-a6,-(sp)
	move.l	44+4(sp),d1
	move.l	44+8(sp),d2
	move.l	44+12(sp),d3
	bsr	ParsePattern
	movem.l	(sp)+,d2-d7/a2-a6
	rts

ParsePattern:
	movem.l	D2-D6/Ptr/A3/a4,-(sp)
	moveq	#0,d6
*
* Maintain a stack of P_ORSTART, P_REPBEG, and P_NOT's.  This allows us to
* match them, and allows us to put the P_REPEND's and P_NOTEND's in the
* right places.
*
* Get size of pattern, and allocate that much off stack
*
preparse_common:
	move.l	d1,a0
1$:	tst.b	(a0)+
	bne.s	1$
	sub.l	d1,a0		; 1 too big
	move.l	a0,d0
	addq.l	#1,d0		; 2 too big
	bclr.b	#0,d0		; make even - we're 1 or 2 too big - need
				; space for a 0 entry on top of stack!

	move.l	a7,a3		; save stack ptr (in case a4 gets unbalanced)
	move.l	a7,a4		; token stack ptr
	clr.b	-(a4)		; make top item 0 (things examine top)
	sub.l	d0,a7

* init stuff
	move.l	D1,Ptr		; source
	move.l	D2,A1		; dest

	moveq	#0,TYpe		; Return value
	moveq	#0,WPending
	moveq	#0,NotPending

*
* main looping point
*
ppLoop:	move.b	(Ptr)+,Curchr	; NULL HANDLED BY PPSTORE!!!
	bsr.s	IsWild		; returns offset into array of wildcard found
	 bne.s	ppRegular	; not a wildcard - store and loop

*
* We want optional use of "*" as a wildcard.  Use a flag in the rootnode.
*
;[ rsd
;	move.w	d0,-(sp)	; only a word
;	bsr	@rootstruct	; FIX! use dosbase!
;	move.l	d0,a0		; rootnode
;	move.w	(sp)+,d0
;
;	;-- careful - it's bit 24 of a longword!
;	btst.b	#RNB_WILDSTAR-24,rn_Flags(a0)
;	 bne.s	1$		; if set, allow "*"
;] rsd
*
** REJ - turned off '*' for Beta 1 - was commented out (cmp #3,d0, ble.s...)
*
	cmp.w	#0,D0			; is wildcard char is *, ignore
	 ble.s	ppRegular		; * is entry 0, and WildStar isn't
					; active

1$:	move.b	WildZ(pc,d0.w),Curchr	; May be a direct substitute char
	add.w	D0,D0			; word offset - bra.s == 2 bytes
	jmp	ppJump(pc,D0.w)

ppJump:
	;-- Must match WildZ table above!!!
	bra.s	ppstore		; *
	bra.s	ppStartNot	; ~
	bra.s	ppStartClass	; [
	bra.s	ppRegular	; ]  ( Ignore here )
	bra.s	ppStartRep	; #
	bra.s	ppstore		; ?
	bra.s	ppStartOr	; (
	bra.s	ppEndOr		; )
	bra.s	ppNextOr	; |
	bra.s	ppLoop		; % Ignore
* Tick ' comes here ...

	move.b	(Ptr)+,Curchr	; ' - Escape a wildcard character
	bsr	IsWild		; cc eq means it got a match
	 beq.s	ppstore		; Next char wild - it was Escaped...
	subq.l	#1,Ptr		; not wild - back up and reinsert '
	move.b	#$27,Curchr
	bra.s	ppRegular	; handle as normal character
*
ppEndOr:
	cmp.b	#P_ORSTART,(a4)+
	 bne	ppFail		; ) without (!!!
	bra.s	ppstore		; stores P_OREND, then checks for # and ~
*
ppNextOr:
	cmp.b	#P_ORSTART,(a4)
	 bne	ppFail		; Not in ()!  FIX!!! Handle x|y|z! ((a4) == 0)
	bra.s	ppstore		; stores P_ORNEXT

* Leave a trailing NOT alone ...
ppStartNot:
	tst.b	(Ptr)		; KLUDGE to help mg (micro-gnu-emacs)
	 beq.s	ppRegular
	bra.s	ppPushToken

*
* Here's where we generally store items in the destination...
*
ppstore:			; store a token (set return value for wildcards)
	moveq	#1,TYpe
ppRegular:			; store a character
	subq.l	#1,Buflen
	 ble.s	ppFail
	move.b	Curchr,d0
;[ rsd
;	tst.w	d6		; is this ParsePatternNoCase?
;	 beq.s	ppnocase
;	bsr	Toupper		; convert d0 to UC, no other regs touched
;] rsd
ppnocase:
	move.b	d0,(Dest)+
	 beq.s	ppDone

ppCleanup:
* These tokens need end markers after the next token (with () doing grouping)
* Check if there is a pending P_REPEND
	cmp.b	#P_REPBEG,(a4)
	 bne.s	10$			; not #, check ~
	moveq.b	#P_REPEND,d0
	bra.s	pop_token		; goes back to pp_Cleanup

* Check if there is a pending P_NOTEND
10$:	cmp.b	#P_NOT,(a4)		; top entry always 0, so it's safe
	 bne.s	pp_braloop		; both failed, loop

	moveq.b	#P_NOTEND,d0	
	;-- fall through

pop_token:
	;-- common 'add token and pop token stack' rtn
	addq.l	#1,a4			; drop token
	subq.l	#1,Buflen
	 ble.s	ppFail
	move.b	D0,(Dest)+		; add end token
	bra.s	ppCleanup		; check again - we may need more ends!

*** Handle #(pat)
ppStartRep:
	cmp.b	#'?',(Ptr)	; handle #? as a single token for speed
	 beq	ppAny
	; fall thru

ppStartOr:
ppPushToken:
	moveq	#1,TYpe
	subq.l	#1,Buflen
	 ble.s	ppFail
	move.b	Curchr,(Dest)+
	move.b	Curchr,-(a4)		; push token onto token stack

pp_braloop:
	bra	ppLoop			; done adding a token, loop

***** Handle P_CLASS stuff here->  ( [<char class>] )
ppStartClass:
	cmp.b	#'~',(Ptr)	; check for [~a-z]
	 bne.s	1$
	move.b	#P_NOTCLASS,Curchr
	addq.l	#1,Ptr		; drop ~
1$:	subq.l	#1,Buflen	; store up the characters in the class
	 ble.s	ppFail
	move.b	Curchr,(Dest)+	; P_CLASS (or NOTCLASS) first time...
	move.b	(Ptr)+,Curchr
	 beq.s	ppstore		; EOS premature ick.
	cmp.b	#$27,Curchr	; '
	 bne.s	10$
	move.b	(Ptr)+,Curchr
	 beq.s	ppstore		; EOS
	bra.s	1$		; don't compare against ]

10$:	cmp.b	#']',Curchr	; FIX!!!!! doesn't handle ']!!!!!!
	 bne.s	1$
	move.b	#P_CLASS,Curchr	; we're done
	bra.s	ppstore

* hit end of pattern.  check for unbalanced token stack
ppDone:
	tst.b	(a4)
	beq.s	ppend

* Come here on buffer overflow or other error
ppFail:
	moveq.l	#-1,TYpe

ppend:	move.l	TYpe,D0
	move.l	a3,a7			; restore stack ptr (drop token stack)
	movem.l	(sp)+,D2-D6/Ptr/A3/a4
	rts

*************************************************************************
*
*	NAME
*	   Matchpattern - perform a wildcard string match
*
*	SYNOPSIS
*	   result = MatchPattern(pat,str)
*	     D0 + Z-Flag	 A0  A1
*
*	FUNCTION
*	   This function implements BCPL-compatible string pattern matching.
*
*	   The input "pat" is the PreParsed pattern string.
*	   The input "str" is a NULL-terminated string to match against.
*
*	   The result in D0 is a boolean TRUE or FALSE value, FALSE = 0,
*	   TRUE = 1.  The zero flag is also set according to the result.
*
*	   The input pattern is identical to the BCPL pattern strings described
*	   in the AmigaDOS Users Manual for the LIST command, with the addition
*	   of the "*" symbol (see PreParse) if enabled by environment variable,
*	   BCPL.  Also controlled by the BCPL environment variable are
*	   character classes as defined below:
*
*	    [  Starts character class,   ]  Terminates character class
*	    If the first character following [ is ~, the class is negated.
*	    A range of characters may be specified with -
*	    Any other characters are compared with the tested char.
*	    To match against -, it must be the FIRST character in a range.
*	    To match against ], it must be escaped with '.
*
*	BUGS
*	  None known.
*
*	SEE ALSO
*
*	Author: Rewritten for ARP V1.3 by CDH
*		1-2-88 cdh  Rework OR code for recursion
*
*************************************************************************

* Register D0 holds current character, and temp values
*	   A0 is pattern string
*	   A1 is string to match

;[ rsd
;_MatchPatternNoCase:
;	move.l	D3,-(sp)
;	moveq	#1,d3
;	bra.s	match_common
;] rsd

_xMatchPattern:
*	   result = MatchPattern(pat,str)
*	     D0 + Z-Flag	 A0  A1
	movem.l	d2-d7/a2-a6,-(sp)
	move.l	44+4(sp),d1
	move.l	44+8(sp),d2
	bsr	MatchPattern
	movem.l	(sp)+,d2-d7/a2-a6
	rts

MatchPattern:
	move.l	D3,-(sp)
;[ rsd
;	moveq	#0,d3
;] rsd
match_common:
	move.l	D1,A0			; Get args internal froboz thanks.
	move.l	D2,A1
	bsr.s	Internal_PatternMatch
	move.l	(sp)+,d3
	rts

* End of pattern; if str is also at end, TRUE else FALSE
chekmatch:
	tst.b	(A1)
	 bne.s	MatchFail		; Failed return...

* P_STOP is a "special" case, returning TRUE, with A1 guaranteed
* to be at current position of STR, and A0 updated to next unused token...
* This is because it matches REPBEG to REPEND, and then tries to match the
* pattern at REPEND, until REPBEG to REPEND match fails.
do_P_STOP:
MatchOK:
	moveq	#1,D0			; Everybody happy return
	rts


* Case of ?, looking for a single character
do_P_SINGLE:
	tst.b	(A1)+
	 beq.s	MatchFail		; No char to match...
	;-- fall through to pm_Loop!!!
*
* Main looping point for PatternMatch - MUST return cc's!!!!!
*
Internal_PatternMatch:

**
* Matched up to NOTEND.  This is success, but NOT if the rest of the pattern
* wouldn't match!  So try to match the rest of the pattern, and if it still
* matches, we return success, aka failure.  If it fails, we return failure,
* which is success.  So effectively, we ignore NOTEND, it's only used if
* we fail to know where the rest of the pattern is.
*
do_P_NOTEND:
do_P_REPEND:
do_P_OREND:				; used to go to pm_Loop
	;-- fall through
pm_Loop:
	move.b	(A0)+,D0		; leaves a0 pointing past current token
	 beq.s	chekmatch		; End of pattern string
	cmp.b	#P_STOP,D0		; the 'highest' entry!!  CAREFUL!
	 ble.s	DoComplex		; Not a simple pattern, process...
	move.b	(a1)+,d1
;[ rsd
;	tst.w	d3
;	 beq.s	nocase
;	;-- assumes they called ParsePatternNoCase
;	exg	d0,d1			; convert d0 to upper, no other regs
;	bsr	Toupper			; are touched
;					; we don't care which is in which reg
;] rsd
nocase:	cmp.b	d1,D0			; a literal character
	 beq.s	pm_Loop			; Loop till end of string

MatchFail:
	moveq	#0,D0
	rts

**
* This is either #? or *
do_P_ANY:
	tst.b	(A0)			; nothing after #?/*
	 beq	MatchOK			; End of pattern - good match

*
* To handle */#?, try pattern matching after each character of the remaining
* input characters.  If starting at each of them all fail, then we have no
* match.
*
* Loop here for remainder of pending str matching continuation of pat
1$:	movem.l	A0/A1,-(sp)		; Need to save current context
	bsr	Internal_PatternMatch
	movem.l	(sp)+,A0/A1		; doesn't affect cc's...
	 bne.s	MatchOK			; Found a match.  Proceed.
	tst.b	(A1)+			; eat another character and repeat
	 bne.s	1$			; Loop on remaining chars...
	bra.s	MatchFail

**
* This is actually P_NOT... - NOTE: negates only the next token!
* I_P returns on P_NOTEND, with cc's set!!
*
* If the pattern matches, we fail immediately.  If the pattern doesn't match,
* consider this pattern to be P_ANY (#?), so we match all strings that do not
* match the pattern.  This is more intuitive in general.
*
* We must search for the NOTEND, because we don't know where it failed
*
do_P_NOT:
	movem.l	a0/a1,-(sp)		; save current position in string
	bsr	Internal_PatternMatch	; returns with cc's set!
	movem.l	(sp)+,a0/a1		; doesn't touch cc's!!!!
	 bne.s	MatchFail		; Match.  Don't look further.
	moveq	#0,D1

* find end of NOT.  Be careful to handle nesting.
	moveq	#0,d1
1$:	tst.b	(A0)			; a0 points past first P_NOT
	 beq.s	MatchOK			; Real Bad Condition. Barfola
	cmp.b	#P_NOT,(A0)
	 bne.s	2$
	addq	#1,D1			; handle ~(...~(...)...)
2$:	cmp.b	#P_NOTEND,(A0)+
	 bne.s	1$
	subq	#1,D1
	 bge.s	1$			; also handles ~(...)~(...)
	bra.s	do_P_ANY		; No match, consider same as #?.

***********************************************************************
*
* Complex pattern, use jump table processing...
*
***********************************************************************
DoComplex:
	add.w	D0,D0
	and.w	#$fe,D0			; Mask out extra bits
	jmp	JumpTab(pc,D0.w)

JumpTab:
	bra.s	do_P_ANY		; MUST match order of token values!
	bra.s	do_P_SINGLE
	bra.s	do_P_ORSTART
	bra.s	do_P_ORNEXT
	bra.s	do_P_OREND
	bra.s	do_P_NOT		; Confiscated for P_NOT P_NOTEND
	bra.s	do_P_NOTEND
	bra.s	do_P_NOTCLASS
	bra.s	do_P_CLASS
	bra.s	do_P_REPBEG
	bra.s	do_P_REPEND
	bra	do_P_STOP		; can be bra.l since it's last!
**
* Found a matched OR - but it may not be the match we want, so skip to end
*	of OR code and try it out.
do_P_ORNEXT:
	moveq	#0,D1

* Loop to end of OR condition, and return (do exactly the same as OREND)
* FIX! this doesn't try others after one match!!!!

1$:	tst.b	(A0)
	 beq	do_P_OREND		; Real Bad Condition. Barfola
	cmp.b	#P_ORSTART,(A0)		; FIX!! Handle a|b|c!!!
	 bne.s	2$
	addq.w	#1,D1			; handle (...|...(...|...))...
2$:	cmp.b	#P_OREND,(A0)+
	 bne.s	1$
	subq.w	#1,D1
	 bge.s	1$			; also handles (...|...)(...)
	bra	pm_Loop			; out of (...), match against rest

**
*	This is the beginning of a (...|...) sequence
*	It's a bit wierd.  I would have handled (...) by having Internal_P...
*	return on P_OREND, and continue matching here.  Instead, P_OREND
*	loops and tries to match the rest at it's level, possibly using
*	a bit more stack (luckily, Internal_PatternMatch uses little stack).
*	Also, having it return might make handling of ~ more predictable and
*	expected.  However, it seems to work reasonably well.
*
*	Late news flash: in order to make ~ work right, OREND _must_ return
*	immediately.  Therefore, ORSTART must go back to the loop on success.
*	In the case of ~(...) or #(...), the next token will be NOTEND or
*	REPEND, and we'll pop out another level to do_P_NOT or do_P_REPBEG.
*
*	Later flash: There is no need for OREND to pop out, now that we
*	understand ~.
*
do_P_ORSTART:
	movem.l	A0/A1,-(sp)
	bsr	Internal_PatternMatch	; pops out on fail or full success
	 beq.s	orFail
	addq.l	#8,sp			; drop old pointers!
	bra	MatchOK

	;-- I_P failed - find the next alternation of the pattern and
	;-- try again.  (i.e. search for next ORNEXT)
	;-- NOTE: if we go to MatchFail, a0 MUST be pointing past OREND!
orFail:
	movem.l	(sp)+,A0/A1		; restore pointers
	moveq	#0,D1			; Nesting
1$:	move.b	(A0)+,D0		; points to next symbol
	 beq	MatchFail		; shouldn't happen!!!!!!
	cmp.b	#P_ORSTART,D0		; handle (...(...|...)...|...)
	 bne.s	2$			; we need to find the next | at this
	 addq.w	#1,D1			; level.
2$:	cmp.b	#P_OREND,D0
	 bne.s	3$
	 subq.w	#1,D1
	  bmi	MatchFail		; we popped out.  Handles (...)(...|...)
3$:	tst.w	D1
	 bne.s	1$
	cmp.b	#P_ORNEXT,D0
	 bne.s	1$
	bra.s	do_P_ORSTART		; we found a OR_NEXT, try again

**
* put here so we're in bra.s range of switch
*
do_P_REPBEG:
	bra.s	real_REPBEG

**
*	Handle character classes - they start AND end with P_CLASS, since they
*	don't nest.
*
do_P_NOTCLASS:
	moveq	#1,d1
	bra.s	class_common
do_P_CLASS:
	moveq	#0,d1
class_common:
	move.b	(A1)+,D0		; Character to match against
	 beq.s	10$			; No character to match!

1$:	tst.b	(A0)
	 beq.s	10$			; Bad format pattern string!!!

	cmp.b	#P_CLASS,(A0)
	 beq.s	10$			; End of class, not found - skip, return

	cmp.b	(A0)+,D0
	 beq.s	20$			; Good match, go skip to P_CLASS

	cmp.b	#'-',(A0)
	 bne.s	1$			; Continue to loop unless RANGE

	addq	#1,A0			; Point to end of RANGE

	cmp.b	-2(A0),D0		; TRICKY!
	 bcs.s	2$			; bra if -2(a0) > d0, unsigned
	cmp.b	(A0),D0
	 bls.s	20$			; Good range matched get out!!!
					; was bcs 20$; beq 20$.  (i.e. bra if
					; (a0) >= d0, unsigned.)  Turns out
					; that bls does that (see moto books).
* No match
2$:	tst.b	(A0)
	 beq.s	10$
	cmp.b	#P_CLASS,(A0)+
	 bne.s	1$

* MatchClass returns..
*	Bad return to MatchFail, doesn't need to skip to P_CLASS...
*	If d1 != 0, then this is success
10$:	tst.b	d1
	 beq	MatchFail
	bra.s	25$

* Good match, skip to end of class
* if d1 != 0, this is failure
20$:	tst.b	d1
	 bne	MatchFail
25$:	tst.b	(A0)
	 beq.s	21$
	cmp.b	#P_CLASS,(A0)+
	 bne.s	25$
21$:	bra	pm_Loop


**
* This handles # not followed by ?.  Any number of the following character,
* class or alternation.
*
real_REPBEG:
	movem.l	A0/A1/A2,-(sp)		; CAREFUL! it uses these values!

* Skip until _matching_ P_REPEND.  Handles #(#a|b), etc

	moveq	#0,d1
1$:	move.b	(A0)+,D0
	 beq.s	BadRep			; no P_REPEND.  Ick. Ick.
	cmp.b	#P_REPBEG,D0
	 bne.s	2$
	addq.w	#1,d1			; hit another REPBEG
	bra.s	1$

2$:	cmp.b	#P_REPEND,D0
	 bne.s	1$
	subq.w	#1,d1			; d1.w = -1 after correct REPEND
	 bge.s	1$

	lea	-1(A0),A2		; Save positon of REPEND

	;-- Match against pat @P_REPEND, current str (0 instances)
	bsr	Internal_PatternMatch
	 bne.s	RepOK			; If yes, yippie go home

	;-- do we match the rest of the pattern yet??
	;-- (this makes 0 instances work also)
repLoop:
	;-- match against pattern including the repeat
	movem.l	(sp),A0/A1		; match against P_REPBEG to the end
	bsr	Internal_PatternMatch
	 bne.s	RepOK			; matches 1 instance plus the rest

	;-- skip one instance of the pattern
	;-- P_STOP makes it stop without checking the # of characters
	;-- matched, and return here with a1 updated.

	movem.l	(sp),A0/A1		; get ptrs back
	move.b	#P_STOP,(a2)		; nuke REPEND to stop evaluation
	bsr	Internal_PatternMatch
	move.b	#P_REPEND,(a2)		; restore pattern!
	tst.l	d0
	 beq.s	BadRep			; doesn't match the repeat pattern

	cmp.l	4(sp),A1		; Hit REPEND without matching anything
	 beq.s	BadRep			; Match was NULL, Futz.

	;-- update string ptr past an instance of the pattern
	move.l	a1,4(sp)		; Match was OK, loop
	bra.s	repLoop			; we now try the rest of the pattern
					; again, or loop until we get no more
					; matches of the repeated pattern.
RepOK:	movem.l	(sp)+,A0/A1/A2
	bra	MatchOK

BadRep:	movem.l	(sp)+,A0/A1/A2
	bra	MatchFail

