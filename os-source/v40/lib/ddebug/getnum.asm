
	SECTION DGetNum
	INCLUDE "assembly.i"

******* ddebug.lib/DGetNum **********************************************
*
*   NAME
*	DGetNum - get a number from the parallel port
*
*   SYNOPSIS
*	number = DGetNum()
*	D0
*
*   FUNCTION
*	get a signed decimal integer from the parallel port.
*
**********************************************************************

   xdef _dgetnum
_dgetnum:

   xdef _DGetNum
_DGetNum:

	xref DGetChar
	xref DPutChar
	xdef DGetNum
DGetNum:
	    MOVEM.L D2-D4,-(SP)
gn0:	    CLEAR   D2
	    CLEAR   D3
	    CLEAR   D4
	    JSR	    DGetChar
	    CMPI.B   #'-',D0
	    BNE.S   gn2
	    JSR	    DPutChar
	    ADDQ.W  #1,D3
	    MOVEQ   #-1,D4
	    BRA.S   gn2
gn1:	    JSR	    DGetChar
gn2:	    CMPI.B  #8,D0
	    BNE.S   gn3
	    TST.W   D3
	    BEQ.S   gn0
	    JSR	    DPutChar
	    MOVEQ   #' ',D0
	    JSR	    DPutChar
	    MOVEQ   #8,D0
	    JSR	    DPutChar
	    SUBQ.W  #1,D3
	    BEQ.S   gn0
	    SWAP    D2
	    CLEAR   D1
	    MOVE.W  D2,D1
	    DIVU    #10,D1
	    SWAP    D1
	    MOVE.W  D1,D2
	    SWAP    D2
	    DIVU    #10,D2
	    MOVE.W  D2,D1
	    MOVE.L  D1,D2
gn3:	    CMPI.B  #'0',D0
	    BCS.S   gn4
	    CMPI.B  #'9',D0
	    BHI.S   gn4
	    MOVE.L  D2,D1
	    ASL.L   #2,D1
	    BVS.S   gn1
	    ADD.L   D2,D1
	    BVS.S   gn1
	    ASL.L   #1,D1
	    SUBI.B  #'0',D0
	    ADD.L   D0,D1
	    BCS.S   gn1
	    ADDI.B  #'0',D0
	    MOVE.L  D1,D2
	    JSR     DPutChar
	    ADDQ.W  #1,D3
gn4:	    CMPI.B  #13,D0
	    BNE.S   gn1
gn5:	    TST.B   D4
	    BEQ.S   gn6
	    NEG.L   D2
gn6:	    MOVE.L  D2,D0
	    MOVEM.L (SP)+,D2-D4
	    RTS

	END
