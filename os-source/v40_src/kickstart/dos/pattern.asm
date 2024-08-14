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


	include	"exec/types.i"
	include "exec/memory.i"

FUNCDEF     MACRO   * function
_LVO\1	    EQU	    FUNC_CNT
FUNC_CNT    SET     FUNC_CNT-6
            ENDM

FUNC_CNT    SET     LIB_NONSTD

	include	"exec/exec_lib.i"
	include	"exec/libraries.i"
	include	"dos/dosasl.i"
	include "dos/dosextens.i"

	xdef	FindFirst,FindNext,FreeAnchorChain
	xdef	PreParse,PreParseNoCase
	xdef	_FindFirst,_MatchPattern,_MatchPatternNoCase

DOS	macro
	XREF	\2
_\1	EQU	\2
	ENDM

	DOS	Examine,_examine
	DOS	UnLock,_freeobj
	DOS	ExNext,_exnext
	DOS	IoErr,@getresult2
	DOS	SetIoErr,@SetIoErr
	DOS	ParentDir,_parent
	DOS	Lock,_lock
	XREF	@toUpper
	XREF	Toupper
	XREF	@rootstruct
	XREF	_PathPart
	XREF	_FilePart
	XREF	_CurrentDir

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

SYSCALL	macro
	IFD	_LVO\1
	ERROR! using lvo!
	jsr	_LVO\1(A6)
	ENDC
	IFND	_LVO\1
*	ERROR!!! You must supply _LVO\1
*	xref	_\1
	jsr	_\1
	ENDC
	ENDM

SYSJMP	macro
	IFD	_LVO\1
	ERROR! using lvo!
	jmp	_LVO\1(A6)
	ENDC
	IFND	_LVO\1
*	ERROR!!! You must supply _LVO\1
*	xref	_\1
	jmp	_\1
	ENDC
	ENDM

LINKEXE MACRO
	move.l	a6,-(sp)
	move.l	AbsExecBase,a6
	jsr	_LVO\1(a6)
	move.l	(sp)+,A6
	ENDM


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
*	  LONG IsWild - whether there were any wildcards in the string
*	  returns -1 if there is a buffer overflow
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

*****
*
*	Here's the ParsePattern entrypoint.....
*
PreParseNoCase:
	movem.l	D2-D6/Ptr/A3/a4,-(sp)
	moveq	#1,d6
	bra.s	preparse_common

**
*
* IsWild: determine if Curchr is a wildcard character, and return the offset
*	  in the wildcard array.  NOTE: returns cc as equal for a wildcard,
*	  cc as not equal for no match!!!!!  Wierd!
*

*** NOTE - WildZ IS REFERENCED FOR FindAll !!!!
*** Careful: tables must match, as well as jumptable!
*** Note: FindNext counts on P_ANY being first here!

WildZ:	dc.b	P_ANY,P_NOT,P_CLASS,']',P_REPBEG
	dc.b	P_SINGLE,P_ORSTART,P_OREND,P_ORNEXT,0,0
**
WildChars:
	dc.b	"*~[]#?()|%'"
Wildstuff:	ds.w	0

NUM_WILDCARDS	EQU	(Wildstuff-WildChars)

IsWild:	lea	Wildstuff,A0 
	moveq	#NUM_WILDCARDS-1,D0	; dbra loop...
1$:	cmp.b	-(A0),Curchr
	 dbeq	D0,1$		; doesn't affect cc's!
	rts

*** Handle #?
ppAny:
	addq	#1,Ptr
	move.b	#P_ANY,Curchr
	bra	ppstore

PreParse:
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
				; each token can only push 1 value max
	move.l	a7,a3		; save stack ptr (in case a4 gets unbalanced)
	move.l	a7,a4		; token stack ptr
	clr.b	-(a4)		; make top item 0 (things examine top)
	sub.l	d0,a7

* init stuff
	move.l	D1,Ptr		; source

	moveq	#ERROR_BAD_TEMPLATE,d1
	SYSCALL	SetIoErr	; call before we dest dest from d2

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
	move.w	d0,-(sp)	; only a word
	bsr	@rootstruct	; FIX! use dosbase!  FIX! must not destroy a1!
	move.l	d0,a0		; rootnode
	move.w	(sp)+,d0

	;-- careful - it's bit 24 of a longword!
	btst.b	#RNB_WILDSTAR-24,rn_Flags(a0)
	 bne.s	1$		; if set, allow "*"
