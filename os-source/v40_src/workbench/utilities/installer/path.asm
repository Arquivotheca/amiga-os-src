			INCLUDE "exec/types.i"
			INCLUDE "libraries/dos.i"
			INCLUDE "libraries/dosextens.i"

			section path.asm,code

xlib		macro	
			xref	_LVO\1
			endm

			xlib	Lock
			xlib	Examine
			xlib	UnLock
			xlib	ParentDir

; wildcard file requester -
;	- has volume / device / parent / subdirectory names
;	- shows filenotes (can be edited?)
;	- shows sizes
;	- shows permissions
;	- shows date (aaag)
;	- has room for additional buttons on the bottom - whatever you want
;		them to be (example - LOAD, SAVE, CANCEL, DELETE, LOCK,
;			LOAD TEXT, etc.
;		buttons can be toggles, multiple choice or single-action
;	- has wildcards for selectively showing files.
;

***************************************************************************
*
* length = ExpandPath(lock,buffer,maxlength);
*	derives the entire pathname of the given lock, and places the result
* 	into buffer. returns length of string (also null terminate).
*
***************************************************************************

; register usage
;	d3 = max string length
;	d4 = recursion count (number of directories)
;	d5 = 'no parent' flag
;	d6 = length of string
;	d7 = directory lock
;	a2 = buffer to store path string into
;	a5 = file info block
;	a6 = lib base pointer

			xdef	_ExpandPath,ExpandPath
			xref	_DOSBase

ExpandPath:				; (a0:Lock, a1:buffer, d0:length)
			movem.l	D2-D7/A2/A3/A5/A6,-(sp) ; save regs
			move.l	a0,d7
			move.l	a1,a2
			move.l	d0,d6
			bra.s	do_dir_text
_ExpandPath:
			movem.l	D2-D7/A2/A3/A5/A6,-(sp) ; save regs
			move.l	4+40(sp),D7		; put dir lock in d0
			move.l	8+40(sp),A2		; put buffer to use in a2
			move.l	12+40(sp),D6	; put length of string in D6
do_dir_text:
			subq.w	#1,d6			; subtract 1 from d6
			bmi.s	dir_error		; if no length, then error
			move.l	sp,A3			; save stack pointer
			move.l	_DOSBase,A6		; get dosbase
			sub		#fib_SIZEOF,sp ; Reserve space on stack for fib
			move.l	sp,D0			; get stack pointer in d0
			btst	#1,D0			; if on a non-long-word-aligned boundary
			beq.s	1$
			subq	#2,sp			; then align to long word
1$			move.l	sp,A5			; Set A5 pointing to fib

			moveq	#0,D4			; Set artificial limit of 20
									; directories to avoid endless loop
			move.l	d4,D3			; maximum string length = 80, start at 0

			clr.b	(A2)			; Set string to "null"
			bsr.s	ascend			; generate path
			clr.b	(A2)			; null terminate.

			move.l	A3,sp			; restore stack pointer
			move.l	D3,d0			; length of string.
			movem.l	(sp)+,D2-D7/A2/A3/A5/A6 ; restore regs
			rts						; return

dir_error	move.l	A3,sp			; restore stack pointer
			moveq	#0,d0			; length = 0, error.
			movem.l	(sp)+,D2-D7/A2/A3/A5/A6 ; restore regs
			rts

***************************************************************************
*
* ascend
*		Recursive routine to create path text.
*		Call with D7 = a valid child lock
*
ascend:
			cmp		#20,D4
			bge.s	dir_error			; Probable disk error looped

			move.l	D7,D1
			jsr		_LVOParentDir(A6)
			move.l	D0,D5
			beq.s	no_parent			; D5 is a flag "no parent" if 0

			addq	#1,D4				; Bump tally of parents
			move.l	D7,-(sp)			; Save the current directory
			move.l	D5,D7				; Now go fetch this one
			move.l	D5,-(sp)
			bsr.s	ascend				; Go up another level

			subq	#1,D4				; Reduce parents level

			move.l	(sp)+,D1			; Restore parent dir into D1
			move.l	(sp)+,D7			; Restore current direcory
			jsr		_LVOUnLock(A6)		; Free the extra ParentDir Lock.

			moveq	#1,D5

* no_parent
*		Here get the text for this directory and append it.
; DAVE - check for ram disk and other oddities.
no_parent:
			move.l	A5,D2				; d2 = address of file info block
			move.l	D7,D1				; d1 = lock
			jsr	_LVOExamine(A6)			; examine it.
			tst.l	D0					; if error
			beq.s	dir_error			; Bad error, couldn't Examine()

			tst.w	d4
			beq.s	npl0
			tst.l	fib_DirEntryType(A5) ; type of entry...
			ble.s	dir_error			; Parent of a dir not a dir. Hmm...

npl0		lea		fib_FileName(A5),A0	; Append this dir string 
npl1:		move.b	(A0)+,D0			; while *str++
			beq.s	npl2
			bsr.s	do_putch			; put to string.
			bra.s	npl1

npl2:		move.b	#':',D0				; if dir level = 0
			tst	D5
			beq.s	do_putch			; if ZERO, it's the base

			move.b	#'/',D0
			tst	D4
			bne.s	do_putch			; if NZ, it's got a parent

npl3:		rts


***************************************************************************
*
* do_putch
*		Append the character in D0 to the string at A2
*		Max length = <d3> characters.
*		
do_putch:	cmp.w	D6,D3
			bge.s	1$
			addq.w	#1,D3
			move.b	D0,(A2)+
1$			rts

			END
