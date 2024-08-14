
;***********************************************************************
;
; asmsupp.i
;
; Copyright (c) 1986, Commodore Amiga Inc., All rights reserved
;
;***********************************************************************



		IFND	ASMSUPP_I
ASMSUPP_I	SET	1


		IFND	EXEC_LIBRARIES_I
		INCLUDE "exec/libraries.i"
		ENDC





XLIB		MACRO
		XREF	_LVO\1
		ENDM

CALLSYS 	MACRO
		CALLLIB _LVO\1
		ENDM

LINKSYS 	MACRO
		LINKLIB _LVO\1,\2
		ENDM

LINKEXE 	MACRO
		XREF	SysBaseOffset
		LINKSYS \1,SysBaseOffset(a6)
		ENDM

CLEAR		MACRO
		MOVEQ	#0,\1
		ENDM

BHS		MACRO
		IFC	'\0',''
			BCC	\1
		ENDC
		IFNC	'\0',''
			BCC.\0	\1
		ENDC
		ENDM

BLO		MACRO
		IFC	'\0',''
			BCS	\1
		ENDC
		IFNC	'\0',''
			BCS.\0	\1
		ENDC
		ENDM
bhs		MACRO
		IFC	'\0',''
			BCC	\1
		ENDC
		IFNC	'\0',''
			BCC.\0	\1
		ENDC
		ENDM

blo		MACRO
		IFC	'\0',''
			BCS	\1
		ENDC
		IFNC	'\0',''
			BCS.\0	\1
		ENDC
		ENDM

PUTMSG		MACRO
		IFGE	INFOLEVEL-\1
		XREF	KPutFmt
		XREF	subsysName
		PEA	subsysName
		MOVEM.L D0/D1/A0/A1,-(SP)
	EXECLIB	Forbid
		LEA	16(sp),a1
		LEA	MSG\@,a0
		JSR	KPutFmt
	EXECLIB	Permit
		MOVEM.L (SP)+,D0/D1/A0/A1
		ADDQ.L	#4,SP
		SECTION section,data
MSG\@		DC.B	\2,10,0
		DS.W	0
		SECTION section,code
		ENDC
		ENDM

INFOMSG 	MACRO
		IFGE	INFOLEVEL-\1
		XREF	KPutFmt
		MOVEM.L D0/D1/A0/A1,-(SP)
		LEA	16(sp),a1
		LEA	MSG\@,a0
		JSR	KPutFmt
		MOVEM.L (SP)+,D0/D1/A0/A1
		SECTION section,data
MSG\@		DC.B	\2,10,0
		DS.W	0
		SECTION section,code
		ENDC
		ENDM

LINEMSG 	MACRO
		IFGE	INFOLEVEL-\1
		XREF	KPutFmt
		MOVEM.L D0/D1/A0/A1,-(SP)
		LEA	16(sp),a1
		LEA	MSG\@,a0
		JSR	KPutFmt
		MOVEM.L (SP)+,D0/D1/A0/A1
		SECTION section,data
MSG\@		DC.B	\2,0
		DS.W	0
		SECTION section,code
		ENDC
		ENDM


EXECLIB		MACRO
		XLIB	\1
		MOVEM.L	A0-A1/A6/D0-D1,-(SP)
		MOVEA.L	4,A6
		CALLSYS	\1
		MOVEM.L	(SP)+,A0-A1/A6/D0-D1
		ENDM


		ENDC	!ASMSUPP_I