*
** REJ - turned off '*' for Beta 1 - was commented out (cmp #3,d0, ble.s...)
*
	cmp.w	#0,D0			; is wildcard char is *, ignore
	 ble.s	ppRegular		; * is entry 0, and WildStar isn't
					; active

1$:	move.b	WildZ(pc,d0.w),Curchr	; May be a direct substitute char
	add.w	D0,D0			; word offset - bra.s == 2 bytes
	jmp	ppJump(pc,D0.w)

* Leave a trailing NOT alone ...
ppStartNot:
	tst.b	(Ptr)		; KLUDGE to help mg (micro-gnu-emacs)
	 bne.s	ppPushToken
	moveq	#'~',Curchr	; replace token with '~'
	bra.s	ppRegular	; store it as a normal character

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
	; fall through....
*
* Here's where we generally store items in the destination...
*
ppstore:			; store a token (set return value for wildcards)
	moveq	#1,TYpe
ppRegular:			; store a character
	subq.l	#1,Buflen
	 ble.s	ppFail
	move.b	Curchr,d0
	tst.w	d6		; is this ParsePatternNoCase?
	 beq.s	ppnocase
	bsr	Toupper		; convert d0 to UC, no other regs touched
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
	move.l	Curchr,d0	; for toupper
	tst.w	d6		; is this ParsePatternNoCase?
	 beq.s	5$		; no
	bsr	Toupper		; convert d0 to UC, no other regs touched
5$	move.b	d0,(Dest)+	; P_CLASS (or NOTCLASS) first time...
	move.b	(Ptr)+,Curchr
	 beq.s	ppFail		; EOS premature ick.
	cmp.b	#$27,Curchr	; '-'
	 bne.s	10$
	move.b	(Ptr)+,Curchr
	 beq.s	ppFail		; EOS
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
	tst.l	Buflen		; did we run out of space?
	 bgt.s	1$
	moveq	#ERROR_LINE_TOO_LONG,d1
	SYSCALL	SetIoErr

1$:	moveq	#-1,TYpe

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

_MatchPatternNoCase:
	movem.l	D3/d4/a2,-(sp)
	moveq	#1,d3
	bra.s	match_common

_MatchPattern:
	movem.l	D3/d4/a2,-(sp)
	moveq	#0,d3
match_common:
	move.l	D1,A2			; Get args internal froboz thanks.
	moveq	#0,d1
	SYSCALL	SetIoErr		; default is no error
	move.l	a2,a0			; get first arg back
	move.l	D2,A1
	move.w	#(1500-256)/12,d4	; # of time we can recurse
					; 12 bytes per resursion, 256 bytes
					; kept free for task switch safety.
	move.l	a7,a2			; keep stack ptr for error out!
	bsr.s	Internal_PatternMatch
match_exit:
	movem.l	(sp)+,d3/d4/a2
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
	;-- increment nesting counter
	addq.w	#1,d4

	moveq	#1,D0			; Everybody happy return
	rts
*
* Main looping point for PatternMatch - MUST return cc's!!!!!
*
Internal_PatternMatch:

	;-- decrement recursion counter, return on error
	subq.w	#1,d4
	 bne.s	pm_Loop			; we're fine

	move.l	a2,a7			; restore stack ptr
	move.l	#ERROR_TOO_MANY_LEVELS,d1
	SYSCALL	SetIoErr
	moveq	#0,d0			; return failure
	bra.s	match_exit

* Case of ?, looking for a single character
do_P_SINGLE:
	tst.b	(A1)+
	 beq.s	MatchFail		; No char to match...
	;-- fall through to pm_Loop!!!
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
	tst.w	d3
	 beq.s	nocase
	;-- assumes they called ParsePatternNoCase
	exg	d0,d1			; convert d0 to upper, no other regs
	bsr	Toupper			; are touched
					; we don't care which is in which reg
nocase:	cmp.b	d1,D0			; a literal character
	 beq.s	pm_Loop			; Loop till end of string

MatchFail:
	;-- increment nesting counter
	addq.w	#1,d4

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
	 beq	MatchFail		; No character to match! (Fail either
					; class or not-class!)
					; (not-class matchs 1 character not in
					; the class.)

	;-- must convert to upper case if case-insensitive!
	tst.w	d3
	 beq.s	class_nocase
	;-- assumes they called ParsePatternNoCase
					; convert d0 to upper, no other regs
	bsr	Toupper			; are touched
					; we don't care which is in which reg
class_nocase:

1$:	tst.b	(A0)
	 beq.s	class_nomatch		; Bad format pattern string!!!

	cmp.b	#P_CLASS,(A0)
	 beq.s	class_nomatch		; End of class, not found - skip, return

	cmp.b	(A0)+,D0
	 beq.s	class_match		; Good match, go skip to P_CLASS

	cmp.b	#'-',(A0)
	 bne.s	1$			; Continue to loop unless RANGE

	addq	#1,A0			; Point to end of RANGE

	cmp.b	-2(A0),D0		; TRICKY!
	 bcs.s	2$			; bra if -2(a0) > d0, unsigned
	cmp.b	(A0),D0
	 bls.s	class_match		; Good range matched get out!!!
					; was bcs 20$; beq 20$.  (i.e. bra if
					; (a0) >= d0, unsigned.)  Turns out
					; that bls does that (see moto books).
* No match
2$:	tst.b	(A0)
	 beq.s	class_nomatch
	cmp.b	#P_CLASS,(A0)+
	 bne.s	1$

* MatchClass returns..
*	Bad return to MatchFail, doesn't need to skip to P_CLASS...
*	If d1 != 0, then this is success
class_nomatch:
	tst.b	d1
	 beq	MatchFail
	bra.s	class_done

* Good match, skip to end of class
* if d1 != 0, this is failure
class_match:
	tst.b	d1
	 bne	MatchFail
class_done:				; eat the class, then continue
	tst.b	(A0)
	 beq.s	21$
	cmp.b	#P_CLASS,(A0)+
	 bne.s	class_done
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

***********************************************************************
*
* New FindFirst/FindNext support follows here.
*
***********************************************************************

AnchorBase	equr	A3		; struct AnchorPath root
BasePath	equr	A5		; Start of current substring
LastPath	equr	A4		; End of current clean string (no P_)
CurPath		equr	A2		; Current substring point
Wild		equr	D3

* These are used by FindNext
Node		equr	A2
LockReg		equr	D3
* AnchorBase is A3

***********************************************************************
*
*  Bool = FindFirst( Pattern,AnchorBase )
*   D0  =		D1 ,    d2
*
*
***********************************************************************
FindFirst:
_FindFirst:
	movem.l	D2-D5/A2-A6,-(sp)

	move.l	D1,CurPath

	move.l	D2,AnchorBase
	clr.l	ap_Base(AnchorBase)	;*** Clear potent garbage

* Turn OFF memory error and simple bits!!!
	and.b	#~((1<<APB_NOMEMERR)+(1<<APB_ITSWILD)),ap_Flags(AnchorBase)

* allocate space for the pattern string
	move.l	CurPath,a0
1$:	tst.b	(a0)+
	bne.s	1$
	sub.l	CurPath,a0		; gives len+1, which is what we want
	move.l	a0,d0			; size
	asl.l	#1,d0			; we need at least twice that space
					; since 1 token can become 2
	move.l	d0,d5			; remember how much we allocated!
					; (d5 is used as space free in buffer)
	bsr	DosAllocMem
	move.l	d0,d4			; save pointer for free
	 beq	ff_error		; we count on AllocVec setting IoErr!

* Copy filename over to buffer, skipping to end of ":" before starting parsing
	move.l	d4,BasePath		; Set START of path substring 1'st time
	move.l	BasePath,A1		; destination
	move.l	CurPath,A0		; Save position to start PreParse...
	move.l	A1,LastPath		; Save position to resume parsing later
	move.l	d5,d1			; temp space counter

3$:	move.b	(CurPath)+,D0
	subq.l	#1,d1			; remember how much buf space is used
	move.b	D0,(A1)+
	 beq.s	5$			; end of string, no ':'
	cmp.b	#':',D0
	 bne.s	3$

	;-- hit a :, remember where!
	move.l	CurPath,A0		; Save position to start PreParse...
	move.l	A1,LastPath		; Save position to resume parsing later
	clr.b	(LastPath)		; make sure string is terminated
	move.l	d1,d5			; we really used the space

* PreParse the string, create tokenized wierdness
*	Here,	A0 = start of string beyond ":" (if any)
*		LastPath = start of buffer beyond ":"
*		BasePath = Buffer start
*
*	If there are wildcards, we need to know where they are.
*	If none, we can just lock the damn thing.
*
5$:	move.l	A0,D1			; src (passed in, past ':' if any)
	move.l	LastPath,D2		; dest (after ':' if any)
	move.l	d5,D3			; sizeof(dest) minus volume, if any
***	SYSCALL	ParsePattern
	bsr	PreParse		; can't fail.  we alloced enough mem
					; above to be safe. 
	addq.l	#1,d0			; error return is -1 (not enough buf)
	 beq	ff_error		; check for error anyways

	;-- start parsing after last : in string
	;-- if there was a ':', LastPath != Basepath (as if we hit a /)
	moveq	#0,Wild
	move.l	LastPath,CurPath

* Scan through the buffer to find any wildcards.
*	BasePath is the start of the current string
*	LastPath is the start or the character after last / seen
*	CurPath is the current position
*	Note: this operates on the parsed result!!!!!

ff_loop:
	move.b	(CurPath),D0
	 beq.s	ff_endloop		; Exit loop
	cmp.b	#'/',(CurPath)+
	 beq.s	20$
	cmp.b	#P_STOP,D0		; it's the 'largest' wildcard token
	 bgt.s	ff_loop			; since it's a SIGNED comparison, and
					; the values are $80-$8b, the most
					; negative values in a byte.
*
* A wildcard - split perhaps
	tst.b	Wild			; is the current node wild already?
	 bne.s	ff_loop			; yes, ITSWILD set already, continue

	;-- current node wasn't wild, now it will be.
	;-- if we have a / in the current accumulated node, make it an anchor
	;-- and start a new one for the pattern.
	move.l	LastPath,D0		; have we accumulated any directories?
	sub.l	BasePath,D0
	 beq.s	12$			* Nothing to add...
	bsr	AddAnchor		; add a node for the directories
	 bne.s	ff_error		* some sort of error locking it
	move.l	LastPath,BasePath	; reset accumulation point

	; we found a wildcard...
12$:
	moveq	#DDF_PatternBit,Wild
	bset.b	#APB_ITSWILD,ap_Flags(AnchorBase)	; Set "ItsWild" bit.
	bra.s	ff_loop

*
* Handle a subdir / (or parent!)
20$:	tst.b	(CurPath)		; is this a final trailing /?
	 bne.s	ff_not_final
	move.l	CurPath,d0		; compare against last / position
	sub.l	LastPath,d0
	 beq.s	ff_not_final		; on foo:/, curpath==lastpath
	subq.l	#1,d0			; if CurPath == LastPath+1
	 beq.s	ff_not_final		; leave slashes

ff_kill_slash:
	clr.b	-(CurPath)		; remove single trailing '/'
	bra.s	ff_endloop

ff_not_final:
	move.l	CurPath,LastPath	* Set the "Last path" for next wild
	tst.b	Wild			* sets LastPath to char after '/'
	 beq.s	ff_loop			; if we hit a wildcard, basepath to
					; lastpath gets added as a node

	; hit a slash at end of wildcarded directory spec
	move.l	CurPath,D0
	sub.l	BasePath,D0
	subq.l	#1,D0			; Wild is already set
	bsr.s	AddAnchor
	 bne.s	ff_error		* error locking it
	move.l	LastPath,BasePath	* Set Start to current
	moveq	#0,Wild			* Wild no more
	bra.s	ff_loop

*
* end of path, make final node
*
ff_endloop:
	;-- Make sure that if there were no wildcards that the initial
	;-- node is the PathPart of string and the final node is the FilePart.
	;-- This means that it won't leave nodes like "df0:foo" as a single
	;-- entry, which causes it to relock the whole string on a DODIR (and
	;-- may get the wrong volume).  It also means that ap_Current->
	;-- an_Lock will be on the parent, and ap_Current->an_Info.fib_FileName
	;-- will be the name of the file in that directory.
	;--
	;-- Actually, the only way to make certain ap_Current->an_Lock is
	;-- right for cases like df0:foo#?/bar/foobar (currently an_Lock
	;-- would be df0:foo, with fib_Filename of foobar) is to make sure
	;-- ANY last file or directory name is separate, regardless of whether
	;-- there were wildcards anywhere or not.  This actually simplifies
	;-- the code slightly.
	;--
	;-- LastPath is set up pointing at last / or initial :.

	move.l	LastPath,d0		; last / or :
	sub.l	BasePath,d0		; d0 is a parameter to AddAnchor
	 beq.s	ff_no_dir_to_add	; actually it was a single filename
					; or filename following wildcard dir.
	bsr.s	AddAnchor		; make a node of it (up to last : or /)
	 bne.s	ff_error		; couldn't lock it or some such
	move.l	LastPath,BasePath	; reset accumulation point

