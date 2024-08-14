
*   intuileap.asm
*
*  $Id: intuileap.asm,v 40.0 94/02/15 17:42:47 davidj Exp $
*
*   This is the IntuitionLeap routine
*   It's the interface between the input.task of the Input Stream Merger
*   and Intuition() itself
*
*   This routine expects a pointer to an InputEvent in A0, and some
*   as of yet unspecified data in A1.  It transfers these to the stack
*   for the C interfacing, and then calls Intuition() which currently
*   wants only the InputEvent, but someday might want both

    XREF	_IntuitionHandler

    XDEF	_IntuiLeap
_IntuiLeap:
    MOVEM.L	A0/A6,-(A7)
    MOVE.L	A1,A6
    JSR		_IntuitionHandler
    MOVEM.L	(A7)+,A0/A6
    RTS

    END

