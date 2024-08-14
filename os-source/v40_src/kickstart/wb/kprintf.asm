*************************************************************************
*
* $Id: kprintf.asm,v 38.1 91/06/24 11:36:53 mks Exp $
*
* $Log:	kprintf.asm,v $
* Revision 38.1  91/06/24  11:36:53  mks
* Initial V38 tree checkin - Log file stripped
* 
*************************************************************************

	SECTION	section

;	Included Files

	INCLUDE 'exec/types.i'
	INCLUDE	'workbench.i'
	INCLUDE	'workbenchbase.i'

	INCLUDE	'asmsupp.i'

;	Imported Names

*------ Data ---------------------------------------------------------


*------ Functions ----------------------------------------------------

	XREF	KPutFmt

*------ Offsets ------------------------------------------------------

;	Exported Names

*------ Functions ----------------------------------------------------


*------ Data ---------------------------------------------------------

;	Local Definitions

	XDEF	_kprintf

_kprintf:
	MOVE.B	wb_Flags2(WBBASE),D0
	BTST	#WB2B_KPrintfOK,D0
	BEQ.S	kprintf_end

	MOVE.L	4(SP),A0
	LEA	8(SP),A1
	JSR    KPutFmt

kprintf_end:
	RTS


        end
