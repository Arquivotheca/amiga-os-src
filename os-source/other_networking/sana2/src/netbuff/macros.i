XSYS		MACRO
		XREF	_LVO\1
		ENDM

CALLSYS		MACRO
		jsr	_LVO\1(a6)
		ENDM

LINKSYS		MACRO	* &func
		LINKLIB	_LVO\1,NBL_SYSLIB(a6)
		ENDM

NB_DISABLE	MACRO	scratch,from
		move.l	\2,\1
		DISABLE_V36	\1,NOFETCH
		ENDM

NB_ENABLE	MACRO	scratch,from
		move.l	\2,\1
		ENABLE_V36	\1,NOFETCH
		ENDM



; The following was lifted from the V36 exec/ables.i include file and
; slightly changed.


;Disable interrupts.  Avoid use of DISABLE if at all possible.
;Please realize the danger of this macro!  Don't disable for long periods!
DISABLE_V36	MACRO		; [scratchReg],[NOFETCH] or have ExecBase in A6.
		IFC		'\1',''		;Case 1: Assume A6=ExecBase
		  move.w	#$04000,_intena	;(NOT IF_SETCLR)+IF_INTEN
		  addq.b	#1,IDNestCnt(A6)
		  MEXIT
		ENDC
		IFC		'\2','NOFETCH'	;Case 2: Assume \1=ExecBase
		  move.w	#$04000,_intena
		  addq.b	#1,IDNestCnt(\1)
		  MEXIT
		ENDC
		IFNC		'\1',''		;Case 3: Use \1 as scratch
		  move.l	4,\1		;Get ExecBase
		  move.w	#$04000,_intena
		  addq.b	#1,IDNestCnt(\1)
		  MEXIT
		ENDC
		ENDM

;Enable interrupts.  Please realize the danger of this macro!
ENABLE_V36	MACRO		; [scratchReg],[NOFETCH] or have ExecBase in A6.
		IFC		'\1',''		;Case 1: Assume A6=ExecBase
		  subq.b	#1,IDNestCnt(a6)
		  bge.s		ENABLE_V36\@
		  move.w	#$0C000,_intena	;IF_SETCLR+IF_INTEN
ENABLE_V36\@:
		  MEXIT
		ENDC
		IFC		'\2','NOFETCH'	;Case 2: Assume \1=ExecBase
		  subq.b	#1,IDNestCnt(\1)
		  bge.s		ENABLE_V36\@
		  move.w	#$0C000,_intena
ENABLE_V36\@:
		  MEXIT
		ENDC
		IFNC		'\1',''		;Case 3: Use \1 as scratch
		  move.l	4,\1		;Get ExecBase
		  subq.b	#1,IDNestCnt(\1)
		  bge.s		ENABLE_V36\@
		  move.w	#$0C000,_intena
ENABLE_V36\@:
		  MEXIT
		ENDC
		ENDM
