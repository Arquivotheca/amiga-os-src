
    SECTION ktty
    INCLUDE 'assembly.i'

    INCLUDE 'exec/types.i'
    INCLUDE 'exec/strings.i'

    XREF    PRawMayGetChar
    XREF    PRawPutChar
    XREF    PRawDoFmt

    XDEF    DPutChar
    XDEF    DPutStr
    XDEF    DGetChar
    XDEF    DMayGetChar
    XDEF    DPutFmt
    XDEF    DDoFmt


******* ddebug.lib/DPutChar ******************************************
*
*   NAME
*	DPutChar - put a character to the parallel port
*
*   SYNOPSIS
*	char = DPutChar(char)
*	D0	       D0
*
*   FUNCTION
*	put a character to the parallel port.
*
**********************************************************************

   xdef	_dputchar
_dputchar:

   xdef	_dputch
_dputch:

   xdef	_DPutCh
_DPutCh:

   xdef	_dputc
_dputc:

   xdef	_DPutChar		* (char)
_DPutChar:
	move.l	4(sp),D0

DPutChar:
		jsr	PRawPutChar
		rts


******* ddebug.lib/DPutStr *******************************************
*
*   NAME
*	DPutStr - put a string to the parallel port
*
*   SYNOPSIS
*	DPutStr(string)
*	       A0
*
*   FUNCTION
*	put a null terminated string to the parallel port.
*
**********************************************************************

   xdef	_dputstr
_dputstr:

   xdef	_dputs
_dputs:

   xdef	_DPutS
_DPutS:

   xdef	_DPutStr			* (string)
_DPutStr:
	move.l	4(sp),A0

DPutStr:
ps0:	    MOVE.B  (A0)+,D0	    ; next character
	    BEQ.S   ps1		    ; done with string
	    BSR.S   DPutChar
	    BRA.S   ps0
ps1:
	    RTS



******* ddebug.lib/DGetChar ******************************************
*
*   NAME
*	DGetChar - get a character from the parallel port
*
*   SYNOPSIS
*	char = DGetChar()
*	D0
*
*   FUNCTION
*	get the next character from the parallel port.
*
**********************************************************************

   xdef	_dgetchar
_dgetchar:

   xdef	_dgetch
_dgetch:

   xdef	_DGetCh
_DGetCh:

   xdef	_dgetc
_dgetc:

   xdef	_DGetChar
_DGetChar:

DGetChar:
	    BSR.S    DMayGetChar
	    TST.L    D0
	    BMI.S   DGetChar
	    RTS


******* ddebug.lib/DMayGetChar ***************************************
*
*   NAME
*	DMayGetChar - return a char iff present, but don't block
*
*   SYNOPSIS
*	flagChar = DMayGetChar()
*	D0
*
*   FUNCTION
*	return either a -1, saying that there is no char present, or
*	the char that was waiting
*
**********************************************************************
   xdef	_DMayGetCh
_DMayGetCh:

   xdef	_DMayGetChar
_DMayGetChar:

DMayGetChar:
		jsr	PRawMayGetChar
		rts


******* ddebug.lib/DPutFmt *******************************************
*
*   NAME
*	DPutFmt - print formatted data to the parallel port
*
*   SYNOPSIS
*	DPutFmt(format,values)
*	       A0     A1
*
*   FUNCTION
*	print formatted data to the parallel port
*
**********************************************************************


   xdef	_DPutFmt			* (format,valueArray)
_DPutFmt:
		move.l	4(sp),A0
		move.l	8(sp),A1
		bra.s   DPutFmt

   xdef	_dprintf
_dprintf:

   xdef	_DPrintF
_DPrintF:
		move.l	4(sp),a0            * format string 
		lea	8(sp),a1            * values to print 
DPutFmt:
	    MOVEM.L A2,-(SP)
	    LEA	    DPutChar,A2
	    BSR.S   DDoFmt
	    MOVEM.L (SP)+,A2
	    RTS


******* ddebug.lib/DDoFmt ********************************************
*
*   NAME
*	DDoFmt -- format data into a character stream.
*
*   SYNOPSIS
*	DDoFmt(FormatString, DataStream, PutChProc, PutChData);
*	      A0	    A1		A2	   A3
*
*   FUNCTION
*	perform "C"-language-like formatting of a data stream,
*	outputting the result a character at a time
*
*   INPUTS
*	FormatString - a "C"-language-like null terminated format
*		string, with the following supported % types:
*	DataStream - a stream of data that is interpreted according to
*		the format string.
*	PutChProc - the procedure to call with each character to be
*		output, called as:
*	    PutChProc(Char,  PutChData);
*		      D0-0:8 A3
*		the procedure is called with a null Char at the end of
*		the format string.
*	PutChData - an address register that passes thru to PutChProc.
*
**********************************************************************

DDoFmt:
		jsr	PRawDoFmt
		rts

   xdef	_DDoFmt			* ()
_DDoFmt:
		movem.l	A2/A3,-(sp)
		movem.l	12(sp),A0/A1/A2/A3
		bsr.s	DDoFmt
		movem.l	(SP)+,A2/A3
		rts

	END

