
************************************************************************
*
*	Naked (No DOS) C Program Startup and Exit
*
************************************************************************
*
*   Source Control:
*
*       $Id: nodos_startup.asm,v 27.3 90/06/01 23:17:57 jesup Exp $
*
*       $Locker:  $
*
*       $Log:	nodos_startup.asm,v $
* Revision 27.3  90/06/01  23:17:57  jesup
* Conform to include standard du jour
* 
* Revision 27.2  89/04/27  23:39:59  jesup
* fixed autodocs
* 
* Revision 27.1  85/06/24  13:40:23  neil
* Upgrade to V27
* 
*	Revision 25.0  85/06/15  14:37:27  carl
*	Placed under source control.
*	
*
************************************************************************


****** Included Files *************************************************



****** Imported *******************************************************

	xref	_main			; C code entry point


****** Exported *******************************************************

	xdef	_SysBase

	xdef	_exit			; standard C exit function


_SysBase	EQU	4

************************************************************************
*
*	Standard Program Entry Point
*
************************************************************************

*		move.l	sp,initialSP	; initial task stack pointer

	;------ call C main entry point
		jsr	_main

	;------ return success code:
		moveq.l	#0,D0
		rts


************************************************************************
*
*	C Program Exit Function
*
************************************************************************

_exit:
		move.l	4(sp),d0	; extract return code
*		move.l	initialSP,sp	; restore stack ptr
		rts


************************************************************************

	DATA

************************************************************************

	END
