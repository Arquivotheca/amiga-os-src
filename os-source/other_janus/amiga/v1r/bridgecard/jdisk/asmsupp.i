
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




XLVO		MACRO
		XREF	_LVO\1
		ENDM

CALLLVO 	MACRO
		jsr	_LVO\1(a6)
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

PUTMSG		MACRO
		IFGE	INFOLEVEL-\1
		XREF	KPutFmt
		XREF	JDiskName
		PEA	JDiskName
		MOVEM.L D0/D1/A0/A1,-(SP)
		LEA	16(sp),a1
		LEA	MSG\@,a0
		JSR	KPutFmt
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



		ENDC	!ASMSUPP_I