ff_no_dir_to_add:
	;-- add final node for the last portion of the path (past last : or /)
	move.l	CurPath,D0		; Last position examined (== \0)
	sub.l	BasePath,D0		; Either original, or after last
*	 beq.s	ff_all_added		; pattern ended with / - ignore it.
	bsr.s	AddAnchor		; addanchor (due to wildcards in dirs)
	 bne.s	ff_error		* can't lock it!

ff_all_added:
	move.l	ap_First(AnchorBase),ap_Current(AnchorBase)

	move.l	d4,a1			; free temporary buffer
	move.l	AbsExecBase,a6
	jsr	_LVOFreeVec(a6)
	
	;-- call MatchNext.  Force dir changed bit on FindFirst (it did)
	move.l  AnchorBase,D1
	bsr	FindNext		; call FindNext (d1)
	bset.b	#APB_DirChanged,ap_Flags(AnchorBase)
	bra.s	ff_exit

	;-- come here on error from AddAnchor.  Error code is in IoErr
	;-- already (normally OBJECT NOT FOUND, NO FREE STORE, etc).
	;-- equivalent to code in FindFirst, except we can get the right
	;-- error code.
ff_error:
	;-- CAREFUL! (No FIX needed) - a6 == Execbase!
	;-- CAREFUL! make sure we don't tromp it before getting here!
	SYSCALL	IoErr			; get IoErr for return code in D0
	move.l	d0,d2			; save it

	move.l	AnchorBase,D1		; FreeAnchorChain will call UnLock()!
	bsr	FreeAnchorChain		; free up alloced memory for AnchorBase

	move.l	d4,a1
	move.l	AbsExecBase,a6
	jsr	_LVOFreeVec(a6)		; free temporary buffer

	move.l	d2,d1			; get error back
	SYSCALL	SetIoErr		; put it in IoErr() as well...
	move.l	d2,d0			; get it back again for return code
