		SECTION	_printf,CODE
		XREF	_AbsExecBase,_LVORawIOInit,_LVORawPutChar
;===========================================================================
; Must be called before the printf function is called for the first time.
;===========================================================================
		XDEF	_DebugInit
_DebugInit	movem.l	d0-d1/a0-a1/a6,-(sp)
		movea.l	_AbsExecBase,a6		need exec library
		jsr	_LVORawIOInit(a6)	initialise serial port
		movem.l	(sp)+,d0-d1/a0-a1/a6
		rts				that's all folks!

;===========================================================================
; Should be called when printf function finished with. (does nothing here).
;===========================================================================
		XDEF	_DebugKill
_DebugKill	rts

;===========================================================================
; The actual code of the printf function.  Generates a formatted string with
; a maximum length of 512 bytes and then calls the appropriate text output
; function.  Text is output as a whole string to speed up the DOS write
; function which goes real slow on single character output.  (This is only
; critical if you are using the cprintf.obj file).  Arguments are passed on
; the stack as: format,arg1,arg2, ... arg8 and it is up to the caller to
; clean up the stack space used, this is handled by the printf macro.
;===========================================================================
		XDEF	_printf
_printf		movem.l	d0-d2/a0-a4/a6,-(sp)	save regs
		movea.l	40(sp),a3		a3 points to string
		lea.l	44(sp),a4		a4 points to args
		suba.w	#80,sp			make room for output string
		movea.l	sp,a2			a2 points to output string

; The main loop, copy characters to the output buffer until a % or \ is
; encountered in which case interpret the characters following in the same
; manner as C's printf function and put the results into the output stream
; instead.  A null byte terminates the output.

_printloop	move.b	(a3)+,d0		get a copy of the byte
_possibleend	move.b	d0,(a2)			move to output string
		beq	_printdone		terminating byte
		cmpi.b	#'\',d0			special character byte ?
		beq.s	_specialchar		yes, go see what it is
		cmpi.b	#'%',d0			number or string byte ?
		beq.s	_introchar		yes, go process that
_bumpptr	addq.l	#1,a2			nothing special, bump ptr...
		bra.s	_printloop		...and go back for the next

; Found a backslash, replace character after with tab or newline (\t or \n)

_specialchar	move.b	(a3)+,d0		get the next character
		beq.s	_possibleend		that could have been nasty!!
		cmpi.b	#'\',d0			did caller want a backslash
		beq.s	_bumpptr		yes, so leave the other
		cmpi.b	#'t',d0			did user want a tab ?
		bne.s	_checkcr		nope, maybe a newline
		move.b	#9,(a2)+		yes, store tab and bump
		bra.s	_printloop		and go for the next char
_checkcr	cmpi.b	#'n',d0			did user want a newline ?
		bne.s	_printloop		nope, ignore all others
		move.b	#13,(a2)+		yes, store cr and bump
		move.b	#10,(a2)+
		bra.s	_printloop		and go for the next char

; Found a percent sign.  Must be introducing a number, string or character

_introchar	move.b	(a3)+,d0		get the next character
		beq.s	_possibleend		another possible oops!
		cmpi.b	#'%',d0			did caller want a percent?
		beq.s	_bumpptr		yes, so leave the last one
		cmpi.b	#'c',d0			print a character ?
		bne.s	_checkstring		nope!
		move.l	(a4)+,d0		get char from arg stack
		move.b	d0,(a2)+		store it and bump ptr
		bra.s	_printloop		and go for the next char
_checkstring	cmpi.b	#'s',d0			print a string ?
		bne.s	_checknumber		nope!
		movea.l	(a4)+,a0		get ptr from arg stack
10$		move.b	(a0)+,(a2)+		and copy the string
		bne.s	10$
		subq.l	#1,a2			went 1 byte too far
		bra.s	_printloop		go for the next char

; wasn't a string or character code so it has to be a number code here

_checknumber	moveq.l	#0,d2			flag, short (word) number
		cmpi.b	#'l',d0			long version of a number ?
		bne.s	_whatnumber		no, it was a short
		moveq.l	#1,d2			flag, longword number
		move.b	(a3)+,d0		and get the next letter
		beq.s	_possibleend		yet another nasty spot!
_whatnumber	cmpi.b	#'x',d0			hex character ?
		bne.s	_decnumber		nope, must be a decimal
		move.l	(a4)+,d0		get hex value to print
		movea.l	a2,a0			buffer to store to
		tst.w	d2			see what size
		bne.s	_dolonghex		it's a longword
		bsr	bin2hex16		it's a word
		bra.s	_findend		find where number ends
_dolonghex	bsr	bin2hex32
		bra.s	_findend		find where number ends

_decnumber	cmpi.b	#'d',d0			really want a decimal ?
		bne	_printloop		nope, illegal character!
		move.l	(a4)+,d0		get decimal value to print
		movea.l	a2,a0			and buffer to store to
		tst.w	d2			see what size
		bne.s	_dolongdec		it's a longword
		bsr	bin2asc16		it's a word
		bra.s	_findend		see what size
_dolongdec	bsr	bin2asc32

; A number has been stored into the buffer, find the end to update A2 ptr

_findend	tst.b	(a2)+			search for null byte
		bne.s	_findend		not there yet
		subq.l	#1,a2			went one byte too far
		bra	_printloop		go for the next char

; We got to the end of the format string and now have a formatted string on
; the stack that is terminated by a null byte.  Output the string to the
; serial port and then fix up the stack for a clean return.

_printdone	movea.l	_AbsExecBase,a6		need exec library node
		movea.l	sp,a2			point to the string
10$		move.b	(a2)+,d0		get byte to print
		beq.s	_cleanup		null terminates output
		jsr	_LVORawPutChar(a6)	and send it out the port
		bra.s	10$			go back for next byte

_cleanup	adda.w	#80,sp			fix up the stack pointer
		movem.l	(sp)+,d0-d2/a0-a4/a6	restore regs
		rts				all done

		INCLUDE	"numbers.asm"
		END

