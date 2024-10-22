**
**  init.asm  -  Open libraries...
**

	SECTION		printer

*------ Included Files -----------------------------------------------

	INCLUDE		"exec/types.i"
	INCLUDE		"exec/nodes.i"
	INCLUDE		"exec/lists.i"
	INCLUDE		"exec/memory.i"
	INCLUDE		"exec/ports.i"
	INCLUDE		"exec/libraries.i"

	INCLUDE		"macros.i"

*------ Imported Functions -------------------------------------------

	XREF_EXE	CloseLibrary
	XREF_EXE	OpenLibrary
	XREF		_AbsExecBase

	XREF		_PEDData

*------ Exported Globals ---------------------------------------------

	XDEF		_Init
	XDEF		_Expunge
	XDEF		_PD
	XDEF		_PED
	XDEF		_SysBase
	XDEF		_DOSBase
	XDEF		_GfxBase
	XDEF		_IntuitionBase
	XDEF		_JCCBase

**********************************************************************

	SECTION		printer,DATA

_PD				DC.L	0
_PED			DC.L	0
_SysBase		DC.L	0
_DOSBase		DC.L	0
_GfxBase		DC.L	0
_IntuitionBase	DC.L	0
_JCCBase		DC.L	0

**********************************************************************

	SECTION		printer,CODE

_Init:
		move.l	4(A7),_PD
		lea		_PEDData(PC),A0
		move.l	A0,_PED
		move.l	A6,-(A7)
		move.l	_AbsExecBase,A6
		move.l	A6,_SysBase

*	    ;------ open the dos library
		lea		DLName(PC),A1
		moveq	#0,D0
		CALLEXE OpenLibrary
		move.l	D0,_DOSBase
		beq		initDLErr

*	    ;------ open the graphics library
		lea		GLName(PC),A1
		moveq	#0,D0
		CALLEXE OpenLibrary
		move.l	D0,_GfxBase
		beq		initGLErr

*	    ;------ open the intuition library
		lea		ILName(PC),A1
		moveq	#0,D0
		CALLEXE OpenLibrary
		move.l	D0,_IntuitionBase
		beq		initILErr

*	    ;------ open the jcc library
		lea		JCName(PC),A1
		moveq	#0,D0
		CALLEXE OpenLibrary
		move.l	D0,_JCCBase
		beq		initJCErr

		moveq	#0,D0
pdiRts:
		move.l	(A7)+,A6
		rts

initErr:
		move.l	_JCCBase,A1
		LINKEXE	CloseLibrary

initJCErr:
		move.l	_IntuitionBase,A1
		LINKEXE	CloseLibrary

initILErr:
		move.l	_GfxBase,A1
		LINKEXE	CloseLibrary

initGLErr:
		move.l	_DOSBase,A1
		LINKEXE	CloseLibrary

initDLErr:
		moveq	#-1,D0
		bra.s	pdiRts

JCName:
		DC.B	'jcc.library'
		DC.B	0
ILName:
		DC.B	'intuition.library'
		DC.B	0
DLName:
		DC.B	'dos.library'
		DC.B	0
GLName:
		DC.B	'graphics.library'
		DC.B	0
		DS.W	0

*---------------------------------------------------------------------
_Expunge:
		move.l	_JCCBase,A1
		LINKEXE	CloseLibrary

		move.l	_IntuitionBase,A1
		LINKEXE	CloseLibrary

		move.l	_GfxBase,A1
		LINKEXE	CloseLibrary

		move.l	_DOSBase,A1
		LINKEXE	CloseLibrary

		moveq	#0,D0
		rts

		END