ff_exit:
	movem.l	(SP)+,D2-D5/A2-A6
	rts

***********************************************************************
*
* AddAnchor(length(d0), AnchorBase, BasePath, Wild)
*	Dedicated function works with FindFirst
* 	Uses AnchorBase, BasePath, Wild, register definitions...
*	String starts at BasePath, length in d0
*	Length of string is in D0 on entry
*
*	Note: depends on "foo/" and "foo" being the same directory/file
*
*	NOTE: Wild must be 0 or DDF_PatternBit!
*
*	Added secondary return to make error checking of AddAnchor calls
*	easier.  D1 = 0 is ok, D1 != 0 is failure (error in IoErr)
*	Returns cc's set from a test of D1 for easy error checking.
*
***********************************************************************
AddAnchor:
	movem.l	A2/D2/D3,-(sp)
	move.l	D0,D2			; Save "length"

	add.l	#an_SIZEOF+1,D0
	bsr	DosAllocMem		; cleared
	move.l	D0,A2
	tst.l	D0
	 beq.s	aa_nomem

* Copy over string, in Upper Case thanks.
	move.l	BasePath,A0
	lea.l	an_String(A2),A1

1$:	move.b	(A0)+,D0
	 beq.s	4$			; string is cleared, no term needed
	move.b	Wild,an_Flags(A2)	; Set the wildcard flag
	 beq.s	5$			; don't copy/UC if not wild!!!!
	bsr	Toupper
