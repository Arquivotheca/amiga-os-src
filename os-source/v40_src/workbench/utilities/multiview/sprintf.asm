*** sprintf.asm **********************************************************
*
*   sprintf.asm	- 	Simple sprintf() based on exec/RawDoFmt()
*
*   Copyright 1989, 1990, Commodore-Amiga, Inc.
*
*   $Id: sprintf.asm,v 39.0 92/06/02 08:54:18 davidj Exp $
*
*   $Log:	sprintf.asm,v $
* Revision 39.0  92/06/02  08:54:18  davidj
* initial RCS
* 
* Revision 1.1  90/07/02  11:45:38  davidj
* Initial revision
* 
*   Revision 36.1  90/03/16  14:05:07  peter
*   RCS Clean up.
*   
*
**************************************************************************

	XDEF _sprintf
	XREF _AbsExecBase
	XREF _LVORawDoFmt

_sprintf:	; ( ostring, format, {values} )
	movem.l a2/a3/a6,-(sp)

	move.l	4*4(sp),a3       ;Get the output string pointer
	move.l	5*4(sp),a0       ;Get the FormatString pointer
	lea.l	6*4(sp),a1       ;Get the pointer to the DataStream
	lea.l	stuffChar(pc),a2
	move.l	_AbsExecBase,a6
	jsr	_LVORawDoFmt(a6)

	movem.l (sp)+,a2/a3/a6
	rts

;------ PutChProc function used by RawDoFmt -----------
stuffChar:
	move.b	d0,(a3)+        ;Put data to output string
	rts
