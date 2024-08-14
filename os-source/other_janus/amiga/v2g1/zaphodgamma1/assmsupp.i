
;***********************************************************************
;
; assmsupp.i, the supp for Manx Aztec Assembler
;
; Copyright (c) 1986, Commodore Amiga Inc., All rights reserved
;
;***********************************************************************


XLIB		MACRO
		PUBLIC	_LVO\1
		ENDM

CALLSYS 	MACRO
		CALLLIB _LVO\1
		ENDM

LINKSYS 	MACRO
		LINKLIB _LVO\1,\2
		ENDM

LINKEXE 	MACRO
		PUBLIC	SysBaseOffset
		LINKSYS \1,SysBaseOffset(a6)
		ENDM

CLEAR		MACRO
		MOVEQ	#0,\1
		ENDM

PUTMSG		MACRO
		IFGE	INFOLEVEL-\1
		PUBLIC	KPutFmt
		MOVEM.L D0/D1/A0/A1,-(SP)
		LEA	16(sp),a1
		LEA	msg\@,a0
		JSR	KPutFmt
		MOVEM.L (SP)+,D0/D1/A0/A1
		BRA	end\@
msg\@:		DC.B	\2,10,0
		DS.W	0
end\@:
		ENDC
		ENDM

SPUTMSG 	MACRO
		IFGE	INFOLEVEL-\1
		PUBLIC	KPutFmt
		PUBLIC	_SubsysName
		PEA	_SubsysName
		MOVEM.L D0/D1/A0/A1,-(SP)
		LEA	16(sp),a1
		LEA	msg\@,a0
		JSR	KPutFmt
		MOVEM.L (SP)+,D0/D1/A0/A1
		ADDQ.L	#4,SP
		BRA	end\@
msg\@:		DC.B	\2,10,0
		DS.W	0
end\@:
		ENDC
		ENDM

* Here's an example of using the PUTMSG macros
*   IFGE    INFOLEVEL-50
*   SPUTMSG  50,<'[%s]ZText:'>
*   MOVEM.L A0-A6,-(SP)
*   PUTMSG  50,<'  A0:%lx 1:%lx 2:%lx 3:%lx 4:%lx 5:%lx 6:%lx'>
*   LEA     7*4(SP),SP
*   MOVEM.L D0-D7,-(SP)
*   PUTMSG  50,<'  D0:%lx 1:%lx 2:%lx 3:%lx 4:%lx 5:%lx 6:%lx 7:%lx'>
*   LEA     8*4(SP),SP
*   ENDC