5$:	move.b	D0,(A1)+
	subq	#1,D2
	 bgt.s	1$
4$:

* Insert the node in the list
	tst.l	ap_Base(AnchorBase)	; is this the first?
	 beq.s	2$

	;-- not the first node, merely add to list
	move.l	ap_Current(AnchorBase),A0
	move.l	A2,an_Child(A0)
	move.l	A0,an_Parent(A2)
	bra.s	98$			; set ap_Current, return

* First entry, it is a bit different
* There is no ap_Base, we need to build one for the current directory
2$:
	move.l	A2,A0			; Points to "" (current dir)
	tst.b	Wild			; was there a wildcard?
	 bne.s	10$

	; first, is this the first part of a wildcard list or is it the
	; final AddAnchor for the entire string.
	; no wildcard - We need to lock the pathpart of the string, and
	; use that for an_lock.  Eventually it will be examined, and it
	; will copy the name part of the string over the fib_FileName.
	; 'BasePath' has the pointer to the beginning of the string.
	; 'LastPath' points character past last / or past :.
	; an_String(a2) has the entire path.
	lea	an_String(a2),a0	; lock this instead of ""
	move.l	a0,d1
	SYSCALL	PathPart		; returns to end of path string (may be
					; past :, at last slash, or after last
					; slash if going up dirs
	move.l	d0,a0
	move.b	(a0),d3			; save '/' or char after ':'
					; (note d3 (Wild) is free here)
	clr.b	(a0)			; terminate string (temp)
	move.l	a0,d2			; save ptr

	; set bit to make matchnext return no_more_entries
	bset.b	#DDB_SINGLE,an_Flags(A2)

	lea	an_String(a2),a0	; ptr to beginning of path (again)
	bsr	LockThisForMePlease
	move.l	d2,a0			; string we locked
	move.b	d3,(a0)			; restore byte in string
	bra.s	20$			; check return from lock call
10$:
	bsr	LockThisForMePlease	; on ""
20$:
	move.l	A2,ap_First(AnchorBase) ; MUST be set before checking error!
	move.l	D0,an_Lock(A2)		;  so it can be freed by FreeAnchorChain
	 beq.s	aa_nomem		; Any error from Lock or an allocation
					; error earlier.

98$:	move.l	A2,ap_Current(AnchorBase)
	moveq	#0,d1			; secondary return - success (ugly)
aa_return:
	move.l	A2,D0
	movem.l	(sp)+,D2/D3/A2
	tst.l	d1			; sets cc's for error
	rts

aa_nomem:
	;-- may also be some other sort of error - code is in IoErr().
	bset.b	#APB_NOMEMERR,ap_Flags(AnchorBase)
	moveq	#1,d1			; secondary return - fail (ugly, sorry)
	bra.s	aa_return


* Shared examine code...
* returns 1 if it's been examined before, or if it's a dir, or if it doesn't
* have any children nodes.  ?????  Return 0 if examine fails, or if
* it has children and isn't a directory.  IMPORTANT: MatchNext relies on
* this (and IoErr being 0 in that case) in order to avoid going into
* files as if they were directories.

DoExamine:
	btst.b	#DDB_ExaminedBit,an_Flags(Node)
	 bne.s	10$
	moveq.l	#an_Info,D2
	add.l	Node,D2
	SYSCALL	Examine
	tst.l	D0
	 beq.s	1$
	bset.b	#DDB_ExaminedBit,an_Flags(Node)
	tst.l	an_Child(Node)
	 beq.s	10$
	tst.l	an_Info+fib_DirEntryType(Node)
	 bge.s	10$

	;-- either an error, OR (it's not a leaf AND it's a file)
	;-- FIX! should we force IoErr to 0?
1$:	moveq	#0,D0
	rts
10$:	moveq	#1,D0
	rts


**************************************
*
* MatchNext main entry point - also called from MatchFirst
*
FindNext:
	movem.l	D2/D3/A2/A3/A5,-(sp)
	move.l	D1,AnchorBase
	move.l	ap_Current(AnchorBase),Node

	;-- reset dir changed bit
	bclr.b	#APB_DirChanged,ap_Flags(AnchorBase)

	moveq	#ERROR_NO_FREE_STORE,D0
	btst.b	#APB_NOMEMERR,ap_Flags(AnchorBase)
	 bne	fn_error		; A memory error, prollably in FF

	tst.l	ap_Base(AnchorBase)
	 beq	fn_done			; Anchor has been freed. Scoot.

	bclr.b	#APB_DODIR,ap_Flags(AnchorBase)
	 beq.s	fn_loop

	;-- don't check for hardlinks if we're following them
	btst.b	#APB_FollowHLinks,ap_Flags(AnchorBase)
	 bne.s	fn_enter_dir_all

	;-- normally don't enter a hardlinked dir on ALL.
	moveq	#ST_LINKDIR,d0
	cmp.l	fib_DirEntryType+ap_Info(AnchorBase),d0
	 beq.s	All_done_with_dir	; stay where we are, return DIDDIR

fn_enter_dir_all:
	lea	WildZ(pc),BasePath		; P_ANY
	moveq	#1,D0				; Length
	moveq	#DDF_PatternBit,Wild
	bsr	AddAnchor		; wild, not base: can't fail a lock
	 beq.s	1$			; however, it can fail an alloc
					; returns cc's set for error
	SYSCALL	IoErr			; get error code (ERROR_NO_FREE_STORE)
	bra	fn_error		; return secondary result

1$:	move.l	D0,BasePath			; BasePath is now a temp...
	bset.b	#DDB_AllBit,an_Flags(BasePath)
	bra	fn_newnode

***
* FindNext Loop Point
***
fn_loop:
	;-- SINGLE means no wildcards in this node.
	;-- SINGLE is either the only node, or the top-level node.
	btst.b	#DDB_SINGLE,an_Flags(Node)
	 bne.s	fn_enter		; Initial node is different...
	move.l	an_Lock(Node),D1
	 beq.s	fn_enter			; Skip null initial nodish
	bsr	DoExamine		; Examine this node, if needed.
	 bne.s	fn_enter		; node examined, process/continue

* UpLevel is dedicated to FindNext.  It goes up a level, until
*	the TOP is reached, or until a pattern is found
UpLevel:
	move.l	an_Lock(Node),D1
	SYSCALL	UnLock
	clr.l	an_Lock(Node)
***
	move.l	Node,A1
	move.l	an_Parent(Node),Node
	move.l	Node,ap_Current(AnchorBase)
	 beq.s	fn_done			; Jump to safety, OK for below.

	;-- set flag to indicate dir change
	bset.b	#APB_DirChanged,ap_Flags(AnchorBase)

	btst.b	#DDB_AllBit,an_Flags(A1)
	 beq.s	fn_NotAll
	bsr	FreeDAList
	clr.l	an_Child(Node)

All_done_with_dir:
	;-- done with an DODIR directory

	bset.b	#APB_DIDDIR,ap_Flags(AnchorBase)
	bra	fn_havematch
	
fn_NotAll:
***
	;-- if this level has no pattern, keep going up...
	btst.b	#DDB_PatternBit,an_Flags(Node)
	 beq.s	UpLevel
	;-- fall though - will go to fn_wildloop

* Check if we need to match a pattern in this node
* on main path of loop
fn_enter:
	btst.b	#DDB_PatternBit,an_Flags(Node)
	 bne.s	fn_wildloop
	;-- Fib entry no longer valid
	bclr.b	#DDB_ExaminedBit,an_Flags(Node)
	btst.b	#DDB_Completed,an_Flags(Node)
	 beq.s	fn_continue

	;-- pop up and check there, until I pop out or find something
	bclr.b	#DDB_Completed,an_Flags(Node)
	bra.s	UpLevel

* Out of entries return
fn_done:
	move	#ERROR_NO_MORE_ENTRIES,D0
	bra	fn_error

***
* Search for the next match for the pattern at this level.
****
fn_wildloop:
	move.l	an_Lock(Node),D1
	moveq.l	#an_Info,D2
	add.l	Node,D2
	SYSCALL	ExNext
	tst.l	D0
	 beq.s	UpLevel

***
	moveq.l	#0,D0
	move.l	ap_BreakBits(AnchorBase),D1
	LINKEXE	SetSignal
	and.l	ap_BreakBits(AnchorBase),D0
	 bne	fn_broke
***
	;-- no need to copy to upper case anymore...

	lea	an_String(Node),A0
	move.l	A0,D1
	lea	an_Info+fib_FileName(Node),A0
	move.l	a0,D2
**	SYSCALL	MatchPattern		; Keep this EXTERNAL REFERENCE
	bsr	_MatchPatternNoCase
	tst.l	D0
	 beq.s	fn_wildloop		; No match, loop some more...

	;-- if this is the end of the chain, we have a match, return.
	tst.l	an_Child(Node)
	 beq	fn_havematch		; Finished()

	;-- if it's a file, we can't enter it - loop
	tst.l	an_Info+fib_DirEntryType(Node)
	 blt.s	fn_wildloop		; files are < 0!

* Continue processing a newly entered node, which has a non-wildcard
* string that needs to be locked/examined, and then perhaps further
* directory traversal

fn_newnode:
	bset.b	#DDB_ExaminedBit,an_Flags(Node)
fn_continue:
	bsr.s	fn_getstring		; Get this node's string
	bsr	DoRelLock
	move.l	D0,LockReg
	 beq.s	40$			; No Lock. Fluff.
					; something's wrong here, this goes
					; to uplevel...(?)
	; it goes to uplevel to handle #?/foo, etc.

	move.l	LockReg,D1
	bsr	DoExamine
	 bne.s	fn_checkmatch		; successfully examined it

	;-- Examine failed? (or it's not a leaf and the node is a file)
	move.l	LockReg,D1
	SYSCALL	UnLock			; DAMN CHARLIE!  Unlock preserves
					; ioerr value (from examine).
40$:	;-- lock failure comes here
	SYSCALL	IoErr
	tst.l	d0			; was this a non-leaf-node file?
	 beq.s	fn_ul			; no error - must be
	move.l	#ERROR_OBJECT_NOT_FOUND,D1
	cmp.l	D0,D1			; was it object not found?
	 bne	fn_error		; no, some other error.  Stop searching

	;-- object not found, continue on (d0.l is OBJECT_NOT_FOUND!)
	;-- check if this is a non-wildcard pattern.
	;-- If it is return OBJECT_NOT_FOUND.
	btst.b	#APB_ITSWILD,ap_Flags(AnchorBase)
	 beq	fn_error		; no wildcards and failed a lock
	; has wildcards somewhere

	;-- Uplevel will return no more entries if it pops out
fn_ul:	bra	UpLevel			; Hope this works ...

* Here, a valid Examined node for a non-wildcarded string.
* If it is the last node, return it after freeing the lock on the item.
* If this is a non-wildcard node, make sure the name is the same we used to
* lockthe entry, NOT the value returned by Examine()!  This is to make this
* work better with links (especially soft links), and with foo/bar/// type
* stuff.
fn_checkmatch:
	;-- we don't need to check if it's a file, as DoExamine returns an
	;-- error if it's a file AND it's not a leaf node!
	btst.b	#DDB_PatternBit,an_Flags(Node)
	 bne.s	fn_no_copy

	;-- a node that has no wildcards - return same name used to lock
	;-- it!
	lea	an_String(Node),a0	; start of path
	move.l	a0,d1
	SYSCALL	FilePart		; returns d0 = ptr to start of fname
	move.l	d0,a0			; pointer to name
	lea	an_Info+fib_FileName(Node),a1
1$	move.b	(a0)+,(a1)+		; copy an_string over examined string!
	bne.s	1$

fn_no_copy:
	;-- is this the end of the chain (do we have a complete match?)
	move.l	an_Child(Node),D0
	 beq.s	fn_match1

	;-- no, need to match child node
	move.l	D0,Node
	move.l	LockReg,an_Lock(Node)
	move.l	Node,ap_Current(AnchorBase)
	bclr.b	#DDB_ExaminedBit,an_Flags(Node)

	;-- set flag to indicate dir change
	bset.b	#APB_DirChanged,ap_Flags(AnchorBase)

	bra	fn_loop

***
* Subroutine to get a string to form a path to the object.  Get an_String
* if not wild, fib_FileName if we found it by ExNext.
*
fn_getstring:
	lea	an_String(Node),A0
	btst.b	#DDB_PatternBit,an_Flags(Node)
	 beq.s	61$
	lea	an_Info+fib_FileName(Node),A0
61$:	rts

**
* we have a non-wildcard match on the final node.
*
fn_match1:
	;-- we have a match, unlock lock on final item
	bset.b	#DDB_Completed,an_Flags(Node)
	move.l	LockReg,D1
	SYSCALL UnLock

* Great stuff.  We have a match (wildcard or not).  Do Something!!!!!
fn_havematch:
	move.l	ap_Current(AnchorBase),Node

	lea	ap_Info(AnchorBase),A0		; Copy over FIB...
	lea	an_Info(Node),A1
	moveq	#(fib_SIZEOF/4)-1,D0
71$:	move.l	(A1)+,(A0)+
	 dbra	D0,71$

** Now use ap_StrLen instead of LONG ap_Length
	move.w	ap_Strlen(AnchorBase),D2	; Max length
	 beq.s	fn_valid				; Nothing to copy!!!

	move.l	ap_First(AnchorBase),Node
	lea	ap_Buf(AnchorBase),BasePath
	move.l	BasePath,A1			; BasePath is for compare

** Build ap_Buf from the AnchorChains.  Tells us how we got there from
** here.
72$:	bsr.s	fn_getstring			; Get this node's string

73$:	subq.w	#1,D2
	 ble.s	fn_overflow			; Out of room in buf...
	move.b	(A0)+,(a1)+
	 bne.s	73$

	move.l	an_Child(Node),Node		; is the the final node?
	move.l	Node,D0
	 beq.s	fn_valid			; All clear tank yew

	subq.l	#1,A1				; handle "" nodes.
	cmpa.l	BasePath,A1
	 beq.s	72$				; Do Nothing  - still strstart

* See if a / is needed here.
	cmp.b	#':',-1(A1)		; did node end in :?
	 beq.s	72$			; yes, nothing needed
	moveq	#'/',D0
	cmp.b	-1(A1),D0		; did it end in '/'?
	 beq.s	72$
	move.b	D0,(A1)+		;No, put a slash on the end
	subq.w	#1,D2			;and decrement our space in buffer
	bra.s	72$

* CheckBreak comes here, offset stack.
fn_broke:
	move.l	D0,ap_FoundBreak(AnchorBase)
	move	#ERROR_BREAK,D0

*	Fall through
***	bra.s	fn_error

* Here, a valid return.  Check to see if the anchor should be freed.
*fn_valid:
*	moveq	#0,D0
*	moveq	#(1<<APB_ITSWILD)+(1<<APB_DOWILD),D1
*	and.b	ap_Flags(AnchorBase),D1
*	 bne.s	fn_rts

fn_error:
	move.l	D0,D2
	move.l	AnchorBase,D1
	bsr.s	FreeAnchorChain
	move.l	D2,D0
	bra.s	fn_rts

fn_valid:
	moveq	#0,d0
	bra.s	fn_rts

fn_overflow:
	move	#ERROR_BUFFER_OVERFLOW,D0
fn_rts:	ext.l	D0			; MUST BE FIXED!!!!!!!!!!!!!!!!
	move.l	D0,D1
	move.l	D0,D2
	SYSCALL	SetIoErr
	move.l	D2,D0
	movem.l	(sp)+,D2/D3/A2/A3/A5
	rts



***********************************************************************
*
* FreeAnchorChain( D1 )
*
*	Frees all stuff associated with D1.  It's an anchorchain,
* which is a semi-private structure used by FindFirst/FindNext.
*
***********************************************************************
FreeAnchorChain:
	move.l	A2,-(sp)
	move.l	D1,A2
1$:	move.l	ap_First(A2),d0
	 beq.s	10$
	move.l	D0,A0
	move.l	an_Lock(A0),D1
	 beq.s	2$
	SYSCALL	UnLock
2$:	move.l	ap_First(A2),A1
	move.l	an_Child(A1),ap_First(A2)
	LINKEXE FreeVec				; A1
	bra.s	1$

10$:	move.l	(sp)+,A2
	rts

***********************************************************************
* 
* lock = RelLock(lock,name)
*  D0  =	  D1,  A0
*
*	Input is a lock in D1, and a name in A0.
*	Return value in D0 is the lock for name, relative to base dir lock.
*
*	NOTE - Register D2 is clobbered!
*
***********************************************************************
DoRelLock:
	btst.b	#DDB_SINGLE,an_Flags(Node)
	 bne.s	LockThisForMePlease
	move.l	an_Lock(Node),D1
	 beq.s	LockThisForMePlease
RelLock:
 	move.l	A0,-(sp)
	SYSCALL	CurrentDir
	move.l	(sp),D1
	move.l	d0,(sp)
	bsr.s	LTFM
	move.l	(sp),D1
	move.l	d0,(sp)
	SYSCALL	CurrentDir
	move.l	(sp)+,D0
	rts

LockThisForMePlease:
	move.l	A0,D1
LTFM:	move.l	d2,-(sp)		; charlie made this trash d2!!!!
	moveq.l	#ACCESS_READ,D2		; this is a pain so I fixed it
	SYSCALL	Lock
	move.l	(sp)+,d2
	rts

***********************************************************************



******************* U S E R  R O U T I N E S *************************
*
*   NAME
*       DosAllocMem -- AmigaDOS compatible memory allocator
*
*   SYNOPSIS
*       memBlock = DosAllocMem( size_in_bytes )
*           d0                      d0
*
*   FUNCTION
*       This function returns a memory block of the size requested, or
*       NULL if the allocation failed.  The memory will satisfy the
*       requirements of MEMF_PUBLIC | MEMF_CLEAR.
*
*       As expected by AmigaDOS, the total size of the memory block is
*       stored at (memblock - 4), so the actual memory allocated will
*       always be four bytes larger than size_in_bytes.
*
*   INPUTS
*       size_in_bytes - the size of the desired block in bytes.
*
*   RESULT
*       memBlock - a pointer to the allocated free block.  This block
*                  will be longword aligned, and the total size of the
*                  block is stored at (memblock - 4).  If the allocation
*                  failed, memBlock will return zero.
*
*   ADDITIONAL CONSIDERATIONS
*       The value returned by DosAllocMem is a real pointer.  If you
*       need a BPTR, you must convert this value yourself.
*
*   BUGS
*       None known.
*
*   SEE ALSO
*       DosFreeMem()
*
*   AUTHOR: SDB
*   REVISED:
***********************************************************************

DosAllocMem:
	move.l	#MEMF_PUBLIC!MEMF_CLEAR,d1
	LINKEXE	AllocVec			; d0,d1
	rts

* FreeDAList
*	Call with A1 = dlist
FreeDAList:
	move.l	a6,-(sp)
	move.l  AbsExecBase,a6
	bra.s	fdentry

fdloop:	move.l	(A1),-(sp)
	jsr	_LVOFreeVec(a6)			; A1
	move.l	(sp)+,A1
fdentry:
	move.l	A1,D0
	 bne.s	fdloop
	move.l	(sp)+,a6
	rts


