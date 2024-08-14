*
*	support2.a
*

        INCLUDE "exec/types.i"
        INCLUDE "exec/nodes.i"
        INCLUDE "exec/lists.i"
        INCLUDE "exec/ports.i"
        INCLUDE "exec/libraries.i"
        INCLUDE "exec/tasks.i"
        INCLUDE "exec/memory.i"
        INCLUDE "exec/execbase.i"
        INCLUDE "dos/dos.i"
        INCLUDE "dos/dosextens.i"
        INCLUDE "dos/dos_lib.i"
	INCLUDE "utility/hooks.i"
	INCLUDE "dos/exall.i"

FUNCDEF	MACRO
	XREF _LVO\1
	ENDM

        INCLUDE "exec/exec_lib.i"

SYSBASE	EQU	4

	XREF @CtoBstr
	XREF _NameFromLock
	XREF _findinput

	XREF _LVOGetSysTime

	XDEF @mysplitname
	XDEF _mysplitname
	XDEF @bsplitname
	XDEF @mysprintf
	XDEF _mysprintf
	XDEF @CheckSignal
	XDEF _CheckSignal
	XDEF @do_match
	XDEF @getSysTime
	XDEF @copyMem
	XDEF _myNameFromLock
	XDEF @finddefault

******* dos.library/SplitName ************
*
*   NAME
*	SplitName -- splits out a component of a pathname into a buffer
*
*   SYNOPSIS
*	newpos = SplitName(name, separator, buf, oldpos, size)
*	D0                  D1      D2      D3     D4     D5
*
*	WORD SplitName(UBYTE *, UBYTE, UBYTE *, WORD, LONG)
*
*   FUNCTION
*	This routine splits out the next piece of a name from a given file
*	name.  Each piece is copied into the buffer, truncating at size-1
*	characters.  The new position is then returned so that it may be
*	passed in to the next call to splitname.  If the separator is not
*	found within 'size' characters, then size-1 characters plus a null will
*	be put into the buffer, and the position of the next separator will
*	be returned.
*	If a a separator cannot be found, -1 is returned (but the characters
*	from the old position to the end of the string are copied into the
*	buffer, up to a maximum of size-1 characters).  Both strings are
*	null-terminated.
*
*   INPUTS
*	name      - Filename being parsed.
*	separator - Separator charactor to split by.
*	buf       - Buffer to hold separated name.
*	oldpos    - Current position in the file.
*	size 	  - Size of buf in bytes (including null termination).
*		    0 is illegal.
*
*   RESULT
*	newpos    - New position for next call to splitname.  -1 for last one.
*
;==============================================================================
; offset = SplitName( prefix,char,string,ptr )
;   d0			d1    d2    d3   d4
;
; Basically, we search the given string starting at the offset in d1.  If we
; find the given character, then we return the offset to the character after.
; A maximum of 30 characters get copied to prefix after we've searched string.
; If the given character is not found then 0 is returned but a maximum of 30
; characters will still have been copied across to prefix.
; NOTE: offsets are 1-based, not 0!!!
;
; Converted from BSTRs to CSTRs. REJ 
;==============================================================================
@bsplitname:	; d1 = prefix (CPTR)
		; d2 = char
		; d3 = string (CSTR)
		; d4 = ptr

		move.l	d1,-(sp)		save for later
		exg	d1,d3			put in right regs
		subq.l	#1,d4			make 0-based
		moveq	#30,d5			BCPL is always 30 max
		bsr.s	@mysplitname
		move.l	(sp)+,a0		input to CtoBstr
		move.l	d0,-(sp)		save return value
		bsr	@CtoBstr		convert prefix to BSTR in place
		move.l	(sp)+,d0		restore
						; WARNING! TRICKY!
		addq.l	#1,d0			make 1-based (and -1 becomes 0)
						; BCPL wants 0 on end of str!
		rts

@mysplitname:
_mysplitname:
		movem.l	d3/d5,-(A7)
		move.l	d3,a0			result buffer
		move.l	d1,a1			name
		move.l	d4,d1			position (0-based)
		moveq	#0,d3			d3 is the length copied
*						(1 for null!)
		clr.b	(a0)			no string copied yet

		;-- d1 is 0-based!!!
		add.w	d1,a1			where to start searching
		move.w	d1,-(A7)		save current pointer
		subq.l	#1,d5			we need space for null

10$:		cmp.w	d5,d3			can we copy it ?
		bge.s	too_many		no, string is already max chars
		move.b	(a1)+,d1		get the character
		move.b	d1,(a0)+		store in prefix (buf)
		beq.s	end_string		C strings terminate in null
		addq.w	#1,d3			bump the counter
		cmp.b	d1,d2			is this our character ?
		beq.s	done			yep, string is copied
		bra.s	10$

