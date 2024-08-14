	INCLUDE "exec/types.i"
	INCLUDE "exec/nodes.i"
	INCLUDE "exec/libraries.i"

	INCLUDE "macros.i"
     
	XDEF	_main

  
;------ Imported Functions -------------------------------------------

	XLVO	AddResource
	XLVO	AllocSignal
	XLVO	MakeLibrary
	XLVO	Wait

	XREF	_SysBase
	XREF	KPutFmt


;--------------------------------------------------------------
_main:	 
		movem.l d2/a2/a6,-(a7)

		;------ get signal to wait on forever
		moveq	#-1,d0
		move.l	_SysBase,a6
		CALLLVO AllocSignal
		move.l	d0,d2
		bmi.s	exit

		;------ make resource for diagnostic functions
		LEA	initialFunctions(PC),A0
		suba.l	A1,A1		    * no initial data
		suba.l	A2,A2		    * no initializer
		MOVEQ	#LIB_SIZE,D0
		CALLLVO MakeLibrary

		tst.l	d0
		beq.s	exit
		MOVE.L	D0,a1
		MOVE.B	#NT_RESOURCE,LN_TYPE(a1)
		CLR.B	LN_PRI(a1)
		MOVE.L	#DRName,LN_NAME(a1)

		;------ tell system about resource
		CALLLVO AddResource

		;------ hand around forever
		moveq	#0,d0
		bset	d2,d0
		CALLLVO Wait 

exit:
		movem.l (a7)+,d2/a2/a6
		rts

;--------------------------------------------------------------

initialFunctions:
		DC.L	KPutFmt
		DC.L	-1

DRName:
		DC.B	'diag.resource',0
		DS.W	0

	END

