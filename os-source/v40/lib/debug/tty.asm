*
*
*
    SECTION ktty
    INCLUDE 'assembly.i'

    INCLUDE 'exec/types.i'
    INCLUDE 'exec/strings.i'

    INCLUDE 'hardware/intbits.i'


    XREF    _intreq
    XREF    _intreqr
    XREF    _serdat
    XREF    _serdatr

    XREF    _AbsExecBase
    XREF    _LVORawMayGetChar
    XREF    _LVORawPutChar
    XREF    _LVORawDoFmt

    XDEF    KDoFmt


******* debug.lib/KPutChar *****************************************************
*
*   NAME
*	KPutChar - put a character to the console
*		   (defaults to the serial port at 9600 baud)
*
*   SYNOPSIS
*	char = KPutChar(char)
*	D0	        D0
*
*   FUNCTION
*	Put a character to the console.  This function will not return
*	until the character has been completely transmitted.
*
*   INPUTS
*	KPutChar is the assembly interface, the character must be in D0.
*	_KPutChar and _kputc are the C interfaces, the character must be
*	a longword on the stack.
*
*******************************************************************************

   xdef	_kputchar
_kputchar:
   xdef	_kputch
_kputch:
   xdef	_KPutCh
_KPutCh:

   xdef	_kputc
_kputc:
   xdef	_KPutChar		* (char)
_KPutChar:
	move.l	4(sp),D0

    XDEF    KPutChar
KPutChar:
		move.l	a6,-(sp)
		move.l	_AbsExecBase,a6
		jsr	_LVORawPutChar(a6)
		move.l	(sp)+,a6
		rts


******* debug.lib/KPutStr ***********************************************
*
*   NAME
*	KPutStr - put a string to the console
*		   (defaults to the serial port at 9600 baud)
*
*   SYNOPSIS
*	KPutStr(string)
*	        A0
*
*   FUNCTION
*	put a null terminated string to the console.  This function will
*	not return until the string has been completely transmitted.
*
*   INPUTS
*	KPutStr is the assembly interface, a string pointer must be in A0.
*	_KPutStr and _kputs are the C interfaces, the string pointer must
*	be on the stack.
**********************************************************************

   xdef	_kputstr
_kputstr:
   xdef	_KPutS
_KPutS:

   xdef	_kputs
_kputs:
   xdef	_KPutStr			* (string)
_KPutStr:
	move.l	4(sp),A0

    XDEF    KPutStr
KPutStr:
ps0:	    MOVE.B  (A0)+,D0	    ; next character
	    BEQ.S   ps1		    ; done with string
	    BSR.S   KPutChar
	    BRA.S   ps0
ps1:
	    RTS



******* debug.lib/KGetChar **********************************************
*
*   NAME
*	KGetChar - get a character from the console
*		   (defaults to the serial port at 9600 baud)
*
*   SYNOPSIS
*	char = KGetChar()
*	D0
*
*   FUNCTION
*	busy wait until a character arrives from the console.
*	KGetChar is the assembly interface, _KGetChar and _kgetc
*	are the C interfaces.
**********************************************************************

   xdef	_kgetchar
_kgetchar:
   xdef	_kgetch
_kgetch:
   xdef	_KGetCh
_KGetCh:

   xdef	_kgetc
_kgetc:
   xdef	_KGetChar
_KGetChar:

    XDEF    KGetChar
KGetChar:
	    BSR.S    KMayGetChar
	    TST.L    D0
	    BMI.S   KGetChar
	    RTS


******* debug.lib/KMayGetChar *******************************************
*
*   NAME
*	KMayGetChar - return a character if present, but don't wait
*		      (defaults to the serial port at 9600 baud)
*
*   SYNOPSIS
*	flagChar = KMayGetChar()
*	D0
*
*   FUNCTION
*	return either a -1, saying that there is no character present, or
*	whatever character was waiting.  KMayGetChar is the assembly
*	interface,  _KMayGetChar is the C interface.
**********************************************************************
   xdef	_KMayGetCh
_KMayGetCh:


   xdef	_KMayGetChar
_KMayGetChar:
    XDEF    KMayGetChar
KMayGetChar:
		move.l	a6,-(sp)
		move.l	_AbsExecBase,a6
		jsr	_LVORawMayGetChar(a6)
		move.l	(sp)+,a6
		rts


******* debug.lib/KPrintF ********************************************
*
*   NAME
*	KPrintF - print formatted data to the console
*	          (defaults to the serial port at 9600 baud)
*
*   SYNOPSIS
*	KPrintF("format string",values)
*	         A0             A1
*
*   FUNCTION
*	print a formatted C-type string to the console.  See the
*	exec RawDoFmt() call for the supported % formatting commands.
*
*   INPUTS
*	"format string" - A C style string with % commands to indicate
*	                  where paramters are to be inserted.
*	values - A pointer to an array of paramters, to be inserted into
*	         specified places in the string.
*
*	KPutFmt and KPrintF are identical assembly interfaces that want the
*	two pointers in registers.  _KPrintF and _kprintf are C interfaces
*	that expect the format string pointer on the stack, and the
*	paramters on the stack above that.
*
*   SEE ALSO
*	exec.library/RawDoFmt, any C compiler's "printf" call.
**********************************************************************


   xdef	_KPutFmt			* (format,valueArray)
_KPutFmt:
		move.l	4(sp),A0
		move.l	8(sp),A1
		bra.s   KPutFmt

   xdef	_kprintf
_kprintf:
   xdef	_KPrintF
_KPrintF:
		move.l	4(sp),a0            * format string 
		lea	8(sp),a1            * values to print

    XDEF    KPrintF		;This was missing -Bryce
KPrintF:
    XDEF    KPutFmt
KPutFmt:
	    	MOVEM.L	A2,-(SP)
	    	LEA	KPutChar,A2
	   	BSR.S	KDoFmt
	    	MOVEM.L	(SP)+,A2
	    	RTS


******o debug.lib/KDoFmt ************************************************
*
*   NAME
*	KDoFmt -- format data into a character stream.
*
*   SYNOPSIS
*	KDoFmt(FormatString, DataStream, PutChProc, PutChData);
*              A0            A1          A2         A3
*
*   FUNCTION
*	perform "C"-language-like formatting of a data stream,
*	outputting the result a character at a time
*
*   INPUTS
*	FormatString - a "C"-language-like null terminated format string
*	DataStream - a stream of data that is interpreted according to
*		the format string.
*	PutChProc - the procedure to call with each character to be
*		output, called as:
*	    PutChProc(Char,  PutChData);
*		      D0-0:8 A3
*		the procedure is called with a null Char at the end of
*		the format string.
*	PutChData - an address register that passes thru to PutChProc.
**********************************************************************

KDoFmt:
		move.l	a6,-(sp)
		move.l	_AbsExecBase,a6
		jsr	_LVORawDoFmt(a6)
		move.l	(sp)+,a6
		rts

   xdef	_KDoFmt			* ()
_KDoFmt:
		movem.l	A2/A3,-(sp)
		movem.l	12(sp),A0/A1/A2/A3
		bsr.s	KDoFmt
		movem.l	(SP)+,A2/A3
		rts

	END