inc_count:	addq.w	#1,d3

		; ok, we're past size characters.  look for terminator.
too_many:	clr.b	(a0)			terminate string
		move.b	(a1)+,d1
		beq.s	end_string
		cmp.b	d1,d2			separator?
		bne.s	inc_count		no
		bra.s	done_noclear		yes! (don't clear -1(a0)!

end_string:	moveq	#-1,d0			ran off end of string
		addq.l	#2,A7			scrap value on stack
		bra.s	exit			and all done

		;-- we have a match - write over character
done:		clr.b	-1(a0)			Make sure Cstr is terminated
done_noclear:	add.w	(A7)+,d3		return new pointer
		move.w	d3,d0			return result in d0
		ext.l	d0			extend to long
exit:		movem.l	(A7)+,d3/d5
		rts

*
* mysprintf - uses RawDoFmt.  Can't be in C due to having to change it's
*	      input parameter A3.
*/* VARGARGS */
*
*void __stdargs
*mysprintf (UBYTE *str, UBYTE *fmt, LONG arg1,...)
*{
*	RawDoFmt(fmt,(APTR) &arg1,(void (*)()) myputstr,(APTR) str);
*}
*
*/* called by rawdofmt */
*
*void ASM
*myputstr (REG(d0) LONG ch, REG(a3) char *str)
*{
*	*str++ = ch;	/* doesn't work.  Need to modify A3! */
*}
*

SysBase	EQU	4

@mysprintf:
_mysprintf:
		; 4 bytes for return addr on stack
		movem.l	a2/a3/a6,-(a7)	; adds 12 bytes to offsets
		move.l	20(a7),a0	; fmt
		lea	24(a7),a1	; &arg1
		lea	myputstr(pc),a2
		move.l	16(a7),a3	; str
		move.l	SysBase,a6
		jsr	_LVORawDoFmt(a6)
		movem.l	(a7)+,a2/a3/a6
		rts

myputstr:
		move.b	d0,(a3)+
		rts


******* dos.library/CheckSignal ************
*
*   NAME
*	CheckSignal -- Checks for break signals
*
*   SYNOPSIS
*	signals = CheckSignal(mask)
*	D0		      D1
*
*   FUNCTION
*	This function checks to see if any signals specified in the mask have
*	been set and if so, returns them.  Otherwise it returns FALSE.
*	All signals specified in mask will be cleared.
*
*   INPUTS
*	mask    - Signals to check for.
*
*   RESULT
*	signals - Signals specified in mask that were set.
*
@CheckSignal:
_CheckSignal:
		movem.l	d2/a6,-(a7)
		move.l	d1,d2		; save
		moveq	#0,d0		; clear the affected signals
		move.l	SysBase,a6
		jsr	_LVOSetSignal(a6)
		and.l	d2,d0		; only return signals in mask
		movem.l (a7)+,d2/a6
		rts

*
* Assembler routine for exall to call hook function
*
@do_match:
	movem.l	d0/a2/a3,-(a7)	; tricky!
	move.l	a7,a2		; points to data (from d0)
	move.l	eac_MatchFunc(a0),a0	; hook structure
	move.l	h_Entry(a0),a3
	jsr	(a3)		; a0=ptr to hook, a1=pointer to param,
				; a2=ptr to object
	movem.l	(a7)+,d1/a2/a3	; don't hit d0!
	rts
*
* Asm stub to call timer.device/GetSysTime.  a0 = timeval structure,
* a1 = io_Device value for timer.device.
*
@getSysTime:
	move.l	a6,-(sp)
	move.l	a1,a6
	jsr	_LVOGetSysTime(a6)
	move.l	(sp)+,a6
	rts

*
* Stub to call CopyMem in exec.  a0 = source, a1 = dest, d0 = len
*
@copyMem:
	move.l	a6,-(sp)
	move.l	SysBase,a6
	jsr	_LVOCopyMem(a6)
	move.l	(a7)+,a6
	rts

*
* downcoding of finddefault from cli_init
* BPTR __regargs finddefault (char *file)
* LONG ASM myNameFromLock (REG(d1) BPTR lock,
*			   REG(d2) char *buffer,
*			   REG(d3) LONG buflen)
*
*
_myNameFromLock:
	lea	_NameFromLock(pc),a1
	bra.s	common_noreq

@finddefault:
	lea	_findinput(pc),a1
	move.l	a0,d1			; findinput wants it in d1

common_noreq:
	movem.l	d7/a2,-(sp)
	move.l	SysBase,a2
	move.l	ThisTask(a2),a2
	lea	pr_WindowPtr(a2),a2
	move.l	(a2),d7			; save old value
	moveq	#-1,d0
	move.l	d0,(a2)			; no requesters
	jsr	(a1)
	move.l	d7,(a2)			; restore
	movem.l	(sp)+,d7/a2
	rts

		END
