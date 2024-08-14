*
*   TinyPrintf.  Tiny no-hassle printf for use from CLI programs.
*
*   Created: Thursday 31-Aug-89 19:29:20 Bryce Nesbitt
*
*
	SECTION assembly,CODE
	BASEREG Blink

	XDEF	_printf

	XREF	_DOSBase
	XREF	_SysBase

_LVORawDoFmt	EQU -522
_LVOWrite	EQU  -48
_LVOOutput	EQU  -60


******* amiga.lib/printf *****************************************************
*
*   NAME
*	printf - print a formatted output line to the standard output.
*
*   SYNOPSIS
*	printf("formatstring",[value],[value],... );
*
*	void printf(char *,...);
*
*   FUNCTION
*	Format the output in accordance with specifications in the format
*	string:
*
*   INPUTS
*	formatstring - a pointer to a null-terminated string describing the
*		       output data, and locations for parameter substitutions.
*	value(s) - numeric variables or addresses of null-terminated strings
*		   to be added to the format information.
*
*	The function printf can handle the following format conversions, in
*	common with the normal C language call to printf:
*
*	 %c  - the next long word in the array is to be formatted
*	       as a character (8-bit) value
*	 %d  - the next long word in the array is to be formatted
*	       as a decimal number
*	 %x  - the next long word in the array is to be formatted
*	       as a hexadecimal number
*	 %s  - the next long word is the starting address of a
*	       null-terminated string of characters
*
*    And "l" (small-L) character must be added between the % and the letter
*    if the value is a long (32 bits) or if the compiler in use forces
*    passed paramters to 32 bits.
*
*    Floating point output is not supported.
*
*    Following the %, you may also specify:
*
*    o	     an optional minus (-) sign that tells the formatter
*	     to left-justify the formatted item within the field
*	     width
*
*    o	     an optional field-width specifier... that is, how
*	     many spaces to allot for the full width of this
*	     item.  If the field width specifier begins with
*	     a zero (0), it means that leading spaces, ahead of
*	     the formatted item (usually a number) are to be
*	     zero-filled instead of blank-filled
*
*    o	     an optional period (.) that separates the width
*	     specifier from a maximum number of characters
*	     specifier
*
*    o	     an optional digit string (for %ls specifications
*	     only) that specifies the maximum number of characters
*	     to print from a string.
*
*    See other books on C language programming for examples of the use
*    of these formatting options (see "printf" in other books).
*
*******************************************************************************/

OUTBUFFER   EQU 1024
ABSEXECBASE EQU 4


_printf:
		movem.l d2/d3/a2/a3/a6,-(sp)

	;------ format the string:
		move.l	6*4(sp),a0          ; string
		lea.l	7*4(sp),a1          ; first arg
		lea.l	stuffChar(pc),a2
		lea.l	-OUTBUFFER(sp),sp   ; local string buffer
		move.l	sp,a3
	       ;move.l	ABSEXECBASE.W,a6
		move.l	_SysBase(A4),a6
		jsr	_LVORawDoFmt(a6)

	;------ find end of formatted string:
		moveq.l #-1,d3
ps_size:	tst.b	(a3)+
		dbeq	d3,ps_size
		not.l	d3
		beq.s	ps_empty

	;------ write the formatted string:
		move.l	_DOSBase(A4),a6
		jsr	_LVOOutput(a6)
		move.l	d0,d1
		beq.s	ps_empty
		move.l	sp,d2		;Get string pointer for output
		;d3 loaded above
		jsr	_LVOWrite(a6)   ;handle,buffer,size

ps_empty:	lea.l	OUTBUFFER(sp),sp
		movem.l (sp)+,d2/d3/a2/a3/a6
		rts


;------ putchar function used by DoFmt: --------------------------------
stuffChar:
		move.b	d0,(a3)+
		rts

		END
