		NOLIST
		IFND	MY_MACROS_ASM
MY_MACROS_ASM	SET	1
*==========================================================================
*=====  MACROS I LIKE TO USE A LOT ========================================
*==========================================================================

* For declaring external system calls
EXT_SYS		MACRO
		XREF	_LVO\1
		ENDM

* For calling system routines by name (eg. SYS  OpenWindow(A6) )
SYS		MACRO
		JSR	_LVO\1
		ENDM

*==========================================================================

* For fetching the exec pointer into A6
GET_EXECPTR	MACRO
		MOVEA.L	_AbsExecBase,A6
		ENDM

* For fetching the intuition pointer into A6 (Requires IntPtr to be defined)
GET_INTPTR	MACRO
		MOVEA.L	IntPtr,A6
		ENDM

* For fetching the graphics pointer into A6 (Requires GfxPtr to be defined)
GET_GFXPTR	MACRO
		MOVEA.L	GfxPtr,A6
		ENDM

* For fetching the dos pointer into A6 (Requires DosPtr to be defined)
GET_DOSPTR	MACRO
		MOVEA.L	DosPtr,A6
		ENDM

*=========================================================================

* For fetching a structure member (GET_MEMBER .W,screen,sc_Width,A0,D0)
* in this example, screen is a longword containing the address of the struct
* A0 is the address register to use and D0 is the destination (can be addr)
GET_MEMBER	MACRO
		MOVEA.L	\2,\4
		MOVE\1	\3(\4),\5
		ENDM

* For fetching address of a structure member (GET_ADDR  screen,sc_BitMap,A0)
* in this example, screen is a longword containing the address of the struct
* A0 is the address register to use for the final address
GET_ADDR	MACRO
		MOVEA.L	\1,\3
		ADDA.L	#\2,\3
		ENDM

*==========================================================================
* For debugging purposes - provides a printf statement for machine code
* Note, arguments for printf must be put in the order in which they
* appear in the format string. there is currently a limit of 5 arguments
*
* eg.	PRINTF	<'num1=%lx num2=%x num3=%d\n'>, D0, .L, D1, .W, D3, .W
*
* As assembler has no type casting, each argument must be followed by its
* size - .B, .W, or .L
* Requires the symbol 'Debug' to be defined for macro expansion to occur
* Also requires linking with the module print.obj to provide Printf call
*==========================================================================

PRINTF		MACRO
	IFD	Debug
		MOVE.L	A7,stk\@
		NOLIST

		IFGE	NARG-11
		LIST
		MOVE\11	\10,-(A7)
		NOLIST
		ENDC

		IFGE	NARG-9
		LIST
		MOVE\9	\8,-(A7)
		NOLIST
		ENDC

		IFGE	NARG-7
		LIST
		MOVE\7	\6,-(A7)
		NOLIST
		ENDC

		IFGE	NARG-5
		LIST
		MOVE\5	\4,-(A7)
		NOLIST
		ENDC

		IFGE	NARG-3
		LIST
		MOVE\3	\2,-(A7)
		NOLIST
		ENDC

		LIST
		PEA.L	s\@
		JSR	Printf
		MOVEA.L	stk\@,A7
		DATA
		CNOP	0,4
stk\@		DC.L	0
s\@		DC.B	\1,0
		CNOP	0,2
		CODE
	ENDC
		ENDM

*==========================

		ENDC
		LIST
